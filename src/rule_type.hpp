#pragma once

#include "h-basic.h"
#include "monster_spell_flag_set.hpp"

/* Define monster generation rules */
struct rule_type
{
	byte mode = 0;                  /* Mode of combination of the monster flags */
	byte percent = 0;               /* Percent of monsters affected by the rule */

	u32b mflags1 = 0;               /* The monster flags that are allowed */
	u32b mflags2 = 0;
	u32b mflags3 = 0;
	u32b mflags7 = 0;
	u32b mflags8 = 0;
	u32b mflags9 = 0;

	monster_spell_flag_set mspells; /* Monster spells the are allowed */

	char r_char[5] = { 0 };         /* Monster race allowed */
};
