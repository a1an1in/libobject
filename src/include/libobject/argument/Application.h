#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <stdio.h>
#include <libobject/core/Map.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Application_s Application;

struct Application_s{
    Command parent;

    int (*construct)(Application *app,char *init_str);
    int (*deconstruct)(Application *app);

    /*virtual methods reimplement*/
    int (*set)(Application *app, char *attrib, void *value);
    void *(*get)(Application *, char *attrib);
    char *(*to_json)(Application *); 
    int (*add_subcommand)(Application *, char *);
    Command *(*get_subcommand)(Application *, char *command_name);
    int (*run)(Application *, int argc, char *argv[]);
    int (*run_command)(Application *);
    int (*help)(Application *);
    int (*load_plugin)(Application *app, char *name, char *path, char *json, void *opaque);

    /*attribs*/
    int argc;
    char **argv;
    String *root;
    Map *plugins;
    void *fshell;
};

extern int app(int argc, char *argv[]);
extern int app_register_cmd(char *cmd);
extern Application *get_global_application();

#define REGISTER_APP_CMD(app_cmd_name, class_info) \
    __attribute__((constructor)) static void register_##app_cmd_name()\
    {\
        ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register app cmd %s\n",\
                     REGISTRY_CTOR_PRIORITY_REGISTER_APP_CMD, #app_cmd_name);\
        \
        __register_ctor_func1(REGISTRY_CTOR_PRIORITY_REGISTER_APP_CMD,\
                (int (*)(void *))app_register_cmd,  #app_cmd_name);\
        __register_ctor_func3(REGISTRY_CTOR_PRIORITY_REGISTER_CLASS,\
                (int (*)(void *, void * , void *))class_deamon_register_class, NULL, #app_cmd_name, class_info);\
    }


#endif
