#pragma once

#include "h-basic.h"
#include "monster_blow.hpp"

/**
 * Monster ego descriptors.
 */
struct monster_ego
{
	const char *name;                       /* Name */
	bool_ before;                           /* Display ego before or after */

	monster_blow blow[4];                   /* Up to four blows per round */
	byte blowm[4][2];

	s16b hdice;                             /* Creatures hit dice count */
	s16b hside;                             /* Creatures hit dice sides */

	s16b ac;				/* Armour Class */

	s16b sleep;				/* Inactive counter (base) */
	s16b aaf;                               /* Area affect radius (1-100) */
	s16b speed;                             /* Speed (normally 110) */

	s32b mexp;				/* Exp value for kill */

	s32b weight;                            /* Weight of the monster */

	byte freq_inate;                /* Inate spell frequency */
	byte freq_spell;		/* Other spell frequency */

	/* Ego flags */
	u32b flags1;                    /* Flags 1 */
	u32b flags2;                    /* Flags 1 */
	u32b flags3;                    /* Flags 1 */
	u32b flags7;                    /* Flags 1 */
	u32b flags8;                    /* Flags 1 */
	u32b flags9;                    /* Flags 1 */
	u32b hflags1;                    /* Flags 1 */
	u32b hflags2;                    /* Flags 1 */
	u32b hflags3;                    /* Flags 1 */
	u32b hflags7;                    /* Flags 1 */
	u32b hflags8;                    /* Flags 1 */
	u32b hflags9;                    /* Flags 1 */

	/* Monster flags */
	u32b mflags1;                    /* Flags 1 (general) */
	u32b mflags2;                    /* Flags 2 (abilities) */
	u32b mflags3;                    /* Flags 3 (race/resist) */
	u32b mflags4;                    /* Flags 4 (inate/breath) */
	u32b mflags5;                    /* Flags 5 (normal spells) */
	u32b mflags6;                    /* Flags 6 (special spells) */
	u32b mflags7;                    /* Flags 7 (movement related abilities) */
	u32b mflags8;                    /* Flags 8 (wilderness info) */
	u32b mflags9;                    /* Flags 9 (drops info) */

	/* Negative Flags, to be removed from the monster flags */
	u32b nflags1;                    /* Flags 1 (general) */
	u32b nflags2;                    /* Flags 2 (abilities) */
	u32b nflags3;                    /* Flags 3 (race/resist) */
	u32b nflags4;                    /* Flags 4 (inate/breath) */
	u32b nflags5;                    /* Flags 5 (normal spells) */
	u32b nflags6;                    /* Flags 6 (special spells) */
	u32b nflags7;                    /* Flags 7 (movement related abilities) */
	u32b nflags8;                    /* Flags 8 (wilderness info) */
	u32b nflags9;                    /* Flags 9 (drops info) */

	s16b level;                     /* Level of creature */
	s16b rarity;                    /* Rarity of creature */


	byte d_attr;			/* Default monster attribute */
	char d_char;			/* Default monster character */

	byte g_attr;                    /* Overlay graphic attribute */
	char g_char;                    /* Overlay graphic character */

	char r_char[5];                 /* Monster race allowed */
	char nr_char[5];                /* Monster race not allowed */
};
