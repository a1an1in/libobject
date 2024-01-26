#ifndef __Node_Center_H__
#define __Node_Center_H__

#include <stdio.h>
#include <libobject/argument/Command.h>
#include <libobject/core/String.h>
#include "Node.h"

typedef struct Node_Center_s Node_Center;

struct Node_Center_s {
	Node parent;

	int (*construct)(Node_Center *command,char *init_str);
	int (*deconstruct)(Node_Center *command);
	int (*set)(Node_Center *command, char *attrib, void *value);
    void *(*get)(Node_Center *command, char *attrib);
    char *(*to_json)(Node_Center *command); 
};

#endif
