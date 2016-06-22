#pragma once

#include "h-basic.h"
#include "skill_flag_set.hpp"
#include "skills_defs.hpp"

/**
 * Skill descriptors and runtime data.
 */
struct skill_type
{
	const char *name = nullptr;             /* Name */
	char *desc = nullptr;                   /* Description */

	const char *action_desc = nullptr;      /* Action Description */

	s16b action_mkey = 0;                   /* Action do to */

	s32b i_value = 0;                       /* Current value */
	s32b i_mod = 0;                         /* Modifier, i.e. how much value 1 skill point gives */

	s32b value = 0;                         /* Current value */
	s32b mod = 0;                           /* Modifier, i.e. how much value 1 skill point gives */

	s16b action[MAX_SKILLS] = { 0 };        /* List of actions against other skills */

	s16b father = 0;                        /* Father in the skill tree */
	bool_ dev = FALSE;                      /* Is the branch developped ? */
	s16b order = 0;                         /* Order in the tree */
	bool_ hidden = FALSE;                   /* Inactive */

	byte random_gain_chance = 0;            /* Chance to gain from Lost Sword quest; if applicable */

	skill_flag_set flags;                   /* Skill flags */
};
