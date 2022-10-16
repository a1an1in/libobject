#ifndef __EXTRACTOR_H__
#define __EXTRACTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Thread.h>
#include <libobject/message/Publisher.h>
#include <libavformat/avformat.h>
#include <libobject/media/DataSource.h>
#include <libobject/media/PacketStream.h>

typedef struct extractor_s Extractor;

struct extractor_s{
    Obj obj;

    int (*construct)(Extractor *,char *init_str);
    int (*deconstruct)(Extractor *);
    int (*set)(Extractor *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_data_source)(Extractor *, void *data_source);
    int (*set_player)(Extractor *, void *player);
    void (*thread_callback)(void *arg);
    int (*run)(Extractor *);
    int (*seek)(Extractor *);
    int (*switch_bitrate)(Extractor *);
    int (*open)(Extractor *);
    int (*stop)(Extractor *);
    int (*close)(Extractor *);
    int (*guard_dog)(Extractor *);

    /*attribs*/
    DataSource *data_source;

    PacketStream *video_packet_stream;
    PacketStream *audio_packet_stream;
    PacketStream *subtitle_packet_stream;
    /* AVStream stream */

    AVStream  *  audio_stream;
    AVStream  *  video_stream;

    Thread *thread;
    void *thread_callback_arg;
    int out_flag;
    int video_stream_index;
    int audio_stream_index;
    int subtile_stream_index;

    int extractor_seek_tag;

    int is_seek_finished;
    int is_switch_fihished;

    int64_t  seek_timestamp;
    int      seek_flags;
    int      seek_bitrate;
    int      switch_flags;

    void *player;
    Publisher *publisher;
};

#endif
