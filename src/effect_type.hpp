#pragma once

#include "h-basic.hpp"

/**
 * Lasting spell effects. (Clouds, etc.)
 */
struct effect_type
{
	s16b    time = 0;       /* For how long */
	s16b    dam = 0;        /* How much damage */
	s16b    type = 0;       /* Of which type */
	s16b    cy = 0;         /* Center of the cast*/
	s16b    cx = 0;         /* Center of the cast*/
	s16b    rad = 0;        /* Radius -- if needed */
	u32b    flags = 0;      /* Flags */
};
