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
#include "dungeon_flag.hpp"
#include "game.hpp"
#include "lua_bind.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
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
			HasFlags(TR_SPELL_CONTAIN),
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
bool is_magestaff()
{
	int i = 0;

	while (p_ptr->body_parts[i] == INVEN_WIELD)
	{
		object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD + i];

		/* Wielding a mage staff */
		if ((o_ptr->k_ptr) && (o_ptr->tval == TV_MSTAFF))
		{
			return true;
		}

		/* Next slot */
		i++;

		/* Paranoia */
		if (i >= (INVEN_TOTAL - INVEN_WIELD))
		{
			break;
		}
	}

	/* Not wielding a mage staff */
	return false;
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
		char label[8];

		if (is_ok_spell(spell_idx, obj->pval))
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

void do_cmd_browse_aux(object_type *o_ptr)
{
	auto const flags = object_flags(o_ptr);

	if (is_school_book()(o_ptr))
	{
		browse_school_spell(o_ptr->sval, o_ptr->pval, o_ptr);
	}
	else if ((flags & TR_SPELL_CONTAIN) && (o_ptr->pval2 != -1))
	{
		browse_school_spell(255, o_ptr->pval2, o_ptr);
	}
}

void do_cmd_browse()
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

void do_poly_self()
{
	auto const &race_info = game->edit_data.race_info;

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
					dec_stat(tmp, randint(6) + 6, (rand_int(3) == 0));
					power -= 1;
				}
				tmp++;
			}

			/* Deformities are discriminated against! */
			dec_stat(A_CHR, randint(6), TRUE);

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
		while (true)
		{
			new_race = rand_int(race_info.size());
			expfact = race_info[new_race].ps.exp;

			if ((new_race != p_ptr->prace) && (expfact <= goalexpfact))
			{
				break;
			}
		}

		if (effect_msg[0])
		{
			msg_format("You turn into a%s %s!",
				   (is_a_vowel(race_info[new_race].title[0]) ? "n" : ""),
				   race_info[new_race].title.c_str());
		}
		else
		{
			msg_format("You turn into a %s %s!", effect_msg,
				   race_info[new_race].title.c_str());
		}

		p_ptr->prace = new_race;
		rp_ptr = &race_info[p_ptr->prace];

		/* Experience factor */
		p_ptr->expfact = rp_ptr->ps.exp + rmp_ptr->ps.exp + cp_ptr->ps.exp;

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
			dec_stat(tmp, randint(6) + 6, (rand_int(3) == 0));
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
void fetch(int dir, int wgt, bool require_los)
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

		while (true)
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
 * Return the symbiote's name or description.
 */
std::string symbiote_name(bool capitalize)
{
	auto const &r_info = game->edit_data.r_info;

	object_type *o_ptr = &p_ptr->inventory[INVEN_CARRY];

	std::string buf;
	buf.reserve(32);

	// Fallback; shouldn't ever be necessary
	if (!o_ptr->k_ptr)
	{
		buf += "A non-existent symbiote";
	}
	else
	{
		auto r_ptr = &r_info[o_ptr->pval];
		std::size_t i = 0;

		if (r_ptr->flags & RF_UNIQUE)
		{
			// Unique monster; no preceding "your" and ignore name
			buf += r_ptr->name;
		}
		else if ((i = o_ptr->inscription.find("#named ")) != std::string::npos)
		{
			// We've named it; extract the name */
			buf += o_ptr->inscription.substr(i);
		}
		else
		{
			// No special cases; just return "Your <monster type>".
			buf += "your ";
			buf += r_ptr->name;
		}
	}

	// Capitalize?
	if (capitalize)
	{
		buf[0] = toupper(buf[0]);
	}

	// Done
	return buf;
}


/*
 * Find monster power
 */
monster_power const *lookup_monster_power(std::size_t idx)
{
	for (auto const &p: monster_powers)
	{
		if (p.monster_spell_index == idx)
		{
			return &p;
		}
	}
	return nullptr;
}


/*
 * Extract powers
 */
std::vector<monster_power const *> extract_monster_powers(monster_race const *r_ptr, bool great)
{
	std::vector<monster_power const *> powers;
	powers.reserve(MONSTER_POWERS_MAX);

	for (std::size_t i = 0; i < monster_spell_flag_set::nbits; i++)
	{
		if (r_ptr->spells.bit(i))
		{
			if (auto power = lookup_monster_power(i))
			{
				if (power->great && (!great))
				{
					continue;
				}
				powers.push_back(power);
			}
		}
	}

	return powers;
}

/**
 * Calculate mana required for a given monster power.
 */
static int calc_monster_spell_mana(monster_power const *mp_ptr)
{
	int mana = mp_ptr->mana / 10;
	if (mana > p_ptr->msp) mana = p_ptr->msp;
	if (!mana) mana = 1;
	return mana;
}

/**
 * Choose a monster power
 */
static std::tuple<int, int> choose_monster_power(monster_race const *r_ptr, bool great, bool symbiosis)
{
	/* Extract available monster powers */
	auto powers = extract_monster_powers(r_ptr, great);
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
		label, (symbiosis ? "symbiote" : "body"));

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Get a spell from the user */
	monster_power const *power = nullptr;
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
				monster_power const *mp_ptr = powers[ctr];

				label = (ctr < 26) ? I2A(ctr) : I2D(ctr - 26);

				byte color = TERM_L_GREEN;
				if (!symbiosis)
				{
					int mana = calc_monster_spell_mana(mp_ptr);
					strnfmt(dummy, 80, " %c) %2d %s",
						label, mana, mp_ptr->name);
					// Gray out if player doesn't have enough mana to cast.
					if (mana > p_ptr->csp) {
						color = TERM_L_DARK;
					}
				}
				else
				{
					strnfmt(dummy, 80, " %c) %s",
						label, mp_ptr->name);
				}

				if (ctr < 17)
				{
					c_prt(color, dummy, y + ctr, x);
				}
				else
				{
					c_prt(color, dummy, y + ctr - 17, x + 40);
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

		/* Save the spell */
		power = powers[i];

		/* Make sure it's actually possible for the player to cast */
		if (!symbiosis)
		{
			if (p_ptr->csp < calc_monster_spell_mana(power))
			{
				bell();
				continue;
			}
		}

		/* Verify it */
		if (ask)
		{
			char tmp_val[160];

			/* Prompt */
			strnfmt(tmp_val, 78, "Use %s? ", power->name);

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
	if (!flag || (power == nullptr))
	{
		return std::make_tuple(-1, num);
	}

	return std::make_tuple(power->monster_spell_index, num);
}


/*
 * Apply the effect of a monster power
 */
static void apply_monster_power(monster_race const *r_ptr, std::size_t monster_spell_idx)
{
	auto const &dungeon_flags = game->dungeon_flags;

	assert(monster_spell_idx < monster_spell_flag_set::nbits);

	/* Shorthand */
	int const x = p_ptr->px;
	int const y = p_ptr->py;
	int const plev = p_ptr->lev;
	int const rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* 'Powerful' monsters have wider radii */
	int rad = (r_ptr->flags & RF_POWERFUL)
			? 1 + (p_ptr->lev / 15)
			: 1 + (p_ptr->lev / 20);

	/* Analyse power */
	switch (monster_spell_idx)
	{
	case SF_SHRIEK_IDX:
		{
			aggravate_monsters( -1);

			break;
		}

	case SF_MULTIPLY_IDX:
		{
			do_cmd_wiz_named_friendly(p_ptr->body_monster, FALSE);

			break;
		}

	case SF_S_ANIMAL_IDX:
		{
			summon_specific_friendly(y, x, rlev, SUMMON_ANIMAL, TRUE);

			break;
		}

	case SF_ROCKET_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ROCKET, dir, p_ptr->lev * 12, 1 + (p_ptr->lev / 20));

			break;
		}

	case SF_ARROW_1_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(1, 6));

			break;
		}

	case SF_ARROW_2_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(3, 6));

			break;
		}

	case SF_ARROW_3_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(5, 6));

			break;
		}

	case SF_ARROW_4_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(7, 6));

			break;
		}

	case SF_BR_ACID_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_ELEC_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_FIRE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_COLD_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_POIS_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_NETH_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_LITE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_LITE, dir, p_ptr->lev * 8, rad);

			break;
		}

	case SF_BR_DARK_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DARK, dir, p_ptr->lev * 8, rad);

			break;
		}

	case SF_BR_CONF_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CONFUSION, dir, p_ptr->lev * 8, rad);

			break;
		}

	case SF_BR_SOUN_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SOUND, dir, p_ptr->lev * 8, rad);

			break;
		}

	case SF_BR_CHAO_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 7, rad);

			break;
		}

	case SF_BR_DISE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISENCHANT, dir, p_ptr->lev * 7, rad);

			break;
		}

	case SF_BR_NEXU_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NEXUS, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BR_TIME_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_TIME, dir, p_ptr->lev * 3, rad);

			break;
		}

	case SF_BR_INER_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_INERTIA, dir, p_ptr->lev * 4, rad);

			break;
		}

	case SF_BR_GRAV_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_GRAVITY, dir, p_ptr->lev * 4, rad);

			break;
		}

	case SF_BR_SHAR_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SHARDS, dir, p_ptr->lev * 8, rad);

			break;
		}

	case SF_BR_PLAS_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_PLASMA, dir, p_ptr->lev * 3, rad);

			break;
		}

	case SF_BR_WALL_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FORCE, dir, p_ptr->lev * 4, rad);

			break;
		}

	case SF_BR_MANA_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, p_ptr->lev * 5, rad);

			break;
		}

	case SF_BA_NUKE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

	case SF_BR_NUKE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

	case SF_BA_CHAO_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 4, 2);

			break;
		}

	case SF_BR_DISI_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISINTEGRATE, dir, p_ptr->lev * 5, 1 + (p_ptr->lev / 20));

			break;
		}

	case SF_BA_ACID_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, randint(p_ptr->lev * 6) + 20, 2);

			break;
		}

	case SF_BA_ELEC_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

	case SF_BA_FIRE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, randint(p_ptr->lev * 7) + 20, 2);

			break;
		}

	case SF_BA_COLD_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

	case SF_BA_POIS_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, damroll(12, 2), 2);

			break;
		}

	case SF_BA_NETH_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

	case SF_BA_WATE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_WATER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

	case SF_BA_MANA_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

	case SF_BA_DARK_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DARK, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

	case SF_CAUSE_1_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8));

			break;
		}

	case SF_CAUSE_2_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(8, 8));

			break;
		}

	case SF_CAUSE_3_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(10, 15));

			break;
		}

	case SF_CAUSE_4_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(15, 15));

			break;
		}

	case SF_BO_ACID_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ACID, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_ELEC_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ELEC, dir, damroll(4, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_FIRE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_FIRE, dir, damroll(9, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_COLD_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_COLD, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_POIS_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_POIS, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_NETH_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_NETHER, dir, damroll(5, 5) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_WATE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_WATER, dir, damroll(10, 10) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_MANA_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_PLAS_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_PLASMA, dir, damroll(8, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_BO_ICEE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ICE, dir, damroll(6, 6) + (p_ptr->lev / 3));

			break;
		}

	case SF_MISSILE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MISSILE, dir, damroll(2, 6) + (p_ptr->lev / 3));

			break;
		}

	case SF_SCARE_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fear_monster(dir, plev);

			break;
		}

	case SF_BLIND_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(1, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_CONF_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_SLOW_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_OLD_SLOW, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_HOLD_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_OLD_SLEEP, dir, damroll(5, 8) + (p_ptr->lev / 3));

			break;
		}

	case SF_HASTE_IDX:
		{
			if (!p_ptr->fast)
			{
				set_fast(randint(20 + (plev) ) + plev, 10);
			}
			else
			{
				set_fast(p_ptr->fast + randint(5), 10);
			}

			break;
		}

	case SF_HAND_DOOM_IDX:
		{
			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(10, 8) + (p_ptr->lev));

			break;
		}

	case SF_HEAL_IDX:
		{
			hp_player(damroll(8, 5));

			break;
		}

	case SF_S_ANIMALS_IDX:
		{
			for (int k = 0; k < 4; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANIMAL, TRUE);
			}

			break;
		}

	case SF_BLINK_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player(10);

			break;
		}

	case SF_TPORT_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player(plev * 5);

			break;
		}

	case SF_TELE_TO_IDX:
		{
			int ii, ij;

			if (dungeon_flags & DF_NO_TELEPORT)
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

	case SF_TELE_AWAY_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			int dir;
			if (!get_aim_dir(&dir)) break;

			fire_beam(GF_AWAY_ALL, dir, plev);

			break;
		}

	case SF_TELE_LEVEL_IDX:
		{
			if (dungeon_flags & DF_NO_TELEPORT)
			{
				msg_print("No teleport on special levels...");
				break;
			}

			teleport_player_level();

			break;
		}

	case SF_DARKNESS_IDX:
		{
			project( -1, 3, p_ptr->py, p_ptr->px, 0, GF_DARK_WEAK,
				       PROJECT_GRID | PROJECT_KILL);

			/* Unlite the room */
			unlite_room(p_ptr->py, p_ptr->px);

			break;
		}

	case SF_S_THUNDERLORD_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_THUNDERLORD, TRUE);
			}

			break;
		}

	case SF_S_KIN_IDX:
		{
			/* Big hack */
			summon_kin_type = r_ptr->d_char;

			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_KIN, TRUE);
			}

			break;
		}

	case SF_S_HI_DEMON_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DEMON, TRUE);
			}

			break;
		}

	case SF_S_MONSTER_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

	case SF_S_MONSTERS_IDX:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

	case SF_S_ANT_IDX:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANT, TRUE);
			}

			break;
		}

	case SF_S_SPIDER_IDX:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_SPIDER, TRUE);
			}

			break;
		}

	case SF_S_HOUND_IDX:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HOUND, TRUE);
			}

			break;
		}

	case SF_S_HYDRA_IDX:
		{
			for (int k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HYDRA, TRUE);
			}

			break;
		}

	case SF_S_ANGEL_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANGEL, TRUE);
			}

			break;
		}

	case SF_S_DEMON_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DEMON, TRUE);
			}

			break;
		}

	case SF_S_UNDEAD_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_UNDEAD, TRUE);
			}

			break;
		}

	case SF_S_DRAGON_IDX:
		{
			for (int k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DRAGON, TRUE);
			}

			break;
		}

	case SF_S_HI_UNDEAD_IDX:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_UNDEAD_NO_UNIQUES, TRUE);
			}

			break;
		}

	case SF_S_HI_DRAGON_IDX:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DRAGON_NO_UNIQUES, TRUE);
			}

			break;
		}

	case SF_S_WRAITH_IDX:
		{
			for (int k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_WRAITH, TRUE);
			}

			break;
		}
	}
}


/*
 * Use a monster power and call the given callback.
 */
static int use_monster_power_aux(monster_race const *r_ptr, bool great, bool symbiosis, std::function<void(monster_power const *power)> f)
{
	int power;
	int num;
	std::tie(power, num) = choose_monster_power(r_ptr, great, symbiosis);

	// Early exit?
	if (power == 0) {
		// No powers available
		return 0;
	} else if (power < 0) {
		// Canceled by user
		energy_use = 0;
		return -1;
	}

	// Apply the effect
	apply_monster_power(r_ptr, power);

	// Post-processing
	f(&monster_powers[power]);

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	return (num);
}

/**
 * Use a power of the monster in symbiosis
 */
int use_symbiotic_power(int r_idx, bool great)
{
	auto const &r_info = game->edit_data.r_info;

	monster_race const *r_ptr = &r_info[r_idx];
	return use_monster_power_aux(r_ptr, great, true, [](monster_power const *) {
		// Don't need to do anything post-cast.
	});
}

/**
 * Use a power of a possessed body.
 */
void use_monster_power(int r_idx, bool great)
{
	auto const &r_info = game->edit_data.r_info;

	monster_race const *r_ptr = &r_info[r_idx];
	use_monster_power_aux(r_ptr, great, false, [r_ptr](monster_power const *power) {
		// Sometimes give a free cast.
		int chance = (power->mana + r_ptr->level);
		int pchance = adj_str_wgt[p_ptr->stat_ind[A_WIS]] / 2 + get_skill(SKILL_POSSESSION);
		if (rand_int(chance) >= pchance)
		{
			p_ptr->csp -= calc_monster_spell_mana(power);
		}
	});
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

	auto spell_idx = find_spell(buf);
	if (!spell_idx)
	{
		return boost::none;
	}
	int const spell = *spell_idx;

	for (int i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Extract object flags */
		auto const flags = object_flags(o_ptr);

		/* Must we wield it to cast from it? */
		if ((wield_slot(o_ptr) != -1) && (i < INVEN_WIELD) && (flags & TR_WIELD_CAST))
		{
			continue;
		}

		/* Is it a non-book? */
		if (!is_school_book()(o_ptr))
		{
			/* Does it contain the appropriate spell? */
			if ((flags & TR_SPELL_CONTAIN) && (o_ptr->pval2 == spell))
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
bool is_ok_spell(s32b spell_idx, s32b pval)
{
	spell_type *spell = spell_at(spell_idx);

	// Calculate availability based on caster's skill level.
	s32b level;
	bool na;
	get_level_school(spell, 50, 0, &level, &na);
	if (na || (level == 0))
	{
		return false;
	}
	// Are we permitted to cast based on item pval? Only music
	// spells have non-zero minimum PVAL.
	if (pval < spell_type_minimum_pval(spell))
	{
		return false;
	}
	// OK, we're permitted to cast it.
	return true;
}


/*
 * Get a spell from a book
 */
s32b get_school_spell(const char *do_what, s16b force_book)
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

		auto const f = object_flags(o_ptr);

		/* If it can be wielded, it must */
		if ((wield_slot(o_ptr) != -1) && (item < INVEN_WIELD) && (f & TR_WIELD_CAST))
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
				/* Save the spell index */
				spell = spell_x(sval, pval, i);

				/* Require "okay" spells */
				if (!is_ok_spell(spell, o_ptr->pval))
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
		/* Require "okay" spells */
		if (is_ok_spell(hack_force_spell, hack_force_spell_pval))
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
	auto const f = object_flags(o_ptr);

	return ((f & TR_SPELL_CONTAIN) && (o_ptr->pval2 == -1));
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
