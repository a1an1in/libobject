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
#include <libobject/media/FF_Video_Codec.h>
#include <libobject/media/FF_Extractor.h>
#include <libobject/media/Player.h>
#include <libobject/media/AVSync.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>


static int __construct(FF_Video_Codec *codec,char *init_str)
{
    allocator_t *allocator = codec->parent.obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"codec construct, codec addr:%p",codec);
    codec->codec = NULL;
    struct SwsContext* sws_context = NULL;

    return 0;
}

static int __deconstrcut(FF_Video_Codec *codec)
{
    dbg_str(DBG_DETAIL,"codec deconstruct,codec addr:%p",codec);
    if (codec->out_frame != NULL) {
        av_free(codec->out_frame->data[0]);
        av_frame_free(&codec->out_frame);
        dbg_str(DBG_WARN,"codec deconstruct outframe");
    }

    if (codec->sws_context != NULL) {
        sws_freeContext(codec->sws_context);
        dbg_str(DBG_WARN,"codec deconstruct sws_context");
    }

    return 0;
}

static int __set(FF_Video_Codec *codec, char *attrib, void *value)
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
    } 
    else {
        dbg_str(DBG_DETAIL,"codec set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(FF_Video_Codec *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"codec get, \"%s\" getting attrib is not supported",
                attrib);
        return NULL;
    }
    return NULL;
}

static void __priv_remove(void *element,void *p,void *arg) 
{
    Queue * queue       = p;
    AVFrame * avframe   = NULL;
    int64_t video_clock = 0;
    int64_t target_pos  = *(int64_t*)(arg);

    void * t;

    if ( !queue->is_empty(queue) ) {
         avframe = element;
         video_clock = avframe->pts;
         if ( video_clock <  target_pos ) {
             queue->remove_front(queue,(void **)&t);
         }
    }
}

static void *__thread_callback(void *arg)
{
    Codec *codec = (Codec *)arg;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)arg;
    AVFrame *frame = av_frame_alloc();
    PacketStream *pck_stream = codec->video_packet_stream;
    allocator_t *allocator = codec->obj.allocator;
    Player * player  = (Player *)codec->player;
    enum player_state_e conditions[10];
    enum player_state_e exceptions[10];
    int ret;

    dbg_str(DBG_DETAIL,"video codec thread  run");

    conditions[0] = STATE_VIDEO_FRAME_QUEUE_FULL;
    conditions[1] = STATE_STOPPING;
    conditions[2] = STATE_STOPPED;
    exceptions[0] = STATE_EXIT;

    while (player->state_exit != 1) {
        AVPacket *pkt = NULL;
        usleep(10000);

        player->wait_if_true(player,
                             conditions,
                             3,
                             exceptions,
                             1,
                             (int (*)(void *))codec->stop, 
                             codec,
                             "video_codec");

        player->state_video_codec_stopped = 0;

        if (player->is_state(player, STATE_VIDEO_PACKET_QUEUE_EMPTY)) {
            usleep(10000);
            continue;
        }

        if (player->is_state(player, STATE_EXIT)) {
            break;
        }

        do {
            ret = avcodec_receive_frame(ff_codec->codec_ctx, frame);
            if (ret >= 0) {
                AVFrame *f = (AVFrame *) av_frame_alloc();
                FrameStream *frame_stream = codec->video_frame_stream;
                av_frame_move_ref(f, frame);
                frame_stream->add_frame(frame_stream, f);
            } else if (ret == AVERROR(EAGAIN)) {
                av_frame_unref(frame);
                dbg_str(DBG_DETAIL,"decode video frame error, ret =%d",
                        ret);
            }
            if (ret == AVERROR_EOF) {
                dbg_str(DBG_WARN,"video codec EOF");
                exit(1);
                return 0;
            }
        } while (ret != AVERROR(EAGAIN));

        pck_stream->remove_packet(pck_stream, (void **)&pkt);
        if (pkt != NULL) {
            ret = avcodec_send_packet(ff_codec->codec_ctx, pkt);
            if (ret == AVERROR(EAGAIN)) {
                dbg_str(DBG_WARN,"Video Codec send packet failed");

            } else {
                dbg_str(DBG_INFO,
                        "send video packet to avcodec, current queue size=%d, ret = %d",
                        pck_stream->size(pck_stream),
                        ret);
            }
            av_packet_free(&pkt);
        } else {
            dbg_str(DBG_WARN,"get video packet from queue err, queue size=%d",
                    pck_stream->size(pck_stream));
        }
    }
    dbg_str(DBG_INFO,"video codec thread callback out");
}
static int __sdl_update_yuvtexture(void *texture, AVFrame *frame)
{
    int ret;

    dbg_str(DBG_DETAIL,"frame width=%d, height=%d",
            frame->width, frame->height);

    if (frame->linesize[0] > 0 && 
        frame->linesize[1] > 0 && 
        frame->linesize[2] > 0) 
    {
        dbg_str(DBG_DETAIL,
                "linesize[0]=%d linesize[1]=%d linesize[2]=%d",
                frame->linesize[0],
                frame->linesize[1], 
                frame->linesize[2]);

        ret = SDL_UpdateYUVTexture(texture,
                                   NULL,
                                   frame->data[0],
                                   frame->linesize[0],
                                   frame->data[1],
                                   frame->linesize[1],
                                   frame->data[2],
                                   frame->linesize[2]);
        return 1;
    } else if (frame->linesize[0] < 0 && 
               frame->linesize[1] < 0 && 
               frame->linesize[2] < 0) 
    {
        dbg_str(DBG_DETAIL,
                "linesize[0]=%d linesize[1]=%d linesize[2]=%d",
                frame->linesize[0],
                frame->linesize[1],
                frame->linesize[2]);

        ret = SDL_UpdateYUVTexture(texture,
                                   NULL,
                                   frame->data[0] + frame->linesize[0] * (frame->height - 1),
                                   -frame->linesize[0],
                                   frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
                                   -frame->linesize[1],
                                   frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1),
                                   -frame->linesize[2]);
        return 1;
    } else {
        dbg_str(DBG_ERROR, "Mixed negative and positive linesizes are not supported.");
        ret = -1;
    }
}

static int 
__scale_yuv420p_frame(Codec *codec, AVFrame *src, 
                      AVFrame *dst, int dst_w, int dst_h)
{
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    Player * player  = (Player *)codec->player;
    struct rect_s *rect = &player->display_rect;
    struct SwsContext* sws_context;
    int ret = 0;

    if ((ff_codec->sws_context != NULL) && 
        player->is_state(player, STATE_SCREENSIZE_CHANGED))
    {
        dbg_str(DBG_WARN,"sws_getContext, STATE_SCREENSIZE_CHANGED");
        sws_freeContext(ff_codec->sws_context);
        ff_codec->sws_context = NULL;
    }

    if (ff_codec->sws_context == NULL) {
        sws_context = sws_getContext(src->width, 
                                     src->height, 
                                     AV_PIX_FMT_YUV420P,
                                     dst_w,
                                     dst_h,
                                     AV_PIX_FMT_YUV420P, 
                                     SWS_BICUBIC,
                                     NULL, NULL, NULL);
        ff_codec->sws_context= sws_context;

        dbg_str(DBG_WARN,"sws_getContext create");
    } else {
        sws_context = ff_codec->sws_context;
    }

    if (NULL == sws_context) {
        dbg_str(DBG_ERROR,"sws_getContext error");
		return -1;
	}

	ret = sws_scale(sws_context,
                    (const unsigned char *const *)src->data,
                    src->linesize,
                    0,
                    src->height,
                    dst->data,
                    dst->linesize);

    return ret;
}

static int __update_texture(Codec *codec, void *texture)
{
    AVFrame *frame = NULL;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    FrameStream *frame_stream = codec->video_frame_stream;
    allocator_t *allocator = codec->obj.allocator;
    Player * player  = (Player *)codec->player;
    AVSync *sync = player->sync;
    struct rect_s *rect = &player->display_rect;
    double frameRate = 0.0;
    uint64_t  pts    = AV_NOPTS_VALUE;
    int ret;

    if (player->is_state(player, STATE_EXIT)) {
        return 0;
    }

    if (player->is_state(player, STATE_END)) {
        dbg_str(DBG_DETAIL,"video codec update_texture, but playing is end");
        return 0;
    }

    if (frame_stream->size(frame_stream) == 0) {
        return 0;
    }

    dbg_str(DBG_INFO,"update video texture, video frame size :%d  ",
            frame_stream->size(frame_stream));

    frame_stream->remove_frame(frame_stream, (void **)&frame);

    if (frame == NULL) {
        dbg_str(DBG_DETAIL,"video frame == NULL ");
        return 0;
    } else  {
        AVFrame *out_frame = ff_codec->out_frame;

        if (out_frame != NULL && 
            player->is_state(player, STATE_SCREENSIZE_CHANGED)) {
            dbg_str(DBG_WARN,"realloc out frame");
            av_free(out_frame->data[0]);
            av_frame_free(&out_frame);
            ff_codec->out_frame = NULL;
        }

        if (ff_codec->out_frame == NULL) {
            out_frame = av_frame_alloc();
            out_frame->width = rect->width;
            out_frame->height = rect->height;

            av_image_alloc(out_frame->data, 
                           out_frame->linesize,
                           out_frame->width, 
                           out_frame->height, 
                           AV_PIX_FMT_YUV420P,
                           1);
            ff_codec->out_frame = out_frame;
        } else {
            out_frame = ff_codec->out_frame;
        }

        pts = av_frame_get_best_effort_timestamp(frame);
        if (pts == AV_NOPTS_VALUE ) {
            codec->cur_render_video_clock = 0.0;
        } else {
            codec->cur_render_video_clock = pts * 
                av_q2d(codec->extractor->video_stream->time_base);
        }

        sync->async_core(sync, frame, frame_stream->queue); // 同步调整

        ret = __scale_yuv420p_frame(codec, frame, out_frame,
                                    rect->width, rect->height);
        if (ret < 0) {
            dbg_str(DBG_ERROR,
                    "scale_yuv420p_frame, in_frame: width=%d, height=%d, out_frame:width=%d height=%d",
                    frame->width,
                    frame->height, 
                    out_frame->width,
                    out_frame->height);
            goto end;
        }
        ret = __sdl_update_yuvtexture(texture, out_frame);
end:
        av_frame_free(&frame);
    }

    return ret;
}

static int 
__update_texture_from_current_frame(Codec *codec, void *texture)
{
    AVFrame *frame = NULL;
    AVFrame *out_frame = NULL;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    Player * player  = (Player *)codec->player;
    struct rect_s *rect = &player->display_rect;
    struct SwsContext* sws_context;
    int ret;

    frame = ff_codec->out_frame;

    out_frame = av_frame_alloc();
    out_frame->width = rect->width;
    out_frame->height = rect->height;

    av_image_alloc(out_frame->data, 
                   out_frame->linesize,
                   out_frame->width, 
                   out_frame->height, 
                   AV_PIX_FMT_YUV420P, 
                   1);


    if ((ff_codec->sws_context != NULL) && 
        player->is_state(player, STATE_SCREENSIZE_CHANGED))
    {
        dbg_str(DBG_WARN,"sws_getContext, STATE_SCREENSIZE_CHANGED");
        sws_freeContext(ff_codec->sws_context);
        ff_codec->sws_context = NULL;
    }

    if (ff_codec->sws_context == NULL) {
        sws_context = sws_getContext(frame->width,
                                     frame->height, 
                                     AV_PIX_FMT_YUV420P,
                                     out_frame->width,
                                     out_frame->height,
                                     AV_PIX_FMT_YUV420P, 
                                     SWS_BICUBIC, 
                                     NULL, NULL, NULL);
        dbg_str(DBG_WARN,"sws_getContext create");
    }

	ret = sws_scale(sws_context,
                    (const unsigned char *const *)frame->data,
                    frame->linesize,
                    0,
                    frame->height,
                    out_frame->data, 
                    out_frame->linesize);


    ret = __sdl_update_yuvtexture(texture, out_frame);

    av_free(frame->data[0]);
    av_frame_free(&frame);
    sws_freeContext(sws_context);
    ff_codec->out_frame = out_frame;

end:
    return ret;
}

#if 0
static int __update_texture2(Codec *codec, void *texture)
{
    AVFrame *frame = NULL;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    FrameStream *frame_stream = codec->video_frame_stream;
    allocator_t *allocator = codec->obj.allocator;
    Player * player  = (Player *)codec->player;
    async_stream *sync = player->sync;
    struct rect_s *rect = &player->display_rect;
    double frameRate = 0.0;
    uint64_t  pts    = AV_NOPTS_VALUE;
    int ret;

    dbg_str(DBG_WARN,"update video texture");

    player->wait_if_false_with_except(player, 
                                      player->is_state,
                                      STATE_PLAYING,
                                      STATE_CLOSE,
                                      "video render");


    if (frame_stream->size(frame_stream) <= MAX_VIDEO_FRAME_QUEUE_SIZE / 3) {
        dbg_str(DBG_DETAIL,"clear STATE_VIDEO_FRAME_QUEUE_FULL");
        player->clear_state(player, STATE_VIDEO_FRAME_QUEUE_FULL);
    }

    if (frame_stream->size(frame_stream) == 0) {
        return 0;
    }

    if (player->is_state(player, STATE_EXIT)) {
        return 0;
    }

    dbg_str(DBG_SUC,"update video texture, video frame size :%d  ",
            frame_stream->size(frame_stream));

    frame_stream->remove_frame(frame_stream, &frame);

    if (frame == NULL) {
        dbg_str(DBG_DETAIL,"video frame == NULL ");
        return 0;
    } else  {
        pts = av_frame_get_best_effort_timestamp(frame);
        if ( pts == AV_NOPTS_VALUE ) {
            codec->cur_render_video_clock = 0.0;
        }else {
            codec->cur_render_video_clock = pts*av_q2d(codec->extractor->video_stream->time_base);
        }
        dbg_str(DBG_INFO, "codec->cur_render_video_clock= %lf",
                codec->cur_render_video_clock);

        sync->async_core(sync,frame, frame_stream->queue); // 同步调整



        static uint8_t  *dst_data[4];
        static int  dst_linesize[4];

        if (dst_data[0] != NULL && 
            player->is_state(player, STATE_SCREENSIZE_CHANGED))
        {
            av_free(dst_data[0]);
            dst_data[0] = NULL;
        }

        if (dst_data[0] == NULL) {
            ret = av_image_alloc(dst_data, dst_linesize,
                    rect->width, rect->height, 
                    AV_PIX_FMT_YUV420P, 1);
            if (ret < 0) {
                dbg_str(DBG_WARN,"av_image_alloc");
            }
        }

        I420Scale(frame->data[0], frame->width,
                  frame->data[1], frame->width >> 1,
                  frame->data[2], frame->width >> 1,
                  frame->width, frame->height,
                  dst_data[0], rect->width,
                  dst_data[1], rect->width >> 1,
                  dst_data[2], rect->width >> 1,
                  rect->width, rect->height, 0);
 

        if (dst_linesize[0] > 0 && 
            dst_linesize[1] > 0 &&
            dst_linesize[2] > 0) 
        {
            ret = SDL_UpdateYUVTexture(texture, NULL,
                                       dst_data[0], dst_linesize[0],
                                       dst_data[1], dst_linesize[1],
                                       dst_data[2], dst_linesize[2]);
            dbg_str(DBG_WARN,"SDL_UpdateYUVTexture, ret=%d", ret);
            ret = 1;
        } 

        /*
         *av_free(dst_data[0]);
         */
end:
        av_frame_free(&frame);
    }

    return ret;
}

#endif 

static int __get_rgb(Codec *codec,
                     enum AVPixelFormat fmt,
                     int target_width,
                     int target_height,
                     void *data, int *len)
{
    AVFrame *frame = NULL;
    AVFrame *out_frame = NULL;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    FrameStream *frame_stream = codec->video_frame_stream;
    Player * player  = (Player *)codec->player;
    AVSync *sync = player->sync;
    struct rect_s *rect = &player->display_rect;
    struct SwsContext* sws_context;
    uint8_t *dst_data[4] = {NULL};
    int dst_linesize[4] = {0};
    int dst_bufsize;
    uint64_t  pts = AV_NOPTS_VALUE;
    int ret;

    if (frame_stream->size(frame_stream) == 0) {
        dbg_str(DBG_DETAIL,"frame stream size = 0");
        return 0;
    }

    frame_stream->remove_frame(frame_stream, (void **)&frame);

    if (frame == NULL) {
        dbg_str(DBG_DETAIL,"video frame == NULL ");
        return 0;
    } else  {
    }

    if ((ret = av_image_alloc(dst_data, dst_linesize,
                              target_width, target_height,
                              fmt, 1)) < 0) {
        dbg_str(DBG_ERROR, "Could not allocate destination image");
        goto end;
    }

    dst_bufsize = ret;

    sws_context = sws_getContext(frame->width,
                                 frame->height, 
                                 AV_PIX_FMT_YUV420P,
                                 target_width,
                                 target_height,
                                 fmt, 
                                 SWS_FAST_BILINEAR, 
                                 NULL, NULL, NULL);
    if (sws_context == NULL) {
        ret = -1;
        goto err_get_context;
    }

    pts = av_frame_get_best_effort_timestamp(frame);
    if ( pts == AV_NOPTS_VALUE ) {
        codec->cur_render_video_clock = 0.0;
    } else {
        codec->cur_render_video_clock = pts * 
            av_q2d(codec->extractor->video_stream->time_base);
    }

    sync->async_core(sync,frame, frame_stream->queue); // 同步调整

	ret = sws_scale(sws_context,
                    (const unsigned char *const *)frame->data,
                    frame->linesize,
                    0,
                    frame->height,
                    dst_data, 
                    dst_linesize);
    if (ret < 0) {
        goto err_scale;
    }

    dbg_str(DBG_DETAIL, "dst_bufsize=%d",dst_bufsize);
    memcpy(data, dst_data[0], dst_bufsize);
    *len = dst_linesize[0];
    ret = dst_bufsize;

err_scale:
    sws_freeContext(sws_context);
err_get_context:
    av_freep(&dst_data[0]);
end:
    av_frame_free(&frame);

    return ret;
}

static int 
__get_rgb_from_current_frame(Codec *codec,
                             enum AVPixelFormat fmt,
                             int target_width,
                             int target_height,
                             void *data, int *len)
{
    AVFrame *frame = NULL;
    AVFrame *out_frame = NULL;
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    Player * player  = (Player *)codec->player;
    struct rect_s *rect = &player->display_rect;
    struct SwsContext* sws_context;
    uint8_t *dst_data[4] = {NULL};
    int dst_linesize[4] = {0};
    int dst_bufsize;
    int ret;

    frame = ff_codec->out_frame;

    if ((ret = av_image_alloc(dst_data, dst_linesize,
                              target_width, target_height,
                              fmt, 1)) < 0) {
        dbg_str(DBG_ERROR, "Could not allocate destination image");
        goto end;
    }

    dst_bufsize = ret;

    sws_context = sws_getContext(frame->width,
                                 frame->height, 
                                 AV_PIX_FMT_YUV420P,
                                 target_width,
                                 target_height,
                                 fmt, 
                                 SWS_FAST_BILINEAR, 
                                 NULL, NULL, NULL);
    if (sws_context == NULL) {
        ret = -1;
        goto err_get_context;
    }

	ret = sws_scale(sws_context,
                    (const unsigned char *const *)frame->data,
                    frame->linesize,
                    0,
                    frame->height,
                    dst_data, 
                    dst_linesize);
    if (ret < 0) {
        goto err_scale;
    }

    dbg_str(DBG_SUC, "dst_bufsize=%d",dst_bufsize);
    memcpy(data, dst_data[0], dst_linesize[0]);
    *len = dst_linesize[0];
    ret = dst_bufsize;

err_scale:
    sws_freeContext(sws_context);
err_get_context:
    av_freep(&dst_data[0]);
end:

    return ret;
}

static int __get_yuv(Codec *codec, void *data[], int len[])
{
}

static int __get_yuv_from_current_frame(Codec *codec, void *data[], int len[])
{
}

static int __seek(Codec *codec)
{
    Queue *video_frame_queue = NULL;
    Player *player  = (Player *)codec->player;
    Extractor *extractor = codec->extractor;
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    AVCodecContext	*codec_ctx = ff_extractor->video_codec_ctx;
    FrameStream * stream = codec->video_frame_stream;
     
    dbg_str(DBG_SUC, "seek video codec");

    stream->clear(stream);
    avcodec_flush_buffers(codec_ctx);

    player->clear_state(player,STATE_VIDEO_FRAME_QUEUE_FULL);
    player->set_state(player,STATE_VIDEO_FRAME_QUEUE_EMPTY);


    return 0;
}

static int __open(Codec *codec)
{
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    Extractor *extractor = codec->extractor;
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    AVCodecContext	*codec_ctx = ff_extractor->video_codec_ctx;
    Player *player  = (Player *)codec->player;
	AVCodec			*inner_codec;

    dbg_str(DBG_SUC,"open video codec, codec_id=%d",
            codec_ctx->codec_id);

	inner_codec = avcodec_find_decoder(codec_ctx->codec_id);
	if(inner_codec == NULL){
        dbg_str(DBG_ERROR,"Codec not found. codec_id=%d", codec_ctx->codec_id);
		return -1;
	}
	if(avcodec_open2(codec_ctx, inner_codec,NULL) < 0){
        dbg_str(DBG_ERROR,"Could not open codec..");
		return -1;
	}

    ff_codec->codec = inner_codec;
    ff_codec->codec_ctx = ff_extractor->video_codec_ctx;
    dbg_str(DBG_DETAIL,"h264 vidio codec_ctx=%p.", ff_codec->codec_ctx);
    dbg_str(DBG_DETAIL,"width=%d height=%d", 
            ff_codec->codec_ctx->width, ff_codec->codec_ctx->height);
    codec->width = ff_codec->codec_ctx->width;
    codec->height = ff_codec->codec_ctx->height;

    player->set_state(player, STATE_VIDEO_CODEC_READY);
    
    return 0;
}

static int __close(Codec *codec)
{
    FF_Video_Codec *ff_codec = (FF_Video_Codec *)codec;
    Extractor *extractor = codec->extractor;
    FF_Extractor *ff_extractor = (FF_Extractor *)extractor;
    AVCodecContext	*codec_ctx = ff_extractor->video_codec_ctx;
    Player *player  = (Player *)codec->player;
    FrameStream * stream = codec->video_frame_stream;
    AVFrame *out_frame = ff_codec->out_frame;
     
    dbg_str(DBG_SUC,"close video codec");

    stream->clear(stream);
    player->clear_state(player,STATE_VIDEO_FRAME_QUEUE_FULL);
    player->clear_state(player,STATE_VIDEO_FRAME_QUEUE_EMPTY);

    /*
     *avcodec_flush_buffers(codec_ctx);
     */

    if (out_frame != NULL)
    {
        av_free(out_frame->data[0]);
        av_frame_free(&out_frame);
        ff_codec->out_frame = NULL;
    }

    if (ff_codec->sws_context != NULL)
    {
        sws_freeContext(ff_codec->sws_context);
        ff_codec->sws_context = NULL;
    }

    if (codec_ctx != NULL) {
        avcodec_close(codec_ctx);
    }
     
    return 0;
}

static int __stop(Codec *codec)
{
    Player *player  = (Player *)codec->player;

    player->state_video_codec_stopped = 1;

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
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","update_texture_from_current_frame",__update_texture_from_current_frame,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_rgb",__get_rgb,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_rgb_from_current_frame",__get_rgb_from_current_frame,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","get_yuv",__get_yuv,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","get_yuv_from_current_frame",__get_yuv_from_current_frame,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","seek",__seek,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","open",__open,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","stop",__stop,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","close",__close,sizeof(void *)},
    [16] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("FF_Video_Codec",concurent_class_info);

