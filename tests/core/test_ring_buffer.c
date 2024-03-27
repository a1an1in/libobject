
#include <stdio.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/Ring_Buffer.h>
#include <libobject/mockery/mockery.h>

int test_ring_rb(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int len, ret;

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, 9);

    rb->write(rb, in, 5);
    rb->read(rb, out, 4);
    rb->write(rb, in + 5, 8);
    rb->read(rb, out + 4, 9);

    dbg_buf(IO_DETAIL, "in:", (uint8_t *)in, 13);
    dbg_buf(IO_DETAIL, "out:", (uint8_t *)out, 13);

    if (strncmp(in, out, 13) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb);

int test_ring_rb_find(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_instance();
    char *test = "abc hello world\r\n you gota work hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);
    addr = rb->find(rb, "\r\n", 2);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 15), addr);
    if (addr == (rb->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find);

int test_ring_rb_find2(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_instance();
    char *test = "abc hello "\
                 "world\r\n "\
                 "you gotta "\
                 "wo rk hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 22;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    addr = rb->find(rb, "\r\n", 2);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 15), addr);
    if (addr == (rb->addr + 15)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find2);

int test_ring_rb_find3(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_instance();
    char *test = "abc hello "\
                 "world\r\n   "\
                 "you gotta "\
                 "wo rk hard";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 30;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    addr = rb->find(rb, "dabc", 4);

    dbg_str(IO_DETAIL, "test addr:%p, find addr:%p", (rb->addr + 39), addr);
    if (addr == (rb->addr + 39)) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_find3);

int test_ring_rb_get_needle_offset(TEST_ENTRY *entry)
{
    Ring_Buffer *rb;
    allocator_t *allocator = allocator_get_default_instance();
    char *test = "abc hello "\
                 "world\r\n yo"\
                 "u gotta wo"\
                 "rk hardddd";
    int len, ret;
    void *addr;

    len = strlen(test);

    rb = OBJECT_NEW(allocator, Ring_Buffer, NULL);

    dbg_str(IO_DETAIL, "rb size=%d", len);
    rb->set_size(rb, len);
    rb->write(rb, test, len);

    rb->w_offset = rb->r_offset = 20;
    rb->last_operation_flag = BUFFER_WRITE_OPERATION;

    len = rb->get_needle_offset(rb, "\r\n", 2);
    len += 2;

    dbg_str(IO_DETAIL, "len=%d", len);
    if (len == 37) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(rb);

    return ret;
}
REGISTER_TEST_FUNC(test_ring_rb_get_needle_offset);