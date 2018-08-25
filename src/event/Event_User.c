/**
 * @file event_user.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-09-24
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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_user.h>
#include <unistd.h>

static int __construct(Event_User *event_user, char *init_str)
{
    dbg_str(EV_DETAIL, "event_user construct, event_user addr:%p", event_user);

    return 0;
}

static int __deconstrcut(Event_User *event_user)
{
    dbg_str(EV_DETAIL, "event_user deconstruct, event_user addr:%p", event_user);

    return 0;
}

static int __set(Event_User *event_user, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        event_user->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        event_user->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        event_user->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        event_user->deconstruct = value;
    } 
    else {
        dbg_str(EV_DETAIL, "event_user set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Event_User *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING, "event_user get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t event_user_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Event_User", event_user_class_info);

/*
 *void test_obj_event_user()
 *{
 *    Event_User *event_user;
 *    allocator_t *allocator = allocator_get_default_alloc();
 *    configurator_t * c;
 *    char *set_str;
 *    cjson_t *root, *e, *s;
 *    char buf[2048];
 *
 *    c = cfg_alloc(allocator); 
 *    dbg_str(EV_SUC, "configurator_t addr:%p", c);
 *    cfg_config(c, "/Event_User", CJSON_STRING, "name", "alan event_user") ;  
 *
 *    event_user = OBJECT_NEW(allocator, Event_User, c->buf);
 *
 *    object_dump(event_user, "Event_User", buf, 2048);
 *    dbg_str(EV_DETAIL, "Event_User dump: %s", buf);
 *
 *    object_destroy(event_user);
 *    cfg_destroy(c);
 *}
 */
static void
test_timeout_cb(int fd, short event, void *arg)
{
    struct timeval newtime, difference;
    double elapsed;
    static struct timeval lasttime;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC, "timeout_cb called at %d: %.3f seconds elapsed.", 
            (int)newtime.tv_sec, elapsed);
}

/*
 *void test_obj_event_thread()
 */
void test_obj_event_user()
{
    Event_User *user;
    allocator_t *allocator = allocator_get_default_alloc();
    event_t event;
    char buf[2048];

    user = OBJECT_NEW(allocator, Event_User, NULL);

    object_dump(user, "Thread", buf, 2048);
    dbg_str(DBG_DETAIL, "Thread dump: %s", buf);

    sleep(1);

    /*
     *event.ev_fd              = -1;
     *event.ev_events          = EV_READ | EV_PERSIST;
     *event.ev_timeout.tv_sec  = 2;
     *event.ev_timeout.tv_usec = 0;
     *event.ev_callback        = test_timeout_cb;
     *event.ev_arg             = &event;
     *user->add_event(user, (void *)&event);
     */

    sleep(10);
    pause();

    object_destroy(user);
}


