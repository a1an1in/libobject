#ifndef __PACKETSTREAM_H__
#define __PACKETSTREAM_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Queue.h>

typedef struct packet_stream_s PacketStream;

enum packet_stream_type_e{
    PACKET_STREAM_VIDEO = 1, 
    PACKET_STREAM_AUDIO, 
};

struct packet_stream_s{
    Obj obj;

    int (*construct)(PacketStream *,char *init_str);
    int (*deconstruct)(PacketStream *);
    int (*set)(PacketStream *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*add_packet)(PacketStream *, void *packet);
    int (*remove_packet)(PacketStream *, void **packet);
    size_t (*size)(PacketStream *);
    size_t (*set_maxsize)(PacketStream *, int maxsize);
    size_t (*set_player)(PacketStream *, void *player);
    size_t (*set_type)(PacketStream *, int type);
    size_t (*empty)(PacketStream *);
    void   (*clear)(PacketStream *);

    /*attribs*/
    Queue *queue;
    int maxsize;
    void *player;
    int type;
};

#endif
