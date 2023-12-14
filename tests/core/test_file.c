#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/io/File.h>

int teset_file_compute_crc32()
{
    char *file_name = "./tests/res/zip/test_zip_extract.txt";
    uint32_t crc, expect_crc = 0xa8e65b40;
    int ret;

    TRY {
        EXEC(file_compute_crc32(file_name, &crc));
        dbg_str(DBG_VIP, "file crc32:%x, expect:%x", crc, expect_crc);
        THROW_IF(crc != expect_crc, -1);
    } CATCH (ret) {}

    return ret;
}
REGISTER_TEST_FUNC(teset_file_compute_crc32);

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