#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/iterator.h>

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
    void (*for_each)(Queue *queue,void (*func)(void *element));
    void (*for_each_arg2)(Queue *queue,void (*func)(void *element, void *arg), void *arg);
    size_t (*size)(Queue *);
    size_t (*empty)(Queue *);
    void   (*clear)(Queue *);
    void (*for_each_arg3)(Queue *queue,void (*func)(void *element, void *arg1,void *arg2),
                                                                                        void *arg1,void *arg2);
    void (*for_each_arg4)(Queue *queue,void (*func)(void *element, void *arg1,void *arg2,void *arg3), 
                                                                                        void *arg1,void *arg2,void *arg3);
    void (*for_each_arg5)(Queue *queue,void (*func)(void *element, void *arg1,void *arg2,void *arg3,void *arg4),
                                                                                         void *arg1,void *arg2,void *arg3,void *arg4);
    void (*for_each_arg6)(Queue *queue,void (*func)(void *element, void *arg1,void *arg2,void *arg3,void *arg4,void *arg5),
                                                                                          void *arg1,void *arg2,void *arg3,void *arg4,void *arg5);
    

    Iterator *(*begin)(Queue *queue);
    Iterator *(*end)(Queue *queue);

    Iterator *b, *e;
};

#endif
