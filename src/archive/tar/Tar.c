/**
 * @file Tar.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include <unistd.h>
#include <libobject/archive/Tar.h>

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

static int __open(Tar *tar, char *archive_name, char *mode)
{
    Vector *infos;

    return 1;
}

static int __list(Tar *tar, Vector **infos)
{
    Archive *archive = (Archive *)&tar->parent;
    allocator_t *allocator = tar->parent.parent.allocator;
    Vector *files = archive->extracting_file_infos;
    File *a = archive->file;
    struct posix_tar_header *header;
    archive_file_info_t *info;
    char buf[512] = {0};
    char *p;
    int ret, size, offset = 0, len = 0;

    TRY {
        dbg_str(DBG_INFO, "------------------------------");
        dbg_str(DBG_INFO, "tar list:%s", STR2A(a->name));
        *infos = archive->extracting_file_infos;
        (*infos)->reset(*infos);
        
        while (1) {
            a->seek(a, offset, SEEK_SET);
            ret = a->read(a, buf, sizeof(buf));
            if (buf[0] == 0) break;

            header = (struct posix_tar_header*)buf;
            p = header->size;
            size = 0;
            while(*p) size = (size * 8) + (*p++ - '0'); //8进制->10进制
            a->seek(a, size, SEEK_CUR);

            info = allocator_mem_alloc(allocator, sizeof(archive_file_info_t));
            THROW_IF(info == NULL, -1);
            info->offset = offset;
            info->file_name = allocator_mem_zalloc(allocator, strlen(header->name) + 1);
            THROW_IF(info->file_name == NULL, -1);
            strcpy(info->file_name, header->name);
            info->size = size;
            offset += ((header->typeflag == '5')? 512 : (512 + ((size + 511) / 512) * 512));
            
            dbg_str(DBG_INFO, "filename:%s, size:%d, typeflag:%d", header->name, size, header->typeflag);
            EXEC(files->add(files, info));
            dbg_str(DBG_INFO, "count:%d", files->count(files));
        }
    } CATCH (ret) {}
    
    return ret;
}

static int __extract_file(Tar *tar, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&tar->parent;
    File *a = archive->file, *file = tar->file;
    struct posix_tar_header *header;
    String *name = tar->file_name;
    char buf[512] = {0}, tmp[512] = {0};
    char *p, *root;
    int ret, len, read_len;

    TRY {
        dbg_str(DBG_INFO, "------------------------------");
        THROW_IF(info == NULL, -1);
        dbg_str(DBG_INFO, "tar extract file:%s, offset:%d", info->file_name, info->offset);

        /* 1.read file header */
        a->seek(a, 0, SEEK_SET);
        a->seek(a, info->offset, SEEK_CUR);
        ret = a->read(a, buf, sizeof(buf));

        header = (struct posix_tar_header*)buf;
        p = header->size;
        len = 0;
        while(*p) len = (len * 8) + (*p++ - '0'); //8进制->10进制

        /* 2.read file data */
        name->assign(name, STR2A(archive->extracting_path));
        name->append(name, header->name, strlen(header->name));
        dbg_str(DBG_INFO, "filename:%s", STR2A(name));
        
        /* 3.如果文件是目录， 则需要创建后退出 */
        if(header->typeflag == '5') {
            fs_mkdir(header->name, 0777);
            THROW(1);
        }

        /* 4.创建父目录，如果父目录不存在 */
        strcpy(tmp, STR2A(name));
        fs_get_path_and_name(tmp, &root, NULL);
        dbg_str(DBG_INFO, "root:%s", root);
        if (!fs_is_exist(root)) {
            fs_mkdir(root, 0777);
        }

        /* 5.保存文件内容 */
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

        /* 6.设置文件属性 */

    } CATCH (ret) {}
    
    return ret;
}

static int __add_file(Tar *tar, archive_file_info_t *info)
{
    int ret, size, read_size, len;
    Archive *archive = (Archive *)&tar->parent;
    File *a = archive->file, *file = tar->file;
    struct posix_tar_header *header;
    char *file_name;
    struct stat stat;
    char buffer[1024] = {0};
    String *path = archive->tmp;
    char *relative_path;

    TRY {
        THROW_IF(info == NULL, -1);
        file_name = info->file_name;

        memset(buffer, 0, sizeof(buffer));
        dbg_str(DBG_INFO, "tar name:%s", a->name->get_cstr(a->name));
        EXEC(size = a->get_size(a));
        dbg_str(DBG_INFO, "tar file size:%d", size);
        if (size != 0) {
            a->seek(a, -1024, SEEK_END);
        }

        /* add file name*/
        memset(buffer, 0, sizeof(buffer));
        header = buffer;
        path->reset(path);
        path->assign(path, file_name);
        dbg_str(DBG_INFO, "add_file, name:%s", STR2A(path));
        EXEC(fs_get_relative_path(STR2A(path), STR2A(archive->adding_path), &relative_path));
        snprintf(header->name, sizeof(header->name), "%s", relative_path);
        dbg_str(DBG_INFO, "file name:%s", header->name);

        /* add file size*/
        size = fs_get_size(file_name);
        snprintf(header->size, sizeof(header->size), "%011o", size);
        dbg_buf(DBG_INFO, "file size:", header->size, 12);
        /* add magic*/
        memcpy(header->magic, "ustar  ", 8);
        /* add mode */
        sprintf(header->mode, "%07d", 777);
        /* add mtime */
        EXEC(fs_get_stat(file_name, &stat));
        snprintf(header->mtime, sizeof(header->mtime), "%011lo", stat.st_mtime);
        dbg_str(DBG_INFO, "file st_mtime:%d, mtime:%s", stat.st_mtime, header->mtime);

        /* add checksum */
        add_check_sum(header);

        if(fs_is_directory(file_name)) {
            header->typeflag = '5';
            EXEC(a->write(a, header, 512));
            memset(buffer, 0, sizeof(buffer));
            EXEC(a->write(a, buffer, sizeof(buffer))); //write tail.
            THROW(1);
        } else {
            EXEC(a->write(a, header, 512));
        }

        /* write file */
        EXEC(file->open(file, file_name, "r+"));
        while (size > 0) {
            memset(buffer, 0, 512);
            read_size = size >= 512 ? 512 : size;
            file->read(file, buffer, read_size);
            a->write(a, buffer, 512);
            size -= read_size;
        }

        memset(buffer, 0, sizeof(buffer));
        EXEC(a->write(a, buffer, sizeof(buffer))); //write tail.
    } CATCH (ret) {}

    return ret;
}

static int __save(Tar *tar)
{
    Archive *archive = (Archive *)&tar->parent;
    File *a = archive->file;
    
    fflush(a->f);

    return 1;
}

static class_info_entry_t tar_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Tar, construct, __construct),
    Init_Nfunc_Entry(2, Tar, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Tar, open, __open),
    Init_Vfunc_Entry(4, Tar, list, __list),
    Init_Vfunc_Entry(5, Tar, extract_file, __extract_file),
    Init_Vfunc_Entry(6, Tar, add_file, __add_file),
    Init_Vfunc_Entry(7, Tar, save, __save),
    Init_End___Entry(8, Tar),
};
REGISTER_CLASS("Tar", tar_class_info);