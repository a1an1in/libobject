#ifndef __MOCKERY_COMMAND_H__
#define __MOCKERY_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>

typedef struct Mockery_Command_s Mockery_Command;

struct Mockery_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);

	/*virtual methods reimplement*/
	int (*set)(Command *command, char *attrib, void *value);
	int (*run_command)(Command *command);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj);

	char *func_name;
	char *argv[20];
	int argc;
	int argument_flag;
	int function_flag;

	Vector *failed_cases;
	Vector *success_cases;
};

#endif
