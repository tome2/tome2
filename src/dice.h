#pragma once

#include "dice_fwd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dice
 */
struct dice_type
{
	long base;  /* Base value to which roll is added. */
	long num;   /* Number of dice */
	long sides; /* Sides per dice */
};

#ifdef __cplusplus
} // extern "C"
#endif
