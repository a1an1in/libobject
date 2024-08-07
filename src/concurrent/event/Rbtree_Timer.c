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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/concurrent/event/Event_Base.h>
#include <libobject/concurrent/event/Rbtree_Timer.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/utils/timeval/timeval.h>

static int timeval_key_cmp_func(void *key1, void *key2, uint32_t size)
{
    return timeval_cmp((struct timeval *)key1, (struct timeval *)key2);
}

static int __construct(Rbtree_Timer *timer, char *init_str)
{
    allocator_t *allocator = timer->parent.obj.allocator;
    int key_size = sizeof(struct timeval);

    dbg_str(EV_DETAIL, "timer construct, timer addr:%p", timer);
    timer->map               = rbtree_map_alloc(allocator);
    timer->map->key_cmp_func = (key_cmp_fpt)timeval_key_cmp_func;

    rbtree_map_init(timer->map); 

    return 0;
}

static int __deconstrcut(Rbtree_Timer *timer)
{
    dbg_str(EV_DETAIL, "timer deconstruct, timer addr:%p", timer);

    rbtree_map_destroy(timer->map);
    return 0;
}

static int __add(Rbtree_Timer *timer, event_t *e)
{
    rbtree_map_t *map = timer->map;
    struct timeval now, null_tv = {0, 0};
    int ret;

    ret = timeval_cmp(&e->ev_timeout, &null_tv);
    if (ret <= 0) {
        return ret;
    }

    dbg_str(EV_DETAIL, "rbtree timer add, event addr : %p", e);

    timeval_now(&now, NULL);
    timeval_add(&e->ev_timeout, &now, &e->ev_timeout);
    rbtree_map_insert(map, &e->ev_timeout, e);

    return 0;
}

static int __del(Rbtree_Timer *timer, event_t *e) 
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct timeval null_tv = {0, 0};
    int ret;

    dbg_str(EV_DETAIL, "rbtree timer del");
    ret = timeval_cmp(&e->ev_timeout, &null_tv);
    if (ret <= 0) {
        return ret;
    }
    rbtree_map_search(map, &e->ev_timeout, &it);
    rbtree_map_delete(map, &it);

    return 0;
}

static int __remove(Rbtree_Timer *timer, event_t *e) 
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct timeval null_tv = {0, 0};
    int ret;

    dbg_str(EV_DETAIL, "rbtree timer del");
    ret = timeval_cmp(&e->ev_timeout, &null_tv);
    if (ret <= 0) {
        return ret;
    }
    rbtree_map_search(map, &e->ev_timeout, &it);
    rbtree_map_delete(map, &it);

    return 0;
}

/**
 * @Synopsis     get next timeout timeval
 *
 * @Param timer
 * @Param tv
 *
 * @Returns   
 */
static int __timeout_next(Rbtree_Timer *timer, struct timeval **tv)
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct rbtree_map_node *mnode;
    struct timeval *key, now;
    int ret;
    
    dbg_str(EV_DETAIL, "rbtree timer next timeout time");

    rbtree_map_begin(map, &it);
    if (it.rb_node_p == NULL) {
        *tv = NULL;
    } else {
        mnode = rb_entry(it.rb_node_p, struct rbtree_map_node, node);
        key = (struct timeval *)mnode->key;
        timeval_now(&now, NULL);

        /*
         *timeval_print(key);
         *timeval_print(&now);
         */

        ret = timeval_cmp(key, &now);
        if (ret > 0) {
            timeval_sub(key, &now, *tv);
            dbg_str(EV_DETAIL, "next timeout time in %d seconds, %d usecs",
                    (int)(*tv)->tv_sec, (int)(*tv)->tv_usec);
        } else {
            timeval_clear(*tv);
        }
    }

    return 0;
}

/**
 * @Synopsis      get first timeout event in the timer
 *
 * @Param timer
 *
 * @Returns       return timeout event
 */
event_t * __first(Rbtree_Timer *timer)
{
    rbtree_map_t *map = timer->map;
    rbtree_map_pos_t it;
    struct rbtree_map_node *mnode;
    char *p;
    event_t *ret = NULL;
    
    rbtree_map_begin(map, &it);
    if (it.rb_node_p == NULL) {
        dbg_str(EV_DETAIL, "rbtree timer, first is NULL");
        return NULL;
    } else {
        ret = (event_t *)rbtree_map_pos_get_pointer(&it);
        dbg_str(EV_DETAIL, "rbtree timer, first event addr:%p", ret);
    }

    return ret;
}

static class_info_entry_t rbtree_timer_class_info[] = {
    Init_Obj___Entry(0 , Timer, parent),
    Init_Nfunc_Entry(1 , Rbtree_Timer, construct, __construct),
    Init_Nfunc_Entry(2 , Rbtree_Timer, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Rbtree_Timer, set, NULL),
    Init_Vfunc_Entry(4 , Rbtree_Timer, get, NULL),
    Init_Vfunc_Entry(5 , Rbtree_Timer, add, __add),
    Init_Vfunc_Entry(6 , Rbtree_Timer, del, __del),
    Init_Vfunc_Entry(7 , Rbtree_Timer, remove, __remove),
    Init_Vfunc_Entry(8 , Rbtree_Timer, timeout_next, __timeout_next),
    Init_Vfunc_Entry(9 , Rbtree_Timer, first, __first),
    Init_End___Entry(10, Rbtree_Timer),
};
REGISTER_CLASS(Rbtree_Timer, rbtree_timer_class_info);

void test_obj_event_rbtree_timer()
{
    Timer *timer;
    allocator_t *allocator = allocator_get_default_instance();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    struct timeval now, tv, *ret = &tv;
    event_t event;

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p", c);
    cfg_config_str(c, "/Rbtree_Timer", "name", "alan timer") ;  

    timer = OBJECT_NEW(allocator, Rbtree_Timer, c->buf);

    object_dump(timer, "Rbtree_Timer", buf, 2048);
    dbg_str(EV_DETAIL, "Rbtree_Timer dump: %s", buf);

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


