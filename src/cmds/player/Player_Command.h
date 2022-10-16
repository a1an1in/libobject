#ifndef __PLAYER_COMMAND_H__
#define __PLAYER_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Player_Command_s Player_Command;

struct Player_Command_s{
    Command parent;

    int (*construct)(Player_Command *command,char *init_str);
    int (*deconstruct)(Player_Command *command);
    int (*set)(Player_Command *command, char *attrib, void *value);
    void *(*get)(Player_Command *command, char *attrib);
    char *(*to_json)(Player_Command *command); 

    int (*run_action)(Player_Command *command);
};

#endif
