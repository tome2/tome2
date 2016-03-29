#pragma once

#include "h-basic.h"

/* Define monster generation rules */
struct rule_type
{
	byte mode = 0;                  /* Mode of combination of the monster flags */
	byte percent = 0;               /* Percent of monsters affected by the rule */

	u32b mflags1 = 0;               /* The monster flags that are allowed */
	u32b mflags2 = 0;
	u32b mflags3 = 0;
	u32b mflags4 = 0;
	u32b mflags5 = 0;
	u32b mflags6 = 0;
	u32b mflags7 = 0;
	u32b mflags8 = 0;
	u32b mflags9 = 0;

	char r_char[5] = { 0 };         /* Monster race allowed */
};
