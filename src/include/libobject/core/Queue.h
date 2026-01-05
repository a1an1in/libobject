#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Iterator.h>

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

    int (*peek_front)(Queue *queue, void **element);
    int (*peek_back)(Queue *queue, void **element);

    void (*for_each)(Queue *queue,void (*func)(void *element));
    void (*for_each_arg)(Queue *queue,void (*func)(void *element, void *arg), void *arg);
    size_t (*size)(Queue *);
    size_t (*is_empty)(Queue *);
    void   (*reset)(Queue *);

    Iterator *(*begin)(Queue *queue);
    Iterator *(*end)(Queue *queue);

    /*attribs*/
    Iterator *b, *e;
    uint8_t trustee_flag, value_type;
};

#endif
