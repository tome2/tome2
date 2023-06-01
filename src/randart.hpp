#pragma once

#include "h-basic.hpp"
#include "object_type_fwd.hpp"

void build_prob(const char *learn);
bool create_artifact(object_type *o_ptr, bool a_scroll, bool get_name);
bool artifact_scroll();
