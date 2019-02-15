/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "cmd2.hpp"

#include "bldg.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "cmd5.hpp"
#include "dungeon_info_type.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "hook_chat_in.hpp"
#include "hook_enter_dungeon_in.hpp"
#include "hook_give_in.hpp"
#include "hook_stair_in.hpp"
#include "hook_stair_out.hpp"
#include "hooks.hpp"
#include "levels.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_race_flag.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "spells3.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-form.h"
#include "z-rand.hpp"
#include "z-term.h"

#include <chrono>
#include <fmt/format.h>
#include <thread>

using std::this_thread::sleep_for;
using std::chrono::milliseconds;

void do_cmd_immovable_special();

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
	auto const &r_info = game->edit_data.r_info;

	int bash, temp;

	bool_ more = TRUE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_BASH_DOOR))
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
	hook_stair_in in = { direction };
	hook_stair_out out = { TRUE }; /* Allow by default */
	process_hooks_new(HOOK_STAIR, &in, &out);
	return (!out.allow);
}

/*
 * Ask for confirmation before leaving level; based
 * on whether the 'confirm_stairs' option is set.
 */
static bool ask_leave()
{
	if (options->confirm_stairs)
	{
		if (get_check("Really leave the level? "))
		{
			return true; // Leave
		}
		else
		{
			return false; // Don't leave
		}
	}
	else
	{
		return true; // Leave
	}
}


/*
 * Go up one level
 */
void do_cmd_go_up()
{
	auto const &d_info = game->edit_data.d_info;
	auto const &dungeon_flags = game->dungeon_flags;

	bool_ go_up = FALSE, go_up_many = FALSE, prob_traveling = FALSE;

	cave_type *c_ptr;

	int oldl = dun_level;

	auto d_ptr = &d_info[dungeon_type];


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
		else if (dungeon_flags & DF_ASK_LEAVE)
		{
			go_up = get_check("Leave this unique level forever? ");
		}
		else if (ask_leave())
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
		else if (dungeon_flags & DF_ASK_LEAVE)
		{
			go_up = get_check("Leave this unique level forever? ");
		}
		else if (ask_leave())
		{
			go_up_many = TRUE;
		}
	}

	/* Quest exit */
	else if (c_ptr->feat == FEAT_QUEST_EXIT)
	{
		leaving_quest = p_ptr->inside_quest;

		if ((dungeon_flags & DF_ASK_LEAVE) &&
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
	else if (!(dungeon_flags & DF_FLAT) &&
	                p_ptr->prob_travel && !p_ptr->inside_quest)
	{
		if (d_ptr->mindepth == dun_level) return;

		if (dungeon_flags & DF_NO_EASY_MOVE)
		{
			msg_print("Some powerful force prevents your from teleporting.");
			return;
		}

		prob_traveling = TRUE;

		if (ask_leave())
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
static bool_ between_effect()
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
void do_cmd_go_down()
{
	auto const &d_info = game->edit_data.d_info;
	auto const &dungeon_flags = game->dungeon_flags;

	cave_type *c_ptr;

	bool_ go_down = FALSE, go_down_many = FALSE, prob_traveling = FALSE;

	char i;

	int old_dun = dun_level;

	auto d_ptr = &d_info[dungeon_type];


	/*  MUST be actived now */
	if (between_effect()) return;

	/* Player grid */
	c_ptr = &cave[p_ptr->py][p_ptr->px];

	if (p_ptr->astral && (dun_level == 98)) return;

	/* test if on special level */
	if (dungeon_flags & DF_ASK_LEAVE)
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
			if (ask_leave())
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
			if (ask_leave())
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

	else if (!(dungeon_flags & DF_FLAT) &&
	                p_ptr->prob_travel && !p_ptr->inside_quest)
	{
		if (d_ptr->maxdepth == dun_level) return;

		if (dungeon_flags & DF_NO_EASY_MOVE)
		{
			msg_print("Some powerful force prevents your from teleporting.");
			return;
		}

		prob_traveling = TRUE;

		if (ask_leave())
		{
			go_down = TRUE;
		}
	}

	else
	{
		msg_print("I see no down staircase here.");
		return;
	}

	if (go_down || go_down_many)
	{
		energy_use = 0;

		if (c_ptr->feat == FEAT_WAY_MORE)
			msg_print("You enter the next area.");
		else
			msg_print("You enter a maze of down staircases.");

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
				auto d_ptr = &d_info[c_ptr->special];

				/* Do the lua scripts refuse ? ;) */
				{
					struct hook_enter_dungeon_in in = { c_ptr->special };
					if (process_hooks_new(HOOK_ENTER_DUNGEON, &in, NULL))
					{
						dun_level = old_dun;
						return;
					}
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

				msg_format("You go into %s", d_info[dungeon_type].text.c_str());
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
	}
}

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
	auto const &r_info = game->edit_data.r_info;

	int i, j;

	cave_type *c_ptr;

	bool_ more = FALSE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_OPEN_DOOR))
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
		i = 100;

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

			/* Open the door */
			cave_set_feat(y, x, FEAT_OPEN);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			/* Experience */
			gain_exp(1);
		}

		/* Failure */
		else
		{
			/* Failure */
			flush_on_failure();

			/* Message */
			msg_print("You failed to pick the lock.");

			/* We may keep trying */
			more = TRUE;
		}
	}

	/* Closed door */
	else
	{
		/* Open the door */
		cave_set_feat(y, x, FEAT_OPEN);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);
	}

	/* Result */
	return (more);
}



/*
 * Open a closed/locked/jammed door or a closed/locked chest.
 *
 * Unlocking a locked door/chest is worth one experience point.
 */
void do_cmd_open()
{
	auto const &r_info = game->edit_data.r_info;

	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_OPEN_DOOR))
	{
		msg_print("You cannot open doors.");

		return;
	}

	/* Pick a direction if there's an obvious target */
	{
		/* Count closed doors (locked or jammed) */
		const int num_doors = count_feats(&y, &x, is_closed, FALSE);

		/* There is nothing the player can open */
		if (num_doors == 0)
		{
			/* Message */
			msg_print("You see nothing there to open.");

			/* Done */
			return;
		}

		/* Set direction if there is only one target */
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
		p_ptr->redraw |= (PR_FRAME);

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

		/* Nothing useful */
		if (!((c_ptr->feat >= FEAT_DOOR_HEAD) &&
				(c_ptr->feat <= FEAT_DOOR_TAIL)))
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

		/* Handle doors */
		else
		{
			/* Open the door */
			more = do_cmd_open_aux(y, x, dir);
		}
	}

	/* Cancel repeat unless we may continue */
	if (!more) disturb();
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
	auto const &r_info = game->edit_data.r_info;

	cave_type *c_ptr;

	bool_ more = FALSE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_OPEN_DOOR))
	{
		msg_print("You cannot close doors.");

		return (FALSE);
	}

	/* Take a turn */
	energy_use = 100;

	/* Get grid and contents */
	c_ptr = &cave[y][x];

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
	}

	/* Result */
	return (more);
}


/*
 * Close an open door.
 */
void do_cmd_close()
{
	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;


	/* Pick a direction if there's an obvious choice */
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
		p_ptr->redraw |= (PR_FRAME);

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
	if (!more) disturb();
}


/*
 * Determine if a given grid may be "tunneled"
 */
static bool_ do_cmd_tunnel_test(int y, int x)
{
	auto const &f_info = game->edit_data.f_info;

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
	if (!(f_info[cave[y][x].feat].flags & FF_TUNNELABLE))
	{
		/* Message */
		msg_print(f_info[cave[y][x].feat].tunnel);

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
static bool_ do_cmd_tunnel_aux(int y, int x, int dir)
{
	auto const &d_info = game->edit_data.d_info;
	auto const &f_info = game->edit_data.f_info;

	int skill_req = 0, skill_req_1pct = 0;
	cave_type *c_ptr = &cave[y][x];

	auto f_ptr = &f_info[c_ptr->feat];

	bool_ more = FALSE;


	/* Must be have something to dig with (except for sandwalls) */
	if ((c_ptr->feat < FEAT_SANDWALL) || (c_ptr->feat > FEAT_SANDWALL_K))
	{
		auto o_ptr = &p_ptr->inventory[INVEN_TOOL];
		if (!o_ptr->k_ptr || (o_ptr->tval != TV_DIGGING))
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

	/* Titanium */
	if (f_ptr->flags & FF_PERMANENT)
	{
		msg_print(f_ptr->tunnel);
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
			msg_print(f_ptr->tunnel);
			more = TRUE;
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
			msg_print(f_ptr->tunnel);
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
			msg_print(f_ptr->tunnel);
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
			msg_print(f_ptr->tunnel);
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
		}

		/* Keep trying */
		else
		{
			int feat;

			if (c_ptr->mimic) feat = c_ptr->mimic;
			else
				feat = c_ptr->feat;

			/* We may continue tunelling */
			msg_print(f_info[feat].tunnel);
			more = TRUE;
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
			msg_print(f_ptr->tunnel);
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
void do_cmd_tunnel()
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
		p_ptr->redraw |= (PR_FRAME);

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

		/* No tunnelling through air */
		if (cave_floor_grid(c_ptr))
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
	if (!more) disturb();
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
	auto const &r_info = game->edit_data.r_info;

	int bash, temp;

	cave_type *c_ptr;

	bool_ more = FALSE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_BASH_DOOR))
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
			cave_set_feat(y, x, FEAT_BROKEN);
		}

		/* Open the door */
		else
		{
			cave_set_feat(y, x, FEAT_OPEN);
		}

		/* Hack -- Fall through the door. Can't disarm while falling. */
		move_player_aux(dir, options->always_pickup, 0);

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
		set_paralyzed(2 + rand_int(2));
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
void do_cmd_bash()
{
	auto const &r_info = game->edit_data.r_info;

	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_BASH_DOOR))
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
		p_ptr->redraw |= (PR_FRAME);

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
	if (!more) disturb();
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
void do_cmd_alter()
{
	auto const &f_info = game->edit_data.f_info;

	int y, x, dir;

	cave_type *c_ptr;

	bool_ more = FALSE;


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_FRAME);

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
		else if (f_info[c_ptr->feat].flags & FF_TUNNELABLE)
		{
			/* Tunnel */
			more = do_cmd_tunnel_aux(y, x, dir);
		}

		/* Oops */
		else
		{
			/* Oops */
			msg_print("You attack the empty air.");
		}
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb();
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

		if (!o_ptr->k_ptr)
		{
			continue;
		}

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
void do_cmd_spike()
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


static void do_cmd_walk_jump(int pickup)
{
	auto const &wf_info = game->edit_data.wf_info;

	int dir;

	bool_ more = FALSE;


	/* Allow repeated command */
	if (command_arg)
	{
		/* Set repeat count */
		command_rep = command_arg - 1;

		/* Redraw the state */
		p_ptr->redraw |= (PR_FRAME);

		/* Cancel the arg */
		command_arg = 0;
	}

	/* Get a "repeated" direction */
	if (get_rep_dir(&dir))
	{
		/* Take a turn */
		energy_use = 100;

		/* Actually move the character */
		move_player(dir, pickup);

		/* Allow more walking */
		more = TRUE;
	}

	/* Hack -- In small scale wilderness it takes MUCH more time to move */
	energy_use *= (p_ptr->wild_mode) ? ((MAX_HGT + MAX_WID) / 2) : 1;

	/* Hack again -- Is there a special encounter ??? */
	auto const &wilderness = game->wilderness;
	if (p_ptr->wild_mode &&
	                magik(wf_info[wilderness(p_ptr->px, p_ptr->py).feat].level - (p_ptr->lev * 2)))
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
	if (!more) disturb();
}


/*
 * Try to ``walk'' using phase door.
 */
static void do_cmd_unwalk()
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
		p_ptr->redraw |= (PR_FRAME);

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
		move_player(dir, options->always_pickup);
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

		move_player(dir, options->always_pickup);
	}

	/* Walking semantics */
	else
	{
		teleport_player_directed(10, dir);
	}

	/* Cancel repetition unless we can continue */
	if (!more) disturb();
}


/*
 * Support code for the "Walk" and "Jump" commands
 */
void do_cmd_walk(int pickup)
{
	/* Move (usually pickup) */

	if (p_ptr->immovable)
	{
		do_cmd_unwalk();
	}
	else
	{
		do_cmd_walk_jump(pickup);
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
void do_cmd_run()
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
		p_ptr->redraw |= (PR_FRAME);

		/* Cancel the arg */
		command_arg = 0;
	}


	/* Take a turn */
	energy_use = 100;

	/* Handle "objects" */
	carry(pickup);


	/* Hack -- enter a store if we are on one */
	if (c_ptr->feat == FEAT_SHOP)
	{
		/* Disturb */
		disturb();

		/* Hack -- enter store */
		command_new = '_';
	}
}

/*
 * Resting allows a player to safely restore his hp	-RAK-
 */
void do_cmd_rest()
{
	auto const &f_info = game->edit_data.f_info;

	/* Can't rest on a Void Jumpgate -- too dangerous */
	if (cave[p_ptr->py][p_ptr->px].feat == FEAT_BETWEEN)
	{
		/* 'R&\n' is one of our favourite macros, so we have to do this */
		flush_on_failure();

		/* Tell the player why */
		msg_print(fmt::format(
			"Resting on a {} is too dangerous!",
			f_info[cave[p_ptr->py][p_ptr->px].feat].name));

		/* Done */
		return;
	}

	/* Can't rest while undead, it would mean dying */
	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		/* 'R&\n' is one of our favourite macros, so we have to do this */
		flush_on_failure();

		/* Tell the player why */
		msg_print("Resting is impossible while undead!");

		/* Done */
		return;
	}

	/* Prompt for time if needed */
	if (command_arg <= 0)
	{
		const char *p = "Rest (0-9999, '*' for HP/SP, '&' as needed): ";

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

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the state */
	p_ptr->redraw |= (PR_FRAME);

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
void do_cmd_fire()
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

	auto const msec = options->delay_factor_ms();


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
	if ((p_ptr->tval_ammo != o_ptr->tval) || (!o_ptr->k_ptr))
	{
		/* Get an item */
		if (!get_item(&item,
			      "Your quiver is empty.  Fire which item? ",
			      "You have nothing to fire.",
			      (USE_INVEN | USE_FLOOR),
			      object_filter::TVal(p_ptr->tval_ammo)))
		{
			return;
		}

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
				sleep_for(milliseconds(msec));
				lite_spot(y, x);
				Term_fresh();
			}

			/* The player cannot see the missile */
			else
			{
				/* Pause anyway, for consistancy */
				sleep_for(milliseconds(msec));
			}


			/* Monster here, Try to hit it */
			if (cave[y][x].m_idx)
			{
				cave_type *c_ptr = &cave[y][x];

				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				auto const r_ptr = m_ptr->race();

				/* Check the visibility */
				visible = m_ptr->ml;

				/* Note the collision */
				hit_body = TRUE;

				/* Did we hit it (penalize range) */
				if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
				{
					bool fear = false;

					/* Assume a default death */
					const char *note_dies = " dies.";

					/* Some monsters get "destroyed" */
					if ((r_ptr->flags & RF_DEMON) ||
					                (r_ptr->flags & RF_UNDEAD) ||
					                (r_ptr->flags & RF_STUPID) ||
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
void do_cmd_throw()
{
	int dir;

	s32b special = 0;

	int j, y, x, ny, nx, ty, tx;

	int chance, tdam, tdis;

	int mul, div;

	int boulder_add = 0;
	int boulder_mult = 0;

	int cur_dis, visible;

	object_type forge;

	object_type *q_ptr;

	bool_ hit_body = FALSE;

	bool_ hit_wall = FALSE;

	byte missile_attr;

	char missile_char;

	char o_name[80];

	auto const msec = options->delay_factor_ms();

	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Throw which item? ",
		      "You have nothing to throw.",
		      (USE_INVEN | USE_FLOOR)))
	{
		return;
	}

	/* Access the item */
	object_type *o_ptr = get_object(item);

	auto const flags = object_flags(o_ptr);

	/* Hack - Cannot throw away 'no drop' cursed items */
	if (cursed_p(o_ptr) && (flags & TR_CURSE_NO_DROP))
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
			sleep_for(milliseconds(msec));
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			sleep_for(milliseconds(msec));
		}


		/* Monster here, Try to hit it */
		if (cave[y][x].m_idx)
		{
			cave_type *c_ptr = &cave[y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			auto r_ptr = m_ptr->race();

			/* Check the visibility */
			visible = m_ptr->ml;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
			{
				bool fear = false;

				/* Assume a default death */
				const char *note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags & RF_DEMON) ||
				                (r_ptr->flags & RF_UNDEAD) ||
				                (r_ptr->flags & RF_STUPID) ||
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
					if (!(q_ptr->k_ptr->tval == TV_POTION))
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
	if (q_ptr->k_ptr->tval == TV_POTION)
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
void do_cmd_boomerang()
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

	auto const msec = options->delay_factor_ms();


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
			sleep_for(milliseconds(msec));
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			sleep_for(milliseconds(msec));
		}


		/* Monster here, Try to hit it */
		if (cave[y][x].m_idx)
		{
			cave_type *c_ptr = &cave[y][x];

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			auto const r_ptr = m_ptr->race();

			/* Check the visibility */
			visible = m_ptr->ml;

			/* Note the collision */
			hit_body = TRUE;

			/* Did we hit it (penalize range) */
			if (test_hit_fire(chance - cur_dis, m_ptr->ac, m_ptr->ml))
			{
				bool fear = false;

				/* Assume a default death */
				const char *note_dies = " dies.";

				/* Some monsters get "destroyed" */
				if ((r_ptr->flags & RF_DEMON) ||
				                (r_ptr->flags & RF_UNDEAD) ||
				                (r_ptr->flags & RF_STUPID) ||
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
					if (!(q_ptr->k_ptr->tval == TV_POTION))
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

						/* Get the monster name (or "it") */
						monster_desc(m_name, m_ptr, 0);

						/* Message */
						msg_format("%^s flees in terror!", m_name);
					}
				}

				/* Chance of breakage (during attacks) */
				j = (hit_body ? breakage_chance(o_ptr) : 0);

				/* Break the boomerang */
				if ((!artifact_p(o_ptr)) && (rand_int(100) < j))
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
			sleep_for(milliseconds(msec));
			lite_spot(y, x);
			Term_fresh();
		}

		/* The player cannot see the missile */
		else
		{
			/* Pause anyway, for consistancy */
			sleep_for(milliseconds(msec));
		}
	}
}


static bool_ tport_vertically(bool_ how)
{
	auto const &d_info = game->edit_data.d_info;
	auto const &dungeon_flags = game->dungeon_flags;

	/* quest? */
	if (p_ptr->inside_quest)
	{
		msg_print("There is no effect.");
		return (FALSE);
	}

	if (dungeon_flags & DF_NO_EASY_MOVE)
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
void do_cmd_immovable_special()
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
	while (true)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Do what special action:", 2, 0);

		/* Give some choices */
		prt("(a) Teleport to a specific place.", 4, 5);
		prt("(b) Fetch an item.", 5, 5);
		prt("(c) Go up one level", 6, 5);
		prt("(d) Go down one level", 7, 5);

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
			py_pickup_floor(options->always_pickup);

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
			p_ptr->redraw |= (PR_FRAME);
		}

		if (lose_hp)
		{
			p_ptr->chp -= lose_hp;
			p_ptr->redraw |= (PR_FRAME);
		}

		energy_use = 100;
	}
}

/* Can we sacrifice it ? */
static bool item_tester_hook_sacrificable(object_type const *o_ptr)
{
	if (p_ptr->pgod == GOD_MELKOR)
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
static bool item_tester_hook_sacrifice_aule(object_type const *o_ptr)
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

	if (!get_item(&item,
		      "Sacrifice which item? ",
		      "You have nothing to sacrifice.",
		      USE_INVEN,
		      item_tester_hook_sacrifice_aule))
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
void do_cmd_sacrifice()
{
	auto const &r_info = game->edit_data.r_info;

	byte on_what = cave[p_ptr->py][p_ptr->px].feat;

	/* Check valididty */
	if ((on_what < FEAT_ALTAR_HEAD) || (on_what > FEAT_ALTAR_TAIL))
	{
		show_god_info();
		return;
	}
	else
	{
		int agod = on_what - FEAT_ALTAR_HEAD + 1;

		/* Not worshipping a god ? ahhhh! */
		if (p_ptr->pgod == GOD_NONE)
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
			if (p_ptr->pgod == GOD_MELKOR)
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
					/* Get an item */
					int item;
					if (!get_item(&item,
						      "Sacrifice which item? ",
						      "You have nothing to sacrifice.",
						      (USE_INVEN),
						      item_tester_hook_sacrificable))
					{
						return;
					}

					object_type *o_ptr = get_object(item);

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

			if (p_ptr->pgod == GOD_AULE)
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
std::vector<s16b> scan_monst(int m_idx)
{
	constexpr std::size_t max_size = 23;

	/* Create output vector. */
	std::vector<s16b> objects;
	objects.reserve(std::min(max_size, m_list[m_idx].hold_o_idxs.size()));

	/* Scan all objects in the grid */
	for (auto const this_o_idx: m_list[m_idx].hold_o_idxs)
	{
		objects.push_back(this_o_idx);
		if (objects.size() == max_size) break;
	}

	/* Result */
	return objects;
}


/*
 * Display a list of the items that the given monster carries.
 * Returns the list of objects.
 */
std::vector<s16b> show_monster_inven(int m_idx)
{
	byte out_color[23];
	char out_desc[23][80];

	/* Default length */
	int len = 79 - 50;

	/* Maximum space allowed for descriptions */
	int lim = 79 - 3;

	/* Require space for weight */
	lim -= 9;

	/* Scan for objects on the monster */
	std::vector<s16b> objects = scan_monst(m_idx);
	assert(objects.size() <= 23);

	/* Calculate width of object names */
	for (std::size_t i = 0; i < objects.size(); i++)
	{
		object_type *o_ptr = &o_list[objects.at(i)];

		/* Describe the object */
		char o_name[80];
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Acquire p_ptr->inventory color */
		out_color[i] = tval_to_attr[o_ptr->tval & 0x7F];

		/* Save the object description */
		strcpy(out_desc[i], o_name);

		/* Find the predicted "line length" */
		int l = strlen(out_desc[i]) + 5;

		/* Account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;
	}

	/* Find the column to start in */
	int col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	std::size_t i = 0;
	for (i = 0; i < objects.size(); i++)
	{
		/* Get the item */
		object_type *o_ptr = &o_list[objects.at(i)];

		/* Clear the line */
		prt("", i + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		char tmp_val[80];
		strnfmt(tmp_val, 80, "%c)", index_to_label(i));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, i + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[i], out_desc[i], i + 1, col + 3);

		/* Display the weight if needed */
		{
			int wgt = o_ptr->weight * o_ptr->number;
			strnfmt(tmp_val, 80, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, i + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (i && (i < 23))
	{
		prt("", i + 1, col ? col - 2 : col);
	}

	return objects;
}


/*
 * Steal an object from a monster
 */
void do_cmd_steal()
{
	auto const &r_info = game->edit_data.r_info;

	int dir = 0, item = -1, k = -1;

	bool_ done = FALSE;

	/* Only works on adjacent monsters */
	if (!get_rep_dir(&dir)) return;
	int y = p_ptr->py + ddy[dir];
	int x = p_ptr->px + ddx[dir];

	cave_type const *c_ptr = &cave[y][x];

	if (!(c_ptr->m_idx))
	{
		msg_print("There is no monster there!");
		return;
	}

	monster_type *m_ptr = &m_list[c_ptr->m_idx];

	/* There were no non-gold items */
	if (m_ptr->hold_o_idxs.empty())
	{
		msg_print("That monster has no objects!");
		return;
	}

	/* The monster is immune */
	if (r_info[m_ptr->r_idx].flags & RF_NO_THEFT)
	{
		msg_print("The monster is guarding the treasures.");
		return;
	}

	screen_save();

	std::vector<s16b> objects = show_monster_inven(c_ptr->m_idx);

	/* Repeat until done */
	while (!done)
	{
		char tmp_val[80];
		char which = ' ';

		/* Build the prompt */
		strnfmt(tmp_val, 80, "Choose an item to steal (a-%c) or ESC:",
			'a' - 1 + objects.size());

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
				if ((k < 0) || (static_cast<std::size_t>(k) >= objects.size()))
				{
					bell();

					break;
				}

				/* Verify the item */
				if (ver && !verify("Try", -objects[k]))
				{
					done = TRUE;

					break;
				}

				/* Accept that choice */
				item = objects[k];
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

		/* Remove from the monster's list of objects */
		m_ptr->hold_o_idxs.erase(m_ptr->hold_o_idxs.begin() + k);

		/* Rogues gain some xp */
		if (race_flags_p(PR_EASE_STEAL))
		{
			s32b max_point;

			/* Max XP gained from stealing */
			max_point = (o_list[item].weight / 2) + (m_ptr->level * 10);

			/* Randomise it a bit, with half a max guaranteed */
			gain_exp((max_point / 2) + (randint(max_point) / 2));

			/* Allow escape */
			if (get_check("Phase door?")) teleport_player(10);
		}

		/* Create the object we're going to copy into */
		object_type forge;
		object_type *o_ptr = &forge;

		/* Special handling for gold */
		if (o_list[item].tval == TV_GOLD)
		{
			/* Collect the gold */
			p_ptr->au += o_list[item].pval;

			/* Redraw gold */
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
		else
		{
			object_copy(o_ptr, &o_list[item]);

			inven_carry(o_ptr, FALSE);
		}

		/* Delete source item */
		o_list[item].k_ptr.reset();
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
	/* Get a "repeated" direction */
	int dir;
	if (!get_rep_dir(&dir)) return;

	/* Get requested location */
	int y = p_ptr->py + ddy[dir];
	int x = p_ptr->px + ddx[dir];

	/* Get requested grid */
	cave_type *c_ptr = &cave[y][x];

	/* No monster in the way */
	if (c_ptr->m_idx == 0)
	{
		msg_print("There is no monster there.");
		return;
	}

	/* Get an item */
	int item;
	if (!get_item(&item,
		      "What item do you want to offer? ",
		      "You have nothing to offer.",
		      USE_INVEN))
	{
		return;
	}

	/* Process hooks if there are any */
	hook_give_in in = { c_ptr->m_idx, item };
	if (!process_hooks_new(HOOK_GIVE, &in, NULL))
	{
		msg_print("The monster does not want your item.");
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
	struct hook_chat_in in = { c_ptr->m_idx };
	if (!process_hooks_new(HOOK_CHAT, &in, NULL))
	{
		msg_print("The monster does not want to chat.");
	}

	/* No energy spent */
}
