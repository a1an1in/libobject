#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/concurrent/worker_api.h>
#include <libobject/mockery/mockery.h>

static int peroid_timer_counter;

static void test_work_callback(void *task)
{
    peroid_timer_counter++;
    dbg_str(DBG_INFO, "test_work_callback, peroid_timer_counter:%d", peroid_timer_counter);
}

int test_worker_peroid_timer(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Worker *worker;
    struct timeval ev_tv;
    int ret;

    TRY {
        sleep(1);

        ev_tv.tv_sec  = 2;
        ev_tv.tv_usec = 0;

        dbg_str(DBG_INFO, "opaque addr:%p", &ev_tv);
        worker = timer_worker(allocator, EV_READ | EV_PERSIST, &ev_tv, test_work_callback, &ev_tv);
        gettimeofday(&worker->lasttime, NULL);

        sleep(10);
        THROW_IF(peroid_timer_counter < 4, -1);
    } CATCH (ret) {} FINALLY {
        worker_destroy(worker);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_worker_peroid_timer);


#if (!defined(WINDOWS_USER_MODE))
static Worker *test_write_worker = NULL;
static int test_pipe_fds[2];
static int test_blocked_count = 0;      /* number of times write blocked */

static void test_write_callback(int fd, short event, void *arg)
{
    ssize_t n;
    char buf[4096];

    dbg_str(DBG_INFO, "test_write_callback fd=%d event=%s",
            fd, (event & EV_WRITE) ? "EV_WRITE" : "OTHER");

    if (event & EV_WRITE) {
        /* Keep writing until pipe buffer is full */
        while (1) {
            memset(buf, 'X', 1024);
            n = write(fd, buf, 1024);
            if (n > 0) {
                /* Data written, continue */
            } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                /* Pipe buffer full, write would block */
                dbg_str(DBG_INFO, "Pipe buffer full, write blocked");
                test_blocked_count++;
                break;
            } else {
                perror("write");
                break;
            }
        }
    }
}

int test_worker_adjust_event()
{
    allocator_t *allocator = allocator_get_default_instance();
    int ret = 0;
    int flags;
    char read_buf[8192];
    ssize_t n;

    TRY {
        /* Reset static variable */
        test_blocked_count = 0;

        /* Create a pipe */
        ret = pipe(test_pipe_fds);
        if (ret < 0) {
            perror("pipe");
            return;
        }

        /* Set write end non-blocking */
        flags = fcntl(test_pipe_fds[1], F_GETFL, 0);
        fcntl(test_pipe_fds[1], F_SETFL, flags | O_NONBLOCK);

        /* Set read end non-blocking */
        flags = fcntl(test_pipe_fds[0], F_GETFL, 0);
        fcntl(test_pipe_fds[0], F_SETFL, flags | O_NONBLOCK);

        /* Try to reduce pipe buffer size to make it easier to fill */
#ifdef F_SETPIPE_SZ
        fcntl(test_pipe_fds[0], F_SETPIPE_SZ, 4096);
        fcntl(test_pipe_fds[1], F_SETPIPE_SZ, 4096);
#endif

        /* Create worker monitoring write fd for EV_WRITE events */
        test_write_worker = io_worker(allocator, test_pipe_fds[1], EV_READ | EV_PERSIST, NULL, NULL,
                                      test_write_callback, NULL, NULL);

        /* Adjust to EV_WRITE | EV_PERSIST (only once) */
        test_write_worker->adjust(test_write_worker, EV_WRITE | EV_PERSIST);

        while (test_blocked_count == 0) {
            usleep(500000); /* 0.5 second */
            dbg_str(DBG_INFO, "Test started, writing to pipe until full...");
        }

        /* Read some data to make pipe writable again */
        n = read(test_pipe_fds[0], read_buf, sizeof(read_buf));
        if (n > 0) {
            dbg_str(DBG_INFO, "Read %zd bytes to create space", n);
        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            dbg_str(DBG_INFO, "No data available to read");
        } else {
            perror("read");
        }

        /* Wait for write callback to be triggered again and possibly block again */
        usleep(300000);

        /* Second read to further drain pipe */
        n = read(test_pipe_fds[0], read_buf, sizeof(read_buf));
        if (n > 0) {
            dbg_str(DBG_INFO, "Second read %zd bytes", n);
        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            dbg_str(DBG_INFO, "No more data to read");
        } else {
            perror("read");
        }

        /* Wait for write callback again */
        usleep(300000);
        THROW_IF(test_blocked_count != 3, -1);
    } CATCH (ret) {
        dbg_str(DBG_WARN, "Write blocked only %d times (expected ==3)", test_blocked_count);
    } FINALLY {
        /* Cleanup */
        worker_destroy(test_write_worker);
        usleep(1000);
        close(test_pipe_fds[0]);
        close(test_pipe_fds[1]);
        test_write_worker = NULL;
    }
    
    return ret;
}
REGISTER_TEST_FUNC(test_worker_adjust_event);
#endif