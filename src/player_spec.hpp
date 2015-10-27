#pragma once

#include "h-basic.h"
#include "skills_defs.hpp"

/**
 * Player class descriptor.
 */
struct player_spec
{
	const char *title;              /* Type of class spec */
	char *desc;                     /* Small desc of the class spec */

	char skill_basem[MAX_SKILLS];   /* Mod for value */
	u32b skill_base[MAX_SKILLS];    /* value */
	char skill_modm[MAX_SKILLS];    /* mod for mod */
	s16b skill_mod[MAX_SKILLS];     /* mod */

	u32b skill_ideal[MAX_SKILLS];   /* Ideal skill levels at level 50 */

	s16b obj_tval[5];
	s16b obj_sval[5];
	s16b obj_pval[5];
	s16b obj_dd[5];
	s16b obj_ds[5];
	s16b obj_num;

	u32b gods;

	u32b flags1;
	u32b flags2;            /* flags */

	struct
	{
		s16b    ability;
		s16b    level;
	} abilities[10];                /* Abilitiers to be gained by level(doesnt take prereqs in account) */
};
