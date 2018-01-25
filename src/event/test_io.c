#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
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

static int create_fifo(char *name) 
{
    struct stat st;
    int socket;

    if (lstat(name, &st) == 0) {
        if ((st.st_mode & S_IFMT) == S_IFREG) {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }

    unlink(name);
    if (mkfifo(name, 0600) == -1) {
        perror("mkname");
        exit(1);
    }

    /* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
    socket = open(name, O_RDWR | O_NONBLOCK, 0);

    if (socket == -1) {
        perror("open");
        exit(1);
    }
}

int test_event_io()
{
    int socket;
    Event_Base *eb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];
    cjson_t *root, *e, *s;
    event_t event;

    dbg_str(DBG_SUC,"Event IO, in test");
    socket = create_fifo((char *)"event.fifo");

    eb = OBJECT_NEW(allocator, Select_Base, NULL);

    /*
     *object_dump(eb, "Select_Base", buf, 2048);
     *dbg_str(DBG_DETAIL,"Select_Base dump: %s",buf);
     */

    event.ev_fd              = socket;
    event.ev_events          = EV_READ | EV_PERSIST;
    event.ev_timeout.tv_sec  = 60;
    event.ev_timeout.tv_usec = 0;
    event.ev_callback        = test_ev_callback;
    event.ev_arg             = &event;

    eb->add(eb, &event);

    dbg_str(DBG_SUC,"Event IO, loop before");
    eb->loop(eb);
    dbg_str(DBG_SUC,"Event IO, out loop");
    object_destroy(eb);
    /*
     *pause();
     */
}
