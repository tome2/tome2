#pragma once

#include "body.hpp"
#include "h-basic.h"
#include "player_defs.hpp"
#include "skills_defs.hpp"

struct player_race_mod
{
	char *title;                    /* Type of race mod */
	char *desc;                     /* Desc */

	bool_ place;                    /* TRUE = race race modifier, FALSE = Race modifier race */

	s16b r_adj[6];                  /* (+) Racial stat bonuses */

	char luck;                      /* Luck */
	s16b mana;                      /* Mana % */

	s16b r_dev;                     /* (+) magic devices */
	s16b r_sav;                     /* (+) saving throw */
	s16b r_stl;                     /* (+) stealth */
	s16b r_thn;                     /* (+) combat (normal) */
	s16b r_thb;                     /* (+) combat (shooting) */

	char r_mhp;                     /* (+) Race mod hit-dice modifier */
	s16b r_exp;                     /* (+) Race mod experience factor */

	char infra;             /* (+) Infra-vision range */

	u32b choice[2];            /* Legal race choices */

	u32b pclass[2];            /* Classes allowed */
	u32b mclass[2];            /* Classes restricted */

	s16b powers[4];        /* Powers of the subrace */

	char body_parts[BODY_MAX];      /* To help to decide what to use when body changing */

	u32b flags1;
	u32b flags2;            /* flags */

	u32b oflags1[PY_MAX_LEVEL + 1];
	u32b oflags2[PY_MAX_LEVEL + 1];
	u32b oflags3[PY_MAX_LEVEL + 1];
	u32b oflags4[PY_MAX_LEVEL + 1];
	u32b oflags5[PY_MAX_LEVEL + 1];
	u32b oesp[PY_MAX_LEVEL + 1];
	s16b opval[PY_MAX_LEVEL + 1];

	byte g_attr;                    /* Overlay graphic attribute */
	char g_char;                    /* Overlay graphic character */

	char skill_basem[MAX_SKILLS];
	u32b skill_base[MAX_SKILLS];
	char skill_modm[MAX_SKILLS];
	s16b skill_mod[MAX_SKILLS];

	s16b obj_tval[5];
	s16b obj_sval[5];
	s16b obj_pval[5];
	s16b obj_dd[5];
	s16b obj_ds[5];
	s16b obj_num;

	struct
	{
		s16b    ability;
		s16b    level;
	} abilities[10];                /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};

