
/**
 * @file Turn_Udp_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/net/turn/Turn_Udp_Client.h>

enum {
    TURN_METHOD_ENUM_ALLOCATE = 0,
    TURN_METHOD_ENUM_MAX,	                           
};

static int __turn_client_resp_callback(void *task);
static int __turn_read_post_callback(Response * Response, void *opaque);
static turn_method_policy_t g_turn_client_method_policies[TURN_METHOD_ENUM_MAX];
static turn_method_map_t g_turn_client_method_vector[TURN_METHOD_ENUM_MAX];

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

        turn->set_read_post_callback(turn, __turn_read_post_callback);
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

static int __count_attrib_len_for_each(int index, void *element, void *arg)
{
    int ret;
    int *len = (int *)arg;
    turn_attrib_header_t *attrib;

    TRY {
        THROW_IF(element == NULL || arg == NULL, 0);

        attrib = (turn_attrib_header_t *)element;
        (*len) += ntohs(attrib->len) + 4;

    } CATCH(ret) {
    }

    return ret;
}

static int __write_attrib_to_send_buffer_for_each(int index, void *element, void *arg)
{
    int ret;
    Buffer *buffer = (Buffer *)arg;
    turn_attrib_header_t *attrib;

    TRY {
        THROW_IF(element == NULL || arg == NULL, 0);

        attrib = (turn_attrib_header_t *)element;
        buffer->write(buffer, attrib,  ntohs(attrib->len) + 4);

    } CATCH(ret) {
    }

    return ret;
}

static int __send(Turn_Udp_Client *turn)
{
    Request *req = turn->parent.req;
    Buffer *buffer = req->buffer;
    Vector *vector = req->attribs;
    int i = 0, attrib_len = 0;
    int ret = 0;
    turn_attrib_header_t *attrib;

    TRY {
        buffer->reset(buffer);

        /* compute attribs len */
        vector->for_each_arg(vector, __count_attrib_len_for_each, &attrib_len);

        dbg_str(DBG_DETAIL, " atttrib len%d", attrib_len);
        req->header->msglen = htons(attrib_len);
        buffer->write(buffer, req->header,  sizeof(turn_header_t));
        vector->for_each_arg(vector, __write_attrib_to_send_buffer_for_each, buffer);

        req->len = buffer->get_len(buffer);
        client_send(turn->c, buffer->addr + buffer->r_offset, req->len, 0);
        dbg_str(DBG_DETAIL, "turn send: atttrib count:%d, len%d", vector->count(vector), req->len);
    } CATCH (ret) {
    }

    return ret;
}

static int 
__allocate_address(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg)
{
    allocator_t *allocator = turn->parent.parent.allocator;
    Request *req = turn->parent.req;
    Vector *vector = req->attribs;
    turn_attrib_requested_transport_t *requested_transport;
    turn_attrib_nonce_t *attr_nonce;
    turn_attrib_realm_t *attr_realm;
    turn_attrib_username_t *attr_username;
    turn_attrib_lifetime_t *attr_lifetime;
    turn_attrib_requested_family_t *attr_family;
    int len;
    int ret;

    TRY {
        THROW_IF(arg == NULL, -1);
        SET_CATCH_INT_PARS(arg->nonce_len, 0);
        THROW_IF(arg->nonce_len > 128, -1);

        req->set_head(req, TURN_METHOD_BINDREQ | TURN_METHOD_ALLOCATE, 0, 0x2112A442);

        EXEC(turn_set_attrib_nonce(vector, arg->nonce, arg->nonce_len));
        EXEC(turn_set_attrib_realm(vector, arg->realm, arg->realm == NULL ? 0 : strlen(arg->realm)));
        EXEC(turn_set_attrib_username(vector, arg->user, arg->user == NULL ? 0 : strlen(arg->user)));
        EXEC(turn_set_attrib_lifetime(vector, arg->lifetime));
        EXEC(turn_set_attrib_requested_family(vector, arg->family));
        EXEC(turn_set_attrib_requested_transport(vector, 17));

        turn->send(turn);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0, Turn::Turn_Client, parent),
    Init_Nfunc_Entry(1, Turn_Udp_Client, construct, __construct),
    Init_Nfunc_Entry(2, Turn_Udp_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Turn_Udp_Client, connect, __connect),
    Init_Vfunc_Entry(4, Turn_Udp_Client, allocate_address, __allocate_address),
    Init_Vfunc_Entry(5, Turn_Udp_Client, send, __send),
    Init_Vfunc_Entry(6, Turn_Udp_Client, set_read_post_callback, NULL),
    Init_Vfunc_Entry(7, Turn_Udp_Client, generate_auth_code, NULL),
    Init_End___Entry(8, Turn_Udp_Client),
};
REGISTER_CLASS("Turn::Turn_Udp_Client", turn_class_info);

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
                memset(&resp->attribs, 0, sizeof(turn_attribs_t));
                break;
            }
        } 
    } CATCH (ret) {
    }

    return ret;
}

static int __turn_read_post_callback(Response * resp, void *opaque)
{
    int ret, method, method_index;
    turn_header_t *header;

    TRY {
        header = resp->header;
        method = header->msgtype & 0xeef; 

        dbg_str(DBG_DETAIL, "__turn_read_post_callback, method=%x", method);
        method_index = turn_get_method_policy_index(method);
        THROW_IF(method_index == -1, -1);

        g_turn_client_method_policies[method_index].policy(resp, opaque);
    } CATCH (ret) {
    }

    return ret;
}

static int __turn_allocate_post_callback(Response * Response, void *opaque)
{
    dbg_str(DBG_DETAIL, "__turn_allocate_post_callback");
    return 0;
}

int turn_get_method_policy_index(int type) 
{
    int i, ret = -1;

    for (i = 0; i < TURN_METHOD_ENUM_MAX; i++) {
        if (g_turn_client_method_vector[i].type == type) {
            ret = g_turn_client_method_vector[i].index;
            break;
        }
    }

    return ret;
}

static turn_method_map_t g_turn_client_method_vector[TURN_METHOD_ENUM_MAX] = {
    {TURN_METHOD_ENUM_ALLOCATE, TURN_METHOD_ALLOCATE},
};

static turn_method_policy_t g_turn_client_method_policies[TURN_METHOD_ENUM_MAX] = {
    [TURN_METHOD_ENUM_ALLOCATE] = {.policy = __turn_allocate_post_callback},
};

static int test_turn_udp(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Turn_Client *turn = NULL;
    char *str = "hello world";
    allocate_address_reqest_arg_t arg = {0};
    uint8_t result[20];
    int ret;

    dbg_str(NET_DETAIL, "test_turn_udp");

    TRY {
        turn = object_new(allocator, "Turn::Turn_Udp_Client", NULL);
        turn->connect(turn, "172.16.49.3", "3478");
        arg.lifetime = -1;
        EXEC(turn->allocate_address(turn, &arg));
        /*
         *EXEC(turn->generate_auth_code(turn, "toto", "domain.org", "4588"));
         */
        /*
         *EXEC(turn->compute_integrity(turn, "12345", 5, "1111111111", 10, result, 20));
         */

        sleep(2);
    } CATCH (ret) {
    }
    object_destroy(turn);
}
REGISTER_TEST_CMD(test_turn_udp);

