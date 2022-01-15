
/**
 * @file Turn_Udp_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Turn_Udp_Client.h"

static int __turn_client_resp_callback(void *task);

static int __construct(Turn_Udp_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.parent.allocator;
    int ret = 0;

    TRY {
        turn->c = client(allocator, CLIENT_TYPE_INET_UDP, 
                         (char *)"0.0.0.0", //char *host, 
                         (char *)"9090", //char *client_port, 
                         __turn_client_resp_callback, turn);
        THROW_IF(turn->c == NULL, -1);
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Turn_Udp_Client *turn)
{
    object_destroy(turn->c);
    return 0;
}

static int __connect(Turn_Udp_Client *turn, char *host, char *service)
{
    return client_connect(turn->c, host, service);
}

static int __send(Turn_Udp_Client *turn)
{
    Request *req = turn->parent.req;
    Map *map = req->attribs;
    int i = 0;
    int ret = 0;

    TRY {
        if (map->count(map) == 0) {
            req->len = sizeof(turn_header_t);
        } else {
        }

        dbg_str(DBG_DETAIL, "turn send: atttrib count:%d, len%d", map->count(map), req->len);
        client_send(turn->c, req->header, req->len, 0);
    } CATCH (ret) {
    }

    return ret;
}

static int __turn_bind_request_read_post_process(Request * request, void *opaque)
{

    dbg_str(DBG_DETAIL, "__turn_bind_request_read_post_process");
    return 0;
}

static int __allocate_address(Turn_Udp_Client *turn)
{
    Request *req = turn->parent.req;
    nonce_t *nonce;
    requested_transport_t requested_transport = {0};

    req->set_head(req, TURN_METHOD_BINDREQ | TURN_METHOD_ALLOCATE, 0, 0x2112A442);
    requested_transport.protocol = 17;
    req->set_attrib(req, TURN_ATR_TYPE_REQUESTED_TRANSPORT, sizeof(requested_transport_t), &requested_transport);
    turn->set_read_post_callback(turn, __turn_bind_request_read_post_process);
    turn->send(turn);
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0, Turn, parent),
    Init_Nfunc_Entry(1, Turn_Udp_Client, construct, __construct),
    Init_Nfunc_Entry(2, Turn_Udp_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Turn_Udp_Client, connect, __connect),
    Init_Vfunc_Entry(4, Turn_Udp_Client, allocate_address, __allocate_address),
    Init_Vfunc_Entry(5, Turn_Udp_Client, send, __send),
    Init_Vfunc_Entry(6, Turn_Udp_Client, set_read_post_callback, NULL),
    Init_End___Entry(7, Turn_Udp_Client),
};
REGISTER_CLASS("Turn_Udp_Client", turn_class_info);

static int __turn_client_resp_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Turn_Udp_Client *turn = t->opaque;
    Response *resp;
    int ret = 0, len = 0;

    TRY {
        THROW_IF(turn == NULL, -1);
        resp = turn->parent.response;

        dbg_str(DBG_DETAIL,"recv buf: len:%d", t->buf_len);

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
                resp->read_post_callback(resp, turn);
                break;
            }
        } 
    } CATCH (ret) {
    }

    return ret;
}
static int test_turn_udp(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Turn_Client *turn = NULL;
    char *str = "hello world";
    int ret;

    dbg_str(NET_DETAIL, "test_turn_udp");

    TRY {
        turn = object_new(allocator, "Turn_Udp_Client", NULL);
        /*
         *turn->connect(turn, "turn.voipturnt.com", "3478");
         */
        turn->connect(turn, "77.72.169.212", "3479");
        /*
         *EXEC(turn->connect(turn, "77.72.169.213", "3478"));
         */
        /*
         *EXEC(turn->connect(turn, "75.2.81.221", "3478"));
         */
        /*
         *EXEC(turn->connect(turn, "212.227.67.34", "3478"));
         */
        /*
         *EXEC(turn->connect(turn, "turn.voip.aebc.com", "3478"));
         */
        /*
         *EXEC(turn->connect(turn, "turn.voiparound.com", "3478"));
         */
        EXEC(turn->allocate_address(turn));
        pause();

    } CATCH (ret) {
    }
    object_destroy(turn);
}
REGISTER_TEST_CMD(test_turn_udp);

