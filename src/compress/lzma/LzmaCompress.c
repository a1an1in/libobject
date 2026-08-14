/**
 * @file LzmaCompress.c
 * @Synopsis
 *     implemented with liblzma (raw LZMA2 codec), which is what the 7z
 *     container format uses for its coded streams.
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <lzma.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File.h>
#include "LzmaCompress.h"

#define LZMA_CHUNK 16384
#define LZMA_PRESET 6
/* the decode window must be at least as large as the encoder's dictionary */
#define LZMA_DICT_SIZE (1 << 26)

static void __set_filters(lzma_filter filters[2], int for_encode)
{
    static lzma_options_lzma opt;

    memset(&opt, 0, sizeof(opt));
    if (for_encode) {
        lzma_lzma_preset(&opt, LZMA_PRESET);
    } else {
        opt.dict_size = LZMA_DICT_SIZE;
    }

    filters[0].id = LZMA_FILTER_LZMA2;
    filters[0].options = &opt;
    filters[1].id = LZMA_VLI_UNKNOWN;
    filters[1].options = NULL;
}

static int __compress_buf(LzmaCompress *c, char *in, int in_len, char *out, int *out_len)
{
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    __set_filters(filters, 1);
    if (lzma_raw_encoder(&strm, filters) != LZMA_OK) {
        dbg_str(DBG_ERROR, "lzma raw_encoder init failed");
        return -1;
    }

    strm.next_in = (const uint8_t *)in;
    strm.avail_in = (size_t)in_len;
    strm.next_out = (uint8_t *)out;
    strm.avail_out = (size_t)*out_len;

    ret = lzma_code(&strm, LZMA_FINISH);
    if (ret == LZMA_STREAM_END)
        *out_len = (int)(strm.next_out - (uint8_t *)out);
    lzma_end(&strm);

    if (ret != LZMA_STREAM_END) {
        dbg_str(DBG_ERROR, "lzma compress_buf error, ret:%d", (int)ret);
        return -1;
    }

    return 0;
}

static int __uncompress_buf(LzmaCompress *c, char *in, int in_len, char *out, int *out_len)
{
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    __set_filters(filters, 0);
    if (lzma_raw_decoder(&strm, filters) != LZMA_OK) {
        dbg_str(DBG_ERROR, "lzma raw_decoder init failed");
        return -1;
    }

    strm.next_in = (const uint8_t *)in;
    strm.avail_in = (size_t)in_len;
    strm.next_out = (uint8_t *)out;
    strm.avail_out = (size_t)*out_len;

    ret = lzma_code(&strm, LZMA_FINISH);
    if (ret == LZMA_STREAM_END)
        *out_len = (int)(strm.next_out - (uint8_t *)out);
    lzma_end(&strm);

    if (ret != LZMA_STREAM_END) {
        dbg_str(DBG_ERROR, "lzma uncompress_buf error, ret:%d", (int)ret);
        return -1;
    }

    return 0;
}

static int __compress_file(LzmaCompress *c, char *file_in, char *file_out)
{
    File *in_file = NULL, *out_file = NULL;
    allocator_t *allocator = c->parent.parent.allocator;
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    uint8_t inbuf[LZMA_CHUNK];
    uint8_t outbuf[LZMA_CHUNK];
    lzma_ret ret;
    size_t have;
    int action;

    TRY {
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        /* File::open returns 1 on success and -1 on failure */
        THROW_IF(in_file->open(in_file, file_in, "r+") < 0, -1);
        THROW_IF(out_file->open(out_file, file_out, "w+") < 0, -1);

        __set_filters(filters, 1);
        THROW_IF(lzma_raw_encoder(&strm, filters) != LZMA_OK, -1);

        do {
            strm.next_in = inbuf;
            strm.avail_in = fread(inbuf, 1, sizeof(inbuf), in_file->f);
            THROW_IF(ferror(in_file->f), -1);
            action = (strm.avail_in == 0 && feof(in_file->f)) ? LZMA_FINISH : LZMA_RUN;

            do {
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
                ret = lzma_code(&strm, action);
                THROW_IF(ret != LZMA_OK && ret != LZMA_STREAM_END, -1);
                have = sizeof(outbuf) - strm.avail_out;
                THROW_IF(have > 0 && fwrite(outbuf, 1, have, out_file->f) != have, -1);
            } while (strm.avail_out == 0);
        } while (action != LZMA_FINISH);

        THROW_IF(ret != LZMA_STREAM_END, -1);
        lzma_end(&strm);
    } CATCH (ret) {} FINALLY {
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}

static int __uncompress_file(LzmaCompress *c, char *file_in, char *file_out)
{
    File *in_file = NULL, *out_file = NULL;
    allocator_t *allocator = c->parent.parent.allocator;
    lzma_filter filters[2];
    lzma_stream strm = LZMA_STREAM_INIT;
    uint8_t inbuf[LZMA_CHUNK];
    uint8_t outbuf[LZMA_CHUNK];
    lzma_ret ret;
    size_t have;

    TRY {
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        /* File::open returns 1 on success and -1 on failure */
        THROW_IF(in_file->open(in_file, file_in, "r+") < 0, -1);
        THROW_IF(out_file->open(out_file, file_out, "w+") < 0, -1);

        __set_filters(filters, 0);
        THROW_IF(lzma_raw_decoder(&strm, filters) != LZMA_OK, -1);

        do {
            strm.next_in = inbuf;
            strm.avail_in = fread(inbuf, 1, sizeof(inbuf), in_file->f);
            THROW_IF(ferror(in_file->f), -1);
            if (strm.avail_in == 0)
                break;

            do {
                strm.next_out = outbuf;
                strm.avail_out = sizeof(outbuf);
                ret = lzma_code(&strm, LZMA_RUN);
                THROW_IF(ret != LZMA_OK && ret != LZMA_STREAM_END, -1);
                have = sizeof(outbuf) - strm.avail_out;
                THROW_IF(have > 0 && fwrite(outbuf, 1, have, out_file->f) != have, -1);
            } while (strm.avail_out == 0);
        } while (ret != LZMA_STREAM_END);

        THROW_IF(ret != LZMA_STREAM_END, -1);
        lzma_end(&strm);
    } CATCH (ret) {} FINALLY {
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, LzmaCompress, construct, NULL),
    Init_Nfunc_Entry(2, LzmaCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, LzmaCompress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, LzmaCompress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, LzmaCompress, compress_file, __compress_file),
    Init_Vfunc_Entry(6, LzmaCompress, uncompress_file, __uncompress_file),
    Init_End___Entry(7, LzmaCompress),
};
REGISTER_CLASS(LzmaCompress, zcompress_class_info);
