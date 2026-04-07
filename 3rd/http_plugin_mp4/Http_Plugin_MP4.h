#ifndef __Http_Plugin_MP4_H__
#define __Http_Plugin_MP4_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Http_Plugin_MP4_s Http_Plugin_MP4;

struct Http_Plugin_MP4_s{
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
