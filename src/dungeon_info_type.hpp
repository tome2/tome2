#pragma once

#include "h-basic.h"
#include "rule_type.hpp"
#include "obj_theme.hpp"

/**
 * Maximum number of towns per dungeon
 */
constexpr int TOWN_DUNGEON = 4;

/* A structure for the != dungeon types */
struct dungeon_info_type
{
	const char *name;               /* Name */
	char *text;                     /* Description */
	char short_name[3];             /* Short name */

	char generator[30];             /* Name of the level generator */

	s16b floor1;                    /* Floor tile 1 */
	byte floor_percent1[2];         /* Chance of type 1 */
	s16b floor2;                    /* Floor tile 2 */
	byte floor_percent2[2];         /* Chance of type 2 */
	s16b floor3;                    /* Floor tile 3 */
	byte floor_percent3[2];         /* Chance of type 3 */
	s16b outer_wall;                /* Outer wall tile */
	s16b inner_wall;                /* Inner wall tile */
	s16b fill_type1;                /* Cave tile 1 */
	byte fill_percent1[2];          /* Chance of type 1 */
	s16b fill_type2;                /* Cave tile 2 */
	byte fill_percent2[2];          /* Chance of type 2 */
	s16b fill_type3;                /* Cave tile 3 */
	byte fill_percent3[2];          /* Chance of type 3 */
	byte fill_method;				/* Smoothing parameter for the above */

	s16b mindepth;                  /* Minimal depth */
	s16b maxdepth;                  /* Maximal depth */

	bool_ principal;                 /* If it's a part of the main dungeon */
	byte min_plev;                  /* Minimal plev needed to enter -- it's an anti-cheating mesure */

	int min_m_alloc_level;          /* Minimal number of monsters per level */
	int max_m_alloc_chance;         /* There is a 1/max_m_alloc_chance chance per round of creating a new monster */

	u32b flags1;                    /* Flags 1 */
	u32b flags2;                    /* Flags 1 */

	int size_x, size_y;             /* Desired numers of panels */

	byte rule_percents[100];        /* Flat rule percents */
	rule_type rules[5];             /* Monster generation rules */

	int final_object;               /* The object you'll find at the bottom */
	int final_artifact;             /* The artifact you'll find at the bottom */
	int final_guardian;             /* The artifact's guardian. If an artifact is specified, then it's NEEDED */

	int ix, iy, ox, oy;             /* Wilderness coordinates of the entrance/output of the dungeon */

	obj_theme objs;                 /* The drops type */

	int d_dice[4];                  /* Number of dices */
	int d_side[4];                  /* Number of sides */
	int d_frequency[4];             /* Frequency of damage (1 is the minimum) */
	int d_type[4];                  /* Type of damage */

	s16b t_idx[TOWN_DUNGEON];       /* The towns */
	s16b t_level[TOWN_DUNGEON];     /* The towns levels */
	s16b t_num;                     /* Number of towns */
};

