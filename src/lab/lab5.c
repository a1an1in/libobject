#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <libobject/utils/dbg/debug.h>

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
    int fds[2];
    char msg = 1;
    char signals[10];
    int n;

    evsig_socketpair(fds);
    send(fds[0], (char*)&msg, 1, 0);
    send(fds[0], (char*)&msg, 1, 0);
    send(fds[0], (char*)&msg, 1, 0);
    send(fds[0], (char*)&msg, 1, 0);

    n = recv(fds[1], signals, sizeof(signals), 0);
    printf("n =%d", n);
}
