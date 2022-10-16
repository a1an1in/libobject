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
#include <libavformat/avformat.h>
#include <libobject/media/FrameStream.h>
#include <libobject/media/Player.h>

static int __construct(FrameStream *frame_stream,char *init_str)
{
    allocator_t *allocator = frame_stream->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"frame_stream construct, frame_stream addr:%p",
            frame_stream);
    frame_stream->queue = OBJECT_NEW(allocator, Linked_Queue, NULL);

    return 0;
}

static int __deconstrcut(FrameStream *frame_stream)
{
    dbg_str(EV_DETAIL,"frame_stream deconstruct,frame_stream addr:%p",
            frame_stream);
    object_destroy(frame_stream->queue);

    return 0;
}

static int __set(FrameStream *frame_stream, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        frame_stream->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        frame_stream->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        frame_stream->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        frame_stream->deconstruct = value;
    } else if (strcmp(attrib, "add_frame") == 0) {
        frame_stream->add_frame = value;
    } else if (strcmp(attrib, "remove_frame") == 0) {
        frame_stream->remove_frame = value;
    } else if (strcmp(attrib, "size") == 0) {
        frame_stream->size = value;
    } else if (strcmp(attrib, "empty") == 0) {
        frame_stream->empty = value;
    } else if (strcmp(attrib, "clear") == 0) {
        frame_stream->clear = value;
    } else if (strcmp(attrib, "set_maxsize") == 0) {
        frame_stream->set_maxsize = value;
    } else if (strcmp(attrib, "set_player") == 0) {
        frame_stream->set_player = value;
    } else if (strcmp(attrib, "set_type") == 0) {
        frame_stream->set_type = value;
    }  
    else {
        dbg_str(EV_DETAIL,"frame_stream set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FrameStream *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"frame_stream get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add_frame(FrameStream *stream, void *frame)
{
    Player *player = stream->player;

    stream->queue->add(stream->queue, frame);

    if (stream->type == FRAME_STREAM_VIDEO) {
        dbg_str(DBG_IMPORTANT,"add a video frame, frame size=%d",
                stream->size(stream));
        if (stream->size(stream) == MAX_VIDEO_FRAME_QUEUE_SIZE) {
            player->set_state(player, STATE_VIDEO_FRAME_QUEUE_FULL);
        } 
        if (player->is_state(player, STATE_VIDEO_FRAME_QUEUE_EMPTY)) {
            player->clear_state(player, STATE_VIDEO_FRAME_QUEUE_EMPTY);
        }
    } else if (stream->type == FRAME_STREAM_AUDIO) {
        dbg_str(DBG_IMPORTANT,"add a audio frame, frame size=%d",
                stream->size(stream));
        if (stream->size(stream) == MAX_AUDIO_FRAME_QUEUE_SIZE) {
            player->set_state(player, STATE_AUDIO_FRAME_QUEUE_FULL);
        } 
        if (player->is_state(player, STATE_AUDIO_FRAME_QUEUE_EMPTY)) {
            player->clear_state(player, STATE_AUDIO_FRAME_QUEUE_EMPTY);
        }
    } else {
    }
    return 0;
}

static int __remove_frame(FrameStream *stream, void **frame)
{
    Player *player = stream->player;

    stream->queue->remove(stream->queue, frame);

    if (stream->type == FRAME_STREAM_VIDEO) {
        if (stream->size(stream) == 0) {
            player->set_state(player, STATE_VIDEO_FRAME_QUEUE_EMPTY);
        } else if (stream->size(stream) <= MAX_VIDEO_FRAME_QUEUE_SIZE / 3) {
            if (player->is_state(player, STATE_VIDEO_FRAME_QUEUE_FULL)) {
                player->clear_state(player, STATE_VIDEO_FRAME_QUEUE_FULL);
            }
        }

    } else if (stream->type == FRAME_STREAM_AUDIO) {
        if (stream->size(stream) == 0) {
            player->set_state(player, STATE_AUDIO_FRAME_QUEUE_EMPTY);
        } else if (stream->size(stream) <= MAX_AUDIO_FRAME_QUEUE_SIZE / 3) {
            if (player->is_state(player, STATE_AUDIO_FRAME_QUEUE_FULL)) {
                player->clear_state(player, STATE_AUDIO_FRAME_QUEUE_FULL);
            }
        }
    } else {
    }

    return 0;
}


static size_t __size(FrameStream *stream)
{
    return stream->queue->size(stream->queue);
}

static size_t __empty(FrameStream *stream)
{
    return stream->queue->is_empty(stream->queue);
}

static int64_t __max_frame_timestamp(FrameStream * stream)
{
    AVFrame * frame = NULL;
    AVFrame * t = NULL;
    void * element;
    Iterator *cur = NULL;
    Iterator *end = NULL;

    if (!stream->empty(stream)) {

        cur = stream->queue->begin(stream->queue);
        end = stream->queue->end(stream->queue);

        for (; !end->equal(end, cur); cur->next(cur)) {
            element=cur->get_vpointer(cur);
            if (element != NULL) {
                t =(AVFrame *)element;
                if (t->pts != AV_NOPTS_VALUE ) {
                    frame = t;
                }
            }
        }
    } else {
        return -1;
    }

    return frame->pts;
}

static int64_t __min_frame_timestamp(FrameStream * stream)
{
    AVFrame * frame = NULL;
    AVFrame * t = NULL;
    void * element;
    Iterator *cur = NULL;
    Iterator *end = NULL;

    if ( !stream->empty(stream) ) {

        cur = stream->queue->begin(stream->queue);
        end = stream->queue->end(stream->queue);

        for (; !end->equal(end, cur); cur->next(cur)) {
            element=cur->get_vpointer(cur);
            if ( element != NULL ) {
                t =(AVFrame *)element;
                if (t->pts != AV_NOPTS_VALUE ) {
                    frame = t;
                    break;
                }
            }
        }
    } else {
        return -1;
    }

    return frame->pts;
}


static void __clear(FrameStream * stream) 
{
    void * element;

    dbg_str(DBG_WARNNING, "clear FrameStream, size=%d",
            stream->size(stream));

    while (!stream->empty(stream)) {
        stream->queue->remove(stream->queue, &element);
        if (element != NULL) {
            /*
             *dbg_str(DBG_WARNNING, "clear a frame");
             */
            av_frame_free((AVFrame **)&element);
        }
    }
}

static size_t __set_maxsize(FrameStream *stream, int maxsize)
{
    stream->maxsize = maxsize;
}

static size_t __set_player(FrameStream *stream, void *player)
{
    stream->player = player;
}

static size_t __set_type(FrameStream *stream, void *type)
{
    stream->type = type;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","add_frame",__add_frame,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","remove_frame",__remove_frame,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","size",__size,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","empty",__empty,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","clear",__clear,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","set_maxsize",__set_maxsize,sizeof(void *)},
    [11] = {ENTRY_TYPE_FUNC_POINTER,"","set_player",__set_player,sizeof(void *)},
    [12] = {ENTRY_TYPE_FUNC_POINTER,"","set_type",__set_type,sizeof(void *)},
    [13] = {ENTRY_TYPE_END},
};

REGISTER_CLASS("FrameStream",concurent_class_info);
