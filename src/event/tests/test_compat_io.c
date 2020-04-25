#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libobject/event/event_compat.h>

static void
fifo_read(int fd, short event, void *arg)
{
    char buf[255];
    int len;
    struct event *ev = arg;

    /* Reschedule this event */
    event_add(ev, NULL);

    dbg_str(DBG_VIP, "fifo_read called with fd: %d, event: %d, arg: %p",
        (int)fd, event, arg);
    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
        perror("read");
        return;
    } else if (len == 0) {
        fprintf(stderr, "Connection closed\n");
        return;
    }

    printf("Read: %s", buf);
}

static void
pipe_read(int fd, short event, void *arg)
{
    char buf[255];
    int len;
    struct event *ev = arg;

    /* Reschedule this event */
    event_add(ev, NULL);

    dbg_str(DBG_VIP, "pipe_read called with fd: %d, event: %d, arg: %p\n",
        (int)fd, event, arg);
    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
        perror("read");
        return;
    } else if (len == 0) {
        fprintf(stderr, "Connection closed\n");
        return;
    }

    dbg_str(DBG_DETAIL,"Read: %s", buf);
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
static int create_pipe(int fds[2])
{
    if (pipe(fds)) {
        dbg_str(SM_ERROR,"cannot create pipe");
        return -1;
    }

    return 0;
}

int test_sigle_io()
{
    struct event evfifo;
    int socket;
    struct event_base *event_base;
    struct timeval tv;

    socket = create_fifo((char *)"event.fifo");
    /* Initalize the event library */
    event_base = event_base_new();

    /* Initalize one event */
    event_assign(&evfifo,event_base, socket, EV_READ | EV_PERSIST, fifo_read, &evfifo);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    event_add(&evfifo, &tv);

    event_base_dispatch(event_base);

    event_base_free(event_base);

    return (0);
}

int test_multi_ios()
{
    struct event evfifo1, evfifo2, evpipe;
    int fds[2] = {-1};
    int socket1, socket2;
    struct event_base *event_base;
    struct timeval tv;

    socket1 = create_fifo((char *)"event.fifo1");
    socket2 = create_fifo((char *)"event.fifo2");

    create_pipe(fds);

    /* Initalize the event library */
    event_base = event_base_new();

    /* Initalize one event */
    event_assign(&evfifo1,event_base, socket1, EV_READ | EV_PERSIST, fifo_read, &evfifo1);
    event_assign(&evfifo2,event_base, socket2, EV_READ | EV_PERSIST, fifo_read, &evfifo2);
    event_assign(&evpipe,event_base, fds[1], EV_READ | EV_PERSIST, pipe_read, &evpipe);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    event_add(&evfifo1, &tv);
    event_add(&evfifo2, &tv);
    event_add(&evpipe, &tv);

    event_base_dispatch(event_base);

    event_base_free(event_base);

    return (0);
}
int test_io()
{
    test_sigle_io();
    /*
     *test_multi_ios();
     */
}
