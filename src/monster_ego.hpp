#pragma once

#include "h-basic.h"
#include "monster_blow.hpp"
#include "monster_spell_flag_set.hpp"

#include <array>

/**
 * Monster ego descriptors.
 */
struct monster_ego
{
	const char *name = nullptr;              /* Name */
	bool_ before = false;                    /* Display ego before or after */

	std::array<monster_blow, 4> blow { };    /* Up to four blows per round */
	byte blowm[4][2] = {
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 }
	};

	s16b hdice = 0;                          /* Creatures hit dice count */
	s16b hside = 0;                          /* Creatures hit dice sides */

	s16b ac = 0;                             /* Armour Class */

	s16b sleep = 0;                          /* Inactive counter (base) */
	s16b aaf = 0;                            /* Area affect radius (1-100) */
	s16b speed = 0;                          /* Speed (normally 110) */

	s32b mexp = 0;                           /* Exp value for kill */

	s32b weight = 0;                         /* Weight of the monster */

	byte freq_inate = 0;                     /* Inate spell frequency */
	byte freq_spell = 0;                     /* Other spell frequency */

	/* Ego flags */
	u32b flags1 = 0;
	u32b flags2 = 0;
	u32b flags3 = 0;
	u32b flags7 = 0;
	u32b flags8 = 0;
	u32b flags9 = 0;
	u32b hflags1 = 0;
	u32b hflags2 = 0;
	u32b hflags3 = 0;
	u32b hflags7 = 0;
	u32b hflags8 = 0;
	u32b hflags9 = 0;

	/* Monster flags */
	u32b mflags1 = 0;
	u32b mflags2 = 0;
	u32b mflags3 = 0;
	u32b mflags7 = 0;
	u32b mflags8 = 0;
	u32b mflags9 = 0;

	/* Monster spells */
	monster_spell_flag_set mspells;

	/* Negative Flags, to be removed from the monster flags */
	u32b nflags1 = 0;
	u32b nflags2 = 0;
	u32b nflags3 = 0;
	u32b nflags7 = 0;
	u32b nflags8 = 0;
	u32b nflags9 = 0;

	/* Negative spells; to be removed from the monster spells */
	monster_spell_flag_set nspells;

	s16b level = 0;                          /* Level of creature */
	s16b rarity = 0;                         /* Rarity of creature */

	byte d_attr = 0;                         /* Default monster attribute */
	char d_char = '\0';                      /* Default monster character */

	byte g_attr = 0;                         /* Overlay graphic attribute */
	char g_char = '\0';                      /* Overlay graphic character */

	char r_char[5] = { '\0' };               /* Monster race allowed */
	char nr_char[5] = { '\0' };              /* Monster race not allowed */
};
