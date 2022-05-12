/**
 * \file src/plugins/intermediate/biflow/DataStructures.cpp
 * \author Simona Bennárová
 * \brief Data structures definitions (source file)
 * \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DataStructures.h"
#include "IanaDefinitions.h"
#include "Converter.hpp"

#include <iostream>

Record::Record() : data(nullptr, &free) {}

Record::~Record() = default;

bool
operator<(const key &key1, const key &key2) {
    return key1.port_src < key2.port_src
           || key1.port_dst < key2.port_dst
           || key1.ip_addr_src < key2.ip_addr_src
           || key1.ip_addr_dst < key2.ip_addr_dst
           || key1.protocol < key2.protocol;
}

bool
operator==(const key &key1, const key &key2) {
    return key1.port_src == key2.port_src
           && key1.port_dst == key2.port_dst
           && key1.ip_addr_src == key2.ip_addr_src
           && key1.ip_addr_dst == key2.ip_addr_dst
           && key1.protocol == key2.protocol;
}

int
get_record_key(fds_drec &drec, struct key &key, bool pair_missing_ports) {
    int ret = 0;
    ret |= extract_uint(&drec, IANA_PEN, IANA_SRC_PORT, &key.port_src);
    ret |= extract_uint(&drec, IANA_PEN, IANA_DST_PORT, &key.port_dst);

    if (ret != 0 && !pair_missing_ports) {
        return IPX_ERR_FORMAT;
    }

    ret = extract_uint(&drec, IANA_PEN, IANA_PROTOCOL_ID, &key.protocol);
    ret |= extract_ip_dst(&drec, &key.ip_addr_dst);
    ret |= extract_ip_src(&drec, &key.ip_addr_src);
    if (ret != 0) {
        return IPX_ERR_FORMAT;
    }

    key.timestamp = get_current_timestamp();

    return IPX_OK;
}

void
get_reversed_key(const key &key, struct key *reversed_key) {
    reversed_key->ip_addr_src = key.ip_addr_dst;
    reversed_key->ip_addr_dst = key.ip_addr_src;
    reversed_key->port_src = key.port_dst;
    reversed_key->port_dst = key.port_src;
    reversed_key->protocol = key.protocol;
}