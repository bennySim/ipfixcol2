/**
 * \file src/plugins/intermediate/biflow/BiflowTemplate.h
 * \author Simona Bennárová
 * \brief Class for holding biflow template (header file)
 * \date 2022
 */

#ifndef IPFIXCOL2_BIFLOWTEMPLATE_H
#define IPFIXCOL2_BIFLOWTEMPLATE_H


#include <libfds.h>

class BiflowTemplate {
public:
    const fds_template * tmplt;
    const fds_template * reversed_tmplt;
    const fds_template * biflow_tmplt;

    BiflowTemplate();

    BiflowTemplate(const fds_template * tmplt, const fds_template * reversed_tmplt,const fds_template * biflow_tmplt);

    ~BiflowTemplate();

    bool is_equal(uint16_t tmplt_id, uint16_t reversed_tmplt_id) const;

    uint16_t get_tmplt_id() const;

};


#endif //IPFIXCOL2_BIFLOWTEMPLATE_H
