/**
 * @file Archive_Command.c
 * @Synopsis
 *     通用"打开任意归档"命令. 一个命令搞定所有压缩包:
 *     按文件魔数自动识别格式(含 squashfs / zip / 7z / tar / tgz / tbz2),
 *     再分发到对应 Archive 子类. 用法:
 *
 *       ./sysroot/linux/x86_64/bin/xtools archive list    <file> [-w 过滤]
 *       ./sysroot/linux/x86_64/bin/xtools archive extract <file> [-o 输出目录]
 *       ./sysroot/linux/x86_64/bin/xtools archive create  <out.sqfs|.zip|.7z|.tar|.tgz|.tbz2> <源路径>
 *       ./sysroot/linux/x86_64/bin/xtools archive add     <file> <源文件>
 *
 * 识别/分发逻辑全部在本文件内部实现(不对外暴露), 对外入口只有命令 archive.
 * @author alan lin
 * @version
 * @date 2026-09-01
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/io/File.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/argument/Application.h>
#include <libobject/argument/Argument.h>
#include <libobject/argument/Option.h>
#include <libobject/compress/Compress.h>
#include <libobject/archive/Archive_Command.h>

/* ================= 格式识别(内部 static, 不对外暴露) ================= */

static const char *__format_class_name(archive_format_e fmt)
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

/* 依据文件头缓冲(前 512 字节)识别格式 */
static archive_format_e __detect_format_buf(const uint8_t *b, size_t len)
{
    if (len >= 4 && memcmp(b, "PK\x03\x04", 4) == 0) return ARCHIVE_FORMAT_ZIP;
    if (len >= 4 && memcmp(b, "PK\x05\x06", 4) == 0) return ARCHIVE_FORMAT_ZIP;
    if (len >= 6 && memcmp(b, "7z\xbc\xaf\x27\x1c", 6) == 0) return ARCHIVE_FORMAT_7Z;
    if (len >= 4 && memcmp(b, "hsqs", 4) == 0) return ARCHIVE_FORMAT_SQUASHFS;
    if (len >= 7 && memcmp(b, "Rar!\x1a\x07", 7) == 0) return ARCHIVE_FORMAT_RAR;
    if (len >= 8 && memcmp(b, "!<arch>\n", 8) == 0) return ARCHIVE_FORMAT_AR;
    if (len >= 6 && memcmp(b, "07070", 5) == 0) return ARCHIVE_FORMAT_CPIO;
    if (len >= 4 && memcmp(b, "MSCF", 4) == 0) return ARCHIVE_FORMAT_CAB;
    if (len >= 2 && memcmp(b, "\x1f\x9d", 2) == 0) return ARCHIVE_FORMAT_Z;
    if (len >= 3 && memcmp(b, "BZh", 3) == 0) return ARCHIVE_FORMAT_BZIP2;
    if (len >= 2 && memcmp(b, "\x1f\x8b", 2) == 0) return ARCHIVE_FORMAT_GZIP;
    if (len >= 6 && memcmp(b, "\xfd7zXZ\x00", 6) == 0) return ARCHIVE_FORMAT_XZ;
    if (len >= 4 && memcmp(b, "\x28\xb5\x2f\xfd", 4) == 0) return ARCHIVE_FORMAT_ZSTD;
    if (len >= 4 && memcmp(b, "\x04\x22\x4d\x18", 4) == 0) return ARCHIVE_FORMAT_LZ4;
    if (len >= 262 && memcmp(b + 257, "ustar", 5) == 0) return ARCHIVE_FORMAT_TAR;
    return ARCHIVE_FORMAT_UNKNOWN;
}

static archive_format_e __detect_format(const char *path)
{
    allocator_t *allocator = allocator_get_default_instance();
    File *file = NULL;
    uint8_t buf[512] = {0};
    int len = 0;
    archive_format_e fmt = ARCHIVE_FORMAT_UNKNOWN;

    file = object_new(allocator, "File", NULL);
    if (file == NULL) return ARCHIVE_FORMAT_UNKNOWN;
    if (file->open(file, (char *)path, "r") >= 0) {
        len = file->read(file, buf, sizeof(buf));
        file->close(file);
        fmt = __detect_format_buf(buf, len);
    }
    object_destroy(file);

    return fmt;
}

static archive_format_e __detect_format_by_name(const char *path)
{
    const char *dot, *ext;
    char lower[16] = {0};
    int i = 0, len;

    if (path == NULL) return ARCHIVE_FORMAT_UNKNOWN;
    dot = strrchr(path, '.');
    if (dot == NULL) return ARCHIVE_FORMAT_UNKNOWN;
    ext = dot + 1;
    len = (int)strlen(ext);
    if (len > 15) len = 15;
    for (i = 0; i < len; i++) {
        char c = ext[i];
        lower[i] = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
    }
    lower[len] = '\0';

    if (strcmp(lower, "tar") == 0) return ARCHIVE_FORMAT_TAR;
    if (strcmp(lower, "tgz") == 0 || strcmp(lower, "gz") == 0) return ARCHIVE_FORMAT_TGZ;
    if (strcmp(lower, "tbz2") == 0 || strcmp(lower, "bz2") == 0) return ARCHIVE_FORMAT_TBZ2;
    if (strcmp(lower, "zip") == 0) return ARCHIVE_FORMAT_ZIP;
    if (strcmp(lower, "7z") == 0) return ARCHIVE_FORMAT_7Z;
    if (strcmp(lower, "sqfs") == 0 || strcmp(lower, "squashfs") == 0) return ARCHIVE_FORMAT_SQUASHFS;
    if (strcmp(lower, "xz") == 0) return ARCHIVE_FORMAT_XZ;
    if (strcmp(lower, "zst") == 0 || strcmp(lower, "zstd") == 0) return ARCHIVE_FORMAT_ZSTD;
    if (strcmp(lower, "lz4") == 0) return ARCHIVE_FORMAT_LZ4;
    if (strcmp(lower, "rar") == 0) return ARCHIVE_FORMAT_RAR;
    if (strcmp(lower, "cpio") == 0) return ARCHIVE_FORMAT_CPIO;
    if (strcmp(lower, "iso") == 0) return ARCHIVE_FORMAT_ISO;

    return ARCHIVE_FORMAT_UNKNOWN;
}

/* 去掉 basename 的压缩后缀, 得到内层 tar 名 */
static void __strip_compress_ext(char *base)
{
    char *p;

    p = strstr(base, ".gz");
    if (p) { *p = '\0'; return; }
    p = strstr(base, ".bz2");
    if (p) { *p = '\0'; return; }
    p = strstr(base, ".tgz");
    if (p) { *p = '\0'; strcat(base, ".tar"); return; }
    p = strstr(base, ".tbz2");
    if (p) { *p = '\0'; strcat(base, ".tar"); return; }
}

/* 打开 tar+gzip / tar+bzip2: 先解出内层 tar(放输入同目录), 再按 Tar 打开 */
static Archive *__open_tar_compressed(allocator_t *allocator, const char *path,
                                      const char *mode, archive_format_e fmt)
{
    const char *compr_class = (fmt == ARCHIVE_FORMAT_TGZ) ? "GZCompress" : "Bz2Compress";
    Archive *a = NULL;
    Compress *c = NULL;
    char tar_path[1024] = {0};
    char path_copy[1024] = {0};
    char base[512] = {0};
    char *dir = NULL, *name = NULL;
    int ret;

    /* 写模式由 create 单独处理(先打 tar 再压缩), 这里只支持读 */
    if (mode[0] == 'w' || mode[0] == 'a') return NULL;

    TRY {
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        if (fs_get_path_and_name(path_copy, &dir, &name) == 0) name = path_copy;
        strncpy(base, name, sizeof(base) - 1);
        __strip_compress_ext(base);

        if (dir && dir[0]) snprintf(tar_path, sizeof(tar_path), "%s/%s", dir, base);
        else snprintf(tar_path, sizeof(tar_path), "./%s", base);

        c = (Compress *)object_new(allocator, (char *)compr_class, NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->uncompress_file(c, (char *)path, tar_path));
        object_destroy(c);
        c = NULL;

        /* 校验内层确为 tar, 否则是纯压缩流而非归档 */
        THROW_IF(__detect_format(tar_path) != ARCHIVE_FORMAT_TAR, -1);

        a = (Archive *)object_new(allocator, "Tar", NULL);
        THROW_IF(a == NULL, -1);
        EXEC(a->open(a, tar_path, "r"));
    } CATCH (ret) {
        if (c) object_destroy(c);
        if (a) { object_destroy(a); a = NULL; }
        fs_rmfile(tar_path);
    }

    return a;
}

/* 规范化目录路径: 补上结尾 '/' (fs_get_relative_path 要求 root 以 '/' 结尾) */
static void __normalize_dir_path(char *buf, size_t size, const char *path)
{
    int len;

    snprintf(buf, size, "%s", path ? path : "./");
    len = strlen(buf);
    if (len > 0 && buf[len - 1] != '/') {
        strncat(buf, "/", size - len - 1);
    }
}

/* 通用打开: 魔数识别(读模式) / 扩展名识别(写模式), 再分发到对应子类 */
static Archive *__open(allocator_t *allocator, const char *path, const char *mode)
{
    archive_format_e fmt = ARCHIVE_FORMAT_UNKNOWN;
    const char *class_name;
    Archive *a = NULL;
    int write_mode = (mode && (mode[0] == 'w' || mode[0] == 'a'));

    if (path == NULL || mode == NULL) return NULL;

    if (write_mode || fs_is_exist((char *)path) == 0) {
        /* 文件不存在(或要新建): 按扩展名识别 */
        fmt = __detect_format_by_name(path);
    } else {
        fmt = __detect_format(path);
        /* gzip/bzip2 魔数可能是 .tar.gz/.tar.bz2, 用扩展名细化 */
        if (fmt == ARCHIVE_FORMAT_GZIP || fmt == ARCHIVE_FORMAT_BZIP2) {
            archive_format_e ext_fmt = __detect_format_by_name(path);
            if (ext_fmt == ARCHIVE_FORMAT_TGZ) fmt = ARCHIVE_FORMAT_TGZ;
            else if (ext_fmt == ARCHIVE_FORMAT_TBZ2) fmt = ARCHIVE_FORMAT_TBZ2;
        }
        if (fmt == ARCHIVE_FORMAT_UNKNOWN) fmt = __detect_format_by_name(path);
    }

    switch (fmt) {
    case ARCHIVE_FORMAT_TGZ:
    case ARCHIVE_FORMAT_TBZ2:
        a = __open_tar_compressed(allocator, path, mode, fmt);
        break;
    case ARCHIVE_FORMAT_GZIP:
    case ARCHIVE_FORMAT_BZIP2:
    case ARCHIVE_FORMAT_XZ:
    case ARCHIVE_FORMAT_ZSTD:
    case ARCHIVE_FORMAT_LZ4:
    case ARCHIVE_FORMAT_RAR:
    case ARCHIVE_FORMAT_ISO:
    case ARCHIVE_FORMAT_CPIO:
    case ARCHIVE_FORMAT_AR:
    case ARCHIVE_FORMAT_CAB:
    case ARCHIVE_FORMAT_Z:
        dbg_str(DBG_WARN, "archive: %s is a raw/unsupported stream, can't open as archive", path);
        a = NULL;
        break;
    default:
        class_name = __format_class_name(fmt);
        if (class_name == NULL) {
            dbg_str(DBG_WARN, "archive: unknown format of %s", path);
            return NULL;
        }
        a = (Archive *)object_new(allocator, (char *)class_name, NULL);
        if (a != NULL) a->open(a, (char *)path, (char *)mode);
        break;
    }

    return a;
}

/* ================= 子命令动作 ================= */

/* 用 -w 通配符过滤 list 结果(命令层统一过滤, 与格式无关);
 * 匹配规则与基类 __filter 的包含通配符一致: 条目名含该子串即命中.
 * 无通配符时直接返回原 infos; 否则新建一个浅引用 Vector(不持有元素). */
static int __select_infos(Archive_Command *command, Archive *a, Vector *infos, Vector **out)
{
    allocator_t *allocator = a->parent.allocator;
    Vector *sel = NULL;
    archive_file_info_t *info;
    const char *wc;
    uint8_t value_type = VALUE_TYPE_STRUCT_POINTER, trustee_flag = 0;
    int ret, i, cnt;

    *out = infos;
    if (command->wildcard == NULL || strlen(STR2A(command->wildcard)) == 0) return 1;
    wc = STR2A(command->wildcard);

    TRY {
        sel = object_new(allocator, "Vector", NULL);
        THROW_IF(sel == NULL, -1);
        sel->set(sel, "/Vector/value_type", &value_type);
        sel->set(sel, "/Vector/trustee_flag", &trustee_flag);
        cnt = infos->count(infos);
        for (i = 0; i < cnt; i++) {
            info = NULL;
            EXEC(infos->peek_at(infos, i, &info));
            if (info == NULL || info->file_name == NULL) continue;
            if (strstr(info->file_name, wc) == NULL) continue;
            EXEC(sel->add(sel, info));
        }
        *out = sel;
        sel = NULL;
    } CATCH (ret) {} FINALLY {
        object_destroy(sel);
    }

    return ret;
}

static int __cmd_list(Archive_Command *command)
{
    allocator_t *allocator = ((Command *)command)->parent.allocator;
    Archive *a = command->archive;
    Vector *infos = NULL, *sel = NULL;
    archive_file_info_t *info;
    int ret, count, i;

    if (a == NULL) a = __open(allocator, command->arg1, "r");
    if (a == NULL) return -1;

    TRY {
        EXEC(a->list(a, &infos));
        EXEC(__select_infos(command, a, infos, &sel));
        count = sel->count(sel);
        printf("total %d entries\n", count);
        for (i = 0; i < count; i++) {
            info = NULL;
            EXEC(sel->peek_at(sel, i, &info));
            if (info != NULL) printf("%s\n", info->file_name);
        }
        command->archive = a;
    } CATCH (ret) {} FINALLY {
        if (sel != NULL && sel != infos) object_destroy(sel);
    }

    return ret;
}

static int __cmd_extract(Archive_Command *command)
{
    allocator_t *allocator = ((Command *)command)->parent.allocator;
    Archive *a = command->archive;
    Vector *infos = NULL, *sel = NULL;
    int ret;

    if (a == NULL) a = __open(allocator, command->arg1, "r");
    if (a == NULL) return -1;

    TRY {
        if (command->output != NULL && strlen(STR2A(command->output)) > 0) {
            char out_dir[1024] = {0};

            /* 规范化输出目录(补结尾 '/'), 避免 tar 等把 extracting_path 与文件名直接拼接 */
            __normalize_dir_path(out_dir, sizeof(out_dir), STR2A(command->output));
            fs_mkdir(out_dir, 0777);
            EXEC(a->set_extracting_path(a, out_dir));
        }
        /* list -> 按 -w 过滤 -> 抽取(命令层统一过滤, 与格式无关) */
        EXEC(a->list(a, &infos));
        EXEC(__select_infos(command, a, infos, &sel));
        EXEC(a->extract_files(a, sel));
        printf("extracted %s to %s\n", command->arg1, STR2A(a->extracting_path));
        command->archive = a;
    } CATCH (ret) {} FINALLY {
        if (sel != NULL && sel != infos) object_destroy(sel);
    }

    return ret;
}

static int __cmd_create(Archive_Command *command)
{
    allocator_t *allocator = ((Command *)command)->parent.allocator;
    Archive *a = NULL;
    archive_format_e fmt;
    char tmp_tar[1024] = {0};
    const char *out = command->arg1;
    int ret;

    if (command->arg1 == NULL || command->arg2 == NULL) {
        dbg_str(DBG_WARN, "usage: archive create <outfile> <srcpath>");
        return -1;
    }

    TRY {
        char add_path[1024] = {0};

        __normalize_dir_path(add_path, sizeof(add_path), command->arg2);
        fmt = __detect_format_by_name(out);
        if (fmt == ARCHIVE_FORMAT_TGZ || fmt == ARCHIVE_FORMAT_TBZ2) {
            /* 先打 tar, 再压缩成 .tgz/.tbz2 */
            Compress *c;
            snprintf(tmp_tar, sizeof(tmp_tar), "%s.tmp.tar", out);
            a = (Archive *)object_new(allocator, "Tar", NULL);
            THROW_IF(a == NULL, -1);
            EXEC(a->open(a, tmp_tar, "w+"));
            EXEC(a->set_adding_path(a, add_path));
            EXEC(a->add(a));
            EXEC(a->close(a));

            c = (Compress *)object_new(allocator, (fmt == ARCHIVE_FORMAT_TGZ) ? "GZCompress" : "Bz2Compress", NULL);
            THROW_IF(c == NULL, -1);
            EXEC(c->compress_file(c, tmp_tar, (char *)out));
            object_destroy(c);
            fs_rmfile(tmp_tar);
        } else {
            a = __open(allocator, out, "w+");
            THROW_IF(a == NULL, -1);
            EXEC(a->set_adding_path(a, add_path));
            EXEC(a->add(a));
        }
        command->archive = a;
        printf("created %s from %s\n", out, command->arg2);
    } CATCH (ret) {}

    return ret;
}

static int __cmd_add(Archive_Command *command)
{
    allocator_t *allocator = ((Command *)command)->parent.allocator;
    Archive *a;
    archive_file_info_t info;
    int ret;

    if (command->arg1 == NULL || command->arg2 == NULL) {
        dbg_str(DBG_WARN, "usage: archive add <archive> <srcfile>");
        return -1;
    }

    TRY {
        a = __open(allocator, command->arg1, "r+");
        THROW_IF(a == NULL, -1);
        memset(&info, 0, sizeof(info));
        info.file_name = command->arg2;
        EXEC(a->add_file(a, &info));
        EXEC(a->save(a));
        command->archive = a;
        printf("added %s to %s\n", command->arg2, command->arg1);
    } CATCH (ret) {}

    return ret;
}

/* ================= run_command 分发 ================= */

static int __run_command(Archive_Command *command)
{
    int ret = 0;

    TRY {
        switch (command->command_type) {
        case ARCHIVE_CMD_LIST:       EXEC(__cmd_list(command));       break;
        case ARCHIVE_CMD_EXTRACT:    EXEC(__cmd_extract(command));    break;
        case ARCHIVE_CMD_CREATE:     EXEC(__cmd_create(command));     break;
        case ARCHIVE_CMD_ADD:        EXEC(__cmd_add(command));        break;
        default:
            dbg_str(DBG_WARN, "archive: unknown sub command, run 'archive help'");
            THROW(-1);
        }
    } CATCH (ret) {}

    return ret;
}

/* ================= 参数 / 选项回调 ================= */

struct archive_subcmd_s {
    const char *name;
    archive_command_type_e type;
} archive_subcmd_table[] = {
    { "list",       ARCHIVE_CMD_LIST },
    { "extract",    ARCHIVE_CMD_EXTRACT },
    { "create",     ARCHIVE_CMD_CREATE },
    { "add",        ARCHIVE_CMD_ADD },
};

static int __argument_arg0_action_callback(Argument *arg, void *opaque)
{
    Archive_Command *c = (Archive_Command *)opaque;
    int i, n = (int)(sizeof(archive_subcmd_table) / sizeof(archive_subcmd_table[0]));

    c->command_type = ARCHIVE_CMD_UNKNOWN;
    for (i = 0; i < n; i++) {
        if (strcmp(STR2A(arg->value), archive_subcmd_table[i].name) == 0) {
            c->command_type = archive_subcmd_table[i].type;
            break;
        }
    }

    return 0;
}

static int __argument_arg1_action_callback(Argument *arg, void *opaque)
{
    Archive_Command *c = (Archive_Command *)opaque;
    c->arg1 = STR2A(arg->value);

    return 0;
}

static int __argument_arg2_action_callback(Argument *arg, void *opaque)
{
    Archive_Command *c = (Archive_Command *)opaque;
    c->arg2 = STR2A(arg->value);

    return 0;
}

static int __option_output_callback(Option *option, void *opaque)
{
    Archive_Command *c = (Archive_Command *)opaque;
    c->output->assign(c->output, STR2A(option->value));

    return 0;
}

static int __option_wildcard_callback(Option *option, void *opaque)
{
    Archive_Command *c = (Archive_Command *)opaque;
    c->wildcard->assign(c->wildcard, STR2A(option->value));

    return 0;
}

/* ================= 构造 / 析构 ================= */

static int __construct(Archive_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;

    command->output = object_new(allocator, "String", NULL);
    command->wildcard = object_new(allocator, "String", NULL);
    command->output->assign(command->output, "");
    command->wildcard->assign(command->wildcard, "");

    c->set(c, "/Command/name", "archive");
    c->add_option(c, "--output", "-o", "", "output directory (extract)",
                  __option_output_callback, command);
    c->add_option(c, "--wildcard", "-w", "", "include wildcard filter (e.g. *.txt)",
                  __option_wildcard_callback, command);
    c->add_argument(c, "", "sub command: list/extract/create/add",
                    __argument_arg0_action_callback, command);
    c->add_argument(c, "", "archive file path",
                    __argument_arg1_action_callback, command);
    c->add_argument(c, "", "source path (create) or source file (add)",
                    __argument_arg2_action_callback, command);
    c->set(c, "/Command/description",
           "universal archive tool: auto-detect format (tar/zip/7z/squashfs/tgz/tbz2) and open it.");

    return 0;
}

static int __deconstruct(Archive_Command *command)
{
    object_destroy(command->output);
    object_destroy(command->wildcard);
    if (command->archive != NULL) object_destroy(command->archive);

    return 0;
}

static class_info_entry_t archive_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Archive_Command, construct, __construct),
    Init_Nfunc_Entry(2, Archive_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Archive_Command, run_command, __run_command),
    Init_End___Entry(4, Archive_Command),
};
REGISTER_APP_CMD(Archive_Command, archive_command_class_info);
