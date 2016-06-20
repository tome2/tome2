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
	const char *name = nullptr;              /* Name */

	bool_ before = FALSE;                    /* Before or after the object name ? */

	byte tval[10] = { 0 };
	byte min_sval[10] = { 0 };
	byte max_sval[10] = { 0 };

	byte rating = 0;                         /* Rating boost */

	byte level = 0;                          /* Minimum level */
	byte rarity = 0;                         /* Object rarity */
	byte mrarity = 0;                        /* Object rarity */

	s16b max_to_h = 0;                       /* Maximum to-hit bonus */
	s16b max_to_d = 0;                       /* Maximum to-dam bonus */
	s16b max_to_a = 0;                       /* Maximum to-ac bonus */

	s16b activate = 0;                       /* Activation Number */

	s32b max_pval = 0;                       /* Maximum pval */

	s32b cost = 0;                           /* Ego-item "cost" */

	byte rar[FLAG_RARITY_MAX] = { 0 };

	u32b flags1[FLAG_RARITY_MAX] = { 0 };    /* Ego-Item Flags, set 1 */
	u32b flags2[FLAG_RARITY_MAX] = { 0 };    /* Ego-Item Flags, set 2 */
	u32b flags3[FLAG_RARITY_MAX] = { 0 };    /* Ego-Item Flags, set 3 */
	u32b flags4[FLAG_RARITY_MAX] = { 0 };    /* Ego-Item Flags, set 4 */
	u32b flags5[FLAG_RARITY_MAX] = { 0 };    /* Ego-Item Flags, set 5 */
	u32b esp[FLAG_RARITY_MAX] = { 0 };       /* ESP flags */
	u32b oflags1[FLAG_RARITY_MAX] = { 0 };   /* Ego-Item Obvious Flags, set 1 */
	u32b oflags2[FLAG_RARITY_MAX] = { 0 };   /* Ego-Item Obvious Flags, set 2 */
	u32b oflags3[FLAG_RARITY_MAX] = { 0 };   /* Ego-Item Obvious Flags, set 3 */
	u32b oflags4[FLAG_RARITY_MAX] = { 0 };   /* Ego-Item Obvious Flags, set 4 */
	u32b oflags5[FLAG_RARITY_MAX] = { 0 };   /* Ego-Item Obvious Flags, set 5 */
	u32b oesp[FLAG_RARITY_MAX] = { 0 };      /* Obvious ESP flags */

	u32b fego[FLAG_RARITY_MAX] = { 0 };      /* ego flags */

	u32b need_flags1 = 0;                    /* Ego-Item Flags, set 1 */
	u32b need_flags2 = 0;                    /* Ego-Item Flags, set 2 */
	u32b need_flags3 = 0;                    /* Ego-Item Flags, set 3 */
	u32b need_flags4 = 0;                    /* Ego-Item Flags, set 4 */
	u32b need_flags5 = 0;                    /* Ego-Item Flags, set 5 */
	u32b need_esp = 0;                       /* ESP flags */
	u32b forbid_flags1 = 0;                  /* Ego-Item Flags, set 1 */
	u32b forbid_flags2 = 0;                  /* Ego-Item Flags, set 2 */
	u32b forbid_flags3 = 0;                  /* Ego-Item Flags, set 3 */
	u32b forbid_flags4 = 0;                  /* Ego-Item Flags, set 4 */
	u32b forbid_flags5 = 0;                  /* Ego-Item Flags, set 5 */
	u32b forbid_esp = 0;                     /* ESP flags */

	s16b power = 0;                          /* Power granted, if any */
};
