/**
 * @file SevenZip.c
 * @Synopsis
 *     7z (7-Zip) archive container.  Coded streams use raw LZMA2/LZMA
 *     provided by liblzma.
 *
 * ---- 7z 文件格式概览 ----
 *
 * 一个 7z 文件（无 zip64/加密/分卷等扩展）的整体布局:
 *
 *   [文件头 32 字节]
 *     签名  6 字节 : 0x37 0x7A 0xBC 0xAF 0x27 0x1C ("7z" + BC AF 27 1C)
 *    版本  2 字节 : 主版本(0) 次版本(4)
 *    NextHeaderOffset  8 字节 : 下一个头的"绝对"偏移(相对归档起点)
 *    NextHeaderSize    8 字节 : 下一个头的字节数
 *    NextHeaderCRC     4 字节 : 下一个头的 CRC32
 *   [数据区]
 *     所有被压缩(packed)的编码流, 从偏移 32 开始连续存放.
 *   [NextHeader]
 *     类型字节: 0x01=kHeader(明文头) 或 0x17=kEncodedHeader(头本身被压缩).
 *     kEncodedHeader 内部又是一个 MainStreamsInfo, 解码后得到的才是真正的 kHeader.
 *
 * NextHeader(kHeader) 由若干"记录"组成, 每条记录以类型字节开头:
 *   kMainStreamsInfo(0x04): 描述数据区如何被编码
 *     kPackInfo(0x06)      : PackPos + 打包流数量 + 每个打包流大小 [+CRC]
 *     kUnpackInfo(0x07)    : kFolder(文件夹/编码器链) + 各 coder 输出大小 + CRC
 *     kSubStreamsInfo(0x08): 每个 folder 里的 substream 数量/大小/CRC
 *   kFilesInfo(0x05)       : 文件数量 + 文件名(UTF-16LE)/空文件标记/大小/CRC
 *   kEnd(0x00)
 *
 * 数字采用 VLV(variable length value)编码: 首字节的最高置位位决定后续跟几个字节,
 * 首字节低若干位是高位. 位向量(bit vector)按"每字节 8 位, 低位在前"存放.
 *
 * Folder(文件夹)结构:
 *   NumCoders -> 每个 coder: flags(id大小/属性标志) + method id + [in/out流数]
 *                + [属性] -> NumBindPairs -> NumPackedStreams(全局打包流索引)
 * 默认 7-Zip 产出通常是一个 folder 里只有一个 LZMA2 coder(1 in/1 out), 本实现
 * 仅支持这种单 coder、无 bind pair、单个 packed stream 的 folder.
 *
 * @author alan lin
 * @version
 * @date 2023-12-19
 */
#include <string.h>
#include <stdlib.h>
#include <lzma.h>
#include <zlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/core/io/File.h>
#include "SevenZip.h"

/* ------------------------------------------------------------------ */
/* pure helpers                                                        */
/* ------------------------------------------------------------------ */

static uint64_t __read_le64(const uint8_t *p)
{
    uint64_t v;
    memcpy(&v, p, 8);
    return byteorder_le64_to_cpu(&v);
}

static uint32_t __read_le32(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return byteorder_le32_to_cpu(&v);
}

static void __write_le32(uint8_t *p, uint32_t v)
{
    byteorder_cpu_to_le32(&v);
    memcpy(p, &v, 4);
}

static void __write_le64(uint8_t *p, uint64_t v)
{
    byteorder_cpu_to_le64(&v);
    memcpy(p, &v, 8);
}

/* 7z variable-length number encoding */
static uint64_t __read_number(const uint8_t **pp, const uint8_t *end)
{
    const uint8_t *p = *pp;
    uint8_t first, mask = 0x80;
    uint64_t value = 0;
    int i;

    if (p >= end) return 0;
    first = *p;
    for (i = 0; i < 8; i++) {
        if ((first & mask) == 0) {
            value |= ((uint64_t)(first & (mask - 1))) << (8 * i);
            *pp = p + 1;
            return value;
        }
        mask >>= 1;
        if (p + 1 >= end) { *pp = p + 1; return value; }
        value |= ((uint64_t)p[1]) << (8 * i);
        p++;
    }
    *pp = p + 1;
    return value;
}

static int __write_number(uint8_t *dest, uint64_t value)
{
    uint8_t first = 0;
    uint8_t mask = 0x80;
    unsigned i = 0;

    for (; i < 8; i++) {
        if (value < ((uint64_t)1 << (7 * (i + 1)))) {
            first |= (uint8_t)(value >> (8 * i));
            break;
        }
        first |= mask;
        mask >>= 1;
        dest[1 + i] = (uint8_t)value;
        value >>= 8;
    }
    dest[0] = first;
    return (int)(i + 1);
}

static int __read_bit_vector(const uint8_t **pp, const uint8_t *end, uint8_t *vec, uint32_t n)
{
    uint32_t bytes = (n + 7) / 8, i;
    memset(vec, 0, bytes);
    for (i = 0; i < bytes; i++) {
        if (*pp >= end) return -1;
        vec[i] = *(*pp)++;
    }
    return 0;
}

static int __get_bit(const uint8_t *vec, uint32_t i)
{
    return (vec[i >> 3] >> (i & 7)) & 1;
}

/* LZMA2 properties byte -> dictionary size */
static uint32_t __lzma2_prop_to_dict(uint8_t prop)
{
    if (prop >= 40) return 0;
    return ((uint32_t)2 | (prop & 1)) << (prop / 2 + 11);
}

/* LZMA (alone) 5-byte properties: [lc/lp/pb byte][dict size LE32] */
static void __lzma_prop_parse(const uint8_t *prop, uint32_t size,
                              uint32_t *dict, uint32_t *lc, uint32_t *lp, uint32_t *pb)
{
    uint32_t d = 0;
    if (size >= 5) d = __read_le32(prop + 1);
    *dict = d;
    *lc = prop[0] % 9;
    *lp = (prop[0] / 9) % 5;
    *pb = prop[0] / 45;
}

/* utf-16le (with NUL terminator) -> utf-8 */
static int __utf16le_to_utf8(const uint8_t *u16, size_t len, char *out, size_t out_max)
{
    size_t i = 0, o = 0;
    while (i + 1 < len && o + 1 < out_max) {
        uint32_t cp = (uint32_t)u16[i] | ((uint32_t)u16[i + 1] << 8);
        i += 2;
        if (cp == 0) break;
        if (cp < 0x80) {
            out[o++] = (char)cp;
        } else if (cp < 0x800) {
            out[o++] = (char)(0xC0 | (cp >> 6));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        } else if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < len) {
            uint32_t lo = (uint32_t)u16[i] | ((uint32_t)u16[i + 1] << 8);
            i += 2;
            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
            out[o++] = (char)(0xF0 | (cp >> 18));
            out[o++] = (char)(0x80 | ((cp >> 12) & 0x3F));
            out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        } else {
            out[o++] = (char)(0xE0 | (cp >> 12));
            out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        }
    }
    out[o] = '\0';
    return (int)o;
}

/* utf-8 (BMP) -> utf-16le with NUL terminator */
static int __utf8_to_utf16le(const char *s, uint8_t *out, size_t out_max)
{
    const uint8_t *p = (const uint8_t *)s;
    size_t o = 0;
    uint32_t cp;

    while (*p && o + 2 <= out_max) {
        uint8_t c = *p;
        if (c < 0x80) { cp = c; p += 1; }
        else if ((c & 0xE0) == 0xC0) { cp = ((uint32_t)(c & 0x1F) << 6) | (p[1] & 0x3F); p += 2; }
        else if ((c & 0xF0) == 0xE0) { cp = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(p[1] & 0x3F) << 6) | (p[2] & 0x3F); p += 3; }
        else { cp = 0x3F; p += 1; }
        out[o++] = (uint8_t)(cp & 0xFF);
        out[o++] = (uint8_t)((cp >> 8) & 0xFF);
    }
    if (o + 2 <= out_max) { out[o++] = 0; out[o++] = 0; }
    return (int)o;
}

/* raw LZMA2 / LZMA decode via liblzma */
static int __lzma_raw_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len,
                             int is_lzma2, uint32_t dict, uint32_t lc, uint32_t lp, uint32_t pb)
{
    lzma_options_lzma opt;
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;
    int r = -1;

    memset(&opt, 0, sizeof(opt));
    opt.dict_size = dict;
    opt.lc = lc; opt.lp = lp; opt.pb = pb;
    filters[0].id = is_lzma2 ? LZMA_FILTER_LZMA2 : LZMA_FILTER_LZMA1;
    filters[0].options = &opt;
    filters[1].id = LZMA_VLI_UNKNOWN;
    filters[1].options = NULL;

    if (lzma_raw_decoder(&strm, filters) == LZMA_OK) {
        strm.next_in = in;
        strm.avail_in = in_len;
        strm.next_out = out;
        strm.avail_out = *out_len;
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret == LZMA_STREAM_END) {
            *out_len = (size_t)(strm.next_out - out);
            r = 0;
        }
        lzma_end(&strm);
    }
    return r;
}

/* raw LZMA2 encode via liblzma */
static int __lzma_raw_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len,
                             uint32_t dict_size)
{
    lzma_options_lzma opt;
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;
    int r = -1;

    memset(&opt, 0, sizeof(opt));
    lzma_lzma_preset(&opt, 6);
    opt.dict_size = dict_size;
    filters[0].id = LZMA_FILTER_LZMA2;
    filters[0].options = &opt;
    filters[1].id = LZMA_VLI_UNKNOWN;
    filters[1].options = NULL;

    if (lzma_raw_encoder(&strm, filters) == LZMA_OK) {
        strm.next_in = in;
        strm.avail_in = in_len;
        strm.next_out = out;
        strm.avail_out = *out_len;
        ret = lzma_code(&strm, LZMA_FINISH);
        if (ret == LZMA_STREAM_END) {
            *out_len = (size_t)(strm.next_out - out);
            r = 0;
        }
        lzma_end(&strm);
    }
    return r;
}

/* ------------------------------------------------------------------ */
/* free helpers                                                       */
/* ------------------------------------------------------------------ */

static void __free_coder(allocator_t *allocator, sz_coder_t *coder)
{
    if (coder->properties) allocator_mem_free(allocator, coder->properties);
}

static void __free_folder(allocator_t *allocator, sz_folder_t *f)
{
    uint32_t i;
    for (i = 0; i < f->num_coders; i++) __free_coder(allocator, &f->coders[i]);
    if (f->coders) allocator_mem_free(allocator, f->coders);
    if (f->bind_in) allocator_mem_free(allocator, f->bind_in);
    if (f->bind_out) allocator_mem_free(allocator, f->bind_out);
    if (f->packed_indices) allocator_mem_free(allocator, f->packed_indices);
    if (f->unpack_sizes) allocator_mem_free(allocator, f->unpack_sizes);
    if (f->substream_sizes) allocator_mem_free(allocator, f->substream_sizes);
    if (f->substream_crcs) allocator_mem_free(allocator, f->substream_crcs);
}

static void __free_streams(SevenZip *sz)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t i;

    if (sz->packed_sizes) allocator_mem_free(allocator, sz->packed_sizes);
    if (sz->packed_crcs) allocator_mem_free(allocator, sz->packed_crcs);
    for (i = 0; i < sz->num_folders; i++) __free_folder(allocator, &sz->folders[i]);
    if (sz->folders) allocator_mem_free(allocator, sz->folders);
    for (i = 0; i < sz->num_files; i++) {
        if (sz->files[i].name) allocator_mem_free(allocator, sz->files[i].name);
    }
    if (sz->files) allocator_mem_free(allocator, sz->files);

    sz->packed_sizes = NULL;
    sz->packed_crcs = NULL;
    sz->folders = NULL;
    sz->files = NULL;
    sz->num_pack_streams = sz->num_folders = sz->num_files = 0;
}

static void __free_all(SevenZip *sz)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    __free_streams(sz);
    if (sz->raw_header) allocator_mem_free(allocator, sz->raw_header);
    sz->raw_header = NULL;
}

/* ------------------------------------------------------------------ */
/* parsers (all use the TRY/CATCH mechanism)                          */
/* ------------------------------------------------------------------ */

static int __read_crc_section(SevenZip *sz, const uint8_t **pp, const uint8_t *end,
                              uint32_t count, uint32_t *crcs)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint8_t *vec = NULL;
    uint32_t i;
    int all_defined, ret = 0;

    TRY {
        for (i = 0; i < count; i++) crcs[i] = 0xFFFFFFFF;
        all_defined = (int)__read_number(pp, end);

        if (all_defined != 0) {
            for (i = 0; i < count; i++) crcs[i] = 0;
        } else {
            vec = allocator_mem_zalloc(allocator, (count + 7) / 8);
            THROW_IF(vec == NULL, -1);
            THROW_IF(__read_bit_vector(pp, end, vec, count) != 0, -1);
            for (i = 0; i < count; i++) {
                if (__get_bit(vec, i)) crcs[i] = 0;
            }
        }

        for (i = 0; i < count; i++) {
            if (crcs[i] == 0xFFFFFFFF) continue;
            THROW_IF(*pp + 4 > end, -1);
            crcs[i] = __read_le32(*pp);
            *pp += 4;
        }
    } CATCH (ret) {} FINALLY {
        if (vec) allocator_mem_free(allocator, vec);
    }

    return ret;
}

/* kPackInfo(0x06) 记录(标准格式):
 *   PackPos            : 打包数据相对"数据起始(32 字节文件头之后)"的偏移
 *   NumPackStreams     : 打包流数量
 *   kSize(0x09)        : 之后是每个打包流的大小
 *   可选 kCRC(0x0a) 记录, 最后 kEnd(0x00)
 */
static int __parse_pack_info(SevenZip *sz, const uint8_t **pp, const uint8_t *end)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t i;
    int ret = 0;

    TRY {
        sz->pack_pos = 32 + (uint32_t)__read_number(pp, end);
        sz->num_pack_streams = (uint32_t)__read_number(pp, end);
        THROW_IF(sz->num_pack_streams == 0, -1);

        sz->packed_sizes = allocator_mem_zalloc(allocator, sizeof(uint64_t) * sz->num_pack_streams);
        sz->packed_crcs = allocator_mem_zalloc(allocator, sizeof(uint32_t) * sz->num_pack_streams);
        THROW_IF(sz->packed_sizes == NULL || sz->packed_crcs == NULL, -1);
        for (i = 0; i < sz->num_pack_streams; i++) {
            sz->packed_crcs[i] = 0xFFFFFFFF;
        }

        while (*pp < end) {
            uint8_t type = *(*pp)++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_SIZE) {
                for (i = 0; i < sz->num_pack_streams; i++) {
                    sz->packed_sizes[i] = __read_number(pp, end);
                }
            } else if (type == SZ_K_CRC) {
                EXEC(__read_crc_section(sz, pp, end, sz->num_pack_streams, sz->packed_crcs));
            } else {
                dbg_str(DBG_ERROR, "pack_info: unknown record type 0x%x", type);
                THROW(-1);
            }
        }
    } CATCH (ret) {}

    return ret;
}

/* Folder(文件夹)结构:
 *   NumCoders
 *   每个 coder: flags | id_size | method_id | [num_in, num_out] | [props_size, props]
 *     flags bit0x10 = 流数量显式给出; bit0x20 = 带属性(properties)
 *   NumBindPairs + (in_index, out_index)*
 *   NumPackedStreams + 每个 packed stream 在本 folder 内的索引
 *   (external 标志在 UnpackInfo 的 kFolder 记录处已读取, 这里不再有)
 */
static int __parse_folder(SevenZip *sz, const uint8_t **pp, const uint8_t *end, sz_folder_t *f)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t i, num;
    int ret = 0;

    TRY {
        memset(f, 0, sizeof(*f));
        num = (uint32_t)__read_number(pp, end);

        f->num_coders = num;
        f->coders = allocator_mem_zalloc(allocator, sizeof(sz_coder_t) * num);
        THROW_IF(f->coders == NULL, -1);

        for (i = 0; i < num; i++) {
            sz_coder_t *coder = &f->coders[i];
            uint8_t flags, id_size;

            /* 标准 coder: flags 低 4 位 = 方法 ID 长度, bit0x10=带流数量, bit0x20=带属性 */
            flags = *(*pp)++;
            id_size = flags & 0x0F;
            THROW_IF(id_size == 0 || id_size > 8 || *pp + id_size > end, -1);
            memcpy(coder->method_id, *pp, id_size);
            *pp += id_size;
            coder->method_id_size = id_size;

            if (flags & 0x10) {
                coder->num_in_streams = (uint32_t)__read_number(pp, end);
                coder->num_out_streams = (uint32_t)__read_number(pp, end);
            } else {
                coder->num_in_streams = 1;
                coder->num_out_streams = 1;
            }
            if (flags & 0x20) {
                coder->properties_size = (uint32_t)__read_number(pp, end);
                THROW_IF(*pp + coder->properties_size > end, -1);
                coder->properties = allocator_mem_alloc(allocator, coder->properties_size);
                THROW_IF(coder->properties == NULL, -1);
                memcpy(coder->properties, *pp, coder->properties_size);
                *pp += coder->properties_size;
            }
            f->total_out_streams += coder->num_out_streams;
        }

        /* 单 coder 且 1 进 1 出时, 标准格式省略 bind pairs 与 packed stream 索引
         * (直接到 kCodersUnpackSize), 这里默认补齐 */
        if (f->num_coders == 1 && f->coders[0].num_in_streams == 1 && f->coders[0].num_out_streams == 1) {
            f->num_bind_pairs = 0;
            f->num_packed_streams = 1;
            f->packed_indices = allocator_mem_zalloc(allocator, sizeof(uint32_t));
            THROW_IF(f->packed_indices == NULL, -1);
            f->packed_indices[0] = 0;
        } else {
            num = (uint32_t)__read_number(pp, end);
            f->num_bind_pairs = num;
            if (num) {
                f->bind_in = allocator_mem_zalloc(allocator, sizeof(uint32_t) * num);
                f->bind_out = allocator_mem_zalloc(allocator, sizeof(uint32_t) * num);
                THROW_IF(f->bind_in == NULL || f->bind_out == NULL, -1);
                for (i = 0; i < num; i++) {
                    f->bind_in[i] = (uint32_t)__read_number(pp, end);
                    f->bind_out[i] = (uint32_t)__read_number(pp, end);
                }
            }

            num = (uint32_t)__read_number(pp, end);
            f->num_packed_streams = num;
            if (num) {
                f->packed_indices = allocator_mem_zalloc(allocator, sizeof(uint32_t) * num);
                THROW_IF(f->packed_indices == NULL, -1);
                for (i = 0; i < num; i++) {
                    f->packed_indices[i] = (uint32_t)__read_number(pp, end);
                }
            }
        }
    } CATCH (ret) {}

    return ret;
}

/* kUnpackInfo(0x07) 记录:
 *   kFolder(0x0b)      : NumFolders + External + [DataOffset] + 每个 folder
 *   随后循环记录直到 kEnd(0x00):
 *     kCodersUnpackSize(0x0c): 每个 coder 的输出流大小
 *     kCRC(0x0a)             : folder 级 CRC
 */
static int __parse_unpack_info(SevenZip *sz, const uint8_t **pp, const uint8_t *end)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t i, num_folders, ext, acc, f;
    int ret = 0;

    TRY {
        THROW_IF(*pp >= end || *(*pp)++ != SZ_K_FOLDER, -1);
        num_folders = (uint32_t)__read_number(pp, end);
        ext = (uint32_t)__read_number(pp, end);
        THROW_IF(ext != 0, -1);

        sz->num_folders = num_folders;
        sz->folders = allocator_mem_zalloc(allocator, sizeof(sz_folder_t) * num_folders);
        THROW_IF(sz->folders == NULL, -1);
        for (i = 0; i < num_folders; i++) {
            EXEC(__parse_folder(sz, pp, end, &sz->folders[i]));
        }

        /* assign each folder its first (global) packed stream index */
        acc = 0;
        for (f = 0; f < num_folders; f++) {
            sz->folders[f].first_packed_stream = acc;
            acc += sz->folders[f].num_packed_streams;
        }

        while (*pp < end) {
            uint8_t type = *(*pp)++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_CODERS_UNPACK_SIZE) {
                for (f = 0; f < num_folders; f++) {
                    uint32_t n = sz->folders[f].total_out_streams;
                    sz->folders[f].unpack_sizes = allocator_mem_zalloc(allocator, sizeof(uint64_t) * (n ? n : 1));
                    THROW_IF(sz->folders[f].unpack_sizes == NULL, -1);
                    for (i = 0; i < n; i++) {
                        sz->folders[f].unpack_sizes[i] = __read_number(pp, end);
                    }
                }
            } else if (type == SZ_K_CRC) {
                uint32_t *crcs = allocator_mem_zalloc(allocator, sizeof(uint32_t) * (num_folders ? num_folders : 1));
                THROW_IF(crcs == NULL, -1);
                EXEC(__read_crc_section(sz, pp, end, num_folders, crcs));
                allocator_mem_free(allocator, crcs);
            } else {
                dbg_str(DBG_ERROR, "unpack_info: unknown record type 0x%x", type);
                THROW(-1);
            }
        }
    } CATCH (ret) {}

    return ret;
}

/* kSubStreamsInfo(0x08) 记录:
 *   kNumUnpackStream(0x0d): 每个 folder 的 substream 数量
 *   kSize(0x09)           : 每个 folder 除最后一个 substream 外的大小
 *                          (最后一个 = folder 总大小 - 前几个之和)
 *   kCRC(0x0a)            : substream 的 CRC
 */
static int __parse_substreams_info(SevenZip *sz, const uint8_t **pp, const uint8_t *end)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t f;
    int ret = 0;

    TRY {
        while (*pp < end) {
            uint8_t type = *(*pp)++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_NUM_UNPACK_STREAM) {
                for (f = 0; f < sz->num_folders; f++) {
                    uint32_t n = (uint32_t)__read_number(pp, end);
                    sz->folders[f].num_substreams = n;
                    if (n > 1) {
                        sz->folders[f].substream_sizes = allocator_mem_zalloc(allocator, sizeof(uint64_t) * (n - 1));
                        THROW_IF(sz->folders[f].substream_sizes == NULL, -1);
                    }
                }
            } else if (type == SZ_K_SIZE) {
                for (f = 0; f < sz->num_folders; f++) {
                    uint32_t n = sz->folders[f].num_substreams, i;
                    if (n <= 1) continue;
                    for (i = 0; i + 1 < n; i++) {
                        sz->folders[f].substream_sizes[i] = __read_number(pp, end);
                    }
                }
            } else if (type == SZ_K_CRC) {
                /* 未显式给出 kNumUnpackStream 时, 每个 folder 默认 1 个 substream */
                uint32_t total = 0, i, idx = 0;
                for (f = 0; f < sz->num_folders; f++) {
                    total += sz->folders[f].num_substreams ? sz->folders[f].num_substreams : 1;
                }
                if (total) {
                    uint32_t *crcs;
                    for (f = 0; f < sz->num_folders; f++) {
                        uint32_t n = sz->folders[f].num_substreams ? sz->folders[f].num_substreams : 1;
                        sz->folders[f].substream_crcs = allocator_mem_zalloc(allocator, sizeof(uint32_t) * n);
                        THROW_IF(sz->folders[f].substream_crcs == NULL, -1);
                    }
                    crcs = allocator_mem_zalloc(allocator, sizeof(uint32_t) * total);
                    THROW_IF(crcs == NULL, -1);
                    EXEC(__read_crc_section(sz, pp, end, total, crcs));
                    for (f = 0; f < sz->num_folders; f++) {
                        uint32_t n = sz->folders[f].num_substreams ? sz->folders[f].num_substreams : 1;
                        for (i = 0; i < n; i++) {
                            sz->folders[f].substream_crcs[i] = crcs[idx++];
                        }
                    }
                    allocator_mem_free(allocator, crcs);
                }
            } else {
                dbg_str(DBG_ERROR, "substreams_info: unknown record type 0x%x", type);
                THROW(-1);
            }
        }
    } CATCH (ret) {}

    return ret;
}

/* kFilesInfo(0x05):
 *   NumFiles
 *   kEmptyStream(0x0e): 位向量, 标记"空流"文件(目录/无数据)
 *   kEmptyFile(0x0f)  : 位向量, 空流中哪些是空文件(长度为 0)
 *   kName(0x11)       : NamesSize + 一串以 \0 结尾的 UTF-16LE 文件名
 *   kCRC(0x0a)        : 文件 CRC
 *   其它(kTime/attrs 等)直接跳过
 */
static int __parse_files_info(SevenZip *sz, const uint8_t **pp, const uint8_t *end)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    uint32_t i, num_files;
    uint8_t *empty_stream = NULL, *empty_file = NULL;
    int ret = 0;

    TRY {
        num_files = (uint32_t)__read_number(pp, end);
        THROW_IF(num_files == 0, -1);
        sz->num_files = num_files;
        sz->files = allocator_mem_zalloc(allocator, sizeof(sz_file_info_t) * num_files);
        THROW_IF(sz->files == NULL, -1);
        for (i = 0; i < num_files; i++) {
            sz->files[i].crc = 0xFFFFFFFF;
            sz->files[i].folder_index = 0xFFFFFFFF;
        }

        while (*pp < end) {
            uint8_t type = *(*pp)++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_EMPTY_STREAM) {
                empty_stream = allocator_mem_zalloc(allocator, (num_files + 7) / 8);
                THROW_IF(empty_stream == NULL, -1);
                THROW_IF(__read_bit_vector(pp, end, empty_stream, num_files) != 0, -1);
                for (i = 0; i < num_files; i++) sz->files[i].empty_stream = __get_bit(empty_stream, i);
            } else if (type == SZ_K_EMPTY_FILE) {
                uint32_t cnt = 0, k = 0;
                for (i = 0; i < num_files; i++) if (sz->files[i].empty_stream) cnt++;
                empty_file = allocator_mem_zalloc(allocator, (cnt + 7) / 8);
                THROW_IF(empty_file == NULL, -1);
                THROW_IF(__read_bit_vector(pp, end, empty_file, cnt) != 0, -1);
                for (i = 0; i < num_files; i++) {
                    if (sz->files[i].empty_stream) sz->files[i].empty_file = __get_bit(empty_file, k++);
                }
            } else if (type == SZ_K_NAME) {
                uint32_t names_size = (uint32_t)__read_number(pp, end);
                const uint8_t *np, *nend;
                THROW_IF(*pp + names_size > end, -1);
                np = *pp;
                nend = np + names_size;
                if (np < nend && *np == 0) np++;   /* 标准格式: 名字数据以 0x00 开头 */
                for (i = 0; i < num_files && np < nend; i++) {
                    char name[1024];
                    int len = __utf16le_to_utf8(np, (size_t)(nend - np), name, sizeof(name));
                    np += 2 * (strlen(name) + 1);
                    sz->files[i].name = allocator_mem_zalloc(allocator, len + 1);
                    THROW_IF(sz->files[i].name == NULL, -1);
                    memcpy(sz->files[i].name, name, len + 1);
                }
                *pp = nend;
            } else if (type == SZ_K_CRC) {
                uint32_t *crcs = allocator_mem_zalloc(allocator, sizeof(uint32_t) * num_files);
                THROW_IF(crcs == NULL, -1);
                EXEC(__read_crc_section(sz, pp, end, num_files, crcs));
                for (i = 0; i < num_files; i++) sz->files[i].crc = crcs[i];
                allocator_mem_free(allocator, crcs);
            } else {
                uint64_t s = __read_number(pp, end);
                THROW_IF(*pp + s > end, -1);
                *pp += s;
            }
        }
    } CATCH (ret) {} FINALLY {
        if (empty_stream) allocator_mem_free(allocator, empty_stream);
        if (empty_file) allocator_mem_free(allocator, empty_file);
    }

    return ret;
}

/* files that are not empty_stream consume the substreams of the folders in
 * order; fill in each file's folder/substream index and size */
static int __map_files_to_substreams(SevenZip *sz)
{
    uint32_t f, i, global = 0;

    for (f = 0; f < sz->num_folders; f++) {
        uint32_t n = sz->folders[f].num_substreams ? sz->folders[f].num_substreams : 1;
        for (i = 0; i < n; i++) {
            if (global >= sz->num_files) break;
            if (!sz->files[global].empty_stream) {
                sz->files[global].folder_index = f;
                sz->files[global].substream_index = i;
                sz->files[global].size = sz->folders[f].substream_sizes ?
                    (i + 1 < n ? sz->folders[f].substream_sizes[i]
                               : sz->folders[f].unpack_sizes[0]) :
                    sz->folders[f].unpack_sizes[0];
                if (sz->folders[f].substream_crcs)
                    sz->files[global].crc = sz->folders[f].substream_crcs[i];
            }
            global++;
        }
    }
    return 0;
}

/* 解码一个 folder: 把它拥有的 packed stream 从归档中读出, 按 coder 的算法
 * (LZMA2 / LZMA / copy) 用 liblzma 解码, 输出该 folder 的完整解码结果.
 * 仅支持单 coder、无 bind pair、单 packed stream 的 folder(7-Zip 默认产出),
 * 其他情况返回 -1. 返回的 *out_buf 由调用方负责释放. */
static int __decode_folder(SevenZip *sz, uint32_t fi, uint8_t **out_buf, uint64_t *out_size)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    Archive *archive = (Archive *)&sz->parent;
    File *a = archive->file;
    sz_folder_t *f = &sz->folders[fi];
    sz_coder_t *coder;
    uint8_t *packed = NULL, *out = NULL;
    uint64_t off = 0, ps;
    uint32_t i;
    int ret = 0;

    TRY {
        THROW_IF(f->num_coders != 1 || f->num_bind_pairs != 0 || f->num_packed_streams != 1, -1);
        coder = &f->coders[0];
        ps = f->first_packed_stream;

        for (i = 0; i < ps; i++) off += sz->packed_sizes[i];
        packed = allocator_mem_alloc(allocator, sz->packed_sizes[ps]);
        THROW_IF(packed == NULL, -1);
        EXEC(a->seek(a, sz->pack_pos + off, SEEK_SET));
        EXEC(a->read(a, packed, sz->packed_sizes[ps]));

        *out_size = f->unpack_sizes[0];
        out = allocator_mem_zalloc(allocator, *out_size ? *out_size : 1);
        THROW_IF(out == NULL, -1);

        if (coder->method_id_size == 0 || coder->method_id[0] == SZ_METHOD_COPY) {
            uint64_t n = *out_size < sz->packed_sizes[ps] ? *out_size : sz->packed_sizes[ps];
            memcpy(out, packed, n);
        } else if (coder->method_id_size == 1 && coder->method_id[0] == SZ_METHOD_LZMA2) {
            uint32_t dict = __lzma2_prop_to_dict(coder->properties ? coder->properties[0] : 0);
            size_t dlen = (size_t)*out_size;
            THROW_IF(__lzma_raw_decode(packed, sz->packed_sizes[ps], out, &dlen, 1, dict, 3, 0, 2) != 0, -1);
            *out_size = dlen;
        } else if (coder->method_id_size == 3 && coder->method_id[0] == SZ_LZMA_ID_0
                   && coder->method_id[1] == SZ_LZMA_ID_1 && coder->method_id[2] == SZ_LZMA_ID_2) {
            uint32_t dict, lc, lp, pb;
            size_t dlen = (size_t)*out_size;
            __lzma_prop_parse(coder->properties, coder->properties_size, &dict, &lc, &lp, &pb);
            THROW_IF(__lzma_raw_decode(packed, sz->packed_sizes[ps], out, &dlen, 0, dict, lc, lp, pb) != 0, -1);
            *out_size = dlen;
        } else {
            dbg_str(DBG_ERROR, "decode: unsupported method id");
            THROW(-1);
        }
        *out_buf = out;
        out = NULL;
    } CATCH (ret) {} FINALLY {
        if (packed) allocator_mem_free(allocator, packed);
        if (out) allocator_mem_free(allocator, out);
    }

    return ret;
}

static int __parse_main_streams(SevenZip *sz, const uint8_t **pp, const uint8_t *end)
{
    int ret = 0;

    TRY {
        while (*pp < end) {
            uint8_t type = *(*pp)++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_PACK_INFO) {
                EXEC(__parse_pack_info(sz, pp, end));
            } else if (type == SZ_K_UNPACK_INFO) {
                EXEC(__parse_unpack_info(sz, pp, end));
            } else if (type == SZ_K_SUBSTREAMS_INFO) {
                EXEC(__parse_substreams_info(sz, pp, end));
            } else {
                dbg_str(DBG_ERROR, "main_streams: unknown type 0x%x", type);
                THROW(-1);
            }
        }
    } CATCH (ret) {}

    return ret;
}

static int __parse_header(SevenZip *sz, const uint8_t *data, uint32_t size)
{
    const uint8_t *p = data, *end = data + size;
    int ret = 0;

    TRY {
        while (p < end) {
            uint8_t type = *p++;
            if (type == SZ_K_END) break;
            if (type == SZ_K_MAIN_STREAMS) {
                EXEC(__parse_main_streams(sz, &p, end));
            } else if (type == SZ_K_FILES_INFO) {
                EXEC(__parse_files_info(sz, &p, end));
            } else if (type == SZ_K_ARCHIVE_PROPERTIES) {
                uint64_t n = __read_number(&p, end);
                THROW_IF(p + n > end, -1);
                p += n;
            } else {
                dbg_str(DBG_ERROR, "header: unknown type 0x%x", type);
                THROW(-1);
            }
        }
        EXEC(__map_files_to_substreams(sz));
    } CATCH (ret) {}

    return ret;
}

/* handle the next-header region: kHeader or kEncodedHeader */
static int __parse_next_header(SevenZip *sz, const uint8_t *data, uint32_t size)
{
    allocator_t *allocator = sz->parent.parent.allocator;
    const uint8_t *p = data, *end = data + size;
    uint8_t *decoded = NULL;
    uint64_t decoded_size = 0;
    int ret = 0;

    TRY {
        THROW_IF(p >= end, -1);
        if (*p == SZ_K_HEADER) {
            p++;
            EXEC(__parse_header(sz, p, end));
        } else if (*p == SZ_K_ENCODED_HEADER) {
            p++;
            /* the encoded header's MainStreamsInfo describes how to decode the
             * real header; the folder/pack state we have parsed so far belongs
             * to the encoded header itself */
            EXEC(__parse_main_streams(sz, &p, end));
            THROW_IF(sz->num_folders != 1, -1);
            EXEC(__decode_folder(sz, 0, &decoded, &decoded_size));
            /* free the encoded-header streams, then parse the real header */
            __free_streams(sz);
            EXEC(__parse_header(sz, decoded, (uint32_t)decoded_size));
        } else {
            dbg_str(DBG_ERROR, "unexpected next header type 0x%x", *p);
            THROW(-1);
        }
    } CATCH (ret) {} FINALLY {
        if (decoded) allocator_mem_free(allocator, decoded);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* object lifecycle                                                    */
/* ------------------------------------------------------------------ */

static int __construct(SevenZip *sz, char *init_str)
{
    allocator_t *allocator = sz->parent.parent.allocator;

    sz->file = object_new(allocator, "File", NULL);
    sz->file_name = object_new(allocator, "String", NULL);
    sz->buffer = object_new(allocator, "Buffer", NULL);
    sz->add_flag = 0;

    return 0;
}

static int __deconstruct(SevenZip *sz)
{
    __free_all(sz);
    object_destroy(sz->buffer);
    object_destroy(sz->file_name);
    object_destroy(sz->file);

    return 0;
}

/* 打开归档: 读取并校验文件头(签名/版本/StartHeader), 定位并解析 NextHeader
 * (支持 kHeader 与 kEncodedHeader 两种), 填充打包流/文件夹/文件元数据. */
static int __open(SevenZip *sz, char *archive_name, char *mode)
{
    Archive *archive = (Archive *)&sz->parent;
    File *a = archive->file;
    allocator_t *allocator = sz->parent.parent.allocator;
    uint8_t head[32];
    uint64_t next_offset, next_size;
    int size, ret = 0;

    TRY {
        dbg_str(DBG_VIP, "SevenZip open archive %s", archive_name);
        size = fs_get_size(archive_name);
        if (size <= 0) THROW(1);   /* empty/new archive */

        /* file header: signature(6) version(2) StartHeaderCRC(4)
         * next_header_offset(8)@12 next_header_size(8)@20 next_header_crc(4)@28 */
        EXEC(a->seek(a, 0, SEEK_SET));
        EXEC(a->read(a, head, 32));
        THROW_IF(memcmp(head, "\x37\x7a\xbc\xaf\x27\x1c", 6) != 0, -1);
        /* next_header_offset 是相对 32 字节文件头之后的偏移, 绝对位置 = 32 + offset */
        next_offset = 32 + __read_le64(head + 12);
        next_size = __read_le64(head + 20);
        THROW_IF(next_offset + next_size > (uint64_t)size, -1);

        sz->raw_header = allocator_mem_zalloc(allocator, next_size);
        THROW_IF(sz->raw_header == NULL, -1);
        sz->raw_header_size = (uint32_t)next_size;
        /* next_offset is absolute from the start of the archive */
        EXEC(a->seek(a, next_offset, SEEK_SET));
        EXEC(a->read(a, sz->raw_header, next_size));

        EXEC(__parse_next_header(sz, sz->raw_header, sz->raw_header_size));
    } CATCH (ret) {}

    return ret;
}

/* compute the byte span of a substream inside a folder's decoded output */
static void __get_substream_span(sz_folder_t *f, uint32_t si, uint64_t *off, uint64_t *len)
{
    uint32_t n = f->num_substreams ? f->num_substreams : 1;
    uint32_t k;
    uint64_t o = 0;

    for (k = 0; k < si && k + 1 < n; k++) o += f->substream_sizes[k];
    *off = o;
    if (si + 1 < n) *len = f->substream_sizes[si];
    else *len = f->unpack_sizes[0] - o;
}

static int __list(SevenZip *sz, Vector **infos)
{
    Archive *archive = (Archive *)&sz->parent;
    Vector *files = archive->extracting_file_infos;
    allocator_t *allocator = files->obj.allocator;
    uint32_t i;
    int ret = 0;

    TRY {
        THROW_IF(files == NULL, -1);
        files->reset(files);

        for (i = 0; i < sz->num_files; i++) {
            archive_file_info_t *info = allocator_mem_alloc(allocator, sizeof(archive_file_info_t));
            THROW_IF(info == NULL, -1);
            memset(info, 0, sizeof(*info));
            info->file_name = allocator_mem_zalloc(allocator, strlen((char *)sz->files[i].name) + 1);
            THROW_IF(info->file_name == NULL, -1);
            strcpy(info->file_name, (char *)sz->files[i].name);
            info->size = (uint32_t)sz->files[i].size;
            EXEC(files->add(files, info));
        }
        *infos = archive->extracting_file_infos;
    } CATCH (ret) {}

    return ret;
}

/* 提取单个文件: 先在 files 中按名字定位, 得到它所属的 folder 与 substream 序号;
 * 解码整个 folder, 再按 substream 的偏移/长度从解码结果中切片, 写入
 * extracting_path + 文件名, 并按需校验 CRC. 空文件/目录(empty_stream)只建文件. */
static int __extract_file(SevenZip *sz, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&sz->parent;
    File *out = sz->file;
    allocator_t *allocator = sz->parent.parent.allocator;
    uint8_t *buf = NULL;
    uint64_t size = 0, off = 0, len = 0;
    uint32_t i, fi = 0xFFFFFFFF, fi2, si;
    char out_name[1024];
    int ret = 0;

    TRY {
        for (i = 0; i < sz->num_files; i++) {
            if (strcmp((char *)sz->files[i].name, info->file_name) == 0) {
                fi = i;
                break;
            }
        }
        THROW_IF(fi == 0xFFFFFFFF, -1);

        THROW_IF(strlen(STR2A(archive->extracting_path)) + strlen((char *)sz->files[fi].name) >= sizeof(out_name), -1);
        strcpy(out_name, STR2A(archive->extracting_path));
        strcat(out_name, (char *)sz->files[fi].name);

        if (sz->files[fi].empty_stream) {
            EXEC(fs_mkfile(out_name, 0777));
            THROW(1);
        }

        fi2 = sz->files[fi].folder_index;
        si = sz->files[fi].substream_index;
        THROW_IF(fi2 == 0xFFFFFFFF, -1);

        EXEC(__decode_folder(sz, fi2, &buf, &size));
        __get_substream_span(&sz->folders[fi2], si, &off, &len);
        THROW_IF(off + len > size, -1);

        /* create the file (and its parent directories) before writing */
        EXEC(fs_mkfile(out_name, 0777));
        EXEC(out->open(out, out_name, "w+"));
        EXEC(out->write(out, buf + off, len));

        if (sz->files[fi].crc != 0xFFFFFFFF) {
            uint32_t crc = (uint32_t)crc32(0, buf + off, (unsigned int)len);
            THROW_IF(crc != sz->files[fi].crc, -1);
        }
        THROW(1);
    } CATCH (ret) {} FINALLY {
        if (buf) allocator_mem_free(allocator, buf);
        out->close(out);
    }

    return ret;
}

/* ------------------------------------------------------------------ */
/* writer                                                             */
/* ------------------------------------------------------------------ */

static uint8_t __lzma2_dict_to_prop(uint32_t dict)
{
    int p;
    for (p = 0; p < 40; p++) {
        if ((((uint32_t)2 | (p & 1)) << (p / 2 + 11)) == dict) return (uint8_t)p;
    }
    return 24;
}

static void __grow_array(allocator_t *allocator, void **ptr, uint32_t old_n, uint32_t new_n, size_t elem)
{
    void *np = allocator_mem_zalloc(allocator, (size_t)new_n * elem);
    if (np == NULL) return;
    if (*ptr && old_n) memcpy(np, *ptr, (size_t)old_n * elem);
    if (*ptr) allocator_mem_free(allocator, *ptr);
    *ptr = np;
}

/* 写入阶段: 把源文件读入内存, 用 raw LZMA2(dict=1<<24) 压缩成 packed 流,
 * 追加写入归档数据区; 并登记一个 folder(单 LZMA2 coder) 与该文件的元数据
 * (名字/大小/CRC). 每个文件一个 folder(非固实), 便于实现与校验. */
static int __add_file(SevenZip *sz, archive_file_info_t *info)
{
    Archive *archive = (Archive *)&sz->parent;
    File *a = archive->file, *file = sz->file;
    allocator_t *allocator = sz->parent.parent.allocator;
    uint8_t *data = NULL, *packed = NULL;
    uint64_t data_size, packed_size;
    uint32_t idx;
    sz_folder_t *f;
    sz_file_info_t *fi;
    int ret = 0;

    TRY {
        THROW_IF(info == NULL || info->file_name == NULL, -1);

        /* read the source file */
        EXEC(file->open(file, info->file_name, "r+"));
        data_size = fs_get_size(info->file_name);
        data = allocator_mem_alloc(allocator, data_size ? data_size : 1);
        THROW_IF(data == NULL, -1);
        EXEC(file->read(file, data, data_size));

        idx = sz->num_pack_streams;

        /* compress with raw LZMA2 (dict 1<<24 -> prop 24) */
        packed = allocator_mem_alloc(allocator, data_size + data_size / 2 + 4096);
        THROW_IF(packed == NULL, -1);
        packed_size = data_size + data_size / 2 + 4096;
        EXEC(__lzma_raw_encode(data, data_size, packed, &packed_size, (uint32_t)1 << 24));

        /* write the packed stream into the archive */
        if (sz->add_flag == 0) {
            uint8_t zero[32] = {0};
            EXEC(a->seek(a, 0, SEEK_SET));
            EXEC(a->write(a, zero, 32));
            sz->central_dir_position = 32;
            sz->add_flag = 1;
        }
        EXEC(a->seek(a, sz->central_dir_position, SEEK_SET));
        EXEC(a->write(a, packed, packed_size));
        sz->central_dir_position += packed_size;

        /* grow and record the packed stream */
        __grow_array(allocator, (void **)&sz->packed_sizes, sz->num_pack_streams, sz->num_pack_streams + 1, sizeof(uint64_t));
        __grow_array(allocator, (void **)&sz->packed_crcs, sz->num_pack_streams, sz->num_pack_streams + 1, sizeof(uint32_t));
        THROW_IF(sz->packed_sizes == NULL || sz->packed_crcs == NULL, -1);
        sz->packed_sizes[idx] = packed_size;
        sz->packed_crcs[idx] = 0xFFFFFFFF;
        sz->num_pack_streams = idx + 1;

        /* grow and record the folder (single LZMA2 coder, 1 packed stream) */
        __grow_array(allocator, (void **)&sz->folders, sz->num_folders, sz->num_folders + 1, sizeof(sz_folder_t));
        THROW_IF(sz->folders == NULL, -1);
        f = &sz->folders[sz->num_folders];
        memset(f, 0, sizeof(*f));
        f->num_coders = 1;
        f->coders = allocator_mem_zalloc(allocator, sizeof(sz_coder_t));
        THROW_IF(f->coders == NULL, -1);
        f->coders[0].method_id_size = 1;
        f->coders[0].method_id[0] = SZ_METHOD_LZMA2;
        f->coders[0].num_in_streams = 1;
        f->coders[0].num_out_streams = 1;
        f->coders[0].properties_size = 1;
        f->coders[0].properties = allocator_mem_alloc(allocator, 1);
        THROW_IF(f->coders[0].properties == NULL, -1);
        f->coders[0].properties[0] = __lzma2_dict_to_prop((uint32_t)1 << 24);
        f->num_packed_streams = 1;
        f->packed_indices = allocator_mem_zalloc(allocator, sizeof(uint32_t));
        THROW_IF(f->packed_indices == NULL, -1);
        f->packed_indices[0] = 0;
        f->first_packed_stream = idx;
        f->total_out_streams = 1;
        f->unpack_sizes = allocator_mem_zalloc(allocator, sizeof(uint64_t));
        THROW_IF(f->unpack_sizes == NULL, -1);
        f->unpack_sizes[0] = data_size;
        f->num_substreams = 1;
        sz->num_folders++;

        /* grow and record the file info */
        __grow_array(allocator, (void **)&sz->files, sz->num_files, sz->num_files + 1, sizeof(sz_file_info_t));
        THROW_IF(sz->files == NULL, -1);
        fi = &sz->files[sz->num_files];
        memset(fi, 0, sizeof(*fi));
        fi->name = allocator_mem_zalloc(allocator, strlen(info->file_name) + 1);
        THROW_IF(fi->name == NULL, -1);
        strcpy((char *)fi->name, info->file_name);
        fi->size = data_size;
        fi->crc = (uint32_t)crc32(0, data, (unsigned int)data_size);
        fi->empty_stream = 0;
        fi->empty_file = 0;
        fi->folder_index = sz->num_folders - 1;
        fi->substream_index = 0;
        sz->num_files++;
    } CATCH (ret) {} FINALLY {
        if (data) allocator_mem_free(allocator, data);
        if (packed) allocator_mem_free(allocator, packed);
    }

    return ret;
}

/* make sure the growing buffer can hold 'extra' more bytes */
static void __buf_reserve(uint8_t **buf, uint32_t *len, uint32_t *cap, uint32_t extra)
{
    allocator_t *allocator = allocator_get_default_instance();
    if (*len + extra > *cap) {
        uint32_t nc = *cap;
        while (nc < *len + extra) nc = nc ? nc * 2 : 256;
        uint8_t *nb = allocator_mem_zalloc(allocator, nc);
        if (nb && *buf) memcpy(nb, *buf, *len);
        if (*buf) allocator_mem_free(allocator, *buf);
        *buf = nb;
        *cap = nc;
    }
}

/* append one byte to a growing buffer */
static void __buf_append(uint8_t **buf, uint32_t *len, uint32_t *cap, uint8_t b)
{
    __buf_reserve(buf, len, cap, 1);
    (*buf)[(*len)++] = b;
}

/* append n bytes to a growing buffer */
static void __buf_append_bytes(uint8_t **buf, uint32_t *len, uint32_t *cap, const void *data, uint32_t n)
{
    __buf_reserve(buf, len, cap, n);
    if (n) memcpy(*buf + *len, data, n);
    *len += n;
}

/* 保存: 构建 NextHeader(kHeader: MainStreamsInfo + FilesInfo), 写入 32 字节
 * 文件头(含 NextHeader 的偏移/大小/CRC32), 打包数据区在 add_file 阶段已写好. */
static int __save(SevenZip *sz)
{
    Archive *archive = (Archive *)&sz->parent;
    File *a = archive->file;
    allocator_t *allocator = sz->parent.parent.allocator;
    uint8_t *buf = NULL;
    uint8_t n[16];
    uint8_t u16[2048];
    uint8_t le4[4];
    uint32_t blen = 0, bcap = 0, i, f, c, k, names_size;
    uint64_t total_packed = 0;
    uint32_t crc;
    uint8_t start_head[32];
    sz_folder_t *fold;
    sz_coder_t *coder;
    uint8_t flags;
    int nl, ul, ret = 0;

    TRY {
        THROW_IF(sz->add_flag == 0, 1);

        for (i = 0; i < sz->num_pack_streams; i++) total_packed += sz->packed_sizes[i];

        /* ---- build the next header ---- */
        __buf_append(&buf, &blen, &bcap, SZ_K_HEADER);

        /* MainStreamsInfo */
        __buf_append(&buf, &blen, &bcap, SZ_K_MAIN_STREAMS);
        /* PackInfo(标准): PackPos(相对数据起始 32) NumPackStreams kSize PackSizes kEnd */
        __buf_append(&buf, &blen, &bcap, SZ_K_PACK_INFO);
        nl = __write_number(n, 0); __buf_append_bytes(&buf, &blen, &bcap, n, nl);       /* pack pos */
        nl = __write_number(n, sz->num_pack_streams); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        __buf_append(&buf, &blen, &bcap, SZ_K_SIZE);
        for (i = 0; i < sz->num_pack_streams; i++) {
            nl = __write_number(n, sz->packed_sizes[i]); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        }
        __buf_append(&buf, &blen, &bcap, SZ_K_END);
        /* UnpackInfo */
        __buf_append(&buf, &blen, &bcap, SZ_K_UNPACK_INFO);
        __buf_append(&buf, &blen, &bcap, SZ_K_FOLDER);
        nl = __write_number(n, sz->num_folders); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        nl = __write_number(n, 0); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        for (f = 0; f < sz->num_folders; f++) {
            fold = &sz->folders[f];
            nl = __write_number(n, fold->num_coders); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
            for (c = 0; c < fold->num_coders; c++) {
                coder = &fold->coders[c];
                /* 标准 coder flags: 低 4 位 = 方法 ID 长度, bit0x10=带流数量, bit0x20=带属性 */
                flags = 0x20 | (coder->method_id_size & 0x0F);
                if (coder->num_in_streams != 1 || coder->num_out_streams != 1) flags |= 0x10;
                __buf_append(&buf, &blen, &bcap, flags);
                __buf_append_bytes(&buf, &blen, &bcap, coder->method_id, coder->method_id_size);
                if (flags & 0x10) {
                    nl = __write_number(n, coder->num_in_streams); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
                    nl = __write_number(n, coder->num_out_streams); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
                }
                nl = __write_number(n, coder->properties_size); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
                __buf_append_bytes(&buf, &blen, &bcap, coder->properties, coder->properties_size);
            }
            /* 单 coder 1 进 1 出时, 标准格式省略 bind pairs 与 packed stream 索引 */
            if (!(fold->num_coders == 1 && fold->coders[0].num_in_streams == 1
                  && fold->coders[0].num_out_streams == 1)) {
                nl = __write_number(n, 0); __buf_append_bytes(&buf, &blen, &bcap, n, nl);   /* bind pairs */
                nl = __write_number(n, fold->num_packed_streams); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
                for (c = 0; c < fold->num_packed_streams; c++) {
                    nl = __write_number(n, fold->packed_indices[c]); __buf_append_bytes(&buf, &blen, &bcap, n, nl);
                }
            }
        }
        __buf_append(&buf, &blen, &bcap, SZ_K_CODERS_UNPACK_SIZE);
        for (f = 0; f < sz->num_folders; f++) {
            nl = __write_number(n, sz->folders[f].unpack_sizes[0]);
            __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        }
        __buf_append(&buf, &blen, &bcap, SZ_K_END);
        /* SubStreamsInfo(标准): 每 folder 默认 1 个 substream, 直接写 kCRC */
        __buf_append(&buf, &blen, &bcap, SZ_K_SUBSTREAMS_INFO);
        __buf_append(&buf, &blen, &bcap, SZ_K_CRC);
        nl = __write_number(n, 1);   /* all defined */
        __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        for (i = 0; i < sz->num_files; i++) {
            __write_le32(le4, sz->files[i].crc);
            __buf_append_bytes(&buf, &blen, &bcap, le4, 4);
        }
        __buf_append(&buf, &blen, &bcap, SZ_K_END);
        __buf_append(&buf, &blen, &bcap, SZ_K_END);
        /* FilesInfo */
        __buf_append(&buf, &blen, &bcap, SZ_K_FILES_INFO);
        nl = __write_number(n, sz->num_files);
        __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        /* name record(标准格式): 数据以 0x00 开头, 随后是各名字的 UTF-16LE + NUL */
        __buf_append(&buf, &blen, &bcap, SZ_K_NAME);
        names_size = 1;
        for (k = 0; k < sz->num_files; k++) names_size += 2 * (strlen((char *)sz->files[k].name) + 1);
        nl = __write_number(n, names_size);
        __buf_append_bytes(&buf, &blen, &bcap, n, nl);
        __buf_append(&buf, &blen, &bcap, 0);
        for (k = 0; k < sz->num_files; k++) {
            ul = __utf8_to_utf16le((char *)sz->files[k].name, u16, sizeof(u16));
            __buf_append_bytes(&buf, &blen, &bcap, u16, (uint32_t)ul);
        }
        __buf_append(&buf, &blen, &bcap, SZ_K_END);
        __buf_append(&buf, &blen, &bcap, SZ_K_END);

        /* ---- write the file header + packed data + next header ---- */
        crc = (uint32_t)crc32(0, buf, blen);

        memset(start_head, 0, sizeof(start_head));
        start_head[0] = 0x37; start_head[1] = 0x7A; start_head[2] = 0xBC;
        start_head[3] = 0xAF; start_head[4] = 0x27; start_head[5] = 0x1C;
        start_head[6] = 0x00; start_head[7] = 0x04;                    /* version */
        /* 7z StartHeader 布局: crc@8(4) next_offset@12(8) next_size@20(8) next_crc@28(4).
         * StartHeaderCRC 覆盖 12..32 共 20 字节(offset + size + next_crc) */
        __write_le64(start_head + 12, total_packed);                  /* 相对 32 字节头之后的偏移 */
        __write_le64(start_head + 20, blen);                          /* next header size */
        __write_le32(start_head + 28, crc);                           /* next header crc */
        __write_le32(start_head + 8, (uint32_t)crc32(0, start_head + 12, 20));  /* StartHeaderCRC */

        EXEC(a->seek(a, 0, SEEK_SET));
        EXEC(a->write(a, start_head, 32));
        EXEC(a->seek(a, 32 + total_packed, SEEK_SET));
        EXEC(a->write(a, buf, blen));

        sz->add_flag = 0;
    } CATCH (ret) {} FINALLY {
        if (buf) allocator_mem_free(allocator, buf);
    }

    return ret;
}

static class_info_entry_t fs_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, SevenZip, construct, __construct),
    Init_Nfunc_Entry(2, SevenZip, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, SevenZip, open, __open),
    Init_Vfunc_Entry(4, SevenZip, extract_file, __extract_file),
    Init_Vfunc_Entry(5, SevenZip, add_file, __add_file),
    Init_Vfunc_Entry(6, SevenZip, list, __list),
    Init_Vfunc_Entry(7, SevenZip, save, __save),
    Init_End___Entry(8, SevenZip),
};
REGISTER_CLASS(SevenZip, fs_class_info);
