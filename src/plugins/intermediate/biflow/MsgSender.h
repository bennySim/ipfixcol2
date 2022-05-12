/**
* \file src/plugins/intermediate/biflow/MsgSender.h
* \author Simona Bennárová
* \brief Class for sending messages (header file)
* \date 2022
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef BIFLOW_MSGSENDER_H
#define BIFLOW_MSGSENDER_H


#include "Builder.hpp"

class MsgSender {
private:
    /** Parameter ODID from configuration                */
    const uint32_t ODID;
    /** Parameter timeoutMessage from configuration      */
    const uint32_t TIMEOUT_MSG;

    /** Current message builder structure                */
    msg_factory current_message{};
    /** Message context                                  */
    ipx_msg_ctx msg_ctx{};
    /** Plugin context                                   */
    ipx_ctx_t *m_ctx;
    /** Sequence number                                  */
    uint32_t seq_num = 0;
    /** Time when was previous message sent              */
    uint64_t last_flushed_msg_timestamp;
    /** Flag whether the session was already initialized */
    bool is_init_session = false;

    // Message initialization (sequence numbers, export time)
    void init_message();
    // Message context initialization
    void init_msg_ctx();
    // Checking message timeout expiration
    bool timeout_expired() const;
    // Gather all unused data and send them to pipeline
    void clean_garbage(fds_tmgr *tmgr);
    // Check whether message contains no records
    bool is_msg_empty();

public:
    /**
     * \brief Class constructor
     *
     * \param[in] m_ctx         plugin context
     * \param[in] timeout_msg   message timeout parameter from configuration
     * \param[in] odid          odid from configuration
     */
    MsgSender(ipx_ctx_t *m_ctx, uint32_t timeout_msg, uint32_t odid);

    /**
     * \brief Class destructor
     */
    ~MsgSender();

    /**
     * \brief Create garbage message from \param object and send it to pipeline
     *
     * \param[in] object        object to be destroyed by garbage message
     * \param[in] garbage_cb    destroy function for \param object
     */
    void send_garbage_msg(void *object, ipx_msg_garbage_cb garbage_cb);

    /**
     * \brief Trigger sending message now
     *
     * \param[in] tmgr  template manager
     */
    void send_message(fds_tmgr *tmgr);

    /**
     * \brief Append record to current message
     *
     * \param[in] data      record's data
     * \param[in] size      record's size
     * \param[in] tmplt_id  record's template id
     * \param[in] tmgr      template manager
     */
    void add_record_to_message(uint8_t *data, uint16_t size, int tmplt_id, fds_tmgr *tmgr);

    /**
     * \brief Trigger message expiration check
     *
     * \param[in] tmgr  template manager
     */
    void check_message_timeout(fds_tmgr *tmgr);
};

#endif //BIFLOW_MSGSENDER_H
