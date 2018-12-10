/**
 * @file audio.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-03
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
#include <libobject/ui/audio.h>

static int __construct(Audio *audio, char *init_str)
{
    dbg_str(OBJ_DETAIL, "audio construct, audio addr:%p", audio);
    memset(audio->audio_buffer, 0, 1024 * 10 * 4);
    audio->audio_buffer_len = 0;
    audio->audio_read_pos = 0;

    return 0;
}

static int __deconstrcut(Audio *audio)
{
    dbg_str(OBJ_DETAIL, "audio deconstruct, audio addr:%p", audio);

    return 0;
}

static int __set(Audio *audio, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        audio->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        audio->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        audio->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        audio->deconstruct = value;
    } else if (strcmp(attrib, "set_freq") == 0) {
        audio->set_freq = value;
    } else if (strcmp(attrib, "set_format") == 0) {
        audio->set_format = value;
    } else if (strcmp(attrib, "set_channels") == 0) {
        audio->set_channels = value;
    } else if (strcmp(attrib, "set_buffer_size") == 0) {
        audio->set_buffer_size = value;
    } else if (strcmp(attrib, "set_callback") == 0) {
        audio->set_callback = value;
    } else if (strcmp(attrib, "set_callback_opaque") == 0) {
        audio->set_callback_opaque = value;
    } else if (strcmp(attrib, "init") == 0) {
        audio->init = value;
    } else if (strcmp(attrib, "play") == 0) {
        audio->play = value;
    } else if (strcmp(attrib, "pause") == 0) {
        audio->pause = value;
    } else {
        dbg_str(OBJ_WARNNING, "audio set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(Audio *audio, char *attrib)
{
    if (strcmp(attrib, "x") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING, "audio get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __set_freq(Audio *audio, int freq)
{
    dbg_str(DBG_DETAIL, "set_freq=%d", freq);
    audio->freq = freq;

    return 0;
}

static int __set_format(Audio *audio, int format)
{
    audio->format = format;
    dbg_str(DBG_DETAIL, "format=%d", format);

    return 0;
}

static int __set_channels(Audio *audio, int num)
{
    dbg_str(DBG_DETAIL, "set_channels=%d", num);
    audio->channel_num = num;

    return 0;
}

static int __set_buffer_size(Audio *audio, int size)
{
    audio->buffer_size =  size;

    return 0;
}

static int 
__set_callback(Audio *audio, int (*callback)(void *, void *))
{
    audio->callback= callback;

    return 0;
}


static int __set_callback_opaque(Audio *audio, void *opaque)
{
    audio->opaque = opaque;

    return 0;
}
static int __init(Audio *audio)
{
    return 0;
}

static int __play(Audio *audio)
{
    return 0;
}

static int __pause(Audio *audio)
{
    return 0;
}

static class_info_entry_t audio_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_freq", __set_freq, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_format", __set_format, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_channels", __set_channels, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_buffer_size", __set_buffer_size, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_callback", __set_callback, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_callback_opaque", __set_callback_opaque, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "init", __init, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "play", __play, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "pause", __pause, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_NORMAL_POINTER, "", "String", NULL, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("Audio", audio_class_info);

void test_obj_audio()
{
    Audio *audio;
    allocator_t *allocator = allocator_get_default_alloc();

    audio = OBJECT_NEW(allocator, Audio, "");
    audio->path->assign(audio->path, "hello world");
}


