#pragma once

#include "h-basic.h"

/**
 * Number of items to choose stock from
 */
constexpr int STORE_CHOICES = 56;

/**
 * Store descriptor.
 */
struct store_info_type
{
	const char *name;               /* Name */

	s16b table[STORE_CHOICES][2];   /* Table -- Legal item kinds */
	byte table_num;                 /* Number of items */
	s16b max_obj;                   /* Number of items this store can hold */

	u16b owners[4];                 /* List of owners(refers to ow_info) */

	u16b actions[6];                /* Actions(refers to ba_info) */

	byte d_attr;			/* Default building attribute */
	char d_char;			/* Default building character */

	byte x_attr;			/* Desired building attribute */
	char x_char;			/* Desired building character */

	u32b flags1;                    /* Flags */
};
