/**
* \file src/plugins/intermediate/biflow/Storage.hpp
* \author Simona Bennárová
* \brief Biflow storage (header file)
* \date 2021
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
    /** Plugin context (only for log!)              */
    fds_tmgr_t *tmgr;

    const bool IGNORE_MISSING_REVERSE;
    const bool PAIR_MISSING_PORTS;

    std::unordered_map<struct key, std::unique_ptr<record>, HashFunction> record_cache;

    MsgSender msg_sender;

    TimeExpiration time_expiration;

    TemplatesHandler template_handler;

    bool has_reversed_key(const struct key &key);

    void delete_record(const struct key &key);

    void delete_reversed_record(const key &key);

    int get_data_for_key(const key &key, record **record);

    int get_data_for_reversed_key(const key &key, record **record);

    void store_record_in_cache(const struct key &key, std::unique_ptr<record> &record_data_ptr);

    int create_biflow_record(struct key &reversed_key, record *reversed_data);

    void add_raw_record_to_message(fds_drec &drec);

    void send_expired_records();

    void init_tmgr(ipx_ctx_t *ctx);

    void process_record(fds_drec &drec);

    void get_record_data(fds_drec &drec, std::unique_ptr<record> &record_data_ptr);

    void send_all_remaining_records();

    void add_fields_to_drec(ipfix_drec *rec, Generator *tmplt_generator, fds_drec_iter &iter, bool is_reversed,
                            bool biflow_tmplt_exists) const;
    void add_record_to_drec(record *record_data, ipfix_drec &drec, Generator &tmplt_generator, const fds_template* tmplt, bool is_reversed, bool biflow_tmplt_exists);

public:

    explicit Storage(ipx_ctx_t *ctx, struct configuration& config);

    ~Storage();

    void process_message(ipx_msg_t *msg);
};


#endif //BIFLOW_STORAGE_HPP
