#pragma once

#include "h-basic.h"

#include <string>

/**
 * Quest descriptor and runtime data.
 */
struct quest_type
{
	bool_ silent;

	char name[40];          /* Quest name */

	char desc[10][80];      /* Quest desc */

	s16b status;            /* Is the quest taken, completed, finished? */

	s16b level;             /* Dungeon level */

	s16b *plot;             /* Which plot does it belongs to? */

	bool_ (*init)();        /* Function that takes care of generating hardcoded quests */

	s32b data[9];           /* Various datas used by the quests */

	std::string (*gen_desc)(); /* Function for generating description. */
};
