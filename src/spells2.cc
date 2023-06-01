/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "spells2.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "cmd7.hpp"
#include "dungeon_flag.hpp"
#include "dungeon_info_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hook_identify_in.hpp"
#include "hooks.hpp"
#include "melee2.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "monster_type.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_race_flag.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells3.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <chrono>
#include <sstream>
#include <thread>
#include <vector>

using boost::algorithm::iequals;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;

#define WEIRD_LUCK      12
#define BIAS_LUCK       20
/*
 * Bias luck needs to be higher than weird luck,
 * since it is usually tested several times...
 */

static void summon_dragon_riders();


/*
 * Grow things
 */
void grow_things(s16b type, int rad)
{
	int a, i, j;

	for (a = 0; a < rad * rad + 11; a++)
	{
		i = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;
		j = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;

		if (!in_bounds(p_ptr->py + j, p_ptr->px + i)) continue;
		if (distance(p_ptr->py, p_ptr->px, p_ptr->py + j, p_ptr->px + i) > rad) continue;

		if (cave_clean_bold(p_ptr->py + j, p_ptr->px + i))
		{
			cave_set_feat(p_ptr->py + j, p_ptr->px + i, type);
		}
	}
}

/*
 * Grow trees
 */
void grow_trees(int rad)
{
	auto const &f_info = game->edit_data.f_info;

	int a, i, j;

	for (a = 0; a < rad * rad + 11; a++)
	{
		i = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;
		j = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;

		if (!in_bounds(p_ptr->py + j, p_ptr->px + i)) continue;
		if (distance(p_ptr->py, p_ptr->px, p_ptr->py + j, p_ptr->px + i) > rad) continue;

		if (cave_clean_bold(p_ptr->py + j, p_ptr->px + i) && (f_info[cave[p_ptr->py][p_ptr->px].feat].flags & FF_SUPPORT_GROWTH))
		{
			cave_set_feat(p_ptr->py + j, p_ptr->px + i, FEAT_TREES);
		}
	}
}

/*
 * Grow grass
 */
void grow_grass(int rad)
{
	auto const &f_info = game->edit_data.f_info;

	int a, i, j;

	for (a = 0; a < rad * rad + 11; a++)
	{
		i = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;
		j = (rand_int((rad * 2) + 1)-rad + rand_int((rad * 2) + 1)-rad) / 2;

		if (!in_bounds(p_ptr->py + j, p_ptr->px + i)) continue;
		if (distance(p_ptr->py, p_ptr->px, p_ptr->py + j, p_ptr->px + i) > rad) continue;

		if (cave_clean_bold(p_ptr->py + j, p_ptr->px + i) && (f_info[cave[p_ptr->py][p_ptr->px].feat].flags & FF_SUPPORT_GROWTH))
		{
			cave_set_feat(p_ptr->py + j, p_ptr->px + i, FEAT_GRASS);
		}
	}
}

/*
 * Increase players hit points, notice effects
 */
void hp_player(int num)
{
	bool dead = p_ptr->chp < 0;

	/* Healing needed */
	if (p_ptr->chp < p_ptr->mhp)
	{
		/* Gain hitpoints */
		p_ptr->chp += num;

		/* Enforce maximum */
		if ((p_ptr->chp >= p_ptr->mhp) ||
		                /* prevent wrapping */
		                (!dead && (p_ptr->chp < 0)))
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Redraw */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Heal 0-4 */
		if (num < 5)
		{
			msg_print("You feel a little better.");
		}

		/* Heal 5-14 */
		else if (num < 15)
		{
			msg_print("You feel better.");
		}

		/* Heal 15-34 */
		else if (num < 35)
		{
			msg_print("You feel much better.");
		}

		/* Heal 35+ */
		else
		{
			msg_print("You feel very good.");
		}
	}
}



/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph()
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(p_ptr->py, p_ptr->px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a glyph */
	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_GLYPH);
}

void explosive_rune()
{
	/* XXX XXX XXX */
	if (!cave_clean_bold(p_ptr->py, p_ptr->px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	/* Create a glyph */
	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_MINOR_GLYPH);
}



/*
 * Array of stat "descriptions"
 */
static const char *desc_stat_pos[] =
{
	"strong",
	"smart",
	"wise",
	"dextrous",
	"healthy",
	"cute"
};

/*
 * Array of long descriptions of stat
 */

static const char *long_desc_stat[] =
{
	"strength",
	"intelligence",
	"wisdom",
	"dexterity",
	"constitution",
	"charisma"
};

/*
 * Array of stat "descriptions"
 */
static const char *desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};


/*
 * Lose a "point"
 */
void do_dec_stat(int stat, int mode)
{
	bool sust = false;

	/* Access the "sustain" */
	switch (stat)
	{
	case A_STR:
		if (p_ptr->sustain_str) sust = true;
		break;
	case A_INT:
		if (p_ptr->sustain_int) sust = true;
		break;
	case A_WIS:
		if (p_ptr->sustain_wis) sust = true;
		break;
	case A_DEX:
		if (p_ptr->sustain_dex) sust = true;
		break;
	case A_CON:
		if (p_ptr->sustain_con) sust = true;
		break;
	case A_CHR:
		if (p_ptr->sustain_chr) sust = true;
		break;
	}

	/* Sustain */
	if (sust)
	{
		/* Message */
		msg_format("You feel %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);
		return;
	}

	/* Attempt to reduce the stat */
	if (dec_stat(stat, 10, mode))
	{
		/* Message */
		msg_format("You feel very %s.", desc_stat_neg[stat]);
		return;
	}
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int stat, bool full)
{
	/* Keep a copy of the current stat, so we can evaluate it if necessary */
	int cur_stat = p_ptr->stat_cur[stat];

	/* Attempt to increase */
	if (res_stat(stat, full))
	{
		/* Message, depending on whether we got stronger or weaker */
		if (cur_stat > p_ptr->stat_max[stat])
		{
			msg_format("You feel your %s boost drain away.", long_desc_stat[stat]);
		}
		else
		{
			msg_format("You feel less %s.", desc_stat_neg[stat]);
		}

		/* Notice */
		return true;
	}

	/* Nothing obvious */
	return false;
}


/*
 * Increases a stat by one randomized level             -RAK-
 *
 * Note that this function (used by stat potions) now restores
 * the stat BEFORE increasing it.
 */
static bool inc_stat(int stat)
{
	int value, gain;

	/* Then augment the current/max stat */
	value = p_ptr->stat_cur[stat];

	/* Cannot go above 18/100 */
	if (value < 18 + 100)
	{
		/* Gain one (sometimes two) points */
		if (value < 18)
		{
			gain = ((rand_int(100) < 75) ? 1 : 2);
			value += gain;
		}

		/* Gain 1/6 to 1/3 of distance to 18/100 */
		else if (value < 18 + 98)
		{
			/* Approximate gain value */
			gain = (((18 + 100) - value) / 2 + 3) / 2;

			/* Paranoia */
			if (gain < 1) gain = 1;

			/* Apply the bonus */
			value += randint(gain) + gain / 2;

			/* Maximal value */
			if (value > 18 + 99) value = 18 + 99;
		}

		/* Gain one point at a time */
		else
		{
			value++;
		}

		/* Save the new value */
		p_ptr->stat_cur[stat] = value;

		/* Bring up the maximum too */
		if (value > p_ptr->stat_max[stat])
		{
			p_ptr->stat_max[stat] = value;
		}

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return true;
	}

	/* Nothing to gain */
	return false;
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int stat)
{
	bool res;

	/* Restore strength */
	res = res_stat(stat, true);

	/* Attempt to increase */
	if (inc_stat(stat))
	{
		/* Message */
		msg_format("Wow!  You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return true;
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return true;
	}

	/* Nothing obvious */
	return false;
}


/*
 * Process all identify hooks
 */
void identify_hooks(object_type *o_ptr)
{
	/* Process the appropriate hooks */
	hook_identify_in in = { o_ptr };
	process_hooks_new(HOOK_IDENTIFY, &in, NULL);
}


/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
bool identify_pack()
{
	/* Simply identify and know every item */
	for (int i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);
	}

	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	return true;
}

/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack_fully()
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		object_aware(o_ptr);
		object_known(o_ptr);
	}

	p_ptr->update |= (PU_BONUS);
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
}

/*
 * Used by the "enchant" function (chance of failure)
 * (modified for Zangband, we need better stuff there...) -- TY
 */
static int enchant_table[16] =
{
	0, 10, 50, 100, 200,
	300, 400, 500, 650, 800,
	950, 987, 993, 995, 998,
	1000
};

static bool remove_curse_object(object_type *o_ptr, bool all)
{
	/* Skip non-objects */
	if (!o_ptr->k_ptr) return false;

	/* Uncursed already */
	if (!cursed_p(o_ptr)) return false;

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);

	/* Heavily Cursed Items need a special spell */
	if (!all && (flags & TR_HEAVY_CURSE)) return false;

	/* Perma-Cursed Items can NEVER be uncursed */
	if (flags & TR_PERMA_CURSE) return false;

	/* Strip curse flags */
	o_ptr->art_flags &= ~TR_CURSED;
	o_ptr->art_flags &= ~TR_HEAVY_CURSE;

	/* Reverse the curse effect */
	/* jk - scrolls of *remove curse* have a 1 in (55-level chance to */
	/* reverse the curse effects - a ring of damage(-15) {cursed} then */
	/* becomes a ring of damage (+15) */
	/* this does not go for artifacts - a Sword of Mormegil +40,+60 would */
	/* be somewhat unbalancing */
	/* due to the nature of this procedure, it only works on cursed items */
	/* ie you get only one chance! */
	if ((randint(55-p_ptr->lev) == 1) && !artifact_p(o_ptr))
	{
		if (o_ptr->to_a < 0) o_ptr->to_a = -o_ptr->to_a;
		if (o_ptr->to_h < 0) o_ptr->to_h = -o_ptr->to_h;
		if (o_ptr->to_d < 0) o_ptr->to_d = -o_ptr->to_d;
		if (o_ptr->pval < 0) o_ptr->pval = -o_ptr->pval;
	}

	/* Recalculate the bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP);

	return true;
}

/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is false, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int all)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		if (!remove_curse_object(o_ptr, all)) continue;

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse()
{
	return (remove_curse_aux(false) ? true : false);
}

/*
 * Remove all curses
 */
bool remove_all_curse()
{
	return (remove_curse_aux(true) ? true : false);
}



/*
 * Restores any drained experience
 */
bool restore_level()
{
	/* Restore experience */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Message */
		msg_print("You feel your life energies returning.");

		/* Restore the experience */
		p_ptr->exp = p_ptr->max_exp;

		/* Check the experience */
		check_experience();

		/* Did something */
		return true;
	}

	/* No effect */
	return false;
}


void alchemy()
{
	int item, amt = 1;
	int old_number;
	long price;
	bool force = false;
	char o_name[80];
	char out_val[160];

	/* Hack -- force destruction */
	if (command_arg > 0) force = true;

	/* Get an item */
	if (!get_item(&item,
		      "Turn which item to gold? ",
		      "You have nothing to turn to gold.",
		      (USE_INVEN | USE_FLOOR),
		      object_filter::True()))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0)
		{
			return;
		}
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, true, 3);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		/* Make a verification */
		sprintf(out_val, "Really turn %s to gold? ", o_name);
		if (!get_check(out_val))
		{
			return;
		}
	}

	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr))
	{
		/* Message */
		msg_format("You fail to turn %s to gold!", o_name);

		/* Done */
		return;
	}

	price = object_value_real(o_ptr);

	if (price <= 0)
		/* Message */
		msg_format("You turn %s to fool's gold.", o_name);
	else
	{
		price /= 3;

		if (amt > 1) price *= amt;

		msg_format("You turn %s to %ld coins worth of gold.", o_name, price);
		p_ptr->au += price;

		/* Redraw gold */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

	}

	/* Eliminate the item */
	inc_stack_size(item, -amt);
}



static int report_magics_aux(int dur)
{
	if (dur <= 5)
	{
		return 0;
	}
	else if (dur <= 10)
	{
		return 1;
	}
	else if (dur <= 20)
	{
		return 2;
	}
	else if (dur <= 50)
	{
		return 3;
	}
	else if (dur <= 100)
	{
		return 4;
	}
	else if (dur <= 200)
	{
		return 5;
	}
	else
	{
		return 6;
	}
}

static const char *report_magic_durations[] =
{
	"for a short time",
	"for a little while",
	"for a while",
	"for a long while",
	"for a long time",
	"for a very long time",
	"for an incredibly long time",
	"until you hit a monster"
};


void report_magics()
{
	int i = 0, j, k;

	char Dummy[80];

	const char *info[128];
	int info2[128];

	if (p_ptr->blind)
	{
		info2[i] = report_magics_aux(p_ptr->blind);
		info[i++] = "You cannot see";
	}
	if (p_ptr->confused)
	{
		info2[i] = report_magics_aux(p_ptr->confused);
		info[i++] = "You are confused";
	}
	if (p_ptr->afraid)
	{
		info2[i] = report_magics_aux(p_ptr->afraid);
		info[i++] = "You are terrified";
	}
	if (p_ptr->poisoned)
	{
		info2[i] = report_magics_aux(p_ptr->poisoned);
		info[i++] = "You are poisoned";
	}
	if (p_ptr->image)
	{
		info2[i] = report_magics_aux(p_ptr->image);
		info[i++] = "You are hallucinating";
	}

	if (p_ptr->blessed)
	{
		info2[i] = report_magics_aux(p_ptr->blessed);
		info[i++] = "You feel righteous";
	}
	if (p_ptr->hero)
	{
		info2[i] = report_magics_aux(p_ptr->hero);
		info[i++] = "You feel heroic";
	}
	if (p_ptr->shero)
	{
		info2[i] = report_magics_aux(p_ptr->shero);
		info[i++] = "You are in a battle rage";
	}
	if (p_ptr->protevil)
	{
		info2[i] = report_magics_aux(p_ptr->protevil);
		info[i++] = "You are protected from evil";
	}
	if (p_ptr->shield)
	{
		info2[i] = report_magics_aux(p_ptr->shield);
		info[i++] = "You are protected by a mystic shield";
	}
	if (p_ptr->invuln)
	{
		info2[i] = report_magics_aux(p_ptr->invuln);
		info[i++] = "You are invulnerable";
	}
	if (p_ptr->tim_wraith)
	{
		info2[i] = report_magics_aux(p_ptr->tim_wraith);
		info[i++] = "You are incorporeal";
	}
	if (p_ptr->confusing)
	{
		info2[i] = 7;
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->word_recall)
	{
		info2[i] = report_magics_aux(p_ptr->word_recall);
		info[i++] = "You waiting to be recalled";
	}
	if (p_ptr->oppose_acid)
	{
		info2[i] = report_magics_aux(p_ptr->oppose_acid);
		info[i++] = "You are resistant to acid";
	}
	if (p_ptr->oppose_elec)
	{
		info2[i] = report_magics_aux(p_ptr->oppose_elec);
		info[i++] = "You are resistant to lightning";
	}
	if (p_ptr->oppose_fire)
	{
		info2[i] = report_magics_aux(p_ptr->oppose_fire);
		info[i++] = "You are resistant to fire";
	}
	if (p_ptr->oppose_cold)
	{
		info2[i] = report_magics_aux(p_ptr->oppose_cold);
		info[i++] = "You are resistant to cold";
	}
	if (p_ptr->oppose_pois)
	{
		info2[i] = report_magics_aux(p_ptr->oppose_pois);
		info[i++] = "You are resistant to poison";
	}

	/* Save the screen */
	screen_save_no_flush();

	/* Erase the screen */
	for (k = 1; k < 24; k++) prt("", k, 13);

	/* Label the information */
	prt("     Your Current Magic:", 1, 15);

	/* We will print on top of the map (column 13) */
	for (k = 2, j = 0; j < i; j++)
	{
		/* Show the info */
		sprintf( Dummy, "%s %s.", info[j],
		         report_magic_durations[info2[j]] );
		prt(Dummy, k++, 15);

		/* Every 20 entries (lines 2 to 21), start over */
		if ((k == 22) && (j + 1 < i))
		{
			prt("-- more --", k, 15);
			inkey();
			for (; k > 2; k--) prt("", k, 15);
		}
	}

	/* Pause */
	prt("[Press any key to continue]", k, 13);
	inkey();

	/* Restore the screen */
	screen_load_no_flush();
}



/*
 * Forget everything
 */
bool lose_all_info()
{
	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Mega-Hack -- Forget the map */
	wiz_dark();

	/* It worked */
	return true;
}


/*
 * Detect all doors on current panel
 */
bool detect_doors(int rad)
{
	int y, x;

	bool detect = false;

	cave_type *c_ptr;


	/* Scan the panel */
	for (y = p_ptr->py - rad; y <= p_ptr->py + rad; y++)
	{
		for (x = p_ptr->px - rad; x <= p_ptr->px + rad; x++)
		{
			if (!in_bounds(y, x)) continue;

			if (distance(p_ptr->py, p_ptr->px, y, x) > rad) continue;

			c_ptr = &cave[y][x];

			/* Detect secret doors */
			if (c_ptr->feat == FEAT_SECRET)
			{
				/* Remove feature mimics */
				cave[y][x].mimic = 0;

				/* Pick a door XXX XXX XXX */
				cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);
			}

			/* Detect doors */
			if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			                (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			                ((c_ptr->feat == FEAT_OPEN) ||
			                 (c_ptr->feat == FEAT_BROKEN)))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Reveal it */
				/* c_ptr->mimic = 0; */

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = true;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of doors!");
	}

	/* Result */
	return (detect);
}


/*
 * Detect all stairs on current panel
 */
bool detect_stairs(int rad)
{
	int y, x;

	bool detect = false;

	cave_type *c_ptr;


	/* Scan the panel */
	for (y = p_ptr->py - rad; y <= p_ptr->py + rad; y++)
	{
		for (x = p_ptr->px - rad; x <= p_ptr->px + rad; x++)
		{
			if (!in_bounds(y, x)) continue;

			if (distance(p_ptr->py, p_ptr->px, y, x) > rad) continue;

			c_ptr = &cave[y][x];

			/* Detect stairs */
			if ((c_ptr->feat == FEAT_LESS) ||
			                (c_ptr->feat == FEAT_MORE) ||
			                (c_ptr->feat == FEAT_SHAFT_DOWN) ||
			                (c_ptr->feat == FEAT_SHAFT_UP) ||
			                (c_ptr->feat == FEAT_WAY_LESS) ||
			                (c_ptr->feat == FEAT_WAY_MORE))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Obvious */
				detect = true;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of ways out of this area!");
	}

	/* Result */
	return (detect);
}


/*
 * Detect any treasure on the current panel
 */
bool detect_treasure(int rad)
{
	int y, x;

	bool detect = false;

	cave_type *c_ptr;


	/* Scan the current panel */
	for (y = p_ptr->py - rad; y <= p_ptr->py + rad; y++)
	{
		for (x = p_ptr->px - rad; x <= p_ptr->px + rad; x++)
		{
			if (!in_bounds(y, x)) continue;

			if (distance(p_ptr->py, p_ptr->px, y, x) > rad) continue;

			c_ptr = &cave[y][x];

			/* Notice embedded gold */
			if ((c_ptr->feat == FEAT_MAGMA_H) ||
			                (c_ptr->feat == FEAT_QUARTZ_H))
			{
				/* Expose the gold */
				cave_set_feat(y, x, c_ptr->feat + 0x02);
			}
			else if (c_ptr->feat == FEAT_SANDWALL_H)
			{
				/* Expose the gold */
				cave_set_feat(y, x, FEAT_SANDWALL_K);
			}

			/* Magma/Quartz + Known Gold */
			if ((c_ptr->feat == FEAT_MAGMA_K) ||
			                (c_ptr->feat == FEAT_QUARTZ_K) ||
			                (c_ptr->feat == FEAT_SANDWALL_K))
			{
				/* Hack -- Memorize */
				c_ptr->info |= (CAVE_MARK);

				/* Redraw */
				lite_spot(y, x);

				/* Detect */
				detect = true;
			}
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print("You sense the presence of buried treasure!");
	}



	/* Result */
	return (detect);
}


/*
 * Detect monsters on current panel using a
 * predicate function P and an update function U.
 * The "update function" is called exactly once if
 * the predicate succeeds. The
 */
template<typename P> static bool detect_monsters_fn(int radius, P p) {
	bool flag = false;
	/* Scan monsters */
	for (int i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx)
		{
			continue;
		}

		/* Location */
		int const y = m_ptr->fy;
		int const x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (m_ptr->cdis > radius)
		{
			continue;
		}
		/* Detect monsters which fulfill the predicate */
		auto r_ptr = m_ptr->race();
		if (p(r_ptr.get()))
		{
			/* Repair visibility later */
			repair_monsters = true;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = true;

			/* Redraw */
			if (panel_contains(y, x)) lite_spot(y, x);

			/* Detect */
			flag = true;
		}
	}
	/* Result */
	return flag;
}

/*
 * Detect all (string) monsters on current panel
 */
static bool detect_monsters_string(const char *chars, int rad)
{
	auto predicate = [chars](monster_race *r_ptr) -> bool {
		return strchr(chars, r_ptr->d_char);
	};

	/* Describe */
	if (detect_monsters_fn(rad, predicate))
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
		return true;
	}
	else
	{
		return false;
	}
}


/**
 * Detect objects on the current panel.
 */
template <typename P> bool detect_objects_fn(int radius, const char *object_message, const char *monsters, P p)
{
	bool detect = false;

	/* Scan objects */
	for (int i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_ptr) continue;

		/* Location */
		int y, x;

		/* Skip held objects */
		if (o_ptr->held_m_idx)
		{
			/* Access the monster */
			monster_type *m_ptr = &m_list[o_ptr->held_m_idx];
			auto const r_ptr = m_ptr->race();

			if (!(r_ptr->flags & RF_MIMIC))
			{
				continue; /* Skip mimics completely */
			}
			else
			{
				/* Location */
				y = m_ptr->fy;
				x = m_ptr->fx;
			}
		}
		else
		{
			/* Location */
			y = o_ptr->iy;
			x = o_ptr->ix;
		}

		/* Only detect nearby objects */
		if (distance(p_ptr->py, p_ptr->px, y, x) > radius)
		{
			continue;
		}

		/* Detect objects that satisfy predicate */
		if (p(o_ptr))
		{
			/* Hack -- memorize it */
			o_ptr->marked = true;

			/* Redraw */
			if (panel_contains(y, x)) lite_spot(y, x);

			/* Detect */
			detect = true;
		}
	}

	/* Describe */
	if (detect)
	{
		msg_print(object_message);
	}

	if (detect_monsters_string(monsters, radius))
	{
		detect = true;
	}

	/* Result */
	return detect;
}


/*
 * Detect all "gold" objects on the current panel
 */
bool detect_objects_gold(int rad)
{
	auto predicate = [](object_type const *o_ptr) -> bool {
		return o_ptr->tval == TV_GOLD;
	};

	return detect_objects_fn(
		rad,
		"You sense the presence of treasure!",
		"$",
		predicate);
}


/*
 * Detect all "normal" objects on the current panel
 */
bool detect_objects_normal(int rad)
{
	auto predicate = [](object_type const *o_ptr) -> bool {
		return o_ptr->tval != TV_GOLD;
	};
	const char *object_message = "You sense the presence of objects!";
	const char *monsters = "!=?|";

	return detect_objects_fn(
		rad,
		object_message,
		monsters,
		predicate);
}


/*
 * Detect all "normal" monsters on the current panel
 */
bool detect_monsters_normal(int rad)
{
	auto predicate = [](monster_race *r_ptr) -> bool {
		return (!(r_ptr->flags & RF_INVISIBLE)) ||
			p_ptr->see_inv || p_ptr->tim_invis;
	};

	if (detect_monsters_fn(rad, predicate))
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
		return true;
	}
	else
	{
		return false;
	}
}


/*
 * Detect all "invisible" monsters on current panel
 */
bool detect_monsters_invis(int rad)
{
	auto predicate = [](monster_race *r_ptr) -> bool {
		return bool(r_ptr->flags & RF_INVISIBLE);
	};

	if (detect_monsters_fn(rad, predicate))
	{
		/* Describe result */
		msg_print("You sense the presence of invisible creatures!");
		return true;
	}
	else
	{
		return false;
	}
}



/*
 * Detect orcs
 */
void detect_monsters_orcs(int rad)
{
	auto predicate = [](monster_race *r_ptr) -> bool {
		return bool(r_ptr->flags & RF_ORC);
	};

	if (detect_monsters_fn(rad, predicate))
	{
		msg_print("You sense the presence of orcs!");
	}
}



/*
 * Detect everything
 */
bool detect_all(int rad)
{
	bool detect = false;

	/* Detect everything */
	if (detect_doors(rad)) detect = true;
	if (detect_stairs(rad)) detect = true;
	if (detect_treasure(rad)) detect = true;
	if (detect_objects_gold(rad)) detect = true;
	if (detect_objects_normal(rad)) detect = true;
	if (detect_monsters_invis(rad)) detect = true;
	if (detect_monsters_normal(rad)) detect = true;

	/* Result */
	return (detect);
}



/*
 * Create stairs at the player location
 */
void stair_creation()
{
	auto const &dungeon_flags = game->dungeon_flags;

	/* XXX XXX XXX */
	if (!cave_valid_bold(p_ptr->py, p_ptr->px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	if (dungeon_flags & DF_FLAT)
	{
		msg_print("No stair creation in non dungeons...");
		return;
	}

	if (dungeon_flags & DF_SPECIAL)
	{
		msg_print("No stair creation on special levels...");
		return;
	}

	/* XXX XXX XXX */
	delete_object(p_ptr->py, p_ptr->px);

	/* Create a staircase */
	if (p_ptr->inside_quest)
	{
		/* quest */
		msg_print("There is no effect!");
	}
	else if (!dun_level)
	{
		/* Town/wilderness */
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_MORE);
	}
	else if (is_quest(dun_level) || (dun_level >= MAX_DEPTH - 1))
	{
		/* Quest level */
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
	}
	else if (rand_int(100) < 50)
	{
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_MORE);
	}
	else
	{
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
	}
}




/*
 * Hook to specify "weapon"
 */
static object_filter_t const &item_tester_hook_weapon()
{
	using namespace object_filter;
	static auto instance =
		Or(
			TVal(TV_MSTAFF),
			TVal(TV_BOOMERANG),
			TVal(TV_SWORD),
			TVal(TV_AXE),
			TVal(TV_HAFTED),
			TVal(TV_POLEARM),
			TVal(TV_BOW),
			TVal(TV_BOLT),
			TVal(TV_ARROW),
			TVal(TV_SHOT),
			And(
				TVal(TV_DAEMON_BOOK),
				SVal(SV_DEMONBLADE)));
	return instance;
}


/*
 * Hook to specify "armour"
 */
static object_filter_t const &item_tester_hook_armour()
{
	using namespace object_filter;
	static auto instance =
		Or(
			TVal(TV_DRAG_ARMOR),
			TVal(TV_HARD_ARMOR),
			TVal(TV_SOFT_ARMOR),
			TVal(TV_SHIELD),
			TVal(TV_CLOAK),
			TVal(TV_CROWN),
			TVal(TV_HELM),
			TVal(TV_BOOTS),
			TVal(TV_GLOVES),
			And(
				TVal(TV_DAEMON_BOOK),
				Or(
						SVal(SV_DEMONHORN),
						SVal(SV_DEMONSHIELD))));
	return instance;
}


/*
 * Check if an object is artifactable
 */
object_filter_t const &item_tester_hook_artifactable()
{
	using namespace object_filter;
	static auto instance = And(
		// Check base item family
		Or(
			item_tester_hook_weapon(),
			item_tester_hook_armour(),
			TVal(TV_DIGGING),
			TVal(TV_RING),
			TVal(TV_AMULET)),
		// Only unenchanted items
		Not(IsArtifactP()),
		Not(IsEgo()));
	return instance;
}


/*
 * Enchants a plus onto an item. -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item. -CFT-
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(object_type *o_ptr, int n, int eflag)
{
	int i, chance, prob;
	bool res = false;
	auto const a = artifact_p(o_ptr);

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);

	/* Break curse if item is cursed the curse isn't permanent */
	auto maybe_break_curse = [o_ptr, &flags]()
	{
		if (cursed_p(o_ptr) && (!(flags & TR_PERMA_CURSE)))
		{
			msg_print("The curse is broken!");
			o_ptr->art_flags &= ~(TR_CURSED | TR_HEAVY_CURSE);
		}
	};

	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	                (o_ptr->tval == TV_ARROW) ||
	                (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i = 0; i < n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if (rand_int(prob) >= 100) continue;

		/* Enchant to hit */
		if (eflag & (ENCH_TOHIT))
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_h++;
				res = true;

				/* break curse? */
				if ((o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					maybe_break_curse();
				}
			}
		}

		/* Enchant to damage */
		if (eflag & (ENCH_TODAM))
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_d++;
				res = true;

				/* break curse? */
				if ((o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					maybe_break_curse();
				}
			}
		}


		/* Enchant to damage */
		if (eflag & (ENCH_PVAL))
		{
			if (o_ptr->pval < 0) chance = 0;
			else if (o_ptr->pval > 6) chance = 1000;
			else chance = enchant_table[o_ptr->pval * 2];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->pval++;
				res = true;

				/* break curse? */
				if ((o_ptr->pval >= 0) && (rand_int(100) < 25))
				{
					maybe_break_curse();
				}
			}
		}

		/* Enchant to armor class */
		if (eflag & (ENCH_TOAC))
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_a++;
				res = true;

				/* break curse? */
				if ((o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					maybe_break_curse();
				}
			}
		}
	}

	/* Failure */
	if (!res) return false;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Success */
	return true;
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns true if attempted, false if cancelled
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac, int num_pval)
{
	int item;
	bool okay = false;
	char o_name[80];
	const char *q;
	const char *s;


	/* Assume enchant weapon */
	object_filter_t object_filter = item_tester_hook_weapon();

	/* Enchant armor if requested */
	if (num_ac)
	{
		object_filter = item_tester_hook_armour();
	}

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR), object_filter)) return false;

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Description */
	object_desc(o_name, o_ptr, false, 0);

	/* Describe */
	msg_format("%s %s glow%s brightly!",
	           ((item >= 0) ? "Your" : "The"), o_name,
	           ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = true;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = true;
	if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = true;
	if (enchant(o_ptr, num_pval, ENCH_PVAL)) okay = true;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		flush_on_failure();

		/* Message */
		msg_print("The enchantment failed.");
	}

	/* Something happened */
	return true;
}

void curse_artifact(object_type * o_ptr)
{
	if (o_ptr->pval) o_ptr->pval = 0 - ((o_ptr->pval) + randint(4));
	if (o_ptr->to_a) o_ptr->to_a = 0 - ((o_ptr->to_a) + randint(4));
	if (o_ptr->to_h) o_ptr->to_h = 0 - ((o_ptr->to_h) + randint(4));
	if (o_ptr->to_d) o_ptr->to_d = 0 - ((o_ptr->to_d) + randint(4));
	o_ptr->art_flags |= TR_HEAVY_CURSE | TR_CURSED;
	if (randint(3) == 1) o_ptr-> art_flags |= TR_TY_CURSE;
	if (randint(2) == 1) o_ptr-> art_flags |= TR_AGGRAVATE;
	if (randint(3) == 1) o_ptr-> art_flags |= TR_DRAIN_EXP;
	if (randint(3) == 1) o_ptr-> art_flags |= TR_BLACK_BREATH;
	if (randint(2) == 1) o_ptr-> art_flags |= TR_TELEPORT;
	else if (randint(3) == 1) o_ptr->art_flags |= TR_NO_TELE;
}



void random_resistance(object_type *o_ptr, int specific)
{
	/* To avoid a number of possible bugs */
	if (!specific)
	{
		if (artifact_bias == BIAS_ACID)
		{
			if (!(o_ptr->art_flags & TR_RES_ACID))
			{
				o_ptr->art_flags |= TR_RES_ACID;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags & TR_IM_ACID))
			{
				o_ptr->art_flags |= TR_IM_ACID;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_ELEC)
		{
			if (!(o_ptr->art_flags & TR_RES_ELEC))
			{
				o_ptr->art_flags |= TR_RES_ELEC;
				if (rand_int(2) == 0) return;
			}
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
					!(o_ptr->art_flags & TR_SH_ELEC))
			{
				o_ptr->art_flags |= TR_SH_ELEC;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags & TR_IM_ELEC))
			{
				o_ptr->art_flags |= TR_IM_ELEC;
				if (rand_int(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_FIRE)
		{
			if (!(o_ptr->art_flags & TR_RES_FIRE))
			{
				o_ptr->art_flags |= TR_RES_FIRE;
				if (rand_int(2) == 0) return;
			}
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
					!(o_ptr->art_flags & TR_SH_FIRE))
			{
				o_ptr->art_flags |= TR_SH_FIRE;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags & TR_IM_FIRE))
			{
				o_ptr->art_flags |= TR_IM_FIRE;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_COLD)
		{
			if (!(o_ptr->art_flags & TR_RES_COLD))
			{
				o_ptr->art_flags |= TR_RES_COLD;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags & TR_IM_COLD))
			{
				o_ptr->art_flags |= TR_IM_COLD;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_POIS)
		{
			if (!(o_ptr->art_flags & TR_RES_POIS))
			{
				o_ptr->art_flags |= TR_RES_POIS;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_WARRIOR)
		{
			if (rand_int(3) && (!(o_ptr->art_flags & TR_RES_FEAR)))
			{
				o_ptr->art_flags |= TR_RES_FEAR;
				if (rand_int(2) == 0) return;
			}
			if ((rand_int(3) == 0) && (!(o_ptr->art_flags & TR_NO_MAGIC)))
			{
				o_ptr->art_flags |= TR_NO_MAGIC;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_NECROMANTIC)
		{
			if (!(o_ptr->art_flags & TR_RES_NETHER))
			{
				o_ptr->art_flags |= TR_RES_NETHER;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags & TR_RES_POIS))
			{
				o_ptr->art_flags |= TR_RES_POIS;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags & TR_RES_DARK))
			{
				o_ptr->art_flags |= TR_RES_DARK;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_CHAOS)
		{
			if (!(o_ptr->art_flags & TR_RES_CHAOS))
			{
				o_ptr->art_flags |= TR_RES_CHAOS;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags & TR_RES_CONF))
			{
				o_ptr->art_flags |= TR_RES_CONF;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags & TR_RES_DISEN))
			{
				o_ptr->art_flags |= TR_RES_DISEN;
				if (rand_int(2) == 0) return;
			}
		}
	}

	switch (specific ? specific : randint(41))
	{
	case 1 :
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, specific);
		else
		{
			o_ptr->art_flags |= TR_IM_ACID;
			/*  if (is_scroll) msg_print("It looks totally incorruptible."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_ACID;
		}
		break;
	case 2:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, specific);
		else
		{
			o_ptr->art_flags |= TR_IM_ELEC;
			/*  if (is_scroll) msg_print("It looks completely grounded."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_ELEC;
		}
		break;
	case 3:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, specific);
		else
		{
			o_ptr->art_flags |= TR_IM_COLD;
			/*  if (is_scroll) msg_print("It feels very warm."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_COLD;
		}
		break;
	case 4:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, specific);
		else
		{
			o_ptr->art_flags |= TR_IM_FIRE;
			/*  if (is_scroll) msg_print("It feels very cool."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_FIRE;
		}
		break;
	case 5:
	case 6:
	case 13:
		o_ptr->art_flags |= TR_RES_ACID;
		/*  if (is_scroll) msg_print("It makes your stomach rumble."); */
		if (!(artifact_bias))
			artifact_bias = BIAS_ACID;
		break;
	case 7:
	case 8:
	case 14:
		o_ptr->art_flags |= TR_RES_ELEC;
		/*  if (is_scroll) msg_print("It makes you feel grounded."); */
		if (!(artifact_bias))
			artifact_bias = BIAS_ELEC;
		break;
	case 9:
	case 10:
	case 15:
		o_ptr->art_flags |= TR_RES_FIRE;
		/*  if (is_scroll) msg_print("It makes you feel cool!");*/
		if (!(artifact_bias))
			artifact_bias = BIAS_FIRE;
		break;
	case 11:
	case 12:
	case 16:
		o_ptr->art_flags |= TR_RES_COLD;
		/*  if (is_scroll) msg_print("It makes you feel full of hot air!");*/
		if (!(artifact_bias))
			artifact_bias = BIAS_COLD;
		break;
	case 17:
	case 18:
		o_ptr->art_flags |= TR_RES_POIS;
		/*  if (is_scroll) msg_print("It makes breathing easier for you."); */
		if (!(artifact_bias) && randint(4) != 1)
			artifact_bias = BIAS_POIS;
		else if (!(artifact_bias) && randint(2) == 1)
			artifact_bias = BIAS_NECROMANTIC;
		else if (!(artifact_bias) && randint(2) == 1)
			artifact_bias = BIAS_ROGUE;
		break;
	case 19:
	case 20:
		o_ptr->art_flags |= TR_RES_FEAR;
		/*  if (is_scroll) msg_print("It makes you feel brave!"); */
		if (!(artifact_bias) && randint(3) == 1)
			artifact_bias = BIAS_WARRIOR;
		break;
	case 21:
		o_ptr->art_flags |= TR_RES_LITE;
		/*  if (is_scroll) msg_print("It makes everything look darker.");*/
		break;
	case 22:
		o_ptr->art_flags |= TR_RES_DARK;
		/*  if (is_scroll) msg_print("It makes everything look brigher.");*/
		break;
	case 23:
	case 24:
		o_ptr->art_flags |= TR_RES_BLIND;
		/*  if (is_scroll) msg_print("It makes you feel you are wearing glasses.");*/
		break;
	case 25:
	case 26:
		o_ptr->art_flags |= TR_RES_CONF;
		/*  if (is_scroll) msg_print("It makes you feel very determined.");*/
		if (!(artifact_bias) && randint(6) == 1)
			artifact_bias = BIAS_CHAOS;
		break;
	case 27:
	case 28:
		o_ptr->art_flags |= TR_RES_SOUND;
		/*  if (is_scroll) msg_print("It makes you feel deaf!");*/
		break;
	case 29:
	case 30:
		o_ptr->art_flags |= TR_RES_SHARDS;
		/*  if (is_scroll) msg_print("It makes your skin feel thicker.");*/
		break;
	case 31:
	case 32:
		o_ptr->art_flags |= TR_RES_NETHER;
		/*  if (is_scroll) msg_print("It makes you feel like visiting a graveyard!");*/
		if (!(artifact_bias) && randint(3) == 1)
			artifact_bias = BIAS_NECROMANTIC;
		break;
	case 33:
	case 34:
		o_ptr->art_flags |= TR_RES_NEXUS;
		/*  if (is_scroll) msg_print("It makes you feel normal.");*/
		break;
	case 35:
	case 36:
		o_ptr->art_flags |= TR_RES_CHAOS;
		/*  if (is_scroll) msg_print("It makes you feel very firm.");*/
		if (!(artifact_bias) && randint(2) == 1)
			artifact_bias = BIAS_CHAOS;
		break;
	case 37:
	case 38:
		o_ptr->art_flags |= TR_RES_DISEN;
		/*  if (is_scroll) msg_print("It is surrounded by a static feeling.");*/
		break;
	case 39:
		if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
			o_ptr->art_flags |= TR_SH_ELEC;
		else
			random_resistance(o_ptr, specific);
		if (!(artifact_bias))
			artifact_bias = BIAS_ELEC;
		break;
	case 40:
		if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
			o_ptr->art_flags |= TR_SH_FIRE;
		else
			random_resistance(o_ptr, specific);
		if (!(artifact_bias))
			artifact_bias = BIAS_FIRE;
		break;
	case 41:
		if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CLOAK ||
		                o_ptr->tval == TV_HELM || o_ptr->tval == TV_HARD_ARMOR)
			o_ptr->art_flags |= TR_REFLECT;
		else
			random_resistance(o_ptr, specific);
		break;
	}
}



/*
 * Make note of found artifacts.
 */
static void note_found_object(object_type *o_ptr)
{
	char note[150];
	char item_name[80];

	if (artifact_p(o_ptr))
	{
		object_desc(item_name, o_ptr, false, 0);

		/* Build note and write */
		sprintf(note, "Found The %s", item_name);
		add_note(note, 'A');
	}
}




/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns true if something was identified, else false.
 */
bool ident_spell()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Identify which item? ",
		      "You have nothing to identify.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR),
		      object_filter::Not(object_known_p))) return false;

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Description */
	char o_name[80];
	object_desc(o_name, o_ptr, true, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format("%^s: %s (%c).",
		           describe_use(item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format("In your pack: %s (%c).",
		           o_name, index_to_label(item));
	}
	else
	{
		msg_format("On the ground: %s.",
		           o_name);
	}

	/* Make note of found artifacts */
	note_found_object(o_ptr);

	/* Something happened */
	return true;
}

/*
 * Identify all objects in the level
 */
bool ident_all()
{
	int i;

	object_type *o_ptr;

	for (i = 1; i < o_max; i++)
	{
		/* Acquire object */
		o_ptr = &o_list[i];

		/* Identify it fully */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Make note of found artifacts */
		note_found_object(o_ptr);
	}

	/* Something happened */
	return true;
}



/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
object_filter_t const &item_tester_hook_recharge()
{
	using namespace object_filter;
	static auto instance =
		And(
			// Must NOT have NO_RECHARGE flag.
			Not(HasFlags(TR_NO_RECHARGE)),
			// ... and must be a device.
			Or(
				TVal(TV_STAFF),
				TVal(TV_WAND),
				TVal(TV_ROD_MAIN)));
	return instance;
}


/*
 * Recharge a wand/staff/rod from the pack or on the floor.
 * This function has been rewritten in Oangband. -LM-
 *
 * Mage -- Recharge I --> recharge(90)
 * Mage -- Recharge II --> recharge(150)
 * Mage -- Recharge III --> recharge(220)
 *
 * Priest or Necromancer -- Recharge --> recharge(140)
 *
 * Scroll of recharging --> recharge(130)
 * Scroll of *recharging* --> recharge(200)
 *
 * It is harder to recharge high level, and highly charged wands, 
 * staffs, and rods.  The more wands in a stack, the more easily and 
 * strongly they recharge.  Staffs, however, each get fewer charges if 
 * stacked.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 */
bool recharge(int power)
{
	int recharge_strength, recharge_amount;
	int lev;
	bool fail = false;
	byte fail_type = 1;

	char o_name[80];

	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Recharge which item? ",
		      "You have nothing to recharge.",
		      (USE_INVEN | USE_FLOOR),
		      item_tester_hook_recharge()))
	{
		return false;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);

	/* Extract the object "level" */
	lev = o_ptr->k_ptr->level;

	/* Recharge a rod */
	if (o_ptr->tval == TV_ROD_MAIN)
	{
		/* Extract a recharge strength by comparing object level to power. */
		recharge_strength = ((power > lev) ? (power - lev) : 0) / 5;

		/* Paranoia */
		if (recharge_strength < 0) recharge_strength = 0;

		/* Back-fire */
		if ((rand_int(recharge_strength) == 0) && (!(flags & TR_RECHARGE)))
		{
			/* Activate the failure code. */
			fail = true;
		}

		/* Recharge */
		else
		{
			/* Recharge amount */
			recharge_amount = (power * damroll(3, 2));

			/* Recharge by that amount */
			if (o_ptr->timeout + recharge_amount < o_ptr->pval2)
				o_ptr->timeout += recharge_amount;
			else
				o_ptr->timeout = o_ptr->pval2;
		}
	}


	/* Recharge wand/staff */
	else
	{
		/* Extract a recharge strength by comparing object level to power.
		 * Divide up a stack of wands' charges to calculate charge penalty.
		 */
		if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			recharge_strength = (100 + power - lev -
			                     (8 * o_ptr->pval / o_ptr->number)) / 15;

		/* All staffs, unstacked wands. */
		else recharge_strength = (100 + power - lev -
			                          (8 * o_ptr->pval)) / 15;


		/* Back-fire XXX XXX XXX */
		if (((rand_int(recharge_strength) == 0) && (!(flags & TR_RECHARGE))) ||
				(flags & TR_NO_RECHARGE))
		{
			/* Activate the failure code. */
			fail = true;
		}

		/* If the spell didn't backfire, recharge the wand or staff. */
		else
		{
			/* Recharge based on the standard number of charges. */
			recharge_amount = randint((power / (lev + 2)) + 1);

			/* Multiple wands in a stack increase recharging somewhat. */
			if ((o_ptr->tval == TV_WAND) && (o_ptr->number > 1))
			{
				recharge_amount +=
				        (randint(recharge_amount * (o_ptr->number - 1))) / 2;
				if (recharge_amount < 1) recharge_amount = 1;
				if (recharge_amount > 12) recharge_amount = 12;
			}

			/* But each staff in a stack gets fewer additional charges,
			 * although always at least one.
			 */
			if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
			{
				recharge_amount /= o_ptr->number;
				if (recharge_amount < 1) recharge_amount = 1;
			}

			/* Recharge the wand or staff. */
			o_ptr->pval += recharge_amount;
		}
	}

	/* Mark as recharged */
	o_ptr->art_flags |= TR_RECHARGED;

	/* Inflict the penalties for failing a recharge. */
	if (fail)
	{
		/* Artifacts are never destroyed. */
		if (artifact_p(o_ptr))
		{
			object_desc(o_name, o_ptr, true, 0);
			msg_format("The recharging backfires - %s is completely drained!", o_name);

			/* Artifact rods. */
			if (o_ptr->tval == TV_ROD_MAIN)
				o_ptr->timeout = 0;

			/* Artifact wands and staffs. */
			else if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
				o_ptr->pval = 0;
		}
		else
		{
			/* Get the object description */
			object_desc(o_name, o_ptr, false, 0);

			/*** Determine Seriousness of Failure ***/

			/* Mages recharge objects more safely. */
			if (p_ptr->has_ability(AB_PERFECT_CASTING))
			{
				/* 10% chance to blow up one rod, otherwise draining. */
				if (o_ptr->tval == TV_ROD_MAIN)
				{
					if (randint(10) == 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 75% chance to blow up one wand, otherwise draining. */
				else if (o_ptr->tval == TV_WAND)
				{
					if (randint(3) != 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 50% chance to blow up one staff, otherwise no effect. */
				else if (o_ptr->tval == TV_STAFF)
				{
					if (randint(2) == 1) fail_type = 2;
					else fail_type = 0;
				}
			}

			/* All other classes get no special favors. */
			else
			{
				/* 33% chance to blow up one rod, otherwise draining. */
				if (o_ptr->tval == TV_ROD_MAIN)
				{
					if (randint(3) == 1) fail_type = 2;
					else fail_type = 1;
				}
				/* 20% chance of the entire stack, else destroy one wand. */
				else if (o_ptr->tval == TV_WAND)
				{
					if (randint(5) == 1) fail_type = 3;
					else fail_type = 2;
				}
				/* Blow up one staff. */
				else if (o_ptr->tval == TV_STAFF)
				{
					fail_type = 2;
				}
			}

			/*** Apply draining and destruction. ***/

			/* Drain object or stack of objects. */
			if (fail_type == 1)
			{
				if (o_ptr->tval == TV_ROD_MAIN)
				{
					msg_print("The recharge backfires, draining the rod further!");
					if (o_ptr->timeout < 10000)
						o_ptr->timeout = 0;
				}
				else if (o_ptr->tval == TV_WAND)
				{
					msg_format("You save your %s from destruction, but all charges are lost.", o_name);
					o_ptr->pval = 0;
				}
				/* Staffs aren't drained. */
			}

			/* Destroy an object or one in a stack of objects. */
			if (fail_type == 2)
			{
				if (o_ptr->number > 1)
					msg_format("Wild magic consumes one of your %s!", o_name);
				else
					msg_format("Wild magic consumes your %s!", o_name);

				/* Reduce rod stack maximum timeout, drain wands. */
				if (o_ptr->tval == TV_WAND) o_ptr->pval = 0;

				/* Reduce and describe */
				inc_stack_size(item, -1);
			}

			/* Destroy all memebers of a stack of objects. */
			if (fail_type == 3)
			{
				if (o_ptr->number > 1)
					msg_format("Wild magic consumes all your %s!", o_name);
				else
					msg_format("Wild magic consumes your %s!", o_name);


				/* Reduce and describe inventory */
				inc_stack_size(item, -999);
			}
		}
	}


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Something was done */
	return true;
}



/*
 * Apply a "project()" directly to all viewable monsters
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
bool project_hack(int typ, int dam)
{
	int i, x, y;
	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;
	bool obvious = false;


	/* Affect all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Require line of sight */
		if (!player_has_los_bold(y, x)) continue;

		/* Jump directly to the target monster */
		if (project(0, 0, y, x, dam, typ, flg)) obvious = true;
	}

	/* Result */
	return (obvious);
}

/*
 * Apply a "project()" a la meteor shower
 */
void project_meteor(int radius, int typ, int dam, u32b flg)
{
	cave_type *c_ptr;
	int x, y, dx, dy, d, count = 0, i;
	int b = radius + randint(radius);
	for (i = 0; i < b; i++)
	{
		for (count = 0; count < 1000; count++)
		{
			x = p_ptr->px - 5 + randint(10);
			y = p_ptr->py - 5 + randint(10);
			if ((x < 0) || (x >= cur_wid) ||
			                (y < 0) || (y >= cur_hgt)) continue;
			dx = (p_ptr->px > x) ? (p_ptr->px - x) : (x - p_ptr->px);
			dy = (p_ptr->py > y) ? (p_ptr->py - y) : (y - p_ptr->py);
			/* Approximate distance */
			d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));
			c_ptr = &cave[y][x];
			/* Check distance */
			if ((d <= 5) &&
			/* Check line of sight */
			    (player_has_los_bold(y, x)) &&
			/* But don't explode IN a wall, letting the player attack through walls */
			    !(c_ptr->info & CAVE_WALL))
				break;
		}
		if (count >= 1000) break;
		project(0, 2, y, x, dam, typ, PROJECT_JUMP | flg);
	}
}


/*
 * Banish evil monsters
 */
bool banish_evil(int dist)
{
	return (project_hack(GF_AWAY_EVIL, dist));
}



/*
 * Dispel undead monsters
 */
void dispel_undead(int dam)
{
	project_hack(GF_DISP_UNDEAD, dam);
}

/*
 * Dispel evil monsters
 */
void dispel_evil(int dam)
{
	project_hack(GF_DISP_EVIL, dam);
}

/*
 * Dispel good monsters
 */
void dispel_good(int dam)
{
	project_hack(GF_DISP_GOOD, dam);
}

/*
 * Dispel all monsters
 */
void dispel_monsters(int dam)
{
	project_hack(GF_DISP_ALL, dam);
}


/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
	int i;
	bool sleep = false;
	bool speed = false;


	/* Aggravate everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip aggravating monster (or player) */
		if (i == who) continue;

		/* Wake up nearby sleeping monsters */
		if (m_ptr->cdis < MAX_SIGHT * 2)
		{
			/* Wake up */
			if (m_ptr->csleep)
			{
				/* Wake up */
				m_ptr->csleep = 0;
				sleep = true;
			}
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx))
		{
			auto const r_ptr = m_ptr->race();

			/* Speed up (instantly) to racial base + 10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				/* Speed up */
				m_ptr->mspeed = r_ptr->speed + 10;
				speed = true;
			}

			/* Pets may get angry (50% chance) */
			if (is_friend(m_ptr))
			{
				if (randint(2) == 1)
				{
					change_side(m_ptr);
				}
			}
		}
	}

	/* Messages */
	if (speed) msg_print("You feel a sudden stirring nearby!");
	else if (sleep) msg_print("You hear a sudden stirring in the distance!");
}

/*
 * Generic genocide race selection
 */
static bool get_genocide_race(const char *msg, char *typ)
{
	int i, j;
	cave_type *c_ptr;

	msg_print(msg);
	if (!tgt_pt(&i, &j)) return false;

	c_ptr = &cave[j][i];

	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		auto const r_ptr = m_ptr->race();

		*typ = r_ptr->d_char;
		return true;
	}

	msg_print("You must select a monster.");
	return false;
}


/*
 * Delete all non-unique/non-quest monsters of a given "type" from the level
 */
static void genocide_aux(char typ)
{
	int i;
	auto const msec = options->delay_factor_ms();
	int dam = 0;

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		auto const r_ptr = m_ptr->race();

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags & RF_UNIQUE) continue;

		/* Hack -- Skip Quest Monsters */
		if (m_ptr->mflag & MFLAG_QUEST) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

		/* Oups */
		if (r_ptr->flags & RF_DEATH_ORB)
		{
			int wx, wy;
			int attempts = 500;
			char buf[256];

			monster_race_desc(buf, m_ptr->r_idx, 0);

			do
			{
				scatter(&wy, &wx, m_ptr->fy, m_ptr->fx, 10);
			}
			while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

			if (place_monster_aux(wy, wx, m_ptr->r_idx, false, true, MSTATUS_ENEMY))
			{
				cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interesting effect on the %s.", buf);
			}

			return;
		}

		/* Delete the monster */
		delete_monster_idx(i);

		/* Keep track of damage */
		dam += randint(4);

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		sleep_for(milliseconds(msec));
	}

	/* Take damage */
	take_hit(dam, "the strain of casting Genocide");

	/* Visual feedback */
	move_cursor_relative(p_ptr->py, p_ptr->px);

	/* Redraw */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff();

	/* Fresh */
	Term_fresh();
}

void genocide()
{
	auto const &dungeon_flags = game->dungeon_flags;

	if (dungeon_flags & DF_NO_GENO)
	{
		return;
	}

	/* Hack -- when you are fated to die, you cant cheat :) */
	if (dungeon_type == DUNGEON_DEATH)
	{
		msg_print("A mysterious force stops the genocide.");
		return;
	}

	/* Mega-Hack -- Get a monster symbol */
	char typ;
	if (!get_genocide_race("Target a monster to select the race to genocide.", &typ))
	{
		return;
	}

	genocide_aux(typ);
}


/*
 * Delete all nearby (non-unique) monsters
 */
void mass_genocide()
{
	auto const &dungeon_flags = game->dungeon_flags;

	int i;
	auto const msec = options->delay_factor_ms();
	int dam = 0;

	/* Prevented? */
	if ((dungeon_flags & DF_NO_GENO) || (dungeon_type == DUNGEON_DEATH))
	{
		msg_print("A mysterious force stops the genocide.");
		return;
	}

	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		auto const r_ptr = m_ptr->race();

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags & RF_UNIQUE) continue;

		/* Hack -- Skip Quest Monsters */
		if (m_ptr->mflag & MFLAG_QUEST) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > MAX_SIGHT) continue;

		/* Oups */
		if (r_ptr->flags & RF_DEATH_ORB)
		{
			int wx, wy;
			int attempts = 500;
			char buf[256];

			monster_race_desc(buf, m_ptr->r_idx, 0);

			do
			{
				scatter(&wy, &wx, m_ptr->fy, m_ptr->fx, 10);
			}
			while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

			if (place_monster_aux(wy, wx, m_ptr->r_idx, false, true, MSTATUS_ENEMY))
			{
				cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interesting effect on the %s.", buf);
			}

			return;
		}

		/* Delete the monster */
		delete_monster_idx(i);

		/* Keep track of damage. */
		dam += randint(3);

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		sleep_for(milliseconds(msec));
	}

	/* Take damage */
	take_hit(dam, "the strain of casting Mass Genocide");

	/* Visual feedback */
	move_cursor_relative(p_ptr->py, p_ptr->px);

	/* Redraw */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff();

	/* Fresh */
	Term_fresh();
}


/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int y1, int x1, int r)
{
	auto const &dungeon_flags = game->dungeon_flags;

	int y, x, k, t;

	cave_type *c_ptr;

	bool flag = false;


	if (dungeon_flags & DF_NO_GENO)
	{
		msg_print("Not on special levels!");
		return;
	}
	if (p_ptr->inside_quest)
	{
		return;
	}

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds(y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Access the grid */
			c_ptr = &cave[y][x];

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_MARK | CAVE_GLOW);

			/* Hack -- Notice player affect */
			if ((x == p_ptr->px) && (y == p_ptr->py))
			{
				/* Hurt the player later */
				flag = true;

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			if ((m_list[c_ptr->m_idx].status != MSTATUS_COMPANION) ||
			                (!(m_list[c_ptr->m_idx].mflag & MFLAG_QUEST)) ||
			                (!(m_list[c_ptr->m_idx].mflag & MFLAG_QUEST2)))
				delete_monster(y, x);

			/* Destroy "valid" grids */
			if (cave_valid_bold(y, x))
			{
				/* Delete objects */
				delete_object(y, x);

				/* Wall (or floor) type */
				t = rand_int(200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					cave_set_feat(y, x, FEAT_WALL_EXTRA);
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					cave_set_feat(y, x, FEAT_QUARTZ);
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					cave_set_feat(y, x, FEAT_MAGMA);
				}

				/* Floor */
				else
				{
					/* Create floor */
					cave_set_feat(y, x, FEAT_FLOOR);
				}
			}
		}
	}


	/* Hack -- Affect player */
	if (flag)
	{
		/* Message */
		msg_print("There is a searing blast of light!");

		/* Blind the player */
		if (!p_ptr->resist_blind && !p_ptr->resist_lite)
		{
			/* Become blind */
			set_blind(p_ptr->blind + 10 + randint(10));
		}
	}


	/* Mega-Hack -- Forget the view */
	p_ptr->update |= (PU_UN_VIEW);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 */
void earthquake(int cy, int cx, int r)
{
	int i, t, y, x, yy, xx, dy, dx, oy, ox;
	int damage = 0;
	int sn = 0, sy = 0, sx = 0;
	bool hurt = false;
	cave_type *c_ptr;
	bool map[32][32];

	if (p_ptr->inside_quest)
	{
		return;
	}

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = false;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[yy][xx];

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (rand_int(100) < 85) continue;

			/* Damage this grid */
			map[16 + yy - cy][16 + xx - cx] = true;

			/* Hack -- Take note of player damage */
			if ((yy == p_ptr->py) && (xx == p_ptr->px)) hurt = true;
		}
	}

	/* First, affect the player (if necessary) */
	if (hurt && !p_ptr->wraith_form)
	{
		/* Check around the player */
		for (i = 0; i < 8; i++)
		{
			/* Access the location */
			y = p_ptr->py + ddy[i];
			x = p_ptr->px + ddx[i];

			/* Skip non-empty grids */
			if (!cave_empty_bold(y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16 + y - cy][16 + x - cx]) continue;

			/* Count "safe" grids */
			sn++;

			/* Randomize choice */
			if (rand_int(sn) > 0) continue;

			/* Save the safe location */
			sy = y;
			sx = x;
		}

		/* Random message */
		switch (randint(3))
		{
		case 1:
			{
				msg_print("The cave ceiling collapses!");
				break;
			}
		case 2:
			{
				msg_print("The cave floor twists in an unnatural way!");
				break;
			}
		default:
			{
				msg_print("The cave quakes!  You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!sn)
		{
			/* Message and damage */
			msg_print("You are severely crushed!");
			damage = 300;
		}

		/* Destroy the grid, and push the player to safety */
		else
		{
			/* Calculate results */
			switch (randint(3))
			{
			case 1:
				{
					msg_print("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
			case 2:
				{
					msg_print("You are bashed by rubble!");
					damage = damroll(10, 4);
					set_stun(p_ptr->stun + randint(50));
					break;
				}
			case 3:
				{
					msg_print("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					set_stun(p_ptr->stun + randint(50));
					break;
				}
			}

			/* Save the old location */
			oy = p_ptr->py;
			ox = p_ptr->px;

			/* Move the player to the safe location */
			p_ptr->py = sy;
			p_ptr->px = sx;

			/* Redraw the old spot */
			lite_spot(oy, ox);

			/* Redraw the new spot */
			lite_spot(p_ptr->py, p_ptr->px);

			/* Check for new panel */
			verify_panel();
		}

		/* Important -- no wall on player */
		map[16 + p_ptr->py - cy][16 + p_ptr->px - cx] = false;

		/* Semi-wraiths have to be hurt *some*, says DG */
		if (race_flags_p(PR_SEMI_WRAITH))
			damage /= 4;

		/* Take some damage */
		if (damage) take_hit(damage, "an earthquake");
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16 + yy - cy][16 + xx - cx]) continue;

			/* Access the grid */
			c_ptr = &cave[yy][xx];

			/* Process monsters */
			if (c_ptr->m_idx)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				auto const r_ptr = m_ptr->race();

				/* Most monsters cannot co-exist with rock */
				if (!(r_ptr->flags & RF_KILL_WALL) &&
				                !(r_ptr->flags & RF_PASS_WALL))
				{
					char m_name[80];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags & RF_NEVER_MOVE))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Access the grid */
							y = yy + ddy[i];
							x = xx + ddx[i];

							/* Skip non-empty grids */
							if (!cave_empty_bold(y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (cave[y][x].feat == FEAT_GLYPH) continue;
							if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

							/* Important -- Skip "quake" grids */
							if (map[16 + y - cy][16 + x - cx]) continue;

							/* Count "safe" grids */
							sn++;

							/* Randomize choice */
							if (rand_int(sn) > 0) continue;

							/* Save the safe grid */
							sy = y;
							sx = x;
						}
					}

					/* Describe the monster */
					monster_desc(m_name, m_ptr, 0);

					/* Scream in pain */
					msg_format("%^s wails out in pain!", m_name);

					/* Take damage from the quake */
					damage = (sn ? damroll(4, 8) : 200);

					/* Monster is certainly awake */
					m_ptr->csleep = 0;

					/* Apply damage directly */
					m_ptr->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Message */
						msg_format("%^s is embedded in the rock!", m_name);

						/* Delete the monster */
						delete_monster(yy, xx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						int m_idx = cave[yy][xx].m_idx;

						/* Update the new location */
						cave[sy][sx].m_idx = m_idx;

						/* Update the old location */
						cave[yy][xx].m_idx = 0;

						/* Move the monster */
						m_ptr->fy = sy;
						m_ptr->fx = sx;

						/* Update the monster (new location) */
						update_mon(m_idx, true);

						/* Redraw the old grid */
						lite_spot(yy, xx);

						/* Redraw the new grid */
						lite_spot(sy, sx);
					}
				}
			}
		}
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16 + yy - cy][16 + xx - cx]) continue;

			/* Access the cave grid */
			c_ptr = &cave[yy][xx];

			/* Paranoia -- never affect player */
			if ((yy == p_ptr->py) && (xx == p_ptr->px)) continue;

			/* Destroy location (if valid) */
			if (cave_valid_bold(yy, xx))
			{
				bool floor = cave_floor_bold(yy, xx);

				/* Delete objects */
				delete_object(yy, xx);

				/* Wall (or floor) type */
				t = (floor ? rand_int(100) : 200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					cave_set_feat(yy, xx, FEAT_WALL_EXTRA);
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					cave_set_feat(yy, xx, FEAT_QUARTZ);
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					cave_set_feat(yy, xx, FEAT_MAGMA);
				}

				/* Floor */
				else
				{
					/* Create floor */
					cave_set_feat(yy, xx, FEAT_FLOOR);
				}
			}
		}
	}


	/* Mega-Hack -- Forget the view */
	p_ptr->update |= (PU_UN_VIEW);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Update the health bar */
	p_ptr->redraw |= (PR_FRAME);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite()
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		cave_type *c_ptr = &cave[y][x];

		/* No longer in the array */
		c_ptr->info &= ~(CAVE_TEMP);

		/* Update only non-CAVE_GLOW grids */
		/* if (c_ptr->info & (CAVE_GLOW)) continue; */

		/* Perma-Lite */
		c_ptr->info |= (CAVE_GLOW);
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

	/* Update stuff */
	update_stuff();

	/* Process the grids */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		cave_type *c_ptr = &cave[y][x];

		/* Redraw the grid */
		lite_spot(y, x);

		/* Process affected monsters */
		if (c_ptr->m_idx)
		{
			int chance = 25;

			monster_type *m_ptr = &m_list[c_ptr->m_idx];
			auto const r_ptr = m_ptr->race();

			/* Update the monster */
			update_mon(c_ptr->m_idx, false);

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags & RF_STUPID) chance = 10;

			/* Smart monsters always wake up */
			if (r_ptr->flags & RF_SMART) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->csleep && (rand_int(100) < chance))
			{
				/* Wake up! */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (m_ptr->ml)
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(m_name, m_ptr, 0);

					/* Dump a message */
					msg_format("%^s wakes up.", m_name);
				}
			}
		}
	}

	/* None left */
	temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite()
{
	int i;

	/* Apply flag changes */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		cave_type *c_ptr = &cave[y][x];

		/* No longer in the array */
		c_ptr->info &= ~(CAVE_TEMP);

		/* Darken the grid */
		c_ptr->info &= ~(CAVE_GLOW);

		/* Hack -- Forget "boring" grids */
		if (cave_plain_floor_grid(c_ptr))
		{
			/* Forget the grid */
			c_ptr->info &= ~(CAVE_MARK);

			/* Notice */
			/* note_spot(y, x); */
		}
	}

	/* Fully update the visuals */
	p_ptr->update |= (PU_UN_VIEW | PU_VIEW | PU_MONSTERS | PU_MON_LITE);

	/* Update stuff */
	update_stuff();

	/* Process the grids */
	for (i = 0; i < temp_n; i++)
	{
		int y = temp_y[i];
		int x = temp_x[i];

		/* Redraw the grid */
		lite_spot(y, x);
	}

	/* None left */
	temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];

	/* Avoid infinite recursion */
	if (c_ptr->info & (CAVE_TEMP)) return;

	/* Do not "leave" the current room */
	if (!(c_ptr->info & (CAVE_ROOM))) return;

	/* Paranoia -- verify space */
	if (temp_n == TEMP_MAX) return;

	/* Mark the grid as "seen" */
	c_ptr->info |= (CAVE_TEMP);

	/* Add it to the "seen" set */
	temp_y[temp_n] = y;
	temp_x[temp_n] = x;
	temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int y1, int x1)
{
	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(y1, x1);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < temp_n; i++)
	{
		x = temp_x[i], y = temp_y[i];

		/* Walls get lit, but stop light */
		if (!cave_floor_bold(y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(y + 1, x);
		cave_temp_room_aux(y - 1, x);
		cave_temp_room_aux(y, x + 1);
		cave_temp_room_aux(y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(y + 1, x + 1);
		cave_temp_room_aux(y - 1, x - 1);
		cave_temp_room_aux(y - 1, x + 1);
		cave_temp_room_aux(y + 1, x - 1);
	}

	/* Now, lite them all up at once */
	cave_temp_room_lite();
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int y1, int x1)
{
	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(y1, x1);

	/* Spread, breadth first */
	for (i = 0; i < temp_n; i++)
	{
		x = temp_x[i], y = temp_y[i];

		/* Walls get dark, but stop darkness */
		if (!cave_floor_bold(y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(y + 1, x);
		cave_temp_room_aux(y - 1, x);
		cave_temp_room_aux(y, x + 1);
		cave_temp_room_aux(y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(y + 1, x + 1);
		cave_temp_room_aux(y - 1, x - 1);
		cave_temp_room_aux(y - 1, x + 1);
		cave_temp_room_aux(y + 1, x - 1);
	}

	/* Now, darken them all at once */
	cave_temp_room_unlite();
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
void lite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("You are surrounded by a white light.");
	}

	/* Hook into the "project()" function */
	project(0, rad, p_ptr->py, p_ptr->px, dam, GF_LITE_WEAK, flg);

	/* Lite up the room */
	lite_room(p_ptr->py, p_ptr->px);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
void unlite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("Darkness surrounds you.");
	}

	/* Hook into the "project()" function */
	project(0, rad, p_ptr->py, p_ptr->px, dam, GF_DARK_WEAK, flg);

	/* Lite up the room */
	unlite_room(p_ptr->py, p_ptr->px);
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int typ, int dir, int dam, int rad)
{
	int tx, ty;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Use the given direction */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(0, (rad > 16) ? 16 : rad, ty, tx, dam, typ, flg));
}

/*
 * Cast a cloud spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
void fire_cloud(int typ, int dir, int dam, int rad, int time)
{
	int tx, ty;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_STAY;

	/* Use the given direction */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		flg &= ~(PROJECT_STOP);
		tx = target_col;
		ty = target_row;
	}
	project_time = time;

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	project(0, (rad > 16) ? 16 : rad, ty, tx, dam, typ, flg);
}

/*
 * Cast a wave spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
void fire_wave(int typ, int dir, int dam, int rad, int time, s32b eff)
{
	project_time_effect = eff;
	fire_cloud(typ, dir, dam, rad, time);
}

/*
 * Cast a persistant beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
void fire_wall(int typ, int dir, int dam, int time)
{
	int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_STAY | PROJECT_GRID;
	project_time = time;
	project_hook(typ, dir, dam, flg);
}



void teleport_swap(int dir)
{
	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	int tx, ty;
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}
	else
	{
		tx = p_ptr->px + ddx[dir];
		ty = p_ptr->py + ddy[dir];
	}

	cave_type *c_ptr = &cave[ty][tx];

	if (!c_ptr->m_idx)
	{
		msg_print("You can't trade places with that!");
	}
	else
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		auto const r_ptr = m_ptr->race();

		if (r_ptr->flags & RF_RES_TELE)
		{
			msg_print("Your teleportation is blocked!");
		}
		else
		{
			cave[p_ptr->py][p_ptr->px].m_idx = c_ptr->m_idx;

			/* Update the old location */
			c_ptr->m_idx = 0;

			/* Move the monster */
			m_ptr->fy = p_ptr->py;
			m_ptr->fx = p_ptr->px;

			/* Move the player */
			p_ptr->px = tx;
			p_ptr->py = ty;

			tx = m_ptr->fx;
			ty = m_ptr->fy;

			/* Update the monster (new location) */
			update_mon(cave[ty][tx].m_idx, true);

			/* Redraw the old grid */
			lite_spot(ty, tx);

			/* Redraw the new grid */
			lite_spot(p_ptr->py, p_ptr->px);

			/* Execute the inscription */
			c_ptr = &cave[m_ptr->fy][m_ptr->fx];
			if (c_ptr->inscription)
			{
				if (inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_MONST_WALK)
				{
					execute_inscription(c_ptr->inscription, m_ptr->fy, m_ptr->fx);
				}
			}
			c_ptr = &cave[p_ptr->py][p_ptr->px];
			if (c_ptr->inscription)
			{
				msg_format("There is an inscription here: %s", inscription_info[c_ptr->inscription].text);
				if (inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_WALK)
				{
					execute_inscription(c_ptr->inscription, p_ptr->py, p_ptr->px);
				}
			}

			/* Check for new panel (redraw map) */
			verify_panel();

			/* Update stuff */
			p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

			/* Update the monsters */
			p_ptr->update |= (PU_DISTANCE);

			/* Window stuff */
			p_ptr->window |= (PW_OVERHEAD);

			/* Handle stuff XXX XXX XXX */
			handle_stuff();
		}
	}
}

void swap_position(int lty, int ltx)
{
	int tx = ltx, ty = lty;
	cave_type * c_ptr;
	monster_type * m_ptr;

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	c_ptr = &cave[ty][tx];

	if (!c_ptr->m_idx)
	{
		/* Keep trace of the old location */
		tx = p_ptr->px;
		ty = p_ptr->py;

		/* Move the player */
		p_ptr->px = ltx;
		p_ptr->py = lty;

		/* Redraw the old grid */
		lite_spot(ty, tx);

		/* Redraw the new grid */
		lite_spot(p_ptr->py, p_ptr->px);

		/* Check for new panel (redraw map) */
		verify_panel();

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff XXX XXX XXX */
		handle_stuff();
	}
	else
	{
		m_ptr = &m_list[c_ptr->m_idx];

		cave[p_ptr->py][p_ptr->px].m_idx = c_ptr->m_idx;

		/* Update the old location */
		c_ptr->m_idx = 0;

		/* Move the monster */
		m_ptr->fy = p_ptr->py;
		m_ptr->fx = p_ptr->px;

		/* Move the player */
		p_ptr->px = tx;
		p_ptr->py = ty;

		tx = m_ptr->fx;
		ty = m_ptr->fy;

		/* Update the monster (new location) */
		update_mon(cave[ty][tx].m_idx, true);

		/* Redraw the old grid */
		lite_spot(ty, tx);

		/* Redraw the new grid */
		lite_spot(p_ptr->py, p_ptr->px);

		/* Check for new panel (redraw map) */
		verify_panel();

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);

		/* Handle stuff XXX XXX XXX */
		handle_stuff();
	}
}


/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
bool project_hook(int typ, int dir, int dam, int flg)
{
	int tx, ty;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Use the given direction */
	tx = p_ptr->px + ddx[dir];
	ty = p_ptr->py + ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}

	/* Analyze the "dir" and the "target", do NOT explode */
	return (project(0, 0, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
void fire_bolt(int typ, int dir, int dam)
{
	project_hook(typ, dir, dam, PROJECT_STOP | PROJECT_KILL);
}


/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
void fire_beam(int typ, int dir, int dam)
{
	project_hook(typ, dir, dam, PROJECT_BEAM | PROJECT_KILL);
}


/*
 * Cast a bolt spell, or rarely, a beam spell
 */
void fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		fire_beam(typ, dir, dam);
	}
	else
	{
		fire_bolt(typ, dir, dam);
	}
}


/*
 * Some of the old functions
 */
void lite_line(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	project_hook(GF_LITE_WEAK, dir, damroll(6, 8), flg);
}


bool drain_life(int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}


void wall_to_mud(int dir)
{
	project_hook(GF_KILL_WALL, dir, 20 + randint(30),
		PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
}


void wizard_lock(int dir)
{
	project_hook(GF_JAM_DOOR, dir, 20 + randint(30),
		PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL);
}

void slow_monster(int dir)
{
	project_hook(GF_OLD_SLOW, dir, p_ptr->lev, PROJECT_STOP | PROJECT_KILL);
}


void sleep_monster(int dir)
{
	project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, PROJECT_STOP | PROJECT_KILL);
}


void confuse_monster(int dir, int plev)
{
	project_hook(GF_OLD_CONF, dir, plev, PROJECT_STOP | PROJECT_KILL);
}


void poly_monster(int dir)
{
	project_hook(GF_OLD_POLY, dir, p_ptr->lev, PROJECT_STOP | PROJECT_KILL);
}


void fear_monster(int dir, int plev)
{
	project_hook(GF_TURN_ALL, dir, plev, PROJECT_STOP | PROJECT_KILL);
}


void teleport_monster(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return;
	}

	project_hook(GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg);
}

void wall_stone(int y, int x)
{
	auto const &f_info = game->edit_data.f_info;

	cave_type *c_ptr = &cave[y][x];
	int flg = PROJECT_GRID | PROJECT_ITEM;
	auto const featflags = f_info[c_ptr->feat].flags;

	project(0, 1, y, x, 0, GF_STONE_WALL, flg);

	if (!(featflags & FF_PERMANENT) && !(featflags & FF_WALL))
		cave_set_feat(y, x, FEAT_FLOOR);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}


void destroy_doors_touch()
{
	project(0, 1, p_ptr->py, p_ptr->px, 0, GF_KILL_DOOR,
		PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE);
}

void sleep_monsters_touch()
{
	project(0, 1, p_ptr->py, p_ptr->px, p_ptr->lev, GF_OLD_SLEEP, PROJECT_KILL | PROJECT_HIDE);
}


void call_chaos()
{
	int Chaos_type, dummy, dir;
	int plev = p_ptr->lev;
	bool line_chaos = false;

	int hurt_types[30] =
	        {
	                GF_ELEC, GF_POIS, GF_ACID, GF_COLD,
	                GF_FIRE, GF_MISSILE, GF_ARROW, GF_PLASMA,
	                GF_HOLY_FIRE, GF_WATER, GF_LITE, GF_DARK,
	                GF_FORCE, GF_INERTIA, GF_MANA, GF_METEOR,
	                GF_ICE, GF_CHAOS, GF_NETHER, GF_DISENCHANT,
	                GF_SHARDS, GF_SOUND, GF_NEXUS, GF_CONFUSION,
	                GF_TIME, GF_GRAVITY, GF_ROCKET, GF_NUKE,
	                GF_HELL_FIRE, GF_DISINTEGRATE
	        };

	Chaos_type = hurt_types[randint(30) - 1];
	if (randint(4) == 1) line_chaos = true;

	if (randint(6) == 1)
	{
		for (dummy = 1; dummy < 10; dummy++)
		{
			if (dummy - 5)
			{
				if (line_chaos)
					fire_beam(Chaos_type, dummy, 75);
				else
					fire_ball(Chaos_type, dummy, 75, 2);
			}
		}
	}
	else if (randint(3) == 1)
	{
		fire_ball(Chaos_type, 0, 300, 8);
	}
	else
	{
		if (!get_aim_dir(&dir)) return;
		if (line_chaos)
			fire_beam(Chaos_type, dir, 150);
		else
			fire_ball(Chaos_type, dir, 150, 3 + (plev / 35));
	}
}


static void activate_hi_summon()
{
	int i;

	for (i = 0; i < (randint(9) + (dun_level / 40)); i++)
	{
		switch (randint(26) + (dun_level / 20) )
		{
		case 1:
		case 2:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANT);
			break;
		case 3:
		case 4:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_SPIDER);
			break;
		case 5:
		case 6:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HOUND);
			break;
		case 7:
		case 8:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HYDRA);
			break;
		case 9:
		case 10:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANGEL);
			break;
		case 11:
		case 12:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNDEAD);
			break;
		case 13:
		case 14:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_DRAGON);
			break;
		case 15:
		case 16:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_DEMON);
			break;
		case 17:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_WRAITH);
			break;
		case 18:
		case 19:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNIQUE);
			break;
		case 20:
		case 21:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_UNDEAD);
			break;
		case 22:
		case 23:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_DRAGON);
			break;
		case 24:
		case 25:
			summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_HI_DEMON);
			break;
		default:
			summon_specific(p_ptr->py, p_ptr->px, (((dun_level * 3) / 2) + 5), 0);
		}
	}
}


/*
 * Activate the evil Topi Ylinen curse
 * rr9: Stop the nasty things when a Cyberdemon is summoned
 * or the player gets paralyzed.
 */
void activate_ty_curse()
{
	int i = 0;
	bool stop_ty = false;

	do
	{
		switch (randint(27))
		{
		case 1:
		case 2:
		case 3:
		case 16:
		case 17:
			aggravate_monsters(1);
			if (randint(6) != 1) break;
		case 4:
		case 5:
		case 6:
			activate_hi_summon();
			if (randint(6) != 1) break;
case 7: case 8: case 9: case 18:
			summon_specific(p_ptr->py, p_ptr->px, dun_level, 0);
			if (randint(6) != 1) break;
case 10: case 11: case 12:
			msg_print("You feel your life draining away...");
			lose_exp(p_ptr->exp / 16);
			if (randint(6) != 1) break;
case 13: case 14: case 15: case 19: case 20:
			if (p_ptr->free_act && (randint(100) < p_ptr->skill_sav))
			{
				/* Do nothing */ ;
			}
			else
			{
				msg_print("You feel like a statue!");
				if (p_ptr->free_act)
					set_paralyzed(randint(3));
				else
					set_paralyzed(randint(13));
				stop_ty = true;
			}
			if (randint(6) != 1) break;
case 21: case 22: case 23:
			do_dec_stat((randint(6)) - 1, STAT_DEC_NORMAL);
			if (randint(6) != 1) break;
		case 24:
			msg_print("Huh? Who am I? What am I doing here?");
			lose_all_info();
			break;
		case 25:
			/*
			 * Only summon Cyberdemons deep in the dungeon.
			 */
			if ((dun_level > 65) && !stop_ty)
			{
				summon_cyber();
				stop_ty = true;
				break;
			}
		default:
			while (i < 6)
			{
				do
				{
					do_dec_stat(i, STAT_DEC_NORMAL);
				}
				while (randint(2) == 1);

				i++;
			}
		}
	}
	while ((randint(3) == 1) && !stop_ty);
}

/*
 * Activate the ultra evil Dark God curse
 */
void activate_dg_curse()
{
	int i = 0;
	bool stop_dg = false;

	do
	{
		switch (randint(30))
		{
		case 1:
		case 2:
		case 3:
		case 16:
		case 17:
			aggravate_monsters(1);
			if (randint(8) != 1) break;
		case 4:
		case 5:
		case 6:
			msg_print("Oh! You feel that the curse is replicating itself!");
			curse_equipment_dg(100, 50 * randint(2));
			if (randint(8) != 1) break;
		case 7:
		case 8:
		case 9:
		case 18:
			curse_equipment(100, 50 * randint(2));
			if (randint(8) != 1) break;
		case 10:
		case 11:
		case 12:
			msg_print("You feel your life draining away...");
			lose_exp(p_ptr->exp / 12);
			if (rand_int(2))
			{
				msg_print("You feel the coldness of the Black Breath attacking you!");
				p_ptr->black_breath = true;
			}
			if (randint(8) != 1) break;
		case 13:
		case 14:
		case 15:
			if (p_ptr->free_act && (randint(100) < p_ptr->skill_sav))
			{
				/* Do nothing */ ;
			}
			else
			{
				msg_print("You feel like a statue!");
				if (p_ptr->free_act)
					set_paralyzed(randint(3));
				else
					set_paralyzed(randint(13));
				stop_dg = true;
			}
			if (randint(7) != 1) break;
		case 19:
		case 20:
			{
				msg_print("Woah! You see 10 little Morgoths dancing before you!");
				set_confused(p_ptr->confused + randint(13 * 2));
				if (rand_int(2)) stop_dg = true;
			}
			if (randint(7) != 1) break;
		case 21:
		case 22:
		case 23:
			do_dec_stat((randint(6)) - 1, STAT_DEC_PERMANENT);
			if (randint(7) != 1) break;
		case 24:
			msg_print("Huh? Who am I? What am I doing here?");
			lose_all_info();
			break;
case 27: case 28: case 29:
			if (p_ptr->inventory[INVEN_WIELD].k_ptr)
			{
				msg_print("Your weapon now seems useless...");
				p_ptr->inventory[INVEN_WIELD].art_flags = TR_NEVER_BLOW;
			}
			break;
		case 25:
			/*
			 * Only summon Thunderlords not too shallow in the dungeon.
			 */
			if ((dun_level > 25) && !stop_dg)
			{
				msg_print("Oh! You attracted some evil Thunderlords!");
				summon_dragon_riders();

				/* This is evil -- DG */
				if (rand_int(2)) stop_dg = true;
				break;
			}
		default:
			while (i < 6)
			{
				do
				{
					do_dec_stat(i, STAT_DEC_NORMAL);
				}
				while (randint(2) == 1);

				i++;
			}
		}
	}
	while ((randint(4) == 1) && !stop_dg);
}


void summon_cyber()
{
	int i;
	int max_cyber = (dun_level / 50) + randint(6);

	for (i = 0; i < max_cyber; i++)
	{
		summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_HI_DEMON);
	}
}

static void summon_dragon_riders()
{
	int i;
	int max_dr = (dun_level / 50) + randint(6);

	for (i = 0; i < max_dr; i++)
	{
		summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_THUNDERLORD);
	}
}

void confuse_monsters(int dam)
{
	project_hack(GF_OLD_CONF, dam);
}

void charm_monsters(int dam)
{
	project_hack(GF_CHARM, dam);
}

void charm_animals(int dam)
{
	project_hack(GF_CONTROL_ANIMAL, dam);
}

void stun_monsters(int dam)
{
	project_hack(GF_STUN, dam);
}

void mindblast_monsters(int dam)
{
	project_hack(GF_PSI, dam);
}

void banish_monsters(int dist)
{
	project_hack(GF_AWAY_ALL, dist);
}

void turn_monsters(int dam)
{
	project_hack(GF_TURN_ALL, dam);
}

void charm_monster(int dir, int plev)
{
	project_hook(GF_CHARM, dir, plev, PROJECT_STOP | PROJECT_KILL);
}

void control_one_undead(int dir, int plev)
{
	project_hook(GF_CONTROL_UNDEAD, dir, plev, PROJECT_STOP | PROJECT_KILL);
}

void charm_animal(int dir, int plev)
{
	project_hook(GF_CONTROL_ANIMAL, dir, plev, PROJECT_STOP | PROJECT_KILL);
}

void change_wild_mode()
{
	if (p_ptr->immovable && !p_ptr->wild_mode)
	{
		msg_print("Hmm, blinking there will take time.");
	}

	if (p_ptr->word_recall && !p_ptr->wild_mode)
	{
		msg_print("You will soon be recalled.");
		return;
	}

	p_ptr->wild_mode = !p_ptr->wild_mode;

	autosave_checkpoint();

	/* Leaving */
	p_ptr->leaving = true;
}


void alter_reality()
{
	msg_print("The world changes!");

	autosave_checkpoint();

	/* Leaving */
	p_ptr->leaving = true;
}

void heal_insanity(int val)
{
	if (p_ptr->csane < p_ptr->msane)
	{
		p_ptr->csane += val;

		if (p_ptr->csane >= p_ptr->msane)
		{
			p_ptr->csane = p_ptr->msane;
			p_ptr->csane_frac = 0;
		}

		p_ptr->redraw |= (PR_FRAME);
		p_ptr->window |= (PW_PLAYER);

		if (val < 5)
		{
			msg_print("You feel a little better.");
		}
		else if (val < 15)
		{
			msg_print("You feel better.");
		}
		else if (val < 35)
		{
			msg_print("You feel much better.");
		}
		else
		{
			msg_print("You feel very good.");
		}
	}
}

/*
 * Send the player shooting through walls in the given direction until
 * they reach a non-wall space, or a monster, or a permanent wall.
 */
bool passwall(int dir, bool safe)
{
	auto const &f_info = game->edit_data.f_info;
	auto const &dungeon_flags = game->dungeon_flags;

	int x = p_ptr->px;
	int y = p_ptr->py;
	int ox = p_ptr->px;
	int oy = p_ptr->py;
	int lx = p_ptr->px;
	int ly = p_ptr->py;
	cave_type *c_ptr;
	bool ok = false;

	if (p_ptr->wild_mode) return false;
	if (p_ptr->inside_quest) return false;
	if (dungeon_flags & DF_NO_TELEPORT) return false;

	/* Must go somewhere */
	if (dir == 5) return false;

	while (true)
	{
		x += ddx[dir];
		y += ddy[dir];
		c_ptr = &cave[y][x];

		/* Perm walls stops the transfer */
		if ((!in_bounds(y, x)) && (f_info[c_ptr->feat].flags & FF_PERMANENT))
		{
			/* get the last working position */
			x -= ddx[dir];
			y -= ddy[dir];
			ok = false;
			break;
		}

		/* Never on a monster */
		if (c_ptr->m_idx) continue;

		/* Never stop in vaults */
		if (c_ptr->info & CAVE_ICKY) continue;

		/* From now on, the location COULD be used in special case */
		lx = x;
		ly = y;

		/* Pass over walls */
		if (f_info[c_ptr->feat].flags & FF_WALL) continue;

		/* So it must be ok */
		ok = true;
		break;
	}

	if (!ok)
	{
		x = lx;
		y = ly;

		if (!safe)
		{
			msg_print("You emerge in the wall!");
			take_hit(damroll(10, 8), "becoming one with a wall");
		}
		place_floor_convert_glass(y, x);
	}

	/* Move */
	p_ptr->px = x;
	p_ptr->py = y;

	/* Redraw the old spot */
	lite_spot(oy, ox);

	/* Redraw the new spot */
	lite_spot(p_ptr->py, p_ptr->px);

	/* Check for new panel (redraw map) */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();

	return true;
}

/*
 * Print a batch of dungeons.
 */
static void print_dungeon_batch(std::vector<int> const &dungeon_idxs,
				int start,
				bool mode)
{
	auto const &d_info = game->edit_data.d_info;

	int i, j;
	byte attr;

	if (mode) prt(format("     %-31s", "Name"), 1, 20);

	for (i = 0, j = start; i < 20 && j < static_cast<int>(dungeon_idxs.size()); i++, j++)
	{
		auto d_ptr = &d_info[dungeon_idxs[j]];

		char buf[80];
		strnfmt(buf, 80, "  %c) %-30s", I2A(i), d_ptr->name.c_str());

		if (mode)
		{
			if (d_ptr->min_plev > p_ptr->lev)
			{
				attr = TERM_L_DARK;
			}
			else
			{
				attr = TERM_WHITE;
			}
			c_prt(attr, buf, 2 + i, 20);
		}
	}
	if (mode) prt("", 2 + i, 20);

	prt(format("Select a dungeon (a-%c), * to list, @ to select by name, +/- to scroll:", I2A(i - 1)), 0, 0);
}

static int find_dungeon_by_name(char const *name)
{
	auto const &d_info = game->edit_data.d_info;

	/* Find the index corresponding to the name */
	for (std::size_t i = 1; i < d_info.size(); i++)
	{
		if (iequals(name, d_info[i].name))
		{
			return i;
		}
	}
	/* Not found */
	return -1;
}

static int reset_recall_aux()
{
	auto const &d_info = game->edit_data.d_info;

	char which;
	int start = 0;
	int ret;
	bool mode = false;

	// Dungeons available for recall
	std::vector<int> dungeons;
	for (size_t i = 1; i < d_info.size(); i++)
	{
		/* skip "blocked" dungeons */
		if (d_info[i].flags & DF_NO_RECALL) continue;

		if (max_dlv[i])
		{
			dungeons.push_back(i);
		}
	}

	screen_save_no_flush();

	while (true)
	{
		print_dungeon_batch(dungeons, start, mode);
		which = inkey();

		if (which == ESCAPE)
		{
			ret = -1;
			break;
		}

		else if (which == '*' || which == '?' || which == ' ')
		{
			mode = (mode) ? false : true;
			screen_load_no_flush();
		}

		else if (which == '+')
		{
			start += 20;
			assert(start > 0);
			if (static_cast<size_t>(start) >= dungeons.size())
			{
				start -= 20;
			}
			screen_load_no_flush();
		}

		else if (which == '-')
		{
			start -= 20;
			if (start < 0)
			{
				start += 20;
			}
			screen_load_no_flush();
		}

		else if (which == '@')
		{
			char buf[80];
			strcpy(buf, d_info[p_ptr->recall_dungeon].name.c_str());

			if (!get_string("Which dungeon? ", buf, 79)) continue;

			/* Find the index corresponding to the name */
			int i = find_dungeon_by_name(buf);

			if (i < 0)
			{
				msg_print("Never heard of that place!");
				msg_print(NULL);
				continue;
			}
			else if (d_info[i].flags & DF_NO_RECALL)
			{
				msg_print("This place blocks my magic!");
				msg_print(NULL);
				continue;
			}
			else if (d_info[i].min_plev > p_ptr->lev)
			{
				msg_print("You cannot go there yet!");
				msg_print(NULL);
				continue;
			}
			ret = i;
			break;
		}

		else
		{
			which = tolower(which);
			int i = start + A2I(which);

			if (i < 0)
			{
				bell();
				continue;
			}
			else if (static_cast<size_t>(i) >= dungeons.size()) // Cast to avoid compilation warning
			{
				bell();
				continue;
			}
			else
			{
				ret = dungeons[i];
				break;
			}
		}
	}

	screen_load_no_flush();

	return ret;
}

bool reset_recall(bool no_trepas_max_depth)
{
	auto const &d_info = game->edit_data.d_info;

	int dun, depth, max;

	/* Choose dungeon */
	dun = reset_recall_aux();

	if (dun < 1) return false;

	/* Choose depth */
	if (!no_trepas_max_depth)
		max = d_info[dun].maxdepth;
	else
		max = max_dlv[dun];

	depth = get_quantity(format("Which level in %s(%d-%d)? ",
				    d_info[dun].name.c_str(),
	                            d_info[dun].mindepth, max),
	                     max);

	if (depth < 1) return false;

	/* Enforce minimum level */
	if (depth < d_info[dun].mindepth) depth = d_info[dun].mindepth;

	/* Mega hack -- Forbid levels 99 and 100 */
	if ((depth == 99) || (depth == 100)) depth = 98;

	p_ptr->recall_dungeon = dun;
	max_dlv[p_ptr->recall_dungeon] = depth;

	return true;
}

/*
 * Creates a between gate
 */
void create_between_gate(int dist, int y, int x)
{
	auto const &f_info = game->edit_data.f_info;
	auto const &dungeon_flags = game->dungeon_flags;

	int ii, ij, plev = get_skill(SKILL_CONVEYANCE);

	if (dungeon_flags & DF_NO_TELEPORT)
	{
		msg_print("Not on special levels!");
		return;
	}

	if ((!x) || (!y))
	{
		msg_print("You open a Void Jumpgate. Choose a destination.");

		if (!tgt_pt(&ii, &ij)) return;
		p_ptr->energy -= 60 - plev;

		if (!cave_empty_bold(ij, ii) ||
		                (cave[ij][ii].info & CAVE_ICKY) ||
		                (distance(ij, ii, p_ptr->py, p_ptr->px) > dist) ||
		                (rand_int(plev * plev / 2) == 0))
		{
			msg_print("You fail to exit the void correctly!");
			p_ptr->energy -= 100;
			get_pos_player(10, &ij, &ii);
		}
	}
	else
	{
		ij = y;
		ii = x;
	}
	if (!(f_info[cave[p_ptr->py][p_ptr->px].feat].flags & FF_PERMANENT))
	{
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_BETWEEN);
		cave[p_ptr->py][p_ptr->px].special = ii + (ij << 8);
	}
	if (!(f_info[cave[ij][ii].feat].flags & FF_PERMANENT))
	{
		cave_set_feat(ij, ii, FEAT_BETWEEN);
		cave[ij][ii].special = p_ptr->px + (p_ptr->py << 8);
	}
}

/**
 * Geomancy
 */
typedef struct geomancy_entry {
	int skill;
	int feat;
	int min_skill_level;
} geomancy_entry;

static int choose_geomancy_feature(int n, geomancy_entry *table)
{
	int feat = -1;
	/* choose feature */
	while (feat < 0) {
		geomancy_entry *t = &table[rand_int(n)];

		/* Do we meet the requirements ?
		   And then select the features based on skill proportions */
		if ((get_skill(t->skill) >= t->min_skill_level) && magik(get_skill_scale(t->skill, 100)))
		{
			feat = t->feat;
		}
	}
	/* return */
	return feat;
}

static int rotate_dir(int dir, int mov)
{
	if (mov > 0)
	{
		switch (dir) {
		case 7: return 8;
		case 8: return 9;
		case 9: return 6;
		case 6: return 3;
		case 3: return 2;
		case 2: return 1;
		case 1: return 4;
		case 4: return 7;
		}
	}
	else if (mov < 0)
	{
		switch (dir) {
		case 7: return 4;
		case 4: return 1;
		case 1: return 2;
		case 2: return 3;
		case 3: return 6;
		case 6: return 9;
		case 9: return 8;
		case 8: return 7;
		}
	}

	return dir;
}

void geomancy_random_wall(int y, int x)
{
	auto const &f_info = game->edit_data.f_info;

#define TABLE_SIZE 4
	cave_type *c_ptr = &cave[y][x];
	int feat = -1;
	geomancy_entry table[TABLE_SIZE] = {
		/* Fire element */
		{ SKILL_FIRE, FEAT_SANDWALL, 1},
		/* Water element */
		{ SKILL_WATER, FEAT_TREES, 1},
		{ SKILL_WATER, FEAT_ICE_WALL, 12},
		/* Earth element */
		{ SKILL_EARTH, FEAT_WALL_EXTRA, 1}
	};

	/* Do not destroy permanent things */
	if (f_info[c_ptr->feat].flags & FF_PERMANENT)
	{
		return;
	}

	/* Choose feature */
	feat = choose_geomancy_feature(TABLE_SIZE, table);
	if (feat >= 0)
	{
		cave_set_feat(y, x, feat);
	}
#undef TABLE_SIZE
}

void geomancy_random_floor(int y, int x, bool kill_wall)
{
	auto const &f_info = game->edit_data.f_info;

#define TABLE_SIZE 9
	cave_type *c_ptr = &cave[y][x];
	int feat = -1;
	geomancy_entry table[TABLE_SIZE] = {
		/* Fire element */
		{ SKILL_FIRE, FEAT_SAND, 1},
		{ SKILL_FIRE, FEAT_SHAL_LAVA, 8},
		{ SKILL_FIRE, FEAT_DEEP_LAVA, 18},
		/* Water element */
		{ SKILL_WATER, FEAT_SHAL_WATER, 1},
		{ SKILL_WATER, FEAT_DEEP_WATER, 8},
		{ SKILL_WATER, FEAT_ICE, 18},
		/* Earth element */
		{ SKILL_EARTH, FEAT_GRASS, 1},
		{ SKILL_EARTH, FEAT_FLOWER, 8},
		{ SKILL_EARTH, FEAT_DARK_PIT, 18}
	};

	/* Do not destroy permanent things */
	if (f_info[c_ptr->feat].flags & FF_PERMANENT) {
		return;
	}
	if (!(kill_wall || (f_info[c_ptr->feat].flags & FF_FLOOR))) {
		return;
	}

	/* Choose feature */
	feat = choose_geomancy_feature(TABLE_SIZE, table);
	if (feat >= 0)
	{
		cave_set_feat(y, x, feat);
	}
#undef TABLE_SIZE
}

static bool geomancy_can_tunnel(int y, int x)
{
	switch (cave[y][x].feat)
	{
	case FEAT_WALL_EXTRA:
	case FEAT_WALL_OUTER:
	case FEAT_WALL_INNER:
	case FEAT_WALL_SOLID:
	case FEAT_MAGMA:
	case FEAT_QUARTZ:
	case FEAT_MAGMA_H:
	case FEAT_QUARTZ_H:
	case FEAT_MAGMA_K:
	case FEAT_QUARTZ_K:
	case FEAT_TREES:
	case FEAT_DEAD_TREE:
	case FEAT_SANDWALL:
	case FEAT_SANDWALL_H:
	case FEAT_SANDWALL_K:
	case FEAT_ICE_WALL:
		return true;
	default:
		return false;
	}
}

void geomancy_dig(int oy, int ox, int dir, int length)
{
	int dy = ddy[dir];
	int dx = ddx[dir];
	int y = dy + oy;
	int x = dx + ox;
	int i;

	for (i=0; i<length; i++)
	{
		/* stop at the end of tunnelable things */
		if (!geomancy_can_tunnel(y, x)) {
			break;
		}

		if (geomancy_can_tunnel(y - 1, x - 1)) { geomancy_random_wall(y - 1, x - 1); }
		if (geomancy_can_tunnel(y - 1, x    )) { geomancy_random_wall(y - 1, x    ); }
		if (geomancy_can_tunnel(y - 1, x + 1)) { geomancy_random_wall(y - 1, x + 1); }

		if (geomancy_can_tunnel(y    , x - 1)) { geomancy_random_wall(y    , x - 1); }
		if (geomancy_can_tunnel(y    , x + 1)) { geomancy_random_wall(y    , x + 1); }

		if (geomancy_can_tunnel(y + 1, x - 1)) { geomancy_random_wall(y + 1, x - 1); }
		if (geomancy_can_tunnel(y + 1, x    )) { geomancy_random_wall(y + 1, x    ); }
		if (geomancy_can_tunnel(y + 1, x + 1)) { geomancy_random_wall(y + 1, x + 1); }

		y = y + dy;
		x = x + dx;
	}

	/* Step back towards origin */
	y = y - dy;
	x = x - dx;
	while ((y != oy) || (x != ox))
	{
		geomancy_random_floor(y, x, true);

		/* Should we branch ? */
		if (magik(20))
		{
			int rot = magik(50) ? -1 : 1;
			geomancy_dig(y, x, rotate_dir(dir, rot), length / 3);
		}

		y = y - dy;
		x = x - dx;
	}
}

void channel_the_elements(int y, int x, int level)
{
	// Type of water to use (if any)
	auto water_type = []() -> int {
		return (get_skill(SKILL_WATER) >= 18) ? GF_WAVE : GF_WATER;
	};
	// Do we use hellfire?
	auto use_hellfire = []() -> bool {
		return get_skill(SKILL_FIRE) >= 15;
	};
	// Type of fire to use (if any)
	auto fire_type = [&use_hellfire]() -> int {
		return use_hellfire() ? GF_HELL_FIRE : GF_FIRE;
	};

	switch (cave[y][x].feat)
	{
	case FEAT_GRASS:
		hp_player(p_ptr->mhp * (5 + get_skill_scale(SKILL_EARTH, 20)) / 100);
		break;

	case FEAT_FLOWER:
		hp_player(p_ptr->mhp * (5 + get_skill_scale(SKILL_EARTH, 30)) / 100);
		break;

	case FEAT_DARK_PIT:
	{
		int dir, type;
		if (!get_aim_dir(&dir)) break;

		type = (get_skill(SKILL_EARTH) >= 18) ? GF_NETHER : GF_DARK;

		fire_bolt(type, dir, damroll(10, get_skill(SKILL_EARTH)));

		break;
	}

	case FEAT_SHAL_WATER:
	{
		int dir;
		if (!get_aim_dir(&dir)) break;

		if (get_skill(SKILL_WATER) >= 8)
		{
			fire_beam(water_type(), dir, damroll(3, get_skill(SKILL_WATER)));
		}
		else
		{
			fire_bolt(water_type(), dir, damroll(3, get_skill(SKILL_WATER)));
		}

		break;
	}

	case FEAT_DEEP_WATER:
	{
		int dir;
		if (!get_aim_dir(&dir)) break;

		if (get_skill(SKILL_WATER) >= 8)
		{
			fire_beam(water_type(), dir, damroll(5, get_skill(SKILL_WATER)));
		}
		else
		{
			fire_bolt(water_type(), dir, damroll(5, get_skill(SKILL_WATER)));
		}

		break;
	}

	case FEAT_ICE:
	{
		int dir;
		if (!get_aim_dir(&dir)) break;

		if (get_skill(SKILL_WATER) >= 12)
		{
			fire_ball(GF_ICE, dir, get_skill_scale(SKILL_WATER, 340), 3);
		}
		else
		{
			fire_bolt(GF_ICE, dir, damroll(3, get_skill(SKILL_WATER)));
		}

		break;
	}

	case FEAT_SAND:
	{
		int type, dur;

		type = use_hellfire() ? SHIELD_GREAT_FIRE : SHIELD_FIRE;

		dur = randint(20) + level + get_skill(SKILL_AIR);
		set_shield(dur, 0, type, 5 + get_skill_scale(SKILL_FIRE, 20), 5 + get_skill_scale(SKILL_FIRE, 14));
		set_blind(dur);

		break;
	}

	case FEAT_SHAL_LAVA:
	{
		int dir;
		if (!get_aim_dir(&dir)) break;

		fire_bolt(fire_type(), dir, damroll(get_skill_scale(SKILL_FIRE, 30), 15));
		break;
	}

	case FEAT_DEEP_LAVA:
	{
		int dir;
		if (!get_aim_dir(&dir)) break;

		fire_ball(fire_type(), dir, damroll(get_skill_scale(SKILL_FIRE, 30), 15), 3);
		break;
	}

	default:
		msg_print("You cannot channel this area.");
		return;
	}

	/* Drain area? */
	if (magik(100 - level))
	{
		if (cave[y][x].feat == FEAT_FLOWER)
		{
			cave_set_feat(y, x, FEAT_GRASS);
		}
		else
		{
			cave_set_feat(y, x, FEAT_FLOOR);
		}
		msg_print("The area is drained.");
	}
}
