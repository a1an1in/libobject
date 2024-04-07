/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */

#include <libobject/attacher/Attacher.h>

extern void *my_malloc(int size);
extern int my_free(void *addr);

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

/* call_address is using malloc and free interfaces, we can't implement it
 * with call_from_lib. so, this func is irreplacable.
 */
static void *__malloc(Attacher *attacher, int size, void *value)
{
    int ret;
    void *addr;
    long long pars[1] = {size};

    TRY {
        THROW_IF(size == 0, 0);
        // addr = attacher->get_function_address(attacher, malloc, "libc.so");
        addr = attacher->get_function_address(attacher, my_malloc, "libobject-testlib.so");
        THROW_IF(addr == NULL, -1);
        addr = attacher->call_address_with_value_pars(attacher, addr, pars, 1);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "attacher malloc addr:%p, size:%d", addr, size);
        if (value != NULL) {
            EXEC(attacher->write(attacher, addr, value, size));
        }
        return addr;
    } CATCH (ret) {}

    return ret;
}

static int __free(Attacher *attacher, void *addr)
{
    int ret;
    void *free_addr;
    long long pars[1] = {addr};

    TRY {
        THROW_IF(addr == 0, -1);
        // free_addr = attacher->get_function_address(attacher, my_free, "libc.so");
        free_addr = attacher->get_function_address(attacher, my_free, "libobject-testlib.so");
        THROW_IF(free_addr == NULL, -1);
        dbg_str(DBG_VIP, "free func addr:%p, free addr:%p", free_addr, addr);
        ret = attacher->call_address_with_value_pars(attacher, free_addr, pars, 1);
        THROW(ret);
    } CATCH (ret) {}

    return ret;
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

/* 
 * name: function name want to call.
 * module_name: only need module name, don't need module path.
 *
 * description:
 * if only we know the func name and which lib this func is, we can call this method. 
 **/

static long __call_from_lib(Attacher *attacher, char *name, attacher_paramater_t pars[], int num, char *module_name)
{
    long ret;
    void *addr;

    TRY {
        /* get local fuction address */
        addr = dl_get_func_addr_by_name(name);
        THROW_IF(addr == NULL, -1);
        /* get remote fuction address */
        addr = attacher->get_function_address(attacher, addr, module_name);
        THROW_IF(addr == NULL, -1);
        
        ret = attacher->call_address(attacher, addr, pars, num);
        printf("call from lib, func name:%s, func_addr:%p, ret:%lx\n", name, addr, ret);
        return ret;
    } CATCH (ret) {}

    return ret;
}

static long __call(Attacher *attacher, void *addr, attacher_paramater_t pars[], int num)
{
    long ret;
    char module_name[64] = {0};
    char func_name[64] = {0};

    TRY {
        /* 1.get module name */
        dbg_str(DBG_VIP, "call addr:%p", addr);
        // dl_get_dynamic_name(-1, addr, module_name, 64);  //old func for get dynamic lib name. it'll do searching every time.
        EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
        dbg_str(DBG_VIP, "module name:%s", module_name);

        /* 2. get funtion name */
        EXEC(dl_get_func_name_by_addr(addr, func_name, sizeof(func_name)));

        /* 3.get remote fuction address */
        addr = attacher->get_function_address(attacher, addr, module_name);
        THROW_IF(addr == NULL, -1);
        
        /* 4.call */
        ret = attacher->call_address(attacher, addr, pars, num);
        printf("call from lib, func name:%s, func_addr:%p, ret:%lx\n", func_name, addr, ret);
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
        EXEC(attacher->call_from_lib(attacher, "my_dlclose", pars, 1, "libobject-testlib.so"));
    } CATCH (ret) {}

    return ret;
}

static stub_t *__alloc_stub(Attacher *attacher)
{
    return attacher->call_from_lib(attacher, "stub_alloc", NULL, 0, "libobject-stub.so");
}

static int 
__add_stub_hooks(Attacher *attacher, stub_t *stub, void *func, void *pre, void *new_fn, 
                 void *post, int para_count)
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
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, func, module_name, 64));
            func = attacher->get_function_address(attacher, func, module_name);
            printf("stub func addr:%p\n", func);
            pars[1].value = func;
        }
        if (pre != NULL) {
            memset(module_name, 0, 64);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, pre, module_name, 64));
            pre = attacher->get_function_address(attacher, pre, module_name);
            pars[2].value = pre;
        }
        if (new_fn != NULL) {
            memset(module_name, 0, 64);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, new_fn, module_name, 64));
            new_fn = attacher->get_function_address(attacher, new_fn, module_name);
            pars[3].value = new_fn;
        }
        if (post != NULL) {
            memset(module_name, 0, 64);
            EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, post, module_name, 64));
            post = attacher->get_function_address(attacher, post, module_name);
            pars[4].value = post;
        }
        
        EXEC(attacher->call_from_lib(attacher, "stub_add_hooks", pars, 6, "libobject-stub.so"));
    } CATCH (ret) {} FINALLY {}
    
    return ret;
}

static int __remove_stub_hooks(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call_from_lib(attacher, "stub_remove_hooks", pars, 1, "libobject-stub.so");
}

static int __free_stub(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call_from_lib(attacher, "stub_free", pars, 1, "libobject-stub.so");
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
       
        EXEC(attacher->call_from_lib(attacher, "execute_ctor_funcs", NULL, 0, "libobject-core.so"));
        EXEC(attacher->call_from_lib(attacher, "stub_admin_init_default_instance", NULL, 0, "libobject-stub.so"));
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
    Init_Vfunc_Entry( 5, Attacher, get_function_address, NULL),
    Init_Vfunc_Entry( 6, Attacher, read, NULL),
    Init_Vfunc_Entry( 7, Attacher, write, NULL),
    Init_Vfunc_Entry( 8, Attacher, malloc, __malloc),
    Init_Vfunc_Entry( 9, Attacher, free, __free),
    Init_Vfunc_Entry(10, Attacher, call_address_with_value_pars, NULL),
    Init_Vfunc_Entry(11, Attacher, call_address, __call_address),
    Init_Vfunc_Entry(12, Attacher, call_from_lib, __call_from_lib),
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
REGISTER_CLASS("Attacher", attacher_class_info);

