#pragma once

#include "h-basic.h"
#include "body.hpp"
#include "object_flag_set.hpp"
#include "object_proto.hpp"
#include "player_defs.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "skills_defs.hpp"
#include "skill_modifiers.hpp"

#include <array>
#include <vector>

/**
 * Player racial descriptior.
 */
struct player_race
{
	const char *title = nullptr;                            /* Type of race */
	char *desc = nullptr;

	s16b r_adj[6] { };                                      /* Racial stat bonuses */

	char luck = '\0';                                       /* Luck */

	byte r_mhp = 0;                                         /* Race hit-dice modifier */
	u16b r_exp = 0;                                         /* Race experience factor */

	byte infra = 0;                                         /* Infra-vision range */

	u32b choice[2] { };                                     /* Legal class choices */

	s16b powers[4] { };                                     /* Powers of the race */

	byte body_parts[BODY_MAX] { };                          /* To help to decide what to use when body changing */

	s16b chart = 0;                                         /* Chart history */

	player_race_flag_set flags;

	std::array<object_flag_set, PY_MAX_LEVEL + 1>  oflags;
	s16b opval[PY_MAX_LEVEL + 1] { };

	struct skill_modifiers skill_modifiers;

	std::vector<object_proto> object_protos;

	std::array<player_race_ability_type, 10> abilities;     /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};
