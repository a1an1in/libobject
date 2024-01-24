#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/core/utils/registry/registry.h>

static struct timeval lasttime;

static void
timeout_cb(int fd, short event, void *arg)
{
    struct timeval newtime, difference;
    double elapsed;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC,"timeout_cb called at %d: %.3f seconds elapsed.",
            (int)newtime.tv_sec, elapsed);
}

static int test_timer(TEST_ENTRY *entry)
{
    struct event timeout;
    struct timeval tv;
    struct event_base* base = event_base_get_default_instance();

    dbg_str(DBG_VIP,"test timer start");

    /* Initalize one event */
    event_assign(&timeout, base, -1, EV_PERSIST, timeout_cb, (void*) &timeout);

    memset(&tv, 0, sizeof(tv));
    tv.tv_sec = 1;
    event_add(&timeout, &tv);

    pause();
    dbg_str(DBG_VIP,"test timer end");

    return (1);
}
REGISTER_TEST_CMD(test_timer);
