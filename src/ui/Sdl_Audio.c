/**
 * @file sdl_audio.c
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
#include <libobject/ui/sdl_audio.h>
#include <libobject/ui/audio.h>
#include <SDL2/SDL.h>

static int __construct(Sdl_Audio *sdl_audio, char *init_str)
{
    dbg_str(OBJ_DETAIL, "sdl_audio construct, sdl_audio addr:%p", sdl_audio);
    return 0;
}

static int __deconstrcut(Sdl_Audio *sdl_audio)
{
    dbg_str(OBJ_DETAIL, "sdl_audio deconstruct, sdl_audio addr:%p", sdl_audio);
#if 1
    dbg_str(DBG_DETAIL, "audio device id=%d", sdl_audio->dev_id);
    SDL_CloseAudioDevice(sdl_audio->dev_id); 
    dbg_str(DBG_DETAIL, "deconstruct audio device out");
#endif
    return 0;
}

static int __set(Sdl_Audio *sdl_audio, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        sdl_audio->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        sdl_audio->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        sdl_audio->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        sdl_audio->deconstruct = value;
    } else if (strcmp(attrib, "init") == 0) {
        sdl_audio->init = value;
    } else if (strcmp(attrib, "play") == 0) {
        sdl_audio->play = value;
    } else if (strcmp(attrib, "pause") == 0) {
        sdl_audio->pause = value;
    } else {
        dbg_str(OBJ_WARNNING, "sdl_audio set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(Sdl_Audio *sdl_audio, char *attrib)
{
    if (strcmp(attrib, "x") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING, "sdl_audio get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

#if 0
static int 
__writing_buffer_callback(void *opaque, void *stream, int len)
{
    Audio *audio = (Audio *)opaque;
    int read_len;

    SDL_memset(stream, 0, len);
    if(audio->audio_buffer_len == 0) {
        audio->audio_read_pos = 0; 
        audio->audio_buffer_len = audio->callback(audio->opaque, audio->audio_buffer); 
    }

    if (audio->audio_buffer_len <= 0) {
        return -1;
    } else {
    }

    dbg_str(DBG_IMPORTANT, "audio_buffer_len =%d, len=%d, read_pos=%d",
            audio->audio_buffer_len, len, audio->audio_read_pos);

	len = (len > audio->audio_buffer_len ? audio->audio_buffer_len : len);	/*  Mix  as  much  data  as  possible  */ 


#if 0
    SDL_MixAudio(stream, &audio->audio_buffer[audio->audio_read_pos], len, SDL_MIX_MAXVOLUME);
    dbg_buf(DBG_IMPORTANT, "write audio data to device:", &audio->audio_buffer[audio->audio_read_pos], 20);
#else
	SDL_MixAudioFormat(stream, audio->audio_buffer + audio->audio_read_pos, AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);
    dbg_buf(DBG_IMPORTANT, "write audio data to device:",
            audio->audio_buffer + audio->audio_read_pos, 20);
#endif
    
	audio->audio_read_pos += len; 
	audio->audio_buffer_len -= len; 

    return 0;
}
#else
static int 
__writing_buffer_callback(void *opaque, void *stream, int len)
{
    Audio *audio = (Audio *)opaque;
    int ret;
    char buffer[1024 * 10 * 2];

    SDL_memset(stream, 0, len);

    while (audio->audio_buffer_len < len) {
        if(audio->audio_buffer_len == 0) {
            ret = audio->callback(audio->opaque, audio->audio_buffer); 
            if (ret <= 0) {
                dbg_str(DBG_WARNNING, "read audio data nil");
                return 0;
            }
            audio->audio_read_pos = 0; 
            audio->audio_buffer_len = ret; 
        } else if (audio->audio_buffer_len >= len) {
        } else if (audio->audio_buffer_len < len && audio->audio_buffer_len > 0) {
            memcpy(buffer, audio->audio_buffer + audio->audio_read_pos,
                   audio->audio_buffer_len);
            memcpy(audio->audio_buffer,
                   buffer, audio->audio_buffer_len);
            audio->audio_read_pos = 0; 
            ret = audio->callback(audio->opaque, buffer); 
            if (ret <= 0) {
                dbg_str(DBG_WARNNING, "read audio data nil");
                return 0;
            }
            memcpy(audio->audio_buffer + audio->audio_buffer_len,
                   buffer, ret);
            audio->audio_buffer_len += ret;
        } else if (audio->audio_buffer_len < 0) {
            dbg_str(DBG_WARNNING, "audio buffer len wrong");
            audio->audio_buffer_len = 0;
            return -1;
        }
    }

    dbg_str(DBG_DETAIL, "audio_buffer_len =%d, read_pos=%d, len=%d",
            audio->audio_buffer_len, audio->audio_read_pos, len);

	SDL_MixAudioFormat(stream,
                       (uint8_t *)audio->audio_buffer + audio->audio_read_pos,
                       AUDIO_S16SYS,
                       len, SDL_MIX_MAXVOLUME);
    dbg_buf(DBG_DETAIL, "write audio data to device:",
            (void *)(audio->audio_buffer + audio->audio_read_pos), 20);
    
	audio->audio_read_pos += len; 
	audio->audio_buffer_len -= len; 

    return 0;
}
#endif

#if 0
static int __init(Sdl_Audio *sdl_audio)
{
    Audio *audio = (Audio *)sdl_audio;
	SDL_AudioSpec wanted_spec;

    dbg_str(DBG_SUC, "sdl audio initing. freq=%d, format=%d, channel_num=%d",
            audio->freq, audio->format, audio->channel_num); 
#if 1
	wanted_spec.freq     = audio->freq;
	wanted_spec.format   = audio->format;
	wanted_spec.channels = audio->channel_num;
	wanted_spec.silence  = 0;
    /*
	 *wanted_spec.samples  = audio->buffer_size;
     */
#else 
	wanted_spec.freq = 44100; 
	wanted_spec.format = AUDIO_S16SYS; 
	wanted_spec.channels = 2; 
	wanted_spec.silence = 0; 
	wanted_spec.samples = 1024; 
#endif
	wanted_spec.samples  = 1024;
	wanted_spec.callback = (void (*)(void *, Uint8 *,int))
                           __writing_buffer_callback; 
    wanted_spec.userdata = sdl_audio;

	if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
		dbg_str(DBG_ERROR, "can't open audio."); 
		return -1; 
	} 
    
    return 0;
}
#else
static int __init(Sdl_Audio *sdl_audio)
{
    Audio *audio = (Audio *)sdl_audio;
	SDL_AudioSpec *wanted_spec = &sdl_audio->wanted_spec;
	SDL_AudioSpec *spec = &sdl_audio->spec;

    dbg_str(DBG_SUC, "sdl audio initing."); 
	wanted_spec->freq     = audio->freq;
	wanted_spec->format   = AUDIO_S16SYS;
    /*
	 *wanted_spec->format   = audio->format;
     */
	wanted_spec->channels = audio->channel_num;
	wanted_spec->silence  = 0;
	wanted_spec->samples  = 1024;
	wanted_spec->callback = (void (*)(void *, Uint8 *,int))
                             __writing_buffer_callback; 
    wanted_spec->userdata = sdl_audio;

    sdl_audio->dev_id = SDL_OpenAudioDevice(NULL, 0, wanted_spec, spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE); 
	if (!sdl_audio->dev_id){ 
		dbg_str(DBG_ERROR, "can't open audio."); 
		exit(-1); 
	} 

    spec->format = AUDIO_S16SYS;

    dbg_str(DBG_IMPORTANT,
            "wanted specification: freq=%d, format=%d, channel_num=%d, samples=%d",
            wanted_spec->freq, wanted_spec->format, wanted_spec->channels, wanted_spec->samples); 
    
    dbg_str(DBG_IMPORTANT,
            "real specification: freq=%d, format=%d, channel_num=%d, samples=%d",
            spec->freq, spec->format, spec->channels, spec->samples); 

    return 0;
}

#endif

static int __play(Sdl_Audio *sdl_audio)
{
#if 0
    SDL_PauseAudio(0); //play
#else
    SDL_PauseAudioDevice(sdl_audio->dev_id, 0);
#endif

    return 0;
}

static int __pause(Sdl_Audio *sdl_audio)
{
#if 0
    SDL_PauseAudio(1); //play
#else
    SDL_PauseAudioDevice(sdl_audio->dev_id, 1);
#endif
    return 0;
}

static class_info_entry_t sdl_audio_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Audio", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "init", __init, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "play", __play, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "pause", __pause, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_NORMAL_POINTER, "", "String", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_END}, 

};
REGISTER_CLASS("Sdl_Audio", sdl_audio_class_info);

void test_obj_sdl_audio()
{
    Sdl_Audio *sdl_audio;
    allocator_t *allocator = allocator_get_default_alloc();

    sdl_audio = OBJECT_NEW(allocator, Sdl_Audio, "");
    sdl_audio->path->assign(sdl_audio->path, "hello world");
}


