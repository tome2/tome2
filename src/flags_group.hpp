#pragma once

#include "h-basic.h"
#include "object_flag_set.hpp"

/**
 * For level gaining artifacts
 */
struct flags_group
{
	char name[30] { };                       /* Name */
	byte color = 0;                          /* Color */

	byte price = 0;                          /* Price to "buy" it */

	object_flag_set flags;                   /* Flags set */
};
