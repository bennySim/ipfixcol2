
#include "Builder.hpp"

#define DEFAULT_MSG_SIZE 1500

void
msg_factory::init(uint32_t exp_time, uint32_t seq_num, uint32_t odid)
{
    const size_t alloc_size = DEFAULT_MSG_SIZE * sizeof(uint8_t);
    fds_ipfix_msg_hdr hdr;

    m_msg.reset(reinterpret_cast<uint8_t *>(malloc(alloc_size)));
    if (!m_msg) {
        throw std::bad_alloc();
    }

    m_msg_alloc = alloc_size;
    m_msg_size = 0;

    m_sets.clear();
    m_drecs.clear();

    m_set_offset = 0;

    // Add IPFIX Message header
    hdr.version = htons(FDS_IPFIX_VERSION);
    hdr.length = 0; // Will be filled later
    hdr.seq_num = htonl(seq_num);
    hdr.export_time = htonl(exp_time);
    hdr.odid = htonl(odid);
    append_bytes(&hdr, sizeof(hdr));
}

void
msg_factory::add_drec(
    const void *data,
    uint16_t data_size,
    const fds_tsnapshot_t *snapshot,
    uint16_t template_id)
{
    struct drec_info info;

    if (!m_msg) {
        throw std::runtime_error("IPFIX Message hasn't been initialized yet");
    }

    if (set_current_id() != template_id) {
        set_open(template_id);
    }

    info.offset = m_msg_size;
    info.size = data_size;
    info.snapshot = snapshot;
    info.tmplt = fds_tsnapshot_template_get(snapshot, template_id);

    if (!info.tmplt) {
        throw std::runtime_error("Required template doesn't exists in the snapshot");
    }

    append_bytes(data, data_size);
    m_drecs.push_back(info);
}

struct ipx_msg_ipfix *
msg_factory::build(const ipx_ctx_t *plugin_ctx, const ipx_msg_ctx *msg_ctx)
{
    struct fds_ipfix_msg_hdr *hdr;
    struct ipx_msg_ipfix *wrapper; // Unable use unique_pointer due to ipx_msg_ipfix_add_drec_ref()
    uint8_t *raw_data;

    if (!m_msg) {
        throw std::runtime_error("IPFIX Message hasn't been initialized yet");
    }

    // Finalize last Set header and IPFIX Message header (i.e. fill block sizes)
    set_close();
    hdr = reinterpret_cast<fds_ipfix_msg_hdr *>(m_msg.get());
    hdr->length = htons(m_msg_size);

    wrapper = ipx_msg_ipfix_create(plugin_ctx, msg_ctx, m_msg.get(), m_msg_size);
    if (!wrapper) {
        throw std::bad_alloc();
    }

    m_msg.release(); // wrapper now owns memory of the raw IPFIX Message
    raw_data = ipx_msg_ipfix_get_packet(wrapper);

    for (const auto &set : m_sets) {
        struct ipx_ipfix_set *set_desc = ipx_msg_ipfix_add_set_ref(wrapper);
        if (!set_desc) {
            ipx_msg_ipfix_destroy(wrapper);
            throw std::bad_alloc();
        }

        set_desc->ptr = reinterpret_cast<fds_ipfix_set_hdr *>(raw_data + set.offset);
    }

    for (const auto &drec : m_drecs) {
        struct ipx_ipfix_record *rec_desc = ipx_msg_ipfix_add_drec_ref(&wrapper);
        if (!rec_desc) {
            ipx_msg_ipfix_destroy(wrapper);
            throw std::bad_alloc();
        }

        rec_desc->rec.data = raw_data + drec.offset;
        rec_desc->rec.size = drec.size;
        rec_desc->rec.snap = drec.snapshot;
        rec_desc->rec.tmplt = drec.tmplt;
    }

    return wrapper;
}

void
msg_factory::append_bytes(const void *data, uint16_t size)
{
    if (m_msg_size + size > UINT16_MAX) {
        throw std::runtime_error("Too big IPFIX Message cannot be created");
    }

    if (m_msg_size + size > m_msg_alloc) {
        const size_t new_size = (m_msg_size + size) * 2; // improve me!
        void *new_buffer = realloc(m_msg.get(), new_size);
        if (!new_buffer) {
            throw std::bad_alloc();
        }

        m_msg.release(); // Dont free reallocated memory!
        m_msg.reset(reinterpret_cast<uint8_t *>(new_buffer));
        m_msg_alloc = new_size;
    }

    memcpy(&m_msg.get()[m_msg_size], data, size);
    m_msg_size += size;
}

void
msg_factory::set_open(uint16_t id)
{
    const bool is_template_set =
        (id == FDS_IPFIX_SET_TMPLT || id == FDS_IPFIX_SET_OPTS_TMPLT);
    const bool is_data_set = (id >= FDS_IPFIX_SET_MIN_DSET);
    fds_ipfix_set_hdr hdr;

    if (!is_template_set && !is_data_set) {
        throw std::runtime_error("Invalid Set ID" + std::to_string(id));
    }

    set_close();    // Close any previously opened IPFIX Set

    hdr.flowset_id = htons(id);
    hdr.length = 0; // Will be update by set_close()

    m_set_offset = m_msg_size;
    append_bytes(&hdr, sizeof(hdr));

    m_sets.push_back({m_set_offset});
}

/**
 * @brief Get current IPFIX Set ID
 * @return Set ID or zero (i.e. no set opened)
 */
uint16_t
msg_factory::set_current_id()
{
    const fds_ipfix_set_hdr *hdr;

    if (m_set_offset == 0) {
        return 0;
    }

    hdr = reinterpret_cast<fds_ipfix_set_hdr *>(&m_msg.get()[m_set_offset]);
    return ntohs(hdr->flowset_id);
}

void
msg_factory::set_close()
{
    fds_ipfix_set_hdr *hdr;
    size_t set_size;

    if (m_set_offset == 0) {
        return;
    }

    set_size = m_msg_size - m_set_offset; // Size of IPFIX Set header + added records

    hdr = reinterpret_cast<fds_ipfix_set_hdr *>(&m_msg.get()[m_set_offset]);
    hdr->length = htons(set_size);

    m_set_offset = 0;
}
