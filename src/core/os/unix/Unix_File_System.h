#ifndef __UNIX_FILE_SYSTEM_H__
#define __UNIX_FILE_SYSTEM_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File_System.h>

typedef struct unix_file_system_s Unix_File_System;

struct unix_file_system_s{
    File_System parent;

	int (*construct)(Unix_File_System *,char *init_str);
	int (*deconstruct)(Unix_File_System *);
	int (*set)(Unix_File_System *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*list_fixed)(Unix_File_System *fs, char *name, char **list, int count, int max_name_len);
    int (*count_list)(Unix_File_System *fs, char *name);
    int (*list)(Unix_File_System *fs, char *path, Vector *vector);
    int (*is_directory)(Unix_File_System *fs, char *name);
    int (*get_mtime)(Unix_File_System *fs, char *path, char *time, int time_max_len);
    int (*get_size)(Unix_File_System *fs, char *path);
    int (*mkdir)(Unix_File_System *fs, char *path, mode_t mode);
    int (*rmdir)(Unix_File_System *fs, char *path);
};

#endif
