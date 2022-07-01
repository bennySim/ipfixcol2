/**
* \file src/plugins/intermediate/biflow/Storage.hpp
* \author Simona Bennárová
* \brief Biflow storage (header file)
* \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef BIFLOW_STORAGE_HPP
#define BIFLOW_STORAGE_HPP

#include <ipfixcol2.h>
#include <unordered_map>
#include <forward_list>
#include "DataStructures.h"
#include "Builder.hpp"
#include "Generator.h"
#include "Config.hpp"
#include "MsgGen.h"
#include "BiflowTemplate.h"
#include "TimeExpiration.h"
#include "MsgSender.h"
#include "TemplatesHandler.h"

class Storage {
private:
    /** Template manager for plugin                     **/
    fds_tmgr_t *tmgr;
    /** Configuration parameter ignoreMissingReverse    **/
    const bool IGNORE_MISSING_REVERSE;
    /** Configuration parameter pairMissingPorts        **/
    const bool PAIR_MISSING_PORTS;
    /** Cache for storing processed records             **/
    std::unordered_map<struct key, Record*, HashFunction> record_cache;
    /** Class for handling message building and sending **/
    MsgSender msg_sender;
    /** Class for handling time expiration algorithm    **/
    TimeExpiration time_expiration;
    /** Class for handling templates                    **/
    TemplatesHandler template_handler;

    // Initialize template manager
    void init_tmgr(ipx_ctx_t *ctx);
    // Check whether reversed key exists in cache
    bool has_reversed_key(const struct key &key);
    // Delete record specified by key from cache
    void delete_record(const struct key &key);
    // Delete record specified by reversed key from cache
    void delete_reversed_record(const key &key);
    // Retrieve data associated with key from cache
    int get_data_for_key(const key &key, Record **record);
    // Retrieve data associated with reversed key from cache
    int get_data_for_reversed_key(const key &key, Record **record);
    // Store processed record in the cache
    void store_record_in_cache(const struct key &key, fds_drec& record_data);
    // Build biflow record
    int create_biflow_record(struct key &reversed_key, fds_drec &reversed_data);
    // Add unprocessed record to message
    void add_raw_record_to_message(fds_drec &drec);
    // Add field to new record (check if is allowed to add)
    void add_field_to_drec(ipfix_drec *rec, Generator *tmplt_generator, fds_drec_iter &iter, bool is_reversed,
                           bool biflow_tmplt_exists) const;
    // Process each field of record and add them to new record
    void add_record_fields_to_drec(Record *record_data, ipfix_drec &drec, Generator &tmplt_generator,
                                   const fds_template *tmplt, bool is_reversed, bool biflow_tmplt_exists);
    void add_record_fields_to_drec(fds_drec &record_data, ipfix_drec &drec, Generator &tmplt_generator, bool is_reversed, bool biflow_tmplt_exists);
    // Extract data from unprocessed record
    Record * get_record_data(fds_drec &drec);
    // Process one record from message
    void process_record(fds_drec &drec);
    // Add all records remained in cache to message
    void send_all_remaining_records();

    // Find all expired records in cache and add them to message
    void send_expired_records();
public:

    /**
     * \brief Class Storage constructor
     * \param[in] ctx       Instance context
     * \param[in] config    Parsed instance configuration
     */
    explicit Storage(ipx_ctx_t *ctx, struct configuration &config);

    /** \brief Class Storage destructor                    **/
    ~Storage();

    /**
     * \brief Process message arrived to plugin
     * \param[in] msg   Message
     */
    void process_message(ipx_msg_t *msg);
};

#endif //BIFLOW_STORAGE_HPP
