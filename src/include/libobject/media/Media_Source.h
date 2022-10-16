#ifndef __MEDIA_SOURCE_H
#define __MEDIA_SOURCE_H

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Linked_List.h>
#include <libobject/core/Obj.h>

typedef struct media_source_s Media_Source;

struct media_source_s{
    Obj obj;

    int (*construct)(Media_Source *,char *init_str);
    int (*deconstruct)(Media_Source *);
    int (*set)(Media_Source *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_url)(Media_Source *, char *url);
    char *(*get_url)(Media_Source *);
    void (*init)(Media_Source *,char *url);
    char *(*get_url_by_bitrate)(Media_Source *,int /*bandwidth*/);
    int  *( *get_list_size)(Media_Source *);
    void (*info)(Media_Source *);

    int  (*isvalid_bitrate)(Media_Source *,int);
    /*attribs*/
    char                  *file_url;
};

#endif //end(__MEDIA_SOURCE_H)
