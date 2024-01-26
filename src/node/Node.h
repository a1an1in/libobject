#ifndef __Node_H__
#define __Node_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>

typedef struct Node_s Node;

struct Node_s {
	Obj parent;

	int (*construct)(Node *command,char *init_str);
	int (*deconstruct)(Node *command);
	int (*set)(Node *command, char *attrib, void *value);
    void *(*get)(Node *command, char *attrib);
    char *(*to_json)(Node *command); 

	int (*run_command)(Node *command);
};

#endif
