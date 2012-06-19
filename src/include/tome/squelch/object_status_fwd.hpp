#ifndef H_3261a8ad_ee1c_4a2b_9d21_7c9955f09542
#define H_3261a8ad_ee1c_4a2b_9d21_7c9955f09542

#include "types_fwd.h"
#include "tome/enum_string_map.hpp"

namespace squelch {

enum class status_type;
EnumStringMap<status_type> &status_mapping();
status_type object_status(object_type *o_ptr);

} // namespace

#endif
