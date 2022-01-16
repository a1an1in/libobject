
/**
 * @file Turn.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "protocol.h"

static int __parse_attrib_mapped_addr(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_mapped_address_t *attr;

    attribs->mapped_address = (turn_attrib_mapped_address_t *)attrib;
    attr = attribs->mapped_address;
    attr->port = ntohs(attr->port);

    if (attr->family == 1) {
        dbg_str(DBG_DETAIL, "mapped addr, port:%d ip:%d:%d:%d:%d",
                attr->port, attr->u.ipv4[3],  
                attr->u.ipv4[2],  attr->u.ipv4[1],  attr->u.ipv4[0]);
    } else {
    }

    return 0;
}

/*
 *static int __parse_attrib_changed_addr(turn_attrib_t *raw, turn_attrib_t *out)
 *{
 *    int ret;
 *    int i, len,  family;
 *    uint8_t *p, *host;
 *
 *    TRY {
 *        family = raw->u.mapped_address.family;
 *        SET_CATCH_INT_PARS(family, 0);
 *        THROW_IF(family != 0x1 && family != 2, -1);
 *        snprintf(out->u.changed_address.service, 8, "%d", ntohs(raw->u.changed_address.port));
 *        len = family == 0x1 ? 4 : 8;
 *        p = raw->u.changed_address.ip;
 *        host = out->u.changed_address.host;
 *        for (i = 0; i < len - 1; i++) {
 *            snprintf(host + strlen(host), 32 - strlen(host), "%d.", *(p + i));
 *        }
 *        snprintf(host + strlen(host), 32 - strlen(host), "%d", *(p + i));
 *        dbg_str(DBG_DETAIL, "parse changed addr, host:%s, server:%s",
 *                out->u.changed_address.host, out->u.changed_address.service);
 *    } CATCH (ret) {
 *        TRY_SHOW_INT_PARS(DBG_ERROR);
 *    }
 *
 *    return ret;
 *}
 */

static int __parse_attrib_changed_addr(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_changed_address_t *attr;

    attribs->changed_address = (turn_attrib_changed_address_t *)attrib;
    attr = attribs->changed_address;
    attr->port = ntohs(attr->port);

    if (attr->family == 1) {
        dbg_str(DBG_DETAIL, "changed addr, port:%d ip:%d:%d:%d:%d",
                attr->port, attr->u.ipv4[3],  
                attr->u.ipv4[2],  attr->u.ipv4[1],  attr->u.ipv4[0]);
    } else {
    }

    return 0;
}

static int __parse_attrib_error_code(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_error_code_t *attr;

    attribs->error_code = (turn_attrib_error_code_t *)attrib;
    attr = attribs->error_code;

    dbg_str(DBG_DETAIL, "errorcode, %d %d, error reason:%s", attr->class_bits,  attr->number_bits,  attr->reason);

    return 0;
}


static attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE] = {
    [TURN_ATR_TYPE_MAPPED_ADDR] = {.policy = __parse_attrib_mapped_addr},
    [TURN_ATR_TYPE_CHANGED_ADDRESS] = {.policy = __parse_attrib_changed_addr},
    [TURN_ATR_TYPE_ERROR_CODE] = {.policy = __parse_attrib_error_code},
};

attrib_parse_policy_t *protocol_get_parse_policies()
{
    return g_parse_attr_policies;
}
