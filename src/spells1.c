/* File: spells1.c */

/* Purpose: Spell code (part 1) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include "spell_type.h"

/* 1/x chance of reducing stats (for elemental attacks) */
#define HURT_CHANCE 32

/* 1/x chance of hurting even if invulnerable!*/
#define PENETRATE_INVULNERABILITY 13

/* Maximum number of tries for teleporting */
#define MAX_TRIES 100


/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
s16b poly_r_idx(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int i, r;

	/* Hack -- Uniques never polymorph */
	if (r_ptr->flags1 & RF1_UNIQUE)
		return (r_idx);

	/* Pick a (possibly new) non-unique race */
	for (i = 0; i < 1000; i++)
	{
		/* Pick a new race, using a level calculation */
		r = get_mon_num((dun_level + r_ptr->level) / 2 + 5);

		/* Handle failure */
		if (!r) break;

		/* Obtain race */
		r_ptr = &r_info[r];

		/* Ignore unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Use that index */
		r_idx = r;

		/* Done */
		break;
	}

	/* Result */
	return (r_idx);
}

/*
 * Teleport player, using a distance and a direction as a rough guide.
 *
 * This function is not at all obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_directed(int rad, int dir)
{
	int y = p_ptr->py;
	int x = p_ptr->px;
	int yfoo = ddy[dir];
	int xfoo = ddx[dir];
	int min = rad / 4;
	int dis = rad;
	int i, d;
	bool_ look = TRUE;
	bool_ y_major = FALSE;
	bool_ x_major = FALSE;
	int y_neg = 1;
	int x_neg = 1;
	cave_type *c_ptr;

	if (xfoo == 0 && yfoo == 0)
	{
		teleport_player(rad);
		return;
	}

	/* Rooted means no move */
	if (p_ptr->tim_roots) return;

	if (yfoo == 0) x_major = TRUE;
	if (xfoo == 0) y_major = TRUE;
	if (yfoo < 0) y_neg = -1;
	if (xfoo < 0) x_neg = -1;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200)
		{
			teleport_player(rad);
			return;
		}

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				if (y_major)
				{
					y = rand_spread(p_ptr->py + y_neg * dis / 2, dis / 2);
				}
				else
				{
					y = rand_spread(p_ptr->py, dis / 3);
				}

				if (x_major)
				{
					x = rand_spread(p_ptr->px + x_neg * dis / 2, dis / 2);
				}
				else
				{
					x = rand_spread(p_ptr->px, dis / 3);
				}

				d = distance(p_ptr->py, p_ptr->px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Require "naked" floor space */
			if (!cave_empty_bold(y, x)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Move player */
	teleport_player_to(y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();

	c_ptr = &cave[y][x];

	/* Hack -- enter a store if we are on one */
	if (c_ptr->feat == FEAT_SHOP)
	{
		/* Disturb */
		disturb(0, 0);

		/* Hack -- enter store */
		command_new = '_';
	}
}


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(int m_idx, int dis)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;
	int tries = 0;

	bool_ look = TRUE;

	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = race_inf(m_ptr);

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		tries++;

		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(ny, nx)) continue;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave[ny][nx].feat == FEAT_GLYPH) continue;
			if (cave[ny][nx].feat == FEAT_MINOR_GLYPH) continue;

			/* ...nor onto the Pattern */
			if ((cave[ny][nx].feat >= FEAT_PATTERN_START) &&
			                (cave[ny][nx].feat <= FEAT_PATTERN_XTRA2)) continue;

			/* No teleporting into vaults and such */
			if (!(p_ptr->inside_quest || p_ptr->inside_arena))
				if (cave[ny][nx].info & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

		/* Stop after MAX_TRIES tries */
		if (tries > MAX_TRIES) return;
	}

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Update the new location */
	cave[ny][nx].m_idx = m_idx;
	last_teleportation_y = ny;
	last_teleportation_x = nx;

	/* Update the old location */
	cave[oy][ox].m_idx = 0;

	/* Move the monster */
	m_ptr->fy = ny;
	m_ptr->fx = nx;

	/* Update the monster (new location) */
	update_mon(m_idx, TRUE);

	/* Redraw the old grid */
	lite_spot(oy, ox);

	/* Redraw the new grid */
	lite_spot(ny, nx);

	/* Update monster light */
	if (r_ptr->flags9 & RF9_HAS_LITE) p_ptr->update |= (PU_MON_LITE);
}


/*
 * Teleport monster next to the player
 */
void teleport_to_player(int m_idx)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;
	int dis = 2;

	bool_ look = TRUE;

	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = race_inf(m_ptr);

	int attempts = 500;

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* "Skill" test */
	if (randint(100) > m_ptr->level) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look && --attempts)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(p_ptr->py, dis);
				nx = rand_spread(p_ptr->px, dis);
				d = distance(p_ptr->py, p_ptr->px, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(ny, nx)) continue;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave[ny][nx].feat == FEAT_GLYPH) continue;
			if (cave[ny][nx].feat == FEAT_MINOR_GLYPH) continue;

			/* ...nor onto the Pattern */
			if ((cave[ny][nx].feat >= FEAT_PATTERN_START) &&
			                (cave[ny][nx].feat <= FEAT_PATTERN_XTRA2)) continue;

			/* No teleporting into vaults and such */
			/* if (cave[ny][nx].info & (CAVE_ICKY)) continue; */

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	if (attempts < 1) return;

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Update the new location */
	cave[ny][nx].m_idx = m_idx;
	last_teleportation_y = ny;
	last_teleportation_x = nx;

	/* Update the old location */
	cave[oy][ox].m_idx = 0;

	/* Move the monster */
	m_ptr->fy = ny;
	m_ptr->fx = nx;

	/* Update the monster (new location) */
	update_mon(m_idx, TRUE);

	/* Redraw the old grid */
	lite_spot(oy, ox);

	/* Redraw the new grid */
	lite_spot(ny, nx);

	/* Update monster light */
	if (r_ptr->flags9 & RF9_HAS_LITE) p_ptr->update |= (PU_MON_LITE);
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
/* It'd be better if this was made an argument ... */
bool_ teleport_player_bypass = FALSE;

void teleport_player(int dis)
{
	int d, i, min, ox, oy, x = 0, y = 0;
	int tries = 0;

	int xx = -1, yy = -1;

	bool_ look = TRUE;

	if (p_ptr->resist_continuum && (!teleport_player_bypass))
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	if (p_ptr->wild_mode) return;

	/* Rooted means no move */
	if (p_ptr->tim_roots) return;

	if (p_ptr->anti_tele && (!teleport_player_bypass))
	{
		msg_print("A mysterious force prevents you from teleporting!");
		return;
	}

	if ((dungeon_flags2 & DF2_NO_TELEPORT) && (!teleport_player_bypass))
	{
		msg_print("No teleport on special levels...");
		return;
	}


	if (dis > 200) dis = 200;  /* To be on the safe side... */

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		tries++;

		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(p_ptr->py, dis);
				x = rand_spread(p_ptr->px, dis);
				d = distance(p_ptr->py, p_ptr->px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Require "naked" floor space */
			if (!cave_naked_bold(y, x)) continue;

			/* No teleporting into vaults and such */
			if (cave[y][x].info & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

		/* Stop after MAX_TRIES tries */
		if (tries > MAX_TRIES) return;
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Save the old location */
	oy = p_ptr->py;
	ox = p_ptr->px;

	/* Move the player */
	p_ptr->py = y;
	p_ptr->px = x;
	last_teleportation_y = y;
	last_teleportation_x = x;

	/* Redraw the old spot */
	lite_spot(oy, ox);

	while (xx < 2)
	{
		yy = -1;

		while (yy < 2)
		{
			if (xx == 0 && yy == 0)
			{
				/* Do nothing */
			}
			else
			{
				if (cave[oy + yy][ox + xx].m_idx)
				{
					monster_race *r_ptr = race_inf(&m_list[cave[oy + yy][ox + xx].m_idx]);

					if ((r_ptr->flags6
					                & RF6_TPORT) &&
					                !(r_ptr->flags3
					                  & RF3_RES_TELE))
						/*
						 * The latter limitation is to avoid
						 * totally unkillable suckers...
						 */
					{
						if (!(m_list[cave[oy + yy][ox + xx].m_idx].csleep))
							teleport_to_player(cave[oy + yy][ox + xx].m_idx);
					}
				}
			}
			yy++;
		}
		xx++;
	}

	/* Redraw the new spot */
	lite_spot(p_ptr->py, p_ptr->px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Redraw trap detection status */
	p_ptr->redraw |= (PR_DTRAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}


/*
 * get a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 */
void get_pos_player(int dis, int *ny, int *nx)
{
	int d, i, min, x = 0, y = 0;
	int tries = 0;

	bool_ look = TRUE;

	if (dis > 200) dis = 200;  /* To be on the safe side... */

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		tries++;

		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(p_ptr->py, dis);
				x = rand_spread(p_ptr->px, dis);
				d = distance(p_ptr->py, p_ptr->px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Require "naked" floor space */
			if (!cave_naked_bold(y, x)) continue;

			/* No teleporting into vaults and such */
			if (cave[y][x].info & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;

		/* Stop after MAX_TRIES tries */
		if (tries > MAX_TRIES) return;
	}

	*ny = y;
	*nx = x;
}

/*
 * Teleport a monster to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 */
void teleport_monster_to(int m_idx, int ny, int nx)
{
	int y, x, oy, ox, dis = 0, ctr = 0;
	monster_type *m_ptr = &m_list[m_idx];

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	if (p_ptr->anti_tele)
	{
		msg_print("A mysterious force prevents you from teleporting!");
		return;
	}

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds(y, x)) break;
		}

		/* Not on the player's grid */
		/* Accept "naked" floor grids */
		if (cave_naked_bold(y, x) && (y != p_ptr->py) && (x != p_ptr->px)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Save the old position */
	oy = m_ptr->fy;
	ox = m_ptr->fx;
	cave[oy][ox].m_idx = 0;

	/* Move the monster */
	m_ptr->fy = y;
	m_ptr->fx = x;
	cave[y][x].m_idx = m_idx;
	last_teleportation_y = y;
	last_teleportation_x = x;

	/* Update the monster (new location) */
	update_mon(m_idx, TRUE);

	/* Redraw the old spot */
	lite_spot(oy, ox);

	/* Redraw the new spot */
	lite_spot(m_ptr->fy, m_ptr->fx);
}


/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int y, x, oy, ox, dis = 0, ctr = 0;

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	if (p_ptr->anti_tele)
	{
		msg_print("A mysterious force prevents you from teleporting!");
		return;
	}

	if (dungeon_flags2 & DF2_NO_TELEPORT)
	{
		msg_print("No teleport on special levels...");
		return;
	}

	/* Rooted means no move */
	if (p_ptr->tim_roots) return;

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds(y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (cave_naked_bold2(y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Save the old location */
	oy = p_ptr->py;
	ox = p_ptr->px;

	/* Move the player */
	p_ptr->py = y;
	p_ptr->px = x;
	last_teleportation_y = y;
	last_teleportation_x = x;

	/* Redraw the old spot */
	lite_spot(oy, ox);

	/* Redraw the new spot */
	lite_spot(p_ptr->py, p_ptr->px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Redraw trap detection status */
	p_ptr->redraw |= (PR_DTRAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{
	/* No effect in arena or quest */
	if (p_ptr->inside_arena || p_ptr->inside_quest)
	{
		msg_print("There is no effect.");
		return;
	}
	if (dungeon_flags2 & DF2_NO_TELEPORT)
	{
		msg_print("No teleport on special levels...");
		return;
	}
	if (dungeon_flags2 & DF2_NO_EASY_MOVE)
	{
		msg_print("Some powerfull force prevents your from teleporting.");
		return;
	}

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	/* Hack -- when you are fated to die, you cant cheat :) */
	if (dungeon_type == DUNGEON_DEATH)
	{
		msg_print("A mysterious force prevents you from teleporting!");
		return;
	}

	if (p_ptr->anti_tele)
	{
		msg_print("A mysterious force prevents you from teleporting!");
		return;
	}

	/* Rooted means no move */
	if (p_ptr->tim_roots) return;

	if (!dun_level)
	{
		msg_print("You sink through the floor.");

		autosave_checkpoint();

		dun_level++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH - 1))
	{
		msg_print("You rise up through the ceiling.");

		autosave_checkpoint();

		dun_level--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else if (rand_int(100) < 50)
	{
		msg_print("You rise up through the ceiling.");

		autosave_checkpoint();

		dun_level--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
	else
	{
		msg_print("You sink through the floor.");

		autosave_checkpoint();

		dun_level++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	/* Sound */
	sound(SOUND_TPLEVEL);
}



/*
 * Recall the player to town or dungeon
 */
void recall_player(int d, int f)
{
	/* Rooted means no move */
	if (p_ptr->tim_roots)
	{
		msg_print("Your roots prevent the recall.");
		return;
	}


	if (dun_level && (max_dlv[dungeon_type] > dun_level) &&
	                !p_ptr->inside_quest)
	{
		if (get_check("Reset recall depth? "))
			max_dlv[dungeon_type] = dun_level;

	}
	if (!p_ptr->word_recall)
	{
		p_ptr->word_recall = rand_int(d) + f;
		msg_print("The air about you becomes charged...");
	}
	else
	{
		p_ptr->word_recall = 0;
		msg_print("A tension leaves the air around you...");
	}
	p_ptr->redraw |= (PR_DEPTH);
}


/*
 * Check the gods
 */
static void project_check_gods(int typ)
{
	if (p_ptr->pgod == GOD_VARDA)
	{
		if ((typ == GF_LITE) || (typ == GF_LITE_WEAK))
		{
			/* Raise piety for using lite */
			set_grace(p_ptr->grace + 1);
		}
	}

	if (p_ptr->pgod == GOD_ULMO)
	{
		if ((typ == GF_FIRE) ||
		    (typ == GF_HELL_FIRE) ||
		    (typ == GF_HOLY_FIRE) ||
		    (typ == GF_LAVA_FLOW) ||
		    (typ == GF_METEOR) ||
		    (typ == GF_NUKE) ||
		    (typ == GF_PLASMA))
		{
			/* Reduce piety for using any kind of fire magic */
			set_grace(p_ptr->grace - 5);
		}
	}
}


/*
 * Get a legal "multi-hued" color for drawing "spells"
 */
static byte mh_attr(int max)
{
	switch (randint(max))
	{
	case 1:
		return (TERM_RED);
	case 2:
		return (TERM_GREEN);
	case 3:
		return (TERM_BLUE);
	case 4:
		return (TERM_YELLOW);
	case 5:
		return (TERM_ORANGE);
	case 6:
		return (TERM_VIOLET);
	case 7:
		return (TERM_L_RED);
	case 8:
		return (TERM_L_GREEN);
	case 9:
		return (TERM_L_BLUE);
	case 10:
		return (TERM_UMBER);
	case 11:
		return (TERM_L_UMBER);
	case 12:
		return (TERM_SLATE);
	case 13:
		return (TERM_WHITE);
	case 14:
		return (TERM_L_WHITE);
	case 15:
		return (TERM_L_DARK);
	}

	return (TERM_WHITE);
}


/*
 * Return a color to use for the bolt/ball spells
 */
byte spell_color(int type)
{
	/* Check if A.B.'s new graphics should be used (rr9) */
	if (streq(ANGBAND_GRAF, "new"))
	{
		/* Analyze */
		switch (type)
		{
		case GF_MISSILE:
			return (0x0F);
		case GF_ACID:
			return (0x04);
		case GF_ELEC:
			return (0x02);
		case GF_FIRE:
			return (0x00);
		case GF_COLD:
			return (0x01);
		case GF_POIS:
			return (0x03);
		case GF_UNBREATH:
			return (0x03);
		case GF_HOLY_FIRE:
			return (0x00);
		case GF_HELL_FIRE:
			return (0x00);
		case GF_MANA:
			return (0x0E);
		case GF_ARROW:
			return (0x0F);
		case GF_WATER:
			return (0x04);
		case GF_WAVE:
			return (0x04);
		case GF_NETHER:
			return (0x07);
		case GF_CHAOS:
			return (mh_attr(15));
		case GF_DISENCHANT:
			return (0x05);
		case GF_NEXUS:
			return (0x0C);
		case GF_CONFUSION:
			return (mh_attr(4));
		case GF_SOUND:
			return (0x09);
		case GF_SHARDS:
			return (0x08);
		case GF_FORCE:
			return (0x09);
		case GF_INERTIA:
			return (0x09);
		case GF_GRAVITY:
			return (0x09);
		case GF_TIME:
			return (0x09);
		case GF_LITE_WEAK:
			return (0x06);
		case GF_LITE:
			return (0x06);
		case GF_DARK_WEAK:
			return (0x07);
		case GF_DARK:
			return (0x07);
		case GF_PLASMA:
			return (0x0B);
		case GF_METEOR:
			return (0x00);
		case GF_ICE:
			return (0x01);
		case GF_ROCKET:
			return (0x0F);
		case GF_DEATH_RAY:
			return (0x07);
		case GF_NUKE:
			return (mh_attr(2));
		case GF_DISINTEGRATE:
			return (0x05);
		case GF_PSI:
		case GF_PSI_DRAIN:
		case GF_TELEKINESIS:
		case GF_DOMINATION:
			return (0x09);
		case GF_INSTA_DEATH:
			return 0;
		case GF_ELEMENTAL_WALL:
		case GF_ELEMENTAL_GROWTH:
			return 0;
		}

	}

	/* Normal tiles or ASCII */
	else
	{
		/* Analyze */
		switch (type)
		{
		case GF_MISSILE:
			return (TERM_SLATE);
		case GF_ACID:
			return (randint(5) < 3 ? TERM_YELLOW : TERM_L_GREEN);
		case GF_ELEC:
			return (randint(7) < 6 ? TERM_WHITE : (randint(4) == 1 ? TERM_BLUE : TERM_L_BLUE));
		case GF_FIRE:
			return (randint(6) < 4 ? TERM_YELLOW : (randint(4) == 1 ? TERM_RED : TERM_L_RED));
		case GF_COLD:
			return (randint(6) < 4 ? TERM_WHITE : TERM_L_WHITE);
		case GF_POIS:
			return (randint(5) < 3 ? TERM_L_GREEN : TERM_GREEN);
		case GF_UNBREATH:
			return (randint(7) < 3 ? TERM_L_GREEN : TERM_GREEN);
		case GF_HOLY_FIRE:
			return (randint(5) == 1 ? TERM_ORANGE : TERM_WHITE);
		case GF_HELL_FIRE:
			return (randint(6) == 1 ? TERM_RED : TERM_L_DARK);
		case GF_MANA:
			return (randint(5) != 1 ? TERM_VIOLET : TERM_L_BLUE);
		case GF_ARROW:
			return (TERM_L_UMBER);
		case GF_WATER:
			return (randint(4) == 1 ? TERM_L_BLUE : TERM_BLUE);
		case GF_WAVE:
			return (randint(4) == 1 ? TERM_L_BLUE : TERM_BLUE);
		case GF_NETHER:
			return (randint(4) == 1 ? TERM_SLATE : TERM_L_DARK);
		case GF_CHAOS:
			return (mh_attr(15));
		case GF_DISENCHANT:
			return (randint(5) != 1 ? TERM_L_BLUE : TERM_VIOLET);
		case GF_NEXUS:
			return (randint(5) < 3 ? TERM_L_RED : TERM_VIOLET);
		case GF_CONFUSION:
			return (mh_attr(4));
		case GF_SOUND:
			return (randint(4) == 1 ? TERM_VIOLET : TERM_WHITE);
		case GF_SHARDS:
			return (randint(5) < 3 ? TERM_UMBER : TERM_SLATE);
		case GF_FORCE:
			return (randint(5) < 3 ? TERM_L_WHITE : TERM_ORANGE);
		case GF_INERTIA:
			return (randint(5) < 3 ? TERM_SLATE : TERM_L_WHITE);
		case GF_GRAVITY:
			return (randint(3) == 1 ? TERM_L_UMBER : TERM_UMBER);
		case GF_TIME:
			return (randint(2) == 1 ? TERM_WHITE : TERM_L_DARK);
		case GF_LITE_WEAK:
			return (randint(3) == 1 ? TERM_ORANGE : TERM_YELLOW);
		case GF_LITE:
			return (randint(4) == 1 ? TERM_ORANGE : TERM_YELLOW);
		case GF_DARK_WEAK:
			return (randint(3) == 1 ? TERM_DARK : TERM_L_DARK);
		case GF_DARK:
			return (randint(4) == 1 ? TERM_DARK : TERM_L_DARK);
		case GF_PLASMA:
			return (randint(5) == 1 ? TERM_RED : TERM_L_RED);
		case GF_METEOR:
			return (randint(3) == 1 ? TERM_RED : TERM_UMBER);
		case GF_ICE:
			return (randint(4) == 1 ? TERM_L_BLUE : TERM_WHITE);
		case GF_ROCKET:
			return (randint(6) < 4 ? TERM_L_RED : (randint(4) == 1 ? TERM_RED : TERM_L_UMBER));
		case GF_DEATH:
		case GF_DEATH_RAY:
			return (TERM_L_DARK);
		case GF_NUKE:
			return (mh_attr(2));
		case GF_DISINTEGRATE:
			return (randint(3) != 1 ? TERM_L_DARK : (randint(2) == 1 ? TERM_ORANGE : TERM_L_UMBER));
		case GF_PSI:
		case GF_PSI_DRAIN:
		case GF_TELEKINESIS:
		case GF_DOMINATION:
			return (randint(3) != 1 ? TERM_L_BLUE : TERM_WHITE);
		case GF_INSTA_DEATH:
			return TERM_DARK;
		case GF_ELEMENTAL_WALL:
		case GF_ELEMENTAL_GROWTH:
			return TERM_GREEN;
		}
	}

	/* Standard "color" */
	return (TERM_WHITE);
}


/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
static u16b bolt_pict(int y, int x, int ny, int nx, int typ)
{
	int base;

	byte k;

	byte a;
	char c;

	/* No motion (*) */
	if ((ny == y) && (nx == x)) base = 0x30;

	/* Vertical (|) */
	else if (nx == x) base = 0x40;

	/* Horizontal (-) */
	else if (ny == y) base = 0x50;

	/* Diagonal (/) */
	else if ((ny - y) == (x - nx)) base = 0x60;

	/* Diagonal (\) */
	else if ((ny - y) == (nx - x)) base = 0x70;

	/* Weird (*) */
	else base = 0x30;

	/* Basic spell color */
	k = spell_color(typ);

	/* Obtain attr/char */
	a = misc_to_attr[base + k];
	c = misc_to_char[base + k];

	/* Create pict */
	return (PICT(a, c));
}

/*
 * Cast the spelbound spells
 */
void spellbinder_trigger()
{
	int i;

	cmsg_print(TERM_L_GREEN, "The spellbinder is triggered!");
	for (i = 0; i < p_ptr->spellbinder_num; i++)
	{
		msg_format("Triggering spell %s.", spell_type_name(spell_at(p_ptr->spellbinder[i])));
		lua_cast_school_spell(p_ptr->spellbinder[i], TRUE);
	}
	p_ptr->spellbinder_num = 0;
	p_ptr->spellbinder_trigger = 0;
}


/*
 * Decreases players hit points and sets death flag if necessary
 *
 * XXX XXX XXX Invulnerability needs to be changed into a "shield"
 *
 * XXX XXX XXX Hack -- this function allows the user to save (or quit)
 * the game when he dies, since the "You die." message is shown before
 * setting the player to "dead".
 */
void take_hit(int damage, cptr hit_from)
{
	object_type *o_ptr = &p_ptr->inventory[INVEN_CARRY];
	int old_chp = p_ptr->chp;

	bool_ pen_invuln = FALSE;
	bool_ monster_take = FALSE;

	char death_message[80];

	int warning = (p_ptr->mhp * hitpoint_warn / 10);
	int percent;

	/* Paranoia */
	if (death) return;

	/* Disturb */
	disturb(1, 0);

	/* Apply "invulnerability" */
	if (p_ptr->invuln && (damage < 9000))
	{
		if (randint(PENETRATE_INVULNERABILITY) == 1)
		{
			pen_invuln = TRUE;
		}
		else
		{
			return;
		}
	}

	/* Apply disruption shield */
	if (p_ptr->disrupt_shield)
	{
		if (p_ptr->csp > (damage * 2))
		{
			p_ptr->csp -= (damage * 2);
			damage = 0;
		}
		else
		{
			damage -= p_ptr->csp / 2;
			p_ptr->csp = 0;
			p_ptr->csp_frac = 0;
		}

		/* Display the mana */
		p_ptr->redraw |= (PR_MANA);
	}

	/* Hurt the wielded monster if any */
	if ((o_ptr->k_idx) && (magik(5 + get_skill(SKILL_SYMBIOTIC))) && (!carried_monster_hit))
	{
		cptr sym_name = symbiote_name(TRUE);

		if (o_ptr->pval2 - damage <= 0)
		{
			cmsg_format(TERM_L_RED,
			            "%s dies from protecting you, you feel very sad...",
			            sym_name);
			inc_stack_size_ex(INVEN_CARRY, -1, OPTIMIZE, NO_DESCRIBE);
			damage -= o_ptr->pval2;
			o_ptr->pval2 = 0;
			p_ptr->redraw |= PR_MH;
		}
		else
		{
			msg_format("%s takes the damage instead of you.", sym_name);
			o_ptr->pval2 -= damage;
			monster_take = TRUE;
		}

		carried_monster_hit = FALSE;

		/* Display the monster hitpoints */
		p_ptr->redraw |= (PR_MH);
	}

	/* Hurt the player */
	if (!monster_take) p_ptr->chp -= damage;

	/* Display the hitpoints */
	p_ptr->redraw |= (PR_HP);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	if (pen_invuln)
		cmsg_print(TERM_YELLOW, "The attack penetrates your shield of invulnerability!");

	/* Dead player */
	if (p_ptr->chp < 0)
	{
		/* Necromancers get a special treatment */
		if (((!has_ability(AB_UNDEAD_FORM)) || ((p_ptr->necro_extra & CLASS_UNDEAD))))
		{
			/* Sound */
			sound(SOUND_DEATH);

			/* Hack -- Note death */
			if (!last_words)
			{
				cmsg_print(TERM_RED, "You die.");
				msg_print(NULL);
			}
			else
			{
				(void)get_rnd_line("death.txt", death_message);
				cmsg_print(TERM_RED, death_message);
			}

			/* Note cause of death */
			(void)strcpy(died_from, hit_from);

			if (p_ptr->image) strcat(died_from, "(?)");

			/* Leaving */
			p_ptr->leaving = TRUE;

			/* No longer a winner */
			total_winner = FALSE;


			/* Note death */
			death = TRUE;

			if (get_check("Dump the screen? "))
			{
				do_cmd_html_dump();
			}

			/* Dead */
			return;
		}
		/* Just turn the necromancer into an undead */
		else
		{
			p_ptr->necro_extra |= CLASS_UNDEAD;
			p_ptr->necro_extra2 = p_ptr->lev + (rand_int(p_ptr->lev / 2) - (p_ptr->lev / 4));
			if (p_ptr->necro_extra2 < 1) p_ptr->necro_extra2 = 1;
			cmsg_format(TERM_L_DARK, "You have to kill %d monster%s to be brought back to life.", p_ptr->necro_extra2, (p_ptr->necro_extra2 == 1) ? "" : "s");

			/* MEGA-HACK !!! */
			calc_hitpoints();

			/* Enforce maximum */
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;

			do_cmd_wiz_cure_all();

			/* Display the hitpoints */
			p_ptr->redraw |= (PR_HP);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
	}

	/* Hitpoint warning */
	if (p_ptr->chp < warning)
	{
		/* Hack -- bell on first notice */
		if (alert_hitpoint && (old_chp > warning)) bell();

		sound(SOUND_WARN);

		/* Message */
		if (p_ptr->necro_extra & CLASS_UNDEAD)
			cmsg_print(TERM_RED, "*** LOW DEATHPOINT WARNING! ***");
		else
			cmsg_print(TERM_RED, "*** LOW HITPOINT WARNING! ***");
		msg_print(NULL);
	}

	/* How much life is left ? */
	percent = p_ptr->chp * 100 / p_ptr->mhp;

	/* Check the spellbinder trigger */
	if (p_ptr->spellbinder_trigger == SPELLBINDER_HP75)
	{
		/* Trigger ?! */
		if (percent <= 75)
			spellbinder_trigger();
	}
	else if (p_ptr->spellbinder_trigger == SPELLBINDER_HP50)
	{
		/* Trigger ?! */
		if (percent <= 50)
			spellbinder_trigger();
	}
	else if (p_ptr->spellbinder_trigger == SPELLBINDER_HP25)
	{
		/* Trigger ?! */
		if (percent <= 25)
			spellbinder_trigger();
	}

	/* Melkor acn summon to help you */
	if (percent < 25)
	{
		PRAY_GOD(GOD_MELKOR)
		{
			int chance = p_ptr->grace / 500;  /*  * 100 / 50000; */

			if (magik(chance - 10))
			{
				int i;
				int type = SUMMON_DEMON;

				if (magik(50))
					type = SUMMON_UNDEAD;

				if (p_ptr->grace > 10000)
				{
					if (type == SUMMON_DEMON)
						type = SUMMON_HI_DEMON;
					else
						type = SUMMON_HI_UNDEAD;
				}

				chance /= 10;
				if (chance < 1) chance = 1;
				for (i = 0; i < chance; i++)
					summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level / 2, type, FALSE);
				msg_print("Melkor summons monsters to help you!");
			}
		}
	}

	if (player_char_health)
		lite_spot(p_ptr->py, p_ptr->px);
}


/* Decrease player's sanity. This is a copy of the function above. */
void take_sanity_hit(int damage, cptr hit_from)
{
	int old_csane = p_ptr->csane;

	char death_message[80];

	int warning = (p_ptr->msane * hitpoint_warn / 10);


	/* Paranoia */
	if (death) return;

	/* Disturb */
	disturb(1, 0);


	/* Hurt the player */
	p_ptr->csane -= damage;

	/* Display the hitpoints */
	p_ptr->redraw |= (PR_SANITY);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Dead player */
	if (p_ptr->csane < 0)
	{
		/* Sound */
		sound(SOUND_DEATH);

		/* Hack -- Note death */
		cmsg_print(TERM_VIOLET, "You turn into an unthinking vegetable.");
		if (!last_words)
		{
			cmsg_print(TERM_RED, "You die.");
			msg_print(NULL);
		}
		else
		{
			(void)get_rnd_line("death.txt", death_message);
			cmsg_print(TERM_RED, death_message);
		}

		/* Note cause of death */
		(void)strcpy(died_from, hit_from);

		if (p_ptr->image) strcat(died_from, "(?)");

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Note death */
		death = TRUE;

		if (get_check("Dump the screen? "))
		{
			do_cmd_html_dump();
		}

		/* Dead */
		return;
	}

	/* Hitpoint warning */
	if (p_ptr->csane < warning)
	{
		/* Hack -- bell on first notice */
		if (alert_hitpoint && (old_csane > warning)) bell();

		sound(SOUND_WARN);

		/* Message */
		cmsg_print(TERM_RED, "*** LOW SANITY WARNING! ***");
		msg_print(NULL);
	}
}


/*
 * Note that amulets, rods, and high-level spell books are immune
 * to "inventory damage" of any kind.  Also sling ammo and shovels.
 */


/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
static bool_ hates_acid(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable items */
	case TV_ARROW:
	case TV_BOLT:
	case TV_BOW:
	case TV_SWORD:
	case TV_AXE:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls are wood/paper */
	case TV_STAFF:
	case TV_SCROLL:
		{
			return (TRUE);
		}

		/* Ouch */
	case TV_CHEST:
		{
			return (TRUE);
		}

		/* Junk is useless */
	case TV_SKELETON:
	case TV_BOTTLE:
	case TV_EGG:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
static bool_ hates_elec(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
	case TV_RING:
	case TV_WAND:
	case TV_EGG:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool_ hates_fire(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Special case for archers */
	case TV_ARROW:
		{
			return TRUE;
		};

		/* Wearable */
	case TV_LITE:
	case TV_BOW:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
		{
			return (TRUE);
		}

		/* Books */
	case TV_BOOK:
	case TV_SYMBIOTIC_BOOK:
	case TV_MUSIC_BOOK:
		{
			return (TRUE);
		}

		/* Chests */
	case TV_CHEST:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls burn */
	case TV_STAFF:
	case TV_SCROLL:
	case TV_EGG:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
static bool_ hates_cold(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
	case TV_POTION2:
	case TV_POTION:
	case TV_FLASK:
	case TV_BOTTLE:
	case TV_EGG:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}









/*
 * Melt something
 */
static int set_acid_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	if (!hates_acid(o_ptr)) return (FALSE);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
	if (f3 & (TR3_IGNORE_ACID)) return (FALSE);
	return (TRUE);
}


/*
 * Electrical damage
 */
static int set_elec_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	if (!hates_elec(o_ptr)) return (FALSE);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
	if (f3 & (TR3_IGNORE_ELEC)) return (FALSE);
	return (TRUE);
}


/*
 * Burn something
 */
static int set_fire_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	if (!hates_fire(o_ptr)) return (FALSE);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
	if (f3 & (TR3_IGNORE_FIRE)) return (FALSE);
	return (TRUE);
}


/*
 * Freeze things
 */
static int set_cold_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	if (!hates_cold(o_ptr)) return (FALSE);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
	if (f3 & (TR3_IGNORE_COLD)) return (FALSE);
	return (TRUE);
}




/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);

/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * Returns number of items destroyed.
 */
static int inven_damage(inven_func typ, int perc)
{
	int i, j, k, amt;

	object_type *o_ptr;

	char o_name[80];


	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr) || o_ptr->art_name) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			/* Count the casualties */
			for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (rand_int(100) < perc) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, o_ptr, FALSE, 3);

				/* Message */
				msg_format("%sour %s (%c) %s destroyed!",
				           ((o_ptr->number > 1) ?
				            ((amt == o_ptr->number) ? "All of y" :
				             (amt > 1 ? "Some of y" : "One of y")) : "Y"),
						           o_name, index_to_label(i),
						           ((amt > 1) ? "were" : "was"));

				/* Potions smash open */
				if (k_info[o_ptr->k_idx].tval == TV_POTION)
		{
					(void)potion_smash_effect(0, p_ptr->py, p_ptr->px, o_ptr->sval);
				}

				/*
				 * Hack -- If rods or wand are destroyed, the total maximum 
				 * timeout or charges of the stack needs to be reduced, 
				 * unless all the items are being destroyed. -LM-
				 */
				if ((o_ptr->tval == TV_WAND)
				                && (amt < o_ptr->number))
				{
					o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;
				}

				/* Destroy "amt" items */
				inc_stack_size_ex(i, -amt, OPTIMIZE, NO_DESCRIBE);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}




/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
	object_type *o_ptr = NULL;

	u32b f1, f2, f3, f4, f5, esp;

	char o_name[80];


	/* Pick a (possibly empty) inventory slot */
	switch (randint(6))
	{
	case 1:
		o_ptr = &p_ptr->inventory[INVEN_BODY];
		break;
	case 2:
		o_ptr = &p_ptr->inventory[INVEN_ARM];
		break;
	case 3:
		o_ptr = &p_ptr->inventory[INVEN_OUTER];
		break;
	case 4:
		o_ptr = &p_ptr->inventory[INVEN_HANDS];
		break;
	case 5:
		o_ptr = &p_ptr->inventory[INVEN_HEAD];
		break;
	case 6:
		o_ptr = &p_ptr->inventory[INVEN_FEET];
		break;
	}

	/* Nothing to damage */
	if (!o_ptr->k_idx) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, FALSE, 0);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Object resists */
	if (f3 & (TR3_IGNORE_ACID))
	{
		msg_format("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Message */
	msg_format("Your %s is damaged!", o_name);

	/* Damage the item */
	o_ptr->to_a--;

	/* Calculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);

	/* Item was damaged */
	return (TRUE);
}


/*
 * Hurt the player with Acid
 */
void acid_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total Immunity */
	if (p_ptr->immune_acid || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->resist_acid) dam = (dam + 2) / 3;
	if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_acid || p_ptr->resist_acid)) &&
	                randint(HURT_CHANCE) == 1)
		(void) do_dec_stat(A_CHR, STAT_DEC_NORMAL);

	/* If any armor gets hit, defend the player */
	if (minus_ac()) dam = (dam + 1) / 2;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->oppose_acid && p_ptr->resist_acid))
		inven_damage(set_acid_destroy, inv);
}


/*
 * Hurt the player with electricity
 */
void elec_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_elec || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->oppose_elec) dam = (dam + 2) / 3;
	if (p_ptr->resist_elec) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_elec || p_ptr->resist_elec)) &&
	                randint(HURT_CHANCE) == 1)
		(void) do_dec_stat(A_DEX, STAT_DEC_NORMAL);

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->oppose_elec && p_ptr->resist_elec))
		inven_damage(set_elec_destroy, inv);
}




/*
 * Hurt the player with Fire
 */
void fire_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Totally immune */
	if (p_ptr->immune_fire || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->sensible_fire) dam = (dam + 2) * 2;
	if (p_ptr->resist_fire) dam = (dam + 2) / 3;
	if (p_ptr->oppose_fire) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_fire || p_ptr->resist_fire)) &&
	                randint(HURT_CHANCE) == 1)
		(void) do_dec_stat(A_STR, STAT_DEC_NORMAL);


	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->resist_fire && p_ptr->oppose_fire))
		inven_damage(set_fire_destroy, inv);
}


/*
 * Hurt the player with Cold
 */
void cold_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_cold || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->resist_cold) dam = (dam + 2) / 3;
	if (p_ptr->oppose_cold) dam = (dam + 2) / 3;

	if ((!(p_ptr->oppose_cold || p_ptr->resist_cold)) &&
	                randint(HURT_CHANCE) == 1)
		(void) do_dec_stat(A_STR, STAT_DEC_NORMAL);

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	if (!(p_ptr->resist_cold && p_ptr->oppose_cold))
		inven_damage(set_cold_destroy, inv);
}





/*
 * Increases a stat by one randomized level             -RAK-
 *
 * Note that this function (used by stat potions) now restores
 * the stat BEFORE increasing it.
 */
bool_ inc_stat(int stat)
{
	int value, gain;

	/* Then augment the current/max stat */
	value = p_ptr->stat_cur[stat];

	/* Cannot go above 18/100 */
	if (value < 18 + 100)
	{
		/* Gain one (sometimes two) points */
		if (value < 18)
		{
			gain = ((rand_int(100) < 75) ? 1 : 2);
			value += gain;
		}

		/* Gain 1/6 to 1/3 of distance to 18/100 */
		else if (value < 18 + 98)
		{
			/* Approximate gain value */
			gain = (((18 + 100) - value) / 2 + 3) / 2;

			/* Paranoia */
			if (gain < 1) gain = 1;

			/* Apply the bonus */
			value += randint(gain) + gain / 2;

			/* Maximal value */
			if (value > 18 + 99) value = 18 + 99;
		}

		/* Gain one point at a time */
		else
		{
			value++;
		}

		/* Save the new value */
		p_ptr->stat_cur[stat] = value;

		/* Bring up the maximum too */
		if (value > p_ptr->stat_max[stat])
		{
			p_ptr->stat_max[stat] = value;
		}

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to gain */
	return (FALSE);
}



/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Amount could be a little higher in extreme cases to mangle very high
 * stats from massive assaults.  -CWS
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool_ dec_stat(int stat, int amount, int mode)
{
	int cur, max, loss = 0, same, res = FALSE;


	/* Acquire current value */
	cur = p_ptr->stat_cur[stat];
	max = p_ptr->stat_max[stat];

	/* Note when the values are identical */
	same = (cur == max);

	/* Damage "current" value */
	if (cur > 3)
	{
		/* Handle "low" values */
		if (cur <= 18)
		{
			if (amount > 90) cur--;
			if (amount > 50) cur--;
			if (amount > 20) cur--;
			cur--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((cur - 18) / 2 + 1) / 2 + 1);

			/* Paranoia */
			if (loss < 1) loss = 1;

			/* Randomize the loss */
			loss = ((randint(loss) + loss) * amount) / 100;

			/* Maximal loss */
			if (loss < amount / 2) loss = amount / 2;

			/* Lose some points */
			cur = cur - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (cur < 18) cur = (amount <= 20) ? 18 : 17;
		}

		/* Prevent illegal values */
		if (cur < 3) cur = 3;

		/* Something happened */
		if (cur != p_ptr->stat_cur[stat]) res = TRUE;
	}

	/* Damage "max" value */
	if ((mode == STAT_DEC_PERMANENT) && (max > 3))
	{
		/* Handle "low" values */
		if (max <= 18)
		{
			if (amount > 90) max--;
			if (amount > 50) max--;
			if (amount > 20) max--;
			max--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((max - 18) / 2 + 1) / 2 + 1);
			loss = ((randint(loss) + loss) * amount) / 100;
			if (loss < amount / 2) loss = amount / 2;

			/* Lose some points */
			max = max - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (max < 18) max = (amount <= 20) ? 18 : 17;
		}

		/* Hack -- keep it clean */
		if (same || (max < cur)) max = cur;

		/* Something happened */
		if (max != p_ptr->stat_max[stat]) res = TRUE;
	}

	/* Apply changes */
	if (res)
	{
		if (mode == STAT_DEC_TEMPORARY)
		{
			u16b dectime;

			/* a little crude, perhaps */
			dectime = rand_int(max_dlv[dungeon_type] * 50) + 50;

			/* Calculate loss */
			loss = p_ptr->stat_cur[stat] - cur;

			/* prevent overflow, stat_cnt = u16b */
			/* or add another temporary drain... */
			if ( ((p_ptr->stat_cnt[stat] + dectime) < p_ptr->stat_cnt[stat]) ||
			                (p_ptr->stat_los[stat] > 0) )

			{
				p_ptr->stat_cnt[stat] += dectime;
				p_ptr->stat_los[stat] += loss;
			}
			else
			{
				p_ptr->stat_cnt[stat] = dectime;
				p_ptr->stat_los[stat] = loss;
			}
		}

		/* Actually set the stat to its new value. */
		p_ptr->stat_cur[stat] = cur;
		p_ptr->stat_max[stat] = max;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	}

	/* Done */
	return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool_ res_stat(int stat, bool_ full)
{
	/* Fully restore */
	if (full)
	{
		/* Restore if needed */
		if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
		{
			/* Restore */
			p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

			/* Remove temporary drain */
			p_ptr->stat_cnt[stat] = 0;
			p_ptr->stat_los[stat] = 0;

			/* Recalculate bonuses */
			p_ptr->update |= (PU_BONUS);

			/* Something happened */
			return (TRUE);
		}
	}

	/* Restore temporary drained stat */
	else
	{
		/* Restore if needed */
		if (p_ptr->stat_los[stat])
		{
			/* Restore */
			p_ptr->stat_cur[stat] += p_ptr->stat_los[stat];

			/* Remove temporary drain */
			p_ptr->stat_cnt[stat] = 0;
			p_ptr->stat_los[stat] = 0;

			/* Recalculate bonuses */
			p_ptr->update |= (PU_BONUS);

			/* Something happened */
			return (TRUE);
		}
	}

	/* Nothing to restore */
	return (FALSE);
}




/*
 * Apply disenchantment to the player's stuff
 *
 * XXX XXX XXX This function is also called from the "melee" code
 *
 * If "mode is set to 0 then a random slot will be used, if not the "mode"
 * slot will be used.
 *
 * Return "TRUE" if the player notices anything
 */
bool_ apply_disenchant(int mode)
{
	int t = mode;
	object_type *o_ptr;
	char o_name[80];

	if (!mode)
	{
		/* Pick a random slot */
		switch (randint(8))
		{
		case 1:
			t = INVEN_WIELD;
			break;
		case 2:
			t = INVEN_BOW;
			break;
		case 3:
			t = INVEN_BODY;
			break;
		case 4:
			t = INVEN_OUTER;
			break;
		case 5:
			t = INVEN_ARM;
			break;
		case 6:
			t = INVEN_HEAD;
			break;
		case 7:
			t = INVEN_HANDS;
			break;
		case 8:
			t = INVEN_FEET;
			break;
		}
	}

	/* Get the item */
	o_ptr = &p_ptr->inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->k_idx) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, o_ptr, FALSE, 0);


	/* Artifacts have 71% chance to resist */
	if ((artifact_p(o_ptr) || o_ptr->art_name) && (rand_int(100) < 71))
	{
		/* Message */
		msg_format("Your %s (%c) resist%s disenchantment!",
		           o_name, index_to_label(t),
		           ((o_ptr->number != 1) ? "" : "s"));

		/* Notice */
		return (TRUE);
	}


	/* Disenchant tohit */
	if (o_ptr->to_h > 0) o_ptr->to_h--;
	if ((o_ptr->to_h > 5) && (rand_int(100) < 20)) o_ptr->to_h--;

	/* Disenchant todam */
	if (o_ptr->to_d > 0) o_ptr->to_d--;
	if ((o_ptr->to_d > 5) && (rand_int(100) < 20)) o_ptr->to_d--;

	/* Disenchant toac */
	if (o_ptr->to_a > 0) o_ptr->to_a--;
	if ((o_ptr->to_a > 5) && (rand_int(100) < 20)) o_ptr->to_a--;

	/* Message */
	msg_format("Your %s (%c) %s disenchanted!",
	           o_name, index_to_label(t),
	           ((o_ptr->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER);

	/* Notice */
	return (TRUE);
}


void corrupt_player(void)
{
	int max1, cur1, max2, cur2, ii, jj;

	/* Pick a pair of stats */
	ii = rand_int(6);
	for (jj = ii; jj == ii; jj = rand_int(6)) /* loop */;

	max1 = p_ptr->stat_max[ii];
	cur1 = p_ptr->stat_cur[ii];
	max2 = p_ptr->stat_max[jj];
	cur2 = p_ptr->stat_cur[jj];

	p_ptr->stat_max[ii] = max2;
	p_ptr->stat_cur[ii] = cur2;
	p_ptr->stat_max[jj] = max1;
	p_ptr->stat_cur[jj] = cur1;

	p_ptr->update |= (PU_BONUS);
}


/*
 * Apply Nexus
 */
static void apply_nexus(monster_type *m_ptr)
{
	if (m_ptr == NULL) return;

	if (!(dungeon_flags2 & DF2_NO_TELEPORT))
	{
		switch (randint(7))
		{
		case 1:
		case 2:
		case 3:
			{
				teleport_player(200);
				break;
			}

		case 4:
		case 5:
			{
				teleport_player_to(m_ptr->fy, m_ptr->fx);
				break;
			}

		case 6:
			{
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
					break;
				}

				/* Teleport Level */
				teleport_player_level();
				break;
			}

		case 7:
			{
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You resist the effects!");
					break;
				}

				msg_print("Your body starts to scramble...");
				corrupt_player();
				break;
			}
		}
	}
}

/*
 * Convert 2 couples of coordonates to a direction
 */
int yx_to_dir(int y2, int x2, int y1, int x1)
{
	int y = y2 - y1, x = x2 - x1;

	if ((y == 0) && (x == 1)) return 6;
	if ((y == 0) && (x == -1)) return 4;
	if ((y == -1) && (x == 0)) return 8;
	if ((y == 1) && (x == 0)) return 2;
	if ((y == -1) && (x == -1)) return 7;
	if ((y == -1) && (x == 1)) return 9;
	if ((y == 1) && (x == 1)) return 3;
	if ((y == 1) && (x == -1)) return 1;

	return 5;
}

/*
 * Give the opposate direction of the given one
 */
int invert_dir(int dir)
{
	if (dir == 4) return 6;
	if (dir == 6) return 4;
	if (dir == 8) return 2;
	if (dir == 2) return 8;
	if (dir == 7) return 3;
	if (dir == 9) return 1;
	if (dir == 1) return 9;
	if (dir == 3) return 7;
	return 5;
}


/*
 * Determine which way the mana path follow
 */
int get_mana_path_dir(int y, int x, int oy, int ox, int pdir, int mana)
{
	int dir[8] = {5, 5, 5, 5, 5, 5, 5, 5}, n = 0, i, r = 0;

	/* Check which case are allowed */
	if (cave[y - 1][x].mana == mana) dir[n++] = 8;
	if (cave[y + 1][x].mana == mana) dir[n++] = 2;
	if (cave[y][x - 1].mana == mana) dir[n++] = 4;
	if (cave[y][x + 1].mana == mana) dir[n++] = 6;

	/* If only 2 possibilities select the only good one */
	if (n == 2)
	{
		if (invert_dir(yx_to_dir(y, x, oy, ox)) != dir[0]) return dir[0];
		if (invert_dir(yx_to_dir(y, x, oy, ox)) != dir[1]) return dir[1];

		/* Should never happen */
		return 5;
	}


	/* Check if it's not your last place */
	for (i = 0; i < n; i++)
	{
		if ((oy == y + ddy[dir[i]]) && (ox == x + ddx[dir[i]]))
		{
			if (dir[i] == 8) dir[i] = 2;
			else if (dir[i] == 2) dir[i] = 8;
			else if (dir[i] == 6) dir[i] = 4;
			else if (dir[i] == 4) dir[i] = 6;
		}
	}

	/* Select the desired one if possible */
	for (i = 0; i < n; i++)
	{
		if ((dir[i] == pdir) &&
		                (cave[y + ddy[dir[i]]][x + ddx[dir[i]]].mana == mana))
		{
			return dir[i];
		}
	}

	/* If not select a random one */
	if (n > 2)
	{
		byte nb = 200;

		while (nb)
		{
			nb--;

			r = rand_int(n);
			if ((dir[r] != 5) && (yx_to_dir(y, x, oy, ox) != dir[r])) break;
		}
		return dir[r];
	}
	/* If nothing is found return 5 */
	else return 5;
}


/*
 * Determine the path taken by a projection.
 *
 * The projection will always start from the grid (y1,x1), and will travel
 * towards the grid (y2,x2), touching one grid per unit of distance along
 * the major axis, and stopping when it enters the destination grid or a
 * wall grid, or has travelled the maximum legal distance of "range".
 *
 * Note that "distance" in this function (as in the "update_view()" code)
 * is defined as "MAX(dy,dx) + MIN(dy,dx)/2", which means that the player
 * actually has an "octagon of projection" not a "circle of projection".
 *
 * The path grids are saved into the grid array pointed to by "gp", and
 * there should be room for at least "range" grids in "gp".  Note that
 * due to the way in which distance is calculated, this function normally
 * uses fewer than "range" grids for the projection path, so the result
 * of this function should never be compared directly to "range".  Note
 * that the initial grid (y1,x1) is never saved into the grid array, not
 * even if the initial grid is also the final grid.  XXX XXX XXX
 *
 * The "flg" flags can be used to modify the behavior of this function.
 *
 * In particular, the "PROJECT_STOP" and "PROJECT_THRU" flags have the same
 * semantics as they do for the "project" function, namely, that the path
 * will stop as soon as it hits a monster, or that the path will continue
 * through the destination grid, respectively.
 *
 * The "PROJECT_JUMP" flag, which for the "project()" function means to
 * start at a special grid (which makes no sense in this function), means
 * that the path should be "angled" slightly if needed to avoid any wall
 * grids, allowing the player to "target" any grid which is in "view".
 * This flag is non-trivial and has not yet been implemented, but could
 * perhaps make use of the "vinfo" array (above).  XXX XXX XXX
 *
 * This function returns the number of grids (if any) in the path.  This
 * function will return zero if and only if (y1,x1) and (y2,x2) are equal.
 *
 * This algorithm is similar to, but slightly different from, the one used
 * by "update_view_los()", and very different from the one used by "los()".
 */
sint project_path(u16b *gp, int range, int y1, int x1, int y2, int x2, int flg)
{
	int y, x, mana = 0, dir = 0;

	int n = 0;
	int k = 0;

	/* Absolute */
	int ay, ax;

	/* Offsets */
	int sy, sx;

	/* Fractions */
	int frac;

	/* Scale factors */
	int full, half;

	/* Slope */
	int m;


	/* No path necessary (or allowed) */
	if ((x1 == x2) && (y1 == y2)) return (0);

	/* Hack -- to make a bolt/beam/ball follow a mana path */
	if (flg & PROJECT_MANA_PATH)
	{
		int oy = y1, ox = x1, pdir = yx_to_dir(y2, x2, y1, x1);

		/* Get the mana path level to follow */
		mana = cave[y1][x1].mana;

		/* Start */
		dir = get_mana_path_dir(y1, x1, y1, x1, pdir, mana);
		y = y1 + ddy[dir];
		x = x1 + ddx[dir];

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if (n >= range + 10) return n;

			/* Always stop at non-initial wall grids */
			if ((n > 0) && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x))) return n;

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (cave[y][x].m_idx != 0)) return n;
			}

			/* Get the new direction */
			dir = get_mana_path_dir(y, x, oy, ox, pdir, mana);
			if (dir == 5) return n;
			oy = y;
			ox = x;
			y += ddy[dir];
			x += ddx[dir];
		}
	}

	/* Analyze "dy" */
	if (y2 < y1)
	{
		ay = (y1 - y2);
		sy = -1;
	}
	else
	{
		ay = (y2 - y1);
		sy = 1;
	}

	/* Analyze "dx" */
	if (x2 < x1)
	{
		ax = (x1 - x2);
		sx = -1;
	}
	else
	{
		ax = (x2 - x1);
		sx = 1;
	}


	/* Number of "units" in one "half" grid */
	half = (ay * ax);

	/* Number of "units" in one "full" grid */
	full = half << 1;


	/* Vertical */
	if (ay > ax)
	{
		/* Start at tile edge */
		frac = ax * ax;

		/* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = y1 + sy;
		x = x1;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x)) && !(flg & PROJECT_WALL)) break;

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (cave[y][x].m_idx != 0)) break;
			}

			/* Slant */
			if (m)
			{
				/* Advance (X) part 1 */
				frac += m;

				/* Horizontal change */
				if (frac >= half)
				{
					/* Advance (X) part 2 */
					x += sx;

					/* Advance (X) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (Y) */
			y += sy;
		}
	}

	/* Horizontal */
	else if (ax > ay)
	{
		/* Start at tile edge */
		frac = ay * ay;

		/* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
		m = frac << 1;

		/* Start */
		y = y1;
		x = x1 + sx;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (k >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x)) && !(flg & PROJECT_WALL)) break;

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (cave[y][x].m_idx != 0)) break;
			}

			/* Slant */
			if (m)
			{
				/* Advance (Y) part 1 */
				frac += m;

				/* Vertical change */
				if (frac >= half)
				{
					/* Advance (Y) part 2 */
					y += sy;

					/* Advance (Y) part 3 */
					frac -= full;

					/* Track distance */
					k++;
				}
			}

			/* Advance (X) */
			x += sx;
		}
	}

	/* Diagonal */
	else
	{
		/* Start */
		y = y1 + sy;
		x = x1 + sx;

		/* Create the projection path */
		while (1)
		{
			/* Save grid */
			gp[n++] = GRID(y, x);

			/* Hack -- Check maximum range */
			if ((n + (n >> 1)) >= range) break;

			/* Sometimes stop at destination grid */
			if (!(flg & (PROJECT_THRU)))
			{
				if ((x == x2) && (y == y2)) break;
			}

			/* Always stop at non-initial wall grids */
			if ((n > 0) && (!cave_sight_bold(y, x) || !cave_floor_bold(y, x)) && !(flg & PROJECT_WALL)) break;

			/* Sometimes stop at non-initial monsters/players */
			if (flg & (PROJECT_STOP))
			{
				if ((n > 0) && (cave[y][x].m_idx != 0)) break;
			}

			/* Advance (Y) */
			y += sy;

			/* Advance (X) */
			x += sx;
		}
	}


	/* Length */
	return (n);
}



/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;



/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * XXX XXX XXX Perhaps we should affect doors?
 */
static bool_ project_f(int who, int r, int y, int x, int dam, int typ)
{
	cave_type *c_ptr = &cave[y][x];

	bool_ obvious = FALSE;

	bool_ flag = FALSE;

	bool_ seen;


	/* XXX XXX XXX */
	who = who ? who : 0;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);

	/* Remember if the grid is with the LoS of player */
	seen = player_can_see_bold(y, x);

	/* Check gods */
	project_check_gods(typ);

	/* Analyze the type */
	switch (typ)
	{
		/* Ignore most effects */
	case GF_ELEC:
	case GF_SOUND:
	case GF_MANA:
	case GF_PSI:
	case GF_PSI_DRAIN:
	case GF_TELEKINESIS:
	case GF_DOMINATION:
		{
			break;
		}

	case GF_COLD:
	case GF_ICE:
		{
			int percent = c_ptr->feat == GF_COLD ? 20 : 50;

			/* Only affects "boring" grids */
			if (!cave_plain_floor_bold(y, x)) break;

			if (rand_int(100) < percent)
			{
				cave_set_feat(y, x, FEAT_ICE);

				if (seen) obvious = TRUE;
			}

			break;
		}

	case GF_BETWEEN_GATE:
		{
			int y1 = randint(cur_hgt) - 1;
			int x1 = randint(cur_wid) - 1;
			int y2 = y1;
			int x2 = x1;
			int tries = 1000;

			/*
			 * Avoid "interesting" and/or permanent features
			 *
			 * If we can make sure that all the "permanent" features
			 * have the remember flag set as well, we can simplify
			 * the conditional... -- pelpel
			 */
			if (!cave_plain_floor_bold(y, x) ||
			                (f_info[cave[y][x].feat].flags1 & FF1_PERMANENT)) break;

			/* Destination shouldn't be "interesting" either */
			while (tries &&
			                (!cave_plain_floor_bold(y2, x2) ||
			                 (f_info[cave[y2][x2].feat].flags1 & FF1_PERMANENT)))
			{
				y2 = y1 = randint(cur_hgt) - 1;
				x2 = x1 = randint(cur_wid) - 1;
				scatter(&y2, &x2, y1, x1, 20);
				tries --;
			}

			/* No boarding grids found */
			if (!tries) break;

			/* Place a pair of between gates */
			cave_set_feat(y, x, FEAT_BETWEEN);
			cave[y][x].special = x2 + (y2 << 8);

			cave_set_feat(y2, x2, FEAT_BETWEEN);
			cave[y2][x2].special = x + (y << 8);

			if (seen)
			{
				obvious = TRUE;
				note_spot(y, x);
			}

			if (player_can_see_bold(y2, x2))
			{
				obvious = TRUE;
				note_spot(y2, x2);
			}

			break;
		}

		/* Burn trees & melt ice */
	case GF_FIRE:
	case GF_METEOR:
	case GF_PLASMA:
	case GF_HOLY_FIRE:
	case GF_HELL_FIRE:
		{
			/* "Permanent" features will stay */
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Trees *will* burn */
			if (c_ptr->feat == FEAT_TREES)
			{
				cave_set_feat(y, x, FEAT_DEAD_TREE);

				/* Silly thing to destroy trees when a yavanna worshipper */
				inc_piety(GOD_YAVANNA, -50);

				if (seen) obvious = TRUE;
			}

			/* Trees *will* burn */
			if (c_ptr->feat == FEAT_SMALL_TREES)
			{
				cave_set_feat(y, x, FEAT_DEAD_SMALL_TREE);

				/* Silly thing to destroy trees when a yavanna worshipper */
				inc_piety(GOD_YAVANNA, -60);

				if (seen) obvious = TRUE;
			}

			/* Ice can melt (chance == 30%) */
			else if (c_ptr->feat == FEAT_ICE)
			{
				int k = rand_int(100);

				if (k >= 30) break;

				/* Melt ice */
				if (k < 10) cave_set_feat(y, x, FEAT_DIRT);
				else if (k < 30) cave_set_feat(y, x, FEAT_SHAL_WATER);

				if (seen) obvious = TRUE;
			}

			/* Floors can become ash or lava (chance == 25%) */
			else if (f_info[c_ptr->feat].flags1 & FF1_FLOOR)
			{
				int k = rand_int(100);

				if (k >= 25) break;

				/* Burn floor */
				if (k < 10) cave_set_feat(y, x, FEAT_SHAL_LAVA);
				else if (k < 25) cave_set_feat(y, x, FEAT_ASH);

				if (seen) obvious = TRUE;
			}

			/* Sandwall can be turned into glass (chance == 30%) */
			else if ((c_ptr->feat == FEAT_SANDWALL) ||
			                (c_ptr->feat == FEAT_SANDWALL_H) ||
			                (c_ptr->feat == FEAT_SANDWALL_K))
			{
				int k = rand_int(100);

				/* Glass it */
				if (k < 30)
				{
					cave_set_feat(y, x, FEAT_GLASS_WALL);

					/* Visibility change */
					p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

					if (seen) obvious = TRUE;
				}

			}

			break;
		}

	case GF_WAVE:
	case GF_WATER:
		{
			int p1 = 0;
			int p2 = 0;
			int f1 = 0;
			int f2 = 0;
			int f = 0;
			int k;

			/* "Permanent" features will stay */
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Needs more than 30 damage */
			if (dam < 30) break;

			if ((c_ptr->feat == FEAT_FLOOR) ||
			                (c_ptr->feat == FEAT_DIRT) ||
			                (c_ptr->feat == FEAT_GRASS))
			{
				/* 35% chance to create shallow water */
				p1 = 35;
				f1 = FEAT_SHAL_WATER;

				/* 5% chance to create deep water */
				p2 = 40;
				f2 = FEAT_DEEP_WATER;
			}
			else if ((c_ptr->feat == FEAT_MAGMA) ||
			                (c_ptr->feat == FEAT_MAGMA_H) ||
			                (c_ptr->feat == FEAT_MAGMA_K) ||
			                (c_ptr->feat == FEAT_SHAL_LAVA))
			{
				/* 15% chance to convert it to normal floor */
				p1 = 15;
				f1 = FEAT_FLOOR;
			}
			else if (c_ptr->feat == FEAT_DEEP_LAVA)
			{
				/* 10% chance to convert it to shallow lava */
				p1 = 10;
				f1 = FEAT_SHAL_LAVA;

				/* 5% chance to convert it to normal floor */
				p2 = 15;
				f2 = FEAT_FLOOR;
			}
			else if ((c_ptr->feat == FEAT_SHAL_WATER) ||
			                (c_ptr->feat == FEAT_DARK_PIT))
			{
				/* 10% chance to convert it to deep water */
				p1 = 10;
				f1 = FEAT_DEEP_WATER;
			}

			k = rand_int(100);

			if (k < p1) f = f1;
			else if (k < p2) f = f2;

			if (f)
			{
				if (f == FEAT_FLOOR) place_floor_convert_glass(y, x);
				else cave_set_feat(y, x, f);

				if (seen) obvious = TRUE;
			}

			break;
		}

	case GF_NETHER:
	case GF_NEXUS:
	case GF_ACID:
	case GF_SHARDS:
	case GF_TIME:
	case GF_FORCE:
	case GF_NUKE:
		{
			/* "Permanent" features will stay */
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			if ((c_ptr->feat == FEAT_TREES) ||
			                (c_ptr->feat == FEAT_SMALL_TREES))
			{
				/* Destroy the grid */
				cave_set_feat(y, x, FEAT_DEAD_TREE);

				/* Silly thing to destroy trees when a yavanna worshipper */
				inc_piety(GOD_YAVANNA, -50);

				if (seen) obvious = TRUE;
			}

			break;
		}

	case GF_DISINTEGRATE:
		{
			/* "Permanent" features will stay */
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			if (((c_ptr->feat == FEAT_TREES) ||
			     (c_ptr->feat == FEAT_SMALL_TREES) ||
			     (f_info[c_ptr->feat].flags1 & FF1_FLOOR)) &&
			    (rand_int(100) < 30))
			{
				/* Flow change */
				if (c_ptr->feat == FEAT_TREES) p_ptr->update |= (PU_FLOW);

				cave_set_feat(y, x, FEAT_ASH);

				/* Silly thing to destroy trees when a yavanna worshipper */
				if (c_ptr->feat == FEAT_TREES || c_ptr->feat == FEAT_SMALL_TREES)
					inc_piety(GOD_YAVANNA, -50);

				/* Visibility change */
				p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

				if (seen) obvious = TRUE;
			}

			break;
		}

		/* Destroy Traps (and Locks) */
	case GF_KILL_TRAP:
		{
			/* Destroy normal traps and disarm monster traps */
			if ((c_ptr->t_idx != 0) || (c_ptr->feat == FEAT_MON_TRAP))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					msg_print("There is a bright flash of light!");
					obvious = TRUE;
				}

				/* Forget the trap */
				c_ptr->info &= ~(CAVE_MARK | CAVE_TRDT);

				/* Destroy normal traps */
				c_ptr->t_idx = 0;

				/* Disarm monster traps */
				if (c_ptr->feat == FEAT_MON_TRAP)
				{
					c_ptr->special = c_ptr->special2 = 0;

					/* Remove the feature */
					if (!(f_info[c_ptr->feat].flags1 & FF1_PERMANENT))
						place_floor_convert_glass(y, x);
				}

				/* Hack -- Force redraw */
				note_spot(y, x);
				lite_spot(y, x);
			}

			/* Secret / Locked doors are found and unlocked */
			else if ((c_ptr->feat == FEAT_SECRET) ||
			                ((c_ptr->feat >= FEAT_DOOR_HEAD + 0x01) &&
			                 (c_ptr->feat <= FEAT_DOOR_HEAD + 0x07)))
			{

				/* Check line of sound */
				if (player_has_los_bold(y, x))
				{
					msg_print("Click!");
					obvious = TRUE;
				}

				/* Remove feature mimic */
				cave[y][x].mimic = 0;

				/* Unlock the door */
				cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);
			}

			break;
		}

		/* Destroy Doors (and traps) */
	case GF_KILL_DOOR:
		{
			/* Destroy all doors and traps, and disarm monster traps */
			if ((c_ptr->feat == FEAT_OPEN) ||
			                (c_ptr->feat == FEAT_BROKEN) ||
			                (c_ptr->t_idx != 0) ||
			                (c_ptr->feat == FEAT_MON_TRAP) ||
			                ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			                 (c_ptr->feat <= FEAT_DOOR_TAIL)))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("There is a bright flash of light!");
					obvious = TRUE;

					/* Visibility change */
					if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
					                (c_ptr->feat <= FEAT_DOOR_TAIL))
					{
						/* Update some things */
						p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);
					}
				}

				/* Forget the door */
				c_ptr->info &= ~(CAVE_MARK | CAVE_TRDT);

				/* Remove normal traps */
				c_ptr->t_idx = 0;

				/* Disarm monster traps */
				if (c_ptr->feat == FEAT_MON_TRAP)
					c_ptr->special = c_ptr->special2 = 0;

				/* Remove the feature */
				if (!(f_info[c_ptr->feat].flags1 & FF1_PERMANENT))
					place_floor_convert_glass(y, x);

				/* Hack -- Force redraw */
				note_spot(y, x);
				lite_spot(y, x);
			}

			break;
		}

	case GF_JAM_DOOR:  /* Jams a door (as if with a spike) */
		{
			if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			                (c_ptr->feat <= FEAT_DOOR_TAIL))
			{
				/* Convert "locked" to "stuck" XXX XXX XXX */
				if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08) c_ptr->feat += 0x08;

				/* Add one spike to the door */
				if (c_ptr->feat < FEAT_DOOR_TAIL) c_ptr->feat++;

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The door seems stuck.");
					obvious = TRUE;
				}
			}

			break;
		}

		/* Destroy walls (and doors) */
	case GF_KILL_WALL:
		{
			/* Non-walls (etc) */
			if (cave_floor_bold(y, x)) break;

			/* "Permanent" features will stay */
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Granite -- How about other wall types? */
			if ((c_ptr->feat >= FEAT_WALL_EXTRA) &&
			                (c_ptr->feat <= FEAT_WALL_SOLID))
			{
				/* Message */
				if (c_ptr->info & (CAVE_MARK))
				{
					msg_print("The wall turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				c_ptr->info &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Quartz / Magma / Sand with treasure */
			else if (((c_ptr->feat >= FEAT_MAGMA_H) &&
			                (c_ptr->feat <= FEAT_QUARTZ_K)) ||
			                (c_ptr->feat == FEAT_SANDWALL_K))
			{
				/* Message */
				if (c_ptr->info & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					msg_print("You have found something!");
					obvious = TRUE;
				}

				/* Forget the wall */
				c_ptr->info &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Place some gold */
				place_gold(y, x);
			}

			/* Quartz / Magma / Sand */
			else if ((c_ptr->feat == FEAT_MAGMA) ||
			                (c_ptr->feat == FEAT_QUARTZ) ||
			                (c_ptr->feat == FEAT_SANDWALL) ||
			                (c_ptr->feat == FEAT_SANDWALL_H))
			{
				/* Message */
				if (c_ptr->info & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				c_ptr->info &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Rubble */
			else if (c_ptr->feat == FEAT_RUBBLE)
			{
				/* Message */
				if (c_ptr->info & (CAVE_MARK))
				{
					msg_print("The rubble turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				c_ptr->info &= ~(CAVE_MARK);

				/* Destroy the rubble */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Hack -- place an object */
				if (rand_int(100) < 10)
				{
					/* Found something */
					if (seen)
					{
						msg_print("There was something buried in the rubble!");
						obvious = TRUE;
					}

					/* Place gold */
					place_object(y, x, FALSE, FALSE, OBJ_FOUND_RUBBLE);
				}
			}

			/* Destroy doors (and secret doors) */
			else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			                (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			                (c_ptr->feat == FEAT_SECRET))
			{
				/* Hack -- special message */
				if (c_ptr->info & (CAVE_MARK))
				{
					msg_print("The door turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				c_ptr->info &= ~(CAVE_MARK);

				/* Remove mimic */
				c_ptr->mimic = 0;

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);

			break;
		}

		/* Make doors */
	case GF_MAKE_DOOR:
		{
			/* Require a "naked" floor grid */
			if (!cave_clean_bold(y, x)) break;

			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Create a closed door */
			cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

			/* Observe */
			if (c_ptr->info & (CAVE_MARK)) obvious = TRUE;

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			break;
		}

		/* Make traps */
	case GF_MAKE_TRAP:
		{
			/* Require a "naked" floor grid */
			if (!cave_clean_bold(y, x)) break;

			/* Place a trap */
			place_trap(y, x);

			break;
		}


	case GF_MAKE_GLYPH:
		{
			/* Require a "naked" floor grid */
			if (!cave_clean_bold(y, x)) break;

			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			cave_set_feat(y, x, FEAT_GLYPH);

			if (seen) obvious = TRUE;

			break;
		}



	case GF_STONE_WALL:
		{
			/* Require a "naked" floor grid */
			if (!cave_clean_bold(y, x)) break;

			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;
			if (!(f_info[c_ptr->feat].flags1 & FF1_FLOOR)) break;

			/* Place a wall */
			cave_set_feat(y, x, FEAT_WALL_EXTRA);

			if (seen) obvious = TRUE;

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			break;
		}

	case GF_WINDS_MANA:
		{
			if (dam >= 256)
			{
				/* With erase mana */

				/* Absorb some of the mana of the grid */
				p_ptr->csp += cave[y][x].mana / 80;
				if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;

				/* Set the new amount */
				cave[y][x].mana = dam - 256;
			}
			else
			{
				/* Without erase mana */
				int amt = cave[y][x].mana + dam;

				/* Check if not overflow */
				if (amt > 255) amt = 255;

				/* Set the new amount */
				cave[y][x].mana = amt;
			}

			break;
		}

	case GF_LAVA_FLOW:
		{
			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Shallow Lava */
			if (dam == 1)
			{
				/* Require a "naked" floor grid */
				if (!cave_naked_bold(y, x)) break;

				/* Place a shallow lava */
				cave_set_feat(y, x, FEAT_SHAL_LAVA);

				if (seen) obvious = TRUE;
			}

			/* Deep Lava */
			else
			{
				/* Require a "naked" floor grid */
				if (cave_perma_bold(y, x) || !dam) break;

				/* Place a deep lava */
				cave_set_feat(y, x, FEAT_DEEP_LAVA);

				if (seen) obvious = TRUE;

				/* Dam is used as a counter for the number of grid to convert */
				dam--;
			}

			break;
		}

		/* Lite up the grid */
	case GF_LITE_WEAK:
	case GF_LITE:
		{
			/* Turn on the light */
			c_ptr->info |= (CAVE_GLOW);

			/* Notice */
			note_spot(y, x);

			/* Redraw */
			lite_spot(y, x);

			/* Observe */
			if (seen) obvious = TRUE;

			/*
			 * Mega-Hack -- Update the monster in the affected grid
			 * This allows "spear of light" (etc) to work "correctly"
			 */
			if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

			break;
		}

		/* Darken the grid */
	case GF_DARK_WEAK:
	case GF_DARK:
		{
			/* Notice */
			if (seen) obvious = TRUE;

			/* Turn off the light. */
			c_ptr->info &= ~(CAVE_GLOW);

			/* Hack -- Forget "boring" grids */
			if (cave_plain_floor_grid(c_ptr))
			{
				/* Forget */
				c_ptr->info &= ~(CAVE_MARK);

				/* Notice */
				note_spot(y, x);
			}

			/* Redraw */
			lite_spot(y, x);

			/*
			 * Mega-Hack -- Update the monster in the affected grid
			 * This allows "spear of light" (etc) to work "correctly"
			 */
			if (c_ptr->m_idx) update_mon(c_ptr->m_idx, FALSE);

			/* All done */
			break;
		}

	case GF_DESTRUCTION:
		{
			int t;

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);

			/* Hack -- Notice player affect */
			if ((x == p_ptr->px) && (y == p_ptr->py))
			{
				/* Hurt the player later */
				flag = TRUE;

				/* Do not hurt this grid */
				break;
				;
			}

			/* Delete the monster (if any) */
			delete_monster(y, x);

			if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) break;

			/* Destroy "valid" grids */
			if (cave_valid_bold(y, x))
			{
				/* Delete objects */
				delete_object(y, x);

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
					cave_set_feat(y, x, FEAT_FLOOR);
				}

				/* Visibility and flow changes */
				p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);
			}

			obvious = TRUE;
			break;
		}

	case GF_ELEMENTAL_WALL:
		{
			if ((p_ptr->py != y) || (p_ptr->px != x)) {
				geomancy_random_wall(y, x);
			}
			break;
		}

	case GF_ELEMENTAL_GROWTH:
		{
			geomancy_random_floor(y, x, FALSE);
			break;
		}
	}

	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg_print("There is a searing blast of light!");

		/* Blind the player */
		if (!p_ptr->resist_blind && !p_ptr->resist_lite)
		{
			/* Become blind */
			(void)set_blind(p_ptr->blind + 10 + randint(10));
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}


/* Array of raisable ego monster */
#define MAX_RAISE 10
static int raise_ego[MAX_RAISE] =
{
	1,       /* Skeleton */
	1,       /* Skeleton */
	1,       /* Skeleton */
	1,       /* Skeleton */
	2,       /* Zombie */
	2,       /* Zombie */
	2,       /* Zombie */
	4,       /* Spectre */
	4,       /* Spectre */
	3,       /* Lich */
};


/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * XXX XXX XXX We also "see" grids which are "memorized", probably a hack
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool_ project_o(int who, int r, int y, int x, int dam, int typ)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;

	bool_ obvious = FALSE;

	u32b f1, f2, f3, f4, f5, esp;

	char o_name[80];

	int o_sval = 0;
	bool_ is_potion = FALSE;


	/* XXX XXX XXX */
	who = who ? who : 0;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);

	/* Check new gods. */
	project_check_gods(typ);

	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		bool_ is_art = FALSE;
		bool_ ignore = FALSE;
		bool_ plural = FALSE;
		bool_ do_kill = FALSE;

		cptr note_kill = NULL;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Get the "plural"-ness */
		if (o_ptr->number > 1) plural = TRUE;

		/* Check for artifact */
		if ((artifact_p(o_ptr) || o_ptr->art_name)) is_art = TRUE;

		/* Analyze the type */
		switch (typ)
		{
			/* makes corpses explode */
		case GF_CORPSE_EXPL:
			{
				if (o_ptr->tval == TV_CORPSE)
				{
					monster_race *r_ptr = &r_info[o_ptr->pval2];
					s32b dama, radius = 7;

					if (r_ptr->flags1 & RF1_FORCE_MAXHP)
						dama = maxroll(r_ptr->hdice, r_ptr->hside);
					else
						dama = damroll(r_ptr->hdice, r_ptr->hside);

					/* Adjust the damage */
					dama = dama * dam / 100;

					/* Adjust the radius */
					radius = radius * dam / 100;

					do_kill = TRUE;
					note_kill = (plural ? " explode!" : " explodes!");
					project(who, radius, y, x, dama, GF_SHARDS, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
				}
				break;
			}

			/* Acid -- Lots of things */
		case GF_ACID:
			{
				if (hates_acid(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " melt!" : " melts!");
					if (f3 & (TR3_IGNORE_ACID)) ignore = TRUE;
				}
				break;
			}

			/* Elec -- Rings and Wands */
		case GF_ELEC:
			{
				if (hates_elec(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (f3 & (TR3_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire -- Flammable objects */
		case GF_FIRE:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				break;
			}

			/* Cold -- potions and flasks */
		case GF_COLD:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
					if (f3 & (TR3_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Elec */
		case GF_PLASMA:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_elec(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (f3 & (TR3_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Cold */
		case GF_METEOR:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " shatter!" : " shatters!");
					if (f3 & (TR3_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Hack -- break potions and such */
		case GF_ICE:
		case GF_SHARDS:
		case GF_FORCE:
		case GF_SOUND:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
				}
				break;
			}

			/* Mana and Chaos -- destroy everything */
		case GF_MANA:
			{
				do_kill = TRUE;
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
				break;
			}

		case GF_DISINTEGRATE:
			{
				do_kill = TRUE;
				note_kill = (plural ? " evaporate!" : " evaporates!");
				break;
			}

		case GF_CHAOS:
			{
				do_kill = TRUE;
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
				if (f2 & (TR2_RES_CHAOS)) ignore = TRUE;
				break;
			}

			/* Holy Fire and Hell Fire -- destroys cursed non-artifacts */
		case GF_HOLY_FIRE:
		case GF_HELL_FIRE:
			{
				if (cursed_p(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
				}
				break;
			}

			/* Unlock chests */
		case GF_KILL_TRAP:
		case GF_KILL_DOOR:
			{
				/* Chests are noticed only if trapped or locked */
				if (o_ptr->tval == TV_CHEST)
				{
					/* Disarm/Unlock traps */
					if (o_ptr->pval > 0)
					{
						/* Disarm or Unlock */
						o_ptr->pval = (0 - o_ptr->pval);

						/* Identify */
						object_known(o_ptr);

						/* Notice */
						if (o_ptr->marked)
						{
							msg_print("Click!");
							obvious = TRUE;
						}
					}
				}

				break;
			}
		case GF_STAR_IDENTIFY:
			{
				/* Identify it fully */
				object_aware(o_ptr);
				object_known(o_ptr);

				/* Mark the item as fully known */
				o_ptr->ident |= (IDENT_MENTAL);

				/* Process the appropriate hooks */
				identify_hooks(0 - this_o_idx, o_ptr, IDENT_FULL);

				/* Squelch ! */
				squeltch_grid();

				break;
			}
		case GF_IDENTIFY:
			{
				object_aware(o_ptr);
				object_known(o_ptr);

				/* Process the appropriate hooks */
				identify_hooks(0 - this_o_idx, o_ptr, IDENT_NORMAL);

				/* Squelch ! */
				squeltch_grid();

				break;
			}
		case GF_RAISE:
			{
				get_pos_player(7, &y, &x);

				/* Only corpses can be raised */
				if (o_ptr->tval == TV_CORPSE)
				{
					int ego = raise_ego[rand_int(MAX_RAISE)];

					if (place_monster_one(y, x, o_ptr->pval2, ego, FALSE, (!who) ? MSTATUS_PET : MSTATUS_ENEMY))
						msg_print("A monster rises from the grave!");
					do_kill = TRUE;
				}
				break;
			}
		case GF_RAISE_DEMON:
			{
				monster_race *r_ptr = &r_info[o_ptr->pval2];
				cptr name;

				if (o_ptr->tval != TV_CORPSE) break;

				if (randint(100) > r_ptr->level - p_ptr->lev)
				{
					if (r_ptr->level < 10) name = "Manes";
					else if (r_ptr->level < 18) name = "Tengu";
					else if (r_ptr->level < 26) name = "Imp";
					else if (r_ptr->level < 34) name = "Arch-vile";
					else if (r_ptr->level < 42) name = "Bodak";
					else if (r_ptr->level < 50) name = "Erynies";
					else if (r_ptr->level < 58) name = "Vrock";
					else if (r_ptr->level < 66) name = "Hezrou";
					else if (r_ptr->level < 74) name = "Glabrezu";
					else if (r_ptr->level < 82) name = "Nalfeshnee";
					else if (r_ptr->level < 90) name = "Marilith";
					else name = "Nycadaemon";

					if (place_monster_one(y, x, test_monster_name(name), 0, FALSE, (!who) ? MSTATUS_PET : MSTATUS_ENEMY))
						msg_print("A demon emerges from Hell!");
				}

				do_kill = TRUE;
				break;
			}
		default:
			break;
		}


		/* Attempt to destroy the object */
		if (do_kill)
		{
			/* Effect "observed" */
			if (o_ptr->marked)
			{
				obvious = TRUE;
				object_desc(o_name, o_ptr, FALSE, 0);
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (o_ptr->marked)
				{
					msg_format("The %s %s unaffected!",
					           o_name, (plural ? "are" : "is"));
				}
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (o_ptr->marked && note_kill)
				{
					msg_format("The %s%s", o_name, note_kill);
				}

				o_sval = o_ptr->sval;
				is_potion = ((k_info[o_ptr->k_idx].tval == TV_POTION) || (k_info[o_ptr->k_idx].tval == TV_POTION2));


				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Potions produce effects when 'shattered' */
				if (is_potion)
				{
					(void)potion_smash_effect(who, y, x, o_sval);
				}


				/* Redraw */
				lite_spot(y, x);
			}
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}

/* Can the monster be hurt ? */
bool_ hurt_monster(monster_type *m_ptr)
{
	if (m_ptr->status == MSTATUS_COMPANION) return FALSE;
	else return TRUE;
}

/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
bool_ project_m(int who, int r, int y, int x, int dam, int typ)
{
	int tmp;

	cave_type *c_ptr = &cave[y][x];

	monster_type *m_ptr = &m_list[c_ptr->m_idx];

	monster_race *r_ptr = race_inf(m_ptr);

	char killer [80];

	cptr name = (r_name + r_ptr->name);

	/* Is the monster "seen"? */
	bool_ seen;

	/* Were the effects "obvious" (if seen)? */
	bool_ obvious = FALSE;

	/* Were the effects "irrelevant"? */
	bool_ skipped = FALSE;


	/* Move setting */
	int x1 = 0;
	int y1 = 0;
	int a = 0;
	int b = 0;
	int do_move = 0;

	/* Polymorph setting (true or false) */
	int do_poly = 0;

	/* Teleport setting (max distance) */
	int do_dist = 0;

	/* Confusion setting (amount to confuse) */
	int do_conf = 0;

	/* Stunning setting (amount to stun) */
	int do_stun = 0;

	/* Bleeding amount */
	int do_cut = 0;

	/* Poison amount */
	int do_pois = 0;

	/* Sleep amount (amount to sleep) */
	int do_sleep = 0;

	/* Fear amount (amount to fear) */
	int do_fear = 0;


	/* Hold the monster name */
	char m_name[80];

	/* Assume no note */
	cptr note = NULL;

	/* Assume a default death */
	cptr note_dies = " dies.";


	/* Nobody here */
	if (!c_ptr->m_idx) return (FALSE);

	/* Never affect projector */
	if (who && (c_ptr->m_idx == who)) return (FALSE);

	/*
	 * Don't affect already dead monsters
	 * Prevents problems with chain reactions of exploding monsters
	 */
	if (m_ptr->hp < 0) return (FALSE);


	/* Remember if the monster is within player's line of sight */
	seen = (m_ptr->ml && ((who != -101) && (who != -100))) ? TRUE : FALSE;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Check gods */
	project_check_gods(typ);

	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(m_name, m_ptr, 0);

	/* Mega Gachk */
	if (r_ptr->flags2 & RF2_DEATH_ORB)
	{
		msg_format("%^s is immune to magic.", m_name);
		return seen;
	}

	/* Some monsters get "destroyed" */
	if ((r_ptr->flags3 & (RF3_DEMON)) ||
	                (r_ptr->flags3 & (RF3_UNDEAD)) ||
	                (r_ptr->flags2 & (RF2_STUPID)) ||
	                (r_ptr->flags3 & (RF3_NONLIVING)) ||
	                (strchr("Evg", r_ptr->d_char)))
	{
		/* Special note at death */
		note_dies = " is destroyed.";
	}

	if (!who && (is_friend(m_ptr) >= 0))
	{
		bool_ get_angry = FALSE;
		/* Grrr? */
		switch (typ)
		{
		case GF_AWAY_UNDEAD:
		case GF_AWAY_EVIL:
		case GF_AWAY_ALL:
		case GF_CHARM:
		case GF_CHARM_UNMOVING:
		case GF_STAR_CHARM:
		case GF_CONTROL_UNDEAD:
		case GF_CONTROL_ANIMAL:
		case GF_CONTROL_DEMON:
		case GF_OLD_HEAL:
		case GF_OLD_SPEED:
		case GF_DARK_WEAK:
		case GF_JAM_DOOR:
		case GF_RAISE:
		case GF_RAISE_DEMON:
		case GF_IDENTIFY:
			break;              /* none of the above anger */
		case GF_TRAP_DEMONSOUL:
			if (r_ptr->flags3 & RF3_DEMON)
				get_angry = TRUE;
			break;
		case GF_KILL_WALL:
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
				get_angry = TRUE;
			break;
		case GF_HOLY_FIRE:
			if (!(r_ptr->flags3 & (RF3_GOOD)))
				get_angry = TRUE;
			break;
		case GF_TURN_UNDEAD:
		case GF_DISP_UNDEAD:
			if (r_ptr->flags3 & RF3_UNDEAD)
				get_angry = TRUE;
			break;
		case GF_TURN_EVIL:
		case GF_DISP_EVIL:
			if (r_ptr->flags3 & RF3_EVIL)
				get_angry = TRUE;
			break;
		case GF_DISP_GOOD:
			if (r_ptr->flags3 & RF3_GOOD)
				get_angry = TRUE;
			break;
		case GF_DISP_DEMON:
			if (r_ptr->flags3 & RF3_DEMON)
				get_angry = TRUE;
			break;
		case GF_DISP_LIVING:
		case GF_UNBREATH:
			if (!(r_ptr->flags3 & (RF3_UNDEAD)) &&
			                !(r_ptr->flags3 & (RF3_NONLIVING)))
				get_angry = TRUE;
			break;
		case GF_PSI:
		case GF_PSI_DRAIN:
			if (!(r_ptr->flags2 & (RF2_EMPTY_MIND)))
				get_angry = TRUE;
			break;
		case GF_DOMINATION:
			if (!(r_ptr->flags3 & (RF3_NO_CONF)))
				get_angry = TRUE;
			break;
		case GF_OLD_POLY:
		case GF_OLD_CLONE:
			if (randint(8) == 1)
				get_angry = TRUE;
			break;
		case GF_LITE:
		case GF_LITE_WEAK:
			if (r_ptr->flags3 & RF3_HURT_LITE)
				get_angry = TRUE;
			break;
		case GF_INSTA_DEATH:
			get_angry = TRUE;
			break;
		case GF_ELEMENTAL_GROWTH:
		case GF_ELEMENTAL_WALL:
			get_angry = FALSE;
			break;
		}

		/* Now anger it if appropriate */
		if (get_angry == TRUE && !(who))
		{
			switch (is_friend(m_ptr))
			{
			case 1:
				if (change_side(m_ptr)) msg_format("%^s gets angry!", m_name);
				break;
			case 0:
				msg_format("%^s gets angry!", m_name);
				m_ptr->status = MSTATUS_NEUTRAL_M;
				break;
			}
		}
	}


	/* Analyze the damage type */
	switch (typ)
	{
	case GF_ATTACK:
		{
			if (seen) obvious = TRUE;

			py_attack(y, x, dam);

			skipped = TRUE;

			dam = 0;
			break;
		}

	case GF_IDENTIFY:
		{
			if (seen) obvious = TRUE;

			/* Probe */
			do_probe(c_ptr->m_idx);

			dam = 0;
			break;
		}

		/* Death -- instant death  */
	case GF_DEATH:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->r_flags1 & RF1_UNIQUE)
			{
				note = " resists.";
				dam = 0;
			}
			else
			{
				/* It KILLS */
				dam = 32535;
			}
			break;
		}
		/* Magic Missile -- pure damage */
	case GF_MISSILE:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Acid */
	case GF_ACID:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags9 & (RF9_SUSCEP_ACID))
			{
				note = " is hit hard.";
				dam *= 3;
				if (seen) r_ptr->r_flags9 |= (RF9_SUSCEP_ACID);
			}
			if (r_ptr->flags3 & (RF3_IM_ACID))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_ACID);
			}
			break;
		}

		/* Electricity */
	case GF_ELEC:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags9 & (RF9_SUSCEP_ELEC))
			{
				note = " is hit hard.";
				dam *= 3;
				if (seen) r_ptr->r_flags9 |= (RF9_SUSCEP_ELEC);
			}
			if (r_ptr->flags3 & (RF3_IM_ELEC))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_ELEC);
			}
			break;
		}

		/* Fire damage */
	case GF_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_SUSCEP_FIRE))
			{
				note = " is hit hard.";
				dam *= 3;
				if (seen) r_ptr->r_flags3 |= (RF3_SUSCEP_FIRE);
			}
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_FIRE);
			}
			break;
		}

		/* Cold */
	case GF_COLD:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_SUSCEP_COLD))
			{
				note = " is hit hard.";
				dam *= 3;
				if (seen) r_ptr->r_flags3 |= (RF3_SUSCEP_COLD);
			}
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_COLD);
			}
			break;
		}

		/* Poison */
	case GF_POIS:
		{
			if (seen) obvious = TRUE;
			if (magik(25)) do_pois = (10 + randint(11) + r) / (r + 1);
			if (r_ptr->flags9 & (RF9_SUSCEP_POIS))
			{
				note = " is hit hard.";
				dam *= 3;
				do_pois *= 2;
				if (seen) r_ptr->r_flags9 |= (RF9_SUSCEP_POIS);
			}
			if (r_ptr->flags3 & (RF3_IM_POIS))
			{
				note = " resists a lot.";
				dam /= 9;
				do_pois = 0;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_POIS);
			}
			break;
		}


		/* Thick Poison */
	case GF_UNBREATH:
		{
			if (seen) obvious = TRUE;
			if (magik(15)) do_pois = (10 + randint(11) + r) / (r + 1);
			if ((r_ptr->flags3 & (RF3_NONLIVING)) || (r_ptr->flags3 & (RF3_UNDEAD)))
			{
				note = " is immune.";
				dam = 0;
				do_pois = 0;
			}
			break;
		}

		/* Nuclear waste */
	case GF_NUKE:
		{
			if (seen) obvious = TRUE;

			if (r_ptr->flags3 & (RF3_IM_POIS))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen) r_ptr->r_flags3 |= (RF3_IM_POIS);
			}
			else if (randint(3) == 1) do_poly = TRUE;
			break;
		}

		/* Holy Orb -- hurts Evil (replaced with Hellfire) */
	case GF_HELL_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam *= 2;
				note = " is hit hard.";
				if (seen) r_ptr->r_flags3 |= (RF3_EVIL);
			}
			break;
		}

		/* Holy Fire -- hurts Evil, Good are immune, others _resist_ */
	case GF_HOLY_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_GOOD))
			{
				dam = 0;
				note = " is immune.";
				if (seen) r_ptr->r_flags3 |= (RF3_GOOD);
			}
			else if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam *= 2;
				note = " is hit hard.";
				if (seen) r_ptr->r_flags3 |= (RF3_EVIL);
			}
			else
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
			}
			break;
		}

		/* Arrow -- XXX no defense */
	case GF_ARROW:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Plasma -- XXX perhaps check ELEC or FIRE */
	case GF_PLASMA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_PLAS))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen)
					r_ptr->r_flags3 |= (RF3_RES_PLAS);
			}
			break;
		}

		/* Nether -- see above */
	case GF_NETHER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				note = " is immune.";
				dam = 0;
				if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);
			}
			else if (r_ptr->flags3 & (RF3_RES_NETH))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);

				if (seen) r_ptr->r_flags3 |= (RF3_RES_NETH);
			}
			else if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam /= 2;
				note = " resists somewhat.";
				if (seen) r_ptr->r_flags3 |= (RF3_EVIL);
			}
			break;
		}

		/* Water (acid) damage -- Water spirits/elementals are immune */
	case GF_WATER:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->d_char == 'E') &&
			                (prefix(name, "W") ||
			                 (strstr((r_name + r_ptr->name), "Unmaker"))))
			{
				note = " is immune.";
				dam = 0;
			}
			else if (r_ptr->flags3 & (RF3_RES_WATE))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen) r_ptr->r_flags3 |= (RF3_RES_WATE);
			}
			break;
		}

		/* Wave = Water + Force */
	case GF_WAVE:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->d_char == 'E') &&
			                (prefix(name, "W") ||
			                 (strstr((r_name + r_ptr->name), "Unmaker"))))
			{
				note = " is immune.";
				dam = 0;
			}
			else if (r_ptr->flags3 & (RF3_RES_WATE))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen) r_ptr->r_flags3 |= (RF3_RES_WATE);
			}

			if (who == 0)
			{
				a = 0;
				b = 0;

				/* Get vector from firer to target */
				x1 = (m_ptr->fx - p_ptr->px) * 10;
				y1 = (m_ptr->fy - p_ptr->py) * 10;

				/* Make sure no zero divides */
				if (x1 == 0) x1 = 1;
				if (y1 == 0) y1 = 1;

				/* Select direction monster is being pushed */

				/* Roughly horizontally */
				if ((2*y1) / x1 == 0)
				{
					if (x1 > 0)
					{
						a = 1, b = 0;
					}
					else
					{
						a = -1, b = 0;
					}
				}

				/* Roughly vertically */
				else if ((2*x1) / y1 == 0)
				{
					if (y1 > 0)
					{
						a = 0, b = 1;
					}
					else
					{
						a = 0, b = -1;
					}
				}

				/* Take diagonals */
				else
				{
					if (y1 > 0)
					{
						b = 1;
					}
					else
					{
						b = -1;
					}
					if (x1 > 0)
					{
						a = 1;
					}
					else
					{
						a = -1;
					}
				}

				/* Move monster 2 offsets back */
				do_move = 2;

				/* Old monster coords in x,y */
				y1 = m_ptr->fy;
				x1 = m_ptr->fx;

				/* Monster move offsets in a,b */
				note = " is thrown away.";
			}
			break;
		}

		/* Chaos -- Chaos breathers resist */
	case GF_CHAOS:
		{
			if (seen) obvious = TRUE;
			do_poly = TRUE;
			do_conf = (5 + randint(11) + r) / (r + 1);
			if ((r_ptr->flags4 & (RF4_BR_CHAO)) ||
			                ((r_ptr->flags3 & (RF3_DEMON)) && (randint(3) == 1)))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				do_poly = FALSE;
			}
			break;
		}

		/* Shards -- Shard breathers resist */
	case GF_SHARDS:
		{
			if (seen) obvious = TRUE;
			if (magik(33)) do_cut = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_SHAR))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				do_cut = 0;
			}
			break;
		}

		/* Rocket: Shard resistance helps */
	case GF_ROCKET:
		{
			if (seen) obvious = TRUE;

			if (magik(12)) do_cut = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_SHAR))
			{
				note = " resists somewhat.";
				dam /= 2;
				do_cut = 0;
			}
			break;
		}


		/* Sound -- Sound breathers resist */
	case GF_SOUND:
		{
			if (seen) obvious = TRUE;
			if (who <= 0)
			{
				if (rand_int(100 - p_ptr->lev) < 50)
					do_stun = (10 + randint(15) + r) / (r + 1);
			}
			else
				do_stun = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_SOUN))
			{
				note = " resists.";
				dam *= 2;
				dam /= (randint(6) + 6);
			}
			break;
		}

		/* Confusion */
	case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;
			do_conf = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_CONF))
			{
				note = " resists.";
				dam *= 2;
				dam /= (randint(6) + 6);
			}
			else if (r_ptr->flags3 & (RF3_NO_CONF))
			{
				note = " resists somewhat.";
				dam /= 2;
			}
			break;
		}

		/* Disenchantment -- Breathers and Disenchanters resist */
	case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_DISE))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen) r_ptr->r_flags3 |= (RF3_RES_DISE);
			}
			break;
		}

		/* Nexus -- Breathers and Existers resist */
	case GF_NEXUS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_RES_NEXU))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				if (seen) r_ptr->r_flags3 |= (RF3_RES_NEXU);
			}
			break;
		}

		/* Force */
	case GF_FORCE:
		{
			if (seen) obvious = TRUE;

			/*
			 * If fired by player, try pushing monster.
			 * First get vector from player to monster.
			 * x10 so we can use pseudo-fixed point maths.
			 *
			 * Really should use get_angle_to_grid (util.c)
			 */
			if (who == 0)
			{
				a = 0;
				b = 0;

				/* Get vector from firer to target */
				x1 = (m_ptr->fx - p_ptr->px) * 10;
				y1 = (m_ptr->fy - p_ptr->py) * 10;

				/* Make sure no zero divides */
				if (x1 == 0) x1 = 1;
				if (y1 == 0) y1 = 1;

				/* Select direction monster is being pushed */

				/* Roughly horizontally */
				if ((2*y1) / x1 == 0)
				{
					if (x1 > 0)
					{
						a = 1, b = 0;
					}
					else
					{
						a = -1, b = 0;
					}
				}

				/* Roughly vertically */
				else if ((2*x1) / y1 == 0)
				{
					if (y1 > 0)
					{
						a = 0, b = 1;
					}
					else
					{
						a = 0, b = -1;
					}
				}

				/* Take diagonals */
				else
				{
					if (y1 > 0)
					{
						b = 1;
					}
					else
					{
						b = -1;
					}
					if (x1 > 0)
					{
						a = 1;
					}
					else
					{
						a = -1;
					}
				}

				/* Move monster 2 offsets back */
				do_move = 2;

				/* Old monster coords in x,y */
				y1 = m_ptr->fy;
				x1 = m_ptr->fx;

				/* Monster move offsets in a,b */
				note = " is thrown away.";
			}

			/* --hack-- Only stun if a monster fired it */
			else do_stun = (randint(15) + r) / (r + 1);

			if (r_ptr->flags4 & (RF4_BR_WALL))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
			}
			break;
		}

		/* Inertia -- breathers resist */
	case GF_INERTIA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_INER))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
			}
			else
			{
				/* Powerful monsters can resist */
				if (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
					note = " starts moving slower.";
				}
			}
			break;
		}

		/* Time -- breathers resist */
	case GF_TIME:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_TIME))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
			}
			break;
		}

		/* Gravity -- breathers resist */
	case GF_GRAVITY:
		{
			bool_ resist_tele = FALSE;

			if (seen) obvious = TRUE;

			if (r_ptr->flags3 & (RF3_RES_TELE))
			{
				if (r_ptr->flags1 & (RF1_UNIQUE))
				{
					if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
					note = " is unaffected!";
					resist_tele = TRUE;
				}
				else if (m_ptr->level > randint(100))
				{
					if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
					note = " resists!";
					resist_tele = TRUE;
				}
			}

			if (!resist_tele) do_dist = 10;
			else do_dist = 0;

			if (r_ptr->flags4 & (RF4_BR_GRAV))
			{
				note = " resists.";
				dam *= 3;
				dam /= (randint(6) + 6);
				do_dist = 0;
			}
			else
			{
				/* 1. slowness */
				/* Powerful monsters can resist */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					obvious = FALSE;
				}
				/* Normal monsters slow down */
				else
				{
					if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
					note = " starts moving slower.";
				}

				/* 2. stun */
				do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

				/* Attempt a saving throw */
				if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
				{
					/* Resist */
					do_stun = 0;
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
				}
			}
			break;
		}

		/* Pure damage */
	case GF_MANA:
		{
			if (seen) obvious = TRUE;
			break;
		}


		/* Pure damage */
	case GF_DISINTEGRATE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				if (seen) r_ptr->r_flags3 |= (RF3_HURT_ROCK);
				note = " loses some skin!";
				note_dies = " evaporates!";
				dam *= 2;
			}

			if (r_ptr->flags1 & RF1_UNIQUE)
			{
				if (rand_int(m_ptr->level + 10) > rand_int(p_ptr->lev))
				{
					note = " resists.";
					dam >>= 3;
				}
			}
			break;
		}

	case GF_FEAR:
		{
			if (r_ptr->flags3 & (RF3_NO_FEAR))
				note = " is unaffected.";
			else
				set_afraid(p_ptr->afraid + (dam / 2) + randint(dam / 2));

			/* No damage */
			dam = 0;
			break;
		}

	case GF_PSI:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
				note = " is immune!";
			}
			else if ((r_ptr->flags2 & RF2_STUPID) ||
			                (r_ptr->flags2 & RF2_WEIRD_MIND) ||
			                (r_ptr->flags3 & RF3_ANIMAL) ||
			                (m_ptr->level > randint(3 * dam)))
			{
				dam /= 3;
				note = " resists.";

				/* Powerful demons & undead can turn a mindcrafter's
				* attacks back on them */
				if (((r_ptr->flags3 & RF3_UNDEAD) ||
				                (r_ptr->flags3 & RF3_DEMON)) &&
				                (m_ptr->level > p_ptr->lev / 2) &&
				                (randint(2) == 1))
				{
					note = NULL;
					msg_format("%^s%s corrupted mind backlashes your attack!",
					           m_name, (seen ? "'s" : "s"));
					/* Saving throw */
					if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
					}
					else
					{
						/* Injure +/- confusion */
						monster_desc(killer, m_ptr, 0x88);
						take_hit(dam, killer);   /* has already been /3 */
						if (randint(4) == 1)
						{
							switch (randint(4))
							{
							case 1:
								set_confused(p_ptr->confused + 3 + randint(dam));
								break;
							case 2:
								set_stun(p_ptr->stun + randint(dam));
								break;
							case 3:
								{
									if (r_ptr->flags3 & (RF3_NO_FEAR))
										note = " is unaffected.";
									else
										set_afraid(p_ptr->afraid + 3 + randint(dam));
									break;
								}
							default:
								if (!p_ptr->free_act)
									(void)set_paralyzed(p_ptr->paralyzed + randint(dam));
								break;
							}
						}
					}
					dam = 0;
				}
			}

			if ((dam > 0) && (randint(4) == 1))
			{
				switch (randint(4))
				{
				case 1:
					do_conf = 3 + randint(dam);
					break;
				case 2:
					do_stun = 3 + randint(dam);
					break;
				case 3:
					do_fear = 3 + randint(dam);
					break;
				default:
					do_sleep = 3 + randint(dam);
					break;
				}
			}

			note_dies = " collapses, a mindless husk.";
			break;
		}

	case GF_PSI_DRAIN:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags2 & RF2_EMPTY_MIND)
			{
				dam = 0;
				note = " is immune!";
			}
			else if ((r_ptr->flags2 & RF2_STUPID) ||
			                (r_ptr->flags2 & RF2_WEIRD_MIND) ||
			                (r_ptr->flags3 & RF3_ANIMAL) ||
			                (m_ptr->level > randint(3 * dam)))
			{
				dam /= 3;
				note = " resists.";

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if (((r_ptr->flags3 & RF3_UNDEAD) ||
				                (r_ptr->flags3 & RF3_DEMON)) &&
				                (m_ptr->level > p_ptr->lev / 2) &&
				                (randint(2) == 1))
				{
					note = NULL;
					msg_format("%^s%s corrupted mind backlashes your attack!",
					           m_name, (seen ? "'s" : "s"));
					/* Saving throw */
					if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
					}
					else
					{
						/* Injure + mana drain */
						monster_desc(killer, m_ptr, 0x88);
						msg_print("Your psychic energy is drained!");
						p_ptr->csp = MAX(0, p_ptr->csp - damroll(5, dam) / 2);
						p_ptr->redraw |= PR_MANA;
						take_hit(dam, killer);   /* has already been /3 */
					}
					dam = 0;
				}
			}
			else if (dam > 0)
			{
				int b = damroll(5, dam) / 4;
				msg_format("You convert %s%s pain into psychic energy!",
				           m_name, (seen ? "'s" : "s"));
				b = MIN(p_ptr->msp, p_ptr->csp + b);
				p_ptr->csp = b;
				p_ptr->redraw |= PR_MANA;
			}

			note_dies = " collapses, a mindless husk.";
			break;
		}

	case GF_TELEKINESIS:
		{
			if (seen) obvious = TRUE;
			do_dist = 7;
			/* 1. stun */
			do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->level > 5 + randint(dam)))
			{
				/* Resist */
				do_stun = 0;
				/* No obvious effect */
				obvious = FALSE;
			}
			break;
		}

		/* Meteor -- powerful magic missile */
	case GF_METEOR:
		{
			if (seen) obvious = TRUE;
			break;
		}

	case GF_DOMINATION:
		{
			if (is_friend(m_ptr) > 0) break;
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (r_ptr->flags3 & (RF3_NO_CONF)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/*
				 * Powerful demons & undead can turn a mindcrafter's
				 * attacks back on them
				 */
				if (((r_ptr->flags3 & RF3_UNDEAD) ||
				                (r_ptr->flags3 & RF3_DEMON)) &&
				                (m_ptr->level > p_ptr->lev / 2) &&
				                (randint(2) == 1))
				{
					note = NULL;
					msg_format("%^s%s corrupted mind backlashes your attack!",
					           m_name, (seen ? "'s" : "s"));
					/* Saving throw */
					if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
					}
					else
					{
						/* Confuse, stun, terrify */
						switch (randint(4))
						{
						case 1:
							set_stun(p_ptr->stun + dam / 2);
							break;
						case 2:
							set_confused(p_ptr->confused + dam / 2);
							break;
						default:
							{
								if (r_ptr->flags3 & (RF3_NO_FEAR))
									note = " is unaffected.";
								else
									set_afraid(p_ptr->afraid + dam);
							}
						}
					}
				}
				else
				{
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
				}
			}
			else
			{
				if ((dam > 29) && (randint(100) < dam))
				{
					note = " is in your thrall!";
					m_ptr->status = MSTATUS_PET;
					if ((r_ptr->flags3 & RF3_ANIMAL) && (!(r_ptr->flags3 & RF3_EVIL)))
						inc_piety(GOD_YAVANNA, m_ptr->level * 2);
				}
				else
				{
					switch (randint(4))
					{
					case 1:
						do_stun = dam / 2;
						break;
					case 2:
						do_conf = dam / 2;
						break;
					default:
						do_fear = dam;
					}
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}



		/* Ice -- Cold + Cuts + Stun */
	case GF_ICE:
		{
			if (seen) obvious = TRUE;
			do_stun = (randint(15) + 1) / (r + 1);
			if (magik(33)) do_cut = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags3 & (RF3_SUSCEP_COLD))
			{
				note = " is hit hard.";
				dam *= 3;
				do_cut *= 2;
				if (seen) r_ptr->r_flags3 |= (RF3_SUSCEP_COLD);
			}
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				note = " resists a lot.";
				dam /= 9;
				do_cut = 0;
				if (seen) r_ptr->r_flags3 |= (RF3_IM_COLD);
			}
			break;
		}


		/* Drain Life */
	case GF_OLD_DRAIN:
		{
			if (seen) obvious = TRUE;

			if ((r_ptr->flags3 & (RF3_UNDEAD)) ||
			                (r_ptr->flags3 & (RF3_DEMON)) ||
			                (r_ptr->flags3 & (RF3_NONLIVING)) ||
			                (strchr("Egv", r_ptr->d_char)))
			{
				if (r_ptr->flags3 & (RF3_UNDEAD))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);
				}
				if (r_ptr->flags3 & (RF3_DEMON))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_DEMON);
				}

				note = " is unaffected!";
				obvious = FALSE;
				dam = 0;
			}

			break;
		}

		/* Death Ray */
	case GF_DEATH_RAY:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->flags3 & (RF3_UNDEAD)) ||
			                (r_ptr->flags3 & (RF3_NONLIVING)))
			{
				if (r_ptr->flags3 & (RF3_UNDEAD))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);
				}

				note = " is immune.";
				obvious = FALSE;
				dam = 0;
			}
			else if (((r_ptr->flags1 & (RF1_UNIQUE)) &&
			                (randint(888) != 666)) ||
			                (((m_ptr->level + randint(20)) > randint((dam) + randint(10))) &&
			                 randint(100) != 66 ))
			{
				note = " resists!";
				obvious = FALSE;
				dam = 0;
			}

			else dam = (p_ptr->lev) * 200;

			break;
		}

		/* Polymorph monster (Use "dam" as "power") */
	case GF_OLD_POLY:
		{
			if (seen) obvious = TRUE;

			/* Attempt to polymorph (see below) */
			do_poly = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				do_poly = FALSE;
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Clone monsters (Ignore "dam") */
	case GF_OLD_CLONE:
		{
			bool_ is_frien = FALSE;

			if (seen) obvious = TRUE;
			if ((is_friend(m_ptr) > 0) && (randint(3) != 1))
				is_frien = TRUE;

			/* Heal fully */
			m_ptr->hp = m_ptr->maxhp;

			/* Speed up */
			if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;

			/* Attempt to clone. */
			if (multiply_monster(c_ptr->m_idx, is_frien, TRUE))
			{
				note = " spawns!";
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Heal Monster (use "dam" as amount of healing) */
	case GF_OLD_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			m_ptr->csleep = 0;

			/* Heal */
			m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Message */
			note = " looks healthier.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Speed Monster (Ignore "dam") */
	case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			/* Speed up */
			if (m_ptr->mspeed < m_ptr->speed + 15) m_ptr->mspeed += 10;
			note = " starts moving faster.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Slow Monster (Use "dam" as "power") */
	case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* Normal monsters slow down */
			else
			{
				if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
				note = " starts moving slower.";
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
	case GF_OLD_SLEEP:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags3 & (RF3_NO_SLEEP)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_SLEEP))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_SLEEP);
				}

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
				note = " falls asleep!";
				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
	case GF_STASIS:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
				note = " is suspended!";
				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Charm monster */
	case GF_CHARM:
		{
			dam += (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1);

			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((m_ptr->mflag & MFLAG_QUEST) ||
			                (r_ptr->flags3 & RF3_NO_CONF) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 5))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				if (is_friend(m_ptr) < 0)
				{
					note = " suddenly seems friendly!";
					m_ptr->status = MSTATUS_FRIEND;
					if ((r_ptr->flags3 & RF3_ANIMAL) && (!(r_ptr->flags3 & RF3_EVIL)))
						inc_piety(GOD_YAVANNA, m_ptr->level * 2);
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* *Charm* monster */
	case GF_STAR_CHARM:
		{
			dam += (adj_con_fix[p_ptr->stat_ind[A_CHR]] - 1);

			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((m_ptr->mflag & MFLAG_QUEST) ||
			                (r_ptr->flags3 & RF3_NO_CONF) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 5))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				if (is_friend(m_ptr) < 0)
				{
					note = " suddenly seems friendly!";
					if (can_create_companion()) m_ptr->status = MSTATUS_COMPANION;
					else m_ptr->status = MSTATUS_PET;

					if ((r_ptr->flags3 & RF3_ANIMAL) && (!(r_ptr->flags3 & RF3_EVIL)))
						inc_piety(GOD_YAVANNA, m_ptr->level * 2);
				}
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Control undead */
	case GF_CONTROL_UNDEAD:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (!(r_ptr->flags3 & RF3_UNDEAD)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				note = " is in your thrall!";
				m_ptr->status = MSTATUS_PET;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Control never-moving */
	case GF_CHARM_UNMOVING:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & RF1_UNIQUE) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (!(r_ptr->flags1 & RF1_NEVER_MOVE)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				note = " is in your thrall!";
				m_ptr->status = MSTATUS_PET;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Tame animal */
	case GF_CONTROL_ANIMAL:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (!(r_ptr->flags3 & (RF3_ANIMAL))) ||
			                (r_ptr->flags3 & (RF3_NO_CONF)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				note = " is tamed!";
				m_ptr->status = MSTATUS_PET;
				inc_piety(GOD_YAVANNA, m_ptr->level * 2);
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Control demon */
	case GF_CONTROL_DEMON:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (!(r_ptr->flags3 & (RF3_DEMON))) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else if (p_ptr->aggravate)
			{
				note = " hates you too much!";
			}
			else
			{
				note = " obeys your commands!";
				m_ptr->status = MSTATUS_PET;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Confusion (Use "dam" as "power") */
	case GF_OLD_CONF:
		{
			if (seen) obvious = TRUE;

			/* Get confused later */
			do_conf = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags3 & (RF3_NO_CONF)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

	case GF_STUN:
		{
			if (seen) obvious = TRUE;

			do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				do_stun = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Confusion (Use "dam" as "power") */
	case GF_CONF_DAM:
		{
			if (seen) obvious = TRUE;

			/* Get confused later */
			do_conf = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags3 & (RF3_NO_CONF)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			break;
		}

	case GF_STUN_DAM:
		{
			if (seen) obvious = TRUE;

			do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				do_stun = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			break;
		}

		/* Implosion is the same than Stun_dam but only affect the living */
	case GF_IMPLOSION:
		{
			if (seen) obvious = TRUE;

			do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				do_stun = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* Non_living resists */
			if (r_ptr->flags3 & (RF3_NONLIVING))
			{
				/* Resist */
				do_stun = 0;
				dam = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			break;
		}

		/* Confusion & Stunning (Use "dam" as "power") */
	case GF_STUN_CONF:
		{
			if (seen) obvious = TRUE;

			/* Get confused later */
			do_conf = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags3 & (RF3_NO_CONF)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) r_ptr->r_flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}

			do_stun = damroll((p_ptr->lev / 10) + 3 , (dam)) + 1;

			/* Attempt a saving throw */
			if ((m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Resist */
				do_stun = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			break;
		}


		/* Lite, but only hurts susceptible creatures */
	case GF_LITE_WEAK:
		{
			/* Hurt by light */
			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				/* Obvious effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (seen) r_ptr->r_flags3 |= (RF3_HURT_LITE);

				/* Special effect */
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}



		/* Lite -- opposite of Dark */
	case GF_LITE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_LITE))
			{
				note = " resists.";
				dam *= 2;
				dam /= (randint(6) + 6);
			}
			else if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (seen) r_ptr->r_flags3 |= (RF3_HURT_LITE);
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
				dam *= 2;
			}
			break;
		}


		/* Dark -- opposite of Lite */
	case GF_DARK:
		{
			if (seen) obvious = TRUE;

			/* Likes darkness... */
			if ((r_ptr->flags4 & (RF4_BR_DARK)) ||
			                (r_ptr->flags3 & RF3_ORC) ||
			                (r_ptr->flags3 & RF3_HURT_LITE))
			{
				note = " resists.";
				dam *= 2;
				dam /= (randint(6) + 6);
			}
			break;
		}


		/* Stone to Mud */
	case GF_KILL_WALL:
		{
			/* Hurt by rock remover */
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				/* Notice effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (seen) r_ptr->r_flags3 |= (RF3_HURT_ROCK);

				/* Cute little message */
				note = " loses some skin!";
				note_dies = " dissolves!";
			}

			/* Usually, ignore the effects */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Teleport undead (Use "dam" as "power") */
	case GF_AWAY_UNDEAD:
		{

			if (dungeon_flags2 & DF2_NO_TELEPORT) break; /* No teleport on special levels */
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				bool_ resists_tele = FALSE;

				if (r_ptr->flags3 & (RF3_RES_TELE))
				{
					if (r_ptr->flags1 & (RF1_UNIQUE))
					{
						if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
						note = " is unaffected!";
						resists_tele = TRUE;
					}
					else if (m_ptr->level > randint(100))
					{
						if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
						note = " resists!";
						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);
					do_dist = dam;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport evil (Use "dam" as "power") */
	case GF_AWAY_EVIL:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT) break; /* No teleport on special levels */
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				bool_ resists_tele = FALSE;

				if (r_ptr->flags3 & (RF3_RES_TELE))
				{
					if (r_ptr->flags1 & (RF1_UNIQUE))
					{
						if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
						note = " is unaffected!";
						resists_tele = TRUE;
					}
					else if (m_ptr->level > randint(100))
					{
						if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
						note = " resists!";
						resists_tele = TRUE;
					}
				}

				if (!resists_tele)
				{
					if (seen) obvious = TRUE;
					if (seen) r_ptr->r_flags3 |= (RF3_EVIL);
					do_dist = dam;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport monster (Use "dam" as "power") */
	case GF_AWAY_ALL:
		{
			bool_ resists_tele = FALSE;

			if (dungeon_flags2 & DF2_NO_TELEPORT) break; /* No teleport on special levels */
			if (r_ptr->flags3 & (RF3_RES_TELE))
			{
				if (r_ptr->flags1 & (RF1_UNIQUE))
				{
					if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
					note = " is unaffected!";
					resists_tele = TRUE;
				}
				else if (m_ptr->level > randint(100))
				{
					if (seen) r_ptr->r_flags3 |= RF3_RES_TELE;
					note = " resists!";
					resists_tele = TRUE;
				}
			}

			if (!resists_tele)
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Prepare to teleport */
				do_dist = dam;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn undead (Use "dam" as "power") */
	case GF_TURN_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn evil (Use "dam" as "power") */
	case GF_TURN_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_EVIL);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn monster (Use "dam" as "power") */
	case GF_TURN_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some fear */
			do_fear = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (r_ptr->flags3 & (RF3_NO_FEAR)) ||
			                (m_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
				do_fear = 0;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Dispel undead */
	case GF_DISP_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_UNDEAD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
	case GF_DISP_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_EVIL);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel good */
	case GF_DISP_GOOD:
		{
			/* Only affect good */
			if (r_ptr->flags3 & (RF3_GOOD))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_GOOD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel living */
	case GF_DISP_LIVING:
		{
			/* Only affect non-undead */
			if (!(r_ptr->flags3 & (RF3_UNDEAD)) &&
			                !(r_ptr->flags3 & (RF3_NONLIVING)))
			{
				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel demons */
	case GF_DISP_DEMON:
		{
			/* Only affect demons */
			if (r_ptr->flags3 & (RF3_DEMON))
			{
				/* Learn about type */
				if (seen) r_ptr->r_flags3 |= (RF3_DEMON);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}

		/* Dispel monster */
	case GF_DISP_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Message */
			note = " shudders.";
			note_dies = " dissolves!";

			break;
		}

		/* Raise Death -- Heal monster */
	case GF_RAISE:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			m_ptr->csleep = 0;

			/* Heal */
			m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (health_who == c_ptr->m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Message */
			note = " looks healthier.";

			/* No "real" damage */
			dam = 0;
			break;
		}

		/* Trap the soul of a demon and leave body */
	case GF_TRAP_DEMONSOUL:
		{
			if (seen) obvious = TRUE;

			/* Check race */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			                (m_ptr->mflag & MFLAG_QUEST) ||
			                (!(r_ptr->flags3 & (RF3_DEMON))))
			{
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
				dam = 0;
			}
			/* Hack : drop corpse if the demon is killed by this
			 * spell */
			else if (dam > m_ptr->hp)
			{
				object_type forge, *i_ptr = &forge;

				/* Wipe the object */
				object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_CORPSE));

				/* Unique corpses are unique */
				if (r_ptr->flags1 & RF1_UNIQUE)
				{
					object_aware(i_ptr);
					i_ptr->name1 = 201;
				}

				/* Length of decay - very long time */
				i_ptr->pval = 100000;

				/* Set weight */
				i_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 10) + 1;

				/* Remember what we are */
				i_ptr->pval2 = m_ptr->r_idx;

				/* Give HP */
				i_ptr->pval3 = maxroll(r_ptr->hdice, r_ptr->hside);

				/* Drop it */
				drop_near(i_ptr, -1, y, x);
			}

			break;
		}

	case GF_INSTA_DEATH:
		{
			if (magik(95) && !(r_ptr->flags1 & RF1_UNIQUE) && !(r_ptr->flags3 & RF3_UNDEAD) && !(r_ptr->flags3 & RF3_NONLIVING)) {
				/* Kill outright, but reduce exp. */
				m_ptr->level = m_ptr->level / 3;
				dam = 32535; /* Should be enough */
				note = " faints.";
				note_dies = " is sucked out of life.";
			} else {
				/* No effect */
				skipped = TRUE;
			}

			break;
		}

	default:
		skipped = TRUE;
		break;
	}


	/* Absolutely no effect */
	if (skipped) return (FALSE);


	/* "Unique" monsters cannot be polymorphed */
	if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;

	/*
	 * "Quest" monsters cannot be polymorphed
	 */
	if (m_ptr->mflag & MFLAG_QUEST)
		do_poly = FALSE;

	/* "Unique" monsters can only be "killed" by the player unless they are player's friends */
	if ((r_ptr->flags1 & RF1_UNIQUE) && (m_ptr->status <= MSTATUS_NEUTRAL_P))
	{
		/* Uniques may only be killed by the player */
		if (who && (who != -2) && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	/*
	 * "Quest" monsters can only be "killed" by the player
	 */
	if (m_ptr->mflag & MFLAG_QUEST)
	{
		if ((who > 0) && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	if (do_pois && (!(r_ptr->flags3 & RF3_IM_POIS)) && (!(r_ptr->flags3 & RF4_BR_POIS)) && hurt_monster(m_ptr))
	{
		if (m_ptr->poisoned) note = " is more poisoned.";
		else note = " is poisoned.";
		m_ptr->poisoned += do_pois;
	}

	if (do_cut && (!(r_ptr->flags4 & RF4_BR_WALL)) && hurt_monster(m_ptr))
	{
		if (m_ptr->bleeding) note = " bleeds more strongly.";
		else note = " starts bleeding.";
		m_ptr->bleeding += do_cut;
	}

	/* Check for death */
	if ((dam > m_ptr->hp) && hurt_monster(m_ptr))
	{
		/* Extract method of death */
		note = note_dies;
	}

	/* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
	else if (do_poly && cave_floor_bold(y, x) && (randint(90) > m_ptr->level))
	{
		/* Default -- assume no polymorph */
		note = " is unaffected!";

		/* Handle polymorph */
		if (do_poly_monster(y, x))
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Monster polymorphs */
			note = " changes!";

			/* Turn off the damage */
			dam = 0;

			/* Hack -- Get new monster */
			m_ptr = &m_list[c_ptr->m_idx];

			/* Hack -- Get new race */
			r_ptr = race_inf(m_ptr);
		}
	}

	/* Handle moving the monster.
	 *
	 * Note: This is a effect of force, but only when used
	 * by the player. (For the moment). The usual stun effect
	 * is not applied.
	 */
	else if (do_move && hurt_monster(m_ptr))
	{
		int back = 0;

		/* Obvious */
		if (seen) obvious = TRUE;

		back = 0;  /* Default of no movement */

		/* How far can we push the monster? */
		for (do_move = 1; do_move < 3; do_move++)
		{
			/* Get monster coords */
			/* And offset position */
			y1 = m_ptr->fy + (b * do_move);
			x1 = m_ptr->fx + (a * do_move);

			if (!in_bounds(y1, x1)) continue;

			/* Require "empty" floor space */
			if (!in_bounds(y1, x1) || !cave_empty_bold(y1, x1)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave[y1][x1].feat == FEAT_GLYPH) continue;

			/* amount moved */
			back = do_move;
		}

		/* Move the monster */
		if (back)
		{
			y1 = m_ptr->fy + (b * back);
			x1 = m_ptr->fx + (a * back);
			monster_swap(m_ptr->fy, m_ptr->fx, y1, x1);

			if (back == 2)
			{
				note = " is knocked back!";
			}
			if (back == 1)
			{
				note = " is knocked back and crushed!";

				/* was kept from being pushed all the way, do extra dam */
				dam = dam * 13 / 10;
			}

			/* Get new position */
			y = y1;
			x = x1;

			/* Hack -- get new grid */
			c_ptr = &cave[y][x];
		}
		else /* could not move the monster */
		{
			note = " is severely crushed!";

			/* Do extra damage (1/3)*/
			dam = dam * 15 / 10;
		}

	}

	/* Handle "teleport" */
	else if (do_dist)
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Message */
		note = " disappears!";

		/* Teleport */
		teleport_away(c_ptr->m_idx, do_dist);

		/* Hack -- get new location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Hack -- get new grid */
		c_ptr = &cave[y][x];
	}

	/* Sound and Impact breathers never stun */
	else if (do_stun &&
	                !(r_ptr->flags4 & (RF4_BR_SOUN)) &&
	                !(r_ptr->flags4 & (RF4_BR_WALL)) && hurt_monster(m_ptr))
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Get confused */
		if (m_ptr->stunned)
		{
			note = " is more dazed.";
			tmp = m_ptr->stunned + (do_stun / 2);
		}
		else
		{
			note = " is dazed.";
			tmp = do_stun;
		}

		/* Apply stun */
		m_ptr->stunned = (tmp < 200) ? tmp : 200;
	}

	/* Confusion and Chaos breathers (and sleepers) never confuse */
	else if (do_conf &&
	                !(r_ptr->flags3 & (RF3_NO_CONF)) &&
	                !(r_ptr->flags4 & (RF4_BR_CONF)) &&
	                !(r_ptr->flags4 & (RF4_BR_CHAO)) && hurt_monster(m_ptr))
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Already partially confused */
		if (m_ptr->confused)
		{
			note = " looks more confused.";
			tmp = m_ptr->confused + (do_conf / 2);
		}

		/* Was not confused */
		else
		{
			note = " looks confused.";
			tmp = do_conf;
		}

		/* Apply confusion */
		m_ptr->confused = (tmp < 200) ? tmp : 200;
	}


	/* Fear */
	if (do_fear && hurt_monster(m_ptr))
	{
		/* Increase fear */
		tmp = m_ptr->monfear + do_fear;

		/* Set fear */
		m_ptr->monfear = (tmp < 200) ? tmp : 200;
	}


	/* If another monster did the damage, hurt the monster by hand */
	if (who > 0)
	{
		bool_ fear = FALSE;

		/* Dead monster */
		if (mon_take_hit_mon(who, c_ptr->m_idx, dam, &fear, note_dies))
		{}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(c_ptr->m_idx, dam);

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;
		}
	}
	/* If the player did it, give him experience, check fear */
	else if (hurt_monster(m_ptr))
	{
		bool_ fear = FALSE;

		/* Hurt the monster, check for fear and death */
		if (mon_take_hit(c_ptr->m_idx, dam, &fear, note_dies))
		{
			/* Dead monster */
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(c_ptr->m_idx, dam);

			/* Take note */
			if ((fear || do_fear) && (m_ptr->ml))
			{
				/* Sound */
				sound(SOUND_FLEE);

				/* Message */
				msg_format("%^s flees in terror!", m_name);
			}

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;
		}
	}


	/* XXX XXX XXX Verify this code */

	/* Update the monster */
	update_mon(c_ptr->m_idx, FALSE);

	/* Redraw the monster grid */
	lite_spot(y, x);


	/* Update monster recall window */
	if (monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}


	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;


	/* Return "Anything seen?" */
	return (obvious);
}


/* Is the spell unsafe for the player ? */
bool_ unsafe = FALSE;


/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * NOTE (Zangband): 'Bolt' attacks can be reflected back, so we need to know
 * if this is actually a ball or a bolt spell
 *
 *
 * We return "TRUE" if any "obvious" effects were observed.  XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static bool_ project_p(int who, int r, int y, int x, int dam, int typ, int a_rad)
{
	int k = 0, do_move = 0, a = 0, b = 0, x1 = 0, y1 = 0;

	/* Hack -- assume obvious */
	bool_ obvious = TRUE;

	/* Player blind-ness */
	bool_ blind = (p_ptr->blind ? TRUE : FALSE);

	/* Player needs a "description" (he is blind) */
	bool_ fuzzy = FALSE;

	/* Source monster */
	monster_type *m_ptr = NULL;

	/* Monster name (for attacks) */
	char m_name[80];

	/* Monster name (for damage) */
	char killer[80];

	/* Hack -- messages */
	cptr act = NULL;


	/* Player is not here */
	if ((x != p_ptr->px) || (y != p_ptr->py)) return (FALSE);

	/* Player cannot hurt himself */
	if ((!who) && (!unsafe)) return (FALSE);

	/* Bolt attack from a monster */
	if ((!a_rad) && get_skill(SKILL_DODGE) && (who > 0))
	{
		int chance = (p_ptr->dodge_chance - ((r_info[who].level * 5) / 6)) / 3;

		if ((chance > 0) && magik(chance))
		{
			msg_print("You dodge a magical attack!");
			return (TRUE);
		}
	}

	/* Effects done by the plane cannot bounce */
	if (p_ptr->reflect && !a_rad && !(randint(10) == 1) && ((who != -101) && (who != -100)))
	{
		int t_y, t_x;
		int max_attempts = 10;

		if (blind) msg_print("Something bounces!");
		else msg_print("The attack bounces!");

		/* Choose 'new' target */
		do
		{
			t_y = m_list[who].fy - 1 + randint(3);
			t_x = m_list[who].fx - 1 + randint(3);
			max_attempts--;
		}
		while (max_attempts && in_bounds2(t_y, t_x) &&
		                !(player_has_los_bold(t_y, t_x)));

		if (max_attempts < 1)
		{
			t_y = m_list[who].fy;
			t_x = m_list[who].fx;
		}

		project(0, 0, t_y, t_x, dam, typ, (PROJECT_STOP | PROJECT_KILL));

		disturb(1, 0);
		return TRUE;
	}

	/* XXX XXX XXX */
	/* Limit maximum damage */
	if (dam > 1600) dam = 1600;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* If the player is blind, be more descriptive */
	if (blind) fuzzy = TRUE;

	/* If the player is hit by a trap, be more descritive */
	if (who == -2) fuzzy = TRUE;

	/* Did ``God'' do it? */
	if (who == -99)
	{
		if (p_ptr->pgod)
		{
			/* Find out the name of player's god. */
			sprintf(killer, "%s",
			        deity_info[p_ptr->pgod].name);
		}
		else strcpy(killer, "Divine Wrath");
	}

	/* Did the dungeon do it? */
	if (who == -100)
	{
		sprintf(killer, "%s",
		        d_name + d_info[dungeon_type].name);
	}
	if (who == -101)
	{
		sprintf(killer, "%s",
		        f_name + f_info[cave[p_ptr->py][p_ptr->px].feat].name);
	}

	if (who >= -1)
	{
		/* Get the source monster */
		m_ptr = &m_list[who];

		/* Get the monster name */
		monster_desc(m_name, m_ptr, 0);

		/* Get the monster's real name */
		monster_desc(killer, m_ptr, 0x88);
	}

	if (who == -2)
	{
		sprintf(killer, "%s",
			t_name + t_info[cave[p_ptr->py][p_ptr->px].t_idx].name);
	}

	/* Analyze the damage */
	switch (typ)
	{
	case GF_DEATH_RAY:
		{
			if (fuzzy) msg_print("You are hit by pure death!");
			take_hit(32000, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
	case GF_ACID:
		{
			if (fuzzy) msg_print("You are hit by acid!");
			acid_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
	case GF_FIRE:
		{
			if (fuzzy) msg_print("You are hit by fire!");
			fire_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
	case GF_COLD:
		{
			if (fuzzy) msg_print("You are hit by cold!");
			cold_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
	case GF_ELEC:
		{
			if (fuzzy) msg_print("You are hit by lightning!");
			elec_dam(dam, killer);
			break;
		}

		/* Standard damage -- also poisons player */
	case GF_POIS:
		{
			if (fuzzy) msg_print("You are hit by poison!");
			if (p_ptr->resist_pois) dam = (dam + 2) / 3;
			if (p_ptr->oppose_pois) dam = (dam + 2) / 3;

			if ((!(p_ptr->oppose_pois || p_ptr->resist_pois)) &&
			                randint(HURT_CHANCE) == 1)
			{
				do_dec_stat(A_CON, STAT_DEC_NORMAL);
			}

			take_hit(dam, killer);

			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				set_poisoned(p_ptr->poisoned + rand_int(dam) + 10);
			}
			break;
		}

		/* Standard damage -- also poisons / mutates player */
	case GF_NUKE:
		{
			if (fuzzy) msg_print("You are hit by radiation!");
			if (p_ptr->resist_pois) dam = (2 * dam + 2) / 5;
			if (p_ptr->oppose_pois) dam = (2 * dam + 2) / 5;
			take_hit(dam, killer);
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				set_poisoned(p_ptr->poisoned + rand_int(dam) + 10);

				if (randint(5) == 1) /* 6 */
				{
					msg_print("You undergo a freakish metamorphosis!");
					if (randint(4) == 1) /* 4 */
						do_poly_self();
					else
						corrupt_player();
				}

				if (randint(6) == 1)
				{
					inven_damage(set_acid_destroy, 2);
				}
			}
			break;
		}

		/* Standard damage */
	case GF_MISSILE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Holy Orb -- Player only takes partial damage */
	case GF_HOLY_FIRE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

	case GF_HELL_FIRE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			take_hit(dam, killer);
			break;
		}

		/* Arrow -- XXX no dodging */
	case GF_ARROW:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");
			take_hit(dam, killer);
			break;
		}

		/* Plasma -- XXX No resist */
	case GF_PLASMA:
		{
			if (fuzzy) msg_print("You are hit by something *HOT*!");
			take_hit(dam, killer);

			if (!p_ptr->resist_sound)
			{
				int k = (randint((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}

			if (!(p_ptr->resist_fire ||
			                p_ptr->oppose_fire ||
			                p_ptr->immune_fire))
			{
				inven_damage(set_acid_destroy, 3);
			}

			break;
		}

		/* Nether -- drain experience */
	case GF_NETHER:
		{
			if (fuzzy) msg_print("You are hit by nether forces!");
			{
				if (p_ptr->immune_neth)
				{
					dam = 0;
				}
				else if (p_ptr->resist_neth)
				{
					dam *= 6;
					dam /= (randint(6) + 6);
				}
				else
				{
					if (p_ptr->hold_life && (rand_int(100) < 75))
					{
						msg_print("You keep hold of your life force!");
					}
					else if (p_ptr->hold_life)
					{
						msg_print("You feel your life slipping away!");
						lose_exp(200 + (p_ptr->exp / 1000) * MON_DRAIN_LIFE);
					}
					else
					{
						msg_print("You feel your life draining away!");
						lose_exp(200 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
					}
				}

				take_hit(dam, killer);
			}

			break;
		}

		/* Water -- stun/confuse */
	case GF_WAVE:
	case GF_WATER:
		{
			if (fuzzy) msg_print("You are hit by something wet!");
			if (!p_ptr->resist_sound)
			{
				set_stun(p_ptr->stun + randint(40));
			}
			if (!p_ptr->resist_conf)
			{
				set_confused(p_ptr->confused + randint(5) + 5);
			}

			if (randint(5) == 1)
			{
				inven_damage(set_cold_destroy, 3);
			}

			take_hit(dam, killer);
			break;
		}

		/* Chaos -- many effects */
	case GF_CHAOS:
		{
			if (fuzzy) msg_print("You are hit by a wave of anarchy!");
			if (p_ptr->resist_chaos)
			{
				dam *= 6;
				dam /= (randint(6) + 6);
			}
			if (!p_ptr->resist_conf)
			{
				(void)set_confused(p_ptr->confused + rand_int(20) + 10);
			}
			if (!p_ptr->resist_chaos)
			{
				(void)set_image(p_ptr->image + randint(10));
			}
			if (!p_ptr->resist_neth && !p_ptr->resist_chaos)
			{
				if (p_ptr->hold_life && (rand_int(100) < 75))
				{
					msg_print("You keep hold of your life force!");
				}
				else if (p_ptr->hold_life)
				{
					msg_print("You feel your life slipping away!");
					lose_exp(500 + (p_ptr->exp / 1000) * MON_DRAIN_LIFE);
				}
				else
				{
					msg_print("You feel your life draining away!");
					lose_exp(5000 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
				}
			}
			if ((!p_ptr->resist_chaos) || (randint(9) == 1))
			{
				inven_damage(set_elec_destroy, 2);
				inven_damage(set_fire_destroy, 2);
			}
			take_hit(dam, killer);
			break;
		}

		/* Shards -- mostly cutting */
	case GF_SHARDS:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");
			if (p_ptr->resist_shard)
			{
				dam *= 6;
				dam /= (randint(6) + 6);
			}
			else
			{
				(void)set_cut(p_ptr->cut + dam);
			}

			if ((!p_ptr->resist_shard) || (randint(13) == 1))
			{
				inven_damage(set_cold_destroy, 2);
			}

			take_hit(dam, killer);
			break;
		}

		/* Sound -- mostly stunning */
	case GF_SOUND:
		{
			if (fuzzy) msg_print("You are hit by a loud noise!");
			if (p_ptr->resist_sound)
			{
				dam *= 5;
				dam /= (randint(6) + 6);
			}
			else
			{
				int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}

			if ((!p_ptr->resist_sound) || (randint(13) == 1))
			{
				inven_damage(set_cold_destroy, 2);
			}

			take_hit(dam, killer);
			break;
		}

		/* Pure confusion */
	case GF_CONFUSION:
		{
			if (fuzzy) msg_print("You are hit by something puzzling!");
			if (p_ptr->resist_conf)
			{
				dam *= 5;
				dam /= (randint(6) + 6);
			}
			if (!p_ptr->resist_conf)
			{
				(void)set_confused(p_ptr->confused + randint(20) + 10);
			}
			take_hit(dam, killer);
			break;
		}

		/* Disenchantment -- see above */
	case GF_DISENCHANT:
		{
			if (fuzzy) msg_print("You are hit by something static!");
			if (p_ptr->resist_disen)
			{
				dam *= 6;
				dam /= (randint(6) + 6);
			}
			else
			{
				(void)apply_disenchant(0);
			}
			take_hit(dam, killer);
			break;
		}

		/* Nexus -- see above */
	case GF_NEXUS:
		{
			if (fuzzy) msg_print("You are hit by something strange!");
			if (p_ptr->resist_nexus)
			{
				dam *= 6;
				dam /= (randint(6) + 6);
			}
			else
			{
				apply_nexus(m_ptr);
			}
			take_hit(dam, killer);
			break;
		}

		/* Force -- mostly stun */
	case GF_FORCE:
		{
			if (fuzzy) msg_print("You are hit by kinetic force!");
			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(20));
				/*
				 * If fired by player, try pushing monster.
				 * First get vector from player to monster.
				 * x10 so we can use pseudo-fixed point maths.
				 *
				 * Really should use get_angle_to_grid (util.c)
				 */
				if (who > 0)
				{
					a = 0;
					b = 0;

					/* Get vector from firer to target */
					x1 = (p_ptr->px - m_ptr->fx) * 10;
					y1 = (p_ptr->py - m_ptr->fy) * 10;

					/* Make sure no zero divides */
					if (x1 == 0) x1 = 1;
					if (y1 == 0) y1 = 1;

					/* Select direction player is being pushed */

					/* Roughly horizontally */
					if ((2*y1) / x1 == 0)
					{
						if (x1 > 0)
						{
							a = 1, b = 0;
						}
						else
						{
							a = -1, b = 0;
						}
					}

					/* Roughly vertically */
					else if ((2*x1) / y1 == 0)
					{
						if (y1 > 0)
						{
							a = 0, b = 1;
						}
						else
						{
							a = 0, b = -1;
						}
					}

					/* Take diagonals */
					else
					{
						if (y1 > 0)
						{
							b = 1;
						}
						else
						{
							b = -1;
						}
						if (x1 > 0)
						{
							a = 1;
						}
						else
						{
							a = -1;
						}
					}

					/* Move monster 2 offsets back */
					do_move = 2;

					/* Old monster coords in x,y */
					y1 = p_ptr->py;
					x1 = p_ptr->px;
				}
			}
			else
				take_hit(dam, killer);
			break;
		}


		/* Rocket -- stun, cut */
	case GF_ROCKET:
		{
			if (fuzzy) msg_print("There is an explosion!");
			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(20));
			}
			if (p_ptr->resist_shard)
			{
				dam /= 2;
			}
			else
			{
				(void)set_cut(p_ptr-> cut + ( dam / 2) );
			}

			if ((!p_ptr->resist_shard) || (randint(12) == 1))
			{
				inven_damage(set_cold_destroy, 3);
			}

			take_hit(dam, killer);
			break;
		}

		/* Inertia -- slowness */
	case GF_INERTIA:
		{
			if (fuzzy) msg_print("You are hit by something slow!");
			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			take_hit(dam, killer);
			break;
		}

		/* Lite -- blinding */
	case GF_LITE:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if (p_ptr->resist_lite)
			{
				dam *= 4;
				dam /= (randint(6) + 6);
			}
			else if (!blind && !p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			if (p_ptr->sensible_lite)
			{
				msg_print("The light scorches your flesh!");
				dam *= 3;
			}
			take_hit(dam, killer);

			if (p_ptr->tim_wraith)
			{
				p_ptr->tim_wraith = 0;
				msg_print("The light forces you out of your incorporeal shadow form.");

				p_ptr->redraw |= PR_MAP;
				/* Update monsters */
				p_ptr->update |= (PU_MONSTERS);
				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD);
			}

			break;
		}

		/* Dark -- blinding */
	case GF_DARK:
		{
			if (fuzzy) msg_print("You are hit by something!");
			if (p_ptr->resist_dark)
			{
				dam *= 4;
				dam /= (randint(6) + 6);
			}
			else if (!blind && !p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			if (p_ptr->wraith_form) hp_player(dam);
			else take_hit(dam, killer);
			break;
		}

		/* Time -- bolt fewer effects XXX */
	case GF_TIME:
		{
			if (fuzzy) msg_print("You are hit by a blast from the past!");

			if (p_ptr->resist_continuum)
			{
				dam *= 4;
				dam /= (randint(6) + 6);
				msg_print("You feel as if time is passing you by.");
			}
			else
			{
				switch (randint(10))
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					{
						msg_print("You feel life has clocked back.");
						lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
						break;
					}

				case 6:
				case 7:
				case 8:
				case 9:
					{
						switch (randint(6))
						{
						case 1:
							k = A_STR;
							act = "strong";
							break;
						case 2:
							k = A_INT;
							act = "bright";
							break;
						case 3:
							k = A_WIS;
							act = "wise";
							break;
						case 4:
							k = A_DEX;
							act = "agile";
							break;
						case 5:
							k = A_CON;
							act = "hardy";
							break;
						case 6:
							k = A_CHR;
							act = "beautiful";
							break;
						}

						msg_format("You're not as %s as you used to be...", act);

						p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
						if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
						p_ptr->update |= (PU_BONUS);
						break;
					}

				case 10:
					{
						msg_print("You're not as powerful as you used to be...");

						for (k = 0; k < 6; k++)
						{
							p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
							if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
						}
						p_ptr->update |= (PU_BONUS);
						break;
					}
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Gravity -- stun plus slowness plus teleport */
	case GF_GRAVITY:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT) break; /* No teleport on special levels */
			if (fuzzy) msg_print("You are hit by something heavy!");
			msg_print("Gravity warps around you.");
			if (!unsafe)
			{
				teleport_player(5);
				if (!p_ptr->ffall)
					(void)set_slow(p_ptr->slow + rand_int(4) + 4);
				if (!(p_ptr->resist_sound || p_ptr->ffall))
				{
					int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
					(void)set_stun(p_ptr->stun + k);
				}
				if (p_ptr->ffall)
				{
					dam = (dam * 2) / 3;
				}

				if ((!p_ptr->ffall) || (randint(13) == 1))
				{
					inven_damage(set_cold_destroy, 2);
				}

				take_hit(dam, killer);
			}
			else
				teleport_player(dam);
			break;
		}

		/* Standard damage */
	case GF_DISINTEGRATE:
		{
			if (fuzzy) msg_print("You are hit by pure energy!");
			take_hit(dam, killer);
			break;
		}

	case GF_OLD_HEAL:
		{
			if (fuzzy) msg_print("You are hit by something invigorating!");
			(void)hp_player(dam);
			dam = 0;
			break;
		}

	case GF_OLD_SPEED:
		{
			if (fuzzy) msg_print("You are hit by something!");
			(void)set_fast(p_ptr->fast + randint(5), 10);
			dam = 0;
			break;
		}

	case GF_OLD_SLOW:
		{
			if (fuzzy) msg_print("You are hit by something slow!");
			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			break;
		}

	case GF_OLD_SLEEP:
		{
			if (p_ptr->free_act) break;
			if (fuzzy) msg_print("You fall asleep!");
			set_paralyzed(p_ptr->paralyzed + dam);
			dam = 0;
			break;
		}

		/* Pure damage */
	case GF_MANA:
		{
			if (fuzzy) msg_print("You are hit by an aura of magic!");
			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
	case GF_METEOR:
		{
			if (fuzzy) msg_print("Something falls from the sky on you!");
			take_hit(dam, killer);
			if ((!p_ptr->resist_shard) || (randint(13) == 1))
			{
				if (!p_ptr->immune_fire) inven_damage(set_fire_destroy, 2);
				inven_damage(set_cold_destroy, 2);
			}

			break;
		}

		/* Ice -- cold plus stun plus cuts */
	case GF_ICE:
		{
			if (fuzzy) msg_print("You are hit by something sharp and cold!");
			cold_dam(dam, killer);
			if (!p_ptr->resist_shard)
			{
				(void)set_cut(p_ptr->cut + damroll(5, 8));
			}
			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(15));
			}

			if ((!(p_ptr->resist_cold || p_ptr->oppose_cold)) || (randint(12) == 1))
			{
				if (!(p_ptr->immune_cold)) inven_damage(set_cold_destroy, 3);
			}

			break;
		}

		/* Knowledge */
	case GF_IDENTIFY:
		{
			if (fuzzy) msg_print("You are hit by pure knowledge!");
			self_knowledge(NULL);
			break;
		}

		/* Psi -- ESP */
	case GF_PSI:
		{
			if (fuzzy) msg_print("You are hit by pure psionic energy!");
			set_tim_esp(p_ptr->tim_esp + dam);
			break;
		}

		/* Statis -- paralyse */
	case GF_STASIS:
		{
			if (fuzzy) msg_print("You are hit by something paralyzing!");
			set_paralyzed(p_ptr->paralyzed + dam);
			break;
		}

		/* Raise Death -- restore life */
	case GF_RAISE:
		{
			if (fuzzy) msg_print("You are hit by pure anti-death energy!");
			restore_level();
			break;
		}

		/* Make Glyph -- Shield */
	case GF_MAKE_GLYPH:
		{
			if (fuzzy) msg_print("You are hit by pure protection!");
			set_shield(p_ptr->shield + dam, 50, 0, 0, 0);
			break;
		}

		/* Default */
	default:
		{
			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Handle moving the player.
	 *
	 * Note: This is a effect of force
	 */
	if (do_move)
	{
		int back = 0;

		back = 0;  /* Default of no movement */

		/* How far can we push the monster? */
		for (do_move = 1; do_move < 3; do_move++)
		{
			/* Get monster coords */
			/* And offset position */
			y1 = p_ptr->py + (b * do_move);
			x1 = p_ptr->px + (a * do_move);

			if (!in_bounds(y1, x1)) continue;

			/* Require "empty" floor space */
			if (!in_bounds(y1, x1) || !cave_empty_bold(y1, x1)) continue;

			/* amount moved */
			back = do_move;
		}

		/* Move the monster */
		if (back)
		{
			y1 = p_ptr->py + (b * back);
			x1 = p_ptr->px + (a * back);
			swap_position(y1, x1);

			if (back == 2)
			{
				msg_print("You are knocked back!");
			}
			if (back == 1)
			{
				msg_print("You are knocked back and crushed!");

				/* was kept from being pushed all the way, do extra dam */
				dam = dam * 13 / 10;
			}

			/* Get new position */
			y = y1;
			x = x1;
		}
		else /* could not move the monster */
		{
			msg_print("You are severely crushed!");

			/* Do extra damage (1/3)*/
			dam = dam * 15 / 10;
		}

		take_hit(dam, killer);

	}


	/* Disturb */
	disturb(1, 0);


	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (negative for "player")
 *        jk -- -2 for traps, only used with project_jump
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits something (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination.  This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff()" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool_ project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
	int i, t, dist;

	int y1, x1;
	int y2, x2;

	int dist_hack = 0;

	int y_saver, x_saver;  /* For reflecting monsters */

	int msec = delay_factor * delay_factor * delay_factor;

	/* Assume the player sees nothing */
	bool_ notice = FALSE;

	/* Assume the player has seen nothing */
	bool_ visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool_ drawn = FALSE;

	/* Is the player blind? */
	bool_ blind = (p_ptr->blind ? TRUE : FALSE);

	/* Number of grids in the "path" */
	int path_n = 0;

	/* Actual grids in the "path" */
	u16b path_g[1024];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	int effect = 0;

	/* Coordinates of the affected grids */
	byte gx[1024], gy[1024];

	/* Encoded "radius" info (see above) */
	byte gm[64];


	/* Hack -- Jump to target */
	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);
	}

	/* Start at player */
	else if ((who <= 0) && ((who != -101) && (who != -100)))
	{
		x1 = p_ptr->px;
		y1 = p_ptr->py;
	}

	/* Start at monster */
	else if (who > 0)
	{
		x1 = m_list[who].fx;
		y1 = m_list[who].fy;
	}

	/* Oops */
	else
	{
		x1 = x;
		y1 = y;
	}

	y_saver = y1;
	x_saver = x1;

	/* Default "destination" */
	y2 = y;
	x2 = x;


	/* Hack -- verify stuff */
	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}


	/* Hack -- Assume there will be no blast (max radius 16) */
	for (dist = 0; dist < 64; dist++) gm[dist] = 0;


	/* Initial grid */
	y = y1;
	x = x1;
	dist = 0;

	/* Collect beam grids */
	if (flg & (PROJECT_BEAM))
	{
		gy[grids] = y;
		gx[grids] = x;
		grids++;
	}


	/* Calculate the projection path */
	if ((who == -101) || (who == -100))
		path_n = 0;
	else
		path_n = project_path(path_g, MAX_RANGE, y1, x1, y2, x2, flg);


	/* Hack -- Handle stuff */
	handle_stuff();

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int oy = y;
		int ox = x;

		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Balls explode before reaching walls */
		if (!cave_floor_bold(ny, nx) && (rad > 0)) break;

		/* Advance */
		y = ny;
		x = nx;

		/* Collect beam grids */
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		/* Only do visuals if requested */
		if (!blind && !(flg & (PROJECT_HIDE)))
		{
			/* Only do visuals if the player can "see" the bolt */
			if (panel_contains(y, x) && player_has_los_bold(y, x))
			{
				u16b p;

				byte a;
				char c;

				/* Obtain the bolt pict */
				p = bolt_pict(oy, ox, y, x, typ);

				/* Extract attr/char */
				a = PICT_A(p);
				c = PICT_C(p);

				/* Visual effects */
				print_rel(c, a, y, x);
				move_cursor_relative(y, x);
				if (fresh_before) Term_fresh();
				Term_xtra(TERM_XTRA_DELAY, msec);
				lite_spot(y, x);
				if (fresh_before) Term_fresh();

				/* Display "beam" grids */
				if (flg & (PROJECT_BEAM))
				{
					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects */
					print_rel(c, a, y, x);
				}

				/* Hack -- Activate delay */
				visual = TRUE;
			}

			/* Hack -- delay anyway for consistency */
			else if (visual)
			{
				/* Delay for consistency */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}
	}


	/* Save the "blast epicenter" */
	y2 = y;
	x2 = x;

	/* Start the "explosion" */
	gm[0] = 0;

	/* Hack -- make sure beams get to "explode" */
	gm[1] = grids;

	dist_hack = dist;

	/* Explode */
	if (TRUE)
	{
		/* Hack -- remove final beam grid */
		if (flg & (PROJECT_BEAM))
		{
			grids--;
		}

		/* Determine the blast area, work from the inside out */
		for (dist = 0; dist <= rad; dist++)
		{
			/* Scan the maximal blast area of radius "dist" */
			for (y = y2 - dist; y <= y2 + dist; y++)
			{
				for (x = x2 - dist; x <= x2 + dist; x++)
				{
					/* Ignore "illegal" locations */
					if (!in_bounds(y, x)) continue;

					/* Enforce a "circular" explosion */
					if (distance(y2, x2, y, x) != dist) continue;

					/* Ball explosions are stopped by walls */
					if (typ == GF_DISINTEGRATE)
					{
						if (cave_valid_bold(y, x) &&
						                (cave[y][x].feat < FEAT_PATTERN_START
						                 || cave[y][x].feat > FEAT_PATTERN_XTRA2))
							cave_set_feat(y, x, FEAT_FLOOR);

						/* Update some things -- similar to GF_KILL_WALL */
						p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);
					}
					else
					{
						if (!los(y2, x2, y, x)) continue;
					}

					/* Save this grid */
					gy[grids] = y;
					gx[grids] = x;
					grids++;
				}
			}

			/* Encode some more "radius" info */
			gm[dist + 1] = grids;
		}
	}


	/* Speed -- ignore "non-explosions" */
	if (!grids) return (FALSE);


	/* Display the "blast area" if requested */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Then do the "blast", from inside out */
		for (t = 0; t <= rad; t++)
		{
			/* Dump everything with this radius */
			for (i = gm[t]; i < gm[t + 1]; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Only do visuals if the player can "see" the blast */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					u16b p;

					byte a;
					char c;

					drawn = TRUE;

					/* Obtain the explosion pict */
					p = bolt_pict(y, x, y, x, typ);

					/* Extract attr/char */
					a = PICT_A(p);
					c = PICT_C(p);

					/* Visual effects -- Display */
					print_rel(c, a, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush each "radius" seperately */
			if (fresh_before) Term_fresh();

			/* Delay (efficiently) */
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Flush the erasing */
		if (drawn)
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					lite_spot(y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			if (fresh_before) Term_fresh();
		}
	}


	/* Check features */
	if (flg & (PROJECT_GRID))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Effect ? */
		if (flg & PROJECT_STAY)
		{
			effect = new_effect(typ, dam, project_time, p_ptr->py, p_ptr->px, rad, project_time_effect);
			project_time = 0;
			project_time_effect = 0;
		}

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist + 1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the feature in that grid */
			if (project_f(who, dist, y, x, dam, typ)) notice = TRUE;

			/* Effect ? */
			if (flg & PROJECT_STAY)
			{
				cave[y][x].effect = effect;
				lite_spot(y, x);
			}
		}
	}


	/* Update stuff if needed */
	if (p_ptr->update) update_stuff();


	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist + 1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the object in the grid */
			if (project_o(who, dist, y, x, dam, typ)) notice = TRUE;
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist + 1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			if (grids > 1)
			{
				/* Affect the monster in the grid */
				if (project_m(who, dist, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				monster_race *ref_ptr = race_inf(&m_list[cave[y][x].m_idx]);

				if ((ref_ptr->flags2 & (RF2_REFLECTING)) && (randint(10) != 1)
				                && (dist_hack > 1))
				{
					int t_y, t_x;
					int max_attempts = 10;

					/* Choose 'new' target */
					do
					{
						t_y = y_saver - 1 + randint(3);
						t_x = x_saver - 1 + randint(3);
						max_attempts--;
					}

					while (max_attempts && in_bounds2(t_y, t_x) &&
					                !(los(y, x, t_y, t_x)));

					if (max_attempts < 1)
					{
						t_y = y_saver;
						t_x = x_saver;
					}

					if (m_list[cave[y][x].m_idx].ml)
					{
						msg_print("The attack bounces!");
						ref_ptr->r_flags2 |= RF2_REFLECTING;
					}

					project(cave[y][x].m_idx, 0, t_y, t_x, dam, typ, flg);
				}
				else
				{
					if (project_m(who, dist, y, x, dam, typ)) notice = TRUE;
				}
			}
		}

		/* Player affected one monster (without "jumping") */
		if ((who < 0) && (project_m_n == 1) && !(flg & (PROJECT_JUMP)))
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave[y][x].m_idx > 0)
			{
				monster_type *m_ptr = &m_list[cave[y][x].m_idx];

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(cave[y][x].m_idx);
			}
		}
	}


	/* Check player */
	if (flg & (PROJECT_KILL))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist + 1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the player */
			if (project_p(who, dist, y, x, dam, typ, rad)) notice = TRUE;
		}
	}

	/* Return "something was noticed" */
	return (notice);
}



/*
 * Potions "smash open" and cause an area effect when
 * (1) they are shattered while in the player's inventory,
 * due to cold (etc) attacks;
 * (2) they are thrown at a monster, or obstacle;
 * (3) they are shattered by a "cold ball" or other such spell
 * while lying on the floor.
 *
 * Arguments:
 *    who   ---  who caused the potion to shatter (0=player)
 *          potions that smash on the floor are assumed to
 *          be caused by no-one (who = 1), as are those that
 *          shatter inside the player inventory.
 *          (Not anymore -- I changed this; TY)
 *    y, x  --- coordinates of the potion (or player if
 *          the potion was in her inventory);
 *    o_ptr --- pointer to the potion object.
 */
bool_ potion_smash_effect(int who, int y, int x, int o_sval)
{
	int radius = 2;
	int dt = 0;
	int dam = 0;
	bool_ ident = FALSE;
	bool_ angry = FALSE;

	switch (o_sval)
	{
	case SV_POTION_SALT_WATER:
	case SV_POTION_SLIME_MOLD:
	case SV_POTION_LOSE_MEMORIES:
	case SV_POTION_DEC_STR:
	case SV_POTION_DEC_INT:
	case SV_POTION_DEC_WIS:
	case SV_POTION_DEC_DEX:
	case SV_POTION_DEC_CON:
	case SV_POTION_DEC_CHR:
	case SV_POTION_WATER:    /* perhaps a 'water' attack? */
	case SV_POTION_APPLE_JUICE:
		return TRUE;

	case SV_POTION_INFRAVISION:
	case SV_POTION_DETECT_INVIS:
	case SV_POTION_SLOW_POISON:
	case SV_POTION_CURE_POISON:
	case SV_POTION_BOLDNESS:
	case SV_POTION_RESIST_HEAT:
	case SV_POTION_RESIST_COLD:
	case SV_POTION_HEROISM:
	case SV_POTION_BESERK_STRENGTH:
	case SV_POTION_RESTORE_EXP:
	case SV_POTION_RES_STR:
	case SV_POTION_RES_INT:
	case SV_POTION_RES_WIS:
	case SV_POTION_RES_DEX:
	case SV_POTION_RES_CON:
	case SV_POTION_RES_CHR:
	case SV_POTION_INC_STR:
	case SV_POTION_INC_INT:
	case SV_POTION_INC_WIS:
	case SV_POTION_INC_DEX:
	case SV_POTION_INC_CON:
	case SV_POTION_INC_CHR:
	case SV_POTION_AUGMENTATION:
	case SV_POTION_ENLIGHTENMENT:
	case SV_POTION_STAR_ENLIGHTENMENT:
	case SV_POTION_SELF_KNOWLEDGE:
	case SV_POTION_EXPERIENCE:
	case SV_POTION_RESISTANCE:
	case SV_POTION_INVULNERABILITY:
	case SV_POTION_NEW_LIFE:
		/* All of the above potions have no effect when shattered */
		return FALSE;
	case SV_POTION_SLOWNESS:
		dt = GF_OLD_SLOW;
		dam = 5;
		ident = TRUE;
		angry = TRUE;
		break;
	case SV_POTION_POISON:
		dt = GF_POIS;
		dam = 3;
		ident = TRUE;
		angry = TRUE;
		break;
	case SV_POTION_BLINDNESS:
		dt = GF_DARK;
		ident = TRUE;
		angry = TRUE;
		break;
	case SV_POTION_CONFUSION:  /* Booze */
		dt = GF_OLD_CONF;
		ident = TRUE;
		angry = TRUE;
		break;
	case SV_POTION_SLEEP:
		dt = GF_OLD_SLEEP;
		angry = TRUE;
		ident = TRUE;
		break;
	case SV_POTION_RUINATION:
	case SV_POTION_DETONATIONS:
		dt = GF_SHARDS;
		dam = damroll(25, 25);
		angry = TRUE;
		ident = TRUE;
		break;
	case SV_POTION_DEATH:
		dt = GF_MANA;     /* !! */
		dam = damroll(10, 10);
		angry = TRUE;
		radius = 1;
		ident = TRUE;
		break;
	case SV_POTION_SPEED:
		dt = GF_OLD_SPEED;
		ident = TRUE;
		break;
	case SV_POTION_CURE_LIGHT:
		dt = GF_OLD_HEAL;
		dam = damroll(2, 3);
		ident = TRUE;
		break;
	case SV_POTION_CURE_SERIOUS:
		dt = GF_OLD_HEAL;
		dam = damroll(4, 3);
		ident = TRUE;
		break;
	case SV_POTION_CURE_CRITICAL:
	case SV_POTION_CURING:
		dt = GF_OLD_HEAL;
		dam = damroll(6, 3);
		ident = TRUE;
		break;
	case SV_POTION_HEALING:
		dt = GF_OLD_HEAL;
		dam = damroll(10, 10);
		ident = TRUE;
		break;
	case SV_POTION_STAR_HEALING:
	case SV_POTION_LIFE:
		dt = GF_OLD_HEAL;
		dam = damroll(50, 50);
		radius = 1;
		ident = TRUE;
		break;
	case SV_POTION_RESTORE_MANA:    /* MANA */
		dt = GF_MANA;
		dam = damroll(10, 10);
		radius = 1;
		ident = TRUE;
		break;
	default:
		/* Do nothing */
		;
	}

	(void) project(who, radius, y, x, dam, dt,
	               (PROJECT_JUMP | PROJECT_ITEM | PROJECT_KILL));

	/* XXX	those potions that explode need to become "known" */
	return angry;
}

/* This is for Thaumaturgy */
static const int destructive_attack_types[10] =
{
	GF_KILL_WALL,
	GF_KILL_WALL,
	GF_KILL_WALL,
	GF_STONE_WALL,
	GF_STONE_WALL,
	GF_STONE_WALL,
	GF_DESTRUCTION,
	GF_DESTRUCTION,
	GF_DESTRUCTION,
	GF_DESTRUCTION,
};

/* Also for Power-mages */
static const int attack_types[25] =
{
	GF_ARROW,
	GF_MISSILE,
	GF_MANA,
	GF_WATER,
	GF_PLASMA,
	GF_METEOR,
	GF_ICE,
	GF_GRAVITY,
	GF_INERTIA,
	GF_FORCE,
	GF_TIME,
	GF_ACID,
	GF_ELEC,
	GF_FIRE,
	GF_COLD,
	GF_POIS,
	GF_LITE,
	GF_DARK,
	GF_CONFUSION,
	GF_SOUND,
	GF_SHARDS,
	GF_NEXUS,
	GF_NETHER,
	GF_CHAOS,
	GF_DISENCHANT,
};

/*
 * Describe the attack using normal names.
 */

void describe_attack_fully(int type, char* r)
{
	switch (type)
	{
	case GF_ARROW:
		strcpy(r, "arrows");
		break;
	case GF_MISSILE:
		strcpy(r, "magic missiles");
		break;
	case GF_MANA:
		strcpy(r, "mana");
		break;
	case GF_LITE_WEAK:
		strcpy(r, "light");
		break;
	case GF_DARK_WEAK:
		strcpy(r, "dark");
		break;
	case GF_WATER:
		strcpy(r, "water");
		break;
	case GF_PLASMA:
		strcpy(r, "plasma");
		break;
	case GF_METEOR:
		strcpy(r, "meteors");
		break;
	case GF_ICE:
		strcpy(r, "ice");
		break;
	case GF_GRAVITY:
		strcpy(r, "gravity");
		break;
	case GF_INERTIA:
		strcpy(r, "inertia");
		break;
	case GF_FORCE:
		strcpy(r, "force");
		break;
	case GF_TIME:
		strcpy(r, "pure time");
		break;
	case GF_ACID:
		strcpy(r, "acid");
		break;
	case GF_ELEC:
		strcpy(r, "lightning");
		break;
	case GF_FIRE:
		strcpy(r, "flames");
		break;
	case GF_COLD:
		strcpy(r, "cold");
		break;
	case GF_POIS:
		strcpy(r, "poison");
		break;
	case GF_LITE:
		strcpy(r, "pure light");
		break;
	case GF_DARK:
		strcpy(r, "pure dark");
		break;
	case GF_CONFUSION:
		strcpy(r, "confusion");
		break;
	case GF_SOUND:
		strcpy(r, "sound");
		break;
	case GF_SHARDS:
		strcpy(r, "shards");
		break;
	case GF_NEXUS:
		strcpy(r, "nexus");
		break;
	case GF_NETHER:
		strcpy(r, "nether");
		break;
	case GF_CHAOS:
		strcpy(r, "chaos");
		break;
	case GF_DISENCHANT:
		strcpy(r, "disenchantment");
		break;
	case GF_KILL_WALL:
		strcpy(r, "wall destruction");
		break;
	case GF_KILL_DOOR:
		strcpy(r, "door destruction");
		break;
	case GF_KILL_TRAP:
		strcpy(r, "trap destruction");
		break;
	case GF_STONE_WALL:
		strcpy(r, "wall creation");
		break;
	case GF_MAKE_DOOR:
		strcpy(r, "door creation");
		break;
	case GF_MAKE_TRAP:
		strcpy(r, "trap creation");
		break;
	case GF_DESTRUCTION:
		strcpy(r, "destruction");
		break;

	default:
		strcpy(r, "something unknown");
		break;
	}
}

/*
 * Give a randomly-generated spell a name.
 * Note that it only describes the first effect!
 */

static void name_spell(random_spell* s_ptr)
{
	char buff[30];
	cptr buff2 = "???";

	if (s_ptr->proj_flags & PROJECT_STOP && s_ptr->radius == 0)
	{
		buff2 = "Bolt";
	}
	else if (s_ptr->proj_flags & PROJECT_BEAM)
	{
		buff2 = "Beam";
	}
	else if (s_ptr->proj_flags & PROJECT_STOP && s_ptr->radius > 0)
	{
		buff2 = "Ball";
	}
	else if (s_ptr->proj_flags & PROJECT_BLAST)
	{
		buff2 = "Blast";
	}
	else if (s_ptr->proj_flags & PROJECT_METEOR_SHOWER)
	{
		buff2 = "Area";
	}
	else if (s_ptr->proj_flags & PROJECT_VIEWABLE)
	{
		buff2 = "View";
	}

	describe_attack_fully(s_ptr->GF, buff);
	strnfmt(s_ptr->name, 30, "%s - %s", buff2, buff);
}

void generate_spell(int plev)
{
	random_spell* rspell;
	int dice, sides, chance, mana, power;
	bool_ destruc_gen = FALSE;
	bool_ simple_gen = TRUE;
	bool_ ball_desc = FALSE;

	if (spell_num == MAX_SPELLS) return;

	rspell = &random_spells[spell_num];

	power = rand_int(5);

	dice = plev / 2;
	sides = plev;
	mana = plev;

	/* Make the spell more or less powerful. */
	dice += power;
	sides += power;
	mana += (plev * power) / 15;

	/* Stay within reasonable bounds. */
	if (dice < 1) dice = 1;

	if (sides < 5) sides = 5;

	if (mana < 1) mana = 1;

	rspell->level = plev;
	rspell->mana = mana;
	rspell->untried = TRUE;

	/* Spells are always maximally destructive. */
	rspell->proj_flags = PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID;

	chance = randint(100);

	/* Hack -- Always start with Magic Missile or derivative at lev. 1 */
	if (plev == 1 || chance < 25)
	{
		rspell->proj_flags |= PROJECT_STOP;
                /* Swap dice and sides for better damage */
		rspell->dam_dice = sides;
		rspell->dam_sides = dice;
		rspell->radius = 0;
	}
	else if (chance < 50)
	{
		rspell->proj_flags |= PROJECT_BEAM;
		rspell->dam_dice = dice;
		rspell->dam_sides = sides;
		rspell->radius = 0;
	}
	else if (chance < 76)
	{
		rspell->proj_flags |= PROJECT_STOP;
		rspell->radius = dice / 3;
		rspell->dam_dice = dice;
		rspell->dam_sides = sides;
		ball_desc = TRUE;
	}
	else if (chance < 83)
	{
		rspell->proj_flags |= PROJECT_BLAST;
		rspell->radius = sides / 3;
		rspell->dam_dice = dice;
		rspell->dam_sides = sides;

		destruc_gen = TRUE;
		simple_gen = FALSE;
	}
	else if (chance < 90)
	{
		rspell->proj_flags |= PROJECT_METEOR_SHOWER;
                /* Area effect spells do way less damage "per shot" */
		rspell->dam_dice = dice / 5;
		rspell->dam_sides = sides / 5;
		rspell->radius = sides / 3;
		if (rspell->radius < 4) rspell->radius = 4;

		destruc_gen = TRUE;
	}
	else
	{
		rspell->proj_flags |= PROJECT_VIEWABLE;
                /* View spells do less damage */
		rspell->dam_dice = dice;
		rspell->dam_sides = sides / 2;
	}

	/* Both a destructive and a simple spell requested --
	 * pick one or the other. */
	if (destruc_gen && simple_gen)
	{
		if (magik(25))
		{
			simple_gen = FALSE;
		}
		else
		{
			destruc_gen = FALSE;
		}
	}

	/* Pick a simple spell */
	if (simple_gen)
	{
		rspell->GF = attack_types[rand_int(25)];
	}
	/* Pick a destructive spell */
	else
	{
		rspell->GF = destructive_attack_types[rand_int(10)];
	}

	/* Give the spell a name. */
	name_spell(rspell);
	if (ball_desc)
	{
		/* 30 character limit on the string! */
		sprintf(rspell->desc, "Dam: %d, Rad: %d, Pow: %d",
			sides, dice, power);
	}
	else
	{
		sprintf(rspell->desc, "Damage: %dd%d, Power: %d",
			dice, sides, power);
	}

	spell_num++;
}

/*
 * Polymorph a monster at given location.
 */
s16b do_poly_monster(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	monster_type *m_ptr;

	s16b hack_m_idx;
	s16b old_m_idx;
	s16b new_m_idx = 0;

	s16b new_r_idx;

	/* Get a "old" monster */
	old_m_idx = c_ptr->m_idx;

	/* Giga-Hack -- Remember monster */
	hack_m_idx = old_m_idx;

	/* Get a monster */
	m_ptr = &m_list[c_ptr->m_idx];

	/* Pick a "new" monster race */
	new_r_idx = poly_r_idx(m_ptr->r_idx);

	/* No polymorph happend */
	if (new_r_idx == m_ptr->r_idx) return 0;

	/* Giga-Hack -- Removes the moster XXX XXX XXX XXX */
	c_ptr->m_idx = 0;

	/*
	 * Handle polymorph --
	 * Create a new monster (no groups)
	 */
	if (place_monster_aux(y, x, new_r_idx, FALSE, FALSE, m_ptr->status))
	{
		/* Get a "new" monster */
		new_m_idx = c_ptr->m_idx;

		/* Giga-Hack -- Remember "new" monster */
		hack_m_idx = new_m_idx;

		/* "Kill" the "old" monster */
		delete_monster_idx(old_m_idx);

		p_ptr->redraw |= (PR_MAP);
	}

	/* Giga-Hack -- restore saved monster XXX XXX XXX */
	c_ptr->m_idx = hack_m_idx;

	return new_m_idx;
}
