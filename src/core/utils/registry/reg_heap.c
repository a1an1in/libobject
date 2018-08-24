#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/registry/reg_heap.h>

static int greater_than(void *element1, void *element2) 
{
    init_func_entry_t *e1, *e2;

    e1 = (init_func_entry_t *) element1;
    e2 = (init_func_entry_t *) element2;

    return (e1->level > e2->level) ? 1 : 0;
}

static void shiftup(reg_heap_t *reg_heap, int index, void *e)
{
    while(index > 0) {
        int parent = (index - 1) / 2;
        if (reg_heap->comparator(e, reg_heap->queue[parent])) {
            break;
        }

        /*if parent smaller than inserting child, exchange position*/
        reg_heap->queue[index] = reg_heap->queue[parent];
        index = parent;
    }
    reg_heap->queue[index] = e;
}

static void shiftdown(reg_heap_t *reg_heap, int index, void *e)
{
    int size = reg_heap->size;
    int half = size / 2;

    while (index < half) {
        int child = 2 * index + 1;
        int right = child + 1;
        if (    right < size &&
                reg_heap->comparator(reg_heap->queue[child], reg_heap->queue[right])) {
            child = right;
        }
        if (reg_heap->comparator(reg_heap->queue[child], e)) {
            break;
        }

        /*save greater child to parent*/
        reg_heap->queue[index] = reg_heap->queue[child]; 
        index = child;
    }

    reg_heap->queue[index] = e;
}

reg_heap_t *reg_heap_alloc()
{
    reg_heap_t *ret;

    ret = (reg_heap_t *)malloc(sizeof(reg_heap_t));
    if (ret != NULL) {
        memset(ret, 0, sizeof(reg_heap_t));
    }
    
    return ret;
}

int reg_heap_set(reg_heap_t *reg_heap, char *key, void *value)
{
    if (strcmp(key, "comparator") == 0) {
        reg_heap->comparator = value;
    } else if (strcmp(key, "capacity") == 0) {
        reg_heap->capacity = *(int *)value;
    } else {
        printf("reg_heap_set, not support %s setting\n", key);
    }
}

reg_heap_t *reg_heap_init(reg_heap_t *reg_heap)
{
    if (reg_heap->capacity == 0) {
        reg_heap->queue = (void **)malloc(sizeof(void *) * 64);
        reg_heap->capacity = 64;
    } else {
        reg_heap->queue = (void **)malloc(sizeof(void *) * reg_heap->capacity);
    }

    return reg_heap;
}

int reg_heap_add(reg_heap_t *reg_heap, void *e)
{

    int index = reg_heap->size;

    if (reg_heap->size == reg_heap->capacity) {
        printf("err: reg_heap is full");
        return -1;
    }
    shiftup(reg_heap, index, e);
    reg_heap->size++;

    return 0;
}

int reg_heap_remove(reg_heap_t *reg_heap, void **element)
{
    if (reg_heap->size <= 0)
        return -1;

    *element = reg_heap->queue[0];

    shiftdown(reg_heap, 0, reg_heap->queue[reg_heap->size - 1]);
    reg_heap->queue[reg_heap->size - 1] = 0;
    reg_heap->size--;

    return 0;
}

int reg_heap_destroy(reg_heap_t *reg_heap)
{
    free(reg_heap->queue);
    free(reg_heap);

    return 0;
}

int reg_heap_size(reg_heap_t *reg_heap)
{
    return reg_heap->size;
}
