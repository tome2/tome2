#pragma once

#include "h-basic.hpp"
#include "monster_race_flag_set.hpp"
#include "monster_spell_flag_set.hpp"

/* Define monster generation rules */
struct rule_type
{
	byte mode = 0;                  /* Mode of combination of the monster flags */
	byte percent = 0;               /* Percent of monsters affected by the rule */

	monster_race_flag_set mflags;   /* The monster flags that are allowed */
	monster_spell_flag_set mspells; /* Monster spells the are allowed */

	char r_char[5] = { 0 };         /* Monster race allowed */
};
