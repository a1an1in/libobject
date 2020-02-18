#ifndef __WORK_TASK_H__
#define __WORK_TASK_H__

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/os/Socket.h>

typedef struct work_task_s{
    void *buf;
    int buf_len;
    int fd;
    void *socket;
    void *opaque;
    void *request;
    allocator_t *allocator;
} work_task_t;

static inline work_task_t *
work_task_alloc(allocator_t *allocator, int len)
{
    work_task_t *task;

    task = (work_task_t *)allocator_mem_alloc(allocator, sizeof(work_task_t));
    if (task == NULL) {
        dbg_str(DBG_ERROR,"net task alloc error");
        return NULL;
    }

    memset(task, 0, sizeof(work_task_t));

    task->buf = allocator_mem_alloc(allocator, len);
    if (task->buf == NULL) {
        dbg_str(DBG_ERROR,"net task buf alloc error");
        allocator_mem_free(allocator, task);
        return NULL;
    }

    memset(task->buf, 0, len);

    task->buf_len = len;
    task->allocator = allocator;

    return task;
}

static inline int work_task_free(work_task_t *task)
{
    allocator_mem_free(task->allocator, task->buf);
    allocator_mem_free(task->allocator, task);

    return 0;
}

#endif
