#pragma once

#include "store_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define STF(tier, index, name) \
   DECLARE_FLAG(store_flag_set, BOOST_PP_CAT(STF_,name), tier, index)
#include "store_flag_list.hpp"
#undef STF
