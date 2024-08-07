#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>
#include <libobject/core/Struct_Adapter.h>

typedef struct file_system_s File_System;
typedef Struct_Adapter FS_File_Info;

#if (defined(WINDOWS_USER_MODE))
typedef unsigned long long uint64_t;
#endif

typedef struct fs_file_info_s {
    char *file_name;
    uint16_t compression_method;
    struct stat st;
} fs_file_info_t;

struct file_system_s {
    Obj obj;

    int (*construct)(File_System *,char *init_str);
    int (*deconstruct)(File_System *);

    /*virtual methods reimplement*/
    int (*list_fixed)(File_System *fs, char *name, char **list, int count, int max_name_len);
    int (*list)(File_System *fs, char *path, Vector *vector);
    int (*is_directory)(File_System *fs, char *name);
    int (*count_list)(File_System *fs, char *name);
    int (*get_mtime)(File_System *fs, char *path, char *time, int time_max_len);
    int (*get_atime)(File_System *fs, char *path, char *time, int time_max_len);
    int (*get_ctime)(File_System *fs, char *path, char *time, int time_max_len);
    int (*get_size)(File_System *fs, char *path);
    int (*get_stat)(File_System *fs, char *path, struct stat *stat);
    int (*mkdir)(File_System *fs, char *path, mode_t mode);
    int (*rmdir)(File_System *fs, char *path);
};

#endif
