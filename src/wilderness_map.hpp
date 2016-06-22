#pragma once

#include "h-basic.h"

/**
 * A structure describing a wilderness map
 */
struct wilderness_map
{
	int feat = 0;                            /* Wilderness feature */
	u32b seed = 0;                           /* Seed for the RNG */
	u16b entrance = 0;                       /* Entrance for dungeons */
	bool_ known = FALSE;                     /* Is it seen by the player ? */
};
