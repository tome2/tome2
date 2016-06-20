#pragma once

#include "monster_race_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define RF(tier, index, name) \
   DECLARE_FLAG(monster_race_flag_set, BOOST_PP_CAT(RF_,name), tier, index)
#include "monster_race_flag_list.hpp"
#undef RF
