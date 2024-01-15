/**
 * @file Archive.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */

#include <libobject/archive/Archive.h>

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
    archive->tmp = object_new(allocator, "String", NULL);

    archive->extracting_file_infos = object_new(allocator, "Vector", NULL);
    headers = archive->extracting_file_infos;
    headers->set_trustee(headers, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback);

    archive->adding_file_infos = object_new(allocator, "Vector", NULL);
    headers = archive->adding_file_infos;
    headers->set_trustee(headers, VALUE_TYPE_STRUCT_POINTER, __free_file_info_callback);

    archive->inclusive_wildchards = object_new(allocator, "Vector", NULL);
    headers = archive->inclusive_wildchards;
    headers->set_trustee(headers, VALUE_TYPE_ALLOC_POINTER, NULL);

    archive->exclusive_wildchards = object_new(allocator, "Vector", NULL);
    headers = archive->exclusive_wildchards;
    headers->set_trustee(headers, VALUE_TYPE_ALLOC_POINTER, NULL);

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

static int __open(Archive *archive, char *archive_name, char *mode)
{
    File *file = archive->file;
    int (*func)(Archive *archive, char *archive_name, char *mode);
    int ret;

    TRY {
        dbg_str(DBG_VIP, "Archive open file %s", archive_name);
        EXEC(file->open(file, archive_name, mode));
        
        func = object_get_progeny_class_first_normal_func(((Obj *)archive)->target_name, "Archive", "open");
        if (func != NULL) {
            EXEC(func(archive, archive_name, mode));
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
static int __can_filter_out(Archive *archive, char *file)
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
            dbg_str(DBG_VIP, "filter check file %s, exclusive wildchard:%s, cnt:%d", file, wildchard, inclusive_cnt);
            THROW_IF(ret >= 1, 0);
        }

        for (i = 0; i < inclusive_cnt; i++) {
            tmp->reset(tmp);
            EXEC(inclusive->peek_at(inclusive, i, &wildchard));
            tmp->assign(tmp, file);
            ret = tmp->find(tmp, wildchard, 0, -1);
            dbg_str(DBG_VIP, "filter check file %s, inclusive wildchard:%s, cnt:%d", file, wildchard, inclusive_cnt);
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
            
            dbg_str(DBG_VIP, "extract_files, info addr:%p", info);
            dbg_str(DBG_VIP, "extract_files, file %s", info->file_name);
            EXEC(a->extract_file(a, info->file_name));
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
            
            dbg_str(DBG_VIP, "add_files, info addr:%p", info);
            dbg_str(DBG_VIP, "add_files, file %s", info->file_name);
            a->add_file(a, info->file_name);
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
        dbg_str(DBG_VIP, "archive add path: %s", STR2A(a->adding_path));
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
            member->file_name = allocator_mem_alloc(allocator, strlen(info->file_name));
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
    Init_Vfunc_Entry(17, Archive, can_filter_out, __can_filter_out),
    Init_Vfunc_Entry(18, Archive, compress, NULL),
    Init_Vfunc_Entry(19, Archive, uncompress, NULL),
    Init_Vfunc_Entry(20, Archive, save, NULL),
    Init_End___Entry(21, Archive),
};
REGISTER_CLASS("Archive", archive_class_info);