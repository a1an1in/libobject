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
#include <libobject/core/Linked_Queue.h>
#include <libobject/media/Codec.h>
#include <libobject/media/Player.h>

static int __construct(Codec *codec,char *init_str)
{
    allocator_t *allocator = codec->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"codec construct, codec addr:%p",codec);
    codec->audio_frame_stream    = OBJECT_NEW(allocator, FrameStream, NULL);
    codec->video_frame_stream    = OBJECT_NEW(allocator, FrameStream, NULL);
    codec->subtitle_frame_stream = OBJECT_NEW(allocator, FrameStream, NULL);

    codec->thread                = OBJECT_NEW(allocator, Thread, NULL);

    codec->cur_render_audio_clock = 0.0;
    codec->cur_render_video_clock = 0.0;
    codec->codec_seek_flag        = 0;
    codec->codec_seek_timestamps  = 0;
    codec->codec_isold            = 1;

    return 0;
}

static int __deconstrcut(Codec *codec)
{
    dbg_str(DBG_DETAIL,"codec deconstruct,codec addr:%p",codec);
    object_destroy(codec->audio_frame_stream);
    object_destroy(codec->video_frame_stream);
    object_destroy(codec->subtitle_frame_stream);
    object_destroy(codec->thread);


    return 0;
}

static int __set(Codec *codec, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        codec->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        codec->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        codec->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        codec->deconstruct = value;
    } else if (strcmp(attrib, "run") == 0) {
        codec->run = value;
    } else if (strcmp(attrib, "set_player") == 0) {
        codec->set_player = value;
    } else if (strcmp(attrib, "set_video_packet_stream") == 0) {
        codec->set_video_packet_stream = value;
    } else if (strcmp(attrib, "set_audio_packet_stream") == 0) {
        codec->set_audio_packet_stream = value;
    } else if (strcmp(attrib, "set_subtitle_packet_stream") == 0) {
        codec->set_subtitle_packet_stream = value;
    } else if (strcmp(attrib, "thread_callback") == 0) {
        codec->thread_callback = value;
    } else if (strcmp(attrib, "update_texture") == 0) {
        codec->update_texture = value;
    } else if (strcmp(attrib, "update_texture_from_current_frame") == 0) {
        codec->update_texture_from_current_frame = value;
    } else if (strcmp(attrib, "get_rgb") == 0) {
        codec->get_rgb = value;
    } else if (strcmp(attrib, "get_rgb_from_current_frame") == 0) {
        codec->get_rgb_from_current_frame = value;
    } else if (strcmp(attrib, "get_yuv") == 0) {
        codec->get_yuv = value;
    } else if (strcmp(attrib, "get_yuv_from_current_frame") == 0) {
        codec->get_yuv_from_current_frame = value;
    } else if (strcmp(attrib, "seek") == 0) {
        codec->seek = value;
    } else if (strcmp(attrib, "open") == 0) {
        codec->open = value;
    } else if (strcmp(attrib, "stop") == 0) {
        codec->stop = value;
    } else if (strcmp(attrib, "close") == 0) {
        codec->close = value;
    }
    else if (strcmp(attrib, "extractor") == 0) {
        codec->extractor = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"codec set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Codec *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"codec get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __thread_callback(void *arg)
{
    dbg_str(DBG_SUC, "run at here");
}


static int __run(Codec *codec)
{
    Thread *thread =  codec->thread;
    Player *player = (Player *)codec->player;
    FrameStream *afs = codec->audio_frame_stream;
    FrameStream *vfs = codec->video_frame_stream;

    afs->set_maxsize(afs, MAX_AUDIO_FRAME_QUEUE_SIZE);
    afs->set_player(afs, player);
    afs->set_type(afs, FRAME_STREAM_AUDIO);

    vfs->set_maxsize(vfs, MAX_VIDEO_FRAME_QUEUE_SIZE);
    vfs->set_player(vfs, player);
    vfs->set_type(vfs, FRAME_STREAM_VIDEO);

    thread->set_start_routine(thread, codec->thread_callback);
    thread->set_start_arg(thread, codec);
    thread->start(thread);
    thread->detach(thread);

    return 0;
}

static int __set_player(Codec *codec, void *player)
{
    codec->player = player;
}

static int __set_video_packet_stream(Codec *codec, void *stream)
{
    codec->video_packet_stream = stream;
}

static int __set_audio_packet_stream(Codec *codec, void *stream)
{
    codec->audio_packet_stream = stream;
}

static int __set_subtitle_packet_stream(Codec *codec, void *stream)
{
    codec->subtitle_packet_stream = stream;
}

static int __update_texture(Codec *codec, void *texture)
{
    return NULL;
}


static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","run",__run,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_player",__set_player,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_video_packet_stream",__set_video_packet_stream,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_audio_packet_stream",__set_audio_packet_stream,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_subtitle_packet_stream",__set_subtitle_packet_stream,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","update_texture",__update_texture,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","update_texture_from_current_frame", NULL, sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","get_rgb",NULL,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","get_rgb_from_current_frame",NULL,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","get_yuv",NULL,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","get_yuv_from_current_frame",NULL,sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER,"","thread_callback",__thread_callback,sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER,"","seek",NULL,sizeof(void *)},
    [18] = {ENTRY_TYPE_VFUNC_POINTER,"","open",NULL,sizeof(void *)},
    [19] = {ENTRY_TYPE_VFUNC_POINTER,"","stop",NULL,sizeof(void *)},
    [20] = {ENTRY_TYPE_VFUNC_POINTER,"","close",NULL,sizeof(void *)},
    [21] = {ENTRY_TYPE_END},
};
REGISTER_CLASS(Codec,concurent_class_info);
