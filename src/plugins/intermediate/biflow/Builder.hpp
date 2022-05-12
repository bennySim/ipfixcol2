/**
 * @file   Builder.hpp
 * @brief  Message builder functions (header file)
 * @author Lukas Hutak <lukas.hutak@hotmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSG_BUILDER
#define MSG_BUILDER

#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <stdexcept>

#include <ipfixcol2.h>
#include <libfds.h>

/**
 * \brief Auxiliary class for building IPFIX Messages
 */
class msg_factory {
public:
    /**
     * \brief Prepare a new message factory.
     *
     * The factory is not prepared for insertion of records yet.
     * First, you have to use init() function to define the IPFIX Message
     * header fields.
     * \param alloc_hint Buffer size that should be able to hold newly
     *   generated message.
     */
    msg_factory() = default;
    ~msg_factory() = default;

    /**
     * \brief Initialize new IPFIX Message
     * \param exp_time Export Time of the IPFIX Message
     * \param seq_num  Sequence Number of the Message
     * \param odid     Observation Domain ID
     */
    void
    init(uint32_t exp_time, uint32_t seq_num, uint32_t odid);

    /**
     * \brief Add a new Data Record.
     *
     * \param data        Pointer to the Data Record
     * \param data_size   Size of <em>data</em> Record
     * \param snapshot    Snapshot with all templates valid at processing time
     * \param template_id Template ID of the Data Record
     */
    void
    add_drec(
        const void *data,
        uint16_t data_size,
        const fds_tsnapshot_t *snapshot,
        uint16_t template_id);

    /**
     * \brief Build an IPFIX Message from previously inserted records.
     *
     * \param plugin_ctx Context of the plugin that tries to generate IPFIX Message
     * \param msg_ctx    Information about flow source
     * \note If you want to create another IPFIX Message, you have to call
     *   init() function again.
     * \return Pointer to parser IPFIX Message suitable for other plugins.
     */
    struct ipx_msg_ipfix *
    build(const ipx_ctx_t *plugin_ctx, const ipx_msg_ctx *msg_ctx);

    uint32_t get_size() const {
        return m_msg_size;
    }

private:
    struct set_info {
        size_t offset;
    };

    struct drec_info {
        size_t offset;
        uint16_t size;
        const fds_tsnapshot_t *snapshot;
        const fds_template *tmplt;
    };

    // IPFIX Message buffer
    std::unique_ptr<uint8_t, decltype(&std::free)> m_msg = {nullptr, &std::free};
    size_t m_msg_alloc;
    size_t m_msg_size;  // also position where to append

    // Offset of the last IPFIX Set (zero == no set opened)
    size_t m_set_offset;

    // Information about inserted IPFIX Sets and IPFIX Data Records.
    std::vector<struct set_info> m_sets;
    std::vector<struct drec_info> m_drecs;

    void
    append_bytes(const void *data, uint16_t size);
    void
    set_open(uint16_t id);
    void
    set_close();
    uint16_t
    set_current_id();
};

#endif
