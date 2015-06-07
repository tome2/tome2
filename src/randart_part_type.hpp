#pragma once

#include "h-basic.h"

/**
 * Random artifact part descriptor.
 */
struct randart_part_type
{
	byte tval[20];
	byte min_sval[20];
	byte max_sval[20];

	byte level;             /* Minimum level */
	byte rarity;            /* Object rarity */
	byte mrarity;           /* Object rarity */

	s16b max_to_h;          /* Maximum to-hit bonus */
	s16b max_to_d;          /* Maximum to-dam bonus */
	s16b max_to_a;          /* Maximum to-ac bonus */

	s32b max_pval;          /* Maximum pval */

	s32b value;             /* power value */
	s16b max;               /* Number of time it can appear on a single item */

	u32b flags1;            /* Ego-Item Flags, set 1 */
	u32b flags2;            /* Ego-Item Flags, set 2 */
	u32b flags3;            /* Ego-Item Flags, set 3 */
	u32b flags4;            /* Ego-Item Flags, set 4 */
	u32b flags5;            /* Ego-Item Flags, set 5 */
	u32b esp;               /* ESP flags */
	u32b fego;              /* ego flags */

	u32b aflags1;            /* Ego-Item Flags, set 1 */
	u32b aflags2;            /* Ego-Item Flags, set 2 */
	u32b aflags3;            /* Ego-Item Flags, set 3 */
	u32b aflags4;            /* Ego-Item Flags, set 4 */
	u32b aflags5;            /* Ego-Item Flags, set 5 */
	u32b aesp;               /* ESP flags */

	s16b power;             /* Power granted(if any) */
};
