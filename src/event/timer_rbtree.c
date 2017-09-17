/**
 * @file timer_rbtree.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-09-17
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/config/config.h>
#include <libobject/event/event_base.h>
#include <libobject/event/timer_rbtree.h>
#include <libobject/utils/miscellany/buffer.h>

static int timeval_cmp(struct timeval *k1,struct timeval *k2)
{
    if (    k1->tv_sec > k2->tv_sec || 
            (k1->tv_sec == k2->tv_sec && k1->tv_usec > k2->tv_usec))
    {
        return 1;
    } else if (k1->tv_sec == k2->tv_sec && k1->tv_usec == k2->tv_usec) {
        return 0;
    } else {
        return -1;
    }
}

static int timeval_add(struct timeval *k1,struct timeval *k2, struct timeval *r)
{
    (r)->tv_sec = (k1)->tv_sec + (k2)->tv_sec;      
    (r)->tv_usec = (k1)->tv_usec + (k2)->tv_usec;       
    if ((r)->tv_usec >= 1000000) {            
        (r)->tv_sec++;                
        (r)->tv_usec -= 1000000;          
    }                           

    return 0;
}

static int timeval_sub(struct timeval *k1,struct timeval *k2, struct timeval *r)
{
    (r)->tv_sec = (k1)->tv_sec - (k2)->tv_sec;      
    (r)->tv_usec = (k1)->tv_usec - (k2)->tv_usec;   
    if ((r)->tv_usec < 0) {               
        (r)->tv_sec--;                
        (r)->tv_usec += 1000000;          
    }                           

    return 0;
}

static int timeval_now(struct timeval *t, struct timezone *tz)
{
    gettimeofday(t, tz);
}

static int timeval_clear(struct timeval *t)
{
    t->tv_sec = t->tv_usec = 0;
    return 0;
}

static int timeval_print(struct timeval *t)
{
    dbg_str(DBG_DETAIL,"timeval tv_sec=%d, tv_usec=%d", t->tv_sec, t->tv_usec);

    return 0;
}

static int timeval_key_cmp_func(void *key1,void *key2,uint32_t size)
{
    return timeval_cmp((struct timeval *)key1, (struct timeval *)key2);
}

static int __construct(Rbtree_Timer *timer,char *init_str)
{
    allocator_t *allocator = timer->parent.obj.allocator;
    int key_size = sizeof(struct timeval);

    dbg_str(EV_DETAIL,"timer construct, timer addr:%p",timer);
    timer->map                = rbtree_map_alloc(allocator);
    timer->map->key_cmp_func  = timeval_key_cmp_func;
    timer->map->fixed_key_len = 1;

    rbtree_map_init(timer->map,key_size,sizeof(void *)); 

    return 0;
}

static int __deconstrcut(Rbtree_Timer *timer)
{
    dbg_str(EV_DETAIL,"timer deconstruct,timer addr:%p",timer);

    rbtree_map_destroy(timer->map);
    return 0;
}

static int __set(Rbtree_Timer *timer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        timer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        timer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        timer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        timer->deconstruct = value;
    } else if (strcmp(attrib, "add") == 0) {
        timer->add = value;
    } else if (strcmp(attrib, "del") == 0) {
        timer->del = value;
    } else if (strcmp(attrib, "timeout_next") == 0) {
        timer->timeout_next = value;
    } else if (strcmp(attrib, "first") == 0) {
        timer->first = value;
    } else {
        dbg_str(EV_DETAIL,"timer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Rbtree_Timer *timer, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(OBJ_WARNNING,"timer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add(Rbtree_Timer *timer, event_t *e)
{
    rbtree_map_t *map = timer->map;
    struct timeval now, null_tv = {0, 0};
    char buffer[16];
    int ret;

    dbg_str(DBG_DETAIL,"rbtree timer add, event addr : %p", e);
    ret = timeval_cmp(&e->ev_timeout, &null_tv);
    if (ret <= 0) {
        return ret;
    }

    addr_to_buffer(e,buffer);
    timeval_now(&now, NULL);
    timeval_add(&e->ev_timeout, &now, &e->ev_timeout);
    rbtree_map_insert(map,&e->ev_timeout,buffer);

    return 0;
}

static int __del(Rbtree_Timer *timer, event_t *e) 
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;

    dbg_str(DBG_DETAIL,"rbtree timer del");
    rbtree_map_search(map, &e->ev_timeout,&it);
    rbtree_map_delete(map, &it);

    return 0;
}

static int __timeout_next(Rbtree_Timer *timer, struct timeval **tv)
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct rbtree_map_node *mnode;
    struct timeval *key, now;
    int ret;
    
    dbg_str(DBG_DETAIL,"rbtree timer next timeout time");

    rbtree_map_begin(map, &it);
    if (it.rb_node_p == NULL) {
        *tv = NULL;
    } else {
        mnode = rb_entry(it.rb_node_p,struct rbtree_map_node,node);
        key = (struct timeval *)mnode->key;
        timeval_now(&now, NULL);

        timeval_print(key);
        timeval_print(&now);

        ret = timeval_cmp(key,&now);
        if (ret > 0) {
            timeval_sub(key, &now, *tv);
            dbg_str(DBG_DETAIL,"next timeout time in %d seconds", (int)(*tv)->tv_sec);
        } else {
            timeval_clear(*tv);
        }
    }

    return 0;
}

event_t * __first(Rbtree_Timer *timer)
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct rbtree_map_node *mnode;
    char *p;
    event_t *ret = NULL;
    
    rbtree_map_begin(map, &it);
    if (it.rb_node_p == NULL) {
        dbg_str(DBG_DETAIL,"rbtree timer, first is NULL");
        return NULL;
    } else {
        p = (struct timeval *)rbtree_map_pos_get_pointer(&it);
        ret = (event_t *)buffer_to_addr(p);
        dbg_str(DBG_DETAIL,"rbtree timer, first event addr:%p", ret);
    }

    return ret;
}

static class_info_entry_t rbtree_timer_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ,"Timer","parent",NULL,sizeof(void *)},
    [1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5] = {ENTRY_TYPE_FUNC_POINTER,"","add",__add,sizeof(void *)},
    [6] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [7] = {ENTRY_TYPE_FUNC_POINTER,"","timeout_next",__timeout_next,sizeof(void *)},
    [8] = {ENTRY_TYPE_FUNC_POINTER,"","first",__first,sizeof(void *)},
    [9] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Rbtree_Timer",rbtree_timer_class_info);

void test_obj_event_rbtree_timer()
{
    Timer *timer;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    struct timeval now, tv, *ret = &tv;
    event_t event;

    c = cfg_alloc(allocator); 
    dbg_str(DBG_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Rbtree_Timer", CJSON_STRING, "name", "alan timer") ;  

    timer = OBJECT_NEW(allocator, Rbtree_Timer,c->buf);

    object_dump(timer, "Rbtree_Timer", buf, 2048);
    dbg_str(DBG_DETAIL,"Rbtree_Timer dump: %s",buf);

    event.ev_timeout.tv_sec  = 2;
    event.ev_timeout.tv_usec = 0;
    timer->add(timer, &event);
    /*
     *timer->del(timer, NULL);
     */
    timer->timeout_next(timer, &ret);

    object_destroy(timer);
    cfg_destroy(c);
}


