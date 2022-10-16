#ifndef __FF_Media_Source_H
#define __FF_Media_Source_H

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/media/Media_Source.h>
#include <libobject/media/hls.h>

typedef struct FF_Media_Source_s FF_Media_Source;


enum type_media_stream {
    HLS_TYPE_MASTER = 0,
    HLS_TYPE_NORMAL ,
    TYPE_OTHERS    
};

struct FF_Media_Source_s{
    Media_Source parent;

    int (*construct)(FF_Media_Source *,char *init_str);
    int (*deconstruct)(FF_Media_Source *);
    int (*set)(FF_Media_Source *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_url)(FF_Media_Source *, char *url);
    char *(*get_url)(FF_Media_Source *);
    void (*info)(FF_Media_Source *);
    void (*init)(FF_Media_Source *,char *url);

    int (*url_sanitycheck)(FF_Media_Source * ,char *);
    /* about hls protocal */
    char *(*get_url_by_bitrate)(FF_Media_Source *,int /*bandwidth*/);
    int  *( *get_list_size)(FF_Media_Source *);
    int  (*isvalid_bitrate)(FF_Media_Source *,int);
    int  (*get_bitrate_array)(FF_Media_Source *,int *);
    int  (*get_bitrate_number)(FF_Media_Source *);
    /* about other protocals vs mp4 mkv .*/

    /*attribs*/
    String                * file_url;
    List                  * m_list;
    int                   n_playlists;
    enum type_media_stream     stream_type;

};

#endif //end(__MEDIA_SOURCE_H)
