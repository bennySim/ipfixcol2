/**
* \file src/plugins/intermediate/biflow/MsgSender.h
* \author Simona Bennárová
* \brief Class for sending messages (header file)
* \date 2021
*/

#ifndef BIFLOW_MSGSENDER_H
#define BIFLOW_MSGSENDER_H


#include "Builder.hpp"

class MsgSender {
private:
    const uint32_t ODID;
    const uint32_t TIMEOUT_MSG;

    msg_factory current_message{};
    ipx_msg_ctx msg_ctx{};
    ipx_ctx_t *m_ctx;
    uint32_t seq_num = 0;
    uint64_t last_flushed_msg_timestamp;

    bool is_init_session = false;

    void init_message();

    bool timeout_expired() const;

    void init_msg_ctx();

    void clean_garbage(fds_tmgr *tmgr);

    bool is_msg_empty();

public:

    MsgSender(ipx_ctx_t *m_ctx, uint32_t timeout_msg, uint32_t odid);

    ~MsgSender();

    void send_garbage_msg(void *object, ipx_msg_garbage_cb garbage_cb);

    void send_message(fds_tmgr *tmgr);

    void add_record_to_message(uint8_t *data, uint16_t size, int tmplt_id, fds_tmgr *tmgr);
};

#endif //BIFLOW_MSGSENDER_H
