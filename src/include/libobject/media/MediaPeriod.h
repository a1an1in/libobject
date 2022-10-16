#ifndef __MEDIAPEROID_H__
#define __MEDIAPEROID_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>

typedef struct media_period_s MediaPeriod;

struct media_period_s{
    Obj obj;

    int (*construct)(MediaPeriod *,char *init_str);
    int (*deconstruct)(MediaPeriod *);
    int (*set)(MediaPeriod *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_url)(MediaPeriod *, char *url);
    char *(*get_url)(MediaPeriod *);

    /*attribs*/
    char *url;
};

#endif
