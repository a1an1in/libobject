#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>
#include <libobject/argument/Option.h>
#include <libobject/argument/Argument.h>

typedef struct Command_s Command;

struct Command_s{
    Obj parent;

    int (*construct)(Command *command,char *init_str);
    int (*deconstruct)(Command *command);

    /*virtual methods reimplement*/
    void *(*get_value)(Command *command,char *command_name, char *flag_name);
    int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 
    int (*add_subcommand)(Command *command, char *);
    Command *(*get_subcommand)(Command *command, char *command_name);
    int (*add_option)(Command *command, char *name, char *alias, 
                      char *value, char *usage,
                      int (*action)(Option *, void *),
                      void *opaque);
    Option *(*get_option)(Command *command, char *option_name);
    int (*add_argument)(Command *command, char *value, char *usage, 
            int (*action)(Argument *, void *), void *opaque);
    Argument *(*get_argment)(Command *command, int index);
    int (*run_command)(Command *);
    int (*run_option_actions)(Command *);
    int (*run_argument_actions)(Command *);
    int (*parse_args)(Command *command);
    int (*set_args)(Command *command, int argc, char **argv);
    int (*help)(Command *command);

    /*attribs*/
    Command *selected_subcommand;
    Vector *subcommands;
    Vector *options;
    Vector *args;
    String *name;
    int argc;
    char **argv;
    void *opaque;
};

#endif
