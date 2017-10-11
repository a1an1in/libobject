#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct queue_s Queue;

struct queue_s{
	Obj obj;

	int (*construct)(Queue *,char *init_str);
	int (*deconstruct)(Queue *);
	int (*set)(Queue *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Queue *queue, void *element);
    int (*add_back)(Queue *queue, void *element);
    int (*add_front)(Queue *queue, void *element);
    int (*remove)(Queue *queue, void **element);
    int (*remove_back)(Queue *queue, void **element);
    int (*remove_front)(Queue *queue, void **element);
};

#endif
