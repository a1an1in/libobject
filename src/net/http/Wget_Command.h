#ifndef __WGET_COMMAND_H__
#define __WGET_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Wget_Command_s Wget_Command;

struct Wget_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	/*virtual methods reimplement*/
    void * (*run_command)(Command *command);

    String *output_file;
    String *append_output;
    String *input_file;
    String *bind_address;
    int help;
    int tries;
};

#endif
