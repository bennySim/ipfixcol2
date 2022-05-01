/**
 * \file src/plugins/intermediate/biflow/BiflowTemplate.cpp
 * \author Simona Bennárová
 * \brief Class for holding biflow template (source file)
 * \date 2022
 */

#include "BiflowTemplate.h"

BiflowTemplate::BiflowTemplate() : tmplt(nullptr), reversed_tmplt(nullptr), biflow_tmplt(nullptr) {}

BiflowTemplate::BiflowTemplate(const fds_template *tmplt, const fds_template *reversed_tmplt, const fds_template *biflow_tmplt)
        : tmplt(tmplt), reversed_tmplt(reversed_tmplt), biflow_tmplt(biflow_tmplt) {}

uint16_t BiflowTemplate::get_tmplt_id() const {
    return biflow_tmplt->id;
}

BiflowTemplate::~BiflowTemplate() = default;

bool BiflowTemplate::is_equal(uint16_t tmplt_id, uint16_t reversed_tmplt_id) const {
    return tmplt_id == tmplt->id && reversed_tmplt_id == reversed_tmplt->id;
}


