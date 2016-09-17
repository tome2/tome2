#pragma once

#include "h-basic.h"
#include "object_proto.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "skill_modifiers.hpp"

#include <array>

/**
 * Player class descriptor.
 */
struct player_spec
{
	const char *title = nullptr;                            /* Type of class spec */
	char *desc = nullptr;                                   /* Small desc of the class spec */

	struct skill_modifiers skill_modifiers;

	std::array<object_proto, 5> obj;
	s16b obj_num = 0;

	u32b gods = 0;

	player_race_flag_set flags;

	std::array<player_race_ability_type, 10> abilities;     /* Abilities to be gained by level(doesnt take prereqs in account) */
};
