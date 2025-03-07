#ifndef __NGINX_COMMAND_H__
#define __NGINX_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Server.h>

typedef struct Httpd_Command_s Httpd_Command;

struct Httpd_Command_s{
	Command parent;

	int (*construct)(Httpd_Command *command,char *init_str);
	int (*deconstruct)(Httpd_Command *command);
	int (*set)(Httpd_Command *command, char *attrib, void *value);
    void *(*get)(Httpd_Command *obj, char *attrib);
    char *(*to_json)(Httpd_Command *obj); 

	/*virtual methods reimplement*/
    void * (*run_command)(Httpd_Command *command);
	int (*load_plugins)(Httpd_Command *command);

    Http_Server *server;
	int loop_flag;
    Map *plugins;
};

#endif
