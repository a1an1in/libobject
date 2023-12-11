#ifndef __WINDOWS_FILE_SYSTEM_H__
#define __WINDOWS_FILE_SYSTEM_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/File_System.h>

typedef struct windows_file_system_s Windows_File_System;

struct windows_file_system_s{
    File_System parent;

	int (*construct)(Windows_File_System *,char *init_str);
	int (*deconstruct)(Windows_File_System *);
	int (*set)(Windows_File_System *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*list)(Windows_File_System *fs, char *name, char **list, int count, int max_name_len);
    int (*count_list)(Windows_File_System *fs, char *name);
    int (*is_directory)(Windows_File_System *fs, char *name);
    int (*get_mtime)(Windows_File_System *fs, char *path, char *time, int time_max_len);
    int (*get_size)(Windows_File_System *fs, char *path);
    int (*mkdir)(Windows_File_System *fs, char *path, mode_t mode);
    int (*rmdir)(Windows_File_System *fs, char *path);
};

#endif
