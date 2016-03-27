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
#include "dungeon_info_type.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "hook_identify_in.hpp"
#include "hooks.hpp"
#include "melee2.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells3.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
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
		i = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;
		j = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;

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
	int a, i, j;

	for (a = 0; a < rad * rad + 11; a++)
	{
		i = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;
		j = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;

		if (!in_bounds(p_ptr->py + j, p_ptr->px + i)) continue;
		if (distance(p_ptr->py, p_ptr->px, p_ptr->py + j, p_ptr->px + i) > rad) continue;

		if (cave_clean_bold(p_ptr->py + j, p_ptr->px + i) && (f_info[cave[p_ptr->py][p_ptr->px].feat].flags1 & FF1_SUPPORT_GROWTH))
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
	int a, i, j;

	for (a = 0; a < rad * rad + 11; a++)
	{
		i = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;
		j = (Rand_mod((rad * 2) + 1)-rad + Rand_mod((rad * 2) + 1)-rad) / 2;

		if (!in_bounds(p_ptr->py + j, p_ptr->px + i)) continue;
		if (distance(p_ptr->py, p_ptr->px, p_ptr->py + j, p_ptr->px + i) > rad) continue;

		if (cave_clean_bold(p_ptr->py + j, p_ptr->px + i) && (f_info[cave[p_ptr->py][p_ptr->px].feat].flags1 & FF1_SUPPORT_GROWTH))
		{
			cave_set_feat(p_ptr->py + j, p_ptr->px + i, FEAT_GRASS);
		}
	}
}

/*
 * Increase players hit points, notice effects
 */
bool_ hp_player(int num)
{
	bool_ dead = p_ptr->chp < 0;

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

		/* Notice */
		return (TRUE);
	}

	/* Ignore */
	return (FALSE);
}



/*
 * Leave a "glyph of warding" which prevents monster movement
 */
void warding_glyph(void)
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

void explosive_rune(void)
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
static cptr desc_stat_pos[] =
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

static cptr long_desc_stat[] =
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
static cptr desc_stat_neg[] =
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
bool_ do_dec_stat(int stat, int mode)
{
	bool_ sust = FALSE;

	/* Access the "sustain" */
	switch (stat)
	{
	case A_STR:
		if (p_ptr->sustain_str) sust = TRUE;
		break;
	case A_INT:
		if (p_ptr->sustain_int) sust = TRUE;
		break;
	case A_WIS:
		if (p_ptr->sustain_wis) sust = TRUE;
		break;
	case A_DEX:
		if (p_ptr->sustain_dex) sust = TRUE;
		break;
	case A_CON:
		if (p_ptr->sustain_con) sust = TRUE;
		break;
	case A_CHR:
		if (p_ptr->sustain_chr) sust = TRUE;
		break;
	}

	/* Sustain */
	if (sust)
	{
		/* Message */
		msg_format("You feel %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (dec_stat(stat, 10, mode))
	{
		/* Message */
		msg_format("You feel very %s.", desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool_ do_res_stat(int stat, bool_ full)
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
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Increases a stat by one randomized level             -RAK-
 *
 * Note that this function (used by stat potions) now restores
 * the stat BEFORE increasing it.
 */
static bool_ inc_stat(int stat)
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
		return (TRUE);
	}

	/* Nothing to gain */
	return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool_ do_inc_stat(int stat)
{
	bool_ res;

	/* Restore strength */
	res = res_stat(stat, TRUE);

	/* Attempt to increase */
	if (inc_stat(stat))
	{
		/* Message */
		msg_format("Wow!  You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format("You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Process all identify hooks
 */
void identify_hooks(int i, object_type *o_ptr, identify_mode mode)
{
	/* Process the appropriate hooks */
	hook_identify_in in = { o_ptr, mode };
	process_hooks_new(HOOK_IDENTIFY, &in, NULL);
}


/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
bool_ identify_pack(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Process the appropriate hooks */
		identify_hooks(i, o_ptr, IDENT_NORMAL);
	}

	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	return TRUE;
}

/*
 * common portions of identify_fully and identify_pack_fully
 */
static void make_item_fully_identified(object_type *o_ptr)
{
	/* Identify it fully */
	object_aware(o_ptr);
	object_known(o_ptr);

	/* Mark the item as fully known */
	o_ptr->ident |= (IDENT_MENTAL);
}

/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack_fully(void)
{
	int i;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		make_item_fully_identified(o_ptr);

		/* Process the appropriate hooks */
		identify_hooks(i, o_ptr, IDENT_FULL);
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

static bool_ remove_curse_object(object_type *o_ptr, bool_ all)
{
	u32b f1, f2, f3, f4, f5, esp;

	/* Skip non-objects */
	if (!o_ptr->k_idx) return FALSE;

	/* Uncursed already */
	if (!cursed_p(o_ptr)) return FALSE;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Heavily Cursed Items need a special spell */
	if (!all && (f3 & (TR3_HEAVY_CURSE))) return FALSE;

	/* Perma-Cursed Items can NEVER be uncursed */
	if (f3 & (TR3_PERMA_CURSE)) return FALSE;

	/* Uncurse it */
	o_ptr->ident &= ~(IDENT_CURSED);

	/* Hack -- Assume felt */
	o_ptr->ident |= (IDENT_SENSE);

	if (o_ptr->art_flags3 & (TR3_CURSED))
		o_ptr->art_flags3 &= ~(TR3_CURSED);

	if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
		o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

	/* Take note */
	o_ptr->sense = SENSE_UNCURSED;

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

	return TRUE;
}

/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
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
bool_ remove_curse(void)
{
	return (remove_curse_aux(FALSE) ? TRUE : FALSE);
}

/*
 * Remove all curses
 */
bool_ remove_all_curse(void)
{
	return (remove_curse_aux(TRUE) ? TRUE : FALSE);
}



/*
 * Restores any drained experience
 */
bool_ restore_level(void)
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
		return (TRUE);
	}

	/* No effect */
	return (FALSE);
}


bool_ alchemy(void) /* Turns an object into gold, gain some of its value in a shop */
{
	int item, amt = 1;
	int old_number;
	long price;
	bool_ force = FALSE;
	char o_name[80];
	char out_val[160];

	/* Hack -- force destruction */
	if (command_arg > 0) force = TRUE;

	/* Get an item */
	if (!get_item(&item,
		      "Turn which item to gold? ",
		      "You have nothing to turn to gold.",
		      (USE_INVEN | USE_FLOOR),
		      object_filter::True()))
	{
		return (FALSE);
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* See how many items */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return FALSE;
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, TRUE, 3);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		/* Make a verification */
		sprintf(out_val, "Really turn %s to gold? ", o_name);
		if (!get_check(out_val)) return FALSE;
	}

	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr) || o_ptr->art_name)
	{
		byte feel = SENSE_SPECIAL;

		/* Message */
		msg_format("You fail to turn %s to gold!", o_name);

		/* Hack -- Handle icky artifacts */
		if (cursed_p(o_ptr)) feel = SENSE_TERRIBLE;

		/* Hack -- inscribe the artifact */
		o_ptr->sense = feel;

		/* We have "felt" it (again) */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
		return FALSE;
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

	return TRUE;
}




/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistances of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge(FILE *fff)
{
	int i = 0, j, k;

	u32b f1 = 0L, f2 = 0L, f3 = 0L, f4 = 0L, f5 = 0L, esp = 0L;

	int iter;  /* Iterator for a loop */

	object_type *o_ptr;

	char Dummy[80];

	cptr info[200];

	strcpy (Dummy, "");

	/* Acquire item flags from equipment */
	for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
	{
		u32b t1, t2, t3, t4, t5, esp;

		o_ptr = &p_ptr->inventory[k];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
		object_flags(o_ptr, &t1, &t2, &t3, &t4, &t5, &esp);

		/* Extract flags */
		f1 |= t1;
		f2 |= t2;
		f3 |= t3;
	}

	if (death)
	{
		static char buf[250];

		sprintf(buf, "You are dead, killed by %s %s.",
		        died_from, describe_player_location());
		info[i++] = buf;
	}

	/* Racial powers... */
	if (p_ptr->body_monster != 0)
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

		if (r_ptr->flags1 & RF1_CHAR_CLEAR ||
		                r_ptr->flags1 & RF1_ATTR_CLEAR)
			info[i++] = "You are transparent.";
		if ((r_ptr->flags1 & RF1_CHAR_MULTI) ||
		                (r_ptr->flags2 & RF2_SHAPECHANGER))
			info[i++] = "Your form constantly changes.";
		if (r_ptr->flags1 & RF1_ATTR_MULTI)
			info[i++] = "Your color constantly changes.";
		if (r_ptr->flags1 & RF1_NEVER_BLOW)
			info[i++] = "You do not have a physical weapon.";
		if (r_ptr->flags1 & RF1_NEVER_MOVE)
			info[i++] = "You cannot move.";
		if ((r_ptr->flags1 & RF1_RAND_25) &&
		                (r_ptr->flags1 & RF1_RAND_50))
			info[i++] = "You move extremely erratically.";
		else if (r_ptr->flags1 & RF1_RAND_50)
			info[i++] = "You move somewhat erratically.";
		else if (r_ptr->flags1 & RF1_RAND_25)
			info[i++] = "You move a bit erratically.";
		if (r_ptr->flags2 & RF2_STUPID)
			info[i++] = "You are very stupid (INT -4).";
		if (r_ptr->flags2 & RF2_SMART)
			info[i++] = "You are very smart (INT +4).";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_CAN_SPEAK)
			info[i++] = "You can speak.";
		else
			info[i++] = "You cannot speak.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_COLD_BLOOD)
			info[i++] = "You are cold blooded.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_EMPTY_MIND)
			info[i++] = "You have an empty mind.";
		if (r_ptr->flags2 & RF2_WEIRD_MIND)
			info[i++] = "You have a weird mind.";
		if (r_ptr->flags4 & RF4_MULTIPLY)
			info[i++] = "You can multiply.";
		if (r_ptr->flags2 & RF2_POWERFUL)
			info[i++] = "You have strong breath.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_ELDRITCH_HORROR)
			info[i++] = "You are an eldritch horror.";
		if (r_ptr->flags2 & RF2_OPEN_DOOR)
			info[i++] = "You can open doors.";
		else
			info[i++] = "You cannot open doors.";
		if (r_ptr->flags2 & RF2_BASH_DOOR)
			info[i++] = "You can bash doors.";
		else
			info[i++] = "You cannot bash doors.";
		if (r_ptr->flags2 & RF2_PASS_WALL)
			info[i++] = "You can pass walls.";
		if (r_ptr->flags2 & RF2_KILL_WALL)
			info[i++] = "You destroy walls.";
		/* Not implemented */
		if (r_ptr->flags2 & RF2_MOVE_BODY)
			info[i++] = "You can move monsters.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_ORC)
			info[i++] = "You have orc blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_TROLL)
			info[i++] = "You have troll blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_GIANT)
			info[i++] = "You have giant blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_DRAGON)
			info[i++] = "You have dragon blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_DEMON)
			info[i++] = "You have demon blood in your veins.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_UNDEAD)
			info[i++] = "You are an undead.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_ANIMAL)
			info[i++] = "You are an animal.";
		/* Not implemented */
		else if (r_ptr->flags3 & RF3_THUNDERLORD)
			info[i++] = "You have thunderlord blood in your veins.";
		if (r_ptr->flags3 & RF3_EVIL)
			info[i++] = "You are inherently evil.";
		else if (r_ptr->flags3 & RF3_GOOD)
			info[i++] = "You are inherently good.";
		if (r_ptr->flags3 & RF3_AURA_COLD)
			info[i++] = "You are surrounded by a chilly aura.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_NONLIVING)
			info[i++] = "You are not living.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_HURT_LITE)
			info[i++] = "Your eyes are vulnerable to bright light.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_HURT_ROCK)
			info[i++] = "You can be hurt by rock remover.";
		if (r_ptr->flags3 & RF3_SUSCEP_FIRE)
			info[i++] = "You are vulnerable to fire.";
		if (r_ptr->flags3 & RF3_SUSCEP_COLD)
			info[i++] = "You are vulnerable to cold.";
		if (r_ptr->flags3 & RF3_RES_TELE)
			info[i++] = "You are resistant to teleportation.";
		if (r_ptr->flags3 & RF3_RES_NETH)
			info[i++] = "You are resistant to nether.";
		if (r_ptr->flags3 & RF3_RES_WATE)
			info[i++] = "You are resistant to water.";
		if (r_ptr->flags3 & RF3_RES_PLAS)
			info[i++] = "You are resistant to plasma.";
		if (r_ptr->flags3 & RF3_RES_WATE)
			info[i++] = "You are resistant to nexus.";
		if (r_ptr->flags3 & RF3_RES_DISE)
			info[i++] = "You are resistant to disease.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_NO_SLEEP)
			info[i++] = "You cannot be slept.";
		/* Not implemented */
		if (r_ptr->flags3 & RF3_UNIQUE_4)
			info[i++] = "You are a Nazgul.";
		if (r_ptr->flags3 & RF3_NO_FEAR)
			info[i++] = "You are immune to fear.";
		if (r_ptr->flags3 & RF3_NO_STUN)
			info[i++] = "You are immune to stun.";
		if (r_ptr->flags3 & RF3_NO_CONF)
			info[i++] = "You are immune to confusion.";
		if (r_ptr->flags3 & RF3_NO_SLEEP)
			info[i++] = "You are immune to sleep.";

		if (r_ptr->flags4 & RF4_SHRIEK)
			info[i++] = "You can aggravate monsters.";
		if (r_ptr->flags4 & RF4_ROCKET)
			info[i++] = "You can fire a rocket.";
		if (r_ptr->flags4 & RF4_ARROW_1)
			info[i++] = "You can fire a light arrow.";
		if (r_ptr->flags4 & RF4_ARROW_2)
			info[i++] = "You can fire a heavy arrow.";
		if (r_ptr->flags4 & RF4_ARROW_3)
			info[i++] = "You can fire a light missile.";
		if (r_ptr->flags4 & RF4_ARROW_4)
			info[i++] = "You can fire a heavy missile.";
		if (r_ptr->flags4 & RF4_BR_ACID)
			info[i++] = "You can breathe acid.";
		if (r_ptr->flags4 & RF4_BR_ELEC)
			info[i++] = "You can breathe electricity.";
		if (r_ptr->flags4 & RF4_BR_FIRE)
			info[i++] = "You can breathe fire.";
		if (r_ptr->flags4 & RF4_BR_COLD)
			info[i++] = "You can breathe cold.";
		if (r_ptr->flags4 & RF4_BR_POIS)
			info[i++] = "You can breathe poison.";
		if (r_ptr->flags4 & RF4_BR_NETH)
			info[i++] = "You can breathe nether.";
		if (r_ptr->flags4 & RF4_BR_LITE)
			info[i++] = "You can breathe light.";
		if (r_ptr->flags4 & RF4_BR_DARK)
			info[i++] = "You can breathe darkness.";
		if (r_ptr->flags4 & RF4_BR_CONF)
			info[i++] = "You can breathe confusion.";
		if (r_ptr->flags4 & RF4_BR_SOUN)
			info[i++] = "You can breathe sound.";
		if (r_ptr->flags4 & RF4_BR_CHAO)
			info[i++] = "You can breathe chaos.";
		if (r_ptr->flags4 & RF4_BR_DISE)
			info[i++] = "You can breathe disenchantment.";
		if (r_ptr->flags4 & RF4_BR_NEXU)
			info[i++] = "You can breathe nexus.";
		if (r_ptr->flags4 & RF4_BR_TIME)
			info[i++] = "You can breathe time.";
		if (r_ptr->flags4 & RF4_BR_INER)
			info[i++] = "You can breathe inertia.";
		if (r_ptr->flags4 & RF4_BR_GRAV)
			info[i++] = "You can breathe gravity.";
		if (r_ptr->flags4 & RF4_BR_SHAR)
			info[i++] = "You can breathe shards.";
		if (r_ptr->flags4 & RF4_BR_PLAS)
			info[i++] = "You can breathe plasma.";
		if (r_ptr->flags4 & RF4_BR_WALL)
			info[i++] = "You can breathe force.";
		if (r_ptr->flags4 & RF4_BR_MANA)
			info[i++] = "You can breathe mana.";
		if (r_ptr->flags4 & RF4_BR_NUKE)
			info[i++] = "You can breathe nuke.";
		if (r_ptr->flags4 & RF4_BR_DISI)
			info[i++] = "You can breathe disintegration.";
		if (r_ptr->flags5 & RF5_BA_ACID)
			info[i++] = "You can cast a ball of acid.";
		if (r_ptr->flags5 & RF5_BA_ELEC)
			info[i++] = "You can cast a ball of electricity.";
		if (r_ptr->flags5 & RF5_BA_FIRE)
			info[i++] = "You can cast a ball of fire.";
		if (r_ptr->flags5 & RF5_BA_COLD)
			info[i++] = "You can cast a ball of cold.";
		if (r_ptr->flags5 & RF5_BA_POIS)
			info[i++] = "You can cast a ball of poison.";
		if (r_ptr->flags5 & RF5_BA_NETH)
			info[i++] = "You can cast a ball of nether.";
		if (r_ptr->flags5 & RF5_BA_WATE)
			info[i++] = "You can cast a ball of water.";
		/* Not implemented */
		if (r_ptr->flags5 & RF5_DRAIN_MANA)
			info[i++] = "You can drain mana.";
		if (r_ptr->flags5 & RF5_MIND_BLAST)
			info[i++] = "You can cause mind blasting.";
		if (r_ptr->flags5 & RF5_BRAIN_SMASH)
			info[i++] = "You can cause brain smashing.";
		if (r_ptr->flags5 & RF5_CAUSE_1)
			info[i++] = "You can cause light wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_2)
			info[i++] = "You can cause serious wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_3)
			info[i++] = "You can cause critical wounds.";
		if (r_ptr->flags5 & RF5_CAUSE_4)
			info[i++] = "You can cause mortal wounds.";
		if (r_ptr->flags5 & RF5_BO_ACID)
			info[i++] = "You can cast a bolt of acid.";
		if (r_ptr->flags5 & RF5_BO_ELEC)
			info[i++] = "You can cast a bolt of electricity.";
		if (r_ptr->flags5 & RF5_BO_FIRE)
			info[i++] = "You can cast a bolt of fire.";
		if (r_ptr->flags5 & RF5_BO_COLD)
			info[i++] = "You can cast a bolt of cold.";
		if (r_ptr->flags5 & RF5_BO_POIS)
			info[i++] = "You can cast a bolt of poison.";
		if (r_ptr->flags5 & RF5_BO_NETH)
			info[i++] = "You can cast a bolt of nether.";
		if (r_ptr->flags5 & RF5_BO_WATE)
			info[i++] = "You can cast a bolt of water.";
		if (r_ptr->flags5 & RF5_BO_MANA)
			info[i++] = "You can cast a bolt of mana.";
		if (r_ptr->flags5 & RF5_BO_PLAS)
			info[i++] = "You can cast a bolt of plasma.";
		if (r_ptr->flags5 & RF5_BO_ICEE)
			info[i++] = "You can cast a bolt of ice.";
		if (r_ptr->flags5 & RF5_MISSILE)
			info[i++] = "You can cast magic missile.";
		if (r_ptr->flags5 & RF5_SCARE)
			info[i++] = "You can terrify.";
		if (r_ptr->flags5 & RF5_BLIND)
			info[i++] = "You can blind.";
		if (r_ptr->flags5 & RF5_CONF)
			info[i++] = "You can use confusion.";
		if (r_ptr->flags5 & RF5_SLOW)
			info[i++] = "You can cast slow.";
		if (r_ptr->flags5 & RF5_HOLD)
			info[i++] = "You can touch to paralyze.";
		if (r_ptr->flags6 & RF6_HASTE)
			info[i++] = "You can haste yourself.";
		if (r_ptr->flags6 & RF6_HAND_DOOM)
			info[i++] = "You can invoke Hand of Doom.";
		if (r_ptr->flags6 & RF6_HEAL)
			info[i++] = "You can heal yourself.";
		if (r_ptr->flags6 & RF6_BLINK)
			info[i++] = "You can blink.";
		if (r_ptr->flags6 & RF6_TPORT)
			info[i++] = "You can teleport.";
		if (r_ptr->flags6 & RF6_TELE_TO)
			info[i++] = "You can go between places.";
		if (r_ptr->flags6 & RF6_TELE_AWAY)
			info[i++] = "You can teleport away.";
		if (r_ptr->flags6 & RF6_TELE_LEVEL)
			info[i++] = "You can teleport level.";
		if (r_ptr->flags6 & RF6_DARKNESS)
			info[i++] = "You can create darkness.";
		/* Not implemented */
		if (r_ptr->flags6 & RF6_FORGET)
			info[i++] = "You can fade memories.";
		if (r_ptr->flags6 & RF6_RAISE_DEAD)
			info[i++] = "You can Raise the Dead.";
		if (r_ptr->flags6 & RF6_S_BUG)
			info[i++] = "You can magically summon a Software Bugs.";
		if (r_ptr->flags6 & RF6_S_RNG)
			info[i++] = "You can magically summon the RNG.";
		if (r_ptr->flags6 & RF6_S_THUNDERLORD)
			info[i++] = "You can magically summon some Thunderlords.";
		if (r_ptr->flags6 & RF6_S_KIN)
			info[i++] = "You can magically summon some Kins.";
		if (r_ptr->flags6 & RF6_S_HI_DEMON)
			info[i++] = "You can magically summon greater demons.";
		if (r_ptr->flags6 & RF6_S_MONSTER)
			info[i++] = "You can magically summon a monster.";
		if (r_ptr->flags6 & RF6_S_MONSTERS)
			info[i++] = "You can magically summon monsters.";
		if (r_ptr->flags6 & RF6_S_ANT)
			info[i++] = "You can magically summon ants.";
		if (r_ptr->flags6 & RF6_S_SPIDER)
			info[i++] = "You can magically summon spiders.";
		if (r_ptr->flags6 & RF6_S_HOUND)
			info[i++] = "You can magically summon hounds.";
		if (r_ptr->flags6 & RF6_S_HYDRA)
			info[i++] = "You can magically summon hydras.";
		if (r_ptr->flags6 & RF6_S_ANGEL)
			info[i++] = "You can magically summon an angel.";
		if (r_ptr->flags6 & RF6_S_DEMON)
			info[i++] = "You can magically summon a demon.";
		if (r_ptr->flags6 & RF6_S_UNDEAD)
			info[i++] = "You can magically summon an undead.";
		if (r_ptr->flags6 & RF6_S_DRAGON)
			info[i++] = "You can magically summon a dragon.";
		if (r_ptr->flags6 & RF6_S_HI_UNDEAD)
			info[i++] = "You can magically summon greater undead.";
		if (r_ptr->flags6 & RF6_S_HI_DRAGON)
			info[i++] = "You can magically summon greater dragons.";
		if (r_ptr->flags6 & RF6_S_WRAITH)
			info[i++] = "You can magically summon a wraith.";
		/* Not implemented */
		if (r_ptr->flags6 & RF6_S_UNIQUE)
			info[i++] = "You can magically summon an unique monster.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_AQUATIC)
			info[i++] = "You are aquatic.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_CAN_SWIM)
			info[i++] = "You can swim.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_CAN_FLY)
			info[i++] = "You can fly.";
		if ((r_ptr->flags7 & RF7_MORTAL) == 0)
			info[i++] = "You are immortal.";
		/* Not implemented */
		if (r_ptr->flags7 & RF7_NAZGUL)
			info[i++] = "You are a Nazgul.";

		if (r_ptr->flags7 & RF7_SPIDER)
			info[i++] = "You are a spider.";

		if (r_ptr->flags8 & RF8_WILD_TOWN)
			info[i++] = "You appear in towns.";
		if (r_ptr->flags8 & RF8_WILD_SHORE)
			info[i++] = "You appear on the shore.";
		if (r_ptr->flags8 & RF8_WILD_OCEAN)
			info[i++] = "You appear in the ocean.";
		if (r_ptr->flags8 & RF8_WILD_WASTE)
			info[i++] = "You appear in the waste.";
		if (r_ptr->flags8 & RF8_WILD_WOOD)
			info[i++] = "You appear in woods.";
		if (r_ptr->flags8 & RF8_WILD_VOLCANO)
			info[i++] = "You appear in volcanos.";
		if (r_ptr->flags8 & RF8_WILD_MOUNTAIN)
			info[i++] = "You appear in the mountains.";
		if (r_ptr->flags8 & RF8_WILD_GRASS)
			info[i++] = "You appear in grassy areas.";

		if (r_ptr->flags9 & RF9_SUSCEP_ACID)
			info[i++] = "You are vulnerable to acid.";
		if (r_ptr->flags9 & RF9_SUSCEP_ELEC)
			info[i++] = "You are vulnerable to electricity.";
		if (r_ptr->flags9 & RF9_SUSCEP_POIS)
			info[i++] = "You are vulnerable to poison.";
		if (r_ptr->flags9 & RF9_KILL_TREES)
			info[i++] = "You can eat trees.";
		if (r_ptr->flags9 & RF9_WYRM_PROTECT)
			info[i++] = "You are protected by great wyrms of power.";
	}

	/* List powers */
	for (iter = 0; iter < POWER_MAX; iter++)
	{
		if (p_ptr->powers[iter])
		{
			info[i++] = powers_type[iter].desc_text;
		}
	}

	if (p_ptr->allow_one_death)
	{
		info[i++] = "The Blood of Life flows through your veins.";
	}
	if (p_ptr->blind)
	{
		info[i++] = "You cannot see.";
	}
	if (p_ptr->confused)
	{
		info[i++] = "You are confused.";
	}
	if (p_ptr->afraid)
	{
		info[i++] = "You are terrified.";
	}
	if (p_ptr->cut)
	{
		info[i++] = "You are bleeding.";
	}
	if (p_ptr->stun)
	{
		info[i++] = "You are stunned.";
	}
	if (p_ptr->poisoned)
	{
		info[i++] = "You are poisoned.";
	}
	if (p_ptr->image)
	{
		info[i++] = "You are hallucinating.";
	}
	if (p_ptr->aggravate)
	{
		info[i++] = "You aggravate monsters.";
	}
	if (p_ptr->teleport)
	{
		info[i++] = "Your position is very uncertain.";
	}
	if (p_ptr->blessed)
	{
		info[i++] = "You feel righteous.";
	}
	if (p_ptr->hero)
	{
		info[i++] = "You feel heroic.";
	}
	if (p_ptr->shero)
	{
		info[i++] = "You are in a battle rage.";
	}
	if (p_ptr->protevil)
	{
		info[i++] = "You are protected from evil.";
	}
	if (p_ptr->protgood)
	{
		info[i++] = "You are protected from good.";
	}
	if (p_ptr->shield)
	{
		info[i++] = "You are protected by a mystic shield.";
	}
	if (p_ptr->invuln)
	{
		info[i++] = "You are temporarily invulnerable.";
	}
	if (p_ptr->confusing)
	{
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->searching)
	{
		info[i++] = "You are looking around very carefully.";
	}
	if (p_ptr->word_recall)
	{
		info[i++] = "You will soon be recalled.";
	}
	if (p_ptr->see_infra)
	{
		info[i++] = "Your eyes are sensitive to infrared light.";
	}
	if (p_ptr->see_inv)
	{
		info[i++] = "You can see invisible creatures.";
	}
	if (p_ptr->magical_breath)
	{
		info[i++] = "You can breathe without air.";
	}
	else if (p_ptr->water_breath)
	{
		info[i++] = "You can breathe underwater.";
	}
	if (p_ptr->ffall)
	{
		info[i++] = "You levitate just over the ground.";
	}
	if (p_ptr->climb)
	{
		info[i++] = "You can climb high mountains.";
	}
	if (p_ptr->free_act)
	{
		info[i++] = "You have free action.";
	}
	if (p_ptr->regenerate)
	{
		info[i++] = "You regenerate quickly.";
	}
	if (p_ptr->slow_digest)
	{
		info[i++] = "Your appetite is small.";
	}
	if (p_ptr->telepathy)
	{
		if (p_ptr->telepathy & ESP_ALL) info[i++] = "You have ESP.";
		else
		{
			if (p_ptr->telepathy & ESP_ORC) info[i++] = "You can sense the presence of orcs.";
			if (p_ptr->telepathy & ESP_TROLL) info[i++] = "You can sense the presence of trolls.";
			if (p_ptr->telepathy & ESP_DRAGON) info[i++] = "You can sense the presence of dragons.";
			if (p_ptr->telepathy & ESP_SPIDER) info[i++] = "You can sense the presence of spiders.";
			if (p_ptr->telepathy & ESP_GIANT) info[i++] = "You can sense the presence of giants.";
			if (p_ptr->telepathy & ESP_DEMON) info[i++] = "You can sense the presence of demons.";
			if (p_ptr->telepathy & ESP_UNDEAD) info[i++] = "You can sense presence of undead.";
			if (p_ptr->telepathy & ESP_EVIL) info[i++] = "You can sense the presence of evil beings.";
			if (p_ptr->telepathy & ESP_ANIMAL) info[i++] = "You can sense the presence of animals.";
			if (p_ptr->telepathy & ESP_THUNDERLORD) info[i++] = "You can sense the presence of thunderlords.";
			if (p_ptr->telepathy & ESP_GOOD) info[i++] = "You can sense the presence of good beings.";
			if (p_ptr->telepathy & ESP_NONLIVING) info[i++] = "You can sense the presence of non-living things.";
			if (p_ptr->telepathy & ESP_UNIQUE) info[i++] = "You can sense the presence of unique beings.";
		}
	}
	if (!luck( -100, 100))
	{
		info[i++] = "You have normal luck.";
	}
	else if (luck( -100, 100) < 0)
	{
		if (luck( -100, 100) < -90)
		{
			info[i++] = "You are incredibly unlucky.";
		}
		else if (luck( -100, 100) < -60)
		{
			info[i++] = "You are extremely unlucky.";
		}
		else if (luck( -100, 100) < -30)
		{
			info[i++] = "You are very unlucky.";
		}
		else
		{
			info[i++] = "You are unlucky.";
		}
	}
	else if (luck( -100, 100) > 0)
	{
		if (luck( -100, 100) > 90)
		{
			info[i++] = "You are incredibly lucky.";
		}
		else if (luck( -100, 100) > 60)
		{
			info[i++] = "You are extremely lucky.";
		}
		else if (luck( -100, 100) > 30)
		{
			info[i++] = "You are very lucky.";
		}
		else
		{
			info[i++] = "You are lucky.";
		}
	}
	if (p_ptr->auto_id)
	{
		info[i++] = "You know everything.";
	}
	if (p_ptr->hold_life)
	{
		info[i++] = "You have a firm hold on your life force.";
	}
	if (p_ptr->reflect)
	{
		info[i++] = "You reflect arrows and bolts.";
	}
	if (p_ptr->sh_fire)
	{
		info[i++] = "You are surrounded with a fiery aura.";
	}
	if (p_ptr->sh_elec)
	{
		info[i++] = "You are surrounded with electricity.";
	}
	if (p_ptr->antimagic)
	{
		info[i++] = "You are surrounded by an anti-magic field.";
	}
	if (p_ptr->anti_magic)
	{
		info[i++] = "You are surrounded by an anti-magic shell.";
	}
	if (p_ptr->wraith_form)
	{
		info[i++] = "You are incorporeal.";
	}
	if (p_ptr->anti_tele)
	{
		info[i++] = "You cannot teleport.";
	}
	if (p_ptr->lite)
	{
		info[i++] = "You are carrying a permanent light.";
	}

	if (p_ptr->immune_acid)
	{
		info[i++] = "You are completely immune to acid.";
	}
	else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid))
	{
		info[i++] = "You resist acid exceptionally well.";
	}
	else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid))
	{
		info[i++] = "You are resistant to acid.";
	}

	if (p_ptr->immune_elec)
	{
		info[i++] = "You are completely immune to lightning.";
	}
	else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec))
	{
		info[i++] = "You resist lightning exceptionally well.";
	}
	else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec))
	{
		info[i++] = "You are resistant to lightning.";
	}

	if (p_ptr->immune_fire)
	{
		info[i++] = "You are completely immune to fire.";
	}
	else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire))
	{
		info[i++] = "You resist fire exceptionally well.";
	}
	else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire))
	{
		info[i++] = "You are resistant to fire.";
	}
	else if (p_ptr->sensible_fire)
	{
		info[i++] = "You are very vulnerable to fire.";
	}

	if (p_ptr->immune_cold)
	{
		info[i++] = "You are completely immune to cold.";
	}
	else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold))
	{
		info[i++] = "You resist cold exceptionally well.";
	}
	else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold))
	{
		info[i++] = "You are resistant to cold.";
	}

	if ((p_ptr->resist_pois) && (p_ptr->oppose_pois))
	{
		info[i++] = "You resist poison exceptionally well.";
	}
	else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois))
	{
		info[i++] = "You are resistant to poison.";
	}

	if (p_ptr->resist_lite)
	{
		info[i++] = "You are resistant to bright light.";
	}
	if (p_ptr->resist_dark)
	{
		info[i++] = "You are resistant to darkness.";
	}
	if (p_ptr->resist_conf)
	{
		info[i++] = "You are resistant to confusion.";
	}
	if (p_ptr->resist_sound)
	{
		info[i++] = "You are resistant to sonic attacks.";
	}
	if (p_ptr->resist_disen)
	{
		info[i++] = "You are resistant to disenchantment.";
	}
	if (p_ptr->resist_chaos)
	{
		info[i++] = "You are resistant to chaos.";
	}
	if (p_ptr->resist_shard)
	{
		info[i++] = "You are resistant to blasts of shards.";
	}
	if (p_ptr->resist_nexus)
	{
		info[i++] = "You are resistant to nexus attacks.";
	}
	if (p_ptr->immune_neth)
	{
		info[i++] = "You are immune to nether forces.";
	}
	else if (p_ptr->resist_neth)
	{
		info[i++] = "You are resistant to nether forces.";
	}
	if (p_ptr->resist_fear)
	{
		info[i++] = "You are completely fearless.";
	}
	if (p_ptr->resist_blind)
	{
		info[i++] = "Your eyes are resistant to blindness.";
	}
	if (p_ptr->resist_continuum)
	{
		info[i++] = "The space-time continuum cannot be disrupted near you.";
	}

	if (p_ptr->sustain_str)
	{
		info[i++] = "Your strength is sustained.";
	}
	if (p_ptr->sustain_int)
	{
		info[i++] = "Your intelligence is sustained.";
	}
	if (p_ptr->sustain_wis)
	{
		info[i++] = "Your wisdom is sustained.";
	}
	if (p_ptr->sustain_con)
	{
		info[i++] = "Your constitution is sustained.";
	}
	if (p_ptr->sustain_dex)
	{
		info[i++] = "Your dexterity is sustained.";
	}
	if (p_ptr->sustain_chr)
	{
		info[i++] = "Your charisma is sustained.";
	}
	if (p_ptr->black_breath)
	{
		info[i++] = "You suffer from Black Breath.";
	}

	if (f1 & (TR1_STR))
	{
		info[i++] = "Your strength is affected by your equipment.";
	}
	if (f1 & (TR1_INT))
	{
		info[i++] = "Your intelligence is affected by your equipment.";
	}
	if (f1 & (TR1_WIS))
	{
		info[i++] = "Your wisdom is affected by your equipment.";
	}
	if (f1 & (TR1_DEX))
	{
		info[i++] = "Your dexterity is affected by your equipment.";
	}
	if (f1 & (TR1_CON))
	{
		info[i++] = "Your constitution is affected by your equipment.";
	}
	if (f1 & (TR1_CHR))
	{
		info[i++] = "Your charisma is affected by your equipment.";
	}

	if (f1 & (TR1_STEALTH))
	{
		info[i++] = "Your stealth is affected by your equipment.";
	}
	if (f1 & (TR1_SEARCH))
	{
		info[i++] = "Your searching ability is affected by your equipment.";
	}
	if (f1 & (TR1_INFRA))
	{
		info[i++] = "Your infravision is affected by your equipment.";
	}
	if (f1 & (TR1_TUNNEL))
	{
		info[i++] = "Your digging ability is affected by your equipment.";
	}
	if (f1 & (TR1_SPEED))
	{
		info[i++] = "Your speed is affected by your equipment.";
	}
	if (f1 & (TR1_BLOWS))
	{
		info[i++] = "Your attack speed is affected by your equipment.";
	}
	if (f5 & (TR5_CRIT))
	{
		info[i++] = "Your ability to score critical hits is affected by your equipment.";
	}


	/* Access the current weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Analyze the weapon */
	if (o_ptr->k_idx)
	{
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Indicate Blessing */
		if (f3 & (TR3_BLESSED))
		{
			info[i++] = "Your weapon has been blessed by the gods.";
		}

		if (f1 & (TR1_CHAOTIC))
		{
			info[i++] = "Your weapon is branded with the Sign of Chaos.";
		}

		/* Hack */
		if (f1 & (TR1_IMPACT))
		{
			info[i++] = "The impact of your weapon can cause earthquakes.";
		}

		if (f1 & (TR1_VORPAL))
		{
			info[i++] = "Your weapon is very sharp.";
		}

		if (f1 & (TR1_VAMPIRIC))
		{
			info[i++] = "Your weapon drains life from your foes.";
		}

		/* Special "Attack Bonuses" */
		if (f1 & (TR1_BRAND_ACID))
		{
			info[i++] = "Your weapon melts your foes.";
		}
		if (f1 & (TR1_BRAND_ELEC))
		{
			info[i++] = "Your weapon shocks your foes.";
		}
		if (f1 & (TR1_BRAND_FIRE))
		{
			info[i++] = "Your weapon burns your foes.";
		}
		if (f1 & (TR1_BRAND_COLD))
		{
			info[i++] = "Your weapon freezes your foes.";
		}
		if (f1 & (TR1_BRAND_POIS))
		{
			info[i++] = "Your weapon poisons your foes.";
		}

		/* Special "slay" flags */
		if (f1 & (TR1_SLAY_ANIMAL))
		{
			info[i++] = "Your weapon strikes at animals with extra force.";
		}
		if (f1 & (TR1_SLAY_EVIL))
		{
			info[i++] = "Your weapon strikes at evil with extra force.";
		}
		if (f1 & (TR1_SLAY_UNDEAD))
		{
			info[i++] = "Your weapon strikes at undead with holy wrath.";
		}
		if (f1 & (TR1_SLAY_DEMON))
		{
			info[i++] = "Your weapon strikes at demons with holy wrath.";
		}
		if (f1 & (TR1_SLAY_ORC))
		{
			info[i++] = "Your weapon is especially deadly against orcs.";
		}
		if (f1 & (TR1_SLAY_TROLL))
		{
			info[i++] = "Your weapon is especially deadly against trolls.";
		}
		if (f1 & (TR1_SLAY_GIANT))
		{
			info[i++] = "Your weapon is especially deadly against giants.";
		}
		if (f1 & (TR1_SLAY_DRAGON))
		{
			info[i++] = "Your weapon is especially deadly against dragons.";
		}

		/* Special "kill" flags */
		if (f1 & (TR1_KILL_DRAGON))
		{
			info[i++] = "Your weapon is a great bane of dragons.";
		}
		/* Special "kill" flags */
		if (f5 & (TR5_KILL_DEMON))
		{
			info[i++] = "Your weapon is a great bane of demons.";
		}
		/* Special "kill" flags */
		if (f5 & (TR5_KILL_UNDEAD))
		{
			info[i++] = "Your weapon is a great bane of undeads.";
		}
	}

	/* Print on screen or in a file ? */
	if (fff == NULL)
	{
		/* Save the screen */
		character_icky = TRUE;
		Term_save();

		/* Erase the screen */
		for (k = 1; k < 24; k++) prt("", k, 13);

		/* Label the information */
		prt("     Your Attributes:", 1, 15);

		/* We will print on top of the map (column 13) */
		for (k = 2, j = 0; j < i; j++)
		{
			/* Show the info */
			prt(info[j], k++, 15);

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
		Term_load();
		character_icky = FALSE;
	}
	else
	{
		/* Label the information */
		fprintf(fff, "     Your Attributes:\n");

		/* We will print on top of the map (column 13) */
		for (j = 0; j < i; j ++)
		{
			/* Show the info */
			fprintf(fff, "%s\n", info[j]);
		}
	}
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

static cptr report_magic_durations[] =
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


void report_magics(void)
{
	int i = 0, j, k;

	char Dummy[80];

	cptr info[128];
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
	if (p_ptr->protgood)
	{
		info2[i] = report_magics_aux(p_ptr->protgood);
		info[i++] = "You are protected from good";
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
	character_icky = TRUE;
	Term_save();

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
	Term_load();
	character_icky = FALSE;
}



/*
 * Forget everything
 */
bool_ lose_all_info(void)
{
	int i;

	/* Forget info about objects */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Allow "protection" by the MENTAL flag */
		if (o_ptr->ident & (IDENT_MENTAL)) continue;

		/* Remove sensing */
		o_ptr->sense = SENSE_NONE;

		/* Hack -- Clear the "empty" flag */
		o_ptr->ident &= ~(IDENT_EMPTY);

		/* Hack -- Clear the "known" flag */
		o_ptr->ident &= ~(IDENT_KNOWN);

		/* Hack -- Clear the "felt" flag */
		o_ptr->ident &= ~(IDENT_SENSE);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Mega-Hack -- Forget the map */
	wiz_dark();

	/* It worked */
	return (TRUE);
}


/*
 * Detect all doors on current panel
 */
bool_ detect_doors(int rad)
{
	int y, x;

	bool_ detect = FALSE;

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
				detect = TRUE;
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
bool_ detect_stairs(int rad)
{
	int y, x;

	bool_ detect = FALSE;

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
				detect = TRUE;
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
bool_ detect_treasure(int rad)
{
	int y, x;

	bool_ detect = FALSE;

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
				detect = TRUE;
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
template<typename P, typename U> static bool detect_monsters_fn(int radius, P p, U u) {
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
			/* Update */
			u(r_ptr.get());

			/* We're assuming the update function does
			 * *something*, so we'll need to update
			 * the recall window if we're currently looking
			 * at it.
			 */
			if (monster_race_idx == m_ptr->r_idx)
			{
				p_ptr->window |= (PW_MONSTER);
			}

			/* Repair visibility later */
			repair_monsters = TRUE;

			/* Hack -- Detect monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Hack -- See monster */
			m_ptr->ml = TRUE;

			/* Redraw */
			if (panel_contains(y, x)) lite_spot(y, x);

			/* Detect */
			flag = TRUE;
		}
	}
	/* Result */
	return flag;
}

/*
 * Detect all (string) monsters on current panel
 */
static bool_ detect_monsters_string(cptr chars, int rad)
{
	auto predicate = [chars](monster_race *r_ptr) -> bool {
		return strchr(chars, r_ptr->d_char);
	};
	auto update = [](monster_race *) -> void {
	};

	/* Describe */
	if (detect_monsters_fn(rad, predicate, update))
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
		return TRUE;
	}
	else
	{
		return FALSE;
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
		if (!o_ptr->k_idx) continue;

		/* Location */
		int y, x;

		/* Skip held objects */
		if (o_ptr->held_m_idx)
		{
			/* Access the monster */
			monster_type *m_ptr = &m_list[o_ptr->held_m_idx];
			auto const r_ptr = m_ptr->race();

			if (!(r_ptr->flags9 & RF9_MIMIC))
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
			o_ptr->marked = TRUE;

			/* Redraw */
			if (panel_contains(y, x)) lite_spot(y, x);

			/* Detect */
			detect = TRUE;
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
bool_ detect_monsters_normal(int rad)
{
	auto predicate = [](monster_race *r_ptr) -> bool {
		return (!(r_ptr->flags2 & (RF2_INVISIBLE))) ||
			p_ptr->see_inv || p_ptr->tim_invis;
	};
	auto update = [](monster_race *) -> void {
	};

	if (detect_monsters_fn(rad, predicate, update))
	{
		/* Describe result */
		msg_print("You sense the presence of monsters!");
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*
 * Detect all "invisible" monsters on current panel
 */
bool_ detect_monsters_invis(int rad)
{
	auto predicate = [](monster_race *r_ptr) -> bool {
		return (r_ptr->flags2 & (RF2_INVISIBLE));
	};
	auto update = [](monster_race *r_ptr) -> void {
		r_ptr->r_flags2 |= (RF2_INVISIBLE);
	};

	if (detect_monsters_fn(rad, predicate, update))
	{
		/* Describe result */
		msg_print("You sense the presence of invisible creatures!");
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



/*
 * A "generic" detect monsters routine, tagged to flags3
 */
bool_ detect_monsters_xxx(u32b match_flag, int rad)
{
	auto predicate = [match_flag](monster_race *r_ptr) -> bool {
		return (r_ptr->flags3 & match_flag);
	};
	auto update = [match_flag](monster_race *r_ptr) -> void {
		r_ptr->r_flags3 |= (match_flag);
	};

	if (detect_monsters_fn(rad, predicate, update))
	{
		cptr desc_monsters = "weird monsters";
		switch (match_flag)
		{
		case RF3_DEMON:
			desc_monsters = "demons";
			break;
		case RF3_UNDEAD:
			desc_monsters = "the undead";
			break;
		case RF3_GOOD:
			desc_monsters = "good";
			break;
		case RF3_ORC:
			desc_monsters = "orcs";
			break;
		}

		/* Describe result */
		msg_format("You sense the presence of %s!", desc_monsters);
		msg_print(NULL);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



/*
 * Detect everything
 */
bool_ detect_all(int rad)
{
	bool_ detect = FALSE;

	/* Detect everything */
	if (detect_doors(rad)) detect = TRUE;
	if (detect_stairs(rad)) detect = TRUE;
	if (detect_treasure(rad)) detect = TRUE;
	if (detect_objects_gold(rad)) detect = TRUE;
	if (detect_objects_normal(rad)) detect = TRUE;
	if (detect_monsters_invis(rad)) detect = TRUE;
	if (detect_monsters_normal(rad)) detect = TRUE;

	/* Result */
	return (detect);
}



/*
 * Create stairs at the player location
 */
void stair_creation(void)
{
	/* XXX XXX XXX */
	if (!cave_valid_bold(p_ptr->py, p_ptr->px))
	{
		msg_print("The object resists the spell.");
		return;
	}

	if (dungeon_flags1 & DF1_FLAT)
	{
		msg_print("No stair creation in non dungeons...");
		return;
	}

	if (dungeon_flags2 & DF2_SPECIAL)
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
bool_ enchant(object_type *o_ptr, int n, int eflag)
{
	int i, chance, prob;
	bool_ res = FALSE;
	bool_ a = (artifact_p(o_ptr) || o_ptr->art_name);
	u32b f1, f2, f3, f4, f5, esp;


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

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
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				                (!(f3 & (TR3_PERMA_CURSE))) &&
				                (o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
						o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
						o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

					o_ptr->sense = SENSE_UNCURSED;
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
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				                (!(f3 & (TR3_PERMA_CURSE))) &&
				                (o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
						o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
						o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

					o_ptr->sense = SENSE_UNCURSED;
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
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				                (!(f3 & (TR3_PERMA_CURSE))) &&
				                (o_ptr->pval >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
						o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
						o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

					o_ptr->sense = SENSE_UNCURSED;
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
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				                (!(f3 & (TR3_PERMA_CURSE))) &&
				                (o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					msg_print("The curse is broken!");
					o_ptr->ident &= ~(IDENT_CURSED);
					o_ptr->ident |= (IDENT_SENSE);

					if (o_ptr->art_flags3 & (TR3_CURSED))
						o_ptr->art_flags3 &= ~(TR3_CURSED);
					if (o_ptr->art_flags3 & (TR3_HEAVY_CURSE))
						o_ptr->art_flags3 &= ~(TR3_HEAVY_CURSE);

					o_ptr->sense = SENSE_UNCURSED;
				}
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Success */
	return (TRUE);
}



/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool_ enchant_spell(int num_hit, int num_dam, int num_ac, int num_pval)
{
	int item;
	bool_ okay = FALSE;
	char o_name[80];
	cptr q, s;


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
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR), object_filter)) return (FALSE);

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Description */
	object_desc(o_name, o_ptr, FALSE, 0);

	/* Describe */
	msg_format("%s %s glow%s brightly!",
	           ((item >= 0) ? "Your" : "The"), o_name,
	           ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;
	if (enchant(o_ptr, num_pval, ENCH_PVAL)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		if (flush_failure) flush();

		/* Message */
		msg_print("The enchantment failed.");
	}

	/* Something happened */
	return (TRUE);
}

void curse_artifact(object_type * o_ptr)
{
	if (o_ptr->pval) o_ptr->pval = 0 - ((o_ptr->pval) + randint(4));
	if (o_ptr->to_a) o_ptr->to_a = 0 - ((o_ptr->to_a) + randint(4));
	if (o_ptr->to_h) o_ptr->to_h = 0 - ((o_ptr->to_h) + randint(4));
	if (o_ptr->to_d) o_ptr->to_d = 0 - ((o_ptr->to_d) + randint(4));
	o_ptr->art_flags3 |= ( TR3_HEAVY_CURSE | TR3_CURSED );
	if (randint(3) == 1) o_ptr-> art_flags3 |= TR3_TY_CURSE;
	if (randint(2) == 1) o_ptr-> art_flags3 |= TR3_AGGRAVATE;
	if (randint(3) == 1) o_ptr-> art_flags3 |= TR3_DRAIN_EXP;
	if (randint(3) == 1) o_ptr-> art_flags4 |= TR4_BLACK_BREATH;
	if (randint(2) == 1) o_ptr-> art_flags3 |= TR3_TELEPORT;
	else if (randint(3) == 1) o_ptr->art_flags3 |= TR3_NO_TELE;
	o_ptr->ident |= IDENT_CURSED;
}



void random_resistance (object_type * o_ptr, bool_ is_scroll, int specific)
{
	/* To avoid a number of possible bugs */
	if (!specific)
	{
		if (artifact_bias == BIAS_ACID)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_ACID))
			{
				o_ptr->art_flags2 |= TR2_RES_ACID;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags2 & TR2_IM_ACID))
			{
				o_ptr->art_flags2 |= TR2_IM_ACID;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_ELEC)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_ELEC))
			{
				o_ptr->art_flags2 |= TR2_RES_ELEC;
				if (rand_int(2) == 0) return;
			}
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
			                !(o_ptr->art_flags3 & TR3_SH_ELEC))
			{
				o_ptr->art_flags2 |= TR3_SH_ELEC;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags2 & TR2_IM_ELEC))
			{
				o_ptr->art_flags2 |= TR2_IM_ELEC;
				if (rand_int(2) == 1) return;
			}
		}
		else if (artifact_bias == BIAS_FIRE)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_FIRE))
			{
				o_ptr->art_flags2 |= TR2_RES_FIRE;
				if (rand_int(2) == 0) return;
			}
			if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR &&
			                !(o_ptr->art_flags3 & TR3_SH_FIRE))
			{
				o_ptr->art_flags2 |= TR3_SH_FIRE;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags2 & TR2_IM_FIRE))
			{
				o_ptr->art_flags2 |= TR2_IM_FIRE;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_COLD)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_COLD))
			{
				o_ptr->art_flags2 |= TR2_RES_COLD;
				if (rand_int(2) == 0) return;
			}
			if (rand_int(BIAS_LUCK) == 0 && !(o_ptr->art_flags2 & TR2_IM_COLD))
			{
				o_ptr->art_flags2 |= TR2_IM_COLD;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_POIS)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_POIS))
			{
				o_ptr->art_flags2 |= TR2_RES_POIS;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_WARRIOR)
		{
			if (rand_int(3) && (!(o_ptr->art_flags2 & TR2_RES_FEAR)))
			{
				o_ptr->art_flags2 |= TR2_RES_FEAR;
				if (rand_int(2) == 0) return;
			}
			if ((rand_int(3) == 0) && (!(o_ptr->art_flags3 & TR3_NO_MAGIC)))
			{
				o_ptr->art_flags3 |= TR3_NO_MAGIC;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_NECROMANTIC)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_NETHER))
			{
				o_ptr->art_flags2 |= TR2_RES_NETHER;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_POIS))
			{
				o_ptr->art_flags2 |= TR2_RES_POIS;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_DARK))
			{
				o_ptr->art_flags2 |= TR2_RES_DARK;
				if (rand_int(2) == 0) return;
			}
		}
		else if (artifact_bias == BIAS_CHAOS)
		{
			if (!(o_ptr->art_flags2 & TR2_RES_CHAOS))
			{
				o_ptr->art_flags2 |= TR2_RES_CHAOS;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_CONF))
			{
				o_ptr->art_flags2 |= TR2_RES_CONF;
				if (rand_int(2) == 0) return;
			}
			if (!(o_ptr->art_flags2 & TR2_RES_DISEN))
			{
				o_ptr->art_flags2 |= TR2_RES_DISEN;
				if (rand_int(2) == 0) return;
			}
		}
	}

	switch (specific ? specific : randint(41))
	{
	case 1 :
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, is_scroll, specific);
		else
		{
			o_ptr->art_flags2 |= TR2_IM_ACID;
			/*  if (is_scroll) msg_print("It looks totally incorruptible."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_ACID;
		}
		break;
	case 2:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, is_scroll, specific);
		else
		{
			o_ptr->art_flags2 |= TR2_IM_ELEC;
			/*  if (is_scroll) msg_print("It looks completely grounded."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_ELEC;
		}
		break;
	case 3:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, is_scroll, specific);
		else
		{
			o_ptr->art_flags2 |= TR2_IM_COLD;
			/*  if (is_scroll) msg_print("It feels very warm."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_COLD;
		}
		break;
	case 4:
		if (randint(WEIRD_LUCK) != 1)
			random_resistance(o_ptr, is_scroll, specific);
		else
		{
			o_ptr->art_flags2 |= TR2_IM_FIRE;
			/*  if (is_scroll) msg_print("It feels very cool."); */
			if (!(artifact_bias))
				artifact_bias = BIAS_FIRE;
		}
		break;
	case 5:
	case 6:
	case 13:
		o_ptr->art_flags2 |= TR2_RES_ACID;
		/*  if (is_scroll) msg_print("It makes your stomach rumble."); */
		if (!(artifact_bias))
			artifact_bias = BIAS_ACID;
		break;
	case 7:
	case 8:
	case 14:
		o_ptr->art_flags2 |= TR2_RES_ELEC;
		/*  if (is_scroll) msg_print("It makes you feel grounded."); */
		if (!(artifact_bias))
			artifact_bias = BIAS_ELEC;
		break;
	case 9:
	case 10:
	case 15:
		o_ptr->art_flags2 |= TR2_RES_FIRE;
		/*  if (is_scroll) msg_print("It makes you feel cool!");*/
		if (!(artifact_bias))
			artifact_bias = BIAS_FIRE;
		break;
	case 11:
	case 12:
	case 16:
		o_ptr->art_flags2 |= TR2_RES_COLD;
		/*  if (is_scroll) msg_print("It makes you feel full of hot air!");*/
		if (!(artifact_bias))
			artifact_bias = BIAS_COLD;
		break;
	case 17:
	case 18:
		o_ptr->art_flags2 |= TR2_RES_POIS;
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
		o_ptr->art_flags2 |= TR2_RES_FEAR;
		/*  if (is_scroll) msg_print("It makes you feel brave!"); */
		if (!(artifact_bias) && randint(3) == 1)
			artifact_bias = BIAS_WARRIOR;
		break;
	case 21:
		o_ptr->art_flags2 |= TR2_RES_LITE;
		/*  if (is_scroll) msg_print("It makes everything look darker.");*/
		break;
	case 22:
		o_ptr->art_flags2 |= TR2_RES_DARK;
		/*  if (is_scroll) msg_print("It makes everything look brigher.");*/
		break;
	case 23:
	case 24:
		o_ptr->art_flags2 |= TR2_RES_BLIND;
		/*  if (is_scroll) msg_print("It makes you feel you are wearing glasses.");*/
		break;
	case 25:
	case 26:
		o_ptr->art_flags2 |= TR2_RES_CONF;
		/*  if (is_scroll) msg_print("It makes you feel very determined.");*/
		if (!(artifact_bias) && randint(6) == 1)
			artifact_bias = BIAS_CHAOS;
		break;
	case 27:
	case 28:
		o_ptr->art_flags2 |= TR2_RES_SOUND;
		/*  if (is_scroll) msg_print("It makes you feel deaf!");*/
		break;
	case 29:
	case 30:
		o_ptr->art_flags2 |= TR2_RES_SHARDS;
		/*  if (is_scroll) msg_print("It makes your skin feel thicker.");*/
		break;
	case 31:
	case 32:
		o_ptr->art_flags2 |= TR2_RES_NETHER;
		/*  if (is_scroll) msg_print("It makes you feel like visiting a graveyard!");*/
		if (!(artifact_bias) && randint(3) == 1)
			artifact_bias = BIAS_NECROMANTIC;
		break;
	case 33:
	case 34:
		o_ptr->art_flags2 |= TR2_RES_NEXUS;
		/*  if (is_scroll) msg_print("It makes you feel normal.");*/
		break;
	case 35:
	case 36:
		o_ptr->art_flags2 |= TR2_RES_CHAOS;
		/*  if (is_scroll) msg_print("It makes you feel very firm.");*/
		if (!(artifact_bias) && randint(2) == 1)
			artifact_bias = BIAS_CHAOS;
		break;
	case 37:
	case 38:
		o_ptr->art_flags2 |= TR2_RES_DISEN;
		/*  if (is_scroll) msg_print("It is surrounded by a static feeling.");*/
		break;
	case 39:
		if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
			o_ptr->art_flags3 |= TR3_SH_ELEC;
		else
			random_resistance(o_ptr, is_scroll, specific);
		if (!(artifact_bias))
			artifact_bias = BIAS_ELEC;
		break;
	case 40:
		if (o_ptr->tval >= TV_CLOAK && o_ptr->tval <= TV_HARD_ARMOR)
			o_ptr->art_flags3 |= TR3_SH_FIRE;
		else
			random_resistance(o_ptr, is_scroll, specific);
		if (!(artifact_bias))
			artifact_bias = BIAS_FIRE;
		break;
	case 41:
		if (o_ptr->tval == TV_SHIELD || o_ptr->tval == TV_CLOAK ||
		                o_ptr->tval == TV_HELM || o_ptr->tval == TV_HARD_ARMOR)
			o_ptr->art_flags2 |= TR2_REFLECT;
		else
			random_resistance(o_ptr, is_scroll, specific);
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

	if (artifact_p(o_ptr) || o_ptr->name1)
	{
		object_desc(item_name, o_ptr, FALSE, 0);

		/* Build note and write */
		sprintf(note, "Found The %s", item_name);
		add_note(note, 'A');
	}
}




/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool_ ident_spell(void)
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Identify which item? ",
		      "You have nothing to identify.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR),
		      object_filter::Not(object_known_p))) return (FALSE);

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
	object_desc(o_name, o_ptr, TRUE, 3);

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

	/* Process the appropriate hooks */
	identify_hooks(item, o_ptr, IDENT_NORMAL);

	/* Something happened */
	return (TRUE);
}

/*
 * Identify all objects in the level
 */
bool_ ident_all(void)
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

		/* Process the appropriate hooks */
		identify_hooks(-i, o_ptr, IDENT_NORMAL);
	}

	/* Something happened */
	return (TRUE);
}



/*
 * Determine if an object is not fully identified
 */
static bool item_tester_hook_no_mental(object_type const *o_ptr)
{
	return ((o_ptr->ident & (IDENT_MENTAL)) ? false : true);
}

/*
 * Fully "identify" an object in the inventory  -BEN-
 * This routine returns TRUE if an item was identified.
 */
bool_ identify_fully(void)
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Identify which item? ",
		      "You have nothing to identify.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR),
		      item_tester_hook_no_mental))
	{
		return (FALSE);
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Do the identification */
	make_item_fully_identified(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Description */
	char o_name[80];
	object_desc(o_name, o_ptr, TRUE, 3);

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

	/* Describe it fully */
	object_out_desc(o_ptr, NULL, FALSE, TRUE);

	/* Process the appropriate hooks */
	identify_hooks(item, o_ptr, IDENT_FULL);

	/* Success */
	return (TRUE);
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
			Not(HasFlag4(TR4_NO_RECHARGE)),
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
bool_ recharge(int power)
{
	int recharge_strength, recharge_amount;
	int lev;
	bool_ fail = FALSE;
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
		return (FALSE);
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Extract the flags */
	u32b f1, f2, f3, f4, f5, esp;
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;

	/* Recharge a rod */
	if (o_ptr->tval == TV_ROD_MAIN)
	{
		/* Extract a recharge strength by comparing object level to power. */
		recharge_strength = ((power > lev) ? (power - lev) : 0) / 5;

		/* Paranoia */
		if (recharge_strength < 0) recharge_strength = 0;

		/* Back-fire */
		if ((rand_int(recharge_strength) == 0) && (!(f4 & TR4_RECHARGE)))
		{
			/* Activate the failure code. */
			fail = TRUE;
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
		if (((rand_int(recharge_strength) == 0) && (!(f4 & TR4_RECHARGE))) ||
		                (f4 & TR4_NO_RECHARGE))
		{
			/* Activate the failure code. */
			fail = TRUE;
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

			if (!(f4 & TR4_RECHARGE))
			{
				/* Hack -- we no longer "know" the item */
				o_ptr->ident &= ~(IDENT_KNOWN);
			}

			/* Hack -- we no longer think the item is empty */
			o_ptr->ident &= ~(IDENT_EMPTY);
		}
	}

	/* Mark as recharged */
	o_ptr->art_flags4 |= TR4_RECHARGED;

	/* Inflict the penalties for failing a recharge. */
	if (fail)
	{
		/* Artifacts are never destroyed. */
		if (artifact_p(o_ptr))
		{
			object_desc(o_name, o_ptr, TRUE, 0);
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
			object_desc(o_name, o_ptr, FALSE, 0);

			/*** Determine Seriousness of Failure ***/

			/* Mages recharge objects more safely. */
			if (has_ability(AB_PERFECT_CASTING))
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
	return (TRUE);
}



/*
 * Apply a "project()" directly to all viewable monsters
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
bool_ project_hack(int typ, int dam)
{
	int i, x, y;
	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;
	bool_ obvious = FALSE;


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
		if (project(0, 0, y, x, dam, typ, flg)) obvious = TRUE;
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
bool_ banish_evil(int dist)
{
	return (project_hack(GF_AWAY_EVIL, dist));
}



/*
 * Dispel undead monsters
 */
bool_ dispel_undead(int dam)
{
	return (project_hack(GF_DISP_UNDEAD, dam));
}

/*
 * Dispel evil monsters
 */
bool_ dispel_evil(int dam)
{
	return (project_hack(GF_DISP_EVIL, dam));
}

/*
 * Dispel good monsters
 */
bool_ dispel_good(int dam)
{
	return (project_hack(GF_DISP_GOOD, dam));
}

/*
 * Dispel all monsters
 */
bool_ dispel_monsters(int dam)
{
	return (project_hack(GF_DISP_ALL, dam));
}


/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int who)
{
	int i;
	bool_ sleep = FALSE;
	bool_ speed = FALSE;


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
				sleep = TRUE;
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
				speed = TRUE;
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
static bool get_genocide_race(cptr msg, char *typ)
{
	int i, j;
	cave_type *c_ptr;

	msg_print(msg);
	if (!tgt_pt(&i, &j)) return FALSE;

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
bool_ genocide_aux(bool_ player_cast, char typ)
{
	int i;
	bool_ result = FALSE;
	int msec = delay_factor * delay_factor * delay_factor;
	int dam = 0;

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		auto const r_ptr = m_ptr->race();

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Hack -- Skip Quest Monsters */
		if (m_ptr->mflag & MFLAG_QUEST) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

		/* Oups */
		if (r_ptr->flags2 & RF2_DEATH_ORB)
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

			if (place_monster_aux(wy, wx, m_ptr->r_idx, FALSE, TRUE, MSTATUS_ENEMY))
			{
				cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interesting effect on the %s.", buf);
			}

			return TRUE;
		}

		/* Delete the monster */
		delete_monster_idx(i);

		if (player_cast)
		{
			/* Keep track of damage */
			dam += randint(4);
		}

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		sleep_for(milliseconds(msec));

		/* Take note */
		result = TRUE;
	}

	if (player_cast)
	{
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

	return (result);
}

bool_ genocide(bool_ player_cast)
{
	char typ;

	if (dungeon_flags2 & DF2_NO_GENO) return (FALSE);

	/* Hack -- when you are fated to die, you cant cheat :) */
	if (dungeon_type == DUNGEON_DEATH)
	{
		msg_print("A mysterious force stops the genocide.");
		return FALSE;
	}

	/* Mega-Hack -- Get a monster symbol */
	if (!get_genocide_race("Target a monster to select the race to genocide.", &typ)) return FALSE;

	return (genocide_aux(player_cast, typ));
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool_ mass_genocide(bool_ player_cast)
{
	int i;
	bool_ result = FALSE;
	int msec = delay_factor * delay_factor * delay_factor;
	int dam = 0;

	if (dungeon_flags2 & DF2_NO_GENO) return (FALSE);

	/* Hack -- when you are fated to die, you cant cheat :) */
	if (dungeon_type == DUNGEON_DEATH)
	{
		msg_print("A mysterious force stops the genocide.");
		return FALSE;
	}

	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		auto const r_ptr = m_ptr->race();

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Hack -- Skip Quest Monsters */
		if (m_ptr->mflag & MFLAG_QUEST) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > MAX_SIGHT) continue;

		/* Oups */
		if (r_ptr->flags2 & RF2_DEATH_ORB)
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

			if (place_monster_aux(wy, wx, m_ptr->r_idx, FALSE, TRUE, MSTATUS_ENEMY))
			{
				cmsg_format(TERM_L_BLUE, "The spell seems to produce an ... interesting effect on the %s.", buf);
			}

			return TRUE;
		}

		/* Delete the monster */
		delete_monster_idx(i);

		if (player_cast)
		{
			/* Keep track of damage. */
			dam += randint(3);
		}

		/* Handle */
		handle_stuff();

		/* Fresh */
		Term_fresh();

		/* Delay */
		sleep_for(milliseconds(msec));

		/* Note effect */
		result = TRUE;
	}

	if (player_cast)
	{
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

	return (result);
}

/* Probe a monster */
void do_probe(int m_idx)
{
	char m_name[80];
	monster_type *m_ptr = &m_list[m_idx];

	/* Get "the monster" or "something" */
	monster_desc(m_name, m_ptr, 0x04);

	/* Describe the monster */
	if (!wizard && (m_ptr->status != MSTATUS_COMPANION)) msg_format("%^s has %d hit points.", m_name, m_ptr->hp);
	else
	{
		int i;
		char t_name[80];
		msg_format("%^s has %d(%d) hit points, %d ac, %d speed.", m_name, m_ptr->hp, m_ptr->maxhp, m_ptr->ac, m_ptr->mspeed - 110);
		msg_format("%^s attacks with:", m_name);

		for (i = 0; i < 4; i++)
		{
			msg_format("    Blow %d: %dd%d", i, m_ptr->blow[i].d_dice, m_ptr->blow[i].d_side);
		}

		if (m_ptr->target > 0)
			monster_desc(t_name, &m_list[m_ptr->target], 0x04);
		else if (!m_ptr->target)
			sprintf(t_name, "you");
		else
			sprintf(t_name, "nothing");
		msg_format("%^s target is %s.", m_name, t_name);

		{
			std::ostringstream buf;
			buf << " has " << m_ptr->exp
			    << " exp and needs " << monster_exp(m_ptr->level + 1) << ".";
			msg_format("%^s%s", m_name, buf.str().c_str());
		}
	}

	/* Learn all of the non-spell, non-treasure flags */
	lore_do_probe(m_idx);
}

/*
 * Probe nearby monsters
 */
bool_ probing(void)
{
	int i;

	bool_ probe = FALSE;


	/* Probe all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Require line of sight */
		if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

		/* Probe visible monsters */
		if (m_ptr->ml)
		{
			/* Start the message */
			if (!probe) msg_print("Probing...");

			/* Actualy probe */
			do_probe(i);

			/* Probe worked */
			probe = TRUE;
		}
	}

	/* Done */
	if (probe)
	{
		msg_print("That's all.");
	}

	/* Result */
	return (probe);
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
	int y, x, k, t;

	cave_type *c_ptr;

	bool_ flag = FALSE;


	if (dungeon_flags2 & DF2_NO_GENO)
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
				flag = TRUE;

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
			(void)set_blind(p_ptr->blind + 10 + randint(10));
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
	bool_ hurt = FALSE;
	cave_type *c_ptr;
	bool_ map[32][32];

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
			map[y][x] = FALSE;
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
			map[16 + yy - cy][16 + xx - cx] = TRUE;

			/* Hack -- Take note of player damage */
			if ((yy == p_ptr->py) && (xx == p_ptr->px)) hurt = TRUE;
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
					(void)set_stun(p_ptr->stun + randint(50));
					break;
				}
			case 3:
				{
					msg_print("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)set_stun(p_ptr->stun + randint(50));
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
		map[16 + p_ptr->py - cy][16 + p_ptr->px - cx] = FALSE;

		/* Semi-wraiths have to be hurt *some*, says DG */
		if (race_flags1_p(PR1_SEMI_WRAITH))
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
				if (!(r_ptr->flags2 & (RF2_KILL_WALL)) &&
				                !(r_ptr->flags2 & (RF2_PASS_WALL)))
				{
					char m_name[80];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags1 & (RF1_NEVER_MOVE)))
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

							/* ... nor on the Pattern */
							if ((cave[y][x].feat <= FEAT_PATTERN_XTRA2) &&
							                (cave[y][x].feat >= FEAT_PATTERN_START))
								continue;

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
						update_mon(m_idx, TRUE);

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
				bool_ floor = cave_floor_bold(yy, xx);

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
static void cave_temp_room_lite(void)
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
			update_mon(c_ptr->m_idx, FALSE);

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags2 & (RF2_STUPID)) chance = 10;

			/* Smart monsters always wake up */
			if (r_ptr->flags2 & (RF2_SMART)) chance = 100;

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
static void cave_temp_room_unlite(void)
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
bool_ lite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("You are surrounded by a white light.");
	}

	/* Hook into the "project()" function */
	(void)project(0, rad, p_ptr->py, p_ptr->px, dam, GF_LITE_WEAK, flg);

	/* Lite up the room */
	lite_room(p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool_ unlite_area(int dam, int rad)
{
	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print("Darkness surrounds you.");
	}

	/* Hook into the "project()" function */
	(void)project(0, rad, p_ptr->py, p_ptr->px, dam, GF_DARK_WEAK, flg);

	/* Lite up the room */
	unlite_room(p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool_ fire_ball(int typ, int dir, int dam, int rad)
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
bool_ fire_cloud(int typ, int dir, int dam, int rad, int time)
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
	return (project(0, (rad > 16) ? 16 : rad, ty, tx, dam, typ, flg));
}

/*
 * Cast a wave spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool_ fire_wave(int typ, int dir, int dam, int rad, int time, s32b eff)
{
	project_time_effect = eff;
	return (fire_cloud(typ, dir, dam, rad, time));
}

/*
 * Cast a persistant beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool_ fire_wall(int typ, int dir, int dam, int time)
{
	int flg = PROJECT_BEAM | PROJECT_KILL | PROJECT_STAY | PROJECT_GRID;
	project_time = time;
	return (project_hook(typ, dir, dam, flg));
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

		if (r_ptr->flags3 & RF3_RES_TELE)
		{
			msg_print("Your teleportation is blocked!");
		}
		else
		{
			sound(SOUND_TELEPORT);

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
			update_mon(cave[ty][tx].m_idx, TRUE);

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
		sound(SOUND_TELEPORT);

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

		sound(SOUND_TELEPORT);

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
		update_mon(cave[ty][tx].m_idx, TRUE);

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
bool_ project_hook(int typ, int dir, int dam, int flg)
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
bool_ fire_bolt(int typ, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool_ fire_beam(int typ, int dir, int dam)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;
	return (project_hook(typ, dir, dam, flg));
}


/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool_ fire_bolt_or_beam(int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		return (fire_beam(typ, dir, dam));
	}
	else
	{
		return (fire_bolt(typ, dir, dam));
	}
}


/*
 * Some of the old functions
 */
bool_ lite_line(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(GF_LITE_WEAK, dir, damroll(6, 8), flg));
}


bool_ drain_life(int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_DRAIN, dir, dam, flg));
}


bool_ wall_to_mud(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
	return (project_hook(GF_KILL_WALL, dir, 20 + randint(30), flg));
}


bool_ wizard_lock(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
	return (project_hook(GF_JAM_DOOR, dir, 20 + randint(30), flg));
}

bool_ slow_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_SLOW, dir, p_ptr->lev, flg));
}


bool_ sleep_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}


bool_ confuse_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_CONF, dir, plev, flg));
}


bool_ poly_monster(int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_OLD_POLY, dir, p_ptr->lev, flg));
}


bool_ fear_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_TURN_ALL, dir, plev, flg));
}


bool_ teleport_monster(int dir)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;

	if (p_ptr->resist_continuum)
	{
		msg_print("The space-time continuum can't be disrupted.");
		return FALSE;
	}

	return (project_hook(GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}

bool_ wall_stone(int y, int x)
{
	cave_type *c_ptr = &cave[y][x];
	int flg = PROJECT_GRID | PROJECT_ITEM;
	int featflags = f_info[c_ptr->feat].flags1;

	bool_ dummy = (project(0, 1, y, x, 0, GF_STONE_WALL, flg));

	if (!(featflags & FF1_PERMANENT) && !(featflags & FF1_WALL))
		cave_set_feat(y, x, FEAT_FLOOR);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	return dummy;
}


bool_ destroy_doors_touch(void)
{
	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0, 1, p_ptr->py, p_ptr->px, 0, GF_KILL_DOOR, flg));
}

bool_ sleep_monsters_touch(void)
{
	int flg = PROJECT_KILL | PROJECT_HIDE;
	return (project(0, 1, p_ptr->py, p_ptr->px, p_ptr->lev, GF_OLD_SLEEP, flg));
}


void call_chaos(void)
{
	int Chaos_type, dummy, dir;
	int plev = p_ptr->lev;
	bool_ line_chaos = FALSE;

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
	if (randint(4) == 1) line_chaos = TRUE;

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


static void activate_hi_summon(void)
{
	int i;

	for (i = 0; i < (randint(9) + (dun_level / 40)); i++)
	{
		switch (randint(26) + (dun_level / 20) )
		{
		case 1:
		case 2:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANT);
			break;
		case 3:
		case 4:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_SPIDER);
			break;
		case 5:
		case 6:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HOUND);
			break;
		case 7:
		case 8:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HYDRA);
			break;
		case 9:
		case 10:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANGEL);
			break;
		case 11:
		case 12:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNDEAD);
			break;
		case 13:
		case 14:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_DRAGON);
			break;
		case 15:
		case 16:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_DEMON);
			break;
		case 17:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_WRAITH);
			break;
		case 18:
		case 19:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNIQUE);
			break;
		case 20:
		case 21:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_UNDEAD);
			break;
		case 22:
		case 23:
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_DRAGON);
			break;
		case 24:
		case 25:
			(void) summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_HI_DEMON);
			break;
		default:
			(void) summon_specific(p_ptr->py, p_ptr->px, (((dun_level * 3) / 2) + 5), 0);
		}
	}
}


/*
 * Activate the evil Topi Ylinen curse
 * rr9: Stop the nasty things when a Cyberdemon is summoned
 * or the player gets paralyzed.
 */
void activate_ty_curse(void)
{
	int i = 0;
	bool_ stop_ty = FALSE;

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
			(void) summon_specific(p_ptr->py, p_ptr->px, dun_level, 0);
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
				stop_ty = TRUE;
			}
			if (randint(6) != 1) break;
case 21: case 22: case 23:
			(void)do_dec_stat((randint(6)) - 1, STAT_DEC_NORMAL);
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
				stop_ty = TRUE;
				break;
			}
		default:
			while (i < 6)
			{
				do
				{
					(void)do_dec_stat(i, STAT_DEC_NORMAL);
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
void activate_dg_curse(void)
{
	int i = 0;
	bool_ stop_dg = FALSE;

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
				p_ptr->black_breath = TRUE;
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
				stop_dg = TRUE;
			}
			if (randint(7) != 1) break;
		case 19:
		case 20:
			{
				msg_print("Woah! You see 10 little Morgoths dancing before you!");
				set_confused(p_ptr->confused + randint(13 * 2));
				if (rand_int(2)) stop_dg = TRUE;
			}
			if (randint(7) != 1) break;
		case 21:
		case 22:
		case 23:
			(void)do_dec_stat((randint(6)) - 1, STAT_DEC_PERMANENT);
			if (randint(7) != 1) break;
		case 24:
			msg_print("Huh? Who am I? What am I doing here?");
			lose_all_info();
			break;
case 27: case 28: case 29:
			if (p_ptr->inventory[INVEN_WIELD].k_idx)
			{
				msg_print("Your weapon now seems useless...");
				p_ptr->inventory[INVEN_WIELD].art_flags4 = TR4_NEVER_BLOW;
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
				if (rand_int(2)) stop_dg = TRUE;
				break;
			}
		default:
			while (i < 6)
			{
				do
				{
					(void)do_dec_stat(i, STAT_DEC_NORMAL);
				}
				while (randint(2) == 1);

				i++;
			}
		}
	}
	while ((randint(4) == 1) && !stop_dg);
}


void summon_cyber(void)
{
	int i;
	int max_cyber = (dun_level / 50) + randint(6);

	for (i = 0; i < max_cyber; i++)
	{
		(void)summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_HI_DEMON);
	}
}

static void summon_dragon_riders()
{
	int i;
	int max_dr = (dun_level / 50) + randint(6);

	for (i = 0; i < max_dr; i++)
	{
		(void)summon_specific(p_ptr->py, p_ptr->px, 100, SUMMON_THUNDERLORD);
	}
}



/*
 * Confuse monsters
 */
bool_ confuse_monsters(int dam)
{
	return (project_hack(GF_OLD_CONF, dam));
}


/*
 * Charm monsters
 */
bool_ charm_monsters(int dam)
{
	return (project_hack(GF_CHARM, dam));
}


/*
 * Charm animals
 */
bool_ charm_animals(int dam)
{
	return (project_hack(GF_CONTROL_ANIMAL, dam));
}


/*
 * Stun monsters
 */
bool_ stun_monsters(int dam)
{
	return (project_hack(GF_STUN, dam));
}


/*
 * Mindblast monsters
 */
bool_ mindblast_monsters(int dam)
{
	return (project_hack(GF_PSI, dam));
}


/*
 * Banish all monsters
 */
bool_ banish_monsters(int dist)
{
	return (project_hack(GF_AWAY_ALL, dist));
}


/*
 * Turn everyone
 */
bool_ turn_monsters(int dam)
{
	return (project_hack(GF_TURN_ALL, dam));
}


bool_ charm_monster(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CHARM, dir, plev, flg));
}

bool_ control_one_undead(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CONTROL_UNDEAD, dir, plev, flg));
}


bool_ charm_animal(int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(GF_CONTROL_ANIMAL, dir, plev, flg));
}

void change_wild_mode(void)
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
	p_ptr->leaving = TRUE;
}


void alter_reality(void)
{
	msg_print("The world changes!");

	autosave_checkpoint();

	/* Leaving */
	p_ptr->leaving = TRUE;
}

/* Heal insanity. */
bool_ heal_insanity(int val)
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

		return TRUE;
	}

	return FALSE;
}

/*
 * Send the player shooting through walls in the given direction until
 * they reach a non-wall space, or a monster, or a permanent wall.
 */
bool_ passwall(int dir, bool_ safe)
{
	int x = p_ptr->px, y = p_ptr->py, ox = p_ptr->px, oy = p_ptr->py, lx = p_ptr->px, ly = p_ptr->py;
	cave_type *c_ptr;
	bool_ ok = FALSE;

	if (p_ptr->wild_mode) return FALSE;
	if (p_ptr->inside_quest) return FALSE;
	if (dungeon_flags2 & DF2_NO_TELEPORT) return FALSE;

	/* Must go somewhere */
	if (dir == 5) return FALSE;

	while (TRUE)
	{
		x += ddx[dir];
		y += ddy[dir];
		c_ptr = &cave[y][x];

		/* Perm walls stops the transfer */
		if ((!in_bounds(y, x)) && (f_info[c_ptr->feat].flags1 & FF1_PERMANENT))
		{
			/* get the last working position */
			x -= ddx[dir];
			y -= ddy[dir];
			ok = FALSE;
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
		if (f_info[c_ptr->feat].flags1 & FF1_WALL) continue;

		/* So it must be ok */
		ok = TRUE;
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

	return (TRUE);
}

/*
 * Print a batch of dungeons.
 */
static void print_dungeon_batch(std::vector<int> const &dungeon_idxs,
				int start,
				bool_ mode)
{
	char buf[80];
	int i, j;
	byte attr;

	if (mode) prt(format("     %-31s", "Name"), 1, 20);

	for (i = 0, j = start; i < 20 && j < static_cast<int>(dungeon_idxs.size()); i++, j++)
	{
		dungeon_info_type *d_ptr = &d_info[dungeon_idxs[j]];

		strnfmt(buf, 80, "  %c) %-30s", I2A(i), d_ptr->name);
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
	/* Find the index corresponding to the name */
	for (int i = 1; i < max_d_idx; i++)
	{
		/* Skip non-initialized entries. */
		if (d_info[i].name == nullptr) {
			continue;
		}
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
	char which;
	int start = 0;
	int ret;
	bool_ mode = FALSE;

	// Dungeons available for recall
	std::vector<int> dungeons;
	for (size_t i = 1; i < max_d_idx; i++)
	{
		/* skip "blocked" dungeons */
		if (d_info[i].flags1 & DF1_NO_RECALL) continue;

		if (max_dlv[i])
		{
			dungeons.push_back(i);
		}
	}

	character_icky = TRUE;
	Term_save();

	while (1)
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
			mode = (mode) ? FALSE : TRUE;
			Term_load();
			character_icky = FALSE;
		}

		else if (which == '+')
		{
			start += 20;
			assert(start > 0);
			if (static_cast<size_t>(start) >= dungeons.size())
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
			strcpy(buf, d_info[p_ptr->recall_dungeon].name);
			if (!get_string("Which dungeon? ", buf, 79)) continue;

			/* Find the index corresponding to the name */
			int i = find_dungeon_by_name(buf);

			if (i < 0)
			{
				msg_print("Never heard of that place!");
				msg_print(NULL);
				continue;
			}
			else if (d_info[i].flags1 & DF1_NO_RECALL)
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

	Term_load();
	character_icky = FALSE;

	return ret;
}

bool_ reset_recall(bool_ no_trepas_max_depth)
{
	int dun, depth, max;

	/* Choose dungeon */
	dun = reset_recall_aux();

	if (dun < 1) return FALSE;

	/* Choose depth */
	if (!no_trepas_max_depth)
		max = d_info[dun].maxdepth;
	else
		max = max_dlv[dun];
	depth = get_quantity(format("Which level in %s(%d-%d)? ",
				    d_info[dun].name,
	                            d_info[dun].mindepth, max),
	                     max);

	if (depth < 1) return FALSE;

	/* Enforce minimum level */
	if (depth < d_info[dun].mindepth) depth = d_info[dun].mindepth;

	/* Mega hack -- Forbid levels 99 and 100 */
	if ((depth == 99) || (depth == 100)) depth = 98;

	p_ptr->recall_dungeon = dun;
	max_dlv[p_ptr->recall_dungeon] = depth;

	return TRUE;
}

/*
 * Creates a between gate
 */
void create_between_gate(int dist, int y, int x)
{
	int ii, ij, plev = get_skill(SKILL_CONVEYANCE);

	if (dungeon_flags2 & DF2_NO_TELEPORT)
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
	if (!(f_info[cave[p_ptr->py][p_ptr->px].feat].flags1 & FF1_PERMANENT))
	{
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_BETWEEN);
		cave[p_ptr->py][p_ptr->px].special = ii + (ij << 8);
	}
	if (!(f_info[cave[ij][ii].feat].flags1 & FF1_PERMANENT))
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
	if (f_info[c_ptr->feat].flags1 & FF1_PERMANENT) {
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

void geomancy_random_floor(int y, int x, bool_ kill_wall)
{
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
	if (f_info[c_ptr->feat].flags1 & FF1_PERMANENT) {
		return;
	}
	if (!(kill_wall || (f_info[c_ptr->feat].flags1 & FF1_FLOOR))) {
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

static bool_ geomancy_can_tunnel(int y, int x)
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
		return TRUE;
	default:
		return FALSE;
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
		geomancy_random_floor(y, x, TRUE);

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
