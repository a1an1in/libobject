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
#include <libobject/media/Player.h>
#include <libobject/media/Extractor.h>

static int __construct(Extractor *extractor,char *init_str)
{
    allocator_t *allocator = extractor->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"extractor construct, extractor addr:%p",extractor);
    extractor->audio_packet_stream    = OBJECT_NEW(allocator, PacketStream, NULL);
    extractor->video_packet_stream    = OBJECT_NEW(allocator, PacketStream, NULL);
    extractor->subtitle_packet_stream = OBJECT_NEW(allocator, PacketStream, NULL);

    extractor->thread                 = OBJECT_NEW(allocator, Thread, NULL);

    extractor->audio_stream           = NULL;
    extractor->video_stream           = NULL;

    extractor->extractor_seek_tag     = 0;
    extractor->is_seek_finished       = 0;
    extractor->seek_timestamp         = 0;

    extractor->is_switch_fihished     = 0;
    extractor->seek_flags             = 0;
    extractor->switch_flags            =0;
    return 0;
}

static int __deconstrcut(Extractor *extractor)
{
    dbg_str(DBG_DETAIL,"extractor deconstruct,extractor addr:%p",extractor);
    object_destroy(extractor->audio_packet_stream);
    object_destroy(extractor->video_packet_stream);
    object_destroy(extractor->subtitle_packet_stream);

    object_destroy(extractor->thread);

    return 0;
}

static int __set(Extractor *extractor, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        extractor->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        extractor->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        extractor->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        extractor->deconstruct = value;
    } else if (strcmp(attrib, "thread_callback") == 0) {
        extractor->thread_callback = value;
    } else if (strcmp(attrib, "set_data_source") == 0) {
        extractor->set_data_source = value;
    } else if (strcmp(attrib, "set_player") == 0) {
        extractor->set_player = value;
    } else if (strcmp(attrib, "run") == 0) {
        extractor->run = value;
    } else if (strcmp(attrib, "guard_dog") == 0) {
        extractor->guard_dog = value;
    } else if (strcmp(attrib, "seek") == 0) {
        extractor->seek = value;
    } else if (strcmp(attrib, "open") == 0) {
        extractor->open = value;
    } else if (strcmp(attrib, "stop") == 0) {
        extractor->stop = value;
    } else if (strcmp(attrib, "close") == 0) {
        extractor->close = value;
    } else if (strcmp(attrib, "switch_bitrate") == 0) {
        extractor->switch_bitrate = value;
    } else if (strcmp(attrib, "data_source") == 0) {
        extractor->data_source = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"extractor set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Extractor *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else if (strcmp(attrib, "data_source") == 0) {
        return obj->data_source;
    } else {
        dbg_str(DBG_WARN,"extractor get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __thread_callback(void *arg)
{
    dbg_str(DBG_SUC, "run at here");
}

static int __set_data_source(Extractor *extractor, void *data_source)
{
    extractor->data_source = data_source;
}

static int __set_player(Extractor *extractor, void *player)
{
    extractor->player = player;
}

static int __run(Extractor *extractor)
{
    Player *player = (Player *)extractor->player;
    Thread *thread =  extractor->thread;

    dbg_str(DBG_SUC, "Extractor");

    thread->set_start_routine(thread, extractor->thread_callback);
    thread->set_start_arg(thread, extractor);
    thread->start(thread);

    return 0;
}

static int __guard_dog(Extractor *extractor)
{
    size_t audio_packet_size = 0; 
    size_t video_packet_size = 0; 

    audio_packet_size = extractor->audio_packet_stream->size(extractor->audio_packet_stream);
    video_packet_size = extractor->video_packet_stream->size(extractor->video_packet_stream);

    if (audio_packet_size >= 200 || video_packet_size >= 400) {
        return 1;
    }

    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_data_source",__set_data_source,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_player",__set_player,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","thread_callback",__thread_callback,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","run",__run,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","guard_dog",__guard_dog,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","seek", NULL, sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","open", NULL, sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","stop", NULL, sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","close", NULL, sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","switch_bitrate", NULL, sizeof(void *)},
    [15] = {ENTRY_TYPE_END},
};

REGISTER_CLASS(Extractor,concurent_class_info);
