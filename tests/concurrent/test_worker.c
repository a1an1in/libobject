#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/concurrent/Producer.h>

static void test_work_callback(void *task)
{
    dbg_str(DBG_SUC, "test_work_callback");
}

int test_peroid_timer_worker(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Worker *worker;
    struct timeval ev_tv;

    sleep(1);

    gettimeofday(&lasttime, NULL);
    ev_tv.tv_sec  = 2;
    ev_tv.tv_usec = 0;

    dbg_str(DBG_SUC, "opaque addr:%p", &ev_tv);
    worker = timer_worker(allocator, EV_READ | EV_PERSIST, &ev_tv, test_work_callback, &ev_tv);

    sleep(10);
    worker_destroy(worker);
    sleep(5);

    return 1;
}
REGISTER_TEST_CMD(test_peroid_timer_worker);


#if (!defined(WINDOWS_USER_MODE))
static void
test_pipe_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    char buf[255];
    int len;

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

    worker->work_callback(NULL);

    return;
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

void test_obj_io_worker()
{
    allocator_t *allocator = allocator_get_default_instance();
    Worker *worker;
    int fd;

    sleep(1);

    fd = create_fifo((char *)"event.fifo");
    worker = io_worker(allocator, fd, NULL, NULL, test_pipe_ev_callback, 
                       test_work_callback, NULL);
    pause();
    pause();
    worker_destroy(worker);
}
#endif