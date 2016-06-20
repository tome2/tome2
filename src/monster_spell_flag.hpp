#pragma once

#include "monster_spell_flag_set.hpp"
#include <boost/preprocessor/cat.hpp>

//
// Define flag set for each flag.
//
#define SF(tier, index, name, is_summon, is_annoy, is_damage, is_bolt, is_smart, is_innate, is_escape, is_tactic, is_haste, is_heal) \
   DECLARE_FLAG(monster_spell_flag_set, BOOST_PP_CAT(SF_,name), tier, index)
#include "monster_spell_flag_list.hpp"
#undef SF

//
// Define index for each flag.
//
#define SF(tier, index, name, is_summon, is_annoy, is_damage, is_bolt, is_smart, is_innate, is_escape, is_tactic, is_haste, is_heal) \
   constexpr std::size_t BOOST_PP_CAT(BOOST_PP_CAT(SF_,name), _IDX) = (tier - 1) * 32 + index;
#include "monster_spell_flag_list.hpp"
#undef SF
