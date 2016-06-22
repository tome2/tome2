#pragma once

#include "player_race_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define PR(tier, index, name) \
   DECLARE_FLAG(player_race_flag_set, BOOST_PP_CAT(PR_,name), tier, index)
#include "player_race_flag_list.hpp"
#undef PR
