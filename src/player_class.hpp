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
	const char *title;              /* Type of class */
	char *desc;                     /* Small desc of the class */
	const char *titles[PY_MAX_LEVEL / 5];
					/* Titles */

	s16b c_adj[6];	                /* Class stat modifier */

	s16b c_dis;			/* class disarming */
	s16b c_dev;			/* class magic devices */
	s16b c_sav;			/* class saving throws */
	s16b c_stl;			/* class stealth */
	s16b c_srh;			/* class searching ability */
	s16b c_fos;			/* class searching frequency */
	s16b c_thn;			/* class to hit (normal) */
	s16b c_thb;			/* class to hit (bows) */

	s16b x_dis;			/* extra disarming */
	s16b x_dev;			/* extra magic devices */
	s16b x_sav;			/* extra saving throws */
	s16b x_stl;			/* extra stealth */
	s16b x_srh;			/* extra searching ability */
	s16b x_fos;			/* extra searching frequency */
	s16b x_thn;			/* extra to hit (normal) */
	s16b x_thb;			/* extra to hit (bows) */

	s16b c_mhp;			/* Class hit-dice adjustment */
	s16b c_exp;			/* Class experience factor */

	s16b powers[4];        /* Powers of the class */

	s16b spell_book;		/* Tval of spell books (if any) */
	s16b spell_stat;		/* Stat for spells (if any)  */
	s16b spell_lev;          /* The higher it is the higher the spells level are */
	s16b spell_fail;         /* The higher it is the higher the spells failure are */
	s16b spell_mana;         /* The higher it is the higher the spells mana are */
	s16b spell_first;        /* Level of first spell */
	s16b spell_weight;       /* Weight that hurts spells */
	byte max_spell_level;   /* Maximun spell level */
	byte magic_max_spell;  /* Maximun numbner of spells one can learn by natural means */

	u32b flags1;            /* flags */
	u32b flags2;            /* flags */

	s16b mana;
	s16b blow_num;
	s16b blow_wgt;
	s16b blow_mul;
	s16b extra_blows;

	s32b sense_base;
	s32b sense_pl;
	s32b sense_plus;
	byte sense_heavy;
	byte sense_heavy_magic;

	s16b obj_tval[5];
	s16b obj_sval[5];
	s16b obj_pval[5];
	s16b obj_dd[5];
	s16b obj_ds[5];
	s16b obj_num;

	char body_parts[BODY_MAX];      /* To help to decide what to use when body changing */

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

	u32b gods;

	player_spec spec[MAX_SPEC];

	struct
	{
		s16b    ability;
		s16b    level;
	} abilities[10];                /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};

