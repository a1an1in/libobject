#ifndef __NET_TASK_H__
#define __NET_TASK_H__

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/allocator.h>

typedef struct net_task_s{
    void *buf;
    int buf_len;
    allocator_t *allocator;
    int fd;
    void *opaque;
} net_task_t;

static inline net_task_t *
net_task_alloc(allocator_t *allocator, int len)
{
    net_task_t *task;

    task = (net_task_t *)allocator_mem_alloc(allocator, sizeof(net_task_t));
    if (task == NULL) {
        dbg_str(DBG_ERROR,"net task alloc error");
        return NULL;
    }

    task->buf = allocator_mem_alloc(allocator, len);
    if (task->buf == NULL) {
        dbg_str(DBG_ERROR,"net task buf alloc error");
        allocator_mem_free(allocator, task);
        return NULL;
    }

    task->buf_len = len;
    task->allocator = allocator;

    return task;
}

static inline int net_task_free(net_task_t *task)
{
    allocator_mem_free(task->allocator, task->buf);
    allocator_mem_free(task->allocator, task);

    return 0;
}

#endif
