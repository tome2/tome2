#pragma once

#include "h-basic.h"
#include "rule_type.hpp"
#include "obj_theme.hpp"
#include "dungeon_flag_set.hpp"

#include <array>
#include <string>

/**
 * Maximum number of towns per dungeon
 */
constexpr int TOWN_DUNGEON = 4;

/* A structure for the != dungeon types */
struct dungeon_info_type
{
	std::string name;                             /* Name */
	std::string text;                             /* Description */
	std::string short_name;                       /* Short name */

	std::string generator;                        /* Name of the level generator */

	s16b floor1 = 0;                              /* Floor tile 1 */
	byte floor_percent1[2] = { 0 };               /* Chance of type 1 */
	s16b floor2 = 0;                              /* Floor tile 2 */
	byte floor_percent2[2] = { 0 };               /* Chance of type 2 */
	s16b floor3 = 0;                              /* Floor tile 3 */
	byte floor_percent3[2] = { 0 };               /* Chance of type 3 */
	s16b outer_wall = 0;                          /* Outer wall tile */
	s16b inner_wall = 0;                          /* Inner wall tile */
	s16b fill_type1 = 0;                          /* Cave tile 1 */
	byte fill_percent1[2] = { 0 };                /* Chance of type 1 */
	s16b fill_type2 = 0;                          /* Cave tile 2 */
	byte fill_percent2[2] = { 0 };                /* Chance of type 2 */
	s16b fill_type3 = 0;                          /* Cave tile 3 */
	byte fill_percent3[2] = { 0 };                /* Chance of type 3 */
	byte fill_method = 0;                         /* Smoothing parameter for the above */

	s16b mindepth = 0;                            /* Minimal depth */
	s16b maxdepth = 0;                            /* Maximal depth */

	bool_ principal = 0;                          /* If it's a part of the main dungeon */
	byte min_plev = 0;                            /* Minimal plev needed to enter -- it's an anti-cheating mesure */

	int min_m_alloc_level = 0;                    /* Minimal number of monsters per level */
	int max_m_alloc_chance = 0;                   /* There is a 1/max_m_alloc_chance chance per round of creating a new monster */

	dungeon_flag_set flags { };                   /* Dungeon flags */

	int size_x = 0;
	int size_y = 0;

	byte rule_percents[100] = {0};                /* Flat rule percents */
	std::array<rule_type, 5> rules { };           /* Monster generation rules */

	int final_object = 0;                         /* The object you'll find at the bottom */
	int final_artifact = 0;                       /* The artifact you'll find at the bottom */
	int final_guardian = 0;                       /* The artifact's guardian. If an artifact is specified, then it's NEEDED */

	int ix = 0;                                   /* Wilderness coordinates of entrance */
	int iy = 0;                                   /* Wilderness coordinates of entrance */
	int ox = 0;                                   /* Wilderness coordinates of exit */
	int oy = 0;                                   /* Wilderness coordinates of exit */

	obj_theme objs;                               /* The drops type */

	int d_dice[4] = { 0 };                        /* Number of dices */
	int d_side[4] = { 0 };                        /* Number of sides */
	int d_frequency[4] = { 0 };                   /* Frequency of damage (1 is the minimum) */
	int d_type[4] = { 0 };                        /* Type of damage */

	s16b t_idx[TOWN_DUNGEON] = { 0 };   /* The towns */
	s16b t_level[TOWN_DUNGEON] = { 0 }; /* The towns levels */
	s16b t_num = 0;                     /* Number of towns */
};

