#pragma once

#include "h-basic.hpp"
#include "terrain.hpp"

/**
 * A structure describing a wilderness area
 * with a terrain, a town or a dungeon entrance
 */
struct wilderness_type_info
{
	const char *name = nullptr;              /* Name */
	const char *text = nullptr;              /* Text */

	u16b entrance = 0;                       /* Which town is there(<1000 i's a town, >=1000 it a dungeon) */
	s32b wild_x = 0;                         /* Map coordinates (backed out while parsing map) */
	s32b wild_y = 0;
	byte road = 0;                           /* Flags of road */
	int level = 0;                           /* Difficulty level */
	byte feat = 0;                           /* The feature of f_info.txt that is used to allow passing, ... and to get a char/color/graph */
	byte terrain_idx = 0;                    /* Terrain index(defined in defines.h) */

	byte terrain[MAX_WILD_TERRAIN] = { 0 };  /* Feature types for the plasma generator */
};
