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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Queue.h>
#include <libobject/event/Event_Base.h>

static int __construct(Queue *queue, char *init_str)
{
    dbg_str(OBJ_DETAIL, "queue construct, queue addr:%p", queue);

    return 1;
}

static int __deconstrcut(Queue *queue)
{
    dbg_str(OBJ_DETAIL, "queue deconstruct, queue addr:%p", queue);
    int ret;
    void *tret;

    return 1;
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

static void __reset(Queue *queue)
{
    Iterator *cur, *end;
    void *element;

    cur = queue->begin(queue);
    end = queue->end(queue);

    for (; !end->equal(end, cur); 
           cur = queue->begin(queue), end = queue->end(queue)) 
    {
        queue->remove(queue, &element);
        if (queue->trustee_flag != 1) {
            continue;
        }

        if (queue->value_type == VALUE_TYPE_OBJ_POINTER && 
            element != NULL) {
            object_destroy(element);
        } else if (queue->value_type  == VALUE_TYPE_STRING &&
                   element != NULL) {
            object_destroy(element);
        } else if (queue->value_type  == VALUE_TYPE_ALLOC_POINTER &&
                   element != NULL) {
            allocator_mem_free(queue->obj.allocator, element);
        } else if (queue->value_type  == VALUE_TYPE_UNKNOWN_POINTER &&
                   element != NULL) {
            dbg_str(DBG_WARNNING, "not support reset unkown pointer");
        } else {
        }

        element = NULL;
    }
}

static class_info_entry_t queue_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Queue, construct, __construct),
    Init_Nfunc_Entry(2 , Queue, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Queue, set, NULL),
    Init_Vfunc_Entry(4 , Queue, get, NULL),
    Init_Vfunc_Entry(5 , Queue, add, NULL),
    Init_Vfunc_Entry(6 , Queue, add_front, NULL),
    Init_Vfunc_Entry(7 , Queue, add_back, NULL),
    Init_Vfunc_Entry(8 , Queue, remove, NULL),
    Init_Vfunc_Entry(9 , Queue, remove_front, NULL),
    Init_Vfunc_Entry(10, Queue, remove_back, NULL),
    Init_Vfunc_Entry(11, Queue, begin, NULL),
    Init_Vfunc_Entry(12, Queue, end, NULL),
    Init_Vfunc_Entry(13, Queue, size, NULL),
    Init_Vfunc_Entry(14, Queue, is_empty, NULL),
    Init_Vfunc_Entry(15, Queue, reset, __reset),
    Init_Vfunc_Entry(16, Queue, for_each, __for_each),
    Init_Vfunc_Entry(17, Queue, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(18, Queue, for_each_arg2, __for_each_arg2),
    Init_Vfunc_Entry(19, Queue, for_each_arg3, __for_each_arg3),
    Init_Vfunc_Entry(20, Queue, for_each_arg4, __for_each_arg4),
    Init_Vfunc_Entry(21, Queue, for_each_arg5, __for_each_arg5),
    Init_Vfunc_Entry(22, Queue, peek_front, NULL),
    Init_Vfunc_Entry(23, Queue, peek_back, NULL),
    Init_U8____Entry(24, Queue, trustee_flag, NULL),
    Init_U8____Entry(25, Queue, value_type, NULL),
    Init_End___Entry(26, Queue),
};
REGISTER_CLASS("Queue", queue_class_info);
