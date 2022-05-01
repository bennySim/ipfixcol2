/**
 * \file src/plugins/intermediate/biflow/TimeExpiration.cpp
 * \author Simona Bennárová
 * \brief Class for handling time expiration of records (source file)
 * \date 2022
 */

#include "TimeExpiration.h"

TimeExpiration::TimeExpiration(uint32_t timeout_cache) : TIMEOUT_CACHE(timeout_cache) {
    current_timestamp = timestamps.begin();

}

void
TimeExpiration::insert_key_timestamp(const key &key) {
    if (timestamps.empty() || current_timestamp->is_old(key.timestamp)) {
        TimestampedBucket bucket(key.timestamp);
        if (timestamps.empty()) {
            timestamps.push_front(bucket);
            current_timestamp = timestamps.begin();
        } else {
            timestamps.insert_after(current_timestamp, bucket);
            current_timestamp++;
        }
    }
    current_timestamp->add_key(key);
}

void
TimeExpiration::get_expired_records(std::vector<struct key>& expired_keys) {
    uint32_t timestamp_sec = get_current_timestamp();
    while (!timestamps.empty() && timestamps.front().is_expired(timestamp_sec, TIMEOUT_CACHE)) {
        std::vector<struct key> &keys = timestamps.front().get_keys();
        expired_keys.insert(expired_keys.end(), keys.begin(), keys.end());
        timestamps.pop_front();
    }
}

TimestampedBucket::TimestampedBucket(std::time_t timestamp_sec)
        : timestamp(timestamp_sec) {
}

bool
TimestampedBucket::is_expired(std::time_t time_now_sec, uint32_t timeout_sec) const {
    return time_now_sec - timeout_sec > timestamp;
}

std::vector<struct key> &
TimestampedBucket::get_keys() {
    return keys;
}

void
TimestampedBucket::add_key(struct key key) {
    keys.push_back(key);
}

bool
TimestampedBucket::is_old(std::time_t time_now_sec) const {
    return time_now_sec > timestamp;
}
