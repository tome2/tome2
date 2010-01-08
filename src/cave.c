/* File: cave.c */

/* Purpose: low level dungeon routines -BEN- */


#include "angband.h"


/*
 * Support for Adam Bolt's tileset, lighting and transparency effects
 * by Robert Ruehlmann (rr9@angband.org)
 */


/*
 * Approximate Distance between two points.
 *
 * When either the X or Y component dwarfs the other component,
 * this function is almost perfect, and otherwise, it tends to
 * over-estimate about one grid per fifteen grids of distance.
 *
 * Algorithm: hypot(dy,dx) = max(dy,dx) + min(dy,dx) / 2
 */
int distance(int y1, int x1, int y2, int x2)
{
	int dy, dx, d;


	/* Find the absolute y/x distance components */
	dy = (y1 > y2) ? (y1 - y2) : (y2 - y1);
	dx = (x1 > x2) ? (x1 - x2) : (x2 - x1);

	/* Hack -- approximate the distance */
	d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

	/* Return the distance */
	return (d);
}


/*
 * Returns TRUE if a grid is considered to be a wall for the purpose
 * of magic mapping / clairvoyance
 */
static bool is_wall(cave_type *c_ptr)
{
	byte feat;


	/* Handle feature mimics */
	if (c_ptr->mimic) feat = c_ptr->mimic;
	else feat = c_ptr->feat;

	/* Paranoia */
	if (feat >= max_f_idx) return FALSE;

	/* Vanilla floors and doors aren't considered to be walls */
	if (feat < FEAT_SECRET) return FALSE;

	/* Exception #1: a glass wall is a wall but doesn't prevent LOS */
	if (feat == FEAT_GLASS_WALL) return FALSE;

	/* Exception #2: an illusion wall is not a wall but obstructs view */
	if (feat == FEAT_ILLUS_WALL) return TRUE;

	/* Exception #3: a small tree is a floor but obstructs view */
	if (feat == FEAT_SMALL_TREES) return TRUE;

	/* Normal cases: use the WALL flag in f_info.txt */
	return (f_info[feat].flags1 & FF1_WALL) ? TRUE : FALSE;
}


/*
 * A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
 * 4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.
 *
 * Returns TRUE if a line of sight can be traced from (x1,y1) to (x2,y2).
 *
 * The LOS begins at the center of the tile (x1,y1) and ends at the center of
 * the tile (x2,y2).  If los() is to return TRUE, all of the tiles this line
 * passes through must be floor tiles, except for (x1,y1) and (x2,y2).
 *
 * We assume that the "mathematical corner" of a non-floor tile does not
 * block line of sight.
 *
 * Because this function uses (short) ints for all calculations, overflow may
 * occur if dx and dy exceed 90.
 *
 * Once all the degenerate cases are eliminated, the values "qx", "qy", and
 * "m" are multiplied by a scale factor "f1 = abs(dx * dy * 2)", so that
 * we can use integer arithmetic.
 *
 * We travel from start to finish along the longer axis, starting at the border
 * between the first and second tiles, where the y offset = .5 * slope, taking
 * into account the scale factor.  See below.
 *
 * Also note that this function and the "move towards target" code do NOT
 * share the same properties.  Thus, you can see someone, target them, and
 * then fire a bolt at them, but the bolt may hit a wall, not them.  However,
 * by clever choice of target locations, you can sometimes throw a "curve".
 *
 * Note that "line of sight" is not "reflexive" in all cases.
 *
 * Use the "projectable()" routine to test "spell/missile line of sight".
 *
 * Use the "update_view()" function to determine player line-of-sight.
 */
bool los(int y1, int x1, int y2, int x2)
{
	/* Delta */
	int dx, dy;

	/* Absolute */
	int ax, ay;

	/* Signs */
	int sx, sy;

	/* Fractions */
	int qx, qy;

	/* Scanners */
	int tx, ty;

	/* Scale factors */
	int f1, f2;

	/* Slope, or 1/Slope, of LOS */
	int m;


	/* Extract the offset */
	dy = y2 - y1;
	dx = x2 - x1;

	/* Extract the absolute offset */
	ay = ABS(dy);
	ax = ABS(dx);


	/* Handle adjacent (or identical) grids */
	if ((ax < 2) && (ay < 2)) return (TRUE);


	/* Paranoia -- require "safe" origin */
	/* if (!in_bounds(y1, x1)) return (FALSE); */


	/* Directly South/North */
	if (!dx)
	{
		/* South -- check for walls */
		if (dy > 0)
		{
			for (ty = y1 + 1; ty < y2; ty++)
			{
				if (!cave_sight_bold(ty, x1)) return (FALSE);
			}
		}

		/* North -- check for walls */
		else
		{
			for (ty = y1 - 1; ty > y2; ty--)
			{
				if (!cave_sight_bold(ty, x1)) return (FALSE);
			}
		}

		/* Assume los */
		return (TRUE);
	}

	/* Directly East/West */
	if (!dy)
	{
		/* East -- check for walls */
		if (dx > 0)
		{
			for (tx = x1 + 1; tx < x2; tx++)
			{
				if (!cave_sight_bold(y1, tx)) return (FALSE);
			}
		}

		/* West -- check for walls */
		else
		{
			for (tx = x1 - 1; tx > x2; tx--)
			{
				if (!cave_sight_bold(y1, tx)) return (FALSE);
			}
		}

		/* Assume los */
		return (TRUE);
	}


	/* Extract some signs */
	sx = (dx < 0) ? -1 : 1;
	sy = (dy < 0) ? -1 : 1;


	/* Vertical "knights" */
	if (ax == 1)
	{
		if (ay == 2)
		{
			if (cave_sight_bold(y1 + sy, x1)) return (TRUE);
		}
	}

	/* Horizontal "knights" */
	else if (ay == 1)
	{
		if (ax == 2)
		{
			if (cave_sight_bold(y1, x1 + sx)) return (TRUE);
		}
	}


	/* Calculate scale factor div 2 */
	f2 = (ax * ay);

	/* Calculate scale factor */
	f1 = f2 << 1;


	/* Travel horizontally */
	if (ax >= ay)
	{
		/* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
		qy = ay * ay;
		m = qy << 1;

		tx = x1 + sx;

		/* Consider the special case where slope == 1. */
		if (qy == f2)
		{
			ty = y1 + sy;
			qy -= f1;
		}
		else
		{
			ty = y1;
		}

		/* Note (below) the case (qy == f2), where */
		/* the LOS exactly meets the corner of a tile. */
		while (x2 - tx)
		{
			if (!cave_sight_bold(ty, tx)) return (FALSE);

			qy += m;

			if (qy < f2)
			{
				tx += sx;
			}
			else if (qy > f2)
			{
				ty += sy;
				if (!cave_sight_bold(ty, tx)) return (FALSE);
				qy -= f1;
				tx += sx;
			}
			else
			{
				ty += sy;
				qy -= f1;
				tx += sx;
			}
		}
	}

	/* Travel vertically */
	else
	{
		/* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
		qx = ax * ax;
		m = qx << 1;

		ty = y1 + sy;

		if (qx == f2)
		{
			tx = x1 + sx;
			qx -= f1;
		}
		else
		{
			tx = x1;
		}

		/* Note (below) the case (qx == f2), where */
		/* the LOS exactly meets the corner of a tile. */
		while (y2 - ty)
		{
			if (!cave_sight_bold(ty, tx)) return (FALSE);

			qx += m;

			if (qx < f2)
			{
				ty += sy;
			}
			else if (qx > f2)
			{
				tx += sx;
				if (!cave_sight_bold(ty, tx)) return (FALSE);
				qx -= f1;
				ty += sy;
			}
			else
			{
				tx += sx;
				qx -= f1;
				ty += sy;
			}
		}
	}

	/* Assume los */
	return (TRUE);
}



/*
 * Returns true if the player's grid is dark
 */
bool no_lite(void)
{
	return (!player_can_see_bold(p_ptr->py, p_ptr->px));
}



/*
 * Determine if a given location may be "destroyed"
 *
 * Used by destruction spells, and for placing stairs, etc.
 */
bool cave_valid_bold(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;


	/* Forbid perma-grids */
	if (cave_perma_grid(c_ptr)) return (FALSE);

	/* Check objects */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Forbid artifact grids */
		if ((o_ptr->art_name) || artifact_p(o_ptr)) return (FALSE);
	}

	/* Accept */
	return (TRUE);
}




/*
 * Hack -- Legal monster codes
 */
static cptr image_monster_hack = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
 * Hack -- Legal monster codes for IBM pseudo-graphics
 *
 * Dropped. Although this option has long left unmaintained, hardcoding
 * code points makes it impossible to update the font and prf files
 * flexibly. And the normal graphics code still works with it -- pelpel
 */

/*
 * Mega-Hack -- Hallucinatory monster
 */
static void image_monster(byte *ap, char *cp)
{
	int n;

	switch (graphics_mode)
	{
		/* Text mode */
	case GRAPHICS_NONE:
		{
			n = strlen(image_monster_hack);

			/* Random symbol from set above */
			*cp = (image_monster_hack[rand_int(n)]);

			/* Random color */
			*ap = randint(15);

			break;
		}

		/* Normal graphics */
	default:
		{
			/* Avoid player ghost */
			n = randint(max_r_idx);

			*cp = r_info[n].x_char;

			*ap = r_info[n].x_attr;

			break;
		}
	}
}




/*
 * Hack -- Legal object codes
 */
static cptr image_object_hack = "?/|\\\"!$()_-=[]{},~";

/*
 * Hardcoded IBM pseudo-graphics code points have been removed
 * for the same reason as stated above -- pelpel
 */

/*
 * Mega-Hack -- Hallucinatory object
 */
static void image_object(byte *ap, char *cp)
{
	int n;

	switch (graphics_mode)
	{
		/* Text mode */
	case GRAPHICS_NONE:
		{
			n = strlen(image_object_hack);

			/* Random symbol from set above */
			*cp = (image_object_hack[rand_int(n)]);

			/* Random color */
			*ap = randint(15);

			/* Done */
			break;
		}

		/* Normal graphics */
	default:
		{
			n = randint(max_k_idx - 1);

			*cp = k_info[n].x_char;
			*ap = k_info[n].x_attr;

			break;
		}
	}
}


/*
 * Hack -- Random hallucination
 */
static void image_random(byte *ap, char *cp)
{
	/* Normally, assume monsters */
	if (rand_int(100) < 75)
	{
		image_monster(ap, cp);
	}

	/* Otherwise, assume objects */
	else
	{
		image_object(ap, cp);
	}
}


/*
 * The 16x16 tile of the terrain supports lighting
 */
#if 1

#define feat_supports_lighting(F) \
((f_info[F].flags1 & FF1_SUPPORT_LIGHT) != 0)

#else

static bool feat_supports_lighting(byte feat)
{
	if (f_info[feat].flags1 & FF1_SUPPORT_LIGHT) return TRUE;
	else return FALSE;
}

#endif


char get_shimmer_color()
{
	switch (randint(7))
	{
	case 1:
		return (TERM_RED);
	case 2:
		return (TERM_L_RED);
	case 3:
		return (TERM_WHITE);
	case 4:
		return (TERM_L_GREEN);
	case 5:
		return (TERM_BLUE);
	case 6:
		return (TERM_L_DARK);
	case 7:
		return (TERM_GREEN);
	}

	return (TERM_VIOLET);
}


/*
 * Table of breath colors.  Must match listings in a single set of 
 * monster spell flags.
 *
 * The value "255" is special.  Monsters with that kind of breath 
 * may be any color.
 */
static byte breath_to_attr[32][2] =
{
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ TERM_SLATE, TERM_L_DARK },        /* RF4_BRTH_ACID */
	{ TERM_BLUE, TERM_L_BLUE },        /* RF4_BRTH_ELEC */
	{ TERM_RED, TERM_L_RED },          /* RF4_BRTH_FIRE */
	{ TERM_WHITE, TERM_L_WHITE },      /* RF4_BRTH_COLD */
	{ TERM_GREEN, TERM_L_GREEN },      /* RF4_BRTH_POIS */
	{ TERM_L_GREEN, TERM_GREEN },      /* RF4_BRTH_NETHR */
	{ TERM_YELLOW, TERM_ORANGE },      /* RF4_BRTH_LITE */
	{ TERM_L_DARK, TERM_SLATE },       /* RF4_BRTH_DARK */
	{ TERM_L_UMBER, TERM_UMBER },      /* RF4_BRTH_CONFU */
	{ TERM_YELLOW, TERM_L_UMBER },     /* RF4_BRTH_SOUND */
	{ 255, 255 },    /* (any color) */ /* RF4_BRTH_CHAOS */
	{ TERM_VIOLET, TERM_VIOLET },      /* RF4_BRTH_DISEN */
	{ TERM_L_RED, TERM_VIOLET },       /* RF4_BRTH_NEXUS */
	{ TERM_L_BLUE, TERM_L_BLUE },      /* RF4_BRTH_TIME */
	{ TERM_L_WHITE, TERM_SLATE },      /* RF4_BRTH_INER */
	{ TERM_L_WHITE, TERM_SLATE },      /* RF4_BRTH_GRAV */
	{ TERM_UMBER, TERM_L_UMBER },      /* RF4_BRTH_SHARD */
	{ TERM_ORANGE, TERM_RED },         /* RF4_BRTH_PLAS */
	{ TERM_UMBER, TERM_L_UMBER },      /* RF4_BRTH_FORCE */
	{ TERM_L_BLUE, TERM_WHITE },       /* RF4_BRTH_MANA */
	{ 0, 0 },      /*  */
	{ TERM_GREEN, TERM_L_GREEN },      /* RF4_BRTH_NUKE */
	{ 0, 0 },      /*  */
	{ TERM_WHITE, TERM_L_RED },        /* RF4_BRTH_DISINT */
};


/*
 * Multi-hued monsters shimmer acording to their breaths.
 *
 * If a monster has only one kind of breath, it uses both colors 
 * associated with that breath.  Otherwise, it just uses the first 
 * color for any of its breaths.
 *
 * If a monster does not breath anything, it can be any color.
 */
static byte multi_hued_attr(monster_race *r_ptr)
{
	byte allowed_attrs[15];

	int i, j;

	int stored_colors = 0;

	int breaths = 0;

	int first_color = 0;

	int second_color = 0;


	/* Monsters with no ranged attacks can be any color */
	if (!r_ptr->freq_inate) return (get_shimmer_color());

	/* Check breaths */
	for (i = 0; i < 32; i++)
	{
		bool stored = FALSE;

		/* Don't have that breath */
		if (!(r_ptr->flags4 & (1L << i))) continue;

		/* Get the first color of this breath */
		first_color = breath_to_attr[i][0];

		/* Breath has no color associated with it */
		if (first_color == 0) continue;

		/* Monster can be of any color */
		if (first_color == 255) return (randint(15));


		/* Increment the number of breaths */
		breaths++;

		/* Monsters with lots of breaths may be any color. */
		if (breaths == 6) return (randint(15));


		/* Always store the first color */
		for (j = 0; j < stored_colors; j++)
		{
			/* Already stored */
			if (allowed_attrs[j] == first_color) stored = TRUE;
		}
		if (!stored)
		{
			allowed_attrs[stored_colors] = first_color;
			stored_colors++;
		}

		/*
		 * Remember (but do not immediately store) the second color 
		 * of the first breath.
		 */
		if (breaths == 1)
		{
			second_color = breath_to_attr[i][1];
		}
	}

	/* Monsters with no breaths may be of any color. */
	if (breaths == 0) return (get_shimmer_color());

	/* If monster has one breath, store the second color too. */
	if (breaths == 1)
	{
		allowed_attrs[stored_colors] = second_color;
		stored_colors++;
	}

	/* Pick a color at random */
	return (allowed_attrs[rand_int(stored_colors)]);
}


/*
 * Extract the attr/char to display at the given (legal) map location
 *
 * Note that this function, since it is called by "lite_spot()" which
 * is called by "update_view()", is a major efficiency concern.
 *
 * Basically, we examine each "layer" of the world (terrain, objects,
 * monsters/players), from the bottom up, extracting a new attr/char
 * if necessary at each layer, and defaulting to "darkness".  This is
 * not the fastest method, but it is very simple, and it is about as
 * fast as it could be for grids which contain no "marked" objects or
 * "visible" monsters.
 *
 * We apply the effects of hallucination during each layer.  Objects will
 * always appear as random "objects", monsters will always appear as random
 * "monsters", and normal grids occasionally appear as random "monsters" or
 * "objects", but note that these random "monsters" and "objects" are really
 * just "colored ascii symbols" (which may look silly on some machines).
 *
 * The hallucination functions avoid taking any pointers to local variables
 * because some compilers refuse to use registers for any local variables
 * whose address is taken anywhere in the function.
 *
 * As an optimization, we can handle the "player" grid as a special case.
 *
 * Note that the memorization of "objects" and "monsters" is not related
 * to the memorization of "terrain".  This allows the player to memorize
 * the terrain of a grid without memorizing any objects in that grid, and
 * to detect monsters without detecting anything about the terrain of the
 * grid containing the monster.
 *
 * The fact that all interesting "objects" and "terrain features" are
 * memorized as soon as they become visible for the first time means
 * that we only have to check the "CAVE_SEEN" flag for "boring" grids.
 *
 * Note that bizarre things must be done when the "attr" and/or "char"
 * codes have the "high-bit" set, since these values are used to encode
 * various "special" pictures in some versions, and certain situations,
 * such as "multi-hued" or "clear" monsters, cause the attr/char codes
 * to be "scrambled" in various ways.
 *
 * Note that the "zero" entry in the feature/object/monster arrays are
 * used to provide "special" attr/char codes, with "monster zero" being
 * used for the player attr/char, "object zero" being used for the "stack"
 * attr/char, and "feature zero" being used for the "nothing" attr/char.
 *
 * Note that eventually we may want to use the "&" symbol for embedded
 * treasure, and use the "*" symbol to indicate multiple objects, but
 * currently, we simply use the attr/char of the first "marked" object
 * in the stack, if any, and so "object zero" is unused.  XXX XXX XXX
 *
 * Note the assumption that doing "x_ptr = &x_info[x]" plus a few of
 * "x_ptr->xxx", is quicker than "x_info[x].xxx", even if "x" is a fixed
 * constant.  If this is incorrect then a lot of code should be changed.
 *
 *
 * Some comments on the "terrain" layer...
 *
 * Note that "boring" grids (floors, invisible traps, and any illegal grids)
 * are very different from "interesting" grids (all other terrain features),
 * and the two types of grids are handled completely separately.  The most
 * important distinction is that "boring" grids may or may not be memorized
 * when they are first encountered, and so we must use the "CAVE_SEEN" flag
 * to see if they are "see-able".
 *
 *
 * Some comments on the "terrain" layer (boring grids)...
 *
 * Note that "boring" grids are always drawn using the picture for "empty
 * floors", which is stored in "f_info[FEAT_FLOOR]".  Sometimes, special
 * lighting effects may cause this picture to be modified.
 *
 * Note that "invisible traps" are always displayes exactly like "empty
 * floors", which prevents various forms of "cheating", with no loss of
 * efficiency.  There are still a few ways to "guess" where traps may be
 * located, for example, objects will never fall into a grid containing
 * an invisible trap.  XXX XXX
 *
 * To determine if a "boring" grid should be displayed, we simply check to
 * see if it is either memorized ("CAVE_MARK"), or currently "see-able" by
 * the player ("CAVE_SEEN").  Note that "CAVE_SEEN" is now maintained by the
 * "update_view()" function.
 *
 * Note the "special lighting effects" which can be activated for "boring"
 * grids using the "view_special_lite" option, causing certain such grids
 * to be displayed using special colors. If the grid is "see-able" by
 * the player, we will use the normal (except that, if the "view_yellow_lite"
 * option is set, and the grid is *only* "see-able" because of the player's
 * torch, then we will use "yellow"), else if the player is "blind", we will
 * use greyscale, else if the grid is not "illuminated", we will use "dark
 * gray", if the "view_bright_lite" option is set, we will use "darker" colour
 * else we will use the normal colour.
 *
 *
 * Some comments on the "terrain" layer (non-boring grids)...
 *
 * Note the use of the "mimic" field in the "terrain feature" processing,
 * which allows any feature to "pretend" to be another feature.  This is
 * used to "hide" secret doors, and to make all "doors" appear the same,
 * and all "walls" appear the same, and "hidden" treasure stay hidden.
 * Note that it is possible to use this field to make a feature "look"
 * like a floor, but the "view_special_lite" flag only affects actual
 * "boring" grids.
 *
 * Since "interesting" grids are always memorized as soon as they become
 * "see-able" by the player ("CAVE_SEEN"), such a grid only needs to be
 * displayed if it is memorized ("CAVE_MARK").  Most "interesting" grids
 * are in fact non-memorized, non-see-able, wall grids, so the fact that
 * we do not have to check the "CAVE_SEEN" flag adds some efficiency, at
 * the cost of *forcing* the memorization of all "interesting" grids when
 * they are first seen.  Since the "CAVE_SEEN" flag is now maintained by
 * the "update_view()" function, this efficiency is not as significant as
 * it was in previous versions, and could perhaps be removed.
 * (so I removed this to simplify the terrain feature handling -- pelpel)
 *
 * Note the "special lighting effects" which can be activated for "wall"
 * grids using the "view_granite_lite" option, causing certain such grids
 * to be displayed using special colors.
 * If the grid is "see-able" by the player, we will use the normal colour
 * else if the player is "blind", we will use grey scale, else if the
 * "view_bright_lite" option is set, we will use reduced colour, else we
 * will use the normal one.
 *
 * Note that "wall" grids are more complicated than "boring" grids, due to
 * the fact that "CAVE_GLOW" for a "wall" grid means that the grid *might*
 * be glowing, depending on where the player is standing in relation to the
 * wall.  In particular, the wall of an illuminated room should look just
 * like any other (dark) wall unless the player is actually inside the room.
 *
 * Thus, we do not support as many visual special effects for "wall" grids
 * as we do for "boring" grids, since many of them would give the player
 * information about the "CAVE_GLOW" flag of the wall grid, in particular,
 * it would allow the player to notice the walls of illuminated rooms from
 * a dark hallway that happened to run beside the room.
 *
 *
 * Some comments on the "object" layer...
 *
 * Currently, we do nothing with multi-hued objects, because there are
 * not any.  If there were, they would have to set "shimmer_objects"
 * when they were created, and then new "shimmer" code in "dungeon.c"
 * would have to be created handle the "shimmer" effect, and the code
 * in "cave.c" would have to be updated to create the shimmer effect.
 * This did not seem worth the effort.  XXX XXX
 *
 *
 * Some comments on the "monster"/"player" layer...
 *
 * Note that monsters can have some "special" flags, including "ATTR_MULTI",
 * which means their color changes, and "ATTR_CLEAR", which means they take
 * the color of whatever is under them, and "CHAR_CLEAR", which means that
 * they take the symbol of whatever is under them.  Technically, the flag
 * "CHAR_MULTI" is supposed to indicate that a monster looks strange when
 * examined, but this flag is currently ignored.  All of these flags are
 * ignored if the "avoid_other" option is set, since checking for these
 * conditions is expensive (and annoying) on some systems.
 *
 * Normally, players could be handled just like monsters, except that the
 * concept of the "torch lite" of others player would add complications.
 * For efficiency, however, we handle the (only) player first, since the
 * "player" symbol always "pre-empts" any other facts about the grid.
 *
 * The "hidden_player" efficiency option, which only makes sense with a
 * single player, allows the player symbol to be hidden while running.
 */

/*
 * Alternative colours for unseen grids
 *
 * Reduced colours - remembered interesting grids and perma-lit floors
 * B&W - currently only used by blindness effect
 */

/* Colour */
static byte dark_attrs[16] =
{
	TERM_DARK, TERM_L_WHITE, TERM_L_DARK, TERM_ORANGE,
	TERM_RED, TERM_GREEN, TERM_BLUE, TERM_UMBER,
	TERM_L_DARK, TERM_SLATE, TERM_VIOLET, TERM_YELLOW,
	TERM_RED, TERM_GREEN, TERM_BLUE, TERM_UMBER
};

/* B&W */
static byte darker_attrs[16] =
{
	TERM_DARK, TERM_L_WHITE, TERM_L_DARK, TERM_SLATE,
	TERM_L_DARK, TERM_L_DARK, TERM_L_DARK, TERM_L_DARK,
	TERM_L_DARK, TERM_SLATE, TERM_L_DARK, TERM_SLATE,
	TERM_SLATE, TERM_SLATE, TERM_SLATE, TERM_SLATE
};


#ifdef USE_TRANSPARENCY
#ifdef USE_EGO_GRAPHICS
void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp,
              byte *eap, char *ecp)
#else /* USE_EGO_GRAPHICS */
void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp)
#endif /* USE_EGO_GRAPHICS */
#else /* USE_TRANSPARENCY */
void map_info(int y, int x, byte *ap, char *cp)
#endif /* USE_TRANSPARENCY */
{
	cave_type *c_ptr;

	feature_type *f_ptr;

	s16b this_o_idx, next_o_idx = 0;

	u16b info;

	s16b t_idx;

	byte feat;

	byte a;

	byte c;

	/*
	 * This means that a port supports graphics overlay as well as lighting
	 * effects. See the step 3 below for the detailed information about
	 * lighting. Basically, it requires "darker" tiles for those terrain
	 * features with SUPPORT_LIGHT flag set, and they must be arranged
	 * this way:
	 *     col  col+1  col+2
	 * row base darker brighter
	 */
	bool graf_new = ((graphics_mode == GRAPHICS_ISO) ||
	                 (graphics_mode == GRAPHICS_NEW));

	/*
	 * I never understand why some coders like shimmering so much.
	 * It just serves to hurt my eyes, IMHO.  If one feels like to show off,
	 * go for better graphics support... Anyway this means a port allows
	 * changing attr independently from its char -- pelpel
	 */
	bool attr_mutable = (!use_graphics ||
	                     (graphics_mode == GRAPHICS_IBM));


	/**** Preparation ****/

	/* Access the grid */
	c_ptr = &cave[y][x];


	/* Cache some frequently used values */

	/* Grid info */
	info = c_ptr->info;

	/* Feature code */
	feat = c_ptr->feat;

	/* Apply "mimic" field */
	if (c_ptr->mimic)
	{
		feat = c_ptr->mimic;
	}
	else
	{
		feat = f_info[feat].mimic;
	}

	/* Access floor */
	f_ptr = &f_info[feat];


#ifdef USE_EGO_GRAPHICS

	/* Reset attr/char */
	*eap = 0;
	*ecp = 0;

#endif /* USE_EGO_GRAPHICS */


	/**** Layer 1 -- Terrain feature ****/

	/* Only memorised or visible grids are displayed */
	if (info & (CAVE_MARK | CAVE_SEEN))
	{
		/**** Step 1 -- Retrieve base attr/char ****/

		/* 'Sane' terrain features */
		if (feat != FEAT_SHOP)
		{
			/* Normal char */
			c = f_ptr->x_char;

			/* Normal attr */
			a = f_ptr->x_attr;
		}

		/* Mega-Hack 1 -- Building don't conform to f_info */
		else
		{
			c = st_info[c_ptr->special].x_char;
			a = st_info[c_ptr->special].x_attr;
		}

		/* Mega-Hack 2 -- stair to dungeon branch are purple */
		if (c_ptr->special && attr_mutable &&
		                ((feat == FEAT_MORE) || (feat == FEAT_LESS)))
		{
			a = TERM_VIOLET;
		}

		/* Mega-Hack 3 -- Traps don't have f_info entries either */
		if ((info & (CAVE_TRDT)) && (feat != FEAT_ILLUS_WALL))
		{
			/* Trap index */
			t_idx = c_ptr->t_idx;

			if (use_graphics &&
			                (t_info[t_idx].g_attr != 0) &&
			                (t_info[t_idx].g_char != 0))
			{

#ifdef USE_EGO_GRAPHICS

				if (graf_new)
				{
					*eap = t_info[t_idx].g_attr;
					*ecp = t_info[t_idx].g_char;
				}
				else
				{
					a = t_info[t_idx].g_attr;
					c = t_info[t_idx].g_char;
				}

#else /* USE_EGO_GRAPHICS */

				a = t_info[t_idx].g_attr;
				c = t_info[t_idx].g_char;

#endif /* USE_EGO_GRAPHICS */

			}
			else
			{
				/*
				 * If trap is set on a floor grid that is not
				 * one of "interesting" features, use a special
				 * symbol to display it. Check for doors is no longer
				 * necessary because they have REMEMBER flag now.
				 *
				 * Cave macros cannot be used safely here, because of
				 * c_ptr->mimic XXX XXX
				 */
				if (!attr_mutable)
				{
					a = f_info[FEAT_TRAP].x_attr;
					c = f_info[FEAT_TRAP].x_char;
				}
				else
				{
					if ((f_ptr->flags1 & (FF1_FLOOR | FF1_REMEMBER)) == FF1_FLOOR)
					{
						c = f_info[FEAT_TRAP].x_char;
					}

					/* Add attr XXX XXX XXX */
					a = t_info[t_idx].color;

					/* Get a new color with a strange formula :) XXX XXX XXX */
					if (t_info[t_idx].flags & FTRAP_CHANGE)
					{
						s32b tmp;

						tmp = dun_level + dungeon_type + feat;

						a = tmp % 16;
					}
				}
			}
		}


		/**** Step 2 -- Apply special random effects ****/
		if (!avoid_other && !avoid_shimmer && attr_mutable)
		{
			/* Special terrain effect */
			if (c_ptr->effect)
			{
				a = spell_color(effects[c_ptr->effect].type);
			}

			/* Multi-hued attr */
			else if (f_ptr->flags1 & FF1_ATTR_MULTI)
			{
				a = f_ptr->shimmer[rand_int(7)];
			}
		}


		/*
		 * Step 3
		 *
		 * Special lighting effects, if specified and applicable
		 * This will never happen for
		 * - any grids in the overhead map
		 * - traps
		 * - (graphics modes) terrain features without corresponding
		 *   "darker" tiles.
		 *
		 * Note the use of f_ptr->flags1 to avoid problems with
		 * c_ptr->mimic.
		 */

		/* view_special_lite: lighting effects for boring features */
		if (view_special_lite &&
		                ((f_ptr->flags1 & (FF1_FLOOR | FF1_REMEMBER)) == FF1_FLOOR))
		{
			if (!p_ptr->wild_mode && !(info & (CAVE_TRDT)) &&
			                (attr_mutable || (graf_new && feat_supports_lighting(feat))))
			{
				/* Handle "seen" grids */
				if (info & (CAVE_SEEN))
				{
					/* Only lit by "torch" light */
					if (view_yellow_lite && !(info & (CAVE_GLOW)))
					{
						if (graf_new)
						{
							/* Use a brightly lit tile */
							c += 2;
						}
						else
						{
							/* Use "yellow" */
							a = TERM_YELLOW;
						}
					}
				}

				/* Handle "blind" */
				else if (p_ptr->blind)
				{
					if (graf_new)
					{
						/* Use a dark tile */
						c++;
					}
					else
					{
						/* Use darker colour */
						a = darker_attrs[a & 0xF];
					}
				}

				/* Handle "dark" grids */
				else if (!(info & (CAVE_GLOW)))
				{
					if (graf_new)
					{
						/* Use a dark tile */
						c++;
					}
					else
					{
						/* Use darkest colour */
						a = TERM_L_DARK;
					}
				}

				/* "Out-of-sight" glowing grids -- handle "view_bright_lite" */
				else if (view_bright_lite)
				{
					if (graf_new)
					{
						/* Use a dark tile */
						c++;
					}
					else
					{
						/* Use darker colour */
						a = dark_attrs[a & 0xF];
					}
				}
			}
		}

		/* view_granite_lite: lighting effects for walls and doors */
		else if (view_granite_lite &&
		                (f_ptr->flags1 & (FF1_NO_VISION | FF1_DOOR)))
		{
			if (!p_ptr->wild_mode && !(info & (CAVE_TRDT)) &&
			                (attr_mutable || (graf_new && feat_supports_lighting(feat))))
			{
				/* Handle "seen" grids */
				if (info & (CAVE_SEEN))
				{
					/* Do nothing */
				}

				/* Handle "blind" */
				else if (p_ptr->blind)
				{
					if (graf_new)
					{
						/* Use a dark tile */
						c++;
					}
					else
					{
						/* Use darker colour */
						a = darker_attrs[a & 0xF];
					}
				}

				/* Handle "view_bright_lite" */
				else if (view_bright_lite)
				{
					if (graf_new)
					{
						/* Use a dark tile */
						c++;
					}
					else
					{
						/* Use darker colour */
						a = dark_attrs[a & 0xF];
					}
				}

				else
				{
					if (graf_new)
					{
						/* Use a brightly lit tile */
						c += 2;
					}
					else
					{
						/* Use normal colour */
					}
				}
			}
		}
	}

	/* Unknown grids */
	else
	{
		/* Access darkness */
		f_ptr = &f_info[FEAT_NONE];

		/* Normal attr */
		a = f_ptr->x_attr;

		/* Normal char */
		c = f_ptr->x_char;
	}

	/*
	 * Hack -- rare random hallucination
	 * Because we cannot be sure which is outer dungeon walls,
	 * the check for 'feat' has been removed
	 */
	if (p_ptr->image && (rand_int(256) == 0))
	{
		/* Hallucinate */
		image_random(ap, cp);
	}

#ifdef USE_TRANSPARENCY

	/* Save the terrain info for the transparency effects */
	*tap = a;
	*tcp = c;

#endif /* USE_TRANSPARENCY */

	/* Save the info */
	*ap = a;
	*cp = c;


	/**** Layer 2 -- Objects ****/

	if (feat != FEAT_MON_TRAP)
	{
		for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type * o_ptr;

			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Memorized objects */
			if (o_ptr->marked)
			{
				/* Normal char */
				*cp = object_char(o_ptr);

				/* Normal attr */
				*ap = object_attr(o_ptr);

				/* Multi-hued attr */
				if (!avoid_other && attr_mutable &&
				                (k_info[o_ptr->k_idx].flags5 & TR5_ATTR_MULTI))
				{
					*ap = get_shimmer_color();
				}

				/* Hack -- hallucination */
				if (p_ptr->image) image_object(ap, cp);

				/* Done */
				break;
			}
		}
	}


	/**** Layer 3 -- Handle monsters ****/

	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		monster_race *r_ptr = race_inf(m_ptr);

		if (r_ptr->flags9 & RF9_MIMIC)
		{
			object_type *o_ptr;

			/* Acquire object */
			o_ptr = &o_list[m_ptr->hold_o_idx];

			/* Memorized objects */
			if (o_ptr->marked)
			{
				/* Normal char */
				*cp = object_char(o_ptr);

				/* Normal attr */
				*ap = object_attr(o_ptr);

				/* Multi-hued attr */
				if (!avoid_other && attr_mutable &&
				                (k_info[o_ptr->k_idx].flags5 & TR5_ATTR_MULTI))
				{
					*ap = get_shimmer_color();
				}

				/* Hack -- hallucination */
				if (p_ptr->image) image_object(ap, cp);
			}
		}
		else
		{
			/* Visible monster */
			if (m_ptr->ml)
			{
				monster_race *r_ptr = race_inf(m_ptr);

#ifdef USE_EGO_GRAPHICS

				/* Reset attr/char */
				*eap = 0;
				*ecp = 0;

#endif /* USE_EGO_GRAPHICS */

				if (use_graphics)
				{

#ifdef USE_EGO_GRAPHICS

					if (graf_new)
					{
						monster_ego *re_ptr = &re_info[m_ptr->ego];

						/* Desired attr */
						*eap = re_ptr->g_attr;

						/* Desired char */
						*ecp = re_ptr->g_char;
					}

#endif /* USE_EGO_GRAPHICS */

					/* Use base monster */
					r_ptr = &r_info[m_ptr->r_idx];
				}

				/* Desired attr/char */
				c = r_ptr->x_char;
				a = r_ptr->x_attr;

				/* Ignore weird codes */
				if (avoid_other)
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Special attr/char codes */
				else if (!attr_mutable)
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Multi-hued monster */
				else if (r_ptr->flags1 & (RF1_ATTR_MULTI))
				{
					/* Is it a shapechanger? */
					if (r_ptr->flags2 & (RF2_SHAPECHANGER))
					{
						image_random(ap, cp);
					}
					else
						*cp = c;

					/* Multi-hued attr */
					if (r_ptr->flags2 & (RF2_ATTR_ANY))
					{
						*ap = randint(15);
					}
					else
					{
						*ap = multi_hued_attr(r_ptr);
					}
				}

				/* Normal monster (not "clear" in any way) */
				else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR | RF1_CHAR_CLEAR)))
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/*
				 * Hack -- Bizarre grid under monster
				 * WAS: else if (*ap & 0x80) || (*cp & 0x80) -- pelpel
				 */
				else if (*ap & 0x80)
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Normal */
				else
				{
					/* Normal (non-clear char) monster */
					if (!(r_ptr->flags1 & (RF1_CHAR_CLEAR)))
					{
						/* Normal char */
						*cp = c;
					}

					/* Normal (non-clear attr) monster */
					else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR)))
					{
						/* Normal attr */
						*ap = a;
					}
				}

				/* Hack -- hallucination */
				if (p_ptr->image)
				{
					/* Hallucinatory monster */
					image_monster(ap, cp);
				}
			}
		}
	}

	/* Handle "player" */
	if ((y == p_ptr->py) && (x == p_ptr->px) &&
	                (!p_ptr->invis || p_ptr->see_inv))
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

#ifdef USE_EGO_GRAPHICS

		/* Reset attr/char */
		*eap = 0;
		*ecp = 0;

#endif /* USE_EGO_GRAPHICS */

		/* Get the "player" attr */
		if (!avoid_other && attr_mutable && (r_ptr->flags1 & RF1_ATTR_MULTI))
		{
			a = get_shimmer_color();
		}
		else
		{
			a = r_ptr->x_attr;
		}

		/* Get the "player" char */
		c = r_ptr->x_char;


		/* Mega-Hack -- Apply modifications to player graphics XXX XXX XXX */
		switch (graphics_mode)
		{
		case GRAPHICS_NONE:
		case GRAPHICS_IBM:
			{
				if (player_char_health)
				{
					int percent = p_ptr->chp * 10 / p_ptr->mhp;

					if (percent < 7)
					{
						c = I2D(percent);
						if (percent < 3) a = TERM_L_RED;
					}
				}

				break;
			}

#ifdef VARIABLE_PLAYER_GRAPH

		case GRAPHICS_OLD:
			{
				if (player_symbols)
				{
					a = BMP_FIRST_PC_CLASS + p_ptr->pclass;
					c = BMP_FIRST_PC_RACE + p_ptr->prace;
				}

				break;
			}

#endif /* VARIABLE_PLAYER_GRAPH */

#ifdef USE_EGO_GRAPHICS

		case GRAPHICS_ISO:
		case GRAPHICS_NEW:
			{
				if (p_ptr->pracem)
				{
					player_race_mod *rmp_ptr = &race_mod_info[p_ptr->pracem];

					/* Desired attr */
					*eap = rmp_ptr->g_attr;

					/* Desired char */
					*ecp = rmp_ptr->g_char;
				}

				/* +AKH 20020421 - Health dispay for graphics, too */
				if (player_char_health && (graphics_mode == GRAPHICS_NEW))
				{
					int percent = p_ptr->chp * 14 / p_ptr->mhp;

					if (percent < 10)
					{
						*eap = 10;
						*ecp = 32 + 14 - percent;
					}
				}

				break;
			}

#endif /* USE_EGO_GRAPHICS */

		}

		/* Save the info */
		*ap = a;
		*cp = c;

	}
}


/*
 * Special version of map_info, for use by cmovie and HTML converter
 * to obtain pure-ASCII image of dungeon map
 */
void map_info_default(int y, int x, byte *ap, char *cp)
{
	cave_type *c_ptr;

	feature_type *f_ptr;

	s16b this_o_idx, next_o_idx = 0;

	u16b info;

	s16b t_idx;

	byte feat;

	byte a;

	byte c;

	bool use_graphics_hack = use_graphics;
	byte graphics_mode_hack = graphics_mode;


	/* Temporarily disable graphics mode -- for some random effects XXX */
	use_graphics = FALSE;
	graphics_mode = GRAPHICS_NONE;

	/**** Preparation ****/

	/* Access the grid */
	c_ptr = &cave[y][x];


	/* Cache some frequently used values */

	/* Grid info */
	info = c_ptr->info;

	/* Feature code */
	feat = c_ptr->feat;

	/* Apply "mimic" field */
	if (c_ptr->mimic)
	{
		feat = c_ptr->mimic;
	}
	else
	{
		feat = f_info[feat].mimic;
	}

	/* Access floor */
	f_ptr = &f_info[feat];


	/**** Layer 1 -- Terrain feature ****/

	/* Only memorised or visible grids are displayed */
	if (info & (CAVE_MARK | CAVE_SEEN))
	{
		/**** Step 1 -- Retrieve base attr/char ****/

		/* 'Sane' terrain features */
		if (feat != FEAT_SHOP)
		{
			/* Default char */
			c = f_ptr->d_char;

			/* Default attr */
			a = f_ptr->d_attr;
		}

		/* Mega-Hack 1 -- Building don't conform to f_info */
		else
		{
			c = st_info[c_ptr->special].d_char;
			a = st_info[c_ptr->special].d_attr;
		}

		/* Mega-Hack 2 -- stair to dungeon branch are purple */
		if (c_ptr->special &&
		                ((feat == FEAT_MORE) || (feat == FEAT_LESS)))
		{
			a = TERM_VIOLET;
		}

		/* Mega-Hack 3 -- Traps don't have f_info entries either */
		if ((info & (CAVE_TRDT)) && (feat != FEAT_ILLUS_WALL))
		{
			/* Trap index */
			t_idx = c_ptr->t_idx;

			/*
			 * If trap is set on a floor grid that is not
			 * one of "interesting" features, use a special
			 * symbol to display it. Check for doors is no longer
			 * necessary because they have REMEMBER flag now.
			 *
			 * Cave macros cannot be used safely here, because of
			 * c_ptr->mimic XXX XXX
			 */
			if ((f_ptr->flags1 & (FF1_FLOOR | FF1_REMEMBER)) == FF1_FLOOR)
			{
				c = f_info[FEAT_TRAP].d_char;
			}

			/* Add attr */
			a = t_info[t_idx].color;

			/* Get a new color with a strange formula :) */
			if (t_info[t_idx].flags & FTRAP_CHANGE)
			{
				s32b tmp;

				tmp = dun_level + dungeon_type + feat;

				a = tmp % 16;
			}
		}


		/**** Step 2 -- Apply special random effects ****/
		if (!avoid_other)
		{
			/* Special terrain effect */
			if (c_ptr->effect)
			{
				a = spell_color(effects[c_ptr->effect].type);
			}

			/* Multi-hued attr */
			else if (f_ptr->flags1 & FF1_ATTR_MULTI)
			{
				a = f_ptr->shimmer[rand_int(7)];
			}
		}


		/*
		 * Step 3
		 *
		 * Special lighting effects, if specified and applicable
		 * This will never happen for
		 * - any grids in the overhead map
		 * - traps
		 * - (graphics modes) terrain features without corresponding
		 *   "darker" tiles.
		 *
		 * All the if's here are flag checks, so changed order shouldn't
		 * affect performance a lot, I hope...
		 */

		/* view_special_lite: lighting effects for boring features */
		if (view_special_lite &&
		                ((f_ptr->flags1 & (FF1_FLOOR | FF1_REMEMBER)) == FF1_FLOOR))
		{
			if (!p_ptr->wild_mode && !(info & (CAVE_TRDT)))
			{
				/* Handle "seen" grids */
				if (info & (CAVE_SEEN))
				{
					/* Only lit by "torch" light */
					if (view_yellow_lite && !(info & (CAVE_GLOW)))
					{
						/* Use "yellow" */
						a = TERM_YELLOW;
					}
				}

				/* Handle "blind" */
				else if (p_ptr->blind)
				{
					/* Use darker colour */
					a = darker_attrs[a & 0xF];
				}

				/* Handle "dark" grids */
				else if (!(info & (CAVE_GLOW)))
				{
					/* Use darkest colour */
					a = TERM_L_DARK;
				}

				/* "Out-of-sight" glowing grids -- handle "view_bright_lite" */
				else if (view_bright_lite)
				{
					/* Use darker colour */
					a = dark_attrs[a & 0xF];
				}
			}
		}

		/* view_granite_lite: lighting effects for walls and doors */
		else if (view_granite_lite &&
		                (f_ptr->flags1 & (FF1_NO_VISION | FF1_DOOR)))
		{
			if (!p_ptr->wild_mode && !(info & (CAVE_TRDT)))
			{
				/* Handle "seen" grids */
				if (info & (CAVE_SEEN))
				{
					/* Do nothing */
				}

				/* Handle "blind" */
				else if (p_ptr->blind)
				{
					/* Use darker colour */
					a = darker_attrs[a & 0xF];
				}

				/* Handle "view_bright_lite" */
				else if (view_bright_lite)
				{
					/* Use darker colour */
					a = dark_attrs[a & 0xF];
				}
			}
		}
	}

	/* Unknown grids */
	else
	{
		/* Access darkness */
		f_ptr = &f_info[FEAT_NONE];

		/* Default attr */
		a = f_ptr->d_attr;

		/* Default char */
		c = f_ptr->d_char;
	}

	/*
	 * Hack -- rare random hallucination
	 * Because we cannot be sure which is outer dungeon walls,
	 * the check for 'feat' has been removed
	 */
	if (p_ptr->image && (rand_int(256) == 0))
	{
		/* Hallucinate */
		image_random(ap, cp);
	}

	/* Save the info */
	*ap = a;
	*cp = c;


	/**** Layer 2 -- Objects ****/

	if (feat != FEAT_MON_TRAP)
	{
		for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type * o_ptr;

			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Memorized objects */
			if (o_ptr->marked)
			{
				/* Normal char */
				*cp = object_char_default(o_ptr);

				/* Normal attr */
				*ap = object_attr_default(o_ptr);

				/* Multi-hued attr */
				if (!avoid_other &&
				                (k_info[o_ptr->k_idx].flags5 & TR5_ATTR_MULTI))
				{
					*ap = get_shimmer_color();
				}

				/* Hack -- hallucination */
				if (p_ptr->image) image_object(ap, cp);

				/* Done */
				break;
			}
		}
	}


	/**** Layer 3 -- Handle monsters ****/

	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		monster_race *r_ptr = race_inf(m_ptr);

		if (r_ptr->flags9 & RF9_MIMIC)
		{
			object_type *o_ptr;

			/* Acquire object */
			o_ptr = &o_list[m_ptr->hold_o_idx];

			/* Memorized objects */
			if (o_ptr->marked)
			{
				/* Normal char */
				*cp = object_char_default(o_ptr);

				/* Normal attr */
				*ap = object_attr_default(o_ptr);

				/* Multi-hued attr */
				if (!avoid_other && !use_graphics &&
				                (k_info[o_ptr->k_idx].flags5 & TR5_ATTR_MULTI))
				{
					*ap = get_shimmer_color();
				}

				/* Hack -- hallucination */
				if (p_ptr->image) image_object(ap, cp);
			}
		}
		else
		{
			/* Visible monster */
			if (m_ptr->ml)
			{
				monster_race *r_ptr = race_inf(m_ptr);

				/* Default attr/char */
				c = r_ptr->d_char;
				a = r_ptr->d_attr;

				/* Ignore weird codes */
				if (avoid_other)
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Multi-hued monster */
				else if (r_ptr->flags1 & (RF1_ATTR_MULTI))
				{
					/* Is it a shapechanger? */
					if (r_ptr->flags2 & (RF2_SHAPECHANGER))
					{
						image_random(ap, cp);
					}
					else
						*cp = c;

					/* Multi-hued attr */
					if (r_ptr->flags2 & (RF2_ATTR_ANY))
					{
						*ap = randint(15);
					}
					else
					{
						*ap = multi_hued_attr(r_ptr);
					}
				}

				/* Normal monster (not "clear" in any way) */
				else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR | RF1_CHAR_CLEAR)))
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Hack -- Bizarre grid under monster */
				else if ((*ap & 0x80) || (*cp & 0x80))
				{
					/* Use char */
					*cp = c;

					/* Use attr */
					*ap = a;
				}

				/* Normal */
				else
				{
					/* Normal (non-clear char) monster */
					if (!(r_ptr->flags1 & (RF1_CHAR_CLEAR)))
					{
						/* Normal char */
						*cp = c;
					}

					/* Normal (non-clear attr) monster */
					else if (!(r_ptr->flags1 & (RF1_ATTR_CLEAR)))
					{
						/* Normal attr */
						*ap = a;
					}
				}

				/* Hack -- hallucination */
				if (p_ptr->image)
				{
					/* Hallucinatory monster */
					image_monster(ap, cp);
				}
			}
		}
	}


	/* Handle "player" */
	if ((y == p_ptr->py) && (x == p_ptr->px) &&
	                (!p_ptr->invis ||
	                 (p_ptr->invis && p_ptr->see_inv)))
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

		/* Get the "player" attr */
		if (!avoid_other && (r_ptr->flags1 & RF1_ATTR_MULTI))
		{
			a = get_shimmer_color();
		}
		else
		{
			a = r_ptr->d_attr;
		}

		/* Get the "player" char */
		c = r_ptr->d_char;

		/* Save the info */
		*ap = a;
		*cp = c;

	}

	/* XXX Restore the graphics mode */
	use_graphics = use_graphics_hack;
	graphics_mode = graphics_mode_hack;
}


/*
 * Calculate panel colum of a location in the map
 */
static int panel_col_of(int col)
{
	col -= panel_col_min;
	if (use_bigtile) col *= 2;
	return col + COL_MAP;
}



/*
 * Moves the cursor to a given MAP (y,x) location
 */
void move_cursor_relative(int row, int col)
{
	/* Real co-ords convert to screen positions */
	row -= panel_row_prt;

	/* Go there */
	Term_gotoxy(panel_col_of(col), row);
}



/*
 * Place an attr/char pair at the given map coordinate, if legal.
 */
void print_rel(char c, byte a, int y, int x)
{
	/* Paranoia -- Only do "legal" locations */
	if (!panel_contains(y, x)) return;

	/* Draw the char using the attr */
	Term_draw(panel_col_of(x), y - panel_row_prt, a, c);

	if (use_bigtile)
	{
		char c2;
		byte a2;

		if (a & 0x80)
		{
			a2 = 255;
			c2 = 255;
		}
		else
		{
			a2 = TERM_WHITE;
			c2 = ' ';
		}
		Term_draw(panel_col_of(x) + 1, y - panel_row_prt, a2, c2);
	}
}





/*
 * Memorize interesting viewable object/features in the given grid
 *
 * This function should only be called on "legal" grids.
 *
 * This function will memorize the object and/or feature in the given
 * grid, if they are (1) viewable and (2) interesting.  Note that all
 * objects are interesting, all terrain features except floors (and
 * invisible traps) are interesting, and floors (and invisible traps)
 * are interesting sometimes (depending on various options involving
 * the illumination of floor grids).
 *
 * The automatic memorization of all objects and non-floor terrain
 * features as soon as they are displayed allows incredible amounts
 * of optimization in various places, especially "map_info()".
 *
 * Note that the memorization of objects is completely separate from
 * the memorization of terrain features, preventing annoying floor
 * memorization when a detected object is picked up from a dark floor,
 * and object memorization when an object is dropped into a floor grid
 * which is memorized but out-of-sight.
 *
 * This function should be called every time the "memorization" of
 * a grid (or the object in a grid) is called into question, such
 * as when an object is created in a grid, when a terrain feature
 * "changes" from "floor" to "non-floor", when any grid becomes
 * "illuminated" or "viewable", and when a "floor" grid becomes
 * "torch-lit".
 *
 * Note the relatively efficient use of this function by the various
 * "update_view()" and "update_lite()" calls, to allow objects and
 * terrain features to be memorized (and drawn) whenever they become
 * viewable or illuminated in any way, but not when they "maintain"
 * or "lose" their previous viewability or illumination.
 *
 * Note the butchered "internal" version of "player_can_see_bold()",
 * optimized primarily for the most common cases, that is, for the
 * non-marked floor grids.
 */
void note_spot(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	u16b info = c_ptr->info;

	s16b this_o_idx, next_o_idx = 0;


	/* Require "seen" flag */
	if (!(info & (CAVE_SEEN))) return;


	/* Hack -- memorize objects */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Memorize objects */
		o_ptr->marked = TRUE;
	}

	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		monster_race *r_ptr = race_inf(m_ptr);

		if (r_ptr->flags9 & RF9_MIMIC)
		{
			object_type *o_ptr = &o_list[m_ptr->hold_o_idx];

			o_ptr->marked = TRUE;
		}
	}


	/* Hack -- memorize grids */
	if (!(info & (CAVE_MARK)))
	{
		/* Memorise some "boring" grids */
		if (cave_plain_floor_grid(c_ptr))
		{
			/* Option -- memorise certain floors */
			if ((info & (CAVE_TRDT)) ||
			                ((info & (CAVE_GLOW)) && view_perma_grids ) ||
			                view_torch_grids)
			{
				/* Memorize */
				c_ptr->info |= (CAVE_MARK);
			}
		}

		/* Memorise all "interesting" grids */
		else
		{
			/* Memorize */
			c_ptr->info |= (CAVE_MARK);
		}
	}
}


/*
 * Redraw (on the screen) a given MAP location
 *
 * This function should only be called on "legal" grids
 */
void lite_spot(int y, int x)
{
	byte a, a2;
	byte c, c2;

#ifdef USE_TRANSPARENCY
	byte ta;
	char tc;

# ifdef USE_EGO_GRAPHICS
	byte ea;
	char ec;

# endif  /* USE_EGO_GRAPHICS */
#endif /* USE_TRANSPARENCY */


	/* Redraw if on screen */
	if (panel_contains(y, x))
	{
#ifdef USE_TRANSPARENCY

# ifdef USE_EGO_GRAPHICS

		/* Examine the grid */
		map_info(y, x, &a, (char*)&c, &ta, &tc, &ea, &ec);

		/* Hack -- Queue it */
		Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c, ta, tc, ea, ec);
		if (use_bigtile)
		{
			if (a & 0x80)
			{
				a2 = 255;
				c2 = 255;
			}
			else
			{
				a2 = TERM_WHITE;
				c2 = ' ';
			}
			Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2, 0, 0, 0, 0);
		}

# else /* USE_EGO_GRAPHICS */

		/* Examine the grid */
		map_info(y, x, &a, &c, &ta, &tc);

		/* Hack -- Queue it */
		Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c, ta, tc);
		if (use_bigtile)
		{
			if (a & 0x80)
			{
				a2 = 255;
				c2 = 255;
			}
			else
			{
				a2 = TERM_WHITE;
				c2 = ' ';
			}
			Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2, 0, 0);
		}

# endif  /* USE_EGO_GRAPHICS */

#else /* USE_TRANSPARENCY */

		/* Examine the grid */
		map_info(y, x, &a, (char *) &c);

		/* Hack -- Queue it */
		Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c);
		if (use_bigtile)
		{
			if (a & 0x80)
			{
				a2 = 255;
				c2 = 255;
			}
			else
			{
				a2 = TERM_WHITE;
				c2 = ' ';
			}
			Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2);
		}

#endif /* USE_TRANSPARENCY */

	}
}




/*
 * Prints the map of the dungeon
 *
 * Note that, for efficiency, we contain an "optimized" version
 * of both "lite_spot()" and "print_rel()", and that we use the
 * "lite_spot()" function to display the player grid, if needed.
 */
void prt_map(void)
{
	int x, y;

	int v;

	/* Access the cursor state */
	(void)Term_get_cursor(&v);

	/* Hide the cursor */
	(void)Term_set_cursor(0);

	/* Dump the map */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		/* Scan the columns of row "y" */
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			byte a, a2;
			char c, c2;

#ifdef USE_TRANSPARENCY
			byte ta;
			char tc;

#ifdef USE_EGO_GRAPHICS
			byte ea;
			char ec;

			/* Determine what is there */
			map_info(y, x, &a, &c, &ta, &tc, &ea, &ec);

			/* Efficiency -- Redraw that grid of the map */
			Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c, ta, tc, ea, ec);
			if (use_bigtile)
			{
				if (a & 0x80)
				{
					a2 = 255;
					c2 = 255;
				}
				else
				{
					a2 = TERM_WHITE;
					c2 = ' ';
				}
				Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2, 0, 0, 0, 0);
			}
#else /* USE_EGO_GRAPHICS */
/* Determine what is there */
			map_info(y, x, &a, &c, &ta, &tc);
			/* Efficiency -- Redraw that grid of the map */
			Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c, ta, tc);
			if (use_bigtile)
			{
				if (a & 0x80)
				{
					a2 = 255;
					c2 = 255;
				}
				else
				{
					a2 = TERM_WHITE;
					c2 = ' ';
				}
				Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2, 0, 0);
			}

#endif /* USE_EGO_GRAPHICS */
#else /* USE_TRANSPARENCY */
			/* Determine what is there */
			map_info(y, x, &a, &c);

			/* Efficiency -- Redraw that grid of the map */
			Term_queue_char(panel_col_of(x), y - panel_row_prt, a, c);
			if (use_bigtile)
			{
				if (a & 0x80)
				{
					a2 = 255;
					c2 = 255;
				}
				else
				{
					a2 = TERM_WHITE;
					c2 = ' ';
				}
				Term_queue_char(panel_col_of(x) + 1, y - panel_row_prt, a2, c2);
			}
#endif /* USE_TRANSPARENCY */
		}
	}

	/* Display player */
	lite_spot(p_ptr->py, p_ptr->px);

	/* Restore the cursor */
	(void)Term_set_cursor(v);
}





/*
 * Display highest priority object in the RATIO by RATIO area
 */

/*
 * Display the entire map
 */
#define MAP_HGT (MAX_HGT / RATIO)
#define MAP_WID (MAX_WID / RATIO)

/*
 * Hack -- priority array (see below)
 *
 * Note that all "walls" always look like "secret doors" (see "map_info()").
 */
static byte priority_table[][2] =
{
	/* Dark */
	{ FEAT_NONE, 2 },

	/* Floors */
	{ FEAT_FLOOR, 5 },

	/* Walls */
	{ FEAT_SECRET, 10 },

	/* Quartz */
	{ FEAT_QUARTZ, 11 },

	/* Magma */
	{ FEAT_MAGMA, 12 },

	/* Rubble */
	{ FEAT_RUBBLE, 13 },

	/* Sandwall */
	{ FEAT_SANDWALL, 14 },

	/* Open doors */
	{ FEAT_OPEN, 15 },
	{ FEAT_BROKEN, 15 },

	/* Closed doors */
	{ FEAT_DOOR_HEAD + 0x00, 17 },

	/* Hidden gold */
	{ FEAT_QUARTZ_K, 19 },
	{ FEAT_MAGMA_K, 19 },
	{ FEAT_SANDWALL_K, 19 },

	/* water, lava, & trees oh my! -KMW- */
	{ FEAT_DEEP_WATER, 20 },
	{ FEAT_SHAL_WATER, 20 },
	{ FEAT_DEEP_LAVA, 20 },
	{ FEAT_SHAL_LAVA, 20 },
	{ FEAT_DIRT, 20 },
	{ FEAT_GRASS, 20 },
	{ FEAT_DARK_PIT, 20 },
	{ FEAT_TREES, 20 },
	{ FEAT_MOUNTAIN, 20 },
	{ FEAT_ICE, 20},
	{ FEAT_SAND, 20},
	{ FEAT_DEAD_TREE, 20},
	{ FEAT_ASH, 20},
	{ FEAT_MUD, 20},

	/* Fountain */
	{ FEAT_FOUNTAIN, 22 },
	{ FEAT_EMPTY_FOUNTAIN, 22 },

	/* Stairs */
	{ FEAT_LESS, 25 },
	{ FEAT_MORE, 25 },

	/* Stairs */
	{ FEAT_WAY_LESS, 25 },
	{ FEAT_WAY_MORE, 25 },

	{ FEAT_SHAFT_UP, 25 },
	{ FEAT_SHAFT_DOWN, 25 },

	/* End */
	{ 0, 0 }
};


/*
 * Hack -- a priority function (see below)
 */
static byte priority(byte a, char c)
{
	int i, p0, p1;

	feature_type *f_ptr;

	/* Scan the table */
	for (i = 0; TRUE; i++)
	{
		/* Priority level */
		p1 = priority_table[i][1];

		/* End of table */
		if (!p1) break;

		/* Feature index */
		p0 = priority_table[i][0];

		/* Access the feature */
		f_ptr = &f_info[p0];

		/* Check character and attribute, accept matches */
		if ((f_ptr->x_char == c) && (f_ptr->x_attr == a)) return (p1);
	}

	/* Default */
	return (20);
}


/*
 * Display a "small-scale" map of the dungeon in the active Term
 *
 * Note that the "map_info()" function must return fully colorized
 * data or this function will not work correctly.
 *
 * Note that this function must "disable" the special lighting
 * effects so that the "priority" function will work.
 *
 * Note the use of a specialized "priority" function to allow this
 * function to work with any graphic attr/char mappings, and the
 * attempts to optimize this function where possible.
 */
void display_map(int *cy, int *cx)
{
	int i, j, x, y;

	byte ta;
	char tc;

	byte tp;

	byte **ma;
	char **mc;

	byte **mp;

	bool old_view_special_lite;
	bool old_view_granite_lite;

	int hgt, wid, yrat, xrat, yfactor, xfactor;


	/* Obtain current size of the Angband window */
	Term_get_size(&wid, &hgt);

	/* Use two characters as one tile in Bigtile mode */
	if (use_bigtile) wid /= 2;

	/*
	 * Calculate the size of the dungeon map area
	 */
	hgt -= ROW_MAP + 2;
	wid -= COL_MAP + 1;

	/* Paranoia */
	if ((hgt < 3) || (wid < 3))
	{
		/* Map is too small, but place the player anyway */
		*cy = ROW_MAP;
		*cx = COL_MAP;

		return;
	}


	/* Save lighting effects */
	old_view_special_lite = view_special_lite;
	old_view_granite_lite = view_granite_lite;

	/* Disable lighting effects */
	view_special_lite = FALSE;
	view_granite_lite = FALSE;


	/* Allocate temporary memory for the maps */
	C_MAKE(ma, hgt + 2, byte *);
	C_MAKE(mc, hgt + 2, char *);
	C_MAKE(mp, hgt + 2, byte *);

	/* Allocate each line in the maps */
	for (i = 0; i < hgt + 2; i++)
	{
		C_MAKE(ma[i], wid + 2, byte);
		C_MAKE(mc[i], wid + 2, char);
		C_MAKE(mp[i], wid + 2, byte);
	}

	/* Clear the chars and attributes */
	for (y = 0; y < hgt + 2; ++y)
	{
		for (x = 0; x < wid + 2; ++x)
		{
			/* Nothing here */
			ma[y][x] = TERM_WHITE;
			mc[y][x] = ' ';

			/* No priority */
			mp[y][x] = 0;
		}
	}

	/* Calculate scaling factors */
	yfactor = ((cur_hgt / hgt < 4) && (cur_hgt > hgt)) ? 10 : 1;
	xfactor = ((cur_wid / wid < 4) && (cur_wid > wid)) ? 10 : 1;

	yrat = (cur_hgt * yfactor + (hgt - 1)) / hgt;
	xrat = (cur_wid * xfactor + (wid - 1)) / wid;

	/* Fill in the map */
	for (j = 0; j < cur_hgt; ++j)
	{
		for (i = 0; i < cur_wid; ++i)
		{
			/* Location */
			y = j * yfactor / yrat + 1;
			x = i * xfactor / xrat + 1;

			/* Extract the current attr/char at that map location */
#ifdef USE_TRANSPARENCY
# ifdef USE_EGO_GRAPHICS
			map_info(j, i, &ta, &tc, &ta, &tc, &ta, &tc);
# else /* USE_EGO_GRAPHICS */
			map_info(j, i, &ta, &tc, &ta, &tc);
# endif  /* USE_EGO_GRAPHICS */
#else /* USE_TRANSPARENCY */
			map_info(j, i, &ta, &tc);
#endif /* USE_TRANSPARENCY */

			/* Extract the priority of that attr/char */
			tp = priority(ta, tc);

			/* Player location has the highest priority */
			if ((p_ptr->py == j) && (p_ptr->px == i)) tp = 255;

			/* Save "best" */
			if (mp[y][x] < tp)
			{
				/* Save the char */
				mc[y][x] = tc;

				/* Save the attr */
				ma[y][x] = ta;

				/* Save priority */
				mp[y][x] = tp;
			}
		}
	}


	/* Corners */
	y = hgt + 1;
	x = wid + 1;

	/* Draw the corners */
	mc[0][0] = mc[0][x] = mc[y][0] = mc[y][x] = '+';

	/* Draw the horizontal edges */
	for (x = 1; x <= wid; x++) mc[0][x] = mc[y][x] = '-';

	/* Draw the vertical edges */
	for (y = 1; y <= hgt; y++) mc[y][0] = mc[y][x] = '|';


	/* Display each map line in order */
	for (y = 0; y < hgt + 2; ++y)
	{
		/* Start a new line */
		Term_gotoxy(COL_MAP - 1, y);

		/* Display the line */
		for (x = 0; x < wid + 2; ++x)
		{
			ta = ma[y][x];
			tc = mc[y][x];

			/* Add the character */
			Term_addch(ta, tc);

			/* Double width tile mode requires filler */
			if (use_bigtile)
			{
				byte a2;
				char c2;

				if (ta & 0x80)
				{
					/* Mega-Hack */
					a2 = 255;
					c2 = 255;
				}
				else
				{
					a2 = TERM_WHITE;
					c2 = ' ';
				}

				Term_addch(a2, c2);
			}
		}
	}

	/* Player location in dungeon */
	*cy = p_ptr->py * yfactor / yrat + ROW_MAP;
	if (!use_bigtile)
	{
		*cx = p_ptr->px * xfactor / xrat + COL_MAP;
	}
	else
	{
		*cx = (p_ptr->px * xfactor / xrat + 1) * 2 - 1 + COL_MAP;
	}

	/* Free each line in the maps */
	for (i = 0; i < hgt + 2; i++)
	{
		C_FREE(ma[i], wid + 2, byte);
		C_FREE(mc[i], wid + 2, char);
		C_FREE(mp[i], wid + 2, byte);
	}

	/* Allocate temporary memory for the maps */
	C_FREE(ma, hgt + 2, byte *);
	C_FREE(mc, hgt + 2, char *);
	C_FREE(mp, hgt + 2, byte *);


	/* Restore lighting effects */
	view_special_lite = old_view_special_lite;
	view_granite_lite = old_view_granite_lite;
}


/*
 * Display a "small-scale" map of the dungeon for the player
 *
 * Currently, the "player" is displayed on the map.  XXX XXX XXX
 */
void do_cmd_view_map(void)
{
	int cy, cx;
	int wid, hgt;

	/* Retrive current screen size */
	Term_get_size(&wid, &hgt);

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Note */
	prt("Please wait...", 0, 0);

	/* Flush */
	Term_fresh();

	/* Clear the screen */
	Term_clear();

	/* Display the map */
	display_map(&cy, &cx);

	/* Wait for it */
	put_str("Hit any key to continue", hgt - 1, (wid - COL_MAP) / 2);

	/* Hilite the player */
	move_cursor(cy, cx);

	/* Get any key */
	inkey();

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}






/*
 * Some comments on the dungeon related data structures and functions...
 *
 * Angband is primarily a dungeon exploration game, and it should come as
 * no surprise that the internal representation of the dungeon has evolved
 * over time in much the same way as the game itself, to provide semantic
 * changes to the game itself, to make the code simpler to understand, and
 * to make the executable itself faster or more efficient in various ways.
 *
 * There are a variety of dungeon related data structures, and associated
 * functions, which store information about the dungeon, and provide methods
 * by which this information can be accessed or modified.
 *
 * Some of this information applies to the dungeon as a whole, such as the
 * list of unique monsters which are still alive.  Some of this information
 * only applies to the current dungeon level, such as the current depth, or
 * the list of monsters currently inhabiting the level.  And some of the
 * information only applies to a single grid of the current dungeon level,
 * such as whether the grid is illuminated, or whether the grid contains a
 * monster, or whether the grid can be seen by the player.  If Angband was
 * to be turned into a multi-player game, some of the information currently
 * associated with the dungeon should really be associated with the player,
 * such as whether a given grid is viewable by a given player.
 *
 * One of the major bottlenecks in ancient versions of Angband was in the
 * calculation of "line of sight" from the player to various grids, such
 * as those containing monsters, using the relatively expensive "los()"
 * function.  This was such a nasty bottleneck that a lot of silly things
 * were done to reduce the dependancy on "line of sight", for example, you
 * could not "see" any grids in a lit room until you actually entered the
 * room, at which point every grid in the room became "illuminated" and
 * all of the grids in the room were "memorized" forever.  Other major
 * bottlenecks involved the determination of whether a grid was lit by the
 * player's torch, and whether a grid blocked the player's line of sight.
 * These bottlenecks led to the development of special new functions to
 * optimize issues involved with "line of sight" and "torch lit grids".
 * These optimizations led to entirely new additions to the game, such as
 * the ability to display the player's entire field of view using different
 * colors than were used for the "memorized" portions of the dungeon, and
 * the ability to memorize dark floor grids, but to indicate by the way in
 * which they are displayed that they are not actually illuminated.  And
 * of course many of them simply made the game itself faster or more fun.
 * Also, over time, the definition of "line of sight" has been relaxed to
 * allow the player to see a wider "field of view", which is slightly more
 * realistic, and only slightly more expensive to maintain.
 *
 * Currently, a lot of the information about the dungeon is stored in ways
 * that make it very efficient to access or modify the information, while
 * still attempting to be relatively conservative about memory usage, even
 * if this means that some information is stored in multiple places, or in
 * ways which require the use of special code idioms.  For example, each
 * monster record in the monster array contains the location of the monster,
 * and each cave grid has an index into the monster array, or a zero if no
 * monster is in the grid.  This allows the monster code to efficiently see
 * where the monster is located, while allowing the dungeon code to quickly
 * determine not only if a monster is present in a given grid, but also to
 * find out which monster.  The extra space used to store the information
 * twice is inconsequential compared to the speed increase.
 *
 * Some of the information about the dungeon is used by functions which can
 * constitute the "critical efficiency path" of the game itself, and so the
 * way in which they are stored and accessed has been optimized in order to
 * optimize the game itself.  For example, the "update_view()" function was
 * originally created to speed up the game itself (when the player was not
 * running), but then it took on extra responsibility as the provider of the
 * new "special effects lighting code", and became one of the most important
 * bottlenecks when the player was running.  So many rounds of optimization
 * were performed on both the function itself, and the data structures which
 * it uses, resulting eventually in a function which not only made the game
 * faster than before, but which was responsible for even more calculations
 * (including the determination of which grids are "viewable" by the player,
 * which grids are illuminated by the player's torch, and which grids can be
 * "seen" in some way by the player), as well as for providing the guts of
 * the special effects lighting code, and for the efficient redisplay of any
 * grids whose visual representation may have changed.
 *
 * Several pieces of information about each cave grid are stored in various
 * two dimensional arrays, with one unit of information for each grid in the
 * dungeon.  Some of these arrays have been intentionally expanded by a small
 * factor to make the two dimensional array accesses faster by allowing the
 * use of shifting instead of multiplication.
 *
 * Several pieces of information about each cave grid are stored in the
 * "cave_info" array, which is a special two dimensional array of bytes,
 * one for each cave grid, each containing eight separate "flags" which
 * describe some property of the cave grid.  These flags can be checked and
 * modified extremely quickly, especially when special idioms are used to
 * force the compiler to keep a local register pointing to the base of the
 * array.  Special location offset macros can be used to minimize the number
 * of computations which must be performed at runtime.  Note that using a
 * byte for each flag set may be slightly more efficient than using a larger
 * unit, so if another flag (or two) is needed later, and it must be fast,
 * then the two existing flags which do not have to be fast should be moved
 * out into some other data structure and the new flags should take their
 * place.  This may require a few minor changes in the savefile code.
 *
 * The "CAVE_ROOM" flag is saved in the savefile and is used to determine
 * which grids are part of "rooms", and thus which grids are affected by
 * "illumination" spells.  This flag does not have to be very fast.
 *
 * The "CAVE_ICKY" flag is saved in the savefile and is used to determine
 * which grids are part of "vaults", and thus which grids cannot serve as
 * the destinations of player teleportation.  This flag does not have to
 * be very fast.
 *
 * The "CAVE_MARK" flag is saved in the savefile and is used to determine
 * which grids have been "memorized" by the player.  This flag is used by
 * the "map_info()" function to determine if a grid should be displayed.
 * This flag is used in a few other places to determine if the player can
 * "know" about a given grid.  This flag must be very fast. 
 *
 * The "CAVE_GLOW" flag is saved in the savefile and is used to determine
 * which grids are "permanently illuminated".  This flag is used by the
 * "update_view()" function to help determine which viewable flags may
 * be "seen" by the player.  This flag is used by the "map_info" function
 * to determine if a grid is only lit by the player's torch.  This flag
 * has special semantics for wall grids (see "update_view()").  This flag
 * must be very fast.
 *
 * The "CAVE_WALL" flag is used to determine which grids block the player's
 * line of sight.  This flag is used by the "update_view()" function to
 * determine which grids block line of sight, and to help determine which
 * grids can be "seen" by the player.  This flag must be very fast.
 *
 * The "CAVE_VIEW" flag is used to determine which grids are currently in
 * line of sight of the player.  This flag is set by (and used by) the
 * "update_view()" function.  This flag is used by any code which needs to
 * know if the player can "view" a given grid.  This flag is used by the
 * "map_info()" function for some optional special lighting effects.  The
 * "player_has_los_bold()" macro wraps an abstraction around this flag, but
 * certain code idioms are much more efficient.  This flag is used to check
 * if a modification to a terrain feature might affect the player's field of
 * view.  This flag is used to see if certain monsters are "visible" to the
 * player.  This flag is used to allow any monster in the player's field of
 * view to "sense" the presence of the player.  This flag must be very fast.
 *
 * The "CAVE_SEEN" flag is used to determine which grids are currently in
 * line of sight of the player and also illuminated in some way.  This flag
 * is set by the "update_view()" function, using computations based on the
 * "CAVE_VIEW" and "CAVE_WALL" and "CAVE_GLOW" flags of various grids.  This
 * flag is used by any code which needs to know if the player can "see" a
 * given grid.  This flag is used by the "map_info()" function both to see
 * if a given "boring" grid can be seen by the player, and for some optional
 * special lighting effects.  The "player_can_see_bold()" macro wraps an
 * abstraction around this flag, but certain code idioms are much more
 * efficient.  This flag is used to see if certain monsters are "visible" to
 * the player.  This flag is never set for a grid unless "CAVE_VIEW" is also
 * set for the grid.  Whenever the "CAVE_WALL" or "CAVE_GLOW" flag changes
 * for a grid which has the "CAVE_VIEW" flag set, the "CAVE_SEEN" flag must
 * be recalculated.  The simplest way to do this is to call "forget_view()"
 * and "update_view()" whenever the "CAVE_WALL" or "CAVE_GLOW" flags change
 * for a grid which has "CAVE_VIEW" set.  This flag must be very fast.
 *
 * The "CAVE_TEMP" flag is used for a variety of temporary purposes.  This
 * flag is used to determine if the "CAVE_SEEN" flag for a grid has changed
 * during the "update_view()" function.  This flag is used to "spread" light
 * or darkness through a room.  This flag is used by the "monster flow code".
 * This flag must always be cleared by any code which sets it, often, this
 * can be optimized by the use of the special "temp_g", "temp_y", "temp_x"
 * arrays (and the special "temp_n" global).  This flag must be very fast.
 *
 * Note that the "CAVE_MARK" flag is used for many reasons, some of which
 * are strictly for optimization purposes.  The "CAVE_MARK" flag means that
 * even if the player cannot "see" the grid, he "knows" about the terrain in
 * that grid.  This is used to "memorize" grids when they are first "seen" by
 * the player, and to allow certain grids to be "detected" by certain magic.
 * Note that most grids are always memorized when they are first "seen", but
 * "boring" grids (floor grids) are only memorized if the "view_torch_grids"
 * option is set, or if the "view_perma_grids" option is set, and the grid
 * in question has the "CAVE_GLOW" flag set.
 *
 * Objects are "memorized" in a different way, using a special "marked" flag
 * on the object itself, which is set when an object is observed or detected.
 * This allows objects to be "memorized" independant of the terrain features.
 *
 * The "update_view()" function is an extremely important function.  It is
 * called only when the player moves, significant terrain changes, or the
 * player's blindness or torch radius changes.  Note that when the player
 * is resting, or performing any repeated actions (like digging, disarming,
 * farming, etc), there is no need to call the "update_view()" function, so
 * even if it was not very efficient, this would really only matter when the
 * player was "running" through the dungeon.  It sets the "CAVE_VIEW" flag
 * on every cave grid in the player's field of view, and maintains an array
 * of all such grids in the global "view_g" array.  It also checks the torch
 * radius of the player, and sets the "CAVE_SEEN" flag for every grid which
 * is in the "field of view" of the player and which is also "illuminated",
 * either by the players torch (if any) or by any permanent light source.
 * It could use and help maintain information about multiple light sources,
 * which would be helpful in a multi-player version of Angband.
 *
 * The "update_view()" function maintains the special "view_g" array, which
 * contains exactly those grids which have the "CAVE_VIEW" flag set.  This
 * array is used by "update_view()" to (only) memorize grids which become
 * newly "seen", and to (only) redraw grids whose "seen" value changes, which
 * allows the use of some interesting (and very efficient) "special lighting
 * effects".  In addition, this array could be used elsewhere to quickly scan
 * through all the grids which are in the player's field of view.
 *
 * Note that the "update_view()" function allows, among other things, a room
 * to be "partially" seen as the player approaches it, with a growing cone
 * of floor appearing as the player gets closer to the door.  Also, by not
 * turning on the "memorize perma-lit grids" option, the player will only
 * "see" those floor grids which are actually in line of sight.  And best
 * of all, you can now activate the special lighting effects to indicate
 * which grids are actually in the player's field of view by using dimmer
 * colors for grids which are not in the player's field of view, and/or to
 * indicate which grids are illuminated only by the player's torch by using
 * the color yellow for those grids.
 *
 * The old "update_view()" algorithm uses the special "CAVE_EASY" flag as a
 * temporary internal flag to mark those grids which are not only in view,
 * but which are also "easily" in line of sight of the player.  This flag
 * is actually just the "CAVE_SEEN" flag, and the "update_view()" function
 * makes sure to clear it for all old "CAVE_SEEN" grids, and then use it in
 * the algorithm as "CAVE_EASY", and then clear it for all "CAVE_EASY" grids,
 * and then reset it as appropriate for all new "CAVE_SEEN" grids.  This is
 * kind of messy, but it works.  The old algorithm may disappear eventually.
 *
 * The new "update_view()" algorithm uses a faster and more mathematically
 * correct algorithm, assisted by a large machine generated static array, to
 * determine the "CAVE_VIEW" and "CAVE_SEEN" flags simultaneously.  See below.
 *
 * It seems as though slight modifications to the "update_view()" functions
 * would allow us to determine "reverse" line-of-sight as well as "normal"
 * line-of-sight", which would allow monsters to have a more "correct" way
 * to determine if they can "see" the player, since right now, they "cheat"
 * somewhat and assume that if the player has "line of sight" to them, then
 * they can "pretend" that they have "line of sight" to the player.  But if
 * such a change was attempted, the monsters would actually start to exhibit
 * some undesirable behavior, such as "freezing" near the entrances to long
 * hallways containing the player, and code would have to be added to make
 * the monsters move around even if the player was not detectable, and to
 * "remember" where the player was last seen, to avoid looking stupid.
 *
 * Note that the "CAVE_GLOW" flag means that a grid is permanently lit in
 * some way.  However, for the player to "see" the grid, as determined by
 * the "CAVE_SEEN" flag, the player must not be blind, the grid must have
 * the "CAVE_VIEW" flag set, and if the grid is a "wall" grid, and it is
 * not lit by the player's torch, then it must touch a grid which does not
 * have the "CAVE_WALL" flag set, but which does have both the "CAVE_GLOW"
 * and "CAVE_VIEW" flags set.  This last part about wall grids is induced
 * by the semantics of "CAVE_GLOW" as applied to wall grids, and checking
 * the technical requirements can be very expensive, especially since the
 * grid may be touching some "illegal" grids.  Luckily, it is more or less
 * correct to restrict the "touching" grids from the eight "possible" grids
 * to the (at most) three grids which are touching the grid, and which are
 * closer to the player than the grid itself, which eliminates more than
 * half of the work, including all of the potentially "illegal" grids, if
 * at most one of the three grids is a "diagonal" grid.  In addition, in
 * almost every situation, it is possible to ignore the "CAVE_VIEW" flag
 * on these three "touching" grids, for a variety of technical reasons.
 * Finally, note that in most situations, it is only necessary to check
 * a single "touching" grid, in fact, the grid which is strictly closest
 * to the player of all the touching grids, and in fact, it is normally
 * only necessary to check the "CAVE_GLOW" flag of that grid, again, for
 * various technical reasons.  However, one of the situations which does
 * not work with this last reduction is the very common one in which the
 * player approaches an illuminated room from a dark hallway, in which the
 * two wall grids which form the "entrance" to the room would not be marked
 * as "CAVE_SEEN", since of the three "touching" grids nearer to the player
 * than each wall grid, only the farthest of these grids is itself marked
 * "CAVE_GLOW". 
 *
 *
 * Here are some pictures of the legal "light source" radius values, in
 * which the numbers indicate the "order" in which the grids could have
 * been calculated, if desired.  Note that the code will work with larger
 * radiuses, though currently yields such a radius, and the game would
 * become slower in some situations if it did.
 *
 *       Rad=0     Rad=1      Rad=2        Rad=3
 *      No-Lite  Torch,etc   Lantern     Artifacts
 *    
 *                                          333
 *                             333         43334
 *                  212       32123       3321233
 *         @        1@1       31@13       331@133
 *                  212       32123       3321233
 *                             333         43334
 *                                          333
 *
 *
 * Here is an illustration of the two different "update_view()" algorithms,
 * in which the grids marked "%" are pillars, and the grids marked "?" are
 * not in line of sight of the player.
 *
 *
 *                    Sample situation
 *
 *                  #####################
 *                  ############.%.%.%.%#
 *                  #...@..#####........#
 *                  #............%.%.%.%#
 *                  #......#####........#
 *                  ############........#
 *                  #####################
 *
 *
 *          New Algorithm             Old Algorithm
 *
 *      ########?????????????    ########?????????????
 *      #...@..#?????????????    #...@..#?????????????
 *      #...........?????????    #.........???????????
 *      #......#####.....????    #......####??????????
 *      ########?????????...#    ########?????????????
 *
 *      ########?????????????    ########?????????????
 *      #.@....#?????????????    #.@....#?????????????
 *      #............%???????    #...........?????????
 *      #......#####........?    #......#####?????????
 *      ########??????????..#    ########?????????????
 *
 *      ########?????????????    ########?????%???????
 *      #......#####........#    #......#####..???????
 *      #.@..........%???????    #.@..........%???????
 *      #......#####........#    #......#####..???????
 *      ########?????????????    ########?????????????
 *
 *      ########??????????..#    ########?????????????
 *      #......#####........?    #......#####?????????
 *      #............%???????    #...........?????????
 *      #.@....#?????????????    #.@....#?????????????
 *      ########?????????????    ########?????????????
 *
 *      ########?????????%???    ########?????????????
 *      #......#####.....????    #......####??????????
 *      #...........?????????    #.........???????????
 *      #...@..#?????????????    #...@..#?????????????
 *      ########?????????????    ########?????????????
 */




/*
 * Maximum number of grids in a single octant
 */
#define VINFO_MAX_GRIDS 161


/*
 * Maximum number of slopes in a single octant
 */
#define VINFO_MAX_SLOPES 126


/*
 * Mask of bits used in a single octant
 */
#define VINFO_BITS_3 0x3FFFFFFF
#define VINFO_BITS_2 0xFFFFFFFF
#define VINFO_BITS_1 0xFFFFFFFF
#define VINFO_BITS_0 0xFFFFFFFF


/*
 * Forward declare
 */
typedef struct vinfo_type vinfo_type;


/*
 * The 'vinfo_type' structure
 */
struct vinfo_type
{
	s16b grid_y[8];
	s16b grid_x[8];

	u32b bits_3;
	u32b bits_2;
	u32b bits_1;
	u32b bits_0;

	vinfo_type *next_0;
	vinfo_type *next_1;

	byte y;
	byte x;
	byte d;
	byte r;
};



/*
 * The array of "vinfo" objects, initialized by "vinfo_init()"
 */
static vinfo_type vinfo[VINFO_MAX_GRIDS];




/*
 * Slope scale factor
 */
#define SCALE 100000L


/*
 * The actual slopes (for reference)
 */

/* Bit :     Slope   Grids */
/* --- :     -----   ----- */
/*   0 :      2439      21 */
/*   1 :      2564      21 */
/*   2 :      2702      21 */
/*   3 :      2857      21 */
/*   4 :      3030      21 */
/*   5 :      3225      21 */
/*   6 :      3448      21 */
/*   7 :      3703      21 */
/*   8 :      4000      21 */
/*   9 :      4347      21 */
/*  10 :      4761      21 */
/*  11 :      5263      21 */
/*  12 :      5882      21 */
/*  13 :      6666      21 */
/*  14 :      7317      22 */
/*  15 :      7692      20 */
/*  16 :      8108      21 */
/*  17 :      8571      21 */
/*  18 :      9090      20 */
/*  19 :      9677      21 */
/*  20 :     10344      21 */
/*  21 :     11111      20 */
/*  22 :     12000      21 */
/*  23 :     12820      22 */
/*  24 :     13043      22 */
/*  25 :     13513      22 */
/*  26 :     14285      20 */
/*  27 :     15151      22 */
/*  28 :     15789      22 */
/*  29 :     16129      22 */
/*  30 :     17241      22 */
/*  31 :     17647      22 */
/*  32 :     17948      23 */
/*  33 :     18518      22 */
/*  34 :     18918      22 */
/*  35 :     20000      19 */
/*  36 :     21212      22 */
/*  37 :     21739      22 */
/*  38 :     22580      22 */
/*  39 :     23076      22 */
/*  40 :     23809      22 */
/*  41 :     24137      22 */
/*  42 :     24324      23 */
/*  43 :     25714      23 */
/*  44 :     25925      23 */
/*  45 :     26315      23 */
/*  46 :     27272      22 */
/*  47 :     28000      23 */
/*  48 :     29032      23 */
/*  49 :     29411      23 */
/*  50 :     29729      24 */
/*  51 :     30434      23 */
/*  52 :     31034      23 */
/*  53 :     31428      23 */
/*  54 :     33333      18 */
/*  55 :     35483      23 */
/*  56 :     36000      23 */
/*  57 :     36842      23 */
/*  58 :     37142      24 */
/*  59 :     37931      24 */
/*  60 :     38461      24 */
/*  61 :     39130      24 */
/*  62 :     39393      24 */
/*  63 :     40740      24 */
/*  64 :     41176      24 */
/*  65 :     41935      24 */
/*  66 :     42857      23 */
/*  67 :     44000      24 */
/*  68 :     44827      24 */
/*  69 :     45454      23 */
/*  70 :     46666      24 */
/*  71 :     47368      24 */
/*  72 :     47826      24 */
/*  73 :     48148      24 */
/*  74 :     48387      24 */
/*  75 :     51515      25 */
/*  76 :     51724      25 */
/*  77 :     52000      25 */
/*  78 :     52380      25 */
/*  79 :     52941      25 */
/*  80 :     53846      25 */
/*  81 :     54838      25 */
/*  82 :     55555      24 */
/*  83 :     56521      25 */
/*  84 :     57575      26 */
/*  85 :     57894      25 */
/*  86 :     58620      25 */
/*  87 :     60000      23 */
/*  88 :     61290      25 */
/*  89 :     61904      25 */
/*  90 :     62962      25 */
/*  91 :     63636      25 */
/*  92 :     64705      25 */
/*  93 :     65217      25 */
/*  94 :     65517      25 */
/*  95 :     67741      26 */
/*  96 :     68000      26 */
/*  97 :     68421      26 */
/*  98 :     69230      26 */
/*  99 :     70370      26 */
/* 100 :     71428      25 */
/* 101 :     72413      26 */
/* 102 :     73333      26 */
/* 103 :     73913      26 */
/* 104 :     74193      27 */
/* 105 :     76000      26 */
/* 106 :     76470      26 */
/* 107 :     77777      25 */
/* 108 :     78947      26 */
/* 109 :     79310      26 */
/* 110 :     80952      26 */
/* 111 :     81818      26 */
/* 112 :     82608      26 */
/* 113 :     84000      26 */
/* 114 :     84615      26 */
/* 115 :     85185      26 */
/* 116 :     86206      27 */
/* 117 :     86666      27 */
/* 118 :     88235      27 */
/* 119 :     89473      27 */
/* 120 :     90476      27 */
/* 121 :     91304      27 */
/* 122 :     92000      27 */
/* 123 :     92592      27 */
/* 124 :     93103      28 */
/* 125 :    100000      13 */



/*
 * Forward declare
 */
typedef struct vinfo_hack vinfo_hack;


/*
 * Temporary data used by "vinfo_init()"
 *
 *	- Number of grids
 *
 *	- Number of slopes
 *
 *	- Slope values
 *
 *	- Slope range per grid
 */
struct vinfo_hack
{

	int num_slopes;

	long slopes[VINFO_MAX_SLOPES];

	long slopes_min[MAX_SIGHT + 1][MAX_SIGHT + 1];
	long slopes_max[MAX_SIGHT + 1][MAX_SIGHT + 1];
};



/*
 * Sorting hook -- comp function -- array of long's (see below)
 *
 * We use "u" to point to an array of long integers.
 */
static bool ang_sort_comp_hook_longs(vptr u, vptr v, int a, int b)
{
	long *x = (long*)(u);

	return (x[a] <= x[b]);
}


/*
 * Sorting hook -- comp function -- array of long's (see below)
 *
 * We use "u" to point to an array of long integers.
 */
static void ang_sort_swap_hook_longs(vptr u, vptr v, int a, int b)
{
	long *x = (long*)(u);

	long temp;

	/* Swap */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;
}



/*
 * Save a slope
 */
static void vinfo_init_aux(vinfo_hack *hack, int y, int x, long m)
{
	int i;

	/* Handle "legal" slopes */
	if ((m > 0) && (m <= SCALE))
	{
		/* Look for that slope */
		for (i = 0; i < hack->num_slopes; i++)
		{
			if (hack->slopes[i] == m) break;
		}

		/* New slope */
		if (i == hack->num_slopes)
		{
			/* Paranoia */
			if (hack->num_slopes >= VINFO_MAX_SLOPES)
			{
				quit_fmt("Too many slopes (%d)!",
				         VINFO_MAX_SLOPES);
			}

			/* Save the slope, and advance */
			hack->slopes[hack->num_slopes++] = m;
		}
	}

	/* Track slope range */
	if (hack->slopes_min[y][x] > m) hack->slopes_min[y][x] = m;
	if (hack->slopes_max[y][x] < m) hack->slopes_max[y][x] = m;
}



/*
 * Initialize the "vinfo" array
 *
 * Full Octagon (radius 20), Grids=1149
 *
 * Quadrant (south east), Grids=308, Slopes=251
 *
 * Octant (east then south), Grids=161, Slopes=126
 *
 * This function assumes that VINFO_MAX_GRIDS and VINFO_MAX_SLOPES
 * have the correct values, which can be derived by setting them to
 * a number which is too high, running this function, and using the
 * error messages to obtain the correct values.
 */
errr vinfo_init(void)
{
	int i, y, x;

	long m;

	vinfo_hack *hack;

	int num_grids = 0;

	int queue_head = 0;
	int queue_tail = 0;
	vinfo_type *queue[VINFO_MAX_GRIDS*2];


	/* Make hack */
	MAKE(hack, vinfo_hack);


	/* Analyze grids */
	for (y = 0; y <= MAX_SIGHT; ++y)
	{
		for (x = y; x <= MAX_SIGHT; ++x)
		{
			/* Skip grids which are out of sight range */
			if (distance(0, 0, y, x) > MAX_SIGHT) continue;

			/* Default slope range */
			hack->slopes_min[y][x] = 999999999;
			hack->slopes_max[y][x] = 0;

			/* Paranoia */
			if (num_grids >= VINFO_MAX_GRIDS)
			{
				quit_fmt("Too many grids (%d >= %d)!",
				         num_grids, VINFO_MAX_GRIDS);
			}

			/* Count grids */
			num_grids++;

			/* Slope to the top right corner */
			m = SCALE * (1000L * y - 500) / (1000L * x + 500);

			/* Handle "legal" slopes */
			vinfo_init_aux(hack, y, x, m);

			/* Slope to top left corner */
			m = SCALE * (1000L * y - 500) / (1000L * x - 500);

			/* Handle "legal" slopes */
			vinfo_init_aux(hack, y, x, m);

			/* Slope to bottom right corner */
			m = SCALE * (1000L * y + 500) / (1000L * x + 500);

			/* Handle "legal" slopes */
			vinfo_init_aux(hack, y, x, m);

			/* Slope to bottom left corner */
			m = SCALE * (1000L * y + 500) / (1000L * x - 500);

			/* Handle "legal" slopes */
			vinfo_init_aux(hack, y, x, m);
		}
	}


	/* Enforce maximal efficiency */
	if (num_grids < VINFO_MAX_GRIDS)
	{
		quit_fmt("Too few grids (%d < %d)!",
		         num_grids, VINFO_MAX_GRIDS);
	}

	/* Enforce maximal efficiency */
	if (hack->num_slopes < VINFO_MAX_SLOPES)
	{
		quit_fmt("Too few slopes (%d < %d)!",
		         hack->num_slopes, VINFO_MAX_SLOPES);
	}


	/* Sort slopes numerically */
	ang_sort_comp = ang_sort_comp_hook_longs;

	/* Sort slopes numerically */
	ang_sort_swap = ang_sort_swap_hook_longs;

	/* Sort the (unique) slopes */
	ang_sort(hack->slopes, NULL, hack->num_slopes);



	/* Enqueue player grid */
	queue[queue_tail++] = &vinfo[0];

	/* Process queue */
	while (queue_head < queue_tail)
	{
		int e;

		vinfo_type *p;


		/* Index */
		e = queue_head;

		/* Dequeue next grid */
		p = queue[queue_head++];

		/* Location of main grid */
		y = vinfo[e].grid_y[0];
		x = vinfo[e].grid_x[0];


		/* Compute grid offsets */
		vinfo[e].grid_y[0] = + y;
		vinfo[e].grid_x[0] = + x;
		vinfo[e].grid_y[1] = + x;
		vinfo[e].grid_x[1] = + y;
		vinfo[e].grid_y[2] = + x;
		vinfo[e].grid_x[2] = -y;
		vinfo[e].grid_y[3] = + y;
		vinfo[e].grid_x[3] = -x;
		vinfo[e].grid_y[4] = -y;
		vinfo[e].grid_x[4] = -x;
		vinfo[e].grid_y[5] = -x;
		vinfo[e].grid_x[5] = -y;
		vinfo[e].grid_y[6] = -x;
		vinfo[e].grid_x[6] = + y;
		vinfo[e].grid_y[7] = -y;
		vinfo[e].grid_x[7] = + x;


		/* Analyze slopes */
		for (i = 0; i < hack->num_slopes; ++i)
		{
			m = hack->slopes[i];

			/* Memorize intersection slopes (for non-player-grids) */
			if ((e > 0) &&
			                (hack->slopes_min[y][x] < m) &&
			                (m < hack->slopes_max[y][x]))
			{
				switch (i / 32)
				{
				case 3:
					vinfo[e].bits_3 |= (1L << (i % 32));
					break;
				case 2:
					vinfo[e].bits_2 |= (1L << (i % 32));
					break;
				case 1:
					vinfo[e].bits_1 |= (1L << (i % 32));
					break;
				case 0:
					vinfo[e].bits_0 |= (1L << (i % 32));
					break;
				}
			}
		}


		/* Default */
		vinfo[e].next_0 = &vinfo[0];

		/* Grid next child */
		if (distance(0, 0, y, x + 1) <= MAX_SIGHT)
		{
			if ((queue[queue_tail - 1]->grid_y[0] != y) ||
			                (queue[queue_tail - 1]->grid_x[0] != x + 1))
			{
				vinfo[queue_tail].grid_y[0] = y;
				vinfo[queue_tail].grid_x[0] = x + 1;
				queue[queue_tail] = &vinfo[queue_tail];
				queue_tail++;
			}

			vinfo[e].next_0 = &vinfo[queue_tail - 1];
		}


		/* Default */
		vinfo[e].next_1 = &vinfo[0];

		/* Grid diag child */
		if (distance(0, 0, y + 1, x + 1) <= MAX_SIGHT)
		{
			if ((queue[queue_tail - 1]->grid_y[0] != y + 1) ||
			                (queue[queue_tail - 1]->grid_x[0] != x + 1))
			{
				vinfo[queue_tail].grid_y[0] = y + 1;
				vinfo[queue_tail].grid_x[0] = x + 1;
				queue[queue_tail] = &vinfo[queue_tail];
				queue_tail++;
			}

			vinfo[e].next_1 = &vinfo[queue_tail - 1];
		}


		/* Hack -- main diagonal has special children */
		if (y == x) vinfo[e].next_0 = vinfo[e].next_1;


		/* Extra values */
		vinfo[e].y = y;
		vinfo[e].x = x;
		vinfo[e].d = ((y > x) ? (y + x / 2) : (x + y / 2));
		vinfo[e].r = ((!y) ? x : (!x) ? y : (y == x) ? y : 0);
	}


	/* Verify maximal bits XXX XXX XXX */
	if (((vinfo[1].bits_3 | vinfo[2].bits_3) != VINFO_BITS_3) ||
	                ((vinfo[1].bits_2 | vinfo[2].bits_2) != VINFO_BITS_2) ||
	                ((vinfo[1].bits_1 | vinfo[2].bits_1) != VINFO_BITS_1) ||
	                ((vinfo[1].bits_0 | vinfo[2].bits_0) != VINFO_BITS_0))
	{
		quit("Incorrect bit masks!");
	}


	/* Kill hack */
	KILL(hack, vinfo_hack);


	/* Success */
	return (0);
}



/*
 * Forget the "CAVE_VIEW" grids, redrawing as needed
 */
void forget_view(void)
{
	int i;

	int fast_view_n = view_n;

	cave_type *c_ptr;


	/* None to forget */
	if (!fast_view_n) return;

	/* Clear them all */
	for (i = 0; i < fast_view_n; i++)
	{
		int y = view_y[i];
		int x = view_x[i];

		/* Access the grid */
		c_ptr = &cave[y][x];

		/* Clear "CAVE_VIEW", "CAVE_SEEN" and player torch flags */
		c_ptr->info &= ~(CAVE_VIEW | CAVE_SEEN | CAVE_PLIT);

		/* Redraw */
		lite_spot(y, x);
	}

	/* None left */
	view_n = 0;
}



/*
 * Calculate the complete field of view using a new algorithm
 *
 * If "view_y/x" and "temp_y/x" were global pointers to arrays of grids, as
 * opposed to actual arrays of grids, then we could be more efficient by
 * using "pointer swapping".
 *
 * Normally, vision along the major axes is more likely than vision
 * along the diagonal axes, so we check the bits corresponding to
 * the lines of sight near the major axes first.
 *
 * We use the "temp_y/x" array (and the "CAVE_TEMP" flag) to keep track of
 * which grids were previously marked "CAVE_SEEN", since only those grids
 * whose "CAVE_SEEN" value changes during this routine must be redrawn.
 *
 * This function is now responsible for maintaining the "CAVE_SEEN"
 * flags as well as the "CAVE_VIEW" flags, which is good, because
 * the only grids which normally need to be memorized and/or redrawn
 * are the ones whose "CAVE_SEEN" flag changes during this routine.
 *
 * Basically, this function divides the "octagon of view" into octants of
 * grids (where grids on the main axes and diagonal axes are "shared" by
 * two octants), and processes each octant one at a time, processing each
 * octant one grid at a time, processing only those grids which "might" be
 * viewable, and setting the "CAVE_VIEW" flag for each grid for which there
 * is an (unobstructed) line of sight from the center of the player grid to
 * any internal point in the grid (and collecting these "CAVE_VIEW" grids
 * into the "view_y/x" array), and setting the "CAVE_SEEN" flag for the grid
 * if, in addition, the grid is "illuminated" in some way.
 *
 * This function relies on a theorem (suggested and proven by Mat Hostetter)
 * which states that in each octant of a field of view, a given grid will
 * be "intersected" by one or more unobstructed "lines of sight" from the
 * center of the player grid if and only if it is "intersected" by at least
 * one such unobstructed "line of sight" which passes directly through some
 * corner of some grid in the octant which is not shared by any other octant.
 * The proof is based on the fact that there are at least three significant
 * lines of sight involving any non-shared grid in any octant, one which
 * intersects the grid and passes though the corner of the grid closest to
 * the player, and two which "brush" the grid, passing through the "outer"
 * corners of the grid, and that any line of sight which intersects a grid
 * without passing through the corner of a grid in the octant can be "slid"
 * slowly towards the corner of the grid closest to the player, until it
 * either reaches it or until it brushes the corner of another grid which
 * is closer to the player, and in either case, the existanc of a suitable
 * line of sight is thus demonstrated.
 *
 * It turns out that in each octant of the radius 20 "octagon of view",
 * there are 161 grids (with 128 not shared by any other octant), and there
 * are exactly 126 distinct "lines of sight" passing from the center of the
 * player grid through any corner of any non-shared grid in the octant.  To
 * determine if a grid is "viewable" by the player, therefore, you need to
 * simply show that one of these 126 lines of sight intersects the grid but
 * does not intersect any wall grid closer to the player.  So we simply use
 * a bit vector with 126 bits to represent the set of interesting lines of
 * sight which have not yet been obstructed by wall grids, and then we scan
 * all the grids in the octant, moving outwards from the player grid.  For
 * each grid, if any of the lines of sight which intersect that grid have not
 * yet been obstructed, then the grid is viewable.  Furthermore, if the grid
 * is a wall grid, then all of the lines of sight which intersect the grid
 * should be marked as obstructed for future reference.  Also, we only need
 * to check those grids for whom at least one of the "parents" was a viewable
 * non-wall grid, where the parents include the two grids touching the grid
 * but closer to the player grid (one adjacent, and one diagonal).  For the
 * bit vector, we simply use 4 32-bit integers.  All of the static values
 * which are needed by this function are stored in the large "vinfo" array
 * (above), which is machine generated by another program.  XXX XXX XXX
 *
 * Hack -- The queue must be able to hold more than VINFO_MAX_GRIDS grids
 * because the grids at the edge of the field of view use "grid zero" as
 * their children, and the queue must be able to hold several of these
 * special grids.  Because the actual number of required grids is bizarre,
 * we simply allocate twice as many as we would normally need.  XXX XXX XXX
 */
void update_view(void)
{
	int i, o;
	int y, x;

	int radius;

	int fast_view_n = view_n;

	int fast_temp_n = 0;

	cave_type *c_ptr;

	u16b info;


	/*** Step 0 -- Begin ***/

	/* Save the old "view" grids for later */
	for (i = 0; i < fast_view_n; i++)
	{
		/* Location */
		y = view_y[i];
		x = view_x[i];

		/* Grid */
		c_ptr = &cave[y][x];

		/* Get grid info */
		info = c_ptr->info;
		;

		/* Save "CAVE_SEEN" grids */
		if (info & (CAVE_SEEN))
		{
			/* Set "CAVE_TEMP" flag */
			info |= (CAVE_TEMP);

			/* Save grid for later */
			temp_y[fast_temp_n] = y;
			temp_x[fast_temp_n++] = x;
		}

		/* Clear "CAVE_VIEW", "CAVE_SEEN" and player torch flags */
		info &= ~(CAVE_VIEW | CAVE_SEEN | CAVE_PLIT);

		/* Save cave info */
		c_ptr->info = info;
	}

	/* Reset the "view" array */
	fast_view_n = 0;

	/* Extract "radius" value */
	radius = p_ptr->cur_lite;

	/* Handle real light */
	if (radius > 0) ++radius;


	/*** Step 1 -- player grid ***/

	/* Player grid */
	c_ptr = &cave[p_ptr->py][p_ptr->px];

	/* Get grid info */
	info = c_ptr->info;

	/* Assume viewable */
	info |= (CAVE_VIEW);

	/* Torch-lit grid */
	if (0 < radius)
	{
		/* Mark as "CAVE_SEEN" and torch-lit */
		info |= (CAVE_SEEN | CAVE_PLIT);
	}


	/* Perma-lit grid */
	else if (info & (CAVE_GLOW))
	{
		/* Mark as "CAVE_SEEN" */
		info |= (CAVE_SEEN);
	}

	/* Save cave info */
	c_ptr->info = info;

	/* Save in array */
	view_y[fast_view_n] = p_ptr->py;
	view_x[fast_view_n++] = p_ptr->px;


	/*** Step 2 -- octants ***/

	/* Scan each octant */
	for (o = 0; o < 8; o++)
	{
		vinfo_type *p;

		/* Last added */
		vinfo_type *last = &vinfo[0];

		/* Grid queue */
		int queue_head = 0;
		int queue_tail = 0;
		vinfo_type *queue[VINFO_MAX_GRIDS*2];

		/* Slope bit vector */
		u32b bits0 = VINFO_BITS_0;
		u32b bits1 = VINFO_BITS_1;
		u32b bits2 = VINFO_BITS_2;
		u32b bits3 = VINFO_BITS_3;

		/* Reset queue */
		queue_head = queue_tail = 0;

		/* Initial grids */
		queue[queue_tail++] = &vinfo[1];
		queue[queue_tail++] = &vinfo[2];

		/* Process queue */
		while (queue_head < queue_tail)
		{
			/* Dequeue next grid */
			p = queue[queue_head++];

			/* Check bits */
			if ((bits0 & (p->bits_0)) ||
			                (bits1 & (p->bits_1)) ||
			                (bits2 & (p->bits_2)) ||
			                (bits3 & (p->bits_3)))
			{
				/* Extract coordinate value */
				y = p_ptr->py + p->grid_y[o];
				x = p_ptr->px + p->grid_x[o];

				/* Access the grid */
				c_ptr = &cave[y][x];

				/* Get grid info */
				info = c_ptr->info;

				/* Handle wall */
				if (info & (CAVE_WALL))
				{
					/* Clear bits */
					bits0 &= ~(p->bits_0);
					bits1 &= ~(p->bits_1);
					bits2 &= ~(p->bits_2);
					bits3 &= ~(p->bits_3);

					/* Newly viewable wall */
					if (!(info & (CAVE_VIEW)))
					{
						/* Mark as viewable */
						info |= (CAVE_VIEW);

						/* Torch-lit grids */
						if (p->d < radius)
						{
							/* Mark as "CAVE_SEEN" and torch-lit */
							info |= (CAVE_SEEN | CAVE_PLIT);
						}

						/* Monster-lit grids */
						else if (info & (CAVE_MLIT))
						{
							/* Mark as "CAVE_SEEN" */
							info |= (CAVE_SEEN);
						}

						/* Perma-lit grids */
						else if (info & (CAVE_GLOW))
						{
							/* Hack -- move towards player */
							int yy = (y < p_ptr->py) ? (y + 1) : (y > p_ptr->py) ? (y - 1) : y;
							int xx = (x < p_ptr->px) ? (x + 1) : (x > p_ptr->px) ? (x - 1) : x;

#ifdef UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION

							/* Check for "complex" illumination */
							if ((!(cave[yy][xx].info & (CAVE_WALL)) &&
							                (cave[yy][xx].info & (CAVE_GLOW))) ||
							                (!(cave[y][xx].info & (CAVE_WALL)) &&
							                 (cave[y][xx].info & (CAVE_GLOW))) ||
							                (!(cave[yy][x].info & (CAVE_WALL)) &&
							                 (cave[yy][x].info & (CAVE_GLOW))))
							{
								/* Mark as seen */
								info |= (CAVE_SEEN);
							}

#else /* UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION */

							/* Check for "simple" illumination */
							if (cave[yy][xx].info & (CAVE_GLOW))
							{
								/* Mark as seen */
								info |= (CAVE_SEEN);
							}

#endif /* UPDATE_VIEW_COMPLEX_WALL_ILLUMINATION */

						}

						/* Save cave info */
						c_ptr->info = info;

						/* Save in array */
						view_y[fast_view_n] = y;
						view_x[fast_view_n++] = x;
					}
				}

				/* Handle non-wall */
				else
				{
					/* Enqueue child */
					if (last != p->next_0)
					{
						queue[queue_tail++] = last = p->next_0;
					}

					/* Enqueue child */
					if (last != p->next_1)
					{
						queue[queue_tail++] = last = p->next_1;
					}

					/* Newly viewable non-wall */
					if (!(info & (CAVE_VIEW)))
					{
						/* Mark as "viewable" */
						info |= (CAVE_VIEW);

						/* Torch-lit grids */
						if (p->d < radius)
						{
							/* Mark as "CAVE_SEEN" and torch-lit */
							info |= (CAVE_SEEN | CAVE_PLIT);
						}

						/* Perma-lit or monster-lit grids */
						else if (info & (CAVE_GLOW | CAVE_MLIT))
						{
							/* Mark as "CAVE_SEEN" */
							info |= (CAVE_SEEN);
						}

						/* Save cave info */
						c_ptr->info = info;

						/* Save in array */
						view_y[fast_view_n] = y;
						view_x[fast_view_n++] = x;
					}
				}
			}
		}
	}


	/*** Step 3 -- Complete the algorithm ***/

	/* Handle blindness */
	if (p_ptr->blind)
	{
		/* Process "new" grids */
		for (i = 0; i < fast_view_n; i++)
		{
			/* Location */
			y = view_y[i];
			x = view_x[i];

			/* Grid cannot be "CAVE_SEEN" */
			cave[y][x].info &= ~(CAVE_SEEN);
		}
	}

	/* Process "new" grids */
	for (i = 0; i < fast_view_n; i++)
	{
		/* Location */
		y = view_y[i];
		x = view_x[i];

		/* Get grid info */
		info = cave[y][x].info;

		/* Was not "CAVE_SEEN", is now "CAVE_SEEN" */
		if ((info & (CAVE_SEEN)) && !(info & (CAVE_TEMP)))
		{
			/* Note */
			note_spot(y, x);

			/* Redraw */
			lite_spot(y, x);
		}
	}

	/* Process "old" grids */
	for (i = 0; i < fast_temp_n; i++)
	{
		/* Location */
		y = temp_y[i];
		x = temp_x[i];

		/* Grid */
		c_ptr = &cave[y][x];

		/* Get grid info */
		info = c_ptr->info;

		/* Clear "CAVE_TEMP" flag */
		info &= ~(CAVE_TEMP);

		/* Save cave info */
		c_ptr->info = info;

		/* Was "CAVE_SEEN", is now not "CAVE_SEEN" */
		if (!(info & (CAVE_SEEN)))
		{
			/* Redraw */
			lite_spot(y, x);
		}
	}


	/* Save 'view_n' */
	view_n = fast_view_n;
}


/*
 * Clear monster light 
 */
void forget_mon_lite(void)
{
	int i, y, x;

	/* Process all the monster-lit grids */
	for (i = 0; i < lite_n; i++)
	{
		/* Access location */
		y = lite_y[i];
		x = lite_x[i];

		/* Clear monster light flag */
		cave[y][x].info &= ~(CAVE_MLIT);
	}

	/* Forget light array */
	lite_n = 0;
}


/*
 * Update squares illuminated by monsters
 *
 * Code taken from Steven Fuerst's work for ZAngband, without support
 * for multiple lite radii, and with necessary modifications for different
 * internal representation of dungeon/wilderness. Other minor changes
 * are mine...
 *
 * I'm not sure if I can handle wide radius well. Consider the following
 * example, with p carrying a radius 3 light source:
 *
 *     ##%#
 *     .x..
 *     p##@
 *
 * % should be illuminated, although the beam path is entirely out of
 * player's los (because of grid-based nature of cave representation)...
 * And I'm extremely reluctant to introduce symmetrical los. The current
 * asymmetrical system has its own merit, and all the rules of games are
 * asymmetrical, in some way or another...
 *
 * The code below exploits special characteristics of radius one light
 * where one can fairly safely use light source's visibility (in terms of los)
 * to determine if we can illuminate walls XXX
 *
 * This function works within the current player's field of view
 * calculated by update_view(), so it should normally be called
 * whenever FoV is updated (== PU_VIEW | PU_MON_LITE). The other
 * case is when RF9_HAS_LITE monsters have moved or dead. Monster
 * creation occurs out of LoS, so I chose not to take this into
 * consideration.
 *
 * The CAVE_TEMP flag is used by the function to remember "old" monster-lit
 * grids so that it can only redraw squares whose visibility has changed.
 *
 * Doing this in the update_view() order (update "new" grids, then "old")
 * would result in bizarre lighting effects XXX XXX
 *
 * It has been made possible again to draw torch/monster-lit grids in
 * different colours, even when they are in permanently lit locations
 * by using (CAVE_PLIT|CAVE_MLIT) as if it were old CAVE_LITE, but I don't
 * think it's appropriate for torch lights to be visible under the Sun :)
 * or brighter light, and it doesn't work well with PernAngband's already
 * colourful terrain features in aesthetically pleasing ways... -- pelpel
 */
void update_mon_lite(void)
{
	int i, y, x, d;
	int fy, fx;

	cave_type *c_ptr;
	u16b info;

	bool invis;

	s16b fast_lite_n = lite_n;
	s16b fast_temp_n;


	/* Mega-Hack -- It's unnecessary there */
	if (p_ptr->wild_mode) return;

	/* Handle special case -- Blindness */
	if (p_ptr->blind)
	{
		for (i = 0; i < fast_lite_n; i++)
		{
			/* Light location */
			y = lite_y[i];
			x = lite_x[i];

			/* Forget monster light and view */
			cave[y][x].info &= ~(CAVE_MLIT | CAVE_SEEN);

			/* Redraw spot */
			/* lite_spot(y, x); */
		}

		/* Clear the light list */
		lite_n = 0;

		/* Done */
		return;
	}


	/* Remember and clear all monster-lit grids */
	for (i = 0; i < fast_lite_n; i++)
	{
		/* Lit location */
		y = lite_y[i];
		x = lite_x[i];

		/* Access grid */
		c_ptr = &cave[y][x];

		/* Access cave info of the grid */
		info = c_ptr->info;

		/* Remember it, by setting the CAVE_TEMP flag */
		info |= (CAVE_TEMP);

		/* Forget monster light */
		info &= ~(CAVE_MLIT);

		/* Unseen unless it's glowing or illuminated by player light source */
		if (!(info & (CAVE_GLOW | CAVE_PLIT)))
		{
			info &= ~(CAVE_SEEN);
		}

		/* Save cave info flags */
		c_ptr->info = info;
	}


	/* Clear the temp list */
	fast_temp_n = 0;

	/* Loop through monsters, adding newly lit grids to changes list */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr;

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip out-of-sight monsters (MAX_SIGHT + max radius) */
		if (m_ptr->cdis > MAX_SIGHT + 1) continue;

		/* Access monster race info (with possible ego mods) */
		r_ptr = race_info_idx(m_ptr->r_idx, m_ptr->ego);

		/* Skip monsters not carrying light source */
		if (!(r_ptr->flags9 & RF9_HAS_LITE)) continue;

		/* Access the location */
		fy = m_ptr->fy;
		fx = m_ptr->fx;

		/* Extract monster grid visibility */
		invis = !player_has_los_bold(fy, fx);

		/* Nested loops may be a bad idea here XXX */
		for (d = 0; d < 9; d++)
		{
			y = fy + ddy_ddd[d];
			x = fx + ddx_ddd[d];

			/* Paranoia */
			/* if (!in_bounds(y, x)) continue; */

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Access cave info flags */
			info = c_ptr->info;

			/* Don't care grids out of player's los */
			if (!(info & (CAVE_VIEW))) continue;

			/*
			 * Avoid processing already monster-lit grids,
			 * for efficiency and to avoid temp array overflow
			 */
			if (info & (CAVE_MLIT)) continue;

			/*
			 * Hack XXX XXX -- light shouldn't penetrate walls
			 *
			 *     OK          NG
			 *  .#.  p#.  | p.   .p.  p..
			 *  p.@  ..@  | .#   .#.  .#.
			 *            | .@   .@.  ..@
			 *
			 * So if a monster carrying light source is out of player LoS,
			 * walls aren't illuminated.
			 *
			 * CAVEAT: % will be illuminated in cases like this:
			 *
			 *  #%..@
			 *  p....
			 *
			 * We don't have four sides for a wall grid, so...
			 */
			if (invis && (f_info[c_ptr->feat].flags1 & FF1_NO_VISION)) continue;

			/* Give monster light to the location */
			c_ptr->info |= (CAVE_MLIT | CAVE_SEEN);

			/* Save the location */
			temp_y[fast_temp_n] = y;
			temp_x[fast_temp_n] = x;
			fast_temp_n++;
		}
	}

	/* Process old grids */
	for (i = 0; i < fast_lite_n; i++)
	{
		/* Access location */
		y = lite_y[i];
		x = lite_x[i];

		/* Access grid */
		c_ptr = &cave[y][x];

		/* Was lit, is no longer lit */
		if (!(c_ptr->info & (CAVE_MLIT)))
		{
			/* Clear the temp flag */
			c_ptr->info &= ~(CAVE_TEMP);

			/* See if there was a visible monster */
			if (player_has_los_bold(y, x) && c_ptr->m_idx)
			{
				/* Hide the monster */
				update_mon(c_ptr->m_idx, FALSE);
			}
			else
			{
				/* Redraw */
				lite_spot(y, x);
			}
		}
	}

	/* Copy the temp array into the light array */
	for (i = 0; i < fast_temp_n; i++)
	{
		/* Access location */
		y = temp_y[i];
		x = temp_x[i];

		/* Access grid */
		c_ptr = &cave[y][x];


		/* No changes in illumination */
		if (c_ptr->info & (CAVE_TEMP))
		{
			/* Clear the temp flag */
			c_ptr->info &= ~(CAVE_TEMP);
		}

		/* Was not lit, is now lit */
		else
		{
			/* Remember the location, if appropriate */
			note_spot(y, x);

			/* See if there is a monster */
			if (c_ptr->m_idx)
			{
				/* Show it */
				update_mon(c_ptr->m_idx, FALSE);
			}
			else
			{
				/* Redraw */
				lite_spot(y, x);
			}
		}


		/* Save the location */
		lite_y[i] = y;
		lite_x[i] = x;
	}

	/* Save lite_n */
	lite_n = fast_temp_n;

	/* Forget temp */
	temp_n = 0;
}






/*
 * Hack -- provide some "speed" for the "flow" code
 * This entry is the "current index" for the "when" field
 * Note that a "when" value of "zero" means "not used".
 *
 * Note that the "cost" indexes from 1 to 127 are for
 * "old" data, and from 128 to 255 are for "new" data.
 *
 * This means that as long as the player does not "teleport",
 * then any monster up to 128 + MONSTER_FLOW_DEPTH will be
 * able to track down the player, and in general, will be
 * able to track down either the player or a position recently
 * occupied by the player.
 */
static int flow_n = 0;


/*
 * Hack -- forget the "flow" information
 */
void forget_flow(void)
{

#ifdef MONSTER_FLOW

	int x, y;

	/* Nothing to forget */
	if (!flow_n) return;

	/* Check the entire dungeon */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			/* Forget the old data */
			cave[y][x].cost = 0;
			cave[y][x].when = 0;
		}
	}

	/* Start over */
	flow_n = 0;

#endif

}


#ifdef MONSTER_FLOW

/*
 * Hack -- Allow us to treat the "seen" array as a queue
 */
static int flow_head = 0;
static int flow_tail = 0;


/*
 * Take note of a reachable grid.  Assume grid is legal.
 */
static void update_flow_aux(int y, int x, int n)
{
	cave_type *c_ptr;

	int old_head = flow_head;


	/* Get the grid */
	c_ptr = &cave[y][x];

	/* Ignore "pre-stamped" entries */
	if (c_ptr->when == flow_n) return;

	/* Ignore "walls" and "rubble" */
	if (c_ptr->feat >= FEAT_RUBBLE) return;

	/* Save the time-stamp */
	c_ptr->when = flow_n;

	/* Save the flow cost */
	c_ptr->cost = n;

	/* Hack -- limit flow depth */
	if (n == MONSTER_FLOW_DEPTH) return;

	/* Enqueue that entry */
	temp_y[flow_head] = y;
	temp_x[flow_head] = x;

	/* Advance the queue */
	if (++flow_head == TEMP_MAX) flow_head = 0;

	/* Hack -- notice overflow by forgetting new entry */
	if (flow_head == flow_tail) flow_head = old_head;
}

#endif


/*
 * Hack -- fill in the "cost" field of every grid that the player
 * can "reach" with the number of steps needed to reach that grid.
 * This also yields the "distance" of the player from every grid.
 *
 * In addition, mark the "when" of the grids that can reach
 * the player with the incremented value of "flow_n".
 *
 * Hack -- use the "seen" array as a "circular queue".
 *
 * We do not need a priority queue because the cost from grid
 * to grid is always "one" and we process them in order.
 */
void update_flow(void)
{

#ifdef MONSTER_FLOW

	int x, y, d;

	/* Hack -- disabled */
	if (!flow_by_sound) return;

	/* Paranoia -- make sure the array is empty */
	if (temp_n) return;

	/* Cycle the old entries (once per 128 updates) */
	if (flow_n == 255)
	{
		/* Rotate the time-stamps */
		for (y = 0; y < cur_hgt; y++)
		{
			for (x = 0; x < cur_wid; x++)
			{
				int w = cave[y][x].when;
				cave[y][x].when = (w > 128) ? (w - 128) : 0;
			}
		}

		/* Restart */
		flow_n = 127;
	}

	/* Start a new flow (never use "zero") */
	flow_n++;


	/* Reset the "queue" */
	flow_head = flow_tail = 0;

	/* Add the player's grid to the queue */
	update_flow_aux(p_ptr->py, p_ptr->px, 0);

	/* Now process the queue */
	while (flow_head != flow_tail)
	{
		/* Extract the next entry */
		y = temp_y[flow_tail];
		x = temp_x[flow_tail];

		/* Forget that entry */
		if (++flow_tail == TEMP_MAX) flow_tail = 0;

		/* Add the "children" */
		for (d = 0; d < 8; d++)
		{
			/* Add that child if "legal" */
			update_flow_aux(y + ddy_ddd[d], x + ddx_ddd[d], cave[y][x].cost + 1);
		}
	}

	/* Forget the flow info */
	flow_head = flow_tail = 0;

#endif

}







/*
 * Hack -- map the current panel (plus some) ala "magic mapping"
 */
void map_area(void)
{
	int i, x, y, y1, y2, x1, x2;

	cave_type *c_ptr;


	/* Pick an area to map */
	y1 = panel_row_min - randint(10);
	y2 = panel_row_max + randint(10);
	x1 = panel_col_min - randint(20);
	x2 = panel_col_max + randint(20);

	/* Speed -- shrink to fit legal bounds */
	if (y1 < 1) y1 = 1;
	if (y2 > cur_hgt - 2) y2 = cur_hgt - 2;
	if (x1 < 1) x1 = 1;
	if (x2 > cur_wid - 2) x2 = cur_wid - 2;

	/* Scan that area */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			c_ptr = &cave[y][x];

			/* All non-walls are "checked" */
			if (!is_wall(c_ptr))
			{
				/* Memorize normal features */
				if (!cave_plain_floor_grid(c_ptr))
				{
					/* Memorize the object */
					c_ptr->info |= (CAVE_MARK);
				}

				/* Memorize known walls */
				for (i = 0; i < 8; i++)
				{
					c_ptr = &cave[y + ddy_ddd[i]][x + ddx_ddd[i]];

					/* Memorize walls (etc) */
					if (is_wall(c_ptr))
					{
						/* Memorize the walls */
						c_ptr->info |= (CAVE_MARK);
					}
				}
			}
		}
	}

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
 * Light up the dungeon using "clairvoyance"
 *
 * This function "illuminates" every grid in the dungeon, memorizes all
 * "objects", memorizes all grids as with magic mapping, and, under the
 * standard option settings (view_perma_grids but not view_torch_grids)
 * memorizes all floor grids too.
 *
 * Note that if "view_perma_grids" is not set, we do not memorize floor
 * grids, since this would defeat the purpose of "view_perma_grids", not
 * that anyone seems to play without this option.
 *
 * Note that if "view_torch_grids" is set, we do not memorize floor grids,
 * since this would prevent the use of "view_torch_grids" as a method to
 * keep track of what grids have been observed directly.
 */
void wiz_lite(void)
{
	int i, y, x;


	/* Memorize objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Memorize */
		o_ptr->marked = TRUE;
	}

	/* Scan all normal grids */
	for (y = 1; y < cur_hgt - 1; y++)
	{
		/* Scan all normal grids */
		for (x = 1; x < cur_wid - 1; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			if (c_ptr->m_idx)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				monster_race *r_ptr = race_inf(m_ptr);

				if (r_ptr->flags9 & RF9_MIMIC)
				{
					object_type *o_ptr = &o_list[m_ptr->hold_o_idx];

					o_ptr->marked = TRUE;
				}
			}

			/* Process all non-walls */
			/* if (c_ptr->feat < FEAT_SECRET) */
			{
				/* Scan all neighbors */
				for (i = 0; i < 9; i++)
				{
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Get the grid */
					c_ptr = &cave[yy][xx];

					/* Perma-lite the grid */
					c_ptr->info |= (CAVE_GLOW);

					/* Memorize normal features */
					if (!cave_plain_floor_grid(c_ptr))
					{
						/* Memorize the grid */
						c_ptr->info |= (CAVE_MARK);
					}

					/* Normally, memorize floors (see above) */
					if (view_perma_grids && !view_torch_grids)
					{
						/* Memorize the grid */
						c_ptr->info |= (CAVE_MARK);
					}
				}
			}
		}
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}

void wiz_lite_extra(void)
{
	int y, x;
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave[y][x].info |= (CAVE_GLOW | CAVE_MARK);
		}
	}
	wiz_lite();
}

/*
 * Forget the dungeon map (ala "Thinking of Maud...").
 */
void wiz_dark(void)
{
	int i, y, x;


	/* Forget every grid */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			/* Process the grid */
			c_ptr->info &= ~(CAVE_MARK);
		}
	}

	/* Forget all objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_idx) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Forget the object */
		o_ptr->marked = FALSE;
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}





/*
 * Change the "feat" flag for a grid, and notice/redraw the grid
 */
void cave_set_feat(int y, int x, int feat)
{
	cave_type *c_ptr = &cave[y][x];

	/* Change the feature */
	c_ptr->feat = feat;

	/*
	 * Handle "wall/door" grids
	 *
	 * XXX XXX XXX This assumes c_ptr->mimic doesn't mimic terrain
	 * features whose LoS behaviour is different from its own, in
	 * most cases. Level boundaries are the most notable exception,
	 * where "real" terrain is always FEAT_PERM_SOLID, and the fact
	 * is (ab)used to prevent out-of-range access to the cave array.
	 * If we were going to implement an evil dungeon type in which
	 * everything is mimicked, then this function, los(), projectable(),
	 * project_path() and maybe some functions in melee2.c might
	 * better use c_ptr->mimic when it's set -- pelpel
	 */
	if (!cave_sight_grid(c_ptr))
	{
		c_ptr->info |= (CAVE_WALL);
	}

	/* Handle "floor"/etc grids */
	else
	{
		c_ptr->info &= ~(CAVE_WALL);
	}

	/* Notice & Redraw */
	if (character_dungeon)
	{
		/* Notice */
		note_spot(y, x);

		/* Redraw */
		lite_spot(y, x);
	}
}


/*
 * Place floor terrain at (y, x) according to dungeon info
 */
void place_floor(int y, int x)
{
	cave_set_feat(y, x, floor_type[rand_int(100)]);
}

/*
 * Place a cave filler at (y, x)
 */
void place_filler(int y, int x)
{
	cave_set_feat(y, x, fill_type[rand_int(100)]);
}


/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
void mmove2(int *y, int *x, int y1, int x1, int y2, int x2)
{
	int dy, dx, dist, shift;

	/* Extract the distance travelled */
	dy = (*y < y1) ? y1 - *y : *y - y1;
	dx = (*x < x1) ? x1 - *x : *x - x1;

	/* Number of steps */
	dist = (dy > dx) ? dy : dx;

	/* We are calculating the next location */
	dist++;


	/* Calculate the total distance along each axis */
	dy = (y2 < y1) ? (y1 - y2) : (y2 - y1);
	dx = (x2 < x1) ? (x1 - x2) : (x2 - x1);

	/* Paranoia -- Hack -- no motion */
	if (!dy && !dx) return;


	/* Move mostly vertically */
	if (dy > dx)
	{

#if 0

		int k;

		/* Starting shift factor */
		shift = dy >> 1;

		/* Extract a shift factor */
		for (k = 0; k < dist; k++)
		{
			if (shift <= 0) shift += dy;
			shift -= dx;
		}

		/* Sometimes move along minor axis */
		if (shift <= 0) (*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

		/* Always move along major axis */
		(*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

#endif

		/* Extract a shift factor */
		shift = (dist * dx + (dy - 1) / 2) / dy;

		/* Sometimes move along the minor axis */
		(*x) = (x2 < x1) ? (x1 - shift) : (x1 + shift);

		/* Always move along major axis */
		(*y) = (y2 < y1) ? (y1 - dist) : (y1 + dist);
	}

	/* Move mostly horizontally */
	else
	{

#if 0

		int k;

		/* Starting shift factor */
		shift = dx >> 1;

		/* Extract a shift factor */
		for (k = 0; k < dist; k++)
		{
			if (shift <= 0) shift += dx;
			shift -= dy;
		}

		/* Sometimes move along minor axis */
		if (shift <= 0) (*y) = (y2 < y1) ? (*y - 1) : (*y + 1);

		/* Always move along major axis */
		(*x) = (x2 < x1) ? (*x - 1) : (*x + 1);

#endif

		/* Extract a shift factor */
		shift = (dist * dy + (dx - 1) / 2) / dx;

		/* Sometimes move along the minor axis */
		(*y) = (y2 < y1) ? (y1 - shift) : (y1 + shift);

		/* Always move along major axis */
		(*x) = (x2 < x1) ? (x1 - dist) : (x1 + dist);
	}
}



/*
 * Determine if a bolt spell cast from (y1,x1) to (y2,x2) will arrive
 * at the final destination, assuming no monster gets in the way.
 *
 * This is slightly (but significantly) different from "los(y1,x1,y2,x2)".
 */
bool projectable(int y1, int x1, int y2, int x2)
{
	int dist, y, x;

	/* Start at the initial location */
	y = y1, x = x1;

	/* See "project()" */
	for (dist = 0; dist <= MAX_RANGE; dist++)
	{
		/* Check for arrival at "final target" */
		/*
		 * NB: this check was AFTER the 'never pass
		 * thru walls' clause, below. Switching them
		 * lets monsters shoot a the player if s/he is
		 * visible but in a wall
		 */
		if ((x == x2) && (y == y2)) return (TRUE);

		/* Never pass through walls */
		if (dist && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x))) break;

		/* Calculate the new location */
		mmove2(&y, &x, y1, x1, y2, x2);
	}


	/* Assume obstruction */
	return (FALSE);
}




/*
 * Standard "find me a location" function
 *
 * Obtains a legal location within the given distance of the initial
 * location, and with "los()" from the source to destination location.
 *
 * This function is often called from inside a loop which searches for
 * locations while increasing the "d" distance.
 *
 * Currently the "m" parameter is unused.
 */
void scatter(int *yp, int *xp, int y, int x, int d, int m)
{
	int nx, ny;
	int attempts_left = 5000;

	/* Unused */
	m = m;


	/* Pick a location */
	while (--attempts_left)
	{
		/* Pick a new location */
		ny = rand_spread(y, d);
		nx = rand_spread(x, d);

		/* Ignore illegal locations and outer walls */
		if (!in_bounds(ny, nx)) continue;

		/* Ignore "excessively distant" locations */
		if ((d > 1) && (distance(y, x, ny, nx) > d)) continue;

		/* Require "line of sight" */
		if (los(y, x, ny, nx)) break;
	}

	if (attempts_left > 0)
	{
		/* Save the location */
		(*yp) = ny;
		(*xp) = nx;
	}
}




/*
 * Track a new monster
 */
void health_track(int m_idx)
{
	/* Track a new guy */
	health_who = m_idx;

	/* Redraw (later) */
	p_ptr->redraw |= (PR_HEALTH);
}



/*
 * Hack -- track the given monster race
 */
void monster_race_track(int r_idx, int ego)
{
	/* Save this monster ID */
	monster_race_idx = r_idx;
	monster_ego_idx = ego;

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);
}



/*
 * Hack -- track the given object kind
 */
void object_track(object_type *o_ptr)
{
	/* Save this monster ID */
	tracked_object = o_ptr;

	/* Window stuff */
	p_ptr->window |= (PW_OBJECT);
}



/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is currently unused, but could induce output flush.
 *
 * All disturbance cancels repeated commands, resting, and running.
 */
void disturb(int stop_search, int unused_flag)
{
	/* Unused */
	unused_flag = unused_flag;

	/* Cancel auto-commands */
	/* command_new = 0; */

	/* Cancel repeated commands */
	if (command_rep)
	{
		/* Cancel */
		command_rep = 0;

		/* Redraw the state (later) */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Cancel Resting */
	if (resting)
	{
		/* Cancel */
		resting = 0;

		/* Redraw the state (later) */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Cancel running */
	if (running)
	{
		/* Cancel */
		running = 0;

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);
	}

	/* Cancel searching if requested */
	if (stop_search && p_ptr->searching)
	{
		/* Cancel */
		p_ptr->searching = FALSE;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Flush the input if requested */
	if (flush_disturb) flush();
}


/*
 * Hack -- Check if a level is a "quest" level
 */
int is_quest(int level)
{
	int i = random_quest_number();

	/* Check quests */
	if (p_ptr->inside_quest)
		return (p_ptr->inside_quest);

	if (i) return (QUEST_RANDOM);

	/* Nope */
	return (0);
}


/*
 * Return the index of the random quest on this level
 * (or zero)
 */
int random_quest_number()
{
	if ((dun_level >= 1) && (dun_level < MAX_RANDOM_QUEST) &&
	                (dungeon_flags1 & DF1_PRINCIPAL) &&
	                (random_quests[dun_level].type) &&
					(!random_quests[dun_level].done) &&
					(!is_randhero(dun_level)))
	{
		return dun_level;
	}

	/* Nope */
	return 0;
}


/*
 * handle spell effects
 */
int effect_pop()
{
	int i;

	for (i = 1; i < MAX_EFFECTS; i++)
		if (!effects[i].time)
			return i;
	return -1;
}

int new_effect(int type, int dam, int time, int cy, int cx, int rad, s32b flags)
{
	int i;

	if ((i = effect_pop()) == -1) return -1;

	effects[i].type = type;
	effects[i].dam = dam;
	effects[i].time = time;
	effects[i].flags = flags;
	effects[i].cx = cx;
	effects[i].cy = cy;
	effects[i].rad = rad;
	return i;
}
