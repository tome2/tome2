/* File: misc.c */

/* Purpose: misc code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Converts stat num into a six-char (right justified) string
 */
void cnv_stat(int val, char *out_val)
{
	if (!linear_stats)
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

	/* Return new value */
	return (value);
}



/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(cptr info, int row, int col)
{
	/* Dump 13 spaces to clear */
	c_put_str(TERM_WHITE, "             ", row, col);

	/* Dump the info itself */
	c_put_str(TERM_L_BLUE, info, row, col);
}

/*
 * Prints players max/cur piety
 */
static void prt_piety(void)
{
	char tmp[32];

	/* Do not show piety unless it matters */
	if (!p_ptr->pgod) return;

	c_put_str(TERM_L_WHITE, "Pt ", ROW_PIETY, COL_PIETY);

	sprintf(tmp, "%9ld", p_ptr->grace);

	c_put_str((p_ptr->praying) ? TERM_L_BLUE : TERM_GREEN, tmp, ROW_PIETY,
		COL_PIETY + 3);
}


/*
 * Prints the player's current sanity.
 */
static void prt_sane(void)
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
	else if (perc > (10 * hitpoint_warn))
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
static void prt_title(void)
{
	cptr p = "";

	/* Mimic shape */
	if (p_ptr->mimic_form)
	{
		call_lua("get_mimic_info", "(d,s)", "s", p_ptr->mimic_form, "show_name", &p);
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
		p = cp_ptr->titles[(p_ptr->lev - 1) / 5] + c_text;

	}

	prt_field(p, ROW_TITLE, COL_TITLE);
}


/*
 * Prints level
 */
static void prt_level(void)
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
static void prt_exp(void)
{
	char out_val[32];

	if (!exp_need)
	{
		(void)sprintf(out_val, "%8ld", (long)p_ptr->exp);
	}
	else
	{
		if ((p_ptr->lev >= PY_MAX_LEVEL) || (p_ptr->lev >= max_plev))
		{
			(void)sprintf(out_val, "********");
		}
		else
		{
			(void)sprintf(out_val, "%8ld", (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L) - p_ptr->exp);
		}
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
static void prt_gold(void)
{
	char tmp[32];

	put_str("AU ", ROW_GOLD, COL_GOLD);
	sprintf(tmp, "%9ld", (long)p_ptr->au);
	c_put_str(TERM_L_GREEN, tmp, ROW_GOLD, COL_GOLD + 3);
}



/*
 * Prints current AC
 */
static void prt_ac(void)
{
	char tmp[32];

	put_str("Cur AC ", ROW_AC, COL_AC);
	sprintf(tmp, "%5d", p_ptr->dis_ac + p_ptr->dis_to_a);
	c_put_str(TERM_L_GREEN, tmp, ROW_AC, COL_AC + 7);
}


/*
 * Prints Cur/Max hit points
 */
static void prt_hp(void)
{
	char tmp[32];

	byte color;

	if (player_char_health) lite_spot(p_ptr->py, p_ptr->px);

	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		c_put_str(TERM_L_DARK, "DP ", ROW_HP, COL_HP);

		sprintf(tmp, "%4d/%4d", p_ptr->chp, p_ptr->mhp);

		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_BLUE;
		}
		else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10)
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
		else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10)
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
static void prt_mh(void)
{
	char tmp[32];

	byte color;

	object_type *o_ptr;
	monster_race *r_ptr;

	/* Get the carried monster */
	o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (!o_ptr->pval2)
	{
		put_str("             ", ROW_MH, COL_MH);
		return;
	}

	r_ptr = &r_info[o_ptr->pval];

	put_str("MH ", ROW_MH, COL_MH);

	sprintf(tmp, "%4d/%4d", o_ptr->pval2, (int)o_ptr->pval3);
	if (o_ptr->pval2 >= o_ptr->pval3)
	{
		color = TERM_L_GREEN;
	}
	else if (o_ptr->pval2 > (o_ptr->pval3 * hitpoint_warn) / 10)
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
static void prt_sp(void)
{
	char tmp[32];
	byte color;


	c_put_str(TERM_L_GREEN, "SP ", ROW_SP, COL_SP);

	sprintf(tmp, "%4d/%4d", p_ptr->csp, p_ptr->msp);
	if (p_ptr->csp >= p_ptr->msp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csp > (p_ptr->msp * hitpoint_warn) / 10)
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
static void prt_depth(void)
{
	char depths[32];
	dungeon_info_type *d_ptr = &d_info[dungeon_type];

	if (p_ptr->wild_mode)
	{
		strcpy(depths, "             ");
	}
	else if (p_ptr->inside_arena)
	{
		strcpy(depths, "Arena");
	}
	else if (get_dungeon_name(depths))
	{
		/* Empty */
	}
	else if (dungeon_flags2 & DF2_SPECIAL)
	{
		strcpy(depths, "Special");
	}
	else if (p_ptr->inside_quest)
	{
		strcpy(depths, "Quest");
	}
	else if (!dun_level)
	{
		if (wf_info[wild_map[p_ptr->wilderness_y][p_ptr->wilderness_x].feat].name + wf_name)
			strcpy(depths, wf_info[wild_map[p_ptr->wilderness_y][p_ptr->wilderness_x].feat].name + wf_name);
		else
			strcpy(depths, "Town/Wild");
	}
	else if (depth_in_feet)
	{
		if (dungeon_flags1 & DF1_TOWER)
		{
			(void)strnfmt(depths, 32, "%c%c%c -%d ft",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level * 50);
		}
		else
		{
			(void)strnfmt(depths, 32, "%c%c%c %d ft",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level * 50);
		}
	}
	else
	{
		if (dungeon_flags1 & DF1_TOWER)
		{
			(void)strnfmt(depths, 32, "%c%c%c -%d",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level);
		}
		else
		{
			(void)strnfmt(depths, 32, "%c%c%c %d",
			              d_ptr->short_name[0],
			              d_ptr->short_name[1],
			              d_ptr->short_name[2],
			              dun_level);
		}
	}

	/* Right-Adjust the "depth", and clear old values */
	if (p_ptr->word_recall)
		c_prt(TERM_ORANGE, format("%13s", depths), ROW_DEPTH, COL_DEPTH);
	else
		prt(format("%13s", depths), ROW_DEPTH, COL_DEPTH);
}


/*
 * Prints status of hunger
 */
static void prt_hunger(void)
{
	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		c_put_str(TERM_RED, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		c_put_str(TERM_ORANGE, "Weak  ", ROW_HUNGRY, COL_HUNGRY);
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		c_put_str(TERM_YELLOW, "Hungry", ROW_HUNGRY, COL_HUNGRY);
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		c_put_str(TERM_L_GREEN, "      ", ROW_HUNGRY, COL_HUNGRY);
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		c_put_str(TERM_L_GREEN, "Full  ", ROW_HUNGRY, COL_HUNGRY);
	}

	/* Gorged */
	else
	{
		c_put_str(TERM_GREEN, "Gorged", ROW_HUNGRY, COL_HUNGRY);
	}
}


/*
 * Prints Blind status
 */
static void prt_blind(void)
{
	if (p_ptr->blind)
	{
		c_put_str(TERM_ORANGE, "Blind", ROW_BLIND, COL_BLIND);
	}
	else
	{
		put_str("     ", ROW_BLIND, COL_BLIND);
	}
}


/*
 * Prints Confusion status
 */
static void prt_confused(void)
{
	if (p_ptr->confused)
	{
		c_put_str(TERM_ORANGE, "Conf", ROW_CONFUSED, COL_CONFUSED);
	}
	else
	{
		put_str("    ", ROW_CONFUSED, COL_CONFUSED);
	}
}


/*
 * Prints Fear status
 */
static void prt_afraid(void)
{
	if (p_ptr->afraid)
	{
		c_put_str(TERM_ORANGE, "Afraid", ROW_AFRAID, COL_AFRAID);
	}
	else
	{
		put_str("      ", ROW_AFRAID, COL_AFRAID);
	}
}


/*
 * Prints Poisoned status
 */
static void prt_poisoned(void)
{
	if (p_ptr->poisoned)
	{
		c_put_str(TERM_ORANGE, "Poison", ROW_POISONED, COL_POISONED);
	}
	else
	{
		put_str("      ", ROW_POISONED, COL_POISONED);
	}
}


/*
 * Prints trap detection status
 */
static void prt_dtrap(void)
{
	if (cave[p_ptr->py][p_ptr->px].info & CAVE_DETECT)
	{
		c_put_str(TERM_L_GREEN, "DTrap", ROW_DTRAP, COL_DTRAP);
	}
	else
	{
		put_str("     ", ROW_DTRAP, COL_DTRAP);
	}
}


/*
 * Prints Searching, Resting, Paralysis, or 'count' status
 * Display is always exactly 10 characters wide (see below)
 *
 * This function was a major bottleneck when resting, so a lot of
 * the text formatting code was optimized in place below.
 */
static void prt_state(void)
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
			(void)sprintf(text, "Rep. %3d00", command_rep / 100);
		}
		else
		{
			(void)sprintf(text, "Repeat %3d", command_rep);
		}
	}

	/* Searching */
	else if (p_ptr->searching)
	{
		strcpy(text, "Searching ");
	}

	/* Nothing interesting */
	else
	{
		strcpy(text, "          ");
	}

	/* Display the info (or blanks) */
	c_put_str(attr, text, ROW_STATE, COL_STATE);
}


/*
 * Prints the speed of a character.			-CJS-
 */
static void prt_speed(void)
{
	int i = p_ptr->pspeed;

	byte attr = TERM_WHITE;
	char buf[32] = "";

	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->searching) i += 10;

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
	c_put_str(attr, format("%-10s", buf), ROW_SPEED, COL_SPEED);
}


static void prt_study(void)
{
	if (p_ptr->skill_points)
	{
		put_str("Skill", ROW_STUDY, COL_STUDY);
	}
	else
	{
		put_str("     ", ROW_STUDY, COL_STUDY);
	}
}


static void prt_cut(void)
{
	int c = p_ptr->cut;

	if (c > 1000)
	{
		c_put_str(TERM_L_RED, "Mortal wound", ROW_CUT, COL_CUT);
	}
	else if (c > 200)
	{
		c_put_str(TERM_RED, "Deep gash   ", ROW_CUT, COL_CUT);
	}
	else if (c > 100)
	{
		c_put_str(TERM_RED, "Severe cut  ", ROW_CUT, COL_CUT);
	}
	else if (c > 50)
	{
		c_put_str(TERM_ORANGE, "Nasty cut   ", ROW_CUT, COL_CUT);
	}
	else if (c > 25)
	{
		c_put_str(TERM_ORANGE, "Bad cut     ", ROW_CUT, COL_CUT);
	}
	else if (c > 10)
	{
		c_put_str(TERM_YELLOW, "Light cut   ", ROW_CUT, COL_CUT);
	}
	else if (c)
	{
		c_put_str(TERM_YELLOW, "Graze       ", ROW_CUT, COL_CUT);
	}
	else
	{
		put_str("            ", ROW_CUT, COL_CUT);
	}
}



static void prt_stun(void)
{
	int s = p_ptr->stun;

	if (s > 100)
	{
		c_put_str(TERM_RED, "Knocked out ", ROW_STUN, COL_STUN);
	}
	else if (s > 50)
	{
		c_put_str(TERM_ORANGE, "Heavy stun  ", ROW_STUN, COL_STUN);
	}
	else if (s)
	{
		c_put_str(TERM_ORANGE, "Stun        ", ROW_STUN, COL_STUN);
	}
	else
	{
		put_str("            ", ROW_STUN, COL_STUN);
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
static void health_redraw(void)
{

#ifdef DRS_SHOW_HEALTH_BAR

	/* Not tracking */
	if (!health_who)
	{
		/* Erase the health bar */
		Term_erase(COL_INFO, ROW_INFO, 12);
	}

	/* Tracking an unseen monster */
	else if (!m_list[health_who].ml)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a hallucinatory monster */
	else if (p_ptr->image)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
	}

	/* Tracking a dead monster (???) */
	else if (!m_list[health_who].hp < 0)
	{
		/* Indicate that the monster health is "unknown" */
		Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");
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
		Term_putstr(COL_INFO, ROW_INFO, 12, TERM_WHITE, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		Term_putstr(COL_INFO + 1, ROW_INFO, len, attr, "**********");
	}

#endif

}



/*
 * Display basic info (mostly left of map)
 */
static void prt_frame_basic(void)
{
	int i;

	/* Race and Class */
	prt_field(rp_ptr->title + rp_name, ROW_RACE, COL_RACE);
	prt_field(spp_ptr->title + c_name, ROW_CLASS, COL_CLASS);

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

	/* Current depth */
	prt_depth();

	/* Special */
	health_redraw();
}


/*
 * Display extra info (mostly below map)
 */
static void prt_frame_extra(void)
{
	/* Cut/Stun */
	prt_cut();
	prt_stun();

	/* Food */
	prt_hunger();

	/* Various */
	prt_blind();
	prt_confused();
	prt_afraid();
	prt_poisoned();
	prt_dtrap();

	/* State */
	prt_state();

	/* Speed */
	prt_speed();

	/* Study spells */
	prt_study();
}


/*
 * Hack -- display inventory in sub-windows
 */
static void fix_inven(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_INVEN))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display inventory */
		display_inven();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display equipment in sub-windows
 */
static void fix_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_EQUIP))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display equipment */
		display_equip();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}

/*
 * Hack -- display character in sub-windows
 */
static void fix_player(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display player */
		display_player(0);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */
void fix_message(void)
{
	int j, i;
	int w, h;
	int x, y;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MESSAGE))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Get size */
		Term_get_size(&w, &h);

		/* Dump messages */
		for (i = 0; i < h; i++)
		{
			/* Dump the message on the appropriate line */
			display_message(0, (h - 1) - i, strlen(message_str((s16b)i)), message_color((s16b)i), message_str((s16b)i));

			/* Cursor */
			Term_locate(&x, &y);

			/* Clear to end of line */
			Term_erase(x, y, 255);
		}

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display recent IRC messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */
void fix_irc_message(void)
{
	int j, i, k;
	int w, h;
	int x, y;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_IRC))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Get size */
		Term_get_size(&w, &h);

		Term_clear();

		/* Dump messages */
		k = 0;
		for (i = 0; ; i++)
		{
			byte type = message_type((s16b)i);

			if (k >= h) break;
			if (MESSAGE_NONE == type) break;
			if (MESSAGE_IRC != type) continue;

			/* Dump the message on the appropriate line */
			display_message(0, (h - 1) - k, strlen(message_str((s16b)i)), message_color((s16b)i), message_str((s16b)i));

			/* Cursor */
			Term_locate(&x, &y);

			/* Clear to end of line */
			Term_erase(x, y, 255);

			k++;
		}

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display overhead view in sub-windows
 *
 * Note that the "player" symbol does NOT appear on the map.
 */
static void fix_overhead(void)
{
	int j;

	int cy, cx;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_OVERHEAD))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Redraw map */
		display_map(&cy, &cx);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display monster recall in sub-windows
 */
static void fix_monster(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MONSTER))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display monster race info */
		if (monster_race_idx) display_roff(monster_race_idx, monster_ego_idx);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display object recall in sub-windows
 */
static void fix_object(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_OBJECT))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Clear */
		Term_clear();

		/* Display object info */
		if (tracked_object)
			if (!object_out_desc(tracked_object, NULL, FALSE, FALSE)) text_out("You see nothing special.");

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}

/* Show the monster list in a window */

static void fix_m_list(void)
{
	int i, j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		int c = 0;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_M_LIST))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Clear */
		Term_clear();

		/* Hallucination */
		if (p_ptr->image)
		{
			c_prt(TERM_WHITE, "You can not see clearly", 0, 0);

			/* Fresh */
			Term_fresh();

			/* Restore */
			Term_activate(old);

			return;
		}

		/* reset visible count */
		for (i = 1; i < max_r_idx; i++)
		{
			monster_race *r_ptr = &r_info[i];

			r_ptr->total_visible = 0;
		}

		/* Count up the number visible in each race */
		for (i = 1; i < m_max; i++)
		{
			monster_type *m_ptr = &m_list[i];
			monster_race *r_ptr = &r_info[m_ptr->r_idx];

			/* Skip dead monsters */
			if (m_ptr->hp < 0) continue;

			/* Skip unseen monsters */
			if (r_ptr->flags9 & RF9_MIMIC)
			{
				object_type *o_ptr;

				/* Acquire object */
				o_ptr = &o_list[m_ptr->hold_o_idx];

				/* Memorized objects */
				if (!o_ptr->marked) continue;
			}
			else
			{
				if (!m_ptr->ml) continue;
			}

			/* Increase for this race */
			r_ptr->total_visible++;

			/* Increase total Count */
			c++;
		}

		/* Are monsters visible? */
		if (c)
		{
			int w, h, num = 0;

			(void)Term_get_size(&w, &h);

			c_prt(TERM_WHITE, format("You can see %d monster%s", c, (c > 1 ? "s:" : ":")), 0, 0);

			for (i = 1; i < max_r_idx; i++)
			{
				monster_race *r_ptr = &r_info[i];

				/* Default Colour */
				byte attr = TERM_SLATE;

				/* Only visible monsters */
				if (!r_ptr->total_visible) continue;

				/* Uniques */
				if (r_ptr->flags1 & RF1_UNIQUE)
				{
					attr = TERM_L_BLUE;
				}

				/* Have we ever killed one? */
				if (r_ptr->r_tkills)
				{
					if (r_ptr->level > dun_level)
					{
						attr = TERM_VIOLET;

						if (r_ptr->flags1 & RF1_UNIQUE)
						{
							attr = TERM_RED;
						}
					}
				}
				else
				{
					if (!(r_ptr->flags1 & RF1_UNIQUE)) attr = TERM_GREEN;
				}


				/* Dump the monster name */
				if (r_ptr->total_visible == 1)
				{
					c_prt(attr, (r_name + r_ptr->name), (num % (h - 1)) + 1, (num / (h - 1) * 26));
				}
				else
				{
					c_prt(attr, format("%s (x%d)", r_name + r_ptr->name, r_ptr->total_visible), (num % (h - 1)) + 1, (num / (h - 1)) * 26);
				}

				num++;

			}

		}
		else
		{
			c_prt(TERM_WHITE, "You see no monsters.", 0, 0);
		}

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
	p_ptr->new_spells = 0;
}

/* Ugly hack */
bool calc_powers_silent = FALSE;

/* Calc the player powers */
static void calc_powers(void)
{
	int i, p = 0;
	bool *old_powers;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	C_MAKE(old_powers, power_max, bool);

	/* Save old powers */
	for (i = 0; i < power_max; i++) old_powers[i] = p_ptr->powers[i];

	/* Get intrinsincs */
	for (i = 0; i < POWER_MAX_INIT; i++) p_ptr->powers[i] = p_ptr->powers_mod[i];
	for (; i < power_max; i++) p_ptr->powers[i] = 0;

	/* Hooked powers */
	process_hooks(HOOK_CALC_POWERS, "()");

	/* Add objects powers */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_idx) continue;

		p = object_power(o_ptr);
		if (p != -1) p_ptr->powers[p] = TRUE;
	}

	if ((!p_ptr->tim_mimic) && (!p_ptr->body_monster))
	{
		/* Add in racial and subracial powers */
		for (i = 0; i < 4; i++)
		{
			p = rp_ptr->powers[i];
			if (p != -1) p_ptr->powers[p] = TRUE;

			p = rmp_ptr->powers[i];
			if (p != -1) p_ptr->powers[p] = TRUE;
		}
	}
	else if (p_ptr->mimic_form)
		call_lua("calc_mimic_power", "(d)", "", p_ptr->mimic_form);

	/* Add in class powers */
	for (i = 0; i < 4; i++)
	{
		p = cp_ptr->powers[i];
		if (p != -1) p_ptr->powers[p] = TRUE;
	}

	if (p_ptr->disembodied)
	{
		p = PWR_INCARNATE;
		p_ptr->powers[p] = TRUE;
	}

	/* Now lets warn the player */
	for (i = 0; i < power_max; i++)
	{
		s32b old = old_powers[i];
		s32b new = p_ptr->powers[i];

		if (new > old)
		{
			if (!calc_powers_silent) cmsg_print(TERM_GREEN, powers_type[i].gain_text);
		}
		else if (new < old)
		{
			if (!calc_powers_silent) cmsg_print(TERM_RED, powers_type[i].lose_text);
		}
	}

	calc_powers_silent = FALSE;
	C_FREE(old_powers, power_max, bool);
}


/*
 * Calculate the player's sanity
 */

void calc_sanity(void)
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

		p_ptr->redraw |= (PR_SANITY);
		p_ptr->window |= (PW_PLAYER);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
	int msp, levels, cur_wgt, max_wgt;
	u32b f1, f2, f3, f4, f5, esp;

	object_type *o_ptr;

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
		monster_race *r_ptr = &r_info[p_ptr->body_monster];
		int f = 100 / (r_ptr->freq_spell ? r_ptr->freq_spell : 1);

		msp = 21 - f;

		if (msp < 1) msp = 1;
	}

	/* Apply race mod mana */
	msp = msp * rmp_ptr->mana / 100;

	/* Apply class mana */
	msp += msp * cp_ptr->mana / 100;

	/* Apply Eru mana */
	GOD(GOD_ERU)
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
		p_ptr->cumber_glove = FALSE;

		/* Get the gloves */
		o_ptr = &p_ptr->inventory[INVEN_HANDS];

		/* Examine the gloves */
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->k_idx &&
		                !(f2 & (TR2_FREE_ACT)) &&
		                !((f1 & (TR1_DEX)) && (o_ptr->pval > 0)) &&
		                !(f5 & TR5_SPELL_CONTAIN))
		{
			/* Encumbered */
			p_ptr->cumber_glove = TRUE;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}
	}

	/* Augment mana */
	if (munchkin_multipliers)
	{
		if (p_ptr->to_m) msp += msp * p_ptr->to_m / 5;
	}
	else
	{
		if (p_ptr->to_m) msp += msp * p_ptr->to_m / 10;
	}

	/* Assume player not encumbered by armor */
	p_ptr->cumber_armor = FALSE;

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
		p_ptr->cumber_armor = TRUE;

		/* Reduce mana */
		msp -= ((cur_wgt - max_wgt) / 10);
	}

	/* When meditating your mana is increased ! */
	if (p_ptr->meditation)
	{
		msp += 50;
		p_ptr->csp += 50;
	}

	/* Sp mods? */
	if (process_hooks_ret(HOOK_CALC_MANA, "d", "(d)", msp))
	{
		msp = process_hooks_return[0].num;
	}

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
		p_ptr->redraw |= (PR_MANA);

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
void calc_hitpoints(void)
{
	int bonus, mhp;

	/* Un-inflate "half-hitpoint bonus per level" value */
	bonus = ((int)(adj_con_mhp[p_ptr->stat_ind[A_CON]]) - 128);

	/* Calculate hitpoints */
	mhp = player_hp[p_ptr->lev - 1] + (bonus * p_ptr->lev / 2);

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
	GOD(GOD_MELKOR)
	{
		mhp -= (p_ptr->melkor_sacrifice * 10);
		if (mhp < 1) mhp = 1;
	}

	/* Factor in the hero / superhero settings */
	if (p_ptr->hero) mhp += 10;
	if (p_ptr->shero) mhp += 30;

	/* Augment Hitpoint */
	if (munchkin_multipliers)
	{
		mhp += mhp * p_ptr->to_l / 5;
	}
	else
	{
		mhp += mhp * p_ptr->to_l / 10;
	}
	if (mhp < 1) mhp = 1;

	if (p_ptr->body_monster)
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];
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
	if (process_hooks_ret(HOOK_CALC_HP, "d", "(d)", mhp))
	{
		mhp = process_hooks_return[0].num;
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
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}



/*
 * Extract and set the current "lite radius"
 *
 * SWD: Experimental modification: multiple light sources have additive effect.
 *
 */
static void calc_torch(void)
{
	int i;
	object_type *o_ptr;
	u32b f1, f2, f3, f4, f5, esp;

	/* Assume no light */
	p_ptr->cur_lite = 0;

	/* Loop through all wielded items */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* does this item glow? */
		if (((f4 & TR4_FUEL_LITE) && (o_ptr->timeout > 0)) || (!(f4 & TR4_FUEL_LITE)))
		{
			if (f3 & TR3_LITE1) p_ptr->cur_lite++;
			if (f4 & TR4_LITE2) p_ptr->cur_lite += 2;
			if (f4 & TR4_LITE3) p_ptr->cur_lite += 3;
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

	/* Hooked powers */
	process_hooks(HOOK_CALC_LITE, "()");

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
	if (running && view_reduce_lite)
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
int weight_limit(void)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[p_ptr->stat_ind[A_STR]] * 100;

	if (process_hooks_ret(HOOK_CALC_WEIGHT, "d", "(d)", i))
		i = process_hooks_return[0].num;

	/* Return the result */
	return (i);
}

void calc_wield_monster()
{
	object_type *o_ptr;
	monster_race *r_ptr;

	/* Get the carried monster */
	o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (o_ptr->k_idx)
	{
		r_ptr = &r_info[o_ptr->pval];

		if (r_ptr->flags2 & RF2_INVISIBLE)
			p_ptr->invis += 20;
		if (r_ptr->flags2 & RF2_REFLECTING)
			p_ptr->reflect = TRUE;
		if (r_ptr->flags7 & RF7_CAN_FLY)
			p_ptr->ffall = TRUE;
		if (r_ptr->flags7 & RF7_AQUATIC)
			p_ptr->water_breath = TRUE;
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
	monster_race *r_ptr = &r_info[p_ptr->body_monster];
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

	/* Do we need more parts ? ;) */
	for (i = 0; i < BODY_MAX; i++)
		p_ptr->extra_body_parts[i] = 0;
	process_hooks(HOOK_BODY_PARTS, "()");

	for (i = 0; i < BODY_MAX; i++)
	{
		int b;

		b = bp[i] + cp_ptr->body_parts[i] + p_ptr->extra_body_parts[i];
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
		if ((!p_ptr->body_parts[i]) && (p_ptr->inventory[i + INVEN_WIELD].k_idx))
		{
			/* Drop it NOW ! */
			inven_takeoff(i + INVEN_WIELD, 255, TRUE);
		}
	}
}

/* Should be called by every calc_bonus call */
void calc_body_bonus()
{
	monster_race *r_ptr = &r_info[p_ptr->body_monster];

	/* If in the player body nothing have to be done */
	if (!p_ptr->body_monster) return;

	if (p_ptr->disembodied)
	{
		p_ptr->wraith_form = TRUE;
		return;
	}

	p_ptr->ac += r_ptr->ac;
	p_ptr->pspeed = r_ptr->speed;

	if (r_ptr->flags1 & RF1_NEVER_MOVE) p_ptr->immovable = TRUE;
	if (r_ptr->flags2 & RF2_STUPID) p_ptr->stat_add[A_INT] -= 1;
	if (r_ptr->flags2 & RF2_SMART) p_ptr->stat_add[A_INT] += 1;
	if (r_ptr->flags2 & RF2_REFLECTING) p_ptr->reflect = TRUE;
	if (r_ptr->flags2 & RF2_INVISIBLE) p_ptr->invis += 20;
	if (r_ptr->flags2 & RF2_REGENERATE) p_ptr->regenerate = TRUE;
	if (r_ptr->flags2 & RF2_AURA_FIRE) p_ptr->sh_fire = TRUE;
	if (r_ptr->flags2 & RF2_AURA_ELEC) p_ptr->sh_elec = TRUE;
	if (r_ptr->flags2 & RF2_PASS_WALL) p_ptr->wraith_form = TRUE;
	if (r_ptr->flags3 & RF3_SUSCEP_FIRE) p_ptr->sensible_fire = TRUE;
	if (r_ptr->flags3 & RF3_IM_ACID) p_ptr->resist_acid = TRUE;
	if (r_ptr->flags3 & RF3_IM_ELEC) p_ptr->resist_elec = TRUE;
	if (r_ptr->flags3 & RF3_IM_FIRE) p_ptr->resist_fire = TRUE;
	if (r_ptr->flags3 & RF3_IM_POIS) p_ptr->resist_pois = TRUE;
	if (r_ptr->flags3 & RF3_IM_COLD) p_ptr->resist_cold = TRUE;
	if (r_ptr->flags3 & RF3_RES_NETH) p_ptr->resist_neth = TRUE;
	if (r_ptr->flags3 & RF3_RES_NEXU) p_ptr->resist_nexus = TRUE;
	if (r_ptr->flags3 & RF3_RES_DISE) p_ptr->resist_disen = TRUE;
	if (r_ptr->flags3 & RF3_NO_FEAR) p_ptr->resist_fear = TRUE;
	if (r_ptr->flags3 & RF3_NO_SLEEP) p_ptr->free_act = TRUE;
	if (r_ptr->flags3 & RF3_NO_CONF) p_ptr->resist_conf = TRUE;
	if (r_ptr->flags7 & RF7_CAN_FLY) p_ptr->ffall = TRUE;
	if (r_ptr->flags7 & RF7_AQUATIC) p_ptr->water_breath = TRUE;
}


byte calc_mimic()
{
	s32b blow = 0;

	call_lua("calc_mimic", "(d)", "d", p_ptr->mimic_form, &blow);
	return blow;
}

/* Returns the blow information based on class */
void analyze_blow(int *num, int *wgt, int *mul)
{
	*num = cp_ptr->blow_num;
	*wgt = cp_ptr->blow_wgt;
	*mul = cp_ptr->blow_mul;

	/* Count bonus abilities */
	if (has_ability(AB_MAX_BLOW1)) (*num)++;
	if (has_ability(AB_MAX_BLOW2)) (*num)++;
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

		if (!o_ptr->k_idx)
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
	object_type *o_ptr;

	i = INVEN_BOW - INVEN_WIELD;
	/* All weapons must be of the same type */
	while (p_ptr->body_parts[i] == INVEN_BOW)
	{
		o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

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
void calc_gods()
{
	/* Boost WIS if the player follows Eru */
	GOD(GOD_ERU)
	{
		if (p_ptr->grace > 10000) p_ptr->stat_add[A_WIS] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_WIS] += 1;
		if (p_ptr->grace > 30000) p_ptr->stat_add[A_WIS] += 1;
	}

	/* Boost str, con, chr and reduce int, wis if the player follows Melkor */
	GOD(GOD_MELKOR)
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

		PRAY_GOD(GOD_MELKOR)
		{
			if (p_ptr->grace > 5000) p_ptr->invis += 30;
			if (p_ptr->grace > 15000) p_ptr->immune_fire = TRUE;
		}
		p_ptr->resist_fire = TRUE;
	}

	/* Gifts of Manwe if the player is praying to Manwe */
	PRAY_GOD(GOD_MANWE)
	{
		s32b add = p_ptr->grace;

		/* provides speed every 5000 grace */
		if (add > 35000) add = 35000;
		add /= 5000;
		p_ptr->pspeed += add;

		/* Provides fly & FA */
		if (p_ptr->grace >= 7000) p_ptr->free_act = TRUE;
		if (p_ptr->grace >= 15000) p_ptr->fly = TRUE;
	}

	/* Manwe bonus not requiring the praying status */
	GOD(GOD_MANWE)
	{
		if (p_ptr->grace >= 2000) p_ptr->ffall = TRUE;
	}

	/* Boost Str and Con if the player is following Tulkas */
	GOD(GOD_TULKAS)
	{
		if (p_ptr->grace > 5000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 10000) p_ptr->stat_add[A_CON] += 1;
		if (p_ptr->grace > 15000) p_ptr->stat_add[A_CON] += 1;

		if (p_ptr->grace > 10000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 15000) p_ptr->stat_add[A_STR] += 1;
		if (p_ptr->grace > 20000) p_ptr->stat_add[A_STR] += 1;
	}
}

/* Apply flags */
static int extra_blows;
static int extra_shots;
void apply_flags(u32b f1, u32b f2, u32b f3, u32b f4, u32b f5, u32b esp, s16b pval, s16b tval, s16b to_h, s16b to_d, s16b to_a)
{
	/* Affect stats */
	if (f1 & (TR1_STR)) p_ptr->stat_add[A_STR] += pval;
	if (f1 & (TR1_INT)) p_ptr->stat_add[A_INT] += pval;
	if (f1 & (TR1_WIS)) p_ptr->stat_add[A_WIS] += pval;
	if (f1 & (TR1_DEX)) p_ptr->stat_add[A_DEX] += pval;
	if (f1 & (TR1_CON)) p_ptr->stat_add[A_CON] += pval;
	if (f1 & (TR1_CHR)) p_ptr->stat_add[A_CHR] += pval;
	if (f5 & (TR5_LUCK)) p_ptr->luck_cur += pval;

	/* Affect spell power */
	if (f1 & (TR1_SPELL)) p_ptr->to_s += pval;

	/* Affect mana capacity */
	if (f1 & (TR1_MANA)) p_ptr->to_m += pval;

	/* Affect life capacity */
	if (f2 & (TR2_LIFE)) p_ptr->to_l += pval;

	/* Affect stealth */
	if (f1 & (TR1_STEALTH)) p_ptr->skill_stl += pval;

	/* Affect searching ability (factor of five) */
	if (f1 & (TR1_SEARCH)) p_ptr->skill_srh += (pval * 5);

	/* Affect searching frequency (factor of five) */
	if (f1 & (TR1_SEARCH)) p_ptr->skill_fos += (pval * 5);

	/* Affect infravision */
	if (f1 & (TR1_INFRA)) p_ptr->see_infra += pval;

	/* Affect digging (factor of 20) */
	if (f1 & (TR1_TUNNEL)) p_ptr->skill_dig += (pval * 20);

	/* Affect speed */
	if (f1 & (TR1_SPEED)) p_ptr->pspeed += pval;

	/* Affect blows */
	if (f1 & (TR1_BLOWS)) extra_blows += pval;
	if (f5 & (TR5_CRIT)) p_ptr->xtra_crit += pval;

	/* Hack -- Sensible fire */
	if (f2 & (TR2_SENS_FIRE)) p_ptr->sensible_fire = TRUE;

	/* Hack -- cause earthquakes */
	if (f1 & (TR1_IMPACT)) p_ptr->impact = TRUE;

	/* Affect invisibility */
	if (f2 & (TR2_INVIS)) p_ptr->invis += (pval * 10);

	/* Boost shots */
	if (f3 & (TR3_XTRA_SHOTS)) extra_shots++;

	/* Various flags */
	if (f3 & (TR3_AGGRAVATE)) p_ptr->aggravate = TRUE;
	if (f3 & (TR3_TELEPORT)) p_ptr->teleport = TRUE;
	if (f5 & (TR5_DRAIN_MANA)) p_ptr->drain_mana++;
	if (f5 & (TR5_DRAIN_HP)) p_ptr->drain_life++;
	if (f3 & (TR3_DRAIN_EXP)) p_ptr->exp_drain = TRUE;
	if (f3 & (TR3_BLESSED)) p_ptr->bless_blade = TRUE;
	if (f3 & (TR3_XTRA_MIGHT)) p_ptr->xtra_might += pval;
	if (f3 & (TR3_SLOW_DIGEST)) p_ptr->slow_digest = TRUE;
	if (f3 & (TR3_REGEN)) p_ptr->regenerate = TRUE;
	if (esp) p_ptr->telepathy |= esp;
	if ((tval != TV_LITE) && (f3 & (TR3_LITE1))) p_ptr->lite = TRUE;
	if ((tval != TV_LITE) && (f4 & (TR4_LITE2))) p_ptr->lite = TRUE;
	if ((tval != TV_LITE) && (f4 & (TR4_LITE3))) p_ptr->lite = TRUE;
	if (f3 & (TR3_SEE_INVIS)) p_ptr->see_inv = TRUE;
	if (f2 & (TR2_FREE_ACT)) p_ptr->free_act = TRUE;
	if (f2 & (TR2_HOLD_LIFE)) p_ptr->hold_life = TRUE;
	if (f3 & (TR3_WRAITH)) p_ptr->wraith_form = TRUE;
	if (f3 & (TR3_FEATHER)) p_ptr->ffall = TRUE;
	if (f4 & (TR4_FLY)) p_ptr->fly = TRUE;
	if (f4 & (TR4_CLIMB)) p_ptr->climb = TRUE;

	/* Immunity flags */
	if (f2 & (TR2_IM_FIRE)) p_ptr->immune_fire = TRUE;
	if (f2 & (TR2_IM_ACID)) p_ptr->immune_acid = TRUE;
	if (f2 & (TR2_IM_COLD)) p_ptr->immune_cold = TRUE;
	if (f2 & (TR2_IM_ELEC)) p_ptr->immune_elec = TRUE;

	/* Resistance flags */
	if (f2 & (TR2_RES_ACID)) p_ptr->resist_acid = TRUE;
	if (f2 & (TR2_RES_ELEC)) p_ptr->resist_elec = TRUE;
	if (f2 & (TR2_RES_FIRE)) p_ptr->resist_fire = TRUE;
	if (f2 & (TR2_RES_COLD)) p_ptr->resist_cold = TRUE;
	if (f2 & (TR2_RES_POIS)) p_ptr->resist_pois = TRUE;
	if (f2 & (TR2_RES_FEAR)) p_ptr->resist_fear = TRUE;
	if (f2 & (TR2_RES_CONF)) p_ptr->resist_conf = TRUE;
	if (f2 & (TR2_RES_SOUND)) p_ptr->resist_sound = TRUE;
	if (f2 & (TR2_RES_LITE)) p_ptr->resist_lite = TRUE;
	if (f2 & (TR2_RES_DARK)) p_ptr->resist_dark = TRUE;
	if (f2 & (TR2_RES_CHAOS)) p_ptr->resist_chaos = TRUE;
	if (f2 & (TR2_RES_DISEN)) p_ptr->resist_disen = TRUE;
	if (f2 & (TR2_RES_SHARDS)) p_ptr->resist_shard = TRUE;
	if (f2 & (TR2_RES_NEXUS)) p_ptr->resist_nexus = TRUE;
	if (f2 & (TR2_RES_BLIND)) p_ptr->resist_blind = TRUE;
	if (f2 & (TR2_RES_NETHER)) p_ptr->resist_neth = TRUE;
	if (f4 & (TR4_IM_NETHER)) p_ptr->immune_neth = TRUE;

	if (f2 & (TR2_REFLECT)) p_ptr->reflect = TRUE;
	if (f3 & (TR3_SH_FIRE)) p_ptr->sh_fire = TRUE;
	if (f3 & (TR3_SH_ELEC)) p_ptr->sh_elec = TRUE;
	if (f3 & (TR3_NO_MAGIC)) p_ptr->anti_magic = TRUE;
	if (f3 & (TR3_NO_TELE)) p_ptr->anti_tele = TRUE;

	/* Sustain flags */
	if (f2 & (TR2_SUST_STR)) p_ptr->sustain_str = TRUE;
	if (f2 & (TR2_SUST_INT)) p_ptr->sustain_int = TRUE;
	if (f2 & (TR2_SUST_WIS)) p_ptr->sustain_wis = TRUE;
	if (f2 & (TR2_SUST_DEX)) p_ptr->sustain_dex = TRUE;
	if (f2 & (TR2_SUST_CON)) p_ptr->sustain_con = TRUE;
	if (f2 & (TR2_SUST_CHR)) p_ptr->sustain_chr = TRUE;

	if (f4 & (TR4_PRECOGNITION)) p_ptr->precognition = TRUE;

	if (f4 & (TR4_ANTIMAGIC_50))
	{
		s32b tmp;

		tmp = 10 + get_skill_scale(SKILL_ANTIMAGIC, 40)
		      - to_h - to_d - pval - to_a;
		if (tmp > 0) p_ptr->antimagic += tmp;

		tmp = 1 + get_skill_scale(SKILL_ANTIMAGIC, 4)
		      - (to_h + to_d + pval + to_a) / 15;
		if (tmp > 0) p_ptr->antimagic_dis += tmp;
	}

	if (f4 & (TR4_ANTIMAGIC_30))
	{
		s32b tmp;

		tmp = 7 + get_skill_scale(SKILL_ANTIMAGIC, 33)
		      - to_h - to_d - pval - to_a;
		if (tmp > 0) p_ptr->antimagic += tmp;

		tmp = 1 + get_skill_scale(SKILL_ANTIMAGIC, 2)
		      - (to_h + to_d + pval + to_a) / 15;
		if (tmp > 0) p_ptr->antimagic_dis += tmp;
	}

	if (f4 & (TR4_ANTIMAGIC_20))
	{
		s32b tmp;

		tmp = 5 + get_skill_scale(SKILL_ANTIMAGIC, 15)
		      - to_h - to_d - pval - to_a;
		if (tmp > 0) p_ptr->antimagic += tmp;

		p_ptr->antimagic_dis += 2;
	}

	if (f4 & (TR4_ANTIMAGIC_10))
	{
		s32b tmp;

		tmp = 1 + get_skill_scale(SKILL_ANTIMAGIC, 9)
		      - to_h - to_d - pval - to_a;
		if (tmp > 0) p_ptr->antimagic += tmp;

		p_ptr->antimagic_dis += 1;
	}

	if (f4 & (TR4_AUTO_ID))
	{
		p_ptr->auto_id = TRUE;
	}

	/* The new code implementing Tolkien's concept of "Black Breath"
	 * takes advantage of the existing drain_exp character flag, renamed
	 * "black_breath". This flag can also be set by a unlucky blow from
	 * an undead.  -LM-
	 */
	if (f4 & (TR4_BLACK_BREATH)) p_ptr->black_breath = TRUE;

	if (f5 & (TR5_IMMOVABLE)) p_ptr->immovable = TRUE;

	/* Breaths */
	if (f5 & (TR5_WATER_BREATH)) p_ptr->water_breath = TRUE;
	if (f5 & (TR5_MAGIC_BREATH))
	{
		p_ptr->magical_breath = TRUE;
		p_ptr->water_breath = TRUE;
	}
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
 * TRUE.
 */
void calc_bonuses(bool silent)
{
	int i, j, hold;
	int old_invis;
	int old_speed;
	u32b old_telepathy;
	int old_see_inv;
	int old_dis_ac;
	int old_dis_to_a;
	object_type *o_ptr;
	u32b f1, f2, f3, f4, f5, esp;


	/* Save the old speed */
	old_speed = p_ptr->pspeed;

	/* Save the old vision stuff */
	old_telepathy = p_ptr->telepathy;
	old_see_inv = p_ptr->see_inv;

	/* Save the old armor class */
	old_dis_ac = p_ptr->dis_ac;
	old_dis_to_a = p_ptr->dis_to_a;

	/* Save the old invisibility */
	old_invis = p_ptr->invis;

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

	/* Reset the "xtra" tval */
	p_ptr->tval_xtra = 0;

	/* Reset the "ammo" tval */
	p_ptr->tval_ammo = 0;

	/* Clear all the flags */
	p_ptr->invis = 0;
	p_ptr->immovable = FALSE;
	p_ptr->aggravate = FALSE;
	p_ptr->teleport = FALSE;
	p_ptr->exp_drain = FALSE;
	p_ptr->drain_mana = 0;
	p_ptr->drain_life = 0;
	p_ptr->bless_blade = FALSE;
	p_ptr->xtra_might = 0;
	p_ptr->auto_id = FALSE;
	p_ptr->impact = FALSE;
	p_ptr->see_inv = FALSE;
	p_ptr->free_act = FALSE;
	p_ptr->slow_digest = FALSE;
	p_ptr->regenerate = FALSE;
	p_ptr->fly = FALSE;
	p_ptr->climb = FALSE;
	p_ptr->ffall = FALSE;
	p_ptr->hold_life = FALSE;
	p_ptr->telepathy = 0;
	p_ptr->lite = FALSE;
	p_ptr->sustain_str = FALSE;
	p_ptr->sustain_int = FALSE;
	p_ptr->sustain_wis = FALSE;
	p_ptr->sustain_con = FALSE;
	p_ptr->sustain_dex = FALSE;
	p_ptr->sustain_chr = FALSE;
	p_ptr->resist_acid = FALSE;
	p_ptr->resist_elec = FALSE;
	p_ptr->resist_fire = FALSE;
	p_ptr->resist_cold = FALSE;
	p_ptr->resist_pois = FALSE;
	p_ptr->resist_conf = FALSE;
	p_ptr->resist_sound = FALSE;
	p_ptr->resist_lite = FALSE;
	p_ptr->resist_dark = FALSE;
	p_ptr->resist_chaos = FALSE;
	p_ptr->resist_disen = FALSE;
	p_ptr->resist_shard = FALSE;
	p_ptr->resist_nexus = FALSE;
	p_ptr->resist_blind = FALSE;
	p_ptr->resist_neth = FALSE;
	p_ptr->immune_neth = FALSE;
	p_ptr->resist_fear = FALSE;
	p_ptr->resist_continuum = FALSE;
	p_ptr->reflect = FALSE;
	p_ptr->sh_fire = FALSE;
	p_ptr->sh_elec = FALSE;
	p_ptr->anti_magic = FALSE;
	p_ptr->anti_tele = FALSE;
	p_ptr->water_breath = FALSE;
	p_ptr->magical_breath = FALSE;

	p_ptr->sensible_fire = FALSE;
	p_ptr->sensible_lite = FALSE;

	p_ptr->immune_acid = FALSE;
	p_ptr->immune_elec = FALSE;
	p_ptr->immune_fire = FALSE;
	p_ptr->immune_cold = FALSE;

	p_ptr->precognition = FALSE;

	p_ptr->wraith_form = FALSE;

	/* The anti magic field surrounding the player */
	p_ptr->antimagic = 0;
	p_ptr->antimagic_dis = 0;


	/* Base infravision (purely racial) */
	p_ptr->see_infra = rp_ptr->infra + rmp_ptr->infra;


	/* Base skill -- disarming */
	p_ptr->skill_dis = 0;

	/* Base skill -- magic devices */
	p_ptr->skill_dev = 0;

	/* Base skill -- saving throw */
	p_ptr->skill_sav = 0;

	/* Base skill -- stealth */
	p_ptr->skill_stl = 0;

	/* Base skill -- searching ability */
	p_ptr->skill_srh = 0;

	/* Base skill -- searching frequency */
	p_ptr->skill_fos = 0;

	/* Base skill -- combat (normal) */
	p_ptr->skill_thn = 0;

	/* Base skill -- combat (shooting) */
	p_ptr->skill_thb = 0;

	/* Base skill -- combat (throwing) */
	p_ptr->skill_tht = 0;


	/* Base skill -- digging */
	p_ptr->skill_dig = 0;

	/* Xtra player flags */
	p_ptr->xtra_f1 = 0;
	p_ptr->xtra_f2 = 0;
	p_ptr->xtra_f3 = 0;
	p_ptr->xtra_f4 = 0;
	p_ptr->xtra_f5 = 0;
	p_ptr->xtra_esp = 0;

	/* Hide the skills that should auto hide */
	for (i = 0; i < max_s_idx; i++)
	{
		if (s_info[i].flags1 & SKF1_AUTO_HIDE)
			s_info[i].hidden = TRUE;
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

	/* Let the scripts do what they need */
	process_hooks(HOOK_CALC_BONUS, "()");

	/* The powers gived by the wielded monster */
	calc_wield_monster();

	for (i = 1; i <= p_ptr->lev; i++)
	{
		apply_flags(cp_ptr->oflags1[i], cp_ptr->oflags2[i], cp_ptr->oflags3[i], cp_ptr->oflags4[i], cp_ptr->oflags5[i], cp_ptr->oesp[i], cp_ptr->opval[i], 0, 0, 0, 0);
	}

	if (p_ptr->melee_style == SKILL_HAND)
	{
		/* Unencumbered Monks become faster every 10 levels */
		if (!(monk_heavy_armor()))
			p_ptr->pspeed += get_skill_scale(SKILL_HAND, 5);

		/* Free action if unencumbered at level 25 */
		if ((get_skill(SKILL_HAND) > 24) && !(monk_heavy_armor()))
			p_ptr->free_act = TRUE;
	}

	if (get_skill(SKILL_ANTIMAGIC))
	{
		p_ptr->antimagic += get_skill(SKILL_ANTIMAGIC);
		p_ptr->antimagic_dis += get_skill_scale(SKILL_ANTIMAGIC, 10) + 1;

		if (p_ptr->antimagic_extra & CLASS_ANTIMAGIC)
		{
			p_ptr->anti_tele = TRUE;
			p_ptr->resist_continuum = TRUE;
		}
	}

	if (get_skill(SKILL_DAEMON) > 20) p_ptr->resist_conf = TRUE;
	if (get_skill(SKILL_DAEMON) > 30) p_ptr->resist_fear = TRUE;

	if ( get_skill(SKILL_MINDCRAFT) >= 40 ) p_ptr->telepathy = ESP_ALL;

	if (p_ptr->astral)
	{
		p_ptr->wraith_form = TRUE;
	}

	/***** Races ****/
	if ((!p_ptr->mimic_form) && (!p_ptr->body_monster))
	{
		int i;

		for (i = 1; i <= p_ptr->lev; i++)
		{
			apply_flags(rp_ptr->oflags1[i], rp_ptr->oflags2[i], rp_ptr->oflags3[i], rp_ptr->oflags4[i], rp_ptr->oflags5[i], rp_ptr->oesp[i], rp_ptr->opval[i], 0, 0, 0, 0);
			apply_flags(rmp_ptr->oflags1[i], rmp_ptr->oflags2[i], rmp_ptr->oflags3[i], rmp_ptr->oflags4[i], rmp_ptr->oflags5[i], rmp_ptr->oesp[i], rmp_ptr->opval[i], 0, 0, 0, 0);
		}

		if (PRACE_FLAG(PR1_HURT_LITE))
			p_ptr->sensible_lite = TRUE;
	}

	/* The extra flags */
	apply_flags(p_ptr->xtra_f1, p_ptr->xtra_f2, p_ptr->xtra_f3, p_ptr->xtra_f4, p_ptr->xtra_f5, p_ptr->xtra_esp, 0, 0, 0, 0, 0);

	/* Hack -- apply racial/class stat maxes */
	if (p_ptr->maximize)
	{
		/* Apply the racial modifiers */
		for (i = 0; i < 6; i++)
		{
			/* Modify the stats for "race" */
			p_ptr->stat_add[i] += (rp_ptr->r_adj[i] + rmp_ptr->r_adj[i] + cp_ptr->c_adj[i]);
		}
	}


	/* Scan the usable inventory */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Extract the item flags */
		object_flags_no_set = TRUE;
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
		object_flags_no_set = FALSE;

		/* MEGA ugly hack -- set spacetime distortion resistance */
		if (o_ptr->name1 == ART_ANCHOR)
		{
			p_ptr->resist_continuum = TRUE;
		}

		/* Hack - don't give the Black Breath when merely inspecting a weapon */
		if (silent)
		{
			f4 &= ~TR4_BLACK_BREATH;
		}

		apply_flags(f1, f2, f3, f4, f5, esp, o_ptr->pval, o_ptr->tval, o_ptr->to_h, o_ptr->to_d, o_ptr->to_a);

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
		if (object_known_p(o_ptr)) p_ptr->dis_to_a += o_ptr->to_a;

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
		if (object_known_p(o_ptr)) p_ptr->dis_to_h += o_ptr->to_h;
		if (object_known_p(o_ptr)) p_ptr->dis_to_d += o_ptr->to_d;
	}

	/* Monks get extra ac for armour _not worn_ */
	if ((p_ptr->melee_style == SKILL_HAND) && !(monk_heavy_armor()))
	{
		if (!(p_ptr->inventory[INVEN_BODY].k_idx))
		{
			p_ptr->to_a += get_skill_scale(SKILL_HAND, 75);
			p_ptr->dis_to_a += get_skill_scale(SKILL_HAND, 75);
		}
		if (!(p_ptr->inventory[INVEN_OUTER].k_idx) && (get_skill(SKILL_HAND) > 15))
		{
			p_ptr->to_a += ((get_skill(SKILL_HAND) - 13) / 3);
			p_ptr->dis_to_a += ((get_skill(SKILL_HAND) - 13) / 3);
		}
		if (!(p_ptr->inventory[INVEN_ARM].k_idx) && (get_skill(SKILL_HAND) > 10))
		{
			p_ptr->to_a += ((get_skill(SKILL_HAND) - 8) / 3);
			p_ptr->dis_to_a += ((get_skill(SKILL_HAND) - 8) / 3);
		}
		if (!(p_ptr->inventory[INVEN_HEAD].k_idx) && (get_skill(SKILL_HAND) > 4))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) - 2) / 3;
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) - 2) / 3;
		}
		if (!(p_ptr->inventory[INVEN_HANDS].k_idx))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) / 2);
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) / 2);
		}
		if (!(p_ptr->inventory[INVEN_FEET].k_idx))
		{
			p_ptr->to_a += (get_skill(SKILL_HAND) / 3);
			p_ptr->dis_to_a += (get_skill(SKILL_HAND) / 3);
		}
	}

	/* Hack -- aura of fire also provides light */
	if (p_ptr->sh_fire) p_ptr->lite = TRUE;

	if (PRACE_FLAG(PR1_AC_LEVEL))
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
			p_ptr->redraw |= (PR_STATS);

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
			p_ptr->redraw |= (PR_STATS);

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
	GOD(GOD_MELKOR)
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
	p_ptr->skill_dis += tactic_info[(byte)p_ptr->tactic].to_disarm;
	p_ptr->skill_sav += tactic_info[(byte)p_ptr->tactic].to_saving;

	p_ptr->pspeed += move_info[(byte)p_ptr->movement].to_speed;
	p_ptr->skill_srh += move_info[(byte)p_ptr->movement].to_search;
	p_ptr->skill_fos += move_info[(byte)p_ptr->movement].to_percep;
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

	/* Breath */
	if (p_ptr->tim_water_breath)
	{
		p_ptr->water_breath = TRUE;
	}
	if (p_ptr->tim_magic_breath)
	{
		p_ptr->magical_breath = TRUE;
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
			p_ptr->reflect = TRUE;
		}
		p_ptr->wraith_form = TRUE;
	}

	/* Temporary holy aura */
	if (p_ptr->holy)
	{
		p_ptr->hold_life = TRUE;
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

	/* Temporary "Meditation" */
	if (p_ptr->meditation)
	{
		p_ptr->to_d -= 25;
		p_ptr->dis_to_d -= 25;
		p_ptr->to_h -= 25;
		p_ptr->dis_to_h -= 25;
	}

	/* Temporary "Reflection" */
	if (p_ptr->tim_reflect)
	{
		p_ptr->reflect = TRUE;
	}

	/* Temporary "Time Resistance" */
	if (p_ptr->tim_res_time)
	{
		p_ptr->resist_continuum = TRUE;
	}

	/* Temporary "Levitation" and "Flying" */
	if (p_ptr->tim_ffall)
	{
		p_ptr->ffall = TRUE;
	}
	if (p_ptr->tim_fly)
	{
		p_ptr->fly = TRUE;
	}

	/* Temporary "Fire Aura" */
	if (p_ptr->tim_fire_aura)
	{
		p_ptr->sh_fire = TRUE;
	}

	/* Oppose Light & Dark */
	if (p_ptr->oppose_ld)
	{
		p_ptr->resist_lite = TRUE;
		p_ptr->resist_dark = TRUE;
	}

	/* Oppose Chaos & Confusion */
	if (p_ptr->oppose_cc)
	{
		p_ptr->resist_chaos = TRUE;
		p_ptr->resist_conf = TRUE;
	}

	/* Oppose Sound & Shards */
	if (p_ptr->oppose_ss)
	{
		p_ptr->resist_sound = TRUE;
		p_ptr->resist_shard = TRUE;
	}

	/* Oppose Nexus */
	if (p_ptr->oppose_nex)
	{
		p_ptr->resist_nexus = TRUE;
	}

	/* Mental barrier */
	if (p_ptr->tim_mental_barrier)
	{
		p_ptr->sustain_int = TRUE;
		p_ptr->sustain_wis = TRUE;
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
		p_ptr->telepathy |= ESP_ALL;
	}

	/* Temporary see invisible */
	if (p_ptr->tim_invis)
	{
		p_ptr->see_inv = TRUE;
	}

	/* Temporary infravision boost */
	if (p_ptr->tim_infra)
	{
		p_ptr->see_infra++;
	}

	/* Hack -- Magic breath -> Water breath */
	if (p_ptr->magical_breath)
	{
		p_ptr->water_breath = TRUE;
	}

	/* Hack -- Can Fly -> Can Levitate */
	if (p_ptr->fly)
	{
		p_ptr->ffall = TRUE;
	}

	/* Hack -- Res Chaos -> Res Conf */
	if (p_ptr->resist_chaos)
	{
		p_ptr->resist_conf = TRUE;
	}

	/* Hack -- Hero/Shero -> Res fear */
	if (p_ptr->hero || p_ptr->shero)
	{
		p_ptr->resist_fear = TRUE;
	}


	/* Hack -- Telepathy Change */
	if (p_ptr->telepathy != old_telepathy)
	{
		p_ptr->update |= (PU_MONSTERS);
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

	/* Searching slows the player down */
	if (p_ptr->searching) p_ptr->pspeed -= 10;

	/* In order to get a "nice" mana path druids need to ahve a 0 speed */
	if ((p_ptr->druid_extra2 == CLASS_MANA_PATH) && (p_ptr->pspeed > 110))
		p_ptr->pspeed = 110;

	/* Display the speed (if needed) */
	if (p_ptr->pspeed != old_speed) p_ptr->redraw |= (PR_SPEED);


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
		p_ptr->redraw |= (PR_ARMOR);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}


	/* Obtain the "hold" value */
	hold = adj_str_hold[p_ptr->stat_ind[A_STR]];


	/* Examine the "current bow" */
	o_ptr = &p_ptr->inventory[INVEN_BOW];


	/* Assume not heavy */
	p_ptr->heavy_shoot = FALSE;

	/* It is hard to carholdry a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
		p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		p_ptr->heavy_shoot = TRUE;
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
	if (o_ptr->k_idx && !p_ptr->heavy_shoot)
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

	if (PRACE_FLAG(PR1_XTRA_MIGHT_BOW) && p_ptr->tval_ammo == TV_ARROW)
		p_ptr->xtra_might += 1;

	if (PRACE_FLAG(PR1_XTRA_MIGHT_SLING) && p_ptr->tval_ammo == TV_SHOT)
		p_ptr->xtra_might += 1;

	if (PRACE_FLAG(PR1_XTRA_MIGHT_XBOW) && p_ptr->tval_ammo == TV_BOLT)
		p_ptr->xtra_might += 1;

	/* Examine the "current tool" */
	o_ptr = &p_ptr->inventory[INVEN_TOOL];

	/* Boost digging skill by tool weight */
	if (o_ptr->k_idx && (o_ptr->tval == TV_DIGGING))
	{
		p_ptr->skill_dig += (o_ptr->weight / 10);
	}

	/* Examine the main weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Assume not heavy */
	p_ptr->heavy_wield = FALSE;

	/* Normal weapons */
	if (o_ptr->k_idx && !p_ptr->heavy_wield)
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

		p_ptr->num_blow = 0;

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
		int str_index, dex_index;
		int num = 0, wgt = 0, mul = 0;
		monster_race *r_ptr = race_info_idx(p_ptr->body_monster, 0);

		analyze_blow(&num, &wgt, &mul);

		/* Access the strength vs weight */
		str_index = (adj_str_blow[p_ptr->stat_ind[A_STR]] * mul / 3);

		/* Maximal value */
		if (str_index > 11) str_index = 11;

		/* Index by dexterity */
		dex_index = (adj_dex_blow[p_ptr->stat_ind[A_DEX]]);

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
	p_ptr->icky_wield = FALSE;
	monk_armour_aux = FALSE;

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
		if (p_ptr->inventory[INVEN_WIELD + i].k_idx && p_ptr->inventory[INVEN_ARM + i].k_idx)
		{
			/* Extract the item flags */
			object_flags(&p_ptr->inventory[INVEN_WIELD + i], &f1, &f2, &f3, &f4, &f5, &esp);

			if (f4 & TR4_COULD2H)
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
		if (((forbid_non_blessed()) && (!p_ptr->bless_blade) &&
		                ((o_ptr->tval == TV_AXE) || (o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM))) && (o_ptr->k_idx))
		{
			/* Reduce the real bonuses */
			p_ptr->to_h -= 15;
			p_ptr->to_d -= 15;

			/* Reduce the mental bonuses */
			p_ptr->dis_to_h -= 15;
			p_ptr->dis_to_d -= 15;

			/* Icky weapon */
			p_ptr->icky_wield = TRUE;
		}

		/* Sorcerer can't wield a weapon unless it's a mage staff */
		if (get_skill(SKILL_SORCERY))
		{
			int malus = get_skill_scale(SKILL_SORCERY, 100);

			if ((o_ptr->tval != TV_MSTAFF) && (o_ptr->k_idx))
			{
				/* Reduce the real bonuses */
				p_ptr->to_h -= malus;
				p_ptr->to_d -= malus;

				/* Reduce the mental bonuses */
				p_ptr->dis_to_h -= malus;
				p_ptr->dis_to_d -= malus;

				/* Icky weapon */
				p_ptr->icky_wield = TRUE;
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
		monk_armour_aux = TRUE;
	}

	/* Affect Skill -- stealth (bonus one) */
	p_ptr->skill_stl += 1;

	/* Affect Skill -- disarming (DEX and INT) */
	p_ptr->skill_dis += adj_dex_dis[p_ptr->stat_ind[A_DEX]];
	p_ptr->skill_dis += adj_int_dis[p_ptr->stat_ind[A_INT]];

	/* Affect Skill -- magic devices (INT) */
	p_ptr->skill_dev += get_skill_scale(SKILL_DEVICE, 20);

	/* Affect Skill -- saving throw (WIS) */
	p_ptr->skill_sav += adj_wis_sav[p_ptr->stat_ind[A_WIS]];

	/* Affect Skill -- digging (STR) */
	p_ptr->skill_dig += adj_str_dig[p_ptr->stat_ind[A_STR]];

	/* Affect Skill -- disarming (skill) */
	p_ptr->skill_dis += (get_skill_scale(SKILL_DISARMING, 75));

	/* Affect Skill -- magic devices (skill) */
	p_ptr->skill_dev += (get_skill_scale(SKILL_DEVICE, 150));

	/* Affect Skill -- saving throw (skill and level) */
	p_ptr->skill_sav += (get_skill_scale(SKILL_SPIRITUALITY, 75));

	/* Affect Skill -- stealth (skill) */
	p_ptr->skill_stl += (get_skill_scale(SKILL_STEALTH, 25));

	/* Affect Skill -- search ability (Sneakiness skill) */
	p_ptr->skill_srh += (get_skill_scale(SKILL_SNEAK, 35));

	/* Affect Skill -- search frequency (Sneakiness skill) */
	p_ptr->skill_fos += (get_skill_scale(SKILL_SNEAK, 25));

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
		else if (p_ptr->inventory[INVEN_BOW].k_idx)
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
		else if (p_ptr->inventory[INVEN_WIELD].k_idx)
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
		else if (p_ptr->inventory[INVEN_WIELD].k_idx)
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
		p_ptr->resist_lite = p_ptr->sensible_lite = FALSE;
	}

	/* resistance to fire cancel sensibility to fire */
	if (p_ptr->resist_fire || p_ptr->oppose_fire || p_ptr->immune_fire)
		p_ptr->sensible_fire = FALSE;

	/* Minimum saving throw */
	if(p_ptr->skill_sav <= 10)
		p_ptr->skill_sav = 10;
	else
		p_ptr->skill_sav += 10;

	/* Let the scripts do what they need */
	process_hooks(HOOK_CALC_BONUS_END, "(d)", silent);
}



/*
 * Handle "p_ptr->notice"
 */
void notice_stuff(void)
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
void update_stuff(void)
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
		calc_bonuses(FALSE);
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
		calc_spells();
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
		update_monsters(TRUE);
	}

	if (p_ptr->update & (PU_MONSTERS))
	{
		p_ptr->update &= ~(PU_MONSTERS);
		update_monsters(FALSE);
	}

	if (p_ptr->update & (PU_MON_LITE))
	{
		p_ptr->update &= ~(PU_MON_LITE);
		if (monster_lite) update_mon_lite();
	}
}


/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(void)
{
	/* Redraw stuff */
	if (!p_ptr->redraw) return;


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;


	/* Should we tell lua to redisplay too ? */
	process_hooks(HOOK_REDRAW, "()");


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


	if (p_ptr->redraw & (PR_BASIC))
	{
		p_ptr->redraw &= ~(PR_BASIC);
		p_ptr->redraw &= ~(PR_MISC | PR_TITLE | PR_STATS);
		p_ptr->redraw &= ~(PR_LEV | PR_EXP | PR_GOLD);
		p_ptr->redraw &= ~(PR_ARMOR | PR_HP | PR_MANA | PR_PIETY | PR_MH);
		p_ptr->redraw &= ~(PR_DEPTH | PR_HEALTH);
		prt_frame_basic();
	}

	if (p_ptr->redraw & (PR_MISC))
	{
		p_ptr->redraw &= ~(PR_MISC);
		prt_field(rp_ptr->title + rp_name, ROW_RACE, COL_RACE);
		prt_field(spp_ptr->title + c_name, ROW_CLASS, COL_CLASS);
	}

	if (p_ptr->redraw & (PR_TITLE))
	{
		p_ptr->redraw &= ~(PR_TITLE);
		prt_title();
	}

	if (p_ptr->redraw & (PR_LEV))
	{
		p_ptr->redraw &= ~(PR_LEV);
		prt_level();
	}

	if (p_ptr->redraw & (PR_EXP))
	{
		p_ptr->redraw &= ~(PR_EXP);
		prt_exp();
	}

	if (p_ptr->redraw & (PR_STATS))
	{
		p_ptr->redraw &= ~(PR_STATS);
		prt_stat(A_STR);
		prt_stat(A_INT);
		prt_stat(A_WIS);
		prt_stat(A_DEX);
		prt_stat(A_CON);
		prt_stat(A_CHR);
	}

	if (p_ptr->redraw & (PR_ARMOR))
	{
		p_ptr->redraw &= ~(PR_ARMOR);
		prt_ac();
	}

	if (p_ptr->redraw & (PR_HP))
	{
		p_ptr->redraw &= ~(PR_HP);
		prt_hp();
	}

	if (p_ptr->redraw & (PR_MANA))
	{
		p_ptr->redraw &= ~(PR_MANA);
		prt_sp();
	}

	if (p_ptr->redraw & (PR_PIETY))
	{
		p_ptr->redraw &= ~(PR_PIETY);
		prt_piety();
	}

	if (p_ptr->redraw & (PR_MH))
	{
		p_ptr->redraw &= ~(PR_MH);
		prt_mh();
	}

	if (p_ptr->redraw & (PR_GOLD))
	{
		p_ptr->redraw &= ~(PR_GOLD);
		prt_gold();
	}

	if (p_ptr->redraw & (PR_DEPTH))
	{
		p_ptr->redraw &= ~(PR_DEPTH);
		prt_depth();
	}

	if (p_ptr->redraw & (PR_HEALTH))
	{
		p_ptr->redraw &= ~(PR_HEALTH);
		health_redraw();
	}


	if (p_ptr->redraw & (PR_EXTRA))
	{
		p_ptr->redraw &= ~(PR_EXTRA);
		p_ptr->redraw &= ~(PR_CUT | PR_STUN);
		p_ptr->redraw &= ~(PR_HUNGER);
		p_ptr->redraw &= ~(PR_BLIND | PR_CONFUSED);
		p_ptr->redraw &= ~(PR_AFRAID | PR_POISONED);
		p_ptr->redraw &= ~(PR_STATE | PR_SPEED | PR_STUDY | PR_SANITY);
		prt_frame_extra();
	}

	if (p_ptr->redraw & (PR_CUT))
	{
		p_ptr->redraw &= ~(PR_CUT);
		prt_cut();
	}

	if (p_ptr->redraw & (PR_STUN))
	{
		p_ptr->redraw &= ~(PR_STUN);
		prt_stun();
	}

	if (p_ptr->redraw & (PR_HUNGER))
	{
		p_ptr->redraw &= ~(PR_HUNGER);
		prt_hunger();
	}

	if (p_ptr->redraw & (PR_BLIND))
	{
		p_ptr->redraw &= ~(PR_BLIND);
		prt_blind();
	}

	if (p_ptr->redraw & (PR_CONFUSED))
	{
		p_ptr->redraw &= ~(PR_CONFUSED);
		prt_confused();
	}

	if (p_ptr->redraw & (PR_AFRAID))
	{
		p_ptr->redraw &= ~(PR_AFRAID);
		prt_afraid();
	}

	if (p_ptr->redraw & (PR_POISONED))
	{
		p_ptr->redraw &= ~(PR_POISONED);
		prt_poisoned();
	}

	if (p_ptr->redraw & (PR_DTRAP))
	{
		p_ptr->redraw &= ~(PR_DTRAP);
		prt_dtrap();
	}

	if (p_ptr->redraw & (PR_STATE))
	{
		p_ptr->redraw &= ~(PR_STATE);
		prt_state();
	}

	if (p_ptr->redraw & (PR_SPEED))
	{
		p_ptr->redraw &= ~(PR_SPEED);
		prt_speed();
	}

	if (p_ptr->redraw & (PR_STUDY))
	{
		p_ptr->redraw &= ~(PR_STUDY);
		prt_study();
	}

	if (p_ptr->redraw & (PR_SANITY))
	{
		p_ptr->redraw &= ~(PR_SANITY);
		prt_sane();
	}
}


/*
 * Handle "p_ptr->window"
 */
void window_stuff(void)
{
	int j;

	u32b mask = 0L;


	/* Nothing to do */
	if (!p_ptr->window) return;

	/* Scan windows */
	for (j = 0; j < 8; j++)
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
	if (p_ptr->window & (PW_IRC))
	{
		p_ptr->window &= ~(PW_IRC);
		fix_irc_message();
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
void handle_stuff(void)
{
	/* Update stuff */
	if (p_ptr->update) update_stuff();

	/* Redraw stuff */
	if (p_ptr->redraw) redraw_stuff();

	/* Window stuff */
	if (p_ptr->window) window_stuff();
}


bool monk_empty_hands(void)
{
	int i;
	object_type *o_ptr;

	if (p_ptr->melee_style != SKILL_HAND) return FALSE;

	i = 0;
	while (p_ptr->body_parts[i] == INVEN_WIELD)
	{
		o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		if (o_ptr->k_idx) return FALSE;

		i++;
	}

	return TRUE;
}

bool monk_heavy_armor(void)
{
	u16b monk_arm_wgt = 0;

	if (p_ptr->melee_style != SKILL_HAND) return FALSE;

	/* Weight the armor */
	monk_arm_wgt += p_ptr->inventory[INVEN_BODY].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_HEAD].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_ARM].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_OUTER].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_HANDS].weight;
	monk_arm_wgt += p_ptr->inventory[INVEN_FEET].weight;

	return (monk_arm_wgt > (100 + (get_skill(SKILL_HAND) * 4))) ;
}

static int get_artifact_idx(int level)
{
	int count = 0, i;
	bool OK = FALSE;

	while (count < 1000)
	{
		artifact_type *a_ptr;

		count++;
		i = randint(max_a_idx - 1);
		a_ptr = &a_info[i];
		if (!a_ptr->tval) continue;

		/* It is found/lost */
		if (a_ptr->cur_num) continue;

		/* OoD */
		if (a_ptr->level > level) continue;

		/* Avoid granting SPECIAL_GENE artifacts */
		if (a_ptr->flags4 & TR4_SPECIAL_GENE) continue;

		OK = TRUE;
		break;
	}

	/* No matches found */
	if (OK == FALSE)
	{
#if 0 /* pelpel */
		/* XXX XXX XXX Grant the Phial */
		i = 1;
#endif /* pelpel */

		/* Grant a randart */
		i = 0;
	}

	return i;
}

/* Chose a fate */
void gain_fate(byte fate)
{
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
					while (TRUE)
					{
						object_kind *k_ptr;
						obj_theme theme;

						/* No themes */
						theme.treasure = 100;
						theme.combat = 100;
						theme.magic = 100;
						theme.tools = 100;
						init_match_theme(theme);

						/* Apply restriction */
						get_obj_num_hook = kind_is_legal;

						/* Rebuild allocation table */
						get_obj_num_prep();

						fates[i].o_idx = get_obj_num(max_dlv[dungeon_type] + randint(10));

						/* Invalidate the cached allocation table */
						alloc_kind_table_valid = FALSE;

						k_ptr = &k_info[fates[i].o_idx];

						if (!(k_ptr->flags3 & TR3_INSTA_ART) && !(k_ptr->flags3 & TR3_NORM_ART)) break;
					}
					level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
					fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
					fates[i].serious = rand_int(2);
					fates[i].know = FALSE;
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
				fates[i].know = FALSE;
				if (wizard) msg_format("New fate : Meet monster %d on level %d", fates[i].r_idx, fates[i].level);
				break;

			case FATE_FIND_A:
				fates[i].a_idx = get_artifact_idx(max_dlv[dungeon_type] + randint(10));
				level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
				fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
				fates[i].serious = TRUE;
				fates[i].know = FALSE;
				if (wizard) msg_format("New fate : Find artifact %d on level %d", fates[i].a_idx, fates[i].level);
				break;

			case FATE_DIE:
				level = rand_range(max_dlv[dungeon_type] - 20, max_dlv[dungeon_type] + 20);
				fates[i].level = (level < 1) ? 1 : (level > 98) ? 98 : level;
				fates[i].serious = TRUE;
				fates[i].know = FALSE;
				if ((wizard) || (p_ptr->precognition)) msg_format("New fate : Death on level %d", fates[i].level);
				break;

			case FATE_NO_DIE_MORTAL:
				fates[i].serious = TRUE;
				p_ptr->no_mortal = TRUE;
				if ((wizard) || (p_ptr->precognition)) msg_format("New fate : Never to die by the hand of a mortal being.");
				break;
			}

			break;
		}
	}
}

void fate_desc(char *desc, int fate)
{
	char buf[120];

	if (fates[fate].serious)
	{
		strcpy(desc, "You are fated to ");
	}
	else
	{
		strcpy(desc, "You may ");
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

			sprintf(buf, "find %s on level %d.", o_name, fates[fate].level);
			strcat(desc, buf);
			break;
		}
	case FATE_FIND_A:
		{
			object_type *q_ptr, forge;
			char o_name[80];
			artifact_type *a_ptr = &a_info[fates[fate].a_idx];
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
				if (a_ptr->flags3 & (TR3_CURSED)) q_ptr->ident |= (IDENT_CURSED);

				random_artifact_resistance(q_ptr);

				object_desc_store(o_name, q_ptr, 1, 0);
			}

			sprintf(buf, "find %s on level %d.", o_name, fates[fate].level);
			strcat(desc, buf);
			break;
		}
	case FATE_FIND_R:
		{
			char m_name[80];

			monster_race_desc(m_name, fates[fate].r_idx, 0);
			sprintf(buf, "meet %s on level %d.", m_name, fates[fate].level);
			strcat(desc, buf);
			break;
		}
	case FATE_DIE:
		{
			sprintf(buf, "die on level %d.", fates[fate].level);
			strcat(desc, buf);
			break;
		}
	case FATE_NO_DIE_MORTAL:
		{
			strcat(desc, "never to die by the hand of a mortal being.");
			break;
		}
	}
}

void dump_fates(FILE *outfile)
{
	int i;
	char buf[120];

	if (!outfile) return;

	for (i = 0; i < MAX_FATES; i++)
	{
		if ((fates[i].fate) && (fates[i].know))
		{
			fate_desc(buf, i);
			fprintf(outfile, "%s\n", buf);
		}
	}
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
