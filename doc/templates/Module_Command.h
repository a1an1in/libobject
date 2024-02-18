#ifndef __TEST_COMMAND_H__
#define __TEST_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Module_Command_s Module_Command;

struct Module_Command_s {
    Command parent;

    int (*construct)(Module_Command *command,char *init_str);
    int (*deconstruct)(Module_Command *command);
    int (*set)(Module_Command *command, char *attrib, void *value);
    void *(*get)(Module_Command *command, char *attrib);
    char *(*to_json)(Module_Command *command); 

    int (*run_command)(Module_Command *command);
};

#endif
