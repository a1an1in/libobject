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
#include <libobject/media/PacketStream.h>
#include <libobject/media/Player.h>

static int __construct(PacketStream *packet_stream,char *init_str)
{
    allocator_t *allocator = packet_stream->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"packet_stream construct, packet_stream addr:%p",
            packet_stream);
    packet_stream->queue = OBJECT_NEW(allocator, Linked_Queue, NULL);

    return 0;
}

static int __deconstrcut(PacketStream *packet_stream)
{
    dbg_str(EV_DETAIL,"packet_stream deconstruct,packet_stream addr:%p",
            packet_stream);
    object_destroy(packet_stream->queue);

    return 0;
}

static int __set(PacketStream *packet_stream, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        packet_stream->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        packet_stream->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        packet_stream->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        packet_stream->deconstruct = value;
    } else if (strcmp(attrib, "add_packet") == 0) {
        packet_stream->add_packet = value;
    } else if (strcmp(attrib, "remove_packet") == 0) {
        packet_stream->remove_packet = value;
    } else if (strcmp(attrib, "size") == 0) {
        packet_stream->size = value;
    } else if (strcmp(attrib, "set_maxsize") == 0) {
        packet_stream->set_maxsize = value;
    } else if (strcmp(attrib, "set_player") == 0) {
        packet_stream->set_player = value;
    } else if (strcmp(attrib, "set_type") == 0) {
        packet_stream->set_type = value;
    } else if (strcmp(attrib, "empty") == 0) {
        packet_stream->empty = value;
    } else if (strcmp(attrib, "clear") == 0) {
        packet_stream->clear = value;
    } 
    else {
        dbg_str(EV_DETAIL,"packet_stream set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(PacketStream *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARN,"packet_stream get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add_packet(PacketStream *stream, void *packet)
{
    Player *player = stream->player;

    stream->queue->add(stream->queue, packet);

    if (stream->type == PACKET_STREAM_VIDEO) {
        dbg_str(DBG_INFO,
                "add a video packet to packet queue, packet size =%d",
                stream->size(stream));

        if (stream->size(stream) == MAX_VIDEO_PACKET_QUEUE_SIZE) {
            player->set_state(player, STATE_VIDEO_PACKET_QUEUE_FULL);
        }
        if (player->is_state(player, STATE_VIDEO_PACKET_QUEUE_EMPTY)) {
            player->clear_state(player, STATE_VIDEO_PACKET_QUEUE_EMPTY);
        }
    } else if (stream->type == PACKET_STREAM_AUDIO) {
        dbg_str(DBG_INFO,
                "add a audio packet to packet queue, packet size =%d", 
                stream->size(stream));

        if (stream->size(stream) == MAX_AUDIO_PACKET_QUEUE_SIZE) {
            player->set_state(player, STATE_AUDIO_PACKET_QUEUE_FULL);
        }
        if (player->is_state(player, STATE_AUDIO_PACKET_QUEUE_EMPTY)) {
            player->clear_state(player, STATE_AUDIO_PACKET_QUEUE_EMPTY);
        }
    } else {
    }
    return 0;
}

static int __add_null_packet(PacketStream *stream, int index)
{
    return 0;
}

static int __remove_packet(PacketStream *stream, void **packet)
{
    Player *player = stream->player;

    stream->queue->remove(stream->queue, packet);

    if (stream->type == PACKET_STREAM_VIDEO) {
        if (stream->size(stream) == 0) {
            player->set_state(player, STATE_VIDEO_PACKET_QUEUE_EMPTY);
        } else if (stream->size(stream) <= (MAX_VIDEO_PACKET_QUEUE_SIZE * 2) / 3) {
            if (player->is_state(player, STATE_VIDEO_PACKET_QUEUE_FULL)) {
                player->clear_state(player, STATE_VIDEO_PACKET_QUEUE_FULL);
            }
        }
    } else if (stream->type == PACKET_STREAM_AUDIO) {
        if (stream->size(stream) == 0) {
            player->set_state(player, STATE_AUDIO_PACKET_QUEUE_EMPTY);
        } else if (stream->size(stream) <= (MAX_AUDIO_PACKET_QUEUE_SIZE * 2) / 3) {
            if (player->is_state(player, STATE_AUDIO_PACKET_QUEUE_FULL)) {
                player->clear_state(player, STATE_AUDIO_PACKET_QUEUE_FULL);
            }
        }
    } else {
    }

    return 0;
}

static size_t __size(PacketStream *stream)
{
    return stream->queue->size(stream->queue);
}

static size_t __set_maxsize(PacketStream *stream, int maxsize)
{
    stream->maxsize = maxsize;
}

static size_t __set_player(PacketStream *stream, void *player)
{
    stream->player = player;
}

static size_t __set_type(PacketStream *stream, void *type)
{
    stream->type = type;
}

static size_t __empty(PacketStream *stream)
{
    return stream->queue->is_empty(stream->queue);
}

static void __clear(PacketStream * stream) 
{
    void * element;

    dbg_str(DBG_WARN, "clear PacketStream, size=%d",
            stream->size(stream));

    while (!stream->empty(stream)) {
        stream->queue->remove(stream->queue, &element);
        if (element != NULL) {
            /*
             *dbg_str(DBG_WARN, "remove a packet");
             */
            av_packet_free((AVPacket **)&element);
        }
    }

}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","add_packet",__add_packet,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","remove_packet",__remove_packet,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","size",__size,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_maxsize",__set_maxsize,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","set_player",__set_player,sizeof(void *)},
    [10] = {ENTRY_TYPE_FUNC_POINTER,"","set_type",__set_type,sizeof(void *)},
    [11] = {ENTRY_TYPE_FUNC_POINTER,"","empty",__empty,sizeof(void *)},
    [12] = {ENTRY_TYPE_FUNC_POINTER,"","clear",__clear,sizeof(void *)},
    [13] = {ENTRY_TYPE_END},
};
REGISTER_CLASS(PacketStream,concurent_class_info);
