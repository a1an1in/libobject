#ifndef __STRATEGY_H__
#define __STRATEGY_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Strategy_s Strategy;

struct Strategy_s{
	Obj parent;

	int (*construct)(Strategy *,char *);
	int (*deconstruct)(Strategy *);

	/*virtual methods reimplement*/
	int (*set)(Strategy *module, char *attrib, void *value);
    void *(*get)(Strategy *, char *attrib);
};

#endif
