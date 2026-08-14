#ifndef __Squashfs_H__
#define __Squashfs_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct Squashfs_s Squashfs;

/* ---- SquashFS 4.0 常量 ---- */
#define SQFS_MAGIC              0x73717368   /* "hsqs" */
#define SQFS_METADATA_SIZE      8192
#define SQFS_BLOCK_HEADER_SIZE  2
#define SQFS_COMPRESSED_BIT     0x8000       /* 数据块/元数据块头 bit15 */
#define SQFS_SIZE_MASK          0x7FFF
#define SQFS_COMPRESSION_ZLIB   1

/* superblock flags */
#define SQFS_FLAG_NOXATTR       0x100    /* 无 xattr 表, 供 unsquashfs 跳过 xattr 解析 */

/* 文件 inode 的 block_list 条目: bit24 表示该数据块未压缩(原样存储),
 * 低 24 位是数据块在磁盘上的大小(不含长度头) */
#define SQFS_BLOCK_UNCOMPRESSED_BIT  (1 << 24)
#define SQFS_BLOCK_SIZE_MASK         (SQFS_BLOCK_UNCOMPRESSED_BIT - 1)

/* inode 类型 */
#define SQFS_INODE_DIR          1
#define SQFS_INODE_FILE         2
#define SQFS_INODE_SYMLINK      3
#define SQFS_INODE_EXT_DIR      8
#define SQFS_INODE_EXT_FILE     9
#define SQFS_INODE_EXT_SYMLINK  10

#define SQFS_NO_FRAGMENT        0xFFFFFFFF   /* 文件 inode 的 fragment 索引 */
#define SQFS_DIR_ENTRY_HEADER   (1 << 15)    /* 目录项头部计数标志 */
#define SQFS_INVALID_BLK        0xFFFFFFFFFFFFFFFFULL  /* 不存在的表(如 xattr)用该值 */

/* 96 字节 superblock */
typedef struct sqfs_superblock_s {
    uint32_t magic;
    uint32_t inode_count;
    uint32_t mtime;
    uint32_t block_size;
    uint32_t fragment_count;
    uint16_t compression;
    uint16_t block_log;
    uint16_t flags;
    uint16_t id_count;
    uint16_t major;
    uint16_t minor;
    uint64_t root_inode;            /* (block << 16) | offset */
    uint64_t bytes_used;
    uint64_t id_table_start;
    uint64_t xattr_table_start;
    uint64_t inode_table_start;
    uint64_t directory_table_start;
    uint64_t fragment_table_start;
    uint64_t lookup_table_start;
} sqfs_superblock_t;

/* 解析出的一个文件条目（list/extract 用） */
typedef struct sqfs_file_entry_s {
    uint8_t *name;                  /* 完整路径 */
    uint64_t size;
    uint32_t inode_block;           /* inode 所在元数据块索引(相对 inode 表) */
    uint16_t inode_offset;          /* inode 在解压后块内的字节偏移 */
    /* 写入端: 该文件各数据块的 on-disk 大小(bit24=未压缩) */
    uint32_t *block_list;
    uint32_t num_blocks;
} sqfs_file_entry_t;

struct Squashfs_s {
    Archive parent;

    int (*construct)(Squashfs *, char *);
    int (*deconstruct)(Squashfs *);
    int (*open)(Squashfs *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(Squashfs *, char *attrib, void *value);
    void *(*get)(Squashfs *, char *attrib);
    char *(*to_json)(Squashfs *);
    int (*extract_file)(Squashfs *, archive_file_info_t *info);
    int (*add_file)(Squashfs *, archive_file_info_t *info);
    int (*list)(Squashfs *zip, Vector **infos);
    int (*save)(Squashfs *a);

    File *file;
	String *file_name;
    Buffer *buffer;

    /* ---- 读取阶段解析出的状态 ---- */
    sqfs_superblock_t super;
    uint8_t *inode_table;           /* 整个 inode 表解压后的连续缓冲 */
    uint64_t inode_table_size;
    uint8_t *dir_table;             /* 整个目录表解压后的连续缓冲 */
    uint64_t dir_table_size;
    uint8_t *id_table;              /* uid/gid 表解压缓冲 */
    uint64_t id_table_size;
    uint8_t *fragment_index;        /* fragment 表索引(指向各 fragment 表块的偏移) */
    uint64_t fragment_index_size;

    uint32_t num_files;
    sqfs_file_entry_t *files;

    /* ---- 写入阶段状态 ---- */
    uint64_t write_pos;             /* 下一个数据块/元数据写入的位置 */
    uint8_t add_flag;
    uint32_t *pending_sizes;        /* 待写入文件的块大小 */
    uint32_t pending_count;
};

#endif
