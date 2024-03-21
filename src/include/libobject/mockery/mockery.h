#ifndef __MOCKERY_H__
#define __MOCKERY_H__

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/reg_heap.h>

int __register_mockery_cmd(int (*func)(void *, void *, void *), const char *func_name, const char *file, int line); 
int __register_mockery_func(int (*func)(void *), const char *func_name, const char *file, int line); 
int assert_equal(void *peer1, void *peer2, unsigned int count);
int assert_file_equal(const char *file1, const char *file2);
reg_heap_t * get_global_testfunc_reg_heap();

#define REGISTER_TEST_FUNC(func) \
    __attribute__((constructor)) static void register_test_##func() {\
        __register_mockery_func((int (*)(void *))func, #func, extract_filename_from_path(__FILE__), __LINE__);\
    }

#define REGISTER_TEST_CMD(func) \
    __attribute__((constructor)) static void register_test_cmd_##func() {\
        __register_mockery_cmd((int (*)(void *, void *, void *))func, #func, extract_filename_from_path(__FILE__), __LINE__);\
    } 

#endif