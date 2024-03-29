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
    int (*get_systimezone)(Date_Time *data);
    char *(*to_format_string)(Date_Time *date, char *fmt);
    Date_Time *(*next_day)(Date_Time *date);
    Date_Time *(*start_of_day)(Date_Time *date);
    Date_Time *(*end_of_day)(Date_Time *date);
    Date_Time *(*next_month)(Date_Time *date);
    Date_Time *(*start_of_month)(Date_Time *date);
    Date_Time *(*end_of_month)(Date_Time *date);
    Date_Time *(*next_year)(Date_Time *date);
    Date_Time *(*start_of_year)(Date_Time *date);
    Date_Time *(*end_of_year)(Date_Time *date);
    int (*cmp)(Date_Time *date, char *target);
    Date_Time *(*for_each_day)(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque);
    Date_Time *(*for_each_month)(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque);
    Date_Time *(*for_each_year)(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque);
    Date_Time *(*now)(Date_Time *date);
    Date_Time *(*add_seconds)(Date_Time *date, int sec);
    char *(*zonetime2local)(Date_Time *date, char *value);
    char *(*zonetime2zonetime)(Date_Time *date, char *zonetime, char *target_zone);

    String *value;
    struct tm tm;
    int timezone;
};

#endif
