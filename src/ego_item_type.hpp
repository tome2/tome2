#pragma once

#include "h-basic.h"
#include "object_flag_set.hpp"

#include <array>

/*
 * Size of flag rarity tables
 */
constexpr int FLAG_RARITY_MAX = 6;

/**
 * Ego item descriptors.
 */
struct ego_item_type
{
	const char *name = nullptr;              /* Name */

	bool_ before = FALSE;                    /* Before or after the object name ? */

	byte tval[10] = { 0 };
	byte min_sval[10] = { 0 };
	byte max_sval[10] = { 0 };

	byte rating = 0;                         /* Rating boost */

	byte level = 0;                          /* Minimum level */
	byte rarity = 0;                         /* Object rarity */
	byte mrarity = 0;                        /* Object rarity */

	s16b max_to_h = 0;                       /* Maximum to-hit bonus */
	s16b max_to_d = 0;                       /* Maximum to-dam bonus */
	s16b max_to_a = 0;                       /* Maximum to-ac bonus */

	s16b activate = 0;                       /* Activation Number */

	s32b max_pval = 0;                       /* Maximum pval */

	s32b cost = 0;                           /* Ego-item "cost" */

	byte rar[FLAG_RARITY_MAX] = { 0 };

	std::array<object_flag_set, FLAG_RARITY_MAX> flags;
	std::array<object_flag_set, FLAG_RARITY_MAX> oflags;

	u32b fego[FLAG_RARITY_MAX] = { 0 };      /* ego flags */

	object_flag_set need_flags;
	object_flag_set forbid_flags;

	s16b power = -1;                        /* Power granted, if any */

public:
	ego_item_type()
	{
		std::fill(std::begin(tval),
		          std::end(tval),
		          255);
	}
};
