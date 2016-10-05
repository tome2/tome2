/*
 * Copyright (c) 2001 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "skills.hpp"

#include "ability_type.hpp"
#include "birth.hpp"
#include "cmd2.hpp"
#include "cmd3.hpp"
#include "cmd5.hpp"
#include "cmd7.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "help.hpp"
#include "hooks.hpp"
#include "lua_bind.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_spec.hpp"
#include "player_type.hpp"
#include "skill_flag.hpp"
#include "skill_type.hpp"
#include "spells1.hpp"
#include "spells4.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

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
	auto &s_info = game->s_info;

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
static void decrease_skill(int i, s16b *invest)
{
	auto &s_info = game->s_info;

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
s16b find_skill(cptr needle)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	/* Scan skill list */
	for (std::size_t i = 1; i < s_descriptors.size(); i++)
	{
		auto const &name = s_descriptors[i].name;
		if (!name.empty() && (name == needle))
		{
			return i;
		}
	}

	/* No match found */
	return -1;
}

s16b find_skill_i(cptr needle)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	/* Scan skill list */
	for (std::size_t i = 1; i < s_descriptors.size(); i++)
	{
		auto const &name = s_descriptors[i].name;
		if (!name.empty() && iequals(name, needle))
		{
			return (i);
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
	auto const &s_info = game->s_info;

	return (s_info[skill].value / SKILL_STEP);
}


/*
 * Return "scale" (a misnomer -- this is max value) * (current skill value)
 * / (max skill value)
 */
s16b get_skill_scale(int skill, u32b scale)
{
	auto const &s_info = game->s_info;

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
	s32b temp = scale * s_info[skill].value;

	return (temp / SKILL_MAX);
}

static std::size_t get_idx(int i)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	for (std::size_t j = 1; j < s_descriptors.size(); j++)
	{
		if (s_descriptors[j].order == i)
		{
			return j;
		}
	}

	return 0;
}

static bool_ is_known(int s_idx)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	if (wizard) return TRUE;
	if (s_info[s_idx].value || s_info[s_idx].mod) return TRUE;

	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		/* It is our child, if we don't know it we continue to search, if we know it it is enough*/
		if (s_descriptors[i].father == s_idx)
		{
			if (is_known(i))
				return TRUE;
		}
	}

	/* Ok know none */
	return FALSE;
}

namespace { // anonymous

struct skill_entry {
	std::size_t skill_idx;
	int indent_level;
};

}

static void init_table_aux(std::vector<skill_entry> *table, int father, int lev, bool_ full)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	for (std::size_t j = 1; j < s_descriptors.size(); j++)
	{
		std::size_t i = get_idx(j);

		if (s_descriptors[i].father != father) continue;
		if (s_info[i].hidden) continue;
		if (!is_known(i)) continue;

		skill_entry entry;
		entry.skill_idx = i;
		entry.indent_level = lev;
		table->emplace_back(entry);

		if (s_info[i].dev || full)
		{
			init_table_aux(table, i, lev + 1, full);
		}
	}
}

static void init_table(std::vector<skill_entry> *table, bool_ full)
{
	table->clear();
	init_table_aux(table, -1, 0, full);
}

static bool has_child(int sel)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	for (std::size_t i = 1; i < s_descriptors.size(); i++)
	{
		if ((s_descriptors[i].father == sel) && is_known(i))
		{
			return true;
		}
	}

	return false;
}


/*
 * Dump the skill tree
 */
void dump_skills(FILE *fff)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	char buf[80];

	std::vector<skill_entry> table;
	table.reserve(s_descriptors.size());
	init_table(&table, TRUE);

	fprintf(fff, "\nSkills (points left: %d)", p_ptr->skill_points);

	for (auto const &entry: table)
	{
		std::size_t i = entry.skill_idx;
		auto const &skill = s_info[i];
		auto const &skill_name = s_descriptors[i].name;

		if ((skill.value == 0) && (i != SKILL_MISC))
		{
			if (skill.mod == 0)
			{
				continue;
			}
		}

		sprintf(buf, "\n");

		for (int z = 0; z < entry.indent_level; z++)
		{
			strcat(buf, "         ");
		}

		if (!has_child(i))
		{
			strcat(buf, format(" . %s", skill_name.c_str()));
		}
		else
		{
			strcat(buf, format(" - %s", skill_name.c_str()));
		}

		fprintf(fff, "%-49s%s%06.3f [%05.3f]",
			buf, skill.value < 0 ? "-" : " ",
			static_cast<double>(ABS(skill.value)) / SKILL_STEP,
			static_cast<double>(skill.mod) / 1000);
	}

	fprintf(fff, "\n");
}


/*
 * Draw the skill tree
 */
static void print_skills(std::vector<skill_entry> const &table, int sel, int start)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	int j;
	int wid, hgt;
	cptr keys;

	Term_clear();
	Term_get_size(&wid, &hgt);

	c_prt(TERM_WHITE, format("%s Skills Screen", game_module), 0, 28);
	keys = format("#BEnter#W to develop a branch, #Bup#W/#Bdown#W to move, #Bright#W/#Bleft#W to modify, #B?#W for help");
	display_message(0, 1, strlen(keys), TERM_WHITE, keys);
	c_prt((p_ptr->skill_points) ? TERM_L_BLUE : TERM_L_RED,
	      format("Skill points left: %d", p_ptr->skill_points), 2, 0);
	print_desc_aux(s_descriptors[table[sel].skill_idx].desc.c_str(), 3, 0);

	for (j = start; j < start + (hgt - 7); j++)
	{
		byte color = TERM_WHITE;
		char deb = ' ', end = ' ';

		if (j >= static_cast<int>(table.size()))
		{
			break;
		}

		std::size_t i = table[j].skill_idx;
		auto const &name = s_descriptors[i].name;
		auto const &skill = s_info[i];

		if ((skill.value == 0) && (i != SKILL_MISC))
		{
			if (skill.mod == 0)
			{
				color = TERM_L_DARK;
			}
			else
			{
				color = TERM_ORANGE;
			}
		}
		else if (skill.value == SKILL_MAX)
		{
			color = TERM_L_BLUE;
		}

		if (skill.hidden)
		{
			color = TERM_L_RED;
		}

		if (j == sel)
		{
			color = TERM_L_GREEN;
			deb = '[';
			end = ']';
		}

		if (!has_child(i))
		{
			c_prt(color, format("%c.%c%s", deb, end, name.c_str()),
			      j + 7 - start, table[j].indent_level * 4);
		}
		else if (skill.dev)
		{
			c_prt(color, format("%c-%c%s", deb, end, name.c_str()),
			      j + 7 - start, table[j].indent_level * 4);
		}
		else
		{
			c_prt(color, format("%c+%c%s", deb, end, name.c_str()),
			      j + 7 - start, table[j].indent_level * 4);
		}

		c_prt(color,
		      format("%s%02ld.%03ld [%01d.%03d]",
			     skill.value < 0 ? "-" : " ",
			     ABS(skill.value) / SKILL_STEP,
			     ABS(skill.value) % SKILL_STEP,
			     ABS(skill.mod) / 1000,
			     ABS(skill.mod) % 1000),
		      j + 7 - start, 60);
	}
}

/*
 * Checks various stuff to do when skills change, like new spells, ...
 */
void recalc_skills(bool_ init)
{
	auto const &s_info = game->s_info;

	static int thaum_level = 0;

	/* TODO: This should be a hook in ToME's lua */
	if (init)
	{
		thaum_level = get_skill_scale(SKILL_THAUMATURGY, 100);
	}
	else
	{
		auto const &random_spells = p_ptr->random_spells;

		int thaum_gain = 0;

		/* Gain thaum spells while there's more to be gained */
		while ((thaum_level < get_skill_scale(SKILL_THAUMATURGY, 100)) &&
		        (random_spells.size() < MAX_SPELLS))
		{
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
static void recalc_skills_theory(
	std::vector<s16b> &invest,
	std::vector<s32b> const &base_val,
	std::vector<s32b> const &base_mod,
	std::vector<s32b> const &bonus)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	/* First we assign the normal points */
	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		/* Calc the base */
		s_info[i].value = base_val[i] + (base_mod[i] * invest[i]) + bonus[i];

		/* It cannot exceed SKILL_MAX */
		if (s_info[i].value > SKILL_MAX)
		{
			s_info[i].value = SKILL_MAX;
		}
	}

	/* Then we modify related skills */
	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		auto const &s_descriptor = s_descriptors[i];

		// Process all exlusions
		if (invest[i])
		{
			for (auto exclude_si: s_descriptor.excludes)
			{
				// Give back skill points invested during this "session"
				p_ptr->skill_points += invest[exclude_si];
				invest[exclude_si] = 0;

				// Turn it off
				s_info[exclude_si].value = 0;
			}
		}

		// Add any bonuses
		for (auto const &increase: s_descriptor.increases)
		{
			auto increase_si = std::get<0>(increase);
			auto increase_pct = std::get<1>(increase);

			/* Increase/decrease by percentage */
			s32b val = s_info[increase_si].value + (invest[i] * s_info[increase_si].mod * increase_pct / 100);

			/* It cannot exceed SKILL_MAX */
			if (val > SKILL_MAX)
			{
				val = SKILL_MAX;
			}

			/* Save the modified value */
			s_info[increase_si].value = val;
		}
	}
}

/*
 * Interreact with skills
 */
void do_cmd_skill()
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	int sel = 0, start = 0;
	char c;
	int wid, hgt;
	s16b skill_points_save;

	recalc_skills(TRUE);

	/* Save the screen */
	screen_save();

	/* Allocate arrays to save skill values and track assigned points */
	std::vector<s32b> skill_values_save; skill_values_save.resize(s_descriptors.size(), 0);
	std::vector<s32b> skill_mods_save;   skill_mods_save.resize(s_descriptors.size(), 0);
	std::vector<s16b> skill_invest;      skill_invest.resize(s_descriptors.size(), 0);
	std::vector<s32b> skill_bonus;       skill_bonus.resize(s_descriptors.size(), 0);

	/* Save skill points */
	skill_points_save = p_ptr->skill_points;

	/* Save skill values */
	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		auto s_ptr = &s_info[i];
		skill_values_save[i] = s_ptr->value;
		skill_mods_save[i] = s_ptr->mod;
	}

	/* Clear the screen */
	Term_clear();

	/* Initialise the skill list */
	std::vector<skill_entry> table;
	table.reserve(s_descriptors.size());
	init_table(&table, FALSE);

	while (TRUE)
	{
		Term_get_size(&wid, &hgt);

		/* Display list of skills */
		recalc_skills_theory(skill_invest, skill_values_save, skill_mods_save, skill_bonus);
		print_skills(table, sel, start);

		/* Wait for user input */
		c = inkey();

		/* Leave the skill screen */
		if (c == ESCAPE) break;

		/* Expand / collapse list of skills */
		else if (c == '\r')
		{
			// Toggle the selected skill
			s_info[table[sel].skill_idx].dev = !s_info[table[sel].skill_idx].dev;
			// Re-populate table
			init_table(&table, FALSE);
		}

		/* Next page */
		else if (c == 'n')
		{
			sel += (hgt - 7);

			if (sel >= static_cast<int>(table.size()))
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

			/* Miscellaneous skills cannot be increased/decreased as a group */
			if ((sel >= 0) && (sel < static_cast<int>(table.size())) && table[sel].skill_idx == SKILL_MISC) continue;

			/* Increase the current skill */
			if (dir == 6) increase_skill(table[sel].skill_idx, skill_invest.data());

			/* Decrease the current skill */
			if (dir == 4) decrease_skill(table[sel].skill_idx, skill_invest.data());

			/* XXX XXX XXX Wizard mode commands outside of wizard2.c */

			/* Increase the skill */
			if (wizard && (c == '+')) skill_bonus[table[sel].skill_idx] += SKILL_STEP;

			/* Decrease the skill */
			if (wizard && (c == '-')) skill_bonus[table[sel].skill_idx] -= SKILL_STEP;

			/* Contextual help */
			if (c == '?')
			{
				help_skill(s_descriptors[table[sel].skill_idx].name);
			}

			/* Handle boundaries and scrolling */
			if (sel < 0) sel = table.size() - 1;
			if (sel >= static_cast<int>(table.size())) sel = 0;
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
			for (std::size_t i = 0; i < s_descriptors.size(); i++)
			{
				auto s_ptr = &s_info[i];
				s_ptr->value = skill_values_save[i];
				s_ptr->mod = skill_mods_save[i];
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
static s16b melee_skills[MAX_MELEE] =
{
	SKILL_MASTERY,
	SKILL_HAND,
	SKILL_BEAR,
};
static const char *melee_names[MAX_MELEE] =
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

cptr get_melee_name()
{
	return melee_names[get_melee_skill()];
}

s16b get_melee_skills()
{
	auto const &s_info = game->s_info;

	int j = 0;

	for (std::size_t i = 0; i < MAX_MELEE; i++)
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
static void print_skill_batch(const std::vector<std::tuple<std::string, int>> &p, int start)
{
	char buff[80];
	int j = 0;

	prt(format("         %-31s", "Name"), 1, 20);

	for (int i = start; i < (start + 20); i++)
	{
		if (static_cast<size_t>(i) >= p.size())
		{
			break;
		}

		sprintf(buff, "  %c - %d) %-30s", I2A(j),
			std::get<1>(p[i]),
			std::get<0>(p[i]).c_str());

		prt(buff, 2 + j, 20);
		j++;
	}
	prt("", 2 + j, 20);
	prt(format("Select a skill (a-%c), @ to select by name, +/- to scroll:", I2A(j - 1)), 0, 0);
}

static int do_cmd_activate_skill_aux()
{
	auto const &ab_info = game->edit_data.ab_info;
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	char which;
	int start = 0;
	int ret;

	std::vector<std::tuple<std::string,int>> p;

	/* More than 1 melee skill ? */
	if (get_melee_skills() > 1)
	{
		p.push_back(std::make_tuple("Change melee mode", 0));
	}

	for (size_t i = 1; i < s_descriptors.size(); i++)
	{
		if (s_descriptors[i].action_mkey && s_info[i].value && ((!s_info[i].hidden) || (i == SKILL_LEARN)))
		{
			bool_ next = FALSE;

			/* Already got it ? */
			for (size_t j = 0; j < p.size(); j++)
			{
				if (s_descriptors[i].action_mkey == std::get<1>(p[j]))
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p.push_back(std::make_tuple(s_descriptors[i].action_desc,
			                            s_descriptors[i].action_mkey));
		}
	}

	for (size_t i = 0; i < ab_info.size(); i++)
	{
		if (ab_info[i].action_mkey && p_ptr->has_ability(i))
		{
			bool_ next = FALSE;

			/* Already got it ? */
			for (size_t j = 0; j < p.size(); j++)
			{
				if (ab_info[i].action_mkey == std::get<1>(p[j]))
				{
					next = TRUE;
					break;
				}
			}
			if (next) continue;

			p.push_back(std::make_tuple(ab_info[i].action_desc,
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
			if (static_cast<size_t>(start) >= p.size())
			{
				start -= 20;
			}
			Term_load();
			character_icky = FALSE;
		}
		else if (which == '-')
		{
			start -= 20;
			if (start < 0)
			{
				start += 20;
			}
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
			size_t i = 0;
			for (; i < p.size(); i++)
			{
				if (std::get<0>(p[i]) == buf)
				{
					break;
				}
			}

			if (i < p.size())
			{
				ret = std::get<1>(p[i]);
				break;
			}
		}
		else
		{
			which = tolower(which);
			if (start + A2I(which) >= static_cast<int>(p.size()))
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
	auto const &ab_info = game->edit_data.ab_info;
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto const &s_info = game->s_info;

	int x_idx;
	bool_ push = TRUE;

	/* Get the skill, if available */
	if (repeat_pull(&x_idx))
	{
		push = FALSE;
	}
	else if (!command_arg)
	{
		x_idx = do_cmd_activate_skill_aux();
	}
	else
	{
		x_idx = command_arg;

		/* Check validity */
		std::size_t i;
		for (i = 1; i < s_descriptors.size(); i++)
		{
			if (s_info[i].value && (s_descriptors[i].action_mkey == x_idx))
			{
				break;
			}
		}

		std::size_t j;
		for (j = 0; j < ab_info.size(); j++)
		{
			if (p_ptr->has_ability(j) && (ab_info[j].action_mkey == x_idx))
			{
				break;
			}
		}

		if ((j == ab_info.size()) && (i == s_descriptors.size()))
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
	case MKEY_SUMMON:
		do_cmd_summoner();
		break;
	case MKEY_NECRO:
		do_cmd_necromancer();
		break;
	case MKEY_SYMBIOTIC:
		do_cmd_symbiotic();
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
		if (o_ptr->tval != TV_POLEARM)
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
	if (p_ptr->pgod == GOD_ERU) return (TRUE);
	return (FALSE);
}


/*
 * Augment skill value/modifier with the given skill_modifiers
 */
static void augment_skills(s32b *v, s32b *m, std::vector<skill_modifier> const &modifiers, std::size_t i)
{
	if (i < modifiers.size()) // Ignore if the skill has no modifiers.
	{
		auto const &s = modifiers[i];
		*v = modify_aux(*v, s.base, s.basem);
		*m = modify_aux(*m, s.mod,  s.modm);
	}
}


/*
 * Gets the base value of a skill, given a race/class/...
 */
void compute_skills(s32b *v, s32b *m, std::size_t i)
{
	auto const &gen_skill = game->edit_data.gen_skill;

	augment_skills(v, m, gen_skill.modifiers, i);
	augment_skills(v, m, rp_ptr->skill_modifiers.modifiers, i);
	augment_skills(v, m, rmp_ptr->skill_modifiers.modifiers, i);
	augment_skills(v, m, cp_ptr->skill_modifiers.modifiers, i);
	augment_skills(v, m, spp_ptr->skill_modifiers.modifiers, i);
}

/*
 * Initialize a skill with given values
 */
void init_skill(s32b value, s32b mod, std::size_t i)
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	s_info[i].value = value;
	s_info[i].mod = mod;
	s_info[i].hidden = (s_descriptors[i].flags & SKF_HIDDEN)
	        ? true
	        : false
	        ;
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
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	/* Check if some skills didn't influence other stuff */
	recalc_skills(TRUE);

	/* Grab the ones we can gain */
	std::vector<size_t> available_skills;
	available_skills.reserve(s_descriptors.size());
	for (std::size_t i = 0; i < s_descriptors.size(); i++)
	{
		if (s_descriptors[i].flags & SKF_RANDOM_GAIN)
		{
			available_skills.push_back(i);
		}
	}

	/* Perform the selection */
	std::vector<s32b> weights;
	for (std::size_t i = 0; i < available_skills.size(); i++)
	{
		weights.push_back(s_descriptors[available_skills[i]].random_gain_chance);
	}

	std::vector<size_t> indexes = wrs(weights);
	assert(indexes.size() >= LOST_SWORD_NSKILLS);

	/* Extract the information needed from the skills */
	int skl[LOST_SWORD_NSKILLS];
	s32b val[LOST_SWORD_NSKILLS];
	s32b mod[LOST_SWORD_NSKILLS];
	std::vector<std::string> items;
	for (std::size_t i = 0; i < LOST_SWORD_NSKILLS; i++)
	{
		s32b s_idx = available_skills[indexes[i]];
		auto s_ptr = &s_info[s_idx];

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
				{
					mod[i] = 500 - s_ptr->mod;
				}
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

		if (s_ptr->value + val[i] > SKILL_MAX)
		{
			val[i] = SKILL_MAX - s_ptr->value;
		}

		skl[i] = s_idx;
		items.push_back(format("%-40s: +%02ld.%03ld value, +%01d.%03d modifier",
				       s_descriptors[s_idx].name.c_str(),
				       val[i] / SKILL_STEP,
				       val[i] % SKILL_STEP,
				       mod[i] / SKILL_STEP,
				       mod[i] % SKILL_STEP));
	}

	// Ask for a skill
	while (TRUE)
	{
		char last = 'a' + (LOST_SWORD_NSKILLS-1);
		char buf[80];
		sprintf(buf, "Choose a skill to learn (a-%c to choose, ESC to cancel)?", last);
		int res = ask_menu(buf, items);

		/* Ok ? lets learn ! */
		if (res > -1)
		{
			std::size_t chosen_skill = skl[res];

			bool_ oppose = FALSE;
			int oppose_skill = -1;

			/* Check we don't oppose a skill the player already has */
			for (std::size_t i = 0; i < s_descriptors.size(); i++)
			{
				auto const &s_descriptor = s_descriptors[i];

				// Only bother if player has skill
				if (s_info[i].value)
				{
					// Check if i'th skill opposes the chosen one.
					auto found = std::find(
						s_descriptor.excludes.begin(),
						s_descriptor.excludes.end(),
						chosen_skill);

					if (found != s_descriptor.excludes.end())
					{
						oppose = TRUE;
						oppose_skill = i;
						break;
					}
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
					     s_descriptors[oppose_skill].name.c_str());

				/* The player rejected the choice; go back to prompt */
				if (!get_check(msg))
				{
					continue;
				}
			}

			auto const s_desc = &s_descriptors[chosen_skill];
			auto const s_ptr = &s_info[chosen_skill];

			s_ptr->value += val[res];
			s_ptr->mod += mod[res];

			if (mod[res])
			{
				msg_format("You can now learn the %s skill.",
					   s_desc->name.c_str());
			}
			else
			{
				msg_format("Your knowledge of the %s skill increases.",
					   s_desc->name.c_str());
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
	auto const &ab_info = game->edit_data.ab_info;

	for (std::size_t i = 0; i < ab_info.size(); i++)
	{
		auto const &ab_name = ab_info[i].name;

		if ((!ab_name.empty()) && (ab_name == name))
		{
			return (i);
		}
	}

	/* No match found */
	return ( -1);
}

/* Do we meet the requirements? */
static bool can_learn_ability(int ab)
{
	auto const &ab_info = game->edit_data.ab_info;

	auto ab_ptr = &ab_info[ab];

	if (p_ptr->has_ability(ab))
	{
		return FALSE;
	}

	if (p_ptr->skill_points < ab_info[ab].cost)
	{
		return FALSE;
	}

	for (auto const &need_skill: ab_ptr->need_skills)
	{
		if (get_skill(need_skill.skill_idx) < need_skill.level)
		{
			return FALSE;
		}
	}

	for (auto const &need_ability: ab_ptr->need_abilities)
	{
		if (!p_ptr->has_ability(need_ability))
		{
			return FALSE;
		}
	}

	for (std::size_t i = 0; i < 6; i++)
	{
		/* Must have stat */
		if (ab_ptr->stat[i] > -1)
		{
			if (p_ptr->stat_ind[i] < ab_ptr->stat[i] - 3)
				return FALSE;
		}
	}

	return TRUE;
}

/* Learn an ability */
static void gain_ability(int ab)
{
	auto const &ab_info = game->edit_data.ab_info;

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
	if (msg_box("Learn this ability (this is permanent)? (y/n)", hgt / 2, wid / 2) != 'y')
	{
		return;
	}

	p_ptr->gain_ability(ab);
	p_ptr->skill_points -= ab_info[ab].cost;
}

static bool compare_abilities(std::size_t ab_idx1, std::size_t ab_idx2)
{
	auto const &ab_info = game->edit_data.ab_info;

	return ab_info[ab_idx1].name < ab_info[ab_idx2].name;
}

/*
 * Print the abilities list
 */
void dump_abilities(FILE *fff)
{
	auto const &ab_info = game->edit_data.ab_info;

	// Find all abilities that the player has.
	std::vector<std::size_t> table;
	for (std::size_t i = 0; i < ab_info.size(); i++)
	{
		if ((!ab_info[i].name.empty()) && p_ptr->has_ability(i))
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
			fprintf(fff, "\n * %s", ab_info[i].name.c_str());
		}

		fprintf(fff, "\n");
	}
}

/*
 * Draw the abilities list
 */
static void print_abilities(const std::vector<std::size_t> &table, int sel, int start)
{
	auto const &ab_info = game->edit_data.ab_info;

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

	print_desc_aux(ab_info[table[sel]].desc.c_str(), 3, 0);

	for (j = start; j < start + (hgt - 7); j++)
	{
		byte color = TERM_WHITE;
		char deb = ' ', end = ' ';

		assert(j >= 0);
		if (static_cast<size_t>(j) >= table.size())
		{
			break;
		}

		i = table[j];

		if (p_ptr->has_ability(i))
		{
			color = TERM_L_BLUE;
		}
		else if (can_learn_ability(i))
		{
			color = TERM_WHITE;
		}
		else
		{
			color = TERM_L_DARK;
		}


		if (j == sel)
		{
			color = TERM_L_GREEN;
			deb = '[';
			end = ']';
		}

		c_prt(color, format("%c.%c%s", deb, end, ab_info[i].name.c_str()),
		      j + 7 - start, 0);

		if (!p_ptr->has_ability(i))
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
 * Interreact with abilities
 */
void do_cmd_ability()
{
	auto const &ab_info = game->edit_data.ab_info;

	int sel = 0, start = 0;
	char c;
	int wid, hgt;

	/* Save the screen */
	screen_save();

	/* Clear the screen */
	Term_clear();

	/* Initialise the abilities list */
	std::vector<std::size_t> table;
	for (std::size_t i = 0; i < ab_info.size(); i++)
	{
		if (!ab_info[i].name.empty())
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
			assert(sel >= 0);
			if (static_cast<size_t>(sel) >= table.size())
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

			/* Wizard mode allows any ability */
			if (wizard && (c == '+'))
			{
				p_ptr->gain_ability(table[sel]);
			}

			if (wizard && (c == '-'))
			{
				p_ptr->lose_ability(table[sel]);
			}

			/* Contextual help */
			if (c == '?')
			{
				help_ability(ab_info[table[sel]].name);
			}

			/* Handle boundaries and scrolling */
			if (sel < 0)
			{
				sel = table.size() - 1;
			}
			if (static_cast<size_t>(sel) >= table.size())
			{
				sel = 0;
			}
			if (sel < start)
			{
				start = sel;
			}
			if (sel >= start + (hgt - 7))
			{
				start = sel - (hgt - 7) + 1;
			}
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
	auto apply = [level](std::vector<player_race_ability_type> const &abilities) -> void
	{
		auto const &ab_info = game->edit_data.ab_info;

		for (auto const &a: abilities)
		{
			if (a.level == level)
			{
				if ((level > 1) && (!p_ptr->has_ability(a.ability)))
				{
					cmsg_format(TERM_L_GREEN, "You have learned the ability '%s'.", ab_info[a.ability].name.c_str());
				}

				p_ptr->gain_ability(a.ability);
			}
		}
	};

	apply(cp_ptr->abilities);
	apply(spp_ptr->abilities);
	apply(rp_ptr->abilities);
	apply(rmp_ptr->abilities);
}
