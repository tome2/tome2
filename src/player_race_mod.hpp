#pragma once

#include "body.hpp"
#include "h-basic.h"
#include "object_flag_set.hpp"
#include "object_proto.hpp"
#include "player_defs.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "skill_modifiers.hpp"
#include "skills_defs.hpp"

#include <array>
#include <vector>

struct player_race_mod
{
	char *title = nullptr;                                  /* Type of race mod */
	char *desc = nullptr;                                   /* Desc */

	bool_ place = FALSE;                                    /* TRUE = race race modifier, FALSE = Race modifier race */

	s16b r_adj[6] { };                                      /* (+) Racial stat bonuses */

	char luck = '\0';                                       /* Luck */
	s16b mana = 0;                                          /* Mana % */

	char r_mhp = 0;                                         /* (+) Race mod hit-dice modifier */
	s16b r_exp = 0;                                         /* (+) Race mod experience factor */

	char infra = '\0';                                      /* (+) Infra-vision range */

	u32b choice[2] { };                                     /* Legal race choices */

	u32b pclass[2] { };                                     /* Classes allowed */
	u32b mclass[2] { };                                     /* Classes restricted */

	s16b powers[4] { };                                     /* Powers of the subrace */

	char body_parts[BODY_MAX] { };                          /* To help to decide what to use when body changing */

	player_race_flag_set flags;

	std::array<object_flag_set, PY_MAX_LEVEL + 1> oflags;
	s16b opval[PY_MAX_LEVEL + 1] { };

	byte g_attr = 0;                                        /* Overlay graphic attribute */
	char g_char = '\0';                                     /* Overlay graphic character */

	struct skill_modifiers skill_modifiers;

	std::vector<object_proto> object_protos;

	std::array<player_race_ability_type, 10> abilities;     /* Abilities to be gained by level; doesnt take prereqs in account */
};

