/*
 * XXX This sample code was once meant to show how to use the basic Libevent
 * interfaces, but it never worked on non-Unix platforms, and some of the
 * interfaces have changed since it was first written.  It should probably
 * be removed or replaced with something better.
 *
 * Compile with:
 * cc -I/usr/local/include -o event-test event-test.c -L/usr/local/lib -levent
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/event/select_base.h>

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
    eb->add(eb, &event);

    eb->loop(eb);

    pause();
    object_destroy(eb);
}
