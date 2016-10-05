#pragma once

#include "ego_flag_set.hpp"
#include "h-basic.h"
#include "object_flag_set.hpp"

#include <vector>

/**
 * Random artifact part descriptor.
 */
struct randart_part_type
{
public:
	struct kind_filter_t {
		byte tval;
		byte min_sval;
		byte max_sval;
	};

	std::vector<kind_filter_t> kind_filter;

	byte level = 0;                          /* Minimum level */
	byte rarity = 0;                         /* Object rarity */
	byte mrarity = 0;                        /* Object rarity */

	s16b max_to_h = 0;                       /* Maximum to-hit bonus */
	s16b max_to_d = 0;                       /* Maximum to-dam bonus */
	s16b max_to_a = 0;                       /* Maximum to-ac bonus */

	s32b max_pval = 0;                       /* Maximum pval */

	s32b value = 0;                          /* power value */
	s16b max = 0;                            /* Number of time it can appear on a single item */

	object_flag_set flags;                   /* Ego item flags */

	ego_flag_set fego;                       /* Ego flags */

	object_flag_set aflags;                  /* Antagonistic ego item flags */

	s16b power = -1;                         /* Power granted(if any) */

};
