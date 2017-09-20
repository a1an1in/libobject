#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/event/select_base.h>
#include <libobject/utils/event/event_compat.h>

static void
evsig_cb(int fd, short what, void *arg)
{
    static char signals[1024];
    int n;
    int i;
    int ncaught[NSIG];
    Event_Base *eb;

    eb = arg;

    memset(&ncaught, 0, sizeof(ncaught));

    while (1) {
        n = recv(fd, signals, sizeof(signals), 0);
        if (n == -1) {
            dbg_str(DBG_ERROR,"evsig_cb recv");
            break;
        } else if (n == 0) {
            break;
        }
        for (i = 0; i < n; ++i) {
            uint8_t sig = signals[i];
            if (sig < NSIG)
                ncaught[sig]++;
        }
    }

    for (i = 0; i < NSIG; ++i) {
        /*
         *if (ncaught[i])
         *    evmap_signal_active(base, i, ncaught[i]);
         */
    }
}

static void signal_handler(int sig)
{
    int save_errno = errno;
    int msg;

    /*
     *msg = sig;
     *send(evsig_base_fd, (char*)&msg, 1, 0);
     *errno = save_errno;
     */
}


int evsig_init(Event_Base *eb, int evsignal)
{
    int fds[2];
    struct sigaction sa; 

    if (pipe(fds)) {
        dbg_str(SM_ERROR,"cannot create pipe");
        return -1;
    }

    /*
     *sa.sa_handler = handler;                    
     *sa.sa_flags |= SA_RESTART;                             
     *sigfillset(&sa.sa_mask);                                                                                                                                                     
     */

    if (sigaction(evsignal, &sa, NULL) == -1) {
        return (-1);
    }


    eb->evsig.fd_snd = fds[0];
    eb->evsig.fd_rcv = fds[1];


    /*
     *char command = 'c';//c --> change state
     *if (write(fds[1], &command, 1) != 1) {
     *    dbg_str(SM_WARNNING,"concurrent_master_notify_slave,write pipe err");
     *}
     */
}


int called = 0;

    static void
signal_cb(int fd, short event, void *arg)
{
    struct event *signal = arg;

    printf("%s: got signal %d\n", __func__,fd);

/*
 *    if (called >= 2)
 *        event_del(signal);
 *
 *    called++;
 */
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

