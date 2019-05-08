/**
 * @file event_base.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-13
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/config.h>
#include <libobject/event/Event_Base.h>
#include <libobject/event/Timer.h>

static int __construct(Timer *timer, char *init_str)
{
    dbg_str(EV_DETAIL, "timer construct, timer addr:%p", timer);

    return 0;
}

static int __deconstrcut(Timer *timer)
{
    dbg_str(EV_DETAIL, "timer deconstruct, timer addr:%p", timer);

    return 0;
}

static int __set(Timer *timer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        timer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        timer->get = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        timer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        timer->deconstruct = value;
    } else if (strcmp(attrib, "add") == 0) {
        timer->add = value;
    } else if (strcmp(attrib, "del") == 0) {
        timer->del = value;
    } else if (strcmp(attrib, "remove") == 0) {
        timer->remove = value;
    } else if (strcmp(attrib, "timeout_next") == 0) {
        timer->timeout_next = value;
    } else if (strcmp(attrib, "first") == 0) {
        timer->first = value;
    } else {
        dbg_str(EV_DETAIL, "timer set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Timer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING, "timer get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}


static class_info_entry_t timer_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Timer, construct, NULL),
    Init_Nfunc_Entry(2 , Timer, deconstruct, NULL),
    Init_Nfunc_Entry(3 , Timer, set, NULL),
    Init_Nfunc_Entry(4 , Timer, get, NULL),
    Init_Vfunc_Entry(5 , Timer, add, NULL),
    Init_Vfunc_Entry(6 , Timer, del, NULL),
    Init_Vfunc_Entry(7 , Timer, remove, NULL),
    Init_Vfunc_Entry(8 , Timer, timeout_next, NULL),
    Init_Vfunc_Entry(9 , Timer, first, NULL),
    Init_End___Entry(10, Timer),
};
REGISTER_CLASS("Timer", timer_class_info);

void test_obj_event_timer()
{
    Timer *timer;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Timer", CJSON_STRING, "name", "alan timer") ;  

    timer = OBJECT_NEW(allocator, Timer, c->buf);

    object_dump(timer, "Timer", buf, 2048);
    dbg_str(DBG_DETAIL, "Timer dump: %s", buf);

    object_destroy(timer);
    cfg_destroy(c);
}


