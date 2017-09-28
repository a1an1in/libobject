#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_compat.h>

static void
signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;

    dbg_str(DBG_SUC, "signal_cb, signal no:%d", event->ev_fd);
}

int test_signal()
{
    struct event signal_int;
    struct event_base* base;

    /* Initalize the event library */
    base = event_base_new();

    /* Initalize one event */
    event_assign(&signal_int, base, SIGINT, EV_SIGNAL|EV_PERSIST,
                 signal_cb, &signal_int);

    event_add(&signal_int, NULL);

    event_base_dispatch(base);
    event_base_free(base);

    return (0);
}
