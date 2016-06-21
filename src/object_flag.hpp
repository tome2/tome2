#pragma once

#include "object_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define TR(tier, index, name, e_name, c_name, c_page, c_col, c_row, c_type, c_prio, is_pval, is_esp) \
   DECLARE_FLAG(object_flag_set, name, tier, index)
#include "object_flag_list.hpp"
#undef TR
