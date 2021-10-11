#ifndef __DATE_H__
#define __DATE_H__

#include <stdio.h>
#include <time.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct Date_Time_s Date_Time;

struct Date_Time_s{
    Obj parent;

    int (*construct)(Date_Time *,char *);
    int (*deconstruct)(Date_Time *);

    /*virtual methods reimplement*/
    int (*set)(Date_Time *, char *attrib, void *value);
    void *(*get)(Date_Time *, char *attrib);
    char *(*to_json)(Date_Time *); 
    int (*assign)(Date_Time *data, char *value);
    int (*get_timezone)(Date_Time *data);
    char *(*to_format_string)(Date_Time *date, char *fmt);

    String *value;
    struct tm tm;
    int timezone;
};

#endif
