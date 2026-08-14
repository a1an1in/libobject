/**
 * @file Bz2Compress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-01-22
 */
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File.h>
#include <bzlib.h>
#include "Bz2Compress.h"

#define BZ2_CHUNK 16384

static int __compress_buf(Bz2Compress *c, char *in, int in_len, char *out, int *out_len)
{
    unsigned int dest_len = (unsigned int)*out_len;
    int ret;

    ret = BZ2_bzBuffToBuffCompress(out, &dest_len, in, (unsigned int)in_len, 9, 0, 0);
    if (ret != BZ_OK) {
        dbg_str(DBG_ERROR, "bz2 compress_buf error, ret:%d", ret);
        return -1;
    }
    *out_len = (int)dest_len;

    return 0;
}

static int __uncompress_buf(Bz2Compress *c, char *in, int in_len, char *out, int *out_len)
{
    unsigned int dest_len = (unsigned int)*out_len;
    int ret;

    ret = BZ2_bzBuffToBuffDecompress(out, &dest_len, in, (unsigned int)in_len, 0, 0);
    if (ret != BZ_OK) {
        dbg_str(DBG_ERROR, "bz2 uncompress_buf error, ret:%d", ret);
        return -1;
    }
    *out_len = (int)dest_len;

    return 0;
}

static int __compress_file(Bz2Compress *c, char *file_in, char *file_out)
{
    File *in_file = NULL, *out_file = NULL;
    allocator_t *allocator = c->parent.parent.allocator;
    unsigned char inbuf[BZ2_CHUNK];
    unsigned char outbuf[BZ2_CHUNK];
    bz_stream strm;
    int ret, have;

    TRY {
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        /* File::open returns 1 on success and -1 on failure */
        THROW_IF(in_file->open(in_file, file_in, "r+") < 0, -1);
        THROW_IF(out_file->open(out_file, file_out, "w+") < 0, -1);

        memset(&strm, 0, sizeof(strm));
        THROW_IF(BZ2_bzCompressInit(&strm, 9, 0, 0) != BZ_OK, -1);

        /* feed phase: run until all input is consumed */
        for (;;) {
            strm.next_in = (char *)inbuf;
            strm.avail_in = (unsigned int)fread(inbuf, 1, sizeof(inbuf), in_file->f);
            THROW_IF(ferror(in_file->f), -1);
            if (strm.avail_in == 0)
                break;

            do {
                strm.next_out = (char *)outbuf;
                strm.avail_out = sizeof(outbuf);
                ret = BZ2_bzCompress(&strm, BZ_RUN);
                THROW_IF(ret != BZ_RUN_OK, -1);
                have = sizeof(outbuf) - strm.avail_out;
                THROW_IF(have > 0 && fwrite(outbuf, 1, have, out_file->f) != have, -1);
            } while (strm.avail_out == 0);
        }

        /* finish phase: flush the remaining data and end the stream */
        do {
            strm.next_out = (char *)outbuf;
            strm.avail_out = sizeof(outbuf);
            ret = BZ2_bzCompress(&strm, BZ_FINISH);
            THROW_IF(ret != BZ_FINISH_OK && ret != BZ_STREAM_END, -1);
            have = sizeof(outbuf) - strm.avail_out;
            THROW_IF(have > 0 && fwrite(outbuf, 1, have, out_file->f) != have, -1);
        } while (ret != BZ_STREAM_END);

        BZ2_bzCompressEnd(&strm);
    } CATCH (ret) {} FINALLY {
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}

static int __uncompress_file(Bz2Compress *c, char *file_in, char *file_out)
{
    File *in_file = NULL, *out_file = NULL;
    allocator_t *allocator = c->parent.parent.allocator;
    unsigned char inbuf[BZ2_CHUNK];
    unsigned char outbuf[BZ2_CHUNK];
    bz_stream strm;
    int ret, have;

    TRY {
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        /* File::open returns 1 on success and -1 on failure */
        THROW_IF(in_file->open(in_file, file_in, "r+") < 0, -1);
        THROW_IF(out_file->open(out_file, file_out, "w+") < 0, -1);

        memset(&strm, 0, sizeof(strm));
        THROW_IF(BZ2_bzDecompressInit(&strm, 0, 0) != BZ_OK, -1);

        do {
            strm.next_in = (char *)inbuf;
            strm.avail_in = (unsigned int)fread(inbuf, 1, sizeof(inbuf), in_file->f);
            THROW_IF(ferror(in_file->f), -1);
            if (strm.avail_in == 0)
                break;

            do {
                strm.next_out = (char *)outbuf;
                strm.avail_out = sizeof(outbuf);
                ret = BZ2_bzDecompress(&strm);
                THROW_IF(ret != BZ_OK && ret != BZ_STREAM_END, -1);
                have = sizeof(outbuf) - strm.avail_out;
                THROW_IF(have > 0 && fwrite(outbuf, 1, have, out_file->f) != have, -1);
            } while (strm.avail_out == 0);
        } while (ret != BZ_STREAM_END);

        THROW_IF(ret != BZ_STREAM_END, -1);
        BZ2_bzDecompressEnd(&strm);
    } CATCH (ret) {} FINALLY {
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, Bz2Compress, construct, NULL),
    Init_Nfunc_Entry(2, Bz2Compress, deconstruct, NULL),
    Init_Vfunc_Entry(3, Bz2Compress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, Bz2Compress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, Bz2Compress, compress_file, __compress_file),
    Init_Vfunc_Entry(6, Bz2Compress, uncompress_file, __uncompress_file),
    Init_End___Entry(7, Bz2Compress),
};
REGISTER_CLASS(Bz2Compress, zcompress_class_info);
