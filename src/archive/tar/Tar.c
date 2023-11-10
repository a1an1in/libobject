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

static int add_check_sum(struct posix_tar_header *header) 
{
	int n, u = 0;

    memcpy(header->chksum, "        ", 8);

	for (n = 0; n < 512; ++n) {
		if (n < 148 || n > 155)
			/* Standard tar checksum adds unsigned bytes. */
			u += ((unsigned char*)header)[n];
		else
			u += 0x20;
	}
	sprintf(header->chksum, "%06o", u);

    return 0;
}

static int __add_file(Tar *tar, char *file_name)
{
    int ret, size;
    Archive *archive = (Archive *)&tar->parent;
    File *a = archive->file, *file = tar->file;
    struct posix_tar_header *header;
    char buffer[1024] = {0};

    TRY {
        dbg_str(DBG_VIP, "add_file, name:%s", file_name);
        dbg_str(DBG_VIP, "tar name:%s", a->name->get_cstr(a->name));
        EXEC(size = a->get_size(a));
        dbg_str(DBG_VIP, "tar file size:%d", size);
        if (size == 0) {
            EXEC(a->write(a, buffer, sizeof(buffer))); //write tail.
        }

        memset(buffer, 0, sizeof(buffer));
        header = buffer;
        /* add file name*/
        snprintf(header->name, sizeof(header->name), "%s", file_name);
        dbg_buf(DBG_VIP, "file name:", header->name, 50);

        /* add file size*/
        size = fs_get_size(file_name);
        snprintf(header->size, sizeof(header->size), "%011o", size);
        dbg_buf(DBG_VIP, "file size:", header->size, 12);

        /* add magic*/
        memcpy(header->magic, "ustar  ", 8);

        /* add mode */
        sprintf(header->mode, "%07d", 777);

        /* add checksum */
        add_check_sum(header);

        a->seek(a, 1024, SEEK_END);
        EXEC(a->write(a, header, 512)); 

        // EXEC(file->open(file, name->get_cstr(name), "w+"));
    } CATCH (ret) {}

    return ret;
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
                fs_mkdir(header->name, 0777);
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
                EXEC(ret = file->write(file, buf, read_len));
                THROW_IF(ret != read_len, -1);  // TODO: need optimize later
                len -= read_len;
                if (read_len < 512) {
                    a->seek(a, 512 - read_len, SEEK_CUR);
                }
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

