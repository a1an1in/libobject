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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/media/AVSync.h>

static int __construct(AVSync * avsync,char *init_str)
{
    allocator_t *allocator = avsync->obj.allocator;
    configurator_t * c;
    char buf[2048];
    avsync->audio_codec = NULL; 
    avsync->video_codec = NULL; 
    dbg_str(DBG_SUC,"avsync construct,avsync addr:%p",avsync);

    avsync->audio_mutex= object_new(allocator, "Mutex_Lock", NULL);
    avsync->video_mutex= object_new(allocator, "Mutex_Lock", NULL);

    return 0;
}

static int __deconstrcut(AVSync *avsync)
{
    dbg_str(DBG_DETAIL,"avsync deconstruct,avsync addr:%p",avsync);
    object_destroy(avsync->audio_mutex);
    object_destroy(avsync->video_mutex);
    return 0;
}

static int __set(AVSync *avsync, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        avsync->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        avsync->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        avsync->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        avsync->deconstruct = value;
    } else if (strcmp(attrib, "set_video_codec") == 0) {
        avsync->set_video_codec = value;
    } else if (strcmp(attrib, "set_audio_codec") == 0) {
        avsync->set_audio_codec = value;
    } else if (strcmp(attrib, "get_video_codec") == 0) {
        avsync->get_video_codec = value;
    } else if (strcmp(attrib, "get_audio_codec") == 0) {
        avsync->get_audio_codec = value;
    } else if (strcmp(attrib, "init") == 0) {
        avsync->init = value;
    }else if (strcmp(attrib, "async_core") == 0) {
        avsync->async_core = value;
    }else if (strcmp(attrib, "get_audio_clock") == 0) {
        avsync->get_audio_clock = value;
    }else if (strcmp(attrib, "get_video_clock") == 0) {
        avsync->get_video_clock = value;
    }else if (strcmp(attrib, "clear") == 0) {
        avsync->clear = value;
    }
    else {
        dbg_str(DBG_DETAIL,"avsync set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(AVSync *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"avsync get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void __set_video_codec(AVSync *obj,Codec *codec)
{
    obj->video_codec = codec;
}

static void __set_audio_codec(AVSync *obj,Codec *codec)
{
    obj->audio_codec = codec;
}

static Codec * __get_video_codec(AVSync *obj)
{
    return obj->video_codec;
}

static Codec * __get_audio_codec(AVSync *obj)
{
    return obj->audio_codec;
}

static void __init(AVSync *obj,Codec *videoc,Codec *audioc)
{
    obj->set_video_codec(obj,videoc);
    obj->set_audio_codec(obj,audioc);
}

static void __async_core(AVSync *obj, AVFrame *frame, Queue *q )
{
    double audio_clock = 0.0;
    double video_clock = 0.0;
    double frameRate   = 0.0;
    double delay       = 0.0;
    Codec *videoc      = NULL;
    double pts         = 0.0;
    //Codec *audioc      = NULL;

    if ((obj->get_audio_codec(obj) == NULL) &&
        (obj->get_video_codec(obj) == NULL)) {
        dbg_str(DBG_WARN,"avsync don't init");
        return ;
    }

    audio_clock = obj->get_audio_clock(obj);//seconds
    video_clock = obj->get_video_clock(obj);//seconds
    videoc      = obj->get_video_codec(obj);

    if (videoc == NULL) {
        dbg_str(DBG_WARN,"avsync VideoCodec can't find failed!");
        return ;
    }

    if (video_clock == 0 || audio_clock == 0) {
        delay = 0;
    } else {
        delay = fabs(video_clock - audio_clock);
    }

    //if audio frame pts error. set audio pts equel video 
    if (audio_clock == AV_NOPTS_VALUE ||
        obj->get_audio_codec(obj) == NULL ||
        audio_clock == 0.0 ) {
        audio_clock = video_clock;
        return;
    }

    dbg_str(DBG_DETAIL,"delay timestamp: %lf   video_current_timestamp :%lf audio_current_timestamp:%lf ",delay ,video_clock,audio_clock);

    frameRate = av_q2d(videoc->extractor->video_stream->avg_frame_rate);
    frameRate += frame->repeat_pict * (frameRate * 0.5);

    //if video pts wrong.then play ok
    if ( video_clock == 0.0 ) {   
        usleep((unsigned long)(frameRate*1000));
    } else {
        if ( delay > 0.05 && delay < 10.0 ) {
            if ( audio_clock < video_clock ) {
                usleep((unsigned long)(delay * 1000000));
            } else if (audio_clock > video_clock) {
                AVFrame * frame ;

#if 0
                while (!q->is_empty(q)) {
                    q->peek_front(q,&frame);
                    //if (frame->pts)
                    if (frame != NULL ) {
                        pts = av_frame_get_best_effort_timestamp(frame);
                        if (pts != AV_NOPTS_VALUE ) {
                            pts*=av_q2d(videoc->extractor->video_stream->time_base);    
                        }else {
                            pts = 0.0;
                        }
                        delay = audio_clock - pts;

                        if ( delay > 0.05 && delay < 10 ) {
                            q->remove(q,&frame);
                            av_frame_free(&frame);
                        } else {
                            break;
                        }

                    }
                }
#endif

            }
        }
    }


}

static double __get_audio_clock(AVSync *obj)
{
    double timestamp = 0.0;

    obj->audio_mutex->lock(obj->audio_mutex);
    timestamp= obj->audio_codec->cur_render_audio_clock;
    obj->audio_mutex->unlock(obj->audio_mutex);

    return timestamp;
}

static double __get_video_clock(AVSync *obj)
{
    double timestamp = 0.0;
    obj->video_mutex->lock(obj->video_mutex);
    timestamp= obj->video_codec->cur_render_video_clock;
    obj->video_mutex->unlock(obj->video_mutex);

    return timestamp;
}

static int __clear(AVSync *obj)
{
    obj->audio_codec->cur_render_audio_clock = 0;
    obj->audio_codec->cur_render_video_clock = 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_video_clock",__get_video_clock,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_audio_clock",__get_audio_clock,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","async_core",__async_core,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","init",__init,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_video_codec",__get_video_codec,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","get_audio_codec",__get_audio_codec,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","set_video_codec",__set_video_codec,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","set_audio_codec",__set_audio_codec,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","clear",__clear,sizeof(void *)},
    [14] = {ENTRY_TYPE_END},
};  
REGISTER_CLASS(AVSync, concurent_class_info);
