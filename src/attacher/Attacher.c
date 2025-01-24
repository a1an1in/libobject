/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */

#include <libobject/attacher/Attacher.h>

extern void *attacher_malloc(int size);
extern int attacher_free(void *addr);

static int tree_node_free_callback(allocator_t *allocator, void *value)
{
    interval_tree_node_t *t = (interval_tree_node_t *)value;
    allocator_mem_free(allocator, t->value);
    allocator_mem_free(allocator, t);
    return 0;
}

static int __construct(Attacher *attacher, char *init_str)
{
    Interval_Tree *tree;

    attacher->map = object_new(attacher->parent.allocator, "RBTree_Map", NULL);
    attacher->map->set_cmp_func(attacher->map, string_key_cmp_func);

    attacher->tree = object_new(attacher->parent.allocator, "Interval_Tree", NULL);
    tree = attacher->tree;
    tree->customize(tree, VALUE_TYPE_STRUCT_POINTER, tree_node_free_callback);

    return 0;
}

static int __deconstrcut(Attacher *attacher)
{
    Map *map = (Map *)attacher->map;
    allocator_t *allocator = attacher->parent.allocator;
    Iterator *cur, *end;
    void *key, *value;

    /* 1. remove libs*/
    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        dbg_str(DB_VIP, "attacher destroy, release lib:%s", key);
        attacher->remove_lib(attacher, key);
        allocator_mem_free(allocator, key);
    }

    /* 2. destroy map*/
    object_destroy(map);
    object_destroy(attacher->tree);

    /* 3. detach */
    if (attacher->pid != 0) {
        attacher->detach(attacher);
    }

    return 0;
}

static void *__malloc(Attacher *attacher, int size, void *value)
{
    int ret;
    void *addr;
    long long pars[1] = {size};

    TRY {
        THROW_IF(size == 0, 0);
        dbg_str(DBG_VIP, "local attacher_malloc addr:%p", attacher_malloc);
        addr = attacher->get_remote_builtin_function_address(attacher, "attacher_malloc", "libattacher-builtin.so");
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher_malloc addr:%p", addr);
        addr = attacher->call_address_with_value_pars(attacher, addr, pars, 1);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher malloc addr:%p, size:%d", addr, size);
        if (value != NULL) {
            EXEC(attacher->write(attacher, addr, value, size));
        }
        return addr;
    } CATCH (ret) { ret = NULL; }

    return ret;
}

static int __free(Attacher *attacher, void *addr)
{
    int ret;
    void *free_addr;
    long long pars[1] = {addr};

    TRY {
        THROW_IF(addr == 0, -1);
        free_addr = attacher->get_remote_builtin_function_address(attacher, "attacher_free", "libattacher-builtin.so");
        THROW_IF(free_addr == NULL, -1);
        dbg_str(DBG_VIP, "free func addr:%p, free addr:%p", free_addr, addr);
        ret = attacher->call_address_with_value_pars(attacher, free_addr, pars, 1);
        THROW(ret);
    } CATCH (ret) {}

    return ret;
}

static void *__get_remote_function_address(Attacher *attacher, char *lib_name, char *name)
{
    long ret;
    void *addr;
    char module_name[64] = {0};
    attacher_paramater_t pars[2] = {{name, strlen(name)}, {lib_name, lib_name == NULL ? 0 : strlen(lib_name)}};

    TRY {
        addr = dl_get_func_addr_by_name(name, NULL);
        if (addr != NULL) { // builtin地址
            // EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            EXEC(dl_get_dynamic_name(-1, addr, module_name, 64));
            addr = attacher->get_remote_builtin_function_address(attacher, name, module_name);
        } else { // 远端自己的地址
            addr = attacher->get_remote_builtin_function_address(attacher, "attacher_get_func_addr_by_name", "libattacher-builtin.so");
            addr = attacher->call_address(attacher, addr, pars, 2);
        }
        
        THROW_IF(addr == NULL, -1);
    } CATCH (ret) {
        addr = NULL;
    }

    return addr;
}

/* name:call_address
 *
 * description:
 * this funtion can call function with remote address. if we know the remote address
 * we can call this method.
 **/
static long __call_address(Attacher *attacher, void *function_address, attacher_paramater_t pars[], int num)
{
    long ret, stat, i;
    int pointer_flag = 0;
    void *paramters[ATTACHER_PARAMATER_ARRAY_MAX_SIZE];

    TRY {
        THROW_IF(num > ATTACHER_PARAMATER_ARRAY_MAX_SIZE, -1);

        /* retmote funtion args pre process*/
        for (i = 0; i < num; i++) {
            if (pars[i].size == 0) {
                paramters[i] = pars[i].value;
                continue;
            }
            /* We require size to be multiples of long because ptrace is written in units of long */
            if (pars[i].size % sizeof(long) != 0) {
                pars[i].size = (pars[i].size / sizeof(long) + 1) * sizeof(long);
                dbg_str(DBG_VIP, "call address prepare paramater %d, newsize:%d", i, pars[i].size);
            }
            
            paramters[i] = attacher->malloc(attacher, pars[i].size, pars[i].value);
            pointer_flag = 1;
        }

        ret = attacher->call_address_with_value_pars(attacher, function_address, paramters, num);
        if (pointer_flag == 0) {
            return ret;
        }

        /* retmote funtion args post process */
        for (i = 0; i < num; i++) {
            if (pars[i].size == 0) continue;
            attacher->free(attacher, paramters[i]);
        }
        return ret;
    } CATCH (ret) {}

    return ret;
}

/* 如果确定调用函数不是builtin，需要指定库名，builtin指的是xtools自带的库  */
static long __call(Attacher *attacher, char *lib_name, char *name, attacher_paramater_t pars[], int num)
{
    long ret;
    void *addr;
    char module_name[64] = {0};

    TRY {
        /* 1.get remote fuction address */
        addr = attacher->get_remote_function_address(attacher, lib_name, name);
        THROW_IF(addr == NULL, -1);
        
        /* 2.call */
        ret = attacher->call_address(attacher, addr, pars, num);
        return ret;
    } CATCH (ret) {}

    return ret;
}

static int __remove_lib(Attacher *attacher, char *name)
{
    int ret;
    void *handle = NULL;
    Map *map = ((Attacher *)attacher)->map;
    attacher_paramater_t pars[1] = {0};

    TRY {
        THROW_IF(name == NULL, -1);
        EXEC(map->remove(map, name, &handle));
        dbg_str(DBG_VIP, "attacher remove_lib, lib name:%s, handle:%p", name, handle);
        THROW_IF(handle == NULL, -1);
        pars[0].value = handle;
        EXEC(attacher->call(attacher, NULL, "attacher_dlclose", pars, 1));
    } CATCH (ret) {}

    return ret;
}

static stub_t *__alloc_stub(Attacher *attacher)
{
    return attacher->call(attacher, NULL, "stub_alloc", NULL, 0);
}

/* 如果有函数属于加载的库，当前不支持。 */
static int 
__add_stub_hooks(Attacher *attacher, stub_t *stub, char *func, char *pre, char *new_fn, 
                 char *post, int para_count)
{
    void *addr;
    char module_name[64] = {0};
    int ret;
    /* 如果size 为0表示是 value类型， 这时也可以传递地址 */
    attacher_paramater_t pars[6] = {{stub, 0}, {NULL, 0},
                                    {NULL, 0}, {NULL, 0},
                                    {NULL, 0}, {para_count, 0}};

    TRY {
        if (func != NULL) {
            addr = dl_get_func_addr_by_name(func, NULL);
            THROW_IF(addr == NULL, -1);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            addr = attacher->get_remote_builtin_function_address(attacher, func, module_name);
            pars[1].value = addr;
        }
        if (pre != NULL) {
            memset(module_name, 0, 64);
            addr = dl_get_func_addr_by_name(pre, NULL);
            THROW_IF(addr == NULL, -1);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            addr = attacher->get_remote_builtin_function_address(attacher, pre, module_name);
            pars[2].value = addr;
        }
        if (new_fn != NULL) {
            memset(module_name, 0, 64);
            addr = dl_get_func_addr_by_name(new_fn, NULL);
            THROW_IF(addr == NULL, -1);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            addr = attacher->get_remote_builtin_function_address(attacher, new_fn, module_name);
            pars[3].value = addr;
        }
        if (post != NULL) {
            memset(module_name, 0, 64);
            addr = dl_get_func_addr_by_name(post, NULL);
            THROW_IF(addr == NULL, -1);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            addr = attacher->get_remote_builtin_function_address(attacher, post, module_name);
            pars[4].value = addr;
        }
        dbg_str(DBG_INFO, "attacher add_stub_hooks, func:%p, pre:%p, target:%p, post:%p", 
                pars[1].value, pars[2].value, pars[3].value, pars[4].value);
        
        EXEC(attacher->call(attacher, NULL, "stub_add_hooks", pars, 6));
    } CATCH (ret) {} FINALLY {}
    
    return ret;
}

static int __remove_stub_hooks(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call(attacher, NULL, "stub_remove_hooks", pars, 1);
}

static int __free_stub(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call(attacher, NULL, "stub_free", pars, 1);
}

static int __init(Attacher *attacher)
{
    int ret;
    void *path_addr;
    allocator_t *allocator = attacher->parent.allocator;

    TRY {
        path_addr = allocator_mem_zalloc(allocator, 128);
        EXEC(dl_get_dynamic_lib_path(-1, "libobject-core.so", path_addr, 128));
        dbg_str(DBG_VIP, "path1:%s", path_addr);
        EXEC(attacher->add_lib(attacher, path_addr));
        path_addr = allocator_mem_zalloc(allocator, 128);
        EXEC(dl_get_dynamic_lib_path(-1, "libobject-stub.so", path_addr, 128));
        dbg_str(DBG_VIP, "path2:%s", path_addr);
        EXEC(attacher->add_lib(attacher, path_addr));
       
        EXEC(attacher->call(attacher, NULL, "execute_ctor_funcs", NULL, 0));
        EXEC(attacher->call(attacher, NULL, "stub_admin_init_default_instance", NULL, 0));

        /* 解析本地动态库地址区间， 后面方便根据本地地址查询对应的函数库 */
        EXEC(dl_parse_dynamic_table(-1, attacher->tree));
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry( 0, Obj, parent),
    Init_Nfunc_Entry( 1, Attacher, construct, __construct),
    Init_Nfunc_Entry( 2, Attacher, deconstruct, __deconstrcut),
    Init_Vfunc_Entry( 3, Attacher, attach, NULL),
    Init_Vfunc_Entry( 4, Attacher, detach, NULL),
    Init_Vfunc_Entry( 5, Attacher, get_remote_function_address, __get_remote_function_address),
    Init_Vfunc_Entry( 6, Attacher, get_remote_builtin_function_address, NULL),
    Init_Vfunc_Entry( 7, Attacher, read, NULL),
    Init_Vfunc_Entry( 8, Attacher, write, NULL),
    Init_Vfunc_Entry( 9, Attacher, malloc, __malloc),
    Init_Vfunc_Entry(10, Attacher, free, __free),
    Init_Vfunc_Entry(11, Attacher, call_address_with_value_pars, NULL),
    Init_Vfunc_Entry(12, Attacher, call_address, __call_address),
    Init_Vfunc_Entry(13, Attacher, call, __call),
    Init_Vfunc_Entry(14, Attacher, add_lib, NULL),
    Init_Vfunc_Entry(15, Attacher, remove_lib, __remove_lib),
    Init_Vfunc_Entry(16, Attacher, alloc_stub, __alloc_stub),
    Init_Vfunc_Entry(17, Attacher, add_stub_hooks, __add_stub_hooks),
    Init_Vfunc_Entry(18, Attacher, remove_stub_hooks, __remove_stub_hooks),
    Init_Vfunc_Entry(19, Attacher, free_stub, __free_stub),
    Init_Vfunc_Entry(20, Attacher, init, __init),
    Init_Vfunc_Entry(21, Attacher, run, NULL),
    Init_Vfunc_Entry(22, Attacher, stop, NULL),
    Init_End___Entry(23, Attacher),
};
REGISTER_CLASS(Attacher, attacher_class_info);