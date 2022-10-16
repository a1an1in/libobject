#ifndef __FRAMESTREAM_H__
#define __FRAMESTREAM_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Queue.h>

enum frame_stream_type_e{
    FRAME_STREAM_VIDEO = 1, 
    FRAME_STREAM_AUDIO, 
};

typedef struct frame_stream_s FrameStream;

struct frame_stream_s{
    Obj obj;

    int (*construct)(FrameStream *,char *init_str);
    int (*deconstruct)(FrameStream *);
    int (*set)(FrameStream *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*add_frame)(FrameStream *, void *frame);
    int (*remove_frame)(FrameStream *, void **frame);
    size_t (*size)(FrameStream *);
    size_t (*empty)(FrameStream *);
    void   (*clear)(FrameStream *);
    size_t (*set_maxsize)(FrameStream *, int maxsize);
    size_t (*set_player)(FrameStream *, void *player);
    size_t (*set_type)(FrameStream *, int type);

    /*attribs*/
    Queue *queue;
    int maxsize;
    void *player;
    int type;

};

#endif
