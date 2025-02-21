/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */
#include <libobject/core/utils/string.h>
#include <libobject/attacher/Attacher.h>

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
    char module_name[64] = {0};
    long long pars[1] = {size};

    TRY {
        THROW_IF(size == 0, 0);
        EXEC(dl_get_dynamic_name(-1, malloc, module_name, 64));
        dbg_str(DBG_INFO, "local malloc addr:%p", malloc);
        addr = attacher->get_remote_builtin_function_address(attacher, "malloc", module_name);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_INFO, "remote alloc addr:%p", addr);
        addr = attacher->call_address_with_value_pars(attacher, addr, pars, 1);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_INFO, "attacher malloc addr:%p, size:%d", addr, size);
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
    char module_name[64] = {0};
    long long pars[1] = {addr};

    TRY {
        THROW_IF(addr == 0, -1);
        EXEC(dl_get_dynamic_name(-1, free, module_name, 64));
        free_addr = attacher->get_remote_builtin_function_address(attacher, "free", module_name);
        THROW_IF(free_addr == NULL, -1);
        dbg_str(DBG_INFO, "free func addr:%p, free addr:%p", free_addr, addr);
        ret = attacher->call_address_with_value_pars(attacher, free_addr, pars, 1);
        THROW(ret);
    } CATCH (ret) {}

    return ret;
}

/* 远端地址有三种类型，1. 远端进程自带函数， 2. attacher加载的内置库函数， 3. attacher为target加载且自己没有的库。
 * 第一和第三种类型的地址需要传给个远端进程获取地址。第二种类型attacher可以自己计算远端地址。
 * 第三种类型需要传入lib名，远端进程才能获取到对应函数地址。
 */
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
        } else { // 远端函数或者动态加载非builtin lib函数地址。
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
                dbg_str(DBG_DETAIL, "call address prepare paramater %d, newsize:%d", i, pars[i].size);
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

/* 如果调用函数不是builtin，需要指定库名，builtin指的是xtools自带的库。
 * 如果函数是attacher为target加载且自己没有的库，则需要指定库名，否则target
 * 不能获取到函数名对应的地址。函数名为空只能查询进程自带函数的地址。
 */
static long __call_name(Attacher *attacher, char *lib_name, char *name, attacher_paramater_t pars[], int num)
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

static long __call(Attacher *attacher, char *lib_name, char *func, long *ret_value)
{
    long ret, cnt, i, len, return_value;
    allocator_t *allocator = attacher->parent.allocator;
    attacher_paramater_t pars[20];
    void *addr;
    char *method_name, *arg;
    String *str;

    TRY {
        dbg_str(DBG_WIP, "attacher call:%s", func);
        /* 1. 获取函数名和参数 */
        str = object_new(allocator, "String", func);
        cnt = str->split(str, "[,\t\n();]", -1);

        THROW_IF(cnt <= 0, 0);
        method_name = str->get_splited_cstr(str, 0);
        for (i = 1; i < cnt; i++) {
            arg = str->get_splited_cstr(str, i);
            arg = str_trim(arg);
            THROW_IF(arg == NULL, -1);

            if (arg[0] == '"')
            /* 参数加引号为表示字符串， 不加引号为数字或者变量名 */
            {
                len = strlen(arg);
                THROW_IF(arg[len - 1] != '"', -1);
                arg[len - 1] = '\0';
                pars[i - 1].value = arg + 1;
                pars[i - 1].size = strlen(pars[i - 1].value);
            } else if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) {
                pars[i - 1].value = str_hex_to_integer(arg);
                pars[i - 1].size = 0;
            } else if (arg[0] == '0' && (arg[1] == 'd' || arg[1] == 'D')) {
                pars[i - 1].value = atoi(arg + 2);
                pars[i - 1].size = 0;
            } else {
                pars[i - 1].value = atoi(arg);
                pars[i - 1].size = 0;
            }
        }

        /* 2.get remote fuction address */
        addr = attacher->get_remote_function_address(attacher, lib_name, method_name);
        THROW_IF(addr == NULL, -1);
        
        /* 3.call */
        return_value = attacher->call_address(attacher, addr, pars, cnt - 1);
        if (ret_value != NULL) {
            *ret_value = return_value;
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(str);
    }

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
        dbg_str(DBG_VIP, "attacher remove lib:%s, handle:%p", name, handle);
        THROW_IF(handle == NULL, -1);
        pars[0].value = handle;
        EXEC(attacher->call_name(attacher, NULL, "__libc_dlopen_mode", pars, 1));
    } CATCH (ret) {}

    return ret;
}

static int __alloc_stub(Attacher *attacher, stub_t **stub)
{
    *stub = attacher->call_name(attacher, NULL, "stub_alloc", NULL, 0);
    dbg_str(DBG_WIP, "alloc_stub addr:%p", *stub);
    return 1;
}

/* 如果有函数属于加载的库，当前不支持。 */
static int 
__add_stub_hooks(Attacher *attacher, stub_t *stub, char *func, char *pre, char *new_fn, 
                 char *post, int para_count)
{
    void *addr;
    char module_name[64] = {0};
    /* 如果size 为0表示是 value类型， 这时也可以传递地址 */
    attacher_paramater_t pars[6] = {{stub, 0}, {NULL, 0},
                                    {NULL, 0}, {NULL, 0},
                                    {NULL, 0}, {para_count, 0}};
    int ret;

    TRY {
        if (func != NULL) {
            // EXEC(dl_get_dynamic_lib_name_from_interval_tree(attacher->tree, addr, module_name, 64));
            addr = attacher->get_remote_function_address(attacher, NULL, func);
            pars[1].value = addr;
        }
        if (pre != NULL) {
            addr = attacher->get_remote_function_address(attacher, NULL, pre);
            pars[2].value = addr;
        }
        if (new_fn != NULL) {
            addr = attacher->get_remote_function_address(attacher, NULL, new_fn);
            pars[3].value = addr;
        }
        if (post != NULL) {
            addr = attacher->get_remote_function_address(attacher, NULL, post);
            pars[4].value = addr;
        }
        dbg_str(DBG_DETAIL, "attacher add_stub_hooks, stub:%p, func:%p, pre:%p, target:%p, post:%p, para count:%d", 
                stub, pars[1].value, pars[2].value, pars[3].value, pars[4].value, para_count);
        
        EXEC(attacher->call_name(attacher, NULL, "stub_add_hooks", pars, 6));
    } CATCH (ret) {} FINALLY {}
    
    return ret;
}

static int __remove_stub_hooks(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call_name(attacher, NULL, "stub_remove_hooks", pars, 1);
}

static int __free_stub(Attacher *attacher, stub_t *stub)
{
    attacher_paramater_t pars[1] = {{stub, 0}};
    return attacher->call_name(attacher, NULL, "stub_free", pars, 1);
}

static int __init(Attacher *attacher)
{
    int ret;
    void *path_addr;
    allocator_t *allocator = attacher->parent.allocator;

    TRY {
        // 获取进程自带或自己运行后加载lib的函数地址，需要依赖libattacher-builtin.so，
        // 所以需要init时提前加载到目标进程。
        path_addr = allocator_mem_zalloc(allocator, 128);
        EXEC(dl_get_dynamic_lib_path(-1, "libattacher-builtin.so", path_addr, 128));
        dbg_str(DBG_DETAIL, "attacher init add lib:%s", path_addr);
        EXEC(attacher->add_lib(attacher, path_addr));

        path_addr = allocator_mem_zalloc(allocator, 128);
        EXEC(dl_get_dynamic_lib_path(-1, "libobject-core.so", path_addr, 128));
        dbg_str(DBG_DETAIL, "attacher init add lib:%s", path_addr);
        EXEC(attacher->add_lib(attacher, path_addr));

        path_addr = allocator_mem_zalloc(allocator, 128);
        EXEC(dl_get_dynamic_lib_path(-1, "libobject-stub.so", path_addr, 128));
        dbg_str(DBG_DETAIL, "attacher init add lib:%s", path_addr);
        EXEC(attacher->add_lib(attacher, path_addr));
       
        EXEC(attacher->call_name(attacher, NULL, "execute_ctor_funcs", NULL, 0));
        EXEC(attacher->call_name(attacher, NULL, "stub_admin_init_default_instance", NULL, 0));

        /* 解析本地动态库地址区间， 后面方便根据本地地址查询对应的函数库 */
        // EXEC(dl_parse_dynamic_table(-1, attacher->tree));
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static class_info_entry_t attacher_class_info[] = {
    INIT_OBJ___ENTRY(Obj, parent),
    INIT_NFUNC_ENTRY(Attacher, construct, __construct),
    INIT_NFUNC_ENTRY(Attacher, deconstruct, __deconstrcut),
    INIT_VFUNC_ENTRY(Attacher, attach, NULL),
    INIT_VFUNC_ENTRY(Attacher, detach, NULL),
    INIT_VFUNC_ENTRY(Attacher, get_remote_function_address, __get_remote_function_address),
    INIT_VFUNC_ENTRY(Attacher, get_remote_builtin_function_address, NULL),
    INIT_VFUNC_ENTRY(Attacher, read, NULL),
    INIT_VFUNC_ENTRY(Attacher, write, NULL),
    INIT_VFUNC_ENTRY(Attacher, malloc, __malloc),
    INIT_VFUNC_ENTRY(Attacher, free, __free),
    INIT_VFUNC_ENTRY(Attacher, call_address_with_value_pars, NULL),
    INIT_VFUNC_ENTRY(Attacher, call_address, __call_address),
    INIT_VFUNC_ENTRY(Attacher, call_name, __call_name),
    INIT_VFUNC_ENTRY(Attacher, call, __call),
    INIT_VFUNC_ENTRY(Attacher, add_lib, NULL),
    INIT_VFUNC_ENTRY(Attacher, remove_lib, __remove_lib),
    INIT_VFUNC_ENTRY(Attacher, alloc_stub, __alloc_stub),
    INIT_VFUNC_ENTRY(Attacher, add_stub_hooks, __add_stub_hooks),
    INIT_VFUNC_ENTRY(Attacher, remove_stub_hooks, __remove_stub_hooks),
    INIT_VFUNC_ENTRY(Attacher, free_stub, __free_stub),
    INIT_VFUNC_ENTRY(Attacher, init, __init),
    INIT_VFUNC_ENTRY(Attacher, run, NULL),
    INIT_VFUNC_ENTRY(Attacher, stop, NULL),
    INIT_END___ENTRY(Attacher),
};
REGISTER_CLASS(Attacher, attacher_class_info);