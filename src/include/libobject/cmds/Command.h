#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>

typedef struct Command_s Command;
typedef struct command_option_s command_option_t;


struct command_option_s {
    char *name;
    char *alias;
    char *usage;
    int (*action)(void *, command_option_t *);
    void *action_arg;
    int type;
    int int_value;
    String *string_value;
};

struct Command_s{
	Obj parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);

	/*virtual methods reimplement*/
    void * (*get_value)(Command *command,char *command_name, char *flag_name);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 
    int (*add_subcommand)(Command *command, void *subcommand);
    Command * (*get_subcommand)(Command *command, char *command_name);

    /*attribs*/
    Vector *subcommands;
    Vector *options;
};

#endif
