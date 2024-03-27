
#include <stdio.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/core/try.h>
#include <libobject/mockery/mockery.h>

int test_buffer(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int len, ret;

    buffer = OBJECT_NEW(allocator, Buffer, NULL);

    buffer->set_capacity(buffer, 14);

    buffer->write(buffer, in, 5);
    buffer->write(buffer, in + 5, 8);
    buffer->read(buffer, out, 4);
    buffer->read(buffer, out + 4, 9);

    dbg_buf(IO_DETAIL, "in:", (uint8_t *)in, 13);
    dbg_buf(IO_DETAIL, "out:", (uint8_t *)out, 13);

    if (strncmp(in, out, 13) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(buffer);

    return ret;
}
REGISTER_TEST_FUNC(test_buffer);

int test_buffer_get_needle_offset(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int offset, ret = -1;

    TRY {
        buffer = OBJECT_NEW(allocator, Buffer, NULL);
        buffer->set_capacity(buffer, 14);
        buffer->write(buffer, in, 14);
        offset = buffer->get_needle_offset(buffer, "gh", 2);

        THROW_IF(offset != 6, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "offset=%d", offset);
    } FINALLY {
        object_destroy(buffer);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_buffer_get_needle_offset);

int test_buffer_read_to_string(TEST_ENTRY *entry)
{
    Buffer *buffer;
    String *str;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    int offset, ret = -1;

    TRY {
        buffer = OBJECT_NEW(allocator, Buffer, NULL);
        str = OBJECT_NEW(allocator, String, NULL);

        buffer->set_capacity(buffer, 14);
        buffer->write(buffer, in, 14);
        EXEC(buffer->read_to_string(buffer, str, 8));
        THROW_IF(!str->equal(str, "abcdefgh"), -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "read str:%s", STR2A(str));
    } FINALLY {
        object_destroy(buffer);
        object_destroy(str);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_buffer_read_to_string);

int test_buffer_rfind(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    uint8_t *addr = NULL;
    int ret = -1;

    TRY {
        buffer = OBJECT_NEW(allocator, Buffer, NULL);
        buffer->set_capacity(buffer, 14);
        buffer->write(buffer, in, 14);
        addr = buffer->rfind(buffer, "gh", 2);

        THROW_IF(addr != buffer->addr + 6, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_buffer_rfind, buffer head addr:%p, find addr:%p", 
                buffer->addr, addr);
    } FINALLY {
        object_destroy(buffer);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_buffer_rfind);

int test_buffer_find(TEST_ENTRY *entry)
{
    Buffer *buffer;
    allocator_t *allocator = allocator_get_default_instance();
    char in[14] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'};
    char out[14] = {'\0'};
    uint8_t *addr = NULL;
    int ret = -1;

    TRY {
        buffer = OBJECT_NEW(allocator, Buffer, NULL);
        buffer->set_capacity(buffer, 14);
        buffer->write(buffer, in, 14);
        addr = buffer->find(buffer, "gh", 2);

        THROW_IF(addr != buffer->addr + 6, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_buffer_find, buffer head addr:%p, find addr:%p", 
                buffer->addr, addr);
    } FINALLY {
        object_destroy(buffer);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_buffer_find);