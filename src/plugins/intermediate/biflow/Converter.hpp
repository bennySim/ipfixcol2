/**
 * \file src/plugins/intermediate/biflow/Converter.hpp
 * \author Simona Bennárová
 * \brief Converter functions for biflow plugin (header file)
 * \date 2021
 */

#ifndef BIFLOW_CONVERTER_HPP
#define BIFLOW_CONVERTER_HPP

#include <libfds.h>
#include "IPAddr.hpp"


int extract_ip(fds_drec *drec, uint32_t pen, uint16_t id, in_addr *dst);

int extract_ip_dst(fds_drec *drec, IPAddr *dst);

int extract_ip_src(fds_drec *drec, IPAddr *dst);

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
