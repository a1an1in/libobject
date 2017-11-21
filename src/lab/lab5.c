#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_base.h>

void print_time_stamp()  
{  
    struct timeval    tv;  
    struct timezone   tz;  
    struct tm         *p;  

    gettimeofday(&tv, &tz);  
    p = localtime(&tv.tv_sec);  
    printf("time_now:%d /%d /%d %d :%d :%d.%3ld", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);  
}  

void lab5()
{
}
