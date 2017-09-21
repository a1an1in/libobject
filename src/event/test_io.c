#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/event/select_base.h>

static void test_ev_callback(int fd, short events, void *arg)
{
    char buf[255];
    int len;

    dbg_str(DBG_SUC,"hello world, event");
    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
        perror("read");
        return;
    } else if (len == 0) {
        fprintf(stderr, "Connection closed\n");
        return;
    }

    buf[len] = '\0';
    fprintf(stdout, "Read: %s\n", buf);
}
int test_event_io()
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
    event.ev_events = EV_READ | EV_PERSIST;
    event.ev_timeout.tv_sec  = 60;
    event.ev_timeout.tv_usec = 0;
    event.ev_callback = test_ev_callback;

    /*
     *dbg_str(DBG_SUC,"at main, base addr:%p, map addr :%p, search:%p, timer:%p",
     *        eb, eb->map, eb->search, eb->timer);
     */
    dbg_str(DBG_SUC,"event addr:%p", &event);

    eb->add(eb, &event);

    eb->loop(eb);

    object_destroy(eb);
}
