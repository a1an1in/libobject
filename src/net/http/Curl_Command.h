#ifndef __CURL_COMMAND_H__
#define __CURL_COMMAND_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Client.h>

typedef struct Curl_Command_s Curl_Command;

struct Curl_Command_s{
	Command parent;

	int (*construct)(Command *command,char *init_str);
	int (*deconstruct)(Command *command);
	int (*set)(Command *command, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	/*virtual methods reimplement*/
    void * (*run_action)(Command *command);

    /*attribs*/
    String *output_file;
    String *append_output;
    String *input_file;
    String *bind_address;
    int help;
    int tries;
    Http_Client *client;
};

#endif
