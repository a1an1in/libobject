#ifndef __FF_AAC_CODEC_H__
#define __FF_AAC_CODEC_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Thread.h>
#include <libavcodec/avcodec.h>
#include <libobject/media/DataSource.h>
#include <libobject/media/PacketStream.h>
#include <libobject/media/Codec.h>

typedef struct ff_aac_codec_s FF_Audio_Codec;

struct ff_aac_codec_s{
    Codec parent;

    int (*construct)(FF_Audio_Codec *,char *init_str);
    int (*deconstruct)(FF_Audio_Codec *);
    int (*set)(FF_Audio_Codec *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    void (*thread_callback)(void *arg);
    int (*update_texture)(Codec *codec, void *buffer);
    int (*seek)(Codec *);
    int (*open)(Codec *);
    int (*stop)(Codec *);
    int (*close)(Codec *);

    /*attribs*/
    AVCodecContext	*codec_ctx;
    AVCodec			*codec;
    struct SwrContext *swr_ctx;
    uint8_t *out_buffer;
};

#endif
