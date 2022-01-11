
/**
 * @file Stun_Udp.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Stun_Udp.h"

static int __stun_client_resp_callback(void *task);

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
        stun->c = client(allocator, CLIENT_TYPE_INET_UDP, 
                (char *)"0.0.0.0", //char *host, 
                (char *)"9090", //char *client_port, 
                __stun_client_resp_callback, stun);
        THROW_IF(stun->c == NULL, -1);
        EXEC(client_connect(stun->c, host, service));
        dbg_str(DBG_DETAIL, "client stun:%p", stun);
    } CATCH (ret) {
    }

    return ret;
}

int __send(Stun_Udp *stun)
{
    Request *req = stun->parent.req;
    Map *map = req->attribs;
    int i = 0;
    int ret = 0;

    TRY {
        if (map->count(map) == 0) {
            req->len = sizeof(stun_header_t);
        } else {
        }

        dbg_str(DBG_DETAIL, "stun send: atttrib count:%d, len%d", map->count(map), req->len);
        client_send(stun->c, req->header, req->len, 0);
    } CATCH (ret) {
    }

    return ret;
}

static int __stun_bind_request_read_post_process(Request * request, void *opaque)
{

    dbg_str(DBG_DETAIL, "__stun_bind_request_read_post_process");
    return 0;
}

int __discovery(Stun_Udp *stun)
{
    Request *req = stun->parent.req;

    req->set_head(req, STUN_BINDREQ, 0, 0x2112A442);
    stun->set_read_post_callback(stun, __stun_bind_request_read_post_process);
    stun->send(stun);
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Stun, parent),
    Init_Nfunc_Entry(1, Stun_Udp, construct, __construct),
    Init_Nfunc_Entry(2, Stun_Udp, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun_Udp, connect, __connect),
    Init_Vfunc_Entry(4, Stun_Udp, discovery, __discovery),
    Init_Vfunc_Entry(5, Stun_Udp, send, __send),
    Init_Vfunc_Entry(6, Stun_Udp, set_read_post_callback, NULL),
    Init_End___Entry(7, Stun_Udp),
};
REGISTER_CLASS("Stun_Udp", stun_class_info);

static int __stun_client_resp_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Stun_Udp *stun = t->opaque;
    Response *resp;
    int ret = 0, len = 0;

    TRY {
        THROW_IF(stun == NULL, -1);
        resp = stun->parent.response;

        while (len != t->buf_len) {
            ret = resp->buffer->write(resp->buffer, t->buf + len, t->buf_len - len);
            if (ret >= 0) {
                len += ret;
                dbg_str(DBG_DETAIL,"task buffer len=%d, write ring buffer len=%d, total wrote len=%d",
                        t->buf_len, ret, len);
            }

            ret = resp->read(resp);
            THROW_IF(ret < 0, -1);
            if (ret == 1) {
                resp->read_post_callback(resp, stun);
                break;
            }
        } 
        dbg_str(DBG_DETAIL,"recv buf: len:%d", t->buf_len);
    } CATCH (ret) {
    }

    return ret;
}
static int test_stun_udp(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Stun *stun = NULL;
    char *str = "hello world";
    int ret;

    dbg_str(NET_DETAIL, "test_stun_udp");

    TRY {
        stun = object_new(allocator, "Stun_Udp", NULL);
        stun->connect(stun, "stun.voipstunt.com", "3478");
        /*
         *EXEC(stun->connect(stun, "77.72.169.213", "3478"));
         */
        /*
         *EXEC(stun->connect(stun, "75.2.81.221", "3478"));
         */
        /*
         *EXEC(stun->connect(stun, "77.72.174.163", "3478"));
         */
        EXEC(stun->discovery(stun));
        pause();

    } CATCH (ret) {
    }
    object_destroy(stun);
}
REGISTER_TEST_CMD(test_stun_udp);

