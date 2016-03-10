/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "cmd5.hpp"

#include "birth.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "corrupt.hpp"
#include "lua_bind.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "school_book.hpp"
#include "skills.hpp"
#include "spell_type.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "spells4.hpp"
#include "spells5.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "quark.hpp"
#include "wizard2.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <cassert>

/* Maximum number of tries for teleporting */
#define MAX_TRIES 300

static object_filter_t const &is_school_book()
{
	using namespace object_filter;
	static auto instance = Or(
		TVal(TV_BOOK),
		TVal(TV_DAEMON_BOOK),
		TVal(TV_INSTRUMENT));
	return instance;
}

/* Does it contains a schooled spell ? */
static object_filter_t const &hook_school_spellable()
{
	using namespace object_filter;
	static auto has_pval2 =
		[=](object_type const *o_ptr) -> bool {
			return (o_ptr->pval2 != -1);
		};
	static auto instance = Or(
		is_school_book(),
		And(
			HasFlag5(TR5_SPELL_CONTAIN),
			has_pval2));
	return instance;
}

/* Is it a browsable for spells? */
static object_filter_t const &item_tester_hook_browsable()
{
	using namespace object_filter;
	static auto instance = Or(
		hook_school_spellable(),
		TVal(TV_BOOK));
	return instance;
}

/*
 * Are we using a mage staff
 */
bool_ is_magestaff()
{
	int i;


	i = 0;

	while (p_ptr->body_parts[i] == INVEN_WIELD)
	{
		object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		/* Wielding a mage staff */
		if ((o_ptr->k_idx) && (o_ptr->tval == TV_MSTAFF)) return (TRUE);

		/* Next slot */
		i++;

		/* Paranoia */
		if (i >= (INVEN_TOTAL - INVEN_WIELD)) break;
	}

	/* Not wielding a mage staff */
	return (FALSE);
}


static int print_book(s16b sval, s32b spell_idx, object_type *obj)
{
	int y = 2;
	int i;

	random_book_setup(sval, spell_idx);

	school_book *school_book = school_books_at(sval);

	/* Parse all spells */
	i = 0;
	for (auto spell_idx : school_book->spell_idxs)
	{
		byte color = TERM_L_DARK;
		bool_ is_ok;
		char label[8];

		is_ok = is_ok_spell(spell_idx, obj->pval);
		if (is_ok)
		{
			color = (get_mana(spell_idx) > get_power(spell_idx)) ? TERM_ORANGE : TERM_L_GREEN;
		}

		sprintf(label, "%c) ", 'a' + i);

		y = print_spell(label, color, y, spell_idx);
		i++;
	}

	prt(format("   %-20s%-16s Level Cost Fail Info", "Name", "School"), 1, 0);
	return y;
}



static void browse_school_spell(int book, int spell_idx, object_type *o_ptr)
{
	int i;
	int num = 0;
	int ask;
	char choice;
	char out_val[160];

	/* Show choices */
	window_stuff();

	num = school_book_length(book);

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(Spells %c-%c, ESC=exit) cast which spell? ",
		I2A(0), I2A(num - 1));

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Display a list of spells */
	print_book(book, spell_idx, o_ptr);

	/* Get a spell from the user */
	while (get_com(out_val, &choice))
	{
		/* Display a list of spells */
		print_book(book, spell_idx, o_ptr);

		/* Note verify */
		ask = (isupper(choice));

		/* Lowercase */
		if (ask) choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Restore the screen */
		Term_load();

		/* Display a list of spells */
		auto where = print_book(book, spell_idx, o_ptr);
		print_spell_desc(spell_x(book, spell_idx, i), where);
	}


	/* Restore the screen */
	Term_load();
	character_icky = FALSE;

	/* Show choices */
	window_stuff();
}


/*
 * Peruse the spells/prayers in a book
 *
 * Note that *all* spells in the book are listed
 *
 * Note that browsing is allowed while confused or blind,
 * and in the dark, primarily to allow browsing in stores.
 */

extern void do_cmd_browse_aux(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (is_school_book()(o_ptr))
	{
		browse_school_spell(o_ptr->sval, o_ptr->pval, o_ptr);
	}
	else if (f5 & TR5_SPELL_CONTAIN && o_ptr->pval2 != -1)
	{
		browse_school_spell(255, o_ptr->pval2, o_ptr);
	}
}

void do_cmd_browse(void)
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Browse which book? ",
		      "You have no books that you can read.",
		      (USE_INVEN | USE_EQUIP | USE_FLOOR),
		      item_tester_hook_browsable()))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	do_cmd_browse_aux(o_ptr);
}

static void do_poly_wounds()
{
	/* Changed to always provide at least _some_ healing */
	s16b wounds = p_ptr->cut;

	s16b hit_p = (p_ptr->mhp - p_ptr->chp);

	s16b change = damroll(p_ptr->lev, 5);

	bool_ Nasty_effect = (randint(5) == 1);


	if (!(wounds || hit_p || Nasty_effect)) return;

	msg_print("Your wounds are polymorphed into less serious ones.");
	hp_player(change);
	if (Nasty_effect)
	{
		msg_print("A new wound was created!");
		take_hit(change / 2, "a polymorphed wound");
		set_cut(change);
	}
	else
	{
		set_cut((p_ptr->cut) - (change / 2));
	}
}

void do_poly_self(void)
{
	int power = p_ptr->lev;
	int poly_power;

	msg_print("You feel a change coming over you...");

	if ((power > rand_int(20)) && (rand_int(3) == 0))
	{
		char effect_msg[80] = "";
		int new_race, expfact, goalexpfact;

		/* Some form of racial polymorph... */
		power -= 10;

		if ((power > rand_int(30)) && (rand_int(5) == 0))
		{
			int tmp = 0;

			/* Harmful deformity */
			power -= 15;

			while (tmp < 6)
			{
				if ( rand_int(2) == 0)
				{
					(void)dec_stat(tmp, randint(6) + 6, (rand_int(3) == 0));
					power -= 1;
				}
				tmp++;
			}

			/* Deformities are discriminated against! */
			(void)dec_stat(A_CHR, randint(6), TRUE);

			if (effect_msg[0])
			{
				char tmp_msg[10];
				strnfmt(tmp_msg, 10, "%s", effect_msg);
				strnfmt(effect_msg, 80, "deformed %s", tmp_msg);
			}
			else
			{
				strcpy(effect_msg, "deformed");
			}
		}

		while ((power > rand_int(20)) && (rand_int(10) == 0))
		{
			/* Polymorph into a less corrupted form */
			power -= 10;

			lose_corruption();
		}

		/*
		 * I'm not sure 'power' is always positive, with *so* many minuses.
		 * Also, passing zero / negative numbers to randint/rand_int can
		 * cause a zero divide exception, IIRC, not to speak of its absurdity
		 * -- pelpel
		 */
		poly_power = (power > 1) ? power : 1;

		/*
		 * Restrict the race choices by exp penalty so weak polymorph
		 * always means weak race
		 */
		goalexpfact = 100 + 3 * rand_int(poly_power);

		/* Roll until an appropriate selection is made */
		while (1)
		{
			new_race = rand_int(max_rp_idx);
			expfact = race_info[new_race].r_exp;

			if ((new_race != p_ptr->prace) && (expfact <= goalexpfact)) break;
		}

		if (effect_msg[0])
		{
			msg_format("You turn into a%s %s!",
				   ((is_a_vowel(*race_info[new_race].title)) ? "n" : ""),
				   race_info[new_race].title);
		}
		else
		{
			msg_format("You turn into a %s %s!", effect_msg,
			           race_info[new_race].title);
		}

		p_ptr->prace = new_race;
		rp_ptr = &race_info[p_ptr->prace];

		/* Experience factor */
		p_ptr->expfact = rp_ptr->r_exp + rmp_ptr->r_exp + cp_ptr->c_exp;

		/* Level up if necessary */
		check_experience();
		p_ptr->max_plv = p_ptr->lev;

		p_ptr->redraw |= (PR_FRAME);

		p_ptr->update |= (PU_BONUS);

		handle_stuff();
		lite_spot(p_ptr->py, p_ptr->px);
	}

	if ((power > rand_int(30)) && (rand_int(6) == 0))
	{
		int tmp = 0;

		/* Abomination! */
		power -= 20;

		msg_print("Your internal organs are rearranged!");
		while (tmp < 6)
		{
			(void)dec_stat(tmp, randint(6) + 6, (rand_int(3) == 0));
			tmp++;
		}
		if (rand_int(6) == 0)
		{
			msg_print("You find living difficult in your present form!");
			take_hit(damroll(randint(10), p_ptr->lev), "a lethal corruption");
			power -= 10;
		}
	}

	if ((power > rand_int(20)) && (rand_int(4) == 0))
	{
		power -= 10;

		do_cmd_rerate();
	}

	while ((power > rand_int(15)) && (rand_int(3) == 0))
	{
		power -= 7;
		gain_random_corruption();
	}

	if (power > rand_int(5))
	{
		power -= 5;
		do_poly_wounds();
	}

	/* Note: earlier deductions may have left power < 0 already. */
	while (power > 0)
	{
		corrupt_player();
		power--;
	}
}

/*
 * Fetch an item (teleport it right underneath the caster)
 */
void fetch(int dir, int wgt, bool_ require_los)
{
	/* Check to see if an object is already there */
	if (!cave[p_ptr->py][p_ptr->px].o_idxs.empty())
	{
		msg_print("You can't fetch when you're already standing on something.");
		return;
	}

	/* Use a target */
	cave_type *c_ptr = nullptr;
	if ((dir == 5) && target_okay())
	{
		int tx = target_col;
		int ty = target_row;

		if (distance(p_ptr->py, p_ptr->px, ty, tx) > MAX_RANGE)
		{
			msg_print("You can't fetch something that far away!");
			return;
		}

		c_ptr = &cave[ty][tx];

		if (c_ptr->o_idxs.empty())
		{
			msg_print("There is no object at this place.");
			return;
		}

		if (require_los && (!player_has_los_bold(ty, tx)))
		{
			msg_print("You have no direct line of sight to that location.");
			return;
		}
	}
	else
	{
		/* Use a direction */
		int ty = p_ptr->py;  /* Where to drop the item */
		int tx = p_ptr->px;

		while (1)
		{
			ty += ddy[dir];
			tx += ddx[dir];
			c_ptr = &cave[ty][tx];

			if ((distance(p_ptr->py, p_ptr->px, ty, tx) > MAX_RANGE) ||
			                !cave_floor_bold(ty, tx)) return;

			if (!c_ptr->o_idxs.empty()) break;
		}
	}

	assert(c_ptr != nullptr);
	assert(!c_ptr->o_idxs.empty());

	/* Pick object from the list */
	auto o_idx = c_ptr->o_idxs.front();

	object_type *o_ptr = &o_list[o_idx];
	if (o_ptr->weight > wgt)
	{
		/* Too heavy to 'fetch' */
		msg_print("The object is too heavy.");
		return;
	}

	/* Move the object between the lists */
	c_ptr->o_idxs.erase(c_ptr->o_idxs.begin());         // Remove
	cave[p_ptr->py][p_ptr->px].o_idxs.push_back(o_idx); // Add

	/* Update object's location */
	o_ptr->iy = p_ptr->py;
	o_ptr->ix = p_ptr->px;

	/* Feedback */
	char o_name[80];
	object_desc(o_name, o_ptr, TRUE, 0);
	msg_format("%^s flies through the air to your feet.", o_name);

	note_spot(p_ptr->py, p_ptr->px);
	p_ptr->redraw |= PR_MAP;
}


/*
 * Handle random effects of player shrieking
 */
void shriek_effect()
{
	switch (randint(9))
	{
	case 1:
	case 5:
	case 8:
	case 9:
		{
			msg_print("You make a high-pitched shriek!");
			aggravate_monsters(1);

			break;
		}
	case 2:
	case 6:
		{
			msg_print("Oops! You call a monster.");
			summon_specific(p_ptr->py, p_ptr->px, max_dlv[dungeon_type], 0);

			break;
		}
	case 3:
	case 7:
		{
			msg_print("The dungeon collapses!");
			earthquake(p_ptr->py, p_ptr->px, 5);

			break;
		}
	case 4:
		{
			msg_print("Your shriek is so horrible that you damage your health!");
			take_hit(damroll(p_ptr->lev / 5, 8), "inner hemorrhaging");

			break;
		}
	}
}

/*
 * Return the symbiote's name or description.
 */
cptr symbiote_name(bool_ capitalize)
{
	object_type *o_ptr = &p_ptr->inventory[INVEN_CARRY];
	static char buf[80];

	/* Make sure there actually is a symbiote there... */
	if (!o_ptr->k_idx)
	{
		strcpy(buf, "A non-existent symbiote");
	}
	else
	{
		monster_race *r_ptr = &r_info[o_ptr->pval];
		cptr s = NULL;

		if (r_ptr->flags1 & RF1_UNIQUE)
		{
			/* Unique monster; no preceding "your", and ignore our name. */
			strncpy(buf, r_ptr->name, sizeof(buf));
		}
		else if (o_ptr->note &&
		                (s = strstr(quark_str(o_ptr->note), "#named ")) != NULL)
		{
			/* We've named it. */
			strncpy(buf, s + 7, sizeof(buf));
		}
		else
		{
			/* No special cases, just return "Your <monster type>". */
			strcpy(buf, "your ");
			strncpy(buf + 5, r_ptr->name, sizeof(buf) - 5);
		}
	}

	/* Just in case... */
	buf[sizeof(buf) - 1] = '\0';
	if (capitalize) buf[0] = toupper(buf[0]);
	return buf;
}

/*
 * Extract powers
 */
std::vector<int> extract_monster_powers(monster_race const *r_ptr, bool great)
{
	std::vector<int> powers;
	powers.reserve(MONSTER_POWERS_MAX);

	/* List the monster powers -- RF4_* */
	for (std::size_t i = 0; i < 32; i++)
	{
		if (r_ptr->flags4 & BIT(i))
		{
			if (monster_powers[i].great && (!great)) continue;
			if (!monster_powers[i].power) continue;
			powers.push_back(i);
		}
	}

	/* List the monster powers -- RF5_* */
	for (std::size_t i = 0; i < 32; i++)
	{
		if (r_ptr->flags5 & BIT(i))
		{
			if (monster_powers[i + 32].great && (!great)) continue;
			if (!monster_powers[i + 32].power) continue;
			powers.push_back(i + 32);
		}
	}

	/* List the monster powers -- RF6_* */
	for (std::size_t i = 0; i < 32; i++)
	{
		if (r_ptr->flags6 & BIT(i))
		{
			if (monster_powers[i + 64].great && (!great)) continue;
			if (!monster_powers[i + 64].power) continue;
			powers.push_back(i + 64);
		}
	}

	return powers;
}

/**
 * Choose a monster power
 */
static std::tuple<int, int> choose_monster_power(monster_race const *r_ptr, bool great, bool no_cost)
{
	/* Extract available monster powers */
	std::vector<int> powers = extract_monster_powers(r_ptr, great);
	int const num = powers.size(); // Avoid signed/unsigned warnings

	if (!num)
	{
		msg_print("You have no powers you can use.");
		return std::make_tuple(0, num);
	}

	/* Get the last label */
	int label = (num <= 26) ? I2A(num - 1) : I2D(num - 1 - 26);

	/* Build a prompt (accept all spells) */
	/* Mega Hack -- if no_cost is false, we're actually a Possessor -dsb */
	char out_val[160];
	strnfmt(out_val, 78,
	        "(Powers a-%c, ESC=exit) Use which power of your %s? ",
	        label, (no_cost ? "symbiote" : "body"));

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Get a spell from the user */
	int power = -1; // Selected power
	bool_ flag = FALSE; // Nothing chosen yet
	while (!flag)
	{
		/* Show the list */
		{
			byte y = 1, x = 0;
			int ctr = 0;
			char dummy[80];

			strcpy(dummy, "");

			prt ("", y++, x);

			while (ctr < num)
			{
				monster_power *mp_ptr = &monster_powers[powers[ctr]];
				int mana = mp_ptr->mana / 10;

				if (mana > p_ptr->msp) mana = p_ptr->msp;

				if (!mana) mana = 1;

				label = (ctr < 26) ? I2A(ctr) : I2D(ctr - 26);

				if (!no_cost)
				{
					strnfmt(dummy, 80, " %c) %2d %s",
						label, mana, mp_ptr->name);
				}
				else
				{
					strnfmt(dummy, 80, " %c) %s",
						label, mp_ptr->name);
				}

				if (ctr < 17)
				{
					prt(dummy, y + ctr, x);
				}
				else
				{
					prt(dummy, y + ctr - 17, x + 40);
				}

				ctr++;
			}

			if (ctr < 17)
			{
				prt ("", y + ctr, x);
			}
			else
			{
				prt ("", y + 17, x);
			}
		}

		char choice;
		if (!get_com(out_val, &choice))
		{
			flag = FALSE;
			break;
		}

		if (choice == '\r' && num == 1)
		{
			choice = 'a';
		}

		int i;
		int ask;
		if (isalpha(choice))
		{
			/* Note verify */
			ask = (isupper(choice));

			/* Lowercase */
			if (ask) choice = tolower(choice);

			/* Extract request */
			i = (islower(choice) ? A2I(choice) : -1);
		}
		else
		{
			/* Can't uppercase digits XXX XXX XXX */
			ask = FALSE;

			i = choice - '0' + 26;
		}

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Save the spell index */
		power = powers[i];

		/* Verify it */
		if (ask)
		{
			char tmp_val[160];

			/* Prompt */
			strnfmt(tmp_val, 78, "Use %s? ", monster_powers[power].name);

			/* Belay that order */
			if (!get_check(tmp_val)) continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
	Term_load();
	character_icky = FALSE;

	/* Abort if needed */
	if (!flag)
	{
		return std::make_tuple(-1, num);
	}

	return std::make_tuple(power, num);
}


/*
 * Apply the effect of a monster power
 */
static void apply_monster_power(monster_race const *r_ptr, int power)
{
	assert(power >= 0);
	assert(power < MONSTER_POWERS_MAX);

	/* Shorthand */
	int const x = p_ptr->px;
	int const y = p_ptr->py;
	int const plev = p_ptr->lev;
	int const rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* 'Powerful' monsters have wider radii */
	int rad = (r_ptr->flags2 & RF2_POWERFUL)
			? 1 + (p_ptr->lev / 15)
			: 1 + (p_ptr->lev / 20);

	/* Analyse power */
	switch (power)
	{
		/**** RF4 (bit position) ****/

		/* SHRIEK */
	case 0:
		{
			aggravate_monsters( -1);

			break;
		}

		/* MULTIPLY */
	case 1:
		{
			do_cmd_wiz_named_friendly(p_ptr->body_monster, FALSE);

			break;
		}

		/* S_ANIMAL */
	case 2:
		{
			summon_specific_friendly(y, x, rlev, SUMMON_ANIMAL, TRUE);

			break;
		}

		/* ROCKET */
	case 3:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ROCKET, dir, p_ptr->lev * 12, 1 + (p_ptr->lev / 20));

			break;
		}

		/* ARROW_1 */
	case 4:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(1, 6));

			break;
		}

		/* ARROW_2 */
	case 5:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(3, 6));

			break;
		}

		/* ARROW_3 */
	case 6:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(5, 6));

			break;
		}

		/* ARROW_4 */
	case 7:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(7, 6));

			break;
		}

		/* BR_ACID */
	case 8:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_ELEC */
	case 9:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_FIRE */
	case 10:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_COLD */
	case 11:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_POIS */
	case 12:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_NETH */
	case 13:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_LITE */
	case 14:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_LITE, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_DARK */
	case 15:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DARK, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_CONF */
	case 16:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CONFUSION, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_SOUN */
	case 17:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SOUND, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_CHAO */
	case 18:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 7, rad);

			break;
		}

		/* BR_DISE */
	case 19:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISENCHANT, dir, p_ptr->lev * 7, rad);

			break;
		}

		/* BR_NEXU */
	case 20:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NEXUS, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_TIME */
	case 21:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_TIME, dir, p_ptr->lev * 3, rad);

			break;
		}

		/* BR_INER */
	case 22:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_INERTIA, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_GRAV */
	case 23:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_GRAVITY, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_SHAR */
	case 24:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SHARDS, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_PLAS */
	case 25:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_PLASMA, dir, p_ptr->lev * 3, rad);

			break;
		}

		/* BR_WALL */
	case 26:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FORCE, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_MANA */
	case 27:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BA_NUKE */
	case 28:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

		/* BR_NUKE */
	case 29:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

		/* BA_CHAO */
	case 30:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 4, 2);

			break;
		}

		/* BR_DISI */
	case 31:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISINTEGRATE, dir, p_ptr->lev * 5, 1 + (p_ptr->lev / 20));

			break;
		}


		/**** RF5 (bit position + 32) ****/

		/* BA_ACID */
	case 32:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, randint(p_ptr->lev * 6) + 20, 2);

			break;
		}

		/* BA_ELEC */
	case 33:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_FIRE */
	case 34:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, randint(p_ptr->lev * 7) + 20, 2);

			break;
		}

		/* BA_COLD */
	case 35:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_POIS */
	case 36:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, damroll(12, 2), 2);

			break;
		}

		/* BA_NETH */
	case 37:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

		/* BA_WATE */
	case 38:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_WATER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

		/* BA_MANA */
	case 39:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_DARK */
	case 40:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DARK, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* 41 DRAIN_MANA -- Not available */

		/* 42 MIND_BLAST -- Not available */

		/* 43 BRAIN_SMASH -- Not available */

		/* CAUSE_1 */
	case 44:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8));

			break;
		}

		/* CAUSE_2 */
	case 45:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(8, 8));

			break;
		}

		/* CAUSE_3 */
	case 46:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(10, 15));

			break;
		}

		/* CAUSE_4 */
	case 47:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(15, 15));

			break;
		}

		/* BO_ACID */
	case 48:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ACID, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_ELEC */
	case 49:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ELEC, dir, damroll(4, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_FIRE */
	case 50:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_FIRE, dir, damroll(9, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_COLD */
	case 51:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_COLD, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_POIS */
	case 52:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_POIS, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_NETH */
	case 53:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_NETHER, dir, damroll(5, 5) + (p_ptr->lev / 3));

			break;
		}

		/* BO_WATE */
	case 54:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_WATER, dir, damroll(10, 10) + (p_ptr->lev / 3));

			break;
		}

		/* BO_MANA */
	case 55:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_PLAS */
	case 56:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_PLASMA, dir, damroll(8, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_ICEE */
	case 57:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ICE, dir, damroll(6, 6) + (p_ptr->lev / 3));

			break;
		}

		/* MISSILE */
	case 58:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MISSILE, dir, damroll(2, 6) + (p_ptr->lev / 3));

			break;
		}

		/* SCARE */
	case 59:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fear_monster(dir, plev);

			break;
		}

		/* BLIND */
	case 60:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(1, 8) + (p_ptr->lev / 3));

			break;
		}

		/* CONF */
	case 61:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* SLOW */
	case 62:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_OLD_SLOW, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

		/* HOLD */
	case 63:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_OLD_SLEEP, dir, damroll(5, 8) + (p_ptr->lev / 3));

			break;
		}


		/**** RF6 (bit position + 64) ****/

		/* HASTE */
	case 64:
		{
			if (!p_ptr->fast)
			{
				(void)set_fast(randint(20 + (plev) ) + plev, 10);
			}
			else
			{
				(void)set_fast(p_ptr->fast + randint(5), 10);
			}

			break;
		}

		/* HAND_DOOM */
	case 65:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(10, 8) + (p_ptr->lev));

			break;
		}

		/* HEAL */
	case 66:
		{
			hp_player(damroll(8, 5));

			break;
		}

		/* S_ANIMALS */
	case 67:
		{
			for (int k = 0; k < 4; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANIMAL, TRUE);
			}

			break;
		}

		/* BLINK */
	case 68:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player(10);

			break;
		}

		/* TPORT */
	case 69:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player(plev * 5);

			break;
		}

		/* TELE_TO */
	case 70:
		{
			int ii, ij;

			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			msg_print("You go between.");

			if (!tgt_pt(&ii, &ij)) break;

			p_ptr->energy -= 60 - plev;

			if (!cave_empty_bold(ij, ii) ||
					(cave[ij][ii].info & CAVE_ICKY) ||
					(distance(ij, ii, p_ptr->py, p_ptr->px) > plev * 20 + 2))
			{
				msg_print("You fail to show the destination correctly!");
				p_ptr->energy -= 100;
				teleport_player(10);
			}
			else teleport_player_to(ij, ii);

			break;
		}

		/* TELE_AWAY */
	case 71:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			int dir;
			if (!get_aim_dir(&dir)) break;

			(void)fire_beam(GF_AWAY_ALL, dir, plev);

			break;
		}

		/* TELE_LEVEL */
	case 72:
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player_level();

			break;
		}

		/* DARKNESS */
	case 73:
		{
			(void)project( -1, 3, p_ptr->py, p_ptr->px, 0, GF_DARK_WEAK,
				       PROJECT_GRID | PROJECT_KILL);

			/* Unlite the room */
			unlite_room(p_ptr->py, p_ptr->px);

			break;
		}

		/* TRAPS */
	case 74:
		{
			trap_creation();

			break;
		}

		/* 75 FORGET -- Not available */

		/* ANIM_DEAD -- Use the same code as the nether spell */
	case 76:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_RAISE, dir, 1, 0);

			break;
		}

		/* 77 S_BUG -- Not available, well we do that anyway ;) */

		/* 78 S_RNG -- Not available, who dares? */

		/* S_THUNDERLORD */
	case 79:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_THUNDERLORD, TRUE);
			}

			break;
		}

		/* S_KIN -- Summon Kin, because we code bugs :) */
	case 80:
		{
			/* Big hack */
			summon_kin_type = r_ptr->d_char;

			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_KIN, TRUE);
			}

			break;
		}

		/* S_HI_DEMON */
	case 81:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DEMON, TRUE);
			}

			break;
		}

		/* S_MONSTER */
	case 82:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

		/* S_MONSTERS */
	case 83:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

		/* S_ANT */
	case 84:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANT, TRUE);
			}

			break;
		}

		/* S_SPIDER */
	case 85:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_SPIDER, TRUE);
			}

			break;
		}

		/* S_HOUND */
	case 86:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HOUND, TRUE);
			}

			break;
		}

		/* S_HYDRA */
	case 87:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HYDRA, TRUE);
			}

			break;
		}

		/* S_ANGEL */
	case 88:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANGEL, TRUE);
			}

			break;
		}

		/* S_DEMON */
	case 89:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DEMON, TRUE);
			}

			break;
		}

		/* S_UNDEAD */
	case 90:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_UNDEAD, TRUE);
			}

			break;
		}

		/* S_DRAGON */
	case 91:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DRAGON, TRUE);
			}

			break;
		}

		/* S_HI_UNDEAD */
	case 92:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_UNDEAD_NO_UNIQUES, TRUE);
			}

			break;
		}

		/* S_HI_DRAGON */
	case 93:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DRAGON_NO_UNIQUES, TRUE);
			}

			break;
		}

		/* S_WRAITH */
	case 94:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_WRAITH, TRUE);
			}

			break;
		}

		/* 95 S_UNIQUE -- Not available */
	}
}


/*
 * Use a power of the monster in symbiosis
 */
int use_symbiotic_power(int r_idx, bool great, bool no_cost)
{
	monster_race const *r_ptr = &r_info[r_idx];

	int power;
	int num;
	std::tie(power, num) = choose_monster_power(r_ptr, great, no_cost);

	// Early exit?
	if (power == 0) {
		// No powers available
		return 0;
	} else if (power < 0) {
		// Canceled by user
		energy_use = 0;
		return -1;
	}

	/* Apply the effect */
	apply_monster_power(r_ptr, power);

	/* Take some SP */
	if (!no_cost)
	{
		int chance = (monster_powers[power].mana + r_ptr->level);
		int pchance = adj_str_wgt[p_ptr->stat_ind[A_WIS]] / 2 + get_skill(SKILL_POSSESSION);

		if (rand_int(chance) >= pchance)
		{
			int m = monster_powers[power].mana / 10;

			if (m > p_ptr->msp) m = p_ptr->msp;
			if (!m) m = 1;

			p_ptr->csp -= m;
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return (num);
}

/*
 * Schooled magic
 */

/*
 * Find a spell in any books/objects
 */
static int hack_force_spell = -1;
static s32b hack_force_spell_pval = -1;

boost::optional<int> get_item_hook_find_spell(object_filter_t const &)
{
	char buf[80];
	strcpy(buf, "Manathrust");
	if (!get_string("Spell name? ", buf, 79))
	{
		return boost::none;
	}

	int const spell = find_spell(buf);
	if (spell == -1)
	{
		return boost::none;
	}

	for (int i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Extract object flags */
		u32b f1, f2, f3, f4, f5, esp;
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Must we wield it to cast from it? */
		if ((wield_slot(o_ptr) != -1) && (i < INVEN_WIELD) && (f5 & TR5_WIELD_CAST))
		{
			continue;
		}

		/* Is it a non-book? */
		if (!is_school_book()(o_ptr))
		{
			/* Does it contain the appropriate spell? */
			if ((f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 == spell))
			{
				hack_force_spell = spell;
				hack_force_spell_pval = o_ptr->pval;
				return i;
			}
		}
		/* A random book ? */
		else if (school_book_contains_spell(o_ptr->sval, spell))
		{
			hack_force_spell = spell;
			hack_force_spell_pval = o_ptr->pval;
			return i;
		}
	}

	return boost::none;
}

/*
 * Is the spell castable?
 */
bool_ is_ok_spell(s32b spell_idx, s32b pval)
{
	spell_type *spell = spell_at(spell_idx);

	// Calculate availability based on caster's skill level.
	s32b level;
	bool_ na;
	get_level_school(spell, 50, 0, &level, &na);
	if (na || (level == 0))
	{
		return FALSE;
	}
	// Are we permitted to cast based on item pval? Only music
	// spells have non-zero minimum PVAL.
	if (pval < spell_type_minimum_pval(spell))
	{
		return FALSE;
	}
	// OK, we're permitted to cast it.
	return TRUE;
}


/*
 * Get a spell from a book
 */
s32b get_school_spell(cptr do_what, s16b force_book)
{
	int i, item;
	s32b spell = -1;
	int num = 0;
	s32b where = 1;
	int ask;
	bool_ flag;
	char out_val[160];
	object_type *o_ptr, forge;
	int tmp;
	int sval, pval;
	u32b f1, f2, f3, f4, f5, esp;

	hack_force_spell = -1;
	hack_force_spell_pval = -1;

	/* Ok do we need to ask for a book ? */
	if (!force_book)
	{
		char buf2[40];
		char buf3[40];
		sprintf(buf2, "You have no book to %s from", do_what);
		sprintf(buf3, "%s from which book?", do_what);

		if (!get_item(&item,
			      buf3,
			      buf2,
			      USE_INVEN | USE_EQUIP,
			      hook_school_spellable(),
			      get_item_hook_find_spell))
		{
			return -1;
		}

		/* Get the item */
		o_ptr = get_object(item);

		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* If it can be wielded, it must */
		if ((wield_slot(o_ptr) != -1) && (item < INVEN_WIELD) && (f5 & TR5_WIELD_CAST))
		{
			msg_format("You cannot %s from that object; it must be wielded first.", do_what);
			return -1;
		}
	}
	else
	{
		o_ptr = &forge;
		o_ptr->tval = TV_BOOK;
		o_ptr->sval = force_book;
		o_ptr->pval = 0;
	}

	if (repeat_pull(&tmp))
	{
		return tmp;
	}

	/* Nothing chosen yet */
	flag = FALSE;

	/* Show choices */
	window_stuff();

	/* No spell to cast by default */
	spell = -1;

	/* Is it a random book, or something else ? */
	if (is_school_book()(o_ptr))
	{
		sval = o_ptr->sval;
		pval = o_ptr->pval;
	}
	else
	{
		sval = 255;
		pval = o_ptr->pval2;
	}

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Go */
	if (hack_force_spell == -1)
	{
		num = school_book_length(sval);

		/* Build a prompt (accept all spells) */
		strnfmt(out_val, 78, "(Spells %c-%c, Descs %c-%c, ESC=exit) %^s which spell? ",
		        I2A(0), I2A(num - 1), I2A(0) - 'a' + 'A', I2A(num - 1) - 'a' + 'A', do_what);

		/* Get a spell from the user */
		while (!flag)
		{
			char choice;

			/* Restore and save screen; this prevents
			   subprompt from leaving garbage when going
			   around the loop multiple times. */
			Term_load();
			Term_save();

			/* Display a list of spells */
			where = print_book(sval, pval, o_ptr);

			/* Input */
			if (!get_com(out_val, &choice))
			{
				flag = FALSE;
				break;
			}

			/* Note verify */
			ask = (isupper(choice));

			/* Lowercase */
			if (ask) choice = tolower(choice);

			/* Extract request */
			i = (islower(choice) ? A2I(choice) : -1);

			/* Totally Illegal */
			if ((i < 0) || (i >= num))
			{
				bell();
				continue;
			}

			/* Verify it */
			if (ask)
			{
				/* Display a list of spells */
				where = print_book(sval, pval, o_ptr);
				print_spell_desc(spell_x(sval, pval, i), where);
			}
			else
			{
				bool_ ok;

				/* Save the spell index */
				spell = spell_x(sval, pval, i);

				/* Do we need to do some pre test */
				ok = is_ok_spell(spell, o_ptr->pval);

				/* Require "okay" spells */
				if (!ok)
				{
					bell();
					msg_format("You may not %s that spell.", do_what);
					spell = -1;
					continue;
				}

				/* Stop the loop */
				flag = TRUE;
			}
		}
	}
	else
	{
		bool_ ok;

		/* Require "okay" spells */
		ok = is_ok_spell(hack_force_spell, hack_force_spell_pval);
		if (ok)
		{
			flag = TRUE;
			spell = hack_force_spell;
		}
		else
		{
			bell();
			msg_format("You may not %s that spell.", do_what);
			spell = -1;
		}
	}


	/* Restore the screen */
	Term_load();
	character_icky = FALSE;


	/* Show choices */
	window_stuff();


	/* Abort if needed */
	if (!flag) return -1;

	tmp = spell;
	repeat_push(tmp);
	return spell;
}

void cast_school_spell()
{
	int spell;

	/* No magic */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}

	/* No magic */
	if (p_ptr->anti_magic)
	{
		msg_print("Your anti-magic shell disrupts any magic attempts.");
		return;
	}

	spell = get_school_spell("cast", 0);

	/* Actualy cast the choice */
	if (spell != -1)
	{
		lua_cast_school_spell(spell, FALSE);
	}
}

/* Can it contains a schooled spell ? */
static bool hook_school_can_spellable(object_type const *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	return ((f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 == -1));
}

/*
 * Copy a spell from a bok to an object
 */
void do_cmd_copy_spell()
{
	int spell = get_school_spell("copy", 0);
	int item;

	if (spell == -1) return;

	/* Spells that cannot be randomly created cannot be copied */
	if (spell_type_random_type(spell_at(spell)) <= 0)
	{
		msg_print("This spell cannot be copied.");
		return;
	}

	if (!get_item(&item,
		      "Copy to which object? ",
		      "You have no object to copy to.",
		      (USE_INVEN | USE_EQUIP),
		      hook_school_can_spellable)) return;
	object_type *o_ptr = get_object(item);

	msg_print("You copy the spell!");
	o_ptr->pval2 = spell;
	inven_item_describe(item);
}
