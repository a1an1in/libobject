#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/io/File.h>

int test_write_file()
{
    File *file;
    char *content = "hello world";
    allocator_t *allocator = allocator_get_default_instance();

    file = OBJECT_NEW(allocator, File, NULL);

    file->open(file, "./tests/res/test.txt", "w+");
    file->write(file, content, strlen(content));
    file->close(file);
    
    object_destroy(file);

    return 1;
}
REGISTER_TEST_CMD(test_write_file);

int test_read_file()
{
    File *file;
    char buffer[1024] = {0};
    int len;
    allocator_t *allocator = allocator_get_default_instance();

    file = OBJECT_NEW(allocator, File, NULL);

    file->open(file, "./tests/res/test.txt", "r+");
    len = file->read(file, buffer, 1024);
    dbg_str(DBG_VIP, "read file len:%d", len);
    dbg_buf(DBG_VIP, "read file content:", buffer, 10);
    file->close(file);

    object_destroy(file);

    return 1;
}
REGISTER_TEST_CMD(test_read_file);