#pragma once

#include "h-basic.h"

/**
 * Terrain feature descriptor.
 */
struct feature_type
{
	char *name;             /* Name */

	const char *text;       /* Text. May point to shared read-only memory, DO NOT FREE! */
	const char *tunnel;     /* Text for tunneling. May point to shared read-only memory, DO NOT FREE! */
	const char *block;      /* Text for blocking. May point to shared read-only memory, DO NOT FREE! */

	byte mimic;             /* Feature to mimic */

	u32b flags1;            /* First set of flags */

	byte d_attr;		/* Default feature attribute */
	char d_char;		/* Default feature character */

	byte x_attr;		/* Desired feature attribute */
	char x_char;		/* Desired feature character */

	byte shimmer[7];        /* Shimmer colors */

	int d_dice[4];                  /* Number of dices */
	int d_side[4];                  /* Number of sides */
	int d_frequency[4];             /* Frequency of damage (1 is the minimum) */
	int d_type[4];                  /* Type of damage */
};
