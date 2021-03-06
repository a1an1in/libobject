#ifndef __CTEST_COMMAND_H__
#define __CTEST_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Ctest_Command_s Ctest_Command;

struct Ctest_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);

	/*virtual methods reimplement*/
	int (*set)(Command *command, char *attrib, void *value);
	int (*run_action)(Command *command);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 
};

#endif
