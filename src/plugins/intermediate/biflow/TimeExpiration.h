/**
 * \file src/plugins/intermediate/biflow/TimeExpiration.h
 * \author Simona Bennárová
 * \brief Class for handling time expiration of records (header file)
 * \date 2022
 */

#ifndef BIFLOW_TIMEEXPIRATION_H
#define BIFLOW_TIMEEXPIRATION_H


#include <forward_list>
#include <vector>
#include <ctime>
#include "DataStructures.h"

class TimestampedBucket {
private:
    std::vector<struct key> keys;
    const std::time_t timestamp;
public:
    explicit TimestampedBucket(std::time_t timestamp);

    bool is_expired(std::time_t time_now_sec, uint32_t timeout) const;

    std::vector<struct key> &get_keys();

    void add_key(struct key key);

    bool is_old(std::time_t time_now_sec) const;
};

class TimeExpiration {
private:

    std::forward_list<TimestampedBucket> timestamps;
    std::forward_list<TimestampedBucket>::iterator current_timestamp;

    const uint32_t TIMEOUT_CACHE;
public:

    explicit TimeExpiration(uint32_t timeout_cache);

    // TODO, what if ket already exists in cache?
    void insert_key_timestamp(const key &key);

    void get_expired_records(std::vector<struct key> &expired_keys);
};


#endif //IPFIXCOL2_TIMEEXPIRATION_H
