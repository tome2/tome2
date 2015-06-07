#pragma once

#include "h-basic.h"

/**
 * Size of allocation table for objects
 */
constexpr int ALLOCATION_MAX = 8;

/**
 * Object "kind" descriptor. Includes player knowledge.
 *
 * Only "aware" and "tried" are saved in the savefile
 */
struct object_kind
{
	const char *name;               /* Name */
	char *text;                     /* Text */

	byte tval;			/* Object type */
	byte sval;			/* Object sub type */

	s32b pval;                      /* Object extra info */
	s32b pval2;                     /* Object extra info */

	s16b to_h;			/* Bonus to hit */
	s16b to_d;			/* Bonus to damage */
	s16b to_a;			/* Bonus to armor */

	s16b activate;		        /* Activation number */

	s16b ac;			/* Base armor */

	byte dd, ds;		/* Damage dice/sides */

	s32b weight;            /* Weight */

	s32b cost;			/* Object "base cost" */

	u32b flags1;		/* Flags, set 1 */
	u32b flags2;		/* Flags, set 2 */
	u32b flags3;		/* Flags, set 3 */
	u32b flags4;            /* Flags, set 4 */
	u32b flags5;            /* Flags, set 5 */

	u32b oflags1;		/* Obvious Flags, set 1 */
	u32b oflags2;		/* Obvious Flags, set 2 */
	u32b oflags3;		/* Obvious Flags, set 3 */
	u32b oflags4;           /* Obvious Flags, set 4 */
	u32b oflags5;           /* Obvious Flags, set 5 */

	byte locale[ALLOCATION_MAX];		/* Allocation level(s) */
	byte chance[ALLOCATION_MAX];		/* Allocation chance(s) */

	byte level;			/* Level */
	byte extra;			/* Something */


	byte d_attr;		/* Default object attribute */
	char d_char;		/* Default object character */


	byte x_attr;		/* Desired object attribute */
	char x_char;		/* Desired object character */


	byte flavor;			/* Special object flavor (or zero) */

	bool_ easy_know;		/* This object is always known (if aware) */


	bool_ aware;			/* The player is "aware" of the item's effects */

	bool_ tried;			/* The player has "tried" one of the items */

	bool_ know;                      /* extractable flag for the alchemist */

	u32b esp;                       /* ESP flags */
	u32b oesp;                      /* Obvious ESP flags */

	byte btval;                     /* Become Object type */
	byte bsval;                     /* Become Object sub type */
	bool_ artifact;                  /* Is it a normal artifact(already generated) */

	s16b power;                     /* Power granted(if any) */
};
