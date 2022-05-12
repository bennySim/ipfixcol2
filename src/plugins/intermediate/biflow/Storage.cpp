/**
* \file src/plugins/intermediate/biflow/Storage.cpp
* \author Simona Bennárová
* \brief Biflow storage (source file)
* \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "Storage.hpp"
#include "IanaDefinitions.h"
#include <algorithm>
#include <unordered_map>

void
Storage::init_tmgr(ipx_ctx_t *ctx) {
    // Create new Template Manager
    tmgr = fds_tmgr_create(FDS_SESSION_FILE);

    // Add Information manager to tmgr
    const fds_iemgr_t *iemgr = ipx_ctx_iemgr_get(ctx);
    int rc = fds_tmgr_set_iemgr(tmgr, iemgr);
    if (rc != FDS_OK) {
        throw std::runtime_error("Failed to add IE definitions to template manager!");
    }

    // Set time
    rc = fds_tmgr_set_time(tmgr, 0);
    if (rc != FDS_OK) {
        throw std::runtime_error("Failed to set time context in template manager!");
    }
}

Storage::Storage(ipx_ctx_t *ctx, struct configuration &config)
        : IGNORE_MISSING_REVERSE(config.ignore_missing_reverse), PAIR_MISSING_PORTS(config.pair_missing_ports),
          msg_sender(ctx, config.timeout_msg, config.odid), time_expiration(config.timeout_cache) {
    init_tmgr(ctx);
}

void
Storage::send_all_remaining_records() {

    if (record_cache.empty()) {
        return;
    }

    for (auto &record_pair : record_cache) {
        // Add record to message
        Record *record_data = record_pair.second.get();
        msg_sender.add_record_to_message(record_data->data.get(), record_data->size, record_data->tmplt_id, tmgr);
    }
}

Storage::~Storage() {
    send_all_remaining_records();
    msg_sender.send_message(tmgr);
    msg_sender.send_garbage_msg(tmgr, (ipx_msg_garbage_cb) fds_tmgr_destroy);
}

bool
Storage::has_reversed_key(const struct key &key) {
    struct key reversed_key{};
    get_reversed_key(key, &reversed_key);
    return record_cache.find(reversed_key) != record_cache.end();
}

void
Storage::get_record_data(fds_drec &drec, std::unique_ptr<Record> &record_data_ptr) {
    // Flush to prevent missing template
    if (template_handler.is_full()) {
        send_all_remaining_records();
    }
    template_handler.set_uniflow_template_id(drec.tmplt, record_data_ptr->tmplt_id, &tmgr);

    uint8_t *data2copy = static_cast<uint8_t *>(std::malloc(drec.size));
    if (!data2copy) {
        throw std::runtime_error("Failed to allocate memory for record data!");
    }

    std::memcpy(data2copy, drec.data, drec.size);
    record_data_ptr->data.reset(data2copy);

    record_data_ptr->size = drec.size;
}

int
Storage::get_data_for_key(const key &key, Record **record) {
    auto element = record_cache.find(key);
    if (element == record_cache.end()) {
        return IPX_ERR_NOTFOUND;
    }
    *record = element->second.get();
    return IPX_OK;
}

int
Storage::get_data_for_reversed_key(const key &key, Record **record) {
    struct key reversed_key;
    get_reversed_key(key, &reversed_key);
    return get_data_for_key(reversed_key, record);
}

void
Storage::delete_record(const key &key) {
    record_cache.erase(key);
}

void
Storage::delete_reversed_record(const key &key) {
    struct key reversed_key{};
    get_reversed_key(key, &reversed_key);
    delete_record(reversed_key);
}

void
Storage::store_record_in_cache(const struct key &key, std::unique_ptr<Record> &record_data_ptr) {

    record_cache.insert({key, std::move(record_data_ptr)});

    time_expiration.add_expiration_for_key(key);
}

void
Storage::add_field_to_drec(ipfix_drec *rec, Generator *tmplt_generator, fds_drec_iter &iter, bool is_reversed,
                           bool biflow_tmplt_exists) const {

    uint16_t element_id = iter.field.info->id;
    uint32_t enterprise_id = iter.field.info->en;
    uint16_t length = iter.field.info->length;

    // Ignore:
    // * key fields
    // * non-reversible fields in reverse direction
    // * reversed fields with missing reverse definitions and configuration to ignore it
    // * ignored data types
    if (is_key_field(enterprise_id, element_id)
        || (is_reversed && is_non_reversible_field(enterprise_id, element_id))
        || (is_reversed && has_missing_reverse_definition(iter) && IGNORE_MISSING_REVERSE)
        || (!has_missing_reverse_definition(iter) && is_ignored_type(iter.field.info->def->data_type))) {
        return;
    }

    // Append to record
    rec->append_field(iter.field);

    if (biflow_tmplt_exists) {
        return;
    }
    // Append to template
    if (!is_reversed || has_missing_reverse_definition(iter)) {
        tmplt_generator->append(element_id, length, enterprise_id);
    } else if (enterprise_id != IANA_PEN) {
        tmplt_generator->append(iter.field.info->def->reverse_elem->id, length, enterprise_id);
    } else {
        tmplt_generator->append(element_id, length, IANA_PEN_REVERSED);
    }
}

void
Storage::add_record_fields_to_drec(Record *record_data, ipfix_drec &drec, Generator &tmplt_generator,
                                   const fds_template *tmplt, bool is_reversed, bool biflow_tmplt_exists) {
    struct fds_drec drec_original;
    drec_original.data = record_data->data.get();
    drec_original.tmplt = tmplt;
    drec_original.size = record_data->size;

    struct fds_drec_iter iter;
    fds_drec_iter_init(&iter, &drec_original, 0);
    while (fds_drec_iter_next(&iter) != FDS_EOC) {
        add_field_to_drec(&drec, &tmplt_generator, iter, is_reversed, biflow_tmplt_exists);
    }
}

void
add_key_fields_to_template(key &reversed_key, Generator *tmplt_generator) {
    // Append IP addresses (according to reverse key)
    if (reversed_key.ip_addr_dst.is_ip6()) {
        tmplt_generator->append(IANA_SRC_IPV6, IANA_IPV6_SIZE);
    } else {
        tmplt_generator->append(IANA_SRC_IPV4, IANA_IPV4_SIZE);
    }

    if (reversed_key.ip_addr_src.is_ip6()) {
        tmplt_generator->append(IANA_DST_IPV6, IANA_IPV6_SIZE);
    } else {
        tmplt_generator->append(IANA_DST_IPV4, IANA_IPV4_SIZE);
    }

    // Append ports (according to reverse key) if exists
    if (reversed_key.port_dst != 0) {
        tmplt_generator->append(IANA_SRC_PORT, IANA_PORT_SIZE);
    }

    if (reversed_key.port_src != 0) {
        tmplt_generator->append(IANA_DST_PORT, IANA_PORT_SIZE);
    }

    // Append protocol
    tmplt_generator->append(IANA_PROTOCOL_ID, IANA_PROTOCOL_SIZE);
}

void
add_key_fields_to_record(key &reversed_key, ipfix_drec &drec) {
    // Append IP addresses (according to reverse key)
    drec.append_ip(reversed_key.ip_addr_dst.to_string());
    drec.append_ip(reversed_key.ip_addr_src.to_string());

    // Append ports (according to reverse key) if exists
    if (reversed_key.port_dst != 0) {
        drec.append_uint(reversed_key.port_dst, IANA_PORT_SIZE);
    }
    if (reversed_key.port_src != 0) {
        drec.append_uint(reversed_key.port_src, IANA_PORT_SIZE);
    }

    // Append protocol
    drec.append_uint(reversed_key.protocol, IANA_PROTOCOL_SIZE);
}

int
Storage::create_biflow_record(struct key &reversed_key, Record *reversed_data) {
    // Initialize fields
    ipfix_drec drec{};
    Generator tmplt_generator{};
    BiflowTemplate biflow_template;

    // Get reverse key data
    Record *data = nullptr;
    get_data_for_reversed_key(reversed_key, &data);

    // Check if biflow template does not exists already
    bool biflow_tmplt_exists_t = template_handler.biflow_tmplt_exists(data->tmplt_id, reversed_data->tmplt_id,
                                                                      biflow_template);
    if (!biflow_tmplt_exists_t) {
        fds_tmgr_template_get(tmgr, data->tmplt_id, &biflow_template.tmplt);
        fds_tmgr_template_get(tmgr, reversed_data->tmplt_id, &biflow_template.reversed_tmplt);
    }

    // Add key fields to message and to template only if needed
    add_key_fields_to_record(reversed_key, drec);
    if (!biflow_tmplt_exists_t) {
        add_key_fields_to_template(reversed_key, &tmplt_generator);
    }

    // Add other fields from record and reversed record to the message and template
    add_record_fields_to_drec(data, drec, tmplt_generator, biflow_template.tmplt, false, biflow_tmplt_exists_t);
    add_record_fields_to_drec(reversed_data, drec, tmplt_generator, biflow_template.reversed_tmplt, true,
                              biflow_tmplt_exists_t);

    // Create biflow template if does not exist
    if (!biflow_tmplt_exists_t) {
        int ret_code = template_handler.create_biflow_template(tmplt_generator, biflow_template, &tmgr);
        if (ret_code != IPX_OK) { // TODO can happen?
            delete_reversed_record(reversed_key);
            return IPX_ERR_FORMAT;
        }
    }

    // Add record to message
    msg_sender.add_record_to_message(drec.front(), drec.size(), biflow_template.get_tmplt_id(), tmgr);
    delete_reversed_record(reversed_key);

    return IPX_OK;
}

void
Storage::add_raw_record_to_message(fds_drec &drec) {
    uint16_t tmplt_id;
    template_handler.set_uniflow_template_id(drec.tmplt, tmplt_id, &tmgr);
    msg_sender.add_record_to_message(drec.data, drec.size, tmplt_id, tmgr);
}

void
Storage::send_expired_records() {
    std::vector<struct key> expired_keys;
    time_expiration.get_expired_records(expired_keys);

    for (auto &expired_key : expired_keys) {
        auto record_pair = record_cache.find(expired_key);
        if (record_pair == record_cache.end() || record_pair->first.timestamp != expired_key.timestamp) {
            // record is not in the cache anymore
            continue;
        }
        Record *record_data = record_pair->second.get();

        // Add record to message
        msg_sender.add_record_to_message(record_data->data.get(), record_data->size, record_data->tmplt_id, tmgr);
        delete_record(expired_key);
    }

    msg_sender.check_message_timeout(tmgr);
}

void
Storage::process_record(fds_drec &drec) {

    // If record is already biflow or OPTION TEMPLATE record forward to output
    int ret_code;
    if (is_biflow_record(drec) || is_option_tmplt(drec)) {
        add_raw_record_to_message(drec);
        return;
    }

    // Get key fields
    struct key record_key{};
    ret_code = get_record_key(drec, record_key, PAIR_MISSING_PORTS);
    if (ret_code != IPX_OK) {
        // Some field was not found, send to output
        add_raw_record_to_message(drec);
        return;
    }

    // Get record data
    std::unique_ptr<Record> record_data_ptr(new Record);
    get_record_data(drec, record_data_ptr);

    // Check biflow
    if (has_reversed_key(record_key)) {
        ret_code = create_biflow_record(record_key, record_data_ptr.get());
        if (ret_code != IPX_OK) {
            // Error occured, send record to output
            add_raw_record_to_message(drec);
        }
        return;
    }

    // Add to cache
    auto iterator = record_cache.find(record_key);
    if (iterator != record_cache.end()) {
        // Key already in cache, send record and store newer to cache
        msg_sender.add_record_to_message(iterator->second->data.get(), iterator->second->size,
                                         iterator->second->tmplt_id, tmgr);
        iterator->second = std::move(record_data_ptr);
    } else {
        store_record_in_cache(record_key, record_data_ptr);
    }
}

void
Storage::process_message(ipx_msg_t *msg) {
    ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);

    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(ipfix_msg);

    // Process all records in message
    for (uint32_t record_index = 0; record_index < rec_cnt; ++record_index) {
        struct ipx_ipfix_record *rec_ptr = ipx_msg_ipfix_get_drec(ipfix_msg, record_index);
        process_record(rec_ptr->rec);
    }

    send_expired_records();

    ipx_msg_destroy(msg);
}
