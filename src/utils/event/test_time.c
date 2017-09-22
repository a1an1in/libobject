#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/utils/event/event_compat.h>

static struct timeval lasttime;

static int timeval_sub(struct timeval *k1,struct timeval *k2, struct timeval *r)
{
    (r)->tv_sec  = (k1)->tv_sec - (k2)->tv_sec;
    (r)->tv_usec = (k1)->tv_usec - (k2)->tv_usec;

    if ((r)->tv_usec < 0) {               
        (r)->tv_sec--;                
        (r)->tv_usec += 1000000;          
    }                           

    return 0;
}

static int timeval_clear(struct timeval *t)
{
    t->tv_sec = t->tv_usec = 0;
    return 0;
}

static void
timeout_cb(int fd, short event, void *arg)
{
    struct timeval newtime, difference;
    double elapsed;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC,"timeout_cb called at %d: %.3f seconds elapsed.\n",
            (int)newtime.tv_sec, elapsed);
}

int test_time()
{
    struct event timeout;
    struct timeval tv;
    struct event_base *base;

    dbg_str(DBG_DETAIL,"size event=%d",sizeof(timeout));

    /* Initalize the event library */
    base = event_base_new();

    /* Initalize one event */
    event_assign(&timeout, base, -1, EV_PERSIST, timeout_cb, (void*) &timeout);

    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = 2;
    event_add(&timeout, &tv);

    gettimeofday(&lasttime, NULL);

    event_base_dispatch(base);

    event_base_free(base);

    return (0);
}
