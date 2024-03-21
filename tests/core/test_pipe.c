#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/io/Pipe.h>
#include <libobject/concurrent/worker_api.h>

static int create_pipe(int fds[2])
{
    // if (pipe(fds)) {
    //     dbg_str(SM_ERROR,"cannot create pipe");
    //     return -1;
    // }

    return 0;
}

static void test_work_callback(void *task)
{
    dbg_str(DBG_SUC, "test_work_callback");
}

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

static int test_pipe(TEST_ENTRY *entry, void *argc, void *argv)
{
    int fds[2];
    allocator_t *allocator = allocator_get_default_instance();
    Worker *worker;
    char *str = "hello world";
    char buffer[1024] = {0};
    Pipe *pipe;
    int ret;

    TRY {
        dbg_str(DBG_VIP,"test_pipe in");

        pipe = object_new(allocator, "Windows_Pipe", NULL);
        EXEC(pipe->init(pipe));
        pipe->write(pipe, str, strlen(str));
        pipe->read(pipe, buffer, 1024);
        dbg_str(DBG_VIP,"Pipe read out:%s", buffer);
        THROW_IF(strcmp(str, buffer) != 0, -1);
        // worker = io_worker(allocator, fds[1], NULL, NULL, test_pipe_ev_callback, 
        //                    test_work_callback, NULL);
    } CATCH (ret) {} FINALLY {
        object_destroy(pipe);
    }

    return ret;
}
REGISTER_TEST_CMD(test_pipe);
