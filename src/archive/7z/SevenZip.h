#ifndef __X_7Z_H__
#define __X_7Z_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/archive/Archive.h>
#include <libobject/compress/Compress.h>

typedef struct X_7Z_s SevenZip;

#define SZ_SIGNATURE_SIZE   6
#define SZ_HEADER_SIZE      32   /* signature(6) + version(2) + start header(20) */

/* 7z record types */
#define SZ_K_END                 0x00
#define SZ_K_HEADER              0x01
#define SZ_K_ARCHIVE_PROPERTIES  0x02
#define SZ_K_ADDITIONAL_STREAMS  0x03
#define SZ_K_MAIN_STREAMS        0x04
#define SZ_K_FILES_INFO          0x05
#define SZ_K_PACK_INFO           0x06
#define SZ_K_UNPACK_INFO         0x07
#define SZ_K_SUBSTREAMS_INFO     0x08
#define SZ_K_SIZE                0x09
#define SZ_K_CRC                 0x0a
#define SZ_K_FOLDER              0x0b
#define SZ_K_CODERS_UNPACK_SIZE  0x0c
#define SZ_K_NUM_UNPACK_STREAM   0x0d
#define SZ_K_EMPTY_STREAM        0x0e
#define SZ_K_EMPTY_FILE          0x0f
#define SZ_K_ANTI                0x10
#define SZ_K_NAME                0x11
#define SZ_K_CTIME               0x12
#define SZ_K_ATIME               0x13
#define SZ_K_MTIME               0x14
#define SZ_K_WIN_ATTRIBUTES      0x15
#define SZ_K_ENCODED_HEADER      0x17
#define SZ_K_START_POS           0x18
#define SZ_K_DUMMY               0x19

/* method ids */
#define SZ_METHOD_LZMA2 0x21
#define SZ_METHOD_COPY  0x00
#define SZ_LZMA_ID_0    0x03
#define SZ_LZMA_ID_1    0x01
#define SZ_LZMA_ID_2    0x01

/* a coder inside a folder */
typedef struct sz_coder_s {
    uint8_t method_id[16];
    uint8_t method_id_size;
    uint32_t num_in_streams;
    uint32_t num_out_streams;
    uint8_t *properties;
    uint32_t properties_size;
} sz_coder_t;

/* a folder: a set of coders wired by bind pairs.
 * 7z 里一个 folder 是"一条编码链": 若干 coder 通过 bind pair 连接, 链头从
 * 打包流(packed stream)取输入, 链尾输出解压后的数据. 一个 folder 的解压输出
 * 可能被切分成多个 substream(固实压缩时一个 folder 装多个文件). */
typedef struct sz_folder_s {
    uint32_t num_coders;            /* coder 数量 */
    sz_coder_t *coders;             /* 每个 coder 的描述(算法 id + 属性) */
    uint32_t num_bind_pairs;        /* coder 之间连接关系数量 */
    uint32_t *bind_in;              /* 输入流 id */
    uint32_t *bind_out;             /* 输出流 id */
    uint32_t num_packed_streams;    /* 该 folder 拥有的打包流数量 */
    uint32_t *packed_indices;       /* 本 folder 内各打包流的局部索引 */
    uint32_t first_packed_stream;   /* 第一个打包流在全局(整个归档)中的索引 */
    uint64_t *unpack_sizes;         /* 每个 coder 输出流的大小(按 out stream id) */
    uint32_t total_out_streams;     /* 所有 coder 输出流数量之和 */
    /* substreams (from SubStreamsInfo) */
    uint32_t num_substreams;        /* 该 folder 解压输出被切成多少段(文件) */
    uint64_t *substream_sizes;      /* 除最后一个外各 substream 的大小 */
    uint32_t *substream_crcs;       /* 各 substream 的 CRC, 0xFFFFFFFF 表示无 */
} sz_folder_t;

/* a file parsed from FilesInfo */
typedef struct sz_file_info_s {
    uint8_t *name;                /* utf-8 name */
    uint64_t size;
    uint32_t crc;                 /* 0xFFFFFFFF if none */
    int empty_stream;
    int empty_file;
    /* mapping to the data */
    uint32_t folder_index;        /* 0xFFFFFFFF if empty */
    uint32_t substream_index;
} sz_file_info_t;

struct X_7Z_s {
    Archive parent;

    int (*construct)(SevenZip *, char *);
    int (*deconstruct)(SevenZip *);
    int (*open)(SevenZip *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(SevenZip *, char *attrib, void *value);
    void *(*get)(SevenZip *, char *attrib);
    char *(*to_json)(SevenZip *);
    int (*extract_file)(SevenZip *, archive_file_info_t *info);
    int (*add_file)(SevenZip *, archive_file_info_t *info);
    int (*list)(SevenZip *zip, Vector **infos);
    int (*save)(SevenZip *a);

    File *file;
	String *file_name;
    Buffer *buffer;

    /* ---- 读取阶段解析出的归档状态 ---- */
    uint64_t pack_pos;              /* 打包数据区起始偏移(相对归档起点, 通常=32) */
    uint32_t num_pack_streams;      /* 打包流总数 */
    uint64_t *packed_sizes;         /* 每个打包流的大小 */
    uint32_t *packed_crcs;          /* 每个打包流的 CRC, 0xFFFFFFFF 表示无 */

    uint32_t num_folders;           /* 文件夹数量 */
    sz_folder_t *folders;           /* 每个文件夹(编码链 + substream 划分) */

    uint32_t num_files;             /* 文件数量 */
    sz_file_info_t *files;          /* 每个文件的名字/大小/CRC/所属 folder+substream */

    uint8_t *raw_header;            /* NextHeader 原始字节(含被解码后的头) */
    uint32_t raw_header_size;

    /* ---- 写入阶段状态 ---- */
    uint64_t central_dir_position;  /* 写入游标(下一个打包流写入的位置) */
    uint8_t add_flag;               /* 是否已有 add_file 调用(是否需要写头) */
};

#endif
