#ifndef __ARCHIVE_COMMAND_H__
#define __ARCHIVE_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/archive/Archive.h>

typedef struct Archive_Command_s Archive_Command;

/* 子命令类型 */
typedef enum archive_command_type_e {
    ARCHIVE_CMD_UNKNOWN = 0,
    ARCHIVE_CMD_LIST,        /* archive list    <file> [-w 过滤]  */
    ARCHIVE_CMD_EXTRACT,     /* archive extract <file> [-o 目录]  */
    ARCHIVE_CMD_CREATE,      /* archive create  <out> <源路径>    */
    ARCHIVE_CMD_ADD,         /* archive add     <file> <源文件>   */
    ARCHIVE_CMD_MAX,
} archive_command_type_e;

struct Archive_Command_s {
    Command parent;

    int (*construct)(Archive_Command *command, char *init_str);
    int (*deconstruct)(Archive_Command *command);

    /*virtual methods reimplement*/
    int (*run_command)(Command *command);

    archive_command_type_e command_type;
    char *arg1;       /* 归档文件路径(或压缩输入文件) */
    char *arg2;       /* 源路径 / 目标路径           */

    Archive *archive; /* 本次打开的归档对象(命令结束时销毁) */
    String *output;   /* -o 输出目录 / 输出文件 */
    String *wildcard; /* -w 通配符过滤 */
};

#endif
