
/**
 * @file Stun.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Stun.h"

static int __construct(Stun *stun, char *init_str)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0;

    TRY {
       stun->req = object_new(allocator, "Stun::Request", NULL);
       stun->response = object_new(allocator, "Stun::Response", NULL);
       THROW_IF(stun->req == NULL || stun->response == NULL, -1);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Stun *stun)
{
    object_destroy(stun->req);
    object_destroy(stun->response);
    return 0;
}


int __set_read_post_callback(Stun *stun, int (*func)(Response *, void *arg)) 
{
    stun->response->read_post_callback = func;

    return 1;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun, connect, NULL),
    Init_Vfunc_Entry(4, Stun, discovery, NULL),
    Init_Vfunc_Entry(5, Stun, send, NULL),
    Init_Vfunc_Entry(6, Stun, set_read_post_callback, __set_read_post_callback),
    Init_End___Entry(7, Stun),
};
REGISTER_CLASS("Stun", stun_class_info);

static int __parse_attrib_mapped_addr(stun_attrib_t *raw, stun_attrib_t *out)
{
    int ret;
    int i, len, family;
    uint8_t *p, *host;

    TRY {
        family = raw->u.mapped_address.family;
        SET_CATCH_INT_PARS(family, 0);
        THROW_IF(family != 0x1 && family != 2, -1);
        snprintf(out->u.mapped_address.service, 8, "%d", ntohs(raw->u.mapped_address.port));
        len = family == 0x1 ? 4 : 8;
        p = raw->u.mapped_address.ip;
        host = out->u.mapped_address.host;
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
        dbg_str(DBG_DETAIL, "parse maped addr, host:%s, server:%s",
                out->u.mapped_address.host, out->u.mapped_address.service);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __parse_attrib_changed_addr(stun_attrib_t *raw, stun_attrib_t *out)
{
    int ret;
    int i, len,  family;
    uint8_t *p, *host;

    TRY {
        family = raw->u.mapped_address.family;
        SET_CATCH_INT_PARS(family, 0);
        THROW_IF(family != 0x1 && family != 2, -1);
        snprintf(out->u.changed_address.service, 8, "%d", ntohs(raw->u.changed_address.port));
        len = family == 0x1 ? 4 : 8;
        p = raw->u.changed_address.ip;
        host = out->u.changed_address.host;
        for (i = 0; i < len - 1; i++) {
            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
        }
        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
        dbg_str(DBG_DETAIL, "parse changed addr, host:%s, server:%s",
                out->u.changed_address.host, out->u.changed_address.service);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE] = {
    [STUN_ATR_TYPE_MAPPED_ADDR] = {.policy = __parse_attrib_mapped_addr},
    [STUN_ATR_TYPE_CHANGED_ADDRESS] = {.policy = __parse_attrib_changed_addr},
};
