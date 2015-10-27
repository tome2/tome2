#pragma once

#include "h-basic.h"

/**
 * A structure describing a wilderness map
 */
struct wilderness_map
{
	int     feat;                   /* Wilderness feature */
	u32b    seed;                   /* Seed for the RNG */
	u16b    entrance;               /* Entrance for dungeons */

	bool_    known;                  /* Is it seen by the player ? */
};
