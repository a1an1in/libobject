#ifndef __INIT_REGISTRY_H__
#define __INIT_REGISTRY_H__

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/reg_heap.h>
#include <libobject/core/try.h>

#define REGISTRY_CTOR_PRIORITY_VERSION                      1 
#define REGISTRY_CTOR_PRIORITY_LIBALLOC_REGISTER_MODULES    2 
#define REGISTRY_CTOR_PRIORITY_SYNC_LOCK_REGISTER_MODULES   3 
#define REGISTRY_CTOR_PRIORITY_LIBDBG_REGISTER_MODULES      5 
#define REGISTRY_CTOR_PRIORITY_REGISTER_MAP_MODULES         6 
#define REGISTRY_CTOR_PRIORITY_DEBUGGER                     7 
#define REGISTRY_CTOR_PRIORITY_DEFAULT_ALLOCATOR            8 
#define REGISTRY_CTOR_PRIORITY_OBJ_DEAMON                   9 
#define REGISTRY_CTOR_PRIORITY_REGISTER_CLASS               10
#define REGISTRY_CTOR_PRIORITY_REGISTER_APP_CMD             11

#define REGISTRY_DTOR_PRIORITY_OBJ_DEAMON                   2
#define REGISTRY_DTOR_PRIORITY_DEFAULT_ALLOCATOR            3
#define REGISTRY_DTOR_PRIORITY_DEBUGGER                     4

int __register_ctor_func(int level, int (*func)()); 
int __register_ctor_func1(int level, int (*func)(void *arg), void *arg);
int __register_ctor_func2(int level, int (*func)(void *arg1, void *arg2), void *arg1, void *arg2);
int __register_ctor_func3(int level, int (*func)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3);
int __register_dtor_func(int level, int (*func)());
int __register_standalone_test_func(int (*func)(void *, void *, void *), const char *func_name, const char *file, int line); 
int __register_test_func(int (*func)(void *), const char *func_name, const char *file, int line); 
int execute_ctor_funcs(); 
int execute_dtor_funcs();
int execute_test_funcs();
int execute_designated_func(char *func_name, int arg1, char **arg2);
int assert_equal(void *peer1, void *peer2, unsigned int count);

#define INIT_LIBOBJECT execute_ctor_funcs

#define REGISTER_CTOR_FUNC(level, func) \
    __attribute__((constructor)) static void register_ctor_##func() {\
        __register_ctor_func(level, func);\
    } 

#define REGISTER_CTOR_FUNC_ARG1(level, func, arg) \
    __attribute__((constructor)) static void register_ctor_##func_arg1() {\
        __register_ctor_func1(level, func, arg);\
    } 

#define REGISTER_CTOR_FUNC_ARG2(level, func, arg1, arg2) \
    __attribute__((constructor)) static void register_ctor_##func_arg2() {\
        __register_ctor_func2(level, func, arg1, arg2);\
    } 

#define REGISTER_DTOR_FUNC(level, func) \
    __attribute__((constructor)) static void register_dtor_##func() {\
        __register_dtor_func(level, func);\
    } 

#define REGISTER_TEST_FUNC(func) \
    __attribute__((constructor)) static void register_test_##func() {\
        __register_test_func((int (*)(void *))func, #func, extract_filename_from_path(__FILE__), __LINE__);\
    }

#define REGISTER_TEST_CMD(func) \
    __attribute__((constructor)) static void register_test_cmd_##func() {\
        __register_standalone_test_func((int (*)(void *, void *, void *))func, #func, extract_filename_from_path(__FILE__), __LINE__);\
    } 

#endif 
