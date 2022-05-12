/**
 * \file src/plugins/intermediate/biflow/TemplatesHandler.cpp
 * \author Simona Bennárová
 * \brief Class for handling templates
 * \date 2022
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdexcept>
#include <ipfixcol2.h>
#include "TemplatesHandler.h"

TemplatesHandler::TemplatesHandler() = default;

TemplatesHandler::~TemplatesHandler() = default;

int
TemplatesHandler::get_next_template_id() {
    return template_id++;
}

bool
TemplatesHandler::uniflow_tmplt_exists(const fds_template *tmplt, uint16_t &tmplt_id) const {
    for (auto &tmplt_ref : uniflow_templates) {
        if (fds_template_cmp(tmplt, tmplt_ref) == 0) {
            tmplt_id = tmplt_ref->id;
            return true;
        }
    }
    return false;
}

bool
TemplatesHandler::biflow_tmplt_exists(uint16_t tmplt_id, uint16_t reversed_tmplt_id, BiflowTemplate &biflow_template) {
    for (auto &biflow_tmplt: biflow_templates) {
        if (biflow_tmplt.is_equal(tmplt_id, reversed_tmplt_id)) {
            biflow_template = biflow_tmplt;
            return true;
        }
    }
    return false;
}

const fds_template *
TemplatesHandler::create_tmplt(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr, bool make_copy) {
    fds_template *tmplt_copy;
    if (make_copy) {
        tmplt_copy = fds_template_copy(tmplt);
    } else {
        tmplt_copy = const_cast<fds_template *>(tmplt);
    }

    // Add template to manager
    tmplt_id = get_next_template_id();
    tmplt_copy->id = tmplt_id;

    int ret_code = fds_tmgr_template_add(*tmgr, tmplt_copy);
    if (ret_code == FDS_ERR_NOMEM) {
        fds_template_destroy(tmplt_copy);
        throw std::runtime_error("Failed to allocate memory for template!");
    }


    const fds_template *added_tmplt = nullptr;
    ret_code = fds_tmgr_template_get(*tmgr, tmplt_id, &added_tmplt);
    if (ret_code == IPX_ERR_NOMEM) {
        throw std::runtime_error("Failed to allocate memory!");
    }
    if (ret_code == IPX_ERR_ARG) {
        throw std::runtime_error("Wrong time context!");
    }

    return added_tmplt;
}

void
TemplatesHandler::set_uniflow_template_id(const fds_template *tmplt, uint16_t &tmplt_id, fds_tmgr **tmgr) {
    if (!uniflow_tmplt_exists(tmplt, tmplt_id)) {
        const fds_template *added_tmplt = create_tmplt(tmplt, tmplt_id, tmgr, true);
        uniflow_templates.push_back(added_tmplt);
    }
}

int
TemplatesHandler::create_biflow_template(Generator &tmplt_generator, BiflowTemplate &biflow_template,
                                         fds_tmgr_t **tmgr) {
    // Parse template
    struct fds_template *biflow_tmplt;
    uint16_t tmplt_id;
    uint16_t size = tmplt_generator.length();
    int ret_code = fds_template_parse(FDS_TYPE_TEMPLATE, tmplt_generator.get(), &size, &biflow_tmplt);
    if (ret_code == FDS_ERR_NOMEM) {
        throw std::runtime_error("Failed to allocate memory for template!");
    }
    if (ret_code == FDS_ERR_FORMAT) {
        return IPX_ERR_FORMAT;
    }

    //Create template
    biflow_template.biflow_tmplt = create_tmplt(biflow_tmplt, tmplt_id, tmgr, false);

    biflow_templates.push_back(biflow_template);
    return IPX_OK;
}

bool
TemplatesHandler::is_full() {
    // If template capacity is full, start from beginning
    if (template_id == TEMPLATE_ID_MAX) {
        template_id = TEMPLATE_ID_MIN;
        return true;
    }
    return false;
}
