#pragma once

#include "h-basic.h"

/*
 * Size of flag rarity tables
 */
constexpr int FLAG_RARITY_MAX = 6;

/**
 * Ego item descriptors.
 */
struct ego_item_type
{
	const char *name;		/* Name (offset) */

	bool_ before;                    /* Before or after the object name ? */

	byte tval[10];
	byte min_sval[10];
	byte max_sval[10];

	byte rating;		/* Rating boost */

	byte level;			/* Minimum level */
	byte rarity;            /* Object rarity */
	byte mrarity;           /* Object rarity */

	s16b max_to_h;          /* Maximum to-hit bonus */
	s16b max_to_d;          /* Maximum to-dam bonus */
	s16b max_to_a;          /* Maximum to-ac bonus */

	s16b activate;			/* Activation Number */

	s32b max_pval;          /* Maximum pval */

	s32b cost;			/* Ego-item "cost" */

	byte rar[FLAG_RARITY_MAX];
	u32b flags1[FLAG_RARITY_MAX];            /* Ego-Item Flags, set 1 */
	u32b flags2[FLAG_RARITY_MAX];            /* Ego-Item Flags, set 2 */
	u32b flags3[FLAG_RARITY_MAX];            /* Ego-Item Flags, set 3 */
	u32b flags4[FLAG_RARITY_MAX];            /* Ego-Item Flags, set 4 */
	u32b flags5[FLAG_RARITY_MAX];            /* Ego-Item Flags, set 5 */
	u32b esp[FLAG_RARITY_MAX];                       /* ESP flags */
	u32b oflags1[FLAG_RARITY_MAX];           /* Ego-Item Obvious Flags, set 1 */
	u32b oflags2[FLAG_RARITY_MAX];           /* Ego-Item Obvious Flags, set 2 */
	u32b oflags3[FLAG_RARITY_MAX];           /* Ego-Item Obvious Flags, set 3 */
	u32b oflags4[FLAG_RARITY_MAX];           /* Ego-Item Obvious Flags, set 4 */
	u32b oflags5[FLAG_RARITY_MAX];           /* Ego-Item Obvious Flags, set 5 */
	u32b oesp[FLAG_RARITY_MAX];              /* Obvious ESP flags */
	u32b fego[FLAG_RARITY_MAX];              /* ego flags */

	u32b need_flags1;            /* Ego-Item Flags, set 1 */
	u32b need_flags2;            /* Ego-Item Flags, set 2 */
	u32b need_flags3;            /* Ego-Item Flags, set 3 */
	u32b need_flags4;            /* Ego-Item Flags, set 4 */
	u32b need_flags5;            /* Ego-Item Flags, set 5 */
	u32b need_esp;                       /* ESP flags */
	u32b forbid_flags1;            /* Ego-Item Flags, set 1 */
	u32b forbid_flags2;            /* Ego-Item Flags, set 2 */
	u32b forbid_flags3;            /* Ego-Item Flags, set 3 */
	u32b forbid_flags4;            /* Ego-Item Flags, set 4 */
	u32b forbid_flags5;            /* Ego-Item Flags, set 5 */
	u32b forbid_esp;                       /* ESP flags */

	s16b power;                     /* Power granted(if any) */
};
