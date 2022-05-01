/**
 * \file src/plugins/intermediate/biflow/TemplatesHandler.h
 * \author Simona Bennárová
 * \brief Class for handling templates
 * \date 2022
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
    std::vector<const fds_template *> uniflow_templates;
    std::vector<BiflowTemplate> biflow_templates;
    uint16_t template_id = TEMPLATE_ID_MIN;

    int get_next_template_id();

    bool uniflow_tmplt_exists(const fds_template *tmplt, uint16_t &tmplt_id) const;

    const fds_template *create_tmplt(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr, bool make_copy);

public:
    TemplatesHandler();

    ~TemplatesHandler();

    bool biflow_tmplt_exists(uint16_t tmplt_id, uint16_t reversed_tmplt_id, BiflowTemplate &biflow_template);

    void set_uniflow_template_id(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr);

    int create_biflow_template(Generator &tmplt_generator, BiflowTemplate &biflow_template, fds_tmgr_t **tmgr);
};

#endif //BIFLOW_TEMPLATESHANDLER_H
