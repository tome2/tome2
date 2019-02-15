#pragma once

#include "h-basic.h"

#include "feature_flag_set.hpp"

#include <string>

/**
 * Terrain feature descriptor.
 */
struct feature_type
{
	std::string name;                        /* Name */

	const char *text = nullptr;              /* Text. May point to shared read-only memory, DO NOT FREE! */
	const char *tunnel = nullptr;            /* Text for tunneling. May point to shared read-only memory, DO NOT FREE! */
	const char *block = nullptr;             /* Text for blocking. May point to shared read-only memory, DO NOT FREE! */

	byte mimic = 0;                          /* Feature to mimic */

	feature_flag_set flags;                  /* First set of flags */

	byte d_attr = 0;                         /* Default feature attribute */
	char d_char = '\0';                      /* Default feature character */

	byte x_attr = 0;                         /* Desired feature attribute */
	char x_char = '\0';                      /* Desired feature character */

	byte shimmer[7];                         /* Shimmer colors */

	int d_dice[4] = { 0 };                   /* Number of dice */
	int d_side[4] = { 0 };                   /* Number of sides */
	int d_frequency[4] = { 0 };              /* Frequency of damage (1 is the minimum) */
	int d_type[4] = { 0 };                   /* Type of damage */
};
