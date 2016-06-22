#pragma once

#include "h-basic.h"
#include "body.hpp"
#include "object_flag_set.hpp"
#include "player_defs.hpp"
#include "player_race_ability_type.hpp"
#include "player_race_flag_set.hpp"
#include "skills_defs.hpp"

#include <array>

/**
 * Player racial descriptior.
 */
struct player_race
{
	const char *title = nullptr;                            /* Type of race */
	char *desc = nullptr;

	s16b r_adj[6] { };                                      /* Racial stat bonuses */

	char luck = '\0';                                       /* Luck */

	s16b r_dis = 0;                                         /* disarming */
	s16b r_dev = 0;                                         /* magic devices */
	s16b r_sav = 0;                                         /* saving throw */
	s16b r_stl = 0;                                         /* stealth */
	s16b r_srh = 0;                                         /* search ability */
	s16b r_fos = 0;                                         /* search frequency */
	s16b r_thn = 0;                                         /* combat (normal) */
	s16b r_thb = 0;                                         /* combat (shooting) */

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

	char skill_basem[MAX_SKILLS] { };
	u32b skill_base[MAX_SKILLS] { };
	char skill_modm[MAX_SKILLS] { };
	s16b skill_mod[MAX_SKILLS] { };

	s16b obj_tval[5] { };
	s16b obj_sval[5] { };
	s16b obj_pval[5] { };
	s16b obj_dd[5] { };
	s16b obj_ds[5] { };
	s16b obj_num = 0;

	std::array<player_race_ability_type, 10> abilities;     /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};
