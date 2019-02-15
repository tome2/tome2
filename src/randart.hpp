#pragma once

#include "h-basic.hpp"
#include "object_type_fwd.hpp"

void build_prob(const char *learn);
bool_ create_artifact(object_type *o_ptr, bool_ a_scroll, bool_ get_name);
bool_ artifact_scroll();
