#pragma once

#include "h-basic.h"
#include "body.hpp"
#include "object_flag_set.hpp"
#include "object_proto.hpp"
#include "player_defs.hpp"
#include "player_level_flag.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "player_shared.hpp"
#include "skill_modifiers.hpp"

#include <array>
#include <string>
#include <vector>


/**
 * Player racial descriptior.
 */
struct player_race
{
	std::string title;                                      /* Type of race */
	std::string desc;

	char luck = '\0';                                       /* Luck */

	player_shared ps;

	byte infra = 0;                                         /* Infra-vision range */

	u32b choice[2] { };                                     /* Legal class choices */

	byte body_parts[BODY_MAX] { };                          /* To help to decide what to use when body changing */

	player_race_flag_set flags;

	std::array<player_level_flag, PY_MAX_LEVEL+1> lflags;

	struct skill_modifiers skill_modifiers;

	std::vector<object_proto> object_protos;

	std::vector<player_race_ability_type> abilities;        /* Abilities to be gained by level; ignores prereqs */
};
