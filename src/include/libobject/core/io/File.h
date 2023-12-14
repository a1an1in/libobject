#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <sys/stat.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/io/Stream.h>

typedef struct file_s File;

struct file_s{
    Stream parent;

    int (*construct)(File *,char *init_str);
    int (*deconstruct)(File *);
    int (*set)(File *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*open)(File *file, char *path, char *mode);
    int (*rename)(File *file, char *path);
    int (*read)(File *, void *dst, int len);
    int (*write)(File *, void *src, int len);
    int (*close)(File *file);
    int (*is_exist)(File *file, char *path);
    int (*mkdir)(File *file, char *path, mode_t mode);
    int (*get_size)(File *file);
    int (*seek)(File *file, long offset, int from);

    /*attribs*/
    String *name;
    FILE *f;
};


int file_compute_crc32(char *file_name, uint32_t *crc);

#endif
