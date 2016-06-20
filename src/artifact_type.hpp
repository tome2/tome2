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
	char const *name = nullptr;              /* Artifact name */
	char *text = nullptr;                    /* Artifact description */

	byte tval = 0;                           /* Artifact type */
	byte sval = 0;                           /* Artifact sub type */

	s16b pval = 0;                           /* Artifact extra info */

	s16b to_h = 0;                           /* Bonus to hit */
	s16b to_d = 0;                           /* Bonus to damage */
	s16b to_a = 0;                           /* Bonus to armor */

	s16b activate = 0;                       /* Activation Number */

	s16b ac = 0;                             /* Base armor */

	byte dd = 0;                             /* Damage dice */
	byte ds = 0;                             /* Damage sides */

	s16b weight = 0;                         /* Weight */

	s32b cost = 0;                           /* Artifact "cost" */

	u32b flags1 = 0;                         /* Artifact Flags, set 1 */
	u32b flags2 = 0;                         /* Artifact Flags, set 2 */
	u32b flags3 = 0;                         /* Artifact Flags, set 3 */
	u32b flags4 = 0;                         /* Artifact Flags, set 4 */
	u32b flags5 = 0;                         /* Artifact Flags, set 5 */

	u32b oflags1 = 0;                        /* Obvious Flags, set 1 */
	u32b oflags2 = 0;                        /* Obvious Flags, set 2 */
	u32b oflags3 = 0;                        /* Obvious Flags, set 3 */
	u32b oflags4 = 0;                        /* Obvious Flags, set 4 */
	u32b oflags5 = 0;                        /* Obvious Flags, set 5 */

	byte level = 0;                          /* Artifact level */
	byte rarity = 0;                         /* Artifact rarity */

	byte cur_num = 0;                        /* Number created (0 or 1) */
	byte max_num = 0;                        /* Unused (should be "1") */

	u32b esp = 0;                            /* ESP flags */
	u32b oesp = 0;                           /* ESP flags */

	s16b power = 0;                          /* Power granted, if any */

	s16b set = 0;                            /* Which set does it belong it, if any? */

};
