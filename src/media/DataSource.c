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
#include <libobject/media/DataSource.h>
#include <libobject/media/MediaPeriod.h>

static int __construct(DataSource *data_source,char *init_str)
{
    allocator_t *allocator = data_source->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"data_source construct, data_source addr:%p",data_source);

    return 0;
}

static int __deconstrcut(DataSource *data_source)
{
    dbg_str(EV_DETAIL,"data_source deconstruct,data_source addr:%p",data_source);

    return 0;
}

static int __set(DataSource *data_source, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        data_source->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        data_source->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        data_source->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        data_source->deconstruct = value;
    } else if (strcmp(attrib, "media_peroid") == 0) {
        data_source->media_peroid = ((MediaPeriod *)value);
    } 
    else {
        dbg_str(EV_DETAIL,"data_source set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(DataSource *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else if (strcmp(attrib, "media_peroid") == 0) {
        return obj->media_peroid;
    } else {
        dbg_str(EV_WARN,"data_source get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("DataSource",concurent_class_info);

