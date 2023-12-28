#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/io/File.h>

int test_file()
{
    File *file;
    char *content = "hello world";
    allocator_t *allocator = allocator_get_default_instance();

    file = OBJECT_NEW(allocator, File, NULL);

    file->open(file, "./test.txt", "w+");
    file->write(file, content, strlen(content));
    file->close(file);
    
#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(file);

    return 1;
}
REGISTER_TEST_CMD(test_file);