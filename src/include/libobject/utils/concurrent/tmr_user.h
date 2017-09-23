#ifndef __EV_TIMER_H__
#define __EV_TIMER_H__

#include <libobject/utils/concurrent/concurrent.h>
#include <libobject/event/event_compat.h>

typedef struct tmr_user_s{
	struct event event;
    allocator_t *allocator;
    void (*tmr_event_handler)(int fd, short event, void *arg);
	void (*slave_work_function)(concurrent_slave_t *slave,void *arg);
	void (*process_timer_task_cb)(void *tmr);
    void *opaque;
    int32_t tmr_user_fd;
	concurrent_master_t *master;
	struct timeval tv;
    uint32_t flags;
}tmr_user_t;

tmr_user_t *tmr_user(allocator_t *allocator,
        struct timeval *tv,
        uint16_t timer_flags,
        void (*process_timer_task_cb)(void *task),
        void *opaque);
int tmr_user_destroy(tmr_user_t *tmr_user);
int tmr_user_stop(tmr_user_t *tmr_user);
tmr_user_t *tmr_user_reuse(tmr_user_t * tmr_user);
#endif
