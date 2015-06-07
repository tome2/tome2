#pragma once

#include "h-basic.h"

/* Define monster generation rules */
struct rule_type
{
	byte mode;                      /* Mode of combination of the monster flags */
	byte percent;                   /* Percent of monsters affected by the rule */

	u32b mflags1;                   /* The monster flags that are allowed */
	u32b mflags2;
	u32b mflags3;
	u32b mflags4;
	u32b mflags5;
	u32b mflags6;
	u32b mflags7;
	u32b mflags8;
	u32b mflags9;

	char r_char[5];                 /* Monster race allowed */
};
