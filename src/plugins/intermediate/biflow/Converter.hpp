/**
 * \file src/plugins/intermediate/biflow/Converter.hpp
 * \author Simona Bennárová
 * \brief Converter functions for biflow plugin (header file)
 * \date 2021
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BIFLOW_CONVERTER_HPP
#define BIFLOW_CONVERTER_HPP

#include <libfds.h>
#include "IPAddr.hpp"

/**
 * \brief Function from extracting destination IP address from record
 *
 * \param[in] drec      original record
 * \param[out] dst      extracted IP address
 * \return IPX_ERR_FORMAT if format of IP address is invalid(length)
 *         IPX_ERR_NOTFOUND if there is no (ipv4 or ipv6) destination address
 *         IPX_OK otherwise
 */
int extract_ip_dst(fds_drec *drec, IPAddr *dst);

/**
 * \brief Function from extracting source IP address from record
 *
 * \param[in] drec      original record
 * \param[out] dst      extracted IP address
 * \return IPX_ERR_FORMAT if format of IP address is invalid(length)
 *         IPX_ERR_NOTFOUND if there is no (ipv4 or ipv6) source address
 *         IPX_OK otherwise
 */
int extract_ip_src(fds_drec *drec, IPAddr *dst);

/**
 * \brief
 *
 * \tparam T        type of element to be extracted
 *
 * \param drec[in]      original record
 * \param pen[in]       enterprise ID of to be extracted element
 * \param id[in]        element ID
 * \param dst[out]      structure for storing extracted value
 * \return IPX_ERR_FORMAT if format of element is invalid
 *         IPX_ERR_NOTFOUND if there is no (ipv4 or ipv6) source address
 *         IPX_OK otherwise
 */
template<typename T>
int
extract_uint(fds_drec *drec, uint32_t pen, uint16_t id, T *dst) {
    struct fds_drec_field field{};
    uint64_t value;
    int ret;

    ret = fds_drec_find(drec, pen, id, &field);
    if (ret == FDS_EOC) {
        return FDS_ERR_NOTFOUND;
    }

    ret = fds_get_uint_be(field.data, field.size, &value);
    if (ret != FDS_OK) {
        return FDS_ERR_FORMAT;
    }
    *dst = value;

    return FDS_OK;
}

#endif //BIFLOW_CONVERTER_HPP
