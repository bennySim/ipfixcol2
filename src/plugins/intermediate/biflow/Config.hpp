/**
 * \file src/plugins/intermediate/biflow/Config.hpp
 * \author Simona Bennárová
 * \brief Configuration of biflow plugin (header file)
 * \date 2021
 */

#ifndef BIFLOW_CONFIG_H
#define BIFLOW_CONFIG_H

#include <ipfixcol2.h>
#include <map>
#include <string>
#include <libfds.h>
#include <memory>
#include <stdexcept>

#define TIMEOUT_CAHE_SEC_DEFAULT     5
#define TIMEOUT_MESSAGE_SEC_DEFAULT  10


/** Parsed configuration of an instance      */
/**
    * \brief Create a new configuration
    * \param[in] params XML configuration of biflow plugin
    * \throw runtime_error in case of invalid configuration
    */
struct configuration {
    uint32_t timeout_cache = TIMEOUT_CAHE_SEC_DEFAULT;
    uint32_t timeout_msg = TIMEOUT_MESSAGE_SEC_DEFAULT;
    bool ignore_missing_reverse = false;
    bool pair_missing_ports = false;
    uint32_t odid;
};

/** XML nodes */
enum params_xml_nodes {
    NODE_ODID,
    NODE_TIMEOUT_CACHE,
    NODE_TIMEOUT_MESSAGE,
    NODE_IGNORE_MISSING_REVERSE,
    NODE_PAIR_MISSING_PORTS
};

/** Definition of the \<params\> node  */
static const struct fds_xml_args args_params[] = {
        FDS_OPTS_ROOT("params"),
        FDS_OPTS_ELEM(NODE_ODID, "odid", FDS_OPTS_T_UINT, 0),
        FDS_OPTS_ELEM(NODE_TIMEOUT_CACHE, "timeoutCache", FDS_OPTS_T_UINT, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(NODE_TIMEOUT_MESSAGE, "timeoutMessage", FDS_OPTS_T_UINT, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(NODE_IGNORE_MISSING_REVERSE, "ignoreMissingReverse", FDS_OPTS_T_BOOL, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(NODE_PAIR_MISSING_PORTS, "pairMissingPorts", FDS_OPTS_T_BOOL, FDS_OPTS_P_OPT),
        FDS_OPTS_END
};

/**
 * \brief Parse all parameters
 *
 * This is the main parser function that process all format specifiers and parser all
 * specifications of outputs.
 * \param[in] params Initialized XML parser context of the root element
 * \throw invalid_argument or runtime_error
 */
inline struct configuration
parse_params(fds_xml_ctx_t *params) {

    struct configuration config;
    const struct fds_xml_cont *content;
    while (fds_xml_next(params, &content) != FDS_EOC) {
        switch (content->id) {
            case NODE_ODID:
                assert(content->type == FDS_OPTS_T_UINT && content->val_uint < UINT32_MAX);
                config.odid = content->val_uint;
                break;
            case NODE_TIMEOUT_CACHE:
                assert(content->type == FDS_OPTS_T_UINT);
                config.timeout_cache = content->val_uint;
                break;
            case NODE_TIMEOUT_MESSAGE:
                assert(content->type == FDS_OPTS_T_UINT);
                config.timeout_msg = content->val_uint;
                break;
            case NODE_IGNORE_MISSING_REVERSE:
                assert(content->type == FDS_OPTS_T_BOOL);
                config.ignore_missing_reverse = content->val_bool;
                break;
            case NODE_PAIR_MISSING_PORTS:
                assert(content->type == FDS_OPTS_T_BOOL);
                config.pair_missing_ports = content->val_bool;
                break;
            default:
                throw std::invalid_argument("Unexpected element within <params>!");
        }
    }
    return config;
}

inline struct configuration
parse_configuration(const char *params) {

    // Create XML parser
    std::unique_ptr<fds_xml_t, decltype(&fds_xml_destroy)> xml(fds_xml_create(), &fds_xml_destroy);
    if (!xml) {
        throw std::runtime_error("Failed to create an XML parser!");
    }

    if (fds_xml_set_args(xml.get(), args_params) != FDS_OK) {
        throw std::runtime_error("Failed to parse the description of an XML document!");
    }

    fds_xml_ctx_t *params_ctx = fds_xml_parse_mem(xml.get(), params, true);
    if (!params_ctx) {
        std::string err = fds_xml_last_err(xml.get());
        throw std::runtime_error("Failed to parse the configuration: " + err);
    }

    // Parse parameters and check configuration
    try {
        return parse_params(params_ctx);
        //check_validity();
    } catch (std::exception &ex) {
        throw std::runtime_error("Failed to parse the configuration: " + std::string(ex.what()));
    }
}

#endif //BIFLOW_CONFIG_H
