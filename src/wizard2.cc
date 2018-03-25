/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "wizard2.hpp"

#include "artifact_type.hpp"
#include "birth.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd4.hpp"
#include "corrupt.hpp"
#include "dungeon_info_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_flag_meta.hpp"
#include "object_kind.hpp"
#include "player_type.hpp"
#include "randart.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
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
#include "z-rand.hpp"

/*
 * Adds a lvl to a monster
 */
static void wiz_inc_monster_level(int level)
{
	monster_type *m_ptr;
	int ii, jj;

	if (!tgt_pt(&ii, &jj)) return;

	if (cave[jj][ii].m_idx)
	{
		m_ptr = &m_list[cave[jj][ii].m_idx];

		m_ptr->exp = monster_exp(m_ptr->level + level);
		monster_check_experience(cave[jj][ii].m_idx, FALSE);
	}
}

static void wiz_align_monster(int status)
{
	monster_type *m_ptr;
	int ii, jj;

	if (!tgt_pt(&ii, &jj)) return;

	if (cave[jj][ii].m_idx)
	{
		m_ptr = &m_list[cave[jj][ii].m_idx];

		m_ptr->status = status;
	}
}

/*
 * Teleport directly to a town
 */
static void teleport_player_town(int town)
{
	auto const &wf_info = game->edit_data.wf_info;

	autosave_checkpoint();

	/* Change town */
	dun_level = 0;
	p_ptr->town_num = town;

	auto const &wilderness = game->wilderness;
	for (std::size_t y = 0; y < wilderness.height(); y++)
	{
		for (std::size_t x = 0; x < wilderness.width(); x++)
		{
			if (p_ptr->town_num == wf_info[wilderness(x, y).feat].entrance)
			{
				p_ptr->wilderness_y = y;
				p_ptr->wilderness_x = x;

				leaving_quest = p_ptr->inside_quest;
				p_ptr->inside_quest = 0;

				/* Leaving */
				p_ptr->leaving = TRUE;

				// Done
				return;
			}
		}
	}
}


/*
 * Hack -- Rerate Hitpoints
 */
void do_cmd_rerate()
{
	auto &player_hp = game->player_hp;

	// Force HP re-roll
	roll_player_hp();

	// Calculate life rating
	int percent = static_cast<int>(
	        (static_cast<long>(player_hp[PY_MAX_LEVEL - 1]) * 200L) /
	                (p_ptr->hitdie + ((PY_MAX_LEVEL - 1) * p_ptr->hitdie)));

	/* Update and redraw hitpoints */
	p_ptr->update |= (PU_HP);
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle stuff */
	handle_stuff();

	/* Message */
	msg_format("Current Life Rating is %d/100.", percent);
}


/*
 * Create the artifact of the specified number -- DAN
 *
 */
static void wiz_create_named_art()
{
	auto const &a_info = game->edit_data.a_info;

	object_type forge;
	object_type *q_ptr;
	int i, a_idx;
	cptr p = "Number of the artifact: ";
	char out_val[80] = "";

	if (!get_string(p, out_val, 4)) return;
	a_idx = atoi(out_val);

	/* Return if out-of-bounds */
	if ((a_idx <= 0) || (a_idx >= static_cast<int>(a_info.size())))
	{
		return;
	}

	auto a_ptr = &a_info[a_idx];

	/* Get local object */
	q_ptr = &forge;

	/* Wipe the object */
	object_wipe(q_ptr);

	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return;

	/* Acquire the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return;

	/* Create the artifact */
	object_prep(q_ptr, i);

	/* Save the name */
	q_ptr->name1 = a_idx;

	apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

	/* Apply any random resistances/powers */
	random_artifact_resistance(q_ptr);

	/* Drop the artifact from heaven */
	drop_near(q_ptr, -1, p_ptr->py, p_ptr->px);

	/* All done */
	msg_print("Allocated.");
}

/* Summon a horde of monsters */
static void do_cmd_summon_horde()
{
	int wy = p_ptr->py, wx = p_ptr->px;
	int attempts = 1000;

	while (--attempts)
	{
		scatter(&wy, &wx, p_ptr->py, p_ptr->px, 3);
		if (cave_naked_bold(wy, wx)) break;
	}

	alloc_horde(wy, wx);
}


/*
 * Hack -- Teleport to the target
 */
static void do_cmd_wiz_bamf()
{
	/* Must have a target */
	if (!target_who) return;

	/* Teleport to the target */
	teleport_player_to(target_row, target_col);
}


/*
 * Aux function for "do_cmd_wiz_change()".	-RAK-
 */
static void do_cmd_wiz_change_aux()
{
	int i;
	int tmp_int;
	long tmp_long;
	char tmp_val[160];
	char ppp[80];


	/* Query the stats */
	for (i = 0; i < 6; i++)
	{
		/* Prompt */
		sprintf(ppp, "%s:  (3-118): ", stat_names[i]);

		/* Default */
		sprintf(tmp_val, "%d", p_ptr->stat_max[i]);

		/* Query */
		if (!get_string(ppp, tmp_val, 3)) return;

		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Verify */
		if (tmp_int > 18 + 100) tmp_int = 18 + 100;
		else if (tmp_int < 3) tmp_int = 3;

		/* Save it */
		p_ptr->stat_cur[i] = p_ptr->stat_max[i] = tmp_int;
	}


	/* Default */
	sprintf(tmp_val, "%ld", (long)(p_ptr->au));

	/* Query */
	if (!get_string("Gold: ", tmp_val, 9)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->au = tmp_long;


	/* Default */
	sprintf(tmp_val, "%ld", (long)(p_ptr->max_exp));

	/* Query */
	if (!get_string("Experience: ", tmp_val, 9)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->max_exp = tmp_long;
	p_ptr->exp = tmp_long;

	/* Update */
	check_experience();


	/* Default */
	sprintf(tmp_val, "%ld", (long) (p_ptr->grace));

	/* Query */
	if (!get_string("Piety: ", tmp_val, 9)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Verify */
	if (tmp_long < 0) tmp_long = 0L;

	/* Save */
	p_ptr->grace = tmp_long;


	/* Default */
	sprintf(tmp_val, "%d", p_ptr->luck_base);

	/* Query */
	if (!get_string("Luck(base): ", tmp_val, 3)) return;

	/* Extract */
	tmp_long = atol(tmp_val);

	/* Save */
	p_ptr->luck_base = tmp_long;
	p_ptr->luck_max = tmp_long;
}


/*
 * Change various "permanent" player variables.
 */
static void do_cmd_wiz_change()
{
	/* Interact */
	do_cmd_wiz_change_aux();

	/* Redraw everything */
	do_cmd_redraw();
}


/*
 * Wizard routines for creating objects		-RAK-
 * And for manipulating them!                   -Bernd-
 *
 * This has been rewritten to make the whole procedure
 * of debugging objects much easier and more comfortable.
 *
 * The following functions are meant to play with objects:
 * Create, modify, roll for them (for statistic purposes) and more.
 * The original functions were by RAK.
 * The function to show an item's debug information was written
 * by David Reeve Sward <sward+@CMU.EDU>.
 *                             Bernd (wiebelt@mathematik.hu-berlin.de)
 *
 * Here are the low-level functions
 * - wiz_display_item()
 *     display an item's debug-info
 * - wiz_create_itemtype()
 *     specify tval and sval (type and subtype of object)
 * - wiz_tweak_item()
 *     specify pval, +AC, +tohit, +todam
 *     Note that the wizard can leave this function anytime,
 *     thus accepting the default-values for the remaining values.
 *     pval comes first now, since it is most important.
 * - wiz_reroll_item()
 *     apply some magic to the item or turn it into an artifact.
 * - wiz_roll_item()
 *     Get some statistics about the rarity of an item:
 *     We create a lot of fake items and see if they are of the
 *     same type (tval and sval), then we compare pval and +AC.
 *     If the fake-item is better or equal it is counted.
 *     Note that cursed items that are better or equal (absolute values)
 *     are counted, too.
 *     HINT: This is *very* useful for balancing the game!
 * - wiz_quantity_item()
 *     change the quantity of an item, but be sane about it.
 *
 * And now the high-level functions
 * - do_cmd_wiz_play()
 *     play with an existing object
 * - wiz_create_item()
 *     create a new object
 *
 * Note -- You do not have to specify "pval" and other item-properties
 * directly. Just apply magic until you are satisfied with the item.
 *
 * Note -- For some items (such as wands, staffs, some rings, etc), you
 * must apply magic, or you will get "broken" or "uncharged" objects.
 *
 * Note -- Redefining artifacts via "do_cmd_wiz_play()" may destroy
 * the artifact.  Be careful.
 *
 * Hack -- this function will allow you to create multiple artifacts.
 * This "feature" may induce crashes or other nasty effects.
 */

/*
 * Just display an item's properties (debug-info)
 * Originally by David Reeve Sward <sward+@CMU.EDU>
 * Verbose item flags by -Bernd-
 */
static void wiz_display_item(object_type *o_ptr)
{
	int i, j = 13;
	char buf[256];

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);

	/* Clear the screen */
	for (i = 1; i <= 23; i++) prt("", i, j - 2);

	/* Describe fully */
	object_desc_store(buf, o_ptr, TRUE, 3);

	prt(buf, 2, j);

	prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
		   o_ptr->k_ptr->idx,
		   o_ptr->k_ptr->level,
		   o_ptr->tval,
		   o_ptr->sval), 4, j);

	prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
	           o_ptr->number, o_ptr->weight,
	           o_ptr->ac, o_ptr->dd, o_ptr->ds), 5, j);

	prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
	           o_ptr->pval, o_ptr->to_a, o_ptr->to_h, o_ptr->to_d), 6, j);

	prt(format("name1 = %-4d  name2 = %-4d  cost = %ld  pval2 = %-5d",
	           o_ptr->name1, o_ptr->name2, (long)object_value(o_ptr), o_ptr->pval2), 7, j);

	prt(format("ident = %04x  timeout = %-d",
	           o_ptr->ident, o_ptr->timeout), 8, j);

	/* Print all the flags which are set */
	prt("Flags:", 10, j);

	int const row0 = 11;
	int row = row0;
	int col = 0;
	for (auto const &object_flag_meta: object_flags_meta())
	{
		// Is the flag set?
		if (object_flag_meta->flag_set & flags)
		{
			// Advance to next row/column
			row += 1;
			if (row >= 23) {
				row = row0 + 1;
				col += 1;
			}
			// Display
			prt(object_flag_meta->name, row, j + 1 + 20 * col);
		}
	}
}



/*
 * Strip an "object name" into a buffer
 */
static std::string strip_name(std::string const &str)
{
	/* Skip past leading characters */
	std::size_t i = 0;
	while ((str[i] == ' ') || (str[i] == '&'))
	{
		i++;
	}

	/* Copy useful chars */
	std::string buf;
	buf.reserve(str.size() - i);
	for (; i < str.size(); i++)
	{
		if (str[i] != '~')
		{
			buf += str[i];
		}
	}

	return buf;
}


/*
 * Hack -- title for each column
 *
 * XXX XXX XXX This will not work with "EBCDIC", I would think.
 */
static char head[4] = { 'a', 'A', '0', ':' };


/*
 * Print a string as required by wiz_create_itemtype().
 * Trims characters from the beginning until it fits in the space
 * before the next row or the edge of the screen.
 */
static void wci_string(cptr string, int num)
{
	int row = 2 + (num % 20), col = 30 * (num / 20);
	int ch = head[num / 20] + (num % 20), max_len = 0;

	if (76-col < (signed)max_len)
		max_len = 76-col;
	else
		max_len = 30-6;

	if (strlen(string) > (unsigned)max_len)
		string = string + (strlen(string) - max_len);

	prt(format("[%c] %s", ch, string), row, col);
}

/*
 * Specify tval and sval (type and subtype of object) originally
 * by RAK, heavily modified by -Bernd-
 *
 * This function returns the k_idx of an object type, or zero if failed
 *
 * List up to 50 choices in three columns
 */
static int wiz_create_itemtype()
{
	auto const &k_info = game->edit_data.k_info;

	int num, max_num;
	int tval;

	cptr tval_desc2;
	char ch;

	/* Clear screen */
	Term_clear();

	/* Print all tval's and their descriptions */
	for (num = 0; (num < 60) && tvals[num].tval; num++)
	{
		wci_string(tvals[num].desc, num);
	}

	/* Me need to know the maximal possible tval_index */
	max_num = num;

	/* Choose! */
	if (!get_com("Get what type of object? ", &ch)) return (0);

	/* Analyze choice */
	num = -1;
	if ((ch >= head[0]) && (ch < head[0] + 20)) num = ch - head[0];
	if ((ch >= head[1]) && (ch < head[1] + 20)) num = ch - head[1] + 20;
	if ((ch >= head[2]) && (ch < head[2] + 17)) num = ch - head[2] + 40;

	/* Bail out if choice is illegal */
	if ((num < 0) || (num >= max_num)) return (0);

	/* Base object type chosen, fill in tval */
	tval = tvals[num].tval;
	tval_desc2 = tvals[num].desc;


	/*** And now we go for k_idx ***/

	/* Clear screen */
	Term_clear();

	/* We have to search the whole itemlist. */
	std::vector<std::size_t> choice;
	choice.reserve(60);
	for (auto &k_entry: k_info)
	{
		auto const &k_ptr = k_entry.second;

		/* Analyze matching items */
		if (k_ptr->tval == tval)
		{
			/* Hack -- Skip instant artifacts */
			if (k_ptr->flags & TR_INSTA_ART)
			{
				continue;
			}

			/* Acquire the "name" of object */
			auto buf = strip_name(k_ptr->name);

			/* Print it */
			wci_string(buf.c_str(), choice.size());

			/* Remember the object index */
			choice.push_back(k_entry.first);
			if (choice.size() >= 60)
			{
				break;
			}
		}
	}

	/* Me need to know the maximal possible remembered object_index */
	max_num = choice.size();

	/* Choose! */
	if (!get_com(format("What Kind of %s? ", tval_desc2), &ch)) return (0);

	/* Analyze choice */
	num = -1;
	if ((ch >= head[0]) && (ch < head[0] + 20)) num = ch - head[0];
	if ((ch >= head[1]) && (ch < head[1] + 20)) num = ch - head[1] + 20;
	if ((ch >= head[2]) && (ch < head[2] + 17)) num = ch - head[2] + 40;

	/* Bail out if choice is "illegal" */
	if ((num < 0) || (num >= max_num)) return (0);

	/* And return successful */
	return (choice[num]);
}


/*
 * Tweak an item
 */
static void wiz_tweak_item(object_type *o_ptr)
{
	cptr p;
	char tmp_val[80];

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);


	p = "Enter new 'pval' setting: ";
	sprintf(tmp_val, "%ld", (long int) o_ptr->pval);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->pval = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'pval2' setting: ";
	sprintf(tmp_val, "%d", o_ptr->pval2);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->pval2 = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'pval3' setting: ";
	sprintf(tmp_val, "%ld", (long int) o_ptr->pval3);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->pval3 = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_a' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_a);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_a = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_h' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_h);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_h = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'to_d' setting: ";
	sprintf(tmp_val, "%d", o_ptr->to_d);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->to_d = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'name2' setting: ";
	sprintf(tmp_val, "%d", o_ptr->name2);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->name2 = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'name2b' setting: ";
	sprintf(tmp_val, "%d", o_ptr->name2b);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->name2b = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'sval' setting: ";
	sprintf(tmp_val, "%d", o_ptr->sval);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->sval = atoi(tmp_val);
	wiz_display_item(o_ptr);

	p = "Enter new 'obj exp' setting: ";
	sprintf(tmp_val, "%ld", (long int) o_ptr->exp);
	if (!get_string(p, tmp_val, 9)) return;
	wiz_display_item(o_ptr);
	o_ptr->exp = atoi(tmp_val);
	if (flags & TR_LEVELS) check_experience_obj(o_ptr);

	p = "Enter new 'timeout' setting: ";
	sprintf(tmp_val, "%d", o_ptr->timeout);
	if (!get_string(p, tmp_val, 5)) return;
	o_ptr->timeout = atoi(tmp_val);
	wiz_display_item(o_ptr);
}


/*
 * Apply magic to an item or turn it into an artifact. -Bernd-
 */
static void wiz_reroll_item(object_type *o_ptr)
{
	object_type forge;
	object_type *q_ptr;

	char ch;

	bool_ changed = FALSE;


	/* Hack -- leave artifacts alone */
	if (artifact_p(o_ptr)) return;

	/* Get the kind index */
	auto const k_idx = o_ptr->k_ptr->idx;

	/* Get local object */
	q_ptr = &forge;

	/* Copy the object */
	object_copy(q_ptr, o_ptr);


	/* Main loop. Ask for magification and artifactification */
	while (TRUE)
	{
		/* Display full item debug information */
		wiz_display_item(q_ptr);

		/* Ask wizard what to do. */
		if (!get_com("[a]ccept, [b]ad, [n]ormal, [g]ood, [e]xcellent, [r]andart? ", &ch))
		{
			changed = FALSE;
			break;
		}

		/* Create/change it! */
		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}

		/* Apply bad magic, but first clear object */
		else if (ch == 'b' || ch == 'B')
		{
			object_prep(q_ptr, k_idx);
			apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE, boost::make_optional(-2));
		}

		/* Apply normal magic, but first clear object */
		else if (ch == 'n' || ch == 'N')
		{
			object_prep(q_ptr, k_idx);
			apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
		}

		/* Apply good magic, but first clear object */
		else if (ch == 'g' || ch == 'g')
		{
			object_prep(q_ptr, k_idx);
			apply_magic(q_ptr, dun_level, FALSE, TRUE, FALSE);
		}

		/* Apply great magic, but first clear object */
		else if (ch == 'e' || ch == 'e')
		{
			object_prep(q_ptr, k_idx);
			apply_magic(q_ptr, dun_level, FALSE, TRUE, TRUE);
		}

		/* Apply great magic, but first clear object */
		else if (ch == 'r' || ch == 'r')
		{
			object_prep(q_ptr, k_idx);
			create_artifact(q_ptr, FALSE, TRUE);
		}
	}


	/* Notice change */
	if (changed)
	{
		/* Apply changes */
		object_copy(o_ptr, q_ptr);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}
}



/*
 * Maximum number of rolls
 */
#define TEST_ROLL 100000


/*
 * Try to create an item again. Output some statistics.    -Bernd-
 *
 * The statistics are correct now.  We acquire a clean grid, and then
 * repeatedly place an object in this grid, copying it into an item
 * holder, and then deleting the object.  We fiddle with the artifact
 * counter flags to prevent weirdness.  We use the items to collect
 * statistics on item creation relative to the initial item.
 */
static void wiz_statistics(object_type *o_ptr)
{
	auto &a_info = game->edit_data.a_info;
	auto &random_artifacts = game->random_artifacts;

	long i, matches, better, worse, other;

	char ch;
	const char *quality;

	bool_ good, great;

	object_type forge;
	object_type	*q_ptr;

	cptr q = "Rolls: %ld, Matches: %ld, Better: %ld, Worse: %ld, Other: %ld";

	/* XXX XXX XXX Mega-Hack -- allow multiple artifacts */
	if (artifact_p(o_ptr))
	{
		if (o_ptr->tval == TV_RANDART)
		{
			random_artifacts[o_ptr->sval].generated = FALSE;
		}
		else
		{
			a_info[o_ptr->name1].cur_num = 0;
		}
	}


	/* Interact */
	while (TRUE)
	{
		cptr pmt = "Roll for [n]ormal, [g]ood, or [e]xcellent treasure? ";

		/* Display item */
		wiz_display_item(o_ptr);

		/* Get choices */
		if (!get_com(pmt, &ch)) break;

		if (ch == 'n' || ch == 'N')
		{
			good = FALSE;
			great = FALSE;
			quality = "normal";
		}
		else if (ch == 'g' || ch == 'G')
		{
			good = TRUE;
			great = FALSE;
			quality = "good";
		}
		else if (ch == 'e' || ch == 'E')
		{
			good = TRUE;
			great = TRUE;
			quality = "excellent";
		}
		else
		{
			good = FALSE;
			great = FALSE;
			break;
		}

		/* Let us know what we are doing */
		msg_format("Creating a lot of %s items. Base level = %d.",
		           quality, dun_level);
		msg_print(NULL);

		/* Set counters to zero */
		matches = better = worse = other = 0;

		/* Let's rock and roll */
		for (i = 0; i <= TEST_ROLL; i++)
		{
			/* Output every few rolls */
			if ((i < 100) || (i % 100 == 0))
			{
				/* Allow interupt */
				if (inkey_scan())
				{
					/* Flush */
					flush();

					/* Stop rolling */
					break;
				}

				/* Dump the stats */
				prt(format(q, i, matches, better, worse, other), 0, 0);
				Term_fresh();
			}


			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Create an object */
			make_object(q_ptr, good, great, obj_theme::defaults());


			/* XXX XXX XXX Mega-Hack -- allow multiple artifacts */
			if (artifact_p(q_ptr))
			{
				if (q_ptr->tval == TV_RANDART)
				{
					random_artifacts[q_ptr->sval].generated = FALSE;
				}
				else
				{
					a_info[q_ptr->name1].cur_num = 0;
				}
			}


			/* Test for the same tval and sval. */
			if ((o_ptr->tval) != (q_ptr->tval)) continue;
			if ((o_ptr->sval) != (q_ptr->sval)) continue;

			/* Check for match */
			if ((q_ptr->pval == o_ptr->pval) &&
			                (q_ptr->to_a == o_ptr->to_a) &&
			                (q_ptr->to_h == o_ptr->to_h) &&
			                (q_ptr->to_d == o_ptr->to_d))
			{
				matches++;
			}

			/* Check for better */
			else if ((q_ptr->pval >= o_ptr->pval) &&
			                (q_ptr->to_a >= o_ptr->to_a) &&
			                (q_ptr->to_h >= o_ptr->to_h) &&
			                (q_ptr->to_d >= o_ptr->to_d))
			{
				better++;
			}

			/* Check for worse */
			else if ((q_ptr->pval <= o_ptr->pval) &&
			                (q_ptr->to_a <= o_ptr->to_a) &&
			                (q_ptr->to_h <= o_ptr->to_h) &&
			                (q_ptr->to_d <= o_ptr->to_d))
			{
				worse++;
			}

			/* Assume different */
			else
			{
				other++;
			}
		}

		/* Final dump */
		msg_format(q, i, matches, better, worse, other);
		msg_print(NULL);
	}


	/* Hack -- Normally only make a single artifact */
	if (artifact_p(o_ptr))
	{
		if (o_ptr->tval == TV_RANDART)
		{
			random_artifacts[o_ptr->sval].generated = TRUE;
		}
		else
		{
			a_info[o_ptr->name1].cur_num = 1;
		}
	}
}


/*
 * Change the quantity of a the item
 */
static void wiz_quantity_item(object_type *o_ptr)
{
	int tmp_int;
	char tmp_val[100];

	/* Default */
	sprintf(tmp_val, "%d", o_ptr->number);

	/* Query */
	if (get_string("Quantity: ", tmp_val, 2))
	{
		/* Extract */
		tmp_int = atoi(tmp_val);

		/* Paranoia */
		if (tmp_int < 1) tmp_int = 1;
		if (tmp_int > 99) tmp_int = 99;

		/* Accept modifications */
		o_ptr->number = tmp_int;
	}
}



/*
 * Play with an item. Options include:
 *   - Output statistics (via wiz_roll_item)
 *   - Reroll item (via wiz_reroll_item)
 *   - Change properties (via wiz_tweak_item)
 *   - Change the number of items (via wiz_quantity_item)
 */
static void do_cmd_wiz_play()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Play with which object? ",
		      "You have nothing to play with.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);


	/* The item was not changed */
	bool_ changed = FALSE;


	/* Icky */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Get local object */
	object_type forge;
	object_type *q_ptr = &forge;

	/* Copy object */
	object_copy(q_ptr, o_ptr);


	/* The main loop */
	while (TRUE)
	{
		char ch;

		/* Display the item */
		wiz_display_item(q_ptr);

		/* Get choice */
		if (!get_com("[a]ccept [s]tatistics [r]eroll [t]weak [q]uantity apply[m]agic? ", &ch))
		{
			changed = FALSE;
			break;
		}

		if (ch == 'A' || ch == 'a')
		{
			changed = TRUE;
			break;
		}

		if (ch == 's' || ch == 'S')
		{
			wiz_statistics(q_ptr);
		}

		if (ch == 'r' || ch == 'r')
		{
			wiz_reroll_item(q_ptr);
		}

		if (ch == 't' || ch == 'T')
		{
			wiz_tweak_item(q_ptr);
		}

		if (ch == 'q' || ch == 'Q')
		{
			wiz_quantity_item(q_ptr);
		}

		if (ch == 'm' || ch == 'M')
		{
			int e = q_ptr->name2, eb = q_ptr->name2b;

			object_prep(q_ptr, q_ptr->k_ptr->idx);
			q_ptr->name2 = e;
			q_ptr->name2b = eb;
			apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
		}
	}


	/* Restore the screen */
	Term_load();

	/* Not Icky */
	character_icky = FALSE;


	/* Accept change */
	if (changed)
	{
		/* Message */
		msg_print("Changes accepted.");

		/* Change */
		object_copy(o_ptr, q_ptr);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	/* Ignore change */
	else
	{
		msg_print("Changes ignored.");
	}
}


/*
 * Wizard routine for creating objects		-RAK-
 * Heavily modified to allow magification and artifactification  -Bernd-
 *
 * Note that wizards cannot create objects on top of other objects.
 *
 * Hack -- this routine always makes a "dungeon object", and applies
 * magic to it, and attempts to decline cursed items.
 */
static void wiz_create_item()
{
	object_type	forge;
	object_type *q_ptr;

	int k_idx;


	/* Icky */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Get object base type */
	k_idx = wiz_create_itemtype();

	/* Restore the screen */
	Term_load();

	/* Not Icky */
	character_icky = FALSE;


	/* Return if failed */
	if (!k_idx) return;

	/* Get local object */
	q_ptr = &forge;

	/* Create the item */
	object_prep(q_ptr, k_idx);

	/* Apply magic (no messages, no artifacts) */
	apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);

	/* Drop the object from heaven */
	drop_near(q_ptr, -1, p_ptr->py, p_ptr->px);

	/* All done */
	msg_print("Allocated.");
}

/*
 * As above, but takes the k_idx as a parameter instead of using menus.
 */
static void wiz_create_item_2()
{
	auto const &k_info = game->edit_data.k_info;

	cptr p = "Number of the object :";
	char out_val[80] = "";

	if (!get_string(p, out_val, 4)) return;
	int k_idx = atoi(out_val);

	/* Return if failed or out-of-bounds */
	if (!k_info.count(k_idx))
	{
		return;
	}

	/* Get local object */
	object_type forge;
	auto q_ptr = &forge;

	/* Create the item */
	object_prep(q_ptr, k_idx);

	/* Apply magic (no messages, no artifacts) */
	apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);

	/* Drop the object from heaven */
	drop_near(q_ptr, -1, p_ptr->py, p_ptr->px);

	/* All done */
	msg_print("Allocated.");
}


/*
 * Cure everything instantly
 */
void do_cmd_wiz_cure_all()
{
	object_type *o_ptr;

	/* Remove curses */
	remove_all_curse();

	/* Restore stats */
	res_stat(A_STR, TRUE);
	res_stat(A_INT, TRUE);
	res_stat(A_WIS, TRUE);
	res_stat(A_CON, TRUE);
	res_stat(A_DEX, TRUE);
	res_stat(A_CHR, TRUE);

	/* Restore the level */
	restore_level();

	/* Heal the player */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Cure insanity of player */
	p_ptr->csane = p_ptr->msane;
	p_ptr->csane_frac = 0;

	/* Heal the player monster */
	/* Get the carried monster */
	o_ptr = &p_ptr->inventory[INVEN_CARRY];
	if (o_ptr->k_ptr)
	{
		o_ptr->pval2 = o_ptr->pval3;
	}

	/* Restore mana */
	p_ptr->csp = p_ptr->msp;
	p_ptr->csp_frac = 0;

	/* Cure stuff */
	set_blind(0);
	set_confused(0);
	set_poisoned(0);
	set_afraid(0);
	set_paralyzed(0);
	set_image(0);
	set_stun(0);
	set_cut(0);
	set_slow(0);
	p_ptr->black_breath = FALSE;

	/* No longer hungry */
	set_food(PY_FOOD_MAX - 1);

	/* Redraw everything */
	do_cmd_redraw();
}


/*
 * Go to any level
 */
static void do_cmd_wiz_jump()
{
	auto const &d_info = game->edit_data.d_info;

	/* Ask for level */
	if (command_arg <= 0)
	{
		char	ppp[80];

		char	tmp_val[160];

		/* Prompt */
		msg_format("dungeon_type : %d", dungeon_type);
		sprintf(ppp, "Jump to level (0-%d): ", d_info[dungeon_type].maxdepth);

		/* Default */
		sprintf(tmp_val, "%d", dun_level);

		/* Ask for a level */
		if (!get_string(ppp, tmp_val, 10)) return;

		/* Extract request */
		command_arg = atoi(tmp_val);
	}

	/* Paranoia */
	if (command_arg < 0) command_arg = 0;

	/* Paranoia */
	if (command_arg > d_info[dungeon_type].maxdepth) command_arg = d_info[dungeon_type].maxdepth;

	/* Accept request */
	msg_format("You jump to dungeon level %d.", command_arg);

	autosave_checkpoint();

	/* Change level */
	dun_level = command_arg;

	leaving_quest = p_ptr->inside_quest;

	p_ptr->inside_quest = 0;

	/* Leaving */
	p_ptr->leaving = TRUE;
}


/*
 * Summon some creatures
 */
static void do_cmd_wiz_summon(int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		summon_specific(p_ptr->py, p_ptr->px, dun_level, 0);
	}
}


/*
 * Summon a creature of the specified type
 *
 * XXX XXX XXX This function is rather dangerous
 */
static void do_cmd_wiz_named(std::size_t r_idx, bool_ slp)
{
	auto const &r_info = game->edit_data.r_info;

	int i, x, y;

	/* Paranoia */
	/* if (!r_idx) return; */

	/* Prevent illegal monsters */
	if (r_idx >= r_info.size()) return;

	/* Try 10 times */
	for (i = 0; i < 10; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, p_ptr->py, p_ptr->px, d);

		/* Require empty grids */
		if (!cave_empty_bold(y, x)) continue;

		/* Place it (allow groups) */
		m_allow_special[r_idx] = TRUE;
		int m_idx = place_monster_aux(y, x, r_idx, slp, TRUE, MSTATUS_ENEMY);
		m_allow_special[r_idx] = FALSE;

		// If summoning succeeded, we stop.
		if (m_idx)
		{
			break;
		}
	}
}


/*
 * Summon a creature of the specified type
 *
 * XXX XXX XXX This function is rather dangerous
 */
void do_cmd_wiz_named_friendly(std::size_t r_idx, bool_ slp)
{
	auto const &r_info = game->edit_data.r_info;

	int i, x, y;

	/* Paranoia */
	/* if (!r_idx) return; */

	/* Prevent illegal monsters */
	if (r_idx >= r_info.size()) return;

	/* Try 10 times */
	for (i = 0; i < 10; i++)
	{
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, p_ptr->py, p_ptr->px, d);

		/* Require empty grids */
		if (!cave_empty_bold(y, x)) continue;

		/* Place it (allow groups) */
		m_allow_special[r_idx] = TRUE;
		int m_idx = place_monster_aux(y, x, r_idx, slp, TRUE, MSTATUS_PET);
		m_allow_special[r_idx] = FALSE;

		// Stop if we succeeded
		if (m_idx)
		{
			break;
		}
	}
}



/*
 * Hack -- Delete all nearby monsters
 */
static void do_cmd_wiz_zap()
{
	int i;

	/* Genocide everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Delete nearby monsters */
		if (m_ptr->cdis <= MAX_SIGHT) delete_monster_idx(i);
	}
}


static void do_cmd_wiz_body(s16b bidx)
	/* Might create problems with equipment slots. For safety,
	be nude when calling this function */
{
	auto const &r_info = game->edit_data.r_info;

	p_ptr->body_monster = bidx;
	p_ptr->disembodied = FALSE;
	p_ptr->chp = maxroll( (&r_info[bidx])->hdice, (&r_info[bidx])->hside);
	do_cmd_redraw();
}


/*
 * Ask for and parse a "debug command"
 * The "command_arg" may have been set.
 */
void do_cmd_debug()
{
	auto const &d_info = game->edit_data.d_info;

	int x, y;
	char cmd;


	/* Get a "debug command" */
	get_com("Debug Command: ", &cmd);

	/* Analyze the command */
	switch (cmd)
	{
		/* Nothing */
	case ESCAPE:
	case ' ':
	case '\n':
	case '\r':
		break;


		/* Hack -- Help */
	case '?':
		do_cmd_help();
		break;


		/* Cure all maladies */
	case 'a':
		do_cmd_wiz_cure_all();
		break;

		/* Teleport to target */
	case 'b':
		do_cmd_wiz_bamf();
		break;

	case 'B':
		do_cmd_wiz_body(command_arg);
		break;

		/* Create any object */
	case '-':
		wiz_create_item_2();
		break;

		/* Create any object */
	case 'c':
		wiz_create_item();
		break;

		/* Create a named artifact */
	case 'C':
		wiz_create_named_art();
		break;

		/* Detect everything */
	case 'd':
		detect_all(DEFAULT_RADIUS);
		break;

		/* Change of Dungeon type */
	case 'D':
		if ((command_arg >= 0) && (std::size_t(command_arg) < d_info.size()))
		{
			dungeon_type = command_arg;
			dun_level = d_info[dungeon_type].mindepth;
			msg_format("You go into %s", d_info[dungeon_type].text.c_str());

			/* Leaving */
			p_ptr->leaving = TRUE;
		}
		break;

		/* Edit character */
	case 'e':
		do_cmd_wiz_change();
		break;

		/* Change grid's mana */
	case 'E':
		cave[p_ptr->py][p_ptr->px].mana = command_arg;
		break;

		/* Good Objects */
	case 'g':
		if (command_arg <= 0) command_arg = 1;
		acquirement(p_ptr->py, p_ptr->px, command_arg, FALSE);
		break;

		/* Hitpoint rerating */
	case 'h':
		do_cmd_rerate(); break;

	case 'H':
		do_cmd_summon_horde(); break;

		/* Go up or down in the dungeon */
	case 'j':
		do_cmd_wiz_jump();
		break;

		/* Magic Mapping */
	case 'm':
		map_area();
		break;

		/* corruption */
	case 'M':
		gain_random_corruption();
		break;

		/* Summon _friendly_ named monster */
	case 'N':
		do_cmd_wiz_named_friendly(command_arg, TRUE);
		break;

		/* Summon Named Monster */
	case 'n':
		do_cmd_wiz_named(command_arg, TRUE);
		break;

		/* Object playing routines */
	case 'o':
		do_cmd_wiz_play();
		break;

		/* Phase Door */
	case 'p':
		teleport_player(10);
		break;

		/* get a Quest */
	case 'q':
		{
			/*                        if (quest[command_arg].status == QUEST_STATUS_UNTAKEN)*/
			if ((command_arg >= 1) && (command_arg < MAX_Q_IDX) && (command_arg != QUEST_RANDOM))
			{
				quest[command_arg].status = QUEST_STATUS_TAKEN;
				*(quest[command_arg].plot) = command_arg;
				quest[command_arg].init();
				break;
			}
			break;
		}

		/* Make every dungeon square "known" to test streamers -KMW- */
	case 'u':
		{
			for (y = 0; y < cur_hgt; y++)
			{
				for (x = 0; x < cur_wid; x++)
				{
					cave[y][x].info |= (CAVE_GLOW | CAVE_MARK);
				}
			}
			wiz_lite();
			break;
		}

	case 'U':
		{
			p_ptr->necro_extra |= CLASS_UNDEAD;
			do_cmd_wiz_named(5, TRUE);

			p_ptr->necro_extra2 = 1;

			/* Display the hitpoints */
			p_ptr->update |= (PU_HP);
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
			break;
		}

		/* Summon Random Monster(s) */
	case 's':
		if (command_arg <= 0) command_arg = 1;
		do_cmd_wiz_summon(command_arg);
		break;

		/* Change the feature of the map */
	case 'S':
		cave[p_ptr->py][p_ptr->px].special = command_arg;
		break;

		/* Teleport */
	case 't':
		teleport_player_bypass = TRUE;
		teleport_player(100);
		teleport_player_bypass = FALSE;
		break;

		/* Teleport to a town */
	case 'T':
		if ((command_arg >= 1) && (command_arg <= max_real_towns))
			teleport_player_town(command_arg);
		break;

		/* Very Good Objects */
	case 'v':
		if (command_arg <= 0) command_arg = 1;
		acquirement(p_ptr->py, p_ptr->px, command_arg, TRUE);
		break;

		/* Wizard Light the Level */
	case 'w':
		wiz_lite();
		break;

		/* Make a wish */
	case 'W':
		make_wish();
		break;

		/* Increase Experience */
	case 'x':
		if (command_arg)
		{
			gain_exp(command_arg);
		}
		else
		{
			gain_exp(p_ptr->exp + 1);
		}
		break;

		/* Zap Monsters (Genocide) */
	case 'z':
		do_cmd_wiz_zap();
		break;

		/* Mimic shape changing */
	case '*':
		p_ptr->tim_mimic = 100;
		p_ptr->mimic_form = command_arg;
		/* Redraw title */
		p_ptr->redraw |= (PR_FRAME);
		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
		break;

		/* Gain a fate */
	case '+':
		{
			int i;
			gain_fate(command_arg);
			for (i = 0; i < MAX_FATES; i++)
				fates[i].know = TRUE;
			break;
		}

		/* Change the feature of the map */
	case 'F':
		msg_format("Old feature: %d", cave[p_ptr->py][p_ptr->px].feat);
		msg_format("Special: %d", cave[p_ptr->py][p_ptr->px].special);
		cave_set_feat(p_ptr->py, p_ptr->px, command_arg);
		break;

	case '=':
		wiz_align_monster(command_arg);
		break;

	case '@':
		wiz_inc_monster_level(command_arg);
		break;

	case '/':
		summon_specific(p_ptr->py, p_ptr->px, max_dlv[dungeon_type], command_arg);
		break;

		/* Not a Wizard Command */
	default:
		msg_print("That is not a valid debug command.");
		break;
	}
}
