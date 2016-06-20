#pragma once

#include "body.hpp"
#include "h-basic.h"
#include "monster_blow.hpp"
#include "monster_spell_flag_set.hpp"
#include "obj_theme.hpp"

#include <array>

/**
 * Monster race descriptors and runtime data, including racial memories.
 *
 * Note that "d_attr" and "d_char" are used for MORE than "visual" stuff.
 *
 * Note that "x_attr" and "x_char" are used ONLY for "visual" stuff.
 *
 * Note that "cur_num" (and "max_num") represent the number of monsters
 * of the given race currently on (and allowed on) the current level.
 * This information yields the "dead" flag for Unique monsters.
 *
 * Note that "max_num" is reset when a new player is created.
 * Note that "cur_num" is reset when a new level is created.
 *
 * Note that several of these fields, related to "recall", can be
 * scrapped if space becomes an issue, resulting in less "complete"
 * monster recall (no knowledge of spells, etc).  All of the "recall"
 * fields have a special prefix to aid in searching for them.
 */
struct monster_race
{
	const char *name = nullptr;              /* Name */
	char *text = nullptr;                    /* Text */

	u16b hdice = 0;                          /* Creatures hit dice count */
	u16b hside = 0;                          /* Creatures hit dice sides */

	s16b ac = 0;                             /* Armour Class */

	s16b sleep = 0;                          /* Inactive counter (base) */
	byte aaf = 0;                            /* Area affect radius (1-100) */
	byte speed = 0;                          /* Speed (normally 110) */

	s32b mexp = 0;                           /* Exp value for kill */

	s32b weight = 0;                         /* Weight of the monster */

	byte freq_inate = 0;                     /* Inate spell frequency */
	byte freq_spell = 0;                     /* Other spell frequency */

	u32b flags1 = 0;			/* Flags 1 (general) */
	u32b flags2 = 0;			/* Flags 2 (abilities) */
	u32b flags3 = 0;			/* Flags 3 (race/resist) */
	u32b flags7 = 0;			/* Flags 7 (movement related abilities) */
	u32b flags8 = 0;			/* Flags 8 (wilderness info) */
	u32b flags9 = 0;			/* Flags 9 (drops info) */

	monster_spell_flag_set spells;          /* Spells */

	std::array<monster_blow, 4> blow { };   /* Up to four blows per round */

	byte body_parts[BODY_MAX] = { 0 };       /* To help to decide what to use when body changing */

	byte artifact_idx = 0;                   /* Artifact index of standard artifact dropped; 0 if none. */
	int  artifact_chance = 0;                /* Percentage chance of dropping the artifact. */

	byte level = 0;                          /* Level of creature */
	byte rarity = 0;                         /* Rarity of creature */

	byte d_attr = 0;                         /* Default monster attribute */
	char d_char = 0;                         /* Default monster character */


	byte x_attr = 0;                         /* Desired monster attribute */
	char x_char = 0;                         /* Desired monster character */

	s16b max_num = 0;                        /* Maximum population allowed per level */
	byte cur_num = 0;                        /* Monster population on current level */

	s16b r_pkills = 0;                       /* Count monsters killed in this life */

	bool_ on_saved = 0;                      /* Is the (unique) on a saved level ? */

	byte total_visible = 0;                  /* Amount of this race that are visible */

	obj_theme drops;                         /* The drops type */

};


