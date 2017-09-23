#ifndef __USER_H__
#define __USER_H__

#include <libobject/utils/concurrent/concurrent.h>


typedef struct io_user_s{
	struct event event;
	int user_fd;
	void (*slave_work_function)(concurrent_slave_t *slave,void *arg);
	void (*io_event_handler)(int fd, short event, void *arg);
	int (*process_task_cb)(void *task);
	void *opaque;
	concurrent_master_t *master;
	allocator_t *allocator;
	void *local_sockaddr;
	void *remote_sockaddr;
	uint8_t user_type;
#define MAX_TYPE_STR_LEN 50
    char type_str[MAX_TYPE_STR_LEN];
    char unix_path[MAX_TYPE_STR_LEN];
#undef MAX_TYPE_STR_LEN
}io_user_t;


io_user_t *io_user(
		allocator_t *allocator,
		int user_fd,
		uint8_t user_type,
		void (*io_event_handler)(int fd, short event, void *arg),
		void (*slave_work_function)(concurrent_slave_t *slave,void *arg),
		int (*process_task_cb)(void *task),
		void *opaque);

int io_user_destroy(io_user_t *io_user);

#endif
