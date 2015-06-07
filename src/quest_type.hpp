#pragma once

#include "h-basic.h"

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

	bool_ (*init)(int q);    /* Function that takes care of generating hardcoded quests */

	s32b data[9];          /* Various datas used by the quests */

	bool_ (*gen_desc)(FILE *fff); /* Function for generating description. */
};
