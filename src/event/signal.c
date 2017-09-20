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

static void
evsig_cb(int fd, short what, void *arg)
{
    static char signals[1024];
    int n;
    int i;
    int ncaught[NSIG];
    struct event_base *base;

    base = arg;

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

int tes_event_io()
{
    struct stat st;
    int socket;
    Event_Base *eb;
    const char *fifo = "event.fifo";
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];
    cjson_t *root, *e, *s;
    event_t event;

    eb = OBJECT_NEW(allocator, Select_Base, NULL);

    dbg_str(DBG_DETAIL,"run at here, eb=%p", eb);

    object_dump(eb, "Select_Base", buf, 2048);
    dbg_str(DBG_DETAIL,"Select_Base dump: %s",buf);

    if (lstat(fifo, &st) == 0) {
        if ((st.st_mode & S_IFMT) == S_IFREG) {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }

    unlink(fifo);
    if (mkfifo(fifo, 0600) == -1) {
        perror("mkfifo");
        exit(1);
    }

    socket = open(fifo, O_RDWR | O_NONBLOCK, 0);
    if (socket == -1) {
        perror("open");
        exit(1);
    }


    event.ev_fd = socket;
    event.ev_events = EV_READ;
    event.ev_timeout.tv_sec  = 60;
    event.ev_timeout.tv_usec = 0;
    event.ev_callback = evsig_cb;

    /*
     *dbg_str(DBG_SUC,"at main, base addr:%p, map addr :%p, search:%p, timer:%p",
     *        eb, eb->map, eb->search, eb->timer);
     */
    dbg_str(DBG_SUC,"event addr:%p", &event);

    eb->add(eb, &event);

    eb->loop(eb);

    object_destroy(eb);
}
