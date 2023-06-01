/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "xtra1.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "corrupt.hpp"
#include "cmd7.hpp"
#include "dungeon_flag.hpp"
#include "dungeon_info_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "hook_calculate_hp_in.hpp"
#include "hook_calculate_hp_out.hpp"
#include "hooks.hpp"
#include "levels.hpp"
#include "messages.hpp"
#include "mimic.hpp"
#include "monster1.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_flag_meta.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "skill_flag.hpp"
#include "skill_type.hpp"
#include "skills.hpp"
#include "spells3.hpp"
#include "spells6.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"

#include <cassert>
#include <fmt/format.h>

/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val)
{
	if (!options->linear_stats)
	{
		/* Above 18 */
		if (val > 18)
		{
			int bonus = (val - 18);

			if (bonus >= 220)
			{
				sprintf(out_val, "18/%3s", "***");
			}
			else if (bonus >= 100)
			{
				sprintf(out_val, "18/%03d", bonus);
			}
			else
			{
				sprintf(out_val, " 18/%02d", bonus);
			}
		}

		/* From 3 to 18 */
		else
		{
			sprintf(out_val, "    %2d", val);
		}
	}
	else
	{
		/* Above 18 */
		if (val > 18)
		{
			int bonus = (val - 18);

			if (bonus >= 220)
			{
				sprintf(out_val, "    40");
			}
			else
			{
				sprintf(out_val, "    %2d", 18 + (bonus / 10));
			}
		}

		/* From 3 to 18 */
		else
		{
			sprintf(out_val, "    %2d", val);
		}
	}
}



/*
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
s16b modify_stat_value(int value, int amount)
{
	int i;

	/* Reward */
	if (amount > 0)
	{
		/* Apply each point */
		for (i = 0; i < amount; i++)
		{
			/* One point at a time */
			if (value < 18) value++;

			/* Ten "points" at a time */
			else value += 10;
		}
	}

	/* Penalty */
	else if (amount < 0)
	{
		/* Apply each point */
		for (i = 0; i < (0 - amount); i++)
		{
			/* Ten points at a time */
			if (value >= 18 + 10) value -= 10;

			/* Hack -- prevent weirdness */
			else if (value > 18) value = 18;

			/* One point at a time */
			else if (value > 3) value--;
		}
	}

	/* Clip to permissible range */
	if (value < 3)
	{
		value = 3;
	}

	/* Return new value */
	return (value);
}



/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(const char *info, int row, int col)
{
	/* Dump 13 spaces to clear */
	c_put_str(TERM_WHITE, "             ", row, col);

	/* Dump the info itself */
	c_put_str(TERM_L_BLUE, info, row, col);
}

/*
 * Prints players max/cur piety
 */
static void prt_piety()
{
	char tmp[32];

	/* Do not show piety unless it matters */
	if (!p_ptr->pgod) return;

	c_put_str(TERM_L_WHITE, "Pt ", ROW_PIETY, COL_PIETY);

	sprintf(tmp, "%9ld", (long) p_ptr->grace);

	c_put_str((p_ptr->praying) ? TERM_L_BLUE : TERM_GREEN, tmp, ROW_PIETY,
		COL_PIETY + 3);
}


/*
 * Prints the player's current sanity.
 */
static void prt_sane()
{
	char tmp[32];
	byte color;
	int perc;

	if (p_ptr->msane == 0)
	{
		perc = 100;
	}
	else
	{
		perc = (100 * p_ptr->csane) / p_ptr->msane;
	}

	c_put_str(TERM_ORANGE, "SN ", ROW_SANITY, COL_SANITY);

	sprintf(tmp, "%4d/%4d", p_ptr->csane, p_ptr->msane);

	if (perc >= 100)
	{
		color = TERM_L_GREEN;
	}
	else if (perc > (10 * options->hitpoint_warn))
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}

	c_put_str(color, tmp, ROW_SANITY, COL_SANITY + 3);
}


/*
 * Print character stat in given row, column
 */
static void prt_stat(int stat)
{
	char tmp[32];

	cnv_stat(p_ptr->stat_use[stat], tmp);

	/* Display "injured" stat */
	if (p_ptr->stat_cur[stat] < p_ptr->stat_max[stat])
	{
		int colour;

		if (p_ptr->stat_cnt[stat])
			colour = TERM_ORANGE;
		else
			colour = TERM_YELLOW;

		put_str(format("%s: ", stat_names_reduced[stat]), ROW_STAT + stat, 0);
		c_put_str(colour, tmp, ROW_STAT + stat, COL_STAT + 6);
	}

	/* Display "healthy" stat */
	else
	{
		put_str(format("%s: ", stat_names[stat]), ROW_STAT + stat, 0);
		c_put_str(TERM_L_GREEN, tmp, ROW_STAT + stat, COL_STAT + 6);
	}

	/* Display "boosted" stat */
	if (p_ptr->stat_cur[stat] > p_ptr->stat_max[stat])
	{
		put_str(format("%s: ", stat_names[stat]), ROW_STAT + stat, 0);
		c_put_str(TERM_VIOLET, tmp, ROW_STAT + stat, COL_STAT + 6);
	}

	/* Indicate natural maximum */
	if (p_ptr->stat_max[stat] == 18 + 100)
	{
		put_str("!", ROW_STAT + stat, 3);
	}
}




/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title()
{
	std::string p;

	/* Mimic shape */
	if (p_ptr->mimic_form)
	{
		p = get_mimic_name(p_ptr->mimic_form);
	}

	/* Wizard */
	else if (wizard)
	{
		p = "[=-WIZARD-=]";
	}

	/* Winner */
	else if (total_winner == WINNER_NORMAL)
	{
		p = "***WINNER***";
	}

	/* Ultra Winner */
	else if (total_winner == WINNER_ULTRA)
	{
		p = "***GOD***";
	}

	/* Normal */
	else
	{
		p = cp_ptr->titles[(p_ptr->lev - 1) / 5];

	}

	prt_field(p.c_str(), ROW_TITLE, COL_TITLE);
}


/*
 * Prints level
 */
static void prt_level()
{
	char tmp[32];

	sprintf(tmp, "%6d", p_ptr->lev);

	if (p_ptr->lev >= p_ptr->max_plv)
	{
		put_str("LEVEL ", ROW_LEVEL, 0);
		c_put_str(TERM_L_GREEN, tmp, ROW_LEVEL, COL_LEVEL + 6);
	}
	else
	{
		put_str("Level ", ROW_LEVEL, 0);
		c_put_str(TERM_YELLOW, tmp, ROW_LEVEL, COL_LEVEL + 6);
	}
}


/*
 * Display the experience
 */
static void prt_exp()
{
	char out_val[32];

	if (p_ptr->lev >= PY_MAX_LEVEL)
	{
		sprintf(out_val, "********");
	}
	else
	{
		sprintf(out_val, "%8ld", (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L) - p_ptr->exp);
	}

	if (p_ptr->exp >= p_ptr->max_exp)
	{
		put_str("EXP ", ROW_EXP, 0);
		c_put_str(TERM_L_GREEN, out_val, ROW_EXP, COL_EXP + 4);
	}
	else
	{
		put_str("Exp ", ROW_EXP, 0);
		c_put_str(TERM_YELLOW, out_val, ROW_EXP, COL_EXP + 4);
	}
}


/*
 * Prints current gold
 */
static void prt_gold()
{
	char tmp[32];

	put_str("AU ", ROW_GOLD, COL_GOLD);
	sprintf(tmp, "%9ld", (long)p_ptr->au);
	c_put_str(TERM_L_GREEN, tmp, ROW_GOLD, COL_GOLD + 3);
}



/*
 * Prints current AC
 */
static void prt_ac()
{
	char tmp[32];

	put_str("Cur AC ", ROW_AC, COL_AC);
	sprintf(tmp, "%5d", p_ptr->dis_ac + p_ptr->dis_to_a);
	c_put_str(TERM_L_GREEN, tmp, ROW_AC, COL_AC + 7);
}


/*
 * Prints Cur/Max hit points
 */
static void prt_hp()
{
	char tmp[32];

	byte color;

	if (options->player_char_health)
	{
		lite_spot(p_ptr->py, p_ptr->px);
	}

	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		c_put_str(TERM_L_DARK, "DP ", ROW_HP, COL_HP);

		sprintf(tmp, "%4d/%4d", p_ptr->chp, p_ptr->mhp);

		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_BLUE;
		}
		else if (p_ptr->chp > (p_ptr->mhp * options->hitpoint_warn) / 10)
		{
			color = TERM_VIOLET;
		}
		else
		{
			color = TERM_L_RED;
		}
		c_put_str(color, tmp, ROW_HP, COL_HP + 3);
	}
	else
	{
		c_put_str(TERM_RED, "HP ", ROW_HP, COL_HP);

		sprintf(tmp, "%4d/%4d", p_ptr->chp, p_ptr->mhp);

		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_GREEN;
		}
		else if (p_ptr->chp > (p_ptr->mhp * options->hitpoint_warn) / 10)
		{
			color = TERM_YELLOW;
		}
		else
		{
			color = TERM_RED;
		}
		c_put_str(color, tmp, ROW_HP, COL_HP + 3);
	}
}

/*
 * Prints Cur/Max monster hit points
 */
static void prt_mh()
{
	char tmp[32];

	byte color;

	object_type *o_ptr;

	/* Get the carried monster */
	o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (!o_ptr->pval2)
	{
		put_str("             ", ROW_MH, COL_MH);
		return;
	}

	put_str("MH ", ROW_MH, COL_MH);

	sprintf(tmp, "%4d/%4d", o_ptr->pval2, (int)o_ptr->pval3);
	if (o_ptr->pval2 >= o_ptr->pval3)
	{
		color = TERM_L_GREEN;
	}
	else if (o_ptr->pval2 > (o_ptr->pval3 * options->hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	c_put_str(color, tmp, ROW_MH, COL_MH + 3);
}


/*
 * Prints players max/cur spell points
 */
static void prt_sp()
{
	char tmp[32];
	byte color;


	c_put_str(TERM_L_GREEN, "SP ", ROW_SP, COL_SP);

	sprintf(tmp, "%4d/%4d", p_ptr->csp, p_ptr->msp);
	if (p_ptr->csp >= p_ptr->msp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csp > (p_ptr->msp * options->hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	c_put_str(color, tmp, ROW_SP, COL_SP + 3);
}


/*
 * Prints depth in stat area
 */
static void prt_depth(int row, int col)
{
	auto const &d_info = game->edit_data.d_info;
	auto const &wf_info = game->edit_data.wf_info;
	auto const &dungeon_flags = game->dungeon_flags;

	char depths[32];
	auto d_ptr = &d_info[dungeon_type];

	if (p_ptr->wild_mode)
	{
		strcpy(depths, "             ");
	}
	else if (auto s = get_dungeon_name())
	{
		strcpy(depths, s->c_str());
	}
	else if (dungeon_flags & DF_SPECIAL)
	{
		strcpy(depths, "Special");
	}
	else if (p_ptr->inside_quest)
	{
		strcpy(depths, "Quest");
	}
	else if (!dun_level)
	{
		auto const &wilderness = game->wilderness;
		auto const &wf = wf_info[wilderness(p_ptr->wilderness_x, p_ptr->wilderness_y).feat];
		if (wf.name)
		{
			strcpy(depths, wf.name);
		}
		else
		{
			strcpy(depths, "Town/Wild");
		}
	}
	else
	{
		if (dungeon_flags & DF_TOWER)
		{
			strnfmt(depths, 32, "%c%c%c -%d",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level);
		}
		else
		{
			strnfmt(depths, 32, "%c%c%c %d",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level);
		}
	}

	/* Right-Adjust the "depth", and clear old values */
	if (p_ptr->word_recall)
		c_prt(TERM_ORANGE, format("%13s", depths), row, col);
	else
		prt(format("%13s", depths), row, col);
}


/*
 * Prints Searching, Resting, Paralysis, or 'count' status
 * Display is always exactly 10 characters wide (see below)
 *
 * This function was a major bottleneck when resting, so a lot of
 * the text formatting code was optimized in place below.
 */
static void prt_state(int row, int col)
{
	byte attr = TERM_WHITE;

	char text[16];


	/* Paralysis */
	if (p_ptr->paralyzed)
	{
		attr = TERM_RED;

		strcpy(text, "Paralyzed!");
	}

	/* Resting */
	else if (resting)
	{
		int i;

		/* Start with "Rest" */
		strcpy(text, "Rest      ");

		/* Extensive (timed) rest */
		if (resting >= 1000)
		{
			i = resting / 100;
			text[9] = '0';
			text[8] = '0';
			text[7] = '0' + (i % 10);
			if (i >= 10)
			{
				i = i / 10;
				text[6] = '0' + (i % 10);
				if (i >= 10)
				{
					text[5] = '0' + (i / 10);
				}
			}
		}

		/* Long (timed) rest */
		else if (resting >= 100)
		{
			i = resting;
			text[9] = '0' + (i % 10);
			i = i / 10;
			text[8] = '0' + (i % 10);
			text[7] = '0' + (i / 10);
		}

		/* Medium (timed) rest */
		else if (resting >= 10)
		{
			i = resting;
			text[9] = '0' + (i % 10);
			text[8] = '0' + (i / 10);
		}

		/* Short (timed) rest */
		else if (resting > 0)
		{
			i = resting;
			text[9] = '0' + (i);
		}

		/* Rest until healed */
		else if (resting == -1)
		{
			text[5] = text[6] = text[7] = text[8] = text[9] = '*';
		}

		/* Rest until done */
		else if (resting == -2)
		{
			text[5] = text[6] = text[7] = text[8] = text[9] = '&';
		}
	}

	/* Repeating */
	else if (command_rep)
	{
		if (command_rep > 999)
		{
			sprintf(text, "Rep. %3d00", command_rep / 100);
		}
		else
		{
			sprintf(text, "Repeat %3d", command_rep);
		}
	}

	/* Nothing interesting */
	else
	{
		strcpy(text, "          ");
	}

	/* Display the info (or blanks) */
	c_put_str(attr, text, row, col);
}


/*
 * Prints the speed of a character.			-CJS-
 */
static void prt_speed(int row, int col)
{
	int i = p_ptr->pspeed;

	byte attr = TERM_WHITE;
	char buf[32] = "";

	/* Fast */
	if (i > 110)
	{
		attr = TERM_L_GREEN;
		sprintf(buf, "Fast (+%d)", (i - 110));
	}

	/* Slow */
	else if (i < 110)
	{
		attr = TERM_L_UMBER;
		sprintf(buf, "Slow (-%d)", (110 - i));
	}

	/* Display the speed */
	c_put_str(attr, format("%-10s", buf), row, col);
}



/*
 * Prints status line
 */
static void prt_status_line()
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);
	int row = hgt - 1;

	/* Fainting / Starving */
	int col = 0;
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		c_put_str(TERM_RED, "Weak  ", row, col);
	}
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		c_put_str(TERM_ORANGE, "Weak  ", row, col);
	}
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		c_put_str(TERM_YELLOW, "Hungry", row, col);
	}
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		c_put_str(TERM_L_GREEN, "      ", row, col);
	}
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		c_put_str(TERM_L_GREEN, "Full  ", row, col);
	}
	else
	{
		c_put_str(TERM_GREEN, "Gorged", row, col);
	}

	/* Blind */
	col = 7;
	if (p_ptr->blind)
	{
		c_put_str(TERM_ORANGE, "Blind", row, col);
	}
	else
	{
		put_str("     ", row, col);
	}

	/* Confusion */
	col = 13;
	if (p_ptr->confused)
	{
		c_put_str(TERM_ORANGE, "Conf", row, col);
	}
	else
	{
		put_str("    ", row, col);
	}

	/* Fear */
        col = 18;
	if (p_ptr->afraid)
	{
		c_put_str(TERM_ORANGE, "Afraid", row, col);
	}
	else
	{
		put_str("      ", row, col);
	}

	/* Poison */
	col = 25;
	if (p_ptr->poisoned)
	{
		c_put_str(TERM_ORANGE, "Poison", row, col);
	}
	else
	{
		put_str("      ", row, col);
	}

	/* State */
	col = 38;
	prt_state(row, col);

	/* Speed */
	col = 49;
	prt_speed(row, col);

	/* "Study" */
	col = 60;
	if (p_ptr->skill_points)
	{
		put_str("Skill", row, col);
	}
	else
	{
		put_str("     ", row, col);
	}

	/* Depth */
	col = wid - 14;
	prt_depth(row, col);
}



static void prt_cut()
{
	int c = p_ptr->cut;
	int hgt;
	Term_get_size(nullptr, &hgt);
	int row = hgt - 3;
	int col = 0;

	if (c > 1000)
	{
		c_put_str(TERM_L_RED, "Mortal wound", row, col);
	}
	else if (c > 200)
	{
		c_put_str(TERM_RED, "Deep gash   ", row, col);
	}
	else if (c > 100)
	{
		c_put_str(TERM_RED, "Severe cut  ", row, col);
	}
	else if (c > 50)
	{
		c_put_str(TERM_ORANGE, "Nasty cut   ", row, col);
	}
	else if (c > 25)
	{
		c_put_str(TERM_ORANGE, "Bad cut     ", row, col);
	}
	else if (c > 10)
	{
		c_put_str(TERM_YELLOW, "Light cut   ", row, col);
	}
	else if (c)
	{
		c_put_str(TERM_YELLOW, "Graze       ", row, col);
	}
	else
	{
		put_str("            ", row, col);
	}
}



static void prt_stun()
{
	int s = p_ptr->stun;
	int hgt;
	Term_get_size(nullptr, &hgt);
	int row = hgt - 2;
	int col = 0;

	if (s > 100)
	{
		c_put_str(TERM_RED, "Knocked out ", row, col);
	}
	else if (s > 50)
	{
		c_put_str(TERM_ORANGE, "Heavy stun  ", row, col);
	}
	else if (s)
	{
		c_put_str(TERM_ORANGE, "Stun        ", row, col);
	}
	else
	{
		put_str("            ", row, col);
	}
}



/*
 * Redraw the "monster health bar"	-DRS-
 * Rather extensive modifications by	-BEN-
 *
 * The "monster health bar" provides visual feedback on the "health"
 * of the monster currently being "tracked".  There are several ways
 * to "track" a monster, including targetting it, attacking it, and
 * affecting it (and nobody else) with a ranged attack.
 *
 * Display the monster health bar (affectionately known as the
 * "health-o-meter").  Clear health bar if nothing is being tracked.
 * Auto-track current target monster when bored.  Note that the
 * health-bar stops tracking any monster that "disappears".
 */
static void health_redraw()
{
	int hgt;
	Term_get_size(nullptr, &hgt);
	int col = 0;
	int row = hgt - 4;

	/* Not tracking */
	if (!health_who)
	{
		/* Erase the health bar */
		Term_erase(col, row, 12);
	}

	/* Tracking an unseen monster */
	else if (!m_list[health_who].ml)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a hallucinatory monster */
	else if (p_ptr->image)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a dead monster (???) */
	else if ((m_list[health_who].hp < 0))
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a visible monster */
	else
	{
		int pct, len;

		monster_type *m_ptr = &m_list[health_who];

		/* Default to almost dead */
		byte attr = TERM_RED;

		/* Extract the "percent" of health */
		pct = 100L * m_ptr->hp / m_ptr->maxhp;

		/* Badly wounded */
		if (pct >= 10) attr = TERM_L_RED;

		/* Wounded */
		if (pct >= 25) attr = TERM_ORANGE;

		/* Somewhat Wounded */
		if (pct >= 60) attr = TERM_YELLOW;

		/* Healthy */
		if (pct >= 100) attr = TERM_L_GREEN;

		/* Afraid */
		if (m_ptr->monfear) attr = TERM_VIOLET;

		/* Asleep */
		if (m_ptr->csleep) attr = TERM_BLUE;

		/* Poisoned */
		if (m_ptr->poisoned) attr = TERM_GREEN;

		/* Bleeding */
		if (m_ptr->bleeding) attr = TERM_RED;

		/* Convert percent into "health" */
		len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;

		/* Default to "unknown" */
		Term_putstr(col, row, 12, TERM_WHITE, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		if (m_ptr->stunned) {
			Term_putstr(col + 1, row, len, attr, "ssssssssss");
		} else if (m_ptr->confused) {
			Term_putstr(col + 1, row, len, attr, "cccccccccc");
		} else {
			Term_putstr(col + 1, row, len, attr, "**********");
		}
	}
}



/*
 * Display basic info (mostly left of map)
 */
static void prt_frame()
{
	int i;

	/* Race and Class */
	prt_field(rp_ptr->title.c_str(), ROW_RACE, COL_RACE);
	prt_field(spp_ptr->title, ROW_CLASS, COL_CLASS);

	/* Title */
	prt_title();

	/* Level/Experience */
	prt_level();
	prt_exp();

	/* All Stats */
	for (i = 0; i < 6; i++) prt_stat(i);

	/* Armor */
	prt_ac();

	/* Hitpoints */
	prt_hp();

	/* Current sanity */
	prt_sane();

	/* Spellpoints */
	prt_sp();

	/* Piety */
	prt_piety();

	/* Monster hitpoints */
	prt_mh();

	/* Gold */
	prt_gold();

	/* Cut/Stun */
	prt_cut();
	prt_stun();

	/* Current depth */
	prt_status_line();

	/* Special */
	health_redraw();
}


/**
 * Fix up each terminal based on whether a window flag
 * is set.
 */
namespace { // anonymous

template <typename F>
static void fixup_display(u32b mask, F callback)
{
	for (int j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & mask)) continue;

		/* Apply callback */
		Term_with_active(angband_term[j], [&callback]() {
			callback();
			Term_fresh();
		});
	}
}

} // namespace anonymous


/*
 * Hack -- display inventory in sub-windows
 */
static void fix_inven()
{
	fixup_display(PW_INVEN, display_inven);
}

/*
 * Hack -- display equipment in sub-windows
 */
static void fix_equip()
{
	fixup_display(PW_EQUIP, display_equip);
}

/*
 * Hack -- display character in sub-windows
 */
static void fix_player()
{
	fixup_display(PW_PLAYER, [] {
		display_player(0);
	});
}



/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */
void fix_message()
{
	auto const &messages = game->messages;

	fixup_display(PW_MESSAGE, [&messages] {
		/* Get size */
		int w, h;
		Term_get_size(&w, &h);

		/* Dump messages */
		for (int i = 0; i < h; i++)
		{
			auto message = messages.at(i);
			auto text_with_count = message.text_with_count();

			/* Dump the message on the appropriate line */
			display_message(0, (h - 1) - i, text_with_count.size(), message.color, text_with_count.c_str());

			/* Cursor */
			int x, y;
			Term_locate(&x, &y);

			/* Clear to end of line */
			Term_erase(x, y, 255);
		}
	});
}


/*
 * Hack -- display overhead view in sub-windows
 *
 * Note that the "player" symbol does NOT appear on the map.
 */
static void fix_overhead()
{
	fixup_display(PW_OVERHEAD, [] {
		int cy, cx;
		display_map(&cy, &cx);
	});
}


/*
 * Hack -- display monster recall in sub-windows
 */
static void fix_monster()
{
	fixup_display(PW_MONSTER, [] {
		/* Display monster race info */
		if (monster_race_idx)
		{
			display_roff(monster_race_idx, monster_ego_idx);
		}
	});
}


/*
 * Hack -- display object recall in sub-windows
 */
static void fix_object()
{
	fixup_display(PW_OBJECT, [] {
		/* Clear */
		Term_clear();

		/* Display object info */
		if (tracked_object &&
			!object_out_desc(tracked_object, NULL, false, false))
		{
			text_out("You see nothing special.");
		}
	});
}

/* Show the monster list in a window */

static void fix_m_list()
{
	auto const &r_info = game->edit_data.r_info;

	// Mirror of the r_info array, index by index. We use a
	// statically allocated value to avoid frequent allocations.
	static auto r_total_visible =
		std::vector<u16b>(r_info.size(), 0);

	fixup_display(PW_M_LIST, [&r_info] {
		/* Clear */
		Term_clear();

		/* Hallucination */
		if (p_ptr->image)
		{
			c_prt(TERM_WHITE, "You can not see clearly", 0, 0);
			return;
		}

		/* reset visible count */
		for (std::size_t i = 1; i < r_info.size(); i++)
		{
			r_total_visible[i] = 0;
		}

		/* Count up the number visible in each race */
		int c = 0;
		for (std::size_t i = 1; i < static_cast<u16b>(m_max); i++)
		{
			auto const m_ptr = &m_list[i];
			auto const r_ptr = &r_info[m_ptr->r_idx];
			auto total_visible = &r_total_visible[m_ptr->r_idx];

			/* Skip dead monsters */
			if (m_ptr->hp < 0) continue;

			/* Skip unseen monsters */
			if (r_ptr->flags & RF_MIMIC)
			{
				/* Acquire object */
				object_type *o_ptr = &o_list[m_ptr->mimic_o_idx()];

				/* Memorized objects */
				if (!o_ptr->marked) continue;
			}
			else
			{
				if (!m_ptr->ml) continue;
			}

			/* Increase for this race */
			(*total_visible)++;

			/* Increase total Count */
			c++;
		}

		/* Are monsters visible? */
		if (c)
		{
			int w, h, num = 0;

			Term_get_size(&w, &h);

			c_prt(TERM_WHITE, format("You can see %d monster%s", c, (c > 1 ? "s:" : ":")), 0, 0);

			for (std::size_t i = 1; i < r_info.size(); i++)
			{
				auto const r_ptr = &r_info[i];
				auto const total_visible = r_total_visible[i];

				/* Default Colour */
				byte attr = TERM_SLATE;

				/* Only visible monsters */
				if (!total_visible)
				{
					continue;
				}

				/* Uniques */
				if (r_ptr->flags & RF_UNIQUE)
				{
					attr = TERM_L_BLUE;
				}

				/* Have we killed one? */
				if (r_ptr->r_pkills)
				{
					if (r_ptr->level > dun_level)
					{
						attr = TERM_VIOLET;

						if (r_ptr->flags & RF_UNIQUE)
						{
							attr = TERM_RED;
						}
					}
				}
				else
				{
					if (!(r_ptr->flags & RF_UNIQUE)) attr = TERM_GREEN;
				}

				/* Dump the monster name */
				if (total_visible == 1)
				{
					c_prt(attr, r_ptr->name, (num % (h - 1)) + 1, (num / (h - 1) * 26));
				}
				else
				{
					c_prt(attr, format("%s (x%d)", r_ptr->name, total_visible), (num % (h - 1)) + 1, (num / (h - 1)) * 26);
				}

				num++;

			}

		}
		else
		{
			c_prt(TERM_WHITE, "You see no monsters.", 0, 0);
		}
	});
}


/*
 * Calculate powers of player given the current set of corruptions.
 */
static void calc_powers_corruption()
{
	/* Map of corruptions to a power */
	int i;

	/* Grant powers according to whatever corruptions the player has */
	for (i = 0; i < CORRUPTIONS_MAX; i++)
	{
		if (player_has_corruption(i))
		{
			if (auto p = get_corruption_power(i))
			{
				p_ptr->powers.insert(*p);
			}
		}
	}
}


/* Ugly hack */
bool calc_powers_silent = false;

/* Add in powers */
static void add_powers(std::vector<s16b> const &powers)
{
	for (auto power_idx: powers)
	{
		p_ptr->powers.insert(power_idx);
	}
}

/* Calc the player powers */
static void calc_powers()
{
	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Save old powers */
	std::vector<std::size_t> old_powers;
	std::copy(
		std::begin(p_ptr->powers),
		std::end(p_ptr->powers),
		std::back_inserter(old_powers));
	std::sort(
		std::begin(old_powers),
		std::end(old_powers));

	/* Get intrinsincs */
	p_ptr->powers = p_ptr->powers_mod;

	/* Calculate powers granted by corruptions */
	calc_powers_corruption();

	/* Add objects powers */
	for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		auto o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_ptr)
		{
			continue;
		}

		if (auto p = object_power(o_ptr))
		{
			p_ptr->powers.insert(*p);
		}
	}

	if ((!p_ptr->tim_mimic) && (!p_ptr->body_monster))
	{
		add_powers(rp_ptr->ps.powers);
		add_powers(rmp_ptr->ps.powers);
	}
	else if (p_ptr->mimic_form)
	{
		calc_mimic_power();
	}

	add_powers(cp_ptr->ps.powers);

	if (p_ptr->disembodied)
	{
		p_ptr->powers.insert(PWR_INCARNATE);
	}

	// Notify player of lost/gained powers.
	std::vector<int> new_powers;
	std::copy(
		std::begin(p_ptr->powers),
		std::end(p_ptr->powers),
		std::back_inserter(new_powers));
	std::sort(
		std::begin(new_powers),
		std::end(new_powers));

	if (!calc_powers_silent)
	{
		// Show removed powers
		{
			std::vector<int> removed_powers;
			std::set_difference(
				std::begin(old_powers), std::end(old_powers),
				std::begin(new_powers), std::end(new_powers),
				std::back_inserter(removed_powers)
			);

			for (auto power_idx: removed_powers)
			{
				cmsg_print(TERM_RED, game->powers.at(power_idx)->lose_text);
			}
		}

		// Show added powers
		{
			std::vector<int> added_powers;
			std::set_difference(
				std::begin(new_powers), std::end(new_powers),
				std::begin(old_powers), std::end(old_powers),
				std::back_inserter(added_powers)
			);

			for (auto power_idx: added_powers)
			{
				cmsg_print(TERM_GREEN, game->powers.at(power_idx)->gain_text);
			}
		}
	}

	calc_powers_silent = false;
}


/*
 * Calculate the player's sanity
 */
static void calc_sanity()
{
	int bonus, msane;

	/* Hack -- use the con/hp table for sanity/wis */
	bonus = ((int)(adj_con_mhp[p_ptr->stat_ind[A_WIS]]) - 128);

	/* Hack -- assume 5 sanity points per level. */
	msane = 5 * (p_ptr->lev + 1) + (bonus * p_ptr->lev / 2);

	if (msane < p_ptr->lev + 1) msane = p_ptr->lev + 1;

	if (p_ptr->msane != msane)
	{
		/* Sanity carries over between levels. */
		p_ptr->csane += (msane - p_ptr->msane);

		p_ptr->msane = msane;

		if (p_ptr->csane >= msane)
		{
			p_ptr->csane = msane;
			p_ptr->csane_frac = 0;
		}

		p_ptr->redraw |= (PR_FRAME);
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana()
{
	auto const &r_info = game->edit_data.r_info;

	int msp, levels, cur_wgt, max_wgt;

	levels = p_ptr->lev;

	/* Hack -- no negative mana */
	if (levels < 0) levels = 0;

	/* Extract total mana */
	msp = get_skill_scale(SKILL_MAGIC, 200) +
	      (adj_mag_mana[
	               (p_ptr->stat_ind[A_INT] > p_ptr->stat_ind[A_WIS]) ?
	               p_ptr->stat_ind[A_INT] : p_ptr->stat_ind[A_WIS]
	       ] * levels / 4);

	/* Hack -- usually add one mana */
	if (msp) msp++;

	/* Possessors mana is different */
	if (p_ptr->body_monster && (!p_ptr->disembodied))
	{
		auto r_ptr = &r_info[p_ptr->body_monster];

		int f = 100 / (r_ptr->freq_spell ? r_ptr->freq_spell : 1);

		msp = 21 - f;

		if (msp < 1) msp = 1;
	}

	/* Apply race mod mana */
	msp = msp * rmp_ptr->mana / 100;

	/* Apply class mana */
	msp += msp * cp_ptr->mana / 100;

	/* Apply Eru mana */
	if (p_ptr->pgod == GOD_ERU)
	{
		s32b tmp = p_ptr->grace;

		if (tmp >= 35000) tmp = 35000;
		tmp /= 100;
		msp += msp * tmp / 1000;
	}

	/* Only mages are affected */
	if (forbid_gloves())
	{
		/* Assume player is not encumbered by gloves */
		p_ptr->cumber_glove = false;

		/* Get the gloves */
		object_type *o_ptr = &p_ptr->inventory[INVEN_HANDS];

		/* Examine the gloves */
		auto const flags = object_flags(o_ptr);

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->k_ptr &&
			!(flags & TR_FREE_ACT) &&
			!((flags & TR_DEX) && (o_ptr->pval > 0)) &&
			!(flags & TR_SPELL_CONTAIN))
		{
			/* Encumbered */
			p_ptr->cumber_glove = true;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}
	}

	/* Augment mana */
	if (p_ptr->to_m)
	{
		msp += msp * p_ptr->to_m / 5;
	}

	/* Assume player not encumbered by armor */
	p_ptr->cumber_armor = false;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += p_ptr->inventory[INVEN_BODY].weight;
	cur_wgt += p_ptr->inventory[INVEN_HEAD].weight;
	cur_wgt += p_ptr->inventory[INVEN_ARM].weight;
	cur_wgt += p_ptr->inventory[INVEN_OUTER].weight;
	cur_wgt += p_ptr->inventory[INVEN_HANDS].weight;
	cur_wgt += p_ptr->inventory[INVEN_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = 200 + get_skill_scale(SKILL_COMBAT, 500);

	/* Heavy armor penalizes mana */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		p_ptr->cumber_armor = true;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}

	/* Sp mods? */
	mana_school_calc_mana(&msp);
	meta_inertia_control_calc_mana(&msp);

	/* Mana can never be negative */
	if (msp < 0) msp = 0;


	/* Maximum mana has changed */
	if (p_ptr->msp != msp)
	{
		/* Save new limit */
		p_ptr->msp = msp;

		/* Enforce new limit */
		if (p_ptr->csp >= msp)
		{
			p_ptr->csp = msp;
			p_ptr->csp_frac = 0;
		}

		/* Display mana later */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}


	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "glove state" changes */
	if (p_ptr->old_cumber_glove != p_ptr->cumber_glove)
	{
		/* Message */
		if (p_ptr->cumber_glove)
		{
			msg_print("Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msg_print("Your hands feel more suitable for spellcasting.");
		}

		/* Save it */
		p_ptr->old_cumber_glove = p_ptr->cumber_glove;
	}


	/* Take note when "armor state" changes */
	if (p_ptr->old_cumber_armor != p_ptr->cumber_armor)
	{
		/* Message */
		if (p_ptr->cumber_armor)
		{
			msg_print("The weight of your armor encumbers your movement.");
		}
		else
		{
			msg_print("You feel able to move more freely.");
		}

		/* Save it */
		p_ptr->old_cumber_armor = p_ptr->cumber_armor;
	}
}



/*
 * Calculate the players (maximal) hit points
 * Adjust current hitpoints if necessary
 */
void calc_hitpoints()
{
	auto const &player_hp = game->player_hp;
	auto const &r_info = game->edit_data.r_info;

	/* Un-inflate "half-hitpoint bonus per level" value */
	int const bonus = ((int)(adj_con_mhp[p_ptr->stat_ind[A_CON]]) - 128);

	/* Calculate hitpoints */
	int mhp = player_hp[p_ptr->lev - 1] + (bonus * p_ptr->lev / 2);

	/* Always have at least one hitpoint per level */
	if (mhp < p_ptr->lev + 1) mhp = p_ptr->lev + 1;

	/* Factor in the pernament hp modifications */
	mhp += p_ptr->hp_mod;
	if (mhp < 1) mhp = 1;

	/* Hack: Sorcery impose a hp penality */
	if (mhp && (get_skill(SKILL_SORCERY)))
	{
		mhp -= mhp * get_skill_scale(SKILL_SORCERY, 50) / 100;
		if (mhp < 1) mhp = 1;
	}

	/* Factor in the melkor hp modifications */
	if (p_ptr->pgod == GOD_MELKOR)
	{
		mhp -= (p_ptr->melkor_sacrifice * 10);
		if (mhp < 1) mhp = 1;
	}

	/* Factor in the hero / superhero settings */
	if (p_ptr->hero) mhp += 10;
	if (p_ptr->shero) mhp += 30;

	/* Augment Hitpoint */
	mhp += mhp * p_ptr->to_l / 5;

	if (mhp < 1) mhp = 1;

	if (p_ptr->body_monster)
	{
		auto r_ptr = &r_info[p_ptr->body_monster];
		u32b rhp = maxroll(r_ptr->hdice, r_ptr->hside);

		/* Adjust the hp with the possession skill */
		rhp = (rhp * (20 + get_skill_scale(SKILL_POSSESSION, 80))) / 100;

		mhp = (rhp + sroot(rhp) + mhp) / 3;
	}
	if (p_ptr->disembodied) mhp = 1;

	/* HACK - being undead means less DP */
	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		int divisor = p_ptr->lev / 4;

		/* Beware of the horrible division by zero ! :) */
		if (divisor == 0) divisor = 1;

		/* Actually decrease the max hp */
		mhp /= divisor;

		/* Never less than 1 */
		if (mhp < 1) mhp = 1;
	}

	/* Hp mods? */
	{
		struct hook_calculate_hp_in in = { mhp };
		struct hook_calculate_hp_out out = { 0 };
		if (process_hooks_new(HOOK_CALC_HP, &in, &out))
		{
			mhp = out.mhp;
		}
	}

	/* Never less than 1 */
	if (mhp < 1) mhp = 1;

	/* New maximum hitpoints */
	if (p_ptr->mhp != mhp)
	{
		/* XXX XXX XXX New hitpoint maintenance */

		/* Enforce maximum */
		if (p_ptr->chp >= mhp)
		{
			p_ptr->chp = mhp;
			p_ptr->chp_frac = 0;
		}

		/* Save the new max-hitpoints */
		p_ptr->mhp = mhp;

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
 * God hooks for light
 */
static void calc_torch_gods()
{
	if (p_ptr->pgod == GOD_VARDA)
	{
		/* increase lite radius */
		p_ptr->cur_lite += 1;
	}
}


/*
 * Extract and set the current "lite radius"
 *
 * SWD: Experimental modification: multiple light sources have additive effect.
 *
 */
static void calc_torch()
{
	int i;
	object_type *o_ptr;

	/* Assume no light */
	p_ptr->cur_lite = 0;

	/* Loop through all wielded items */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Extract the flags */
		auto const flags = object_flags(o_ptr);

		/* does this item glow? */
		if (((flags & TR_FUEL_LITE) && (o_ptr->timeout > 0)) || (!(flags & TR_FUEL_LITE)))
		{
			if (flags & TR_LITE1) p_ptr->cur_lite++;
			if (flags & TR_LITE2) p_ptr->cur_lite += 2;
			if (flags & TR_LITE3) p_ptr->cur_lite += 3;
		}

	}

	if (p_ptr->tim_lite) p_ptr->cur_lite += 2;

	if (p_ptr->holy) p_ptr->cur_lite += 1;

	/* max radius is 5 without rewriting other code -- */
	/* see cave.c:update_lite() and defines.h:LITE_MAX */
	if (p_ptr->cur_lite > 5) p_ptr->cur_lite = 5;

	/* check if the player doesn't have a lite source, */
	/* but does glow as an intrinsic.                  */
	if (p_ptr->cur_lite == 0 && p_ptr->lite) p_ptr->cur_lite = 1;

	/* gods */
	calc_torch_gods();

	/* end experimental mods */

	/* Reduce lite in the small-scale wilderness map */
	if (p_ptr->wild_mode)
	{
		/* Reduce the lite radius if needed */
		if (p_ptr->cur_lite > WILDERNESS_SEE_RADIUS)
		{
			p_ptr->cur_lite = WILDERNESS_SEE_RADIUS;
		}
	}


	/* Reduce lite when running if requested */
	if (running && options->view_reduce_lite)
	{
		/* Reduce the lite radius if needed */
		if (p_ptr->cur_lite > 1) p_ptr->cur_lite = 1;
	}

	/* Notice changes in the "lite radius" */
	if (p_ptr->old_lite != p_ptr->cur_lite)
	{
		/* Update the view */
		p_ptr->update |= (PU_VIEW);

		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Remember the old lite */
		p_ptr->old_lite = p_ptr->cur_lite;
	}
}



/*
 * Computes current weight limit.
 */
int weight_limit()
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100;

	/* Return the result */
	return (i);
}

void calc_wield_monster()
{
	auto const &r_info = game->edit_data.r_info;

	/* Get the carried monster */
	auto o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (o_ptr->k_ptr)
	{
		auto r_ptr = &r_info[o_ptr->pval];

		if (r_ptr->flags & RF_INVISIBLE)
		{
			p_ptr->invis += 20;
		}

		if (r_ptr->flags & RF_REFLECTING)
		{
			p_ptr->reflect = true;
		}

		if (r_ptr->flags & RF_CAN_FLY)
		{
			p_ptr->ffall = true;
		}

		if (r_ptr->flags & RF_AQUATIC)
		{
			p_ptr->water_breath = true;
		}
	}
}

/*
 * Calc which body parts the player have, based on the
 * monster he incarnate, note that that's bnot a hack
 * since body parts of the player when in it's own body
 * are also defined in r_info(monster 0)
 */
void calc_body()
{
	auto const &r_info = game->edit_data.r_info;

	auto r_ptr = &r_info[p_ptr->body_monster];
	int i, b_weapon, b_legs, b_arms;
	byte *body_parts, bp[BODY_MAX];

	if (!p_ptr->body_monster)
	{
		body_parts = bp;
		for (i = 0; i < BODY_MAX; i++)
		{
			int b;

			b = rp_ptr->body_parts[i] + rmp_ptr->body_parts[i];
			if (b < 0) b = 0;

			if (p_ptr->mimic_form == resolve_mimic_name("Bear"))
			{
				if (i == BODY_ARMS) b = 0;
				else if (i == BODY_LEGS) b = 0;
			}

			bp[i] = b;
		}
	}
	else
	{
		body_parts = bp;
		for (i = 0; i < BODY_MAX; i++)
		{
			int b;

			b = r_ptr->body_parts[i];
			if (b < 0) b = 0;

			bp[i] = b;
		}
	}

	for (i = 0; i < BODY_MAX; i++)
	{
		int b;

		b = bp[i] + cp_ptr->body_parts[i];
		if (b < 0) b = 0;
		if (b > max_body_part[i]) b = max_body_part[i];

		bp[i] = b;
	}

	b_weapon = body_parts[BODY_WEAPON];
	b_arms = body_parts[BODY_ARMS];
	b_legs = body_parts[BODY_LEGS];

	if (p_ptr->mimic_extra & CLASS_ARMS)
	{
		b_weapon++;
		b_arms++;

		if (b_weapon > 3) b_weapon = 3;
		if (b_arms > 3) b_arms = 3;
	}

	if (p_ptr->mimic_extra & CLASS_LEGS)
	{
		b_legs++;

		if (b_legs > 2) b_legs = 2;
	}

	for (i = 0; i < INVEN_TOTAL - INVEN_WIELD; i++)
		p_ptr->body_parts[i] = 0;

	for (i = 0; i < b_weapon; i++)
		p_ptr->body_parts[INVEN_WIELD - INVEN_WIELD + i] = INVEN_WIELD;
	if (body_parts[BODY_WEAPON])
		p_ptr->body_parts[INVEN_BOW - INVEN_WIELD] = INVEN_BOW;

	for (i = 0; i < body_parts[BODY_TORSO]; i++)
	{
		p_ptr->body_parts[INVEN_BODY - INVEN_WIELD + i] = INVEN_BODY;
		p_ptr->body_parts[INVEN_OUTER - INVEN_WIELD + i] = INVEN_OUTER;
		p_ptr->body_parts[INVEN_LITE - INVEN_WIELD + i] = INVEN_LITE;
		p_ptr->body_parts[INVEN_AMMO - INVEN_WIELD + i] = INVEN_AMMO;
		p_ptr->body_parts[INVEN_CARRY - INVEN_WIELD + i] = INVEN_CARRY;
	}

	for (i = 0; i < body_parts[BODY_FINGER]; i++)
		p_ptr->body_parts[INVEN_RING - INVEN_WIELD + i] = INVEN_RING;

	for (i = 0; i < body_parts[BODY_HEAD]; i++)
	{
		p_ptr->body_parts[INVEN_HEAD - INVEN_WIELD + i] = INVEN_HEAD;
		p_ptr->body_parts[INVEN_NECK - INVEN_WIELD + i] = INVEN_NECK;
	}

	for (i = 0; i < b_arms; i++)
	{
		p_ptr->body_parts[INVEN_ARM - INVEN_WIELD + i] = INVEN_ARM;
		p_ptr->body_parts[INVEN_HANDS - INVEN_WIELD + i] = INVEN_HANDS;
	}
	if (body_parts[BODY_ARMS])
		p_ptr->body_parts[INVEN_TOOL - INVEN_WIELD] = INVEN_TOOL;

	for (i = 0; i < b_legs; i++)
		p_ptr->body_parts[INVEN_FEET - INVEN_WIELD + i] = INVEN_FEET;

	/* Ok now if the player lost a body part, he must drop the object he had on it */
	for (i = 0; i < INVEN_TOTAL - INVEN_WIELD; i++)
	{
		if ((!p_ptr->body_parts[i]) && p_ptr->inventory[i + INVEN_WIELD].k_ptr)
		{
			/* Drop it NOW ! */
			inven_takeoff(i + INVEN_WIELD, 255, true);
		}
	}
}

/* Should be called by every calc_bonus call */
void calc_body_bonus()
{
	auto const &r_info = game->edit_data.r_info;

	auto r_ptr = &r_info[p_ptr->body_monster];

	/* If in the player body nothing have to be done */
	if (!p_ptr->body_monster)
	{
		return;
	}

	if (p_ptr->disembodied)
	{
		p_ptr->wraith_form = true;
		return;
	}

	p_ptr->ac += r_ptr->ac;
	p_ptr->pspeed = r_ptr->speed;

	if (r_ptr->flags & RF_NEVER_MOVE) p_ptr->immovable = true;
	if (r_ptr->flags & RF_STUPID) p_ptr->stat_add[A_INT] -= 1;
	if (r_ptr->flags & RF_SMART) p_ptr->stat_add[A_INT] += 1;
	if (r_ptr->flags & RF_REFLECTING) p_ptr->reflect = true;
	if (r_ptr->flags & RF_INVISIBLE) p_ptr->invis += 20;
	if (r_ptr->flags & RF_REGENERATE) p_ptr->regenerate = true;
	if (r_ptr->flags & RF_AURA_FIRE) p_ptr->sh_fire = true;
	if (r_ptr->flags & RF_AURA_ELEC) p_ptr->sh_elec = true;
	if (r_ptr->flags & RF_PASS_WALL) p_ptr->wraith_form = true;
	if (r_ptr->flags & RF_SUSCEP_FIRE) p_ptr->sensible_fire = true;
	if (r_ptr->flags & RF_IM_ACID) p_ptr->resist_acid = true;
	if (r_ptr->flags & RF_IM_ELEC) p_ptr->resist_elec = true;
	if (r_ptr->flags & RF_IM_FIRE) p_ptr->resist_fire = true;
	if (r_ptr->flags & RF_IM_POIS) p_ptr->resist_pois = true;
	if (r_ptr->flags & RF_IM_COLD) p_ptr->resist_cold = true;
	if (r_ptr->flags & RF_RES_NETH) p_ptr->resist_neth = true;
	if (r_ptr->flags & RF_RES_NEXU) p_ptr->resist_nexus = true;
	if (r_ptr->flags & RF_RES_DISE) p_ptr->resist_disen = true;
	if (r_ptr->flags & RF_NO_FEAR) p_ptr->resist_fear = true;
	if (r_ptr->flags & RF_NO_SLEEP) p_ptr->free_act = true;
	if (r_ptr->flags & RF_NO_CONF) p_ptr->resist_conf = true;
	if (r_ptr->flags & RF_CAN_FLY) p_ptr->ffall = true;
	if (r_ptr->flags & RF_AQUATIC) p_ptr->water_breath = true;
}


/* Returns the number of extra blows based on abilities. */
static int get_extra_blows_ability() {
        /* Count bonus abilities */
        int num = 0;
	if (p_ptr->has_ability(AB_MAX_BLOW1)) num++;
	if (p_ptr->has_ability(AB_MAX_BLOW2)) num++;
        return num;
}

/* Returns the blow information based on class */
void analyze_blow(int *num, int *wgt, int *mul)
{
	*num = cp_ptr->blow_num;
	*wgt = cp_ptr->blow_wgt;
	*mul = cp_ptr->blow_mul;

	/* Count bonus abilities */
        (*num) += get_extra_blows_ability();
}

/* Are all the weapons wielded of the right type ? */
int get_weaponmastery_skill()
{
	int i, skill = 0;
	object_type *o_ptr;

	i = 0;
	/* All weapons must be of the same type */
	while ((p_ptr->body_parts[i] == INVEN_WIELD) && (i < INVEN_TOTAL))
	{
		o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		if (!o_ptr->k_ptr)
		{
			i++;
			continue;
		}
		switch (o_ptr->tval)
		{
		case TV_DAEMON_BOOK:
		case TV_SWORD:
			if ((!skill) || (skill == SKILL_SWORD)) skill = SKILL_SWORD;
			else skill = -1;
			break;
		case TV_AXE:
			if ((!skill) || (skill == SKILL_AXE)) skill = SKILL_AXE;
			else skill = -1;
			break;
		case TV_HAFTED:
			if ((!skill) || (skill == SKILL_HAFTED)) skill = SKILL_HAFTED;
			else skill = -1;
			break;
		case TV_POLEARM:
			if ((!skill) || (skill == SKILL_POLEARM)) skill = SKILL_POLEARM;
			else skill = -1;
			break;
		}
		i++;
	}

	/* Everything is ok */
	return skill;
}

/* Are all the ranged weapons wielded of the right type ? */
int get_archery_skill()
{
	int i, skill = 0;

	i = INVEN_BOW - INVEN_WIELD;
	/* All weapons must be of the same type */
	while (p_ptr->body_parts[i] == INVEN_BOW)
	{
		if (p_ptr->inventory[INVEN_WIELD + i].tval == TV_BOW)
		{
			switch (p_ptr->inventory[INVEN_WIELD + i].sval / 10)
			{
			case 0:
				if ((!skill) || (skill == SKILL_SLING)) skill = SKILL_SLING;
				else skill = -1;
				break;
			case 1:
				if ((!skill) || (skill == SKILL_BOW)) skill = SKILL_BOW;
				else skill = -1;
				break;
			case 2:
				if ((!skill) || (skill == SKILL_XBOW)) skill = SKILL_XBOW;
				else skill = -1;
				break;
			}
		}
		else
		{
			if ((!skill) || (skill == SKILL_BOOMERANG)) skill = SKILL_BOOMERANG;
			else skill = -1;
		}

		i++;
	}

	/* Everything is ok */
	return skill;
}

/* Apply gods */
static void calc_gods()
{
	/* Boost WIS if the player follows Eru */
	if (p_ptr->pgod == GOD_ERU)
	{
		if (p_ptr->grace > 10000) p_ptr->stat_add[A_WIS] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_WIS] += 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_WIS] += 1;
	}

	/* Boost str, con, chr and reduce int, wis if the player follows Melkor */
	if (p_ptr->pgod == GOD_MELKOR)
	{
		if (p_ptr->grace > 10000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_STR] += 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_CON] += 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_CHR] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_CHR] += 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_CHR] += 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_INT] -= 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_INT] -= 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_INT] -= 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_WIS] -= 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_WIS] -= 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_WIS] -= 1;

		if (praying_to(GOD_MELKOR))
		{
			if (p_ptr->grace > 5000) p_ptr->invis += 30;
			if (p_ptr->grace > 15000) p_ptr->immune_fire = true;
		}
		p_ptr->resist_fire = true;
	}

	/* Gifts of Manwe if the player is praying to Manwe */
	if (praying_to(GOD_MANWE))
	{
		s32b add = p_ptr->grace;

		/* provides speed every 5000 grace */
		if (add > 35000) add = 35000;
		add /= 5000;
		p_ptr->pspeed += add;

		/* Provides fly & FA */
		if (p_ptr->grace >= 7000) p_ptr->free_act = true;
		if (p_ptr->grace >= 15000) p_ptr->fly = true;
	}

	/* Manwe bonus not requiring the praying status */
	if (p_ptr->pgod == GOD_MANWE)
	{
		if (p_ptr->grace >= 2000) p_ptr->ffall = true;
	}

	/* Boost Str and Con if the player is following Tulkas */
	if (p_ptr->pgod == GOD_TULKAS)
	{
		if (p_ptr->grace > 5000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 10000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 15000) p_ptr->stat_add[A_CON] += 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 15000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_STR] += 1;
	}

	/* Aule provides to-hit/damage bonuses and fire resistance */
	if (p_ptr->pgod == GOD_AULE)
	{
		if (p_ptr->grace > 0)
		{
			int bonus;
			/* Resist fire*/
			if (p_ptr->grace > 5000)
			{
				p_ptr->resist_fire = true;
			}

			bonus = p_ptr->grace / 5000;
			if (bonus > 5)
			{
				bonus = 5;
			}

			p_ptr->to_h = p_ptr->to_h + bonus;
			p_ptr->dis_to_h = p_ptr->dis_to_h + bonus;
			p_ptr->to_d = p_ptr->to_d + bonus;
			p_ptr->dis_to_d = p_ptr->dis_to_d + bonus;
		}
	}

	/* Mandos provides nether resistance and, while praying,
	   nether immunity and prevents teleportation. */
	if (p_ptr->pgod == GOD_MANDOS)
	{
		p_ptr->resist_neth = true;

		if (p_ptr->praying && (p_ptr->grace > 10000))
		{
			p_ptr->resist_continuum = true;
		}

		if (p_ptr->praying && (p_ptr->grace > 20000))
		{
			p_ptr->immune_neth = true;
		}
	}

	/* Ulmo provides water breath and, while praying can
	   provide poison resistance and magic breath. */
	if (p_ptr->pgod == GOD_ULMO)
	{
		p_ptr->water_breath = true;

		if (p_ptr->praying && (p_ptr->grace > 1000))
		{
			p_ptr->resist_pois = true;
		}

		if (p_ptr->praying && (p_ptr->grace > 15000))
		{
			p_ptr->magical_breath = true;
		}
	}
}

/* Apply spell schools */
static void calc_schools()
{
	if (get_skill(SKILL_AIR) >= 50)
	{
		p_ptr->magical_breath = true;
	}

	if (get_skill(SKILL_WATER) >= 30)
	{
		p_ptr->water_breath = true;
	}
}

/* Apply corruptions */
static void calc_corruptions()
{
	auto &s_info = game->s_info;

	if (player_has_corruption(CORRUPT_BALROG_AURA))
	{
		p_ptr->xtra_flags |= TR_SH_FIRE;
		p_ptr->xtra_flags |= TR_LITE1;
	}

	if (player_has_corruption(CORRUPT_BALROG_WINGS))
	{
		p_ptr->xtra_flags |= TR_FLY;
		p_ptr->stat_add[A_CHR] -= 4;
		p_ptr->stat_add[A_DEX] -= 2;
	}

	if (player_has_corruption(CORRUPT_BALROG_STRENGTH))
	{
		p_ptr->stat_add[A_STR] += 3;
		p_ptr->stat_add[A_CON] += 1;
		p_ptr->stat_add[A_DEX] -= 3;
		p_ptr->stat_add[A_CHR] -= 1;
	}

	if (player_has_corruption(CORRUPT_DEMON_SPIRIT))
	{
		p_ptr->stat_add[A_INT] += 1;
		p_ptr->stat_add[A_CHR] -= 2;
	}

	if (player_has_corruption(CORRUPT_DEMON_HIDE))
	{
		p_ptr->to_a     = p_ptr->to_a     + p_ptr->lev;
		p_ptr->dis_to_a = p_ptr->dis_to_a + p_ptr->lev;
		p_ptr->pspeed = p_ptr->pspeed - (p_ptr->lev / 7);
		if (p_ptr->lev >= 40)
		{
			p_ptr->xtra_flags |= TR_IM_FIRE;
		}
	}

	if (player_has_corruption(CORRUPT_DEMON_REALM))
	{
		/* 1500 may seem a lot, but people are rather unlikely to
		   get the corruption very soon due to the dependencies. */
		if (s_info[SKILL_DAEMON].mod == 0)
		{
			s_info[SKILL_DAEMON].mod = 1500;
		}
		s_info[SKILL_DAEMON].hidden = false;
	}

	if (player_has_corruption(CORRUPT_RANDOM_TELEPORT))
	{
		p_ptr->xtra_flags |= TR_TELEPORT;
	}

	if (player_has_corruption(CORRUPT_ANTI_TELEPORT))
	{
		if (!p_ptr->corrupt_anti_teleport_stopped)
		{
			p_ptr->resist_continuum = true;
		}
	}

	if (player_has_corruption(CORRUPT_TROLL_BLOOD))
	{
		p_ptr->xtra_flags |= (TR_REGEN | TR_AGGRAVATE | ESP_TROLL);
	}
}

/* Apply flags */
static int extra_blows;
static int extra_shots;
void apply_flags(object_flag_set const &f, s16b pval, s16b tval, s16b to_h, s16b to_d, s16b to_a)
{
        s16b antimagic_mod;

	// Mix into computed flags
	p_ptr->computed_flags |= f;

	/* Affect stats */
	if (f & TR_STR) p_ptr->stat_add[A_STR] += pval;
	if (f & TR_INT) p_ptr->stat_add[A_INT] += pval;
	if (f & TR_WIS) p_ptr->stat_add[A_WIS] += pval;
	if (f & TR_DEX) p_ptr->stat_add[A_DEX] += pval;
	if (f & TR_CON) p_ptr->stat_add[A_CON] += pval;
	if (f & TR_CHR) p_ptr->stat_add[A_CHR] += pval;
	if (f & TR_LUCK) p_ptr->luck_cur += pval;

	/* Affect spell power */
	if (f & TR_SPELL) p_ptr->to_s += pval;

	/* Affect mana capacity */
	if (f & TR_MANA) p_ptr->to_m += pval;

	/* Affect life capacity */
	if (f & TR_LIFE) p_ptr->to_l += pval;

	/* Affect stealth */
	if (f & TR_STEALTH) p_ptr->skill_stl += pval;

	/* Affect infravision */
	if (f & TR_INFRA) p_ptr->see_infra += pval;

	/* Affect digging (factor of 20) */
	if (f & TR_TUNNEL) p_ptr->skill_dig += (pval * 20);

	/* Affect speed */
	if (f & TR_SPEED) p_ptr->pspeed += pval;

	/* Affect blows */
	if (f & TR_BLOWS) extra_blows += pval;
	if (f & TR_CRIT) p_ptr->xtra_crit += pval;

	/* Hack -- Sensible fire */
	if (f & TR_SENS_FIRE) p_ptr->sensible_fire = true;

	/* Hack -- cause earthquakes */
	if (f & TR_IMPACT) p_ptr->impact = true;

	/* Affect invisibility */
	if (f & TR_INVIS) p_ptr->invis += (pval * 10);

	/* Boost shots */
	if (f & TR_XTRA_SHOTS) extra_shots++;

	/* Various flags */
	if (f & TR_AGGRAVATE) p_ptr->aggravate = true;
	if (f & TR_TELEPORT) p_ptr->teleport = true;
	if (f & TR_DRAIN_MANA) p_ptr->drain_mana++;
	if (f & TR_DRAIN_HP) p_ptr->drain_life++;
	if (f & TR_DRAIN_EXP) p_ptr->exp_drain = true;
	if (f & TR_BLESSED) p_ptr->bless_blade = true;
	if (f & TR_XTRA_MIGHT) p_ptr->xtra_might += pval;
	if (f & TR_SLOW_DIGEST) p_ptr->slow_digest = true;
	if (f & TR_REGEN) p_ptr->regenerate = true;
	if ((tval != TV_LITE) && (f & TR_LITE1)) p_ptr->lite = true;
	if ((tval != TV_LITE) && (f & TR_LITE2)) p_ptr->lite = true;
	if ((tval != TV_LITE) && (f & TR_LITE3)) p_ptr->lite = true;
	if (f & TR_SEE_INVIS) p_ptr->see_inv = true;
	if (f & TR_FREE_ACT) p_ptr->free_act = true;
	if (f & TR_HOLD_LIFE) p_ptr->hold_life = true;
	if (f & TR_WRAITH) p_ptr->wraith_form = true;
	if (f & TR_FEATHER) p_ptr->ffall = true;
	if (f & TR_FLY) p_ptr->fly = true;
	if (f & TR_CLIMB) p_ptr->climb = true;

	/* Immunity flags */
	if (f & TR_IM_FIRE) p_ptr->immune_fire = true;
	if (f & TR_IM_ACID) p_ptr->immune_acid = true;
	if (f & TR_IM_COLD) p_ptr->immune_cold = true;
	if (f & TR_IM_ELEC) p_ptr->immune_elec = true;
	if (f & TR_IM_NETHER) p_ptr->immune_neth = true;

	/* Resistance flags */
	if (f & TR_RES_ACID) p_ptr->resist_acid = true;
	if (f & TR_RES_ELEC) p_ptr->resist_elec = true;
	if (f & TR_RES_FIRE) p_ptr->resist_fire = true;
	if (f & TR_RES_COLD) p_ptr->resist_cold = true;
	if (f & TR_RES_POIS) p_ptr->resist_pois = true;
	if (f & TR_RES_FEAR) p_ptr->resist_fear = true;
	if (f & TR_RES_CONF) p_ptr->resist_conf = true;
	if (f & TR_RES_SOUND) p_ptr->resist_sound = true;
	if (f & TR_RES_LITE) p_ptr->resist_lite = true;
	if (f & TR_RES_DARK) p_ptr->resist_dark = true;
	if (f & TR_RES_CHAOS) p_ptr->resist_chaos = true;
	if (f & TR_RES_DISEN) p_ptr->resist_disen = true;
	if (f & TR_RES_SHARDS) p_ptr->resist_shard = true;
	if (f & TR_RES_NEXUS) p_ptr->resist_nexus = true;
	if (f & TR_RES_BLIND) p_ptr->resist_blind = true;
	if (f & TR_RES_NETHER) p_ptr->resist_neth = true;

	if (f & TR_REFLECT) p_ptr->reflect = true;
	if (f & TR_SH_FIRE) p_ptr->sh_fire = true;
	if (f & TR_SH_ELEC) p_ptr->sh_elec = true;
	if (f & TR_NO_MAGIC) p_ptr->anti_magic = true;
	if (f & TR_NO_TELE) p_ptr->anti_tele = true;

	/* Sustain flags */
	if (f & TR_SUST_STR) p_ptr->sustain_str = true;
	if (f & TR_SUST_INT) p_ptr->sustain_int = true;
	if (f & TR_SUST_WIS) p_ptr->sustain_wis = true;
	if (f & TR_SUST_DEX) p_ptr->sustain_dex = true;
	if (f & TR_SUST_CON) p_ptr->sustain_con = true;
	if (f & TR_SUST_CHR) p_ptr->sustain_chr = true;

	if (f & TR_PRECOGNITION) p_ptr->precognition = true;

        antimagic_mod = to_h + to_d + to_a;

	if (f & TR_ANTIMAGIC_50)
	{
		s32b tmp;

                tmp = 10 + get_skill_scale(SKILL_ANTIMAGIC, 40) - antimagic_mod;
		if (tmp > 0) p_ptr->antimagic += tmp;

                tmp = 1 + get_skill_scale(SKILL_ANTIMAGIC, 4) - antimagic_mod / 15;
		if (tmp > 0) p_ptr->antimagic_dis += tmp;
	}

	/* The new code implementing Tolkien's concept of "Black Breath"
	 * takes advantage of the existing drain_exp character flag, renamed
	 * "black_breath". This flag can also be set by a unlucky blow from
	 * an undead.  -LM-
	 */
	if (f & TR_BLACK_BREATH) p_ptr->black_breath = true;

	if (f & TR_IMMOVABLE) p_ptr->immovable = true;

	/* Breaths */
	if (f & TR_WATER_BREATH)
	{
		p_ptr->water_breath = true;
	}

	if (f & TR_MAGIC_BREATH)
	{
		p_ptr->magical_breath = true;
		p_ptr->water_breath = true;
	}
}


/**
 * Apply player level flags
 */
template <class LF>
static void apply_lflags(LF const &lflags)
{
	for (int i = 1; i <= p_ptr->lev; i++)
	{
		apply_flags(lflags[i].oflags, lflags[i].pval, 0, 0, 0, 0);
	}
}


/**
 * Are barehand fighter's hands empty?
 */
static bool monk_empty_hands()
{
	if (p_ptr->melee_style != SKILL_HAND)
	{
		return false;
	}

	for (int i = 0; p_ptr->body_parts[i] == INVEN_WIELD; i++)
	{
		auto o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		if (o_ptr->k_ptr)
		{
			return false;
		}
	}

	return true;
}



/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * This function induces various "status" messages, unless silent is
 * true.
 */
void calc_bonuses(bool silent)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &r_info = game->edit_data.r_info;
	auto &s_info = game->s_info;
	auto const &a_info = game->edit_data.a_info;

	static bool monk_notify_aux = false;
	int i, j, hold;
	int old_speed;
	bool old_see_inv;
	int old_dis_ac;
	int old_dis_to_a;
	object_type *o_ptr;
	bool monk_armour_aux;


	/* Save the old computed_flags */
	auto old_computed_flags = p_ptr->computed_flags;

	/* Save the old speed */
	old_speed = p_ptr->pspeed;

	/* Save the old vision stuff */
	old_see_inv = p_ptr->see_inv;

	/* Save the old armor class */
	old_dis_ac = p_ptr->dis_ac;
	old_dis_to_a = p_ptr->dis_to_a;

	/* Clear extra blows/shots */
	extra_blows = extra_shots = 0;

	/* Clear the stat modifiers */
	for (i = 0; i < 6; i++) p_ptr->stat_add[i] = 0;

	/* Mana multiplier */
	p_ptr->to_m = 0;

	/* Life multiplier */
	p_ptr->to_l = 0;

	/* Spell power */
	p_ptr->to_s = 0;

	/* Clear the Displayed/Real armor class */
	p_ptr->dis_ac = p_ptr->ac = 0;

	/* Clear the Displayed/Real Bonuses */
	p_ptr->dis_to_h = p_ptr->to_h = p_ptr->to_h_melee = p_ptr->to_h_ranged = 0;
	p_ptr->dis_to_d = p_ptr->to_d = p_ptr->to_d_melee = p_ptr->to_d_ranged = 0;
	p_ptr->dis_to_a = p_ptr->to_a = 0;

	/* Start with "normal" speed */
	p_ptr->pspeed = 110;

	/* Start with 0% additionnal crits */
	p_ptr->xtra_crit = 0;

	/* Start with a single blow per turn */
	p_ptr->num_blow = 1;

	/* Start with a single shot per turn */
	p_ptr->num_fire = 1;

	/* Starts with single throwing damage */
	p_ptr->throw_mult = 1;

	/* Reset the "ammo" tval */
	p_ptr->tval_ammo = 0;

	/* Clear all the flags */
	p_ptr->invis = 0;
	p_ptr->immovable = false;
	p_ptr->aggravate = false;
	p_ptr->teleport = false;
	p_ptr->exp_drain = false;
	p_ptr->drain_mana = 0;
	p_ptr->drain_life = 0;
	p_ptr->bless_blade = false;
	p_ptr->xtra_might = 0;
	p_ptr->impact = false;
	p_ptr->see_inv = false;
	p_ptr->free_act = false;
	p_ptr->slow_digest = false;
	p_ptr->regenerate = false;
	p_ptr->fly = false;
	p_ptr->climb = false;
	p_ptr->ffall = false;
	p_ptr->hold_life = false;
	p_ptr->computed_flags = object_flag_set();
	p_ptr->lite = false;
	p_ptr->sustain_str = false;
	p_ptr->sustain_int = false;
	p_ptr->sustain_wis = false;
	p_ptr->sustain_con = false;
	p_ptr->sustain_dex = false;
	p_ptr->sustain_chr = false;
	p_ptr->resist_acid = false;
	p_ptr->resist_elec = false;
	p_ptr->resist_fire = false;
	p_ptr->resist_cold = false;
	p_ptr->resist_pois = false;
	p_ptr->resist_conf = false;
	p_ptr->resist_sound = false;
	p_ptr->resist_lite = false;
	p_ptr->resist_dark = false;
	p_ptr->resist_chaos = false;
	p_ptr->resist_disen = false;
	p_ptr->resist_shard = false;
	p_ptr->resist_nexus = false;
	p_ptr->resist_blind = false;
	p_ptr->resist_neth = false;
	p_ptr->resist_fear = false;
	p_ptr->resist_continuum = false;
	p_ptr->reflect = false;
	p_ptr->sh_fire = false;
	p_ptr->sh_elec = false;
	p_ptr->anti_magic = false;
	p_ptr->anti_tele = false;

	p_ptr->water_breath = false;
	p_ptr->magical_breath = false;

	p_ptr->sensible_fire = false;
	p_ptr->sensible_lite = false;

	p_ptr->immune_acid = false;
	p_ptr->immune_elec = false;
	p_ptr->immune_fire = false;
	p_ptr->immune_cold = false;
	p_ptr->immune_neth = false;

	p_ptr->precognition = false;

	p_ptr->wraith_form = false;

	/* The anti magic field surrounding the player */
	p_ptr->antimagic = 0;
	p_ptr->antimagic_dis = 0;


	/* Base infravision (purely racial) */
	p_ptr->see_infra = rp_ptr->infra + rmp_ptr->infra;

	/* Base skill -- magic devices */
	p_ptr->skill_dev = 0;

	/* Base skill -- saving throw */
	p_ptr->skill_sav = 0;

	/* Base skill -- stealth */
	p_ptr->skill_stl = 0;

	/* Base skill -- combat (normal) */
	p_ptr->skill_thn = 0;

	/* Base skill -- combat (shooting) */
	p_ptr->skill_thb = 0;

	/* Base skill -- combat (throwing) */
	p_ptr->skill_tht = 0;


	/* Base skill -- digging */
	p_ptr->skill_dig = 0;

	/* Xtra player flags */
	p_ptr->xtra_flags = object_flag_set();

	/* Hide the skills that should auto hide */
	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		if (s_descriptors[i].flags & SKF_AUTO_HIDE)
		{
			s_info[i].hidden = true;
		}
	}

	/* Base Luck */
	p_ptr->luck_cur = p_ptr->luck_base;

	/* Mimic override body's bonuses */
	if (p_ptr->mimic_form)
	{
		extra_blows += calc_mimic();
	}
	else
	{
		calc_body_bonus();
	}

	/* Take care of spell schools */
	calc_schools();

	/* Take care of corruptions */
	calc_corruptions();

	/* The powers gived by the wielded monster */
	calc_wield_monster();

	/* Apply all the level-dependent class flags */
	apply_lflags(cp_ptr->lflags);

	if (p_ptr->melee_style == SKILL_HAND)
	{
		/* Unencumbered Monks become faster every 10 levels */
		if (!(monk_heavy_armor()))
			p_ptr->pspeed += get_skill_scale(SKILL_HAND, 5);

		/* Free action if unencumbered at level 25 */
		if ((get_skill(SKILL_HAND) > 24) && !(monk_heavy_armor()))
			p_ptr->free_act = true;
	}

	if (get_skill(SKILL_ANTIMAGIC))
	{
		p_ptr->antimagic += get_skill(SKILL_ANTIMAGIC);
		p_ptr->antimagic_dis += get_skill_scale(SKILL_ANTIMAGIC, 10) + 1;

		if (p_ptr->antimagic_extra & CLASS_ANTIMAGIC)
		{
			p_ptr->anti_tele = true;
			p_ptr->resist_continuum = true;
		}
	}

	if (get_skill(SKILL_DAEMON) > 20) p_ptr->resist_conf = true;
	if (get_skill(SKILL_DAEMON) > 30) p_ptr->resist_fear = true;

	if ( get_skill(SKILL_MINDCRAFT) >= 40 )
	{
		p_ptr->computed_flags |= ESP_ALL;
	}

	if (p_ptr->astral)
	{
		p_ptr->wraith_form = true;
	}

	/***** Races ****/
	if ((!p_ptr->mimic_form) && (!p_ptr->body_monster))
	{
		/* Apply level-dependent flags from race/sub-race */
		apply_lflags(rp_ptr->lflags);
		apply_lflags(rmp_ptr->lflags);

		/* Is the player's race hurt by light? */
		if (race_flags_p(PR_HURT_LITE))
		{
			p_ptr->sensible_lite = true;
		}
	}

	/* The extra flags */
	apply_flags(p_ptr->xtra_flags, 0, 0, 0, 0, 0);

	/* Apply the racial modifiers */
	for (i = 0; i < 6; i++)
	{
		/* Modify the stats for "race" */
		p_ptr->stat_add[i] += (rp_ptr->ps.adj[i] + rmp_ptr->ps.adj[i] + cp_ptr->ps.adj[i]);
	}


	/* Scan the usable inventory */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Extract the item flags */
		object_flags_no_set = true;
		auto flags = object_flags(o_ptr);
		object_flags_no_set = false;

		/* MEGA ugly hack -- set spacetime distortion resistance */
		if (o_ptr->name1 == ART_ANCHOR)
		{
			p_ptr->resist_continuum = true;
		}

		/* Hack - don't give the Black Breath when merely inspecting a weapon */
		if (silent)
		{
			flags &= ~TR_BLACK_BREATH;
		}

		apply_flags(flags, o_ptr->pval, o_ptr->tval, o_ptr->to_h, o_ptr->to_d, o_ptr->to_a);

		if (o_ptr->name1)
		{
			apply_set(o_ptr->name1, a_info[o_ptr->name1].set);
		}

		/* Modify the base armor class */
		p_ptr->ac += o_ptr->ac;

		/* The base armor class is always known */
		p_ptr->dis_ac += o_ptr->ac;

		/* Apply the bonuses to armor class */
		p_ptr->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		p_ptr->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (p_ptr->body_parts[i - INVEN_WIELD] == INVEN_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (p_ptr->body_parts[i - INVEN_WIELD] == INVEN_BOW) continue;

		/* Hack -- do not apply "ammo" bonuses */
		if (p_ptr->body_parts[i - INVEN_WIELD] == INVEN_AMMO) continue;

		/* Hack -- do not apply "tool" bonuses */
		if (p_ptr->body_parts[i - INVEN_WIELD] == INVEN_TOOL) continue;

		/* Apply the bonuses to hit/damage */
		p_ptr->to_h += o_ptr->to_h;
		p_ptr->to_d += o_ptr->to_d;

		/* Apply the mental bonuses tp hit/damage, if known */
		p_ptr->dis_to_h += o_ptr->to_h;
		p_ptr->dis_to_d += o_ptr->to_d;
	}

	/* Monks get extra ac for armour _not worn_ */
	if ((p_ptr->melee_style == SKILL_HAND) && !(monk_heavy_armor()))
	{
		if (!(p_ptr->inventory[INVEN_BODY].k_ptr))
		{
			p_ptr->to_a += get_skill_scale(SKILL_HAND, 75);
			p_ptr->dis_to_a += get_skill_scale(SKILL_HAND, 75);
		}
		if (!(p_ptr->inventory[INVEN_OUTER].k_ptr) && (get_skill(SKILL_HAND) > 15))
		{
			p_ptr->to_a += ((get_skill(SKILL_HAND) - 13) / 3);
			p_ptr->dis_to_a += ((get_skill(SKILL_HAND) - 13) / 3);
		}
		if (!(p_ptr->inventory[INVEN_ARM].k_ptr) && (get_skill(SKILL_HAND) > 10))
		{
			p_ptr->to_a += ((get_skill(SKILL_HAND) - 8) / 3);
			p_ptr->dis_to_a += ((get_skill(SKILL_HAND) - 8) / 3);
		}
		if (!(p_ptr->inventory[INVEN_HEAD].k_ptr) && (get_skill(SKILL_HAND) > 4))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) - 2) / 3;
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) - 2) / 3;
		}
		if (!(p_ptr->inventory[INVEN_HANDS].k_ptr))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) / 2);
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) / 2);
		}
		if (!(p_ptr->inventory[INVEN_FEET].k_ptr))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) / 3);
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) / 3);
		}
	}

	/* Hack -- aura of fire also provides light */
	if (p_ptr->sh_fire)
	{
		p_ptr->lite = true;
	}

	if (race_flags_p(PR_AC_LEVEL))
	{
		p_ptr->to_a += 20 + (p_ptr->lev / 5);
		p_ptr->dis_to_a += 20 + (p_ptr->lev / 5);
	}

	/* Take care of gods */
	calc_gods();

	/* Calculate stats */
	for (i = 0; i < 6; i++)
	{
		int top, use, ind;


		/* Extract the new "stat_use" value for the stat */
		top = modify_stat_value(p_ptr->stat_max[i], p_ptr->stat_add[i]);

		/* Notice changes */
		if (p_ptr->stat_top[i] != top)
		{
			/* Save the new value */
			p_ptr->stat_top[i] = top;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}

		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(p_ptr->stat_cur[i], p_ptr->stat_add[i]);

		/* Notice changes */
		if (p_ptr->stat_use[i] != use)
		{
			/* Save the new value */
			p_ptr->stat_use[i] = use;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}


		/* Values: 3, 4, ..., 17 */
		if (use <= 18) ind = (use - 3);

		/* Ranges: 18/00-18/09, ..., 18/210-18/219 */
		else if (use <= 18 + 219) ind = (15 + (use - 18) / 10);

		/* Range: 18/220+ */
		else ind = (37);

		/* Notice changes */
		if (p_ptr->stat_ind[i] != ind)
		{
			/* Save the new index */
			p_ptr->stat_ind[i] = ind;

			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				p_ptr->update |= (PU_HP);
			}

			/* Change in WIS affects Sanity Points */
			else if (i == A_WIS)
			{
				p_ptr->update |= (PU_MANA | PU_SANITY);
			}

			/* Change in spell stat affects Mana/Spells */
			if (i == A_INT)
			{
				p_ptr->update |= (PU_MANA | PU_SPELLS);
			}

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
	}

	/* Provide the damage we get from sacrifice */
	if (p_ptr->pgod == GOD_MELKOR)
	{
		int x = wisdom_scale(4);
		if (x < 1) x = 1;

		p_ptr->dis_to_d += x * p_ptr->melkor_sacrifice;
		p_ptr->to_d += x * p_ptr->melkor_sacrifice;
	}

	/* jk - add in the tactics */
	p_ptr->dis_to_h += tactic_info[(byte)p_ptr->tactic].to_hit;
	p_ptr->to_h += tactic_info[(byte)p_ptr->tactic].to_hit;
	p_ptr->dis_to_d += tactic_info[(byte)p_ptr->tactic].to_dam;
	p_ptr->to_d += tactic_info[(byte)p_ptr->tactic].to_dam;
	p_ptr->dis_to_a += tactic_info[(byte)p_ptr->tactic].to_ac;
	p_ptr->to_a += tactic_info[(byte)p_ptr->tactic].to_ac;

	p_ptr->skill_stl += tactic_info[(byte)p_ptr->tactic].to_stealth;
	p_ptr->skill_sav += tactic_info[(byte)p_ptr->tactic].to_saving;

	p_ptr->pspeed += move_info[(byte)p_ptr->movement].to_speed;
	p_ptr->skill_stl += move_info[(byte)p_ptr->movement].to_stealth;

	/* Apply temporary "stun" */
	if (p_ptr->stun > 50)
	{
		p_ptr->to_h -= 20;
		p_ptr->dis_to_h -= 20;
		p_ptr->to_d -= 20;
		p_ptr->dis_to_d -= 20;
	}
	else if (p_ptr->stun)
	{
		p_ptr->to_h -= 5;
		p_ptr->dis_to_h -= 5;
		p_ptr->to_d -= 5;
		p_ptr->dis_to_d -= 5;
	}


	/* Invulnerability */
	if (p_ptr->invuln)
	{
		p_ptr->to_a += 100;
		p_ptr->dis_to_a += 100;
	}

	/* Temporary precognition */
	if (p_ptr->tim_precognition > 0)
	{
		apply_flags(TR_PRECOGNITION, 0, 0, 0, 0, 0);
	}

	/* Breath */
	if (p_ptr->tim_water_breath)
	{
		p_ptr->water_breath = true;
	}
	if (p_ptr->tim_magic_breath)
	{
		p_ptr->magical_breath = true;
	}

	/* wraith_form */
	if (p_ptr->tim_wraith)
	{
		if (p_ptr->disembodied)
		{
			p_ptr->to_a += 10;
			p_ptr->dis_to_a += 10;
		}
		else
		{
			p_ptr->to_a += 50;
			p_ptr->dis_to_a += 50;
			p_ptr->reflect = true;
		}
		p_ptr->wraith_form = true;
	}

	/* Temporary holy aura */
	if (p_ptr->holy)
	{
		p_ptr->hold_life = true;
		p_ptr->luck_cur += 5;
	}

	/* Temporary blessing */
	if (p_ptr->blessed)
	{
		p_ptr->to_a += 5;
		p_ptr->dis_to_a += 5;
		p_ptr->to_h += 10;
		p_ptr->dis_to_h += 10;
	}

	/* Temporary invisibility */
	if (p_ptr->tim_invisible)
	{
		p_ptr->invis += p_ptr->tim_inv_pow;
	}

	/* Temporary shield */
	if (p_ptr->shield)
	{
		p_ptr->to_a += p_ptr->shield_power;
		p_ptr->dis_to_a += p_ptr->shield_power;
	}

	/* Temporary "Hero" */
	if (p_ptr->hero)
	{
		p_ptr->to_h += 12;
		p_ptr->dis_to_h += 12;
	}

	/* Temporary "roots" */
	if (p_ptr->tim_roots)
	{
		set_stun(0);
		p_ptr->to_d_melee += p_ptr->tim_roots_dam;
		p_ptr->to_a += p_ptr->tim_roots_ac;
		p_ptr->dis_to_a += p_ptr->tim_roots_ac;
	}

	/* Temporary "Beserk" */
	if (p_ptr->shero)
	{
		p_ptr->to_h += 24;
		p_ptr->dis_to_h += 24;
		p_ptr->to_a -= 10;
		p_ptr->dis_to_a -= 10;
	}

	/* Temporary "Accurancy" */
	if (p_ptr->strike)
	{
		p_ptr->to_d += 15;
		p_ptr->dis_to_d += 15;
		p_ptr->to_h += 15;
		p_ptr->dis_to_h += 15;
	}

	/* Temporary "Reflection" */
	if (p_ptr->tim_reflect)
	{
		p_ptr->reflect = true;
	}

	/* Temporary "Levitation" and "Flying" */
	if (p_ptr->tim_ffall)
	{
		p_ptr->ffall = true;
	}
	if (p_ptr->tim_fly)
	{
		p_ptr->fly = true;
	}

	/* Oppose Chaos & Confusion */
	if (p_ptr->oppose_cc)
	{
		p_ptr->resist_chaos = true;
		p_ptr->resist_conf = true;
	}

	/* Temporary "fast" */
	if (p_ptr->fast)
	{
		p_ptr->pspeed += p_ptr->speed_factor;
	}

	/* Temporary "light speed" */
	if (p_ptr->lightspeed)
	{
		p_ptr->pspeed += 50;
	}

	/* Temporary "slow" */
	if (p_ptr->slow)
	{
		p_ptr->pspeed -= 10;
	}

	if (p_ptr->tim_esp)
	{
		p_ptr->computed_flags |= ESP_ALL;
	}

	/* Temporary see invisible */
	if (p_ptr->tim_invis)
	{
		p_ptr->see_inv = true;
	}

	/* Temporary infravision boost */
	if (p_ptr->tim_infra)
	{
		p_ptr->see_infra++;
	}

	/* Hack -- Magic breath -> Water breath */
	if (p_ptr->magical_breath)
	{
		p_ptr->water_breath = true;
	}

	/* Hack -- Can Fly -> Can Levitate */
	if (p_ptr->fly)
	{
		p_ptr->ffall = true;
	}

	/* Hack -- Res Chaos -> Res Conf */
	if (p_ptr->resist_chaos)
	{
		p_ptr->resist_conf = true;
	}

	/* Hack -- Hero/Shero -> Res fear */
	if (p_ptr->hero || p_ptr->shero)
	{
		p_ptr->resist_fear = true;
	}


	/* Hack -- Telepathy Change */
	{
		auto const &esp_mask = object_flags_esp();
		if ((p_ptr->computed_flags & esp_mask) != (old_computed_flags & esp_mask))
		{
			p_ptr->update |= (PU_MONSTERS);
		}
	}

	/* Hack -- See Invis Change */
	if (p_ptr->see_inv != old_see_inv)
	{
		p_ptr->update |= (PU_MONSTERS);
	}


	/* Extract the current weight (in tenth pounds) */
	j = calc_total_weight();

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit();

	/* XXX XXX XXX Apply "encumbrance" from weight */
	if (j > i / 2) p_ptr->pspeed -= ((j - (i / 2)) / (i / 10));

	/* Bloating slows the player down (a little) */
	if (p_ptr->food >= PY_FOOD_MAX) p_ptr->pspeed -= 10;

	/* Display the speed (if needed) */
	if (p_ptr->pspeed != old_speed) p_ptr->redraw |= (PR_FRAME);


	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
	p_ptr->to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->dis_to_a += ((int)(adj_dex_ta[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->dis_to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128);
	p_ptr->dis_to_h += ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
	p_ptr->dis_to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128);

	/* Redraw armor (if needed) */
	if ((p_ptr->dis_ac != old_dis_ac) || (p_ptr->dis_to_a != old_dis_to_a))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}


	/* Obtain the "hold" value */
	hold = adj_str_hold[p_ptr->stat_ind[A_STR]];


	/* Examine the "current bow" */
	o_ptr = &p_ptr->inventory[INVEN_BOW];


	/* Assume not heavy */
	p_ptr->heavy_shoot = false;

	/* It is hard to carholdry a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
		p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		p_ptr->heavy_shoot = true;
	}

	/* Take note of required "tval" for missiles */
	switch (o_ptr->sval)
	{
	case SV_SLING:
		{
			p_ptr->tval_ammo = TV_SHOT;
			break;
		}

	case SV_SHORT_BOW:
	case SV_LONG_BOW:
		{
			p_ptr->tval_ammo = TV_ARROW;
			break;
		}

	case SV_LIGHT_XBOW:
	case SV_HEAVY_XBOW:
		{
			p_ptr->tval_ammo = TV_BOLT;
			break;
		}
	}

	/* Compute "extra shots" if needed */
	if (o_ptr->k_ptr && !p_ptr->heavy_shoot)
	{
		int archery = get_archery_skill();

		if (archery != -1)
		{
			p_ptr->to_h_ranged += get_skill_scale(archery, 25);
			p_ptr->num_fire += (get_skill(archery) / 16);
			p_ptr->xtra_might += (get_skill(archery) / 25);
			switch (archery)
			{
			case SKILL_SLING:
				if (p_ptr->tval_ammo == TV_SHOT) p_ptr->xtra_might += get_skill(archery) / 30;
				break;
			case SKILL_BOW:
				if (p_ptr->tval_ammo == TV_ARROW) p_ptr->xtra_might += get_skill(archery) / 30;
				break;
			case SKILL_XBOW:
				if (p_ptr->tval_ammo == TV_BOLT) p_ptr->xtra_might += get_skill(archery) / 30;
				break;
			}
		}

		/* Add in the "bonus shots" */
		p_ptr->num_fire += extra_shots;

		/* Require at least one shot */
		if (p_ptr->num_fire < 1) p_ptr->num_fire = 1;
	}

	if (race_flags_p(PR_XTRA_MIGHT_BOW) && p_ptr->tval_ammo == TV_ARROW)
		p_ptr->xtra_might += 1;

	if (race_flags_p(PR_XTRA_MIGHT_SLING) && p_ptr->tval_ammo == TV_SHOT)
		p_ptr->xtra_might += 1;

	if (race_flags_p(PR_XTRA_MIGHT_XBOW) && p_ptr->tval_ammo == TV_BOLT)
		p_ptr->xtra_might += 1;

	/* Examine the "current tool" */
	o_ptr = &p_ptr->inventory[INVEN_TOOL];

	/* Boost digging skill by tool weight */
	if (o_ptr->k_ptr && (o_ptr->tval == TV_DIGGING))
	{
		p_ptr->skill_dig += (o_ptr->weight / 10);
	}

	/* Examine the main weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Assume not heavy */
	p_ptr->heavy_wield = false;

	/* Normal weapons */
	if (o_ptr->k_ptr && !p_ptr->heavy_wield)
	{
		int str_index, dex_index;

		int num = 0, wgt = 0, mul = 0, div = 0;

		analyze_blow(&num, &wgt, &mul);

		/* Enforce a minimum "weight" (tenth pounds) */
		div = ((o_ptr->weight < wgt) ? wgt : o_ptr->weight);

		/* Access the strength vs weight */
		str_index = (adj_str_blow[p_ptr->stat_ind[A_STR]] * mul / div);

		/* Maximal value */
		if (str_index > 11) str_index = 11;

		/* Index by dexterity */
		dex_index = (adj_dex_blow[p_ptr->stat_ind[A_DEX]]);

		/* Maximal value */
		if (dex_index > 11) dex_index = 11;

		/* Use the blows table */
		p_ptr->num_blow = blows_table[str_index][dex_index];

		/* Maximal value */
		if (p_ptr->num_blow > num) p_ptr->num_blow = num;

		/* Add in the "bonus blows" */
		p_ptr->num_blow += extra_blows;

		/* Special class bonus blows */
		p_ptr->num_blow += p_ptr->lev * cp_ptr->extra_blows / 50;

		/* Weapon specialization bonus blows */
		if (get_weaponmastery_skill() != -1)
			p_ptr->num_blow += get_skill_scale(get_weaponmastery_skill(), 2);

		/* Bonus blows for plain weaponmastery skill */
		p_ptr->num_blow += get_skill_scale(SKILL_MASTERY, 3);

		/* Require at least one blow */
		if (p_ptr->num_blow < 1) p_ptr->num_blow = 1;
	}
	/* Different calculation for bear form with empty hands */
	else if ((p_ptr->melee_style == SKILL_HAND) && monk_empty_hands())
	{
		int plev = get_skill(SKILL_HAND);

                p_ptr->num_blow = get_extra_blows_ability();

		if (plev > 9) p_ptr->num_blow++;
		if (plev > 19) p_ptr->num_blow++;
		if (plev > 29) p_ptr->num_blow++;
		if (plev > 34) p_ptr->num_blow++;
		if (plev > 39) p_ptr->num_blow++;
		if (plev > 44) p_ptr->num_blow++;
		if (plev > 49) p_ptr->num_blow++;

		if (monk_heavy_armor()) p_ptr->num_blow /= 2;

		p_ptr->num_blow += 1 + extra_blows;

		if (!monk_heavy_armor())
		{
			p_ptr->to_h += (plev / 3);
			p_ptr->to_d += (plev / 3);

			p_ptr->dis_to_h += (plev / 3);
			p_ptr->dis_to_d += (plev / 3);
		}
	}

	/* Monsters that only have their "natural" attacks */
	else if (!r_info[p_ptr->body_monster].body_parts[BODY_WEAPON])
	{
		int num = 0;
		int wgt = 0;
		int mul = 0;
		analyze_blow(&num, &wgt, &mul);

		/* Access the strength vs weight */
		int str_index = (adj_str_blow[p_ptr->stat_ind[A_STR]] * mul / 3);

		/* Maximal value */
		if (str_index > 11) str_index = 11;

		/* Index by dexterity */
		int dex_index = (adj_dex_blow[p_ptr->stat_ind[A_DEX]]);

		/* Maximal value */
		if (dex_index > 11) dex_index = 11;

		/* Use the blows table */
		p_ptr->num_blow = blows_table[str_index][dex_index];

		/* Add in the "bonus blows" */
		p_ptr->num_blow += extra_blows;

		/* Maximal value */
		if (p_ptr->num_blow > 4) p_ptr->num_blow = 4;

		/* Require at least one blow */
		if (p_ptr->num_blow < 1) p_ptr->num_blow = 1;

		/* Limit as defined by monster body */
		auto r_ptr = race_info_idx(p_ptr->body_monster, 0);
		for (num = 0; num < p_ptr->num_blow; num++)
			if (!r_ptr->blow[num].effect)
				break;
		p_ptr->num_blow = num;
	}

	/* Different calculation for monks with empty hands */
	else if ((!p_ptr->body_monster) && (p_ptr->mimic_form == resolve_mimic_name("Bear")) && (p_ptr->melee_style == SKILL_BEAR))
	{
		int plev = get_skill(SKILL_BEAR);

		p_ptr->num_blow = 0;

		p_ptr->num_blow += 2 + (plev / 5) + extra_blows;

		p_ptr->to_h -= (plev / 5);
		p_ptr->dis_to_h -= (plev / 5);

		p_ptr->to_d += (plev / 2);
		p_ptr->dis_to_d += (plev / 2);
	}

	/* Assume okay */
	p_ptr->icky_wield = false;
	monk_armour_aux = false;

	if (get_weaponmastery_skill() != -1)
	{
		int lev = get_skill(get_weaponmastery_skill());

		p_ptr->to_h_melee += lev;
		p_ptr->to_d_melee += lev / 2;
	}

	if (get_skill(SKILL_COMBAT))
	{
		int lev = get_skill_scale(SKILL_COMBAT, 10);

		p_ptr->to_d += lev;
		p_ptr->dis_to_d += lev;
	}

	if (get_skill(SKILL_DODGE))
	{
		/* Get the armor weight */
		int cur_wgt = 0;

		cur_wgt += p_ptr->inventory[INVEN_BODY].weight;
		cur_wgt += p_ptr->inventory[INVEN_HEAD].weight;
		cur_wgt += p_ptr->inventory[INVEN_ARM].weight;
		cur_wgt += p_ptr->inventory[INVEN_OUTER].weight;
		cur_wgt += p_ptr->inventory[INVEN_HANDS].weight;
		cur_wgt += p_ptr->inventory[INVEN_FEET].weight;

		/* Base dodge chance */
		p_ptr->dodge_chance = get_skill_scale(SKILL_DODGE, 150) + get_skill(SKILL_HAND);

		/* Armor weight bonus/penalty */
		p_ptr->dodge_chance -= cur_wgt * 2;

		/* Encumberance bonus/penalty */
		p_ptr->dodge_chance = p_ptr->dodge_chance - (calc_total_weight() / 100);

		/* Never below 0 */
		if (p_ptr->dodge_chance < 0) p_ptr->dodge_chance = 0;
	}
	else
	{
		p_ptr->dodge_chance = 0;
	}

	/* Parse all the weapons */
	i = 0;
	while (p_ptr->body_parts[i] == INVEN_WIELD)
	{
		o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		/* 2handed weapon and shield = less damage */
		if (o_ptr->k_ptr && p_ptr->inventory[INVEN_ARM + i].k_ptr)
		{
			auto const flags = object_flags(&p_ptr->inventory[INVEN_WIELD + i]);
			if (flags & TR_COULD2H)
			{
				int tmp;

				/* Reduce the bonuses */
				tmp = o_ptr->to_h / 2;
				if (tmp < 0) tmp = -tmp;
				p_ptr->to_h_melee -= tmp;

				tmp = o_ptr->to_d / 2;
				if (tmp < 0) tmp = -tmp;
				tmp += (o_ptr->dd * o_ptr->ds) / 2;
				p_ptr->to_d_melee -= tmp;
			}
		}

		/* Priest weapon penalty for non-blessed edged weapons */
		if (((forbid_non_blessed()) &&
			(!p_ptr->bless_blade) &&
			((o_ptr->tval == TV_AXE) || (o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM))) &&
			o_ptr->k_ptr)
		{
			/* Reduce the real bonuses */
			p_ptr->to_h -= 15;
			p_ptr->to_d -= 15;

			/* Reduce the mental bonuses */
			p_ptr->dis_to_h -= 15;
			p_ptr->dis_to_d -= 15;

			/* Icky weapon */
			p_ptr->icky_wield = true;
		}

		/* Sorcerer can't wield a weapon unless it's a mage staff */
		if (get_skill(SKILL_SORCERY))
		{
			int malus = get_skill_scale(SKILL_SORCERY, 100);

			if ((o_ptr->tval != TV_MSTAFF) &&
				o_ptr->k_ptr)
			{
				/* Reduce the real bonuses */
				p_ptr->to_h -= malus;
				p_ptr->to_d -= malus;

				/* Reduce the mental bonuses */
				p_ptr->dis_to_h -= malus;
				p_ptr->dis_to_d -= malus;

				/* Icky weapon */
				p_ptr->icky_wield = true;
			}
			else
			{
				/* Reduce the real bonuses */
				p_ptr->to_h -= malus / 10;
				p_ptr->to_d -= malus / 10;

				/* Reduce the mental bonuses */
				p_ptr->dis_to_h -= malus / 10;
				p_ptr->dis_to_d -= malus / 10;
			}
		}

		/* Check next weapon */
		i++;
	}

	if (monk_heavy_armor())
	{
		monk_armour_aux = true;
	}

	/* Affect Skill -- stealth (bonus one) */
	p_ptr->skill_stl += 1;

	/* Affect Skill -- saving throw (WIS) */
	p_ptr->skill_sav += adj_wis_sav[p_ptr->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	p_ptr->skill_dig += adj_str_dig[p_ptr->stat_ind[A_STR]];

	/* Affect Skill -- magic devices (skill) */
	p_ptr->skill_dev += (get_skill_scale(SKILL_DEVICE, 150));

	/* Affect Skill -- saving throw (skill and level) */
	p_ptr->skill_sav += (get_skill_scale(SKILL_SPIRITUALITY, 75));

	/* Affect Skill -- stealth (skill) */
	p_ptr->skill_stl += (get_skill_scale(SKILL_STEALTH, 25));

	/* Affect Skill -- combat (Combat skill + mastery) */
	p_ptr->skill_thn += (50 * (((7 * get_skill(p_ptr->melee_style)) + (3 * get_skill(SKILL_COMBAT))) / 10) / 10);

	/* Affect Skill -- combat (shooting) (Level, by Class) */
	p_ptr->skill_thb += (50 * (((7 * get_skill(SKILL_ARCHERY)) + (3 * get_skill(SKILL_COMBAT))) / 10) / 10);

	/* Affect Skill -- combat (throwing) (Level) */
	p_ptr->skill_tht += (50 * p_ptr->lev / 10);


	/* Limit Skill -- stealth from 0 to 30 */
	if (p_ptr->skill_stl > 30) p_ptr->skill_stl = 30;
	if (p_ptr->skill_stl < 0) p_ptr->skill_stl = 0;

	/* Limit Skill -- digging from 1 up */
	if (p_ptr->skill_dig < 1) p_ptr->skill_dig = 1;

	if ((p_ptr->anti_magic) && (p_ptr->skill_sav < 95)) p_ptr->skill_sav = 95;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "heavy bow" changes */
	if (p_ptr->old_heavy_shoot != p_ptr->heavy_shoot)
	{
		if (silent)
		{
			/* do nothing */
		}
		/* Message */
		else if (p_ptr->heavy_shoot)
		{
			msg_print("You have trouble wielding such a heavy bow.");
		}
		else if (p_ptr->inventory[INVEN_BOW].k_ptr)
		{
			msg_print("You have no trouble wielding your bow.");
		}
		else
		{
			msg_print("You feel relieved to put down your heavy bow.");
		}

		/* Save it */
		p_ptr->old_heavy_shoot = p_ptr->heavy_shoot;
	}


	/* Take note when "heavy weapon" changes */
	if (p_ptr->old_heavy_wield != p_ptr->heavy_wield)
	{
		if (silent)
		{
			/* do nothing */
		}
		/* Message */
		else if (p_ptr->heavy_wield)
		{
			msg_print("You have trouble wielding such a heavy weapon.");
		}
		else if (p_ptr->inventory[INVEN_WIELD].k_ptr)
		{
			msg_print("You have no trouble wielding your weapon.");
		}
		else
		{
			msg_print("You feel relieved to put down your heavy weapon.");
		}

		/* Save it */
		p_ptr->old_heavy_wield = p_ptr->heavy_wield;
	}


	/* Take note when "illegal weapon" changes */
	if (p_ptr->old_icky_wield != p_ptr->icky_wield)
	{
		if (silent)
		{
			/* do nothing */
		}
		/* Message */
		else if (p_ptr->icky_wield)
		{
			msg_print("You do not feel comfortable with your weapon.");
		}
		else if (p_ptr->inventory[INVEN_WIELD].k_ptr)
		{
			msg_print("You feel comfortable with your weapon.");
		}
		else
		{
			msg_print("You feel more comfortable after removing your weapon.");
		}

		/* Save it */
		p_ptr->old_icky_wield = p_ptr->icky_wield;
	}

	if (monk_armour_aux != monk_notify_aux)
	{
		if ((p_ptr->melee_style != SKILL_HAND) || silent)
		{
			/* do nothing */
		}
		else if (monk_heavy_armor())
			msg_print("The weight of your armor disrupts your balance.");
		else
			msg_print("You regain your balance.");
		monk_notify_aux = monk_armour_aux;
	}

	/* Resist lite & senseible lite negates one an other */
	if (p_ptr->resist_lite && p_ptr->sensible_lite)
	{
		p_ptr->resist_lite = p_ptr->sensible_lite = false;
	}

	/* resistance to fire cancel sensibility to fire */
	if (p_ptr->resist_fire || p_ptr->oppose_fire || p_ptr->immune_fire)
		p_ptr->sensible_fire = false;

	/* Minimum saving throw */
	if(p_ptr->skill_sav <= 10)
		p_ptr->skill_sav = 10;
	else
		p_ptr->skill_sav += 10;
}



/*
 * Handle "p_ptr->notice"
 */
void notice_stuff()
{
	/* Notice stuff */
	if (!p_ptr->notice) return;


	/* Combine the pack */
	if (p_ptr->notice & (PN_COMBINE))
	{
		p_ptr->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Reorder the pack */
	if (p_ptr->notice & (PN_REORDER))
	{
		p_ptr->notice &= ~(PN_REORDER);
		reorder_pack();
	}
}


/*
 * Handle "p_ptr->update"
 */
void update_stuff()
{
	/* Update stuff */
	if (!p_ptr->update) return;


	if (p_ptr->update & (PU_BODY))
	{
		p_ptr->update &= ~(PU_BODY);
		calc_body();
	}

	if (p_ptr->update & (PU_BONUS))
	{
		/* Ok now THAT is an ugly hack */
		p_ptr->update &= ~(PU_POWERS);
		calc_powers();

		p_ptr->update &= ~(PU_BONUS);
		calc_bonuses(false);
	}

	if (p_ptr->update & (PU_TORCH))
	{
		p_ptr->update &= ~(PU_TORCH);
		calc_torch();
	}

	if (p_ptr->update & (PU_HP))
	{
		p_ptr->update &= ~(PU_HP);
		calc_hitpoints();
	}

	if (p_ptr->update & (PU_SANITY))
	{
		p_ptr->update &= ~(PU_SANITY);
		calc_sanity();
	}

	if (p_ptr->update & (PU_MANA))
	{
		p_ptr->update &= ~(PU_MANA);
		calc_mana();
	}

	if (p_ptr->update & (PU_SPELLS))
	{
		p_ptr->update &= ~(PU_SPELLS);
	}

	if (p_ptr->update & (PU_POWERS))
	{
		p_ptr->update &= ~(PU_POWERS);
		calc_powers();
	}

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	if (p_ptr->update & (PU_UN_VIEW))
	{
		p_ptr->update &= ~(PU_UN_VIEW);
		forget_view();
	}

	if (p_ptr->update & (PU_VIEW))
	{
		p_ptr->update &= ~(PU_VIEW);
		update_view();
	}

	if (p_ptr->update & (PU_FLOW))
	{
		p_ptr->update &= ~(PU_FLOW);
		update_flow();
	}

	if (p_ptr->update & (PU_DISTANCE))
	{
		p_ptr->update &= ~(PU_DISTANCE);
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(true);
	}

	if (p_ptr->update & (PU_MONSTERS))
	{
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(false);
	}

	if (p_ptr->update & (PU_MON_LITE))
	{
		p_ptr->update &= ~(PU_MON_LITE);
		update_mon_lite();
	}
}


/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff()
{
	/* Redraw stuff */
	if (!p_ptr->redraw) return;


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	/* Hack -- clear the screen */
	if (p_ptr->redraw & (PR_WIPE))
	{
		p_ptr->redraw &= ~(PR_WIPE);
		msg_print(NULL);
		Term_clear();
	}


	if (p_ptr->redraw & (PR_MAP))
	{
		p_ptr->redraw &= ~(PR_MAP);
		prt_map();
	}


	if (p_ptr->redraw & (PR_FRAME))
	{
		p_ptr->redraw &= ~(PR_FRAME);
		prt_frame();
	}
}


/*
 * Handle "p_ptr->window"
 */
void window_stuff()
{
	int j;

	u32b mask = 0L;


	/* Nothing to do */
	if (!p_ptr->window) return;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* Save usable flags */
		if (angband_term[j]) mask |= window_flag[j];
	}

	/* Apply usable flags */
	p_ptr->window &= mask;

	/* Nothing to do */
	if (!p_ptr->window) return;


	/* Display p_ptr->inventory */
	if (p_ptr->window & (PW_INVEN))
	{
		p_ptr->window &= ~(PW_INVEN);
		fix_inven();
	}

	/* Display equipment */
	if (p_ptr->window & (PW_EQUIP))
	{
		p_ptr->window &= ~(PW_EQUIP);
		fix_equip();
	}

	/* Display player */
	if (p_ptr->window & (PW_PLAYER))
	{
		p_ptr->window &= ~(PW_PLAYER);
		fix_player();
	}

	/* Display monster list */
	if (p_ptr->window & (PW_M_LIST))
	{
		p_ptr->window &= ~(PW_M_LIST);
		fix_m_list();
	}

	/* Display overhead view */
	if (p_ptr->window & (PW_MESSAGE))
	{
		p_ptr->window &= ~(PW_MESSAGE);
		fix_message();
	}

	/* Display overhead view */
	if (p_ptr->window & (PW_OVERHEAD))
	{
		p_ptr->window &= ~(PW_OVERHEAD);
		fix_overhead();
	}

	/* Display monster recall */
	if (p_ptr->window & (PW_MONSTER))
	{
		p_ptr->window &= ~(PW_MONSTER);
		fix_monster();
	}

	/* Display object recall */
	if (p_ptr->window & (PW_OBJECT))
	{
		p_ptr->window &= ~(PW_OBJECT);
		fix_object();
	}
}


/*
 * Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window"
 */
void handle_stuff()
{
	/* Update stuff */
	if (p_ptr->update) update_stuff();

	/* Redraw stuff */
	if (p_ptr->redraw) redraw_stuff();

	/* Window stuff */
	if (p_ptr->window) window_stuff();
}


bool monk_heavy_armor()
{
	u16b monk_arm_wgt = 0;

	if (p_ptr->melee_style != SKILL_HAND)
	{
		return false;
	}

	/* Weight the armor */
	monk_arm_wgt += p_ptr->inventory[INVEN_BODY].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_HEAD].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_ARM].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_OUTER].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_HANDS].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_FEET].weight;

	return monk_arm_wgt > (100 + (get_skill(SKILL_HAND) * 4));
}

static int get_artifact_idx(int level)
{
	auto const &a_info = game->edit_data.a_info;

	int count = 0, i;

	while (count < 1000)
	{

		count++;
		i = rand_int(a_info.size());

		auto a_ptr = &a_info[i];
		if (!a_ptr->tval) continue;

		/* It is found/lost */
		if (a_ptr->cur_num) continue;

		/* OoD */
		if (a_ptr->level > level) continue;

		/* Avoid granting SPECIAL_GENE artifacts */
		if (a_ptr->flags & TR_SPECIAL_GENE) continue;

		return i;
	}

	/* No matches found */
	/* Grant a randart */
	return 0;
}

/* Chose a fate */
void gain_fate(byte fate)
{
	auto const &k_info = game->edit_data.k_info;
	auto &alloc = game->alloc;

	int i;
	int level;

	for (i = 0; i < MAX_FATES; i++)
	{
		if (!fates[i].fate)
		{
			fates[i].level = 0;

			cmsg_print(TERM_VIOLET, "More of your prophecy has been unearthed!");
			cmsg_print(TERM_VIOLET, "You should see a soothsayer quickly.");

			if (fate)
				fates[i].fate = fate;
			else
				/* If lucky (current luck > 0) avoid death fate */
				switch (rand_int(p_ptr->luck_cur > 0 ? 17 : 18))
				{
				case 6:
				case 2:
				case 3:
				case 7:
				case 8:
				case 9:
				case 13:
					fates[i].fate = FATE_FIND_O;
					break;
				case 1:
				case 4:
				case 5:
				case 10:
				case 11:
				case 12:
				case 14:
					fates[i].fate = FATE_FIND_R;
					break;
				case 15:
				case 16:
					fates[i].fate = FATE_FIND_A;
					break;
				case 17:
					fates[i].fate = FATE_DIE;
					break;
				case 0:
					{
						/* The deepest the better */
						int chance = dun_level / 4;

						/* No more than 1/2 chances */
						if (chance > 50) chance = 50;

						/* It's HARD to get now */
						if (magik(chance))
						{
							fates[i].fate = FATE_NO_DIE_MORTAL;
						}
						else
						{
							fates[i].fate = FATE_FIND_O;
						}
						break;
					}
				}

			switch (fates[i].fate)
			{
			case FATE_FIND_O:
				{
					while (true)
					{
						obj_theme theme;

						/* No themes */
						theme.treasure = 100;
						theme.combat = 100;
						theme.magic = 100;
						theme.tools = 100;
						init_match_theme(theme);

						/* Apply restriction */
						get_object_hook = kind_is_legal;

						/* Rebuild allocation table */
						get_obj_num_prep();

						fates[i].o_idx = get_obj_num(max_dlv[dungeon_type] + randint(10));

						/* Invalidate the cached allocation table */
						alloc.kind_table_valid = false;

						auto const &k_ptr = k_info.at(fates[i].o_idx);

						if (!(k_ptr->flags & TR_INSTA_ART)
							&& !(k_ptr->flags & TR_NORM_ART))
						{
							break;
						}
					}
					level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
					fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
					fates[i].serious = rand_int(2);
					fates[i].know = false;
					if (wizard) msg_format("New fate : Find object %d on level %d", fates[i].o_idx, fates[i].level);
					break;
				}
			case FATE_FIND_R:
				/* Prepare allocation table */
				get_mon_num_prep();

				fates[i].r_idx = get_mon_num(max_dlv[dungeon_type] + randint(10));
				level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
				fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
				fates[i].serious = rand_int(2);
				fates[i].know = false;
				if (wizard) msg_format("New fate : Meet monster %d on level %d", fates[i].r_idx, fates[i].level);
				break;

			case FATE_FIND_A:
				fates[i].a_idx = get_artifact_idx(max_dlv[dungeon_type] + randint(10));
				level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
				fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
				fates[i].serious = true;
				fates[i].know = false;
				if (wizard) msg_format("New fate : Find artifact %d on level %d", fates[i].a_idx, fates[i].level);
				break;

			case FATE_DIE:
				level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
				fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
				fates[i].serious = true;
				fates[i].know = false;
				if ((wizard) || (p_ptr->precognition)) msg_format("New fate : Death on level %d", fates[i].level);
				break;

			case FATE_NO_DIE_MORTAL:
				fates[i].serious = true;
				p_ptr->no_mortal = true;
				if ((wizard) || (p_ptr->precognition)) msg_format("New fate : Never to die by the hand of a mortal being.");
				break;
			}

			break;
		}
	}
}

std::string fate_desc(int fate)
{
	auto const &a_info = game->edit_data.a_info;

	fmt::MemoryWriter w;

	if (fates[fate].serious)
	{
		w.write("You are fated to ");
	}
	else
	{
		w.write("You may ");
	}

	switch (fates[fate].fate)
	{
	case FATE_FIND_O:
		{
			object_type *o_ptr, forge;
			char o_name[80];

			o_ptr = &forge;
			object_prep(o_ptr, fates[fate].o_idx);
			object_desc_store(o_name, o_ptr, 1, 0);

			w.write("find {} on level {}.", o_name, fates[fate].level);
			break;
		}
	case FATE_FIND_A:
		{
			object_type *q_ptr, forge;
			char o_name[80];
			auto a_ptr = &a_info[fates[fate].a_idx];
			int I_kind;

			/* Failed artefact allocation XXX XXX XXX */
			if (fates[fate].a_idx == 0)
			{
				strcpy(o_name, "something special");
			}

			/* Legal artefacts */
			else
			{
				/* Get local object */
				q_ptr = &forge;

				/* Wipe the object */
				object_wipe(q_ptr);

				/* Acquire the "kind" index */
				I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

				/* Create the artifact */
				object_prep(q_ptr, I_kind);

				/* Save the name */
				q_ptr->name1 = fates[fate].a_idx;

				/* Extract the fields */
				q_ptr->pval = a_ptr->pval;
				q_ptr->ac = a_ptr->ac;
				q_ptr->dd = a_ptr->dd;
				q_ptr->ds = a_ptr->ds;
				q_ptr->to_a = a_ptr->to_a;
				q_ptr->to_h = a_ptr->to_h;
				q_ptr->to_d = a_ptr->to_d;
				q_ptr->weight = a_ptr->weight;

				/* Hack -- acquire "cursed" flag */
				if (a_ptr->flags & TR_CURSED)
				{
					q_ptr->art_flags |= TR_CURSED;
				}

				random_artifact_resistance(q_ptr);

				object_desc_store(o_name, q_ptr, 1, 0);
			}

			w.write("find {} on level {}.", o_name, fates[fate].level);
			break;
		}
	case FATE_FIND_R:
		{
			char m_name[80];
			monster_race_desc(m_name, fates[fate].r_idx, 0);

			w.write("meet {} on level {}.", m_name, fates[fate].level);
			break;
		}
	case FATE_DIE:
		{
			w.write("die on level {}.", fates[fate].level);
			break;
		}
	case FATE_NO_DIE_MORTAL:
		{
			w.write("never to die by the hand of a mortal being.");
			break;
		}
	}

	return w.str();
}

std::string dump_fates()
{
	bool pending = false;

	fmt::MemoryWriter w;

	for (int i = 0; i < MAX_FATES; i++)
	{
		if ((fates[i].fate) && (fates[i].know))
		{
			w.write("{}\n", fate_desc(i));
		}

		// Pending gets set if there's at least one fate we don't know
		pending |= ((fates[i].fate) && !(fates[i].know));
	}

	if (pending)
	{
		w.write("You do not know all of your fate.\n");
	}

	return w.str();
}

/*
 * Return a luck number between a certain range
 */
int luck(int min, int max)
{
	int luck = p_ptr->luck_cur;
	int range = max - min;

	if (luck < -30) luck = -30;
	if (luck > 30) luck = 30;
	luck += 30;

	luck *= range;
	luck /= 60;

	return (luck + min);
}

bool race_flags_p(player_race_flag_set const &flags_mask)
{
	return bool((rp_ptr->flags | rmp_ptr->flags | cp_ptr->flags | spp_ptr->flags) & flags_mask);
}
