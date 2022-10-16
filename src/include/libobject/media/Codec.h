#ifndef __CODEC_H__
#define __CODEC_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/message/Subscriber.h>
#include <libobject/message/Publisher.h> 
#include <libobject/media/DataSource.h>
#include <libobject/media/PacketStream.h>
#include <libobject/media/FrameStream.h>
#include <libobject/media/Extractor.h>

typedef struct codec_s Codec;

struct codec_s{
    Obj obj;

    int (*construct)(Codec *,char *init_str);
    int (*deconstruct)(Codec *);
    int (*set)(Codec *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    void (*thread_callback)(void *arg);
    int (*set_thread_callback_arg)(Extractor *, void *arg);
    int (*run)(Codec *);
    int (*set_player)(Codec *codec, void *player);
    int (*update_texture)(Codec *codec, void *texture);
    int (*update_texture_from_current_frame)(Codec *codec, void *texture);
    int (*get_rgb)(Codec *codec,
                   enum AVPixelFormat fmt,
                   int target_width,
                   int target_height,
                   void *data, int *len);
    int (*get_rgb_from_current_frame)(Codec *codec,
                                      enum AVPixelFormat fmt,
                                      int target_width,
                                      int target_height,
                                      void *data, int *len);
    int (*get_yuv)(Codec *codec, void *data[], int len[]);
    int (*get_yuv_from_current_frame)(Codec *codec, void *data[], int len[]);
    int (*set_video_packet_stream)(Codec *codec, void *stream);
    int (*set_audio_packet_stream)(Codec *codec, void *stream);
    int (*set_subtitle_packet_stream)(Codec *codec, void *stream);
    int (*seek)(Codec *);
    int (*open)(Codec *);
    int (*stop)(Codec *);
    int (*close)(Codec *);

    /*attribs*/
    PacketStream *video_packet_stream;
    PacketStream *audio_packet_stream;
    PacketStream *subtitle_packet_stream;

    FrameStream *audio_frame_stream;
    FrameStream *video_frame_stream;
    FrameStream *subtitle_frame_stream;

    // Mem_Cache   *video_memory_cache;
    // Mem_Cache   *audio_memory_cache;

    Extractor *extractor;

    Thread *thread;
    void *thread_callback_arg;

    int out_flag;
    void *player;
    int audio_freq;
    int audio_channel_num;
    int audio_format;
    int width, height;

    double cur_render_audio_clock;
    double cur_render_video_clock;

    int codec_seek_flag;
    int64_t codec_seek_timestamps;
    int codec_isold;
    Subscriber *subscriber;
    Publisher *publisher;
};

#endif
