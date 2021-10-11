/**
 * @file Date_Time.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

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
 * */
static int __assign(Date_Time *date, char *value)
{
    allocator_t *allocator = date->parent.allocator;
    int timezone_offset = 0;
    time_t time;

    date->value->assign(date->value, value);
    sscanf(value, "%d-%d-%d %d:%d:%d UTC%d", 
           &date->tm.tm_year, &date->tm.tm_mon, &date->tm.tm_mday,
           &date->tm.tm_hour, &date->tm.tm_min, &date->tm.tm_sec, 
           &date->timezone);
    date->tm.tm_year -= 1900;
    date->tm.tm_mon -= 1;

    if (date->timezone != 0) {
        timezone_offset = (date->timezone / 100) - date->get_timezone(date);
        time = mktime(&date->tm);
        dbg_str(DBG_DETAIL, "time:%lld", time);
        time += (3600 * (timezone_offset));
        dbg_str(DBG_DETAIL, "time:%lld", time);
        localtime_r(&time, &date->tm); 
    }

    return 0;
}

static char * __to_format_string(Date_Time *date, char *fmt)
{
    strftime(STR2A(date->value), date->value->value_max_len, fmt, &date->tm);

    return STR2A(date->value);
}

static char * __get_timezone(Date_Time *date)
{
    struct tm tm;

    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 70;
    tm.tm_mday = 2;

    return (24 * 3600 - (int)mktime(&tm)) / 3600;
}

static class_info_entry_t module_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Date_Time, construct, __construct),
    Init_Nfunc_Entry(2, Date_Time, assign, __assign),
    Init_Nfunc_Entry(3, Date_Time, to_format_string, __to_format_string),
    Init_Nfunc_Entry(4, Date_Time, get_timezone, __get_timezone),
    Init_End___Entry(5, Date_Time),
};
REGISTER_CLASS("Date_Time", module_class_info);

