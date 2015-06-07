#pragma once

#include "h-basic.h"
#include "terrain.hpp"

/**
 * A structure describing a wilderness area
 * with a terrain, a town or a dungeon entrance
 */
struct wilderness_type_info
{
	const char *name;               /* Name */
	const char *text;               /* Text */

	u16b    entrance;               /* Which town is there(<1000 i's a town, >=1000 it a dungeon) */
	s32b	wild_x;			/* Map coordinates (backed out while parsing map) */
	s32b	wild_y;
	byte    road;                   /* Flags of road */
	int     level;                  /* Difficulty level */
	u32b    flags1;                 /* Some flags */
	byte    feat;                   /* The feature of f_info.txt that is used to allow passing, ... and to get a char/color/graph */
	byte    terrain_idx;            /* Terrain index(defined in defines.h) */

	byte    terrain[MAX_WILD_TERRAIN];/* Feature types for the plasma generator */
};
