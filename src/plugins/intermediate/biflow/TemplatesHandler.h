/**
 * \file src/plugins/intermediate/biflow/TemplatesHandler.h
 * \author Simona Bennárová
 * \brief Class for handling templates
 * \date 2022
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BIFLOW_TEMPLATESHANDLER_H
#define BIFLOW_TEMPLATESHANDLER_H


#include <libfds.h>
#include <vector>
#include <memory>
#include "BiflowTemplate.h"
#include "DataStructures.h"
#include "Generator.h"

#define TEMPLATE_ID_MAX     UINT16_MAX
#define TEMPLATE_ID_MIN     256

class TemplatesHandler {
private:
    /** List of uniflow templates                */
    std::vector<const fds_template *> uniflow_templates;
    /** List of biflow templates                 */
    std::vector<BiflowTemplate> biflow_templates;
    /** Next unused template ID                  */
    uint16_t template_id = TEMPLATE_ID_MIN;

    // Return current template ID and increment
    int get_next_template_id();
    // Check whether template already exist in the list of uniflow templates
    bool uniflow_tmplt_exists(const fds_template *tmplt, uint16_t &tmplt_id) const;
    // Add template to list and template manager
    const fds_template *create_tmplt(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr, bool make_copy);

public:
    /**
     * \brief Constructor
     */
    TemplatesHandler();

    /**
     * \brief Destructor
     */
    ~TemplatesHandler();

    /**
     * \brief Check whether template already exists in list of biflow templates
     *
     * \param[in] tmplt_id              forward record template ID
     * \param[in] reversed_tmplt_id     reverse record template ID
     * \param[out] biflow_template      not initialized biflow template structure
     * \return true if template already exists in the list, \param biflow_template is set
     *         otherwise false
     */
    bool biflow_tmplt_exists(uint16_t tmplt_id, uint16_t reversed_tmplt_id, BiflowTemplate &biflow_template);

    /**
     * \brief Check whether uniflow template exists in manager, if not, create one
     *
     * \param tmplt[in]         template to be checked
     * \param tmplt_id[out]     ID of template from manager - set by this function
     * \param tmgr[in]          template manager
     */
    void set_uniflow_template_id(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr);

    /**
     * \brief Build newly constructed template and add to the manager
     *
     * \param[in] tmplt_generator   template builder structure
     * \param[in] biflow_template   biflow template class with biflow template missing
     * \param[in] tmgr              template manager
     * \return IPX_ERR_FORMAT if some error while parsing occurs, IPX_OK otherwise
     */
    int create_biflow_template(Generator &tmplt_generator, BiflowTemplate &biflow_template, fds_tmgr_t **tmgr);

    /**
     * \brief Check whether template manager is full
     *
     * \return true if template manager is full, false otherwise
     */
    bool is_full();
};

#endif //BIFLOW_TEMPLATESHANDLER_H
