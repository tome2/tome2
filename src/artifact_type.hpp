#pragma once

#include "h-basic.h"

/**
 * Artifact descriptor.
 *
 * Note that the save-file only writes "cur_num" to the savefile.
 *
 * Note that "max_num" is always "1" (if that artifact "exists")
 */
struct artifact_type
{
	char const *name;		/* Artifact name */
	char *text;			/* Artifact description */

	byte tval;			/* Artifact type */
	byte sval;			/* Artifact sub type */

	s16b pval;			/* Artifact extra info */

	s16b to_h;			/* Bonus to hit */
	s16b to_d;			/* Bonus to damage */
	s16b to_a;			/* Bonus to armor */

	s16b activate;			/* Activation Number */

	s16b ac;			/* Base armor */

	byte dd, ds;		/* Damage when hits */

	s16b weight;		/* Weight */

	s32b cost;			/* Artifact "cost" */

	u32b flags1;		/* Artifact Flags, set 1 */
	u32b flags2;		/* Artifact Flags, set 2 */
	u32b flags3;		/* Artifact Flags, set 3 */
	u32b flags4;            /* Artifact Flags, set 4 */
	u32b flags5;            /* Artifact Flags, set 5 */

	u32b oflags1;		/* Obvious Flags, set 1 */
	u32b oflags2;		/* Obvious Flags, set 2 */
	u32b oflags3;		/* Obvious Flags, set 3 */
	u32b oflags4;           /* Obvious Flags, set 4 */
	u32b oflags5;           /* Obvious Flags, set 5 */

	byte level;			/* Artifact level */
	byte rarity;		/* Artifact rarity */

	byte cur_num;		/* Number created (0 or 1) */
	byte max_num;		/* Unused (should be "1") */

	u32b esp;                       /* ESP flags */
	u32b oesp;                       /* ESP flags */

	s16b power;                     /* Power granted(if any) */

	s16b set;               /* Does it belongs to a set ?*/
};
