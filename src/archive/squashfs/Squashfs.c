/**
 * @file Squashfs.c
 * @Synopsis
 *     SquashFS 4.0 只读压缩文件系统镜像的读写. 数据/元数据块用 zlib 压缩,
 *     由 libz 提供. 本实现把 SquashFS 当作普通归档处理: open/list/extract;
 *     写入生成一个最小合法镜像(mksquashfs 风格).
 *
 * ---- SquashFS 4.0 布局 ----
 *   [superblock 96 字节]
 *   [数据块区]   文件数据块(每个: 2 字节头 + 数据) 与 fragment
 *   [inode 表]   元数据块(每个解压后 8KB, 2 字节头标记压缩)
 *   [directory 表]
 *   [fragment 表 + 索引]  [export/lookup 表]  [uid/gid 表]
 *
 * 元数据块头: u16, bit15=未压缩标志, 低 15 位 = 后续数据长度.
 *   - 压缩: 数据是 zlib 流, 解压为 8192 字节
 *   - 未压缩: 数据直接就是 8192 字节
 * inode 定位: 48 位 (block<<16 | offset), block 是相对表起始的元数据块序号,
 *             offset 是解压后块内的字节偏移.
 * @author alan lin
 * @version
 * @date 2023-12-19
 */
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/File.h>
#include "Squashfs.h"

/* ------------------------------------------------------------------ */
/* helpers (use byteorder.h)                                          */
/* ------------------------------------------------------------------ */

static uint16_t __le16(const uint8_t *p)
{
    uint16_t v;
    memcpy(&v, p, 2);
    return byteorder_le16_to_cpu(&v);
}

static uint32_t __le32(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return byteorder_le32_to_cpu(&v);
}

static uint64_t __le64(const uint8_t *p)
{
    uint64_t v;
    memcpy(&v, p, 8);
    return byteorder_le64_to_cpu(&v);
}

static void __put_le16(uint8_t *p, uint16_t v)
{
    byteorder_cpu_to_le16(&v);
    memcpy(p, &v, 2);
}

static void __put_le32(uint8_t *p, uint32_t v)
{
    byteorder_cpu_to_le32(&v);
    memcpy(p, &v, 4);
}

static void __put_le64(uint8_t *p, uint64_t v)
{
    byteorder_cpu_to_le64(&v);
    memcpy(p, &v, 8);
}

/* 取路径最后一个 '/' 之后的部分(裸文件名); 无 '/' 则原样返回 */
static const char *__basename(const char *path)
{
    const char *s = path ? strrchr(path, '/') : NULL;
    return (s && s[1]) ? s + 1 : path;
}

/* zlib 解压一个块(数据块/元数据块) */
static int __zlib_uncompress(const uint8_t *in, uint32_t in_len, uint8_t *out, uLongf *out_len)
{
    return uncompress(out, out_len, in, in_len) == Z_OK ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* free helpers                                                       */
/* ------------------------------------------------------------------ */

static void __free_all(Squashfs *sq)
{
    allocator_t *allocator = sq->parent.parent.allocator;
    uint32_t i;

    if (sq->inode_table) allocator_mem_free(allocator, sq->inode_table);
    if (sq->dir_table) allocator_mem_free(allocator, sq->dir_table);
    if (sq->id_table) allocator_mem_free(allocator, sq->id_table);
    if (sq->fragment_index) allocator_mem_free(allocator, sq->fragment_index);
    for (i = 0; i < sq->num_files; i++) {
        if (sq->files[i].name) allocator_mem_free(allocator, sq->files[i].name);
        if (sq->files[i].block_list) allocator_mem_free(allocator, sq->files[i].block_list);
    }
    if (sq->files) allocator_mem_free(allocator, sq->files);
    if (sq->pending_sizes) allocator_mem_free(allocator, sq->pending_sizes);

    sq->inode_table = sq->dir_table = sq->id_table = sq->fragment_index = NULL;
    sq->files = NULL;
    sq->pending_sizes = NULL;
    sq->num_files = 0;
}

/* ------------------------------------------------------------------ */
/* metadata 表加载                                                    */
/* ------------------------------------------------------------------ */

/* 读取 start..end 之间的一串元数据块并逐块解压, 拼成连续缓冲.
 * (SquashFS 的 inode/directory/id/fragment 表都由元数据块组成) */
static int __load_metadata_region(Squashfs *sq, uint64_t start, uint64_t end,
                                  uint8_t **out, uint64_t *out_size)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t *buf = NULL;
    uint64_t cap = 0, len = 0, pos = start;
    int ret = 0;

    TRY {
        while (pos + SQFS_BLOCK_HEADER_SIZE <= end) {
            uint8_t hdr[2];
            uint16_t h, csize;
            int uncompressed;
            uint8_t *raw;
            uLongf dlen = SQFS_METADATA_SIZE;

            EXEC(a->seek(a, pos, SEEK_SET));
            EXEC(a->read(a, hdr, 2));
            h = (uint16_t)__le16(hdr);
            uncompressed = (h & SQFS_COMPRESSED_BIT) != 0;
            csize = h & SQFS_SIZE_MASK;
            pos += 2;
            if (csize == 0) break;
            THROW_IF(pos + csize > end, -1);

            raw = allocator_mem_alloc(allocator, csize);
            THROW_IF(raw == NULL, -1);
            EXEC(a->read(a, raw, csize));
            pos += csize;

            if (len + SQFS_METADATA_SIZE > cap) {
                uint64_t ncap = cap ? cap * 2 : SQFS_METADATA_SIZE * 16;
                uint8_t *nb = allocator_mem_zalloc(allocator, ncap);
                THROW_IF(nb == NULL, -1);
                if (buf) memcpy(nb, buf, len);
                if (buf) allocator_mem_free(allocator, buf);
                buf = nb;
                cap = ncap;
            }
            if (uncompressed) {
                memcpy(buf + len, raw, csize);
                len += csize;
            } else {
                THROW_IF(__zlib_uncompress(raw, csize, buf + len, &dlen) != 0, -1);
                len += dlen;
            }
            allocator_mem_free(allocator, raw);
        }
        *out = buf;
        *out_size = len;
        buf = NULL;
    } CATCH (ret) {} FINALLY {
        if (buf) allocator_mem_free(allocator, buf);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* inode 解析                                                         */
/* ------------------------------------------------------------------ */

typedef struct sqfs_inode_s {
    uint32_t type;
    uint32_t mode;
    uint64_t inode_number;
    /* regular file */
    uint64_t start_block;       /* 数据块区绝对偏移 */
    uint32_t fragment;          /* fragment 索引或 SQFS_NO_FRAGMENT */
    uint32_t fragment_offset;
    uint64_t file_size;
    uint32_t block_count;
    uint32_t *block_list;       /* 各数据块在磁盘上的大小(含 2 字节头) */
    /* directory */
    uint32_t dir_start;         /* 目录数据所在元数据块在 dir 表内的相对字节偏移 */
    uint16_t dir_offset;        /* 目录数据在解压块内的字节偏移 */
    uint32_t dir_file_size;     /* 目录数据大小 + 3 */
} sqfs_inode_t;

/* 在解压后的 inode 表缓冲中解析 (block, offset) 处的 inode */
static int __parse_inode(Squashfs *sq, uint32_t block, uint16_t offset, sqfs_inode_t *ino)
{
    allocator_t *allocator = sq->parent.parent.allocator;
    const uint8_t *p;
    uint64_t off;
    uint32_t i;
    int ret = 0;

    TRY {
        off = (uint64_t)block * SQFS_METADATA_SIZE + offset;
        THROW_IF(off + 16 > sq->inode_table_size, -1);
        p = sq->inode_table + off;

        memset(ino, 0, sizeof(*ino));
        ino->type = __le16(p);
        ino->mode = __le16(p + 2);
        ino->inode_number = __le32(p + 12);
        p += 16;

        if (ino->type == SQFS_INODE_FILE || ino->type == SQFS_INODE_EXT_FILE) {
            /* 基本文件 inode(32 字节头): basic(16) + start_block(4) + fragment(4)
             * + offset(4, 碎片内偏移, 无 fragment 也恒存在) + file_size(4) + block_list[] */
            ino->start_block = __le32(p); p += 4;
            ino->fragment = __le32(p); p += 4;
            ino->fragment_offset = __le32(p); p += 4;
            ino->file_size = __le32(p); p += 4;
            /* 数据块数量: 若无 fragment 尾部, 最后一小块也是一个数据块 */
            ino->block_count = (uint32_t)((ino->file_size + sq->super.block_size - 1) / sq->super.block_size);
            if (ino->fragment != SQFS_NO_FRAGMENT && ino->file_size % sq->super.block_size != 0) {
                ino->block_count = (uint32_t)(ino->file_size / sq->super.block_size);
            }
            if (ino->block_count) {
                ino->block_list = allocator_mem_alloc(allocator, sizeof(uint32_t) * ino->block_count);
                THROW_IF(ino->block_list == NULL, -1);
                for (i = 0; i < ino->block_count; i++) {
                    ino->block_list[i] = __le32(p); p += 4;
                }
            }
        } else if (ino->type == SQFS_INODE_DIR || ino->type == SQFS_INODE_EXT_DIR) {
            /* 基本目录 inode(32 字节): basic(16) + start_block(4) + nlink(4)
             * + file_size(2) + offset(2) + parent_inode(4) */
            ino->dir_start = __le32(p); p += 4;      /* start_block */
            p += 4;                                  /* nlink */
            ino->dir_file_size = __le16(p); p += 2;  /* 目录大小 + 3 */
            ino->dir_offset = __le16(p); p += 2;     /* 解压块内偏移 */
            /* parent_inode(4) 可忽略; 扩展目录还有 index 信息, 这里不解析 */
        }
    } CATCH (ret) {} FINALLY {
        if (ino->block_list && ret != 0) allocator_mem_free(allocator, ino->block_list);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* 目录遍历: 从根目录递归收集所有文件                                  */
/* ------------------------------------------------------------------ */

static int __dir_add_file(Squashfs *sq, const char *path, uint32_t block, uint16_t offset)
{
    allocator_t *allocator = sq->parent.parent.allocator;
    sqfs_file_entry_t *nf, *f;
    sqfs_inode_t ino;
    int ret = 0;

    TRY {
        nf = allocator_mem_zalloc(allocator, sizeof(sqfs_file_entry_t) * (sq->num_files + 1));
        THROW_IF(nf == NULL, -1);
        if (sq->files && sq->num_files) {
            memcpy(nf, sq->files, sizeof(sqfs_file_entry_t) * sq->num_files);
        }
        if (sq->files) allocator_mem_free(allocator, sq->files);
        sq->files = nf;
        f = &sq->files[sq->num_files];
        memset(f, 0, sizeof(*f));
        f->name = allocator_mem_zalloc(allocator, strlen(path) + 1);
        THROW_IF(f->name == NULL, -1);
        strcpy((char *)f->name, path);
        f->inode_block = block;
        f->inode_offset = offset;

        /* 解析该文件 inode, 记录大小(供 list/extract 使用) */
        EXEC(__parse_inode(sq, block, offset, &ino));
        f->size = ino.file_size;

        sq->num_files++;
    } CATCH (ret) {}

    return ret;
}

static int __walk_dir(Squashfs *sq, const char *path, uint32_t dstart);

static int __walk_dir_entries(Squashfs *sq, const char *path, const uint8_t *dir, uint32_t dir_size)
{
    const uint8_t *p = dir, *end = dir + dir_size;
    uint32_t count, start_block, base_inode, i;
    int ret = 0;

    TRY {
        while (p + 12 <= end) {
            uint32_t raw_count = __le32(p);
            count = (raw_count & 0x7FFF) + 1;    /* 磁盘上存的是 count-1 */
            start_block = __le32(p + 4);
            base_inode = __le32(p + 8);
            p += 12;

            if (raw_count & SQFS_DIR_ENTRY_HEADER) {
                /* bit15 置位表示带目录 index 块, 跳过它
                 * (index 块: index, start_block, size, name[]) */
                uint32_t isize = __le32(p + 8);
                THROW_IF(p + 12 + isize + 1 > end, -1);
                p += 12 + isize + 1;
            }

            for (i = 0; i < count; i++) {
                uint16_t offset, size, type;
                int16_t ino_delta;
                uint32_t child_block = start_block;
                uint16_t child_offset;
                char name[256];
                char full[1024];
                uint32_t child_inode_number;

                THROW_IF(p + 8 > end, -1);
                offset = __le16(p);
                ino_delta = (int16_t)__le16(p + 2);
                type = __le16(p + 4);
                size = __le16(p + 6);
                p += 8;
                THROW_IF(p + (uint32_t)(size + 1) > end, -1);
                memcpy(name, p, size + 1);
                p += size + 1;
                name[size + 1] = '\0';

                child_inode_number = base_inode + (uint32_t)ino_delta;
                child_offset = offset;
                (void)child_inode_number;

                THROW_IF(strlen(path) + 1 + strlen(name) + 1 >= sizeof(full), -1);
                if (path[0]) {
                    strcpy(full, path);
                    strcat(full, "/");
                } else {
                    full[0] = '\0';
                }
                strcat(full, name);

                if (type == SQFS_INODE_FILE || type == SQFS_INODE_EXT_FILE) {
                    EXEC(__dir_add_file(sq, full, child_block, child_offset));
                } else if (type == SQFS_INODE_DIR || type == SQFS_INODE_EXT_DIR) {
                    EXEC(__walk_dir(sq, full, ((uint32_t)child_block << 16) | child_offset));
                }
            }
        }
    } CATCH (ret) {}

    return ret;
}

static int __walk_dir(Squashfs *sq, const char *path, uint32_t dstart)
{
    uint32_t block = dstart >> 16;
    uint16_t offset = dstart & 0xFFFF;
    sqfs_inode_t ino;
    const uint8_t *dir;
    uint64_t off;
    int ret = 0;

    TRY {
        EXEC(__parse_inode(sq, block, offset, &ino));
        THROW_IF(ino.type != SQFS_INODE_DIR && ino.type != SQFS_INODE_EXT_DIR, -1);
        /* 目录数据定位: dir_start 是该数据所在元数据块在 dir 表内的相对
         * 字节偏移, dir_offset 是解压块内偏移. 仅支持单块目录表(start=0) */
        THROW_IF(ino.dir_start != 0, -1);
        off = ino.dir_offset;
        THROW_IF(off >= sq->dir_table_size, -1);
        dir = sq->dir_table + off;
        /* 目录 inode 的 file_size 存的是 实际大小+3, 读时减 3 */
        EXEC(__walk_dir_entries(sq, path, dir, ino.dir_file_size > 3 ? ino.dir_file_size - 3 : 0));
    } CATCH (ret) {}

    return ret;
}

/* ------------------------------------------------------------------ */
/* 数据块 / fragment 读取                                             */
/* ------------------------------------------------------------------ */

/* 在 image 的 *pos 处读取一个数据块(无长度头, 大小/压缩标志由调用方从
 * inode 的 block_list 或 fragment 表给出), 解压并写入 out.
 * *pos 推进到该块之后, 支持顺序读取连续的数据块. */
static int __read_data_block(Squashfs *sq, uint64_t *pos, uint32_t csize, int uncompressed,
                             uint8_t *out, uint32_t *out_len, uint32_t max_out)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t *raw = NULL;
    uLongf dlen = max_out;
    int ret = 0;

    TRY {
        THROW_IF(csize == 0, -1);
        raw = allocator_mem_alloc(allocator, csize);
        THROW_IF(raw == NULL, -1);
        EXEC(a->seek(a, *pos, SEEK_SET));
        EXEC(a->read(a, raw, csize));
        *pos += csize;

        if (uncompressed) {
            *out_len = csize < max_out ? csize : max_out;
            memcpy(out, raw, *out_len);
        } else {
            THROW_IF(__zlib_uncompress(raw, csize, out, &dlen) != 0, -1);
            *out_len = dlen;
        }
        allocator_mem_free(allocator, raw);
        raw = NULL;
    } CATCH (ret) {} FINALLY {
        if (raw) allocator_mem_free(allocator, raw);
    }

    return ret;
}

/* 读取 fragment 表, 得到第 idx 个 fragment 条目 (start, size) */
static int __read_fragment_entry(Squashfs *sq, uint32_t idx, uint64_t *fstart, uint32_t *fsize)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint64_t block_pos;
    uint8_t *raw;
    uint16_t h, csize;
    uLongf dlen = SQFS_METADATA_SIZE;
    uint8_t *block = NULL;
    int uncompressed, ret = 0;

    TRY {
        THROW_IF(sq->super.fragment_count == 0, -1);
        THROW_IF(idx >= sq->super.fragment_count, -1);

        /* fragment 表由元数据块组成, fragment_index 指向各表块在镜像中的偏移 */
        block_pos = __le64(sq->fragment_index + ((idx / 512) * 8));
        EXEC(a->seek(a, block_pos, SEEK_SET));
        EXEC(a->read(a, (uint8_t *)&h, 2));
        /* 头在镜像里是小端 */
        h = (uint16_t)(((uint8_t *)&h)[0] | ((uint8_t *)&h)[1] << 8);
        uncompressed = (h & SQFS_COMPRESSED_BIT) != 0;
        csize = h & SQFS_SIZE_MASK;
        raw = allocator_mem_alloc(allocator, csize);
        THROW_IF(raw == NULL, -1);
        EXEC(a->read(a, raw, csize));
        block = allocator_mem_alloc(allocator, SQFS_METADATA_SIZE);
        THROW_IF(block == NULL, -1);
        if (uncompressed) {
            memcpy(block, raw, csize);
        } else {
            THROW_IF(__zlib_uncompress(raw, csize, block, &dlen) != 0, -1);
        }
        allocator_mem_free(allocator, raw);

        {
            uint32_t off = (idx % 512) * 16;
            *fstart = __le64(block + off);
            *fsize = __le32(block + off + 8);
        }
        allocator_mem_free(allocator, block);
    } CATCH (ret) {} FINALLY {
        if (raw) allocator_mem_free(allocator, raw);
        if (block) allocator_mem_free(allocator, block);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* 对象生命周期                                                       */
/* ------------------------------------------------------------------ */

static int __construct(Squashfs *sq, char *init_str)
{
    allocator_t *allocator = sq->parent.parent.allocator;

    sq->file = object_new(allocator, "File", NULL);
    sq->file_name = object_new(allocator, "String", NULL);
    sq->buffer = object_new(allocator, "Buffer", NULL);
    sq->add_flag = 0;
    /* 写新镜像时的默认块大小(读取时会被 superblock 覆盖) */
    sq->super.block_size = 131072;
    sq->super.block_log = 17;

    return 0;
}

static int __deconstruct(Squashfs *sq)
{
    __free_all(sq);
    object_destroy(sq->buffer);
    object_destroy(sq->file_name);
    object_destroy(sq->file);

    return 0;
}

static int __open(Squashfs *sq, char *archive_name, char *mode)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t sb[96];
    uint64_t dir_end;
    int size, ret = 0;

    TRY {
        dbg_str(DBG_VIP, "Squashfs open archive %s", archive_name);
        size = fs_get_size(archive_name);
        if (size <= 0) THROW(1);   /* empty/new archive */

        EXEC(a->seek(a, 0, SEEK_SET));
        EXEC(a->read(a, sb, 96));
        sq->super.magic = __le32(sb);
        THROW_IF(sq->super.magic != SQFS_MAGIC, -1);
        sq->super.inode_count = __le32(sb + 4);
        sq->super.mtime = __le32(sb + 8);
        sq->super.block_size = __le32(sb + 12);
        sq->super.fragment_count = __le32(sb + 16);
        sq->super.compression = __le16(sb + 20);
        sq->super.block_log = __le16(sb + 22);
        sq->super.flags = __le16(sb + 24);
        sq->super.id_count = __le16(sb + 26);
        sq->super.major = __le16(sb + 28);
        sq->super.minor = __le16(sb + 30);
        sq->super.root_inode = __le64(sb + 32);
        sq->super.bytes_used = __le64(sb + 40);
        sq->super.id_table_start = __le64(sb + 48);
        sq->super.xattr_table_start = __le64(sb + 56);
        sq->super.inode_table_start = __le64(sb + 64);
        sq->super.directory_table_start = __le64(sb + 72);
        sq->super.fragment_table_start = __le64(sb + 80);
        sq->super.lookup_table_start = __le64(sb + 88);

        THROW_IF(sq->super.compression != SQFS_COMPRESSION_ZLIB, -1);

        /* 加载 inode / directory / id 表 */
        EXEC(__load_metadata_region(sq, sq->super.inode_table_start, sq->super.directory_table_start,
                                    &sq->inode_table, &sq->inode_table_size));
        /* 目录表结束边界: 取下一张表(fragment/lookup/id)的位置; 0 或 INVALID_BLK 视为不存在, 否则到 bytes_used */
        dir_end = sq->super.fragment_table_start;
        if (dir_end == 0 || dir_end == SQFS_INVALID_BLK) dir_end = sq->super.lookup_table_start;
        if (dir_end == 0 || dir_end == SQFS_INVALID_BLK) dir_end = sq->super.id_table_start;
        if (dir_end == 0 || dir_end == SQFS_INVALID_BLK) dir_end = sq->super.bytes_used;
        EXEC(__load_metadata_region(sq, sq->super.directory_table_start, dir_end,
                                    &sq->dir_table, &sq->dir_table_size));
        /* id 表: 仅用于 uid/gid 映射, list/extract 不需要, 不加载 */
        /* fragment 索引表(结束边界取 lookup 表位置, INVALID_BLK 视为不存在) */
        if (sq->super.fragment_count && sq->super.fragment_table_start) {
            uint64_t frag_end = sq->super.lookup_table_start;
            if (frag_end == 0 || frag_end == SQFS_INVALID_BLK) frag_end = sq->super.bytes_used;
            EXEC(__load_metadata_region(sq, sq->super.fragment_table_start, frag_end,
                                        &sq->fragment_index, &sq->fragment_index_size));
        }

        /* 从根目录递归收集文件 */
        EXEC(__walk_dir(sq, "", (uint32_t)sq->super.root_inode));
    } CATCH (ret) {}

    return ret;
}

static int __list(Squashfs *sq, Vector **infos)
{
    Archive *archive = (Archive *)&sq->parent;
    Vector *files = archive->extracting_file_infos;
    allocator_t *allocator = files->obj.allocator;
    uint32_t i;
    int ret = 0;

    TRY {
        THROW_IF(files == NULL, -1);
        files->reset(files);
        for (i = 0; i < sq->num_files; i++) {
            archive_file_info_t *info = allocator_mem_alloc(allocator, sizeof(archive_file_info_t));
            THROW_IF(info == NULL, -1);
            memset(info, 0, sizeof(*info));
            info->file_name = allocator_mem_zalloc(allocator, strlen((char *)sq->files[i].name) + 1);
            THROW_IF(info->file_name == NULL, -1);
            strcpy(info->file_name, (char *)sq->files[i].name);
            info->size = (uint32_t)sq->files[i].size;
            EXEC(files->add(files, info));
        }
        *infos = archive->extracting_file_infos;
    } CATCH (ret) {}

    return ret;
}

/* 提取一个文件: 解析 inode, 依次读取连续的数据块, 再处理 fragment 尾部 */
static int __extract_file(Squashfs *sq, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&sq->parent;
    File *out = sq->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint32_t i, fi = 0xFFFFFFFF;
    uint64_t data_pos, remaining;
    uint32_t got, want, fsize;
    sqfs_inode_t ino;
    uint8_t *block = NULL;
    char out_name[1024];
    const char *wanted;
    int ret = 0;

    TRY {
        /* 镜像内存的是裸文件名(如 test.txt), 用请求名的 basename 匹配 */
        wanted = __basename(info->file_name);
        for (i = 0; i < sq->num_files; i++) {
            if (strcmp((char *)sq->files[i].name, wanted) == 0) {
                fi = i;
                break;
            }
        }
        THROW_IF(fi == 0xFFFFFFFF, -1);

        THROW_IF(strlen(STR2A(archive->extracting_path)) + strlen((char *)sq->files[fi].name) >= sizeof(out_name), -1);
        strcpy(out_name, STR2A(archive->extracting_path));
        strcat(out_name, (char *)sq->files[fi].name);

        EXEC(__parse_inode(sq, sq->files[fi].inode_block, sq->files[fi].inode_offset, &ino));
        THROW_IF(ino.type != SQFS_INODE_FILE && ino.type != SQFS_INODE_EXT_FILE, -1);

        EXEC(fs_mkfile(out_name, 0777));
        EXEC(out->open(out, out_name, "w+"));

        block = allocator_mem_alloc(allocator, sq->super.block_size);
        THROW_IF(block == NULL, -1);

        data_pos = ino.start_block;
        remaining = ino.file_size;
        for (i = 0; i < ino.block_count && remaining > 0; i++) {
            uint32_t blk = ino.block_list[i];
            got = 0;
            want = (uint32_t)(remaining > sq->super.block_size ? sq->super.block_size : remaining);
            EXEC(__read_data_block(sq, &data_pos, blk & SQFS_BLOCK_SIZE_MASK,
                                   (blk & SQFS_BLOCK_UNCOMPRESSED_BIT) != 0,
                                   block, &got, want));
            THROW_IF(got > remaining, -1);
            EXEC(out->write(out, block, got));
            remaining -= got;
        }
        /* fragment 尾部(文件剩余字节存在共享 fragment 块中) */
        if (remaining > 0 && ino.fragment != SQFS_NO_FRAGMENT) {
            uint64_t fstart = 0;
            EXEC(__read_fragment_entry(sq, ino.fragment, &fstart, &fsize));
            EXEC(__read_data_block(sq, &fstart, fsize & SQFS_BLOCK_SIZE_MASK,
                                   (fsize & SQFS_BLOCK_UNCOMPRESSED_BIT) != 0,
                                   block, &got, sq->super.block_size));
            THROW_IF((uint64_t)ino.fragment_offset + remaining > got, -1);
            EXEC(out->write(out, block + ino.fragment_offset, remaining));
            remaining = 0;
        }
        THROW_IF(remaining != 0, -1);
        out->close(out);
        THROW(1);
    } CATCH (ret) {} FINALLY {
        if (block) allocator_mem_free(allocator, block);
        out->close(out);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* 写入端: 生成最小合法 squashfs 镜像                                   */
/*   - 数据区: 每文件拆成 block_size 数据块, zlib 压缩(无收益则原样存储) */
/*   - 不使用 fragment / export / lookup 表                             */
/*   - inode 表: 根目录 inode + 每文件一个 inode                         */
/*   - directory 表: 根目录项                                          */
/* ------------------------------------------------------------------ */

/* 压缩一块数据, 返回压缩后长度; -1 表示压缩失败 */
static long __zlib_compress_buf(const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_cap)
{
    uLongf clen = out_cap;
    if (compress2(out, &clen, in, in_len, Z_BEST_COMPRESSION) != Z_OK) return -1;
    return (long)clen;
}

/* 把一个元数据块(8192 或更小)写入镜像, *pos 推进 */
static int __write_metadata_block(Squashfs *sq, uint64_t *pos, const uint8_t *data, uint32_t size)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t hdr[2];
    uint8_t *cmpr;
    uLongf clen;
    long c;
    int ret = 0;

    TRY {
        cmpr = allocator_mem_alloc(allocator, size + 64);
        THROW_IF(cmpr == NULL, -1);
        clen = size + 64;
        c = __zlib_compress_buf(data, size, cmpr, size + 64);
        EXEC(a->seek(a, *pos, SEEK_SET));
        if (c >= 0 && (uint32_t)c + 2 < size) {
            __put_le16(hdr, (uint16_t)c);
            EXEC(a->write(a, hdr, 2));
            EXEC(a->write(a, cmpr, (uint32_t)c));
            *pos += 2 + (uint32_t)c;
        } else {
            __put_le16(hdr, (uint16_t)(SQFS_COMPRESSED_BIT | size));
            EXEC(a->write(a, hdr, 2));
            EXEC(a->write(a, (void *)data, size));
            *pos += 2 + size;
        }
        allocator_mem_free(allocator, cmpr);
    } CATCH (ret) {}

    return ret;
}

static int __add_file(Squashfs *sq, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file, *file = sq->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t *data = NULL, *cmpr = NULL, zero[96];
    uint64_t data_size, pos;
    uint32_t i, blocks, chunk, stored;
    uint32_t *ns, *blist = NULL;
    sqfs_file_entry_t *nf, *f;
    long clen;
    int ret = 0;

    TRY {
        THROW_IF(info == NULL || info->file_name == NULL, -1);

        EXEC(file->open(file, info->file_name, "r+"));
        data_size = fs_get_size(info->file_name);
        data = allocator_mem_alloc(allocator, data_size ? data_size : 1);
        THROW_IF(data == NULL, -1);
        EXEC(file->read(file, data, data_size));

        /* 首次 add: 写入 96 字节 superblock 占位 */
        if (sq->add_flag == 0) {
            memset(zero, 0, sizeof(zero));
            EXEC(a->seek(a, 0, SEEK_SET));
            EXEC(a->write(a, zero, 96));
            sq->write_pos = 96;
            sq->add_flag = 1;
        }

        blocks = (uint32_t)((data_size + sq->super.block_size - 1) / sq->super.block_size);
        if (blocks == 0) blocks = 1;

        ns = allocator_mem_zalloc(allocator, sizeof(uint32_t) * (sq->pending_count + 1));
        THROW_IF(ns == NULL, -1);
        if (sq->pending_sizes && sq->pending_count) {
            memcpy(ns, sq->pending_sizes, sizeof(uint32_t) * sq->pending_count);
        }
        if (sq->pending_sizes) allocator_mem_free(allocator, sq->pending_sizes);
        sq->pending_sizes = ns;
        sq->pending_sizes[sq->pending_count] = (uint32_t)sq->write_pos;   /* 数据起始偏移 */

        cmpr = allocator_mem_alloc(allocator, sq->super.block_size + 64);
        THROW_IF(cmpr == NULL, -1);
        blist = allocator_mem_zalloc(allocator, sizeof(uint32_t) * blocks);
        THROW_IF(blist == NULL, -1);

        /* 数据块无长度头: 压缩(或原样)存储, 大小与压缩标志记录在 block_list */
        pos = sq->write_pos;
        for (i = 0; i < blocks; i++) {
            chunk = (uint32_t)((data_size - (uint64_t)i * sq->super.block_size) > sq->super.block_size
                               ? sq->super.block_size
                               : (data_size - (uint64_t)i * sq->super.block_size));
            if (chunk == 0) { blist[i] = 0; continue; }   /* 空块 = 稀疏 */
            clen = __zlib_compress_buf(data + (uint64_t)i * sq->super.block_size, chunk, cmpr, sq->super.block_size + 64);
            EXEC(a->seek(a, pos, SEEK_SET));
            if (clen < 0 || (uint32_t)clen >= chunk) {
                blist[i] = chunk | SQFS_BLOCK_UNCOMPRESSED_BIT;
                stored = chunk;
            } else {
                blist[i] = (uint32_t)clen;
                stored = (uint32_t)clen;
            }
            EXEC(a->write(a, stored == chunk ? data + (uint64_t)i * sq->super.block_size : cmpr, stored));
            pos += stored;
        }
        sq->write_pos = pos;
        sq->pending_count++;

        /* 记录文件元数据(供 save 生成 inode) */
        nf = allocator_mem_zalloc(allocator, sizeof(sqfs_file_entry_t) * (sq->num_files + 1));
        THROW_IF(nf == NULL, -1);
        if (sq->files && sq->num_files) memcpy(nf, sq->files, sizeof(sqfs_file_entry_t) * sq->num_files);
        if (sq->files) allocator_mem_free(allocator, sq->files);
        sq->files = nf;
        f = &sq->files[sq->num_files];
        memset(f, 0, sizeof(*f));
        f->name = allocator_mem_zalloc(allocator, strlen(info->file_name) + 1);
        THROW_IF(f->name == NULL, -1);
        strcpy((char *)f->name, info->file_name);
        f->size = data_size;
        f->inode_block = sq->pending_count - 1;   /* 数据起始偏移的下标, save 时用 */
        f->block_list = blist;
        f->num_blocks = blocks;
        blist = NULL;
        sq->num_files++;
    } CATCH (ret) {} FINALLY {
        if (data) allocator_mem_free(allocator, data);
        if (cmpr) allocator_mem_free(allocator, cmpr);
        if (blist) allocator_mem_free(allocator, blist);
    }

    return ret;
}

/* 写 96 字节 superblock(放在最后, 此时所有表的位置已知) */
static int __write_superblock(Squashfs *sq, uint32_t root_inode_loc)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    uint8_t sb[96];
    int ret = 0;

    TRY {
        memset(sb, 0, sizeof(sb));
        __put_le32(sb, SQFS_MAGIC);
        __put_le32(sb + 4, sq->num_files + 1);              /* inode_count(根目录 + 文件) */
        __put_le32(sb + 8, 0);                              /* mtime */
        __put_le32(sb + 12, sq->super.block_size);
        __put_le32(sb + 16, 0);                             /* fragment_count */
        __put_le16(sb + 20, SQFS_COMPRESSION_ZLIB);
        __put_le16(sb + 22, sq->super.block_log);
        __put_le16(sb + 24, SQFS_FLAG_NOXATTR);             /* flags: 无 xattr 表 */
        __put_le16(sb + 26, 1);                             /* id_count */
        __put_le16(sb + 28, 4);                             /* major */
        __put_le16(sb + 30, 0);                             /* minor */
        __put_le64(sb + 32, root_inode_loc);
        __put_le64(sb + 40, sq->write_pos);                 /* bytes_used */
        __put_le64(sb + 48, sq->super.id_table_start);
        __put_le64(sb + 56, SQFS_INVALID_BLK);              /* xattr: 无 xattr 表用 INVALID_BLK */
        __put_le64(sb + 64, sq->super.inode_table_start);
        __put_le64(sb + 72, sq->super.directory_table_start);
        __put_le64(sb + 80, 0);                             /* fragment_table_start(无 fragment) */
        __put_le64(sb + 88, SQFS_INVALID_BLK);              /* lookup(export)表不存在用 INVALID_BLK */

        EXEC(a->seek(a, 0, SEEK_SET));
        EXEC(a->write(a, sb, 96));
    } CATCH (ret) {}

    return ret;
}

static int __save(Squashfs *sq)
{
    Archive *archive = (Archive *)&sq->parent;
    File *a = archive->file;
    allocator_t *allocator = sq->parent.parent.allocator;
    uint8_t *inodes = NULL, *dirs = NULL;
    uint64_t inode_len = 0, dir_len = 0;
    uint32_t i, j, k, base_inode, dir_off;
    uint64_t id_data_pos;
    uint8_t id_data[4] = {0, 0, 0, 0};
    uint8_t id_index[8];
    sqfs_file_entry_t *f;
    int ret = 0;

    TRY {
        THROW_IF(sq->add_flag == 0, 1);

        /* 目录项必须按名字升序且唯一(unsquashfs 会校验), 先按 basename
         * 排序 files 及其对应的 pending_sizes(数据偏移) */
        for (i = 1; i < sq->num_files; i++) {
            k = i;
            while (k > 0 && strcmp(__basename((char *)sq->files[k].name),
                                   __basename((char *)sq->files[k - 1].name)) < 0) {
                sqfs_file_entry_t tf = sq->files[k];
                uint32_t ts = sq->pending_sizes[k];
                sq->files[k] = sq->files[k - 1];
                sq->files[k - 1] = tf;
                sq->pending_sizes[k] = sq->pending_sizes[k - 1];
                sq->pending_sizes[k - 1] = ts;
                k--;
            }
        }

        /* ---- 构建 inode 表: 根目录 inode + 每文件 inode ---- */
        /* 根目录 inode(32 字节): basic(16) + start_block(4) + nlink(4)
         * + file_size(2) + offset(2) + parent_inode(4) */
        inodes = allocator_mem_zalloc(allocator, 8192);
        THROW_IF(inodes == NULL, -1);
        __put_le16(inodes + 0, SQFS_INODE_DIR);
        __put_le16(inodes + 2, 0x41ED);                  /* S_IFDIR | 0755 */
        __put_le16(inodes + 4, 0);                       /* uid 索引 */
        __put_le16(inodes + 6, 0);                       /* gid 索引 */
        __put_le32(inodes + 8, 0);                       /* mtime */
        __put_le32(inodes + 12, 1);                      /* inode_number = 1 */
        __put_le32(inodes + 16, 0);                      /* start_block: dir 表内相对字节偏移(单块=0) */
        __put_le32(inodes + 20, 2);                      /* nlink = 子目录数 + 2 */
        __put_le16(inodes + 24, 0);                      /* file_size = 目录大小+3(由下面计算) */
        __put_le16(inodes + 26, 0);                      /* offset: 解压块内字节偏移 */
        __put_le32(inodes + 28, 1);                      /* parent_inode */
        inode_len = 32;

        /* 每个文件的 inode: block_list 直接来自 add 时记录的 on-disk 大小 */
        for (i = 0; i < sq->num_files; i++) {
            uint64_t start = sq->pending_sizes[i];
            uint32_t blocks = sq->files[i].num_blocks;
            uint8_t *p = inodes + inode_len;

            /* 文件 inode(32 字节头 + block_list): basic(16) + start_block(4)
             * + fragment(4) + offset(4, 恒存在) + file_size(4) */
            __put_le16(p, SQFS_INODE_FILE);
            __put_le16(p + 2, 0x81A4);                       /* S_IFREG | 0644 */
            __put_le16(p + 4, 0);
            __put_le16(p + 6, 0);
            __put_le32(p + 8, 0);
            __put_le32(p + 12, 2 + i);                       /* inode_number */
            __put_le32(p + 16, (uint32_t)start);             /* start_block */
            __put_le32(p + 20, SQFS_NO_FRAGMENT);            /* fragment */
            __put_le32(p + 24, 0);                           /* offset(碎片内偏移, 无 fragment=0) */
            __put_le32(p + 28, (uint32_t)sq->files[i].size); /* file_size */
            p += 32;
            for (j = 0; j < blocks; j++) {
                __put_le32(p, sq->files[i].block_list[j]);
                p += 4;
            }
            inode_len = (uint64_t)(p - inodes);
        }

        /* 写入 inode 表(根目录 inode 位于 block0 offset0, 所以 root_inode_loc=0) */
        sq->super.inode_table_start = sq->write_pos;
        EXEC(__write_metadata_block(sq, &sq->write_pos, inodes, (uint32_t)inode_len));

        /* ---- 构建 directory 表: 根目录项 ---- */
        dirs = allocator_mem_zalloc(allocator, 8192);
        THROW_IF(dirs == NULL, -1);
        base_inode = 2;                                       /* 第一个文件的 inode 号 */
        __put_le32(dirs, sq->num_files > 0 ? sq->num_files - 1 : 0);  /* 磁盘存 count-1 */
        __put_le32(dirs + 4, 0);                              /* 共享 start_block = inode 表 block0 */
        __put_le32(dirs + 8, base_inode);                     /* base inode_number */
        dir_len = 12;
        dir_off = 32;                                         /* 第一个文件 inode 在 inode 表内的偏移 */
        for (i = 0; i < sq->num_files; i++) {
            const char *nm = __basename((char *)sq->files[i].name);
            uint32_t name_len = (uint32_t)strlen(nm);
            uint8_t *p = dirs + dir_len;
            __put_le16(p, (uint16_t)dir_off);                 /* inode 偏移 */
            __put_le16(p + 2, (uint16_t)((int)(2 + i) - (int)base_inode));  /* inode 号差值 */
            __put_le16(p + 4, SQFS_INODE_FILE);
            __put_le16(p + 6, (uint16_t)(name_len - 1));      /* size = 名字长 - 1 */
            memcpy(p + 8, nm, name_len);
            dir_len += 8 + name_len;
            dir_off += 32 + 4 * sq->files[i].num_blocks;
        }
        /* 回填根目录 inode 的 dir file_size(实际大小 + 3) */
        __put_le16(inodes + 24, (uint16_t)(dir_len + 3));
        /* 覆盖更新 inode 表(根目录 inode 在 block0 内, 需要重新写块) */
        sq->write_pos = sq->super.inode_table_start;
        EXEC(__write_metadata_block(sq, &sq->write_pos, inodes, (uint32_t)inode_len));

        sq->super.directory_table_start = sq->write_pos;
        EXEC(__write_metadata_block(sq, &sq->write_pos, dirs, (uint32_t)dir_len));

        /* ---- id 表: 一个 id(0) ----
         * 格式: id_data 元数据块(存 id 值) + 末尾原始 8 字节索引(指向 id_data).
         * 索引不含元数据块头, 且须紧贴镜像末尾, 以通过 unsquashfs 的
         * length == table_start - id_table_start 校验 */
        id_data_pos = sq->write_pos;
        EXEC(__write_metadata_block(sq, &sq->write_pos, id_data, 4));
        sq->super.id_table_start = sq->write_pos;
        __put_le64(id_index, id_data_pos);
        EXEC(a->seek(a, sq->super.id_table_start, SEEK_SET));
        EXEC(a->write(a, id_index, 8));
        sq->write_pos += 8;

        /* ---- 写 superblock ---- */
        EXEC(__write_superblock(sq, 0));                     /* root_inode = (block0<<16)|0 */
        sq->add_flag = 0;
    } CATCH (ret) {} FINALLY {
        if (inodes) allocator_mem_free(allocator, inodes);
        if (dirs) allocator_mem_free(allocator, dirs);
    }

    return ret;
}

static class_info_entry_t fs_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Squashfs, construct, __construct),
    Init_Nfunc_Entry(2, Squashfs, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Squashfs, open, __open),
    Init_Vfunc_Entry(4, Squashfs, extract_file, __extract_file),
    Init_Vfunc_Entry(5, Squashfs, add_file, __add_file),
    Init_Vfunc_Entry(6, Squashfs, list, __list),
    Init_Vfunc_Entry(7, Squashfs, save, __save),
    Init_End___Entry(8, Squashfs),
};
REGISTER_CLASS(Squashfs, fs_class_info);
