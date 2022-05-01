/**
 * \file src/plugins/intermediate/biflow/IanaDefinitions.h
 * \author Simona Bennárová
 * \brief IANA Information Element IDs
 *        Taken from https://www.iana.org/assignments/ipfix/ipfix.xhtml
 * \date 2021
 */
#ifndef BIFLOW_IANADEFINITIONS_H
#define BIFLOW_IANADEFINITIONS_H

/** Type aliases */
#include <unordered_set>

using element_id = uint16_t;
template<typename T>
using set_t = std::unordered_set<T>;

/** Private Enterprise Number for IANA elements */
#define IANA_PEN             0
#define IANA_PEN_REVERSED    29305

/** Aliases for IPFIX Information Elements IDs */
#define IANA_PROTOCOL_ID          4
#define IANA_SRC_PORT             7
#define IANA_SRC_IPV4             8
#define IANA_DST_PORT             11
#define IANA_DST_IPV4             12
#define IANA_SRC_IPV6             27
#define IANA_DST_IPV6             28

// Identifiers
#define IANA_FLOW_ID                 148
#define IANA_TEMPLATE_ID             145
#define IANA_OBSERVATION_DOMAIN_ID   149
#define IANA_COMMON_PROPERTIES_ID    137

// Metering and Exporting Process Configuration
#define IANA_EXPORTER_IPV4_ADDRESS     130
#define IANA_EXPORTER_IPV6_ADDRESS     131
#define IANA_EXPORTER_TRANSPORT_PORT   217
#define IANA_COLLECTOR_IPV4_ADDRESS    211
#define IANA_COLLECTOR_IPV6_ADDRESS    212
#define IANA_EXPORT_INTERFACE          213
#define IANA_EXPORT_PROTOCOL_VERSION   214
#define IANA_EXPORT_TRANSPORT_PROTOCOL 215
#define IANA_COLLECTOR_TRANSPORT_PORT  216
#define IANA_FLOW_KEY_INDICATOR        173

// Metering and Exporting Process Statistics
#define IANA_EXPORTED_MESSAGE_TOTAL_COUNT       41
#define IANA_EXPORTED_OCTET_TOTAL_COUNT         40
#define IANA_EXPORTED_FLOW_RECORD_TOTAL_COUNT   42
#define IANA_OBSERVED_FLOW_TOTAL_COUNT          163
#define IANA_IGNORED_PACKET_TOTAL_COUNT         164
#define IANA_IGNORED_OCTET_TOTAL_COUNT          165
#define IANA_NOT_SENT_FLOW_TOTAL_COUNT          166
#define IANA_NOT_SENT_PACKET_TOTAL_COUNT        167
#define IANA_NOT_SENT_OCTET_TOTAL_COUNT         168

#define IANA_PADDING_OCTETS            210
#define IANA_BIFLOW_DIRECTION          239

/** Sizes in bytes for IPFIX Information Elements IDs */
#define IANA_PROTOCOL_SIZE                   1
#define IANA_PORT_SIZE                       2
#define IANA_IPV4_SIZE                       4
#define IANA_IPV6_SIZE                       16

inline bool
is_ignored_type(fds_iemgr_element_type data_type) {
    // Lists are not supported for now
    switch (data_type) {
        case FDS_ET_BASIC_LIST:
        case FDS_ET_SUB_TEMPLATE_LIST:
        case FDS_ET_SUB_TEMPLATE_MULTILIST:
        case FDS_ET_UNASSIGNED:
            return true;
        default:
            return false;
    }
}

inline bool
is_key_field(uint32_t en, element_id id) {
    if (en != IANA_PEN) {
        return false;
    }

    switch (id) {
        case IANA_PROTOCOL_ID:
        case IANA_DST_IPV4:
        case IANA_SRC_IPV4:
        case IANA_SRC_IPV6:
        case IANA_DST_IPV6:
        case IANA_DST_PORT:
        case IANA_SRC_PORT:
            return true;
        default:
            return false;
    }
}

inline bool
is_non_reversible_field(uint32_t enterprise_id, element_id id) {
    if (enterprise_id != IANA_PEN) {
        return false;
    }
    switch (id) {
        // identifiers
        case IANA_FLOW_ID:
        case IANA_TEMPLATE_ID:
        case IANA_OBSERVATION_DOMAIN_ID:
        case IANA_COMMON_PROPERTIES_ID:
            // process configuration
        case IANA_EXPORTER_IPV4_ADDRESS:
        case IANA_EXPORTER_IPV6_ADDRESS:
        case IANA_EXPORTER_TRANSPORT_PORT:
        case IANA_EXPORT_INTERFACE:
        case IANA_EXPORT_PROTOCOL_VERSION:
        case IANA_EXPORT_TRANSPORT_PROTOCOL:
        case IANA_COLLECTOR_IPV4_ADDRESS:
        case IANA_COLLECTOR_IPV6_ADDRESS:
        case IANA_COLLECTOR_TRANSPORT_PORT:
        case IANA_FLOW_KEY_INDICATOR:
            // process statistics
        case IANA_EXPORTED_MESSAGE_TOTAL_COUNT:
        case IANA_EXPORTED_OCTET_TOTAL_COUNT:
        case IANA_EXPORTED_FLOW_RECORD_TOTAL_COUNT:
        case IANA_OBSERVED_FLOW_TOTAL_COUNT:
        case IANA_IGNORED_PACKET_TOTAL_COUNT:
        case IANA_IGNORED_OCTET_TOTAL_COUNT:
        case IANA_NOT_SENT_FLOW_TOTAL_COUNT:
        case IANA_NOT_SENT_PACKET_TOTAL_COUNT:
        case IANA_NOT_SENT_OCTET_TOTAL_COUNT:
            // other
        case IANA_BIFLOW_DIRECTION:
        case IANA_PADDING_OCTETS:
            return true;
        default:
            return false;
    }
}

inline bool
has_missing_reverse_definition(fds_drec_iter &iter) {
    const fds_tfield *field_info = iter.field.info;
    return field_info->en != IANA_PEN
           && (field_info->def == nullptr || field_info->def->reverse_elem == nullptr);
}

inline bool is_biflow_record(fds_drec& drec) {
    return drec.tmplt->flags & FDS_TEMPLATE_BIFLOW;
}

inline bool is_option_tmplt(fds_drec& drec) {
    return drec.tmplt->type == FDS_TYPE_TEMPLATE_OPTS;
}

#endif //BIFLOW_IANADEFINITIONS_H
