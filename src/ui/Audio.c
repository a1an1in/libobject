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

static int __close(Audio *audio)
{
    return 0;
}

static class_info_entry_t audio_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Audio, construct, __construct),
    Init_Nfunc_Entry(2 , Audio, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Audio, set, NULL),
    Init_Vfunc_Entry(4 , Audio, get, NULL),
    Init_Vfunc_Entry(5 , Audio, set_freq, __set_freq),
    Init_Vfunc_Entry(6 , Audio, set_format, __set_format),
    Init_Vfunc_Entry(7 , Audio, set_channels, __set_channels),
    Init_Vfunc_Entry(8 , Audio, set_buffer_size, __set_buffer_size),
    Init_Vfunc_Entry(9 , Audio, set_callback, __set_callback),
    Init_Vfunc_Entry(10, Audio, set_callback_opaque, __set_callback_opaque),
    Init_Vfunc_Entry(11, Audio, init, __init),
    Init_Vfunc_Entry(12, Audio, play, __play),
    Init_Vfunc_Entry(13, Audio, pause, __pause),
    Init_Vfunc_Entry(14, Audio, close, __close),
    Init_Str___Entry(15, Audio, path, NULL),
    Init_End___Entry(16, Audio),
};
REGISTER_CLASS("Audio", audio_class_info);

void test_obj_audio()
{
    Audio *audio;
    allocator_t *allocator = allocator_get_default_alloc();

    audio = OBJECT_NEW(allocator, Audio, "");
    audio->path->assign(audio->path, "hello world");
}


