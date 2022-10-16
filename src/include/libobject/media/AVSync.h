#ifndef _____ASYNC___STREAM_TOOLS_H
#define _____ASYNC___STREAM_TOOLS_H

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Thread.h>
#include <libobject/core/Lock.h>
#include "Codec.h"

typedef struct async_stream_t AVSync ;

struct async_stream_t {
    Obj obj;

    int (*construct)(AVSync *,char *init_str);
    int (*deconstruct)(AVSync *);
    int (*set)(AVSync *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);


    /*set */
    void (*set_video_codec)(AVSync *,Codec *);
    void (*set_audio_codec)(AVSync *,Codec *);
    Codec *(*get_video_codec)(AVSync *);
    Codec *(*get_audio_codec)(AVSync *);

    void (*init)(AVSync *, Codec *, Codec *);
    int (*clear)(AVSync *);
    /* attribute */
    void (*async_core)(AVSync *, AVFrame *frame, Queue *);

    double (*get_audio_clock)(AVSync *);
    double (*get_video_clock)(AVSync *);

    Lock  *audio_mutex;
    Lock  *video_mutex;
    Codec *audio_codec;
    Codec *video_codec;
};



#endif 

