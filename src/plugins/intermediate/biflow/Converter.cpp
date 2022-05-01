/**
 * \file src/plugins/intermediate/biflow/Converter.cpp
 * \author Simona Bennárová
 * \brief Converter functions for biflow plugin (source file)
 * \date 2021
 */

#include <ipfixcol2.h>
#include "Converter.hpp"
#include "IanaDefinitions.h"

template<typename T>
int
extract_ip(fds_drec *drec, uint32_t pen, uint16_t id, IPAddr *dst) {
    struct fds_drec_field field{};
    int ret_code;

    ret_code = fds_drec_find(drec, pen, id, &field);
    if (ret_code == FDS_EOC) {
        return IPX_ERR_NOTFOUND;
    }

    T ip;
    ret_code = fds_get_ip(field.data, field.size, &ip);
    if (ret_code != FDS_OK) {
        return IPX_ERR_FORMAT;
    }
    *dst = IPAddr(ip);
    return IPX_OK;
}

int
extract_ipv4(fds_drec *drec, uint32_t pen, uint16_t id, IPAddr *dst) {
    return extract_ip<uint32_t>(drec, pen, id, dst);
}

int
extract_ipv6(fds_drec *drec, uint32_t pen, uint16_t id, IPAddr *dst) {
    return extract_ip<uint8_t[16]>(drec, pen, id, dst);
}

int
extract_ip_universal(fds_drec *drec, IPAddr *dst, uint16_t ipv4_element_id, uint16_t ipv6_element_id) {
    int ret_code = extract_ipv4(drec, IANA_PEN, ipv4_element_id, dst);
    if (ret_code != IPX_ERR_NOTFOUND) {
        return ret_code;
    }
    ret_code = extract_ipv6(drec, IANA_PEN, ipv6_element_id, dst);
    return ret_code;
}

int
extract_ip_src(fds_drec *drec, IPAddr *dst) {
    return extract_ip_universal(drec, dst, IANA_SRC_IPV4, IANA_SRC_IPV6);
}

int
extract_ip_dst(fds_drec *drec, IPAddr *dst) {
    return extract_ip_universal(drec, dst, IANA_DST_IPV4, IANA_DST_IPV6);
}