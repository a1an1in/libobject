#ifndef __LINKED_QUEUE_H__
#define __LINKED_QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/link_list.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Linkedlist_Iterator.h>

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
    size_t (*size)(Linked_Queue *);
    size_t (*is_empty)(Linked_Queue *);
    void   (*reset)(Linked_Queue *);

    int (*peek_front)(Linked_Queue *queue, void **element);
    int (*peek_back)(Linked_Queue *queue, void **element);

    Iterator *(*begin)(Queue *queue);
    Iterator *(*end)(Queue *queue);

    llist_t *llist;
};

#endif
