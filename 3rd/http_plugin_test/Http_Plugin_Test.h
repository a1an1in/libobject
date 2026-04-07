#ifndef __Http_Plugin_Test_H__
#define __Http_Plugin_Test_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Http_Plugin_Test_s Http_Plugin_Test;

struct Http_Plugin_Test_s{
    Command parent;

    int (*construct)(Command *command,char *init_str);
    int (*deconstruct)(Command *command);
    int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 
    int (*run_command)(Command *);

    /*virtual methods reimplement*/
    void * (*get_value)(Command *command,char *command_name, char *flag_name);

    int help;
    String *option;
};

#endif
