#pragma once

#include "h-basic.h"
#include "body.hpp"
#include "player_defs.hpp"
#include "skills_defs.hpp"

/**
 * Player racial descriptior.
 */
struct player_race
{
	const char *title;              /* Type of race */
	char *desc;

	s16b r_adj[6];                  /* Racial stat bonuses */

	char luck;                      /* Luck */

	s16b r_dis;			/* disarming */
	s16b r_dev;			/* magic devices */
	s16b r_sav;			/* saving throw */
	s16b r_stl;			/* stealth */
	s16b r_srh;			/* search ability */
	s16b r_fos;			/* search frequency */
	s16b r_thn;			/* combat (normal) */
	s16b r_thb;			/* combat (shooting) */

	byte r_mhp;			/* Race hit-dice modifier */
	u16b r_exp;                     /* Race experience factor */

	byte b_age;			/* base age */
	byte m_age;			/* mod age */

	byte m_b_ht;		/* base height (males) */
	byte m_m_ht;		/* mod height (males) */
	byte m_b_wt;		/* base weight (males) */
	byte m_m_wt;		/* mod weight (males) */

	byte f_b_ht;		/* base height (females) */
	byte f_m_ht;		/* mod height (females)	  */
	byte f_b_wt;		/* base weight (females) */
	byte f_m_wt;		/* mod weight (females) */

	byte infra;             /* Infra-vision range */

	u32b choice[2];            /* Legal class choices */

	s16b powers[4];         /* Powers of the race */

	byte body_parts[BODY_MAX];      /* To help to decide what to use when body changing */

	s16b chart;             /* Chart history */

	u32b flags1;
	u32b flags2;            /* flags */

	u32b oflags1[PY_MAX_LEVEL + 1];
	u32b oflags2[PY_MAX_LEVEL + 1];
	u32b oflags3[PY_MAX_LEVEL + 1];
	u32b oflags4[PY_MAX_LEVEL + 1];
	u32b oflags5[PY_MAX_LEVEL + 1];
	u32b oesp[PY_MAX_LEVEL + 1];
	s16b opval[PY_MAX_LEVEL + 1];

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
