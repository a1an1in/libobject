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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_compat.h>

static void
signal_cb(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;

    dbg_str(DBG_SUC, "signal_cb, signal no:%d, arg=%p", event->ev_fd, event->ev_arg);
}

static void
signal_cb2(int fd, short event_res, void *arg)
{
    struct event *event = (struct event *)arg;

    dbg_str(DBG_SUC, "signal_cb2, signal no:%d, arg=%p", event->ev_fd, event->ev_arg);
}
static int test_signal(TEST_ENTRY *entry, void *argc, void *argv)
{
    struct event signal_int;
    struct event_base* base;

    /* Initalize the event library */

    dbg_str(DBG_DETAIL, "test_signal");
    base = event_base_new();

    /* Initalize one event */
    event_assign(&signal_int, base, SIGUSR1, EV_SIGNAL|EV_PERSIST,
                 signal_cb, &signal_int);

    event_add(&signal_int, NULL);

    event_base_dispatch(base);
    event_base_free(base);
    dbg_str(DBG_DETAIL, "test_signal end 2");

    return (1);
}
REGISTER_STANDALONE_TEST_FUNC(test_signal);

static int test_two_same_signal(TEST_ENTRY *entry, void *argc, void *argv)
{
    struct event signal_event1;
    struct event signal_event2;
    struct event_base* base = get_default_event_base();

    /* Initalize the event library */

    dbg_str(DBG_DETAIL, "test_two_same_signal");

    /* Initalize one event */
    event_assign(&signal_event1, base, SIGUSR1, EV_SIGNAL|EV_PERSIST,
                 signal_cb, &signal_event1);

    event_add(&signal_event1, (void *)base);

    event_assign(&signal_event2, base, SIGUSR1, EV_SIGNAL|EV_PERSIST,
                 signal_cb2, &signal_event1);

    event_add(&signal_event2,(void *)base);

    pause();
    pause();
    dbg_str(DBG_DETAIL, "test_two_same_signal end 2");

    return (1);
}
REGISTER_STANDALONE_TEST_FUNC(test_two_same_signal);
