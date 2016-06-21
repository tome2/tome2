#pragma once

#include "feature_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define FF(tier, index, name) \
   DECLARE_FLAG(feature_flag_set, BOOST_PP_CAT(FF_,name), tier, index)
#include "feature_flag_list.hpp"
#undef FF
