#ifndef __ARRAY_STACK_H__
#define __ARRAY_STACK_H__

#include <libobject/core/utils/alloc/allocator.h>

typedef struct array_stack_s {
    allocator_t *allocator;
    uint8_t step;
    uint8_t *cur;
    int size;
    uint8_t *top;
}array_stack_t;
 
array_stack_t *array_stack_alloc(allocator_t *allocator);
int array_stack_init(array_stack_t *as);
int array_stack_push(array_stack_t *as, void *data);
int array_stack_pop(array_stack_t *as,void **out);
int array_stack_destroy(array_stack_t *as);

#endif 
