#pragma once

#include <string>
#include <vector>

#include "h-basic.hpp"

/**
 * Abilities.
 */
struct ability_type
{
public:
	struct skill_requirement {
		s16b skill_idx = 0;
		s16b level = 0;
	};

public:
	std::string name;                             /* Name */
	std::string desc;                             /* Description */

	std::string action_desc;                      /* Action Description */

	s16b action_mkey = 0;                         /* Action do to */

	s16b cost = 0;                                /* Skill points cost */

	std::vector<skill_requirement> need_skills;   /* List of prereq skills */

	s16b stat[6] { };                             /* List of prereq stats */

	std::vector<s16b> need_abilities;             /* List of prereq abilities */

	/**
	 * Default constructor
	 */
	ability_type()
	{
		for (auto &stat_ref: stat)
		{
			// Requirement is always met unless otherwise specified.
			stat_ref = -1;
		}
	}

};
