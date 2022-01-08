
/**
 * @file Stun_Udp.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Stun_Udp.h"

static int __construct(Stun_Udp *stun, char *init_str)
{
    return 0;
}

static int __deconstruct(Stun_Udp *stun)
{
    return 0;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Stun, parent),
    Init_Nfunc_Entry(1, Stun_Udp, construct, __construct),
    Init_Nfunc_Entry(2, Stun_Udp, deconstruct, __deconstruct),
    Init_End___Entry(3, Stun_Udp),
};
REGISTER_CLASS("Stun_Udp", stun_class_info);

static int test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    dbg_str(DBG_DETAIL,"%s", t->buf);

}
static int test_stun_udp(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(NET_DETAIL, "test_obj_client_send");

    c = client(allocator, 
               CLIENT_TYPE_INET_TCP, 
               (char *)"127.0.0.1", //char *host, 
               (char *)"19900", //char *client_port, 
               test_work_callback, 
               NULL);
    client_connect(c, "127.0.0.1", "11011");
    client_send(c, str, strlen(str), 0);
    pause();

    object_destroy(c);
}
REGISTER_TEST_CMD(test_stun_udp);

