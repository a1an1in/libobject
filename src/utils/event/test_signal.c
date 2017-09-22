#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/utils/event/event_compat.h>
#include <libobject/utils/dbg/debug.h>

int called = 0;

static void
signal_cb(int fd, short event, void *arg)
{
    struct event *signal = arg;

    dbg_str(DBG_SUC, "signal_cb, called = %d", called);

    called++;
}

int test_signal()
{
    struct event signal_int;
    struct event_base* base;

    /* Initalize the event library */
    base = event_base_new();

    /* Initalize one event */
    event_assign(&signal_int, base, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb,
        &signal_int);

    event_add(&signal_int, NULL);

    event_base_dispatch(base);
    event_base_distroy(base);

    return (0);
}
