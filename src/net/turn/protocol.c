
/**
 * @file Turn.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <sys/socket.h>
#include <netdb.h>
#include <libobject/net/turn/protocol.h>

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

static int turn_parse_attrib_xor_mapped_addr(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_xor_mapped_address_t *attr;

    attribs->xor_mapped_address = (turn_attrib_xor_mapped_address_t *)attrib;
    attr = attribs->xor_mapped_address;
    attr->port = ntohs(attr->port);

    if (attr->family == 1) {
        dbg_str(DBG_DETAIL, "xor mapped addr, port:%d ip:%d:%d:%d:%d",
                attr->port, attr->u.ipv4[3],  
                attr->u.ipv4[2],  attr->u.ipv4[1],  attr->u.ipv4[0]);
    } else {
    }

    return 0;
}

static int turn_parse_attrib_xor_relayed_addr(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_xor_relayed_address_t *attr;

    attribs->xor_relayed_address = (turn_attrib_xor_relayed_address_t *)attrib;
    attr = attribs->xor_relayed_address;
    attr->port = ntohs(attr->port);

    if (attr->family == 1) {
        dbg_str(DBG_DETAIL, "xor relayed addr, port:%d ip:%d:%d:%d:%d",
                attr->port, attr->u.ipv4[3],  
                attr->u.ipv4[2],  attr->u.ipv4[1],  attr->u.ipv4[0]);
    } else {
    }

    return 0;
}

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

    dbg_str(DBG_DETAIL, "errorcode, %d%02d, error reason:%s", attr->class_bits,  attr->number_bits,  attr->reason);

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

static int turn_parse_attrib_data(turn_attribs_t *attribs, uint8_t *attrib)
{
    turn_attrib_data_t *attr;

    attribs->data = (turn_attrib_data_t *)attrib;

    return 0;
}

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
    {TURN_ATTR_ENUM_DATA, TURN_ATTR_TYPE_DATA},
};

static attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE] = {
    [TURN_ATTR_ENUM_MAPPED_ADDR] = {.policy = turn_parse_attrib_mapped_addr},
    [TURN_ATTR_ENUM_CHANGED_ADDRESS] = {.policy = turn_parse_attrib_changed_addr},
    [TURN_ATTR_ENUM_ERROR_CODE] = {.policy = turn_parse_attrib_error_code},
    [TURN_ATTR_ENUM_REALM] = {.policy = turn_parse_attrib_realm},
    [TURN_ATTR_ENUM_NONCE] = {.policy = turn_parse_attrib_nonce},
    [TURN_ATTR_ENUM_SOFTWARE] = {.policy = turn_parse_attrib_software},
    [TURN_ATTR_ENUM_FINGERPRINT] = {.policy = turn_parse_attrib_fingerprint},
    [TURN_ATTR_ENUM_XOR_MAPPED_ADDRESS] = {.policy = turn_parse_attrib_xor_mapped_addr},
    [TURN_ATTR_ENUM_XOR_RELAYED_ADDRESS] = {.policy = turn_parse_attrib_xor_relayed_addr},
    [TURN_ATTR_ENUM_DATA] = {.policy = turn_parse_attrib_data},
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

int turn_set_attrib_requested_transport(Vector *vector, uint8_t protocol)
{
    turn_attrib_requested_transport_t *attrib;
    allocator_t *allocator;
    int ret;

    TRY {
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_requested_transport_t));
        attrib->protocol = protocol;
        attrib->type = htons(TURN_ATTR_TYPE_REQUESTED_TRANSPORT);
        attrib->len = htons(4);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_nonce(Vector *vector, uint8_t *nonce, uint8_t nonce_len)
{
    allocator_t *allocator;
    turn_attrib_nonce_t *attrib;
    int ret;

    TRY {
        SET_CATCH_INT_PARS(nonce_len, 0);
        THROW_IF(nonce_len == 0, 0);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_nonce_t) + nonce_len);
        attrib->len = htons(nonce_len);
        attrib->type = htons(TURN_ATTR_TYPE_NONCE);
        memcpy(attrib->value, nonce, nonce_len);

        vector->add_back(vector, attrib);

    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_realm(Vector *vector, uint8_t *realm, uint8_t realm_len)
{
    allocator_t *allocator;
    turn_attrib_realm_t *attrib;
    int ret;

    TRY {
        THROW_IF(realm_len == 0, 0);
        SET_CATCH_INT_PARS(realm_len, 0);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_realm_t) + realm_len);
        attrib->len = htons(realm_len);
        attrib->type = htons(TURN_ATTR_TYPE_REALM);
        memcpy(attrib->value, realm, realm_len);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_username(Vector *vector, uint8_t *username, uint8_t username_len)
{
    allocator_t *allocator;
    turn_attrib_username_t *attrib;
    int ret;

    TRY {
        THROW_IF(username_len == 0, 0);
        SET_CATCH_INT_PARS(username_len, 0);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_username_t) + username_len);
        attrib->len = htons(username_len);
        attrib->type = htons(TURN_ATTR_TYPE_USERNAME);
        memcpy(attrib->value, username, username_len);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_lifetime(Vector *vector, int lifetime)
{
    allocator_t *allocator;
    turn_attrib_lifetime_t *attrib;
    int ret;

    TRY {
        THROW_IF(lifetime < 0, 0);
        SET_CATCH_INT_PARS(lifetime, 0);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_lifetime_t));
        attrib->len = htons(4);
        attrib->type = htons(TURN_ATTR_TYPE_LIFETIME);
        attrib->value = htonl(lifetime);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_requested_family(Vector *vector, uint8_t family)
{
    allocator_t *allocator;
    turn_attrib_requested_family_t *attrib;
    int ret;

    TRY {
        THROW_IF(family != 1 || family != 2, 0);
        SET_CATCH_INT_PARS(family, 0);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_requested_family_t));
        attrib->len = htons(4);
        attrib->type = htons(TURN_ATTR_TYPE_REQUESTED_FAMILY);
        attrib->family = family;

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_integrity(Vector *vector, uint8_t *value, int len)
{
    allocator_t *allocator;
    turn_attrib_integrity_t *attrib;
    int ret;

    TRY {
        SET_CATCH_INT_PARS(len, 0);
        THROW_IF(len != 20, -1);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_integrity_t));
        attrib->len = htons(len);
        attrib->type = htons(TURN_ATTR_TYPE_INTEGRITY);
        memcpy(attrib->value, value, len);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_xor_peer_address(Vector *vector, struct addrinfo *addr, uint32_t cookie)
{
    allocator_t *allocator;
    turn_attrib_xor_peered_address_t *attrib;
    struct sockaddr_in ipv4;
    struct sockaddr_in6 ipv6;
    uint8_t *ipaddr, *cookie_addr = &cookie;
    uint8_t len, family;
    uint16_t port, msb_cookie;
    int ret;

    TRY {
        allocator = vector->obj.allocator;

        switch (addr->ai_family) {
            case AF_INET:
                memcpy(&ipv4, addr->ai_addr, sizeof(struct sockaddr_in));
                port = ntohs(ipv4.sin_port);
                ipaddr = (uint8_t *)&ipv4.sin_addr;
                len  = 4;
                family = 1;
                break;
            case AF_INET6:
                THROW(0);
                break;
            default:
                THROW(0);
                break;
        }

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_xor_peered_address_t) + len);
        cookie = htonl(cookie);
        /* host order port XOR most-significant 16 bits of the cookie */
        msb_cookie = ((uint8_t*)cookie_addr)[0] << 8 | ((uint8_t*)cookie_addr)[1];
        port ^= msb_cookie;

        for (int i = 0; i < len; i++) {
            ipaddr[i] ^= cookie_addr[i];
        }

        attrib->type = htons(TURN_ATTR_TYPE_XOR_PEER_ADDRESS);
        attrib->len = htons(4 + len);
        attrib->family = family;
        attrib->port = htons(port); 

        memcpy(&attrib->u, ipaddr, len);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

int turn_set_attrib_data(Vector *vector, uint8_t *value, int len)
{
    allocator_t *allocator;
    turn_attrib_data_t *attrib;
    int ret;

    TRY {
        SET_CATCH_INT_PARS(len, 0);
        THROW_IF(len > 2048, -1);
        allocator = vector->obj.allocator;

        attrib = allocator_mem_zalloc(allocator, sizeof(turn_attrib_data_t) + len);
        attrib->len = htons(len);
        attrib->type = htons(TURN_ATTR_TYPE_DATA);
        memcpy(attrib->value, value, len);

        vector->add_back(vector, attrib);
    } CATCH (ret) {
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}
