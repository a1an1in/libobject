#ifndef __REGISTRY_HEAP_H__
#define __REGISTRY_HEAP_H__

enum func_entry_type_e {
    FUNC_ENTRY_TYPE_NORMAL, 
    FUNC_ENTRY_TYPE_STANDALONE, 
};

typedef struct reg_heap_s {
    void **queue;
    int size;
    int capacity;
    int (*comparator)(void *element1, void *element2);
} reg_heap_t;

typedef struct init_func_entry_s {
    int level;
    int args_count;
    void *arg1, *arg2, *arg3;
    int line;
    const char *file;
    const char *func_name;
    int ret;
    enum func_entry_type_e type;

    int (*func)();
    int (*func1)(void * arg);
    int (*func2)(void * arg1, void *arg2);
    int (*func3)(void * arg1, void *arg2, void *arg3);
} init_func_entry_t;

#define TEST_ENTRY struct init_func_entry_s 

reg_heap_t *reg_heap_alloc();
int reg_heap_set(reg_heap_t *reg_heap, char *key, void *value);
reg_heap_t *reg_heap_init(reg_heap_t *reg_heap);
int reg_heap_add(reg_heap_t *reg_heap, void *e);
int reg_heap_remove(reg_heap_t *reg_heap, void **element);
int reg_heap_destroy(reg_heap_t *reg_heap);
int reg_heap_size(reg_heap_t *reg_heap);

#endif 
