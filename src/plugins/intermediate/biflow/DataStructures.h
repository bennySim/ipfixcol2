/**
 * \file src/plugins/intermediate/biflow/DataStructures.h
 * \author Simona Bennárová
 * \brief Data structures definitions
 * \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BIFLOW_DATASTRUCTURES_H
#define BIFLOW_DATASTRUCTURES_H

#include <ipfixcol2.h>
#include <memory>
#include <lzma.h>
#include "IPAddr.hpp"

/**
 * Structure for parsed key from record
 * Used as key in cache in Storage class
 */
struct key {
    IPAddr ip_addr_src;     /**  Source IP address               */
    IPAddr ip_addr_dst;     /**  Destination IP address          */
    uint16_t port_src = 0;  /**  Source port                     */
    uint16_t port_dst = 0;  /**  Destination port                */
    uint8_t protocol = 0;   /**  Protocol number                 */
    uint64_t timestamp;     /**  Time of arrival to plugin       */
};

/**
 * Class for computing hash for key structure
 */
class HashFunction {
public:
    /**
     * \brief Calculate hash from all fields in structure key
     * \param[in] key       record key
     * \return hash for given key
     */
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

/**
 * class Record used for storing parsed record data
 */
class Record {
public:
    data_ptr data;             /** Raw data of record                 */
    uint16_t size;             /** Record's size in bytes             */
    uint16_t tmplt_id;         /** ID of template describing record   */

    /**
     * \brief Class destructor
     */
    ~Record();

    /**
     * \brief Class constructor
     */
    Record();
};

/**
 * \brief Function for ordering keys
 *
 * \param key1[in]      key
 * \param key2[in]      key
 * \return true if key1 is smaller than key2, false otherwise
 */
bool operator<(const key &key1, const key &key2);

/**
 * \brief  Function for equality of keys
 *
 * \param[in] key1      key
 * \param[in] key2      key
 * \return true if keys are the same, false otherwise
 */
bool operator==(const key &key1, const key &key2);

/**
 * \brief Function for extracting key from record
 *
 * \param[in] drec                  original record structure
 * \param[out] key                  extracted key, filled in function
 * \param[in] pair_missing_ports    parameter from configuration pairMissingPorts
 * \return true if extraction was successful, false otherwise
 */
int get_record_key(fds_drec& drec, struct key &key, bool pair_missing_ports);

/**
 * \brief Create reversed key for a key
 *
 * \param[in] key               key
 * \param[out] reversed_key     reverse key for \param key, filled in this function
 */
void get_reversed_key(const key &key, struct key *reversed_key);

/**
 * \return current time in milliseconds
 */
inline uint64_t get_current_timestamp() {
    struct timespec monotime{};
    clock_gettime(CLOCK_MONOTONIC, &monotime);
    return monotime.tv_sec;
}

#endif //BIFLOW_DATASTRUCTURES_H
