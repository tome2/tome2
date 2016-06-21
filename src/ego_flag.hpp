#pragma once

#include "ego_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define ETR(tier, index, name) \
   DECLARE_FLAG(ego_flag_set, BOOST_PP_CAT(ETR_,name), tier, index)
#include "ego_flag_list.hpp"
#undef ETR
