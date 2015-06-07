#pragma once

#include "h-basic.h"

/**
 * Power descriptor. (Racial, class, mutation, artifacts, ...)
 */
struct power_type
{
	const char *name;              /* Name */
	const char *desc_text;         /* Text describing power */
	const char *gain_text;         /* Text displayed on gaining the power */
	const char *lose_text;         /* Text displayed on losing the power */

	byte level;             /* Min level */
	byte cost;              /* Mana/Life cost */
	byte stat;              /* Stat used */
	byte diff;              /* Difficulty */
};
