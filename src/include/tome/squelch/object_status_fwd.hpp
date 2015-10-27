#pragma once

#include "../object_type_fwd.hpp"
#include "tome/enum_string_map.hpp"

namespace squelch {

enum class status_type;
EnumStringMap<status_type> &status_mapping();
status_type object_status(object_type *o_ptr);

} // namespace
