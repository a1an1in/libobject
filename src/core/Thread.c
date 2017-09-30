/**
 * @thread Thread.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-27
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
#include <libobject/core/thread.h>

static int __construct(Thread *thread,char *init_str)
{
    allocator_t *allocator = thread->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"thread construct, thread addr:%p",thread);

    return 0;
}

static int __deconstrcut(Thread *thread)
{
    dbg_str(DBG_DETAIL,"thread deconstruct,thread addr:%p",thread);
    int ret;
    void *tret;

    ret = pthread_join(thread->tid, &tret);
    if (ret != 0) {
        dbg_str(DBG_WARNNING,"can't join with thread tid=%d", thread->tid);
    }

    return 0;
}

static int __set(Thread *thread, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        thread->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        thread->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        thread->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        thread->deconstruct = value;
    } else if (strcmp(attrib, "start") == 0) {
        thread->start = value;
    } else if (strcmp(attrib, "set_start_routine") == 0) {
        thread->set_start_routine = value;
    }
    else if (strcmp(attrib, "start_routine") == 0) {
        thread->start_routine = value;
    }
    else {
        dbg_str(DBG_DETAIL,"thread set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Thread *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"thread get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __start(Thread *thread)
{
    if (thread->start_routine == NULL) {
        return -1;
    }

    if ((pthread_create(&thread->tid, NULL, thread->start_routine, thread->arg)) != 0) {
        dbg_str(DBG_WARNNING,"pthread start error");
    }

    return 0;
}

static int __set_start_routine(Thread *thread, void *routine)
{
    thread->start_routine = routine;
}

static void *__start_routine(void *arg)
{
    dbg_str(DBG_DETAIL,"start_routine");

    return NULL;
}

static class_info_entry_t thread_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5] = {ENTRY_TYPE_FUNC_POINTER,"","start",__start,sizeof(void *)},
    [6] = {ENTRY_TYPE_FUNC_POINTER,"","set_start_routine",__set_start_routine,sizeof(void *)},
    [7] = {ENTRY_TYPE_VFUNC_POINTER,"","start_routine",__start_routine,sizeof(void *)},
    [8] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Thread",thread_class_info);

void *test_func(void *arg)
{
    dbg_str(DBG_SUC,"test func");
}
void test_obj_thread()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p",c);
    /*
     *cfg_config(c, "/Thread", CJSON_STRING, "name", "alan thread") ;  
     */

    thread = OBJECT_NEW(allocator, Thread, NULL);

    object_dump(thread, "Thread", buf, 2048);
    dbg_str(DBG_DETAIL,"Thread dump: %s",buf);
    thread->set_start_routine(thread, test_func);
    thread->start(thread);

    sleep(1);

    object_destroy(thread);
    cfg_destroy(c);
}
