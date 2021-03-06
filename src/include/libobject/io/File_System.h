#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct file_system_s File_System;

struct file_system_s{
    Obj obj;

    int (*construct)(File_System *,char *init_str);
    int (*deconstruct)(File_System *);

    /*virtual methods reimplement*/
    int (*list)(File_System *fs, char *name, char **list, int count, int max_name_len);
    int (*is_directory)(File_System *fs, char *name);
    int (*count_list)(File_System *fs, char *name);
    int (*get_mtime)(File_System *fs, char *path, char *time, int time_max_len);
    int (*get_size)(File_System *fs, char *path);
};

#endif
