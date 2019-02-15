#pragma once

#include "h-basic.hpp"

/**
 * Surface-level void gates descriptor.
 */
struct between_exit
{
	s16b corresp;           /* Corresponding between gate */

	s16b wild_x;            /* Wilderness spot to land onto */
	s16b wild_y;            /* Wilderness spot to land onto */
	s16b px, py;            /* Location of the map */

	s16b d_idx;             /* Dungeon to land onto */
	s16b level;
};
