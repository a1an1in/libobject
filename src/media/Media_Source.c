/**
 * @file  
 * @author  jackwu
 * @version 
 * @date  
 */
/* Copyright (c) 2015-2020
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
#include <libobject/media/Media_Source.h>

static int __set(Media_Source *media, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        media->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        media->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        media->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        media->deconstruct = value;
    } else if (strcmp(attrib, "set_url") == 0) {
        media->set_url = value;
    } else if (strcmp(attrib, "get_url") == 0) {
        media->get_url = value;
    } else if (strcmp(attrib, "get_list_size") == 0) {
        media->get_list_size = value;
    } else if (strcmp(attrib, "get_url_by_bitrate") == 0) {
        media->get_url_by_bitrate = value;
    } else if (strcmp(attrib, "init") == 0) {
        media->init = value;
    } else if (strcmp(attrib, "info") == 0) {
        media->info = value;
    } else if (strcmp(attrib, "isvalid_bitrate") == 0) {
        media->isvalid_bitrate = value;
    } else {
        dbg_str(DBG_DETAIL,"Media_Source set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Media_Source *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"Media_Source get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}


static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",NULL,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",NULL,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_url", NULL,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_url",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_url_by_bitrate",NULL,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","init",NULL,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_list_size",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","info",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","isvalid_bitrate",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Media_Source",concurent_class_info);

