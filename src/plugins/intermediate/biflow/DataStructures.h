/**
 * \file src/plugins/intermediate/biflow/DataStructures.h
 * \author Simona Bennárová
 * \brief Data structures definitions
 * \date 2021
 */

#ifndef BIFLOW_DATASTRUCTURES_H
#define BIFLOW_DATASTRUCTURES_H

#include <ipfixcol2.h>
#include <memory>
#include <lzma.h>
#include "IPAddr.hpp"

struct key {
    IPAddr ip_addr_src;
    IPAddr ip_addr_dst;
    uint16_t port_src = 0;
    uint16_t port_dst = 0;
    uint8_t protocol = 0;
    uint64_t timestamp;
};

class HashFunction {
public:
    size_t operator()(const key& key) const {
        if (key.ip_addr_dst.is_ip6()) {
            uint8_t id[37] = {0};
            memcpy(id + 0, &key.ip_addr_src, 16);
            memcpy(id + 16, &key.ip_addr_dst, 16);
            memcpy(id + 32, &key.port_src, 2);
            memcpy(id + 34, &key.port_dst, 2);
            memcpy(id + 36, &key.protocol, 1);
            return lzma_crc64(id, 37, 0);
        } else {
            uint8_t id[13] = {0};
            uint32_t ipv4 = key.ip_addr_src.get_ip4_as_uint();
            memcpy(id + 0, &ipv4, 4);
            ipv4 = key.ip_addr_dst.get_ip4_as_uint();
            memcpy(id + 4, &ipv4, 4);
            memcpy(id + 8, &key.port_src, 2);
            memcpy(id + 10, &key.port_dst, 2);
            memcpy(id + 12, &key.protocol, 1);
            return lzma_crc64(id, 13, 0);
        }
    }
};

using data_ptr = std::unique_ptr<uint8_t, decltype(&free)>;

class record {
public:
    data_ptr data;             /**< Start of the record                     */
    uint16_t size;             /**< Size of the record (in bytes)           */
    uint16_t tmplt_id;
    ~record();

    record();
};

bool operator<(const key &key1, const key &key2);

bool operator==(const key &key1, const key &key2);

int get_record_key(fds_drec& drec, struct key &key, bool pair_missing_ports);

void get_reversed_key(const key &key, struct key *reversed_key);

inline uint64_t get_current_timestamp() {
    struct timespec monotime{};
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    return monotime.tv_sec;
}

#endif //BIFLOW_DATASTRUCTURES_H
