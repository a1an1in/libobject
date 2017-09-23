#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libobject/utils/concurrent/concurrent.h>

typedef struct concurrent_task_s{
    uint32_t conn;
    char key[10];
    concurrent_ta_t *task_admin;
}concurrent_task_t;

void work(concurrent_slave_t *slave,void *arg)
{
    hash_map_pos_t pos;
    concurrent_task_t *task = (concurrent_task_t *)arg;

    dbg_str(DBG_DETAIL,"work done,rev conn =%d,task addr=%p,task key %s",task->conn,task,task->key);
    concurrent_ta_search(slave->task_admin,(void *)task->key,&pos);
    if(pos.hlist_node_p != NULL)
        concurrent_ta_del(slave->task_admin,&pos);
    dbg_str(DBG_DETAIL,"run at here");
}

int test_concurrent1()
{
    allocator_t *allocator;
    concurrent_master_t *master;
    concurrent_task_t task1,task2,task3,task4;
    concurrent_task_t  *task;
    struct concurrent_message_s message1,message2,message3,message4;
    struct concurrent_message_s *message;
    int ret = 0;


    allocator = allocator_create(ALLOCATOR_TYPE_SYS_MALLOC,0);
    master = concurrent_master_create(allocator);
    concurrent_master_init(master,
            SERVER_WORK_TYPE_THREAD,//uint8_t concurrent_work_type,
            sizeof(concurrent_task_t),
            2);

    task1.conn = 1;
    task1.task_admin = master->task_admin;
    memcpy(task1.key,"1111",master->task_admin->key_size);
    concurrent_master_init_message(&message1, work,&task1,1);
    task2.conn = 2;
    task2.task_admin = master->task_admin;
    memcpy(task1.key,"2222",master->task_admin->key_size);
    concurrent_master_init_message(&message2, work,&task2,2);
    task3.conn = 3;
    task3.task_admin = master->task_admin;
    memcpy(task1.key,"3333",master->task_admin->key_size);
    concurrent_master_init_message(&message3, work,&task3,3);
    task4.conn = 4;
    task4.task_admin = master->task_admin;
    memcpy(task1.key,"4444",master->task_admin->key_size);
    concurrent_master_init_message(&message4, work,&task4,4);
    dbg_str(DBG_DETAIL,"task1=%p,task2= %p,task3=%p,task4=%p",&task1,&task2,&task3,&task4); 

    task = &task1;
    concurrent_ta_add(master->task_admin,(void *)task->key,task);
    task = &task2;
    concurrent_ta_add(master->task_admin,(void *)task->key,task);
    task = &task3;
    concurrent_ta_add(master->task_admin,(void *)task->key,task);
    task = &task4;
    concurrent_ta_add(master->task_admin,(void *)task->key,task);

    message = &message1;
    concurrent_master_add_message(master,message);
    message = &message2;
    concurrent_master_add_message(master,message);
    message = &message3;
    concurrent_master_add_message(master,message);
    message = &message4;
    concurrent_master_add_message(master,message);

    pause();
    return ret;
}
int test_concurrent_add_task(concurrent_master_t *master,void *task)
{
    struct concurrent_message_s message;

    concurrent_ta_add(master->task_admin,(void *)(((concurrent_task_t *)task)->key),task);
    concurrent_master_init_message(&message, work,task,0);
    concurrent_master_add_message(master,&message);

    return 0;
}
int test_concurrent_init_task(concurrent_task_t *task,int32_t conn,void *key,int8_t key_size)
{
    task->conn = conn;
    memcpy(task->key,key,key_size);
}
void concurrent_master_process_listen_event(int fd, short event, void *arg)
{
    concurrent_master_t *master = (concurrent_master_t *)arg;

    char buf[1];          
    dbg_str(DBG_VIP,"concurrent_master_process_listen_event");
    if (read(fd, buf, 1) != 1){
        dbg_str(DBG_WARNNING,"cannot read form pipe");
        return;
    }
    concurrent_task_t task;
    test_concurrent_init_task(&task,fd,"1111",strlen("1111"));
    test_concurrent_add_task(master,&task);
}
int test_concurrent3()
{
    allocator_t *allocator;
    concurrent_master_t *master;
    int ret = 0;
    struct event event;


    allocator = allocator_create(ALLOCATOR_TYPE_SYS_MALLOC,0);

    dbg_str(DBG_DETAIL,"run at here");
    master = concurrent_master_create(allocator);
    concurrent_master_init(master,
            SERVER_WORK_TYPE_THREAD,//uint8_t concurrent_work_type,
            sizeof(concurrent_task_t),
            2);

    int fds[2];
    if(pipe(fds)) {
        dbg_str(DBG_ERROR,"cannot create pipe");
        exit(1);
    }


    sleep(1);
    event_assign(&master->event,master->event_base,fds[0], EV_READ | EV_PERSIST, concurrent_master_process_listen_event, master);
    if (event_add(&master->event, 0) == -1) {
        dbg_str(DBG_WARNNING,"event_add err");
    }


    if (write(master->snd_add_new_event_fd, "a", 1) != 1) {
        dbg_str(DBG_ERROR,"cannot write pipe");
    }

    if (write(fds[1], "c", 1) != 1) {
        dbg_str(DBG_ERROR,"cannot write pipe");
    }
    pause();

    close(fds[0]);
    close(fds[1]);

    return ret;
}
