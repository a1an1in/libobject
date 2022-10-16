#ifndef __FF_EXTRACTOR_H__
#define __FF_EXTRACTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Thread.h>
#include <libavformat/avformat.h>
#include <libobject/media/DataSource.h>
#include <libobject/media/PacketStream.h>
#include <libobject/media/Extractor.h>

typedef struct ff_ts_extractor_s FF_Extractor;

struct ff_ts_extractor_s{
    Extractor parent;

    int (*construct)(FF_Extractor *,char *init_str);
    int (*deconstruct)(FF_Extractor *);
    int (*set)(FF_Extractor *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    void (*thread_callback)(void *arg);
    int (*seek)(Extractor *);
    int (*open)(Extractor *);
    int (*stop)(Extractor *);
    int (*close)(Extractor *);
    int  (*switch_bitrate)(Extractor *);

    /*attribs*/
    AVFormatContext	*format_ctx;
    AVCodecContext	*video_codec_ctx;
    AVCodecContext	*audio_codec_ctx;
};

#endif
