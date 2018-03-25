#pragma once

#include "h-basic.h"
#include "deity_type_fwd.hpp"

struct school_type
{
	const char *name;               /* Name */
	s16b skill;                     /* Skill used for that school */
	bool spell_power;              /* Does spell power affect spells in this school? */
	bool sorcery;                  /* Does Sorcery affect this school? */

	int deity_idx;     /* Deity; if <=0, no deity required */
	deity_type *deity; /* Direct pointer to deity */

	int (*bonus_levels)(); /* Calculate number of bonus levels */

	bool (*depends_satisfied)(); /* Are dependendies satisfied? */

	struct school_provider_list *providers; /* List of secondary providers of this school */
};
