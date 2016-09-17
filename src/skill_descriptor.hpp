#pragma once

#include "skill_descriptor_fwd.hpp"

#include "h-basic.h"
#include "skill_flag_set.hpp"
#include "skills_defs.hpp"

/**
 * Skill descriptor.
 */
struct skill_descriptor {

	const char *name = nullptr;             /* Name */
	char *desc = nullptr;                   /* Description */

	const char *action_desc = nullptr;      /* Action Description */

	s16b action_mkey = 0;                   /* Action do to */

	s16b action[MAX_SKILLS] = { 0 };        /* List of actions against other skills */

	s16b father = 0;                        /* Father in the skill tree */
	s16b order = 0;                         /* Order in the tree */

	byte random_gain_chance = 100;          /* Chance to gain from Lost Sword quest; if applicable */

	skill_flag_set flags;                   /* Skill flags */

};
