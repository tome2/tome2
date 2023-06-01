#pragma once

#include "h-basic.hpp"
#include "store_flag_set.hpp"
#include "store_item.hpp"

#include <vector>

/**
 * Store descriptor.
 */
struct store_info_type
{
	std::string name;                        /* Name */

	std::vector<store_item> items;           /* Table -- Legal item kinds */

	s16b max_obj = 0;                        /* Number of items this store can hold */

	std::vector<u16b> owners;                /* List of owners; refers to ow_info */

	std::vector<u16b> actions;               /* Actions; refers to ba_info */

	byte d_attr = 0;                         /* Default building attribute */
	char d_char = '\0';                      /* Default building character */

	byte x_attr = 0;                         /* Desired building attribute */
	char x_char = '\0';                      /* Desired building character */

	store_flag_set flags;                    /* Flags */
};
