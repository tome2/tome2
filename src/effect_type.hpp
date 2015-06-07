#pragma once

#include "h-basic.h"

/**
 * Lasting spell effects. (Clouds, etc.)
 */
struct effect_type
{
	s16b    time;           /* For how long */
	s16b    dam;            /* How much damage */
	s16b    type;           /* Of which type */
	s16b    cy;             /* Center of the cast*/
	s16b    cx;             /* Center of the cast*/
	s16b    rad;            /* Radius -- if needed */
	u32b    flags;          /* Flags */
};
