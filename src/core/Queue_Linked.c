/**
 * @file Linked_Queue.c
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/utils/config/config.h>
#include <libobject/utils/timeval/timeval.h>
#include <libobject/core/queue_linked.h>

static int __construct(Linked_Queue *queue,char *init_str)
{
    dbg_str(DBG_DETAIL,"queue construct, queue addr:%p",queue);

    return 0;
}

static int __deconstrcut(Linked_Queue *queue)
{
    dbg_str(DBG_DETAIL,"queue deconstruct,queue addr:%p",queue);
    int ret;

    return 0;
}

static int __set(Linked_Queue *queue, char *attrib, void *value)
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
    }
    else {
        dbg_str(DBG_DETAIL,"queue set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Linked_Queue *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"queue get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t linked_queue_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Queue","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add",NULL,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add_front",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add_back",NULL,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","remove",NULL,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","remove_front",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","remove_back",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Linked_Queue",linked_queue_class_info);
