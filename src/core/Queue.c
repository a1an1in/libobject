/**
 * @file Queue.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-07
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/queue.h>
#include <libobject/event/event_base.h>

static int __construct(Queue *queue, char *init_str)
{
    allocator_t *allocator = queue->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(OBJ_DETAIL, "queue construct, queue addr:%p", queue);

    return 0;
}

static int __deconstrcut(Queue *queue)
{
    dbg_str(OBJ_DETAIL, "queue deconstruct, queue addr:%p", queue);
    int ret;
    void *tret;

    return 0;
}

static int __set(Queue *queue, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        queue->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        queue->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        queue->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        queue->deconstruct = value;
    }
    else if (strcmp(attrib, "add") == 0) {
        queue->add = value;
    } else if (strcmp(attrib, "add_front") == 0) {
        queue->add_front = value;
    } else if (strcmp(attrib, "add_back") == 0) {
        queue->add_back = value;
    } else if (strcmp(attrib, "remove") == 0) {
        queue->remove = value;
    } else if (strcmp(attrib, "remove_front") == 0) {
        queue->remove_front = value;
    } else if (strcmp(attrib, "remove_back") == 0) {
        queue->remove_back = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        queue->for_each = value;
    } else if (strcmp(attrib, "for_each_arg2") == 0) {
        queue->for_each_arg2 = value;
    } else if (strcmp(attrib, "begin") == 0) {
        queue->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        queue->end = value;
    }else if (strcmp(attrib, "size") == 0) {
        queue->size = value;
    }else if (strcmp(attrib, "is_empty") == 0) {
        queue->is_empty = value;
    }else if (strcmp(attrib, "clear") == 0) {
        queue->clear = value;
    }else if (strcmp(attrib, "for_each_arg") == 0) {
        queue->for_each_arg = value;
    }else if (strcmp(attrib, "for_each_arg3") == 0) {
        queue->for_each_arg3 = value;
    }else if (strcmp(attrib, "for_each_arg4") == 0) {
        queue->for_each_arg4 = value;
    }else if (strcmp(attrib, "for_each_arg5") == 0) {
        queue->for_each_arg5 = value;
    }else if (strcmp(attrib, "peek_front") == 0) {
        queue->peek_front = value;
    }else if (strcmp(attrib, "peek_back") == 0) {
        queue->peek_back = value;
    }
    else {
        dbg_str(OBJ_DETAIL, "queue set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Queue *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(OBJ_WARNNING, "queue get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }

    return NULL;
}

static void __for_each(Queue *queue, void (*func)(void *element))
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each");
    cur = queue->begin(queue);
    end = queue->end(queue);

    element = end->get_vpointer(end);
    dbg_str(DBG_IMPORTANT, "queue for_each, end element:%p", element);
    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element);
    }
}

static void 
__for_each_arg(Queue *queue,
               void (*func)(void *element, void *arg),
               void *arg)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each arg");
    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg);
    }
}

static void 
__for_each_arg2(Queue *queue,
                void (*func)(void *element, void *arg,void *arg1),
                void *arg1,void *arg2)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each arg2");
    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg1, arg2);
    }
}

static void
__for_each_arg3(Queue *queue,
                void (*func)(void *element, void *arg,
                             void *arg1,void *arg2),
                void *arg1,void *arg2,void *arg3)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each arg3");
    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg1, arg2, arg3);
    }
}

static void 
__for_each_arg4(Queue *queue,
                void (*func)(void *element, void *arg,
                             void *arg1,void *arg2,void *arg3), 
                void *arg1,void *arg2,void *arg3,void *arg4)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each arg4");
    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg1, arg2, arg3, arg4);
    }
}

static void 
__for_each_arg5(Queue *queue, 
                void (*func)(void *element, void *arg1,
                             void *arg2,void *arg3,void *arg4,void *arg5), 
                void *arg1,void *arg2,void *arg3,void *arg4, void *arg5)
{
    Iterator *cur, *end;
    void *element;

    dbg_str(OBJ_IMPORTANT, "queue for_each arg5");
    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); cur->next(cur)) {
        element = cur->get_vpointer(cur);
        func(element, arg1, arg2, arg3, arg4 , arg5);
    }
}


static class_info_entry_t queue_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add", NULL, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_front", NULL, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_back", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove_front", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove_back", NULL, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each", __for_each, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg", __for_each_arg, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "begin", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "end", NULL, sizeof(void *)}, 
	[15] = {ENTRY_TYPE_VFUNC_POINTER, "", "size", NULL, sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "is_empty", NULL, sizeof(void *)},  
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "clear", NULL, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg2", __for_each_arg2, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg3", __for_each_arg3, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg4", __for_each_arg4, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each_arg5", __for_each_arg5, sizeof(void *)},
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "peek_front", NULL, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_VFUNC_POINTER, "", "peek_back", NULL, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Queue", queue_class_info);
