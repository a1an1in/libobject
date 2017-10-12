#ifndef __LINKED_QUEUE_H__
#define __LINKED_QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/queue.h>

typedef struct linked_queue_s Linked_Queue;

struct linked_queue_s{
	Queue parent;

	int (*construct)(Linked_Queue *,char *init_str);
	int (*deconstruct)(Linked_Queue *);
	int (*set)(Linked_Queue *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Linked_Queue *queue, void *element);
    int (*add_back)(Linked_Queue *queue, void *element);
    int (*add_front)(Linked_Queue *queue, void *element);
    int (*remove)(Linked_Queue *queue, void **element);
    int (*remove_back)(Linked_Queue *queue, void **element);
    int (*remove_front)(Linked_Queue *queue, void **element);
};

#endif
