#ifndef __WORK_TASK_H__
#define __WORK_TASK_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/io/Socket.h>

#define WORKER_TASK_MAX_BUF_LEN 1024 * 10

typedef struct work_task_s{
    void *buf;
    int buf_len;
    int fd;
    void *socket;
    void *opaque;
    void *cache;
    allocator_t *allocator;
    short event;
    // char name[24];
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
        dbg_str(DBG_ERROR,"net task buf alloc error, len=%d", len);
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
    if (task == NULL) return 0;
    allocator_mem_free(task->allocator, task->buf);
    allocator_mem_free(task->allocator, task);

    return 1;
}

#endif
