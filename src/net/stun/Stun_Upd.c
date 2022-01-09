
/**
 * @file Stun_Udp.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Stun_Udp.h"

static int __stun_client_response_callback(void *task);

static int __construct(Stun_Udp *stun, char *init_str)
{
    return 0;
}

static int __deconstruct(Stun_Udp *stun)
{
    object_destroy(stun->c);
    return 0;
}

static int __connect(Stun_Udp *stun, char *host, char *service)
{
    allocator_t *allocator = stun->parent.parent.allocator;
    int ret = 0;

    TRY {
        stun->c = client(allocator, CLIENT_TYPE_INET_TCP, 
                (char *)"127.0.0.1", //char *host, 
                (char *)"19900", //char *client_port, 
                __stun_client_response_callback, NULL);
        THROW_IF(stun->c == NULL, -1);
        EXEC(client_connect(stun->c, host, service));
    } CATCH (ret) {
    }

    return ret;
}

int __send(Stun_Udp *stun, void *buf, int len, int flags)
{
    return stun->c->send(stun->c, buf, len, flags); 
}

int __discovery(Stun_Udp *stun)
{
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Stun, parent),
    Init_Nfunc_Entry(1, Stun_Udp, construct, __construct),
    Init_Nfunc_Entry(2, Stun_Udp, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun_Udp, connect, __connect),
    Init_Vfunc_Entry(4, Stun_Udp, discovery, __discovery),
    Init_Vfunc_Entry(5, Stun_Udp, send, __send),
    Init_End___Entry(6, Stun_Udp),
};
REGISTER_CLASS("Stun_Udp", stun_class_info);

static int __stun_client_response_callback(void *task)
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

    client_send(c, str, strlen(str), 0);
    pause();

    object_destroy(c);
}
REGISTER_TEST_CMD(test_stun_udp);

