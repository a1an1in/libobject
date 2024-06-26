/**
 * @file Sender.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2018-10-28
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/message/Publisher.h> 
#include <libobject/concurrent/message/Centor.h>
#include <libobject/concurrent/message/Subscriber.h>

static int __construct(Publisher *publisher, char *init_str)
{
    allocator_t *allocator = publisher->obj.allocator;

    dbg_str(DBG_DETAIL, "publisher construct, publisher addr:%p", publisher);

    return 0;
}

static int __deconstrcut(Publisher *publisher)
{
    dbg_str(DBG_DETAIL, "publisher deconstruct, publisher addr:%p", publisher);

    return 0;
}

static int __connect_centor(Publisher *publish, Centor *centor)
{
    dbg_str(DBG_DETAIL, "publish message");
    publish->centor = centor;
}

static int __publish(Publisher *publish, message_t *message)
{
    dbg_str(DBG_DETAIL, "publish message, unsurported now");
}

int 
__publish_message(Publisher *publisher, char *what, void *opaque)
{
    message_t * message;
    Centor *centor = (Centor *)publisher->centor;
    allocator_t *allocator = publisher->obj.allocator;

    dbg_str(DBG_SUC, "publish raw message, publisher %p, what %s, opaque %p", publisher, what, opaque);

    message = message_alloc(allocator);
    message_set(message, "what", what);
    message_set(message, "opaque", opaque);
    message_set(message, "publisher", publisher);

    centor->message_queue->add(centor->message_queue, message);
    centor->c->send(centor->c, "p", 1, 0);
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Publisher, construct, __construct),
    Init_Nfunc_Entry(2 , Publisher, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Publisher, set, NULL),
    Init_Vfunc_Entry(4 , Publisher, get, NULL),
    Init_Vfunc_Entry(5 , Publisher, publish, __publish),
    Init_Vfunc_Entry(6 , Publisher, publish_message, __publish_message),
    Init_Vfunc_Entry(7 , Publisher, connect_centor, __connect_centor),
    Init_End___Entry(8 , Publisher),
};
REGISTER_CLASS(Publisher, concurent_class_info);

