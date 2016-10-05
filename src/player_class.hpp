#pragma once

#include "body.hpp"
#include "h-basic.h"
#include "object_flag_set.hpp"
#include "object_proto.hpp"
#include "player_defs.hpp"
#include "player_level_flag.hpp"
#include "player_race_flag_set.hpp"
#include "player_shared.hpp"
#include "player_spec.hpp"
#include "skill_modifiers.hpp"

/**
 * Player descriptor and runtime data.
 */
struct player_class
{
	const char *title = nullptr;                         /* Type of class */
	char *desc = nullptr;                                /* Small desc of the class */
	const char *titles[PY_MAX_LEVEL / 5] { };            /* Titles */

	int display_order_idx;                               /* Display order index; lowest first */

	player_shared ps;

	player_race_flag_set flags;

	s16b mana = 0;
	s16b blow_num = 0;
	s16b blow_wgt = 0;
	s16b blow_mul = 0;
	s16b extra_blows = 0;

	std::vector<object_proto> object_protos;

	char body_parts[BODY_MAX] { };                          /* To help to decide what to use when body changing */

	std::array<player_level_flag, PY_MAX_LEVEL+1> lflags;

	struct skill_modifiers skill_modifiers;

	u32b gods = 0;

	std::vector<player_spec> spec;

	std::vector<player_race_ability_type> abilities;        /* Abilities to be gained by level; ignores prereqs */
};

