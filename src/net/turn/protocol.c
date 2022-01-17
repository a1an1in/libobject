
/**
 * @file Turn.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "protocol.h"

static int turn_parse_attrib_mapped_addr(turn_attribs_t *attribs, uint8_t *attrib)
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
 *static int turn_parse_attrib_changed_addr(turn_attrib_t *raw, turn_attrib_t *out)
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

static int turn_parse_attrib_changed_addr(turn_attribs_t *attribs, uint8_t *attrib)
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

static int turn_parse_attrib_error_code(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_error_code_t *attr;

    attribs->error_code = (turn_attrib_error_code_t *)attrib;
    attr = attribs->error_code;

    dbg_str(DBG_DETAIL, "errorcode, %d %d, error reason:%s", attr->class_bits,  attr->number_bits,  attr->reason);

    return 0;
}

static int turn_parse_attrib_realm(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_realm_t *attr;

    attribs->realm = (turn_attrib_realm_t *)attrib;
    attr = attribs->realm;

    dbg_str(DBG_DETAIL, "realm:%s", attr->value);

    return 0;
}

static int turn_parse_attrib_nonce(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_nonce_t *nonce;
    uint8_t buf[128] = {0};

    attribs->nonce = (turn_attrib_nonce_t *)attrib;
    nonce = attribs->nonce;
    memcpy(buf, nonce->value, nonce->len);
    dbg_str(DBG_DETAIL, "nonce, len:%d value:%s", nonce->len, buf);

    return 0;
}

static int turn_parse_attrib_software(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_software_t *software;
    uint8_t buf[128] = {0};

    attribs->software = (turn_attrib_software_t *)attrib;
    software = attribs->software;
    
    memcpy(buf, software->value, software->len);
    dbg_str(DBG_DETAIL, "software, len:%d value:%s", software->len, buf);

    return 0;
}

static int turn_parse_attrib_fingerprint(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_fingerprint_t *attr;

    attribs->fingerprint = (turn_attrib_fingerprint_t *)attrib;
    attr = attribs->fingerprint;
    attr->crc32 = ntohl(attr->crc32);
    
    dbg_str(DBG_DETAIL, "fingerprint, crc32:%x", attr->crc32);

    return 0;
}

enum {
    TURN_ATTR_ENUM_MAPPED_ADDR = 0,
    TURN_ATTR_ENUM_RESPONSE_ADDRESS,	                
    TURN_ATTR_ENUM_CHANGE_REQUEST,	                
    TURN_ATTR_ENUM_SOURCE_ADDRESS,	                
    TURN_ATTR_ENUM_CHANGED_ADDRESS,	                
    TURN_ATTR_ENUM_USERNAME,			                
    TURN_ATTR_ENUM_PASSWORD,			                
    TURN_ATTR_ENUM_INTEGRITY,		                
    TURN_ATTR_ENUM_ERROR_CODE,		               	
    TURN_ATTR_ENUM_UNKNOWN_ATTRIBUTES,               
    TURN_ATTR_ENUM_REFLECTED_FROM,	               	
    TURN_ATTR_ENUM_XOR_MAPPED_ADDRESS,               
    TURN_ATTR_ENUM_CHANNEL_NUMBER,                    
    TURN_ATTR_ENUM_LIFETIME,                          
    TURN_ATTR_ENUM_XOR_PEER_ADDRESS,                  
    TURN_ATTR_ENUM_DATA,                 
    TURN_ATTR_ENUM_REALM,                             
    TURN_ATTR_ENUM_NONCE,                             
    TURN_ATTR_ENUM_XOR_RELAYED_ADDRESS,               
    TURN_ATTR_ENUM_EVEN_PORT,              
    TURN_ATTR_ENUM_REQUESTED_TRANSPORT,               
    TURN_ATTR_ENUM_DONT_FRAGMENT,              
    TURN_ATTR_ENUM_Reserved,                          
    TURN_ATTR_ENUM_RESERVATION_TOKEN,                 
    TURN_ATTR_ENUM_SOFTWARE, 
    TURN_ATTR_ENUM_FINGERPRINT, 
    TURN_ATTR_ENUM_MAX,	                            
};

static attrib_type_map_t g_parse_policies_type_map[TURN_ATTR_ENUM_MAX] = {
    {TURN_ATTR_ENUM_MAPPED_ADDR, TURN_ATTR_TYPE_MAPPED_ADDR},
    {TURN_ATTR_ENUM_RESPONSE_ADDRESS, TURN_ATTR_TYPE_RESPONSE_ADDRESS},
    {TURN_ATTR_ENUM_CHANGE_REQUEST, TURN_ATTR_TYPE_CHANGE_REQUEST},
    {TURN_ATTR_ENUM_SOURCE_ADDRESS, TURN_ATTR_TYPE_SOURCE_ADDRESS},
    {TURN_ATTR_ENUM_CHANGED_ADDRESS, TURN_ATTR_TYPE_CHANGED_ADDRESS},
    {TURN_ATTR_ENUM_USERNAME, TURN_ATTR_TYPE_USERNAME},
    {TURN_ATTR_ENUM_PASSWORD, TURN_ATTR_TYPE_PASSWORD},
    {TURN_ATTR_ENUM_INTEGRITY, TURN_ATTR_TYPE_INTEGRITY},
    {TURN_ATTR_ENUM_ERROR_CODE, TURN_ATTR_TYPE_ERROR_CODE},
    {TURN_ATTR_ENUM_UNKNOWN_ATTRIBUTES, TURN_ATTR_TYPE_UNKNOWN_ATTRIBUTES},
    {TURN_ATTR_ENUM_REFLECTED_FROM, TURN_ATTR_TYPE_REFLECTED_FROM},
    {TURN_ATTR_ENUM_CHANNEL_NUMBER, TURN_ATTR_TYPE_CHANNEL_NUMBER},
    {TURN_ATTR_ENUM_LIFETIME, TURN_ATTR_TYPE_LIFETIME},
    {TURN_ATTR_ENUM_XOR_PEER_ADDRESS, TURN_ATTR_TYPE_XOR_PEER_ADDRESS },
    {TURN_ATTR_ENUM_DATA, TURN_ATTR_TYPE_DATA },
    {TURN_ATTR_ENUM_REALM, TURN_ATTR_TYPE_REALM },
    {TURN_ATTR_ENUM_NONCE, TURN_ATTR_TYPE_NONCE },
    {TURN_ATTR_ENUM_XOR_RELAYED_ADDRESS, TURN_ATTR_TYPE_XOR_RELAYED_ADDRESS },
    {TURN_ATTR_ENUM_EVEN_PORT, TURN_ATTR_TYPE_EVEN_PORT },
    {TURN_ATTR_ENUM_REQUESTED_TRANSPORT, TURN_ATTR_TYPE_REQUESTED_TRANSPORT },
    {TURN_ATTR_ENUM_XOR_MAPPED_ADDRESS, TURN_ATTR_TYPE_XOR_MAPPED_ADDRESS},
    {TURN_ATTR_ENUM_RESERVATION_TOKEN, TURN_ATTR_TYPE_RESERVATION_TOKEN},
    {TURN_ATTR_ENUM_SOFTWARE, TURN_ATTR_TYPE_SOFTWARE},
    {TURN_ATTR_ENUM_FINGERPRINT, TURN_ATTR_TYPE_FINGERPRINT},
};

static attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE] = {
    [TURN_ATTR_ENUM_MAPPED_ADDR] = {.policy = turn_parse_attrib_mapped_addr},
    [TURN_ATTR_ENUM_CHANGED_ADDRESS] = {.policy = turn_parse_attrib_changed_addr},
    [TURN_ATTR_ENUM_ERROR_CODE] = {.policy = turn_parse_attrib_error_code},
    [TURN_ATTR_ENUM_REALM] = {.policy = turn_parse_attrib_realm},
    [TURN_ATTR_ENUM_NONCE] = {.policy = turn_parse_attrib_nonce},
    [TURN_ATTR_ENUM_SOFTWARE] = {.policy = turn_parse_attrib_software},
    [TURN_ATTR_ENUM_FINGERPRINT] = {.policy = turn_parse_attrib_fingerprint},
};

attrib_parse_policy_t *turn_get_parser_policies()
{
    return g_parse_attr_policies;
}

int turn_get_parser_policy_index(int type) 
{
    int i, ret = -1;

    for (i = 0; i < TURN_ATTR_ENUM_MAX; i++) {
        if (g_parse_policies_type_map[i].type == type) {
            ret = g_parse_policies_type_map[i].index;
            break;
        }
    }

    return ret;
}

int turn_set_attrib_requested_transport(turn_attrib_requested_transport_t *attrib, uint8_t protocol)
{
    int ret;

    TRY {
        attrib->protocol = protocol;
        attrib->type = htons(TURN_ATTR_TYPE_REQUESTED_TRANSPORT);
        attrib->len = htons(4);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_nonce(turn_attrib_nonce_t *attrib, uint8_t attr_len, uint8_t *nonce, uint8_t nonce_len)
{
    int ret;

    TRY {
        SET_CATCH_INT_PARS(attr_len, nonce_len);
        THROW_IF(attr_len < nonce_len + sizeof(turn_attrib_header_t), -1);

        attrib->len = htons(nonce_len);
        attrib->type = htons(TURN_ATTR_TYPE_NONCE);
        memcpy(attrib->value, nonce, nonce_len);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_realm(turn_attrib_realm_t *attrib, uint8_t attr_len, uint8_t *realm, uint8_t realm_len)
{
    int ret;

    TRY {
        SET_CATCH_INT_PARS(attr_len, realm_len);
        THROW_IF(attr_len < realm_len + sizeof(turn_attrib_header_t), -1);

        attrib->len = htons(realm_len);
        attrib->type = htons(TURN_ATTR_TYPE_REALM);
        memcpy(attrib->value, realm, realm_len);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_username(turn_attrib_username_t *attrib, uint8_t attr_len, uint8_t *username, uint8_t username_len)
{
    int ret;

    TRY {
        SET_CATCH_INT_PARS(attr_len, username_len);
        THROW_IF(attr_len < username_len + sizeof(turn_attrib_header_t), -1);

        attrib->len = htons(username_len);
        attrib->type = htons(TURN_ATTR_TYPE_USERNAME);
        memcpy(attrib->value, username, username_len);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_lifetime(turn_attrib_lifetime_t *attrib, uint32_t lifetime)
{
    int ret;

    TRY {
        SET_CATCH_INT_PARS(lifetime, 0);

        attrib->len = htons(4);
        attrib->type = htons(TURN_ATTR_TYPE_LIFETIME);
        attrib->value = htonl(lifetime);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_requested_family(turn_attrib_requested_family_t *attrib, uint8_t family)
{
    int ret;

    TRY {
        SET_CATCH_INT_PARS(family, 0);

        attrib->len = htons(4);
        attrib->type = htons(TURN_ATTR_TYPE_REQUESTED_FAMILY);
        attrib->family = family;

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

/*
 *int turn_set_attrib_requested_family(turn_attrib_requested_family_t *attrib, uint8_t family)
 *{
 *    int ret;
 *
 *    TRY {
 *        SET_CATCH_INT_PARS(family, 0);
 *
 *        attrib->len = htons(4);
 *        attrib->type = htons(TURN_ATTR_TYPE_REQUESTED_FAMILY);
 *        attrib->family = family;
 *
 *    } CATCH (ret) {
 *        TRY_SHOW_INT_PARS(DBG_ERROR);
 *    }
 *
 *    return ret;
 *}
 */
