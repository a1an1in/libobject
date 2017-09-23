#include <fcntl.h>              
#include <unistd.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/data_structure/vector.h>
#include <libobject/utils/concurrent/io_user.h>
#include <libobject/utils/concurrent/tmr_user.h>

static void test_pipe_event_handler(int fd, short event, void *arg)
{
    char buf;
    dbg_str(SM_DETAIL,"test_pipe_event_handler");
    if (read(fd, &buf, 1) != 1) {
        dbg_str(SM_WARNNING,"read pipe err");
    }
}
void test_io_user()
{
    allocator_t *allocator;

    if((allocator = allocator_create(ALLOCATOR_TYPE_SYS_MALLOC,0) ) == NULL){
        dbg_str(SM_ERROR,"proxy_create allocator_create err");
        return ;
    }
    /*
     *s = state_machine(allocator, entry_config,NULL);
     *state_machine_change_state(s, 1);
     */

    int fds[2];
    if(pipe(fds)) {
        dbg_str(SM_ERROR,"cannot create pipe");
        return ;
    }
    io_user(allocator,//allocator_t *allocator,
            fds[0],//int user_fd,
            0,//user_type
            test_pipe_event_handler,//void (*user_event_handler)(int fd, short event, void *arg),
            NULL,//void (*slave_work_function)(concurrent_slave_t *slave,void *arg),
            NULL,//int (*process_task_cb)(user_task_t *task),
            NULL);//void *opaque)
    char command = 'c';//c --> change state
    if (write(fds[1], &command, 1) != 1) {
        dbg_str(SM_WARNNING,"concurrent_master_notify_slave,write pipe err");
    }
    pause();
}
