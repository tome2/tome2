#pragma once

#include "h-basic.hpp"
#include "skill_flag_set.hpp"
#include "skills_defs.hpp"

#include <vector>
#include <tuple>

/**
 * Skill descriptor.
 */
struct skill_descriptor {

	std::string name;                       /* Name */
	std::string desc;                       /* Description */

	std::string action_desc;                /* Action Description */
	s16b action_mkey = 0;                   /* Action do to */

	std::vector<std::size_t> excludes;      /* List of skills that this skill excludes;
						   any skill points assigned completely nullify
						   the listed skills */

	std::vector<std::tuple<std::size_t, int>> increases;
						/* List of skills the this skill increases,
						   along with the modifier that gets applied.
						   The first tuple element is the skill index. */

	s16b father = 0;                        /* Father in the skill tree */
	s16b order = 0;                         /* Order in the tree */

	byte random_gain_chance = 100;          /* Chance to gain from Lost Sword quest; if applicable */

	skill_flag_set flags;                   /* Skill flags */

};
