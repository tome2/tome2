#pragma once

#include "h-basic.h"

/**
 * Surface-level void gates descriptor.
 */
struct between_exit
{
	s16b corresp;           /* Corresponding between gate */
	bool_ dungeon;           /* Do we exit in a dungeon or in the wild ? */

	s16b wild_x, wild_y;    /* Wilderness spot to land onto */
	s16b px, py;            /* Location of the map */

	s16b d_idx;             /* Dungeon to land onto */
	s16b level;
};
