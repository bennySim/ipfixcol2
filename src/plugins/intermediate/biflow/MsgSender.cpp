/**
* \file src/plugins/intermediate/biflow/MsgSender.cpp
* \author Simona Bennárová
* \brief Class for sending messages (source file)
* \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "MsgSender.h"
#include "DataStructures.h"


#define MSG_HEADER_SIZE     sizeof(fds_ipfix_msg_hdr)

void
MsgSender::init_message() {
    struct timespec realtime{};
    clock_gettime(CLOCK_REALTIME, &realtime);
    current_message.init(htonl(realtime.tv_sec), htonl(0), ODID);
}

void
MsgSender::init_msg_ctx() {
    msg_ctx.stream = 0;
    msg_ctx.odid = ODID;
    msg_ctx.session = ipx_session_new_file("intermediate:biflow");

    // Open session
    ipx_msg_session_t *open_event = ipx_msg_session_create(msg_ctx.session, IPX_MSG_SESSION_OPEN);
    if (!open_event) {
        IPX_CTX_WARNING(m_ctx, "Failed to create a Session message! Instances of "
                               "plugins will not be informed about the new Transport Session '%s' (%s:%d).",
                        msg_ctx.session->ident, __FILE__, __LINE__);
    } else {
        if (ipx_ctx_msg_pass(m_ctx, ipx_msg_session2base(open_event)) != IPX_OK) {
            ipx_msg_session_destroy(open_event);
            throw std::runtime_error("Failed to pass a Transport Session notification");
        }
    }
}

MsgSender::MsgSender(ipx_ctx_t *m_ctx, uint32_t timeout_msg, uint32_t odid)
        : ODID(odid), m_ctx(m_ctx), TIMEOUT_MSG(timeout_msg) {
    init_message();
    last_flushed_msg_timestamp = get_current_timestamp();
}

MsgSender::~MsgSender() {
    // Close session
    ipx_msg_session_t *close_event = ipx_msg_session_create(msg_ctx.session, IPX_MSG_SESSION_CLOSE);
    ipx_ctx_msg_pass(m_ctx, ipx_msg_session2base(close_event));

    send_garbage_msg((void *) msg_ctx.session, (ipx_msg_garbage_cb) ipx_session_destroy);
}

bool
MsgSender::timeout_expired() const {
    uint64_t time_now = get_current_timestamp();
    return TIMEOUT_MSG == 0 || time_now - TIMEOUT_MSG > last_flushed_msg_timestamp;
}

void
MsgSender::send_garbage_msg(void *object, ipx_msg_garbage_cb garbage_cb) {
    ipx_msg_garbage_t *msg_garbage = ipx_msg_garbage_create(object, garbage_cb);
    if (!msg_garbage) {
        throw std::runtime_error("Failed to allocate memory for garbage message!");
    }

    int ret_code = ipx_ctx_msg_pass(m_ctx, ipx_msg_garbage2base(msg_garbage));
    if (ret_code != IPX_OK) {
        throw std::runtime_error("Plugin does not have permissions to send a message");
    }
}

void
MsgSender::clean_garbage(fds_tmgr *tmgr) {
    fds_tgarbage_t *fds_garbage;
    if (fds_tmgr_garbage_get(tmgr, &fds_garbage) == FDS_OK && fds_garbage != nullptr) {
        send_garbage_msg(fds_garbage, (ipx_msg_garbage_cb) fds_tmgr_garbage_destroy);
    }
}

bool
MsgSender::is_msg_empty() {
    return current_message.get_size() == MSG_HEADER_SIZE;
}

void
MsgSender::send_message(fds_tmgr *tmgr) {
    if (!is_init_session) {
        init_msg_ctx();
        is_init_session = true;
    }

    if (is_msg_empty()) {
        last_flushed_msg_timestamp = get_current_timestamp();
        return;
    }

    // Create IPFIX envelope for new message
    ipx_msg_ipfix_t *new_msg = current_message.build(m_ctx, &msg_ctx);

    // Always pass the message
    int ret_code = ipx_ctx_msg_pass(m_ctx, ipx_msg_ipfix2base(new_msg));
    last_flushed_msg_timestamp = get_current_timestamp();
    if (ret_code != IPX_OK) {
        throw std::runtime_error("Plugin does not have permissions to send a message");
    }

    clean_garbage(tmgr);
}

void
MsgSender::check_message_timeout(fds_tmgr *tmgr) {
    if (timeout_expired()) {
        send_message(tmgr);
        init_message();
    }
}

void
MsgSender::add_record_to_message(uint8_t *data, uint16_t size, int tmplt_id, fds_tmgr *tmgr) {
    if (!is_init_session) {
        init_msg_ctx();
        is_init_session = true;
    }
    uint32_t total_size = current_message.get_size() + size + MSG_HEADER_SIZE;
    if (total_size > UINT16_MAX) {
        send_message(tmgr);
        init_message();
    }

    // Get snapshot
    const fds_tsnapshot_t *snapshot;
    int ret_code = fds_tmgr_snapshot_get(tmgr, &snapshot);
    if (ret_code == FDS_ERR_NOMEM) {
        throw std::runtime_error("Failed to allocate memory for snapshot!");
    }
    if (ret_code == FDS_ERR_ARG) {
        throw std::runtime_error("Failed to get snapshot, time context is not defined!");
    }

    // Add to current message
    current_message.add_drec(data, size, snapshot, tmplt_id);
    seq_num++;
}
