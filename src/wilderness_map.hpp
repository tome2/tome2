#pragma once

#include "h-basic.hpp"
#include "seed.hpp"

/**
 * A structure describing a wilderness map
 */
struct wilderness_map
{
	int feat = 0;                            /* Wilderness feature */
	seed_t seed = seed_t::system();          /* Seed for the RNG when building tile */
	u16b entrance = 0;                       /* Entrance for dungeons */
	bool_ known = FALSE;                     /* Is it seen by the player ? */
};
