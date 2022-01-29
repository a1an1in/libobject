
/**
 * @file Turn_Udp_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/net/turn/Turn_Udp_Client.h>

static int __turn_client_resp_callback(void *task);
static int turn_read_post_callback(Response * Response, void *opaque);

static int __construct(Turn_Udp_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.parent.allocator;
    int ret = 0;

    TRY {
        turn->c = client(allocator, CLIENT_TYPE_INET_UDP, 
                         (char *)"0.0.0.0", //char *host, 
                         NULL,
                         __turn_client_resp_callback, turn);
        THROW_IF(turn->c == NULL, -1);

        turn->set_read_post_callback(turn, turn_read_post_callback);
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
    Buffer *buffer = req->buffer;
    Vector *vector = req->attribs;
    int i = 0, attrib_len = 0;
    turn_attrib_header_t *attrib;
    int ret = 0;

    TRY {
        buffer->reset(buffer);

        /* compute attribs len */
        vector->for_each_arg(vector, count_attrib_len_for_each, &attrib_len);

        dbg_str(DBG_DETAIL, " atttrib len%d", attrib_len);
        req->header->msglen = htons(attrib_len);
        buffer->write(buffer, req->header,  sizeof(turn_header_t));
        vector->for_each_arg(vector, write_attrib_to_send_buffer_for_each, buffer);

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
    turn_attrib_nonce_t *nonce = turn->parent.nonce;
    Request *req = turn->parent.req;
    Vector *vector = req->attribs;
    uint8_t result[20] = {0}, key[16] = {0};
    int len;
    int ret;

    TRY {
        dbg_str(DBG_SUC, "allocate_address");
        THROW_IF(arg == NULL, -1);
        SET_CATCH_INT_PARS(arg->nonce_len, 0);
        THROW_IF(arg->nonce_len > 128, -1);

        req->set_head(req, STUN_QUEST | TURN_METHOD_ALLOCATE, 0, 0x2112A442);

        if (nonce->len != 0) {
            EXEC(turn_set_attrib_nonce(vector, nonce->value, nonce->len));
            EXEC(turn_set_attrib_realm(vector, arg->realm, arg->realm == NULL ? 0 : strlen(arg->realm)));
            EXEC(turn_set_attrib_username(vector, arg->user, arg->user == NULL ? 0 : strlen(arg->user)));
            EXEC(turn_set_attrib_lifetime(vector, arg->lifetime));
            EXEC(turn_set_attrib_requested_family(vector, arg->family));
            EXEC(turn_set_attrib_requested_transport(vector, 17));

            EXEC(turn->generate_auth_code(turn, arg->user, arg->realm, arg->password, key, 16));
            EXEC(turn->compute_integrity(turn, key, 16, result, 20));
            EXEC(turn_set_attrib_integrity(vector, result, 20));
        }

        turn->send(turn);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int 
__create_permission(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg)
{
    allocator_t *allocator = turn->parent.parent.allocator;
    turn_attrib_nonce_t *nonce = turn->parent.nonce;
    char *realm = arg->realm;
    char *user = arg->user;
    Request *req = turn->parent.req;
    Vector *vector = req->attribs;
    uint8_t result[20] = {0}, key[16] = {0};
    int len;
    int ret;

    TRY {
        THROW_IF(arg == NULL, -1);
        THROW_IF(nonce == NULL, -1);

        dbg_str(DBG_SUC, "create_permission");
        req->clear(req);
        req->set_head(req, STUN_QUEST | TURN_METHOD_CREATEPERMISSION, 0, 0x2112A442);

        EXEC(turn_set_attrib_nonce(vector,nonce->value, nonce->len));
        EXEC(turn_set_attrib_realm(vector, realm, realm == NULL ? 0 : strlen(realm)));
        EXEC(turn_set_attrib_username(vector, user, user == NULL ? 0 : strlen(user)));

        EXEC(turn_set_attrib_xor_peer_address(vector, arg->addr, 0x2112A442));
        EXEC(turn->compute_integrity(turn, turn->parent.auth_code, 16, result, 20));
        EXEC(turn_set_attrib_integrity(vector, result, 20));

        turn->send(turn);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int
__send_indication(Turn_Udp_Client *turn, allocate_address_reqest_arg_t *arg, uint8_t *value, int len)
{
    allocator_t *allocator = turn->parent.parent.allocator;
    turn_attrib_nonce_t *nonce = turn->parent.nonce;
    char *realm = arg->realm;
    char *user = arg->user;
    Request *req = turn->parent.req;
    Vector *vector = req->attribs;
    uint8_t result[20] = {0}, key[16] = {0};
    int ret;

    TRY {
        dbg_str(DBG_SUC, "send indication");
        req->clear(req);
        req->set_head(req, STUN_INDICATION | TURN_METHOD_SEND, 0, 0x2112A442);

        EXEC(turn_set_attrib_nonce(vector,nonce->value, nonce->len));
        EXEC(turn_set_attrib_realm(vector, realm, realm == NULL ? 0 : strlen(realm)));
        EXEC(turn_set_attrib_username(vector, user, user == NULL ? 0 : strlen(user)));

        EXEC(turn_set_attrib_data(vector, value, len));
        EXEC(turn_set_attrib_xor_peer_address(vector, arg->addr, 0x2112A442));
        EXEC(turn->compute_integrity(turn, turn->parent.auth_code, 16, result, 20));
        EXEC(turn_set_attrib_integrity(vector, result, 20));

        turn->send(turn);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __wait(Turn_Udp_Client *turn, enum turn_status_e status, int time)
{
    time = time * 1000;
    while (time-- > 0 && turn->parent.status != status) {
        usleep(1000);
    } 
    
    return time > 0 ? 1 : -1;
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0 , Turn::Turn_Client, parent),
    Init_Nfunc_Entry(1 , Turn_Udp_Client, construct, __construct),
    Init_Nfunc_Entry(2 , Turn_Udp_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Turn_Udp_Client, connect, __connect),
    Init_Vfunc_Entry(4 , Turn_Udp_Client, allocate_address, __allocate_address),
    Init_Vfunc_Entry(5 , Turn_Udp_Client, create_permission, __create_permission),
    Init_Vfunc_Entry(6 , Turn_Udp_Client, send, __send),
    Init_Vfunc_Entry(7 , Turn_Udp_Client, send_indication, __send_indication),
    Init_Vfunc_Entry(8 , Turn_Udp_Client, set_read_post_callback, NULL),
    Init_Vfunc_Entry(9 , Turn_Udp_Client, generate_auth_code, NULL),
    Init_Vfunc_Entry(10, Turn_Udp_Client, compute_integrity, NULL),
    Init_Vfunc_Entry(11, Turn_Udp_Client, wait, __wait),
    Init_End___Entry(12, Turn_Udp_Client),
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


static int test_turn_udp(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Turn_Client *turn = NULL;
    char *str = "hello world";
    allocate_address_reqest_arg_t arg = {0};
    struct addrinfo *addr, hint;
    int ret;

    dbg_str(NET_DETAIL, "test_turn_udp");

    TRY {
        bzero(&hint, sizeof(hint));
        hint.ai_family   = AF_INET;
        hint.ai_socktype = SOCK_DGRAM;
        THROW_IF(getaddrinfo("172.17.0.5", "4588", &hint, &addr) != 0, -1);

        arg.user = "toto";
        arg.realm = "domain.org";
        arg.password = "4588";
        arg.addr = addr;
        arg.lifetime = -1;

        turn = object_new(allocator, "Turn::Turn_Udp_Client", NULL);
        EXEC(turn->connect(turn, "122.112.200.183", "3478"));
        EXEC(turn->generate_auth_code(turn, arg.user, arg.realm, arg.password, turn->auth_code, 16));

        EXEC(turn->allocate_address(turn, &arg));
        EXEC(turn->wait(turn, TURN_STATUS_ALLOC_ERROR, 60));

        EXEC(turn->allocate_address(turn, &arg));
        EXEC(turn->wait(turn, TURN_STATUS_ALLOC_SUC, 60));

        EXEC(turn->create_permission(turn, &arg));
        EXEC(turn->wait(turn, TURN_STATUS_PERMISION_SUC, 60));

        EXEC(turn->send_indication(turn, &arg, "hello world", 12));

        sleep(10);
    } CATCH (ret) {
    }
    object_destroy(turn);
}
REGISTER_TEST_CMD(test_turn_udp);

