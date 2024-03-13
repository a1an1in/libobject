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
#include <libobject/message/Publisher.h>
#include <libobject/media/FF_Extractor.h>
#include <libobject/media/FF_Video_Codec.h>
#include <libobject/media/FF_Audio_Codec.h>
#include <libobject/media/Player.h>

static int __construct(FF_Extractor *extractor,char *init_str)
{
    allocator_t *allocator = extractor->parent.obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"extractor construct, extractor addr:%p",extractor);

    av_register_all();
    avformat_network_init();

    extractor->format_ctx      = NULL;
    extractor->video_codec_ctx = NULL;
    extractor->audio_codec_ctx = NULL;

    return 0;
}

static int __deconstrcut(FF_Extractor *extractor)
{
    dbg_str(DBG_DETAIL,"extractor deconstruct,extractor addr:%p",extractor);

    if (extractor->format_ctx != NULL) 
        avformat_close_input(&extractor->format_ctx);
    extractor->format_ctx = NULL;

    return 0;
}

static int __set(FF_Extractor *extractor, char *attrib, void *value)
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
    } else if (strcmp(attrib, "switch_bitrate") == 0) {
        extractor->switch_bitrate = value;
    } else {
        dbg_str(DBG_DETAIL,"extractor set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FF_Extractor *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"extractor get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}


static void *__thread_callback(void *arg)
{
    FF_Extractor *ff_extractor = NULL;
    Extractor *extractor = NULL;
    char *on_extrator_ready = "on_extrator_ready";
    char *on_extractor_seek_message = "on_extractor_seek_message";
    Publisher *publisher ;
    PacketStream *aps;
    PacketStream *vps;
    Player * player= NULL;
    Thread * thread = NULL;
    Thread * thread_videoc = NULL;
    Thread * thread_audioc = NULL;
    enum player_state_e conditions[10];
    enum player_state_e exceptions[10];
    AVPacket pkt1, *pkt = &pkt1;
    int i, ret;

    dbg_str(DBG_INFO,"extractor thread run");

    conditions[0] = STATE_AUDIO_PACKET_QUEUE_FULL;
    conditions[1] = STATE_VIDEO_PACKET_QUEUE_FULL;
    conditions[2] = STATE_STOPPING;
    conditions[3] = STATE_STOPPED;
    exceptions[0] = STATE_EXIT;

    ff_extractor  = (FF_Extractor *)arg;
    extractor     = (Extractor *)arg;
    publisher     = extractor->publisher;
    player        = (Player *)extractor->player;
    aps           = extractor->audio_packet_stream;
    vps           = extractor->video_packet_stream;
    thread        = extractor->thread ;
    thread_videoc = player->video_codec->thread;
    thread_audioc = player->audio_codec->thread;

    player->set_state(player, STATE_EXTRACTOR_READY);

    while (player->state_exit != 1) {
        usleep(1000);

        player->check_loading(player);

        player->wait_if_true(player,
                             conditions, 4,
                             exceptions, 1,
                             (int (*)(void *))extractor->stop, extractor,
                             "extractor");

        player->state_extractor_stopped = 0;

        if (player->is_state(player, STATE_EXIT)) {
            dbg_str(DBG_WARN,"exit extrator");
            break;
        }

        ret = av_read_frame(ff_extractor->format_ctx, pkt);
        if (ret < 0) {
            if ((ret == AVERROR_EOF || avio_feof(ff_extractor->format_ctx->pb))) {
                if (player->is_state(player,STATE_LOADING)) {
                    player->clear_state(player,STATE_LOADING);
                }

                player->set_state(player, STATE_EXTRACTOR_EOF);
                usleep(500000);
                dbg_str(DBG_WARN,"av_read_frame EOF.");
                /*
                 *return 0;
                 */
            } else {
                dbg_str(DBG_WARN,"av_read_frame error.");
            }
            continue;
        } else {
            dbg_str(DBG_DETAIL,"read a packet.");
            player->clear_state(player, STATE_EXTRACTOR_EOF);
        }

        if (pkt->stream_index == extractor->video_stream_index) {
            AVPacket *packet = (AVPacket *)av_packet_alloc();
            if (pkt == NULL) {
                dbg_str(DBG_ERROR,"allocator_mem_alloc error.");
                return ;
            }

            av_packet_move_ref(packet, pkt);

            //check isvalid packet
            if ( packet->pts < extractor->seek_timestamp )  {
                av_packet_unref(packet);
                continue;
            }
            vps->add_packet(vps, packet);

            player->video_buffered_time = packet->pts
                * av_q2d(extractor->video_stream->time_base);

        } else if (pkt->stream_index == extractor->audio_stream_index) {
            AVPacket *packet = (AVPacket *)av_packet_alloc();
            if (packet == NULL) {
                dbg_str(DBG_ERROR,"av_packet_alloc error.");
                return ;
            }

            av_packet_move_ref(packet, pkt);

            if ( packet->pts < extractor->seek_timestamp )  {
                av_packet_unref(packet);
                continue;
            }
            aps->add_packet(aps, packet);

            player->audio_buffered_time = packet->pts 
                * av_q2d(extractor->audio_stream->time_base);
        } else {
        }
    }

    /*
     *thread->join(thread,thread_videoc);
     *thread->join(thread,thread_audioc);
     */

    dbg_str(DBG_INFO,"extractor thread callback out");
    return NULL;
}


static int __seek(Extractor *extractor)
{
    int ret ;
    Player * player            = NULL;
    FF_Extractor *ff_extractor = NULL;
    PacketStream *video_stream = NULL;
    PacketStream *audio_stream = NULL;
    ff_extractor               = (FF_Extractor *)extractor;
    player                     = (Player *)extractor->player;
    int64_t  seek_timestamp;
    int64_t duration;

    duration = player->get_duration(player);
    dbg_str(DBG_SUC,"seek to %lld, duration=%lld",
            player->seek_timestamp,
            duration);
    if (player->seek_timestamp > duration || player->seek_timestamp < 0) {
        return ret;
    }

    video_stream = extractor->video_packet_stream;
    audio_stream = extractor->audio_packet_stream;

    video_stream->clear(video_stream);
    audio_stream->clear(audio_stream);
    player->clear_state(player, STATE_VIDEO_PACKET_QUEUE_FULL);
    player->clear_state(player, STATE_AUDIO_PACKET_QUEUE_FULL);

    if (extractor->audio_stream_index != -1) { 
#if 1
        extractor->seek_timestamp = av_rescale_q(
                player->seek_timestamp * AV_TIME_BASE,
                AV_TIME_BASE_Q,
                extractor->audio_stream->time_base);

        ret = av_seek_frame(ff_extractor->format_ctx,
                extractor->audio_stream_index,
                extractor->seek_timestamp,
                0);
#else
        extractor->seek_timestamp = (int64_t)((float)player->seek_timestamp/av_q2d(extractor->audio_stream->time_base));
        uint64_t duration_timestamp = (int64_t)(duration/av_q2d(extractor->audio_stream->time_base));
        dbg_str(DBG_SUC,"timebase den=%d num=%d", extractor->audio_stream->time_base.den, extractor->audio_stream->time_base.num);
        dbg_str(DBG_SUC,"seek timestamp=%lld, duration_timestamp=%lld", 
                extractor->seek_timestamp,
                duration_timestamp);
        dbg_str(DBG_SUC,"seek timestamp=%d, duration_timestamp=%d", 
                extractor->seek_timestamp,
                duration_timestamp);

        /*
         *ret = avformat_seek_file(ff_extractor->format_ctx, 
         *                         extractor->video_stream_index,
         *                         extractor->seek_timestamp, 
         *                         extractor->seek_timestamp, 
         *                         extractor->seek_timestamp, 
         *                         AVSEEK_FLAG_FRAME);
         */
        ret = avformat_seek_file(ff_extractor->format_ctx, 
                extractor->video_stream_index,
                extractor->seek_timestamp, 
                extractor->seek_timestamp, 
                extractor->seek_timestamp, 
                AVSEEK_FLAG_FRAME);
#endif
        if ( ret < 0 ) {
            dbg_str(DBG_ERROR,"seek audio failed!");
            goto end;
        }

        dbg_str(DBG_ERROR,"audio stream seek_timestamp:%lld ret:%d",
                extractor->seek_timestamp,ret);
    } else {
        dbg_str(DBG_ERROR,"seek async error audio failed!");
    }

    if (extractor->video_stream_index != -1) {
#if 1
        extractor->seek_timestamp = av_rescale_q(
                player->seek_timestamp * AV_TIME_BASE,
                AV_TIME_BASE_Q,
                extractor->video_stream->time_base);

        ret = av_seek_frame(ff_extractor->format_ctx, 
                extractor->video_stream_index,
                extractor->seek_timestamp,
                0);
#else
        extractor->seek_timestamp = (int64_t)((float)player->seek_timestamp / av_q2d(extractor->video_stream->time_base));
        ret = avformat_seek_file(ff_extractor->format_ctx, 
                extractor->video_stream_index,
                extractor->seek_timestamp, 
                extractor->seek_timestamp, 
                extractor->seek_timestamp, 
                AVSEEK_FLAG_FRAME);
#endif
        if (ret < 0) {
            dbg_str(DBG_ERROR,"seek video failed!");
        }

        dbg_str(DBG_ERROR,"video stream seek_timestamp:%lld ret:%d",
                extractor->seek_timestamp,ret);
    } else {
        dbg_str(DBG_ERROR,"seek async error video failed!");
    }


end:
    extractor->seek_timestamp = 0;
    return ret;
}

static int __open(Extractor *extractor)
{
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    allocator_t *allocator     = extractor->obj.allocator;
    DataSource *data_source    = extractor->data_source;
    MediaPeriod *media_period  = data_source->media_peroid;
    Player * player            = extractor->player;
    AVFormatContext	*format_ctx;
    AVCodecContext	*codec_ctx;
    int i, ret;
    char *url;

    url = media_period->get_url(media_period);
    dbg_str(DBG_SUC,"open extractor, url:%s", url);

    format_ctx = avformat_alloc_context();

    if ( avformat_open_input(&format_ctx,url,NULL,NULL) < 0) {
        dbg_str(DBG_ERROR,"Couldn't open input stream, url:%s", url);
        return -1;
    }
    if ( avformat_find_stream_info(format_ctx,NULL) < 0 ) {
        dbg_str(DBG_ERROR,"Couldn't find stream information.");
        return -1;
    }

    extractor->video_stream_index = -1;
    extractor->audio_stream_index = -1;
    extractor->subtile_stream_index = -1;

    for (i = 0; i<format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            extractor->video_stream_index =i;
            break;
        }
    }

    for (i = 0; i<format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            extractor->audio_stream_index = i;
            break;
        } 
    }

    if (extractor->video_stream_index == -1) {
        dbg_str(DBG_WARN,"Didn't find a video stream.");
        return -1;
    } else {
        ff_extractor->video_codec_ctx = format_ctx->streams[extractor->video_stream_index]->codec;
        extractor->video_stream = format_ctx->streams[extractor->video_stream_index];
        dbg_str(DBG_WARN,"video_codec_ctx=%p.", ff_extractor->video_codec_ctx);
    }

    if (extractor->audio_stream_index == -1) {
        dbg_str(DBG_WARN,"Didn't find a audio stream.");
        return -1;
    } else {
        ff_extractor->audio_codec_ctx = format_ctx->streams[extractor->audio_stream_index]->codec;
        extractor->audio_stream = format_ctx->streams[extractor->audio_stream_index];
        dbg_str(DBG_DETAIL,"audio_codec_ctx=%p.", ff_extractor->audio_codec_ctx);
    }

    dbg_str(DBG_SUC,"video_codec_id=%d audio_codec_id=%d.",
            ff_extractor->video_codec_ctx->codec_id,
            ff_extractor->audio_codec_ctx->codec_id);

    dbg_str(DBG_SUC,"video_stream_index=%d audio_stream_index=%d.",
            extractor->video_stream_index,
            extractor->audio_stream_index);

    dbg_str(DBG_SUC,"video time_base: %lf  audio time_base: %lf",
            av_q2d(extractor->video_stream->time_base),
            av_q2d(extractor->audio_stream->time_base));

    if (extractor->video_stream->start_time == AV_NOPTS_VALUE) {
        extractor->video_stream->start_time = 0;
    }

    if (extractor->audio_stream->start_time == AV_NOPTS_VALUE) {
        extractor->audio_stream->start_time = 0;
    }

    dbg_str(DBG_SUC,"video start_time: %lf  audio start_time: %lf",
            (extractor->video_stream->start_time) 
            * av_q2d(extractor->audio_stream->time_base),
            (extractor->audio_stream->start_time)
            * av_q2d(extractor->audio_stream->time_base));

    ff_extractor->format_ctx = format_ctx;

    dbg_str(DBG_SUC,"open extractor out");

    return 0;
}

static int __close(Extractor *extractor)
{
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    PacketStream *video_stream = NULL;
    PacketStream *audio_stream = NULL;
    Player * player = extractor->player;

    dbg_str(DBG_SUC,"close extractor");
    video_stream = extractor->video_packet_stream;
    audio_stream = extractor->audio_packet_stream;

    if (ff_extractor->format_ctx != NULL) {
        avformat_close_input(&ff_extractor->format_ctx);
        ff_extractor->format_ctx = NULL;
    }

    video_stream->clear(video_stream);
    audio_stream->clear(audio_stream);
    player->clear_state(player, STATE_VIDEO_PACKET_QUEUE_FULL);
    player->clear_state(player, STATE_AUDIO_PACKET_QUEUE_FULL);


    return 0;
}

static int __stop(Extractor *extractor)
{
    Player * player = extractor->player;

    player->state_extractor_stopped = 1;

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
    [0 ] = {ENTRY_TYPE_OBJ,"Extractor","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","thread_callback",__thread_callback,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","seek",__seek,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","open",__open,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","stop",__stop,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","close",__close,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","switch_bitrate",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("FF_Extractor",concurent_class_info);
