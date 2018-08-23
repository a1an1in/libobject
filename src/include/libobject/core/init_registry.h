#ifndef __INIT_REGISTRY_H__
#define __INIT_REGISTRY_H__

int __register_init_func(int level, int (*func)()); 
int __register_init_func1(int level, int (*func)(void *arg), void *arg);
int __register_init_func2(int level, int (*func)(void *arg1, void *arg2), void *arg1, void *arg2);
int __register_init_func3(int level, int (*func)(void *arg1, void *arg2, void *arg3), void *arg1, void *arg2, void *arg3);
int execute_init_funcs(); 

#define REGISTER_INIT_FUNC(level, func) \
    __attribute__((constructor)) static void register_init_func() {\
        __register_init_func(level, func);\
    } 

#define REGISTER_INIT_FUNC_ARG1(level, func, arg) \
    __attribute__((constructor)) static void register_init_func_arg1() {\
        __register_init_func1(level, func, arg);\
    } 

#define REGISTER_INIT_FUNC_ARG2(level, func, arg1, arg2) \
    __attribute__((constructor)) static void register_init_func_arg2() {\
        __register_init_func2(level, func, arg1, arg2);\
    } 

#endif 
