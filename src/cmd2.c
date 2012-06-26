/* File: cmd2.c */

/* Purpose: Movement commands (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

void do_cmd_immovable_special(void);

/*
 * Try to bash an altar
 */
static bool_ do_cmd_bash_altar(int y, int x)
{
	msg_print("Are you mad? You want to anger the gods?");
	return (FALSE);
}


/*
 * Try to bash a fountain
 */
static bool_ do_cmd_bash_fountain(int y, int x)
{
	int bash, temp;

	bool_ more = TRUE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_BASH_DOOR))
	{
		msg_print("You cannot do that.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Message */
	msg_print("You smash into the fountain!");

	/* Hack -- Bash power based on strength */
	/* (Ranges from 3 to 20 to 100 to 200) */
	bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

	/* Compare bash power to door power XXX XXX XXX */
	temp = (bash - 50);

	/* Hack -- always have a chance */
	if (temp < 1) temp = 1;

	/* Hack -- attempt to bash down the door */
	if (rand_int(200) < temp)
	{
		/* Message */
		msg_print("The fountain breaks!");

		fire_ball(GF_WATER, 5, damroll(6, 8), 2);

		cave_set_feat(y, x, FEAT_DEEP_WATER);
		more = FALSE;
	}

	return (more);
}

/*
 * Stair hooks
 */
static bool_ stair_hooks(stairs_direction direction)
{
	cptr direction_s = (direction == STAIRS_UP) ? "up" : "down";

	/* Old-style hooks */
	if (process_hooks(HOOK_STAIR, "(s)", direction_s))
	{
		return TRUE; /* Prevent movement */
	}

	/* New-style hooks */
	{
		hook_stair_in in = { direction };
		hook_stair_out out = { TRUE }; /* Allow by default */

		process_hooks_new(HOOK_STAIR, &in, &out);

		return (!out.allow);
	}
}


/*
 * Go up one level
 */
void do_cmd_go_up(void)
{
	bool_ go_up = FALSE, go_up_many = FALSE, prob_traveling = FALSE;

	cave_type *c_ptr;

	int oldl = dun_level;

	dungeon_info_type *d_ptr = &d_info[dungeon_type];


	/* Player grid */
	c_ptr = &cave[p_ptr->py][p_ptr->px];

	/* Can we ? */
	if (stair_hooks(STAIRS_UP))
	{
		return;
	}

	/* Normal up stairs */
	if ((c_ptr->feat == FEAT_LESS) || (c_ptr->feat == FEAT_WAY_LESS))
	{
		if (!dun_level)
		{
			go_up = TRUE;
		}
		else if ((dungeon_flags2 & DF2_ASK_LEAVE))
		{
			go_up = get_check("Leave this unique level forever? ");
		}
		else if (confirm_stairs)
		{
			go_up = get_check("Really leave the level? ");
		}
		else
		{
			go_up = TRUE;
		}
	}

	/* Shaft up */
	else if (c_ptr->feat == FEAT_SHAFT_UP)
	{
		if (dun_level == 1)
		{
			go_up = TRUE;
		}
		else if ((dungeon_flags2 & DF2_ASK_LEAVE))
		{
			go_up = get_check("Leave this unique level forever? ");
		}
		else if (confirm_stairs)
		{
			go_up_many = get_check("Really leave the level? ");
		}
		else
		{
			go_up_many = TRUE;
		}
	}

	/* Quest exit */
	else if (c_ptr->feat == FEAT_QUEST_EXIT)
	{
		leaving_quest = p_ptr->inside_quest;

		if ((dungeon_flags2 & DF2_ASK_LEAVE) &&
				!get_check("Leave this unique level forever? "))
			return;

		p_ptr->inside_quest = c_ptr->special;
		dun_level = 0;
		p_ptr->oldpx = 0;
		p_ptr->oldpy = 0;
		p_ptr->leaving = TRUE;

		return;
	}

	/* Exits to previous area in flat terrains */
	else if (!(dungeon_flags1 & DF1_FLAT) &&
	                p_ptr->prob_travel && !p_ptr->inside_quest)
	{
		if (d_ptr->mindepth == dun_level) return;

		if (dungeon_flags2 & DF2_NO_EASY_MOVE)
		{
			msg_print("Some powerful force prevents your from teleporting.");
			return;
		}

		prob_traveling = TRUE;

		if (confirm_stairs)
		{
			if (get_check("Really leave the level? "))
				go_up = TRUE;
		}
		else
		{
			go_up = TRUE;
		}
	}
	else
	{
		msg_print("I see no up staircase here.");
		return;
	}

	if (go_up || go_up_many)
	{

		energy_use = 0;

		/* Success */
		if (c_ptr->feat == FEAT_WAY_LESS)
			msg_print("You enter the previous area.");
		else
			msg_print("You enter a maze of up staircases.");

		autosave_checkpoint();

		if (p_ptr->inside_quest)
		{
			dun_level = 1;
			leaving_quest = p_ptr->inside_quest;

			p_ptr->inside_quest = c_ptr->special;
		}

		/* Create a way back */
		if (go_up_many)
			create_down_shaft = TRUE;
		else
			create_down_stair = TRUE;

		/* New depth */
		if (go_up)
			dun_level--;
		else
		{
			dun_level -= randint(3) + 1;
			if (dun_level <= 0) dun_level = 0;
		}

		if (c_ptr->special && (!prob_traveling))
		{
			dun_level = oldl;
			dun_level = get_flevel();
			dungeon_type = c_ptr->special;
			dun_level += d_info[dungeon_type].mindepth;
		}

		/* Leaving */
		p_ptr->leaving = TRUE;
	}
}


/*
 * Returns TRUE if we are in the Between...
 */
static bool_ between_effect(void)
{
	byte bx, by;


	if (cave[p_ptr->py][p_ptr->px].feat == FEAT_BETWEEN)
	{

		bx = cave[p_ptr->py][p_ptr->px].special & 255;
		by = cave[p_ptr->py][p_ptr->px].special >> 8;

		msg_print("You fall into the void.");
		msg_print("Brrrr! It's deadly cold.");

		swap_position(by, bx);

		/* To avoid being teleported back */
		energy_use = 100;

		return (TRUE);
	}

	else if (cave[p_ptr->py][p_ptr->px].feat == FEAT_BETWEEN2)
	{
		between_exit *be_ptr = &between_exits[cave[p_ptr->py][p_ptr->px].special];

		p_ptr->wild_mode = FALSE;
		p_ptr->wilderness_x = be_ptr->wild_x;
		p_ptr->wilderness_y = be_ptr->wild_y;
		p_ptr->oldpx = p_ptr->px = be_ptr->px;
		p_ptr->oldpy = p_ptr->py = be_ptr->py;
		dungeon_type = be_ptr->d_idx;
		dun_level = be_ptr->level;
		p_ptr->leaving = TRUE;

		return (TRUE);
	}
	else
		return (FALSE);
}

/*
 * Go down one level
 */
void do_cmd_go_down(void)
{
	cave_type *c_ptr;

	bool_ go_down = FALSE, go_down_many = FALSE, prob_traveling = FALSE;

	bool_ fall_trap = FALSE;

	char i;

	int old_dun = dun_level;

	dungeon_info_type *d_ptr = &d_info[dungeon_type];


	/*  MUST be actived now */
	if (between_effect()) return;

	/* Player grid */
	c_ptr = &cave[p_ptr->py][p_ptr->px];

	if (p_ptr->astral && (dun_level == 98)) return;

	if (c_ptr->t_idx == TRAP_OF_SINKING) fall_trap = TRUE;

	/* test if on special level */
	if ((dungeon_flags2 & DF2_ASK_LEAVE))
	{
		prt("Leave this unique level forever (y/n) ? ", 0, 0);
		flush();
		i = inkey();
		prt("", 0, 0);
		if (i != 'y') return;
	}

	/* Can we ? */
	if (stair_hooks(STAIRS_DOWN))
	{
		return;
	}

	/* Normal up stairs */
	if (c_ptr->feat == FEAT_SHAFT_DOWN)
	{
		if (!dun_level)
		{
			go_down = TRUE;

			/* Save old player position */
			p_ptr->oldpx = p_ptr->px;
			p_ptr->oldpy = p_ptr->py;
		}
		else
		{
			if (confirm_stairs)
			{
				if (get_check("Really leave the level? "))
					go_down_many = TRUE;
			}
			else
			{
				go_down_many = TRUE;
			}
		}
	}

	/* Normal stairs */
	else if ((c_ptr->feat == FEAT_MORE) || (c_ptr->feat == FEAT_WAY_MORE))
	{
		if (p_ptr->prob_travel)
		{
			if (d_ptr->maxdepth == dun_level) return;
		}
		if (!dun_level)
		{
			go_down = TRUE;

			/* Save old player position */
			p_ptr->oldpx = p_ptr->px;
			p_ptr->oldpy = p_ptr->py;
		}
		else
		{
			if (confirm_stairs)
			{
				if (get_check("Really leave the level? "))
					go_down = TRUE;
			}
			else
			{
				go_down = TRUE;
			}
		}
	}

	/* Handle quest areas -KMW- */
	else if (c_ptr->feat == FEAT_QUEST_ENTER)
	{
		/* Enter quest level */
		enter_quest();

		return;
	}

	else if (!(dungeon_flags1 & DF1_FLAT) &&
	                p_ptr->prob_travel && !p_ptr->inside_quest)
	{
		if (d_ptr->maxdepth == dun_level) return;

		if (dungeon_flags2 & DF2_NO_EASY_MOVE)
		{
			msg_print("Some powerfull force prevents your from teleporting.");
			return;
		}

		prob_traveling = TRUE;

		if (confirm_stairs)
		{
			if (get_check("Really leave the level? "))
				go_down = TRUE;
		}
		else
		{
			go_down = TRUE;
		}
	}

	else if (!(fall_trap))
	{
		msg_print("I see no down staircase here.");
		return;
	}

	if (go_down || go_down_many)
	{
		energy_use = 0;

		if (fall_trap)
			msg_print("You deliberately jump through the trap door.");
		else
		{
			if (c_ptr->feat == FEAT_WAY_MORE)
				msg_print("You enter the next area.");
			else
				msg_print("You enter a maze of down staircases.");
		}

		autosave_checkpoint();

		/* Go down */
		if (go_down)
		{
			dun_level++;
		}
		else if (go_down_many)
		{
			int i = randint(3) + 1, j;

			for (j = 1; j < i; j++)
			{
				dun_level++;
				if (is_quest(dun_level + i - 1)) break;
				if (d_ptr->maxdepth == dun_level) break;
			}
		}

		/* We change place */
		if (c_ptr->special && (!prob_traveling))
		{
			if (d_info[c_ptr->special].min_plev <= p_ptr->lev)
			{
				dungeon_info_type *d_ptr = &d_info[c_ptr->special];

				/* Do the lua scripts refuse ? ;) */
				if (process_hooks(HOOK_ENTER_DUNGEON, "(d)", c_ptr->special))
				{
					dun_level = old_dun;
					return;
				}

				/* Ok go in the new dungeon */
				dungeon_type = c_ptr->special;
				d_ptr = &d_info[dungeon_type];

				if ((p_ptr->wilderness_x == d_ptr->ix) &&
				                (p_ptr->wilderness_y == d_ptr->iy))
				{
					dun_level = d_ptr->mindepth;
				}
				else if ((p_ptr->wilderness_x == d_ptr->ox) &&
				                (p_ptr->wilderness_y == d_ptr->oy))
				{
					dun_level = d_ptr->maxdepth;
				}
				else
				{
					dun_level = d_ptr->mindepth;
				}

				msg_format("You go into %s",
				           d_text + d_info[dungeon_type].text);
			}
			else
			{
				msg_print
				("You don't feel yourself experienced enough to go there...");
				dun_level = old_dun;
				return;
			}
		}

		/* Leaving */
		p_ptr->leaving = TRUE;

		if (!fall_trap)
		{
			/* Create a way back */
			if (go_down_many)
				create_up_shaft = TRUE;
			else
				create_up_stair = TRUE;
		}
	}
}



/*
 * Simple command to "search" for one turn
 */
void do_cmd_search(void)
{
	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Take a turn */
	energy_use = 100;

	/* Search */
	search();
}


/*
 * Hack -- toggle search mode
 */
void do_cmd_toggle_search(void)
{
	/* Stop searching */
	if (p_ptr->searching)
	{
		/* Clear the searching flag */
		p_ptr->searching = FALSE;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
	}

	/* Start searching */
	else
	{
		/* Set the searching flag */
		p_ptr->searching = TRUE;

		/* Update stuff */
		p_ptr->update |= (PU_BONUS);

		/* Redraw stuff */
		p_ptr->redraw |= (PR_STATE | PR_SPEED);
	}
}



/*
 * Determine if a grid contains a chest
 */
static s16b chest_check(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;


	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Skip unknown chests XXX XXX */
		/* if (!o_ptr->marked) continue; */

		/* Check for chest */
		if (o_ptr->tval == TV_CHEST) return (this_o_idx);
	}

	/* No chest */
	return (0);
}


/*
 * Allocates objects upon opening a chest    -BEN-
 *
 * Disperse treasures from the given chest, centered at (x,y).
 *
 * Small chests often contain "gold", while Large chests always contain
 * items.  Wooden chests contain 2 items, Iron chests contain 4 items,
 * and Steel chests contain 6 items.  The "value" of the items in a
 * chest is based on the "power" of the chest, which is in turn based
 * on the level on which the chest is generated.
 */
static void chest_death(int y, int x, s16b o_idx)
{
	int number;

	bool_ small;

	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr = &o_list[o_idx];


	/* Small chests often hold "gold" */
	small = (o_ptr->sval < SV_CHEST_MIN_LARGE);

	/* Determine how much to drop (see above) */
	number = (o_ptr->sval % SV_CHEST_MIN_LARGE) * 2;

	/* Zero pval means empty chest */
	if (!o_ptr->pval) number = 0;

	/* Opening a chest */
	opening_chest = TRUE;

	/* Determine the "value" of the items */
	object_level = ABS(o_ptr->pval) + 10;

	/* Drop some objects (non-chests) */
	for (; number > 0; --number)
	{
		/* Get local object */
		q_ptr = &forge;

		/* Wipe the object */
		object_wipe(q_ptr);

		/* Small chests often drop gold */
		if (small && (rand_int(100) < 75))
		{
			/* Make some gold */
			if (!make_gold(q_ptr)) continue;
		}

		/* Otherwise drop an item */
		else
		{
			/* Make an object */
			if (!make_object(q_ptr, FALSE, FALSE, d_info[dungeon_type].objs))
				continue;
		}

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, y, x);
	}

	/* Reset the object level */
	object_level = dun_level;

	/* No longer opening a chest */
	opening_chest = FALSE;

	/* Empty */
	o_ptr->pval = 0;
	o_ptr->pval2 = 0;

	/* Known */
	object_known(o_ptr);
}


/*
 * Chests have traps too.
 *
 * Exploding chest destroys contents (and traps).
 * Note that the chest itself is never destroyed.
 */
static void chest_trap(int y, int x, s16b o_idx)
{
	int trap;

	object_type *o_ptr = &o_list[o_idx];

	bool_ ident = FALSE;


	/* Ignore disarmed chests */
	if (o_ptr->pval <= 0) return;

	/* Obtain the trap */
	trap = o_ptr->pval;

	/* Message */
	msg_print("You found a trap!");

	/* Set off trap */
	ident = player_activate_trap_type(y, x, o_ptr, o_idx);
	if (ident)
	{
		t_info[o_ptr->pval].ident = TRUE;
		msg_format("You identified the trap as %s.",
		           t_name + t_info[trap].name);
	}
}


/*
 * Attempt to open the given chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool_ do_cmd_open_chest(int y, int x, s16b o_idx)
{
	int i, j;

	bool_ flag = TRUE;

	bool_ more = FALSE;

	object_type *o_ptr = &o_list[o_idx];

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_OPEN_DOOR))
	{
		msg_print("You cannot open chests.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Attempt to unlock it */
	if (o_ptr->pval > 0)
	{
		/* Assume locked, and thus not open */
		flag = FALSE;

		/* Get the "disarm" factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite()) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the difficulty */
		j = i - o_ptr->pval;

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success -- May still have traps */
		if (rand_int(100) < j)
		{
			msg_print("You have picked the lock.");
			gain_exp(1);
			flag = TRUE;
		}

		/* Failure -- Keep trying */
		else
		{
			/* We may continue repeating */
			more = TRUE;

			if (flush_failure) flush();

			msg_print("You failed to pick the lock.");
		}
	}

	/* Allowed to open */
	if (flag)
	{
		/* Apply chest traps, if any */
		chest_trap(y, x, o_idx);

		/* Let the Chest drop items */
		chest_death(y, x, o_idx);
	}

	/* Result */
	return (more);
}


/*
 * Original code by TNB, improvement for Angband 2.9.3 by rr9
 * Slightly modified for ToME because of its trap implementation
 */

/*
 * Return TRUE if the given grid is an open door
 */
static bool_ is_open(cave_type *c_ptr)
{
	return (c_ptr->feat == FEAT_OPEN);
}


/*
 * Return TRUE if the given grid is a closed door
 */
static bool_ is_closed(cave_type *c_ptr)
{
	byte feat;

	if (c_ptr->mimic) feat = c_ptr->mimic;
	else feat = c_ptr->feat;

	return ((feat >= FEAT_DOOR_HEAD) && (feat <= FEAT_DOOR_TAIL));
}


/*
 * Return TRUE if the given grid has a trap
 */
static bool_ is_trap(cave_type *c_ptr)
{
	return ((c_ptr->info & (CAVE_TRDT)) != 0);
}


/*
 * Return the number of doors/traps around (or under)
 * the character using the filter function 'test'
 */
static int count_feats(int *y, int *x, bool_ (*test) (cave_type *c_ptr),
                       bool_ under)
{
	int d;

	int xx, yy;

	int count;


	/* Clear match counter */
	count = 0;

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{
		/* Ignore current grid if told so -- See tables.c */
		if ((d == 8) && !under) continue;

		/* Extract adjacent (legal) location */
		yy = p_ptr->py + ddy_ddd[d];
		xx = p_ptr->px + ddx_ddd[d];

		/* Paranoia */
		if (!in_bounds(yy, xx)) continue;

		/* Must have knowledge */
		if (!(cave[yy][xx].info & (CAVE_MARK))) continue;

		/* Not looking for this feature */
		if (!(*test) (&cave[yy][xx])) continue;

		/* Count it */
		count++;

		/* Remember the location. Only meaningful if there's
		   exactly one match */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return (count);
}


/*
 * Return the number of chests around (or under) the character.
 * If requested, count only trapped chests.
 */
static int count_chests(int *y, int *x, bool_ trapped)
{
	int d, count, o_idx;

	object_type *o_ptr;


	/* Count how many matches */
	count = 0;

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++)
	{

		/* Extract adjacent (legal) location */
		int yy = p_ptr->py + ddy_ddd[d];
		int xx = p_ptr->px + ddx_ddd[d];

		/* No (visible) chest is there */
		if ((o_idx = chest_check(yy, xx)) == 0) continue;

		/* Grab the object */
		o_ptr = &o_list[o_idx];

		/* Already open */
		if (o_ptr->pval == 0) continue;

		/* No (known) traps here */
		if (trapped && (!object_known_p(o_ptr) || !o_ptr->pval)) continue;

		/* OK */
		++count;

		/* Remember the location. Only useful if only one match */
		*y = yy;
		*x = xx;
	}

	/* All done */
	return (count);
}


/*
 * Convert an adjacent location to a direction.
 */
static int coords_to_dir(int y, int x)
{
	int d[3][3] =
	        {
	                {7, 4, 1},
	                {8, 5, 2},
	                {9, 6, 3} };

	int dy, dx;


	dy = y - p_ptr->py;
	dx = x - p_ptr->px;

	/* Paranoia */
	if (ABS(dx) > 1 || ABS(dy) > 1) return (0);

	return d[dx + 1][dy + 1];
}


/*
 * Perform the basic "open" command on doors
 *
 * Assume destination is a closed/locked/jammed door
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool_ do_cmd_open_aux(int y, int x, int dir)
{
	int i, j;

	cave_type *c_ptr;

	bool_ more = FALSE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_OPEN_DOOR))
	{
		msg_print("You cannot open doors.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Get requested grid */
	c_ptr = &cave[y][x];

	/* Jammed door */
	if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x08)
	{
		/* Stuck */
		msg_print("The door appears to be stuck.");
	}

	/* Locked door */
	else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x01)
	{
		/* Disarm factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite()) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the lock power */
		j = c_ptr->feat - FEAT_DOOR_HEAD;

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (rand_int(100) < j)
		{
			/* Message */
			msg_print("You have picked the lock.");

			/* Set off trap */
			if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

			/* Open the door */
			cave_set_feat(y, x, FEAT_OPEN);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			/* Sound */
			sound(SOUND_OPENDOOR);

			/* Experience */
			gain_exp(1);
		}

		/* Failure */
		else
		{
			/* Failure */
			if (flush_failure) flush();

			/* Message */
			msg_print("You failed to pick the lock.");

			/* We may keep trying */
			more = TRUE;
		}
	}

	/* Closed door */
	else
	{
		/* Set off trap */
		if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

		/* Open the door */
		cave_set_feat(y, x, FEAT_OPEN);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

		/* Sound */
		sound(SOUND_OPENDOOR);
	}

	/* Result */
	return (more);
}



/*
 * Open a closed/locked/jammed door or a closed/locked chest.
 *
 * Unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open(void)
{
	int y, x, dir;

	s16b o_idx;

	cave_type *c_ptr;

	bool_ more = FALSE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_OPEN_DOOR))
	{
		msg_print("You cannot open doors.");

		return;
	}

	/* Option: Pick a direction */
	if (easy_open)
	{
		int num_doors, num_chests;

		/* Count closed doors (locked or jammed) */
		num_doors = count_feats(&y, &x, is_closed, FALSE);

		/* Count chests (locked) */
		num_chests = count_chests(&y, &x, FALSE);

		/* There is nothing the player can open */
		if ((num_doors + num_chests) == 0)
		{
			/* Message */
			msg_print("You see nothing there to open.");

			/* Done */
			return;
		}

		/* Set direction if there is only one target */
		else if ((num_doors + num_chests) == 1)
		{
			command_dir = coords_to_dir(y, x);
		}
	}

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Get requested location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get requested grid */
		c_ptr = &cave[y][x];

		/* Check for chest */
		o_idx = chest_check(y, x);

		/* Nothing useful */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL)) && !o_idx)
		{
			/* Message */
			msg_print("You see nothing there to open.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx)
		{
			/* Take a turn */
			energy_use = 100;

			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		/* Handle chests */
		else if (o_idx)
		{
			/* Open the chest */
			more = do_cmd_open_chest(y, x, o_idx);
		}

		/* Handle doors */
		else
		{
			/* Open the door */
			more = do_cmd_open_aux(y, x, dir);
		}
	}

	/* Process the appropriate hooks */
	process_hooks(HOOK_OPEN, "(d)", is_quest(dun_level));

	/* Cancel repeat unless we may continue */
	if (!more) disturb(0, 0);
}



/*
 * Perform the basic "close" command
 *
 * Assume destination is an open/broken door
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool_ do_cmd_close_aux(int y, int x, int dir)
{
	cave_type *c_ptr;

	bool_ more = FALSE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_OPEN_DOOR))
	{
		msg_print("You cannot close doors.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Get grid and contents */
	c_ptr = &cave[y][x];

	/* Set off trap */
	if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

	/* Broken door */
	if (c_ptr->feat == FEAT_BROKEN)
	{
		/* Message */
		msg_print("The door appears to be broken.");
	}

	/* Open door */
	else
	{
		/* Close the door */
		cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

		/* Sound */
		sound(SOUND_SHUTDOOR);
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close(void)
{
	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;


	/* Option: Pick a direction */
	if (easy_open)
	{
		int num_doors;

		/* Count open doors */
		num_doors = count_feats(&y, &x, is_open, FALSE);

		/* There are no doors the player can close */
		if (num_doors == 0)
		{
			/* Message */
			msg_print("You see nothing there to close.");

			/* Done */
			return;
		}

		/* Exactly one closeable door */
		else if (num_doors == 1)
		{
			command_dir = coords_to_dir(y, x);
		}
	}

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Get requested location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[y][x];

		/* Require open/broken door */
		if ((c_ptr->feat != FEAT_OPEN) && (c_ptr->feat != FEAT_BROKEN))
		{
			/* Message */
			msg_print("You see nothing there to close.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx)
		{
			/* Take a turn */
			energy_use = 100;

			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		/* Close the door */
		else
		{
			/* Close the door */
			more = do_cmd_close_aux(y, x, dir);
		}
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(0, 0);
}


/*
 * Determine if a given grid may be "tunneled"
 */
static bool_ do_cmd_tunnel_test(int y, int x)
{
	/* Must have knowledge(execpt on "forget" levels) */
	if (!(cave[y][x].info & (CAVE_MARK)))
	{
		/* Message */
		msg_print("You see nothing there.");

		/* Nope */
		return (FALSE);
	}

	/* Must be a wall/door/etc */
	if (cave_floor_bold(y, x))
	{
		/* Message */
		msg_print("You see nothing there to tunnel.");

		/* Nope */
		return (FALSE);
	}

	/* Must be tunnelable */
	if (!(f_info[cave[y][x].feat].flags1 & FF1_TUNNELABLE))
	{
		/* Message */
		msg_print(f_text + f_info[cave[y][x].feat].tunnel);

		/* Nope */
		return (FALSE);
	}

	/* Okay */
	return (TRUE);
}



/*
 * Tunnel through wall.  Assumes valid location.
 *
 * Note that it is impossible to "extend" rooms past their
 * outer walls (which are actually part of the room).
 *
 * This will, however, produce grids which are NOT illuminated
 * (or darkened) along with the rest of the room.
 */
static bool_ twall(int y, int x, byte feat)
{
	cave_type *c_ptr = &cave[y][x];


	/* Paranoia -- Require a wall or door or some such */
	if (cave_floor_bold(y, x)) return (FALSE);

	/* Forget the wall */
	c_ptr->info &= ~(CAVE_MARK);

	/* Remove the feature */
	cave_set_feat(y, x, feat);

	/* Update some things */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);

	/* Result */
	return (TRUE);
}



/*
 * Perform the basic "tunnel" command
 *
 * Assumes that the destination is a wall, a vein, a secret
 * door, or rubble.
 *
 * Assumes that no monster is blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
bool_ do_cmd_tunnel_aux(int y, int x, int dir)
{
	int skill_req = 0, skill_req_1pct = 0;
	cave_type *c_ptr = &cave[y][x];

	feature_type *f_ptr = &f_info[c_ptr->feat];

	bool_ more = FALSE;


	/* Must be have something to dig with (except for sandwalls) */
	if ((c_ptr->feat < FEAT_SANDWALL) || (c_ptr->feat > FEAT_SANDWALL_K))
	{
		if (!p_ptr->inventory[INVEN_TOOL].k_idx ||
		                (p_ptr->inventory[INVEN_TOOL].tval != TV_DIGGING))
		{
			msg_print("You need to have a shovel or pick in your tool slot.");

			return (FALSE);
		}
	}

	/* Verify legality */
	if (!do_cmd_tunnel_test(y, x)) return (FALSE);

	/* Take a turn */
	energy_use = 100;

	/* Get grid */
	c_ptr = &cave[y][x];

	/* Sound */
	sound(SOUND_DIG);

	/* Titanium */
	if (f_ptr->flags1 & FF1_PERMANENT)
	{
		msg_print(f_text + f_ptr->tunnel);
	}

	else if ((c_ptr->feat == FEAT_TREES) || (c_ptr->feat == FEAT_DEAD_TREE))
	{
		/* Chop Down */
		skill_req = 10;
		skill_req_1pct = 14;
		if ((p_ptr->skill_dig > 10 + rand_int(400)) && twall(y, x, FEAT_GRASS))
		{
			msg_print("You have cleared away the trees.");
		}

		/* Keep trying */
		else
		{
			/* We may continue chopping */
			msg_print(f_text + f_ptr->tunnel);
			more = TRUE;

			/* Occasional Search XXX XXX */
			if (rand_int(100) < 25) search();
		}
	}


	/* Granite */
	else if ((c_ptr->feat >= FEAT_WALL_EXTRA) &&
	                (c_ptr->feat <= FEAT_WALL_SOLID))
	{
		/* Tunnel */
		skill_req = 40;
		skill_req_1pct = 56;
		if ((p_ptr->skill_dig > 40 + rand_int(1600)) && twall(y, x, FEAT_FLOOR))
		{
			msg_print("You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg_print(f_text + f_ptr->tunnel);
			more = TRUE;
		}
	}


	/* Quartz / Magma / Sandwall */
	else if (((c_ptr->feat >= FEAT_MAGMA) &&
	                (c_ptr->feat <= FEAT_QUARTZ_K)) ||
	                ((c_ptr->feat >= FEAT_SANDWALL) &&
	                 (c_ptr->feat <= FEAT_SANDWALL_K)))
	{
		bool_ okay = FALSE;
		bool_ gold = FALSE;
		bool_ hard = FALSE;
		bool_ soft = FALSE;

		/* Found gold */
		if ((c_ptr->feat >= FEAT_MAGMA_H) &&
		                (c_ptr->feat <= FEAT_QUARTZ_K)) gold = TRUE;

		if ((c_ptr->feat == FEAT_SANDWALL_H) ||
		                (c_ptr->feat == FEAT_SANDWALL_K))
		{
			gold = TRUE;
			soft = TRUE;
		}
		else
			/* Extract "quartz" flag XXX XXX XXX */
			if ((c_ptr->feat - FEAT_MAGMA) & 0x01) hard = TRUE;

		/* Quartz */
		if (hard)
		{
			skill_req = 20;
			skill_req_1pct = 28;
			okay = (p_ptr->skill_dig > 20 + rand_int(800));
		}

		/* Sandwall */
		else if (soft)
		{
			skill_req = 5;
			skill_req_1pct = 8;
			okay = (p_ptr->skill_dig > 5 + rand_int(250));
		}

		/* Magma */
		else
		{
			skill_req = 10;
			skill_req_1pct = 14;
			okay = (p_ptr->skill_dig > 10 + rand_int(400));
		}

		/* Success */
		if (okay && twall(y, x, FEAT_FLOOR))
		{
			/* Found treasure */
			if (gold)
			{
				/* Place some gold */
				place_gold(y, x);

				/* Message */
				msg_print("You have found something!");
			}

			/* Found nothing */
			else
			{
				/* Message */
				msg_print("You have finished the tunnel.");
			}
		}

		/* Failure */
		else
		{
			/* Message, continue digging */
			msg_print(f_text + f_ptr->tunnel);
			more = TRUE;
		}
	}

	/* Rubble */
	else if (c_ptr->feat == FEAT_RUBBLE)
	{
		/* Remove the rubble */
		skill_req = 0;
		skill_req_1pct = 2;
		if ((p_ptr->skill_dig > rand_int(200)) &&
		                twall(y, x, d_info[dungeon_type].floor1))
		{
			/* Message */
			msg_print("You have removed the rubble.");

			/* Hack -- place an object */
			if (rand_int(100) < 10)
			{
				/* Create a simple object */
				place_object(y, x, FALSE, FALSE, OBJ_FOUND_RUBBLE);

				/* Observe new object */
				if (player_can_see_bold(y, x))
				{
					msg_print("You have found something!");
				}
			}
		}

		else
		{
			/* Message, keep digging */
			msg_print(f_text + f_ptr->tunnel);
			more = TRUE;
		}
	}

	/* Secret doors */
	else if (c_ptr->feat >= FEAT_SECRET)
	{
		/* Tunnel */
		skill_req = 30;
		skill_req_1pct = 42;
		if ((p_ptr->skill_dig > 30 + rand_int(1200)) && twall(y, x, FEAT_FLOOR))
		{
			msg_print("You have finished the tunnel.");
			c_ptr->mimic = 0;
			lite_spot(y, x);

			/* Set off trap */
			if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);
		}

		/* Keep trying */
		else
		{
			int feat;

			if (c_ptr->mimic) feat = c_ptr->mimic;
			else
				feat = c_ptr->feat;

			/* We may continue tunelling */
			msg_print(f_text + f_info[feat].tunnel);
			more = TRUE;

			/* Occasional Search XXX XXX */
			if (rand_int(100) < 25) search();
		}
	}

	/* Doors */
	else
	{
		/* Tunnel */
		skill_req = 30;
		skill_req_1pct = 42;
		if ((p_ptr->skill_dig > 30 + rand_int(1200)) && twall(y, x, FEAT_FLOOR))
		{
			msg_print("You have finished the tunnel.");
		}

		/* Keep trying */
		else
		{
			/* We may continue tunelling */
			msg_print(f_text + f_ptr->tunnel);
			more = TRUE;
		}
	}

	if (more && magik(2))
	{
		if (p_ptr->skill_dig < skill_req)
		{
			msg_print("You fail to make even the slightest of progress.");
			more = FALSE;
		}
		else if (p_ptr->skill_dig < skill_req_1pct)
		{
			msg_print("This will take some time.");
		}
	}

	/* Notice new floor grids */
	if (!cave_floor_bold(y, x))
	{
		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MONSTERS | PU_MON_LITE);
	}

	/* Result */
	return (more);
}


/*
 * Tunnels through "walls" (including rubble and closed doors)
 *
 * Note that you must tunnel in order to hit invisible monsters
 * in walls, though moving into walls still takes a turn anyway.
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 */
void do_cmd_tunnel(void)
{
	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;


	if (p_ptr->wild_mode) return;

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a direction to tunnel, or Abort */
	if (get_rep_dir(&dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid */
		c_ptr = &cave[y][x];

		/* No tunnelling through doors */
		if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL)) || (c_ptr->feat == FEAT_SHOP))
		{
			/* Message */
			msg_print("You cannot tunnel through doors.");
		}

		/* No tunnelling through air */
		else if (cave_floor_grid(c_ptr))
		{
			/* Message */
			msg_print("You cannot tunnel through air.");
		}

		/* A monster is in the way */
		else if (c_ptr->m_idx)
		{
			/* Take a turn */
			energy_use = 100;

			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		/* Try digging */
		else
		{
			/* Tunnel through walls */
			more = do_cmd_tunnel_aux(y, x, dir);
		}
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(0, 0);
}


/*
 * easy_open_door --
 *
 *	If there is a jammed/closed/locked door at the given location,
 *	then attempt to unlock/open it. Return TRUE if an attempt was
 *	made (successful or not), otherwise return FALSE.
 *
 *	The code here should be nearly identical to that in
 *	do_cmd_open_test() and do_cmd_open_aux().
 */

bool_ easy_open_door(int y, int x)
{
	int i, j;

	cave_type *c_ptr = &cave[y][x];

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_OPEN_DOOR))
	{
		msg_print("You cannot open doors.");

		return (FALSE);
	}

	/* Must be a closed door */
	if (!((c_ptr->feat >= FEAT_DOOR_HEAD) && (c_ptr->feat <= FEAT_DOOR_TAIL)))
	{
		/* Nope */
		return (FALSE);
	}

	/* Jammed door */
	if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x08)
	{
		/* Stuck */
		msg_print("The door appears to be stuck.");
	}

	/* Locked door */
	else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x01)
	{
		/* Disarm factor */
		i = p_ptr->skill_dis;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite()) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the lock power */
		j = c_ptr->feat - FEAT_DOOR_HEAD;

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (rand_int(100) < j)
		{
			/* Message */
			msg_print("You have picked the lock.");

			/* Set off trap */
			if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

			/* Open the door */
			cave_set_feat(y, x, FEAT_OPEN);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			/* Sound */
			sound(SOUND_OPENDOOR);

			/* Process the appropriate hooks */
			process_hooks(HOOK_OPEN, "(d)", is_quest(dun_level));

			/* Experience */
			gain_exp(1);
		}

		/* Failure */
		else
		{
			/* Failure */
			if (flush_failure) flush();

			/* Message */
			msg_print("You failed to pick the lock.");
		}
	}

	/* Closed door */
	else
	{
		/* Set off trap */
		if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

		/* Open the door */
		cave_set_feat(y, x, FEAT_OPEN);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

		/* Sound */
		sound(SOUND_OPENDOOR);
	}

	/* Result */
	return (TRUE);
}


/*
 * Perform the basic "disarm" command
 *
 * Assume destination is a visible trap
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool_ do_cmd_disarm_chest(int y, int x, s16b o_idx)
{
	int i, j;

	bool_ more = FALSE;

	object_type *o_ptr = &o_list[o_idx];

	trap_type *t_ptr = &t_info[o_ptr->pval];


	/* Take a turn */
	energy_use = 100;

	/* Get the "disarm" factor */
	i = p_ptr->skill_dis;

	/* Penalize some conditions */
	if (p_ptr->blind || no_lite()) i = i / 10;
	if (p_ptr->confused || p_ptr->image) i = i / 10;

	/* Extract the difficulty */
	j = i - t_ptr->difficulty * 3;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Must find the trap first. */
	if (!object_known_p(o_ptr))
	{
		msg_print("I don't see any traps.");
	}

	/* Already disarmed/unlocked */
	else if (o_ptr->pval <= 0)
	{
		msg_print("The chest is not trapped.");
	}

	/* Success (get a lot of experience) */
	else if (rand_int(100) < j)
	{
		msg_print("You have disarmed the chest.");
		gain_exp(t_ptr->difficulty * 3);
		o_ptr->pval = (0 - o_ptr->pval);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint(i) > 5))
	{
		/* We may keep trying */
		more = TRUE;
		if (flush_failure) flush();
		msg_print("You failed to disarm the chest.");
	}

	/* Failure -- Set off the trap */
	else
	{
		msg_print("You set off a trap!");
		sound(SOUND_FAIL);
		chest_trap(y, x, o_idx);
	}

	/* Result */
	return (more);
}


/*
 * Perform the basic "disarm" command
 *
 * Assume destination is a visible trap
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
bool_ do_cmd_disarm_aux(int y, int x, int dir, int do_pickup)
{
	int i, j, power;

	cave_type *c_ptr;

	cptr name;

	bool_ more = FALSE;


	/* Take a turn */
	energy_use = 100;

	/* Get grid and contents */
	c_ptr = &cave[y][x];

	/* Access trap name */
	if (t_info[c_ptr->t_idx].ident)
		name = (t_name + t_info[c_ptr->t_idx].name);
	else
		name = "unknown trap";

	/* Get the "disarm" factor */
	i = p_ptr->skill_dis;

	/* Penalize some conditions */
	if (p_ptr->blind || no_lite()) i = i / 10;
	if (p_ptr->confused || p_ptr->image) i = i / 10;

	/* XXX XXX XXX Variable power? */

	/* Extract trap "power" */
	power = t_info[c_ptr->t_idx].difficulty;

	/* Extract the difficulty */
	j = i - power;

	/* Always have a small chance of success */
	if (j < 2) j = 2;

	/* Success */
	if (rand_int(100) < j)
	{
		/* Message */
		msg_format("You have disarmed the %s.", name);

		/* Reward */
		gain_exp(power);

		/* Forget the trap */
		c_ptr->info &= ~(CAVE_MARK | CAVE_TRDT);

		/* Remove the trap */
		c_ptr->t_idx = 0;

		/* Move the player onto the trap */
		if (!(f_info[c_ptr->feat].flags1 & FF1_DOOR))
			move_player_aux(dir, do_pickup, 0, TRUE);

		/* Remove trap attr from grid */
		note_spot(y, x);
		lite_spot(y, x);
	}

	/* Failure -- Keep trying */
	else if ((i > 5) && (randint(i) > 5))
	{
		/* Failure */
		if (flush_failure) flush();

		/* Message */
		msg_format("You failed to disarm the %s.", name);

		/* We may keep trying */
		more = TRUE;
	}

	/* Failure -- Set off the trap */
	else
	{
		/* Message */
		msg_format("You set off the %s!", name);

		/* Move the player onto the trap */
		if (!(f_info[c_ptr->feat].flags1 & FF1_DOOR))
			move_player_aux(dir, do_pickup, 0, FALSE);
	}

	/* Result */
	return (more);
}


/*
 * Disamrs the monster traps(no failure)
 */
void do_cmd_disarm_mon_trap(int y, int x)
{
	msg_print("You disarm the monster trap.");

	place_floor_convert_glass(y, x);
	cave[p_ptr->py][p_ptr->px].special = cave[p_ptr->py][p_ptr->px].special2 = 0;
}


/*
 * Disarms a trap, or chest
 */
void do_cmd_disarm(void)
{
	int y, x, dir;

	s16b o_idx;

	cave_type *c_ptr;

	bool_ more = FALSE;


	/* Option: Pick a direction */
	if (easy_disarm)
	{
		int num_traps, num_chests;

		/* Count visible traps */
		num_traps = count_feats(&y, &x, is_trap, TRUE);

		/* Count chests (trapped) */
		num_chests = count_chests(&y, &x, TRUE);

		/* See if only one target */
		if (num_traps || num_chests)
		{
			if (num_traps + num_chests <= 1)
				command_dir = coords_to_dir(y, x);
		}
	}

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a direction (or abort) */
	if (get_rep_dir(&dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[y][x];

		/* Check for chests */
		o_idx = chest_check(y, x);

		/* Disarm a trap */
		if (((c_ptr->t_idx == 0) || (!(c_ptr->info & CAVE_TRDT))) &&
		                !o_idx && (c_ptr->feat != FEAT_MON_TRAP))
		{
			/* Message */
			msg_print("You see nothing there to disarm.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx)
		{
			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		/* Disarm chest */
		else if (o_idx)
		{
			/* Disarm the chest */
			more = do_cmd_disarm_chest(y, x, o_idx);
		}

		/* Disarm trap */
		else
		{
			/* Disarm the trap */
			if (c_ptr->feat == FEAT_MON_TRAP)
			{
				do_cmd_disarm_mon_trap(y, x);
				more = FALSE;
			}
			else
				more = do_cmd_disarm_aux(y, x, dir, always_pickup);
		}
	}

	/* Cancel repeat unless told not to */
	if (!more) disturb(0, 0);
}


/*
 * Perform the basic "bash" command
 *
 * Assume destination is a closed/locked/jammed door
 *
 * Assume there is no monster blocking the destination
 *
 * Returns TRUE if repeated commands may continue
 */
static bool_ do_cmd_bash_aux(int y, int x, int dir)
{
	int bash, temp;

	cave_type *c_ptr;

	bool_ more = FALSE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_BASH_DOOR))
	{
		msg_print("You cannot do that.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Get grid */
	c_ptr = &cave[y][x];

	/* Message */
	msg_print("You smash into the door!");

	/* Hack -- Bash power based on strength */
	/* (Ranges from 3 to 20 to 100 to 200) */
	bash = adj_str_blow[p_ptr->stat_ind[A_STR]];

	/* Extract door power */
	temp = ((c_ptr->feat - FEAT_DOOR_HEAD) & 0x07);

	/* Compare bash power to door power XXX XXX XXX */
	temp = (bash - (temp * 10));

	/* Hack -- always have a chance */
	if (temp < 1) temp = 1;

	/* Hack -- attempt to bash down the door */
	if (rand_int(100) < temp)
	{
		/* Message */
		msg_print("The door crashes open!");

		/* Break down the door */
		if (rand_int(100) < 50)
		{
			/* Set off trap */
			if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

			cave_set_feat(y, x, FEAT_BROKEN);
		}

		/* Open the door */
		else
		{
			/* Set off trap */
			if (c_ptr->t_idx != 0) player_activate_door_trap(y, x);

			cave_set_feat(y, x, FEAT_OPEN);
		}

		/* Sound */
		sound(SOUND_OPENDOOR);

		/* Hack -- Fall through the door. Can't disarm while falling. */
		move_player_aux(dir, always_pickup, 0, FALSE);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MON_LITE);
		p_ptr->update |= (PU_DISTANCE);
	}

	/* Saving throw against stun */
	else if (rand_int(100) < adj_dex_safe[p_ptr->stat_ind[A_DEX]] + p_ptr->lev)
	{
		/* Message */
		msg_print("The door holds firm.");

		/* Allow repeated bashing */
		more = TRUE;
	}

	/* High dexterity yields coolness */
	else
	{
		/* Message */
		msg_print("You are off-balance.");

		/* Hack -- Lose balance ala paralysis */
		(void)set_paralyzed(p_ptr->paralyzed + 2 + rand_int(2));
	}

	/* Result */
	return (more);
}


/*
 * Bash open a door, success based on character strength
 *
 * For a closed door, pval is positive if locked; negative if stuck.
 *
 * For an open door, pval is positive for a broken door.
 *
 * A closed door can be opened - harder if locked. Any door might be
 * bashed open (and thereby broken). Bashing a door is (potentially)
 * faster! You move into the door way. To open a stuck door, it must
 * be bashed. A closed door can be jammed (see do_cmd_spike()).
 *
 * Creatures can also open or bash doors, see elsewhere.
 */
void do_cmd_bash(void)
{
	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;

	monster_race *r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags2 & RF2_BASH_DOOR))
	{
		msg_print("You cannot do that.");

		return;
	}

	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Bash location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid */
		c_ptr = &cave[y][x];

		/* Nothing useful */
		if ((c_ptr->feat < FEAT_DOOR_HEAD ||
		                c_ptr->feat > FEAT_DOOR_TAIL) &&
		                (c_ptr->feat < FEAT_ALTAR_HEAD ||
		                 c_ptr->feat > FEAT_ALTAR_TAIL) && (c_ptr->feat != FEAT_FOUNTAIN))
		{
			/* Message */
			msg_print("You see nothing there to bash.");
		}

		/* Monster in the way */
		else if (c_ptr->m_idx)
		{
			/* Take a turn */
			energy_use = 100;

			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		else if (c_ptr->feat >= FEAT_ALTAR_HEAD &&
		                c_ptr->feat <= FEAT_ALTAR_TAIL)
		{
			more = do_cmd_bash_altar(y, x);
		}
		/* Bash a closed door */
		else if (c_ptr->feat == FEAT_FOUNTAIN)
		{
			more = do_cmd_bash_fountain(y, x);
		}
		else
		{
			/* Bash the door */
			more = do_cmd_bash_aux(y, x, dir);
		}
	}

	/* Unless valid action taken, cancel bash */
	if (!more) disturb(0, 0);
}



/*
 * Manipulate an adjacent grid in some way
 *
 * Attack monsters, tunnel through walls, disarm traps, open doors.
 *
 * Consider confusion XXX XXX XXX
 *
 * This command must always take a turn, to prevent free detection
 * of invisible monsters.
 */
void do_cmd_alter(void)
{
	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a direction */
	if (get_rep_dir(&dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid */
		c_ptr = &cave[y][x];

		/* Take a turn */
		energy_use = 100;

		/* Attack monsters */
		if (c_ptr->m_idx)
		{
			/* Attack */
			py_attack(y, x, -1);
		}

		/* Open closed doors */
		else if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL))
		{
			/* Tunnel */
			more = do_cmd_open_aux(y, x, dir);
		}

		/* Tunnel through walls */
		else if (f_info[c_ptr->feat].flags1 & FF1_TUNNELABLE)
		{
			/* Tunnel */
			more = do_cmd_tunnel_aux(y, x, dir);
		}

		/* Disarm traps */
		else if (c_ptr->t_idx != 0)
		{
			/* Tunnel */
			more = do_cmd_disarm_aux(y, x, dir, always_pickup);
		}

		/* Oops */
		else
		{
			/* Oops */
			msg_print("You attack the empty air.");
		}
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(0, 0);
}


/*
 * Find the index of some "spikes", if possible.
 *
 * XXX XXX XXX Let user choose a pile of spikes, perhaps?
 */
static bool_ get_spike(int *ip)
{
	int i;


	/* Check every item in the pack */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Check the "tval" code */
		if (o_ptr->tval == TV_SPIKE)
		{
			/* Save the spike index */
			(*ip) = i;

			/* Success */
			return (TRUE);
		}
	}

	/* Oops */
	return (FALSE);
}



/*
 * Jam a closed door with a spike
 *
 * This command may NOT be repeated
 */
void do_cmd_spike(void)
{
	int y, x, dir, item;

	cave_type *c_ptr;


	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Get location */
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];

		/* Get grid and contents */
		c_ptr = &cave[y][x];

		/* Require closed door */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		                (c_ptr->feat <= FEAT_DOOR_TAIL)))
		{
			/* Message */
			msg_print("You see nothing there to spike.");
		}

		/* Get a spike */
		else if (!get_spike(&item))
		{
			/* Message */
			msg_print("You have no spikes!");
		}

		/* Is a monster in the way? */
		else if (c_ptr->m_idx)
		{
			/* Take a turn */
			energy_use = 100;

			/* Message */
			msg_print("There is a monster in the way!");

			/* Attack */
			py_attack(y, x, -1);
		}

		/* Go for it */
		else
		{
			/* Take a turn */
			energy_use = 100;

			/* Successful jamming */
			msg_print("You jam the door with a spike.");

			/* Convert "locked" to "stuck" XXX XXX XXX */
			if (c_ptr->feat < FEAT_DOOR_HEAD + 0x08) c_ptr->feat += 0x08;

			/* Add one spike to the door */
			if (c_ptr->feat < FEAT_DOOR_TAIL) c_ptr->feat++;

			/* Use up, and describe, a single spike, from the bottom */
			inc_stack_size(item, -1);
		}
	}
}


static void do_cmd_walk_jump(int pickup, bool_ disarm)
{
	int dir;

	bool_ more = FALSE;


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Take a turn */
		energy_use = 100;

		/* Actually move the character */
		move_player(dir, pickup, disarm);

		/* Allow more walking */
		more = TRUE;
	}

	/* Hack -- In small scale wilderness it takes MUCH more time to move */
	energy_use *= (p_ptr->wild_mode) ? ((MAX_HGT + MAX_WID) / 2) : 1;

	/* Hack again -- Is there a special encounter ??? */
	if (p_ptr->wild_mode &&
	                magik(wf_info[wild_map[p_ptr->py][p_ptr->px].feat].level - (p_ptr->lev * 2)))
	{
		/* Go into large wilderness view */
		p_ptr->wilderness_x = p_ptr->px;
		p_ptr->wilderness_y = p_ptr->py;
		energy_use = 100;
		change_wild_mode();

		/* HACk -- set the encouter flag for the wilderness generation */
		generate_encounter = TRUE;
		p_ptr->oldpx = MAX_WID / 2;
		p_ptr->oldpy = MAX_HGT / 2;

		/* Inform the player of his horrible fate :=) */
		msg_print("You are ambushed!");
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb(0, 0);
}


/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup, bool_ disarm)
{
	/* Move (usually pickup) */

	if (p_ptr->immovable)
	{
		do_cmd_unwalk();
	}
	else
	{
		do_cmd_walk_jump(pickup, disarm);
	}
}


void do_cmd_run_run()
{
	int dir;


	/* Hack -- no running when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Hack -- Set the run counter */
		running = (command_arg ? command_arg : 1000);

		/* First step */
		run_step(dir);
	}
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * Start running.
 */
void do_cmd_run(void)
{
	if (p_ptr->immovable)
	{
		return;
	}
	else
	{
		do_cmd_run_run();
	}
}



/*
 * Stay still.  Search.  Enter stores.
 * Pick up treasure if "pickup" is true.
 */
void do_cmd_stay(int pickup)
{
	cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}


	/* Take a turn */
	energy_use = 100;


	/* Spontaneous Searching */
	if ((p_ptr->skill_fos >= 50) || (0 == rand_int(50 - p_ptr->skill_fos)))
	{
		search();
	}

	/* Continuous Searching */
	if (p_ptr->searching)
	{
		search();
	}


	/* Handle "objects" */
	carry(pickup);


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
 * Resting allows a player to safely restore his hp	-RAK-
 */
void do_cmd_rest(void)
{
	/* Can't rest on a Void Jumpgate -- too dangerous */
	if (cave[p_ptr->py][p_ptr->px].feat == FEAT_BETWEEN)
	{
		/* 'R&\n' is one of our favourite macros, so we have to do this */
		if (flush_failure) flush();

		/* Tell the player why */
		msg_print(format("Resting on a %s is too dangerous!",
		                 f_name + f_info[cave[p_ptr->py][p_ptr->px].feat].name));

		/* Done */
		return;
	}

	/* Can't rest while undead, it would mean dying */
	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		/* 'R&\n' is one of our favourite macros, so we have to do this */
		if (flush_failure) flush();

		/* Tell the player why */
		msg_print("Resting is impossible while undead!");

		/* Done */
		return;
	}

	/* Prompt for time if needed */
	if (command_arg <= 0)
	{
		cptr p = "Rest (0-9999, '*' for HP/SP, '&' as needed): ";

		char out_val[80];

		/* Default */
		strcpy(out_val, "&");

		/* Ask for duration */
		if (!get_string(p, out_val, 4)) return;

		/* Rest until done */
		if (out_val[0] == '&')
		{
			command_arg = ( -2);
		}

		/* Rest a lot */
		else if (out_val[0] == '*')
		{
			command_arg = ( -1);
		}

		/* Rest some */
		else
		{
			command_arg = atoi(out_val);
			if (command_arg <= 0) return;
		}
	}


	/* Paranoia */
	if (command_arg > 9999) command_arg = 9999;


	/* Take a turn XXX XXX XXX (?) */
	energy_use = 100;

	/* Save the rest code */
	resting = command_arg;

	/* Cancel searching */
	p_ptr->searching = FALSE;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff();

	/* Refresh */
	Term_fresh();
}






/*
 * Determines the odds of an object breaking when thrown at a monster
 *
 * Note that artifacts never break, see the "drop_near()" function.
 */
int breakage_chance(object_type *o_ptr)
{
	int reducer =
	        1 + ((get_skill(SKILL_ARCHERY)) ? (get_skill_scale(SKILL_ARCHERY, 10)) : 0);

	/* Examine the item type */
	switch (o_ptr->tval)
	{
		/* Always break */
	case TV_FLASK:
	case TV_POTION:
	case TV_POTION2:
	case TV_BOTTLE:
	case TV_FOOD:
		{
			return (100);
		}

		/* Often break */
	case TV_LITE:
	case TV_SCROLL:
	case TV_SKELETON:
		{
			return (50);
		}

	case TV_ARROW:
		{
			return (50 / reducer);
		}

		/* Sometimes break */
	case TV_WAND:
	case TV_SPIKE:
		{
			return (25);
		}

	case TV_SHOT:
	case TV_BOLT:
		{
			return (25 / reducer);
		}
	case TV_BOOMERANG:
		{
			return 1;
		}
	}

	/* Rarely break */
	return (10);
}

/*
 * Return multiplier of an object
 */
int get_shooter_mult(object_type *o_ptr)
{
	/* Assume a base multiplier */
	int tmul = 1;

	/* Analyze the launcher */
	switch (o_ptr->sval)
	{
	case SV_SLING:
		{
			/* Sling and ammo */
			tmul = 2;
			break;
		}

	case SV_SHORT_BOW:
		{
			/* Short Bow and Arrow */
			tmul = 2;
			break;
		}

	case SV_LONG_BOW:
		{
			/* Long Bow and Arrow */
			tmul = 3;
			break;
		}

		/* Light Crossbow and Bolt */
	case SV_LIGHT_XBOW:
		{
			tmul = 3;
			break;
		}

		/* Heavy Crossbow and Bolt */
	case SV_HEAVY_XBOW:
		{
			tmul = 4;
			break;
		}
	}
	return tmul;
}


/*
 * Fire an object from the pack or floor.
 *
 * You may only fire items that "match" your missile launcher.
 *
 * You must use slings + pebbles/shots, bows + arrows, xbows + bolts.
 *
 * See "calc_bonuses()" for more calculations and such.
 *
 * Note that "firing" a missile is MUCH better than "throwing" it.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Objects are more likely to break if they "attempt" to hit a monster.
 *
 * Rangers (with Bows) and Anyone (with "Extra Shots") get extra shots.
 *
 * The "extra shot" code works by decreasing the amount of energy
 * required to make each shot, spreading the shots out over time.
 *
 * Note that when firing missiles, the launcher multiplier is applied
 * after all the bonuses are added in, making multipliers very useful.
 *
 * Note that Bows of "Extra Might" get extra range and an extra bonus
 * for the damage multiplier.
 *
 * Note that Bows of "Extra Shots" give an extra shot.
 */
void do_cmd_fire(void)
{
	int dir, item;

	int j, y, x, ny, nx, ty, tx, by, bx;

	int oldtdam, tdam, tdis, thits, tmul;

	int bonus, chance;

	int cur_dis, visible;

	int breakage = -1, num_pierce = 0;

	s32b special = 0;

	object_type forge;

	object_type *q_ptr;

	object_type *o_ptr;

	object_type *j_ptr;

	bool_ hit_body = FALSE;

	byte missile_attr;

	char missile_char;

	char o_name[80];

	cptr q, s;

	int msec = delay_factor * delay_factor * delay_factor;


	/* Get the "bow" (if any) */
	j_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Require a launcher */
	if (!j_ptr->tval)
	{
		msg_print("You have nothing with which to fire.");
		return;
	}

	/* XXX HACK */
	if (j_ptr->tval == TV_INSTRUMENT)
	{
		msg_print("You cannot fire with an instrument.");
		return;
	}

	/* Get the "ammo" (if any) */
	o_ptr = &p_ptr->inventory[INVEN_AMMO];

	item = INVEN_AMMO;

	/* If nothing correct try to choose from the backpack */
	if ((p_ptr->tval_ammo != o_ptr->tval) || (!o_ptr->k_idx))
	{
		/* Require proper missile */
		item_tester_tval = p_ptr->tval_ammo;

		/* Get an item */
		q = "Your quiver is empty.  Fire which item? ";
		s = "You have nothing to fire.";
		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;


		/* Access the item */
		o_ptr = get_object(item);
	}


	/* Get a direction (or cancel) */
	if (!get_aim_dir(&dir)) return;


	/* Get local object */
	q_ptr = &forge;

	/* Obtain a local object */
	object_copy(q_ptr, o_ptr);

	/* Single object */
	q_ptr->number = 1;

	/* Reduce stack and describe */
	inc_stack_size(item, -1);

	/* Break goi/manashield */
	if (p_ptr->invuln)
	{
		set_invuln(0);
	}
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(0);
	}


	/* Sound */
	sound(SOUND_SHOOT);


	/* Describe the object */
	object_desc(o_name, q_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(q_ptr);
	missile_char = object_char(q_ptr);


	/* Use the proper number of shots */
	thits = p_ptr->num_fire;

	/* Use a base distance */
	tdis = 10;

	/* Base damage from thrown object plus launcher bonus */
	tdam = damroll(q_ptr->dd, q_ptr->ds) + q_ptr->to_d + j_ptr->to_d;

	/* Actually "fire" the object */
	bonus = (p_ptr->to_h + p_ptr->to_h_ranged + q_ptr->to_h + j_ptr->to_h);

	chance = (p_ptr->skill_thb + (bonus * BTH_PLUS_ADJ));
	if (chance < 5) chance = 5;

	tmul = get_shooter_mult(j_ptr);

	/* Get extra "power" from "extra might" */
	tmul += p_ptr->xtra_might;

	/* Boost the damage */
	tdam *= tmul;

	/* Add in the player damage */
	tdam += p_ptr->to_d_ranged;

	/* Base range */
	tdis = 10 + 5 * tmul;


	/* Take a (partial) turn */
	energy_use = (100 / thits);

	/* piercing shots ? */
	if (p_ptr->use_piercing_shots)
	{
		num_pierce = (get_skill(SKILL_COMBAT) / 10) - 1;
		num_pierce = (num_pierce < 0) ? 0 : num_pierce;
	}

	/* Start at the player */
	by = p_ptr->py;
	bx = p_ptr->px;
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff();

	oldtdam = tdam;
	while (TRUE)
	{
		/* Reset after a piercing shot */
		tdam = oldtdam;

		/* Travel until stopped */
		for (cur_dis = 0; cur_dis <= tdis; )
		{
			/* Hack -- Stop at the target */
			if ((y == ty) && (x == tx)) break;

			/* Calculate the new location (see "project()") */
			ny = y;
			nx = x;
			mmove2(&ny, &nx, by, bx, ty, tx);

			/* Stopped by walls/doors */
			if (!cave_floor_bold(ny, nx)) break;

			/* Advance the distance */
			cur_dis++;

			/* Save the new location */
			x = nx;
			y = ny;


			/* The player can see the (on screen) missile */
			if (panel_contains(y, x) && player_can_see_bold(y, x))
			{
				/* Draw, Hilite, Fresh, Pause, Erase */
				print_rel(missile_char, missile_attr, y, x);
				move_cursor_relative(y, x);
				Term_fresh();
				Term_xtra(TERM_XTRA_DELAY, msec);
				lite_spot(y, x);
				Term_fresh();
			}

			/* The player cannot see the missile */
			else
			{
				/* Pause anyway, for consistancy */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}


			/* Monster here, Try to hit it */
			if (cave[y][x].m_idx)
			{
				cave_type *c_ptr = &cave[y][x];

				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				monster_race *r_ptr = race_inf(m_ptr);

				/* Check the visibility */
				visible = m_ptr->ml;

				/* Note the collision */
				hit_body = TRUE;

				/* Did we hit it (penalize range) */
				if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
				{
					bool_ fear = FALSE;

					/* Assume a default death */
					cptr note_dies = " dies.";

					/* Some monsters get "destroyed" */
					if ((r_ptr->flags3 & (RF3_DEMON)) ||
					                (r_ptr->flags3 & (RF3_UNDEAD)) ||
					                (r_ptr->flags2 & (RF2_STUPID)) ||
					                (strchr("Evg", r_ptr->d_char)))
					{
						/* Special note at death */
						note_dies = " is destroyed.";
					}


					/* Handle unseen monster */
					if (!visible)
					{
						/* Invisible monster */
						msg_format("The %s finds a mark.", o_name);
					}

					/* Handle visible monster */
					else
					{
						char m_name[80];

						/* Get "the monster" or "it" */
						monster_desc(m_name, m_ptr, 0);

						/* Message */
						msg_format("The %s hits %s.", o_name, m_name);

						/* Hack -- Track this monster race */
						if (m_ptr->ml) monster_race_track(m_ptr->r_idx,
							                                  m_ptr->ego);

						/* Hack -- Track this monster */
						if (m_ptr->ml) health_track(c_ptr->m_idx);

						/* Anger friends */
						{
							char m_name[80];
							monster_desc(m_name, m_ptr, 0);
							switch (is_friend(m_ptr))
							{
							case 1:
								{
									msg_format("%^s gets angry!", m_name);
									change_side(m_ptr);
									break;
								}
							case 0:
								{
									msg_format("%^s gets angry!", m_name);
									m_ptr->status = MSTATUS_NEUTRAL_M;
									break;
								}
							}
						}
					}

					/* Apply special damage XXX XXX XXX */
					tdam = tot_dam_aux(q_ptr, tdam, m_ptr, &special);
					tdam = critical_shot(q_ptr->weight, q_ptr->to_h, tdam, SKILL_ARCHERY);

					/* No negative damage */
					if (tdam < 0) tdam = 0;

					/* Complex message */
					if (wizard)
					{
						msg_format("You do %d (out of %d) damage.",
						           tdam, m_ptr->hp);
					}

					/* Hit the monster, check for death */
					if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies))
					{
						/* Dead monster */
					}

					/* No death */
					else
					{
						/* Message */
						message_pain(c_ptr->m_idx, tdam);

						if (special) attack_special(m_ptr, special, tdam);

						/* Take note */
						if (fear && m_ptr->ml)
						{
							char m_name[80];

							/* Sound */
							sound(SOUND_FLEE);

							/* Get the monster name (or "it") */
							monster_desc(m_name, m_ptr, 0);

							/* Message */
							msg_format("%^s flees in terror!", m_name);
						}
					}
				}

				/* Stop looking */
				break;
			}
		}

		/* Exploding arrow ? */
		if (q_ptr->pval2 != 0)
		{
			int rad = 0, dam =
			                  (damroll(q_ptr->dd, q_ptr->ds) + q_ptr->to_d) * 2;
			int flag =
			        PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL |
			        PROJECT_JUMP;
			switch (q_ptr->sval)
			{
			case SV_AMMO_LIGHT:
				rad = 2;
				dam /= 2;
				break;
			case SV_AMMO_NORMAL:
				rad = 3;
				break;
			case SV_AMMO_HEAVY:
				rad = 4;
				dam *= 2;
				break;
			}

			project(0, rad, y, x, dam, q_ptr->pval2, flag);
		}

		/* Chance of breakage (during attacks) */
		j = (hit_body ? breakage_chance(q_ptr) : 0);

		/* Break ? */
		if ((q_ptr->pval2 != 0) || (rand_int(100) < j))
		{
			breakage = 100;
			break;
		}

		/* If the ammo doesn't break, it can pierce through */
		if ((num_pierce) && (hit_body) &&
		                (magik(45 + get_skill(SKILL_ARCHERY))))
		{
			num_pierce--;
			hit_body = FALSE;

			/* If target isn't reached, continue moving to target */
			if ( !((tx < x && x < bx) || (bx < x && x < tx)) &&
			                !((ty < y && y < by) || (by < y && y < ty)))
			{
				/* Continue moving in same direction if we reached the target */
				int dx = tx - bx;
				int dy = ty - by;
				tx = x + 99 * dx;
				ty = y + 99 * dy;

				/* New base location */
				by = y;
				bx = x;
			}

			msg_format("The %s pierces through!", o_name);
		}
		else
			break;
	}

	/* Drop (or break) near that location */
	drop_near(q_ptr, breakage, y, x);
}


/*
 * Why is this here? even if it's temporary boost...
 * Moved into player_type, hoping it might be useful in future extensions
 * -- pelpel
 */
/* int throw_mult = 1; */

/*
 * Throw an object from the pack or floor.
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 */
void do_cmd_throw(void)
{
	int dir, item;

	s32b special = 0;

	int j, y, x, ny, nx, ty, tx;

	int chance, tdam, tdis;

	int mul, div;

	int boulder_add = 0;
	int boulder_mult = 0;

	int cur_dis, visible;

	object_type forge;

	object_type *q_ptr;

	object_type *o_ptr;

	bool_ hit_body = FALSE;

	bool_ hit_wall = FALSE;

	byte missile_attr;

	char missile_char;

	char o_name[80];

	int msec = delay_factor * delay_factor * delay_factor;

	cptr q, s;

	u32b f1, f2, f3, f4, f5, esp;


	/* Get an item */
	q = "Throw which item? ";
	s = "You have nothing to throw.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Access the item */
	o_ptr = get_object(item);


	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Hack - Cannot throw away 'no drop' cursed items */
	if (cursed_p(o_ptr) && (f4 & TR4_CURSE_NO_DROP))
	{
		/* Oops */
		msg_print("Hmmm, you seem to be unable to throw it.");

		/* Nope */
		return;
	}

	/* Boulder throwing */
	if ((o_ptr->tval == TV_JUNK) && (o_ptr->sval == SV_BOULDER) && (get_skill(SKILL_BOULDER)))
	{
		boulder_add = get_skill_scale(SKILL_BOULDER, 80);
		boulder_mult = get_skill_scale(SKILL_BOULDER, 6);
	}

	/* Get a direction (or cancel) */
	if (!get_aim_dir(&dir)) return;

	/* Break goi/manashield */
	if (p_ptr->invuln)
	{
		set_invuln(0);
	}
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(0);
	}

	/* Get local object */
	q_ptr = &forge;

	/* Obtain a local object */
	object_copy(q_ptr, o_ptr);

	/*
	 * Hack -- If rods or wands are thrown, the total maximum timeout or
	 * charges need to be allocated between the two stacks.
	 */
	if (o_ptr->tval == TV_WAND)
	{
		q_ptr->pval = o_ptr->pval / o_ptr->number;

		if (o_ptr->number > 1) o_ptr->pval -= q_ptr->pval;
	}

	/* Single object */
	q_ptr->number = 1;

	/* Reduce stack and describe */
	inc_stack_size(item, -1);

	/* Description */
	object_desc(o_name, q_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(q_ptr);
	missile_char = object_char(q_ptr);

	/* Extract a "distance multiplier" */
	/* Changed for 'launcher' corruption */
	mul = 10 + (2 * (p_ptr->throw_mult - 1)) + (2 * boulder_mult);

	/* Enforce a minimum "weight" of one pound */
	div = ((q_ptr->weight > 10) ? q_ptr->weight : 10);

	/* Hack -- Distance -- Reward strength, penalize weight */
	tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

	/* Max distance of 10-18 */
	if (tdis > mul) tdis = mul;

	/* Hack -- Base damage from thrown object */
	tdam = damroll(q_ptr->dd, q_ptr->ds) + q_ptr->to_d + boulder_add;
	tdam *= p_ptr->throw_mult + boulder_mult;

	/* Chance of hitting - adjusted for Weaponmasters -- Gumby */
	chance = (p_ptr->skill_tht + (p_ptr->to_h * BTH_PLUS_ADJ));

	/* Take a turn */
	energy_use = 100;


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff();


	/* Travel until stopped */
	for (cur_dis = 0; cur_dis <= tdis; )
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);

		/* Stopped by walls/doors */
		if (!cave_floor_bold(ny, nx))
		{
			hit_wall = TRUE;
			break;
		}

		/* Advance the distance */
		cur_dis++;

		/* Save the new location */
		x = nx;
		y = ny;


		/* The player can see the (on screen) missile */
		if (panel_contains(y, x) && player_can_see_bold(y, x))
		{
			/* Draw, Hilite, Fresh, Pause, Erase */
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);
			Term_fresh();
			Term_xtra(TERM_XTRA_DELAY, msec);
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}


		/* Monster here, Try to hit it */
		if (cave[y][x].m_idx)
		{
			cave_type *c_ptr = &cave[y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = race_inf(m_ptr);

			/* Check the visibility */
			visible = m_ptr->ml;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
			{
				bool_ fear = FALSE;

				/* Assume a default death */
				cptr note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags3 & (RF3_DEMON)) ||
				                (r_ptr->flags3 & (RF3_UNDEAD)) ||
				                (r_ptr->flags2 & (RF2_STUPID)) ||
				                (strchr("Evg", r_ptr->d_char)))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}


				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msg_format("The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(m_name, m_ptr, 0);

					/* Message */
					msg_format("The %s hits %s.", o_name, m_name);

					/* Hack -- Track this monster race */
					if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

					/* Hack -- Track this monster */
					if (m_ptr->ml) health_track(c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux(q_ptr, tdam, m_ptr, &special);
				tdam = critical_shot(q_ptr->weight, q_ptr->to_h, tdam, o_ptr->sval == SV_BOULDER ? SKILL_BOULDER : SKILL_ARCHERY);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* Complex message */
				if (wizard)
				{
					msg_format("You do %d (out of %d) damage.",
					           tdam, m_ptr->hp);
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(c_ptr->m_idx, tdam);

					if (special) attack_special(m_ptr, special, tdam);

					/* Anger friends */
					if (!(k_info[q_ptr->k_idx].tval == TV_POTION))
					{
						char m_name[80];
						monster_desc(m_name, m_ptr, 0);
						switch (is_friend(m_ptr))
						{
						case 1:
							msg_format("%^s gets angry!", m_name);
							change_side(m_ptr);
							break;
						case 0:
							msg_format("%^s gets angry!", m_name);
							m_ptr->status = MSTATUS_NEUTRAL_M;
							break;
						}
					}

					/* Take note */
					if (fear && m_ptr->ml)
					{
						char m_name[80];

						/* Sound */
						sound(SOUND_FLEE);

						/* Get the monster name (or "it") */
						monster_desc(m_name, m_ptr, 0);

						/* Message */
						msg_format("%^s flees in terror!", m_name);
					}
				}
			}

			/* Stop looking */
			break;
		}
	}

	/* Chance of breakage (during attacks) */
	j = (hit_body ? breakage_chance(q_ptr) : 0);

	/* Potions smash open */
	if (k_info[q_ptr->k_idx].tval == TV_POTION)
	{
		if ((hit_body) || (hit_wall) || (randint(100) < j))
		{
			/* Message */
			msg_format("The %s shatters!", o_name);

			if (potion_smash_effect(0, y, x, q_ptr->sval))
			{
				if (cave[y][x].m_idx)
				{
					char m_name[80];
					monster_desc(m_name, &m_list[cave[y][x].m_idx], 0);
					switch (is_friend(&m_list[cave[y][x].m_idx]))
					{
					case 1:
						msg_format("%^s gets angry!", m_name);
						change_side(&m_list[cave[y][x].m_idx]);
						break;
					case 0:
						msg_format("%^s gets angry!", m_name);
						m_list[cave[y][x].m_idx].status = MSTATUS_NEUTRAL_M;
						break;
					}
				}
			}

			return;
		}
		else
		{
			j = 0;
		}
	}

	/* Drop (or break) near that location */
	drop_near(q_ptr, j, y, x);
}


/*
 * Throw a boomerang object from the equipement(bow).
 *
 * Note: "unseen" monsters are very hard to hit.
 *
 * Should throwing a weapon do full damage?  Should it allow the magic
 * to hit bonus of the weapon to have an effect?  Should it ever cause
 * the item to be destroyed?  Should it do any damage at all?
 */
void do_cmd_boomerang(void)
{
	int dir;

	int j, y, x, ny, nx, ty, tx;

	int chance, tdam, tdis;

	int mul, div;

	int cur_dis, visible;

	object_type forge;

	object_type *q_ptr;

	object_type *o_ptr;

	bool_ hit_body = FALSE;

	byte missile_attr;

	char missile_char;

	char o_name[80];

	s32b special = 0;

	int msec = delay_factor * delay_factor * delay_factor;


	/* Get the "bow" (if any) */
	o_ptr = &p_ptr->inventory[INVEN_BOW];


	/* Get a direction (or cancel) */
	if (!get_aim_dir(&dir)) return;


	/* Get local object */
	q_ptr = &forge;

	/* Obtain a local object */
	object_copy(q_ptr, o_ptr);

	/* Single object */
	q_ptr->number = 1;

	/* Description */
	object_desc(o_name, q_ptr, FALSE, 3);

	/* Find the color and symbol for the object for throwing */
	missile_attr = object_attr(q_ptr);
	missile_char = object_char(q_ptr);

	/* Extract a "distance multiplier" */
	/* Changed for 'launcher' corruption */
	mul = 10 + 2 * (p_ptr->throw_mult - 1);

	/* Enforce a minimum "weight" of one pound */
	div = ((q_ptr->weight > 10) ? q_ptr->weight : 10);

	/* Hack -- Distance -- Reward strength, penalize weight */
	tdis = (adj_str_blow[p_ptr->stat_ind[A_STR]] + 20) * mul / div;

	/* Max distance of 10-18 */
	if (tdis > mul) tdis = mul;

	/* Hack -- Base damage from thrown object */
	tdam = damroll(q_ptr->dd, q_ptr->ds) + q_ptr->to_d;
	tdam *= p_ptr->throw_mult;

	/* Chance of hitting */
	chance =
	        (p_ptr->skill_tht +
	         ((p_ptr->to_h + p_ptr->to_h_ranged) * BTH_PLUS_ADJ));

	chance += get_skill(SKILL_BOOMERANG);

	/* Take a turn */
	energy_use = 100;


	/* Start at the player */
	y = p_ptr->py;
	x = p_ptr->px;

	/* Predict the "target" location */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Check for "target request" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}


	/* Hack -- Handle stuff */
	handle_stuff();


	/* Travel until stopped */
	for (cur_dis = 0; cur_dis <= tdis; )
	{
		/* Hack -- Stop at the target */
		if ((y == ty) && (x == tx)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, p_ptr->py, p_ptr->px, ty, tx);

		/* Stopped by walls/doors */
		if (!cave_floor_bold(ny, nx))
		{
			break;
		}

		/* Advance the distance */
		cur_dis++;

		/* Save the new location */
		x = nx;
		y = ny;


		/* The player can see the (on screen) missile */
		if (panel_contains(y, x) && player_can_see_bold(y, x))
		{
			/* Draw, Hilite, Fresh, Pause, Erase */
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);
			Term_fresh();
			Term_xtra(TERM_XTRA_DELAY, msec);
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}


		/* Monster here, Try to hit it */
		if (cave[y][x].m_idx)
		{
			cave_type *c_ptr = &cave[y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			monster_race *r_ptr = race_inf(m_ptr);

			/* Check the visibility */
			visible = m_ptr->ml;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
			{
				bool_ fear = FALSE;

				/* Assume a default death */
				cptr note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags3 & (RF3_DEMON)) ||
				                (r_ptr->flags3 & (RF3_UNDEAD)) ||
				                (r_ptr->flags2 & (RF2_STUPID)) ||
				                (strchr("Evg", r_ptr->d_char)))
				{
					/* Special note at death */
					note_dies = " is destroyed.";
				}


				/* Handle unseen monster */
				if (!visible)
				{
					/* Invisible monster */
					msg_format("The %s finds a mark.", o_name);
				}

				/* Handle visible monster */
				else
				{
					char m_name[80];

					/* Get "the monster" or "it" */
					monster_desc(m_name, m_ptr, 0);

					/* Message */
					msg_format("The %s hits %s.", o_name, m_name);

					/* Hack -- Track this monster race */
					if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

					/* Hack -- Track this monster */
					if (m_ptr->ml) health_track(c_ptr->m_idx);
				}

				/* Apply special damage XXX XXX XXX */
				tdam = tot_dam_aux(q_ptr, tdam, m_ptr, &special);
				tdam = critical_shot(q_ptr->weight, q_ptr->to_h, tdam, SKILL_ARCHERY);

				/* No negative damage */
				if (tdam < 0) tdam = 0;

				/* Complex message */
				if (wizard)
				{
					msg_format("You do %d (out of %d) damage.",
					           tdam, m_ptr->hp);
				}

				/* Hit the monster, check for death */
				if (mon_take_hit(c_ptr->m_idx, tdam, &fear, note_dies))
				{
					/* Dead monster */
				}

				/* No death */
				else
				{
					/* Message */
					message_pain(c_ptr->m_idx, tdam);

					if (special) attack_special(m_ptr, special, tdam);

					/* Anger friends */
					if (!(k_info[q_ptr->k_idx].tval == TV_POTION))
					{
						char m_name[80];
						monster_desc(m_name, m_ptr, 0);
						switch (is_friend(m_ptr))
						{
						case 1:
							msg_format("%^s gets angry!", m_name);
							change_side(m_ptr);
							break;
						case 0:
							msg_format("%^s gets angry!", m_name);
							m_ptr->status = MSTATUS_NEUTRAL_M;
							break;
						}
					}

					/* Take note */
					if (fear && m_ptr->ml)
					{
						char m_name[80];

						/* Sound */
						sound(SOUND_FLEE);

						/* Get the monster name (or "it") */
						monster_desc(m_name, m_ptr, 0);

						/* Message */
						msg_format("%^s flees in terror!", m_name);
					}
				}

				/* Chance of breakage (during attacks) */
				j = (hit_body ? breakage_chance(o_ptr) : 0);

				/* Break the boomerang */
				if (!(o_ptr->art_name || artifact_p(o_ptr)) &&
				                (rand_int(100) < j))
				{
					msg_print(format("Your %s is destroyed.", o_name));
					inc_stack_size_ex(INVEN_BOW, -1, OPTIMIZE, NO_DESCRIBE);
				}
			}

			/* Stop looking */
			break;
		}
	}

	/* Travel back to the player */
	for (cur_dis = 0; cur_dis <= tdis; )
	{
		/* Hack -- Stop at the target */
		if ((y == p_ptr->py) && (x == p_ptr->px)) break;

		/* Calculate the new location (see "project()") */
		ny = y;
		nx = x;
		mmove2(&ny, &nx, ty, tx, p_ptr->py, p_ptr->px);

		/* Advance the distance */
		cur_dis++;

		/* Save the new location */
		x = nx;
		y = ny;


		/* The player can see the (on screen) missile */
		if (panel_contains(y, x) && player_can_see_bold(y, x))
		{
			/* Draw, Hilite, Fresh, Pause, Erase */
			print_rel(missile_char, missile_attr, y, x);
			move_cursor_relative(y, x);
			Term_fresh();
			Term_xtra(TERM_XTRA_DELAY, msec);
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			Term_xtra(TERM_XTRA_DELAY, msec);
		}
	}
}


/*
 * Try to ``walk'' using phase door.
 */
void do_cmd_unwalk()
{
	int dir, y, x, feat;

	cave_type *c_ptr;

	bool_ more = FALSE;


	if (!get_rep_dir(&dir)) return;

	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	c_ptr = &cave[y][x];
	feat = c_ptr->feat;

	/* Must have knowledge to know feature XXX XXX */
	if (!(c_ptr->info & (CAVE_MARK))) feat = FEAT_NONE;

	/* Take a turn */
	energy_use = 100;
	energy_use *= (p_ptr->wild_mode) ? (5 * (MAX_HGT + MAX_WID) / 2) : 1;


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);

		/* Cancel the arg */
		command_arg = 0;
	}


	/* Attack monsters */
	if (c_ptr->m_idx > 0)
	{
		/* Attack */
		py_attack(y, x, -1);
	}

	/* Exit the area */
	else if ((!dun_level) && (!p_ptr->wild_mode) &&
	                ((x == 0) || (x == cur_wid - 1) || (y == 0) || (y == cur_hgt - 1)))
	{
		/* Can the player enter the grid? */
		if (player_can_enter(c_ptr->mimic))
		{
			/* Hack: move to new area */
			if ((y == 0) && (x == 0))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = cur_wid - 2;
				ambush_flag = FALSE;
			}

			else if ((y == 0) && (x == MAX_WID - 1))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = 1;
				ambush_flag = FALSE;
			}

			else if ((y == MAX_HGT - 1) && (x == 0))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = cur_wid - 2;
				ambush_flag = FALSE;
			}

			else if ((y == MAX_HGT - 1) && (x == MAX_WID - 1))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = 1;
				ambush_flag = FALSE;
			}

			else if (y == 0)
			{
				p_ptr->wilderness_y--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = x;
				ambush_flag = FALSE;
			}

			else if (y == cur_hgt - 1)
			{
				p_ptr->wilderness_y++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = x;
				ambush_flag = FALSE;
			}

			else if (x == 0)
			{
				p_ptr->wilderness_x--;
				p_ptr->oldpx = cur_wid - 2;
				p_ptr->oldpy = y;
				ambush_flag = FALSE;
			}

			else if (x == cur_wid - 1)
			{
				p_ptr->wilderness_x++;
				p_ptr->oldpx = 1;
				p_ptr->oldpy = y;
				ambush_flag = FALSE;
			}

			p_ptr->leaving = TRUE;

			return;
		}
	}

	/* Hack -- Ignore weird terrain types. */
	else if (!cave_floor_grid(c_ptr))
	{
		teleport_player(10);
	}

	/* Enter quests */
	else if (((feat >= FEAT_QUEST_ENTER) && (feat <= FEAT_QUEST_UP)) ||
	                ((feat >= FEAT_LESS) && (feat <= FEAT_MORE)))
	{
		move_player(dir, always_pickup, TRUE);
		more = FALSE;
	}

	/* Hack -- Ignore wilderness mofe. */
	else if (p_ptr->wild_mode)
	{
		/* Chance to not blink right */
		if (magik(15))
		{
			do
			{
				dir = rand_range(1, 9);
			}
			while (dir == 5);
		}

		move_player(dir, always_pickup, TRUE);
	}

	/* Walking semantics */
	else
	{
		teleport_player_directed(10, dir);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb(0, 0);
}


static bool_ tport_vertically(bool_ how)
{
	/* arena or quest -KMW- */
	if ((p_ptr->inside_arena) || (p_ptr->inside_quest))
	{
		msg_print("There is no effect.");
		return (FALSE);
	}

	if (dungeon_flags2 & DF2_NO_EASY_MOVE)
	{
		msg_print("Some powerful force prevents you from teleporting.");
		return FALSE;
	}

	/* Go down */
	if (how)
	{
		if (dun_level >= d_info[dungeon_type].maxdepth)
		{
			msg_print("The floor is impermeable.");
			return (FALSE);
		}

		msg_print("You sink through the floor.");
		dun_level++;
		p_ptr->leaving = TRUE;
	}
	else
	{
		if (dun_level < d_info[dungeon_type].mindepth)
		{
			msg_print("There is nothing above you but air.");
			return (FALSE);
		}

		msg_print("You rise through the ceiling.");
		dun_level--;
		p_ptr->leaving = TRUE;
	}

	return (TRUE);
}


/*
 * Do a special ``movement'' action. Meant to be used for ``immovable''
 * characters.
 */
void do_cmd_immovable_special(void)
{
	int i, ii, ij, dir;

	int foo = p_ptr->immov_cntr;

	int lose_sp = 0;

	int lose_hp = 0;

	bool_ did_act = FALSE;

	bool_ did_load = FALSE;


	if (foo > 1)
	{
		if (p_ptr->csp > foo / 2)
		{

			msg_format("This will drain %d mana points!", foo / 2);
			if (!get_check("Proceed? ")) return;

			lose_sp = foo / 2;

		}
		else if (p_ptr->chp > foo / 2)
		{

			msg_format("Warning: This will drain %d hit points!", foo / 2);
			if (!get_check("Proceed? ")) return;

			lose_hp = foo / 2;

		}
		else
		{
			msg_print("You can't use your powers yet.");
			return;
		}
	}

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Do what special action:", 2, 0);

		/* Give some choices */
		prt("(a) Teleport to a specific place.", 4, 5);
		prt("(b) Fetch an item.", 5, 5);
		prt("(c) Go up 50'", 6, 5);
		prt("(d) Go down 50'", 7, 5);

		/* Prompt */
		prt("Command: ", 9, 0);

		/* Prompt */
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		/* Tele-to */
		if (i == 'a')
		{
			Term_load();
			character_icky = FALSE;
			did_load = TRUE;

			if (!tgt_pt(&ii, &ij)) break;

			/* Teleport to the target */
			teleport_player_to(ij, ii);

			did_act = TRUE;
			break;
		}

		/* Fetch item */
		else if (i == 'b')
		{
			Term_load();
			character_icky = FALSE;
			did_load = TRUE;

			if (!get_aim_dir(&dir)) return;
			fetch(dir, p_ptr->lev * 15, FALSE);
			py_pickup_floor(always_pickup);

			did_act = TRUE;
			break;
		}

		/* Move up */
		else if (i == 'c')
		{
			Term_load();
			character_icky = FALSE;
			did_load = TRUE;

			if (!tport_vertically(FALSE)) return;

			did_act = TRUE;
			break;
		}

		/* Move down */
		else if (i == 'd')
		{
			Term_load();
			character_icky = FALSE;
			did_load = TRUE;

			if (!tport_vertically(TRUE)) return;

			did_act = TRUE;
			break;
		}

		/* Unknown option */
		else
		{
			bell();
		}

	}

	/* Check if screen was restored before */
	if (!did_load)
	{
		/* Restore the screen */
		Term_load();

		/* Leave "icky" mode */
		character_icky = FALSE;
	}

	/* Apply stat losses if something was done */
	if (did_act)
	{
		p_ptr->immov_cntr += 101 - (p_ptr->lev * 2);

		if (lose_sp)
		{
			p_ptr->csp -= lose_sp;
			p_ptr->redraw |= (PR_MANA);
		}

		if (lose_hp)
		{
			p_ptr->chp -= lose_hp;
			p_ptr->redraw |= (PR_HP);
		}

		energy_use = 100;
	}
}

/* Can we sacrifice it ? */
static bool_ item_tester_hook_sacrifiable(object_type *o_ptr)
{
	GOD(GOD_MELKOR)
	{
		/* Corpses are */
		if (o_ptr->tval == TV_CORPSE && o_ptr->sval == SV_CORPSE_CORPSE)
			return (TRUE);

		/* Books without any udun spells */
		if ((o_ptr->tval == TV_BOOK) && udun_in_book(o_ptr->sval, o_ptr->pval) <= 0)
		{
			return TRUE;
		}
	}

	/* Assume not */
	return (FALSE);
}

/*
 * Is item eligible for sacrifice to Aule?
 */
static bool_ item_tester_hook_sacrifice_aule(object_type *o_ptr)
{
	/* perhaps restrict this only to metal armour and weapons  */
	return (o_ptr->found == OBJ_FOUND_SELFMADE);
}

/*
 * Handle sacrifices to Aule
 */
static void do_cmd_sacrifice_aule()
{
	int item;

	item_tester_hook = item_tester_hook_sacrifice_aule;
	if (!get_item(&item,
		      "Sacrifice which item? ",
		      "You have nothing to sacrifice.",
		      USE_INVEN))
	{
		return;
	}

	/* Increase piety by the value of the item / 10. */
	{
		object_type *o_ptr = get_object(item);
		s32b delta = object_value(o_ptr) / 10;

		inc_piety(GOD_ALL, delta);
	}

	/* Destroy the object */
	inc_stack_size(item, -1);
}

/*
 * Handle sacrifices.
 * Grace is increased by value of sacrifice.
 */
void do_cmd_sacrifice(void)
{
	byte on_what = cave[p_ptr->py][p_ptr->px].feat;

	/* Check valididty */
	if ((on_what < FEAT_ALTAR_HEAD) || (on_what > FEAT_ALTAR_TAIL))
	{
		show_god_info(FALSE);
		return;
	}
	else
	{
		int agod = on_what - FEAT_ALTAR_HEAD + 1;

		/* Not worshipping a god ? ahhhh! */
		GOD(GOD_NONE)
		{
			int i;

			for (i = 0; i < 10; i++)
			{
				if (deity_info[agod].desc[i] != NULL)
					msg_print(deity_info[agod].desc[i]);
			}
			if (get_check(format("Do you want to worship %s? ", deity_info[agod].name)))
			{
				follow_god(agod, FALSE);
				p_ptr->grace = -200;
				inc_piety(p_ptr->pgod, 0);
			}
		}
		else if (p_ptr->pgod == agod)
		{
			GOD(GOD_MELKOR)
			{
				/* One can sacrifice some HP for piety or damage */
				if ((p_ptr->mhp > 10) && (p_ptr->chp > 10) && get_check("Do you want to sacrifice a part of yourself? "))
				{
					/* 10 HP = 300 * wis piety */
					if (get_check("Do you want to sacrifice for more piety instead of damage? "))
					{
						int x = wisdom_scale(6);
						if (x < 1) x = 1;

						p_ptr->hp_mod -= 10;
						take_hit(10, "self sacrifice to Melkor");
						msg_print("Your life slips away, and Melkor seems happier.");
						inc_piety(GOD_MELKOR, x * 300);
						p_ptr->update |= (PU_HP);
					}
					/* 10 HP = +wis damage */
					else
					{
						take_hit(10, "self sacrifice to Melkor");
						msg_print("Your life slips away, and your arms grow stronger.");
						p_ptr->melkor_sacrifice++;
						p_ptr->update |= (PU_BONUS | PU_HP);
					}
				}
				else
				{
					int item;
					object_type *o_ptr;

					/* Restrict choices to food */
					item_tester_hook = item_tester_hook_sacrifiable;

					/* Get an item */
					if (!get_item(&item, "Sacrifice which item? ", "You have nothing to sacrifice.", (USE_INVEN))) return;
					o_ptr = get_object(item);

					/* Piety for corpses is based on monster level */
					if (o_ptr->tval == TV_CORPSE)
					{
						inc_piety(GOD_MELKOR, 2 * r_info[o_ptr->pval2].level);
					}

					/* In books it depends of the spell levels*/
					if (o_ptr->tval == TV_BOOK)
					{
						int x = levels_in_book(o_ptr->sval, o_ptr->pval);

						inc_piety(GOD_MELKOR, 2 * x);
					}

					/* Remove the item */
					inc_stack_size(item, -1);
				}
			}

			GOD(GOD_AULE)
			{
				do_cmd_sacrifice_aule();
			}

		}
	}
}


/*
 * scan_monst --
 *
 * Return a list of o_list[] indexes of items of the given monster
 */
bool_ scan_monst(int *items, int *item_num, int m_idx)
{
	int this_o_idx, next_o_idx;

	int num = 0;


	(*item_num) = 0;

	/* Scan all objects in the grid */
	for (this_o_idx = m_list[m_idx].hold_o_idx; this_o_idx;
	                this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Accept this item */
		items[num++] = this_o_idx;

		/* XXX Hack -- Enforce limit */
		if (num == 23) break;
	}

	/* Number of items */
	(*item_num) = num;

	/* Result */
	return (num != 0);
}


/*
 * Display a list of the items that the given monster carries.
 */
byte show_monster_inven(int m_idx, int *monst_list)
{
	int i, j, k, l;

	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[23];

	byte out_color[23];

	char out_desc[23][80];

	int monst_num;


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight */
	lim -= 9;

	/* Scan for objects on the monster */
	(void)scan_monst(monst_list, &monst_num, m_idx);

	/* Display the p_ptr->inventory */
	for (k = 0, i = 0; i < monst_num; i++)
	{
		o_ptr = &o_list[monst_list[i]];

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Acquire p_ptr->inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval & 0x7F];

		/* Save the object description */
		strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = monst_list[out_index[j]];

		/* Get the item */
		o_ptr = &o_list[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		strnfmt(tmp_val, 80, "%c)", index_to_label(j));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		{
			int wgt = o_ptr->weight * o_ptr->number;
			strnfmt(tmp_val, 80, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

	return monst_num;
}


/*
 * Steal an object from a monster
 */
void do_cmd_steal()
{
	int x, y, dir = 0, item = -1, k = -1;

	cave_type *c_ptr;

	monster_type *m_ptr;

	object_type *o_ptr, forge;

	byte num = 0;

	bool_ done = FALSE;

	int monst_list[23];


	/* Only works on adjacent monsters */
	if (!get_rep_dir(&dir)) return;
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];
	c_ptr = &cave[y][x];

	if (!(c_ptr->m_idx))
	{
		msg_print("There is no monster there!");
		return;
	}

	m_ptr = &m_list[c_ptr->m_idx];

	/* There were no non-gold items */
	if (!m_ptr->hold_o_idx)
	{
		msg_print("That monster has no objects!");
		return;
	}

	/* The monster is immune */
	if (r_info[m_ptr->r_idx].flags7 & (RF7_NO_THEFT))
	{
		msg_print("The monster is guarding the treasures.");
		return;
	}

	screen_save();

	num = show_monster_inven(c_ptr->m_idx, monst_list);

	/* Repeat until done */
	while (!done)
	{
		char tmp_val[80];
		char which = ' ';

		/* Build the prompt */
		strnfmt(tmp_val, 80, "Choose an item to steal (a-%c) or ESC:",
		        'a' - 1 + num);

		/* Show the prompt */
		prt(tmp_val, 0, 0);

		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which)
		{
		case ESCAPE:
			{
				done = TRUE;

				break;
			}

		default:
			{
				int ver;

				/* Extract "query" setting */
				ver = isupper(which);
				which = tolower(which);

				k = islower(which) ? A2I(which) : -1;
				if (k < 0 || k >= num)
				{
					bell();

					break;
				}

				/* Verify the item */
				if (ver && !verify("Try", 0 - monst_list[k]))
				{
					done = TRUE;

					break;
				}

				/* Accept that choice */
				item = monst_list[k];
				done = TRUE;

				break;
			}
		}
	}

	if (item != -1)
	{
		int chance;

		chance = 40 - p_ptr->stat_ind[A_DEX];
		chance +=
		        o_list[item].weight / (get_skill_scale(SKILL_STEALING, 19) + 1);
		chance += get_skill_scale(SKILL_STEALING, 29) + 1;
		chance -= (m_ptr->csleep) ? 10 : 0;
		chance += m_ptr->level;

		/* Failure check */
		if (rand_int(chance) > 1 + get_skill_scale(SKILL_STEALING, 25))
		{
			/* Take a turn */
			energy_use = 100;

			/* Wake up */
			m_ptr->csleep = 0;

			/* Speed up because monsters are ANGRY when you try to thief them */
			m_ptr->mspeed += 5;

			screen_load();

			msg_print("Oops! The monster is now really *ANGRY*!");

			return;
		}

		/* Reconnect the objects list */
		if (num == 1) m_ptr->hold_o_idx = 0;
		else
		{
			if (k > 0) o_list[monst_list[k - 1]].next_o_idx = monst_list[k + 1];
			if (k + 1 >= num) o_list[monst_list[k - 1]].next_o_idx = 0;
			if (k == 0) m_ptr->hold_o_idx = monst_list[k + 1];
		}

		/* Rogues gain some xp */
		if (PRACE_FLAGS(PR1_EASE_STEAL))
		{
			s32b max_point;

			/* Max XP gained from stealing */
			max_point = (o_list[item].weight / 2) + (m_ptr->level * 10);

			/* Randomise it a bit, with half a max guaranteed */
			gain_exp((max_point / 2) + (randint(max_point) / 2));

			/* Allow escape */
			if (get_check("Phase door?")) teleport_player(10);
		}

		/* Get the item */
		o_ptr = &forge;

		/* Special handling for gold */
		if (o_list[item].tval == TV_GOLD)
		{
			/* Collect the gold */
			p_ptr->au += o_list[item].pval;

			/* Redraw gold */
			p_ptr->redraw |= (PR_GOLD);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
		else
		{
			object_copy(o_ptr, &o_list[item]);

			inven_carry(o_ptr, FALSE);
		}

		/* Delete it */
		o_list[item].k_idx = 0;
	}

	screen_load();

	/* Take a turn */
	energy_use = 100;
}


/*
 * Give an item to a monster
 */
void do_cmd_give()
{
	int dir, x, y;

	cave_type *c_ptr;

	cptr q, s;

	int item;


	/* Get a "repeated" direction */
	if (!get_rep_dir(&dir)) return;

	/* Get requested location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Get requested grid */
	c_ptr = &cave[y][x];

	/* No monster in the way */
	if (c_ptr->m_idx == 0)
	{
		msg_print("There is no monster there.");
		return;
	}

	/* Get an item */
	q = "What item do you want to offer? ";
	s = "You have nothing to offer.";
	if (!get_item(&item, q, s, USE_INVEN)) return;

	/* Process hooks if there are any */
	if (!process_hooks(HOOK_GIVE, "(d,d)", c_ptr->m_idx, item))
	{
		hook_give_in in = { c_ptr->m_idx, item };
		if (!process_hooks_new(HOOK_GIVE, &in, NULL))
		{
			msg_print("The monster does not want your item.");
		}
	}

	/* Take a turn, even if the offer is declined */
	energy_use = 100;
}


/*
 * Chat with a monster
 */
void do_cmd_chat()
{
	int dir, x, y;

	cave_type *c_ptr;


	/* Get a "repeated" direction */
	if (!get_rep_dir(&dir)) return;

	/* Get requested location */
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];

	/* Get requested grid */
	c_ptr = &cave[y][x];

	/* No monster in the way */
	if (c_ptr->m_idx == 0)
	{
		msg_print("There is no monster there.");
		return;
	}

	/* Process hook if there are any */
	if (!process_hooks(HOOK_CHAT, "(d)", c_ptr->m_idx))
	{
		msg_print("The monster does not want to chat.");
	}

	/* No energy spent */
}
