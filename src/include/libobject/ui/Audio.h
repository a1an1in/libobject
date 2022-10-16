#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/Queue.h>

typedef struct audio_s Audio;
struct audio_s{
    Obj obj;

    /*normal methods*/
    int (*construct)(Audio *audio,char *init_str);
    int (*deconstruct)(Audio *audio);
    int (*set)(Audio *audio, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
    int (*set_freq)(Audio *audio, int freq);
    int (*set_format)(Audio *audio, int format);
    int (*set_channels)(Audio *audio, int num);
    int (*set_buffer_size)(Audio *audio, int size);
    int (*set_frame_queue)(Audio *audio, Queue *queue);
    int (*set_callback)(Audio *audio, int (*callback)(void *, void *));
    int (*set_callback_opaque)(Audio *audio, void *opaque);
    int (*init)(Audio *audio);
    int (*play)(Audio *audio);
    int (*pause)(Audio *audio);
    int (*close)(Audio *audio);

    /*attribs*/
    String *path;
    int freq;
    int format;
    int channel_num;
    int buffer_size;
    int (*callback)(void *, void *);
    void *opaque;
    char audio_buffer[1024 * 10 * 4];
    int audio_buffer_len;
    int audio_read_pos;
};

#endif
