/**
 * \file src/plugins/intermediate/biflow/BiflowTemplate.h
 * \author Simona Bennárová
 * \brief Class for holding biflow template (header file)
 * \date 2022
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPFIXCOL2_BIFLOWTEMPLATE_H
#define IPFIXCOL2_BIFLOWTEMPLATE_H


#include <libfds.h>

class BiflowTemplate {
public:
    /** Forward template      */
    const fds_template * tmplt;
    /** Reversed tempalte     */
    const fds_template * reversed_tmplt;
    /** Biflow template composed from forward and reversed template  */
    const fds_template * biflow_tmplt;

    /**
     * \brief Class constructor
     */
    BiflowTemplate();

    /**
     * \brief Class constructor
     *
     * \param[in] tmplt             forward template
     * \param[in] reversed_tmplt    reversed template
     * \param[in] biflow_tmplt      biflow template
     */
    BiflowTemplate(const fds_template * tmplt, const fds_template * reversed_tmplt,const fds_template * biflow_tmplt);

    /**
     * \brief Class destructor
     */
    ~BiflowTemplate();

    /**
     * \brief Check whether another biflow template is equal to this one
     *
     * \param[in] tmplt_id              another forward template ID
     * \param[in] reversed_tmplt_id     another reversed template ID
     * \return true if IDs match, false otherwise
     */
    bool is_equal(uint16_t tmplt_id, uint16_t reversed_tmplt_id) const;

    /**
     * \brief Getter for biflow template id
     *
     * \return biflow template id
     */
    uint16_t get_tmplt_id() const;
};


#endif //IPFIXCOL2_BIFLOWTEMPLATE_H
