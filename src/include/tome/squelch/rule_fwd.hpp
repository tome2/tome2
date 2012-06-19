#ifndef H_4a8d2cfb_182c_4138_983d_606a9ac70784
#define H_4a8d2cfb_182c_4138_983d_606a9ac70784

#include "tome/enum_string_map.hpp"

namespace squelch {

enum class action_type;

EnumStringMap<action_type> &action_mapping();

class Rule;

} // namespace

#endif
