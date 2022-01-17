
/**
 * @file Turn_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/crypto/md5.h>
#include "Turn_Client.h"

static int __construct(Turn_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.allocator;
    int ret = 0;

    TRY {
       turn->req = object_new(allocator, "Turn::Request", NULL);
       turn->response = object_new(allocator, "Turn::Response", NULL);
       THROW_IF(turn->req == NULL || turn->response == NULL, -1);

       turn->digest = object_new(allocator, "Digest_Sha1", NULL);

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

static int __generate_auth_code(Turn_Client *turn, char *username, char *realm, char *password)
{
    digest_md5_t ctx;
    unsigned char buffer[1024] = {0};
    unsigned char result[20] = {0};
    char result_hex[100] = {0};
    int i, len;
    int ret = 0;

    TRY {
        THROW_IF(username == NULL || realm == NULL || password == NULL, -1);
        len = strlen(username) + strlen(realm) + strlen(password) + 2;
        THROW_IF(len > 1024 - 1, -1); 
        snprintf(buffer, 1024, "%s:%s:%s", username, realm, password);
        digest_md5_init(&ctx);
        digest_md5_update(&ctx, buffer, len);
        digest_md5_final(&ctx, result);

        for (i = 0; i < 16; i++) {
            sprintf(result_hex + strlen(result_hex), "%02x", result[i]);
        }
        dbg_str(DBG_ERROR, "auth code:%s", result_hex);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __compute_integrity(Turn_Client *turn)
{
    Digest *digest = turn->digest;
    unsigned char buffer[1024] = {1};
    unsigned char result[20] = {0};
    char result_hex[100] = {0};
    int i, len;
    int ret = 0;

    TRY {
        digest->init(digest);
        digest->update(digest, buffer, 99);
        digest->final(digest, result, 20);

        for (i= 0; i < 20; i++) {
            sprintf(result_hex + strlen(result_hex), "%02x", result[i]);
        }
        dbg_str(DBG_ERROR, "compute_integrity code:%s", result_hex);

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
REGISTER_CLASS("Turn_Client", turn_class_info);
