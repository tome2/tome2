/* File: generate.c */

/* Purpose: Wilderness & Town related things */

/*
 * Copyright (c) 2001 James E. Wilson, Robert A. Koeneke, DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"



/*
 * Various defines for the wilderness
 */
#define DUN_WILD_VAULT          50      /* Chance of finding a wilderness vault. */

/*
 * Various defines for the towns
 */
#define TOWN_NORMAL_FLOOR       70
#define TOWN_BORDER             90


/*
 * Helper for plasma generation.
 */
static void perturb_point_mid(int x1, int x2, int x3, int x4,
                              int xmid, int ymid, int rough, int depth_max)
{
	/*
	 * Average the four corners & perturb it a bit.
	 * tmp is a random int +/- rough
	 */
	int tmp2 = rough * 2 + 1;
	int tmp = randint(tmp2) - (rough + 1);

	int avg = ((x1 + x2 + x3 + x4) / 4) + tmp;

	/* Division always rounds down, so we round up again */
	if (((x1 + x2 + x3 + x4) % 4) > 1) avg++;

	/* Normalize */
	if (avg < 0) avg = 0;
	if (avg > depth_max) avg = depth_max;

	/* Set the new value. */
	cave[ymid][xmid].feat = (byte)avg;
}


static void perturb_point_end(int x1, int x2, int x3,
                              int xmid, int ymid, int rough, int depth_max)
{
	/*
	 * Average the three corners & perturb it a bit.
	 * tmp is a random int +/- rough
	 */
	int tmp2 = rough * 2 + 1;
	int tmp = randint(tmp2) - (rough + 1);

	int avg = ((x1 + x2 + x3) / 3) + tmp;

	/* Division always rounds down, so we round up again */
	if ((x1 + x2 + x3) % 3) avg++;

	/* Normalize */
	if (avg < 0) avg = 0;
	if (avg > depth_max) avg = depth_max;

	/* Set the new value. */
	cave[ymid][xmid].feat = (byte)avg;
}


/*
 * A generic function to generate the plasma fractal.
 * Note that it uses ``cave_feat'' as temporary storage.
 * The values in ``cave_feat'' after this function
 * are NOT actual features; They are raw heights which
 * need to be converted to features.
 *
 * So we shouldn't call cave_set_feat in the helper functions
 * above.
 */
static void plasma_recursive(int x1, int y1, int x2, int y2,
                             int depth_max, int rough)
{
	/* Find middle */
	int xmid = (x2 - x1) / 2 + x1;
	int ymid = (y2 - y1) / 2 + y1;

	/* Are we done? */
	if (x1 + 1 == x2) return;

	perturb_point_mid(cave[y1][x1].feat, cave[y2][x1].feat, cave[y1][x2].feat,
	                  cave[y2][x2].feat, xmid, ymid, rough, depth_max);

	perturb_point_end(cave[y1][x1].feat, cave[y1][x2].feat, cave[ymid][xmid].feat,
	                  xmid, y1, rough, depth_max);

	perturb_point_end(cave[y1][x2].feat, cave[y2][x2].feat, cave[ymid][xmid].feat,
	                  x2, ymid, rough, depth_max);

	perturb_point_end(cave[y2][x2].feat, cave[y2][x1].feat, cave[ymid][xmid].feat,
	                  xmid, y2, rough, depth_max);

	perturb_point_end(cave[y2][x1].feat, cave[y1][x1].feat, cave[ymid][xmid].feat,
	                  x1, ymid, rough, depth_max);


	/* Recurse the four quadrants */
	plasma_recursive(x1, y1, xmid, ymid, depth_max, rough);
	plasma_recursive(xmid, y1, x2, ymid, depth_max, rough);
	plasma_recursive(x1, ymid, xmid, y2, depth_max, rough);
	plasma_recursive(xmid, ymid, x2, y2, depth_max, rough);
}


/*
 * Load a town or generate a terrain level using "plasma" fractals.
 *
 * x and y are the coordinates of the area in the wilderness.
 * Border and corner are optimization flags to speed up the
 * generation of the fractal terrain.
 * If border is set then only the border of the terrain should
 * be generated (for initializing the border structure).
 * If corner is set then only the corners of the area are needed.
 *
 * Return the number of floor grids
 */
int generate_area(int y, int x, bool_ border, bool_ corner, bool_ refresh)
{
	int road, entrance;
	int x1, y1;
	int hack_floor = 0;

	/* Number of the town (if any) */
	p_ptr->town_num = wf_info[wild_map[y][x].feat].entrance;
	if (!p_ptr->town_num) p_ptr->town_num = wild_map[y][x].entrance;

	{
		int roughness = 1;  /* The roughness of the level. */
		int terrain[3][3];  /* The terrain around the current area */
		int ym, xm, yp, xp;

		/* Place the player at the center */
		if (!p_ptr->oldpx) p_ptr->oldpx = MAX_WID / 2;
		if (!p_ptr->oldpy) p_ptr->oldpy = MAX_HGT / 2;

		/* Initialize the terrain array */
		ym = ((y - 1) < 0) ? 0 : (y - 1);
		xm = ((x - 1) < 0) ? 0 : (x - 1);
		yp = ((y + 1) >= max_wild_y) ? (max_wild_y - 1) : (y + 1);
		xp = ((x + 1) >= max_wild_x) ? (max_wild_x - 1) : (x + 1);
		terrain[0][0] = wild_map[ym][xm].feat;
		terrain[0][1] = wild_map[ym][x].feat;
		terrain[0][2] = wild_map[ym][xp].feat;
		terrain[1][0] = wild_map[y][xm].feat;
		terrain[1][1] = wild_map[y][x].feat;
		terrain[1][2] = wild_map[y][xp].feat;
		terrain[2][0] = wild_map[yp][xm].feat;
		terrain[2][1] = wild_map[yp][x].feat;
		terrain[2][2] = wild_map[yp][xp].feat;

		/* Hack -- Use the "simple" RNG */
		Rand_quick = TRUE;

		/* Hack -- Induce consistant town layout */
		Rand_value = wild_map[y][x].seed;

		if (!corner)
		{
			/* Create level background */
			for (y1 = 0; y1 < MAX_HGT; y1++)
			{
				for (x1 = 0; x1 < MAX_WID; x1++)
				{
					cave_set_feat(y1, x1, MAX_WILD_TERRAIN / 2);
				}
			}
		}

		/*
		 * Initialize the four corners
		 * ToDo: calculate the medium height of the adjacent
		 * terrains for every corner.
		 */
		cave_set_feat(1, 1, (byte)rand_int(MAX_WILD_TERRAIN));
		cave_set_feat(MAX_HGT - 2, 1, (byte)rand_int(MAX_WILD_TERRAIN));
		cave_set_feat(1, MAX_WID - 2, (byte)rand_int(MAX_WILD_TERRAIN));
		cave_set_feat(MAX_HGT - 2, MAX_WID - 2, (byte)rand_int(MAX_WILD_TERRAIN));

		if (!corner)
		{
			/* x1, y1, x2, y2, num_depths, roughness */
			plasma_recursive(1, 1, MAX_WID - 2, MAX_HGT - 2, MAX_WILD_TERRAIN - 1, roughness);
		}

		/* Use the complex RNG */
		Rand_quick = FALSE;

		for (y1 = 1; y1 < MAX_HGT - 1; y1++)
		{
			for (x1 = 1; x1 < MAX_WID - 1; x1++)
			{
				cave_set_feat(y1, x1,
				              wf_info[terrain[1][1]].terrain[cave[y1][x1].feat]);
			}
		}

	}

	/* Should we create a town ? */
	if ((p_ptr->town_num > 0) && (p_ptr->town_num < 1000))
	{
		/* Create the town */
		int xstart = 0;
		int ystart = 0;

		/* Initialize the town */
		init_flags = INIT_CREATE_DUNGEON;
		process_dungeon_file("t_info.txt", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	}
	else
	{
		/* Reset the town flag */
		p_ptr->town_num = 0;
	}

	if (!corner)
	{
		/*
		 * Place roads in the wilderness
		 * ToDo: make the road a bit more interresting
		 */
		road = wf_info[wild_map[y][x].feat].road;

		if (road & ROAD_NORTH)
		{
			/* North road */
			for (y1 = 1; y1 < MAX_HGT / 2; y1++)
			{
				x1 = MAX_WID / 2;
				cave_set_feat(y1, x1, FEAT_FLOOR);
			}
		}

		if (road & ROAD_SOUTH)
		{
			/* North road */
			for (y1 = MAX_HGT / 2; y1 < MAX_HGT - 1; y1++)
			{
				x1 = MAX_WID / 2;
				cave_set_feat(y1, x1, FEAT_FLOOR);
			}
		}

		if (road & ROAD_EAST)
		{
			/* East road */
			for (x1 = MAX_WID / 2; x1 < MAX_WID - 1; x1++)
			{
				y1 = MAX_HGT / 2;
				cave_set_feat(y1, x1, FEAT_FLOOR);
			}
		}

		if (road & ROAD_WEST)
		{
			/* West road */
			for (x1 = 1; x1 < MAX_WID / 2; x1++)
			{
				y1 = MAX_HGT / 2;
				cave_set_feat(y1, x1, FEAT_FLOOR);
			}
		}
	}

	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant town layout */
	Rand_value = wild_map[y][x].seed;

	entrance = wf_info[wild_map[y][x].feat].entrance;
	if (!entrance) entrance = wild_map[y][x].entrance;

	/* Create the dungeon if requested on the map */
	if (entrance >= 1000)
	{
		int dy, dx;

		dy = rand_range(6, cur_hgt - 6);
		dx = rand_range(6, cur_wid - 6);

		cave_set_feat(dy, dx, FEAT_MORE);
		cave[dy][dx].special = entrance - 1000;
		cave[dy][dx].info |= (CAVE_GLOW | CAVE_MARK);
	}

	/* Use the complex RNG */
	Rand_quick = FALSE;

	/* MEGA HACK -- set at least one floor grid */
	for (y1 = 1; y1 < cur_hgt - 1; y1++)
	{
		for (x1 = 1; x1 < cur_wid - 1; x1++)
		{
			if (cave_floor_bold(y1, x1)) hack_floor++;
		}
	}

	/* NO floor ? put one */
	if (!hack_floor)
	{
		cave_set_feat(cur_hgt / 2, cur_wid / 2, FEAT_GRASS);
		cave[cur_hgt / 2][cur_wid / 2].special = 0;
		hack_floor = 1;
	}

	/* Set the monster generation level to the wilderness level */
	monster_level = wf_info[wild_map[y][x].feat].level;

	/* Set the object generation level to the wilderness level */
	object_level = wf_info[wild_map[y][x].feat].level;

	return hack_floor;
}

/*
 * Border of the wilderness area
 */
static border_type border;

/*
 * Build the wilderness area outside of the town.
 * -KMW-
 */
void wilderness_gen(int refresh)
{
	int i, y, x, hack_floor;
	bool_ daytime;
	int xstart = 0;
	int ystart = 0;
	cave_type *c_ptr;

	/* Init the wilderness */
	process_dungeon_file("w_info.txt", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);

	x = p_ptr->wilderness_x;
	y = p_ptr->wilderness_y;

	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	/* North border */
	generate_area(y - 1, x, TRUE, FALSE, refresh);

	for (i = 1; i < MAX_WID - 1; i++)
	{
		border.north[i] = cave[MAX_HGT - 2][i].feat;
	}

	/* South border */
	generate_area(y + 1, x, TRUE, FALSE, refresh);

	for (i = 1; i < MAX_WID - 1; i++)
	{
		border.south[i] = cave[1][i].feat;
	}

	/* West border */
	generate_area(y, x - 1, TRUE, FALSE, refresh);

	for (i = 1; i < MAX_HGT - 1; i++)
	{
		border.west[i] = cave[i][MAX_WID - 2].feat;
	}

	/* East border */
	generate_area(y, x + 1, TRUE, FALSE, refresh);

	for (i = 1; i < MAX_HGT - 1; i++)
	{
		border.east[i] = cave[i][1].feat;
	}

	/* North west corner */
	generate_area(y - 1, x - 1, FALSE, TRUE, refresh);
	border.north_west = cave[MAX_HGT - 2][MAX_WID - 2].feat;

	/* North east corner */
	generate_area(y - 1, x + 1, FALSE, TRUE, refresh);
	border.north_east = cave[MAX_HGT - 2][1].feat;

	/* South west corner */
	generate_area(y + 1, x - 1, FALSE, TRUE, refresh);
	border.south_west = cave[1][MAX_WID - 2].feat;

	/* South east corner */
	generate_area(y + 1, x + 1, FALSE, TRUE, refresh);
	border.south_east = cave[1][1].feat;


	/* Create terrain of the current area */
	hack_floor = generate_area(y, x, FALSE, FALSE, refresh);


	/* Special boundary walls -- North */
	for (i = 0; i < MAX_WID; i++)
	{
		cave[0][i].mimic = border.north[i];
		cave_set_feat(0, i, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- South */
	for (i = 0; i < MAX_WID; i++)
	{
		cave[MAX_HGT - 1][i].mimic = border.south[i];
		cave_set_feat(MAX_HGT - 1, i, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- West */
	for (i = 0; i < MAX_HGT; i++)
	{
		cave[i][0].mimic = border.west[i];
		cave_set_feat(i, 0, FEAT_PERM_SOLID);
	}

	/* Special boundary walls -- East */
	for (i = 0; i < MAX_HGT; i++)
	{
		cave[i][MAX_WID - 1].mimic = border.east[i];
		cave_set_feat(i, MAX_WID - 1, FEAT_PERM_SOLID);
	}

	/* North west corner */
	cave[0][0].mimic = border.north_west;

	/* Make sure it has correct CAVE_WALL flag set */
	cave_set_feat(0, 0, cave[0][0].feat);

	/* North east corner */
	cave[0][MAX_WID - 1].mimic = border.north_east;

	/* Make sure it has correct CAVE_WALL flag set */
	cave_set_feat(0, MAX_WID - 1, cave[0][MAX_WID - 1].feat);

	/* South west corner */
	cave[MAX_HGT - 1][0].mimic = border.south_west;

	/* Make sure it has correct CAVE_WALL flag set */
	cave_set_feat(MAX_HGT - 1, 0, cave[MAX_HGT - 1][0].feat);

	/* South east corner */
	cave[MAX_HGT - 1][MAX_WID - 1].mimic = border.south_east;

	/* Make sure it has correct CAVE_WALL flag set */
	cave_set_feat(MAX_HGT - 1, MAX_WID - 1, cave[MAX_HGT - 1][MAX_WID - 1].feat);


	/* Day time */
	if ((turn % (10L * DAY)) < ((10L * DAY) / 2))
		daytime = TRUE;
	else
		daytime = FALSE;

	/* Light up or darken the area */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			/* Get the cave grid */
			c_ptr = &cave[y][x];

			if (daytime)
			{
				/* Assume lit */
				c_ptr->info |= (CAVE_GLOW);

				/* Hack -- Memorize lit grids if allowed */
				if (view_perma_grids) c_ptr->info |= (CAVE_MARK);
			}
			else
			{
				/* Darken "boring" features */
				if (!(f_info[c_ptr->feat].flags1 & FF1_REMEMBER))
				{
					/* Forget the grid */
					c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);
				}
			}
		}
	}

	player_place(p_ptr->oldpy, p_ptr->oldpx);

	if (!refresh)
	{
		int lim = (generate_encounter == TRUE) ? 60 : MIN_M_ALLOC_TN;

		/*
		 * Can't have more monsters than floor grids -1(for the player,
		 * not needed but safer
		 */
		if (lim > hack_floor - 1) lim = hack_floor - 1;

		/* Make some residents */
		for (i = 0; i < lim; i++)
		{
			/* Make a resident */
			(void)alloc_monster((generate_encounter == TRUE) ? 0 : 3, (generate_encounter == TRUE) ? FALSE : TRUE);
		}

		if (generate_encounter) ambush_flag = TRUE;
		generate_encounter = FALSE;
	}

	/* Set rewarded quests to finished */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		if (quest[i].status == QUEST_STATUS_REWARDED)
		{
			quest[i].status = QUEST_STATUS_FINISHED;
		}
	}

	process_hooks(HOOK_WILD_GEN, "(d)", FALSE);
}

/*
 * Build the wilderness area.
 * -DG-
 */
void wilderness_gen_small()
{
	int i, j, entrance;
	int xstart = 0;
	int ystart = 0;

	/* To prevent stupid things */
	for (i = 0; i < MAX_WID; i++)
	{
		for (j = 0; j < MAX_HGT; j++)
		{
			cave_set_feat(j, i, FEAT_EKKAIA);
		}
	}

	/* Init the wilderness */
	process_dungeon_file("w_info.txt", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);

	/* Fill the map */
	for (i = 0; i < max_wild_x; i++)
	{
		for (j = 0; j < max_wild_y; j++)
		{
			entrance = wf_info[wild_map[j][i].feat].entrance;
			if (!entrance) entrance = wild_map[j][i].entrance;

			if (wild_map[j][i].entrance)
			{
				cave_set_feat(j, i, FEAT_MORE);
			}
			else
			{
				cave_set_feat(j, i, wf_info[wild_map[j][i].feat].feat);
			}

			if ((cave[j][i].feat == FEAT_MORE) && (entrance >= 1000))
			{
				cave[j][i].special = entrance - 1000;
			}

			/* Show it if we know it */
			if (wild_map[j][i].known)
			{
				cave[j][i].info |= (CAVE_GLOW | CAVE_MARK);
			}
		}
	}

	/* Place the player */
	p_ptr->px = p_ptr->wilderness_x;
	p_ptr->py = p_ptr->wilderness_y;

	/* Set rewarded quests to finished */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		if (quest[i].status == QUEST_STATUS_REWARDED)
		{
			quest[i].status = QUEST_STATUS_FINISHED;
		}
	}

	process_hooks(HOOK_WILD_GEN, "(d)", TRUE);
}

/* Show a small radius of wilderness around the player */
void reveal_wilderness_around_player(int y, int x, int h, int w)
{
	int i, j;

	/* Circle or square ? */
	if (h == 0)
	{
		for (i = x - w; i < x + w; i++)
		{
			for (j = y - w; j < y + w; j++)
			{
				/* Bound checking */
				if (!in_bounds(j, i)) continue;

				/* Severe bound checking */
				if ((i < 0) || (i >= max_wild_x) || (j < 0) || (j >= max_wild_y)) continue;

				/* We want a radius, not a "squarus" :) */
				if (distance(y, x, j, i) >= w) continue;

				/* New we know here */
				wild_map[j][i].known = TRUE;

				/* Only if we are in overview */
				if (p_ptr->wild_mode)
				{
					cave[j][i].info |= (CAVE_GLOW | CAVE_MARK);

					/* Show it */
					lite_spot(j, i);
				}
			}
		}
	}
	else
	{
		for (i = x; i < x + w; i++)
		{
			for (j = y; j < y + h; j++)
			{
				/* Bound checking */
				if (!in_bounds(j, i)) continue;

				/* New we know here */
				wild_map[j][i].known = TRUE;

				/* Only if we are in overview */
				if (p_ptr->wild_mode)
				{
					cave[j][i].info |= (CAVE_GLOW | CAVE_MARK);

					/* Show it */
					lite_spot(j, i);
				}
			}
		}
	}
}

/*
 * Builds a store at a given pseudo-location
 *
 * As of 2.8.1 (?) the town is actually centered in the middle of a
 * complete level, and thus the top left corner of the town itself
 * is no longer at (0,0), but rather, at (qy,qx), so the constants
 * in the comments below should be mentally modified accordingly.
 *
 * As of 2.7.4 (?) the stores are placed in a more "user friendly"
 * configuration, such that the four "center" buildings always
 * have at least four grids between them, to allow easy running,
 * and the store doors tend to face the middle of town.
 *
 * The stores now lie inside boxes from 3-9 and 12-18 vertically,
 * and from 7-17, 21-31, 35-45, 49-59.  Note that there are thus
 * always at least 2 open grids between any disconnected walls.
 *
 * Note the use of "town_illuminate()" to handle all "illumination"
 * and "memorization" issues.
 */
static void build_store(int qy, int qx, int n, int yy, int xx)
{
	int y, x, y0, x0, y1, x1, y2, x2, tmp;

	/* Find the "center" of the store */
	y0 = qy + yy * 9 + 6;
	x0 = qx + xx * 14 + 12;

	/* Determine the store boundaries */
	y1 = y0 - randint((yy == 0) ? 3 : 2);
	y2 = y0 + randint((yy == 1) ? 3 : 2);
	x1 = x0 - randint(5);
	x2 = x0 + randint(5);

	/* Build an invulnerable rectangular building */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			/* Create the building */
			cave_set_feat(y, x, FEAT_PERM_EXTRA);
		}
	}

	/* Pick a door direction (S,N,E,W) */
	tmp = rand_int(4);

	/* Re-roll "annoying" doors */
	if (((tmp == 0) && (yy == 1)) ||
	                ((tmp == 1) && (yy == 0)) ||
	                ((tmp == 2) && (xx == 3)) ||
	                ((tmp == 3) && (xx == 0)))
	{
		/* Pick a new direction */
		tmp = rand_int(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
	case 0:
		{
			y = y2;
			x = rand_range(x1, x2);
			break;
		}

		/* Top side */
	case 1:
		{
			y = y1;
			x = rand_range(x1, x2);
			break;
		}

		/* Right side */
	case 2:
		{
			y = rand_range(y1, y2);
			x = x2;
			break;
		}

		/* Left side */
	default:
		{
			y = rand_range(y1, y2);
			x = x1;
			break;
		}
	}

	/* Clear previous contents, add a store door */
	cave_set_feat(y, x, FEAT_SHOP);
	cave[y][x].special = n;
	cave[y][x].info |= CAVE_FREE;
}

static void build_store_circle(int qy, int qx, int n, int yy, int xx)
{
	int tmp, y, x, y0, x0, rad = 2 + rand_int(2);

	/* Find the "center" of the store */
	y0 = qy + yy * 9 + 6;
	x0 = qx + xx * 14 + 12;

	/* Determine the store boundaries */

	/* Build an invulnerable circular building */
	for (y = y0 - rad; y <= y0 + rad; y++)
	{
		for (x = x0 - rad; x <= x0 + rad; x++)
		{
			if (distance(y0, x0, y, x) > rad) continue;

			/* Create the building */
			cave_set_feat(y, x, FEAT_PERM_EXTRA);
		}
	}

	/* Pick a door direction (S,N,E,W) */
	tmp = rand_int(4);

	/* Re-roll "annoying" doors */
	if (((tmp == 0) && (yy == 1)) ||
	                ((tmp == 1) && (yy == 0)) ||
	                ((tmp == 2) && (xx == 3)) ||
	                ((tmp == 3) && (xx == 0)))
	{
		/* Pick a new direction */
		tmp = rand_int(4);
	}

	/* Extract a "door location" */
	switch (tmp)
	{
		/* Bottom side */
	case 0:
		{
			for (y = y0; y <= y0 + rad; y++) cave_set_feat(y, x0, FEAT_FLOOR);
			break;
		}

		/* Top side */
	case 1:
		{
			for (y = y0 - rad; y <= y0; y++) cave_set_feat(y, x0, FEAT_FLOOR);
			break;
		}

		/* Right side */
	case 2:
		{
			for (x = x0; x <= x0 + rad; x++) cave_set_feat(y0, x, FEAT_FLOOR);
			break;
		}

		/* Left side */
	default:
		{
			for (x = x0 - rad; x <= x0; x++) cave_set_feat(y0, x, FEAT_FLOOR);
			break;
		}
	}

	/* Clear previous contents, add a store door */
	cave_set_feat(y0, x0, FEAT_SHOP);
	cave[y0][x0].special = n;
	cave[y0][x0].info |= CAVE_FREE;
}

static void build_store_hidden(int n, int yy, int xx)
{
	/* Clear previous contents, add a store door */
	cave_set_feat(yy, xx, FEAT_SHOP);
	cave[yy][xx].special = n;
	cave[yy][xx].info |= CAVE_FREE;
}

/* Return a list of stores */
static int get_shops(int *rooms)
{
	int i, n, num = 0;

	for (n = 0; n < max_st_idx; n++)
	{
		rooms[n] = 0;
	}

	for (n = 0; n < max_st_idx; n++)
	{
		int chance = 50;

		if (st_info[n].flags1 & SF1_COMMON) chance += 30;
		if (st_info[n].flags1 & SF1_RARE) chance -= 20;
		if (st_info[n].flags1 & SF1_VERY_RARE) chance -= 30;

		if (!magik(chance)) continue;

		for (i = 0; i < num; ++i)
			if (rooms[i] == n)
				continue;

		if (st_info[n].flags1 & SF1_RANDOM) rooms[num++] = n;
	}

	return num;
}

/* Generate town borders */
static void set_border(int y, int x)
{
	cave_type *c_ptr;

	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Was a floor */
	if (cave_floor_bold(y, x) ||
	                (f_info[cave[y][x].feat].flags1 & FF1_DOOR))
	{
		cave_set_feat(y, x, FEAT_DOOR_HEAD);
	}

	/* Was a wall */
	else
	{
		cave_set_feat(y, x, FEAT_PERM_SOLID);

	}

	/* Access grid */
	c_ptr = &cave[y][x];

	/* Clear special attrs */
	c_ptr->mimic = 0;
	c_ptr->special = 0;
	c_ptr->info |= (CAVE_ROOM);
}


static void town_borders(int t_idx, int qy, int qx)
{
	int y, x;

	x = qx;
	for (y = qy; y < qy + SCREEN_HGT - 1; y++)
	{
		set_border(y, x);
	}

	x = qx + SCREEN_WID - 1;
	for (y = qy; y < qy + SCREEN_HGT - 1; y++)
	{
		set_border(y, x);
	}

	y = qy;
	for (x = qx; x < qx + SCREEN_WID - 1; x++)
	{
		set_border(y, x);
	}

	y = qy + SCREEN_HGT - 1;
	for (x = qx; x < qx + SCREEN_WID; x++)
	{
		set_border(y, x);
	}
}

static bool_ create_townpeople_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->d_char == 't') return TRUE;
	else return FALSE;
}


/*
 * Generate the "consistent" town features, and place the player
 *
 * Hack -- play with the R.N.G. to always yield the same town
 * layout, including the size and shape of the buildings, the
 * locations of the doorways, and the location of the stairs.
 */
static void town_gen_hack(int t_idx, int qy, int qx)
{
	int y, x, floor, num = 0;
	bool_ (*old_get_mon_num_hook)(int r_idx);

	int *rooms;

	/* Do we use dungeon floor or normal one */
	if (magik(TOWN_NORMAL_FLOOR)) floor = FEAT_FLOOR;
	else floor = 0;

	/* Place some floors */
	for (y = qy + 1; y < qy + SCREEN_HGT - 1; y++)
	{
		for (x = qx + 1; x < qx + SCREEN_WID - 1; x++)
		{
			/* Create empty floor */
			cave_set_feat(y, x, (floor) ? floor : floor_type[rand_int(100)]);
			cave[y][x].info |= (CAVE_ROOM | CAVE_FREE);
		}
	}

	/* Prepare an Array of "remaining stores", and count them */
	C_MAKE(rooms, max_st_idx, int);
	num = get_shops(rooms);

	/* Place two rows of stores */
	for (y = 0; y < 2; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 4; x++)
		{
			if(--num > -1)
			{
				/* Build that store at the proper location */
				build_store(qy, qx, rooms[num], y, x);
			}
		}
	}
	C_FREE(rooms, max_st_idx, int);

	/* Generates the town's borders */
	if (magik(TOWN_NORMAL_FLOOR)) town_borders(t_idx, qy, qx);


	/* Some inhabitants(leveled .. hehe :) */

	/* Backup the old hook */
	old_get_mon_num_hook = get_mon_num_hook;

	/* Require "okay" monsters */
	get_mon_num_hook = create_townpeople_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	for (x = qx; x < qx + SCREEN_WID; x++)
		for (y = qy; y < qy + SCREEN_HGT; y++)
		{
			int m_idx, r_idx;

			/* Only in town */
			if (!in_bounds(y, x)) continue;
			if (!(cave[y][x].info & CAVE_FREE)) continue;
			if (!cave_empty_bold(y, x)) continue;

			if (rand_int(100)) continue;

			r_idx = get_mon_num(0);
			m_allow_special[r_idx] = TRUE;
			m_idx = place_monster_one(y, x, r_idx, 0, TRUE, MSTATUS_ENEMY);
			m_allow_special[r_idx] = FALSE;

			if (m_idx)
			{
				monster_type *m_ptr = &m_list[m_idx];
				if (m_ptr->level < (dun_level / 2))
				{
					m_ptr->exp = MONSTER_EXP(m_ptr->level + (dun_level / 2) + randint(dun_level / 2));
					monster_check_experience(m_idx, TRUE);
				}
			}
		}

	/* Reset restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();
}

static void town_gen_circle(int t_idx, int qy, int qx)
{
	int y, x, cy, cx, rad, floor, num = 0;
	bool_ (*old_get_mon_num_hook)(int r_idx);

	int *rooms;

	/* Do we use dungeon floor or normal one */
	if (magik(TOWN_NORMAL_FLOOR)) floor = FEAT_FLOOR;
	else floor = 0;

	rad = (SCREEN_HGT / 2);

	y = qy;
	for (x = qx + rad; x < qx + SCREEN_WID - rad; x++)
	{
		set_border(y, x);
	}

	y = qy + SCREEN_HGT - 1;
	for (x = qx + rad; x < qx + SCREEN_WID - rad; x++)
	{
		set_border(y, x);
	}
	/* Place some floors */
	for (y = qy + 1; y < qy + SCREEN_HGT - 1; y++)
	{
		for (x = qx + rad; x < qx + SCREEN_WID - rad; x++)
		{
			/* Create empty floor */
			cave_set_feat(y, x, (floor) ? floor : floor_type[rand_int(100)]);
			cave[y][x].info |= CAVE_ROOM | CAVE_FREE;
		}
	}

	cy = qy + (SCREEN_HGT / 2);

	cx = qx + rad;
	for (y = cy - rad; y < cy + rad; y++)
		for (x = cx - rad; x < cx + 1; x++)
		{
			int d = distance(cy, cx, y, x);

			if ((d == rad) || (d == rad - 1)) set_border(y, x);

			if (d < rad - 1)
			{
				cave_set_feat(y, x, (floor) ? floor : floor_type[rand_int(100)]);
				cave[y][x].info |= CAVE_ROOM | CAVE_FREE;
			}
		}

	cx = qx + SCREEN_WID - rad - 1;
	for (y = cy - rad; y < cy + rad; y++)
		for (x = cx; x < cx + rad + 1; x++)
		{
			int d = distance(cy, cx, y, x);

			if ((d == rad) || (d == rad - 1)) set_border(y, x);

			if (d < rad - 1)
			{
				cave_set_feat(y, x, (floor) ? floor : floor_type[rand_int(100)]);
				cave[y][x].info |= CAVE_ROOM | CAVE_FREE;
			}
		}

	/* Prepare an Array of "remaining stores", and count them */
	C_MAKE(rooms, max_st_idx, int);
	num = get_shops(rooms);

	/* Place two rows of stores */
	for (y = 0; y < 2; y++)
	{
		/* Place four stores per row */
		for (x = 0; x < 4; x++)
		{
			if(--num > -1)
			{
				/* Build that store at the proper location */
				build_store_circle(qy, qx, rooms[num], y, x);
			}
		}
	}
	C_FREE(rooms, max_st_idx, int);

	/* Some inhabitants(leveled .. hehe :) */

	/* Backup the old hook */
	old_get_mon_num_hook = get_mon_num_hook;

	/* Require "okay" monsters */
	get_mon_num_hook = create_townpeople_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	for (x = qx; x < qx + SCREEN_WID; x++)
		for (y = qy; y < qy + SCREEN_HGT; y++)
		{
			int m_idx, r_idx;

			/* Only in town */
			if (!in_bounds(y, x)) continue;
			if (!(cave[y][x].info & CAVE_FREE)) continue;
			if (!cave_empty_bold(y, x)) continue;

			if (rand_int(100)) continue;

			r_idx = get_mon_num(0);
			m_allow_special[r_idx] = TRUE;
			m_idx = place_monster_one(y, x, r_idx, 0, TRUE, MSTATUS_ENEMY);
			m_allow_special[r_idx] = FALSE;
			if (m_idx)
			{
				monster_type *m_ptr = &m_list[m_idx];
				if (m_ptr->level < (dun_level / 2))
				{
					m_ptr->exp = MONSTER_EXP(m_ptr->level + (dun_level / 2) + randint(dun_level / 2));
					monster_check_experience(m_idx, TRUE);
				}
			}
		}

	/* Reset restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();
}


static void town_gen_hidden(int t_idx, int qy, int qx)
{
	int y, x, n, num = 0, i;

	int *rooms;

	/* Prepare an Array of "remaining stores", and count them */
	C_MAKE(rooms, max_st_idx, int);
	num = get_shops(rooms);

	/* Get a number of stores to place */
	n = rand_int(num / 2) + (num / 2);

	/* Place k stores */
	for (i = 0; i < n; i++)
	{
		/* Find a good spot */
		while (TRUE)
		{
			y = rand_range(1, cur_hgt - 2);
			x = rand_range(1, cur_wid - 2);

			if (cave_empty_bold(y, x)) break;
		}

		if(--num > -1)
		{
			/* Build that store at the proper location */
			build_store_hidden(rooms[num], y, x);
		}
	}
	C_FREE(rooms, max_st_idx, int);
}



/*
 * Town logic flow for generation of new town
 *
 * We start with a fully wiped cave of normal floors.
 *
 * Note that town_gen_hack() plays games with the R.N.G.
 *
 * This function does NOT do anything about the owners of the stores,
 * nor the contents thereof.  It only handles the physical layout.
 *
 * We place the player on the stairs at the same time we make them.
 *
 * Hack -- since the player always leaves the dungeon by the stairs,
 * he is always placed on the stairs, even if he left the dungeon via
 * word of recall or teleport level.
 */
void town_gen(int t_idx)
{
	int qy, qx;

	/* Level too small to contain a town */
	if (cur_hgt < SCREEN_HGT) return;
	if (cur_wid < SCREEN_WID) return;

	/* Center fo the level */
	qy = (cur_hgt - SCREEN_HGT) / 2;
	qx = (cur_wid - SCREEN_WID) / 2;

	/* Build stuff */
	switch (rand_int(3))
	{
	case 0:
		{
			town_gen_hack(t_idx, qy, qx);
			if (wizard)
			{
				msg_format("Town level(normal) (%d, seed %d)",
				           t_idx, town_info[t_idx].seed);
			}
			break;
		}

	case 1:
		{
			town_gen_circle(t_idx, qy, qx);
			if (wizard)
			{
				msg_format("Town level(circle)(%d, seed %d)",
				           t_idx, town_info[t_idx].seed);
			}
			break;
		}

	case 2:
		{
			town_gen_hidden(t_idx, qy, qx);
			if (wizard)
			{
				msg_format("Town level(hidden)(%d, seed %d)",
				           t_idx, town_info[t_idx].seed);
			}
			break;
		}
	}

	p_ptr->town_num = t_idx;
}
