#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Command_s Command;

struct Command_s{
	Obj parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	/*virtual methods reimplement*/
    void * (*get_value)(Command *command,char *command_name, char *flag_name);
};

#endif
