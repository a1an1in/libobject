/**
 * @file Bz2Compress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-01-22
 */
#ifndef GZIP_H
#define GZIP_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/io/File.h>
#include "zlib.h"
#include "Bz2Compress.h"

 static int __compress_file(Bz2Compress *c, char *file_in, char *file_out)
 {
    File *in_file, *out_file;
    allocator_t *allocator = c->parent.parent.allocator;
    int ret;

    TRY {
      in_file = object_new(allocator, "File", NULL);
      out_file = object_new(allocator, "File", NULL);
      in_file->open(in_file, file_in, "r+");
      out_file->open(out_file, file_out, "w+");

      // EXEC(gz_compress(in_file->f, out_file->f));
    } CATCH (ret) {} FINALLY {
      object_destroy(in_file);
      object_destroy(out_file);
    }

    return ret;
 }

 static int __uncompress_file(Bz2Compress *c, char *file_in, char *file_out)
 {
    File *in_file, *out_file;
    allocator_t *allocator = c->parent.parent.allocator;
    int ret;

    TRY {
      in_file = object_new(allocator, "File", NULL);
      out_file = object_new(allocator, "File", NULL);
      in_file->open(in_file, file_in, "r+");
      out_file->open(out_file, file_out, "w+");
    
      // EXEC(gz_uncompress(in_file->f, out_file->f));
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
    Init_Vfunc_Entry(3, Bz2Compress, compress_file, __compress_file),
    Init_Vfunc_Entry(4, Bz2Compress, uncompress_file, __uncompress_file),
    Init_End___Entry(5, Bz2Compress),
};
REGISTER_CLASS("Bz2Compress", zcompress_class_info);
#endif

