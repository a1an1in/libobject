/**
 * @file message_t.c
 * @Synopsis  
 * @author alan lin
 * @version 1
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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/message/message.h> 
#include <libobject/message/publisher.h>

message_t *message_alloc(allocator_t *allocator)
{
    message_t *ret = NULL;

    dbg_str(DBG_DETAIL, "message_create");

    ret = (message_t *)allocator_mem_alloc(allocator, sizeof(message_t));
    if (ret == NULL) {
        dbg_str(DBG_ERROR, "allock err");
    }

    memset(ret,  0,  sizeof(message_t));

    ret->allocator = allocator;
    dbg_str(DBG_SUC, "message_create end, ret =%p", ret);

    return ret;
}

int message_set(message_t *message, char *attrib, void *value)
{
    dbg_str(DBG_DETAIL, "message_set, message addr:%p", message);

    if (!strcmp(attrib, "what")) {
        message->what = value;
    } else if (!strcmp(attrib, "opaque")) {
        message->opaque = value;
    } else if (!strcmp(attrib, "publisher")) {
        message->publisher = value;
    } else {
        dbg_str(DBG_WARNNING, "not support attrib setting, please check");
        return -1;
    }

    return 0;
}

int message_destroy(message_t * message)
{
    int ret = 0;

    allocator_mem_free(message->allocator, message);

    return ret;
}

