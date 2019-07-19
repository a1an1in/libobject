#ifndef __HEAP_H__
#define __HEAP_H__

#include <libobject/core/utils/alloc/allocator.h>

typedef struct heap_s {
    allocator_t *allocator;
    void **queue;
    int size;
    int capacity;
    int (*comparator)(void *element1, void *element2);
}heap_t;

heap_t *heap_alloc(allocator_t *allocator);
int heap_set(heap_t *heap, char *key, void *value);
heap_t *heap_init(heap_t *heap);
void heap_add(heap_t *heap, void *e);
int heap_remove(heap_t *heap, void **element);
int heap_destroy(heap_t *heap);
int heap_size(heap_t *heap);

#endif 
