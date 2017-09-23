#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_compat.h>
#include "libobject/utils/data_structure/link_list.h"
#include "libobject/utils/data_structure/hash_list.h"

enum SERVER_WORK_TYPE{
	SERVER_WORK_TYPE_THREAD = 0,
	SERVER_WORK_TYPE_PROCESS
};

typedef struct concurrent_master_s concurrent_master_t;

typedef struct concurrent_ta_s{
	sync_lock_t admin_lock;
	uint8_t admin_lock_type;
	uint8_t hmap_lock_type;
	hash_map_t *hmap;
	uint32_t task_count;
	allocator_t *allocator;
	uint8_t key_size;
	uint32_t value_size;
	uint32_t bucket_size;
}concurrent_ta_t;

typedef struct concurrent_slave_s{
	struct event_base *event_base;
	struct event message_event;
	allocator_t *allocator;
	int rcv_notify_fd;
	union{
		pthread_t tid;
		pid_t pid;
	}id;
	uint8_t work_id;
	llist_t *message_que;
	concurrent_ta_t *task_admin;
    concurrent_master_t *master;
}concurrent_slave_t;

struct concurrent_message_s{
#define MAX_TASK_KEY_LEN 10
	void *task;
	void *event;
    struct event_base *ev_base;
    int ev_fd;
    short ev_events;
    void *ev_callback;
    void *ev_callback_arg;
    struct timeval *tv;
	void (*work_func)(concurrent_slave_t *slave,void *arg);
	uint32_t message_id;
#undef MAX_TASK_KEY_LEN
};

typedef struct concurrent_master_s{
	struct event_base *event_base;
	struct event event;
	uint8_t concurrent_work_type;
	allocator_t *allocator;
	llist_t *message_que;
	llist_t *new_ev_que;
	int *snd_notify_fd;
	concurrent_slave_t *slave;
	uint8_t assignment_count;
	uint8_t slave_amount;
	uint32_t message_count;
	concurrent_ta_t *task_admin;
	union{
		pthread_t tid;
		pid_t pid;
	}id;
	int rcv_add_new_event_fd;
	int snd_add_new_event_fd;
	uint8_t concurrent_master_inited_flag;
	uint8_t concurrent_slave_inited_flag;
}concurrent_master_t;

typedef struct concurrent_s{
	struct event new_event;
	uint8_t concurrent_work_type;
	allocator_t *allocator;
	llist_t *new_ev_que;
    /*
	 *llist_t *del_ev_que;
     */
	int snd_add_new_event_fd;
	concurrent_master_t *master;
	sync_lock_t concurrent_lock;
}concurrent_t;

concurrent_ta_t *concurrent_ta_create(allocator_t *allocator);
int concurrent_ta_init(concurrent_ta_t *task_admin, uint8_t key_size,uint32_t data_size,uint32_t bucket_size, uint8_t admin_lock_type,uint8_t hmap_lock_type);
int concurrent_ta_add(concurrent_ta_t *task_admin,void *key,void *data);
int concurrent_ta_search(concurrent_ta_t *task_admin,void *key,hash_map_pos_t *pos);
int concurrent_ta_del(concurrent_ta_t *task_admin,hash_map_pos_t *pos);
int concurrent_ta_destroy(concurrent_ta_t *task_admin);
concurrent_master_t *concurrent_master_create(allocator_t *allocator);
void *concurrent_slave_thread(void *arg);
int __concurrent_create_slave(concurrent_master_t *master,uint8_t slave_id);
int concurrent_master_create_slaves(concurrent_master_t *master);
int concurrent_master_destroy_slaves(concurrent_master_t *master);
int concurrent_master_add_message(concurrent_master_t *master,struct concurrent_message_s *message);
int concurrent_master_init_message(struct concurrent_message_s *message, void (*work_func)(concurrent_slave_t *slave,void *arg), void *task, int32_t message_id);
int concurrent_master_destroy(concurrent_master_t *master);
int concurrent_master_init(concurrent_master_t *master, uint8_t concurrent_work_type, uint32_t task_size, uint8_t slave_amount);
void concurrent_master_process_event(int fd, short event, void *arg);
int concurrent_slave_add_new_event(concurrent_slave_t *slave, int fd,int event_flag, struct event *event, void (*event_handler)(int fd, short event, void *arg), void *task);
int concurrent_master_add_new_event(concurrent_master_t *master, int fd,int event_flag, struct event *event, void (*event_handler)(int fd, short event, void *arg), void *arg);
int concurrent_ta_del_by_key(concurrent_ta_t *task_admin, void *key);
int concurrent_master_choose_slave(concurrent_master_t *master);
int concurrent_master_add_task_and_message(concurrent_master_t *master, void *task,void *key, void (*work_func)(concurrent_slave_t *slave,void *arg));
void *concurrent_master_add_task(concurrent_master_t *master, void *task,void *key);

concurrent_t *concurrent_create(allocator_t *allocator);
int concurrent_init(concurrent_t *c, uint8_t concurrent_work_type, uint32_t task_size, uint8_t slave_amount, uint8_t concurrent_lock_type);
int concurrent_add_event_to_master(concurrent_t *c,
                                   int fd,
                                   int event_flag,
                                   struct event *event,
                                   struct timeval *tv,
                                   void (*event_handler)(int fd, short event, void *arg),
                                   void *arg);
int concurrent_del_event_of_master(concurrent_t *c,
		                           struct event *event);
void concurrent_destroy(concurrent_t *c);

concurrent_t *concurrent_get_global_concurrent_addr();

#endif
