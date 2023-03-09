#ifndef __FSHELL_COMMAND_H__
#define __FSHELL_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct FShell_Command_s FShell_Command;

struct FShell_Command_s{
    Command parent;

    int (*construct)(FShell_Command *command,char *init_str);
    int (*deconstruct)(FShell_Command *command);
    int (*set)(FShell_Command *command, char *attrib, void *value);
    void *(*get)(FShell_Command *command, char *attrib);
    char *(*to_json)(FShell_Command *command); 

    int (*run_action)(FShell_Command *command);

    void *shell;
};

#endif
