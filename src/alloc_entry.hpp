#pragma once

#include "h-basic.h"

/**
 * An entry for the object/monster allocation functions
 *
 * Pass 1 is determined from allocation information
 * Pass 2 is determined from allocation restriction
 * Pass 3 is determined from allocation calculation
 */
struct alloc_entry
{
	s16b index = -1;        /* The actual index */

	byte level = 0;         /* Base dungeon level */
	byte prob1 = 0;         /* Probability, pass 1 */
	byte prob2 = 0;         /* Probability, pass 2 */
	byte prob3 = 0;         /* Probability, pass 3 */
};
