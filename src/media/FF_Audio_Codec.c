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
#include <libobject/media/FF_Audio_Codec.h>
#include <libobject/media/Player.h>
#include <libobject/media/FF_Extractor.h>
#include <libobject/media/AVSync.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>


#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

static int __construct(FF_Audio_Codec *codec,char *init_str)
{
    allocator_t *allocator = codec->parent.obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"codec construct, codec addr:%p",codec);
    codec->codec = NULL;

    return 0;
}

static int __deconstrcut(FF_Audio_Codec *codec)
{
    dbg_str(DBG_DETAIL,"codec deconstruct,codec addr:%p",codec);
    av_free(codec->out_buffer);

    return 0;
}

static int __set(FF_Audio_Codec *codec, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        codec->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        codec->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        codec->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        codec->deconstruct = value;
    } else if (strcmp(attrib, "thread_callback") == 0) {
        codec->thread_callback = value;
    } else if (strcmp(attrib, "update_texture") == 0) {
        codec->update_texture = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"codec set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FF_Audio_Codec *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"codec get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static void __priv_remove(void *element,void *p,void *arg) 
{
    Queue * queue        = p;
    AVFrame * avframe    = NULL;
    int64_t audio_clock  = 0;
    int64_t target_pos   = *(int64_t*)(arg);

    void * t;

    if (!queue->is_empty(queue)) {
        avframe = element;
        audio_clock = avframe->pts;
        if ( audio_clock <  target_pos ) {
            queue->remove_front(queue,(void **)&t);
        }
    }
}

static void *__thread_callback(void *arg)
{
    Codec *codec                = (Codec *)arg;
    Player *player              = (Player *)codec->player;
    FF_Audio_Codec *ff_codec    = (FF_Audio_Codec *)arg;
    AVFrame *frame              = av_frame_alloc();
    PacketStream *packet_stream = codec->audio_packet_stream;
    FrameStream *frame_stream   = codec->audio_frame_stream;
    allocator_t *allocator      = codec->obj.allocator;
    char *on_aac_codec_ready    = "on_audio_codec_ready";

    enum player_state_e conditions[10];
    enum player_state_e exceptions[10];
    int ret;
    static int i = 0;

    dbg_str(DBG_DETAIL,"audio thread callback run");

    conditions[0] = STATE_AUDIO_FRAME_QUEUE_FULL;
    conditions[1] = STATE_STOPPING;
    conditions[2] = STATE_STOPPED;
    exceptions[0] = STATE_EXIT;

    while (player->state_exit != 1) {
        AVPacket *pkt = NULL;

        usleep(10000);

        player->wait_if_true(player,
                conditions, 3,
                exceptions, 1,
                (int (*)(void *))codec->stop, codec,
                "audio_codec");

        player->state_audio_codec_stopped = 0;

        if (player->is_state(player, STATE_EXIT)) {
            break;
        }

        if (player->is_state(player, STATE_AUDIO_PACKET_QUEUE_EMPTY)) {
            usleep(10000);
            continue;
        }

        do {
            ret = avcodec_receive_frame(ff_codec->codec_ctx, frame);
            if (ret >= 0) {
                AVFrame *f = (AVFrame *) av_frame_alloc();
                frame->pts = i++;
                av_frame_move_ref(f, frame);
                frame_stream->add_frame(frame_stream, f);
            } else {
            }
            if (ret == AVERROR_EOF) {
                dbg_str(DBG_WARN,"audio codec EOF");
                return 0;
            }
        } while (ret != AVERROR(EAGAIN));

        packet_stream->remove_packet(packet_stream, (void **)&pkt);
        if (pkt != NULL) {
            if (avcodec_send_packet(ff_codec->codec_ctx, pkt) == AVERROR(EAGAIN)) {
                dbg_str(DBG_WARN,"Audio Codec send packet failed");
            }
            av_packet_free(&pkt);
        }
    }

    av_frame_free(&frame);
    dbg_str(DBG_INFO,"audio codec thread callback out");
}

static int __update_texture(Codec *codec, void *buffer)
{
    allocator_t *allocator      = codec->obj.allocator;
    FF_Audio_Codec *ff_codec    = (FF_Audio_Codec *)codec;
    FrameStream *frame_stream   = codec->audio_frame_stream;
    struct SwrContext *swr_ctx  = ff_codec->swr_ctx;
    AVCodecContext	*codec_ctx  = ff_codec->codec_ctx;
    uint8_t			*out_buffer = ff_codec->out_buffer;
    Player * player  = (Player *)codec->player;
    AVSync *sync = player->sync;
    AVFrame *frame = NULL;
    int size , ret;
    int out_channel_layout, out_sample_fmt, out_sample_rate;
    int out_channels;
    int out_nb_samples;
    enum player_state_e conditions[10];
    enum player_state_e exceptions[10];

    conditions[0] = STATE_STOPPING;
    conditions[1] = STATE_STOPPED;
    conditions[2] = STATE_LOADING;

    exceptions[0] = STATE_CLOSE;
    exceptions[1] = STATE_OPENING;
    exceptions[2] = STATE_EXIT;

    player->wait_if_true(player,
                         conditions, 3,
                         exceptions, 3,
                         (int (*)(void *))codec->stop, codec,
                         "audio_render");


    if (player->is_state(player, STATE_EXIT)) {
        dbg_str(DBG_DETAIL,"audio codec update_texture");
        return 0;
    }

    if (player->is_state(player, STATE_CLOSE)) {
        dbg_str(DBG_DETAIL,"audio codec update_texture, but state is close");
        return 0;
    }

    if (player->is_state(player, STATE_OPENING)) {
        dbg_str(DBG_DETAIL,"audio codec update_texture, but state is STATE_OPENING");
        return 0;
    }

    if (player->is_state(player, STATE_END)) {
        dbg_str(DBG_DETAIL,"audio codec update_texture, but playing is end");
        return 0;
    }

    if (frame_stream->size(frame_stream) == 0) {
        usleep(100000);
        return 0;
    }

    out_channel_layout = 3;
    out_sample_fmt     = 1;
    out_sample_rate    = codec->audio_freq;
    out_channels       = av_get_channel_layout_nb_channels(out_channel_layout);
    if (codec_ctx->frame_size == 0) {
        out_nb_samples  = 1024;
    } else {
        out_nb_samples  = codec_ctx->frame_size;
    }

    if (swr_ctx == NULL && (out_channel_layout != codec_ctx->channel_layout ||
                            out_sample_fmt != codec_ctx->sample_fmt ||
                            out_sample_rate != codec_ctx->sample_rate)) {
        swr_ctx = swr_alloc_set_opts(NULL,
                                     out_channel_layout,
                                     out_sample_fmt,
                                     out_sample_rate,
                                     codec_ctx->channel_layout,
                                     codec_ctx->sample_fmt,
                                     codec_ctx->sample_rate,
                                     0,
                                     NULL);
        if (swr_ctx != NULL) {
            ret = swr_init(swr_ctx);
            if (ret < 0) {
                dbg_str(DBG_ERROR, "swr_init error");
                return -1;
            }
            ff_codec->swr_ctx = swr_ctx;
        }

        if (!out_buffer) {
            out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
            ff_codec->out_buffer = out_buffer;
        }
    }

    dbg_str(DBG_INFO,"update audio texture, AUDIO frame size :%d  ",
            frame_stream->size(frame_stream));
    ret = frame_stream->remove_frame(frame_stream, (void **)&frame);

    if (frame != NULL) {
        if (frame->pkt_pts != AV_NOPTS_VALUE) {
            codec->cur_render_audio_clock = frame->pkt_pts * 
                                            av_q2d(codec->extractor->audio_stream->time_base);
            dbg_str(DBG_DETAIL, "codec->cur_render_audio_clock = %lf",
                    codec->cur_render_audio_clock);
            /*
             *dbg_str(DBG_ERROR, "xxxxxxframe->pkt_pts = %x frame->pts = %x\n",
             *        frame->pkt_pts, frame->pts);
             */
        }
    }

    if (frame != NULL) {
        size = av_samples_get_buffer_size(NULL,
                                          out_channels ,
                                          out_nb_samples,
                                          out_sample_fmt,
                                          1);
        dbg_str(DBG_DETAIL, "nb_sample=%d, out_buffer_size=%d, pts=%d, out_nb_samples =%d",
                frame->nb_samples, size, frame->pts, out_nb_samples);
        dbg_buf(DBG_DETAIL, "audio add data:", frame->data[0], 20);

        ret = swr_convert(swr_ctx, &out_buffer,
                          MAX_AUDIO_FRAME_SIZE,
                          (const uint8_t **)frame->data ,
                          frame->nb_samples);
        if (ret < 0) {
            dbg_str(DBG_ERROR, "swr_convert error");
            exit(1);
        }

        //Write PCM
        if (size > 0) {
            memcpy(buffer, out_buffer, size);
            dbg_buf(DBG_DETAIL, "audio data:", out_buffer, 20);
            ret = size;
        }
        av_frame_free(&frame);
    } else {
        dbg_str(DBG_WARN,"audio data queue is null");
    }

    return ret;
}

static int __seek(Codec *codec)
{
    Queue *audio_frame_queue   = NULL;
    Player *player             = (Player *)codec->player;
    FF_Extractor *ff_extractor = (FF_Extractor *)player->extractor;
    FrameStream *stream        = codec->audio_frame_stream;

    dbg_str(DBG_VIP, "seek audio codec");

    stream->clear(stream);
    avcodec_flush_buffers(ff_extractor->audio_codec_ctx);

    player->clear_state(player,STATE_AUDIO_FRAME_QUEUE_FULL);
    player->set_state(player,STATE_AUDIO_FRAME_QUEUE_EMPTY);

    return 0;
}

static int __open(Codec *codec)
{
    FF_Audio_Codec *ff_codec   = (FF_Audio_Codec *)codec;
    Extractor *extractor       = codec->extractor;
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    AVCodecContext	*codec_ctx = ff_extractor->audio_codec_ctx;
    Player *player             = (Player *)codec->player;
    AVCodec			*inner_codec;

    dbg_str(DBG_VIP,"open audio codec, codec_id=%d", 
            codec_ctx->codec_id);

    inner_codec = avcodec_find_decoder(codec_ctx->codec_id);
    if(inner_codec == NULL){
        dbg_str(DBG_ERROR,"audio Codec not found. codec_id=%d",
                codec_ctx->codec_id);
        return -1;
    }

    if(avcodec_open2(codec_ctx, inner_codec,NULL)<0){
        dbg_str(DBG_ERROR,"Could not open audio codec..");
        return -1;
    }

    ff_codec->codec          = inner_codec;
    ff_codec->codec_ctx      = ff_extractor->audio_codec_ctx;

    codec->audio_freq        = codec_ctx->sample_rate;
    codec->audio_channel_num = codec_ctx->channels;
    codec->audio_format      = codec_ctx->sample_fmt;

    player->set_state(player, STATE_AUDIO_CODEC_READY);

    dbg_str(DBG_DETAIL,"aac audio_codec_ctx=%p, audio codec id=%d, audio_freq=%d, audio_channel_num=%d, audio_format=%d",
            ff_codec->codec_ctx, inner_codec->id,
            codec->audio_freq, codec->audio_channel_num,
            codec->audio_format);

    return 0;
}

static int __close(Codec *codec)
{
    FF_Audio_Codec *ff_codec   = (FF_Audio_Codec *)codec;
    Extractor *extractor       = codec->extractor;
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    AVCodecContext	*codec_ctx = ff_extractor->audio_codec_ctx;
    struct SwrContext *swr_ctx = ff_codec->swr_ctx;
    Queue *audio_frame_queue   = NULL;
    FrameStream * stream       = codec->audio_frame_stream;
    Player *player             = (Player *)codec->player;

    dbg_str(DBG_VIP,"close audio codec");

    stream->clear(stream);
    player->clear_state(player,STATE_AUDIO_FRAME_QUEUE_FULL);
    player->clear_state(player,STATE_AUDIO_PACKET_LOADING);
    player->set_state(player,STATE_AUDIO_FRAME_QUEUE_EMPTY);

    /*
     *avcodec_flush_buffers(codec_ctx);
     */

    if (swr_ctx != NULL) {
        swr_free(&swr_ctx);
        ff_codec->swr_ctx = NULL;
    }
    if (codec_ctx != NULL) {
        avcodec_close(codec_ctx);
    }


    return 0;
}

static int __stop(Codec *codec)
{
    Player *player = (Player *)codec->player;

    player->state_audio_codec_stopped = 1;

    if (player->state_stopping == 1 &&
        player->state_extractor_stopped == 1 &&
        player->state_video_codec_stopped == 1 &&
        player->state_audio_codec_stopped == 1)
    {
        player->set_state(player, STATE_STOPPED);
    }

    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Codec","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","thread_callback",__thread_callback,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","update_texture",__update_texture,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","seek",__seek,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","open",__open,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","stop",__stop,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","close",__close,sizeof(void *)},
    [11] = {ENTRY_TYPE_END},
};
REGISTER_CLASS(FF_Audio_Codec,concurent_class_info);
