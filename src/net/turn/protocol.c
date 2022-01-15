
/**
 * @file Turn.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "protocol.h"

static int __parse_attrib_mapped_addr(turn_attrib_t *raw, turn_attrib_t *out)
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
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __parse_attrib_changed_addr(turn_attrib_t *raw, turn_attrib_t *out)
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
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE] = {
    [TURN_ATR_TYPE_MAPPED_ADDR] = {.policy = __parse_attrib_mapped_addr},
    [TURN_ATR_TYPE_CHANGED_ADDRESS] = {.policy = __parse_attrib_changed_addr},
};

attrib_parse_policy_t *protocol_get_parse_policies()
{
    return g_parse_attr_policies;
}
