
/**
 * @file Turn_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-1-17
 */

#include <libobject/crypto/md5.h>
#include <libobject/net/turn/Turn_Client.h>

static int __construct(Turn_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.allocator;
    int ret = 0;

    TRY {
       turn->req = object_new(allocator, "Turn::Request", NULL);
       turn->response = object_new(allocator, "Turn::Response", NULL);
       THROW_IF(turn->req == NULL || turn->response == NULL, -1);

       turn->digest = object_new(allocator, "Openssl::Digest_HmacSha1", NULL);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Turn_Client *turn)
{
    object_destroy(turn->req);
    object_destroy(turn->response);
    object_destroy(turn->digest);
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
        dbg_str(DBG_ERROR, "auth code:%s", result_hex);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __count_attrib_len_for_each(int index, void *element, void *arg)
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

static int __write_attrib_to_send_buffer_for_each(int index, void *element, void *arg)
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

        vector->for_each_arg(vector, __count_attrib_len_for_each, &attrib_len);
        req->header->msglen = htons(attrib_len + sizeof(turn_attrib_integrity_t));
        dbg_str(DBG_DETAIL, "integrity len:%d", attrib_len + sizeof(turn_header_t) + sizeof(turn_attrib_integrity_t));
        dbg_str(DBG_DETAIL, "attrib_len len:%d", attrib_len);
        dbg_str(DBG_DETAIL, "turn_attrib_integrity_t len:%d", sizeof(turn_attrib_integrity_t));
        dbg_str(DBG_DETAIL, "turn_header_t len:%d", sizeof(turn_header_t));

        buffer->write(buffer, req->header,  sizeof(turn_header_t));
        vector->for_each_arg(vector, __write_attrib_to_send_buffer_for_each, buffer);

        digest->init_with_key(digest, key, key_len);
        dbg_str(DBG_DETAIL, "compute len:%d", buffer->get_len(buffer));
        for (int i = 0; i <  buffer->get_len(buffer); i++) {
            printf("%02x", ((uint8_t *)buffer->addr)[i]);
        }
        printf("\n");
        digest->update(digest, buffer->addr + buffer->r_offset, buffer->get_len(buffer));
        digest->final(digest, out, out_len);

        for (i= 0; i < 20; i++) {
            sprintf(result_hex + strlen(result_hex), "%02x", out[i]);
        }
        dbg_str(DBG_SUC, "compute_integrity code:%s", result_hex);
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Turn_Client, construct, __construct),
    Init_Nfunc_Entry(2, Turn_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Turn_Client, connect, NULL),
    Init_Vfunc_Entry(4, Turn_Client, allocate_address, NULL),
    Init_Vfunc_Entry(5, Turn_Client, send, NULL),
    Init_Vfunc_Entry(6, Turn_Client, set_read_post_callback, __set_read_post_callback),
    Init_Vfunc_Entry(7, Turn_Client, generate_auth_code, __generate_auth_code),
    Init_Vfunc_Entry(8, Turn_Client, compute_integrity, __compute_integrity),
    Init_End___Entry(9, Turn_Client),
};
REGISTER_CLASS("Turn::Turn_Client", turn_class_info);
