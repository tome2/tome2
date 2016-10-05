#pragma once

#include <string>

#include "h-basic.h"

/**
 * Abilities.
 */
struct ability_type
{
	std::string name;                       /* Name */
	std::string desc;                       /* Description */

	std::string action_desc;                /* Action Description */

	s16b action_mkey;                       /* Action do to */

	s16b cost;                              /* Skill points cost */

	/* Prereqs */
	s16b skills[10];                	/* List of prereq skills(10 max) */
	s16b skill_levels[10];                  /* List of prereq skills(10 max) */
	s16b stat[6];                		/* List of prereq stats */
	s16b need_abilities[10];              	/* List of prereq abilities(10 max) */
	s16b forbid_abilities[10];		/* List of forbidden abilities(10 max) */
};
