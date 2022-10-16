#ifndef __FF_CODEC_H__
#define __FF_CODEC_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Thread.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libobject/media/DataSource.h>
#include <libobject/media/PacketStream.h>
#include <libobject/media/Codec.h>

typedef struct ff_h264_codec_s FF_Video_Codec;

struct ff_h264_codec_s{
    Codec parent;

    int (*construct)(FF_Video_Codec *,char *init_str);
    int (*deconstruct)(FF_Video_Codec *);
    int (*set)(FF_Video_Codec *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    void (*thread_callback)(void *arg);
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
    int (*seek)(Codec *);
    int (*open)(Codec *);
    int (*stop)(Codec *);
    int (*close)(Codec *);

    /*attribs*/
    AVCodecContext	*codec_ctx;
    AVCodec			*codec;
    struct SwsContext* sws_context;
    AVFrame *out_frame;
};

#endif
