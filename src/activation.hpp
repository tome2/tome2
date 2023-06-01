#pragma once

#include "h-basic.hpp"

/**
 * Activation descriptor.
 */
struct activation
{
	char desc[80];          /* Desc of the activation */
	u32b cost;              /* costs value */
	s16b spell;             /* Spell. */
};
