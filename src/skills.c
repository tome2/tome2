/* File: skills.c */

/* Purpose: player skills */

/*
 * Copyright (c) 2001 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Advance the skill point of the skill specified by i and
 * modify related skills
 */
void increase_skill(int i, s16b *invest)
{
	s32b max_skill_overage;

	/* No skill points to be allocated */
	if (!p_ptr->skill_points) return;

	/* The skill cannot be increased */
	if (!s_info[i].mod) return;

	/* The skill is already maxed */
	if (s_info[i].value >= SKILL_MAX) return;

	/* Cannot allocate more than player level + max_skill_overage levels */
	call_lua("get_module_info", "(s)", "d", "max_skill_overage", &max_skill_overage);
	if (((s_info[i].value + s_info[i].mod) / SKILL_STEP) >= (p_ptr->lev + max_skill_overage + 1))
	{
		int hgt, wid;

		Term_get_size(&wid, &hgt);
		msg_box(format("Cannot raise a skill value above %i + player level.", max_skill_overage), (int)(hgt / 2), (int)(wid / 2));
		return;
	}

	/* Spend an unallocated skill point */
	p_ptr->skill_points--;

	/* Increase the skill */
	s_info[i].value += s_info[i].mod;
	invest[i]++;
}


/*
 * Descrease the skill point of the skill specified by i and
 * modify related skills
 */
void decrease_skill(int i, s16b *invest)
{
	/* Cannot decrease more */
	if (!invest[i]) return;

	/* The skill cannot be decreased */
	if (!s_info[i].mod) return;

	/* The skill already has minimal value */
	if (!s_info[i].value) return;

	/* Free a skill point */
	p_ptr->skill_points++;

	/* Decrease the skill */
	s_info[i].value -= s_info[i].mod;
	invest[i]--;
}


/*
 * Given the name of a skill, returns skill index or -1 if no
 * such skill is found
 */
s16b find_skill(cptr name)
{
	u16b i;

	/* Scan skill list */
	for (i = 1; i < max_s_idx; i++)
	{
		/* The name matches */
		if (streq(s_info[i].name + s_name, name)) return (i);
	}

	/* No match found */
	return ( -1);
}
s16b find_skill_i(cptr name)
{
	u16b i;

	/* Scan skill list */
	for (i = 1; i < max_s_idx; i++)
	{
		/* The name matches */
		if (0 == stricmp(s_info[i].name + s_name, name)) return (i);
	}

	/* No match found */
	return ( -1);
}


/*
 *
 */
s16b get_skill(int skill)
{
	return (s_info[skill].value / SKILL_STEP);
}


/*
 * Return "scale" (a misnomer -- this is max value) * (current skill value)
 * / (max skill value)
 */
s16b get_skill_scale(int skill, u32b scale)
{
	s32b temp;

	/*
	* SKILL_STEP shouldn't matter here because the second parameter is
	* relatively small (the largest one being somewhere around 200),
	* AND because we could have used much simpler 0--50 if the ability
	* progression were only possible at step boundaries.
	*
	* Because I'm not at all certain about my interpretation of the mysterious
	* formula given above, I verified this works the same by using a tiny
	* scheme program... -- pelpel
	*/
	temp = scale * s_info[skill].value;

	return (temp / SKILL_MAX);
}


/*
 *
 */
int get_idx(int i)
{
	int j;

	for (j = 1; j < max_s_idx; j++)
	{
		if (s_info[j].order == i)
			return (j);
	}
	return (0);
}

static bool is_known(int s_idx)
{
	int i;

	if (wizard) return TRUE;
	if (s_info[s_idx].value || s_info[s_idx].mod) return TRUE;

	for (i = 0; i < max_s_idx; i++)
	{
		/* It is our child, if we don't know it we continue to search, if we know it it is enough*/
		if (s_info[i].father == s_idx)
		{
			if (is_known(i))
				return TRUE;
		}
	}

	/* Ok know none */
	return FALSE;
}

/*
 *
 */
void init_table_aux(int table[MAX_SKILLS][2], int *idx, int father, int lev,
                    bool full)
{
	int j, i;

	for (j = 1; j < max_s_idx; j++)
	{
		i = get_idx(j);
		if (s_info[i].father != father) continue;
		if (s_info[i].hidden) continue;
		if (!is_known(i)) continue;

		table[*idx][0] = i;
		table[*idx][1] = lev;
		(*idx)++;
		if (s_info[i].dev || full) init_table_aux(table, idx, i, lev + 1, full);
	}
}


void init_table(int table[MAX_SKILLS][2], int *max, bool full)
{
	*max = 0;
	init_table_aux(table, max, -1, 0, full);
}


bool has_child(int sel)
{
	int i;

	for (i = 1; i < max_s_idx; i++)
	{
		if ((s_info[i].father == sel) && (is_known(i)))
			return (TRUE);
	}
	return (FALSE);
}


/*
 * Dump the skill tree
 */
void dump_skills(FILE *fff)
{
	int i, j, max = 0;
	int table[MAX_SKILLS][2];
	char buf[80];

	init_table(table, &max, TRUE);

	fprintf(fff, "\nSkills (points left: %d)", p_ptr->skill_points);

	for (j = 0; j < max; j++)
	{
		int z;

		i = table[j][0];

		if ((s_info[i].value == 0) && (i != SKILL_MISC))
		{
			if (s_info[i].mod == 0) continue;
		}

		sprintf(buf, "\n");

		for (z = 0; z < table[j][1]; z++) strcat(buf, "         ");

		if (!has_child(i))
		{
			strcat(buf, format(" . %s", s_info[i].name + s_name));
		}
		else
		{
			strcat(buf, format(" - %s", s_info[i].name + s_name));
		}

		fprintf(fff, "%-49s%s%02ld.%03ld [%01ld.%03ld]",
		        buf, s_info[i].value < 0 ? "-" : " ",
			ABS(s_info[i].value) / SKILL_STEP,
			ABS(s_info[i].value) % SKILL_STEP,
		        s_info[i].mod / 1000, s_info[i].mod % 1000);
	}

	fprintf(fff, "\n");
}


/*
 * Draw the skill tree
 */
void print_skills(int table[MAX_SKILLS][2], int max, int sel, int start)
{
	int i, j;
	int wid, hgt;
	cptr keys;

	Term_clear();
	Term_get_size(&wid, &hgt);

	c_prt(TERM_WHITE, format("%s Skills Screen", game_module), 0, 28);
	keys = format("#BEnter#W to develop a branch, #Bup#W/#Bdown#W to move, #Bright#W/#Bleft#W to modify, #B?#W for help");
	display_message(0, 1, strlen(keys), TERM_WHITE, keys);
	c_prt((p_ptr->skill_points) ? TERM_L_BLUE : TERM_L_RED,
	      format("Skill points left: %d", p_ptr->skill_points), 2, 0);
	print_desc_aux(s_info[table[sel][0]].desc + s_text, 3, 0);

	for (j = start; j < start + (hgt - 7); j++)
	{
		byte color = TERM_WHITE;
		char deb = ' ', end = ' ';

		if (j >= max) break;

		i = table[j][0];

		if ((s_info[i].value == 0) && (i != SKILL_MISC))
		{
			if (s_info[i].mod == 0) color = TERM_L_DARK;
			else color = TERM_ORANGE;
		}
		else if (s_info[i].value == SKILL_MAX) color = TERM_L_BLUE;
		if (s_info[i].hidden) color = TERM_L_RED;
		if (j == sel)
		{
			color = TERM_L_GREEN;
			deb = '[';
			end = ']';
		}
		if (!has_child(i))
		{
			c_prt(color, format("%c.%c%s", deb, end, s_info[i].name + s_name),
			      j + 7 - start, table[j][1] * 4);
		}
		else if (s_info[i].dev)
		{
			c_prt(color, format("%c-%c%s", deb, end, s_info[i].name + s_name),
			      j + 7 - start, table[j][1] * 4);
		}
		else
		{
			c_prt(color, format("%c+%c%s", deb, end, s_info[i].name + s_name),
			      j + 7 - start, table[j][1] * 4);
		}
		c_prt(color,
		      format("%s%02ld.%03ld [%01d.%03d]",
		             s_info[i].value < 0 ? "-" : " ",
			     ABS(s_info[i].value) / SKILL_STEP,
			     ABS(s_info[i].value) % SKILL_STEP,
			     ABS(s_info[i].mod) / 1000,
			     ABS(s_info[i].mod) % 1000),
		      j + 7 - start, 60);
	}
}

/*
 * Checks various stuff to do when skills change, like new spells, ...
 */
void recalc_skills(bool init)
{
	static int thaum_level = 0;

	/* TODO: This should be a hook in ToME's lua */
	if (init)
	{
		thaum_level = get_skill_scale(SKILL_THAUMATURGY, 100);
	}
	else
	{
		int thaum_gain = 0;

		/* Gain thaum spells */
		while (thaum_level < get_skill_scale(SKILL_THAUMATURGY, 100))
		{
			if (spell_num == MAX_SPELLS) break;
			thaum_level++;
			generate_spell((thaum_level + 1) / 2);
			thaum_gain++;
		}
		if (thaum_gain)
		{
			if (thaum_gain == 1)
				msg_print("You have gained one new thaumaturgy spell.");
			else
				msg_format("You have gained %d new thaumaturgy spells.", thaum_gain);
		}

		process_hooks(HOOK_RECALC_SKILLS, "()");

		/* Update stuffs */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
		                  PU_SANITY | PU_BODY);

		/* Redraw various info */
		p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);
	}
}

/*
 * Recalc the skill value
 */
void recalc_skills_theory(s16b *invest, s32b *base_val, s32b *base_mod, s32b *bonus)
{
	int i, j;

	/* First we assign the normal points */
	for (i = 0; i < max_s_idx; i++)
	{
		/* Calc the base */
		s_info[i].value = base_val[i] + (base_mod[i] * invest[i]) + bonus[i];

		/* It cannot exceed SKILL_MAX */
		if (s_info[i].value > SKILL_MAX) s_info[i].value = SKILL_MAX;
	}

	/* Then we modify related skills */
	for (i = 0; i < max_s_idx; i++)
	{
		for (j = 1; j < max_s_idx; j++)
		{
			/* Ignore self */
			if (j == i) continue;

			/* Exclusive skills */
			if ((s_info[i].action[j] == SKILL_EXCLUSIVE) && invest[i])
			{
				/* Turn it off */
				p_ptr->skill_points += invest[j];
				invest[j] = 0;
				s_info[j].value = 0;
			}

			/* Non-exclusive skills */
			else if (s_info[i].action[j])
			{
				/* Increase / decrease with a % */
				s32b val = s_info[j].value + (invest[i] * s_info[j].mod * s_info[i].action[j] / 100);

				/* It cannot exceed SKILL_MAX */
				if (val > SKILL_MAX) val = SKILL_MAX;

				/* Save the modified value */
				s_info[j].value = val;
			}
		}
	}
}

/*
 * Interreact with skills
 */
void do_cmd_skill()
{
	int sel = 0, start = 0, max;
	char c;
	int table[MAX_SKILLS][2];
	int i;
	int wid, hgt;
	s16b skill_points_save;
	s32b *skill_values_save;
	s32b *skill_mods_save;
	s16b *skill_rates_save;
	s16b *skill_invest;
	s32b *skill_bonus;

	recalc_skills(TRUE);

	/* Save the screen */
	screen_save();

	/* Allocate arrays to save skill values */
	C_MAKE(skill_values_save, MAX_SKILLS, s32b);
	C_MAKE(skill_mods_save, MAX_SKILLS, s32b);
	C_MAKE(skill_rates_save, MAX_SKILLS, s16b);
	C_MAKE(skill_invest, MAX_SKILLS, s16b);
	C_MAKE(skill_bonus, MAX_SKILLS, s32b);

	/* Save skill points */
	skill_points_save = p_ptr->skill_points;

	/* Save skill values */
	for (i = 0; i < max_s_idx; i++)
	{
		skill_type *s_ptr = &s_info[i];

		skill_values_save[i] = s_ptr->value;
		skill_mods_save[i] = s_ptr->mod;
		skill_rates_save[i] = s_ptr->rate;
		skill_invest[i] = 0;
	}

	/* Clear the screen */
	Term_clear();

	/* Initialise the skill list */
	init_table(table, &max, FALSE);

	while (TRUE)
	{
		Term_get_size(&wid, &hgt);

		/* Display list of skills */
		recalc_skills_theory(skill_invest, skill_values_save, skill_mods_save, skill_bonus);
		print_skills(table, max, sel, start);

		/* Wait for user input */
		c = inkey();

		/* Leave the skill screen */
		if (c == ESCAPE) break;

		/* Expand / collapse list of skills */
		else if (c == '\r')
		{
			if (s_info[table[sel][0]].dev) s_info[table[sel][0]].dev = FALSE;
			else s_info[table[sel][0]].dev = TRUE;
			init_table(table, &max, FALSE);
		}

		/* Next page */
		else if (c == 'n')
		{
			sel += (hgt - 7);
			if (sel >= max) sel = max - 1;
		}

		/* Previous page */
		else if (c == 'p')
		{
			sel -= (hgt - 7);
			if (sel < 0) sel = 0;
		}

		/* Select / increase a skill */
		else
		{
			int dir;

			/* Allow use of numpad / arrow keys / roguelike keys */
			dir = get_keymap_dir(c);

			/* Move cursor down */
			if (dir == 2) sel++;

			/* Move cursor up */
			if (dir == 8) sel--;

			/* Miscellaneous skills cannot be increased/decreased as a group */
			if (table[sel][0] == SKILL_MISC) continue;

			/* Increase the current skill */
			if (dir == 6) increase_skill(table[sel][0], skill_invest);

			/* Decrease the current skill */
			if (dir == 4) decrease_skill(table[sel][0], skill_invest);

			/* XXX XXX XXX Wizard mode commands outside of wizard2.c */

			/* Increase the skill */
			if (wizard && (c == '+')) skill_bonus[table[sel][0]] += SKILL_STEP;

			/* Decrease the skill */
			if (wizard && (c == '-')) skill_bonus[table[sel][0]] -= SKILL_STEP;

			/* Contextual help */
			if (c == '?') exec_lua(format("ingame_help('select_context', 'skill', '%s')", s_info[table[sel][0]].name + s_name));
			;

			/* Handle boundaries and scrolling */
			if (sel < 0) sel = max - 1;
			if (sel >= max) sel = 0;
			if (sel < start) start = sel;
			if (sel >= start + (hgt - 7)) start = sel - (hgt - 7) + 1;
		}
	}


	/* Some skill points are spent */
	if (p_ptr->skill_points != skill_points_save)
	{
		/* Flush input as we ask an important and irreversible question */
		flush();

		/* Ask we can commit the change */
		if (msg_box("Save and use these skill values? (y/n)", (int)(hgt / 2), (int)(wid / 2)) != 'y')
		{
			/* User declines -- restore the skill values before exiting */

			/* Restore skill points */
			p_ptr->skill_points = skill_points_save;

			/* Restore skill values */
			for (i = 0; i < max_s_idx; i++)
			{
				skill_type *s_ptr = &s_info[i];

				s_ptr->value = skill_values_save[i];
				s_ptr->mod = skill_mods_save[i];
				s_ptr->rate = skill_rates_save[i];
			}
		}
	}


	/* Free arrays to save skill values */
	C_FREE(skill_values_save, MAX_SKILLS, s32b);
	C_FREE(skill_mods_save, MAX_SKILLS, s32b);
	C_FREE(skill_rates_save, MAX_SKILLS, s16b);
	C_FREE(skill_invest, MAX_SKILLS, s16b);
	C_FREE(skill_bonus, MAX_SKILLS, s32b);

	/* Load the screen */
	screen_load();

	recalc_skills(FALSE);
}



/*
 * List of melee skills
 */
s16b melee_skills[MAX_MELEE] =
{
	SKILL_MASTERY,
	SKILL_HAND,
	SKILL_BEAR,
};
char *melee_names[MAX_MELEE] =
{
	"Weapon combat",
	"Barehanded combat",
	"Bearform combat",
};
static bool melee_bool[MAX_MELEE];
static int melee_num[MAX_MELEE];

s16b get_melee_skill()
{
	int i;

	for (i = 0; i < MAX_MELEE; i++)
	{
		if (p_ptr->melee_style == melee_skills[i])
			return (i);
	}
	return (0);
}

s16b get_melee_skills()
{
	int i, j = 0;

	for (i = 0; i < MAX_MELEE; i++)
	{
		if ((s_info[melee_skills[i]].value > 0) && (!s_info[melee_skills[i]].hidden))
		{
			melee_bool[i] = TRUE;
			j++;
		}
		else
			melee_bool[i] = FALSE;
	}

	return (j);
}

static void choose_melee()
{
	int i, j, z = 0;
	int force_drop = FALSE, style_unchanged = FALSE;

	character_icky = TRUE;
	Term_save();
	Term_clear();

	j = get_melee_skills();
	prt("Choose a melee style:", 0, 0);
	for (i = 0; i < MAX_MELEE; i++)
	{
		if (melee_bool[i])
		{
			prt(format("%c) %s", I2A(z), melee_names[i]), z + 1, 0);
			melee_num[z] = i;
			z++;
		}
	}

	while (TRUE)
	{
		char c = inkey();

		if (c == ESCAPE) break;
		if (A2I(c) < 0) continue;
		if (A2I(c) >= j) continue;

		for (i = 0, z = 0; z < A2I(c); i++)
			if (melee_bool[i]) z++;

		if (p_ptr->melee_style == melee_skills[melee_num[z]])
		{
			style_unchanged = TRUE;
			break;
		}

		for (i = INVEN_WIELD; p_ptr->body_parts[i - INVEN_WIELD] == INVEN_WIELD; i++)
		{
			if (p_ptr->inventory[i].k_idx)
			{
				if (cursed_p(&p_ptr->inventory[i]))
				{
					char name[80];
					object_desc(name, &p_ptr->inventory[i], 0, 0);
					msg_format("Hmmm, your %s seems to be cursed.", name);
					break;
				}
				else if (INVEN_PACK == inven_takeoff(i, 255, force_drop))
				{
					force_drop = TRUE;
				}
			}
		}
		p_ptr->melee_style = melee_skills[melee_num[z]];
		energy_use = 100;
		break;
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoint */
	p_ptr->update |= (PU_HP);

	/* Redraw monster hitpoint */
	p_ptr->redraw |= (PR_MH);

	Term_load();
	character_icky = FALSE;

	if (style_unchanged)
	{
		msg_format("You are already using %s.", melee_names[melee_num[z]]);
	}
}

void select_default_melee()
{
	int i;

	get_melee_skills();
	p_ptr->melee_style = SKILL_MASTERY;
	for (i = 0; i < MAX_MELEE; i++)
	{
		if (melee_bool[i])
		{
			p_ptr->melee_style = melee_skills[i];
			break;
		}
	}
}

/*
 * Print a batch of skills.
 */
static void print_skill_batch(int *p, cptr *p_desc, int start, int max, bool mode)
{
	char buff[80];
	int i = start, j = 0;

	if (mode) prt(format("         %-31s", "Name"), 1, 20);

	for (i = start; i < (start + 20); i++)
	{
		if (i >= max) break;

		if (p[i] > 0)
			sprintf(buff, "  %c - %d) %-30s", I2A(j), p[i], p_desc[i]);
		else
			sprintf(buff, "  %c - %d) %-30s", I2A(j), p[i], "Change melee style");

		if (mode) prt(buff, 2 + j, 20);
		j++;
	}
	if (mode) prt("", 2 + j, 20);
	prt(format("Select a skill (a-%c), @ to select by name, +/- to scroll:", I2A(j - 1)), 0, 0);
}

int do_cmd_activate_skill_aux()
{
	char which;
	int max = 0, i, start = 0;
	int ret;
	bool mode = FALSE;
	int *p;
	cptr *p_desc;

	C_MAKE(p, max_s_idx + max_ab_idx, int);
	C_MAKE(p_desc, max_s_idx + max_ab_idx, cptr);

	/* Count the max */

	/* More than 1 melee skill ? */
	if (get_melee_skills() > 1)
	{
		p_desc[max] = "Change melee mode";
		p[max++] = 0;
	}

	for (i = 1; i < max_s_idx; i++)
	{
		if (s_info[i].action_mkey && s_info[i].value && ((!s_info[i].hidden) || (i == SKILL_LEARN)))
		{
			int j;
			bool next = FALSE;

			/* Already got it ? */
			for (j = 0; j < max; j++)
			{
				if (s_info[i].action_mkey == p[j])
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p_desc[max] = s_text + s_info[i].action_desc;
			p[max++] = s_info[i].action_mkey;
		}
	}

	for (i = 0; i < max_ab_idx; i++)
	{
		if (ab_info[i].action_mkey && ab_info[i].acquired)
		{
			int j;
			bool next = FALSE;

			/* Already got it ? */
			for (j = 0; j < max; j++)
			{
				if (ab_info[i].action_mkey == p[j])
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p_desc[max] = ab_text + ab_info[i].action_desc;
			p[max++] = ab_info[i].action_mkey;
		}
	}

	if (!max)
	{
		msg_print("You don't have any activable skills or abilities.");
		return -1;
	}

	character_icky = TRUE;
	Term_save();

	while (1)
	{
		print_skill_batch(p, p_desc, start, max, mode);
		which = inkey();

		if (which == ESCAPE)
		{
			ret = -1;
			break;
		}
		else if (which == '*' || which == '?' || which == ' ')
		{
			mode = (mode) ? FALSE : TRUE;
			Term_load();
			character_icky = FALSE;
		}
		else if (which == '+')
		{
			start += 20;
			if (start >= max) start -= 20;
			Term_load();
			character_icky = FALSE;
		}
		else if (which == '-')
		{
			start -= 20;
			if (start < 0) start += 20;
			Term_load();
			character_icky = FALSE;
		}
		else if (which == '@')
		{
			char buf[80];

			strcpy(buf, "Cast a spell");
			if (!get_string("Skill action? ", buf, 79))
				return FALSE;

			/* Find the skill it is related to */
			for (i = 0; i < max; i++)
			{
				if (!strcmp(buf, p_desc[i]))
					break;
			}
			if ((i < max))
			{
				ret = p[i];
				break;
			}

		}
		else
		{
			which = tolower(which);
			if (start + A2I(which) >= max)
			{
				bell();
				continue;
			}
			if (start + A2I(which) < 0)
			{
				bell();
				continue;
			}

			ret = p[start + A2I(which)];
			break;
		}
	}
	Term_load();
	character_icky = FALSE;

	C_FREE(p, max_s_idx + max_ab_idx, int);
	C_FREE(p_desc, max_s_idx + max_ab_idx, cptr);

	return ret;
}

/* Ask & execute a skill */
void do_cmd_activate_skill()
{
	int x_idx;
	bool push = TRUE;

	/* Get the skill, if available */
	if (repeat_pull(&x_idx))
	{
		push = FALSE;
	}
	else if (!command_arg) x_idx = do_cmd_activate_skill_aux();
	else
	{
		int i, j;

		x_idx = command_arg;

		/* Check validity */
		for (i = 1; i < max_s_idx; i++)
		{
			if (s_info[i].value && (s_info[i].action_mkey == x_idx))
				break;
		}
		for (j = 0; j < max_ab_idx; j++)
		{
			if (ab_info[j].acquired && (ab_info[j].action_mkey == x_idx))
				break;
		}

		if ((j == max_ab_idx) && (i == max_s_idx))
		{
			msg_print("Uh?");
			return;
		}
	}

	if (x_idx == -1) return;

	if (push) repeat_push(x_idx);

	if (!x_idx)
	{
		choose_melee();
		return;
	}

	/* Break goi/manashield */
	if (p_ptr->invuln)
	{
		set_invuln(0);
	}
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(0);
	}

	switch (x_idx)
	{
	case MKEY_ANTIMAGIC:
		do_cmd_unbeliever();
		break;
	case MKEY_MINDCRAFT:
		do_cmd_mindcraft();
		break;
	case MKEY_ALCHEMY:
		do_cmd_alchemist();
		break;
	case MKEY_MIMIC:
		do_cmd_mimic();
		break;
	case MKEY_POWER_MAGE:
		do_cmd_powermage();
		break;
	case MKEY_RUNE:
		do_cmd_runecrafter();
		break;
	case MKEY_FORGING:
		do_cmd_archer();
		break;
	case MKEY_INCARNATION:
		do_cmd_possessor();
		break;
	case MKEY_TELEKINESIS:
		do_cmd_portable_hole();
		break;
	case MKEY_BLADE:
		do_cmd_blade();
		break;
	case MKEY_SUMMON:
		do_cmd_summoner();
		break;
	case MKEY_NECRO:
		do_cmd_necromancer();
		break;
	case MKEY_SYMBIOTIC:
		do_cmd_symbiotic();
		break;
	case MKEY_TRAP:
		do_cmd_set_trap();
		break;
	case MKEY_STEAL:
		do_cmd_steal();
		break;
	case MKEY_DODGE:
		use_ability_blade();
		break;
	case MKEY_SCHOOL:
		cast_school_spell();
		break;
	case MKEY_COPY:
		do_cmd_copy_spell();
		break;
	case MKEY_BOULDER:
		do_cmd_create_boulder();
		break;
	case MKEY_COMPANION:
		if (get_skill(SKILL_LORE) >= 12)
			do_cmd_companion();
		else
			msg_print("You need a skill level of at least 12.");
		break;
	case MKEY_PIERCING:
		do_cmd_set_piercing();
		break;
	default:
		process_hooks(HOOK_MKEY, "(d)", x_idx);
		break;
	}
}


/* Which magic forbids non FA gloves */
bool forbid_gloves()
{
	if (get_skill(SKILL_SORCERY)) return (TRUE);
	if (get_skill(SKILL_MANA)) return (TRUE);
	if (get_skill(SKILL_FIRE)) return (TRUE);
	if (get_skill(SKILL_AIR)) return (TRUE);
	if (get_skill(SKILL_WATER)) return (TRUE);
	if (get_skill(SKILL_EARTH)) return (TRUE);
	if (get_skill(SKILL_THAUMATURGY)) return (TRUE);
	return (FALSE);
}

/* Which gods forbid edged weapons */
bool forbid_non_blessed()
{
	GOD(GOD_ERU) return (TRUE);
	return (FALSE);
}




/*
 * The autoskiller gets fed with the desired skill values at level 50 and determines
 * where to invest each levels
 */

/* Checks if a given autoskill chart is realisable */
int validate_autoskiller(s32b *ideal)
{
	int i;
	s32b count = 0;

	for (i = 0; i < max_s_idx; i++)
	{
		s32b z, c;

		skill_type *s_ptr = &s_info[i];

		if (!ideal[i]) continue;

		/* How much */
		z = (ideal[i] * SKILL_STEP) - s_ptr->value;

		/* How many points will we need to get there ? */
		if (s_ptr->mod)
			c = z / s_ptr->mod;
		else
			c = 99999;
		count += c;
	}
	/* DGDGDGDG: I dont work, fix me */
	/*        return (SKILL_NB_BASE * 49) - count;*/
	return 0;
}

void autoskiller_level(s32b *ideal)
{
#if 0
	/* DGDGDGDG */
	int chart[196];
	int final[196];
	int ideal[3] = {50, 30, 25};
	int real[3] = {0, 0, 0};
	int mod[3] = {450, 1000, 1100};
	float pl[3], left[3] = {0, 0, 0}, finalization = 1;
	int c[3], z;
	int i, max = 0, ok = 3, a;

	for (i = 0; i < 3; i++)
	{
		c[i] = (ideal[i] * 1000) / mod[i];
		printf("point need c[%d] = %d\n", i, c[i]);
		max += c[i];

		pl[i] = (float)c[i] / 50.0;
		printf("pl[%d] = %f\n", i, pl[i]);
	}
	printf("skill balance %d\n", 196 - max);

	for (i = 0; i < 196; i++)
	{
		chart[i] = -1;
	}

	a = 0;
	while ((a < 196) && (ok))
	{
		int z = 0;

		for (z = 0; z < 3; z++)
		{
			int tmp, ii;

			if (real[z] == c[z])
				continue;
			left[z] += pl[z];
			tmp = left[z];
			left[z] -= tmp;
			printf("skill %02d: left %f tmp %d\n", z, left[z], tmp);

			while (tmp >= 1)
			{
				chart[a++] = z;
				real[z]++;
				tmp--;
				if (real[z] == c[z])
				{
					ok--;
					break;
				}
				if (a == 196)
				{
					ok = 0;
					break;
				}
			}
			if (!ok) break;
		}
	}
	printf("ok %d, a %d interval %f\n", ok, a, (float)a / 196);
	real[0] = real[1] = real[2] = 0;
	for (i = 0; i < 196; i++)
	{
		real[chart[i]]++;
	}
	z = 0;
	i = 0;
	while (z < a)
	{
		if (finalization > 1)
		{
			final[i] = chart[z++];
			finalization--;
		}
		else
			final[i] = -1;
		finalization += ((float)a / 196);
		i++;
	}
	for (z = 0; z < 3; z++)
	{
		printf("skill %d real %d ideal %d\n", z, real[z], c[z]);
	}

	for (i = 2; i <= 50; i++)
	{
		printf("level %02d, skills %02d %02d %02d %02d\n", i, final[i * 4], final[i * 4 + 1], final[i * 4 + 2], final[i * 4 + 3]);
	}
#endif
}

/*
 * Gets the base value of a skill, given a race/class/...
 */
void compute_skills(s32b *v, s32b *m, int i)
{
	s32b value, mod;

	/***** general skills *****/

	/* If the skill correspond to the magic school lets pump them a bit */
	value = gen_skill_base[i];
	mod = gen_skill_mod[i];

	*v = modify_aux(*v,
	                value, gen_skill_basem[i]);
	*m = modify_aux(*m,
	                mod, gen_skill_modm[i]);

	/***** race skills *****/

	value = rp_ptr->skill_base[i];
	mod = rp_ptr->skill_mod[i];

	*v = modify_aux(*v,
	                value, rp_ptr->skill_basem[i]);
	*m = modify_aux(*m,
	                mod, rp_ptr->skill_modm[i]);

	/***** race mod skills *****/

	value = rmp_ptr->skill_base[i];
	mod = rmp_ptr->skill_mod[i];

	*v = modify_aux(*v,
	                value, rmp_ptr->skill_basem[i]);
	*m = modify_aux(*m,
	                mod, rmp_ptr->skill_modm[i]);

	/***** class skills *****/

	value = cp_ptr->skill_base[i];
	mod = cp_ptr->skill_mod[i];

	*v = modify_aux(*v,
	                value, cp_ptr->skill_basem[i]);
	*m = modify_aux(*m,
	                mod, cp_ptr->skill_modm[i]);

	/***** class spec skills *****/

	value = spp_ptr->skill_base[i];
	mod = spp_ptr->skill_mod[i];

	*v = modify_aux(*v,
	                value, spp_ptr->skill_basem[i]);
	*m = modify_aux(*m,
	                mod, spp_ptr->skill_modm[i]);
}

/*
 * Initialize a skill with given values
 */
void init_skill(s32b value, s32b mod, int i)
{
	s_info[i].value = value;
	s_info[i].mod = mod;

	if (s_info[i].flags1 & SKF1_HIDDEN)
		s_info[i].hidden = TRUE;
	else
		s_info[i].hidden = FALSE;
}

void do_get_new_skill()
{
	char *items[4];
	int skl[4];
	s32b val[4], mod[4];
	bool used[MAX_SKILLS];
	int available_skills[MAX_SKILLS];
	int max = 0, max_a = 0, res, i;

	/* Check if some skills didn't influence other stuff */
	recalc_skills(TRUE);

	/* Grab the ones we can gain */
	max = 0;
	for (i = 0; i < max_s_idx; i++)
	{
		if (s_info[i].flags1 & SKF1_RANDOM_GAIN)
			available_skills[max++] = i;
	}
	available_skills[max++] = -1;

	/* Init */
	for (max = 0; max < MAX_SKILLS; max++)
	{
		used[max] = FALSE;
	}

	/* Count the number of available skills */
	while (available_skills[max_a] != -1) max_a++;

	/* Get 4 skills */
	for (max = 0; max < 4; max++)
	{
		int i;
		skill_type *s_ptr;

		/* Get an non used skill */
		do
		{
			i = rand_int(max_a);

			/* Does it pass the check? */
			if (!magik(s_info[available_skills[i]].random_gain_chance))
				continue;
		}
		while (used[available_skills[i]]);

		s_ptr = &s_info[available_skills[i]];
		used[available_skills[i]] = TRUE;

		if (s_ptr->mod)
		{
			if (s_ptr->mod < 300)
			{
				val[max] = 1000;
				mod[max] = 300 - s_ptr->mod;
			}
			else if (s_ptr->mod < 500)
			{
				val[max] = s_ptr->mod * 1;
				mod[max] = 100;
				if (mod[max] + s_ptr->mod > 500)
					mod[max] = 500 - s_ptr->mod;
			}
			else
			{
				val[max] = s_ptr->mod * 3;
				mod[max] = 0;
			}
		}
		else
		{
			mod[max] = 300;
			val[max] = 1000;
		}
		if (s_ptr->value + val[max] > SKILL_MAX) val[max] = SKILL_MAX - s_ptr->value;
		skl[max] = available_skills[i];
		items[max] = (char *)string_make(format("%-40s: +%02ld.%03ld value, +%01d.%03d modifier", s_ptr->name + s_name, val[max] / SKILL_STEP, val[max] % SKILL_STEP, mod[max] / SKILL_STEP, mod[max] % SKILL_STEP));
	}

	while (TRUE)
	{
		res = ask_menu("Choose a skill to learn(a-d to choose, ESC to cancel)?", (char **)items, 4);

		/* Ok ? lets learn ! */
		if (res > -1)
		{
			skill_type *s_ptr;
			bool oppose = FALSE;
			int oppose_skill = -1;

			/* Check we don't oppose an existing skill */
			for (i = 0; i < max_s_idx; i++)
			{
				if ((s_info[i].action[skl[res]] == SKILL_EXCLUSIVE) &&
				                (s_info[i].value != 0))
				{
					oppose = TRUE;
					oppose_skill = i;
					break;
				}
			}

			/* Ok we oppose, so be sure */
			if (oppose)
			{
				cptr msg;

				/*
				 * Because this is SO critical a question, we must flush
				 * input to prevent killing character off -- pelpel
				 */
				flush();

				/* Prepare prompt */
				msg = format("This skill is mutually exclusive with "
				             "at least %s, continue?",
				             s_info[oppose_skill].name + s_name);

				/* The player rejected the choice */
				if (!get_check(msg)) continue;
			}

			s_ptr = &s_info[skl[res]];
			s_ptr->value += val[res];
			s_ptr->mod += mod[res];
			if (mod[res])
			{
				msg_format("You can now learn the %s skill.",
				           s_ptr->name + s_name);
			}
			else
			{
				msg_format("Your knowledge of the %s skill increases.",
				           s_ptr->name + s_name);
			}
			break;
		}
	}

	/* Free them ! */
	for (max = 0; max < 4; max++)
	{
		string_free(items[max]);
	}

	/* Check if some skills didn't influence other stuff */
	recalc_skills(FALSE);
}




/**************************************** ABILITIES *****************************************/

/*
 * Given the name of an ability, returns ability index or -1 if no
 * such ability is found
 */
s16b find_ability(cptr name)
{
	u16b i;

	/* Scan ability list */
	for (i = 0; i < max_ab_idx; i++)
	{
		/* The name matches */
		if (streq(ab_info[i].name + ab_name, name)) return (i);
	}

	/* No match found */
	return ( -1);
}

/*
 * Do the player have the ability
 */
bool has_ability(int ab)
{
	return ab_info[ab].acquired;
}

/* Do we meet the requirements */
bool can_learn_ability(int ab)
{
	ability_type *ab_ptr = &ab_info[ab];
	int i;

	if (ab_ptr->acquired)
		return FALSE;

	if (p_ptr->skill_points < ab_info[ab].cost)
		return FALSE;

	for (i = 0; i < 10; i++)
	{
		/* Must have skill level */
		if (ab_ptr->skills[i] > -1)
		{
			if (get_skill(ab_ptr->skills[i]) < ab_ptr->skill_levels[i])
				return FALSE;
		}

		/* Must have ability */
		if (ab_ptr->need_abilities[i] > -1)
		{
			if (!ab_info[ab_ptr->need_abilities[i]].acquired)
				return FALSE;
		}

		/* Must not have ability */
		if (ab_ptr->forbid_abilities[i] > -1)
		{
			if (ab_info[ab_ptr->forbid_abilities[i]].acquired)
				return FALSE;
		}
	}

	for (i = 0; i < 6; i++)
	{
		/* Must have stat */
		if (ab_ptr->stat[i] > -1)
		{
			if (p_ptr->stat_ind[i] < ab_ptr->stat[i] - 3)
				return FALSE;
		}
	}

	/* Do the script allow us? */
	if (process_hooks(HOOK_LEARN_ABILITY, "(d)", ab))
		return FALSE;

	return TRUE;
}

/* Learn an ability */
void gain_ability(int ab)
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);

	if (!can_learn_ability(ab))
	{
		msg_box("You cannot learn this ability.", (int)(hgt / 2), (int)(wid / 2));
		return;
	}

	/* Flush input as we ask an important and irreversible question */
	flush();

	/* Ask we can commit the change */
	if (msg_box("Learn this ability(this is permanent)? (y/n)", (int)(hgt / 2), (int)(wid / 2)) != 'y')
	{
		return;
	}

	ab_info[ab].acquired = TRUE;
	p_ptr->skill_points -= ab_info[ab].cost;
}

/* helper function to generate a sorted table */
static void add_sorted_ability(int *table, int *max, int ab)
{
	int i;

	for (i = 0; i < *max; i++)
	{
		if (strcmp(ab_name + ab_info[ab].name, ab_name + ab_info[table[i]].name) < 0)
		{
			int z;

			/* Move all indexes up */
			for (z = *max; z > i; z--)
			{
				table[z] = table[z - 1];
			}
			break;
		}
	}
	table[i] = ab;

	(*max)++;
}

/*
 * Print the abilities list
 */
void dump_abilities(FILE *fff)
{
	int i, j;
	int *table;
	int max = 0;

	C_MAKE(table, max_ab_idx, int);

	/* Initialise the abilities list */
	for (i = 0; i < max_ab_idx; i++)
	{
		if (ab_info[i].name && has_ability(i))
			add_sorted_ability(table, &max, i);
	}

	if (max)
	{
		fprintf(fff, "\nAbilities");

		for (j = 0; j < max; j++)
		{
			i = table[j];

			fprintf(fff, "\n * %s", ab_info[i].name + ab_name);
		}

		fprintf(fff, "\n");
	}
}

/*
 * Draw the abilities list
 */
void print_abilities(int table[], int max, int sel, int start)
{
	int i, j;
	int wid, hgt;
	cptr keys;

	Term_clear();
	Term_get_size(&wid, &hgt);

	c_prt(TERM_WHITE, format("%s Abilities Screen", game_module), 0, 28);
	keys = format("#Bup#W/#Bdown#W to move, #Bright#W to buy, #B?#W for help");
	display_message(0, 1, strlen(keys), TERM_WHITE, keys);
	c_prt((p_ptr->skill_points) ? TERM_L_BLUE : TERM_L_RED,
	      format("Skill points left: %d", p_ptr->skill_points), 2, 0);

	print_desc_aux(ab_info[table[sel]].desc + ab_text, 3, 0);

	for (j = start; j < start + (hgt - 7); j++)
	{
		byte color = TERM_WHITE;
		char deb = ' ', end = ' ';

		if (j >= max) break;

		i = table[j];

		if (ab_info[i].acquired)
			color = TERM_L_BLUE;
		else if (can_learn_ability(i))
			color = TERM_WHITE;
		else
			color = TERM_L_DARK;


		if (j == sel)
		{
			color = TERM_L_GREEN;
			deb = '[';
			end = ']';
		}

		c_prt(color, format("%c.%c%s", deb, end, ab_info[i].name + ab_name),
		      j + 7 - start, 0);

		if (!ab_info[i].acquired)
		{
			c_prt(color, format("%d", ab_info[i].cost), j + 7 - start, 60);
		}
		else
		{
			c_prt(color, "Known", j + 7 - start, 60);
		}
	}
}

/*
 * Interreact with abilitiess
 */
void do_cmd_ability()
{
	int sel = 0, start = 0, max = 0;
	char c;
	int *table;
	int i;
	int wid, hgt;

	C_MAKE(table, max_ab_idx, int);

	/* Save the screen */
	screen_save();

	/* Clear the screen */
	Term_clear();

	/* Initialise the abilities list */
	for (i = 0; i < max_ab_idx; i++)
	{
#if 0 /* Only show learnable */
		if (ab_info[i].name && ((can_learn_ability(i) || has_ability(i)) || wizard))
#else /* Show all */
if (ab_info[i].name)
#endif
			add_sorted_ability(table, &max, i);
	}

	while (TRUE)
	{
		Term_get_size(&wid, &hgt);

		/* Display list of skills */
		print_abilities(table, max, sel, start);

		/* Wait for user input */
		c = inkey();

		/* Leave the skill screen */
		if (c == ESCAPE) break;

		/* Next page */
		else if (c == 'n')
		{
			sel += (hgt - 7);
			if (sel >= max) sel = max - 1;
		}

		/* Previous page */
		else if (c == 'p')
		{
			sel -= (hgt - 7);
			if (sel < 0) sel = 0;
		}

		/* Select / increase a skill */
		else
		{
			int dir;

			/* Allow use of numpad / arrow keys / roguelike keys */
			dir = get_keymap_dir(c);

			/* Move cursor down */
			if (dir == 2) sel++;

			/* Move cursor up */
			if (dir == 8) sel--;

			/* gain ability */
			if (dir == 6) gain_ability(table[sel]);

			/* XXX XXX XXX Wizard mode commands outside of wizard2.c */

			if (wizard && (c == '+')) ab_info[table[sel]].acquired = TRUE;
			if (wizard && (c == '-')) ab_info[table[sel]].acquired = FALSE;

			/* Contextual help */
			if (c == '?') exec_lua(format("ingame_help('select_context', 'ability', '%s')", ab_info[table[sel]].name + ab_name));
			;

			/* Handle boundaries and scrolling */
			if (sel < 0) sel = max - 1;
			if (sel >= max) sel = 0;
			if (sel < start) start = sel;
			if (sel >= start + (hgt - 7)) start = sel - (hgt - 7) + 1;
		}
	}

	/* Load the screen */
	screen_load();

	C_FREE(table, max_ab_idx, int);

	/* Update stuffs */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
	                  PU_SANITY | PU_BODY);

	/* Redraw various info */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);
}

/*
 * Apply abilities to be granted this level
 */
void apply_level_abilities(int level)
{
	int i;

	for (i = 0; i < 10; i++)
	{
		if (cp_ptr->abilities[i].level == level)
		{
			if ((level > 1) && (!ab_info[cp_ptr->abilities[i].ability].acquired))
				cmsg_format(TERM_L_GREEN, "You have learned the ability '%s'.", ab_name + ab_info[cp_ptr->abilities[i].ability].name);
			ab_info[cp_ptr->abilities[i].ability].acquired = TRUE;
		}
		if (spp_ptr->abilities[i].level == level)
		{
			if ((level > 1) && (!ab_info[spp_ptr->abilities[i].ability].acquired))
				cmsg_format(TERM_L_GREEN, "You have learned the ability '%s'.", ab_name + ab_info[spp_ptr->abilities[i].ability].name);
			ab_info[spp_ptr->abilities[i].ability].acquired = TRUE;
		}
		if (rp_ptr->abilities[i].level == level)
		{
			if ((level > 1) && (!ab_info[rp_ptr->abilities[i].ability].acquired))
				cmsg_format(TERM_L_GREEN, "You have learned the ability '%s'.", ab_name + ab_info[rp_ptr->abilities[i].ability].name);
			ab_info[rp_ptr->abilities[i].ability].acquired = TRUE;
		}
		if (rmp_ptr->abilities[i].level == level)
		{
			if ((level > 1) && (!ab_info[rmp_ptr->abilities[i].ability].acquired))
				cmsg_format(TERM_L_GREEN, "You have learned the ability '%s'.", ab_name + ab_info[rmp_ptr->abilities[i].ability].name);
			ab_info[rmp_ptr->abilities[i].ability].acquired = TRUE;
		}
	}
}
