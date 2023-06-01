#pragma once

#include "h-basic.hpp"
#include "object_proto.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "skill_modifiers.hpp"

#include <array>
#include <vector>

/**
 * Player class descriptor.
 */
struct player_spec
{
	const char *title = nullptr;                            /* Type of class spec */
	char *desc = nullptr;                                   /* Small desc of the class spec */

	struct skill_modifiers skill_modifiers;

	std::vector<object_proto> object_protos;

	u32b gods = 0;

	player_race_flag_set flags;

	std::vector<player_race_ability_type> abilities;        /* Abilities to be gained by level; ignores prereqs */
};
