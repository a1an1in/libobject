
/**
 * @file Turn_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-1-17
 */

#include <libobject/encoding/md5.h>
#include <libobject/net/turn/Turn_Client.h>

enum {
    TURN_METHOD_ENUM_ALLOCATE = 0,
    TURN_METHOD_ENUM_CREATE_PERMISSION,
    TURN_METHOD_ENUM_DATA_INDICATION,
    TURN_METHOD_ENUM_MAX,	                           
};

static turn_method_policy_t g_turn_client_method_policies[TURN_METHOD_ENUM_MAX];
static turn_method_map_t g_turn_client_method_vector[TURN_METHOD_ENUM_MAX];

static int __construct(Turn_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.allocator;
    int ret = 0;

    TRY {
       turn->req = object_new(allocator, "Turn::Request", NULL);
       turn->response = object_new(allocator, "Turn::Response", NULL);
       THROW_IF(turn->req == NULL || turn->response == NULL, -1);

       turn->digest = object_new(allocator, "Openssl::Digest_HmacSha1", NULL);

       turn->nonce = allocator_mem_zalloc(allocator, 150);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Turn_Client *turn)
{
    object_destroy(turn->req);
    object_destroy(turn->response);
    object_destroy(turn->digest);
    allocator_mem_free(turn->parent.allocator, turn->nonce);
    return 0;
}

static int __set_read_post_callback(Turn_Client *turn, int (*func)(Response *, void *arg)) 
{
    turn->response->read_post_callback = func;

    return 1;
}

static int __generate_auth_code(Turn_Client *turn, char *username, char *realm, char *password, uint8_t *out, uint32_t out_len)
{
    digest_md5_t ctx;
    unsigned char buffer[1024] = {0};
    char result_hex[100] = {0};
    int i, len;
    int ret = 0;

    TRY {
        THROW_IF(username == NULL || realm == NULL || password == NULL, -1);
        THROW_IF(out_len < 16, -1);
        len = strlen(username) + strlen(realm) + strlen(password) + 2;
        THROW_IF(len > 1024 - 1, -1); 
        snprintf(buffer, 1024, "%s:%s:%s", username, realm, password);
        digest_md5_init(&ctx);
        digest_md5_update(&ctx, buffer, len);
        digest_md5_final(&ctx, out);

        for (i = 0; i < 16; i++) {
            sprintf(result_hex + strlen(result_hex), "%02x", out[i]);
        }
        dbg_str(DBG_DETAIL, "auth code:%s", result_hex);

    } CATCH (ret) {
    }
    
    return ret;
}


static int 
__compute_integrity(Turn_Client *turn, 
                    uint8_t *key, uint8_t key_len,
                    uint8_t *out, uint8_t out_len)
{
    Request *req = turn->req;
    Buffer *buffer = req->buffer;
    Vector *vector = req->attribs;
    Digest *digest = turn->digest;
    char result_hex[100] = {0};
    int i, ret = 0, attrib_len = 0;

    TRY {
        THROW_IF(out_len < 20 || key == NULL, -1);
        buffer->reset(buffer);

        vector->for_each_arg(vector, count_attrib_len_for_each, &attrib_len);
        req->header->msglen = htons(attrib_len + sizeof(turn_attrib_integrity_t));

        buffer->write(buffer, req->header,  sizeof(turn_header_t));
        vector->for_each_arg(vector, write_attrib_to_send_buffer_for_each, buffer);

        digest->init_with_key(digest, key, key_len);
        digest->update(digest, buffer->addr + buffer->r_offset, buffer->get_len(buffer));
        digest->final(digest, out, out_len);

        dbg_buf(DBG_DETAIL, "data:", buffer->addr + buffer->r_offset, buffer->get_len(buffer));
        memset(result_hex, 0, 100);
        for (i= 0; i < 20; i++) {
            sprintf(result_hex + strlen(result_hex), "%02x", out[i]);
        }
        dbg_str(DBG_DETAIL, "compute_integrity code:%s", result_hex);
    } CATCH (ret) {
    }

    return ret;
}

static int __set_method_policy(Turn_Client *turn, int index, void *callback) 
{
    int ret;

    TRY {
        THROW_IF(index > TURN_METHOD_ENUM_MAX, -1);
        g_turn_client_method_policies[index].policy = callback;
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Turn_Client, construct, __construct),
    Init_Nfunc_Entry(2 , Turn_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Turn_Client, connect, NULL),
    Init_Vfunc_Entry(4 , Turn_Client, allocate_address, NULL),
    Init_Vfunc_Entry(5 , Turn_Client, create_permission, NULL),
    Init_Vfunc_Entry(6 , Turn_Client, send, NULL),
    Init_Vfunc_Entry(7 , Turn_Client, send_indication, NULL),
    Init_Vfunc_Entry(8 , Turn_Client, set_read_post_callback, __set_read_post_callback),
    Init_Vfunc_Entry(9 , Turn_Client, generate_auth_code, __generate_auth_code),
    Init_Vfunc_Entry(10, Turn_Client, compute_integrity, __compute_integrity),
    Init_Vfunc_Entry(11, Turn_Client, wait, NULL),
    Init_Vfunc_Entry(12, Turn_Client, set_method_policy, __set_method_policy),
    Init_End___Entry(13, Turn_Client),
};
REGISTER_CLASS("Turn::Turn_Client", turn_class_info);

static int __turn_allocate_post_callback(Response * response, void *opaque)
{
    allocate_address_reqest_arg_t arg = {0};
    Turn_Client *turn = opaque;
    int ret;

    TRY {
        if ((response->header->msgtype & 0x110) == 0x110) {
            turn->status = TURN_STATUS_ALLOC_ERROR;
            memcpy(turn->nonce, response->attribs.nonce, response->attribs.nonce->len + 4);
        } else if ((response->header->msgtype & 0x110) == 0x100) {
            dbg_str(DBG_SUC, "allocation success");
            turn->status = TURN_STATUS_ALLOC_SUC;
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __turn_create_permisson_post_callback(Response * response, void *opaque)
{
    allocate_address_reqest_arg_t arg = {0};
    Turn_Client *turn = opaque;
    int ret;

    TRY {
        dbg_str(DBG_SUC, "__turn_create_permisson_post_callback");
        if ((response->header->msgtype & 0x118) == 0x118) {
            turn->status = TURN_STATUS_PERMISION_ERROR;
        } else if ((response->header->msgtype & 0x118) == 0x108) {
            turn->status = TURN_STATUS_PERMISION_SUC;
            dbg_str(DBG_SUC, "TURN_STATUS_PERMISION_SUC");
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __turn_data_indication_post_callback(Response * response, void *opaque)
{
    allocate_address_reqest_arg_t arg = {0};
    turn_attrib_data_t *data;
    Turn_Client *turn = opaque;
    int ret;

    TRY {
        data = response->attribs.data;
        dbg_str(DBG_SUC, "__turn_data_indication_post_callback");
        dbg_str(DBG_SUC, "recv ind data:%s", data->value);
        dbg_buf(DBG_SUC, "recv ind data:", data->value, data->len);
    } CATCH (ret) {
    }

    return ret;
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
    {TURN_METHOD_ENUM_CREATE_PERMISSION, TURN_METHOD_CREATEPERMISSION},
    {TURN_METHOD_ENUM_DATA_INDICATION, TURN_METHOD_DATA},
};

static turn_method_policy_t g_turn_client_method_policies[TURN_METHOD_ENUM_MAX] = {
    [TURN_METHOD_ENUM_ALLOCATE]          = {.policy = __turn_allocate_post_callback},
    [TURN_METHOD_ENUM_CREATE_PERMISSION] = {.policy = __turn_create_permisson_post_callback},
    [TURN_METHOD_ENUM_DATA_INDICATION]   = {.policy = __turn_data_indication_post_callback},
};

int turn_read_post_callback(Response * resp, void *opaque)
{
    int ret, method, method_index;
    turn_header_t *header;

    TRY {
        header = resp->header;
        method = header->msgtype & 0xeef; 

        dbg_str(DBG_DETAIL, "turn_read_post_callback, method=%x", method);
        method_index = turn_get_method_policy_index(method);
        THROW_IF(method_index == -1, -1);

        g_turn_client_method_policies[method_index].policy(resp, opaque);
    } CATCH (ret) {
    }

    return ret;
}

int count_attrib_len_for_each(int index, void *element, void *arg)
{
    int ret;
    int *len = (int *)arg;
    turn_attrib_header_t *attrib;
    int res;

    TRY {
        THROW_IF(element == NULL || arg == NULL, 0);

        attrib = (turn_attrib_header_t *)element;
        res = (ntohs(attrib->len) + 4) % 4;
        (*len) += (ntohs(attrib->len) + 4 + res);

    } CATCH(ret) {
    }

    return ret;
}

int write_attrib_to_send_buffer_for_each(int index, void *element, void *arg)
{
    int ret;
    Buffer *buffer = (Buffer *)arg;
    turn_attrib_header_t *attrib;
    char padding[4] = {0};
    int res;

    TRY {
        THROW_IF(element == NULL || arg == NULL, 0);

        attrib = (turn_attrib_header_t *)element;
        buffer->write(buffer, attrib,  ntohs(attrib->len) + 4);
        res = (ntohs(attrib->len) + 4) % 4;
        if (res != 0) {
            buffer->write(buffer, padding, res);
            dbg_str(DBG_DETAIL, "add padding, count=%d, index=%d", res, index);
        }

    } CATCH(ret) {
    }

    return ret;
}

