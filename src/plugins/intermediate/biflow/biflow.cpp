/**
 * \file src/plugins/intermediate/biflow/biflow.cpp
 * \author Simona Bennárová
 * \brief Biflow converter plugin
 * \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <memory>
#include <unordered_map>
#include "Config.hpp"
#include "Storage.hpp"

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
        // Plugin identification name
        "biflow",
        // Brief description of plugin
        "Convert uniflows into biflows",
        // Plugin type
        IPX_PT_INTERMEDIATE,
        // Configuration flags (reserved for future use)
        0,
        // Plugin version string (like "1.2.3")
        "1.0.0",
        // Minimal IPFIXcol version string (like "1.2.3")
        "2.1.0"
};

int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params) {
    Storage *storage;
    try {
        // Create and parse the configuration
        struct configuration config = parse_configuration(params);
        std::unique_ptr<Storage> storage_data(new Storage(ctx, config));

        // Success
        storage = storage_data.release();
    } catch (std::exception &ex) {
        IPX_CTX_ERROR(ctx, "%s", ex.what());
        return IPX_ERR_DENIED;
    } catch (...) {
        IPX_CTX_ERROR(ctx, "Unexpected exception has occurred!", '\0');
        return IPX_ERR_DENIED;
    }

    ipx_ctx_private_set(ctx, storage);
    return IPX_OK;
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg) {
    (void) ctx; // supress warning
    int ret_code = IPX_OK;

    try {
        Storage *storage = reinterpret_cast<Storage *>(cfg);
        storage->process_message(msg);
    } catch (std::exception &ex) {
        IPX_CTX_ERROR(ctx, "%s", ex.what());
        ret_code = IPX_ERR_DENIED;
    } catch (...) {
        IPX_CTX_ERROR(ctx, "Unexpected exception has occurred!", '\0');
        ret_code = IPX_ERR_DENIED;
    }

    return ret_code;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg) {
    (void) ctx; // Suppress warnings

    delete reinterpret_cast<Storage *>(cfg);
}