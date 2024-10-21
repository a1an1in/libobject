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
#include <libobject/core/Thread.h>
#include <libobject/ui/Sdl_Window.h>
#include <libobject/ui/Sdl_Image.h>
#include <libobject/ui/Sdl_Audio.h>
#include <libobject/event/event_compat.h>
#include <libobject/media/Player.h>
#include <libobject/media/FF_Extractor.h>
#include <libobject/media/FF_Video_Codec.h>
#include <libobject/media/FF_Audio_Codec.h>
#include <libobject/media/FF_Media_Source.h>
#include <libobject/media/FF_Window.h>
#include <libobject/media/FF_Event.h>
#include <libobject/media/message.h>


static int __clear_player_state(Player *player);

static void
out_event_callback(int fd, short event_res, void *arg)
{
    Player *player = (Player *)arg;

    dbg_str(DBG_WARN,"out event callback");
    player->out_flag = 1;
    player->extractor->out_flag = 1;
    player->video_codec->out_flag = 1;
    player->set_state(player, STATE_EXIT);
    player->state_exit = 1;

}

static int __construct(Player *player,char *init_str)
{
    allocator_t *allocator = player->obj.allocator;
    configurator_t * c;
    char buf[2048];
    struct event_base* base = event_base_get_default_instance();

    dbg_str(DBG_VIP,"construct player in");

    /* Initalize one event */
    event_assign(&player->out_event, base, SIGQUIT, EV_SIGNAL|EV_PERSIST,
                 out_event_callback, player);

    event_add(&player->out_event, NULL);

    player->out_flag         = 0;
    player->stoped           = 1;
    player->seeking          = 1;
    player->disable_subtitle = 1;
    player->disable_audio    = 1;
    player->disable_video    = 0;

    player->message_centor   = OBJECT_NEW(allocator, Centor, NULL);
    player->publisher        = OBJECT_NEW(allocator, Publisher, NULL);
    player->subscriber_queue = OBJECT_NEW(allocator, Linked_Queue, NULL);

    player->media_period     = OBJECT_NEW(allocator, MediaPeriod, NULL);
    player->data_source      = OBJECT_NEW(allocator, DataSource, NULL);
    player->extractor        = OBJECT_NEW(allocator, FF_Extractor, NULL);
    player->video_codec      = OBJECT_NEW(allocator, FF_Video_Codec, NULL);
    player->audio_codec      = OBJECT_NEW(allocator, FF_Audio_Codec, NULL);
    player->media_source     = OBJECT_NEW(allocator, FF_Media_Source,NULL);
    player->thread           = OBJECT_NEW(allocator, Thread, NULL);
    player->sync             = OBJECT_NEW(allocator, AVSync,NULL);
    player->streamfile       = OBJECT_NEW(allocator, String,NULL);
    player->switch_flag      = 0;
    pthread_mutex_init( &player->mutex, NULL);
    pthread_cond_init( &player->condition, NULL);

    __clear_player_state(player);

    dbg_str(DBG_VIP,"construct player out");
    return 0;
}

static int __deconstrcut(Player *player)
{
    Image *image = player->image;
    Component *c = (Component *)image;
    Render *r = c->render;

    dbg_str(DBG_VIP,"deconstruct player in");

    if (player->texture) {
        r->destroy_texture(r, player->texture);
    }

    object_destroy(player->message_centor);
    object_destroy(player->publisher);
    object_destroy(player->subscriber_queue);

    object_destroy(player->media_period);
    object_destroy(player->data_source);
    object_destroy(player->extractor);
    object_destroy(player->video_codec);
    object_destroy(player->audio_codec);
    object_destroy(player->media_source);

    object_destroy(player->thread);
    object_destroy(player->sync);
    
    object_destroy(player->streamfile);
    pthread_mutex_destroy(&player->mutex );
    pthread_cond_destroy(&player->condition );

    dbg_str(DBG_VIP,"deconstruct player out");
    return 0;
}

static int __set(Player *player, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        player->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        player->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        player->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        player->deconstruct = value;
    } else if (strcmp(attrib, "init") == 0) {
        player->init = value;
    } else if (strcmp(attrib, "start") == 0) {
        player->start = value;
    } else if (strcmp(attrib, "stop") == 0) {
        player->stop = value;
    } else if (strcmp(attrib, "exit") == 0) {
        player->exit = value;
    } else if (strcmp(attrib, "seek_to") == 0) {
        player->seek_to = value;
    } else if (strcmp(attrib, "get_duration") == 0) {
        player->get_duration = value;
    } else if (strcmp(attrib, "get_position") == 0) {
        player->get_position = value;
    } else if (strcmp(attrib, "get_buffered_position") == 0) {
        player->get_buffered_position = value;
    } else if (strcmp(attrib, "get_buffered_duration") == 0) {
        player->get_buffered_duration = value;
    } else if (strcmp(attrib, "add_subscriber") == 0) {
        player->add_subscriber = value;
    } else if (strcmp(attrib, "del_subscriber") == 0) {
        player->del_subscriber = value;
    } else if (strcmp(attrib, "run") == 0) {
        player->run = value;
    } else if (strcmp(attrib, "switch_bitrate") == 0) {
        player->switch_bitrate = value;
    } else if (strcmp(attrib, "wait_if_true") == 0) {
        player->wait_if_true = value;
    } else if (strcmp(attrib, "broadcast") == 0) {
        player->broadcast = value;
    } else if (strcmp(attrib, "signal") == 0) {
        player->signal = value;
    } else if (strcmp(attrib, "is_state") == 0) {
        player->is_state = value;
    } else if (strcmp(attrib, "set_state") == 0) {
        player->set_state = value;
    } else if (strcmp(attrib, "clear_state") == 0) {
        player->clear_state = value;
    } else if (strcmp(attrib, "state_to_str") == 0) {
        player->state_to_str = value;
    } else if (strcmp(attrib, "add_image") == 0) {
        player->add_image = value;
    } else if (strcmp(attrib, "add_audio") == 0) {
        player->add_audio = value;
    } else if (strcmp(attrib, "draw_image") == 0) {
        player->draw_image = value;
    } else if (strcmp(attrib, "set_media_url") == 0) {
        player->set_media_url = value;
    } else if (strcmp(attrib, "get_media_url") == 0) {
        player->get_media_url = value;
    } else if (strcmp(attrib, "draw_image") == 0) {
        player->draw_image = value;
    } else if (strcmp(attrib, "draw_current_image") == 0) {
        player->draw_current_image = value;
    } else if (strcmp(attrib, "get_rgb") == 0) {
        player->get_rgb = value;
    } else if (strcmp(attrib, "get_suitable_rgb_of_screen") == 0) {
        player->get_suitable_rgb_of_screen = value;
    } else if (strcmp(attrib, "get_rgb_from_current_frame") == 0) {
        player->get_rgb_from_current_frame = value;
    } else if (strcmp(attrib, "get_yuv") == 0) {
        player->get_yuv = value;
    } else if (strcmp(attrib, "get_yuv_from_current_frame") == 0) {
        player->get_yuv_from_current_frame = value;
    } else if (strcmp(attrib, "publish_message") == 0) {
        player->publish_message = value;
    } else if (strcmp(attrib, "open") == 0) {
        player->open = value;
    } else if (strcmp(attrib, "close") == 0) {
        player->close = value;
    } else if (strcmp(attrib, "print") == 0) {
        player->print = value;
    } else if (strcmp(attrib, "check_loading") == 0) {
        player->check_loading = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"player set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Player *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARN,"player get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __audio_callback(void *opaque, void *buffer)
{

    int buffersize = 0;
    Player *player = (Player *)opaque;
    Codec *audio_codec = player->audio_codec;

    buffersize = audio_codec->update_texture(audio_codec, buffer);

    return  buffersize;
}

static int __null_audio_callback(void *opaque, void *buffer)
{
    return  0;
}

static void __open_audio_device(Player *player)
{
    Audio *audio = player->audio;
    Codec *audio_codec = player->audio_codec;

    dbg_str(DBG_VIP,
            "open audio device: freq=%d, format=%d, channel_num=%d",
            audio_codec->audio_freq,
            audio_codec->audio_format,
            audio_codec->audio_channel_num);

    audio->set_freq(audio, audio_codec->audio_freq);
    audio->set_format(audio, audio_codec->audio_format);
    audio->set_channels(audio, audio_codec->audio_channel_num);
    audio->set_callback(audio, __audio_callback);
    audio->set_callback_opaque(audio, player);
    audio->init(audio);
    audio->play(audio);
}

static void __close_audio_device(Player *player)
{
    Audio *audio = player->audio;

    /*
     *audio->set_callback(audio, __null_audio_callback);
     */
    audio->pause(audio);
    audio->close(audio);
}

static int __set_media_url(Player *self,char *url)
{
    self->streamfile->assign(self->streamfile,url);
    return 1;
}

static char * __get_media_url(Player *self)
{
    return self->streamfile->get_cstr(self->streamfile);
}

static int __open_inner(Player *player, char *url)
{
    Extractor *extractor       = player->extractor;
    Codec *video_codec         = player->video_codec;
    Codec *audio_codec         = player->audio_codec;
    MediaPeriod *media_period  = player->media_period;
    DataSource *data_source    = player->data_source;
    int ret = 0;
    
    if (player->is_state(player,STATE_INITED)) {
        dbg_str(DBG_WARN,"close input");
        while (!player->is_state(player, STATE_STOPPED)) {
            usleep(40000);
        } 

        player->close(player);
    }

    media_period->set_url(player->media_period,url);

    data_source->set(data_source,"media_peroid", media_period);
    extractor->set_data_source(extractor, data_source);
    extractor->set_player(extractor, player);

    ret = extractor->open(extractor);
    if (ret < 0) {
        return ret;
    }

    ret = video_codec->open(video_codec);
    if (ret < 0) {
        return ret;
    }

    ret = audio_codec->open(audio_codec);
    if (ret < 0) {
        return ret;
    }
    
    __open_audio_device(player);

    player->set_state(player,STATE_INITED);
    return ret;
}

static int __open(Player *player, char *url)
{
    Media_Source *media_source = player->media_source;
    int ret = 0;

    if (player->is_state(player, STATE_OPENING)) {
        return -1;
    }

    player->set_state(player, STATE_OPENING);

    player->set_media_url(player, url);
    media_source->init(media_source, url);
    url = media_source->get_url(media_source);

    dbg_str(DBG_VIP,"open url:%s",url);

    player->sync->clear(player->sync);

    ret = __open_inner(player, url);

    player->set_state(player, STATE_OPENED);

    return ret;
}

static int __close(Player *player)
{
    Extractor *extractor       = player->extractor;
    Codec *video_codec         = player->video_codec;
    Codec *audio_codec         = player->audio_codec;
    
    dbg_str(DBG_VIP,"close player in 1");

    player->set_state(player,STATE_CLOSE);

    __close_audio_device(player);

    video_codec->close(video_codec);
    audio_codec->close(audio_codec);
    extractor->close(extractor);

    player->clear_state(player,STATE_END);
    player->clear_state(player,STATE_CLOSE);
    player->clear_state(player,STATE_LOADING);
    
    dbg_str(DBG_VIP,"close player out");

    return 0;
}

static int __init(Player *player)
{
    Extractor *extractor = player->extractor;
    Codec *video_codec   = player->video_codec;
    Codec *audio_codec   = player->audio_codec;
    Audio *audio         = player->audio;
    AVSync *sync         = player->sync;
    PacketStream *vps    = extractor->video_packet_stream;
    PacketStream *aps    = extractor->audio_packet_stream;
    Media_Source *media_source = player->media_source;
    char *url;

    dbg_str(DBG_VIP,"init player in");

    video_codec->set(video_codec, "extractor", extractor);
    video_codec->set_player(video_codec, player);
    video_codec->set_video_packet_stream(video_codec, vps);
    vps->set_maxsize(vps, MAX_VIDEO_PACKET_QUEUE_SIZE);
    vps->set_player(vps, player);
    vps->set_type(vps, PACKET_STREAM_VIDEO);

    audio_codec->set(audio_codec, "extractor", extractor);
    audio_codec->set_player(audio_codec, player);
    audio_codec->set_audio_packet_stream(audio_codec, aps);
    aps->set_maxsize(aps, MAX_AUDIO_PACKET_QUEUE_SIZE);
    aps->set_player(aps, player);
    aps->set_type(aps, PACKET_STREAM_AUDIO);

    sync->set_video_codec(sync,video_codec);
    sync->set_audio_codec(sync,audio_codec);

    player->publisher->connect_centor(player->publisher, 
                                      player->message_centor);

    /*
     *player->set_state(player,STATE_STOPPING);
     */

    dbg_str(DBG_VIP,"init player out");

    return 0;
}

static int __run(Player *player)
{
    Extractor *extractor = player->extractor;
    Codec *video_codec = player->video_codec;
    Codec *audio_codec = player->audio_codec;

    extractor->run(extractor);
    video_codec->run(video_codec);
    audio_codec->run(audio_codec);
    player->start(player);
    player->set_state(player,STATE_RUNNED);

    return 0;
}


static int __start(Player *player)
{
    int ret = 0;

    ret = player->clear_state(player,STATE_STOPPING);
    ret = player->clear_state(player,STATE_STOPPED);

    if (player->is_state(player, STATE_END)) {
        return -1;
    }

    while (player->is_state(player, STATE_LOADING)) {
        usleep(40000);
    } 

    ret = player->set_state(player,STATE_PLAYING);

    return ret;
}

static int __stop(Player *player)
{
   int ret;
   ret = player->set_state(player,STATE_STOPPING);
   return 0;
}

static int __exit(Player *player)
{
    dbg_str(DBG_VIP,"exit player");

    player->set_state(player, STATE_CLOSE);
    player->set_state(player, STATE_EXIT);
    return 0;
}

static int __add_image(Player *player, Image *image)
{
    player->image = image;
    player->screen_width  = image->width;
    player->screen_height = image->height;
    return 0;
}

static int __add_audio(Player *player, Audio *audio)
{
    player->audio = audio;
    return 0;
}

static int __seek_to(Player *player, int position)
{
    int ret = 0;
    Extractor *extractor = player->extractor;
    Codec *video_codec   = player->video_codec;
    Codec *audio_codec   = player->audio_codec;
    int64_t duration;

    if (player->is_state(player, STATE_SEEKING)) {
        return -1;
    }

    duration = player->get_duration(player);
    if (position >= duration) {
        return -1;
    }

    ret = player->set_state(player,STATE_SEEKING);

    usleep(500000);

    dbg_str(DBG_VIP,"seek to %d", position);
    player->seek_timestamp = (int64_t)position; 

    player->sync->clear(player->sync);

    ret = extractor->seek(extractor);
    if (ret < 0) {
        return ret;
    }

    ret = video_codec->seek(video_codec);
    if (ret < 0) {
        return ret;
    }

    ret = audio_codec->seek(audio_codec);
    if (ret < 0) {
        return ret;
    }
    usleep(500000);

    ret = player->set_state(player,STATE_SEEKED);
    player->seek_timestamp = 0;

    return ret;
}

static int __seek_to_inner(Player *player, int position)
{
    int ret = 0;
    Extractor *extractor = player->extractor;
    Codec *video_codec   = player->video_codec;
    Codec *audio_codec   = player->audio_codec;

    player->seek_timestamp = (int64_t)position; 

    extractor->seek(extractor);
    video_codec->seek(video_codec);
    audio_codec->seek(audio_codec);

    player->seek_timestamp = 0;

    return ret;
}

static int __switch_bitrate(Player * player,int bitrate) 
{
    int64_t audio_current_timestamp;
    int64_t video_current_timestamp;
    int64_t current_timestamp;
    Extractor *extractor = player->extractor;
    AVSync *sync = player->sync;
    AVRational time_base = extractor->audio_stream->time_base;;
    MediaPeriod *media_period = player->media_period;
    DataSource *data_source = player->data_source;
    Media_Source *media_source = player->media_source;
    int ret = 0;
    char *url;

    ret = player->media_source->isvalid_bitrate(player->media_source,
                                                bitrate);
    if ( ! ret ) {
        dbg_str(DBG_ERROR,"unvalid bitrate of stream file!");
        ret = -1;
        return ret;
    }
    player->set_state(player,STATE_STOPPING);
    player->set_state(player,STATE_SWITCH);
    player->current_bitrate = bitrate;
    
    current_timestamp = (int64_t)(sync->get_audio_clock(sync))/av_q2d(time_base);

    url = media_source->get_url_by_bitrate(media_source,bitrate);
    
    current_timestamp = av_rescale_q(current_timestamp,
            extractor->audio_stream->time_base,
            AV_TIME_BASE_Q);

    __open_inner(player, url);

    sleep(1);
    __seek_to_inner(player, current_timestamp);
        
    player->clear_state(player,STATE_SWITCH);
    player->clear_state(player,STATE_STOPPING);
    extractor->seek_timestamp = 0;

    return ret;
}


static int64_t __get_duration(Player *player)
{
   int64_t duration = 0;
   FF_Extractor * extractor = (FF_Extractor *)player->extractor;
   AVFormatContext	*format_ctx = extractor->format_ctx;

   if ( format_ctx != NULL ) {
       if (format_ctx->duration != AV_NOPTS_VALUE ) {
          duration = (format_ctx->duration + 5000)/AV_TIME_BASE;
       }else {
           dbg_str(DBG_ERROR,"get player duration failed!");
       }
   }

   return duration;
}


static double __get_position(Player *player)
{
    double current_timestamp;
    Extractor * extractor = player->extractor;
    AVSync *sync = player->sync;
   
    if (player->is_state(player, STATE_SEEKING)) {
        dbg_str(DBG_WARN,"get position, but player is seeking");
        return player->seek_timestamp;
    }

    if ( extractor != NULL && player != NULL) {
        current_timestamp = sync->get_video_clock(sync);
        dbg_str(DBG_DETAIL,"get position1:%lf", current_timestamp);
         
    } else {
        current_timestamp = 0.0;
    }

    return  current_timestamp ;
}

static double __get_buffered_position(Player *player)
{
    double abt, vbt;

    vbt = player->video_buffered_time;
    abt = player->audio_buffered_time; 

    return vbt > abt ? abt: vbt;
}

static double __get_buffered_duration(Player *player)
{
    double current_time;
    double buffered_time;
    double buffered_duration;

    current_time = player->get_position(player);
    buffered_time = player->get_buffered_position(player);

    buffered_duration = buffered_time - current_time;

    return buffered_duration;
}

static int __add_subscriber(Player *player, Subscriber *subscriber)
{
    subscriber->connect_centor(subscriber, player->message_centor);
    subscriber->subscribe(subscriber, player->publisher);

    return 0;
}

static int __del_subscriber(Player *player, Subscriber *subscriber)
{
}

static int __publish_message(Player *player, char *message)
{
    player->publisher->publish_message(player->publisher,
                                       message, strlen(message));
}

static void 
__calculate_display_rect(struct rect_s *rect,
                       int win_x, int win_y,
                       int win_width, int win_height,
                       int pic_width, int pic_height)
{
    float aspect_ratio = 1.0;
    int width, height, x, y;

    aspect_ratio *= (float)pic_width / (float)pic_height;

    height = win_height;
    width = lrint(height * aspect_ratio);

    if (width > win_width) {
        width = win_width;
        height = rint(width / aspect_ratio);
    }

    x = (win_width - width) / 2;
    y = (win_height - height) / 2;

    rect->x      = win_x + x;
    rect->y      = win_y  + y;
    rect->width  = width;
    rect->height = height;
}

static int 
__is_rect_same(struct rect_s *rect1, 
               struct rect_s *rect2) 
{
    if (rect1->x == rect2->x &&
        rect1->y == rect2->y &&
        rect1->width == rect2->width &&
        rect1->height == rect2->height)
    {
        return 1;
    } else {
        return 0;
    }
}

static int __draw_image(void *arg)
{
    Player *player = (Player *)arg;
    Image *image = player->image;
    Sdl_Image *si = (Sdl_Image *)player->image;
    void *texture = si->texture;
    Component *c = (Component *)image;
    Render *r = c->render;
    Codec *codec = (Codec *)player->video_codec;
    Extractor *extractor = player->extractor;
    Codec *video_codec = player->video_codec;
    Codec *audio_codec = player->audio_codec;
    AVSync *sync = player->sync;
    struct rect_s rect;
    int count = 0; //test
    int ret = 0;

    __calculate_display_rect(&rect,
                             0, //int win_x,
                             0, //int win_y,
                             player->screen_width, //int win_width, 
                             player->screen_height, //int win_height,
                             video_codec->width, //int pic_width, 
                             video_codec->height); //int pic_height)
    if (!__is_rect_same(&rect, &player->display_rect)) {
        image->change_size(image, rect.width, rect.height);
        player->set_state(player, STATE_SCREENSIZE_CHANGED);
        texture = si->texture;

    } else {
        player->clear_state(player, STATE_SCREENSIZE_CHANGED);
    }

    player->display_rect = rect;

    if (texture == NULL) {
        dbg_str(DBG_ERROR, "display video, but no texture");
        /*
         *texture = r->create_yuvtexture(r, rect.width, rect.height);
         *si->texture = texture;
         */
        return 0;
    } 

    ret = codec->update_texture(codec, texture);
    if (ret == 1) {
        int x, y;
        /*
         *dbg_str(DBG_WARN,
         *        "drawing a texture, x=%d, y=%d, width=%d height=%d",
         *        rect.x, rect.y, rect.width, rect.height);
         */

        r->set_color(r, 0, 0, 0, 0);
        r->clear(r);

        r->draw_texture(r, rect.x, rect.y, rect.width, rect.height, texture);  
        r->present(r);

        /*
         *r->destroy_texture(r, texture);
         *player->texture = NULL;
         */
    } else {
        dbg_str(DBG_DETAIL,"waiting video frame....");
        usleep(40 * 1000);
    } 
    
    return ret;
}

static int __draw_current_image(Player *player)
{
    Image *image = player->image;
    Sdl_Image *si = (Sdl_Image *)player->image;
    Component *c = (Component *)image;
    Render *r = c->render;
    Codec *codec = (Codec *)player->video_codec;
    Extractor *extractor = player->extractor;
    Codec *video_codec = player->video_codec;
    Codec *audio_codec = player->audio_codec;
    AVSync *sync = player->sync;
    void *texture;
    struct rect_s rect, old_rect;
    int count = 0; //test
    int ret = 0;

    dbg_str(DBG_VIP, "draw_current_image");

    __calculate_display_rect(&rect,
                             0, //int win_x,
                             0, //int win_y,
                             player->screen_width, //int win_width, 
                             player->screen_height, //int win_height,
                             video_codec->width, //int pic_width, 
                             video_codec->height); //int pic_height)

    image->change_size(image, rect.width, rect.height);
    player->display_rect = rect;
    player->set_state(player, STATE_SCREENSIZE_CHANGED);
    texture = si->texture;

    ret = codec->update_texture_from_current_frame(codec, texture);
    if (ret == 1) {
        int x, y;
        r->set_color(r, 0, 0, 0, 0xff);
        r->clear(r);
        r->draw_texture(r, rect.x, rect.y, rect.width, rect.height, texture);  
        r->present(r);
        dbg_str(DBG_VIP,
                "drawing current frame texture, x=%d, y=%d, width=%d, "
                "height=%d",
                rect.x, rect.y, rect.width, rect.height);

    } else {
        dbg_str(DBG_DETAIL,"draw current frame error");
    } 


    return ret;
}

static int
__get_suitable_rgb_of_screen(Player *player,
                             int screen_width,
                             int screen_height,
                             enum AVPixelFormat fmt,
                             int *width,
                             int *height,
                             void *data, int *len)
{
    Codec *codec = (Codec *)player->video_codec;
    Codec *video_codec = player->video_codec;
    struct rect_s rect;
    int ret = 0;

    __calculate_display_rect(&rect,
                             0, //int win_x,
                             0, //int win_y,
                             player->screen_width, //int win_width, 
                             player->screen_height, //int win_height,
                             video_codec->width, //int pic_width, 
                             video_codec->height); //int pic_height)
    if (!__is_rect_same(&rect, &player->display_rect)) {
        player->set_state(player, STATE_SCREENSIZE_CHANGED);
    } else {
        player->clear_state(player, STATE_SCREENSIZE_CHANGED);
    }

    player->display_rect = rect;

    ret = video_codec->get_rgb(video_codec,
                               fmt,
                               rect.width,
                               rect.height,
                               data, len);

    *width =  rect.width;
    *height = rect.height;

    return ret;
}

static int 
__get_rgb(Player *player,
          enum AVPixelFormat fmt,
          int target_width,
          int target_height,
          void *data, int *len)
{
    Codec *video_codec = player->video_codec;
    int ret;

    ret = video_codec->get_rgb(video_codec,
                               fmt,
                               target_width,
                               target_height,
                               data, len);

    return ret;
}

int __get_rgb_from_current_frame(Player *player,
                                 enum AVPixelFormat fmt,
                                 int target_width,
                                 int target_height,
                                 void *data, int *len)
{
    Codec *video_codec = player->video_codec;
    int ret;

    ret = video_codec->get_rgb_from_current_frame(
            video_codec,
            fmt,
            target_width,
            target_height,
            data, len);

    return ret;
}

static int 
__get_yuv(Player *player, void *data[], int len[])
{
}

static int 
__get_yuv_from_current_frame(Player *player, void *data[], int len[])
{
}

static int 
__wait_if_true(Player *player,
               enum player_state_e conditions[],
               int condition_cnt,
               enum player_state_e exceptions[],
               int exception_cnt,
               int (*action)(void *arg),
               void *action_arg,
               char *who)
{
    int i = 0,  flag = 0, first_flag = 1;

    pthread_mutex_lock(&player->mutex);
    while(1) {
        for (i  = 0; i < condition_cnt; i++) {
            if (player->is_state(player, conditions[i])) {
                flag = 1;
            }
        }

        for (i  = 0; i < exception_cnt; i++) {
            if (player->is_state(player, exceptions[i])) {
                flag = 0;
            }
        }
        if (flag == 0) {
            break;
        }

        if (first_flag) {
            for (i  = 0; i < condition_cnt; i++) {
                if (player->is_state(player, conditions[i])) {
                    dbg_str(DBG_VIP,"%s is waiting state is not :%s",
                            (char *)who, player->state_to_str(player, conditions[i]));
                }
            }
        }

        if (action != NULL) {
            action(action_arg);
        }

        pthread_cond_wait(&player->condition, &player->mutex);
        flag = 0;
        first_flag = 0;
    }
    pthread_mutex_unlock(&player->mutex);
}

static int __broadcast(Player *player)
{           
    return pthread_cond_broadcast(&player->condition);
}           

static int __signal(Player *player)
{
    return pthread_cond_signal(&player->condition);
}

static int __is_state(Player *player, enum player_state_e state)
{
    int ret = 0;

    switch (state) {
        case STATE_RUNNED:
            ret = (player->state_runned == 1 ? 1 : 0);
            break;
        case STATE_IDLE:
            ret = (player->state_idle == 1 ? 1 : 0);
            break;
        case STATE_INITED:
            ret = (player->state_inited == 1 ? 1 : 0);
            break;
        case STATE_OPENING:
            ret = (player->state_opening == 1 ? 1 : 0);
            break;
        case STATE_OPENED:
            ret = (player->state_opened == 1 ? 1 : 0);
            break;
        case STATE_EXTRACTOR_READY:
            ret = (player->state_extractor_ready == 1 ? 1 : 0);
            break;
        case STATE_EXTRACTOR_STOPPED:
            ret = (player->state_extractor_stopped == 1 ? 1 : 0);
            break;
        case STATE_EXTRACTOR_EOF:
            ret = (player->state_extractor_eof == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_CODEC_READY:
            ret = (player->state_audio_codec_ready == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_CODEC_STOPPED:
            ret = (player->state_audio_codec_stopped == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_CODEC_READY:
            ret = (player->state_video_codec_ready == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_CODEC_STOPPED:
            ret = (player->state_video_codec_stopped == 1 ? 1 : 0);
            break;
        case STATE_PLAYING:
            ret = (player->state_playing == 1 ? 1 : 0);
            break;
        case STATE_STOPPING:
            ret = (player->state_stopping == 1 ? 1 : 0);
            break;
        case STATE_STOPPED:
            ret = (player->state_stopped == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_PACKET_QUEUE_EMPTY:
            ret = (player->state_audio_packet_queue_empty == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_PACKET_QUEUE_FULL:
            ret = (player->state_audio_packet_queue_full == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_PACKET_QUEUE_EMPTY:
            ret = (player->state_video_packet_queue_empty == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_PACKET_QUEUE_FULL:
            ret = (player->state_video_packet_queue_full == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_FRAME_QUEUE_EMPTY:
            ret = (player->state_audio_frame_queue_empty == 1 ? 1 : 0);
            break;
        case STATE_AUDIO_FRAME_QUEUE_FULL:
            ret = (player->state_audio_frame_queue_full == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_FRAME_QUEUE_EMPTY:
            ret = (player->state_video_frame_queue_empty == 1 ? 1 : 0);
            break;
        case STATE_VIDEO_FRAME_QUEUE_FULL:
            ret = (player->state_video_frame_queue_full == 1 ? 1 : 0);
            break;
        case STATE_SEEKING:
            ret = (player->state_seeking == 1 ? 1 : 0);
            break;
        case STATE_SEEKED:
            ret = (player->state_seeked == 1 ? 1 : 0);
            break;
        case STATE_SWITCH:
            ret = (player->state_switch == 1 ? 1 : 0);
            break;
        case STATE_END:
            ret = (player->state_end == 1 ? 1 : 0);
            break;
        case STATE_EXIT:
            ret = (player->state_exit == 1 ? 1 : 0);
            break;
        case STATE_CLOSE:
            ret = (player->state_close == 1 ? 1 : 0);
            break;
        case STATE_FULLSCREEN:
            ret = (player->state_full_screen == 1 ? 1 : 0);
            break;
        case STATE_SCREENSIZE_CHANGED:
            ret = (player->state_screensize_changed == 1 ? 1 : 0);
            break;
        case STATE_LOADING:
            ret = ((player->state_audio_packet_loading && player->state_video_packet_loading) == 1 ? 1 : 0);
            break;    
        default:
            break;
    }
    /*
     *dbg_str(DBG_DETAIL,"is state :%s %d",
     *        player->state_to_str(player, state), ret);
     */

    return ret;
}

static int __set_state(Player *player, enum player_state_e state)
{

    dbg_str(DBG_DETAIL,"set state :%s",
            player->state_to_str(player, state));

    switch (state) {
        case STATE_RUNNED:
            player->state_runned = 1;
            break;
        case STATE_IDLE:
            player->state_idle = 1;
            break;
        case STATE_INITED:
            player->state_inited = 1;
            player->publish_message(player, ON_INITED);
            break;
        case STATE_OPENING:
            player->state_opening = 1;
            player->state_stopping = 1;
            player->state_stopped = 0;
            player->state_opened = 0;
            player->state_loading = 0;
            player->state_end = 0;
            player->state_seeking = 0;
            player->publish_message(player, ON_OPENING);
            player->set_state(player, STATE_STOPPING);
            break;
        case STATE_OPENED:
            player->state_opened = 1;
            player->state_opening = 0;
            player->publish_message(player, ON_OPENED);
            break;
        case STATE_EXTRACTOR_READY:
            {
                player->state_extractor_ready = 1;
                player->publish_message(player, ON_EXTRATOR_READY);
            }
            break;
        case STATE_EXTRACTOR_STOPPED:
            {
                player->state_extractor_stopped = 1;
                player->publish_message(player, STATE_EXTRACTOR_STOPPED);

            }
            break;
        case STATE_EXTRACTOR_EOF:
            {
                player->state_extractor_eof = 1;
                /*
                 *player->publish_message(player, ON_EXTRATOR_EOF);
                 */
            }
            break;
        case STATE_AUDIO_CODEC_READY:
            {
                player->state_audio_codec_ready = 1;
                player->publish_message(player, ON_AUDIO_CODEC_READY);
            }
            break;
        case STATE_AUDIO_CODEC_STOPPED:
            {
                player->state_audio_codec_stopped = 1;
                player->publish_message(player, STATE_AUDIO_CODEC_STOPPED);
            }
            break;
        case STATE_VIDEO_CODEC_READY:
            {
                player->state_video_codec_ready = 1;
                player->publish_message(player, ON_VIDEO_CODEC_READY);
            }
            break;
        case STATE_VIDEO_CODEC_STOPPED:
            {
                player->state_video_codec_stopped = 1;
                player->publish_message(player, STATE_VIDEO_CODEC_STOPPED);
            }
            break;
        case STATE_PLAYING:
            player->state_playing = 1;
            player->state_loading = 0;
            player->state_video_packet_loading = 0;
            player->state_audio_packet_loading = 0;
            player->state_stopping = 0;
            player->state_stopped = 0;
            player->state_video_codec_stopped = 0;
            player->state_audio_codec_stopped = 0;
            player->state_extractor_stopped = 0;
            player->state_extractor_eof = 0;
            player->state_seeking = 0;
            player->publish_message(player, ON_PLAY);
            break;
        case STATE_STOPPING:
            player->state_stopping = 1;
            player->state_stopped = 0;
            player->clear_state(player,STATE_PLAYING);
            player->publish_message(player, ON_STOPPING);
            break;
        case STATE_STOPPED:
            player->state_stopped = 1;
            player->state_stopping = 0;
            player->clear_state(player,STATE_PLAYING);
            player->publish_message(player, ON_STOPPED);
            break;
        case STATE_AUDIO_PACKET_QUEUE_EMPTY:
            player->state_audio_packet_queue_empty = 1;
            player->state_audio_packet_queue_full = 0;

            if (player->state_audio_packet_queue_empty == 1 &&
                player->state_audio_frame_queue_empty == 1 &&
                player->state_video_packet_queue_empty == 1 &&
                player->state_video_frame_queue_empty == 1 && 
                player->state_extractor_eof == 1)
            {
                __set_state(player, STATE_END);
            } else if (player->state_audio_packet_queue_empty == 1
                    && player->state_audio_frame_queue_empty == 1
                    && player->state_video_packet_queue_empty == 1
                    && player->state_video_frame_queue_empty == 1
                    && player->state_extractor_eof == 0)
            {
                dbg_str(DBG_WARN, "player has no data");
            }
            break;
        case STATE_AUDIO_PACKET_QUEUE_FULL:
            player->state_audio_packet_queue_full = 1;
            player->state_audio_packet_queue_empty = 0;
            break;
        case STATE_VIDEO_PACKET_QUEUE_EMPTY:
            player->state_video_packet_queue_empty = 1;
            player->state_video_packet_queue_full = 0;

            if (player->state_audio_packet_queue_empty == 1 &&
                player->state_audio_frame_queue_empty == 1 && 
                player->state_video_packet_queue_empty == 1 && 
                player->state_video_frame_queue_empty == 1 &&
                player->state_extractor_eof == 1)
            {
                __set_state(player, STATE_END);
            } else if (player->state_audio_packet_queue_empty == 1
                    && player->state_audio_frame_queue_empty == 1
                    && player->state_video_packet_queue_empty == 1
                    && player->state_video_frame_queue_empty == 1
                    && player->state_extractor_eof == 0)
            {
                dbg_str(DBG_WARN, "player has no data");
            }
            break;
        case STATE_VIDEO_PACKET_QUEUE_FULL:
            player->state_video_packet_queue_full = 1;
            player->state_video_packet_queue_empty = 0;
            break;
        case STATE_AUDIO_FRAME_QUEUE_EMPTY:
            player->state_audio_frame_queue_empty = 1;
            player->state_audio_frame_queue_full = 0;

            if (player->state_audio_packet_queue_empty == 1 &&
                player->state_audio_frame_queue_empty == 1 && 
                player->state_video_packet_queue_empty == 1 && 
                player->state_video_frame_queue_empty == 1 &&
                player->state_extractor_eof == 1)
            {
                __set_state(player, STATE_END);
            } else if (player->state_audio_packet_queue_empty == 1 &&
                       player->state_audio_frame_queue_empty == 1 && 
                       player->state_video_packet_queue_empty == 1 && 
                       player->state_video_frame_queue_empty == 1 &&
                       player->state_extractor_eof == 0) {
                dbg_str(DBG_WARN, "player has no data");
            }
            break;
        case STATE_AUDIO_FRAME_QUEUE_FULL:
            player->state_audio_frame_queue_full = 1;
            player->state_audio_frame_queue_empty = 0;
            break;
        case STATE_VIDEO_FRAME_QUEUE_EMPTY:
            player->state_video_frame_queue_empty = 1;
            player->state_video_frame_queue_full = 0;

            if (player->state_audio_packet_queue_empty == 1 &&
                player->state_audio_frame_queue_empty == 1 && 
                player->state_video_packet_queue_empty == 1 && 
                player->state_video_frame_queue_empty == 1 && 
                player->state_extractor_eof == 1) {
                __set_state(player, STATE_END);
            } else {
                dbg_str(DBG_DETAIL,
                        "state_audio_packet_queue_empty = %d, state_audio_frame_queue_empty =%d, state_video_packet_queue_empty=%d, state_video_frame_queue_empty=%d",
                        player->state_audio_packet_queue_empty,
                        player->state_audio_frame_queue_empty,
                        player->state_video_packet_queue_empty,
                        player->state_video_frame_queue_empty);
            }
            break;
        case STATE_VIDEO_FRAME_QUEUE_FULL:
            player->state_video_frame_queue_full = 1;
            player->state_video_frame_queue_empty = 0;
            break;
        case STATE_SEEKING:
            player->state_seeking = 1;
            player->state_seeked = 0;
            player->state_playing = 0;
            player->state_end = 0;
            player->publish_message(player, ON_SEEKING);
            __set_state(player, STATE_STOPPING);
            break;
        case STATE_SEEKED:
            player->state_seeking = 0;
            player->state_seeked = 1;
            player->state_stopping = 0;
            player->state_stopped = 0;
            player->publish_message(player, ON_SEEKED);
            break;
        case STATE_SWITCH:
            player->state_switch = 1;
            player->state_stopping = 1;
            player->publish_message(player, ON_SWITCH);
            break;
        case STATE_END:
            player->state_end = 1;
            player->publish_message(player, ON_END);
            __set_state(player, STATE_STOPPING);
            break;
        case STATE_EXIT:
            player->state_exit = 1;
            player->publish_message(player, ON_EXIT);
            break;
        case STATE_CLOSE:
            player->state_close = 1;
            /*
             *player->state_stopping = 1;
             */
            player->state_inited = 0;
            player->publish_message(player, ON_CLOSE);
            __set_state(player, STATE_STOPPING);
            break;
        case STATE_FULLSCREEN:
            player->state_full_screen = 1;
            break;
        case STATE_SCREENSIZE_CHANGED:
            player->state_screensize_changed = 1;
            break;
        case STATE_LOADING:
            player->state_loading = 1;
            player->state_playing = 0;
            player->state_audio_packet_loading = 1;
            player->state_video_packet_loading = 1;
            player->publish_message(player,ON_LOADING);
            break;
        case STATE_VIDEO_PACKET_LOADING:
            player->state_video_packet_loading = 1;
            break;
        case STATE_AUDIO_PACKET_LOADING:
            player->state_audio_packet_loading = 1;
            break;
        default:
            break;
    }
    player->broadcast(player);

    return 0;
}

static int __clear_state(Player *player, enum player_state_e state)
{
    switch (state) {
        case STATE_RUNNED:
            player->state_runned = 0;
            break;
        case STATE_IDLE:
            player->state_idle = 0;
            break;
        case STATE_INITED:
            player->state_inited = 0;
            break;
        case STATE_OPENING:
            player->state_opening = 0;
            break;
        case STATE_OPENED:
            player->state_opened = 0;
            break;
        case STATE_EXTRACTOR_READY:
            player->state_extractor_ready = 0;
            break;
        case STATE_EXTRACTOR_STOPPED:
            player->state_extractor_stopped = 0;
            break;
        case STATE_EXTRACTOR_EOF:
            player->state_extractor_eof = 0;
            break;
        case STATE_AUDIO_CODEC_READY:
            player->state_audio_codec_ready = 0;
            break;
        case STATE_AUDIO_CODEC_STOPPED:
            player->state_audio_codec_stopped = 0;
            break;
        case STATE_VIDEO_CODEC_READY:
            player->state_video_codec_ready = 0;
            break;
        case STATE_VIDEO_CODEC_STOPPED:
            player->state_video_codec_stopped = 0;
            break;
        case STATE_PLAYING:
            player->state_playing = 0;
            break;
        case STATE_STOPPING:
            dbg_str(DBG_INFO,"clear state :%s",
                    player->state_to_str(player, state));
            player->state_stopping = 0;
            break;
        case STATE_STOPPED:
            dbg_str(DBG_INFO,"clear state :%s",
                    player->state_to_str(player, state));
            player->state_stopped = 0;
            break;
        case STATE_AUDIO_PACKET_QUEUE_EMPTY:
            player->state_audio_packet_queue_empty = 0;
            break;
        case STATE_AUDIO_PACKET_QUEUE_FULL:
            dbg_str(DBG_INFO,"clear state :%s",
                    player->state_to_str(player, state));

            player->state_audio_packet_queue_full = 0;
            break;
        case STATE_VIDEO_PACKET_QUEUE_EMPTY:
            player->state_video_packet_queue_empty = 0;
            break;
        case STATE_VIDEO_PACKET_QUEUE_FULL:
            dbg_str(DBG_INFO,"clear state :%s",
                    player->state_to_str(player, state));
            player->state_video_packet_queue_full = 0;
            break;
        case STATE_AUDIO_FRAME_QUEUE_EMPTY:
            player->state_audio_frame_queue_empty = 0;
            break;
        case STATE_AUDIO_FRAME_QUEUE_FULL:
            dbg_str(DBG_DETAIL,"clear state :%s",
                    player->state_to_str(player, state));
            player->state_audio_frame_queue_full = 0;
            break;
        case STATE_VIDEO_FRAME_QUEUE_EMPTY:
            player->state_video_frame_queue_empty = 0;
            break;
        case STATE_VIDEO_FRAME_QUEUE_FULL:
            dbg_str(DBG_DETAIL,"clear state :%s",
                    player->state_to_str(player, state));
            player->state_video_frame_queue_full = 0;
            break;
        case STATE_SEEKING:
            player->state_seeking = 0;
            break;
        case STATE_SEEKED:
            player->state_seeked = 0;
            break;
        case STATE_SWITCH:
            player->state_switch = 0;
            break;
        case STATE_END:
            player->state_end = 0;
            break;
        case STATE_EXIT:
            player->state_exit = 0;
            break;
        case STATE_CLOSE:
            player->state_close = 0;
            break;
        case STATE_FULLSCREEN:
            player->state_full_screen = 0;
            break;
        case STATE_SCREENSIZE_CHANGED:
            player->state_screensize_changed = 0;
        case STATE_LOADING:
            player->state_playing =1;
            player->state_loading = 0;
            break;
        case STATE_AUDIO_PACKET_LOADING:
            player->state_audio_packet_loading = 0;
            break;
        case STATE_VIDEO_PACKET_LOADING:
            player->state_video_packet_loading = 0;
            break;
        default:
            break;
    }
    player->broadcast(player);

    return 0;
}

static char *__state_to_str(Player *player, enum player_state_e state)
{
    switch (state) {
        case STATE_RUNNED:
            return "STATE_RUNNED";
        case STATE_IDLE:
            return "STATE_IDLE";
        case STATE_INITED:
            return "STATE_INITED";
        case STATE_OPENING:
            return "STATE_OPENING";
        case STATE_OPENED:
            return "STATE_OPENED";
        case STATE_EXTRACTOR_READY:
            return "STATE_EXTRACTOR_READY";
        case STATE_EXTRACTOR_STOPPED:
            return "STATE_EXTRACTOR_STOPPED";
        case STATE_EXTRACTOR_EOF:
            return "STATE_EXTRACTOR_EOF";
        case STATE_AUDIO_CODEC_READY:
            return "STATE_AUDIO_CODEC_READY";
        case STATE_AUDIO_CODEC_STOPPED:
            return "STATE_AUDIO_CODEC_STOPPED";
        case STATE_VIDEO_CODEC_READY:
            return "STATE_VIDEO_CODEC_READY";
        case STATE_VIDEO_CODEC_STOPPED:
            return "STATE_VIDEO_CODEC_STOPPED";
        case STATE_PLAYING:
            return "STATE_PLAYING";
        case STATE_STOPPING:
            return "STATE_STOPPING";
        case STATE_STOPPED:
            return "STATE_STOPPED";
        case STATE_AUDIO_PACKET_QUEUE_EMPTY:
            return "STATE_AUDIO_PACKET_QUEUE_EMPTY";
        case STATE_AUDIO_PACKET_QUEUE_FULL:
            return "STATE_AUDIO_PACKET_QUEUE_FULL";
        case STATE_VIDEO_PACKET_QUEUE_EMPTY:
            return "STATE_VIDEO_PACKET_QUEUE_EMPTY";
        case STATE_VIDEO_PACKET_QUEUE_FULL:
            return "STATE_VIDEO_PACKET_QUEUE_FULL";
        case STATE_AUDIO_FRAME_QUEUE_EMPTY:
            return "STATE_AUDIO_FRAME_QUEUE_EMPTY";
        case STATE_AUDIO_FRAME_QUEUE_FULL:
            return "STATE_AUDIO_FRAME_QUEUE_FULL";
        case STATE_VIDEO_FRAME_QUEUE_EMPTY:
            return "STATE_VIDEO_FRAME_QUEUE_EMPTY";
        case STATE_VIDEO_FRAME_QUEUE_FULL:
            return "STATE_VIDEO_FRAME_QUEUE_FULL";
        case STATE_SEEKING:
            return "STATE_SEEKING";
        case STATE_SEEKED:
            return "STATE_SEEKED";
        case STATE_SWITCH:
            return "STATE_SWITCH";
        case STATE_END:
            return "STATE_END";
        case STATE_EXIT:
            return "STATE_EXIT";
            break;
        case STATE_CLOSE:
            return "STATE_CLOSE";
            break;
        case STATE_FULLSCREEN:
            return "STATE_FULLSCREEN";
        case STATE_SCREENSIZE_CHANGED:
            return "STATE_SCREENSIZE_CHANGED";
        case STATE_LOADING:
            return "STATE_LOADING";
        default:
            break;
    }

    return "";
}
static int __clear_player_state(Player *player)
{
    player->state_runned= 0;
    player->state_idle = 0;
    player->state_opening = 0;
    player->state_opened = 0;
    player->state_extractor_ready = 0;
    player->state_extractor_stopped = 0;
    player->state_extractor_eof = 0;
    player->state_audio_codec_ready = 0;
    player->state_audio_codec_stopped = 0;
    player->state_video_codec_ready = 0; 
    player->state_video_codec_stopped = 0; 
    player->state_all_ready = 0; 
    player->state_playing = 0;
    player->state_stopping = 0; 
    player->state_stopped = 0; 
    player->state_seeking = 0;
    player->state_seeked = 0;
    player->state_switch = 0; 
    player->state_end = 0;
    player->state_exit = 0; 
    player->state_close = 0; 
    player->state_audio_packet_queue_empty = 0; 
    player->state_audio_packet_queue_full = 0; 
    player->state_video_packet_queue_empty = 0; 
    player->state_video_packet_queue_full = 0; 
    player->state_audio_frame_queue_empty = 0; 
    player->state_audio_frame_queue_full = 0; 
    player->state_video_frame_queue_empty = 0; 
    player->state_video_frame_queue_full = 0;
    player->state_full_screen = 0;
    player->state_screensize_changed = 0;
    player->state_audio_packet_loading = 0;
    player->state_video_packet_loading = 0;
    player->state_loading = 0;
    
}

static int __print(Player *player)
{
    Extractor *extractor = player->extractor;
    Codec *audio_codec   = player->audio_codec;
    Codec *video_codec   = player->video_codec;
    PacketStream *aps    = extractor->audio_packet_stream;
    PacketStream *vps    = extractor->video_packet_stream;
    FrameStream *afs     = audio_codec->audio_frame_stream;
    FrameStream *vfs     = video_codec->video_frame_stream;

    int ret = 0;
    dbg_str(DBG_VIP, "state_idle = %d, "
                     "state_opening = %d, "
                     "state_opened = %d, "
                     "state_extractor_eof = %d, "
                     "state_playing = %d, "
                     "state_seeking = %d, "
                     "state_seeked = %d, "
                     "state_switch = %d, " 
                     "state_end = %d, "
                     "state_exit = %d, " 
                     "state_close = %d, " 
                     "state_full_screen = %d, "
                     "state_screensize_changed = %d, "
                     "state_loading = %d",
                     player->state_idle,
                     player->state_opening,
                     player->state_opened,
                     player->state_extractor_eof,
                     player->state_playing,
                     player->state_seeking,
                     player->state_seeked,
                     player->state_switch, 
                     player->state_end,
                     player->state_exit, 
                     player->state_close, 
                     player->state_full_screen,
                     player->state_screensize_changed,
                     player->state_loading);

    dbg_str(DBG_VIP, "state_extractor_ready = %d, "
                     "state_audio_codec_ready = %d, "
                     "state_video_codec_ready = %d", 
                     player->state_extractor_ready,
                     player->state_audio_codec_ready,
                     player->state_video_codec_ready);


    dbg_str(DBG_VIP, "state_audio_packet_queue_empty = %d, " 
                     "state_audio_frame_queue_empty = %d, " 
                     "state_video_packet_queue_empty = %d, " 
                     "state_video_frame_queue_empty = %d, " 
                     "state_audio_packet_queue_full = %d, " 
                     "state_audio_frame_queue_full = %d, " 
                     "state_video_packet_queue_full = %d, " 
                     "state_video_frame_queue_full = %d",
                     player->state_audio_packet_queue_empty, 
                     player->state_audio_frame_queue_empty, 
                     player->state_video_packet_queue_empty, 
                     player->state_video_frame_queue_empty, 
                     player->state_audio_packet_queue_full, 
                     player->state_audio_frame_queue_full, 
                     player->state_video_packet_queue_full, 
                     player->state_video_frame_queue_full);

    dbg_str(DBG_VIP, "state_extractor_stopped = %d, "
                     "state_audio_codec_stopped = %d, "
                     "state_video_codec_stopped = %d, " 
                     "state_stopping = %d, " 
                     "state_stopped = %d", 
                     player->state_extractor_stopped,
                     player->state_audio_codec_stopped,
                     player->state_video_codec_stopped, 
                     player->state_stopping, 
                     player->state_stopped); 

    dbg_str(DBG_VIP, "vps size=%d, aps size=%d",
                     vps->size(vps),
                     aps->size(aps));

    dbg_str(DBG_VIP, "vfs size=%d, afs size=%d",
                     vfs->size(vfs),
                     afs->size(afs));

    dbg_str(DBG_VIP, "video_current_timestamp :%lf audio_current_timestamp:%lf ",
                     player->sync->get_video_clock(player->sync),
                     player->sync->get_audio_clock(player->sync));

    return ret;
}

static int __check_loading(Player *player)
{
    Extractor *extractor = player->extractor;
    Codec *audio_codec   = player->audio_codec;
    Codec *video_codec   = player->video_codec;
    PacketStream *aps    = extractor->audio_packet_stream;
    PacketStream *vps    = extractor->video_packet_stream;
    FrameStream *afs     = audio_codec->audio_frame_stream;
    FrameStream *vfs     = video_codec->video_frame_stream;
    int video_size, audio_size;

    video_size = vps->size(vps) + vfs->size(vfs);
    audio_size = aps->size(aps) + afs->size(afs);

    if (video_size < LOADING_LOWWATER || audio_size < LOADING_LOWWATER) {
        if (!player->is_state(player, STATE_LOADING) &&
            !player->is_state(player, STATE_EXTRACTOR_EOF) && 
            !player->is_state(player, STATE_END)) {
            player->set_state(player,STATE_LOADING);
        }
    } else if(video_size >= LOADING_HIGHWATER && 
              audio_size >= LOADING_HIGHWATER) {
        if (player->is_state(player,STATE_LOADING)) {
            player->set_state(player,STATE_PLAYING);
        }
    }
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "init", __init, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "start", __start, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "stop", __stop, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "exit", __exit, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "seek_to", __seek_to, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_duration", __get_duration, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_position", __get_position, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_buffered_position", __get_buffered_position, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_buffered_duration", __get_buffered_duration, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_subscriber", __add_subscriber, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "del_subscriber", __del_subscriber, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "run", __run, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "switch_bitrate", __switch_bitrate, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "wait_if_true", __wait_if_true, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "broadcast", __broadcast, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "signal", __signal, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "is_state", __is_state, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_state", __set_state, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_VFUNC_POINTER, "", "clear_state", __clear_state, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_VFUNC_POINTER, "", "state_to_str", __state_to_str, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_image", __add_image, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_audio", __add_audio, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_image", __draw_image, sizeof(void *)}, 
    [28] = {ENTRY_TYPE_VFUNC_POINTER, "", "draw_current_image", __draw_current_image, sizeof(void *)}, 
    [29] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_rgb", __get_rgb, sizeof(void *)}, 
    [30] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_suitable_rgb_of_screen", __get_suitable_rgb_of_screen, sizeof(void *)}, 
    [31] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_rgb_from_current_frame", __get_rgb_from_current_frame, sizeof(void *)}, 
    [32] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_yuv", __get_yuv, sizeof(void *)}, 
    [33] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_yuv_from_current_frame", __get_yuv_from_current_frame, sizeof(void *)}, 
    [34] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_media_url", __get_media_url, sizeof(void *)}, 
    [35] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_media_url", __set_media_url, sizeof(void *)}, 
    [36] = {ENTRY_TYPE_VFUNC_POINTER, "", "publish_message", __publish_message, sizeof(void *)}, 
    [37] = {ENTRY_TYPE_VFUNC_POINTER, "", "open", __open, sizeof(void *)}, 
    [38] = {ENTRY_TYPE_VFUNC_POINTER, "", "close", __close, sizeof(void *)}, 
    [39] = {ENTRY_TYPE_VFUNC_POINTER, "", "print", __print, sizeof(void *)}, 
    [40] = {ENTRY_TYPE_VFUNC_POINTER, "", "check_loading", __check_loading, sizeof(void *)}, 
    [41] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS(Player, concurent_class_info);

