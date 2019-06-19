#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <stdio.h>
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
    void * (*get_value)(Application *app,char *app_name, char *flag_name);
};

#endif
