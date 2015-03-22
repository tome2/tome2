#pragma once

#include "dice_fwd.hpp"

/**
 * Dice
 */
struct dice_type
{
	long base;  /* Base value to which roll is added. */
	long num;   /* Number of dice */
	long sides; /* Sides per dice */
};
