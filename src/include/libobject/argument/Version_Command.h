#ifndef __VERSION_COMMAND_H__
#define __VERSION_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Version_Command_s Version_Command;

struct Version_Command_s{
    Command parent;

    int (*construct)(Command *command,char *init_str);
    int (*deconstruct)(Command *command);
    int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

    int (*run_command)(Command *command);
};

#endif
