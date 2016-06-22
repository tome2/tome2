#pragma once

#include "h-basic.h"
#include "skills_defs.hpp"

/**
 * Skill descriptors and runtime data.
 */
struct skill_type
{
	const char *name;                       /* Name */
	char *desc;                             /* Description */

	const char *action_desc;                /* Action Description */

	s16b action_mkey;                       /* Action do to */

	s32b i_value;                           /* Actual value */
	s32b i_mod;                             /* Modifier(1 skill point = modifier skill) */

	s32b value;                             /* Actual value */
	s32b mod;                               /* Modifier(1 skill point = modifier skill) */

	s16b action[MAX_SKILLS];                /* List of actions against other skills */

	s16b father;                            /* Father in the skill tree */
	bool_ dev;                               /* Is the branch developped ? */
	s16b order;                             /* Order in the tree */
	bool_ hidden;                            /* Innactive */

	byte random_gain_chance;                /* random gain chance, still needs the flag */

	u32b flags1;                            /* Skill flags */
};
