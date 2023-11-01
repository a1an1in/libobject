/**
 * @file Tar.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include <unistd.h>
#include "Tar.h"

#define min(a,b) (((a) < (b)) ? a : b)

static int __construct(Tar *tar, char *init_str)
{
    allocator_t *allocator = tar->parent.parent.allocator;

    tar->file = object_new(allocator, "File", NULL);
    tar->file_name = object_new(allocator, "String", NULL);

    return 0;
}

static int __deconstruct(Tar *tar)
{
    object_destroy(tar->file);
    object_destroy(tar->file_name);

    return 0;
}

static int __add_file(Tar *tar, char *file_name)
{

}

static int __extract(Tar *tar)
{
    Archive *archive = (Archive *)&tar->parent;
    String *path = archive->path;
    File *a = archive->file, *file = tar->file;
    struct posix_tar_header *header;
    String *name = tar->file_name;
    char buf[512] = {0};
    char *p;
    int ret, len, read_len;

    TRY {
        dbg_str(DBG_VIP, "extract file:%s, dest path:%s", a->name->get_cstr(a->name), path->get_cstr(path));
        while (1) {
            ret = a->read(a, buf, sizeof(buf));
            dbg_str(DBG_VIP, "buf[0]:%x, ret=%x", buf[0], ret);
            if (buf[0] == 0)
                break;
            header = (struct posix_tar_header*)buf;
            p = header->size;
            len = 0;
            while(*p) len = (len * 8) + (*p++ - '0'); //8进制->10进制

            dbg_str(DBG_VIP, "filename:%s, len:%d", header->name, len);
            if(header->typeflag == '5') {
                continue;
            }
            // name->assign(name, path->get_cstr(path));
            // name->append(name, header->name, strlen(header->name));
            name->assign(name, header->name);
            dbg_str(DBG_VIP, "filename:%s", name->get_cstr(name));
            EXEC(file->open(file, name->get_cstr(name), "w+"));

            while (len) {
                read_len = min(512, len);
                EXEC(a->read(a, buf, read_len));
                EXEC(file->write(file, buf, read_len));
                len -= read_len;
            }
            EXEC(file->close(file));
        }
    } CATCH (ret) {}
    
    return ret;
}

static class_info_entry_t tar_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Tar, construct, __construct),
    Init_Nfunc_Entry(2, Tar, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Tar, add_file, __add_file),
    Init_Vfunc_Entry(4, Tar, extract, __extract),
    Init_End___Entry(5, Tar),
};
REGISTER_CLASS("Tar", tar_class_info);

