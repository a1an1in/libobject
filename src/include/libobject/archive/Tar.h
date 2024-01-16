#ifndef __GZIP_H__
#define __GZIP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/File.h>
#include <libobject/archive/Archive.h>

typedef struct Tar_s Tar;

struct posix_tar_header
{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[8];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
};

struct Tar_s {
    Archive parent;

    int (*construct)(Tar *, char *);
    int (*deconstruct)(Tar *);
	int (*open)(Tar *zip, char *archive_name, char *mode);

    /*virtual methods reimplement*/
    int (*set)(Tar *tar, char *attrib, void *value);
    void *(*get)(Tar *, char *attrib);
    char *(*to_json)(Tar *);
	int (*add_file)(Tar *, char *file_name);
	int (*extract_file)(Tar *, archive_file_info_t *info);
	int (*list)(Tar *Tar, Vector **infos);
	int (*save)(Tar *a);

    File *file;
	String *file_name;
};

#endif
