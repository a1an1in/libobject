#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/array_stack.h>

array_stack_t *array_stack_alloc(allocator_t *allocator)
{
     array_stack_t *as;

     if( (as = (array_stack_t *)
               allocator_mem_alloc(allocator,
                                   sizeof(array_stack_t))) == NULL )
     {
         dbg_str(DBG_DETAIL,"allocator_mem_alloc");
         return NULL;
     }

     memset(as, 0 , sizeof(array_stack_t));

     as->allocator = allocator;

     return as;
}

int array_stack_init(array_stack_t *as)
{
    int ret = 0;

    if(as->size == 0) {
        as->size = 100;
    }
    
    if(as->step == 0) {
        as->step = sizeof(void *);
    }

    as->top = (uint8_t *)allocator_mem_alloc(as->allocator,as->size);
    if(as->top == NULL) {
        dbg_str(DBG_WARNNING,"allocator_mem_alloc");
        return -1;
    }
    as->cur = as->top;

    return ret;
}

int array_stack_push(array_stack_t *as, void *data)
{
    int ret = 0;
    
    if(as->cur - as->top + as->step < as->size) {
        memset(as->cur, 0, as->step);
        *((void **)as->cur) = data;
        as->cur += as->step;
    } else {
        ret = -1;
        dbg_str(DBG_WARNNING,"array stack is full");
    }

    return ret;
}

int array_stack_pop(array_stack_t *as,void **out)
{
    int ret = 0;

    if(as->cur > as->top) {
        as->cur -= as->step;
        *out = *((void **)(as->cur));
    } else {
        dbg_str(DBG_WARNNING,"array stack is null");
        ret = -1;
    }

    return ret;
}

int array_stack_destroy(array_stack_t *as)
{
    int ret = 0;

    allocator_mem_free(as->allocator,as->top);
    allocator_mem_free(as->allocator,as);

    return ret;
}

int test_array_stack()
{
    int ret = 0;
    array_stack_t *as;
    allocator_t *allocator = allocator_get_default_alloc();
    int *p;
    void *a;

    as = array_stack_alloc(allocator);
    as->step = 8;
    array_stack_init(as);

    dbg_str(DBG_DETAIL,"as addr:%p",as);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)4 );
    array_stack_push(as, a);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)5 );
    array_stack_push(as, a);
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)6 );
    array_stack_push(as, a); 
    dbg_str(DBG_DETAIL,"push data:%d", a = (void *)7);
    array_stack_push(as, a);


    array_stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    array_stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    array_stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);
    array_stack_pop(as, (void **)&p);
    dbg_str(DBG_DETAIL,"pop data:%d", p);

    array_stack_destroy(as);

    return ret;
}
