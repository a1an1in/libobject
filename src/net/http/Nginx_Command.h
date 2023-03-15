#ifndef __NGINX_COMMAND_H__
#define __NGINX_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Server.h>

typedef struct Nginx_Command_s Nginx_Command;

struct Nginx_Command_s{
	Command parent;

	int (*construct)(Nginx_Command *command,char *init_str);
	int (*deconstruct)(Nginx_Command *command);
	int (*set)(Nginx_Command *command, char *attrib, void *value);
    void *(*get)(Nginx_Command *obj, char *attrib);
    char *(*to_json)(Nginx_Command *obj); 

	/*virtual methods reimplement*/
    void * (*run_action)(Nginx_Command *command);

    Http_Server *server;
};

#endif
