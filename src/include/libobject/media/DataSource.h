#ifndef __DATASOURCE_H__
#define __DATASOURCE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/media/MediaPeriod.h>

typedef struct data_source_s DataSource;

struct data_source_s{
    Obj obj;

    int (*construct)(DataSource *,char *init_str);
    int (*deconstruct)(DataSource *);
    int (*set)(DataSource *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_media_period)(DataSource *, char *media_period);
    MediaPeriod *(*get_media_period)(DataSource *);

    /*attribs*/
    MediaPeriod *media_peroid;

};

#endif
