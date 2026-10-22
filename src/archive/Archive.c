/**
 * @file Archive.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include <libobject/archive/Archive.h>

/* ================= 格式识别(对外可用, 供 CLI/插件复用) ================= */

/* 依据文件头缓冲(前 512 字节)识别格式 */
archive_format_e archive_detect_format_by_buffer(const uint8_t *b, size_t len)
{
    if (b == NULL) {
        return ARCHIVE_FORMAT_UNKNOWN;
    }
    if (len >= 4 && memcmp(b, "PK\x03\x04", 4) == 0) return ARCHIVE_FORMAT_ZIP;
    if (len >= 4 && memcmp(b, "PK\x05\x06", 4) == 0) return ARCHIVE_FORMAT_ZIP;
    if (len >= 6 && memcmp(b, "7z\xbc\xaf\x27\x1c", 6) == 0) return ARCHIVE_FORMAT_7Z;
    if (len >= 4 && memcmp(b, "hsqs", 4) == 0) return ARCHIVE_FORMAT_SQUASHFS;
    if (len >= 4 && memcmp(b, "sqsh", 4) == 0) return ARCHIVE_FORMAT_SQUASHFS;
    if (len >= 7 && memcmp(b, "Rar!\x1a\x07", 7) == 0) return ARCHIVE_FORMAT_RAR;
    if (len >= 8 && memcmp(b, "!<arch>\n", 8) == 0) return ARCHIVE_FORMAT_AR;
    if (len >= 3 && memcmp(b, "BZh", 3) == 0) return ARCHIVE_FORMAT_BZIP2;
    if (len >= 2 && memcmp(b, "\x1f\x8b", 2) == 0) return ARCHIVE_FORMAT_GZIP;
    if (len >= 6 && memcmp(b, "\xfd7zXZ\x00", 6) == 0) return ARCHIVE_FORMAT_XZ;
    if (len >= 4 && memcmp(b, "\x28\xb5\x2f\xfd", 4) == 0) return ARCHIVE_FORMAT_ZSTD;
    if (len >= 4 && memcmp(b, "\x04\x22\x4d\x18", 4) == 0) return ARCHIVE_FORMAT_LZ4;
    if (len >= 262 && memcmp(b + 257, "ustar", 5) == 0) return ARCHIVE_FORMAT_TAR;

    return ARCHIVE_FORMAT_UNKNOWN;
}

/* 依据文件内容识别格式 */
archive_format_e archive_detect_format(char *path)
{
    allocator_t *allocator = allocator_get_default_instance();
    File *file = NULL;
    uint8_t buf[512] = {0};
    int len = 0;
    archive_format_e fmt = ARCHIVE_FORMAT_UNKNOWN;

    if (path == NULL) {
        return ARCHIVE_FORMAT_UNKNOWN;
    }

    file = object_new(allocator, "File", NULL);
    if (file == NULL) {
        return ARCHIVE_FORMAT_UNKNOWN;
    }
    if (file->open(file, path, "rb") >= 0) {
        len = file->read(file, buf, sizeof(buf));
        file->close(file);
        fmt = archive_detect_format_by_buffer(buf, len);
    }
    object_destroy(file);

    return fmt;
}

/* 格式 -> Archive 子类名(NULL 表示该格式不支持) */
const char *archive_format_class_name(archive_format_e fmt)
{
    switch (fmt) {
    case ARCHIVE_FORMAT_TAR:      return "Tar";
    case ARCHIVE_FORMAT_TGZ:      return "Tgz";
    case ARCHIVE_FORMAT_TBZ2:     return "Tbz2";
    case ARCHIVE_FORMAT_ZIP:      return "Zip";
    case ARCHIVE_FORMAT_7Z:       return "SevenZip";
    case ARCHIVE_FORMAT_SQUASHFS: return "Squashfs";
    default:                      return NULL;
    }
}

static int __free_file_info_callback(allocator_t *allocator, archive_file_info_t *info)
{
    allocator_mem_free(allocator, info->file_name);
    allocator_mem_free(allocator, info);

    return 1;
}

static int __construct(Archive *archive, char *init_str)
{
    allocator_t *allocator = archive->parent.allocator;
    Vector *headers;
    
    archive->file = object_new(allocator, "File", NULL);
    
    archive->extracting_path = object_new(allocator, "String", NULL);
    archive->extracting_path->assign(archive->extracting_path, "./");
    archive->adding_path = object_new(allocator, "String", NULL);
    archive->adding_path->assign(archive->adding_path, "./");
    archive->tmp = object_new(allocator, "String", NULL);

    archive->extracting_file_infos = object_new(allocator, "Vector", NULL);
    headers = archive->extracting_file_infos;
    headers->customize(headers, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback);

    archive->adding_file_infos = object_new(allocator, "Vector", NULL);
    headers = archive->adding_file_infos;
    headers->customize(headers, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback);

    archive->inclusive_wildchards = object_new(allocator, "Vector", NULL);
    headers = archive->inclusive_wildchards;
    headers->customize(headers, VALUE_TYPE_ALLOC_POINTER, NULL);

    archive->exclusive_wildchards = object_new(allocator, "Vector", NULL);
    headers = archive->exclusive_wildchards;
    headers->customize(headers, VALUE_TYPE_ALLOC_POINTER, NULL);

    return 0;
}

static int __deconstruct(Archive *archive)
{
    object_destroy(archive->file);
    object_destroy(archive->inclusive_wildchards);
    object_destroy(archive->exclusive_wildchards);
    object_destroy(archive->extracting_path);
    object_destroy(archive->adding_file_infos);
    object_destroy(archive->adding_path);
    object_destroy(archive->tmp);
    object_destroy(archive->extracting_file_infos);

    return 0;
}

/*
 * 归档文件里是二进制内容(压缩流/魔数), 必须以二进制模式打开,
 * 否则在 windows 上文本模式会做 CRLF 转换, 把数据改坏。
 * 调用方传什么模式都无所谓, 这里统一加上 'b'。
 */
static char *__binary_mode(char *mode)
{
    static char binary_mode[8];
    int i, len;

    if (mode == NULL) {
        return mode;
    }

    len = strlen(mode);
    if (len >= (int)sizeof(binary_mode) - 1) {
        len = sizeof(binary_mode) - 2;
    }
    memcpy(binary_mode, mode, len);
    binary_mode[len] = '\0';

    for (i = 0; i < len; i++) {
        if (binary_mode[i] == 'r' || binary_mode[i] == 'w' ||
            binary_mode[i] == 'a') {
            if (i + 1 < len && binary_mode[i + 1] == 'b') {
                break;  /* 已经是二进制模式了 */
            }
            memmove(&binary_mode[i + 2], &binary_mode[i + 1], len - i - 1);
            binary_mode[i + 1] = 'b';
            binary_mode[len + 1] = '\0';
            break;
        }
    }

    return binary_mode;
}

static int __open(Archive *archive, char *archive_name, char *mode)
{
    File *file = archive->file;
    int (*func)(Archive *archive, char *archive_name, char *mode);
    int ret;
    char *bin_mode = __binary_mode(mode);

    TRY {
        dbg_str(DBG_INFO, "Archive open file %s", archive_name);
        EXEC(file->open(file, archive_name, bin_mode));
        
        /* 这里去调用子类的方法 */
        func = object_get_progeny_class_first_private_func(((Obj *)archive)->target_name, "Archive", "open");
        if (func != NULL) {
            EXEC(func(archive, archive_name, bin_mode));
        }
    } CATCH (ret) {}

    return ret;
}

static int __close(Archive *archive)
{
    File *file = archive->file;

    return TRY_EXEC(file->close(file));
}

static int __set_wildchard(Archive *archive, wildchard_type_e type, char *wildchard)
{
    Vector *wildchards;
    String *str = NULL;
    allocator_t *allocator = archive->parent.allocator;
    char *value, *split_str;
    int ret, i, count, len;

    TRY {
        THROW_IF(type >= MAX_WILDCHARD_TYPE, -1);

        if (type == SET_INCLUSIVE_WILDCHARD_TYPE || type == CLEAR_INCLUSIVE_WILDCHARD_TYPE) {
            wildchards = archive->inclusive_wildchards;
        } else if (type == SET_EXCLUSIVE_WILDCHARD_TYPE || type == CLEAR_EXCLUSIVE_WILDCHARD_TYPE){
            wildchards = archive->exclusive_wildchards;
        }

        if (type == CLEAR_INCLUSIVE_WILDCHARD_TYPE || type == CLEAR_EXCLUSIVE_WILDCHARD_TYPE) {
            wildchards->reset(wildchards);
            THROW(1);
        }

        str = object_new(allocator, "String", NULL);
        str->assign(str, wildchard);
        count = str->split(str, ",", -1);

        for (i = 0; i < count; i++) {
            split_str = str->get_splited_cstr(str, i);
            len = strlen(split_str);
            value = allocator_mem_zalloc(allocator, len + 1);
            strcpy(value, split_str);
            wildchards->add(wildchards, value);
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(str);
    }

    return ret;
}

static int __set_extracting_path(Archive *archive, char *path)
{
    String *p = archive->extracting_path;

    p->assign(p, path);

    return 0;
}

static int __set_adding_path(Archive *archive, char *path)
{
    String *p = archive->adding_path;

    p->assign(p, path);

    return 0;
}

/* 
 * can the specifed file be filtered out by inclusive and 
 * exclusive wildchards.
 **/
static int __filter(Archive *archive, char *file)
{
    Vector *inclusive = archive->inclusive_wildchards;
    Vector *exclusive = archive->exclusive_wildchards;
    String *tmp = archive->tmp;
    char *wildchard;
    int ret, inclusive_cnt, exclusive_cnt, i;

    TRY {
        THROW_IF(file == NULL, -1);

        inclusive_cnt = inclusive->count(inclusive);
        exclusive_cnt = exclusive->count(exclusive);
        THROW_IF((inclusive_cnt == 0 && exclusive_cnt == 0), 1);

        for (i = 0; i < exclusive_cnt; i++) {
            tmp->reset(tmp);
            EXEC(exclusive->peek_at(exclusive, i, &wildchard));
            tmp->assign(tmp, file);
            ret = tmp->find(tmp, wildchard, 0, -1);
            dbg_str(DBG_INFO, "filter check file %s, exclusive wildchard:%s, cnt:%d", file, wildchard, inclusive_cnt);
            THROW_IF(ret >= 1, 0);
        }

        for (i = 0; i < inclusive_cnt; i++) {
            tmp->reset(tmp);
            EXEC(inclusive->peek_at(inclusive, i, &wildchard));
            tmp->assign(tmp, file);
            ret = tmp->find(tmp, wildchard, 0, -1);
            dbg_str(DBG_INFO, "filter check file %s, inclusive wildchard:%s, cnt:%d", file, wildchard, inclusive_cnt);
            THROW_IF(ret >= 1, 1);
        }

        THROW(0);
    } CATCH (ret) { }

    return ret;
}

/* extract specified files */
static int __extract_files(Archive *a, Vector *files)
{
    int ret, count, i;
    archive_file_info_t *info;

    TRY {
        THROW_IF(files == 0, -1);

        count = files->count(files);
        for (i = 0; count > 0; i++) {
            info = NULL;
            EXEC(files->peek_at(files, i, &info));
            CONTINUE_IF(info == NULL);
            
            dbg_str(DBG_INFO, "extract_files, info addr:%p", info);
            dbg_str(DBG_INFO, "extract_files, file %s", info->file_name);
            EXEC(a->extract_file(a, info));
            count--;
        }

    } CATCH (ret) {}

    return ret;
}

static int __extract(Archive *a)
{
    Vector *infos;

    TRY_EXEC(a->list(a, &infos));
    TRY_EXEC(a->extract_files(a, infos));

    return 1;
}

static int __add_files(Archive *a, Vector *files)
{
    int ret, i, cnt;
    archive_file_info_t *info;

    TRY {
        cnt = files->count(files);
        THROW_IF(cnt == 0, 0);

        for (i = 0; i < cnt; i++) {
            info = NULL;
            EXEC(files->peek_at(files, i, &info));
            CONTINUE_IF(info == NULL);
            
            dbg_str(DBG_INFO, "add_files, info addr:%p", info);
            dbg_str(DBG_INFO, "add_files, file %s", info->file_name);
            EXEC(a->add_file(a, info));
        }
    } CATCH (ret) {}

    return ret;
}

static int __add(Archive *a)
{
    allocator_t *allocator = a->parent.allocator;
    Vector *list;
    archive_file_info_t archive_file_info = {0};
    fs_file_info_t *fs_file_info;
    int ret, count, i, len;

    TRY {
        dbg_str(DBG_INFO, "archive add path: %s", STR2A(a->adding_path));
        list = object_new(allocator, "Vector", NULL);
        
        count = fs_tree(STR2A(a->adding_path), list, -1);
        THROW_IF(count < 0, -1);
        fs_print_file_info_list(list);

        for (i = 0; i < count; i++) {
            fs_file_info = NULL;
            memset(&archive_file_info, 0, sizeof(archive_file_info_t));
            EXEC(list->peek_at(list, i, &fs_file_info));
            THROW_IF(fs_file_info == NULL, -1);

            len = strlen(fs_file_info->file_name);
            if (fs_file_info->file_name[len - 1] == '.') continue;
            /* 跳过目录节点(如子目录名): fs_tree 已递归收录其下文件, 避免把目录当文件写入各格式 */
            if (S_ISDIR(fs_file_info->st.st_mode)) continue;
            archive_file_info.file_name = fs_file_info->file_name;
            a->add_adding_file_info(a, &archive_file_info);
        }

        EXEC(a->add_files(a, a->adding_file_infos));
        EXEC(a->save(a));
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __add_adding_file_info(Archive *a, archive_file_info_t *info)
{
    allocator_t *allocator = a->parent.allocator;
    Vector *infos = a->adding_file_infos;
    archive_file_info_t *member;
    int ret;

    TRY {
        member = allocator_mem_alloc(allocator, sizeof(archive_file_info_t));
        THROW_IF(member == NULL, -1);
        memcpy(member, info, sizeof(archive_file_info_t));

        if (info->file_name) {
            member->file_name = allocator_mem_zalloc(allocator, strlen(info->file_name) + 1);
            strcpy(member->file_name, info->file_name);
        }
        EXEC(infos->add(infos, member));
    } CATCH (ret) {}

    return ret;
}

static int __get_adding_file_infos(Archive *archive, Vector **infos)
{
    String *adding_path = archive->adding_path;
    allocator_t *allocator = archive->parent.allocator;
    int ret;

    TRY {

    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t archive_class_info[] = {
    Init_Obj___Entry( 0, Obj, parent),
    Init_Nfunc_Entry( 1, Archive, construct, __construct),
    Init_Nfunc_Entry( 2, Archive, deconstruct, __deconstruct),
    Init_Nfunc_Entry( 3, Archive, open, __open),
    Init_Nfunc_Entry( 4, Archive, close, __close),
    Init_Vfunc_Entry( 5, Archive, set_wildchard, __set_wildchard),
    Init_Vfunc_Entry( 6, Archive, set_extracting_path, __set_extracting_path),
    Init_Vfunc_Entry( 7, Archive, set_adding_path, __set_adding_path),
    Init_Vfunc_Entry( 8, Archive, extract_file, NULL),
    Init_Vfunc_Entry( 9, Archive, extract_files, __extract_files),
    Init_Vfunc_Entry(10, Archive, extract, __extract),
    Init_Vfunc_Entry(11, Archive, add_file, NULL),
    Init_Vfunc_Entry(12, Archive, add_files, __add_files),
    Init_Vfunc_Entry(13, Archive, add, __add),
    Init_Vfunc_Entry(14, Archive, list, NULL),
    Init_Vfunc_Entry(15, Archive, add_adding_file_info, __add_adding_file_info),
    Init_Vfunc_Entry(16, Archive, get_adding_file_infos, __get_adding_file_infos),
    Init_Vfunc_Entry(17, Archive, filter, __filter),
    Init_Vfunc_Entry(18, Archive, compress, NULL),
    Init_Vfunc_Entry(19, Archive, uncompress, NULL),
    Init_Vfunc_Entry(20, Archive, save, NULL),
    Init_End___Entry(21, Archive),
};
REGISTER_CLASS(Archive, archive_class_info);