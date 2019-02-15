#pragma once

#include "h-basic.hpp"
#include "monster_blow.hpp"
#include "monster_race_flag_set.hpp"
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
	monster_race_flag_set flags;
	monster_race_flag_set hflags;

	/* Monster flags */
	monster_race_flag_set mflags;

	/* Monster spells */
	monster_spell_flag_set mspells;

	/* Negative flags, to be removed from the monster flags */
	monster_race_flag_set nflags;

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
