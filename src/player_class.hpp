#pragma once

#include "body.hpp"
#include "h-basic.h"
#include "player_defs.hpp"
#include "player_spec.hpp"

/**
 * Maximum number of specialties.
 */
constexpr int MAX_SPEC = 20;

/**
 * Player descriptor and runtime data.
 */
struct player_class
{
	const char *title = nullptr;                         /* Type of class */
	char *desc = nullptr;                                /* Small desc of the class */
	const char *titles[PY_MAX_LEVEL / 5] { };            /* Titles */

	s16b c_adj[6] { };	                             /* Class stat modifier */

	s16b c_dis = 0;                                      /* class disarming */
	s16b c_dev = 0;                                      /* class magic devices */
	s16b c_sav = 0;                                      /* class saving throws */
	s16b c_stl = 0;                                      /* class stealth */
	s16b c_srh = 0;                                      /* class searching ability */
	s16b c_fos = 0;                                      /* class searching frequency */
	s16b c_thn = 0;                                      /* class to hit (normal) */
	s16b c_thb = 0;                                      /* class to hit (bows) */

	s16b x_dis = 0;                                      /* extra disarming */
	s16b x_dev = 0;                                      /* extra magic devices */
	s16b x_sav = 0;                                      /* extra saving throws */
	s16b x_stl = 0;                                      /* extra stealth */
	s16b x_srh = 0;                                      /* extra searching ability */
	s16b x_fos = 0;                                      /* extra searching frequency */
	s16b x_thn = 0;                                      /* extra to hit (normal) */
	s16b x_thb = 0;                                      /* extra to hit (bows) */

	s16b c_mhp = 0;                                      /* Class hit-dice adjustment */
	s16b c_exp = 0;                                      /* Class experience factor */

	s16b powers[4] { };                                  /* Powers of the class */

	s16b spell_book = 0;                                 /* Tval of spell books (if any) */
	s16b spell_stat = 0;                                 /* Stat for spells (if any)  */
	s16b spell_lev = 0;                                  /* The higher it is the higher the spells level are */
	s16b spell_fail = 0;                                 /* The higher it is the higher the spells failure are */
	s16b spell_mana = 0;                                 /* The higher it is the higher the spells mana are */
	s16b spell_first = 0;                                /* Level of first spell */
	s16b spell_weight = 0;                               /* Weight that hurts spells */
	byte max_spell_level = 0;                            /* Maximun spell level */
	byte magic_max_spell = 0;                            /* Maximun numbner of spells one can learn by natural means */

	u32b flags1 = 0;
	u32b flags2 = 0;

	s16b mana = 0;
	s16b blow_num = 0;
	s16b blow_wgt = 0;
	s16b blow_mul = 0;
	s16b extra_blows = 0;

	s32b sense_base = 0;
	s32b sense_pl = 0;
	s32b sense_plus = 0;
	byte sense_heavy = 0;
	byte sense_heavy_magic = 0;

	s16b obj_tval[5] { };
	s16b obj_sval[5] { };
	s16b obj_pval[5] { };
	s16b obj_dd[5] { };
	s16b obj_ds[5] { };
	s16b obj_num = 0;

	char body_parts[BODY_MAX] { };                          /* To help to decide what to use when body changing */

	u32b oflags1[PY_MAX_LEVEL + 1] { };
	u32b oflags2[PY_MAX_LEVEL + 1] { };
	u32b oflags3[PY_MAX_LEVEL + 1] { };
	u32b oflags4[PY_MAX_LEVEL + 1] { };
	u32b oflags5[PY_MAX_LEVEL + 1] { };
	u32b oesp[PY_MAX_LEVEL + 1] { };
	s16b opval[PY_MAX_LEVEL + 1] { };

	char skill_basem[MAX_SKILLS] { };
	u32b skill_base[MAX_SKILLS] { };
	char skill_modm[MAX_SKILLS] { };
	s16b skill_mod[MAX_SKILLS] { };

	u32b gods = 0;

	std::array<player_spec, MAX_SPEC> spec;

	std::array<player_race_ability_type, 10> abilities;     /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};

