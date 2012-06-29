/* File: generate.c */

/* Purpose: Dungeon generation */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#define SAFE_MAX_ATTEMPTS 5000

/*
 * Note that Level generation is *not* an important bottleneck,
 * though it can be annoyingly slow on older machines...  Thus
 * we emphasize "simplicity" and "correctness" over "speed".
 *
 * This entire file is only needed for generating levels.
 * This may allow smart compilers to only load it when needed.
 *
 * Consider the "v_info.txt" file for vault generation.
 *
 * In this file, we use the "special" granite and perma-wall sub-types,
 * where "basic" is normal, "inner" is inside a room, "outer" is the
 * outer wall of a room, and "solid" is the outer wall of the dungeon
 * or any walls that may not be pierced by corridors.  Thus the only
 * wall type that may be pierced by a corridor is the "outer granite"
 * type.  The "basic granite" type yields the "actual" corridors.
 *
 * Note that we use the special "solid" granite wall type to prevent
 * multiple corridors from piercing a wall in two adjacent locations,
 * which would be messy, and we use the special "outer" granite wall
 * to indicate which walls "surround" rooms, and may thus be "pierced"
 * by corridors entering or leaving the room.
 *
 * Note that a tunnel which attempts to leave a room near the "edge"
 * of the dungeon in a direction toward that edge will cause "silly"
 * wall piercings, but will have no permanently incorrect effects,
 * as long as the tunnel can *eventually* exit from another side.
 * And note that the wall may not come back into the room by the
 * hole it left through, so it must bend to the left or right and
 * then optionally re-enter the room (at least 2 grids away).  This
 * is not a problem since every room that is large enough to block
 * the passage of tunnels is also large enough to allow the tunnel
 * to pierce the room itself several times.
 *
 * Note that no two corridors may enter a room through adjacent grids,
 * they must either share an entryway or else use entryways at least
 * two grids apart.  This prevents "large" (or "silly") doorways.
 *
 * To create rooms in the dungeon, we first divide the dungeon up
 * into "blocks" of 11x11 grids each, and require that all rooms
 * occupy a rectangular group of blocks.  As long as each room type
 * reserves a sufficient number of blocks, the room building routines
 * will not need to check bounds.  Note that most of the normal rooms
 * actually only use 23x11 grids, and so reserve 33x11 grids.
 *
 * Note that the use of 11x11 blocks (instead of the old 33x11 blocks)
 * allows more variability in the horizontal placement of rooms, and
 * at the same time has the disadvantage that some rooms (two thirds
 * of the normal rooms) may be "split" by panel boundaries.  This can
 * induce a situation where a player is in a room and part of the room
 * is off the screen.  It may be annoying enough to go back to 33x11
 * blocks to prevent this visual situation.
 *
 * Note that the dungeon generation routines are much different (2.7.5)
 * and perhaps "DUN_ROOMS" should be less than 50.
 *
 * XXX XXX XXX Note that it is possible to create a room which is only
 * connected to itself, because the "tunnel generation" code allows a
 * tunnel to leave a room, wander around, and then re-enter the room.
 *
 * XXX XXX XXX Note that it is possible to create a set of rooms which
 * are only connected to other rooms in that set, since there is nothing
 * explicit in the code to prevent this from happening.  But this is less
 * likely than the "isolated room" problem, because each room attempts to
 * connect to another room, in a giant cycle, thus requiring at least two
 * bizarre occurances to create an isolated section of the dungeon.
 *
 * Note that (2.7.9) monster pits have been split into monster "nests"
 * and monster "pits".  The "nests" have a collection of monsters of a
 * given type strewn randomly around the room (jelly, animal, or undead),
 * while the "pits" have a collection of monsters of a given type placed
 * around the room in an organized manner (orc, troll, giant, dragon, or
 * demon).  Note that both "nests" and "pits" are now "level dependant",
 * and both make 16 "expensive" calls to the "get_mon_num()" function.
 *
 * Note that the cave grid flags changed in a rather drastic manner
 * for Angband 2.8.0 (and 2.7.9+), in particular, dungeon terrain
 * features, such as doors and stairs and traps and rubble and walls,
 * are all handled as a set of 64 possible "terrain features", and
 * not as "fake" objects (440-479) as in pre-2.8.0 versions.
 *
 * The 64 new "dungeon features" will also be used for "visual display"
 * but we must be careful not to allow, for example, the user to display
 * hidden traps in a different way from floors, or secret doors in a way
 * different from granite walls, or even permanent granite in a different
 * way from granite.  XXX XXX XXX
 */


/*
 * Dungeon generation values
 */
#define DUN_ROOMS      50	/* Number of rooms to attempt */
#define DUN_UNUSUAL   194	/* Level/chance of unusual room (was 200) */
#define DUN_DEST       18	/* 1/chance of having a destroyed level */
#define SMALL_LEVEL     6	/* 1/chance of smaller size (3->6) */
#define EMPTY_LEVEL    15	/* 1/chance of being 'empty' (15)*/
#define DARK_EMPTY      5	/* 1/chance of arena level NOT being lit (2)*/
#define XTRA_MAGIC     10	/* 1/chance of having a level with more magic (10)*/
#define DUN_WILD_VAULT 50	/* Chance of finding a wilderness vault. */
#define DUN_WAT_RNG     2	/* Width of rivers */
#define DUN_WAT_CHG    50	/* 1 in 50 chance of junction in river */
#define DUN_CAVERN     30	/* 1/chance of having a cavern level */

/*
 * Dungeon tunnel generation values
 */
#define DUN_TUN_RND    10	/* Chance of random direction */
#define DUN_TUN_CHG    30	/* Chance of changing direction */
#define DUN_TUN_CON    15	/* Chance of extra tunneling */
#define DUN_TUN_PEN    25	/* Chance of doors at room entrances */
#define DUN_TUN_JCT    90	/* Chance of doors at tunnel junctions */

/*
 * Dungeon streamer generation values
 */
#define DUN_STR_DEN     5	/* Density of streamers */
#define DUN_STR_RNG     2	/* Width of streamers */
#define DUN_STR_MAG     3	/* Number of magma streamers */
#define DUN_STR_MC     90	/* 1/chance of treasure per magma */
#define DUN_STR_QUA	    2	/* Number of quartz streamers */
#define DUN_STR_QC     40	/* 1/chance of treasure per quartz */
#define DUN_STR_SAN     1	/* Number of sand streamers */
#define DUN_STR_SC     10	/* 1/chance of treasure per sandwall */
#define DUN_STR_WLW     1	/* Width of lava & water streamers -KMW- */
#define DUN_STR_DWLW    8	/* Density of water & lava streams -KMW- */


/*
 * Dungeon treausre allocation values
 */
#define DUN_AMT_ROOM    9	/* Amount of objects for rooms */
#define DUN_AMT_ITEM    3	/* Amount of objects for rooms/corridors */
#define DUN_AMT_GOLD    3	/* Amount of treasure for rooms/corridors */
#define DUN_AMT_ALTAR   1	/* Amount of altars */
#define DUN_AMT_BETWEEN 2	/* Amount of between gates */
#define DUN_AMT_FOUNTAIN 1	/* Amount of fountains */

/*
 * Hack -- Dungeon allocation "places"
 */
#define ALLOC_SET_CORR  1	/* Hallway */
#define ALLOC_SET_ROOM  2	/* Room */
#define ALLOC_SET_BOTH  3	/* Anywhere */

/*
 * Hack -- Dungeon allocation "types"
 */
#define ALLOC_TYP_RUBBLE	1	/* Rubble */
#define ALLOC_TYP_TRAP		3	/* Trap */
#define ALLOC_TYP_GOLD		4	/* Gold */
#define ALLOC_TYP_OBJECT	5	/* Object */
#define ALLOC_TYP_ALTAR         6       /* Altar */
#define ALLOC_TYP_BETWEEN       7       /* Between */
#define ALLOC_TYP_FOUNTAIN      8       /* Fountain */


/*
 * The "size" of a "generation block" in grids
 */
#define BLOCK_HGT	11
#define BLOCK_WID	11

/*
 * Maximum numbers of rooms along each axis (currently 6x6)
 */
#define MAX_ROOMS_ROW	(MAX_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL	(MAX_WID / BLOCK_WID)


/*
 * Bounds on some arrays used in the "dun_data" structure.
 * These bounds are checked, though usually this is a formality.
 */
#define CENT_MAX	100
#define DOOR_MAX	200
#define WALL_MAX	500
#define TUNN_MAX	900


/*
 * Maximal number of room types
 */
#define ROOM_MAX	12



/*
 * Simple structure to hold a map location
 */


typedef struct coord coord;

struct coord
{
	byte y;
	byte x;
};


/*
 * Room type information
 */

typedef struct room_data room_data;

struct room_data
{
	/* Required size in blocks */
	s16b dy1, dy2, dx1, dx2;

	/* Hack -- minimum level */
	s16b level;
};


/*
 * Structure to hold all "dungeon generation" data
 */

typedef struct dun_data dun_data;

struct dun_data
{
	/* Array of centers of rooms */
	int cent_n;
	coord cent[CENT_MAX];

	/* Array of possible door locations */
	int door_n;
	coord door[DOOR_MAX];

	/* Array of wall piercing locations */
	int wall_n;
	coord wall[WALL_MAX];

	/* Array of tunnel grids */
	int tunn_n;
	coord tunn[TUNN_MAX];

	/* Number of blocks along each axis */
	int row_rooms;
	int col_rooms;

	/* Array of which blocks are used */
	bool_ room_map[MAX_ROOMS_ROW][MAX_ROOMS_COL];

	/* Hack -- there is a pit/nest on this level */
	bool_ crowded;
};

/*
 * Level generator type
 */

typedef struct level_generator_type level_generator_type;
struct level_generator_type
{
	cptr name;
	bool_ (*generator)(cptr);

	bool_ default_stairs;
	bool_ default_monsters;
	bool_ default_objects;
	bool_ default_miscs;

	struct level_generator_type *next;
};

static level_generator_type *level_generators = NULL;

/*
 * Add a new generator
 */
void add_level_generator(cptr name, bool_ (*generator)(), bool_ stairs, bool_ monsters, bool_ objects, bool_ miscs)
{
	level_generator_type *g;

	MAKE(g, level_generator_type);
	g->name = string_make(name);
	g->generator = generator;

	g->default_stairs = stairs;
	g->default_monsters = monsters;
	g->default_objects = objects;
	g->default_miscs = miscs;

	g->next = level_generators;
	level_generators = g;
}


/*
 * Dungeon generation data -- see "cave_gen()"
 */
static dun_data *dun;

/*
 * ???
 */
static int template_race;



/*
 * Array of room types depths
 */
static s16b roomdep[] =
{
	0, 	/* 0 = Nothing */
	1, 	/* 1 = Simple (33x11) */
	1, 	/* 2 = Overlapping (33x11) */
	3, 	/* 3 = Crossed (33x11) */
	3, 	/* 4 = Large (33x11) */
	5, 	/* 5 = Monster nest (33x11) */
	5, 	/* 6 = Monster pit (33x11) */
	5, 	/* 7 = Lesser vault (33x22) */
	10, 	/* 8 = Greater vault (66x44) */
	1, 	/* 9 = Circular rooms (22x22) */
	3, 	/* 10 = Fractal cave (42x24) */
	10, 	/* 11 = Random vault (44x22) */
	10, 	/* 12 = Crypts (22x22) */
};


/*
 * Always picks a correct direction
 */
static void correct_dir(int *rdir, int *cdir, int y1, int x1, int y2, int x2)
{
	/* Extract vertical and horizontal directions */
	*rdir = (y1 == y2) ? 0 : (y1 < y2) ? 1 : -1;
	*cdir = (x1 == x2) ? 0 : (x1 < x2) ? 1 : -1;

	/* Never move diagonally */
	if (*rdir && *cdir)
	{
		if (rand_int(100) < 50)
		{
			*rdir = 0;
		}
		else
		{
			*cdir = 0;
		}
	}
}


/*
 * Pick a random direction
 */
static void rand_dir(int *rdir, int *cdir)
{
	/* Pick a random direction */
	int i = rand_int(4);

	/* Extract the dy/dx components */
	*rdir = ddy_ddd[i];
	*cdir = ddx_ddd[i];
}


/*
 * Convert existing terrain type to "up stairs"
 */
static void place_up_stairs(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	/* Create up stairs */
	if ((rand_int(3) != 0) || (dungeon_flags2 & DF2_NO_SHAFT))
	{
		cave_set_feat(y, x, FEAT_LESS);
	}
	else
	{
		cave_set_feat(y, x, FEAT_SHAFT_UP);
	}

	c_ptr->special = 0;
}


/*
 * Convert existing terrain type to "down stairs" with dungeon changing.
 */
static void place_magical_stairs(int y, int x, byte next)
{
	cave_type *c_ptr = &cave[y][x];

	/* Create up stairs */
	cave_set_feat(y, x, FEAT_MORE);
	c_ptr->special = next;
}


/*
 * Convert existing terrain type to "down stairs"
 */
static void place_down_stairs(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	/*
	 * Create down stairs
	 * All thoses tests are necesary because a shaft can jump up to 4 levels
	 */
	if ((dun_level + 4 > d_info[dungeon_type].maxdepth) ||
	                (rand_int(3) != 0) || (dungeon_flags2 & DF2_NO_SHAFT))
	{
		cave_set_feat(y, x, FEAT_MORE);
	}
	else
	{
		cave_set_feat(y, x, FEAT_SHAFT_DOWN);
	}

	c_ptr->special = 0;
}


/*
 * Helper function for place_new_way. Determine if y, x is one of
 * floor features of the current dungeon
 */
static bool_ is_safe_floor(int y, int x)
{
	dungeon_info_type *d_ptr = &d_info[dungeon_type];
	byte feat = cave[y][x].feat;

	/* One of the legal floor types */
	if (feat == d_ptr->floor1) return (TRUE);
	if (feat == d_ptr->floor2) return (TRUE);
	if (feat == d_ptr->floor3) return (TRUE);

	/* Assume non-floor */
	return (FALSE);
}


/*
 * Place a way to next / previoous level on flat places
 */
void place_new_way(int *y, int *x)
{
	int xx, yy;
	int x0, x1, x2;
	int y0, y1, y2;
	cave_type *c_ptr;
	bool_ ok;
	int i, way_n;
	byte way_x[MAX_WID], way_y[MAX_WID];


	/* Find valid location XXX XXX XXX */
	while (TRUE)
	{
		/* A way on vertical edge */
		if (rand_int(cur_hgt + cur_wid) < cur_hgt)
		{
			/* Pick a random grid */
			yy = *y = rand_int(cur_hgt - 2) + 1;
			xx = *x = 1 + (rand_int(2) * (cur_wid - 3));

			/* Pick a direction */
			if (xx == 1)
			{
				/* Left */
				x0 = + 1;
				y0 = 0;

				/* Sides */
				x1 = 0;
				y1 = -1;
				x2 = 0;
				y2 = + 1;
			}
			else
			{
				/* Right */
				x0 = -1;
				y0 = 0;

				/* Sides */
				x1 = 0;
				y1 = -1;
				x2 = 0;
				y2 = + 1;
			}
		}

		/* A way on horizontal edge */
		else
		{
			/* Pick a random grid */
			xx = *x = rand_int(cur_wid - 2) + 1;
			yy = *y = 1 + (rand_int(2) * (cur_hgt - 3));

			/* Pick a direction */
			if (yy == 1)
			{
				/* Down */
				x0 = 0;
				y0 = + 1;

				/* Sides */
				x1 = -1;
				y1 = 0;
				x2 = + 1;
				y2 = 0;
			}
			else
			{
				/* Up */
				x0 = 0;
				y0 = -1;

				/* Sides */
				x1 = -1;
				y1 = 0;
				x2 = + 1;
				y2 = 0;
			}
		}


		/* Look at the starting location */
		c_ptr = &cave[yy][xx];

		/* Reject locations inside vaults */
		if (c_ptr->info & (CAVE_ICKY)) continue;

		/* Reject permanent features */
		if ((f_info[c_ptr->feat].flags1 & (FF1_PERMANENT)) &&
		                (f_info[c_ptr->feat].flags1 & (FF1_FLOOR))) continue;

		/* Reject room walls */
		if ((c_ptr->info & (CAVE_ROOM)) &&
		                (c_ptr->feat == feat_wall_outer)) continue;

		/* Look at a neighbouring edge */
		c_ptr = &cave[yy + y1][xx + x1];

		/* Reject two adjacent ways */
		if ((c_ptr->feat == FEAT_WAY_MORE) ||
		                (c_ptr->feat == FEAT_WAY_LESS)) continue;

		/* Look at the other neighbouring edge */
		c_ptr = &cave[yy + y2][xx + x2];

		/* Reject two adjacent ways */
		if ((c_ptr->feat == FEAT_WAY_MORE) ||
		                (c_ptr->feat == FEAT_WAY_LESS)) continue;

		/* Look ahead */
		c_ptr = &cave[yy + y0][xx + x0];

		/* Reject two adjacent ways -- relatively rare, but this can happen */
		if ((c_ptr->feat == FEAT_WAY_MORE) ||
		                (c_ptr->feat == FEAT_WAY_LESS)) continue;


		/* Reset counter */
		way_n = 0;

		/* Assume bad location */
		ok = FALSE;

		/* Check if it connects to current dungeon */
		while (in_bounds(yy, xx))
		{
			/* Check grids ahead */
			if (is_safe_floor(yy + y0, xx + x0)) ok = TRUE;

			/* Check side grids */
			if (is_safe_floor(yy + y1, xx + x1)) ok = TRUE;
			if (is_safe_floor(yy + y2, xx + x2)) ok = TRUE;

			/* Connected */
			if (ok) break;

			/* Access grid (ahead) */
			c_ptr = &cave[yy + y0][xx + x0];

			/* Avoid opening vaults */
			if (c_ptr->feat == FEAT_PERM_OUTER)
			{
				/* Comment this out if you find any problems... */
				ok = TRUE;

				break;
			}

			/* Paranoia */
			if (c_ptr->feat == FEAT_PERM_SOLID) break;

			/*
			 * Hack -- Avoid digging room corner
			 *
			 * CAVEAT: Can't handle situations like this:
			 *
			 *     .....########
			 *     .....########
			 *     ######.....>#
			 *     #############
			 *     .....#
			 *     .....#
			 */
			if (c_ptr->info & (CAVE_ROOM))
			{
				cave_type *c1_ptr = &cave[yy + y0 + y1][xx + x0 + x1];
				cave_type *c2_ptr = &cave[yy + y0 + y2][xx + x0 + x2];

				/* Bend the way */
				if ((c1_ptr->info & (CAVE_ROOM)) &&
				                !(c2_ptr->info & (CAVE_ROOM)))
				{
					way_x[way_n] = xx + x1;
					way_y[way_n] = yy + y1;
					way_n++;
					way_x[way_n] = xx + x1 + x0;
					way_y[way_n] = yy + y1 + y0;
					way_n++;
				}

				/* Bend the way -- the other direction */
				else if ((c2_ptr->info & (CAVE_ROOM)) &&
				                !(c1_ptr->info & (CAVE_ROOM)))
				{
					way_x[way_n] = xx + x2;
					way_y[way_n] = yy + y2;
					way_n++;
					way_x[way_n] = xx + x2 + x0;
					way_y[way_n] = yy + y2 + y0;
					way_n++;
				}

				else
				{
					way_x[way_n] = xx + x0;
					way_y[way_n] = yy + y0;
					way_n++;
				}

				ok = TRUE;
				break;
			}

			/* Remember the location */
			way_x[way_n] = xx + x0;
			way_y[way_n] = yy + y0;
			way_n++;

			/* Advance */
			xx += x0;
			yy += y0;
		}

		/* Accept connected corridor */
		if (ok) break;

		/* Try again, until valid location is found */
	}


	/* Actually dig a corridor connecting the way to dungeon */
	for (i = 0; i < way_n; i++)
	{
		/* Dig */
		place_floor(way_y[i], way_x[i]);
	}
}


/*
 * Returns random co-ordinates for player/monster/object
 */
bool_ new_player_spot(int branch)
{
	int	y, x;
	int max_attempts = 5000;

	/* Place the player */
	if (dungeon_flags1 & DF1_FLAT)
	{
		place_new_way(&y, &x);
	}
	else
	{
		while (max_attempts--)
		{
			/* Pick a legal spot */
			y = rand_range(1, cur_hgt - 2);
			x = rand_range(1, cur_wid - 2);

			/* Must be a "naked" floor grid */
			if (!cave_naked_bold(y, x)) continue;

			/* Refuse to start on anti-teleport grids */
			if (cave[y][x].info & (CAVE_ICKY)) continue;

			/* Done */
			break;

		}
	}

	/* Should be -1, actually if we failed... */
	if (max_attempts < 1) return (FALSE);


	/* Save the new player grid */
	p_ptr->py = y;
	p_ptr->px = x;

	/* XXX XXX XXX */
	if (dungeon_stair && !(dungeon_flags2 & DF2_NO_STAIR) && dun_level &&
	                (!is_quest(dun_level) || (old_dun_level < dun_level)) && !branch)
	{
		if (old_dun_level < dun_level)
		{
			place_up_stairs(p_ptr->py , p_ptr->px);
			if (dungeon_flags1 & DF1_FLAT)
			{
				cave_set_feat(p_ptr->py, p_ptr->px, FEAT_WAY_LESS);
			}
		}
		else
		{
			place_down_stairs(p_ptr->py , p_ptr->px);
			if (dungeon_flags1 & DF1_FLAT)
			{
				cave_set_feat(p_ptr->py, p_ptr->px, FEAT_WAY_MORE);
			}
		}
	}

	return (TRUE);
}



/*
 * Count the number of walls adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y, x)"
 *
 * We count only granite walls and permanent walls.
 */
static int next_to_walls(int y, int x)
{
	int	k = 0;

	if (f_info[cave[y + 1][x].feat].flags1 & FF1_WALL) k++;
	if (f_info[cave[y - 1][x].feat].flags1 & FF1_WALL) k++;
	if (f_info[cave[y][x + 1].feat].flags1 & FF1_WALL) k++;
	if (f_info[cave[y][x - 1].feat].flags1 & FF1_WALL) k++;

	return (k);
}



/*
 * Convert existing terrain type to rubble
 */
static void place_rubble(int y, int x)
{
	/* Create rubble */
	cave_set_feat(y, x, FEAT_RUBBLE);
}


/*
 * Place an altar at the given location
 */
static void place_altar(int y, int x)
{
	if (magik(10))
		cave_set_feat(y, x, 164);
}


/*
 * Place a fountain at the given location
 */
static void place_fountain(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];
	int svals[SV_POTION_LAST + SV_POTION2_LAST + 1], maxsval = 0, k;

	/* List of usable svals */
	for (k = 1; k < max_k_idx; k++)
	{
		object_kind *k_ptr = &k_info[k];

		if (((k_ptr->tval == TV_POTION) || (k_ptr->tval == TV_POTION2)) &&
		                (k_ptr->level <= dun_level) && (k_ptr->flags4 & TR4_FOUNTAIN))
		{
			if (k_ptr->tval == TV_POTION2) svals[maxsval] = k_ptr->sval + SV_POTION_LAST;
			else svals[maxsval] = k_ptr->sval;
			maxsval++;
		}
	}

	if (maxsval == 0) return;

	/* Place the fountain */
	if (randint(100) < 30)
	{
		cave_set_feat(y, x, FEAT_EMPTY_FOUNTAIN);
		c_ptr->special2 = 0;
	}
	else
	{
		cave_set_feat(y, x, FEAT_FOUNTAIN);
		c_ptr->special2 = damroll(3, 4);
	}

	c_ptr->special = svals[rand_int(maxsval)];
}


/*
 * Place a between gate at the given location
 */
static void place_between(int y, int x)
{
	cave_type *c_ptr = &cave[y][x], *c1_ptr;
	int gx, gy;

	while (TRUE)
	{
		/* Location */
		gy = rand_int(cur_hgt);
		gx = rand_int(cur_wid);

		/* Require "naked" floor grid */
		if (cave_naked_bold(gy, gx)) break;
	}

	/* Access the target grid */
	c1_ptr = &cave[gy][gx];

	/* Place a pair of between gates */
	cave_set_feat(y, x, FEAT_BETWEEN);
	c_ptr->special = gx + (gy << 8);
	cave_set_feat(gy, gx, FEAT_BETWEEN);
	c1_ptr->special = x + (y << 8);
}


/*
 * Place an up/down staircase at given location
 */
static void place_random_stairs(int y, int x)
{
	/* Paranoia */
	if (!cave_clean_bold(y, x)) return;

	/* Choose a staircase */
	if (!dun_level)
	{
		place_down_stairs(y, x);
	}
	else if (is_quest(dun_level) && (dun_level > 1))
	{
		place_up_stairs(y, x);
	}
	else if (dun_level >= d_info[dungeon_type].maxdepth)
	{
		if (d_info[dungeon_type].next)
		{
			place_magical_stairs(y, x, d_info[dungeon_type].next);
		}
		else
		{
			place_up_stairs(y, x);
		}
	}
	else if (rand_int(100) < 50)
	{
		place_down_stairs(y, x);
	}
	else
	{
		place_up_stairs(y, x);
	}
}


/*
 * Place a locked door at the given location
 */
static void place_locked_door(int y, int x)
{
	/* Create locked door */
	cave_set_feat(y, x, FEAT_DOOR_HEAD + randint(7));
}


/*
 * Place a secret door at the given location
 */
static void place_secret_door(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	/* Vaults */
	if (c_ptr->info & CAVE_ICKY)
	{
		c_ptr->mimic = FEAT_WALL_INNER;
	}

	/* Ordinary room -- use current outer or inner wall */
	else if (c_ptr->info & CAVE_ROOM)
	{
		/* Determine if it's inner or outer XXX XXX XXX */
		if ((cave[y - 1][x].info & CAVE_ROOM) &&
		                (cave[y + 1][x].info & CAVE_ROOM) &&
		                (cave[y][x - 1].info & CAVE_ROOM) &&
		                (cave[y][x + 1].info & CAVE_ROOM))
		{
			c_ptr->mimic = feat_wall_inner;
		}
		else
		{
			c_ptr->mimic = feat_wall_outer;
		}
	}
	else
	{
		c_ptr->mimic = fill_type[rand_int(100)];
	}

	/* Create secret door */
	cave_set_feat(y, x, FEAT_SECRET);
}


/*
 * Place a random type of door at the given location
 */
static void place_random_door(int y, int x)
{
	int tmp;

	/* Choose an object */
	tmp = rand_int(1000);

	/* Open doors (300/1000) */
	if (tmp < 300)
	{
		/* Create open door */
		cave_set_feat(y, x, FEAT_OPEN);
	}

	/* Broken doors (100/1000) */
	else if (tmp < 400)
	{
		/* Create broken door */
		cave_set_feat(y, x, FEAT_BROKEN);
	}

	/* Secret doors (200/1000) */
	else if (tmp < 600)
	{
		/* Create secret door */
		place_secret_door(y, x);
	}

	/* Closed doors (300/1000) */
	else if (tmp < 900)
	{
		/* Create closed door */
		cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);
	}

	/* Locked doors (99/1000) */
	else if (tmp < 999)
	{
		/* Create locked door */
		cave_set_feat(y, x, FEAT_DOOR_HEAD + randint(7));
	}

	/* Stuck doors (1/1000) */
	else
	{
		/* Create jammed door */
		cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x08 + (byte)rand_int(8));
	}
}



/*
 * Places some staircases near walls
 */
static void alloc_stairs(int feat, int num, int walls, int branch)
{
	int y, x, i, j, cnt;

	/* Place "num" stairs */
	for (cnt = 0, i = 0; i < num || (cnt < 1 && num > 1); i++)
	{
		/* Try several times, then decrease "walls" */
		for (j = 0; j <= SAFE_MAX_ATTEMPTS; j++)
		{
			if (dungeon_flags1 & DF1_FLAT)
			{
				place_new_way(&y, &x);
			}
			else
			{
				/* Pick a random grid */
				y = rand_int(cur_hgt);
				x = rand_int(cur_wid);

				/* Require "naked" floor grid */
				if (!cave_naked_bold(y, x)) continue;

				/* Require a certain number of adjacent walls */
				if (next_to_walls(y, x) < walls) continue;
			}

			/* Town -- must go down */
			if (!dun_level)
			{
				/* Clear previous contents, add down stairs */
				if (dungeon_flags1 & DF1_FLAT)
				{
					cave_set_feat(y, x, FEAT_WAY_MORE);
				}
				else if ((rand_int(3) == 0) && (!(dungeon_flags2 & DF2_NO_SHAFT)))
				{
					cave_set_feat(y, x, FEAT_SHAFT_DOWN);
				}
				else
				{
					cave_set_feat(y, x, FEAT_MORE);
				}
			}

			/* Quest -- must go up */
			else if ((is_quest(dun_level) && (dun_level >= 1)) ||
			                ((dun_level >= d_info[dungeon_type].maxdepth) &&
			                 (!(dungeon_flags1 & DF1_FORCE_DOWN))))
			{
				/* Clear previous contents, add up stairs */
				if (dungeon_flags1 & DF1_FLAT)
				{
					cave_set_feat(y, x, FEAT_WAY_LESS);
				}
				else if ((rand_int(3) == 0) && (!(dungeon_flags2 & DF2_NO_SHAFT)))
				{
					cave_set_feat(y, x, FEAT_SHAFT_UP);
				}
				else
				{
					cave_set_feat(y, x, FEAT_LESS);
				}
			}

			/* Requested type */
			else
			{
				/* Clear previous contents, add stairs */
				cave_set_feat(y, x, feat);
			}

			cave[y][x].special = branch;

			/* Count the number of stairs we've actually managed to place. */
			cnt++;

			/* All done */
			break;
		}

		/* Require fewer walls */
		if (walls) walls--;
	}
}




/*
 * Allocates some objects (using "place" and "type")
 */
static void alloc_object(int set, int typ, int num)
{
	int y = 1, x = 1, k;
	int dummy = 0;

	/* Place some objects */
	for (k = 0; k < num; k++)
	{
		/* Pick a "legal" spot */
		while (dummy < SAFE_MAX_ATTEMPTS)
		{
			bool_ room;

			dummy++;

			/* Location */
			y = rand_int(cur_hgt);
			x = rand_int(cur_wid);

			/* Require "naked" floor grid */
			if (!cave_naked_bold(y, x)) continue;

			/* Check for "room" */
			room = (cave[y][x].info & (CAVE_ROOM)) ? TRUE : FALSE;

			/* Require corridor? */
			if ((set == ALLOC_SET_CORR) && room) continue;

			/* Require room? */
			if ((set == ALLOC_SET_ROOM) && !room) continue;

			/* Accept it */
			break;
		}

		if (dummy >= SAFE_MAX_ATTEMPTS)
		{
			if (cheat_room)
			{
				msg_format("Warning! Could not place object, type : %d!", typ);
			}

			return;
		}


		/* Place something */
		switch (typ)
		{
		case ALLOC_TYP_RUBBLE:
			{
				place_rubble(y, x);
				break;
			}

		case ALLOC_TYP_TRAP:
			{
				place_trap(y, x);
				break;
			}

		case ALLOC_TYP_GOLD:
			{
				place_gold(y, x);
				break;
			}

		case ALLOC_TYP_OBJECT:
			{
				place_object(y, x, FALSE, FALSE, OBJ_FOUND_FLOOR);
				break;
			}

		case ALLOC_TYP_ALTAR:
			{
				place_altar(y, x);
				break;
			}

		case ALLOC_TYP_BETWEEN:
			{
				place_between(y, x);
				break;
			}

		case ALLOC_TYP_FOUNTAIN:
			{
				place_fountain(y, x);
				break;
			}
		}
	}
}


/*
 * The following functions create a rectangle (e.g. outer wall of rooms)
 */
void build_rectangle(int y1, int x1, int y2, int x2, int feat, int info)
{
	int y, x;

	/* Top and bottom boundaries */
	for (x = x1; x <= x2; x++)
	{
		cave_set_feat(y1, x, feat);
		cave[y1][x].info |= (info);

		cave_set_feat(y2, x, feat);
		cave[y2][x].info |= (info);
	}

	/* Top and bottom boundaries */
	for (y = y1; y <= y2; y++)
	{
		cave_set_feat(y, x1, feat);
		cave[y][x1].info |= (info);

		cave_set_feat(y, x2, feat);
		cave[y][x2].info |= (info);
	}
}


/*
 * Place water through the dungeon using recursive fractal algorithm
 *
 * Why do those good at math and/or algorithms tend *not* to 
 * place any spaces around binary operators? I've been always
 * wondering. This seems almost a unversal phenomenon...
 * Tried to make those conform to the rule, but there may still
 * some left untouched...
 */
static void recursive_river(int x1, int y1, int x2, int y2,
                            int feat1, int feat2, int width)
{
	int dx, dy, length, l, x, y;
	int changex, changey;
	int ty, tx;


	length = distance(x1, y1, x2, y2);

	if (length > 4)
	{
		/*
		 * Divide path in half and call routine twice.
		 * There is a small chance of splitting the river
		 */
		dx = (x2 - x1) / 2;
		dy = (y2 - y1) / 2;

		if (dy != 0)
		{
			/* perturbation perpendicular to path */
			changex = randint(abs(dy)) * 2 - abs(dy);
		}
		else
		{
			changex = 0;
		}

		if (dx != 0)
		{
			/* perturbation perpendicular to path */
			changey = randint(abs(dx)) * 2 - abs(dx);
		}
		else
		{
			changey = 0;
		}



		/* construct river out of two smaller ones */
		recursive_river(x1, y1, x1 + dx + changex, y1 + dy + changey,
		                feat1, feat2, width);
		recursive_river(x1 + dx + changex, y1 + dy + changey, x2, y2,
		                feat1, feat2, width);

		/* Split the river some of the time -junctions look cool */
		if ((width > 0) && (rand_int(DUN_WAT_CHG) == 0))
		{
			recursive_river(x1 + dx + changex, y1 + dy + changey,
			                x1 + 8 * (dx + changex), y1 + 8 * (dy + changey),
			                feat1, feat2, width - 1);
		}
	}

	/* Actually build the river */
	else
	{
		for (l = 0; l < length; l++)
		{
			x = x1 + l * (x2 - x1) / length;
			y = y1 + l * (y2 - y1) / length;

			for (ty = y - width - 1; ty <= y + width + 1; ty++)
			{
				for (tx = x - width - 1; tx <= x + width + 1; tx++)
				{
					if (!in_bounds(ty, tx)) continue;

					if (cave[ty][tx].feat == feat1) continue;
					if (cave[ty][tx].feat == feat2) continue;

					if (distance(ty, tx, y, x) > rand_spread(width, 1)) continue;

					/* Do not convert permanent features */
					if (cave_perma_bold(ty, tx)) continue;

					/*
					 * Clear previous contents, add feature
					 * The border mainly gets feat2, while the center
					 * gets feat1
					 */
					if (distance(ty, tx, y, x) > width)
					{
						cave_set_feat(ty, tx, feat2);
					}
					else
					{
						cave_set_feat(ty, tx, feat1);
					}

					/* Lava terrain glows */
					if ((feat1 == FEAT_DEEP_LAVA) ||
					                (feat1 == FEAT_SHAL_LAVA))
					{
						cave[ty][tx].info |= CAVE_GLOW;
					}

					/* Hack -- don't teleport here */
					cave[ty][tx].info |= CAVE_ICKY;
				}
			}
		}
	}
}


/*
 * Places water through dungeon.
 */
static void add_river(int feat1, int feat2)
{
	int y2, x2;
	int y1 = 0, x1 = 0, wid;


	/* Hack -- Choose starting point */
	y2 = randint(cur_hgt / 2 - 2) + cur_hgt / 2;
	x2 = randint(cur_wid / 2 - 2) + cur_wid / 2;

	/* Hack -- Choose ending point somewhere on boundary */
	switch (randint(4))
	{
	case 1:
		{
			/* top boundary */
			x1 = randint(cur_wid - 2) + 1;
			y1 = 1;
			break;
		}
	case 2:
		{
			/* left boundary */
			x1 = 1;
			y1 = randint(cur_hgt - 2) + 1;
			break;
		}
	case 3:
		{
			/* right boundary */
			x1 = cur_wid - 1;
			y1 = randint(cur_hgt - 2) + 1;
			break;
		}
	case 4:
		{
			/* bottom boundary */
			x1 = randint(cur_wid - 2) + 1;
			y1 = cur_hgt - 1;
			break;
		}
	}
	wid = randint(DUN_WAT_RNG);
	recursive_river(x1, y1, x2, y2, feat1, feat2, wid);
}


/*
 * Places "streamers" of rock through dungeon
 *
 * Note that their are actually six different terrain features used
 * to represent streamers.  Three each of magma and quartz, one for
 * basic vein, one with hidden gold, and one with known gold.  The
 * hidden gold types are currently unused.
 */
static void build_streamer(int feat, int chance)
{
	int i, tx, ty;
	int y, x, dir;
	int dummy = 0;
	cave_type *c_ptr;


	/* Hack -- Choose starting point */
	y = rand_spread(cur_hgt / 2, 10);
	x = rand_spread(cur_wid / 2, 15);

	/* Choose a random compass direction */
	dir = ddd[rand_int(8)];

	/* Place streamer into dungeon */
	while (dummy < SAFE_MAX_ATTEMPTS)
	{
		dummy++;

		/* One grid per density */
		for (i = 0; i < DUN_STR_DEN; i++)
		{
			int d = DUN_STR_RNG;

			/* Pick a nearby grid */
			while (1)
			{
				ty = rand_spread(y, d);
				tx = rand_spread(x, d);
				if (!in_bounds2(ty, tx)) continue;
				break;
			}

			/* Access the grid */
			c_ptr = &cave[ty][tx];

			/* Only convert "granite" walls */
			if ((c_ptr->feat != feat_wall_inner) &&
			                (c_ptr->feat != feat_wall_outer) &&
			                (c_ptr->feat != d_info[dungeon_type].fill_type1) &&
			                (c_ptr->feat != d_info[dungeon_type].fill_type2) &&
			                (c_ptr->feat != d_info[dungeon_type].fill_type3)) continue;

			/* Clear mimic feature to avoid nasty consequences */
			c_ptr->mimic = 0;

			/* Clear previous contents, add proper vein type */
			cave_set_feat(ty, tx, feat);

			/* Hack -- Add some (known) treasure */
			if (rand_int(chance) == 0)
			{
				cave_set_feat(ty, tx, c_ptr->feat + 0x04);
			}
		}

		if (dummy >= SAFE_MAX_ATTEMPTS)
		{
			if (cheat_room)
			{
				msg_print("Warning! Could not place streamer!");
			}
			return;
		}


		/* Advance the streamer */
		y += ddy[dir];
		x += ddx[dir];

		/* Quit before leaving the dungeon */
		if (!in_bounds(y, x)) break;
	}
}



/*
 * Place streams of water, lava, & trees -KMW-
 * This routine varies the placement based on dungeon level
 * otherwise is similar to build_streamer
 */
static void build_streamer2(int feat, int killwall)
{
	int i, j, mid, tx, ty;
	int y, x, dir;
	int poolchance;
	int poolsize;
	cave_type *c_ptr;

	poolchance = randint(10);

	/* Hack -- Choose starting point */
	y = rand_spread(cur_hgt / 2, 10);
	x = rand_spread(cur_wid / 2, 15);

	/* Choose a random compass direction */
	dir = ddd[rand_int(8)];

	/* Place streamer into dungeon */
	if (poolchance > 2)
	{
		while (TRUE)
		{
			/* One grid per density */
			for (i = 0; i < (DUN_STR_DWLW + 1); i++)
			{
				int d = DUN_STR_WLW;

				/* Pick a nearby grid */
				while (1)
				{
					ty = rand_spread(y, d);
					tx = rand_spread(x, d);
					if (in_bounds(ty, tx)) break;
				}

				/* Access grid */
				c_ptr = &cave[ty][tx];

				/* Never convert vaults */
				if (c_ptr->info & (CAVE_ICKY)) continue;

				/* Reject permanent features */
				if ((f_info[c_ptr->feat].flags1 & (FF1_PERMANENT)) &&
				                (f_info[c_ptr->feat].flags1 & (FF1_FLOOR))) continue;

				/* Avoid converting walls when told so */
				if (killwall == 0)
				{
					if (f_info[c_ptr->feat].flags1 & FF1_WALL) continue;
				}

				/* Clear mimic feature to avoid nasty consequences */
				c_ptr->mimic = 0;

				/* Clear previous contents, add proper vein type */
				cave_set_feat(ty, tx, feat);
			}

			/* Advance the streamer */
			y += ddy[dir];
			x += ddx[dir];

			/* Change direction */
			if (rand_int(20) == 0) dir = ddd[rand_int(8)];

			/* Stop at dungeon edge */
			if (!in_bounds(y, x)) break;
		}
	}

	/* Create pool */
	else if ((feat == FEAT_DEEP_WATER) || (feat == FEAT_DEEP_LAVA))
	{
		poolsize = 5 + randint(10);
		mid = poolsize / 2;

		/* One grid per density */
		for (i = 0; i < poolsize; i++)
		{
			for (j = 0; j < poolsize; j++)
			{
				tx = x + j;
				ty = y + i;

				if (!in_bounds(ty, tx)) continue;

				if (i < mid)
				{
					if (j < mid)
					{
						if ((i + j + 1) < mid)
							continue;
					}
					else if (j > (mid + i))
						continue;
				}
				else if (j < mid)
				{
					if (i > (mid + j))
						continue;
				}
				else if ((i + j) > ((mid * 3)-1))
					continue;

				/* Only convert non-permanent features */
				if (f_info[cave[ty][tx].feat].flags1 & FF1_PERMANENT) continue;

				/* Clear mimic feature to avoid nasty consequences */
				cave[ty][tx].mimic = 0;

				/* Clear previous contents, add proper vein type */
				cave_set_feat(ty, tx, feat);
			}
		}
	}
}



/*
 * Build a destroyed level
 */
static void destroy_level(void)
{
	int y1, x1, y, x, k, t, n;

	cave_type *c_ptr;

	/* Note destroyed levels */
	if ((cheat_room) || (p_ptr->precognition)) msg_print("Destroyed Level");

	/* Drop a few epi-centers (usually about two) */
	for (n = 0; n < randint(5); n++)
	{
		/* Pick an epi-center */
		x1 = rand_range(5, cur_wid - 1 - 5);
		y1 = rand_range(5, cur_hgt - 1 - 5);

		/* Big area of affect */
		for (y = (y1 - 15); y <= (y1 + 15); y++)
		{
			for (x = (x1 - 15); x <= (x1 + 15); x++)
			{
				/* Skip illegal grids */
				if (!in_bounds(y, x)) continue;

				/* Extract the distance */
				k = distance(y1, x1, y, x);

				/* Stay in the circle of death */
				if (k >= 16) continue;

				/* Delete the monster (if any) */
				delete_monster(y, x);

				/* Destroy valid grids */
				if (cave_valid_bold(y, x))
				{
					/* Delete objects */
					delete_object(y, x);

					/* Access the grid */
					c_ptr = &cave[y][x];

					/* Wall (or floor) type */
					t = rand_int(200);

					/* Granite */
					if (t < 20)
					{
						/* Create granite wall */
						cave_set_feat(y, x, FEAT_WALL_EXTRA);
					}

					/* Quartz */
					else if (t < 60)
					{
						/* Create quartz vein */
						cave_set_feat(y, x, FEAT_QUARTZ);
					}

					/* Magma */
					else if (t < 90)
					{
						/* Create magma vein */
						cave_set_feat(y, x, FEAT_MAGMA);
					}

					/* Sand */
					else if (t < 110)
					{
						/* Create sand vein */
						cave_set_feat(y, x, FEAT_SANDWALL);
					}

					/* Floor */
					else
					{
						/* Create floor */
						place_floor(y, x);
					}

					/* No longer part of a room or vault */
					c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

					/* No longer illuminated or known */
					c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);
				}
			}
		}
	}
}


/*
 * Function that sees if a square is a floor (Includes range checking)
 */
static bool_ get_is_floor(int x, int y)
{
	/* Out of bounds */
	if (!in_bounds(y, x)) return (FALSE);

	/* Do the real check: */
	if (f_info[cave[y][x].feat].flags1 & FF1_FLOOR) return (TRUE);

	return (FALSE);
}


/*
 * Tunnel around a room if it will cut off part of a cave system
 */
static void check_room_boundary(int x1, int y1, int x2, int y2)
{
	int count, x, y;
	bool_ old_is_floor, new_is_floor;

	/* Avoid doing this in irrelevant places -- pelpel */
	if (!(dungeon_flags1 & DF1_CAVERN)) return;

	/* Initialize */
	count = 0;

	old_is_floor = get_is_floor(x1 - 1, y1);

	/*
	 * Count the number of floor-wall boundaries around the room
	 * Note: diagonal squares are ignored since the player can move diagonally
	 * to bypass these if needed.
	 */

	/* Above the top boundary */
	for (x = x1; x <= x2; x++)
	{
		new_is_floor = get_is_floor(x, y1 - 1);

		/* increment counter if they are different */
		if (new_is_floor != old_is_floor) count++;

		old_is_floor = new_is_floor;
	}

	/* Right boundary */
	for (y = y1; y <= y2; y++)
	{
		new_is_floor = get_is_floor(x2 + 1, y);

		/* increment counter if they are different */
		if (new_is_floor != old_is_floor) count++;

		old_is_floor = new_is_floor;
	}

	/* Bottom boundary*/
	for (x = x2; x >= x1; x--)
	{
		new_is_floor = get_is_floor(x, y2 + 1);

		/* Increment counter if they are different */
		if (new_is_floor != old_is_floor) count++;

		old_is_floor = new_is_floor;
	}

	/* Left boundary */
	for (y = y2; y >= y1; y--)
	{
		new_is_floor = get_is_floor(x1 - 1, y);

		/* Increment counter if they are different */
		if (new_is_floor != old_is_floor) count++;

		old_is_floor = new_is_floor;
	}


	/* If all the same, or only one connection exit */
	if ((count == 0) || (count == 2)) return;


	/* Tunnel around the room so to prevent problems with caves */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			if (in_bounds(y, x)) place_floor(y, x);
		}
	}
}


/*
 * Create up to "num" objects near the given coordinates
 * Only really called by some of the "vault" routines.
 */
static void vault_objects(int y, int x, int num)
{
	int dummy = 0;
	int i = 0, j = y, k = x;


	/* Attempt to place 'num' objects */
	for (; num > 0; --num)
	{
		/* Try up to 11 spots looking for empty space */
		for (i = 0; i < 11; ++i)
		{
			/* Pick a random location */
			while (dummy < SAFE_MAX_ATTEMPTS)
			{
				j = rand_spread(y, 2);
				k = rand_spread(x, 3);
				dummy++;
				if (in_bounds(j, k)) break;
			}


			if (dummy >= SAFE_MAX_ATTEMPTS)
			{
				if (cheat_room)
				{
					msg_print("Warning! Could not place vault object!");
				}
			}


			/* Require "clean" floor space */
			if (!cave_clean_bold(j, k)) continue;

			/* Place an item */
			if (rand_int(100) < 75)
			{
				place_object(j, k, FALSE, FALSE, OBJ_FOUND_FLOOR);
			}

			/* Place gold */
			else
			{
				place_gold(j, k);
			}

			/* Placement accomplished */
			break;
		}
	}
}


/*
 * Place a trap with a given displacement of point
 */
static void vault_trap_aux(int y, int x, int yd, int xd)
{
	int count = 0, y1 = y, x1 = x;
	int dummy = 0;

	/* Place traps */
	for (count = 0; count <= 5; count++)
	{
		/* Get a location */
		while (dummy < SAFE_MAX_ATTEMPTS)
		{
			y1 = rand_spread(y, yd);
			x1 = rand_spread(x, xd);
			dummy++;
			if (in_bounds(y1, x1)) break;
		}

		if (dummy >= SAFE_MAX_ATTEMPTS)
		{
			if (cheat_room)
			{
				msg_print("Warning! Could not place vault trap!");
			}
		}


		/* Require "naked" floor grids */
		if (!cave_naked_bold(y1, x1)) continue;

		/* Place the trap */
		place_trap(y1, x1);

		/* Done */
		break;
	}
}


/*
 * Place some traps with a given displacement of given location
 */
static void vault_traps(int y, int x, int yd, int xd, int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		vault_trap_aux(y, x, yd, xd);
	}
}


/*
 * Hack -- Place some sleeping monsters near the given location
 */
static void vault_monsters(int y1, int x1, int num)
{
	int k, i, y, x;

	/* Try to summon "num" monsters "near" the given location */
	for (k = 0; k < num; k++)
	{
		/* Try nine locations */
		for (i = 0; i < 9; i++)
		{
			int d = 1;

			/* Pick a nearby location */
			scatter(&y, &x, y1, x1, d);

			/* Require "empty" floor grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Place the monster (allow groups) */
			monster_level = dun_level + 2;
			(void)place_monster(y, x, TRUE, TRUE);
			monster_level = dun_level;
		}
	}
}

/*
 * Allocate the space needed by a room in the room_map array.
 *
 * width, height represent the size of the room (0...x-1) by (0...y-1).
 * crowded is used to denote a monset nest.
 * by0, bx0 are the positions in the room_map array given to the build_type'x'
 * function.
 * cx, cy are the returned center of the allocated room in coordinates for
 * cave.feat and cave.info etc.
 */
bool_ room_alloc(int width, int height, bool_ crowded, int by0, int bx0, int *cx, int *cy)
{
	int temp, eby, ebx, by, bx;

	/* Calculate number of room_map squares to allocate */

	/* Total number along width */
	temp = ((width - 1) / BLOCK_WID) + 1;

	for (ebx = bx0 + temp; bx0 > 0 && ebx > dun->col_rooms; bx0--, ebx--);

	if (ebx > dun->col_rooms) return (FALSE);

	/* Total number along height */
	temp = ((height - 1) / BLOCK_HGT) + 1;

	for (eby = by0 + temp; by0 > 0 && eby > dun->row_rooms; by0--, eby--);

	/* Never run off the screen */
	if (eby > dun->row_rooms) return (FALSE);

	/* Verify open space */
	for (by = by0; by < eby; by++)
	{
		for (bx = bx0; bx < ebx; bx++)
		{
			if (dun->room_map[by][bx]) return (FALSE);
		}
	}

	/*
	 * It is *extremely* important that the following calculation
	 * be *exactly* correct to prevent memory errors XXX XXX XXX
	 */

	/* Acquire the location of the room */
	*cy = ((by0 + eby) * BLOCK_HGT) / 2;
	*cx = ((bx0 + ebx) * BLOCK_WID) / 2;

	/* Save the room location */
	if (dun->cent_n < CENT_MAX)
	{
		dun->cent[dun->cent_n].y = *cy;
		dun->cent[dun->cent_n].x = *cx;
		dun->cent_n++;
	}

	/* Reserve some blocks */
	for (by = by0; by < eby; by++)
	{
		for (bx = bx0; bx < ebx; bx++)
		{
			dun->room_map[by][bx] = TRUE;
		}
	}

	/* Count "crowded" rooms */
	if (crowded) dun->crowded = TRUE;

	/*
	 * Hack -- See if room will cut off a cavern.
	 * If so, fix by tunneling outside the room in such a way as
	 * to conect the caves.
	 */
	check_room_boundary(*cx - width / 2 - 1, *cy - height / 2 - 1,
	                    *cx + width / 2 + 1, *cy + height / 2 + 1);

	/* Success */
	return (TRUE);
}

/*
 * Room building routines.
 *
 * Room types:
 *   1 -- normal
 *   2 -- overlapping
 *   3 -- cross shaped
 *   4 -- large room with features
 *   5 -- monster nests
 *   6 -- monster pits
 *   7 -- simple vaults
 *   8 -- greater vaults
 *   9 -- circular rooms
 */

/*
 * Type 1 -- normal rectangular rooms
 */
static void build_type1(int by0, int bx0)
{
	u16b info;
	int y, x = 1, y2, x2, yval, xval;
	int y1, x1, xsize, ysize;

	/* Pick a room size */
	y1 = rand_range(1, 4);
	x1 = rand_range(1, 10);
	y2 = rand_range(1, 3);
	x2 = rand_range(1, 9);

	xsize = x1 + x2;
	ysize = y1 + y2;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 2, ysize + 2, FALSE, by0, bx0, &xval, &yval)) return;

	/* Get corner values */
	y1 = yval - ysize / 2;
	x1 = xval - xsize / 2;
	y2 = y1 + ysize - 1;
	x2 = x1 + xsize - 1;

	info = (dun_level <= randint(25)) ? (CAVE_ROOM|CAVE_GLOW) : CAVE_ROOM;

	/* Place a full floor under the room */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}

	/* Walls around the room */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, info);

	/* Hack -- Occasional pillar room */
	if (ysize > 2 && xsize > 2)
	{
		if (rand_int(20) == 0)
		{
			for (y = y1; y <= y2; y += 2)
			{
				for (x = x1; x <= x2; x += 2)
				{
					cave_set_feat(y, x, feat_wall_inner);
				}
			}
		}

		/* Hack -- Occasional ragged-edge room */
		else if (rand_int(50) == 0)
		{
			for (y = y1 + 2; y <= y2 - 2; y += 2)
			{
				cave_set_feat(y, x1, feat_wall_inner);
				cave_set_feat(y, x2, feat_wall_inner);
			}
			for (x = x1 + 2; x <= x2 - 2; x += 2)
			{
				cave_set_feat(y1, x, feat_wall_inner);
				cave_set_feat(y2, x, feat_wall_inner);
			}
		}
	}
}

/*
 * Type 2 -- Overlapping rectangular rooms
 */
static void build_type2(int by0, int bx0)
{
	u16b info;
	int y, x, yval, xval;
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(25, 11, FALSE, by0, bx0, &xval, &yval)) return;

	/* Determine extents of the first room */
	y1a = yval - randint(4);
	y2a = yval + randint(3);
	x1a = xval - randint(14);
	x2a = xval + randint(6);

	/* Determine extents of the second room */
	y1b = yval - randint(3);
	y2b = yval + randint(4);
	x1b = xval - randint(6);
	x2b = xval + randint(14);

	info = (dun_level <= randint(25)) ? (CAVE_ROOM|CAVE_GLOW) : CAVE_ROOM;

	/* Place the walls around room "a" */
	build_rectangle(y1a - 1, x1a - 1, y2a + 1, x2a + 1, feat_wall_outer, info);

	/* Place the walls around room "a" */
	build_rectangle(y1b - 1, x1b - 1, y2b + 1, x2b + 1, feat_wall_outer, info);

	/* Replace the floor for room "a" */
	for (y = y1a; y <= y2a; y++)
	{
		for (x = x1a; x <= x2a; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}

	/* Replace the floor for room "b" */
	for (y = y1b; y <= y2b; y++)
	{
		for (x = x1b; x <= x2b; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}
}

/*
 * Type 3 -- Cross shaped rooms
 *
 * Builds a room at a row, column coordinate
 *
 * Room "a" runs north/south, and Room "b" runs east/east
 * So the "central pillar" runs from x1a,y1b to x2a,y2b.
 *
 * Note that currently, the "center" is always 3x3, but I think that
 * the code below will work (with "bounds checking") for 5x5, or even
 * for unsymetric values like 4x3 or 5x3 or 3x4 or 3x5, or even larger.
 */
static void build_type3(int by0, int bx0)
{
	u16b info;
	int y, x, dy, dx, wy, wx;
	int y1a, x1a, y2a, x2a;
	int y1b, x1b, y2b, x2b;
	int yval, xval;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(25, 11, FALSE, by0, bx0, &xval, &yval)) return;

	/* For now, always 3x3 */
	wx = wy = 1;

	/* Pick max vertical size (at most 4) */
	dy = rand_range(3, 4);

	/* Pick max horizontal size (at most 11) */
	dx = rand_range(3, 11);

	/* Determine extents of the north/south room */
	y1a = yval - dy;
	y2a = yval + dy;
	x1a = xval - wx;
	x2a = xval + wx;

	/* Determine extents of the east/west room */
	y1b = yval - wy;
	y2b = yval + wy;
	x1b = xval - dx;
	x2b = xval + dx;

	info = (dun_level <= randint(25)) ? (CAVE_ROOM|CAVE_GLOW) : CAVE_ROOM;

	/* Place the walls around room "a" */
	build_rectangle(y1a - 1, x1a - 1, y2a + 1, x2a + 1, feat_wall_outer, info);

	/* Place the walls around room "a" */
	build_rectangle(y1b - 1, x1b - 1, y2b + 1, x2b + 1, feat_wall_outer, info);

	/* Replace the floor for room "a" */
	for (y = y1a; y <= y2a; y++)
	{
		for (x = x1a; x <= x2a; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}

	/* Replace the floor for room "b" */
	for (y = y1b; y <= y2b; y++)
	{
		for (x = x1b; x <= x2b; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}

	/* Special features (3/4) */
	switch (rand_int(4))
	{
		/* Large solid middle pillar */
	case 1:
		{
			for (y = y1b; y <= y2b; y++)
			{
				for (x = x1a; x <= x2a; x++)
				{
					cave_set_feat(y, x, feat_wall_inner);
				}
			}
			break;
		}

		/* Inner treasure vault */
	case 2:
		{
			/* Build the vault */
			build_rectangle(y1b, x1a, y2b, x2a, feat_wall_inner, info);

			/* Place a secret door on the inner room */
			switch (rand_int(4))
			{
			case 0:
				place_secret_door(y1b, xval);
				break;
			case 1:
				place_secret_door(y2b, xval);
				break;
			case 2:
				place_secret_door(yval, x1a);
				break;
			case 3:
				place_secret_door(yval, x2a);
				break;
			}

			/* Place a treasure in the vault */
			place_object(yval, xval, FALSE, FALSE, OBJ_FOUND_FLOOR);

			/* Let's guard the treasure well */
			vault_monsters(yval, xval, rand_int(2) + 3);

			/* Traps naturally */
			vault_traps(yval, xval, 4, 4, rand_int(3) + 2);

			break;
		}

		/* Something else */
	case 3:
		{
			/* Occasionally pinch the center shut */
			if (rand_int(3) == 0)
			{
				/* Pinch the east/west sides */
				for (y = y1b; y <= y2b; y++)
				{
					if (y == yval) continue;
					cave_set_feat(y, x1a - 1, feat_wall_inner);
					cave_set_feat(y, x2a + 1, feat_wall_inner);
				}

				/* Pinch the north/south sides */
				for (x = x1a; x <= x2a; x++)
				{
					if (x == xval) continue;
					cave_set_feat(y1b - 1, x, feat_wall_inner);
					cave_set_feat(y2b + 1, x, feat_wall_inner);
				}

				/* Sometimes shut using secret doors */
				if (rand_int(3) == 0)
				{
					place_secret_door(yval, x1a - 1);
					place_secret_door(yval, x2a + 1);
					place_secret_door(y1b - 1, xval);
					place_secret_door(y2b + 1, xval);
				}
			}

			/* Occasionally put a "plus" in the center */
			else if (rand_int(3) == 0)
			{
				cave_set_feat(yval, xval, feat_wall_inner);
				cave_set_feat(y1b, xval, feat_wall_inner);
				cave_set_feat(y2b, xval, feat_wall_inner);
				cave_set_feat(yval, x1a, feat_wall_inner);
				cave_set_feat(yval, x2a, feat_wall_inner);
			}

			/* Occasionally put a pillar in the center */
			else if (rand_int(3) == 0)
			{
				cave_set_feat(yval, xval, feat_wall_inner);
			}

			break;
		}
	}
}

/*
 * Type 4 -- Large room with inner features
 *
 * Possible sub-types:
 *	1 - Just an inner room with one door
 *	2 - An inner room within an inner room
 *	3 - An inner room with pillar(s)
 *	4 - Inner room has a maze
 *	5 - A set of four inner rooms
 */
static void build_type4(int by0, int bx0)
{
	u16b info;
	int y, x, y1, x1;
	int y2, x2, tmp, yval, xval;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(25, 11, FALSE, by0, bx0, &xval, &yval)) return;

	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;

	info = (dun_level <= randint(25)) ? (CAVE_ROOM|CAVE_GLOW) : CAVE_ROOM;

	/* Place a full floor under the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= info;
		}
	}

	/* Outer Walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, info);

	/* The inner room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_inner, info);

	/* Inner room variations */
	switch (randint(5))
	{
		/* Just an inner room with a monster */
	case 1:
		{
			/* Place a secret door */
			switch (randint(4))
			{
			case 1:
				place_secret_door(y1 - 1, xval);
				break;
			case 2:
				place_secret_door(y2 + 1, xval);
				break;
			case 3:
				place_secret_door(yval, x1 - 1);
				break;
			case 4:
				place_secret_door(yval, x2 + 1);
				break;
			}

			/* Place a monster in the room */
			vault_monsters(yval, xval, 1);

			break;
		}

		/* Treasure Vault (with a door) */
	case 2:
		{
			/* Place a secret door */
			switch (randint(4))
			{
			case 1:
				place_secret_door(y1 - 1, xval);
				break;
			case 2:
				place_secret_door(y2 + 1, xval);
				break;
			case 3:
				place_secret_door(yval, x1 - 1);
				break;
			case 4:
				place_secret_door(yval, x2 + 1);
				break;
			}

			/* Place another inner room */
			build_rectangle(yval - 1, xval - 1, yval + 1, xval + 1,
			                feat_wall_inner, info);

			/* Place a locked door on the inner room */
			switch (randint(4))
			{
			case 1:
				place_locked_door(yval - 1, xval);
				break;
			case 2:
				place_locked_door(yval + 1, xval);
				break;
			case 3:
				place_locked_door(yval, xval - 1);
				break;
			case 4:
				place_locked_door(yval, xval + 1);
				break;
			}

			/* Monsters to guard the "treasure" */
			vault_monsters(yval, xval, randint(3) + 2);

			/* Object (80%) */
			if (rand_int(100) < 80)
			{
				place_object(yval, xval, FALSE, FALSE, OBJ_FOUND_FLOOR);
			}

			/* Stairs (20%) */
			else
			{
				place_random_stairs(yval, xval);
			}

			/* Traps to protect the treasure */
			vault_traps(yval, xval, 4, 10, 2 + randint(3));

			break;
		}

		/* Inner pillar(s). */
	case 3:
		{
			/* Place a secret door */
			switch (randint(4))
			{
			case 1:
				place_secret_door(y1 - 1, xval);
				break;
			case 2:
				place_secret_door(y2 + 1, xval);
				break;
			case 3:
				place_secret_door(yval, x1 - 1);
				break;
			case 4:
				place_secret_door(yval, x2 + 1);
				break;
			}

			/* Large Inner Pillar */
			for (y = yval - 1; y <= yval + 1; y++)
			{
				for (x = xval - 1; x <= xval + 1; x++)
				{
					cave_set_feat(y, x, feat_wall_inner);
				}
			}

			/* Occasionally, two more Large Inner Pillars */
			if (rand_int(2) == 0)
			{
				tmp = randint(2);
				for (y = yval - 1; y <= yval + 1; y++)
				{
					for (x = xval - 5 - tmp; x <= xval - 3 - tmp; x++)
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
					for (x = xval + 3 + tmp; x <= xval + 5 + tmp; x++)
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
				}
			}

			/* Occasionally, some Inner rooms */
			if (rand_int(3) == 0)
			{
				/* Long horizontal walls */
				for (x = xval - 5; x <= xval + 5; x++)
				{
					cave_set_feat(yval - 1, x, feat_wall_inner);
					cave_set_feat(yval + 1, x, feat_wall_inner);
				}

				/* Close off the left/right edges */
				cave_set_feat(yval, xval - 5, feat_wall_inner);
				cave_set_feat(yval, xval + 5, feat_wall_inner);

				/* Secret doors (random top/bottom) */
				place_secret_door(yval - 3 + (randint(2) * 2), xval - 3);
				place_secret_door(yval - 3 + (randint(2) * 2), xval + 3);

				/* Monsters */
				vault_monsters(yval, xval - 2, randint(2));
				vault_monsters(yval, xval + 2, randint(2));

				/* Objects */
				if (rand_int(3) == 0) place_object(yval, xval - 2, FALSE, FALSE, OBJ_FOUND_FLOOR);
				if (rand_int(3) == 0) place_object(yval, xval + 2, FALSE, FALSE, OBJ_FOUND_FLOOR);
			}

			break;
		}

		/* Maze inside. */
	case 4:
		{
			/* Place a secret door */
			switch (randint(4))
			{
			case 1:
				place_secret_door(y1 - 1, xval);
				break;
			case 2:
				place_secret_door(y2 + 1, xval);
				break;
			case 3:
				place_secret_door(yval, x1 - 1);
				break;
			case 4:
				place_secret_door(yval, x2 + 1);
				break;
			}

			/* Maze (really a checkerboard) */
			for (y = y1; y <= y2; y++)
			{
				for (x = x1; x <= x2; x++)
				{
					if (0x1 & (x + y))
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
				}
			}

			/* Monsters just love mazes. */
			vault_monsters(yval, xval - 5, randint(3));
			vault_monsters(yval, xval + 5, randint(3));

			/* Traps make them entertaining. */
			vault_traps(yval, xval - 3, 2, 8, randint(3));
			vault_traps(yval, xval + 3, 2, 8, randint(3));

			/* Mazes should have some treasure too. */
			vault_objects(yval, xval, 3);

			break;
		}

		/* Four small rooms. */
	case 5:
		{
			/* Inner "cross" */
			for (y = y1; y <= y2; y++)
			{
				cave_set_feat(y, xval, feat_wall_inner);
			}

			for (x = x1; x <= x2; x++)
			{
				cave_set_feat(yval, x, feat_wall_inner);
			}

			/* Doors into the rooms */
			if (rand_int(100) < 50)
			{
				int i = randint(10);
				place_secret_door(y1 - 1, xval - i);
				place_secret_door(y1 - 1, xval + i);
				place_secret_door(y2 + 1, xval - i);
				place_secret_door(y2 + 1, xval + i);
			}
			else
			{
				int i = randint(3);
				place_secret_door(yval + i, x1 - 1);
				place_secret_door(yval - i, x1 - 1);
				place_secret_door(yval + i, x2 + 1);
				place_secret_door(yval - i, x2 + 1);
			}

			/* Treasure, centered at the center of the cross */
			vault_objects(yval, xval, 2 + randint(2));

			/* Gotta have some monsters. */
			vault_monsters(yval + 1, xval - 4, randint(4));
			vault_monsters(yval + 1, xval + 4, randint(4));
			vault_monsters(yval - 1, xval - 4, randint(4));
			vault_monsters(yval - 1, xval + 4, randint(4));

			break;
		}
	}
}


/*
 * Determine if the given monster is appropriate for inclusion in
 * a monster nest or monster pit or the given type.
 *
 * None of the pits/nests are allowed to include "unique" monsters,
 * or monsters which can "multiply".
 *
 * Some of the pits/nests are asked to avoid monsters which can blink
 * away or which are invisible.  This is probably a hack.
 *
 * The old method made direct use of monster "names", which is bad.
 *
 * Note the use of Angband 2.7.9 monster race pictures in various places.
 */


/*
 * Helper function for "monster nest (jelly)"
 */
static bool_ vault_aux_jelly(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Also decline evil jellies (like death molds and shoggoths) */
	if (r_ptr->flags3 & (RF3_EVIL)) return (FALSE);

	/* Require icky thing, jelly, mold, or mushroom */
	if (!strchr("ijm,", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (animal)"
 */
static bool_ vault_aux_animal(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require "animal" flag */
	if (!(r_ptr->flags3 & (RF3_ANIMAL))) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (undead)"
 */
static bool_ vault_aux_undead(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require Undead */
	if (!(r_ptr->flags3 & (RF3_UNDEAD))) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (chapel)"
 */
static bool_ vault_aux_chapel(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require "priest" or Angel */
	if (!((r_ptr->d_char == 'A') ||
	                strstr((r_name + r_ptr->name), "riest")))
	{
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (kennel)"
 */
static bool_ vault_aux_kennel(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require a Zephyr Hound or a dog */
	return ((r_ptr->d_char == 'Z') || (r_ptr->d_char == 'C'));

}


/*
 * Helper function for "monster nest (treasure)"
 */
static bool_ vault_aux_treasure(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Require "priest" or Angel */
	if (!((r_ptr->d_char == '!') || (r_ptr->d_char == '|') ||
	                (r_ptr->d_char == '$') || (r_ptr->d_char == '?') ||
	                (r_ptr->d_char == '=')))
	{
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster nest (clone)"
 */
static bool_ vault_aux_clone(int r_idx)
{
	return (r_idx == template_race);
}


/*
 * Helper function for "monster nest (symbol clone)"
 */
static bool_ vault_aux_symbol(int r_idx)
{
	return ((r_info[r_idx].d_char == (r_info[template_race].d_char))
	        && !(r_info[r_idx].flags1 & RF1_UNIQUE));
}


/*
 * Helper function for "monster pit (orc)"
 */
static bool_ vault_aux_orc(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "o" monsters */
	if (!strchr("o", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}



/*
 * Helper function for "monster pit (troll)"
 */
static bool_ vault_aux_troll(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "T" monsters */
	if (!strchr("T", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (giant)"
 */
static bool_ vault_aux_giant(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "P" monsters */
	if (!strchr("P", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Hack -- breath type for "vault_aux_dragon()"
 */
static u32b vault_aux_dragon_mask4;


/*
 * Helper function for "monster pit (dragon)"
 */
static bool_ vault_aux_dragon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "d" or "D" monsters */
	if (!strchr("Dd", r_ptr->d_char)) return (FALSE);

	/* Hack -- Require correct "breath attack" */
	if (r_ptr->flags4 != vault_aux_dragon_mask4) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Helper function for "monster pit (demon)"
 */
static bool_ vault_aux_demon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Decline unique monsters */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Hack -- Require "U" monsters */
	if (!strchr("U", r_ptr->d_char)) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Type 5 -- Monster nests
 *
 * A monster nest is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type strewn about the room.
 *
 * The monsters are chosen from a set of 64 randomly selected monster
 * races, to allow the nest creation to fail instead of having "holes".
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the nest.
 *
 * Currently, a monster nest is one of
 *   a nest of "jelly" monsters   (Dungeon level 5 and deeper)
 *   a nest of "animal" monsters  (Dungeon level 30 and deeper)
 *   a nest of "undead" monsters  (Dungeon level 50 and deeper)
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which
 * case the nest will be empty, and will not affect the level rating.
 *
 * Note that "monster nests" will never contain "unique" monsters.
 */
static void build_type5(int by0, int bx0)
{
	int y, x, y1, x1, y2, x2, xval, yval;
	int tmp, i;
	cptr name;
	bool_ empty = FALSE;
	bool_ (*old_get_mon_num_hook)(int r_idx);
	s16b what[64];

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(25, 11, TRUE, by0, bx0, &xval, &yval)) return;

	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;

	/* Place the floor area */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= (CAVE_ROOM);
		}
	}

	/* Place the outer walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, CAVE_ROOM);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_inner, CAVE_ROOM);

	/* Place a secret door */
	switch (randint(4))
	{
	case 1:
		place_secret_door(y1 - 1, xval);
		break;
	case 2:
		place_secret_door(y2 + 1, xval);
		break;
	case 3:
		place_secret_door(yval, x1 - 1);
		break;
	case 4:
		place_secret_door(yval, x2 + 1);
		break;
	}

	/* Hack -- Choose a nest type */
	tmp = randint(dun_level);

	old_get_mon_num_hook = get_mon_num_hook;

	if ((tmp < 25) && (rand_int(2) != 0))
	{
		while (1)
		{
			template_race = randint(max_r_idx - 2);

			/* Reject uniques */
			if (r_info[template_race].flags1 & RF1_UNIQUE) continue;

			/* Reject OoD monsters in a loose fashion */
			if (((r_info[template_race].level) + randint(5)) >
			                (dun_level + randint(5))) continue;

			/* Don't like 'break's like this, but this cannot be made better */
			break;
		}

		if ((dun_level >= (25 + randint(15))) && (rand_int(2) != 0))
		{
			name = "symbol clone";
			get_mon_num_hook = vault_aux_symbol;
		}
		else
		{
			name = "clone";
			get_mon_num_hook = vault_aux_clone;
		}
	}
	else if (tmp < 25)
		/* Monster nest (jelly) */
	{
		/* Describe */
		name = "jelly";

		/* Restrict to jelly */
		get_mon_num_hook = vault_aux_jelly;
	}

	else if (tmp < 50)
	{
		name = "treasure";
		get_mon_num_hook = vault_aux_treasure;
	}

	/* Monster nest (animal) */
	else if (tmp < 65)
	{
		if (rand_int(3) == 0)
		{
			name = "kennel";
			get_mon_num_hook = vault_aux_kennel;
		}
		else
		{
			/* Describe */
			name = "animal";

			/* Restrict to animal */
			get_mon_num_hook = vault_aux_animal;
		}
	}

	/* Monster nest (undead) */
	else
	{
		if (rand_int(3) == 0)
		{
			name = "chapel";
			get_mon_num_hook = vault_aux_chapel;
		}
		else
		{
			/* Describe */
			name = "undead";

			/* Restrict to undead */
			get_mon_num_hook = vault_aux_undead;
		}
	}

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick some monster types */
	for (i = 0; i < 64; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(dun_level + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Remove restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Oops */
	if (empty) return;

	/* Describe */
	if (cheat_room || p_ptr->precognition)
	{
		/* Room type */
		msg_format("Monster nest (%s)", name);
	}

	/* Increase the level rating */
	rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Nests") */
	if ((dun_level <= 40) && (randint(dun_level * dun_level + 50) < 300))
	{
		good_item_flag = TRUE;
	}

	/* Place some monsters */
	for (y = yval - 2; y <= yval + 2; y++)
	{
		for (x = xval - 9; x <= xval + 9; x++)
		{
			int r_idx = what[rand_int(64)];

			/* Place that "random" monster (no groups) */
			(void)place_monster_aux(y, x, r_idx, FALSE, FALSE, MSTATUS_ENEMY);
		}
	}
}



/*
 * Type 6 -- Monster pits
 *
 * A monster pit is a "big" room, with an "inner" room, containing
 * a "collection" of monsters of a given type organized in the room.
 *
 * Monster types in the pit  (list out of date...)
 *   orc pit	(Dungeon Level 5 and deeper)
 *   troll pit	(Dungeon Level 20 and deeper)
 *   giant pit	(Dungeon Level 40 and deeper)
 *   dragon pit	(Dungeon Level 60 and deeper)
 *   demon pit	(Dungeon Level 80 and deeper)
 *
 * The inside room in a monster pit appears as shown below, where the
 * actual monsters in each location depend on the type of the pit
 *
 *   #####################
 *   #0000000000000000000#
 *   #0112233455543322110#
 *   #0112233467643322110#
 *   #0112233455543322110#
 *   #0000000000000000000#
 *   #####################
 *
 * Note that the monsters in the pit are now chosen by using "get_mon_num()"
 * to request 16 "appropriate" monsters, sorting them by level, and using
 * the "even" entries in this sorted list for the contents of the pit.
 *
 * Hack -- all of the "dragons" in a "dragon" pit must be the same "color",
 * which is handled by requiring a specific "breath" attack for all of the
 * dragons.  This may include "multi-hued" breath.  Note that "wyrms" may
 * be present in many of the dragon pits, if they have the proper breath.
 *
 * Note the use of the "get_mon_num_prep()" function, and the special
 * "get_mon_num_hook()" restriction function, to prepare the "monster
 * allocation table" in such a way as to optimize the selection of
 * "appropriate" non-unique monsters for the pit.
 *
 * Note that the "get_mon_num()" function may (rarely) fail, in which case
 * the pit will be empty, and will not effect the level rating.
 *
 * Note that "monster pits" will never contain "unique" monsters.
 */
static void build_type6(int by0, int bx0)
{
	int tmp, what[16];
	int i, j, y, x, y1, x1, y2, x2, xval, yval;
	bool_ empty = FALSE;
	cptr name;
	bool_ (*old_get_mon_num_hook)(int r_idx);

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(25, 11, TRUE, by0, bx0, &xval, &yval)) return;

	/* Large room */
	y1 = yval - 4;
	y2 = yval + 4;
	x1 = xval - 11;
	x2 = xval + 11;

	/* Place the floor area */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			place_floor(y, x);
			cave[y][x].info |= (CAVE_ROOM);
		}
	}

	/* Place the outer walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, CAVE_ROOM);

	/* Advance to the center room */
	y1 = y1 + 2;
	y2 = y2 - 2;
	x1 = x1 + 2;
	x2 = x2 - 2;

	/* The inner walls */
	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, CAVE_ROOM);

	/* Place a secret door */
	switch (randint(4))
	{
	case 1:
		place_secret_door(y1 - 1, xval);
		break;
	case 2:
		place_secret_door(y2 + 1, xval);
		break;
	case 3:
		place_secret_door(yval, x1 - 1);
		break;
	case 4:
		place_secret_door(yval, x2 + 1);
		break;
	}

	/* Choose a pit type */
	tmp = randint(dun_level);

	old_get_mon_num_hook = get_mon_num_hook;

	/* Orc pit */
	if (tmp < 20)
	{
		/* Message */
		name = "orc";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_orc;
	}

	/* Troll pit */
	else if (tmp < 40)
	{
		/* Message */
		name = "troll";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_troll;
	}

	/* Giant pit */
	else if (tmp < 55)
	{
		/* Message */
		name = "giant";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_giant;
	}

	else if (tmp < 70)
	{
		if (randint(4) != 1)
		{
			/* Message */
			name = "ordered clones";

			do
			{
				template_race = randint(max_r_idx - 2);
			}
			while ((r_info[template_race].flags1 & RF1_UNIQUE)
			                || (((r_info[template_race].level) + randint(5)) >
			                    (dun_level + randint(5))));

			/* Restrict selection */
			get_mon_num_hook = vault_aux_symbol;
		}
		else
		{
			name = "ordered chapel";
			get_mon_num_hook = vault_aux_chapel;
		}

	}

	/* Dragon pit */
	else if (tmp < 80)
	{
		/* Pick dragon type */
		switch (rand_int(6))
		{
			/* Black */
		case 0:
			{
				/* Message */
				name = "acid dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_ACID;

				/* Done */
				break;
			}

			/* Blue */
		case 1:
			{
				/* Message */
				name = "electric dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_ELEC;

				/* Done */
				break;
			}

			/* Red */
		case 2:
			{
				/* Message */
				name = "fire dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_FIRE;

				/* Done */
				break;
			}

			/* White */
		case 3:
			{
				/* Message */
				name = "cold dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_COLD;

				/* Done */
				break;
			}

			/* Green */
		case 4:
			{
				/* Message */
				name = "poison dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = RF4_BR_POIS;

				/* Done */
				break;
			}

			/* Multi-hued */
		default:
			{
				/* Message */
				name = "multi-hued dragon";

				/* Restrict dragon breath type */
				vault_aux_dragon_mask4 = (RF4_BR_ACID | RF4_BR_ELEC |
				                          RF4_BR_FIRE | RF4_BR_COLD |
				                          RF4_BR_POIS);

				/* Done */
				break;
			}

		}

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_dragon;
	}

	/* Demon pit */
	else
	{
		/* Message */
		name = "demon";

		/* Restrict monster selection */
		get_mon_num_hook = vault_aux_demon;
	}

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick some monster types */
	for (i = 0; i < 16; i++)
	{
		/* Get a (hard) monster type */
		what[i] = get_mon_num(dun_level + 10);

		/* Notice failure */
		if (!what[i]) empty = TRUE;
	}

	/* Remove restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Oops */
	if (empty) return;

	/* XXX XXX XXX */
	/* Sort the entries */
	for (i = 0; i < 16 - 1; i++)
	{
		/* Sort the entries */
		for (j = 0; j < 16 - 1; j++)
		{
			int i1 = j;
			int i2 = j + 1;

			int p1 = r_info[what[i1]].level;
			int p2 = r_info[what[i2]].level;

			/* Bubble */
			if (p1 > p2)
			{
				int tmp = what[i1];
				what[i1] = what[i2];
				what[i2] = tmp;
			}
		}
	}

	/* Select the entries */
	for (i = 0; i < 8; i++)
	{
		/* Every other entry */
		what[i] = what[i * 2];
	}

	/* Message */
	if (cheat_room || p_ptr->precognition)
	{
		/* Room type */
		msg_format("Monster pit (%s)", name);

		if (cheat_hear || p_ptr->precognition)
		{
			/* Contents */
			for (i = 0; i < 8; i++)
			{
				/* Message */
				msg_print(r_name + r_info[what[i]].name);
			}
		}
	}

	/* Increase the level rating */
	rating += 10;

	/* (Sometimes) Cause a "special feeling" (for "Monster Pits") */
	if ((dun_level <= 40) && (randint(dun_level * dun_level + 50) < 300))
	{
		good_item_flag = TRUE;
	}

	/* Top and bottom rows */
	for (x = xval - 9; x <= xval + 9; x++)
	{
		place_monster_aux(yval - 2, x, what[0], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(yval + 2, x, what[0], FALSE, FALSE, MSTATUS_ENEMY);
	}

	/* Middle columns */
	for (y = yval - 1; y <= yval + 1; y++)
	{
		place_monster_aux(y, xval - 9, what[0], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 9, what[0], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 8, what[1], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 8, what[1], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 7, what[1], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 7, what[1], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 6, what[2], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 6, what[2], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 5, what[2], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 5, what[2], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 4, what[3], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 4, what[3], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 3, what[3], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 3, what[3], FALSE, FALSE, MSTATUS_ENEMY);

		place_monster_aux(y, xval - 2, what[4], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(y, xval + 2, what[4], FALSE, FALSE, MSTATUS_ENEMY);
	}

	/* Above/Below the center monster */
	for (x = xval - 1; x <= xval + 1; x++)
	{
		place_monster_aux(yval + 1, x, what[5], FALSE, FALSE, MSTATUS_ENEMY);
		place_monster_aux(yval - 1, x, what[5], FALSE, FALSE, MSTATUS_ENEMY);
	}

	/* Next to the center monster */
	place_monster_aux(yval, xval + 1, what[6], FALSE, FALSE, MSTATUS_ENEMY);
	place_monster_aux(yval, xval - 1, what[6], FALSE, FALSE, MSTATUS_ENEMY);

	/* Center monster */
	place_monster_aux(yval, xval, what[7], FALSE, FALSE, MSTATUS_ENEMY);
}

/*
 * Hack -- fill in "vault" rooms
 */
static void build_vault(int yval, int xval, int ymax, int xmax, cptr data)
{
	int dx, dy, x, y, bwy[8], bwx[8], i;

	cptr t;

	cave_type *c_ptr;

	/* Clean the between gates arrays */
	for (i = 0; i < 8; i++)
	{
		bwy[i] = bwx[i] = 9999;
	}

	/* Place dungeon features and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the location */
			x = xval - (xmax / 2) + dx;
			y = yval - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Lay down a floor */
			place_floor(y, x);

			/* Part of a vault */
			c_ptr->info |= (CAVE_ROOM | CAVE_ICKY);

			/* Analyze the grid */
			switch (*t)
			{
				/* Granite wall (outer) */
			case '%':
				{
					cave_set_feat(y, x, FEAT_WALL_OUTER);
					break;
				}

				/* Granite wall (inner) */
			case '#':
				{
					cave_set_feat(y, x, FEAT_WALL_INNER);
					break;
				}

				/* Permanent wall (inner) */
			case 'X':
				{
					cave_set_feat(y, x, FEAT_PERM_INNER);
					break;
				}

				/* Treasure/trap */
			case '*':
				{
					if (rand_int(100) < 75)
					{
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_VAULT);
					}
					else
					{
						place_trap(y, x);
					}
					break;
				}

				/* Secret doors */
			case '+':
				{
					place_secret_door(y, x);
					break;
				}

				/* Trap */
			case '^':
				{
					place_trap(y, x);
					break;
				}

				/* Glass wall */
			case 'G':
				{
					cave_set_feat(y, x, FEAT_GLASS_WALL);
					break;
				}

				/* Illusion wall */
			case 'I':
				{
					cave_set_feat(y, x, FEAT_ILLUS_WALL);
					break;
				}
			}
		}
	}

	/* Place dungeon monsters and objects */
	for (t = data, dy = 0; dy < ymax; dy++)
	{
		for (dx = 0; dx < xmax; dx++, t++)
		{
			/* Extract the grid */
			x = xval - (xmax / 2) + dx;
			y = yval - (ymax / 2) + dy;

			/* Hack -- skip "non-grids" */
			if (*t == ' ') continue;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Analyze the symbol */
			switch (*t)
			{
				/* Monster */
			case '&':
				{
					monster_level = dun_level + 5;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					break;
				}

				/* Meaner monster */
			case '@':
				{
					monster_level = dun_level + 11;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					break;
				}

				/* Meaner monster, plus treasure */
			case '9':
				{
					monster_level = dun_level + 9;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					object_level = dun_level + 7;
					place_object(y, x, TRUE, FALSE, OBJ_FOUND_VAULT);
					object_level = dun_level;
					break;
				}

				/* Nasty monster and treasure */
			case '8':
				{
					monster_level = dun_level + 40;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					object_level = dun_level + 20;
					place_object(y, x, TRUE, TRUE, OBJ_FOUND_VAULT);
					object_level = dun_level;
					break;
				}

				/* Monster and/or object */
			case ',':
				{
					if (rand_int(100) < 50)
					{
						monster_level = dun_level + 3;
						place_monster(y, x, TRUE, TRUE);
						monster_level = dun_level;
					}
					if (rand_int(100) < 50)
					{
						object_level = dun_level + 7;
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_VAULT);
						object_level = dun_level;
					}
					break;
				}

			case 'p':
				{
					cave_set_feat(y, x, FEAT_PATTERN_START);
					break;
				}

			case 'a':
				{
					cave_set_feat(y, x, FEAT_PATTERN_1);
					break;
				}

			case 'b':
				{
					cave_set_feat(y, x, FEAT_PATTERN_2);
					break;
				}

			case 'c':
				{
					cave_set_feat(y, x, FEAT_PATTERN_3);
					break;
				}

			case 'd':
				{
					cave_set_feat(y, x, FEAT_PATTERN_4);
					break;
				}

			case 'P':
				{
					cave_set_feat(y, x, FEAT_PATTERN_END);
					break;
				}

			case 'B':
				{
					cave_set_feat(y, x, FEAT_PATTERN_XTRA1);
					break;
				}

			case 'A':
				{
					object_level = dun_level + 12;
					place_object(y, x, TRUE, FALSE, OBJ_FOUND_VAULT);
					object_level = dun_level;
					break;
				}


				/* Between gates */
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				{
					/* Not found before */
					if (bwy[*t - '0'] == 9999)
					{
						cave_set_feat(y, x, FEAT_BETWEEN);
						bwy[*t - '0'] = y;
						bwx[*t - '0'] = x;
					}
					/* The second time */
					else
					{
						cave_set_feat(y, x, FEAT_BETWEEN);
						c_ptr->special = bwx[*t - '0'] + (bwy[*t - '0'] << 8);
						cave[bwy[*t - '0']][bwx[*t - '0']].special = x + (y << 8);
					}
					break;
				}
			}
		}
	}
}

/*
 * Type 7 -- simple vaults (see "v_info.txt")
 */
static void build_type7(int by0, int bx0)
{
	vault_type *v_ptr = NULL;
	int dummy = 0, xval, yval;

	/* Pick a lesser vault */
	while (dummy < SAFE_MAX_ATTEMPTS)
	{
		dummy++;

		/* Access a random vault record */
		v_ptr = &v_info[rand_int(max_v_idx)];

		/* Accept the first lesser vault */
		if (v_ptr->typ == 7) break;
	}

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(v_ptr->wid, v_ptr->hgt, FALSE, by0, bx0, &xval, &yval))
	{
		if (cheat_room) msg_print("Could not allocate this vault here");
		return;
	}

	if (dummy >= SAFE_MAX_ATTEMPTS)
	{
		if (cheat_room)
		{
			msg_print("Warning! Could not place lesser vault!");
		}
		return;
	}


	/* Message */
	if (cheat_room || p_ptr->precognition) msg_print("Lesser Vault");

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((dun_level <= 50) ||
	                (randint((dun_level - 40) * (dun_level - 40) + 50) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(yval, xval, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}



/*
 * Type 8 -- greater vaults (see "v_info.txt")
 */
static void build_type8(int by0, int bx0)
{
	vault_type *v_ptr = NULL;
	int dummy = 0, xval, yval;

	/* Pick a lesser vault */
	while (dummy < SAFE_MAX_ATTEMPTS)
	{
		dummy++;

		/* Access a random vault record */
		v_ptr = &v_info[rand_int(max_v_idx)];

		/* Accept the first greater vault */
		if (v_ptr->typ == 8) break;
	}

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(v_ptr->wid, v_ptr->hgt, FALSE, by0, bx0, &xval, &yval))
	{
		if (cheat_room) msg_print("Could not allocate this vault here");
		return;
	}

	if (dummy >= SAFE_MAX_ATTEMPTS)
	{
		if (cheat_room)
		{
			msg_print("Warning! Could not place greater vault!");
		}
		return;
	}


	/* Message */
	if (cheat_room || p_ptr->precognition) msg_print("Greater Vault");

	/* Boost the rating */
	rating += v_ptr->rat;

	/* (Sometimes) Cause a special feeling */
	if ((dun_level <= 50) ||
	                (randint((dun_level - 40) * (dun_level - 40) + 50) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Hack -- Build the vault */
	build_vault(yval, xval, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
}

/*
 * DAG:
 * Build an vertical oval room.
 * For every grid in the possible square, check the distance.
 * If it's less than or == than the radius, make it a room square.
 * If its less, make it a normal grid. If it's == make it an outer
 * wall.
 */
static void build_type9(int by0, int bx0)
{
	u16b info;
	int rad, x, y, x0, y0;

	rad = 2 + rand_int(8);

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(rad*2 + 1, rad*2 + 1, FALSE, by0, bx0, &x0, &y0)) return;

	info = (randint(dun_level) <= 5) ? (CAVE_ROOM|CAVE_GLOW) : CAVE_ROOM;

	for (x = x0 - rad; x <= x0 + rad; x++)
	{
		for (y = y0 - rad; y <= y0 + rad; y++)
		{
			if (distance(y0, x0, y, x) == rad)
			{
				cave[y][x].info |= info;
				cave_set_feat(y, x, feat_wall_outer);
			}

			if (distance(y0, x0, y, x) < rad)
			{
				cave[y][x].info |= info;
				place_floor(y, x);
			}
		}
	}
}


/*
 * Store routine for the fractal cave generator
 * this routine probably should be an inline function or a macro
 */
static void store_height(int x, int y, int x0, int y0, byte val,
                         int xhsize, int yhsize, int cutoff)
{
	/* Only write to points that are "blank" */
	if (cave[y + y0 - yhsize][x + x0 - xhsize].feat != 255) return;

	/* If on boundary set val > cutoff so walls are not as square */
	if (((x == 0) || (y == 0) || (x == xhsize * 2) || (y == yhsize * 2)) &&
	                (val <= cutoff)) val = cutoff + 1;

	/* Store the value in height-map format */
	/* Meant to be temporary, hence no cave_set_feat */
	cave[y + y0 - yhsize][x + x0 - xhsize].feat = val;

	return;
}



/*
 * Explanation of the plasma fractal algorithm:
 *
 * A grid of points is created with the properties of a 'height-map'
 * This is done by making the corners of the grid have a random value.
 * The grid is then subdivided into one with twice the resolution.
 * The new points midway between two 'known' points can be calculated
 * by taking the average value of the 'known' ones and randomly adding
 * or subtracting an amount proportional to the distance between those
 * points.  The final 'middle' points of the grid are then calculated
 * by averaging all four of the originally 'known' corner points.  An
 * random amount is added or subtracted from this to get a value of the
 * height at that point.  The scaling factor here is adjusted to the
 * slightly larger distance diagonally as compared to orthogonally.
 *
 * This is then repeated recursively to fill an entire 'height-map'
 * A rectangular map is done the same way, except there are different
 * scaling factors along the x and y directions.
 *
 * A hack to change the amount of correlation between points is done using
 * the grd variable.  If the current step size is greater than grd then
 * the point will be random, otherwise it will be calculated by the
 * above algorithm.  This makes a maximum distance at which two points on
 * the height map can affect each other.
 *
 * How fractal caves are made:
 *
 * When the map is complete, a cut-off value is used to create a cave.
 * Heights below this value are "floor", and heights above are "wall".
 * This also can be used to create lakes, by adding more height levels
 * representing shallow and deep water/ lava etc.
 *
 * The grd variable affects the width of passages.
 * The roug variable affects the roughness of those passages
 *
 * The tricky part is making sure the created cave is connected.  This
 * is done by 'filling' from the inside and only keeping the 'filled'
 * floor.  Walls bounding the 'filled' floor are also kept.  Everything
 * else is converted to the normal granite FEAT_WALL_EXTRA.
 */


/*
 * Note that this uses the cave.feat array in a very hackish way
 * the values are first set to zero, and then each array location
 * is used as a "heightmap"
 * The heightmap then needs to be converted back into the "feat" format.
 *
 * grd=level at which fractal turns on.  smaller gives more mazelike caves
 * roug=roughness level.  16=normal.  higher values make things more
 * convoluted small values are good for smooth walls.
 * size=length of the side of the square cave system.
 */

void generate_hmap(int y0, int x0, int xsiz, int ysiz, int grd,
                   int roug, int cutoff)
{
	int xhsize, yhsize, xsize, ysize, maxsize;

	/*
	 * fixed point variables- these are stored as 256 x normal value
	 * this gives 8 binary places of fractional part + 8 places of normal part
	 */
	u16b xstep, xhstep, ystep, yhstep, i, j, diagsize, xxsize, yysize;


	/* Redefine size so can change the value if out of range */
	xsize = xsiz;
	ysize = ysiz;

	/* Paranoia about size of the system of caves*/
	if (xsize > 254) xsize = 254;
	if (xsize < 4) xsize = 4;
	if (ysize > 254) ysize = 254;
	if (ysize < 4) ysize = 4;

	/* Get offsets to middle of array */
	xhsize = xsize / 2;
	yhsize = ysize / 2;

	/* Fix rounding problem */
	xsize = xhsize * 2;
	ysize = yhsize * 2;

	/*
	 * Scale factor for middle points:
	 * About sqrt(2)*256 - correct for a square lattice
	 * approximately correct for everything else.
	 */
	diagsize = 362;

	/* Maximum of xsize and ysize */
	maxsize = (xsize > ysize) ? xsize : ysize;

	/* Clear the section */
	for (i = 0; i <= xsize; i++)
	{
		for (j = 0; j <= ysize; j++)
		{
			cave_type *c_ptr;

			/* Access the grid */
			c_ptr = &cave[j + y0 - yhsize][i + x0 - xhsize];

			/* 255 is a flag for "not done yet" */
			c_ptr->feat = 255;

			/* Clear icky flag because may be redoing the cave */
			c_ptr->info &= ~(CAVE_ICKY);
		}
	}

	/* Set the corner values just in case grd>size. */
	store_height(0, 0, x0, y0, maxsize, xhsize, yhsize, cutoff);
	store_height(0, ysize, x0, y0, maxsize, xhsize, yhsize, cutoff);
	store_height(xsize, 0, x0, y0, maxsize, xhsize, yhsize, cutoff);
	store_height(xsize, ysize, x0, y0, maxsize, xhsize, yhsize, cutoff);

	/* Set the middle square to be an open area. */
	store_height(xhsize, yhsize, x0, y0, 0, xhsize, yhsize, cutoff);


	/* Initialise the step sizes */
	xstep = xhstep = xsize * 256;
	ystep = yhstep = ysize * 256;
	xxsize = xsize * 256;
	yysize = ysize * 256;

	/*
	 * Fill in the rectangle with fractal height data - like the
	 * 'plasma fractal' in fractint
	 */
	while ((xstep / 256 > 1) || (ystep / 256 > 1))
	{
		/* Halve the step sizes */
		xstep = xhstep;
		xhstep /= 2;
		ystep = yhstep;
		yhstep /= 2;

		/* Middle top to bottom */
		for (i = xhstep; i <= xxsize - xhstep; i += xstep)
		{
			for (j = 0; j <= yysize; j += ystep)
			{
				/* If greater than 'grid' level then is random */
				if (xhstep / 256 > grd)
				{
					store_height(i / 256, j / 256, x0, y0, randint(maxsize),
					             xhsize, yhsize, cutoff);
				}
				else
				{
					cave_type *l, *r;
					byte val;

					/* Left point */
					l = &cave[j / 256 + y0 - yhsize][(i - xhstep) / 256 + x0 - xhsize];

					/* Right point */
					r = &cave[j / 256 + y0 - yhsize][(i + xhstep) / 256 + x0 - xhsize];

					/* Average of left and right points + random bit */
					val = (l->feat + r->feat) / 2 +
					      (randint(xstep / 256) - xhstep / 256) * roug / 16;

					store_height(i / 256, j / 256, x0, y0, val,
					             xhsize, yhsize, cutoff);
				}
			}
		}


		/* Middle left to right */
		for (j = yhstep; j <= yysize - yhstep; j += ystep)
		{
			for (i = 0; i <= xxsize; i += xstep)
			{
				/* If greater than 'grid' level then is random */
				if (xhstep / 256 > grd)
				{
					store_height(i / 256, j / 256, x0, y0, randint(maxsize),
					             xhsize, yhsize, cutoff);
				}
				else
				{
					cave_type *u, *d;
					byte val;

					/* Up point */
					u = &cave[(j - yhstep) / 256 + y0 - yhsize][i / 256 + x0 - xhsize];

					/* Down point */
					d = &cave[(j + yhstep) / 256 + y0 - yhsize][i / 256 + x0 - xhsize];

					/* Average of up and down points + random bit */
					val = (u->feat + d->feat) / 2 +
					      (randint(ystep / 256) - yhstep / 256) * roug / 16;

					store_height(i / 256, j / 256, x0, y0, val,
					             xhsize, yhsize, cutoff);
				}
			}
		}

		/* Center */
		for (i = xhstep; i <= xxsize - xhstep; i += xstep)
		{
			for (j = yhstep; j <= yysize - yhstep; j += ystep)
			{
				/* If greater than 'grid' level then is random */
				if (xhstep / 256 > grd)
				{
					store_height(i / 256, j / 256, x0, y0, randint(maxsize),
					             xhsize, yhsize, cutoff);
				}
				else
				{
					cave_type *ul, *dl, *ur, *dr;
					byte val;

					/* Up-left point */
					ul = &cave[(j - yhstep) / 256 + y0 - yhsize][(i - xhstep) / 256 + x0 - xhsize];

					/* Down-left point */
					dl = &cave[(j + yhstep) / 256 + y0 - yhsize][(i - xhstep) / 256 + x0 - xhsize];

					/* Up-right point */
					ur = &cave[(j - yhstep) / 256 + y0 - yhsize][(i + xhstep) / 256 + x0 - xhsize];

					/* Down-right point */
					dr = &cave[(j + yhstep) / 256 + y0 - yhsize][(i + xhstep) / 256 + x0 - xhsize];

					/*
					 * average over all four corners + scale by diagsize to
					 * reduce the effect of the square grid on the shape
					 * of the fractal
					 */
					val = (ul->feat + dl->feat + ur->feat + dr->feat) / 4 +
					      (randint(xstep / 256) - xhstep / 256) *
					      (diagsize / 16) / 256 * roug;

					store_height(i / 256, j / 256, x0, y0, val,
					             xhsize, yhsize , cutoff);
				}
			}
		}
	}
}


/*
 * Convert from height-map back to the normal Angband cave format
 */
static bool_ hack_isnt_wall(int y, int x, int cutoff)
{
	/* Already done */
	if (cave[y][x].info & CAVE_ICKY)
	{
		return (FALSE);
	}

	else
	{
		/* Show that have looked at this square */
		cave[y][x].info |= (CAVE_ICKY);

		/* If less than cutoff then is a floor */
		if (cave[y][x].feat <= cutoff)
		{
			place_floor(y, x);
			return (TRUE);
		}

		/* If greater than cutoff then is a wall */
		else
		{
			cave_set_feat(y, x, feat_wall_outer);
			return (FALSE);
		}
	}
}


/*
 * Quick and nasty fill routine used to find the connected region
 * of floor in the middle of the cave
 */
static void fill_hack(int y0, int x0, int y, int x, int xsize, int ysize,
                      int cutoff, int *amount)
{
	int i, j;

	/* check 8 neighbours +self (self is caught in the isnt_wall function) */
	for (i = -1; i <= 1; i++)
	{
		for (j = -1; j <= 1; j++)
		{
			/* If within bounds */
			if ((x + i > 0) && (x + i < xsize) &&
			                (y + j > 0) && (y + j < ysize))
			{
				/* If not a wall or floor done before */
				if (hack_isnt_wall(y + j + y0 - ysize / 2,
				                   x + i + x0 - xsize / 2, cutoff))
				{
					/* then fill from the new point*/
					fill_hack(y0, x0, y + j, x + i, xsize, ysize,
					          cutoff, amount);

					/* keep tally of size of cave system */
					(*amount)++;
				}
			}

			/* Affect boundary */
			else
			{
				cave[y0 + y + j - ysize / 2][x0 + x + i - xsize / 2].info |= (CAVE_ICKY);
			}
		}
	}
}


bool_ generate_fracave(int y0, int x0, int xsize, int ysize,
                      int cutoff, bool_ light, bool_ room)
{
	int x, y, i, amount, xhsize, yhsize;
	cave_type *c_ptr;

	/* Offsets to middle from corner */
	xhsize = xsize / 2;
	yhsize = ysize / 2;

	/* Reset tally */
	amount = 0;

	/*
	 * Select region connected to center of cave system
	 * this gets rid of alot of isolated one-sqaures that
	 * can make teleport traps instadeaths...
	 */
	fill_hack(y0, x0, yhsize, xhsize, xsize, ysize, cutoff, &amount);

	/* If tally too small, try again */
	if (amount < 10)
	{
		/* Too small -- clear area and try again later */
		for (x = 0; x <= xsize; ++x)
		{
			for (y = 0; y < ysize; ++y)
			{
				place_filler(y0 + y - yhsize, x0 + x - xhsize);
				cave[y0 + y - yhsize][x0 + x - xhsize].info &= ~(CAVE_ICKY | CAVE_ROOM);
			}
		}
		return FALSE;
	}


	/*
	 * Do boundaries -- check to see if they are next to a filled region
	 * If not then they are set to normal granite
	 * If so then they are marked as room walls
	 */
	for (i = 0; i <= xsize; ++i)
	{
		/* Access top boundary grid */
		c_ptr = &cave[0 + y0 - yhsize][i + x0 - xhsize];

		/* Next to a 'filled' region? -- set to be room walls */
		if (c_ptr->info & CAVE_ICKY)
		{
			cave_set_feat(0 + y0 - yhsize, i + x0 - xhsize, feat_wall_outer);

			if (light) c_ptr->info |= (CAVE_GLOW);
			if (room)
			{
				c_ptr->info |= (CAVE_ROOM);
			}
			else
			{
				place_filler(0 + y0 - yhsize, i + x0 - xhsize);
			}
		}

		/* Outside of the room -- set to be normal granite */
		else
		{
			place_filler(0 + y0 - yhsize, i + x0 - xhsize);
		}

		/* Clear the icky flag -- don't need it any more */
		c_ptr->info &= ~(CAVE_ICKY);


		/* Access bottom boundary grid */
		c_ptr = &cave[ysize + y0 - yhsize][i + x0 - xhsize];

		/* Next to a 'filled' region? -- set to be room walls */
		if (c_ptr->info & CAVE_ICKY)
		{
			cave_set_feat(ysize + y0 - yhsize, i + x0 - xhsize, feat_wall_outer);
			if (light) c_ptr->info |= (CAVE_GLOW);
			if (room)
			{
				c_ptr->info |= (CAVE_ROOM);
			}
			else
			{
				place_filler(ysize + y0 - yhsize, i + x0 - xhsize);
			}
		}

		/* Outside of the room -- set to be normal granite */
		else
		{
			place_filler(ysize + y0 - yhsize, i + x0 - xhsize);
		}

		/* Clear the icky flag -- don't need it any more */
		c_ptr->info &= ~(CAVE_ICKY);
	}


	/* Do the left and right boundaries minus the corners (done above) */
	for (i = 1; i < ysize; ++i)
	{
		/* Access left boundary grid */
		c_ptr = &cave[i + y0 - yhsize][0 + x0 - xhsize];

		/* Next to a 'filled' region? -- set to be room walls */
		if (c_ptr->info & CAVE_ICKY)
		{
			cave_set_feat(i + y0 - yhsize, 0 + x0 - xhsize, feat_wall_outer);
			if (light) c_ptr->info |= (CAVE_GLOW);
			if (room)
			{
				c_ptr->info |= (CAVE_ROOM);
			}
			else
			{
				place_filler(i + y0 - yhsize, 0 + x0 - xhsize);
			}
		}

		/* Outside of the room -- set to be normal granite */
		else
		{
			place_filler(i + y0 - yhsize, 0 + x0 - xhsize);
		}

		/* Clear the icky flag -- don't need it any more */
		c_ptr->info &= ~(CAVE_ICKY);


		/* Access left boundary grid */
		c_ptr = &cave[i + y0 - yhsize][xsize + x0 - xhsize];

		/* Next to a 'filled' region? -- set to be room walls */
		if (c_ptr->info & CAVE_ICKY)
		{
			cave_set_feat(i + y0 - yhsize, xsize + x0 - xhsize, feat_wall_outer);
			if (light) c_ptr->info |= (CAVE_GLOW);
			if (room)
			{
				c_ptr->info |= (CAVE_ROOM);
			}
			else
			{
				place_filler(i + y0 - yhsize, xsize + x0 - xhsize);
			}
		}

		/* Outside of the room -- set to be normal granite */
		else
		{
			place_filler(i + y0 - yhsize, xsize + x0 - xhsize);
		}

		/* Clear the icky flag -- don't need it any more */
		c_ptr->info &= ~(CAVE_ICKY);
	}


	/*
	 * Do the rest: convert back to the normal format
	 * In other variants, may want to check to see if cave.feat< some value
	 * if so, set to be water:- this will make interesting pools etc.
	 * (I don't do this for standard Angband.)
	 */
	for (x = 1; x < xsize; ++x)
	{
		for (y = 1; y < ysize; ++y)
		{
			/* Access the grid */
			c_ptr = &cave[y + y0 - yhsize][x + x0 - xhsize];

			/* A floor grid to be converted */
			if ((f_info[c_ptr->feat].flags1 & FF1_FLOOR) &&
			                (c_ptr->info & CAVE_ICKY))

			{
				/* Clear the icky flag in the filled region */
				c_ptr->info &= ~(CAVE_ICKY);

				/* Set appropriate flags */
				if (light) c_ptr->info |= (CAVE_GLOW);
				if (room) c_ptr->info |= (CAVE_ROOM);
			}

			/* A wall grid to be convereted */
			else if ((c_ptr->feat == feat_wall_outer) &&
			                (c_ptr->info & CAVE_ICKY))
			{
				/* Clear the icky flag in the filled region */
				c_ptr->info &= ~(CAVE_ICKY);

				/* Set appropriate flags */
				if (light) c_ptr->info |= (CAVE_GLOW);
				if (room)
				{
					c_ptr->info |= (CAVE_ROOM);
				}
				else
				{
					place_filler(y + y0 - yhsize, x + x0 - xhsize);
				}
			}

			/* None of the above -- clear the unconnected regions */
			else
			{
				place_filler(y + y0 - yhsize, x + x0 - xhsize);
				c_ptr->info &= ~(CAVE_ICKY | CAVE_ROOM);
			}
		}
	}

	/*
	 * XXX XXX XXX There is a slight problem when tunnels pierce the caves:
	 * Extra doors appear inside the system.  (Its not very noticeable though.)
	 * This can be removed by "filling" from the outside in.  This allows
	 * a separation from FEAT_WALL_OUTER with FEAT_WALL_INNER.  (Internal
	 * walls are  F.W.OUTER instead.)
	 * The extra effort for what seems to be only a minor thing (even
	 * non-existant if you think of the caves not as normal rooms, but as
	 * holes in the dungeon), doesn't seem worth it.
	 */

	return (TRUE);
}


/*
 * Makes a cave system in the center of the dungeon
 */
static void build_cavern(void)
{
	int grd, roug, cutoff, xsize, ysize, x0, y0;
	bool_ done, light, room;

	light = done = room = FALSE;
	if (dun_level <= randint(25)) light = TRUE;

	/* Make a cave the size of the dungeon */
	xsize = cur_wid - 1;
	ysize = cur_hgt - 1;
	x0 = xsize / 2;
	y0 = ysize / 2;

	/* Paranoia: make size even */
	xsize = x0 * 2;
	ysize = y0 * 2;

	while (!done)
	{
		/* Testing values for these parameters: feel free to adjust */
		grd = 1 << (randint(4) + 4);

		/* Want average of about 16 */
		roug = randint(8) * randint(4);

		/* About size/2 */
		cutoff = xsize / 2;

		/* Make it */
		generate_hmap(y0, x0, xsize, ysize, grd, roug, cutoff);

		/* Convert to normal format+ clean up*/
		done = generate_fracave(y0, x0, xsize, ysize, cutoff, light, room);
	}
}

/*
 * Driver routine to create fractal cave system
 */
static void build_type10(int by0, int bx0)
{
	int grd, roug, cutoff, xsize, ysize, y0, x0;

	bool_ done, light, room;

	/* Get size: note 'Evenness'*/
	xsize = randint(22) * 2 + 6;
	ysize = randint(15) * 2 + 6;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 1, ysize + 1, FALSE, by0, bx0, &x0, &y0)) return;

	light = done = FALSE;
	room = TRUE;

	if (dun_level <= randint(25)) light = TRUE;

	while (!done)
	{
		/*
		 * Note: size must be even or there are rounding problems
		 * This causes the tunnels not to connect properly to the room
		 */

		/* Testing values for these parameters feel free to adjust */
		grd = 1 << (randint(4));

		/* Want average of about 16 */
		roug = randint(8) * randint(4);

		/* About size/2 */
		cutoff = randint(xsize / 4) + randint(ysize / 4) +
		         randint(xsize / 4) + randint(ysize / 4);

		/* Make it */
		generate_hmap(y0, x0, xsize, ysize, grd, roug, cutoff);

		/* Convert to normal format + clean up*/
		done = generate_fracave(y0, x0, xsize, ysize, cutoff, light, room);
	}
}


/*
 * Random vault generation from Z 2.5.1
 */

/*
 * Make a very small room centred at (x0, y0)
 *
 * This is used in crypts, and random elemental vaults.
 *
 * Note - this should be used only on allocated regions
 * within another room.
 */
static void build_small_room(int x0, int y0)
{
	build_rectangle(y0 - 1, x0 - 1, y0 + 1, x0 + 1, feat_wall_inner, CAVE_ROOM);

	/* Place a secret door on one side */
	switch (rand_int(4))
	{
	case 0:
		{
			place_secret_door(y0, x0 - 1);
			break;
		}

	case 1:
		{
			place_secret_door(y0, x0 + 1);
			break;
		}

	case 2:
		{
			place_secret_door(y0 - 1, x0);
			break;
		}

	case 3:
		{
			place_secret_door(y0 + 1, x0);
			break;
		}
	}

	/* Add inner open space */
	place_floor(y0, x0);
}


/*
 * Add a door to a location in a random vault
 *
 * Note that range checking has to be done in the calling routine.
 *
 * The doors must be INSIDE the allocated region.
 */
static void add_door(int x, int y)
{
	/* Need to have a wall in the center square */
	if (cave[y][x].feat != feat_wall_outer) return;

	/*
	 * Look at:
	 *  x#x
	 *  .#.
	 *  x#x
	 *
	 *  where x=don't care
	 *  .=floor, #=wall
	 */

	if (get_is_floor(x, y - 1) && get_is_floor(x, y + 1) &&
	                (cave[y][x - 1].feat == feat_wall_outer) &&
	                (cave[y][x + 1].feat == feat_wall_outer))
	{
		/* secret door */
		place_secret_door(y, x);

		/* set boundarys so don't get wide doors */
		place_filler(y, x - 1);
		place_filler(y, x + 1);
	}


	/*
	 * Look at:
	 *  x#x
	 *  .#.
	 *  x#x
	 *
	 *  where x = don't care
	 *  .=floor, #=wall
	 */
	if ((cave[y - 1][x].feat == feat_wall_outer) &&
	                (cave[y + 1][x].feat == feat_wall_outer) &&
	                get_is_floor(x - 1, y) && get_is_floor(x + 1, y))
	{
		/* secret door */
		place_secret_door(y, x);

		/* set boundarys so don't get wide doors */
		place_filler(y - 1, x);
		place_filler(y + 1, x);
	}
}


/*
 * Fill the empty areas of a room with treasure and monsters.
 */
static void fill_treasure(int x1, int x2, int y1, int y2, int difficulty)
{
	int x, y, cx, cy, size;
	s32b value;

	/* center of room:*/
	cx = (x1 + x2) / 2;
	cy = (y1 + y2) / 2;

	/* Rough measure of size of vault= sum of lengths of sides */
	size = abs(x2 - x1) + abs(y2 - y1);

	for (x = x1; x <= x2; x++)
	{
		for (y = y1; y <= y2; y++)
		{
			/*
			 * Thing added based on distance to center of vault
			 * Difficulty is 1-easy to 10-hard
			 */
			value = (((s32b)distance(cx, cy, x, y) * 100) / size) +
			        randint(10) - difficulty;

			/* Hack -- Empty square part of the time */
			if ((randint(100) - difficulty * 3) > 50) value = 20;

			/* If floor, shallow water or lava */
			if (get_is_floor(x, y) ||
			                (cave[y][x].feat == FEAT_SHAL_WATER) ||
			                (cave[y][x].feat == FEAT_SHAL_LAVA))
			{
				/* The smaller 'value' is, the better the stuff */
				if (value < 0)
				{
					/* Meanest monster + treasure */
					monster_level = dun_level + 40;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					object_level = dun_level + 20;
					place_object(y, x, TRUE, FALSE, OBJ_FOUND_FLOOR);
					object_level = dun_level;
				}
				else if (value < 5)
				{
					/* Mean monster +treasure */
					monster_level = dun_level + 20;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					object_level = dun_level + 10;
					place_object(y, x, TRUE, FALSE, OBJ_FOUND_FLOOR);
					object_level = dun_level;
				}
				else if (value < 10)
				{
					/* Monster */
					monster_level = dun_level + 9;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
				}
				else if (value < 17)
				{
					/* Intentional Blank space */

					/*
					 * (Want some of the vault to be empty
					 * so have room for group monsters.
					 * This is used in the hack above to lower
					 * the density of stuff in the vault.)
					 */
				}
				else if (value < 23)
				{
					/* Object or trap */
					if (rand_int(100) < 25)
					{
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_FLOOR);
					}
					else
					{
						place_trap(y, x);
					}
				}
				else if (value < 30)
				{
					/* Monster and trap */
					monster_level = dun_level + 5;
					place_monster(y, x, TRUE, TRUE);
					monster_level = dun_level;
					place_trap(y, x);
				}
				else if (value < 40)
				{
					/* Monster or object */
					if (rand_int(100) < 50)
					{
						monster_level = dun_level + 3;
						place_monster(y, x, TRUE, TRUE);
						monster_level = dun_level;
					}
					if (rand_int(100) < 50)
					{
						object_level = dun_level + 7;
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_FLOOR);
						object_level = dun_level;
					}
				}
				else if (value < 50)
				{
					/* Trap */
					place_trap(y, x);
				}
				else
				{
					/* Various Stuff */

					/* 20% monster, 40% trap, 20% object, 20% blank space */
					if (rand_int(100) < 20)
					{
						place_monster(y, x, TRUE, TRUE);
					}
					else if (rand_int(100) < 50)
					{
						place_trap(y, x);
					}
					else if (rand_int(100) < 50)
					{
						place_object(y, x, FALSE, FALSE, OBJ_FOUND_FLOOR);
					}
				}

			}
		}
	}
}


/*
 * Creates a random vault that looks like a collection of bubbles
 *
 * It works by getting a set of coordinates that represent the center of
 * each bubble.  The entire room is made by seeing which bubble center is
 * closest. If two centers are equidistant then the square is a wall,
 * otherwise it is a floor. The only exception is for squares really
 * near a center, these are always floor.
 * (It looks better than without this check.)
 *
 * Note: If two centers are on the same point then this algorithm will create a
 * blank bubble filled with walls. - This is prevented from happening.
 */

#define BUBBLENUM 10 /* number of bubbles */

static void build_bubble_vault(int x0, int y0, int xsize, int ysize)
{
	/* array of center points of bubbles */
	coord center[BUBBLENUM];

	int i, j, k, x = 0, y = 0;
	u16b min1, min2, temp;
	bool_ done;

	/* Offset from center to top left hand corner */
	int xhsize = xsize / 2;
	int yhsize = ysize / 2;

	if (cheat_room) msg_print("Bubble Vault");

	/* Allocate center of bubbles */
	center[0].x = randint(xsize - 3) + 1;
	center[0].y = randint(ysize - 3) + 1;

	for (i = 1; i < BUBBLENUM; i++)
	{
		done = FALSE;

		/* Get center and check to see if it is unique */
		for (k = 0; !done && (k < 2000); k++)
		{
			done = TRUE;

			x = randint(xsize - 3) + 1;
			y = randint(ysize - 3) + 1;

			for (j = 0; j < i; j++)
			{
				/* Rough test to see if there is an overlap */
				if ((x == center[j].x) || (y == center[j].y)) done = FALSE;
			}
		}

		/* Too many failures */
		if (k >= 2000) return;

		center[i].x = x;
		center[i].y = y;
	}

	build_rectangle(y0 - yhsize, x0 - xhsize,
	                y0 - yhsize + ysize - 1, x0 - xhsize + xsize - 1,
	                feat_wall_outer, CAVE_ROOM | CAVE_ICKY);

	/* Fill in middle with bubbles */
	for (x = 1; x < xsize - 1; x++)
	{
		for (y = 1; y < ysize - 1; y++)
		{
			cave_type *c_ptr;

			/* Get distances to two closest centers */

			/* Initialise */
			min1 = distance(x, y, center[0].x, center[0].y);
			min2 = distance(x, y, center[1].x, center[1].y);

			if (min1 > min2)
			{
				/* Swap if in wrong order */
				temp = min1;
				min1 = min2;
				min2 = temp;
			}

			/* Scan the rest */
			for (i = 2; i < BUBBLENUM; i++)
			{
				temp = distance(x, y, center[i].x, center[i].y);

				if (temp < min1)
				{
					/* Smallest */
					min2 = min1;
					min1 = temp;
				}
				else if (temp < min2)
				{
					/* Second smallest */
					min2 = temp;
				}
			}

			/* Access the grid */
			c_ptr = &cave[y + y0 - yhsize][x + x0 - xhsize];

			/*
			 * Boundary at midpoint+ not at inner region of bubble
			 *
			 * SCSCSC: was feat_wall_outer
			 */
			if (((min2 - min1) <= 2) && (!(min1 < 3)))
			{
				place_filler(y + y0 - yhsize, x + x0 - xhsize);
			}

			/* Middle of a bubble */
			else
			{
				place_floor(y + y0 - yhsize, x + x0 - xhsize);
			}

			/* Clean up rest of flags */
			c_ptr->info |= (CAVE_ROOM | CAVE_ICKY);
		}
	}

	/* Try to add some random doors */
	for (i = 0; i < 500; i++)
	{
		x = randint(xsize - 3) - xhsize + x0 + 1;
		y = randint(ysize - 3) - yhsize + y0 + 1;
		add_door(x, y);
	}

	/* Fill with monsters and treasure, low difficulty */
	fill_treasure(x0 - xhsize + 1, x0 - xhsize + xsize - 2,
	              y0 - yhsize + 1, y0 - yhsize + ysize - 2, randint(5));
}


/*
 * Convert FEAT_WALL_EXTRA (used by random vaults) to normal dungeon wall
 */
static void convert_extra(int y1, int x1, int y2, int x2)
{
	int x, y;

	for (x = x1; x <= x2; x++)
	{
		for (y = y1; y <= y2; y++)
		{
			if (cave[y][x].feat == FEAT_WALL_OUTER)
			{
				place_filler(y, x);
			}
		}
	}
}


/*
 * Overlay a rectangular room given its bounds
 *
 * This routine is used by build_room_vault (hence FEAT_WALL_OUTER)
 * The area inside the walls is not touched: only granite is removed
 * and normal walls stay
 */
static void build_room(int x1, int x2, int y1, int y2)
{
	int x, y, xsize, ysize, temp;

	/* Check if rectangle has no width */
	if ((x1 == x2) || (y1 == y2)) return;

	/* initialize */
	if (x1 > x2)
	{
		/* Swap boundaries if in wrong order */
		temp = x1;
		x1 = x2;
		x2 = temp;
	}

	if (y1 > y2)
	{
		/* Swap boundaries if in wrong order */
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	/* Get total widths */
	xsize = x2 - x1;
	ysize = y2 - y1;

	build_rectangle(y1, x1, y2, x2, feat_wall_outer, CAVE_ROOM | CAVE_ICKY);

	/* Middle */
	for (x = 1; x < xsize; x++)
	{
		for (y = 1; y < ysize; y++)
		{
			if (cave[y1 + y][x1 + x].feat == FEAT_WALL_OUTER)
			{
				/* Clear the untouched region */
				place_floor(y1 + y, x1 + x);
				cave[y1 + y][x1 + x].info |= (CAVE_ROOM | CAVE_ICKY);
			}
			else
			{
				/* Make it a room -- but don't touch */
				cave[y1 + y][x1 + x].info |= (CAVE_ROOM | CAVE_ICKY);
			}
		}
	}
}


/*
 * Create a random vault that looks like a collection of overlapping rooms
 */
static void build_room_vault(int x0, int y0, int xsize, int ysize)
{
	int i, x1, x2, y1, y2, xhsize, yhsize;

	/* Get offset from center */
	xhsize = xsize / 2;
	yhsize = ysize / 2;

	if (cheat_room) msg_print("Room Vault");

	/* Fill area so don't get problems with arena levels */
	for (x1 = 0; x1 <= xsize; x1++)
	{
		int x = x0 - xhsize + x1;

		for (y1 = 0; y1 <= ysize; y1++)
		{
			int y = y0 - yhsize + y1;

			cave_set_feat(y, x, FEAT_WALL_OUTER);
			cave[y][x].info &= ~(CAVE_ICKY);
		}
	}

	/* Add ten random rooms */
	for (i = 0; i < 10; i++)
	{
		x1 = randint(xhsize) * 2 + x0 - xhsize;
		x2 = randint(xhsize) * 2 + x0 - xhsize;
		y1 = randint(yhsize) * 2 + y0 - yhsize;
		y2 = randint(yhsize) * 2 + y0 - yhsize;

		build_room(x1, x2, y1, y2);
	}

	convert_extra(y0 - yhsize, x0 - xhsize, y0 - yhsize + ysize,
	              x0 - xhsize + xsize);

	/* Add some random doors */
	for (i = 0; i < 500; i++)
	{
		x1 = randint(xsize - 2) - xhsize + x0 + 1;
		y1 = randint(ysize - 2) - yhsize + y0 + 1;
		add_door(x1, y1);
	}

	/* Fill with monsters and treasure, high difficulty */
	fill_treasure(x0 - xhsize + 1, x0 - xhsize + xsize - 1,
	              y0 - yhsize + 1, y0 - yhsize + ysize - 1, randint(5) + 5);
}


/*
 * Create a random vault out of a fractal cave
 */
static void build_cave_vault(int x0, int y0, int xsiz, int ysiz)
{
	int grd, roug, cutoff, xhsize, yhsize, xsize, ysize, x, y;
	bool_ done, light, room;

	/* Round to make sizes even */
	xhsize = xsiz / 2;
	yhsize = ysiz / 2;
	xsize = xhsize * 2;
	ysize = yhsize * 2;

	if (cheat_room) msg_print("Cave Vault");

	light = done = FALSE;
	room = TRUE;

	while (!done)
	{
		/* Testing values for these parameters feel free to adjust */
		grd = 1 << rand_int(4);

		/* Want average of about 16 */
		roug = randint(8) * randint(4);

		/* About size/2 */
		cutoff = randint(xsize / 4) + randint(ysize / 4) +
		         randint(xsize / 4) + randint(ysize / 4);

		/* Make it */
		generate_hmap(y0, x0, xsize, ysize, grd, roug, cutoff);

		/* Convert to normal format + clean up */
		done = generate_fracave(y0, x0, xsize, ysize, cutoff, light, room);
	}

	/* Set icky flag because is a vault */
	for (x = 0; x <= xsize; x++)
	{
		for (y = 0; y <= ysize; y++)
		{
			cave[y0 - yhsize + y][x0 - xhsize + x].info |= CAVE_ICKY;
		}
	}

	/* Fill with monsters and treasure, low difficulty */
	fill_treasure(x0 - xhsize + 1, x0 - xhsize + xsize - 1,
	              y0 - yhsize + 1, y0 - yhsize + ysize - 1, randint(5));
}


/*
 * Maze vault -- rectangular labyrinthine rooms
 *
 * maze vault uses two routines:
 *    r_visit - a recursive routine that builds the labyrinth
 *    build_maze_vault - a driver routine that calls r_visit and adds
 *                   monsters, traps and treasure
 *
 * The labyrinth is built by creating a spanning tree of a graph.
 * The graph vertices are at
 *    (x, y) = (2j + x1, 2k + y1)   j = 0,...,m-1    k = 0,...,n-1
 * and the edges are the vertical and horizontal nearest neighbors.
 *
 * The spanning tree is created by performing a suitably randomized
 * depth-first traversal of the graph. The only adjustable parameter
 * is the rand_int(3) below; it governs the relative density of
 * twists and turns in the labyrinth: smaller number, more twists.
 */
static void r_visit(int y1, int x1, int y2, int x2,
                    int node, int dir, int *visited)
{
	int i, j, m, n, temp, x, y, adj[4];

	/* Dimensions of vertex array */
	m = (x2 - x1) / 2 + 1;
	n = (y2 - y1) / 2 + 1;

	/* Mark node visited and set it to a floor */
	visited[node] = 1;
	x = 2 * (node % m) + x1;
	y = 2 * (node / m) + y1;
	place_floor(y, x);

	/* Setup order of adjacent node visits */
	if (rand_int(3) == 0)
	{
		/* Pick a random ordering */
		for (i = 0; i < 4; i++)
		{
			adj[i] = i;
		}
		for (i = 0; i < 4; i++)
		{
			j = rand_int(4);
			temp = adj[i];
			adj[i] = adj[j];
			adj[j] = temp;
		}
		dir = adj[0];
	}
	else
	{
		/* Pick a random ordering with dir first */
		adj[0] = dir;
		for (i = 1; i < 4; i++)
		{
			adj[i] = i;
		}
		for (i = 1; i < 4; i++)
		{
			j = 1 + rand_int(3);
			temp = adj[i];
			adj[i] = adj[j];
			adj[j] = temp;
		}
	}

	for (i = 0; i < 4; i++)
	{
		switch (adj[i])
		{
			/* (0,+) - check for bottom boundary */
		case 0:
			{
				if ((node / m < n - 1) && (visited[node + m] == 0))
				{
					place_floor(y + 1, x);
					r_visit(y1, x1, y2, x2, node + m, dir, visited);
				}
				break;
			}

			/* (0,-) - check for top boundary */
		case 1:
			{
				if ((node / m > 0) && (visited[node - m] == 0))
				{
					place_floor(y - 1, x);
					r_visit(y1, x1, y2, x2, node - m, dir, visited);
				}
				break;
			}

			/* (+,0) - check for right boundary */
		case 2:
			{
				if ((node % m < m - 1) && (visited[node + 1] == 0))
				{
					place_floor(y, x + 1);
					r_visit(y1, x1, y2, x2, node + 1, dir, visited);
				}
				break;
			}

			/* (-,0) - check for left boundary */
		case 3:
			{
				if ((node % m > 0) && (visited[node - 1] == 0))
				{
					place_floor(y, x - 1);
					r_visit(y1, x1, y2, x2, node - 1, dir, visited);
				}
				break;
			}
		}
	}
}


static void build_maze_vault(int x0, int y0, int xsize, int ysize)
{
	int y, x, dy, dx;
	int y1, x1, y2, x2;
	int i, m, n, num_vertices, *visited;
	bool_ light;
	cave_type *c_ptr;


	if (cheat_room) msg_print("Maze Vault");

	/* Choose lite or dark */
	light = (dun_level <= randint(25));

	/* Pick a random room size - randomized by calling routine */
	dy = ysize / 2 - 1;
	dx = xsize / 2 - 1;

	y1 = y0 - dy;
	x1 = x0 - dx;
	y2 = y0 + dy;
	x2 = x0 + dx;

	/* Generate the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[y][x];

			c_ptr->info |= (CAVE_ROOM | CAVE_ICKY);

			if ((x == x1 - 1) || (x == x2 + 1) ||
			                (y == y1 - 1) || (y == y2 + 1))
			{
				cave_set_feat(y, x, feat_wall_outer);
			}
			else
			{
				cave_set_feat(y, x, feat_wall_inner);
			}
			if (light) c_ptr->info |= (CAVE_GLOW);
		}
	}

	/* Dimensions of vertex array */
	m = dx + 1;
	n = dy + 1;
	num_vertices = m * n;

	/* Allocate an array for visited vertices */
	C_MAKE(visited, num_vertices, int);

	/* Initialise array of visited vertices */
	for (i = 0; i < num_vertices; i++)
	{
		visited[i] = 0;
	}

	/* Traverse the graph to create a spaning tree, pick a random root */
	r_visit(y1, x1, y2, x2, rand_int(num_vertices), 0, visited);

	/* Fill with monsters and treasure, low difficulty */
	fill_treasure(x1, x2, y1, y2, randint(5));

	/* Free the array for visited vertices */
	C_FREE(visited, num_vertices, int);
}


/*
 * Build a "mini" checkerboard vault
 *
 * This is done by making a permanent wall maze and setting
 * the diagonal sqaures of the checker board to be granite.
 * The vault has two entrances on opposite sides to guarantee
 * a way to get in even if the vault abuts a side of the dungeon.
 */
static void build_mini_c_vault(int x0, int y0, int xsize, int ysize)
{
	int dy, dx;
	int y1, x1, y2, x2, y, x, total;
	int i, m, n, num_vertices;
	int *visited;

	if (cheat_room) msg_print("Mini Checker Board Vault");

	/* Pick a random room size */
	dy = ysize / 2 - 1;
	dx = xsize / 2 - 1;

	y1 = y0 - dy;
	x1 = x0 - dx;
	y2 = y0 + dy;
	x2 = x0 + dx;


	/* Generate the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			cave[y][x].info |= (CAVE_ROOM | CAVE_ICKY);

			/* Permanent walls */
			cave_set_feat(y, x, FEAT_PERM_INNER);
		}
	}


	/* Dimensions of vertex array */
	m = dx + 1;
	n = dy + 1;
	num_vertices = m * n;

	/* Allocate an array for visited vertices */
	C_MAKE(visited, num_vertices, int);

	/* Initialise array of visited vertices */
	for (i = 0; i < num_vertices; i++)
	{
		visited[i] = 0;
	}

	/* Traverse the graph to create a spannng tree, pick a random root */
	r_visit(y1, x1, y2, x2, rand_int(num_vertices), 0, visited);

	/* Make it look like a checker board vault */
	for (x = x1; x <= x2; x++)
	{
		for (y = y1; y <= y2; y++)
		{
			total = x - x1 + y - y1;

			/* If total is odd and is a floor, then make a wall */
			if ((total % 2 == 1) && get_is_floor(x, y))
			{
				cave_set_feat(y, x, feat_wall_inner);
			}
		}
	}

	/* Make a couple of entrances */
	if (rand_int(2) == 0)
	{
		/* Left and right */
		y = randint(dy) + dy / 2;
		cave_set_feat(y1 + y, x1 - 1, feat_wall_outer);
		cave_set_feat(y1 + y, x2 + 1, feat_wall_outer);
	}
	else
	{
		/* Top and bottom */
		x = randint(dx) + dx / 2;
		cave_set_feat(y1 - 1, x1 + x, feat_wall_outer);
		cave_set_feat(y2 + 1, x1 + x, feat_wall_outer);
	}

	/* Fill with monsters and treasure, highest difficulty */
	fill_treasure(x1, x2, y1, y2, 10);

	/* Free the array for visited vertices */
	C_FREE(visited, num_vertices, int);
}


/*
 * Build a town/ castle by using a recursive algorithm.
 * Basically divide each region in a probalistic way to create
 * smaller regions.  When the regions get too small stop.
 *
 * The power variable is a measure of how well defended a region is.
 * This alters the possible choices.
 */
static void build_recursive_room(int x1, int y1, int x2, int y2, int power)
{
	int xsize, ysize;
	int x, y;
	int choice;

	/* Temp variables */
	int t1, t2, t3, t4;

	xsize = x2 - x1;
	ysize = y2 - y1;

	if ((power < 3) && (xsize > 12) && (ysize > 12))
	{
		/* Need outside wall +keep */
		choice = 1;
	}
	else
	{
		if (power < 10)
		{
			/* Make rooms + subdivide */
			if ((randint(10) > 2) && (xsize < 8) && (ysize < 8))
			{
				choice = 4;
			}
			else
			{
				choice = randint(2) + 1;
			}
		}
		else
		{
			/* Mostly subdivide */
			choice = randint(3) + 1;
		}
	}

	/* Based on the choice made above, do something */
	switch (choice)
	{
		/* Outer walls */
	case 1:
		{
			/* Top and bottom */
			for (x = x1; x <= x2; x++)
			{
				cave_set_feat(y1, x, feat_wall_outer);
				cave_set_feat(y2, x, feat_wall_outer);
			}

			/* Left and right */
			for (y = y1 + 1; y < y2; y++)
			{
				cave_set_feat(y, x1, feat_wall_outer);
				cave_set_feat(y, x2, feat_wall_outer);
			}

			/* Make a couple of entrances */
			if (rand_int(2) == 0)
			{
				/* Left and right */
				y = randint(ysize) + y1;
				place_floor(y, x1);
				place_floor(y, x2);
			}
			else
			{
				/* Top and bottom */
				x = randint(xsize) + x1;
				place_floor(y1, x);
				place_floor(y2, x);
			}

			/* Select size of keep */
			t1 = randint(ysize / 3) + y1;
			t2 = y2 - randint(ysize / 3);
			t3 = randint(xsize / 3) + x1;
			t4 = x2 - randint(xsize / 3);

			/* Do outside areas */

			/* Above and below keep */
			build_recursive_room(x1 + 1, y1 + 1, x2 - 1, t1, power + 1);
			build_recursive_room(x1 + 1, t2, x2 - 1, y2, power + 1);

			/* Left and right of keep */
			build_recursive_room(x1 + 1, t1 + 1, t3, t2 - 1, power + 3);
			build_recursive_room(t4, t1 + 1, x2 - 1, t2 - 1, power + 3);

			/* Make the keep itself: */
			x1 = t3;
			x2 = t4;
			y1 = t1;
			y2 = t2;
			xsize = x2 - x1;
			ysize = y2 - y1;
			power += 2;

			/* Fall through */
		}

		/* Try to build a room */
	case 4:
		{
			if ((xsize < 3) || (ysize < 3))
			{
				for (y = y1; y < y2; y++)
				{
					for (x = x1; x < x2; x++)
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
				}

				/* Too small */
				return;
			}

			/* Make outside walls */

			/* Top and bottom */
			for (x = x1 + 1; x <= x2 - 1; x++)
			{
				cave_set_feat(y1 + 1, x, feat_wall_inner);
				cave_set_feat(y2 - 1, x, feat_wall_inner);
			}

			/* Left and right */
			for (y = y1 + 1; y <= y2 - 1; y++)
			{
				cave_set_feat(y, x1 + 1, feat_wall_inner);
				cave_set_feat(y, x2 - 1, feat_wall_inner);
			}

			/* Make a door */
			y = randint(ysize - 3) + y1 + 1;

			if (rand_int(2) == 0)
			{
				/* Left */
				place_floor(y, x1 + 1);
			}
			else
			{
				/* Right */
				place_floor(y, x2 - 1);
			}

			/* Build the room */
			build_recursive_room(x1 + 2, y1 + 2, x2 - 2, y2 - 2, power + 3);

			break;
		}

		/* Try and divide vertically */
	case 2:
		{
			if (xsize < 3)
			{
				/* Too small */
				for (y = y1; y < y2; y++)
				{
					for (x = x1; x < x2; x++)
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
				}
				return;
			}

			t1 = randint(xsize - 2) + x1 + 1;
			build_recursive_room(x1, y1, t1, y2, power - 2);
			build_recursive_room(t1 + 1, y1, x2, y2, power - 2);

			break;
		}

		/* Try and divide horizontally */
	case 3:
		{
			if (ysize < 3)
			{
				/* Too small */
				for (y = y1; y < y2; y++)
				{
					for (x = x1; x < x2; x++)
					{
						cave_set_feat(y, x, feat_wall_inner);
					}
				}
				return;
			}

			t1 = randint(ysize - 2) + y1 + 1;
			build_recursive_room(x1, y1, x2, t1, power - 2);
			build_recursive_room(x1, t1 + 1, x2, y2, power - 2);

			break;
		}
	}
}


/*
 * Build a castle
 *
 * Clear the region and call the recursive room routine.
 *
 * This makes a vault that looks like a castle or city in the dungeon.
 */
static void build_castle_vault(int x0, int y0, int xsize, int ysize)
{
	int dy, dx;
	int y1, x1, y2, x2;
	int y, x;

	/* Pick a random room size */
	dy = ysize / 2 - 1;
	dx = xsize / 2 - 1;

	y1 = y0 - dy;
	x1 = x0 - dx;
	y2 = y0 + dy;
	x2 = x0 + dx;

	if (cheat_room) msg_print("Castle Vault");

	/* Generate the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			cave[y][x].info |= (CAVE_ROOM | CAVE_ICKY);

			/* Make everything a floor */
			place_floor(y, x);
		}
	}

	/* Make the castle */
	build_recursive_room(x1, y1, x2, y2, randint(5));

	/* Fill with monsters and treasure, low difficulty */
	fill_treasure(x1, x2, y1, y2, randint(3));
}


/*
 * Add outer wall to a floored region
 *
 * Note: no range checking is done so must be inside dungeon
 * This routine also stomps on doors
 */
static void add_outer_wall(int x, int y, int light, int x1, int y1,
                           int x2, int y2)
{
	int i, j;

	if (!in_bounds(y, x)) return;

	/*
	 * Hack -- Check to see if square has been visited before
	 * if so, then exit (use room flag to do this)
	 */
	if (cave[y][x].info & CAVE_ROOM) return;

	/* Set room flag */
	cave[y][x].info |= (CAVE_ROOM);

	if (get_is_floor(x, y))
	{
		for (i = -1; i <= 1; i++)
		{
			for (j = -1; j <= 1; j++)
			{
				if ((x + i >= x1) && (x + i <= x2) &&
				                (y + j >= y1) && (y + j <= y2))
				{
					add_outer_wall(x + i, y + j, light, x1, y1, x2, y2);
					if (light) cave[y][x].info |= CAVE_GLOW;
				}
			}
		}
	}

	/* Set bounding walls */
	else if (cave[y][x].feat == FEAT_WALL_EXTRA)
	{
		cave[y][x].feat = feat_wall_outer;
		if (light == TRUE) cave[y][x].info |= CAVE_GLOW;
	}

	/* Set bounding walls */
	else if (cave[y][x].feat == FEAT_PERM_OUTER)
	{
		if (light == TRUE) cave[y][x].info |= CAVE_GLOW;
	}
}


/*
 * Hacked distance formula - gives the 'wrong' answer
 *
 * Used to build crypts
 */
static int dist2(int x1, int y1, int x2, int y2,
                 int h1, int h2, int h3, int h4)
{
	int dx, dy;
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	/*
	 * Basically this works by taking the normal pythagorean formula
	 * and using an expansion to express this in a way without the
	 * square root.  This approximate formula is then perturbed to give
	 * the distorted results.  (I found this by making a mistake when I was
	 * trying to fix the circular rooms.)
	 */

	/* h1-h4 are constants that describe the metric */
	if (dx >= 2 * dy) return (dx + (dy * h1) / h2);
	if (dy >= 2 * dx) return (dy + (dx * h1) / h2);

	/* 128/181 is approx. 1/sqrt(2) */
	return (((dx + dy) * 128) / 181 +
	        (dx * dx / (dy * h3) + dy * dy / (dx * h3)) * h4);
}


/*
 * Build target vault
 *
 * This is made by two concentric "crypts" with perpendicular
 * walls creating the cross-hairs.
 */
static void build_target_vault(int x0, int y0, int xsize, int ysize)
{
	int rad, x, y;

	int h1, h2, h3, h4;


	/* Make a random metric */
	h1 = randint(32) - 16;
	h2 = randint(16);
	h3 = randint(32);
	h4 = randint(32) - 16;

	if (cheat_room) msg_print("Target Vault");

	/* Work out outer radius */
	if (xsize > ysize)
	{
		rad = ysize / 2;
	}
	else
	{
		rad = xsize / 2;
	}

	/* Make floor */
	for (x = x0 - rad; x <= x0 + rad; x++)
	{
		for (y = y0 - rad; y <= y0 + rad; y++)
		{
			cave_type *c_ptr;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Clear room flag */
			c_ptr->info &= ~(CAVE_ROOM);

			/* Grids in vaults are required to be "icky" */
			c_ptr->info |= (CAVE_ICKY);

			/* Inside -- floor */
			if (dist2(y0, x0, y, x, h1, h2, h3, h4) <= rad - 1)
			{
				place_floor(y, x);
			}

			/* Outside -- make it granite so that arena works */
			else
			{
				c_ptr->feat = FEAT_WALL_EXTRA;
			}

			/* Proper boundary for arena */
			if (((y + rad) == y0) || ((y - rad) == y0) ||
			                ((x + rad) == x0) || ((x - rad) == x0))
			{
				cave_set_feat(y, x, feat_wall_outer);
			}
		}
	}

	/* Find visible outer walls and set to be FEAT_OUTER */
	add_outer_wall(x0, y0, FALSE, x0 - rad - 1, y0 - rad - 1,
	               x0 + rad + 1, y0 + rad + 1);

	/* Add inner wall */
	for (x = x0 - rad / 2; x <= x0 + rad / 2; x++)
	{
		for (y = y0 - rad / 2; y <= y0 + rad / 2; y++)
		{
			if (dist2(y0, x0, y, x, h1, h2, h3, h4) == rad / 2)
			{
				/* Make an internal wall */
				cave_set_feat(y, x, feat_wall_inner);
			}
		}
	}

	/* Add perpendicular walls */
	for (x = x0 - rad; x <= x0 + rad; x++)
	{
		cave_set_feat(y0, x, feat_wall_inner);
	}

	for (y = y0 - rad; y <= y0 + rad; y++)
	{
		cave_set_feat(y, x0, feat_wall_inner);
	}

	/* Make inner vault */
	for (y = y0 - 1; y <= y0 + 1; y++)
	{
		cave_set_feat(y, x0 - 1, feat_wall_inner);
		cave_set_feat(y, x0 + 1, feat_wall_inner);
	}
	for (x = x0 - 1; x <= x0 + 1; x++)
	{
		cave_set_feat(y0 - 1, x, feat_wall_inner);
		cave_set_feat(y0 + 1, x, feat_wall_inner);
	}

	place_floor(y0, x0);


	/*
	 * Add doors to vault
	 *
	 * Get two distances so can place doors relative to centre
	 */
	x = (rad - 2) / 4 + 1;
	y = rad / 2 + x;

	add_door(x0 + x, y0);
	add_door(x0 + y, y0);
	add_door(x0 - x, y0);
	add_door(x0 - y, y0);
	add_door(x0, y0 + x);
	add_door(x0, y0 + y);
	add_door(x0, y0 - x);
	add_door(x0, y0 - y);

	/* Fill with stuff - medium difficulty */
	fill_treasure(x0 - rad, x0 + rad, y0 - rad, y0 + rad, randint(3) + 3);
}


/*
 * Random vaults
 */
static void build_type11(int by0, int bx0)
{
	int y0, x0, xsize, ysize, vtype;

	/* Get size -- gig enough to look good, small enough to be fairly common */
	xsize = randint(22) + 22;
	ysize = randint(11) + 11;

	/* Allocate in room_map.  If will not fit, exit */
	if (!room_alloc(xsize + 2, ysize + 2, FALSE, by0, bx0, &x0, &y0)) return;

	/*
	 * Boost the rating -- Higher than lesser vaults and lower than
	 * greater vaults
	 */
	rating += 10;

	/* (Sometimes) Cause a special feeling */
	if ((dun_level <= 50) ||
	                (randint((dun_level - 40) * (dun_level - 40) + 1) < 400))
	{
		good_item_flag = TRUE;
	}

	/* Select type of vault */
	vtype = randint(8);

	switch (vtype)
	{
		/* Build an appropriate room */
	case 1:
		{
			build_bubble_vault(x0, y0, xsize, ysize);
			break;
		}

	case 2:
		{
			build_room_vault(x0, y0, xsize, ysize);
			break;
		}

	case 3:
		{
			build_cave_vault(x0, y0, xsize, ysize);
			break;
		}

	case 4:
		{
			build_maze_vault(x0, y0, xsize, ysize);
			break;
		}

	case 5:
		{
			build_mini_c_vault(x0, y0, xsize, ysize);
			break;
		}

	case 6:
		{
			build_castle_vault(x0, y0, xsize, ysize);
			break;
		}

	case 7:
		{
			build_target_vault(x0, y0, xsize, ysize);
			break;
		}

		/* I know how to add a few more... give me some time. */

		/* Paranoia */
	default:
		{
			return;
		}
	}
}

/*
 * Crypt room generation from Z 2.5.1
 */

/*
 * Build crypt room.
 * For every grid in the possible square, check the (fake) distance.
 * If it's less than the radius, make it a room square.
 *
 * When done fill from the inside to find the walls,
 */
static void build_type12(int by0, int bx0)
{
	int light, rad, x, y, x0, y0;
	bool_ emptyflag = TRUE;
	int h1, h2, h3, h4;

	/* Make a random metric */
	h1 = randint(32) - 16;
	h2 = randint(16);
	h3 = randint(32);
	h4 = randint(32) - 16;

	/* Occasional light */
	light = (randint(dun_level) <= 5) ? TRUE : FALSE;

	rad = randint(9);

	/* Allocate in room_map.  If will not fit, exit */
	if (!room_alloc(rad * 2 + 3, rad * 2 + 3, FALSE, by0, bx0, &x0, &y0)) return;

	/* Make floor */
	for (x = x0 - rad; x <= x0 + rad; x++)
	{
		for (y = y0 - rad; y <= y0 + rad; y++)
		{
			/* Clear room flag */
			cave[y][x].info &= ~(CAVE_ROOM);

			/* Inside -- floor */
			if (dist2(y0, x0, y, x, h1, h2, h3, h4) <= rad - 1)
			{
				place_floor(y, x);
			}
			else if (distance(y0, x0, y, x) < 3)
			{
				place_floor(y, x);
			}

			/* Outside -- make it granite so that arena works */
			else
			{
				cave_set_feat(y, x, feat_wall_outer);
			}

			/* Proper boundary for arena */
			if (((y + rad) == y0) || ((y - rad) == y0) ||
			                ((x + rad) == x0) || ((x - rad) == x0))
			{
				cave_set_feat(y, x, feat_wall_outer);
			}
		}
	}

	/* Find visible outer walls and set to be FEAT_OUTER */
	add_outer_wall(x0, y0, light, x0 - rad - 1, y0 - rad - 1,
	               x0 + rad + 1, y0 + rad + 1);

	/* Check to see if there is room for an inner vault */
	for (x = x0 - 2; x <= x0 + 2; x++)
	{
		for (y = y0 - 2; y <= y0 + 2; y++)
		{
			if (!get_is_floor(x, y))
			{
				/* Wall in the way */
				emptyflag = FALSE;
			}
		}
	}

	if (emptyflag && (rand_int(2) == 0))
	{
		/* Build the vault */
		build_small_room(x0, y0);

		/* Place a treasure in the vault */
		place_object(y0, x0, FALSE, FALSE, OBJ_FOUND_FLOOR);

		/* Let's guard the treasure well */
		vault_monsters(y0, x0, rand_int(2) + 3);

		/* Traps naturally */
		vault_traps(y0, x0, 4, 4, rand_int(3) + 2);
	}
}


/*
 * Constructs a tunnel between two points
 *
 * This function must be called BEFORE any streamers are created,
 * since we use the special "granite wall" sub-types to keep track
 * of legal places for corridors to pierce rooms.
 *
 * We use "door_flag" to prevent excessive construction of doors
 * along overlapping corridors.
 *
 * We queue the tunnel grids to prevent door creation along a corridor
 * which intersects itself.
 *
 * We queue the wall piercing grids to prevent a corridor from leaving
 * a room and then coming back in through the same entrance.
 *
 * We "pierce" grids which are "outer" walls of rooms, and when we
 * do so, we change all adjacent "outer" walls of rooms into "solid"
 * walls so that no two corridors may use adjacent grids for exits.
 *
 * The "solid" wall check prevents corridors from "chopping" the
 * corners of rooms off, as well as "silly" door placement, and
 * "excessively wide" room entrances.
 *
 * Useful "feat" values:
 *   FEAT_WALL_EXTRA -- granite walls
 *   FEAT_WALL_INNER -- inner room walls
 *   FEAT_WALL_OUTER -- outer room walls
 *   FEAT_WALL_SOLID -- solid room walls
 *   FEAT_PERM_EXTRA -- shop walls (perma)
 *   FEAT_PERM_INNER -- inner room walls (perma)
 *   FEAT_PERM_OUTER -- outer room walls (perma)
 *   FEAT_PERM_SOLID -- dungeon border (perma)
 */
static void build_tunnel(int row1, int col1, int row2, int col2, bool_ water)
{
	int i, y, x;
	int tmp_row, tmp_col;
	int row_dir, col_dir;
	int start_row, start_col;
	int main_loop_count = 0;

	bool_ door_flag = FALSE;

	cave_type *c_ptr;


	/* Reset the arrays */
	dun->tunn_n = 0;
	dun->wall_n = 0;

	/* Save the starting location */
	start_row = row1;
	start_col = col1;

	/* Start out in the correct direction */
	correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

	/* Keep going until done (or bored) */
	while ((row1 != row2) || (col1 != col2))
	{
		/* Mega-Hack -- Paranoia -- prevent infinite loops */
		if (main_loop_count++ > 2000) break;

		/* Allow bends in the tunnel */
		if (rand_int(100) < DUN_TUN_CHG)
		{
			/* Acquire the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}
		}

		/* Get the next location */
		tmp_row = row1 + row_dir;
		tmp_col = col1 + col_dir;


		/* Extremely Important -- do not leave the dungeon */
		while (!in_bounds(tmp_row, tmp_col))
		{
			/* Acquire the correct direction */
			correct_dir(&row_dir, &col_dir, row1, col1, row2, col2);

			/* Random direction */
			if (rand_int(100) < DUN_TUN_RND)
			{
				rand_dir(&row_dir, &col_dir);
			}

			/* Get the next location */
			tmp_row = row1 + row_dir;
			tmp_col = col1 + col_dir;
		}


		/* Access the location */
		c_ptr = &cave[tmp_row][tmp_col];


		/* Avoid the edge of the dungeon */
		if (c_ptr->feat == FEAT_PERM_SOLID) continue;

		/* Avoid the edge of vaults */
		if (c_ptr->feat == FEAT_PERM_OUTER) continue;

		/* Avoid "solid" granite walls */
		if (c_ptr->feat == FEAT_WALL_SOLID) continue;

		/*
		 * Pierce "outer" walls of rooms
		 * Cannot trust feat code any longer...
		 */
		if ((c_ptr->feat == feat_wall_outer) &&
		                (c_ptr->info & CAVE_ROOM))
		{
			/* Acquire the "next" location */
			y = tmp_row + row_dir;
			x = tmp_col + col_dir;

			/* Hack -- Avoid outer/solid permanent walls */
			if (cave[y][x].feat == FEAT_PERM_SOLID) continue;
			if (cave[y][x].feat == FEAT_PERM_OUTER) continue;

			/* Hack -- Avoid outer/solid granite walls */
			if ((cave[y][x].feat == feat_wall_outer) &&
			                (cave[y][x].info & CAVE_ROOM)) continue;
			if (cave[y][x].feat == FEAT_WALL_SOLID) continue;

			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the wall location */
			if (dun->wall_n < WALL_MAX)
			{
				dun->wall[dun->wall_n].y = row1;
				dun->wall[dun->wall_n].x = col1;
				dun->wall_n++;
			}

			/* Forbid re-entry near this piercing */
			for (y = row1 - 1; y <= row1 + 1; y++)
			{
				for (x = col1 - 1; x <= col1 + 1; x++)
				{
					/* Convert adjacent "outer" walls as "solid" walls */
					if ((cave[y][x].feat == feat_wall_outer) &&
					                (cave[y][x].info & CAVE_ROOM))
					{
						/* Change the wall to a "solid" wall */
						/* Mega-Hack -- to be brought back later... */
						cave_set_feat(y, x, FEAT_WALL_SOLID);
					}
				}
			}
		}

		/* Travel quickly through rooms */
		else if (c_ptr->info & (CAVE_ROOM))
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;
		}

		/* Tunnel through all other walls */
		else if ((c_ptr->feat == d_info[dungeon_type].fill_type1) ||
		                (c_ptr->feat == d_info[dungeon_type].fill_type2) ||
		                (c_ptr->feat == d_info[dungeon_type].fill_type3))
		{
			/* Accept this location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Save the tunnel location */
			if (dun->tunn_n < TUNN_MAX)
			{
				dun->tunn[dun->tunn_n].y = row1;
				dun->tunn[dun->tunn_n].x = col1;
				dun->tunn_n++;
			}

			/* Allow door in next grid */
			door_flag = FALSE;
		}

		/* Handle corridor intersections or overlaps */
		else
		{
			/* Accept the location */
			row1 = tmp_row;
			col1 = tmp_col;

			/* Collect legal door locations */
			if (!door_flag)
			{
				/* Save the door location */
				if (dun->door_n < DOOR_MAX)
				{
					dun->door[dun->door_n].y = row1;
					dun->door[dun->door_n].x = col1;
					dun->door_n++;
				}

				/* No door in next grid */
				door_flag = TRUE;
			}

			/* Hack -- allow pre-emptive tunnel termination */
			if (rand_int(100) >= DUN_TUN_CON)
			{
				/* Distance between row1 and start_row */
				tmp_row = row1 - start_row;
				if (tmp_row < 0) tmp_row = ( -tmp_row);

				/* Distance between col1 and start_col */
				tmp_col = col1 - start_col;
				if (tmp_col < 0) tmp_col = ( -tmp_col);

				/* Terminate the tunnel */
				if ((tmp_row > 10) || (tmp_col > 10)) break;
			}
		}
	}


	/* Turn the tunnel into corridor */
	for (i = 0; i < dun->tunn_n; i++)
	{
		/* Access the grid */
		y = dun->tunn[i].y;
		x = dun->tunn[i].x;

		/* Access the grid */
		c_ptr = &cave[y][x];

		/* Clear previous contents, add a floor */
		if (!water)
		{
			place_floor(y, x);
		}
		else
		{
			cave_set_feat(y, x, FEAT_SHAL_WATER);
		}
	}


	/* Apply the piercings that we found */
	for (i = 0; i < dun->wall_n; i++)
	{
		/* Access the grid */
		y = dun->wall[i].y;
		x = dun->wall[i].x;

		/* Access the grid */
		c_ptr = &cave[y][x];

		/* Clear previous contents, add up floor */
		place_floor(y, x);

		/* Occasional doorway */
		if (!(dungeon_flags1 & DF1_NO_DOORS) &&
		                (rand_int(100) < DUN_TUN_PEN))
		{
			/* Place a random door */
			place_random_door(y, x);
		}
	}
}




/*
 * Count the number of "corridor" grids adjacent to the given grid.
 *
 * Note -- Assumes "in_bounds(y1, x1)"
 *
 * XXX XXX This routine currently only counts actual "empty floor"
 * grids which are not in rooms.  We might want to also count stairs,
 * open doors, closed doors, etc.
 */
static int next_to_corr(int y1, int x1)
{
	int i, y, x, k = 0;

	cave_type *c_ptr;

	/* Scan adjacent grids */
	for (i = 0; i < 4; i++)
	{
		/* Extract the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Skip non floors */
		if (!cave_floor_bold(y, x)) continue;

		/* Access the grid */
		c_ptr = &cave[y][x];

		/* Skip non "empty floor" grids */
		if ((c_ptr->feat != d_info[dungeon_type].floor1) &&
		                (c_ptr->feat != d_info[dungeon_type].floor2) &&
		                (c_ptr->feat != d_info[dungeon_type].floor3))
		{
			continue;
		}

		/* Skip grids inside rooms */
		if (c_ptr->info & (CAVE_ROOM)) continue;

		/* Count these grids */
		k++;
	}

	/* Return the number of corridors */
	return (k);
}


/*
 * Determine if the given location is "between" two walls,
 * and "next to" two corridor spaces.  XXX XXX XXX
 *
 * Assumes "in_bounds(y,x)"
 */
static bool_ possible_doorway(int y, int x)
{
	/* Count the adjacent corridors */
	if (next_to_corr(y, x) >= 2)
	{
		/* Check Vertical */
		if ((f_info[cave[y - 1][x].feat].flags1 & FF1_WALL) &&
		                (f_info[cave[y + 1][x].feat].flags1 & FF1_WALL))
		{
			return (TRUE);
		}

		/* Check Horizontal */
		if ((f_info[cave[y][x - 1].feat].flags1 & FF1_WALL) &&
		                (f_info[cave[y][x + 1].feat].flags1 & FF1_WALL))
		{
			return (TRUE);
		}
	}

	/* No doorway */
	return (FALSE);
}


/*
 * Places doors around y, x position
 */
static void try_doors(int y, int x)
{
	bool_ dir_ok[4];
	int i, k, n;
	int yy, xx;

	/* Paranoia */
	/* if (!in_bounds(y, x)) return; */

	/* Some dungeons don't have doors at all */
	if (dungeon_flags1 & (DF1_NO_DOORS)) return;

	/* Reset tally */
	n = 0;

	/* Look four cardinal directions */
	for (i = 0; i < 4; i++)
	{
		/* Assume NG */
		dir_ok[i] = FALSE;

		/* Access location */
		yy = y + ddy_ddd[i];
		xx = x + ddx_ddd[i];

		/* Out of level boundary */
		if (!in_bounds(yy, xx)) continue;

		/* Ignore walls */
		if (f_info[cave[yy][xx].feat].flags1 & (FF1_WALL)) continue;

		/* Ignore room grids */
		if (cave[yy][xx].info & (CAVE_ROOM)) continue;

		/* Not a doorway */
		if (!possible_doorway(yy, xx)) continue;

		/* Accept the direction */
		dir_ok[i] = TRUE;

		/* Count good spots */
		n++;
	}

	/* Use the traditional method 75% of time */
	if (rand_int(100) < 75)
	{
		for (i = 0; i < 4; i++)
		{
			/* Bad locations */
			if (!dir_ok[i]) continue;

			/* Place one of various kinds of doors */
			if (rand_int(100) < DUN_TUN_JCT)
			{
				/* Access location */
				yy = y + ddy_ddd[i];
				xx = x + ddx_ddd[i];

				/* Place a door */
				place_random_door(yy, xx);
			}
		}
	}

	/* Use alternative method */
	else
	{
		/* A crossroad */
		if (n == 4)
		{
			/* Clear OK flags XXX */
			for (i = 0; i < 4; i++) dir_ok[i] = FALSE;

			/* Put one or two secret doors */
			dir_ok[rand_int(4)] = TRUE;
			dir_ok[rand_int(4)] = TRUE;
		}

		/* A T-shaped intersection or two possible doorways */
		else if ((n == 3) || (n == 2))
		{
			/* Pick one random location from the list */
			k = rand_int(n);

			for (i = 0; i < 4; i++)
			{
				/* Reject all but k'th OK direction */
				if (dir_ok[i] && (k-- != 0)) dir_ok[i] = FALSE;
			}
		}

		/* Place secret door(s) */
		for (i = 0; i < 4; i++)
		{
			/* Bad location */
			if (!dir_ok[i]) continue;

			/* Access location */
			yy = y + ddy_ddd[i];
			xx = x + ddx_ddd[i];

			/* Place a secret door */
			place_secret_door(yy, xx);
		}
	}
}


/*
 * Attempt to build a room of the given type at the given block
 *
 * Note that we restrict the number of "crowded" rooms to reduce
 * the chance of overflowing the monster list during level creation.
 */
static bool_ room_build(int y, int x, int typ)
{
	/* Restrict level */
	if ((dun_level < roomdep[typ]) && !ironman_rooms) return (FALSE);

	/* Restrict "crowded" rooms */
	if (dun->crowded && ((typ == 5) || (typ == 6))) return (FALSE);

	/* Build a room */
	switch (typ)
	{
		/* Build an appropriate room */
	case 12:
		build_type12(y, x);
		break;
	case 11:
		build_type11(y, x);
		break;
	case 10:
		build_type10(y, x);
		break;
	case 9:
		build_type9 (y, x);
		break;
	case 8:
		build_type8 (y, x);
		break;
	case 7:
		build_type7 (y, x);
		break;
	case 6:
		build_type6 (y, x);
		break;
	case 5:
		build_type5 (y, x);
		break;
	case 4:
		build_type4 (y, x);
		break;
	case 3:
		build_type3 (y, x);
		break;
	case 2:
		build_type2 (y, x);
		break;
	case 1:
		build_type1 (y, x);
		break;

		/* Paranoia */
	default:
		return (FALSE);
	}

	/* Success */
	return (TRUE);
}

/*
 * Set level boundaries
 */
void set_bounders(bool_ empty_level)
{
	int y, x;

	/* Special boundary walls -- Top */
	for (x = 0; x < cur_wid; x++)
	{
		/* XXX XXX */
		if (empty_level) cave[0][x].mimic = fill_type[rand_int(100)];
		else cave[0][x].mimic = cave[0][x].feat;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(0, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- Bottom */
	for (x = 0; x < cur_wid; x++)
	{
		/* XXX XXX */
		if (empty_level) cave[cur_hgt - 1][x].mimic = fill_type[rand_int(100)];
		else cave[cur_hgt - 1][x].mimic = cave[cur_hgt - 1][x].feat;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(cur_hgt - 1, x, FEAT_PERM_SOLID);
	}

	/* Special boundary walls --  Left */
	for (y = 1; y < cur_hgt - 1; y++)
	{
		/* XXX XXX */
		if (empty_level) cave[y][0].mimic = fill_type[rand_int(100)];
		else cave[y][0].mimic = cave[y][0].feat;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, 0, FEAT_PERM_SOLID);
	}

	/* Special boundary walls --  Right */
	for (y = 1; y < cur_hgt - 1; y++)
	{
		/* XXX XXX */
		if (empty_level) cave[y][cur_wid - 1].mimic = fill_type[rand_int(100)];
		else cave[y][cur_wid - 1].mimic = cave[y][cur_wid - 1].feat;

		/* Clear previous contents, add "solid" perma-wall */
		cave_set_feat(y, cur_wid - 1, FEAT_PERM_SOLID);
	}
}

/* Needed to refill empty levels */
static void fill_level(bool_ use_floor, byte smooth);

/*
 * Generate a normal dungeon level
 */
bool_ level_generate_dungeon()
{
	int i, k, y, x, y1, x1, branch = get_branch();
	dungeon_info_type *d_ptr = &d_info[dungeon_type];

	int max_vault_ok = 2;

	bool_ destroyed = FALSE;
	bool_ empty_level = FALSE;
	bool_ cavern = FALSE;
	s16b town_level = 0;

	/* Is it a town level ? */
	for (i = 0; i < TOWN_DUNGEON; i++)
	{
		if (d_ptr->t_level[i] == dun_level) town_level = d_ptr->t_idx[i];
	}

	/* Check for arena level */
	if ((dungeon_flags1 & (DF1_EMPTY)) ||
	                (empty_levels && (rand_int(EMPTY_LEVEL) == 0)))
	{
		empty_level = TRUE;

		if (cheat_room || p_ptr->precognition)
		{
			msg_print("Arena level.");
		}

		/* Refill the level with floor tiles */
		fill_level(empty_level, d_ptr->fill_method);
	}

	/* Possible cavern */
	if ((dungeon_flags1 & DF1_CAVERN) && (rand_int(dun_level / 2) > DUN_CAVERN))
	{
		cavern = TRUE;

		/* Make a large fractal cave in the middle of the dungeon */
		if (cheat_room)
		{
			msg_print("Cavern on level.");
		}

		build_cavern();
	}

	/* Possible "destroyed" level */
	if ((dun_level > 10) && (rand_int(DUN_DEST) == 0))
	{
		destroyed = TRUE;
	}

	/* Hack -- No destroyed "quest" levels */
	if (is_quest(dun_level)) destroyed = FALSE;

	/* Hack -- No destroyed "small" levels */
	if ((cur_wid != MAX_WID) || (cur_hgt != MAX_HGT)) destroyed = FALSE;

	/* Hack -- No destroyed levels */
	if (dungeon_flags1 & DF1_NO_DESTROY) destroyed = FALSE;

	/* Actual maximum number of rooms on this level */
	dun->row_rooms = cur_hgt / BLOCK_HGT;
	dun->col_rooms = cur_wid / BLOCK_WID;


	/* Initialize the room table */
	for (y = 0; y < dun->row_rooms; y++)
	{
		for (x = 0; x < dun->col_rooms; x++)
		{
			dun->room_map[y][x] = FALSE;
		}
	}

	/* No "crowded" rooms yet */
	dun->crowded = FALSE;

	/* No rooms yet */
	dun->cent_n = 0;

	/* Pick a block for the room */
	y = rand_int(dun->row_rooms);
	x = rand_int(dun->col_rooms);

	/* Align dungeon rooms */
	if (dungeon_align)
	{
		/* Slide some rooms right */
		if ((x % 3) == 0) x++;

		/* Slide some rooms left */
		if ((x % 3) == 2) x--;
	}

	/* Ugly */
	process_hooks(HOOK_BUILD_ROOM1, "(d,d)", y, x);

	/* Build some rooms */
	for (i = 0; i < DUN_ROOMS; i++)
	{
		/* Pick a block for the room */
		y = rand_int(dun->row_rooms);
		x = rand_int(dun->col_rooms);

		/* Align dungeon rooms */
		if (dungeon_align)
		{
			/* Slide some rooms right */
			if ((x % 3) == 0) x++;

			/* Slide some rooms left */
			if ((x % 3) == 2) x--;
		}

		/* Destroyed levels are boring */
		if (destroyed)
		{
			/* The deeper you are, the more cavelike the rooms are */

			/* no caves when cavern exists: they look bad */
			k = randint(100);

			if (!cavern && (k < dun_level))
			{
				/* Type 10 -- Fractal cave */
				if (room_build(y, x, 10)) continue;
			}
			else
			{
				/* Attempt a "trivial" room */
				if ((dungeon_flags1 & DF1_CIRCULAR_ROOMS) &&
				                room_build(y, x, 9))
				{
					continue;
				}
				else if (room_build(y, x, 1)) continue;
			}

			/* Never mind */
			continue;
		}

		/* Attempt an "unusual" room -- no vaults on town levels */
		if (!town_level &&
		                (ironman_rooms || (rand_int(DUN_UNUSUAL) < dun_level)))
		{
			/* Roll for room type */
			k = (ironman_rooms ? 0 : rand_int(100));

			/* Attempt a very unusual room */ /* test hack */
			if (ironman_rooms || (rand_int(DUN_UNUSUAL) < dun_level))
			{
				/* Type 8 -- Greater vault (10%) */
				if (k < 10)
				{
					if (max_vault_ok > 1)
					{
						if (room_build(y, x, 8)) continue;
					}
					else
					{
						if (cheat_room) msg_print("Refusing a greater vault.");
					}
				}

				/* Type 7 -- Lesser vault (15%) */
				if (k < 25)
				{
					if (max_vault_ok > 0)
					{
						if (room_build(y, x, 7)) continue;
					}
					else
					{
						if (cheat_room) msg_print("Refusing a lesser vault.");
					}
				}


				/* Type 5 -- Monster nest (15%) */
				if ((k < 40) && room_build(y, x, 5)) continue;

				/* Type 6 -- Monster pit (15%) */
				if ((k < 55) && room_build(y, x, 6)) continue;

				/* Type 11 -- Random vault (5%) */
				if ((k < 60) && room_build(y, x, 11)) continue;
			}

			/* Type 4 -- Large room (25%) */
			if ((k < 25) && room_build(y, x, 4)) continue;

			/* Type 3 -- Cross room (20%) */
			if ((k < 45) && room_build(y, x, 3)) continue;

			/* Type 2 -- Overlapping (20%) */
			if ((k < 65) && room_build(y, x, 2)) continue;

			/* Type 10 -- Fractal cave (15%) */
			if ((k < 80) && room_build(y, x, 10)) continue;

			/* Type 9 -- Circular (10%) */
			/* Hack - build standard rectangular rooms if needed */
			if (k < 90)
			{
				if ((dungeon_flags1 & DF1_CIRCULAR_ROOMS) && room_build(y, x, 1)) continue;
				else if (room_build(y, x, 9)) continue;
			}

			/* Type 12 -- Crypt (10%) */
			if ((k < 100) && room_build(y, x, 12)) continue;
		}

		/* Attempt a trivial room */
		if (dungeon_flags1 & DF1_CAVE)
		{
			if (room_build(y, x, 10)) continue;
		}
		else
		{
			if ((dungeon_flags1 & DF1_CIRCULAR_ROOMS) && room_build(y, x, 9)) continue;
			else if (room_build(y, x, 1)) continue;
		}
	}

	/* If no rooms are allocated... */
	while (dun->cent_n == 0)
	{
		/* ...force the creation of a small rectangular room */
		(void)room_build(0, 0, 1);
	}

	/* Hack -- Scramble the room order */
	for (i = 0; i < dun->cent_n; i++)
	{
		int pick1 = rand_int(dun->cent_n);
		int pick2 = rand_int(dun->cent_n);
		y1 = dun->cent[pick1].y;
		x1 = dun->cent[pick1].x;
		dun->cent[pick1].y = dun->cent[pick2].y;
		dun->cent[pick1].x = dun->cent[pick2].x;
		dun->cent[pick2].y = y1;
		dun->cent[pick2].x = x1;
	}

	/* Start with no tunnel doors */
	dun->door_n = 0;

	/* Hack -- connect the first room to the last room */
	y = dun->cent[dun->cent_n - 1].y;
	x = dun->cent[dun->cent_n - 1].x;

	/* Connect all the rooms together */
	for (i = 0; i < dun->cent_n; i++)
	{
		/* Connect the room to the previous room */
		build_tunnel(dun->cent[i].y, dun->cent[i].x, y, x, FALSE);

		/* Remember the "previous" room */
		y = dun->cent[i].y;
		x = dun->cent[i].x;
	}

	/* Mega-Hack -- Convert FEAT_WALL_SOLID back into outer walls */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			if (cave[y][x].feat == FEAT_WALL_SOLID)
			{
				cave_set_feat(y, x, feat_wall_outer);
			}
		}
	}

	/* Place intersection doors	 */
	for (i = 0; i < dun->door_n; i++)
	{
		/* Extract junction location */
		y = dun->door[i].y;
		x = dun->door[i].x;

		/* Try placing doors */
		try_doors(y, x);
	}

	if (strcmp(game_module, "ToME") == 0)
	{
		/* Hack -- Add some magma streamers */
		if ((dungeon_type == DUNGEON_MORDOR) || (dungeon_type == DUNGEON_ANGBAND))
			for (i = 0; i < DUN_STR_MAG; i++)
			{
				build_streamer(FEAT_MAGMA, DUN_STR_MC);
			}

		/* Hack -- Add some quartz streamers */
		if ((dungeon_type == DUNGEON_MORDOR) || (dungeon_type == DUNGEON_ANGBAND))
			for (i = 0; i < DUN_STR_QUA; i++)
			{
				build_streamer(FEAT_QUARTZ, DUN_STR_QC);
			}
	}

	/* Add some sand streamers */
	if ((dungeon_flags1 & DF1_SAND_VEIN) && !rand_int(4))
	{
		if ((cheat_room) || (p_ptr->precognition)) msg_print("Sand vein.");
		build_streamer(FEAT_SANDWALL, DUN_STR_SC);
	}

	/* Destroy the level if necessary */
	if (destroyed) destroy_level();

	/* Create the town if needed */
	if (town_level)
	{
		town_gen(town_level);
	}

	/* Hack -- Add some rivers if requested */
	if ((dungeon_flags1 & DF1_WATER_RIVER) && !rand_int(4))
	{
		if (cheat_room || p_ptr->precognition) msg_print("River of water.");
		add_river(FEAT_DEEP_WATER, FEAT_SHAL_WATER);
	}
	if ((dungeon_flags1 & DF1_LAVA_RIVER) && !rand_int(4))
	{
		if ((cheat_room) || (p_ptr->precognition)) msg_print("River of lava.");
		add_river(FEAT_DEEP_LAVA, FEAT_SHAL_LAVA);
	}

	if (dungeon_flags1 & DF1_WATER_RIVERS)
	{
		int max = 3 + rand_int(2);
		bool_ said = FALSE;

		for (i = 0; i < max; i++)
		{
			if (rand_int(3) == 0)
			{
				add_river(FEAT_DEEP_WATER, FEAT_SHAL_WATER);
				if (!said && ((cheat_room) || (p_ptr->precognition))) msg_print("Rivers of water.");
				said = TRUE;
			}
		}
	}

	if (dungeon_flags1 & DF1_LAVA_RIVERS)
	{
		int max = 2 + rand_int(2);
		bool_ said = FALSE;

		for (i = 0; i < max; i++)
		{
			if (rand_int(3) == 0)
			{
				add_river(FEAT_DEEP_LAVA, FEAT_SHAL_LAVA);
				if (!said && ((cheat_room) || (p_ptr->precognition))) msg_print("Rivers of lava.");
				said = TRUE;
			}
		}
	}

	/* Add streamers of trees, water, or lava -KMW- */
	if (!(dungeon_flags1 & DF1_NO_STREAMERS))
	{
		int num;

		/*
		 * Flat levels (was: levels 1--2)
		 *
		 * Small trees (penetrate walls)
		 */
		if ((dungeon_flags1 & DF1_FLAT) && (randint(20) > 15))
		{
			num = randint(DUN_STR_QUA);

			for (i = 0; i < num; i++)
			{
				build_streamer2(FEAT_SMALL_TREES, 1);
			}
		}

		/*
		 * Levels 1 -- 33 (was: 1 -- 19)
		 *
		 * Shallow water (preserve walls)
		 * Deep water (penetrate walls)
		 */
		if (!(dun_level <= 33) && (randint(20) > 15))
		{
			num = randint(DUN_STR_QUA - 1);

			for (i = 0; i < num; i++)
			{
				build_streamer2(FEAT_SHAL_WATER, 0);
			}

			if (randint(20) > 15)
			{
				num = randint(DUN_STR_QUA);

				for (i = 0; i < num; i++)
				{
					build_streamer2(FEAT_DEEP_WATER, 1);
				}
			}
		}

		/*
		 * Levels 34 -- (was: 20 --)
		 */
		else if (dun_level > 33)
		{
			/*
			 * Shallow lava (preserve walls)
			 * Deep lava (penetrate walls)
			 */
			if (randint(20) > 15)
			{
				num = randint(DUN_STR_QUA);

				for (i = 0; i < num; i++)
				{
					build_streamer2(FEAT_SHAL_LAVA, 0);
				}

				if (randint(20) > 15)
				{
					num = randint(DUN_STR_QUA - 1);

					for (i = 0; i < num; i++)
					{
						build_streamer2(FEAT_DEEP_LAVA, 1);
					}
				}
			}

			/*
			 * Shallow water (preserve walls)
			 * Deep water (penetrate walls)
			 */
			else if (randint(20) > 15)
			{
				num = randint(DUN_STR_QUA - 1);

				for (i = 0; i < num; i++)
				{
					build_streamer2(FEAT_SHAL_WATER, 0);
				}

				if (randint(20) > 15)
				{
					num = randint(DUN_STR_QUA);

					for (i = 0; i < num; i++)
					{
						build_streamer2(FEAT_DEEP_WATER, 1);
					}
				}
			}
		}
	}

	/* Hack, seems like once a room overrode the level boundaries, this is BAD */
	set_bounders(empty_level);

	/* Determine the character location */
	if (!new_player_spot(branch))
		return FALSE;

	return TRUE;
}

/*
 * Bring the imprinted pets from the old level
 */
void replace_all_friends()
{
	int i;

	if (p_ptr->wild_mode) return;

	/* Scan every saved pet */
	for (i = 0; i < max_m_idx; i++)
	{
		if ((km_list[i].r_idx) && (km_list[i].status == MSTATUS_COMPANION))
		{
			int y = p_ptr->py, x = p_ptr->px;
			cave_type *c_ptr;
			monster_type *m_ptr;

			/* Find a suitable location */
			get_pos_player(5, &y, &x);
			c_ptr = &cave[y][x];

			/* Get a m_idx to use */
			c_ptr->m_idx = m_pop();
			m_ptr = &m_list[c_ptr->m_idx];

			/* Actualy place the monster */
			m_list[c_ptr->m_idx] = km_list[i];
			m_ptr->fy = y;
			m_ptr->fx = x;
			m_ptr->hold_o_idx = 0;
		}
	}
}

/*
 * Save the imprinted pets from the old level
 */
void save_all_friends()
{
	if (p_ptr->old_wild_mode) return;

	C_COPY(km_list, m_list, max_m_idx, monster_type);
}



/*
 * Return the dungeon type of the current level(it can only return the
 * principal dungeons)
 */
byte calc_dungeon_type()
{
	int i;

	for (i = 0; i < max_d_idx; i++)
	{
		if ((dun_level >= d_info[i].mindepth) &&
		                (dun_level <= d_info[i].maxdepth) &&
		                (d_info[i].flags1 & DF1_PRINCIPAL))
			return (i);
	}
	return (0);
}


/*
 * Build probability tables for walls and floors and set feat_wall_outer
 * and feat_wall_inner according to the current information in d_info.txt
 *
 * *hint* *hint* with this made extern, and we no longer have to
 * store fill_type and floor_type in the savefile...
 */
static void init_feat_info(void)
{
	dungeon_info_type *d_ptr = &d_info[dungeon_type];
	int i;
	int cur_depth, max_depth;
	int p1, p2;
	int floor_lim1, floor_lim2;
	int fill_lim1, fill_lim2;


	/* Retrieve dungeon depth info (base 1, to avoid zero divide errors) */
	cur_depth = (dun_level - d_ptr->mindepth) + 1;
	max_depth = (d_ptr->maxdepth - d_ptr->mindepth) + 1;


	/* Set room wall types */
	feat_wall_outer = d_ptr->outer_wall;
	feat_wall_inner = d_ptr->inner_wall;


	/* Setup probability info -- Floors */
	p1 = d_ptr->floor_percent1[0];
	p2 = d_ptr->floor_percent1[1];
	floor_lim1 = p1 + (p2 - p1) * cur_depth / max_depth;

	p1 = d_ptr->floor_percent2[0];
	p2 = d_ptr->floor_percent2[1];
	floor_lim2 = floor_lim1 + p1 + (p2 - p1) * cur_depth / max_depth;

	/* Setup probability info -- Fillers */
	p1 = d_ptr->fill_percent1[0];
	p2 = d_ptr->fill_percent1[1];
	fill_lim1 = p1 + (p2 - p1) * cur_depth / max_depth;

	p1 = d_ptr->fill_percent2[0];
	p2 = d_ptr->fill_percent2[1];
	fill_lim2 = fill_lim1 + p1 + (p2 - p1) * cur_depth / max_depth;


	/* Fill the arrays of floors and walls in the good proportions */
	for (i = 0; i < 100; i++)
	{
		if (i < floor_lim1)
		{
			floor_type[i] = d_ptr->floor1;
		}
		else if (i < floor_lim2)
		{
			floor_type[i] = d_ptr->floor2;
		}
		else
		{
			floor_type[i] = d_ptr->floor3;
		}

		if (i < fill_lim1)
		{
			fill_type[i] = d_ptr->fill_type1;
		}
		else if (i < fill_lim2)
		{
			fill_type[i] = d_ptr->fill_type2;
		}
		else
		{
			fill_type[i] = d_ptr->fill_type3;
		}
	}
}


/*
 * Fill a level with wall type specified in A: or L: line of d_info.txt
 *
 * 'use_floor', when it is TRUE, tells the function to use floor type
 * terrains (L:) instead of walls (A:).
 *
 * Filling behaviour can be controlled by the second parameter 'smooth',
 * with the following options available:
 *
 * smooth  behaviour
 * ------  ------------------------------------------------------------
 * 0       Fill the entire level with fill_type1 / floor1
 * 1       All the grids are randomly selected (== --P5.1.2)
 * 2       Slightly smoothed -- look like scattered patches
 * 3       More smoothed -- tend to look like caverns / small scale map
 * 4--     Max smoothing -- tend to look like landscape/island/
 *         continent etc.
 *
 * I put it here, because there's another filler generator in
 * wild.c, but it works better there, in fact...
 *
 * CAVEAT: smoothness of 3 or greater doesn't work well with the
 * current secret door implementation. Outer walls also need some
 * rethinking.
 *
 * -- pelpel
 */

/*
 * Thou shalt not invoke the name of thy RNG in vain.
 * The Angband RNG generates 28 bit pseudo-random number, hence
 * 28 / 2 = 14
 */
#define MAX_SHIFTS 14

static void fill_level(bool_ use_floor, byte smooth)
{
	int y, x;
	int step;
	int shift;


	/* Convert smoothness to initial step */
	if (smooth == 0) step = 0;
	else if (smooth == 1) step = 1;
	else if (smooth == 2) step = 2;
	else if (smooth == 3) step = 4;
	else step = 8;

	/*
	 * Paranoia -- step must be less than or equal to a half of
	 * width or height, whichever shorter
	 */
	if ((cur_hgt < 16) && (step > 4)) step = 4;
	if ((cur_wid < 16) && (step > 4)) step = 4;


	/* Special case -- simple fill */
	if (step == 0)
	{
		byte filler;

		/* Pick a filler XXX XXX XXX */
		if (use_floor) filler = d_info[dungeon_type].floor1;
		else filler = d_info[dungeon_type].fill_type1;

		/* Fill the level with the filler without calling RNG */
		for (y = 0; y < cur_hgt; y++)
		{
			for (x = 0; x < cur_wid; x++)
			{
				cave_set_feat(y, x, filler);
			}
		}

		/* Done */
		return;
	}


	/*
	 * Fill starting positions -- every 'step' grids horizontally and
	 * vertically
	 */
	for (y = 0; y < cur_hgt; y += step)
	{
		for (x = 0; x < cur_wid; x += step)
		{
			/*
			 * Place randomly selected terrain feature using the prebuilt
			 * probability table
			 *
			 * By slightly modifying this, you can build streamers as
			 * well as normal fillers all at once, but this calls for
			 * modifications to the other part of the dungeon generator.
			 */
			if (use_floor) place_floor(y, x);
			else place_filler(y, x);
		}
	}


	/*
	 * Fill spaces between, randomly picking one of their neighbours
	 *
	 * This simple yet powerful algorithm was described by Mike Anderson:
	 *
	 * A   B      A | B      A a B
	 *        ->  --+--  ->  d e b
	 * D   C      D | C      D c C
	 *
	 * a can be either A or B, b B or C, c C or D and d D or A.
	 * e is chosen from A, B, C and D.
	 * Subdivide and repeat the process as many times as you like.
	 *
	 * All the nasty tricks that obscure this simplicity are mine (^ ^;)
	 */

	/* Initialise bit shift counter */
	shift = MAX_SHIFTS;

	/* Repeat subdivision until all the grids are filled in */
	while ((step = step >> 1) > 0)
	{
		bool_ y_even, x_even;
		s16b y_wrap, x_wrap;
		s16b y_sel, x_sel;
		u32b selector = 0;

		/* Hacklette -- Calculate wrap-around locations */
		y_wrap = ((cur_hgt - 1) / (step * 2)) * (step * 2);
		x_wrap = ((cur_wid - 1) / (step * 2)) * (step * 2);

		/* Initialise vertical phase */
		y_even = 0;

		for (y = 0; y < cur_hgt; y += step)
		{
			/* Flip vertical phase */
			y_even = !y_even;

			/* Initialise horizontal phase */
			x_even = 0;

			for (x = 0; x < cur_wid; x += step)
			{
				/* Flip horizontal phase */
				x_even = !x_even;

				/* Already filled in by previous iterations */
				if (y_even && x_even) continue;

				/*
				 * Retrieve next two bits from pseudo-random bit sequence
				 *
				 * You can do well not caring so much about their randomness.
				 *
				 * This is not really necessary, but I don't like to invoke
				 * relatively expensive RNG when we can do with much smaller
				 * number of calls.
				 */
				if (shift >= MAX_SHIFTS)
				{
					selector = rand_int(0x10000000L);
					shift = 0;
				}
				else
				{
					selector >>= 2;
					shift++;
				}

				/* Vertically in sync */
				if (y_even) y_sel = y;

				/* Bit 1 selects neighbouring y */
				else y_sel = (selector & 2) ? y + step : y - step;

				/* Horizontally in sync */
				if (x_even) x_sel = x;

				/* Bit 0 selects neighbouring x */
				else x_sel = (selector & 1) ? x + step : x - step;

				/* Hacklette -- Fix out of range indices by wrapping around */
				if (y_sel >= cur_hgt) y_sel = 0;
				else if (y_sel < 0) y_sel = y_wrap;
				if (x_sel >= cur_wid) x_sel = 0;
				else if (x_sel < 0) x_sel = x_wrap;

				/*
				 * Fill the grid with terrain feature of the randomly
				 * picked up neighbour
				 */
				cave_set_feat(y, x, cave[y_sel][x_sel].feat);
			}
		}
	}
}


/*
 * Generate a new dungeon level
 *
 * Note that "dun_body" adds about 4000 bytes of memory to the stack.
 */
static bool_ cave_gen(void)
{
	int i, k, y, x, y1, x1, branch;
	dungeon_info_type *d_ptr = &d_info[dungeon_type];

	int max_vault_ok = 2;

	bool_ empty_level = FALSE;

	level_generator_type *generator;

	dun_data dun_body;

	char generator_name[100];

	if (!get_dungeon_generator(generator_name))
		strnfmt(generator_name, 99, "%s", d_ptr->generator);

	/*
	 * We generate a double dungeon. First we should halve the desired
	 * width/height, generate the dungeon normally, then double it
	 * in both directions
	 */
	if (dungeon_flags1 & DF1_DOUBLE)
	{
		cur_wid /= 2;
		cur_hgt /= 2;
	}

	/* Fill the arrays of floors and walls in the good proportions */
	init_feat_info();

	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Global data */
	dun = &dun_body;

	if (!(max_panel_rows)) max_vault_ok--;
	if (!(max_panel_cols)) max_vault_ok--;

	/*
	 * Hack -- Start with fill_type's
	 *
	 * Need a way to know appropriate smoothing factor for the current
	 * dungeon. Maybe we need another d_info flag/value.
	 */
	fill_level(empty_level, d_ptr->fill_method);

	set_bounders(empty_level);

	/*
	 * Call the good level generator
	 */
	generator = level_generators;
	while (generator)
	{
		if (!strcmp(generator->name, generator_name))
		{
			if (!generator->generator(generator->name))
				return FALSE;
			break;
		}

		generator = generator->next;
	}

	/* Only if requested */
	if (generator->default_stairs)
	{
		/* Is there a dungeon branch ? */
		if ((branch = get_branch()))
		{
			/* Place 5 down stair some walls */
			alloc_stairs(FEAT_MORE, 5, 3, branch);
		}

		/* Is there a father dungeon branch ? */
		if ((branch = get_fbranch()))
		{
			/* Place 1 down stair some walls */
			alloc_stairs(FEAT_LESS, 5, 3, branch);
		}

		if ((dun_level < d_ptr->maxdepth) || ((dun_level == d_ptr->maxdepth) && (dungeon_flags1 & DF1_FORCE_DOWN)))
		{
			/* Place 3 or 4 down stairs near some walls */
			alloc_stairs((dungeon_flags1 & DF1_FLAT) ? FEAT_WAY_MORE : FEAT_MORE, rand_range(3, 4), 3, 0);

			/* Place 0 or 1 down shafts near some walls */
			if (!(dungeon_flags2 & DF2_NO_SHAFT)) alloc_stairs((dungeon_flags1 & DF1_FLAT) ? FEAT_WAY_MORE : FEAT_SHAFT_DOWN, rand_range(0, 1), 3, 0);
		}

		if ((dun_level > d_ptr->mindepth) || ((dun_level == d_ptr->mindepth) && (!(dungeon_flags1 & DF1_NO_UP))))
		{
			/* Place 1 or 2 up stairs near some walls */
			alloc_stairs((dungeon_flags1 & DF1_FLAT) ? FEAT_WAY_LESS : FEAT_LESS, rand_range(1, 2), 3, 0);

			/* Place 0 or 1 up shafts near some walls */
			if (!(dungeon_flags2 & DF2_NO_SHAFT)) alloc_stairs((dungeon_flags1 & DF1_FLAT) ? FEAT_WAY_LESS : FEAT_SHAFT_UP, rand_range(0, 1), 3, 0);
		}
	}

	process_hooks(HOOK_GEN_LEVEL, "(d)", is_quest(dun_level));

	/* Basic "amount" */
	k = (dun_level / 3);
	if (k > 10) k = 10;
	if (k < 2) k = 2;

	/* Only if requested */
	if (generator->default_monsters)
	{

		/*
		 * Pick a base number of monsters
		 */
		i = d_ptr->min_m_alloc_level;

		/* To make small levels a bit more playable */
		if ((cur_hgt < MAX_HGT) || (cur_wid < MAX_WID))
		{
			int small_tester = i;

			i = (i * cur_hgt) / MAX_HGT;
			i = (i * cur_wid) / MAX_WID;
			i += 1;

			if (i > small_tester) i = small_tester;
			else if (cheat_hear)
			{
				msg_format("Reduced monsters base from %d to %d", small_tester, i);
			}
		}

		i += randint(8);

		/* Put some monsters in the dungeon */
		for (i = i + k; i > 0; i--)
		{
			(void)alloc_monster(0, TRUE);
		}
	}

	/* Check fates */
	for (i = 0; i < MAX_FATES; i++)
	{
		/* Ignore empty slots */
		if (fates[i].fate == FATE_NONE) continue;

		/* Check dungeon depth */
		if (fates[i].level != dun_level) continue;

		/* Non-serious fates don't always fire */
		if ((!fates[i].serious) && (randint(2) != 1)) continue;

		/* Player meets his/her fate now... */
		fate_flag = TRUE;

		switch (fates[i].fate)
		{
		case FATE_FIND_O:
			{
				int oy = p_ptr->py + 1;
				int ox = p_ptr->px;
				object_type *q_ptr, forge;

				/* Get local object */
				q_ptr = &forge;

				/* Mega-Hack */
				object_prep(q_ptr, fates[i].o_idx);

				/* Mega-Hack */
				apply_magic(q_ptr, dun_level, TRUE, TRUE, fates[i].serious);

				get_pos_player(10, &oy, &ox);

				/* Drop it from the heaven */
				drop_near(q_ptr, -1, oy, ox);

				/* Make it icky */
				fates[i].icky = TRUE;
				break;
			}
		case FATE_FIND_R:
			{
				int oy = p_ptr->py + 1;
				int ox = p_ptr->px;

				get_pos_player(10, &oy, &ox);

				place_monster_one(oy, ox, fates[i].r_idx, 0, fates[i].serious, MSTATUS_ENEMY);

				fates[i].icky = TRUE;
				break;
			}
		case FATE_FIND_A:
			{
				int oy = p_ptr->py + 1;
				int ox = p_ptr->px;
				object_type *q_ptr = NULL, forge;

				get_pos_player(10, &oy, &ox);

				/* XXX XXX XXX Grant a randart */
				if (fates[i].a_idx == 0)
				{
					int obj_lev;
					s16b k_idx;

					/* Apply restriction */
					get_obj_num_hook = kind_is_artifactable;

					/* Object level a la find object fates */
					obj_lev = max_dlv[dungeon_type] + randint(10);

					/* Rebuild allocation table */
					get_obj_num_prep();

					/* Roll for an object */
					k_idx = get_obj_num(obj_lev);

					/* Reset restriction */
					get_obj_num_hook = kind_is_legal;

					/* Invalidate the allocation table */
					alloc_kind_table_valid = FALSE;

					/* Get a local object */
					q_ptr = &forge;

					/* Wipe it */
					object_wipe(q_ptr);

					/* Create the object */
					object_prep(q_ptr, k_idx);

					/* SoAC it */
					create_artifact(q_ptr, FALSE, TRUE);

					/* Drop the artifact from heaven */
					drop_near(q_ptr, -1, oy, ox);
				}

				/* Grant a normal artefact */
				else if (a_info[fates[i].a_idx].cur_num == 0)
				{
					artifact_type *a_ptr = &a_info[fates[i].a_idx];
					s16b I_kind;

					/* Get local object */
					q_ptr = &forge;

					/* Wipe the object */
					object_wipe(q_ptr);

					/* Acquire the "kind" index */
					I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

					/* Create the artifact */
					object_prep(q_ptr, I_kind);

					/* Save the name */
					q_ptr->name1 = fates[i].a_idx;

					apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

					/* Drop the artifact from heaven */
					drop_near(q_ptr, -1, oy, ox);
				}

				fates[i].icky = TRUE;
				break;
			}
		}
	}

	/* Re scan the list to eliminate the inutile fate */
	for (i = 0; i < MAX_FATES; i++)
	{
		switch (fates[i].fate)
		{
		case FATE_FIND_A:
			{
				if (a_info[fates[i].a_idx].cur_num == 1) fates[i].icky = TRUE;
				break;
			}
		case FATE_FIND_R:
			{
				if ((r_info[fates[i].r_idx].cur_num == 1) && (r_info[fates[i].r_idx].flags1 & RF1_UNIQUE)) fates[i].icky = TRUE;
				break;
			}
		}
	}

	/* Only if requested */
	if (generator->default_miscs)
	{
		/* Place some traps in the dungeon */
		alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_TRAP, randint(k * 2));

		/* Put some rubble in corridors */
		alloc_object(ALLOC_SET_CORR, ALLOC_TYP_RUBBLE, randint(k));
	}

	/* Only if requested */
	if (generator->default_objects)
	{
		/* Put some objects in rooms */
		if (dungeon_type != DUNGEON_DEATH) alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ROOM, 3));

		/* Put some objects/gold in the dungeon */
		if (dungeon_type != DUNGEON_DEATH) alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_OBJECT, randnor(DUN_AMT_ITEM, 3));
		if (dungeon_type != DUNGEON_DEATH) alloc_object(ALLOC_SET_BOTH, ALLOC_TYP_GOLD, randnor(DUN_AMT_GOLD, 3));
	}

	/* Only if requested */
	if (generator->default_miscs)
	{
		/* Put some altars */
		alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_ALTAR, randnor(DUN_AMT_ALTAR, 3));

		/* Put some between gates */
		alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_BETWEEN, randnor(DUN_AMT_BETWEEN, 3));

		/* Put some fountains */
		alloc_object(ALLOC_SET_ROOM, ALLOC_TYP_FOUNTAIN, randnor(DUN_AMT_FOUNTAIN, 3));
	}

	/* Put an Artifact and Artifact Guardian is requested */
	if (d_ptr->final_guardian && (d_ptr->maxdepth == dun_level))
	{
		int oy;
		int ox;
		int m_idx, tries = 10000;

		/* Find a good position */
		while (tries)
		{
			/* Get a random spot */
			oy = randint(cur_hgt - 4) + 2;
			ox = randint(cur_wid - 4) + 2;

			/* Is it a good spot ? */
			if (cave_empty_bold(oy, ox)) break;

			/* One less try */
			tries--;
		}

		/* Place the guardian */
		m_allow_special[d_ptr->final_guardian] = TRUE;
		place_monster_one(oy, ox, d_ptr->final_guardian, 0, FALSE, MSTATUS_ENEMY);
		m_allow_special[d_ptr->final_guardian] = FALSE;


		m_idx = cave[oy][ox].m_idx;

		if (!m_idx && wizard) cmsg_print(TERM_L_RED, "WARNING: Could not place guardian.");

		/*
		 * If guardian is successfully created and his/her/its
		 * treasure hasn't been found, let him/her/it own that
		 */
		if (m_idx && d_ptr->final_artifact &&
		                (a_info[d_ptr->final_artifact].cur_num == 0))
		{
			artifact_type *a_ptr = &a_info[d_ptr->final_artifact];
			object_type *q_ptr, forge, *o_ptr;
			int I_kind, o_idx;

			/* Get new object */
			o_idx = o_pop();

			/* Proceed only if there's an object slot available */
			if (o_idx)
			{
				a_allow_special[d_ptr->final_artifact] = TRUE;

				/* Get local object */
				q_ptr = &forge;

				/* Wipe the object */
				object_wipe(q_ptr);

				/* Acquire the "kind" index */
				I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

				/* Create the artifact */
				object_prep(q_ptr, I_kind);

				/* Save the name */
				q_ptr->name1 = d_ptr->final_artifact;

				/* Actually create it */
				apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

				/* Where it is found ? */
				q_ptr->found = OBJ_FOUND_MONSTER;
				q_ptr->found_aux1 = d_ptr->final_guardian;
				q_ptr->found_aux2 = 0;
				q_ptr->found_aux3 = dungeon_type;
				q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

				a_allow_special[d_ptr->final_artifact] = FALSE;

				/* Get the item */
				o_ptr = &o_list[o_idx];

				/* Structure copy */
				object_copy(o_ptr, q_ptr);

				/* Build a stack */
				o_ptr->next_o_idx = m_list[m_idx].hold_o_idx;

				o_ptr->held_m_idx = m_idx;
				o_ptr->ix = 0;
				o_ptr->iy = 0;

				m_list[m_idx].hold_o_idx = o_idx;
			}
		}

		if (m_idx && d_ptr->final_object &&
		                (k_info[d_ptr->final_object].artifact == FALSE))
		{
			object_type *q_ptr, forge, *o_ptr;
			int o_idx;

			/* Get new object */
			o_idx = o_pop();

			/* Proceed only if there's an object slot available */
			if (o_idx)
			{
				/* Get local object */
				q_ptr = &forge;

				k_allow_special[d_ptr->final_object] = TRUE;

				/* Wipe the object */
				object_wipe(q_ptr);

				/* Create the final object */
				object_prep(q_ptr, d_ptr->final_object);
				apply_magic(q_ptr, 1, FALSE, FALSE, FALSE);

				/* Where it is found ? */
				q_ptr->found = OBJ_FOUND_MONSTER;
				q_ptr->found_aux1 = d_ptr->final_guardian;
				q_ptr->found_aux2 = 0;
				q_ptr->found_aux3 = dungeon_type;
				q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

				k_allow_special[d_ptr->final_object] = FALSE;

				k_info[d_ptr->final_object].artifact = TRUE;

				/* Get the item */
				o_ptr = &o_list[o_idx];

				/* Structure copy */
				object_copy(o_ptr, q_ptr);

				/* Build a stack */
				o_ptr->next_o_idx = m_list[m_idx].hold_o_idx;

				o_ptr->held_m_idx = m_idx;
				o_ptr->ix = 0;
				o_ptr->iy = 0;

				m_list[m_idx].hold_o_idx = o_idx;
			}
		}
	}

	if ((empty_level) && (randint(DARK_EMPTY) != 1 || (randint(100) > dun_level)))
		wiz_lite();

	/* Now double the generated dungeon */
	if (dungeon_flags1 & DF1_DOUBLE)
	{
		/* We begin at the bottom-right corner and from there move
		 * up/left (this way we don't need another array for the
		 * dungeon data) */
		/* Note: we double the border permanent walls, too. It is
		 * easier this way and I think it isn't too ugly */
		for (y = cur_hgt - 1, y1 = y * 2; y >= 0; y--, y1 -= 2)
			for (x = cur_wid - 1, x1 = x * 2; x >= 0; x--, x1 -= 2)
			{
				int disp[4][2] = {{0, 0}, {0, + 1}, { + 1, 0}, { + 1, + 1}};

				cave_type *c_ptr[4], *cc_ptr = &cave[y][x];
				object_type *o_ptr = &o_list[cc_ptr->o_idx];
				monster_type *m_ptr = &m_list[cc_ptr->m_idx];

				/*
				 * Now we copy the generated data to the
				 * appropriate grids
				 */
				for (i = 0; i < 4; i++)
				{
					c_ptr[i] = &cave[y1 + disp[i][0]][x1 + disp[i][1]];
					*c_ptr[i] = *cc_ptr;
					c_ptr[i]->o_idx = 0;
					c_ptr[i]->m_idx = 0;

					if (cc_ptr->feat == FEAT_BETWEEN)
					{
						int xxx = cc_ptr->special & 0xFF;
						int yyy = cc_ptr->special >> 8;

						xxx *= 2;
						yyy *= 2;
						xxx += disp[i][1];
						yyy += disp[i][0];
						c_ptr[i]->special = xxx + (yyy << 8);
					}
				}

				/* Objects should be put only in 1 of the
				 * new grids (otherwise we would segfault
				 * a lot) ... */
				if (cc_ptr->o_idx != 0)
				{
					i = rand_int(4);
					c_ptr[i]->o_idx = cc_ptr->o_idx;
					o_ptr->iy = y1 + disp[i][0];
					o_ptr->ix = x1 + disp[i][1];
				}

				/* ..just like monsters */
				if (cc_ptr->m_idx != 0)
				{
					i = rand_int(4);
					c_ptr[i]->m_idx = cc_ptr->m_idx;
					m_ptr->fy = y1 + disp[i][0];
					m_ptr->fx = x1 + disp[i][1];
				}
			}

		/* Set the width/height ... */
		cur_wid *= 2;
		cur_hgt *= 2;

		/* ... and player position to the right place */
		p_ptr->py *= 2;
		p_ptr->px *= 2;
	}

	return TRUE;
}


/*
 * Builds the arena after it is entered -KMW-
 */
static void build_arena(void)
{
	int yval, y_height, y_depth, xval, x_left, x_right;
	register int i, j;

	yval = SCREEN_HGT / 2;
	xval = SCREEN_WID / 2;
	y_height = yval - 10 + SCREEN_HGT;
	y_depth = yval + 10 + SCREEN_HGT;
	x_left = xval - 32 + SCREEN_WID;
	x_right = xval + 32 + SCREEN_WID;

	for (i = y_height; i <= y_height + 5; i++)
	{
		for (j = x_left; j <= x_right; j++)
		{
			cave_set_feat(i, j, FEAT_PERM_EXTRA);
			cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}
	for (i = y_depth; i >= y_depth - 5; i--)
	{
		for (j = x_left; j <= x_right; j++)
		{
			cave_set_feat(i, j, FEAT_PERM_EXTRA);
			cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}
	for (j = x_left; j <= x_left + 17; j++)
	{
		for (i = y_height; i <= y_depth; i++)
		{
			cave_set_feat(i, j, FEAT_PERM_EXTRA);
			cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}
	for (j = x_right; j >= x_right - 17; j--)
	{
		for (i = y_height; i <= y_depth; i++)
		{
			cave_set_feat(i, j, FEAT_PERM_EXTRA);
			cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}

	cave_set_feat(y_height + 6, x_left + 18, FEAT_PERM_EXTRA);
	cave[y_height + 6][x_left + 18].info |= (CAVE_GLOW | CAVE_MARK);
	cave_set_feat(y_depth - 6, x_left + 18, FEAT_PERM_EXTRA);
	cave[y_depth - 6][x_left + 18].info |= (CAVE_GLOW | CAVE_MARK);
	cave_set_feat(y_height + 6, x_right - 18, FEAT_PERM_EXTRA);
	cave[y_height + 6][x_right - 18].info |= (CAVE_GLOW | CAVE_MARK);
	cave_set_feat(y_depth - 6, x_right - 18, FEAT_PERM_EXTRA);
	cave[y_depth - 6][x_right - 18].info |= (CAVE_GLOW | CAVE_MARK);

	i = y_height + 5;
	j = xval + SCREEN_WID;
	cave_set_feat(i, j, FEAT_SHOP);
	cave[i][j].info |= (CAVE_GLOW | CAVE_MARK);
	player_place(i + 1, j);
}


/*
 * Town logic flow for generation of arena -KMW-
 */
static void arena_gen(void)
{
	int y, x;
	int qy = SCREEN_HGT;
	int qx = SCREEN_WID;
	bool_ daytime;

	/* Day time */
	if ((turn % (10L * DAY)) < ((10L * DAY) / 2))
		daytime = TRUE;

	/* Night time */
	else
		daytime = FALSE;

	/* Start with solid walls */
	for (y = 0; y < MAX_HGT; y++)
	{
		for (x = 0; x < MAX_WID; x++)
		{
			/* Create "solid" perma-wall */
			cave_set_feat(y, x, FEAT_PERM_SOLID);

			/* Illuminate and memorize the walls */
			cave[y][x].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}

	/* Then place some floors */
	for (y = qy + 1; y < qy + SCREEN_HGT - 1; y++)
	{
		for (x = qx + 1; x < qx + SCREEN_WID - 1; x++)
		{
			/* Create empty floor */
			cave_set_feat(y, x, FEAT_FLOOR);

			/* Darken and forget the floors */
			cave[y][x].info &= ~(CAVE_GLOW | CAVE_MARK);

			/* Day time */
			if (daytime)
			{
				/* Perma-Lite */
				cave[y][x].info |= (CAVE_GLOW);

				/* Memorize */
				if (view_perma_grids) cave[y][x].info |= (CAVE_MARK);
			}
		}
	}

	build_arena();

	place_monster_aux(p_ptr->py + 5, p_ptr->px, arena_monsters[p_ptr->arena_number],
	                  FALSE, FALSE, MSTATUS_ENEMY);
}


/*
 * Generate a quest level
 */
static void quest_gen(void)
{
	process_hooks(HOOK_GEN_QUEST, "(d)", is_quest(dun_level));
}

/*
 * Creates a special level
 */

/* Mega-Hack */
#define REGEN_HACK 0x02

bool_ build_special_level(void)
{
	char buf[80];
	int y, x, ystart = 2, xstart = 2;
	s16b level;

	/* No special levels on the surface */
	if (!dun_level) return FALSE;

	level = dun_level - d_info[dungeon_type].mindepth;
	if ((!get_dungeon_save(buf)) && (special_lvl[level][dungeon_type])) return FALSE;
	if (!get_dungeon_special(buf)) return FALSE;

	/* Big town */
	cur_hgt = MAX_HGT;
	cur_wid = MAX_WID;

	/* Determine number of panels */
	max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
	max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

	/* Assume illegal panel */
	panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
	panel_col_min = max_panel_cols * (SCREEN_WID / 2);

	/* Start with perm walls */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_set_feat(y, x, FEAT_PERM_SOLID);
		}
	}
	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON | INIT_POSITION;
	process_dungeon_file(buf, &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	special_lvl[level][dungeon_type] = REGEN_HACK;
	generate_special_feeling = TRUE;

	/* Special feeling because it's special */
	good_item_flag = TRUE;

	/*
	 * Hack -- It's better/more dangerous than a greater vault.
	 * Better to have a rating field in special level description.
	 */
	rating += 40;

	return TRUE;
}

/*
 * Prepare regeneration of a special level, which should not happen,
 * but just in case...
 */
static void wipe_special_level(void)
{
	s16b level;
	char buf[80];

	/* No special levels on the surface */
	if (!dun_level) return;

	process_hooks(HOOK_LEVEL_REGEN, "()");

	/* Calculate relative depth */
	level = dun_level - d_info[dungeon_type].mindepth;

	/* No special level at this depth */
	if ((!get_dungeon_save(buf)) &&
	                special_lvl[level][dungeon_type]) return;
	if (!get_dungeon_special(buf)) return;

	/* Clear the Mega-Hack flag */
	if (special_lvl[level][dungeon_type] == REGEN_HACK)
		special_lvl[level][dungeon_type] = FALSE;
}

/*
 * Finalise generation of a special level
 */
static void finalise_special_level(void)
{
	s16b level;
	char buf[80];

	/* No special levels on the surface */
	if (!dun_level) return;

	process_hooks(HOOK_LEVEL_END_GEN, "()");
	process_hooks_new(HOOK_LEVEL_END_GEN, NULL, NULL);

	/* Calculate relative depth */
	level = dun_level - d_info[dungeon_type].mindepth;

	/* No special level at this depth */
	if ((!get_dungeon_save(buf)) &&
	                special_lvl[level][dungeon_type]) return;
	if (!get_dungeon_special(buf)) return;

	/* Set the "generated" flag */
	if (special_lvl[level][dungeon_type] == REGEN_HACK)
		special_lvl[level][dungeon_type] = TRUE;
}

/*
 * Give some magical energy to the each grid of the level
 */
void generate_grid_mana()
{
	int y, x, mana, mult;
	bool_ xtra_magic = FALSE;

	if (randint(XTRA_MAGIC) == 1)
	{
		xtra_magic = TRUE;

		if (cheat_room || p_ptr->precognition)
		{
			msg_print("Magical level");
		}
	}

	mult = ((xtra_magic) ? 3 : 2);

	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			/* Calculate the amount of mana in each grid */
			mana = mult * m_bonus(255, dun_level) / 2;
			if (xtra_magic) mana += 10 + rand_int(10);

			/* Never more than 255 or less than 0(paranoia) */
			if (mana < 0) mana = 0;
			if (mana > 255) mana = 255;

			c_ptr->mana = mana;
		}
	}
}


/*
 * Generates a random dungeon level			-RAK-
 *
 * Hack -- regenerate any "overflow" levels
 *
 * Hack -- allow auto-scumming via a gameplay option.
 */
void generate_cave(void)
{
	dungeon_info_type *d_ptr = &d_info[dungeon_type];
	int tester_1, tester_2;
	int y, x, num, i;
	bool_ loaded = FALSE;
	char buf[80];
	s16b town_level = 0;

	/* The dungeon is not ready */
	character_dungeon = FALSE;
	generate_special_feeling = FALSE;

	/* Initialize the flags with the basic dungeon flags */
	if (!dun_level)
	{
		dungeon_flags1 = d_info[DUNGEON_WILDERNESS].flags1;
		dungeon_flags2 = d_info[DUNGEON_WILDERNESS].flags2;
	}
	else
	{
		dungeon_flags1 = d_ptr->flags1;
		dungeon_flags2 = d_ptr->flags2;
	}

	/* Is it a town level ? */
	for (i = 0; i < TOWN_DUNGEON; i++)
	{
		if (d_ptr->t_level[i] == dun_level) town_level = d_ptr->t_idx[i];
	}

	/* Save the imprinted monsters */
	save_all_friends();
	wipe_m_list();

	/* Seed the RNG if appropriate */
	if (town_level)
	{
		Rand_quick = TRUE;
		Rand_value = town_info[town_level].seed;
	}

	process_hooks(HOOK_GEN_LEVEL_BEGIN, "");

	/* Try to load a saved level */
	if (get_dungeon_save(buf))
	{
		/* No effects */
		for (i = 0; i < MAX_EFFECTS; i++)
		{
			effects[i].time = 0;
		}

		/* Start with a blank cave */
		for (y = 0; y < MAX_HGT; y++)
		{
			for (x = 0; x < MAX_WID; x++)
			{
				/* No flags */
				cave[y][x].info = 0;

				/* No features */
				cave_set_feat(y, x, FEAT_PERM_INNER);

				/* No objects */
				cave[y][x].o_idx = 0;

				/* No monsters */
				cave[y][x].m_idx = 0;

				/* No traps */
				cave[y][x].t_idx = 0;

				/* No mimic */
				cave[y][x].mimic = 0;

				/* No effects */
				cave[y][x].effect = 0;

				/* No inscription */
				cave[y][x].inscription = 0;

				/* No flow */
				cave[y][x].cost = 0;
				cave[y][x].when = 0;
			}
		}

		loaded = load_dungeon(buf);
	}

	/* No saved level -- generate new one */
	if (!loaded)
	{
		if (!get_dungeon_special(buf) ||
		                !special_lvl[dun_level - d_info[dungeon_type].mindepth][dungeon_type])
		{
			get_level_flags();
		}

		/* Generate */
		for (num = 0; TRUE; num++)
		{
			bool_ okay = TRUE;

			cptr why = NULL;

			/* No effects */
			for (i = 0; i < MAX_EFFECTS; i++)
			{
				effects[i].time = 0;
			}

			/* Start with a blank cave */
			for (y = 0; y < MAX_HGT; y++)
			{
				for (x = 0; x < MAX_WID; x++)
				{
					/* No flags */
					cave[y][x].info = 0;

					/* No features */
					cave_set_feat(y, x, FEAT_PERM_INNER);

					/* No objects */
					cave[y][x].o_idx = 0;

					/* No monsters */
					cave[y][x].m_idx = 0;

					/* No traps */
					cave[y][x].t_idx = 0;

					/* No mimic */
					cave[y][x].mimic = 0;

					/* No effect */
					cave[y][x].effect = 0;

					/* No inscription */
					cave[y][x].inscription = 0;

					/* No flow */
					cave[y][x].cost = 0;
					cave[y][x].when = 0;
				}
			}


			/* XXX XXX XXX XXX */
			o_max = 1;


			/* Mega-Hack -- no player yet */
			p_ptr->px = p_ptr->py = 0;


			/* Mega-Hack -- no panel yet */
			panel_row_min = 0;
			panel_row_max = 0;
			panel_col_min = 0;
			panel_col_max = 0;


			/* Reset the monster generation level */
			if (dungeon_type != DUNGEON_DEATH) monster_level = dun_level;
			else monster_level = (p_ptr->lev * 2) + 10 + rand_int(40);

			/* Reset the object generation level */
			object_level = dun_level;

			/* Nothing special here yet */
			good_item_flag = FALSE;

			/* Nothing good here yet */
			rating = 0;

			/* No ambush here yet */
			ambush_flag = FALSE;

			/* No fated level here yet */
			fate_flag = FALSE;

			/* Build the arena -KMW- */
			if (p_ptr->inside_arena)
			{
				/* Small arena */
				arena_gen();
			}

			/* Quest levels -KMW- */
			else if (p_ptr->inside_quest)
			{
				quest_gen();
			}

			/* Special levels */
			else if (build_special_level())
			{
				/* nothing */
			}

			/* Build the town */
			else if (!dun_level)
			{
				/* Big town */
				cur_hgt = MAX_HGT;
				cur_wid = MAX_WID;

				/* Determine number of panels */
				max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
				max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

				/* Assume illegal panel */
				panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
				panel_col_min = max_panel_cols * (SCREEN_WID / 2);

				/* Big wilderness mode */
				if (!p_ptr->wild_mode)
				{
					/* Make the wilderness */
					wilderness_gen(0);
				}

				/* Small wilderness mode */
				else
				{
					/* Make the wilderness */
					wilderness_gen_small();
				}


				okay = TRUE;
			}

			/* Build a dungeon level */
			else
			{
				/* Requested size level */
				if (d_ptr->size_x != -1)
				{
					if (cheat_room || p_ptr->precognition)
					{
						msg_print ("A 'size' dungeon level.");
					}

					cur_hgt = d_ptr->size_y * SCREEN_HGT;
					cur_wid = d_ptr->size_x * SCREEN_WID;

					/* Determine number of panels */
					max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
					max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

					/* Assume illegal panel */
					panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
					panel_col_min = max_panel_cols * (SCREEN_WID / 2);

					if (cheat_room)
					{
						msg_format("X:%d, Y:%d.", max_panel_cols, max_panel_rows);
					}
				}
				/* Very small (1 x 1 panel) level */
				else if (!(dungeon_flags1 & DF1_BIG) &&
				                (dungeon_flags1 & DF1_SMALLEST))
				{
					if (cheat_room || p_ptr->precognition)
					{
						msg_print ("A 'small' dungeon level.");
					}

					cur_hgt = SCREEN_HGT;
					cur_wid = SCREEN_WID;

					/* Determine number of panels */
					max_panel_rows = 1;
					max_panel_cols = 1;

					/* Assume illegal panel */
					panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
					panel_col_min = max_panel_cols * (SCREEN_WID / 2);

					if (cheat_room)
					{
						msg_format("X:1, Y:1.");
					}
				}

				/* Small level */
				else if (!(dungeon_flags1 & DF1_BIG) &&
				                (always_small_level ||
				                 (dungeon_flags1 & DF1_SMALL) ||
				                 (small_levels && rand_int(SMALL_LEVEL) == 0)))
				{
					if (cheat_room || p_ptr->precognition)
					{
						msg_print ("A 'small' dungeon level.");
					}

					tester_1 = rand_range(1, (MAX_HGT / SCREEN_HGT));
					tester_2 = rand_range(1, (MAX_WID / SCREEN_WID) - 1);

					cur_hgt = tester_1 * SCREEN_HGT;
					cur_wid = tester_2 * SCREEN_WID;

					/* Determine number of panels */
					max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
					max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

					/* Assume illegal panel */
					panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
					panel_col_min = max_panel_cols * (SCREEN_WID / 2);

					if (cheat_room)
					{
						msg_format("X:%d, Y:%d.", max_panel_cols, max_panel_rows);
					}
				}

				/* Normal level */
				else
				{
					/* Use full panels */
					cur_hgt = MAX_HGT;
					cur_wid = MAX_WID;

					/* Determine number of panels */
					max_panel_rows = (cur_hgt / SCREEN_HGT) * 2 - 2;
					max_panel_cols = (cur_wid / SCREEN_WID) * 2 - 2;

					/* Assume illegal panel */
					panel_row_min = max_panel_rows * (SCREEN_HGT / 2);
					panel_col_min = max_panel_cols * (SCREEN_WID / 2);
				}

				/* Generate a level */
				if (!cave_gen())
				{
					why = "could not place player";
					okay = FALSE;
				}
			}

			/* Extract the feeling */
			if (rating > 100) feeling = 2;
			else if (rating > 80) feeling = 3;
			else if (rating > 60) feeling = 4;
			else if (rating > 40) feeling = 5;
			else if (rating > 30) feeling = 6;
			else if (rating > 20) feeling = 7;
			else if (rating > 10) feeling = 8;
			else if (rating > 0) feeling = 9;
			else feeling = 10;

			/* Hack -- Have a special feeling sometimes */
			if (good_item_flag && !p_ptr->preserve) feeling = 1;

			/* It takes 1000 game turns for "feelings" to recharge */
			if ((turn - old_turn) < 1000) feeling = 0;

			/* Hack -- no feeling in the town */
			if (!dun_level) feeling = 0;


			/* Prevent object over-flow */
			if (o_max >= max_o_idx)
			{
				/* Message */
				why = "too many objects";

				/* Message */
				okay = FALSE;
			}

			/* Prevent monster over-flow */
			if (m_max >= max_m_idx)
			{
				/* Message */
				why = "too many monsters";

				/* Message */
				okay = FALSE;
			}

			/* Mega-Hack -- "auto-scum" */
			if (auto_scum && (num < 100) && !p_ptr->inside_quest && dun_level)
			{
				/* Require "goodness" */
				if ((feeling > 9) ||
				                ((dun_level >= 5) && (feeling > 8)) ||
				                ((dun_level >= 10) && (feeling > 7)) ||
				                ((dun_level >= 20) && (feeling > 6)) ||
				                ((dun_level >= 40) && (feeling > 5)))
				{
					/* Give message to cheaters */
					if (cheat_room || cheat_hear ||
					                cheat_peek || cheat_xtra || p_ptr->precognition)
					{
						/* Message */
						why = "boring level";
					}

					/* Try again */
					okay = FALSE;
				}
			}

			/* Accept */
			if (okay || town_level) break;

			/* Message */
			if (why) msg_format("Generation restarted (%s)", why);

			/* Wipe the objects */
			wipe_o_list();

			/* Wipe the monsters */
			wipe_m_list();

			/* Clear the fate icky flags */
			for (i = 0; i < MAX_FATES; i++) fates[i].icky = FALSE;

			/*
			 * Mega-Hack -- Reset special level flag if necessary
			 * XXX XXX XXX
			 */
			wipe_special_level();
		}

		/* Give some mana to each grid -- DG */
		generate_grid_mana();
	}

	/* Put the kept monsters -- DG */
	if (!p_ptr->wild_mode) replace_all_friends();

	/* Hack -- Clear used up fates */
	for (i = 0; i < MAX_FATES; i++)
	{
		if (fates[i].icky)
		{
			/* Mark the artefact as generated */
			if ((fates[i].fate == FATE_FIND_A) && fates[i].a_idx)
			{
				a_info[fates[i].a_idx].cur_num = 1;
			}
			fates[i].fate = FATE_NONE;
			fates[i].icky = FALSE;
		}
	}

	/* Set special level generated flag if applicable */
	finalise_special_level();

	/* No teleporatations yet */
	last_teleportation_y = -1;
	last_teleportation_x = -1;

	/* Mark the dungeon town as found */
	if (town_level)
	{
		/* Set the known flag */
		town_info[town_level].flags |= (TOWN_KNOWN);
	}

	/* The dungeon is ready */
	character_dungeon = TRUE;

	/* Remember when this level was "created" */
	old_turn = turn;

	/* Provide astral chars with the full map */
	if (p_ptr->astral && dun_level)
	{
		wiz_lite_extra();
	}

	/* Player should get the first move upon entering the dungeon */
	p_ptr->energy = 100;
}
