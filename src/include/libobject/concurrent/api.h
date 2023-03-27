#ifndef __LIBOBJECT_CONCURRENT_INIT_H__
#define __LIBOBJECT_CONCURRENT_INIT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/core/io/Socket.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/concurrent/event/event.h>

Worker *io_worker(allocator_t *allocator, int fd, 
          struct timeval *ev_tv, Producer *producer, void *ev_callback, 
          void *work_callback, void *opaque);
        
Worker *signal_worker(allocator_t *allocator, 
                      int fd, void *work_callback, void *opaque);

Worker *timer_worker(allocator_t *allocator, int ev_events, struct timeval *ev_tv, 
                      void *work_callback, void *opaque);                      

int worker_destroy(Worker *worker);

extern struct timeval lasttime;

#endif
