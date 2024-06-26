/**
 * @file concurrent.c
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
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/media/MediaPeriod.h>

static int __construct(MediaPeriod *media_period,char *init_str)
{
    allocator_t *allocator = media_period->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"media_period construct, media_period addr:%p",media_period);

    return 0;
}

static int __deconstrcut(MediaPeriod *media_period)
{
    dbg_str(DBG_DETAIL,"media_period deconstruct,media_period addr:%p",media_period);

    return 0;
}

static int __set(MediaPeriod *media_period, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        media_period->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        media_period->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        media_period->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        media_period->deconstruct = value;
    } else if (strcmp(attrib, "set_url") == 0) {
        media_period->set_url = value;
    } else if (strcmp(attrib, "get_url") == 0) {
        media_period->get_url = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"media_period set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(MediaPeriod *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"media_period get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void *__set_url(MediaPeriod *period, char *url)
{
    period->url = url;
    return NULL;
}

static void *__get_url(MediaPeriod *period)
{
    return period->url;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_url",__set_url,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_url",__get_url,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS(MediaPeriod,concurent_class_info);
