
#include <stdio.h>
#include <unistd.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Linked_Queue.h>


struct test{
    int a;
    int b;
};
static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

static void queue_print(void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(DBG_DETAIL, "t->a=%d, t->b=%d", t->a, t->b);
}


static void queue_print_int(void *element)
{
    int *p =  (int*)element;
    dbg_str(DBG_INFO," element %d",*p);
}

int test_peek_linked_queue_peek()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    void * element = NULL;

    allocator_t *allocator = allocator_get_default_instance();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 1; i < 10; i++) {
        buf[i] = i;
        queue->add(queue, &buf[i]);
    }

    /*
     *queue->for_each(queue, queue_print_int);
     */

    dbg_str(DBG_DETAIL, " peek front at: %p ", element);

    queue->peek_front(queue, &element);
    ret = assert_equal(&buf[1], element, sizeof(void *));
    if (ret == 0) {
        goto end;
    }

    queue->peek_back(queue, &element);
    ret = assert_equal(&buf[9], element, sizeof(void *));

end:
    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_peek_linked_queue_peek);

static int test_linked_queue_reset()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_instance();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    queue->reset(queue);
    
    if (queue->size(queue) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(queue);

    return ret;

}
REGISTER_TEST_FUNC(test_linked_queue_reset);

static int test_linked_queue_is_empty()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_instance();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    if (queue->is_empty(queue)) {
        ret = 0;
        goto end;
    } 
    for( i = 0; i < 10; i++) {
        queue->remove(queue, NULL);
    }

    if (queue->is_empty(queue)) {
        ret = 1;
        goto end;
    } 

end:
    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_linked_queue_is_empty);

static int test_linked_queue_size()
{
    int ret,i,j;
    int buf[10];
    Queue * queue ;
    allocator_t *allocator = allocator_get_default_instance();

    queue = OBJECT_NEW(allocator, Linked_Queue, NULL);
    
    for( i = 0; i < 10; i++) {
        queue->add(queue,&i);
        buf[i] = i;
    }

    if (queue->size(queue) == 10) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(queue);

    return ret;
}
REGISTER_TEST_FUNC(test_linked_queue_size);