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

#include "hooks.h"
#include "util.hpp"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
#include <tuple>

using boost::algorithm::iequals;

/*
 * Advance the skill point of the skill specified by i and
 * modify related skills
 */
static void increase_skill(int i, s16b *invest)
{
	s32b max_skill_overage;

	/* No skill points to be allocated */
	if (!p_ptr->skill_points) return;

	/* The skill cannot be increased */
	if (!s_info[i].mod) return;

	/* The skill is already maxed */
	if (s_info[i].value >= SKILL_MAX) return;

	/* Cannot allocate more than player level + max_skill_overage levels */
	max_skill_overage = modules[game_module_idx].skills.max_skill_overage;
	if (((s_info[i].value + s_info[i].mod) / SKILL_STEP) >= (p_ptr->lev + max_skill_overage + 1))
	{
		int hgt, wid;
		char buf[256];

		sprintf(buf,
			"Cannot raise a skill value above " FMTs32b " + player level.",
			max_skill_overage);

		Term_get_size(&wid, &hgt);
		msg_box(buf, hgt / 2, wid / 2);
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
		if (s_info[i].name > 0)
		{
			if (streq(s_info[i].name + s_name, name)) return (i);
		}
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
		if (s_info[i].name > 0) {
			if (iequals(s_info[i].name + s_name, name)) return (i);
		}
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

static bool_ is_known(int s_idx)
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
                    bool_ full)
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


void init_table(int table[MAX_SKILLS][2], int *max, bool_ full)
{
	*max = 0;
	init_table_aux(table, max, -1, 0, full);
}


bool_ has_child(int sel)
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

		fprintf(fff, "%-49s%s%06.3f [%05.3f]",
		        buf, s_info[i].value < 0 ? "-" : " ",
			static_cast<double>(ABS(s_info[i].value)) / SKILL_STEP,
		        static_cast<double>(s_info[i].mod) / 1000);
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
void recalc_skills(bool_ init)
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

		/* Antimagic means you don't believe in gods. */
		if ((p_ptr->pgod != GOD_NONE) &&
		    (s_info[SKILL_ANTIMAGIC].value > 0))
		{
			msg_print("You no longer believe.");
			abandon_god(GOD_ALL);
		}

		process_hooks(HOOK_RECALC_SKILLS, "()");
		process_hooks_new(HOOK_RECALC_SKILLS, NULL, NULL);

		/* Update stuffs */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
		                  PU_SANITY | PU_BODY);

		/* Redraw various info */
		p_ptr->redraw |= (PR_WIPE | PR_FRAME | PR_MAP);
	}
}

/*
 * Recalc the skill value
 */
static void recalc_skills_theory(s16b *invest, s32b *base_val, s32b *base_mod, s32b *bonus)
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

	recalc_skills(TRUE);

	/* Save the screen */
	screen_save();

	/* Allocate arrays to save skill values */
	std::unique_ptr<s32b[]> skill_values_save(new s32b[MAX_SKILLS]);
	std::unique_ptr<s32b[]> skill_mods_save(new s32b[MAX_SKILLS]);
	std::unique_ptr<s16b[]> skill_rates_save(new s16b[MAX_SKILLS]);
	std::unique_ptr<s16b[]> skill_invest(new s16b[MAX_SKILLS]);
	std::unique_ptr<s32b[]> skill_bonus(new s32b[MAX_SKILLS]);

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
		skill_bonus[i] = 0;
	}

	/* Clear the screen */
	Term_clear();

	/* Initialise the skill list */
	init_table(table, &max, FALSE);

	while (TRUE)
	{
		Term_get_size(&wid, &hgt);

		/* Display list of skills */
		recalc_skills_theory(skill_invest.get(), skill_values_save.get(), skill_mods_save.get(), skill_bonus.get());
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
			if (dir == 6) increase_skill(table[sel][0], skill_invest.get());

			/* Decrease the current skill */
			if (dir == 4) decrease_skill(table[sel][0], skill_invest.get());

			/* XXX XXX XXX Wizard mode commands outside of wizard2.c */

			/* Increase the skill */
			if (wizard && (c == '+')) skill_bonus[table[sel][0]] += SKILL_STEP;

			/* Decrease the skill */
			if (wizard && (c == '-')) skill_bonus[table[sel][0]] -= SKILL_STEP;

			/* Contextual help */
			if (c == '?')
			{
				help_skill(s_info[table[sel][0]].name + s_name);
			}

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
		if (msg_box("Save and use these skill values? (y/n)", hgt / 2, wid / 2) != 'y')
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
const char *melee_names[MAX_MELEE] =
{
	"Weapon combat",
	"Barehanded combat",
	"Bearform combat",
};
static bool_ melee_bool[MAX_MELEE];
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
	p_ptr->redraw |= (PR_FRAME);

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
static void print_skill_batch(const std::vector<std::tuple<cptr, int>> &p, int start)
{
	char buff[80];
	int i = start, j = 0;

	prt(format("         %-31s", "Name"), 1, 20);

	for (i = start; i < (start + 20); i++)
	{
		if (i >= p.size())
		{
			break;
		}

		sprintf(buff, "  %c - %d) %-30s", I2A(j),
			std::get<1>(p[i]),
			std::get<0>(p[i]));

		prt(buff, 2 + j, 20);
		j++;
	}
	prt("", 2 + j, 20);
	prt(format("Select a skill (a-%c), @ to select by name, +/- to scroll:", I2A(j - 1)), 0, 0);
}

int do_cmd_activate_skill_aux()
{
	char which;
	int i, start = 0;
	int ret;

	std::vector<std::tuple<cptr,int>> p;

	/* More than 1 melee skill ? */
	if (get_melee_skills() > 1)
	{
		p.push_back(std::make_tuple("Change melee mode", 0));
	}

	for (i = 1; i < max_s_idx; i++)
	{
		if (s_info[i].action_mkey && s_info[i].value && ((!s_info[i].hidden) || (i == SKILL_LEARN)))
		{
			int j;
			bool_ next = FALSE;

			/* Already got it ? */
			for (j = 0; j < p.size(); j++)
			{
				if (s_info[i].action_mkey == std::get<1>(p[j]))
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p.push_back(std::make_tuple(s_text + s_info[i].action_desc,
						    s_info[i].action_mkey));
		}
	}

	for (i = 0; i < max_ab_idx; i++)
	{
		if (ab_info[i].action_mkey && ab_info[i].acquired)
		{
			int j;
			bool_ next = FALSE;

			/* Already got it ? */
			for (j = 0; j < p.size(); j++)
			{
				if (ab_info[i].action_mkey == std::get<1>(p[j]))
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p.push_back(std::make_tuple(ab_text + ab_info[i].action_desc,
						    ab_info[i].action_mkey));
		}
	}

	if (p.empty())
	{
		msg_print("You don't have any activable skills or abilities.");
		return -1;
	}

	character_icky = TRUE;
	Term_save();

	while (1)
	{
		print_skill_batch(p, start);
		which = inkey();

		if (which == ESCAPE)
		{
			ret = -1;
			break;
		}
		else if (which == '+')
		{
			start += 20;
			if (start >= p.size()) start -= 20;
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
			for (i = 0; i < p.size(); i++)
			{
				if (!strcmp(buf, std::get<0>(p[i])))
					break;
			}
			if ((i < p.size()))
			{
				ret = std::get<1>(p[i]);
				break;
			}

		}
		else
		{
			which = tolower(which);
			if (start + A2I(which) >= p.size())
			{
				bell();
				continue;
			}
			if (start + A2I(which) < 0)
			{
				bell();
				continue;
			}

			ret = std::get<1>(p[start + A2I(which)]);
			break;
		}
	}
	Term_load();
	character_icky = FALSE;

	return ret;
}

/* Ask & execute a skill */
void do_cmd_activate_skill()
{
	int x_idx;
	bool_ push = TRUE;

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
	case MKEY_DEATH_TOUCH:
	{
		if (p_ptr->csp > 40)
		{
			increase_mana(-40);
			set_project(randint(30) + 10,
				    GF_INSTA_DEATH,
				    1,
				    0,
				    PROJECT_STOP | PROJECT_KILL);
			energy_use = 100;
		}
		else
		{
			msg_print("You need at least 40 mana.");
		}
		break;
	}
	case MKEY_GEOMANCY:
	{
		s32b s = -1;
		object_type *o_ptr = NULL;

		/* No magic */
		if (p_ptr->antimagic > 0)
		{
			msg_print("Your anti-magic field disrupts any magic attempts.");
			break;
		}

		o_ptr = get_object(INVEN_WIELD);
		if ((o_ptr->k_idx <= 0) ||
		    (o_ptr->tval != TV_MSTAFF))
		{
			msg_print("You must wield a magestaff to use Geomancy.");
			break;
		}

		s = get_school_spell("cast", BOOK_GEOMANCY);
		if (s >= 0)
		{
			lua_cast_school_spell(s, FALSE);
		}

		break;
	}
	case MKEY_REACH_ATTACK:
	{
		object_type *o_ptr = NULL;
		int dir, dy, dx, targetx, targety, max_blows, flags;

		o_ptr = get_object(INVEN_WIELD);
		if (o_ptr->tval == TV_POLEARM)
		{
			msg_print("You will need a long polearm for this!");
			return;
		}

		if (!get_rep_dir(&dir))
		{
			return;
		}

		dy = ddy[dir];
		dx = ddx[dir];
		dy = dy * 2;
		dx = dx * 2;
		targety = p_ptr->py + dy;
		targetx = p_ptr->px + dx;

		max_blows = get_skill_scale(SKILL_POLEARM, p_ptr->num_blow / 2);
		if (max_blows == 0)
		{
			max_blows = 1;
		}

		energy_use = energy_use + 200;

		flags = PROJECT_BEAM | PROJECT_KILL;
		if (get_skill(SKILL_POLEARM) < 40)
		{
			flags |= PROJECT_STOP;
		}

		project(0, 0, targety, targetx,
			max_blows, GF_ATTACK, flags);

		break;
	}
	default:
		break;
	}
}


/* Which magic forbids non FA gloves */
bool_ forbid_gloves()
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
bool_ forbid_non_blessed()
{
	GOD(GOD_ERU) return (TRUE);
	return (FALSE);
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

/*
 * Perform weighted random shuffle according to the algorithm given in
 * "Weighted Random Sampling" (2005, Efraimidis, Spirakis).
 *
 * @param weights is the vector of weights.
 * @return an output vector of the same size as the input weights vector containing,
 *   in order of selection, the indices to select. For example, if you
 *   need to choose two items, you would use v[0], v[1] as the indices
 *   to pick.
 */
static std::vector<size_t> wrs(const std::vector<s32b> &unscaled_weights)
{
	const size_t n = unscaled_weights.size();

	/* Rescale weights into unit interval for numerical stability */
	std::vector<double> weights(unscaled_weights.size());
	{
		s32b scale = 0;
		for (s32b weight: unscaled_weights)
		{
			scale += weight;
		}

		for (size_t i = 0; i < n; i++) {
			weights[i] =
				((double) unscaled_weights[i]) /
				((double) scale);
		}
	}

	/* Generate the keys and indexes to use for selection.  This
	   is the only randomized portion of the algorithm. */
	std::vector<std::tuple<double,size_t>> keys_and_indexes(unscaled_weights.size());
	for (size_t i = 0; i < n; i++) {
		/* Randomized keys according to the algorithm. */
		double u = static_cast<double>(rand_int(100000)) / 100000;
		double k = std::pow(u, 1/weights[i]);
		/* Set up the key and index. We negate the k value
		   here so that keys will be sorted in descending
		   order rather than ascending order. */
		keys_and_indexes[i] = std::make_tuple(-k, i);
	}

	/* Sort indexes according to keys. Since the keys have been
	   negated and we're using a lexicographical sort, we're
	   effectively sorting in descending order of key. */
	std::sort(std::begin(keys_and_indexes),
		  std::end(keys_and_indexes));

	/* Produce the output vector consisting of indexes only. */
	std::vector<size_t> indexes;
	for (auto const &key_and_index: keys_and_indexes) {
		indexes.push_back(std::get<1>(key_and_index));
	}
	return indexes;
}

void do_get_new_skill()
{
	std::vector<std::string> items;
	int skl[LOST_SWORD_NSKILLS];
	s32b val[LOST_SWORD_NSKILLS], mod[LOST_SWORD_NSKILLS];
	int available_skills[MAX_SKILLS];
	int max_a = 0, res, i;

	/* Check if some skills didn't influence other stuff */
	recalc_skills(TRUE);

	/* Grab the ones we can gain */
	max_a = 0;
	for (i = 0; i < max_s_idx; i++)
	{
		if (s_info[i].flags1 & SKF1_RANDOM_GAIN) {
			available_skills[max_a] = i;
			max_a++;
		}
	}

	/* Perform the selection */
	std::vector<s32b> weights;
	for (i = 0; i < max_a; i++) {
		weights.push_back(s_info[available_skills[i]].random_gain_chance);
	}

	std::vector<size_t> indexes = wrs(weights);
	assert(indexes.size() >= LOST_SWORD_NSKILLS);

	/* Extract the information needed from the skills */
	for (i = 0; i < LOST_SWORD_NSKILLS; i++)
	{
		s32b s_idx = available_skills[indexes[i]];
		skill_type *s_ptr = &s_info[s_idx];

		if (s_ptr->mod)
		{
			if (s_ptr->mod < 300)
			{
				val[i] = 1000;
				mod[i] = 300 - s_ptr->mod;
			}
			else if (s_ptr->mod < 500)
			{
				val[i] = s_ptr->mod * 1;
				mod[i] = 100;
				if (mod[i] + s_ptr->mod > 500)
					mod[i] = 500 - s_ptr->mod;
			}
			else
			{
				val[i] = s_ptr->mod * 3;
				mod[i] = 0;
			}
		}
		else
		{
			mod[i] = 300;
			val[i] = 1000;
		}

		if (s_ptr->value + val[i] > SKILL_MAX) {
			val[i] = SKILL_MAX - s_ptr->value;
		}

		skl[i] = s_idx;
		items.push_back(format("%-40s: +%02ld.%03ld value, +%01d.%03d modifier", s_ptr->name + s_name, val[i] / SKILL_STEP, val[i] % SKILL_STEP, mod[i] / SKILL_STEP, mod[i] % SKILL_STEP));
	}

	while (TRUE)
	{
		char last = 'a' + (LOST_SWORD_NSKILLS-1);
		char buf[80];
		sprintf(buf, "Choose a skill to learn(a-%c to choose, ESC to cancel)?", last);
		res = ask_menu(buf, items);

		/* Ok ? lets learn ! */
		if (res > -1)
		{
			skill_type *s_ptr;
			bool_ oppose = FALSE;
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
		if (ab_info[i].name > 0) {
			if (streq(ab_info[i].name + ab_name, name)) return (i);
		}
	}

	/* No match found */
	return ( -1);
}

/*
 * Do the player have the ability
 */
bool_ has_ability(int ab)
{
	return ab_info[ab].acquired;
}

/* Do we meet the requirements */
bool_ can_learn_ability(int ab)
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
		msg_box("You cannot learn this ability.", hgt / 2, wid / 2);
		return;
	}

	/* Flush input as we ask an important and irreversible question */
	flush();

	/* Ask we can commit the change */
	if (msg_box("Learn this ability(this is permanent)? (y/n)", hgt / 2, wid / 2) != 'y')
	{
		return;
	}

	ab_info[ab].acquired = TRUE;
	p_ptr->skill_points -= ab_info[ab].cost;
}

static bool compare_abilities(const int ab_idx1, const int ab_idx2)
{
	return strcmp(ab_name + ab_info[ab_idx1].name,
		      ab_name + ab_info[ab_idx2].name) < 0;
}

/*
 * Print the abilities list
 */
void dump_abilities(FILE *fff)
{
	int i;

	// Find all abilities that the player has.
	std::vector<int> table;
	for (i = 0; i < max_ab_idx; i++)
	{
		if (ab_info[i].name && has_ability(i))
		{
			table.push_back(i);
		}
	}

	// Sort
	std::sort(std::begin(table),
		  std::end(table),
		  compare_abilities);

	// Show
	if (!table.empty())
	{
		fprintf(fff, "\nAbilities");

		for (int i : table)
		{
			fprintf(fff, "\n * %s", ab_info[i].name + ab_name);
		}

		fprintf(fff, "\n");
	}
}

/*
 * Draw the abilities list
 */
static void print_abilities(const std::vector<int> &table, int sel, int start)
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

		if (j >= table.size())
		{
			break;
		}

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
	int sel = 0, start = 0;
	char c;
	int i;
	int wid, hgt;

	/* Save the screen */
	screen_save();

	/* Clear the screen */
	Term_clear();

	/* Initialise the abilities list */
	std::vector<int> table;
	for (i = 0; i < max_ab_idx; i++)
	{
		if (ab_info[i].name)
		{
			table.push_back(i);
		}
	}

	std::sort(std::begin(table),
		  std::end(table),
		  compare_abilities);

	while (TRUE)
	{
		Term_get_size(&wid, &hgt);

		/* Display list of skills */
		print_abilities(table, sel, start);

		/* Wait for user input */
		c = inkey();

		/* Leave the skill screen */
		if (c == ESCAPE)
		{
			break;
		}

		/* Next page */
		else if (c == 'n')
		{
			sel += (hgt - 7);
			if (sel >= table.size())
			{
				sel = table.size() - 1;
			}
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
			if (c == '?')
			{
				help_ability(ab_info[table[sel]].name + ab_name);
			}

			/* Handle boundaries and scrolling */
			if (sel < 0) sel = table.size() - 1;
			if (sel >= table.size()) sel = 0;
			if (sel < start) start = sel;
			if (sel >= start + (hgt - 7)) start = sel - (hgt - 7) + 1;
		}
	}

	/* Load the screen */
	screen_load();

	/* Update stuffs */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
	                  PU_SANITY | PU_BODY);

	/* Redraw various info */
	p_ptr->redraw |= (PR_WIPE | PR_FRAME | PR_MAP);
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
