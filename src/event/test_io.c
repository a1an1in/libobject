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
    dbg_str(DBG_SUC,"hello world, event");
}
int test_event_io()
{
    struct stat st;
    const char *fifo = "event.fifo";
    int socket;

    Event_Base *eb;
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
    event.ev_callback = test_ev_callback;

    eb->add(eb, &event);

    eb->loop(eb);

    object_destroy(eb);
}
