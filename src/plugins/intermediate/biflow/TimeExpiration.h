/**
 * \file src/plugins/intermediate/biflow/TimeExpiration.h
 * \author Simona Bennárová
 * \brief Class for handling time expiration of records (header file)
 * \date 2022
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BIFLOW_TIMEEXPIRATION_H
#define BIFLOW_TIMEEXPIRATION_H

#include <forward_list>
#include <vector>
#include <ctime>
#include "DataStructures.h"

class TimestampedBucket {
private:
    /** Keys of record stored in bucket         **/
    std::vector<struct key> keys;
    /** Timestamp of creation of the bucket     **/
    const std::time_t timestamp;
public:

    /**
     * \brief Class constructor
     * \param[in] timestamp     creation of the bucket
     */
    explicit TimestampedBucket(std::time_t timestamp);

    /**
     * \brief Check whether the bucket is expired or not
     * \param time_now_sec      current time in seconds
     * \param timeout           timeout in seconds from configuration
     * \return true if bucket is expired, false otherwise
     */
    bool is_expired(std::time_t time_now_sec, uint32_t timeout) const;

    /**
     * \brief Get all keys in the bucket
     * \return all keys stored in bucket
     */
    std::vector<struct key> &get_keys();

    /**
     * \brief Add key to the bucket
     * \param key   key to be added
     */
    void add_key(struct key key);

    /**
     * \brief Checking whether the bucket is still actual
     *        (newly arrived keys can be added here)
     * \param time_now_sec      current time in seconds
     * \return false if time_now_sec is same as bucket timestamp,
     *         true otherwise
     */
    bool is_old(std::time_t time_now_sec) const;
};

class TimeExpiration {
private:
    /** List of all buckets                     **/
    std::forward_list<TimestampedBucket> timestamps;
    /** Refernce to newest timestamp            **/
    std::forward_list<TimestampedBucket>::iterator current_timestamp;
    /** Configuration parameter timeoutCache    **/
    const uint32_t TIMEOUT_CACHE;
public:
    /**
     * \brief Class constructor
     * \param timeout_cache     timeoutCache configuration parameter
     */
    explicit TimeExpiration(uint32_t timeout_cache);

    /**
     * \brief Insert key to correct bucket
     * \param key   key to insert
     */
    void add_expiration_for_key(const key &key);

    /**
     * \brief Find all keys, which are expired
     * \param expired_keys  List of all expired keys
     */
    void get_expired_records(std::vector<struct key> &expired_keys);
};

#endif //IPFIXCOL2_TIMEEXPIRATION_H
