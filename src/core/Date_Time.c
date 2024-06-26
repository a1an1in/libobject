/**
 * @file Date_Time.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
#if (!defined(WINDOWS_USER_MODE))
#include <libobject/core/Date_Time.h>

static int __construct(Date_Time *date, char *init_str)
{
    allocator_t *allocator = date->parent.allocator;

    if (date->value == NULL) {
        date->value = (String *)object_new(allocator, (char *)"String", NULL);
    }
    return 0;
}

static int __deconstruct(Date_Time *date)
{
    object_destroy(date->value);
    return 0;
}

/*
 * value 为iso 8601 日期格式
 **/
static int __assign(Date_Time *date, char *value)
{
    int timezone_offset = 0;
    time_t time;
    int ret;

    TRY {
        date->value->assign(date->value, value);
        sscanf(value, "%d-%d-%d %d:%d:%d", 
               &date->tm.tm_year, &date->tm.tm_mon, &date->tm.tm_mday,
               &date->tm.tm_hour, &date->tm.tm_min, &date->tm.tm_sec);

        date->tm.tm_year -= 1900;
        date->tm.tm_mon -= 1;
        date->tm.tm_isdst = -1;
    } CATCH (ret) {}

    return ret;
}

static char * __to_format_string(Date_Time *date, char *fmt)
{
    strftime(STR2A(date->value), date->value->value_max_len, fmt, &date->tm);

    return STR2A(date->value);
}

static int __get_systimezone(Date_Time *date)
{
    time_t time_utc;
    struct tm tm_local;
    struct tm tm_gmt;

    // Get the UTC time
    time(&time_utc);
    // Get the local time, Use localtime_r for threads safe
    localtime_r(&time_utc, &tm_local);
    // Change it to GMT tm
    gmtime_r(&time_utc, &tm_gmt);

    int time_zone = tm_local.tm_hour - tm_gmt.tm_hour;
    if (time_zone < -12) {
        time_zone += 24;
    } else if (time_zone > 12) {
        time_zone -= 24;
    }

    return time_zone * 100;
}

static Date_Time *__next_day(Date_Time *date)
{
    time_t time;

    time = mktime(&date->tm);
    time += (23 - date->tm.tm_hour) * 3600 + (59 - date->tm.tm_min) * 60 + (60 - date->tm.tm_sec);
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__end_of_day(Date_Time *date)
{
    time_t time;

    time = mktime(&date->tm);
    time += (23 - date->tm.tm_hour) * 3600 + (59 - date->tm.tm_min) * 60 + (59 - date->tm.tm_sec);
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__next_month(Date_Time *date)
{
    time_t time;

    date->tm.tm_year += (date->tm.tm_mon + 1) / 12;
    date->tm.tm_mon = (date->tm.tm_mon + 1) % 12;
    date->tm.tm_mday = 1;
    date->tm.tm_hour = 0;
    date->tm.tm_min = 0;
    date->tm.tm_sec = 0;
    date->tm.tm_isdst = 0;

    time = mktime(&date->tm);
    localtime_r(&time, &date->tm);
    date->tm.tm_hour = 0;

    return date;
}

static Date_Time *__end_of_month(Date_Time *date)
{
    time_t time;

    date->next_month(date);
    time = mktime(&date->tm);
    time -= 1;
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__next_year(Date_Time *date)
{
    time_t time;

    date->tm.tm_year = (date->tm.tm_year + 1);
    date->tm.tm_mon = 0;
    date->tm.tm_mday = 1;
    date->tm.tm_hour = 0;
    date->tm.tm_min = 0;
    date->tm.tm_sec = 0;
    time = mktime(&date->tm);
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__end_of_year(Date_Time *date)
{
    time_t time;

    date->next_year(date);
    time = mktime(&date->tm);
    time -= 1;
    localtime_r(&time, &date->tm); 

    return date;
}


static int __cmp(Date_Time *date, char *target)
{
    char *str = date->to_format_string(date, (char *)"%F %T");

    return strncmp(str, target, strlen(str)); 
}

static Date_Time *
__for_each_day(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque) 
{
    char s[50];
    char *e;
    int ret = 0;

    TRY {
        for (; date->cmp(date, end) <= 0; date->next_day(date)) {
            strcpy(s, date->to_format_string(date, (char *)"%F %T"));
            date->end_of_day(date);
            e = date->to_format_string(date, (char *)"%F %T");
            if (date->cmp(date, end) > 0) { e = end; }
            SET_CATCH_STR_PARS(s, e);
            callback(s, e, opaque);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "Date_Time::for_each_day() error, par1=%s, par2=%s",
                ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static Date_Time *
__for_each_month(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque) 
{
    char s[50];
    char *e;
    int ret = 0;

    TRY {
        for (; date->cmp(date, end) <= 0; date->next_month(date)) {
            strcpy(s, date->to_format_string(date, (char *)"%F %T"));
            date->end_of_month(date);
            e = date->to_format_string(date, (char *)"%F %T");
            if (date->cmp(date, end) > 0) { e = end; }
            SET_CATCH_STR_PARS(s, e);
            callback(s, e, opaque);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "Date_Time::for_each_month() error, par1=%s, par2=%s",
                ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static Date_Time *
__for_each_year(Date_Time *date, char *end, int (*callback)(char *start, char *end, void *opaque), void *opaque) 
{
    char s[50];
    char *e;
    int ret = 0;

    TRY {
        for (; date->cmp(date, end) <= 0; date->next_year(date)) {
            strcpy(s, date->to_format_string(date, (char *)"%F %T"));
            date->end_of_year(date);
            e = date->to_format_string(date, (char *)"%F %T");
            if (date->cmp(date, end) > 0) { e = end; }
            SET_CATCH_STR_PARS(s, e);
            callback(s, e, opaque);
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "Date_Time::for_each_year() error, par1=%s, par2=%s",
                ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static Date_Time *__now(Date_Time *date)
{
    time_t rawtime;     

    time(&rawtime); 
    localtime_r(&rawtime, &date->tm); 

    return date;
}

static Date_Time *__start_of_day(Date_Time *date)
{
    time_t time;

    date->tm.tm_hour = 0;
    date->tm.tm_min = 0;
    date->tm.tm_sec = 0;
    time = mktime(&date->tm);
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__start_of_month(Date_Time *date)
{
    time_t time;

    date->tm.tm_mday = 1;
    date->tm.tm_hour = 0;
    date->tm.tm_min = 0;
    date->tm.tm_sec = 0;
    time = mktime(&date->tm);
    localtime_r(&time, &date->tm); 

    return date;
}

static Date_Time *__start_of_year(Date_Time *date)
{
    time_t time;

    date->tm.tm_mon = 0;
    date->tm.tm_mday = 1;
    date->tm.tm_hour = 0;
    date->tm.tm_min = 0;
    date->tm.tm_sec = 0;
    time = mktime(&date->tm);
    localtime_r(&time, &date->tm); 

    return date;
}
static Date_Time *__add_seconds(Date_Time *date, int secs)
{
    time_t time;

    time = mktime(&date->tm);
    time += secs;
    localtime_r(&time, &date->tm); 

    return date;
}

static char *__zonetime2local(Date_Time *date, char *value)
{
    int timezone_offset = 0;
    time_t time;
    char *p;
    int ret;

    TRY {
        p = strstr(value, "UTC");
        THROW_IF(p == NULL, -1);
        date->value->assign(date->value, value);
        sscanf(value, "%d-%d-%d %d:%d:%d UTC%d", 
               &date->tm.tm_year, &date->tm.tm_mon, &date->tm.tm_mday,
               &date->tm.tm_hour, &date->tm.tm_min, &date->tm.tm_sec,
               &date->timezone);

        date->tm.tm_year -= 1900;
        date->tm.tm_mon -= 1;
        date->tm.tm_isdst = -1;
        dbg_str(DBG_DETAIL, "systimezone:%d timezone:%d", date->get_systimezone(date), date->timezone);

        if (date->timezone != 0) {
            timezone_offset = (date->timezone - date->get_systimezone(date)) / 100;
            time = mktime(&date->tm);
            THROW_IF(time == -1, -1);
            
            dbg_str(DBG_DETAIL, "timezone_offset:%d, time:%d", timezone_offset, time);
            time += (3600 * (timezone_offset));
            
            localtime_r(&time, &date->tm); 
            dbg_str(DBG_DETAIL,"localtime:%d-%d-%d %d:%d:%d UTC%d", 
                    date->tm.tm_year, date->tm.tm_mon, date->tm.tm_mday,
                    date->tm.tm_hour, date->tm.tm_min, date->tm.tm_sec, 
                    date->get_systimezone(date));
        }
        p = date->to_format_string(date, "%F %T UTC%z");

    } CATCH (ret) {
        p = NULL;
    }

    return p;
}

static char * __zonetime2zonetime(Date_Time *date, char *zonetime, char *target_zone)
{

}

static class_info_entry_t module_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Date_Time, construct, __construct),
    Init_Nfunc_Entry(2 , Date_Time, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3 , Date_Time, assign, __assign),
    Init_Nfunc_Entry(4 , Date_Time, to_format_string, __to_format_string),
    Init_Nfunc_Entry(5 , Date_Time, get_systimezone, __get_systimezone),
    Init_Nfunc_Entry(6 , Date_Time, next_day, __next_day),
    Init_Nfunc_Entry(7 , Date_Time, end_of_day, __end_of_day),
    Init_Nfunc_Entry(8 , Date_Time, next_month, __next_month),
    Init_Nfunc_Entry(9 , Date_Time, end_of_month, __end_of_month),
    Init_Nfunc_Entry(10, Date_Time, next_year, __next_year),
    Init_Nfunc_Entry(11, Date_Time, end_of_year, __end_of_year),
    Init_Nfunc_Entry(12, Date_Time, cmp, __cmp),
    Init_Nfunc_Entry(13, Date_Time, for_each_day, __for_each_day),
    Init_Nfunc_Entry(14, Date_Time, for_each_month, __for_each_month),
    Init_Nfunc_Entry(15, Date_Time, for_each_year, __for_each_year),
    Init_Nfunc_Entry(16, Date_Time, now, __now),
    Init_Nfunc_Entry(17, Date_Time, start_of_day, __start_of_day),
    Init_Nfunc_Entry(18, Date_Time, start_of_month, __start_of_month),
    Init_Nfunc_Entry(19, Date_Time, start_of_year, __start_of_year),
    Init_Nfunc_Entry(20, Date_Time, add_seconds, __add_seconds),
    Init_Nfunc_Entry(21, Date_Time, zonetime2local, __zonetime2local),
    Init_Nfunc_Entry(22, Date_Time, zonetime2zonetime, __zonetime2zonetime),
    Init_End___Entry(23, Date_Time),
};
REGISTER_CLASS(Date_Time, module_class_info);

#endif