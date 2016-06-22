#pragma once

#include "h-basic.h"
#include "store_flag_set.hpp"

/**
 * Number of items to choose stock from
 */
constexpr int STORE_CHOICES = 56;

/**
 * Store descriptor.
 */
struct store_info_type
{
	const char *name = nullptr;              /* Name */

	s16b item_kind[STORE_CHOICES] = { 0 };   /* Table -- Legal item kinds */
	s16b item_chance[STORE_CHOICES] = { 0 };
	byte item_num = 0;                       /* Number of items */

	s16b max_obj = 0;                        /* Number of items this store can hold */

	u16b owners[4] = { 0 };                  /* List of owners(refers to ow_info) */

	u16b actions[6] = { 0 };                 /* Actions(refers to ba_info) */

	byte d_attr = 0;                         /* Default building attribute */
	char d_char = '\0';                      /* Default building character */

	byte x_attr = 0;                         /* Desired building attribute */
	char x_char = '\0';                      /* Desired building character */

	store_flag_set flags;                    /* Flags */
};
