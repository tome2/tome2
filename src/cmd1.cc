/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "cmd1.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd4.hpp"
#include "cmd5.hpp"
#include "dungeon_info_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "format_ext.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "hooks.hpp"
#include "hook_move_in.hpp"
#include "lua_bind.hpp"
#include "melee1.hpp"
#include "melee2.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "monster_type.hpp"
#include "object_flag.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "options.hpp"
#include "player_race_flag.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "spells3.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wild.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::equals;

#define MAX_VAMPIRIC_DRAIN 100


/*
 * Determine if the player "hits" a monster (normal combat).
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_fire(int chance, int ac, int vis)
{
	/* Percentile dice */
	int const k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Never hit */
	if (chance <= 0)
	{
		return false;
	}

	/* Invisible monsters are harder to hit */
	if (!vis) chance = (chance + 1) / 2;

	/* Power competes against armor */
	if (rand_int(chance + luck( -10, 10)) < (ac * 3 / 4))
	{
		return false;
	}

	/* Assume hit */
	return true;
}



/*
 * Determine if the player "hits" a monster (normal combat).
 *
 * Note -- Always miss 5%, always hit 5%, otherwise random.
 */
bool test_hit_norm(int chance, int ac, int vis)
{
	/* Percentile dice */
	int const k = rand_int(100);

	/* Hack -- Instant miss or hit */
	if (k < 10) return (k < 5);

	/* Wimpy attack never hits */
	if (chance <= 0)
	{
		return false;
	}

	/* Penalize invisible targets */
	if (!vis) chance = (chance + 1) / 2;

	/* Power must defeat armor */
	if (rand_int(chance + luck( -10, 10)) < (ac * 3 / 4))
	{
		return false;
	}

	/* Assume hit */
	return true;
}



/*
 * Critical hits (from objects thrown by player)
 * Factor in item weight, total plusses, and player level.
 */
s16b critical_shot(int weight, int plus, int dam, int skill)
{
	int i, k;


	/* Extract "shot" power */
	i = (weight + ((p_ptr->to_h + plus) * 4) +
	     get_skill_scale(skill, 100));
	i += 50 * p_ptr->xtra_crit;
	i += luck( -100, 100);

	/* Critical hit */
	if (randint(5000) <= i)
	{
		k = weight + randint(500);

		if (k < 500)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 1000)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
	}

	return (dam);
}

/*
 * Critical hits (by player)
 *
 * Factor in weapon weight, total plusses, player level.
 */
s16b critical_norm(int weight, int plus, int dam, int weapon_tval, bool *done_crit)
{
	int i, k, num = randint(5000);

	*done_crit = false;

	/* Extract "blow" power */
	i = (weight + ((p_ptr->to_h + plus) * 5) +
	     get_skill_scale(p_ptr->melee_style, 150));
	i += 50 * p_ptr->xtra_crit;
	if ((weapon_tval == TV_SWORD) && (weight < 50) && get_skill(SKILL_CRITS))
	{
		i += get_skill_scale(SKILL_CRITS, 40 * 50);
	}
	i += luck( -100, 100);

	/* Force good strikes */
	if (p_ptr->tim_deadly)
	{
		set_tim_deadly(p_ptr->tim_deadly - 1);
		msg_print("It was a *GREAT* hit!");
		dam = 3 * dam + 20;
		*done_crit = true;
	}

	/* Chance */
	else if (num <= i)
	{
		k = weight + randint(650);
		if ((weapon_tval == TV_SWORD) && (weight < 50) && get_skill(SKILL_CRITS))
		{
			k += get_skill_scale(SKILL_CRITS, 400);
		}

		if (k < 400)
		{
			msg_print("It was a good hit!");
			dam = 2 * dam + 5;
		}
		else if (k < 700)
		{
			msg_print("It was a great hit!");
			dam = 2 * dam + 10;
		}
		else if (k < 900)
		{
			msg_print("It was a superb hit!");
			dam = 3 * dam + 15;
		}
		else if (k < 1300)
		{
			msg_print("It was a *GREAT* hit!");
			dam = 3 * dam + 20;
		}
		else
		{
			msg_print("It was a *SUPERB* hit!");
			dam = ((7 * dam) / 2) + 25;
		}
		*done_crit = true;
	}

	return (dam);
}



/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.  XXX XXX
 *
 * Note that most brands and slays are x3, except Slay Animal (x2),
 * Slay Evil (x2), and Kill dragon (x5).
 */
s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr,
                 s32b *special)
{
	int mult = 1;

	auto const r_ptr = m_ptr->race();

	/* Extract the flags */
	auto const f = object_flags(o_ptr);

	/* Some "weapons" and "ammo" do extra damage */
	switch (o_ptr->tval)
	{
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
	case TV_BOOMERANG:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
	case TV_AXE:
	case TV_DIGGING:
		{
			/* Slay Animal */
			if ((f & TR_SLAY_ANIMAL) && (r_ptr->flags & RF_ANIMAL))
			{
				if (mult < 2) mult = 2;
			}

			/* Slay Evil */
			if ((f & TR_SLAY_EVIL) && (r_ptr->flags & RF_EVIL))
			{
				if (mult < 2) mult = 2;
			}

			/* Slay Undead */
			if ((f & TR_SLAY_UNDEAD) && (r_ptr->flags & RF_UNDEAD))
			{
				if (mult < 3) mult = 3;
			}

			/* Slay Demon */
			if ((f & TR_SLAY_DEMON) && (r_ptr->flags & RF_DEMON))
			{
				if (mult < 3) mult = 3;
			}

			/* Slay Orc */
			if ((f & TR_SLAY_ORC) && (r_ptr->flags & RF_ORC))
			{
				if (mult < 3) mult = 3;
			}

			/* Slay Troll */
			if ((f & TR_SLAY_TROLL) && (r_ptr->flags & RF_TROLL))
			{
				if (mult < 3) mult = 3;
			}

			/* Slay Giant */
			if ((f & TR_SLAY_GIANT) && (r_ptr->flags & RF_GIANT))
			{
				if (mult < 3) mult = 3;
			}

			/* Slay Dragon  */
			if ((f & TR_SLAY_DRAGON) && (r_ptr->flags & RF_DRAGON))
			{
				if (mult < 3) mult = 3;
			}

			/* Execute Dragon */
			if ((f & TR_KILL_DRAGON) && (r_ptr->flags & RF_DRAGON))
			{
				if (mult < 5) mult = 5;
			}

			/* Execute Undead */
			if ((f & TR_KILL_UNDEAD) && (r_ptr->flags & RF_UNDEAD))
			{

				if (mult < 5) mult = 5;
			}

			/* Execute Demon */
			if ((f & TR_KILL_DEMON) && (r_ptr->flags & RF_DEMON))
			{
				if (mult < 5) mult = 5;
			}


			/* Brand (Acid) */
			if (f & TR_BRAND_ACID)
			{
				if (r_ptr->flags & RF_IM_ACID)
				{
					// No additional multiplier
				}
				else if (r_ptr->flags & RF_SUSCEP_ACID)
				{
					if (mult < 6) mult = 6;
				}
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Elec) */
			if (f & TR_BRAND_ELEC)
			{
				if (r_ptr->flags & RF_IM_ELEC)
				{
					// No additional multiplier
				}
				else if (r_ptr->flags & RF_SUSCEP_ELEC)
				{
					if (mult < 6) mult = 6;
				}
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Fire) */
			if (f & TR_BRAND_FIRE)
			{
				if (r_ptr->flags & RF_IM_FIRE)
				{
					// No additional multiplier
				}
				else if (r_ptr->flags & RF_SUSCEP_FIRE)
				{
					if (mult < 6) mult = 6;
				}
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Cold) */
			if (f & TR_BRAND_COLD)
			{
				if (r_ptr->flags & RF_IM_COLD)
				{
					// No additional multiplier
				}
				else if (r_ptr->flags & RF_SUSCEP_COLD)
				{
					if (mult < 6) mult = 6;
				}
				else
				{
					if (mult < 3) mult = 3;
				}
			}

			/* Brand (Poison) */
			if ((f & TR_BRAND_POIS) || (p_ptr->tim_poison))
			{
				if (r_ptr->flags & RF_IM_POIS)
				{
					// No additional damage
				}
				else if (r_ptr->flags & RF_SUSCEP_POIS)
				{
					if (mult < 6) mult = 6;
					if (magik(95)) *special |= SPEC_POIS;
				}
				else
				{
					if (mult < 3) mult = 3;
					if (magik(50)) *special |= SPEC_POIS;
				}
			}

			/* Wounding */
			if (f & TR_WOUNDING)
			{
				if (r_ptr->flags & RF_NO_CUT)
				{
					// No additional damage
				}
				else
				{
					if (magik(50)) *special |= SPEC_CUT;
				}
			}
			break;
		}
	}


	/* Return the total damage */
	return (tdam * mult);
}


/*
 * Player "wants" to pick up an object or gold.
 * Note that we ONLY handle things that can be picked up.
 * See "move_player()" for handling of other things.
 */
void carry(int pickup)
{
	if (!p_ptr->disembodied)
	{
		py_pickup_floor(pickup);
	}
}


static void touch_zap_player(monster_type *m_ptr)
{
	auto r_ptr = m_ptr->race();

	if (r_ptr->flags & RF_AURA_FIRE)
	{
		if (!(p_ptr->immune_fire))
		{
			char aura_dam[80];

			int aura_damage =
			        damroll(1 + (m_ptr->level / 26), 1 + (m_ptr->level / 17));

			/* Hack -- Get the "died from" name */
			monster_desc(aura_dam, m_ptr, 0x88);

			msg_print("You are suddenly very hot!");

			if (p_ptr->oppose_fire) aura_damage = (aura_damage + 2) / 3;
			if (p_ptr->resist_fire) aura_damage = (aura_damage + 2) / 3;
			if (p_ptr->sensible_fire) aura_damage = (aura_damage + 2) * 2;

			take_hit(aura_damage, aura_dam);
			handle_stuff();
		}
	}


	if (r_ptr->flags & RF_AURA_ELEC)
	{
		if (!(p_ptr->immune_elec))
		{
			char aura_dam[80];

			int aura_damage =
			        damroll(1 + (m_ptr->level / 26), 1 + (m_ptr->level / 17));

			/* Hack -- Get the "died from" name */
			monster_desc(aura_dam, m_ptr, 0x88);

			if (p_ptr->oppose_elec) aura_damage = (aura_damage + 2) / 3;
			if (p_ptr->resist_elec) aura_damage = (aura_damage + 2) / 3;

			msg_print("You get zapped!");
			take_hit(aura_damage, aura_dam);
			handle_stuff();
		}
	}
}


/*
 * Carried monster can attack too.
 * Based on monst_attack_monst.
 */
static void carried_monster_attack(s16b m_idx, int x, int y)
{
	auto const &r_info = game->edit_data.r_info;

	monster_type *t_ptr = &m_list[m_idx];

	int ap_cnt;

	int ac, rlev, pt;

	char t_name[80];

	char temp[80];

	bool blinked = false, touched = false;

	byte y_saver = t_ptr->fy;

	byte x_saver = t_ptr->fx;

	/* Get the carried monster */
	auto o_ptr = &p_ptr->inventory[INVEN_CARRY];
	if (!o_ptr->k_ptr)
	{
		return;
	}

	/* Get monster race of the symbiote */
	auto r_ptr = &r_info[o_ptr->pval];

	/* Not allowed to attack */
	if (r_ptr->flags & RF_NEVER_BLOW) return;

	/* Total armor */
	ac = t_ptr->ac;

	/* Extract the effective monster level */
	rlev = ((o_ptr->elevel >= 1) ? o_ptr->elevel : 1);

	/* Get the monster name (or "it") */
	monster_desc(t_name, t_ptr, 0);

	/* Assume no blink */
	blinked = false;

	if (!t_ptr->ml)
	{
		msg_print("You hear noise.");
	}

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;

		/* Stop attacking if the target dies! */
		if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
			break;

		/* Hack -- no more attacks */
		if (!method) break;

		if (blinked)			/* Stop! */
		{
			/* break; */
		}

		/* Extract the attack "power" */
		power = get_attack_power(effect);

		/* Monster hits */
		if (!effect || check_hit2(power, rlev, ac))
		{
			/* Always disturbing */
			disturb();

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hits %s.";
					touched = true;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touches %s.";
					touched = true;
					break;
				}

			case RBM_PUNCH:
				{
					act = "punches %s.";
					touched = true;
					break;
				}

			case RBM_KICK:
				{
					act = "kicks %s.";
					touched = true;
					break;
				}

			case RBM_CLAW:
				{
					act = "claws %s.";
					touched = true;
					break;
				}

			case RBM_BITE:
				{
					act = "bites %s.";
					touched = true;
					break;
				}

			case RBM_STING:
				{
					act = "stings %s.";
					touched = true;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's %s.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butts %s.";
					touched = true;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crushes %s.";
					touched = true;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulfs %s.";
					touched = true;
					break;
				}

			case RBM_CHARGE:
				{
					act = "charges %s.";
					touched = true;
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawls on %s.";
					touched = true;
					break;
				}

			case RBM_DROOL:
				{
					act = "drools on %s.";
					touched = false;
					break;
				}

			case RBM_SPIT:
				{
					act = "spits on %s.";
					touched = false;
					break;
				}

			case RBM_GAZE:
				{
					act = "gazes at %s.";
					touched = false;
					break;
				}

			case RBM_WAIL:
				{
					act = "wails at %s.";
					touched = false;
					break;
				}

			case RBM_SPORE:
				{
					act = "releases spores at %s.";
					touched = false;
					break;
				}

			case RBM_XXX4:
				{
					act = "projects XXX4's at %s.";
					touched = false;
					break;
				}

			case RBM_BEG:
				{
					act = "begs %s for money.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_INSULT:
				{
					act = "insults %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_MOAN:
				{
					act = "moans at %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_SHOW:
				{
					act = "sings to %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}
			}

			/* Message */
			if (act)
			{
				strnfmt(temp, sizeof(temp), act, t_name);
				if (t_ptr->ml)
				{
					msg_format("%s %s", symbiote_name(true).c_str(), temp);
				}

			}

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			pt = GF_MISSILE;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					damage = 0;
					pt = 0;
					break;
				}

			case RBE_HURT:
			case RBE_SANITY:
				{
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);
					break;
				}

			case RBE_POISON:
			case RBE_DISEASE:
				{
					pt = GF_POIS;
					break;
				}

			case RBE_UN_BONUS:
			case RBE_UN_POWER:
			case RBE_ABOMINATION:
				{
					pt = GF_DISENCHANT;
					break;
				}

			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
				{
					pt = damage = 0;
					break;
				}

			case RBE_EAT_ITEM:
			case RBE_EAT_GOLD:
				{
					pt = damage = 0;
					if (randint(2) == 1) blinked = true;
					break;
				}

			case RBE_ACID:
				{
					pt = GF_ACID;
					break;
				}

			case RBE_ELEC:
				{
					pt = GF_ELEC;
					break;
				}

			case RBE_FIRE:
				{
					pt = GF_FIRE;
					break;
				}

			case RBE_COLD:
				{
					pt = GF_COLD;
					break;
				}

			case RBE_BLIND:
				{
					break;
				}

			case RBE_CONFUSE:
			case RBE_HALLU:
				{
					pt = GF_CONFUSION;
					break;
				}

			case RBE_TERRIFY:
				{
					pt = GF_TURN_ALL;
					break;
				}

			case RBE_PARALYZE:
				{
					pt = GF_OLD_SLEEP; 	/* sort of close... */
					break;
				}

			case RBE_LOSE_STR:
			case RBE_LOSE_INT:
			case RBE_LOSE_WIS:
			case RBE_LOSE_DEX:
			case RBE_LOSE_CON:
			case RBE_LOSE_CHR:
			case RBE_LOSE_ALL:
			case RBE_PARASITE:
				{
					break;
				}

			case RBE_SHATTER:
				{
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
							earthquake(p_ptr->py, p_ptr->px, 8);
					}
					break;
				}

			case RBE_EXP_10:
			case RBE_EXP_20:
			case RBE_EXP_40:
			case RBE_EXP_80:
				{
					pt = GF_NETHER;
					break;
				}

			case RBE_TIME:
				{
					pt = GF_TIME;
					break;
				}

			default:
				{
					pt = 0;
					break;
				}
			}

			if (pt)
			{
				/* Do damage if not exploding */
				project(0, 0, t_ptr->fy, t_ptr->fx,
				        (pt == GF_OLD_SLEEP ? r_ptr->level : damage), pt,
				        PROJECT_KILL | PROJECT_STOP);

				if (touched)
				{
					auto tr_ptr = t_ptr->race();
					/* Aura fire */
					if ((tr_ptr->flags & RF_AURA_FIRE) &&
					                !(r_ptr->flags & RF_IM_FIRE))
					{
						if (t_ptr->ml)
						{
							blinked = false;
							msg_format("You are suddenly very hot!");
						}
						project(m_idx, 0, p_ptr->py, p_ptr->px,
						        damroll(1 + ((t_ptr->level) / 26),
						                1 + ((t_ptr->level) / 17)),
						        GF_FIRE, PROJECT_KILL | PROJECT_STOP);
					}

					/* Aura elec */
					if ((tr_ptr->flags & RF_AURA_ELEC) &&
					                !(r_ptr->flags & RF_IM_ELEC))
					{
						if (t_ptr->ml)
						{
							blinked = false;
							msg_format("You get zapped!");
						}
						project(m_idx, 0, p_ptr->py, p_ptr->px,
						        damroll(1 + ((t_ptr->level) / 26),
						                1 + ((t_ptr->level) / 17)),
						        GF_ELEC, PROJECT_KILL | PROJECT_STOP);
					}
				}
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
			case RBM_HIT:
			case RBM_TOUCH:
			case RBM_PUNCH:
			case RBM_KICK:
			case RBM_CLAW:
			case RBM_BITE:
			case RBM_STING:
			case RBM_XXX1:
			case RBM_BUTT:
			case RBM_CRUSH:
			case RBM_ENGULF:
			case RBM_CHARGE:
				{
					/* Disturb */
					disturb();

					/* Message */
					msg_format("%s misses %s.", symbiote_name(true).c_str(), t_name);
					break;
				}
			}
		}
	}

	/* Blink away */
	if (blinked)
	{
		msg_format("You and %s flee laughing!", symbiote_name(false).c_str());

		teleport_player(MAX_SIGHT * 2 + 5);
	}
}

/*
 * Carried monster can attack too.
 * Based on monst_attack_monst.
 */
static void incarnate_monster_attack(s16b m_idx, int x, int y)
{
	auto const &r_info = game->edit_data.r_info;

	monster_type *t_ptr = &m_list[m_idx];

	auto tr_ptr = t_ptr->race();

	int ap_cnt;

	int ac, rlev, pt;

	char t_name[80];

	char temp[80];

	bool blinked = false, touched = false;

	byte y_saver = t_ptr->fy;

	byte x_saver = t_ptr->fx;


	if (!p_ptr->body_monster) return;

	auto r_ptr = &r_info[p_ptr->body_monster];

	/* Not allowed to attack */
	if (r_ptr->flags & RF_NEVER_BLOW) return;

	/* Total armor */
	ac = t_ptr->ac;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Get the monster name (or "it") */
	monster_desc(t_name, t_ptr, 0);

	/* Assume no blink */
	blinked = false;

	if (!t_ptr->ml)
	{
		msg_print("You hear noise.");
	}

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < std::min<int>(p_ptr->num_blow, 4); ap_cnt++)
	{
		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;

		/* Stop attacking if the target dies! */
		if (t_ptr->fx != x_saver || t_ptr->fy != y_saver)
			break;

		/* Hack -- no more attacks */
		if (!method) break;

		if (blinked)			/* Stop! */
		{
			/* break; */
		}

		/* Extract the attack "power" */
		power = get_attack_power(effect);

		/* Monster hits */
		if (!effect || check_hit2(power, rlev, ac))
		{
			/* Always disturbing */
			disturb();

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hit %s.";
					touched = true;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touch %s.";
					touched = true;
					break;
				}

			case RBM_PUNCH:
				{
					act = "punch %s.";
					touched = true;
					break;
				}

			case RBM_KICK:
				{
					act = "kick %s.";
					touched = true;
					break;
				}

			case RBM_CLAW:
				{
					act = "claw %s.";
					touched = true;
					break;
				}

			case RBM_BITE:
				{
					act = "bite %s.";
					touched = true;
					break;
				}

			case RBM_STING:
				{
					act = "sting %s.";
					touched = true;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's %s.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butt %s.";
					touched = true;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crush %s.";
					touched = true;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulf %s.";
					touched = true;
					break;
				}

			case RBM_CHARGE:
				{
					act = "charge %s.";
					touched = true;
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawl on %s.";
					touched = true;
					break;
				}

			case RBM_DROOL:
				{
					act = "drool on %s.";
					touched = false;
					break;
				}

			case RBM_SPIT:
				{
					act = "spit on %s.";
					touched = false;
					break;
				}

			case RBM_GAZE:
				{
					act = "gaze at %s.";
					touched = false;
					break;
				}

			case RBM_WAIL:
				{
					act = "wail at %s.";
					touched = false;
					break;
				}

			case RBM_SPORE:
				{
					act = "release spores at %s.";
					touched = false;
					break;
				}

			case RBM_XXX4:
				{
					act = "project XXX4's at %s.";
					touched = false;
					break;
				}

			case RBM_BEG:
				{
					act = "beg %s for money.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_INSULT:
				{
					act = "insult %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_MOAN:
				{
					act = "moan at %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}

			case RBM_SHOW:
				{
					act = "sing to %s.";
					touched = false;
					t_ptr->csleep = 0;
					break;
				}
			}

			/* Message */
			if (act)
			{
				strnfmt(temp, sizeof(temp), act, t_name);
				if (t_ptr->ml)
					msg_format("You %s", temp);

			}

			/* Roll out the damage */
			damage = damroll(d_dice, d_side) + p_ptr->to_d;

			pt = GF_MISSILE;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					damage = 0;
					pt = 0;
					break;
				}

			case RBE_HURT:
			case RBE_SANITY:
				{
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);
					break;
				}

			case RBE_POISON:
			case RBE_DISEASE:
				{
					pt = GF_POIS;
					break;
				}

			case RBE_UN_BONUS:
			case RBE_UN_POWER:
				{
					pt = GF_DISENCHANT;
					break;
				}

			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
				{
					pt = damage = 0;
					break;
				}

			case RBE_EAT_ITEM:
			case RBE_EAT_GOLD:
				{
					pt = damage = 0;
					if (randint(2) == 1) blinked = true;
					break;
				}

			case RBE_ACID:
				{
					pt = GF_ACID;
					break;
				}

			case RBE_ELEC:
				{
					pt = GF_ELEC;
					break;
				}

			case RBE_FIRE:
				{
					pt = GF_FIRE;
					break;
				}

			case RBE_COLD:
				{
					pt = GF_COLD;
					break;
				}

			case RBE_BLIND:
				{
					break;
				}

			case RBE_HALLU:
			case RBE_CONFUSE:
				{
					pt = GF_CONFUSION;
					break;
				}

			case RBE_TERRIFY:
				{
					pt = GF_TURN_ALL;
					break;
				}

			case RBE_PARALYZE:
				{
					pt = GF_OLD_SLEEP; 	/* sort of close... */
					break;
				}

			case RBE_LOSE_STR:
			case RBE_LOSE_INT:
			case RBE_LOSE_WIS:
			case RBE_LOSE_DEX:
			case RBE_LOSE_CON:
			case RBE_LOSE_CHR:
			case RBE_LOSE_ALL:
			case RBE_PARASITE:
				{
					break;
				}

			case RBE_SHATTER:
				{
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
							earthquake(p_ptr->py, p_ptr->px, 8);
					}
					break;
				}

			case RBE_EXP_10:
			case RBE_EXP_20:
			case RBE_EXP_40:
			case RBE_EXP_80:
				{
					pt = GF_NETHER;
					break;
				}

			case RBE_TIME:
				{
					pt = GF_TIME;
					break;
				}

			default:
				{
					pt = 0;
					break;
				}
			}

			if (pt)
			{
				/* Do damage if not exploding */
				project(0, 0, t_ptr->fy, t_ptr->fx,
				        (pt == GF_OLD_SLEEP ? p_ptr->lev * 2 : damage), pt,
				        PROJECT_KILL | PROJECT_STOP);

				if (touched)
				{
					/* Aura fire */
					if ((tr_ptr->flags & RF_AURA_FIRE) &&
					                !(r_ptr->flags & RF_IM_FIRE))
					{
						if (t_ptr->ml)
						{
							blinked = false;
							msg_format("You are suddenly very hot!");
						}
						project(m_idx, 0, p_ptr->py, p_ptr->px,
						        damroll(1 + ((t_ptr->level) / 26),
						                1 + ((t_ptr->level) / 17)),
						        GF_FIRE, PROJECT_KILL | PROJECT_STOP);
					}

					/* Aura elec */
					if ((tr_ptr->flags & RF_AURA_ELEC) &&
					                !(r_ptr->flags & RF_IM_ELEC))
					{
						if (t_ptr->ml)
						{
							blinked = false;
							msg_format("You get zapped!");
						}
						project(m_idx, 0, p_ptr->py, p_ptr->px,
						        damroll(1 + ((t_ptr->level) / 26),
						                1 + ((t_ptr->level) / 17)),
						        GF_ELEC, PROJECT_KILL | PROJECT_STOP);
					}

				}
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
			case RBM_HIT:
			case RBM_TOUCH:
			case RBM_PUNCH:
			case RBM_KICK:
			case RBM_CLAW:
			case RBM_BITE:
			case RBM_STING:
			case RBM_XXX1:
			case RBM_BUTT:
			case RBM_CRUSH:
			case RBM_ENGULF:
			case RBM_CHARGE:
				{
					/* Disturb */
					disturb();

					/* Message */
					msg_format("You miss %s.", t_name);

					break;
				}
			}
		}
	}

	/* Blink away */
	if (blinked)
	{
		msg_print("You flee laughing!");

		teleport_player(MAX_SIGHT * 2 + 5);
	}
}


/*
 * Fetch an attack description from dam_*.txt files.
 */

static void flavored_attack(int percent, char *output)
{
	int insanity = (p_ptr->msane - p_ptr->csane) * 100 / p_ptr->msane;
	bool insane = (rand_int(100) < insanity);

	if (percent < 5)
	{
		if (!insane)
			strcpy(output, "You scratch %s.");
		else
			get_rnd_line("dam_none.txt", output);

	}
	else if (percent < 30)
	{
		if (!insane)
			strcpy(output, "You hit %s.");
		else
			get_rnd_line("dam_med.txt", output);
	}
	else if (percent < 60)
	{
		if (!insane)
			strcpy(output, "You wound %s.");
		else
			get_rnd_line("dam_lots.txt", output);
	}
	else if (percent < 95)
	{
		if (!insane)
			strcpy(output, "You cripple %s.");
		else
			get_rnd_line("dam_huge.txt", output);

	}
	else
	{
		if (!insane)
			strcpy(output, "You demolish %s.");
		else
			get_rnd_line("dam_xxx.txt", output);
	}
}


/*
 * Apply the special effects of an attack
 */
void attack_special(monster_type *m_ptr, s32b special, int dam)
{
	auto const r_ptr = m_ptr->race();

	/* Extract monster name (or "it") */
	char m_name[80];
	monster_desc(m_name, m_ptr, 0);

	/* Special - Cut monster */
	if (special & SPEC_CUT)
	{
		if (r_ptr->flags & RF_NO_CUT)
		{
			// No damage
		}
		else if (rand_int(100) >= r_ptr->level)
		{
			if (m_ptr->bleeding)
			{
				msg_format("%^s is bleeding more strongly.", m_name);
			}
			else
			{
				msg_format("%^s is bleeding.", m_name);
			}

			m_ptr->bleeding += dam * 2;
		}
	}

	/* Special - Poison monster */
	if (special & SPEC_POIS)
	{
		if (r_ptr->flags & RF_IM_POIS)
		{
			// No damage
		}
		else if (r_ptr->flags & RF_SUSCEP_POIS)
		{
			if (m_ptr->poisoned)
			{
				msg_format("%^s is more poisoned.", m_name);
			}
			else
			{
				msg_format("%^s is poisoned.", m_name);
			}

			m_ptr->poisoned += dam * 2;
		}
		else if (rand_int(100) >= r_ptr->level)
		{
			if (m_ptr->poisoned)
			{
				msg_format("%^s is more poisoned.", m_name);
			}
			else
			{
				msg_format("%^s is poisoned.", m_name);
			}

			m_ptr->poisoned += dam;
		}
	}
}


/*
 * Bare handed attacks
 */
static void py_attack_hand(int *k, monster_type *m_ptr, s32b *special)
{
	s16b special_effect = 0, stun_effect = 0, times = 0;
	martial_arts *blow_table = ma_blows;
	int resist_stun = 0;
	int max = MAX_MA;
	bool desc = false;
	int plev = p_ptr->lev;

	if ((!p_ptr->body_monster) && (p_ptr->mimic_form == resolve_mimic_name("Bear")) &&
	                (p_ptr->melee_style == SKILL_BEAR))
	{
		blow_table = bear_blows;
		max = MAX_BEAR;
		plev = get_skill(SKILL_BEAR);
	}
	if (p_ptr->melee_style == SKILL_HAND)
	{
		blow_table = ma_blows;
		max = MAX_MA;
		plev = get_skill(SKILL_HAND);
	}
	martial_arts *ma_ptr = &blow_table[0];
	martial_arts *old_ptr = &blow_table[0];

	/* Extract monster name (or "it") */
	auto const r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_UNIQUE) resist_stun += 88;
	if (r_ptr->flags & RF_NO_CONF) resist_stun += 44;
	if (r_ptr->flags & RF_NO_SLEEP) resist_stun += 44;
	if ((r_ptr->flags & RF_UNDEAD) ||
	                (r_ptr->flags & RF_NONLIVING)) resist_stun += 88;

	if (plev)
	{
		for (times = 0; times < (plev < 7 ? 1 : plev / 7); times++)
		{
			do
			{
				ma_ptr = &blow_table[(randint(max)) - 1];
			}
			while ((ma_ptr->min_level > plev) || (randint(plev) < ma_ptr->chance));

			/* keep the highest level attack available we found */
			if ((ma_ptr->min_level > old_ptr->min_level) &&
			                !(p_ptr->stun || p_ptr->confused))
			{
				old_ptr = ma_ptr;

				if (wizard && options->cheat_xtra)
				{
					msg_print("Attack re-selected.");
				}
			}
			else
			{
				ma_ptr = old_ptr;
			}
		}
	}

	*k = damroll(ma_ptr->dd, ma_ptr->ds);

	/* Extract name */
	char m_name[80];
	monster_desc(m_name, m_ptr, 0);

	/* Describe attack */
	if (ma_ptr->effect & MA_KNEE)
	{
		if (r_ptr->flags & RF_MALE)
		{
			if (!desc) msg_format("You hit %s in the groin with your knee!",
				                      m_name);
			special_effect = MA_KNEE;
		}
		else if (!desc) msg_format(ma_ptr->desc, m_name);

		desc = true;
	}
	if (ma_ptr->effect & MA_FULL_SLOW)
	{
		special_effect = MA_SLOW;
		if (!desc) msg_format(ma_ptr->desc, m_name);

		desc = true;
	}
	if (ma_ptr->effect & MA_SLOW)
	{
		if (!
		                ((r_ptr->flags & RF_NEVER_MOVE) ||
		                 strchr("UjmeEv$,DdsbBFIJQSXclnw!=?", r_ptr->d_char)))
		{
			if (!desc) msg_format("You kick %s in the ankle.", m_name);
			special_effect = MA_SLOW;
		}
		else if (!desc) msg_format(ma_ptr->desc, m_name);

		desc = true;
	}
	if (ma_ptr->effect & MA_STUN)
	{
		if (ma_ptr->power)
		{
			stun_effect = (ma_ptr->power / 2) + randint(ma_ptr->power / 2);
		}

		if (!desc) msg_format(ma_ptr->desc, m_name);
		desc = true;
	}
	if (ma_ptr->effect & MA_WOUND)
	{
		if (magik(ma_ptr->power))
		{
			*special |= SPEC_CUT;
		}
		if (!desc) msg_format(ma_ptr->desc, m_name);
		desc = true;
	}

	bool done_crit;
	*k = critical_norm(plev * (randint(10)), ma_ptr->min_level, *k, -1, &done_crit);

	if ((special_effect & MA_KNEE) && ((*k + p_ptr->to_d) < m_ptr->hp))
	{
		msg_format("%^s moans in agony!", m_name);
		stun_effect = 7 + randint(13);
		resist_stun /= 3;
	}
	if (((special_effect & MA_FULL_SLOW) || (special_effect & MA_SLOW)) &&
	                ((*k + p_ptr->to_d) < m_ptr->hp))
	{
		if (!(r_ptr->flags & RF_UNIQUE) &&
		                (randint(plev) > m_ptr->level) && m_ptr->mspeed > 60)
		{
			msg_format("%^s starts limping.", m_name);
			m_ptr->mspeed -= 10;
		}
	}

	if (stun_effect && ((*k + p_ptr->to_d) < m_ptr->hp))
	{
		if (plev > randint(m_ptr->level + resist_stun + 10))
		{
			if (m_ptr->stunned)
				msg_format("%^s is still stunned.", m_name);
			else
				msg_format("%^s is stunned.", m_name);

			m_ptr->stunned += (stun_effect);
		}
	}
}


/*
 * Apply nazgul effects
 */
static void do_nazgul(int *k, int *num, int num_blow, int weap, std::shared_ptr<monster_race> r_ptr,
               object_type *o_ptr)
{
	bool allow_shatter = true;

	/* Extract mundane-ness of the current weapon */
	auto const f = object_flags(o_ptr);

	/* It should be Slay Evil, Slay Undead, or *Slay Undead* */
	bool const mundane =
		!(f & TR_SLAY_EVIL) &&
		!(f & TR_SLAY_UNDEAD) &&
		!(f & TR_KILL_UNDEAD);

	/* Some blades can resist shattering */
	if (f & TR_RES_MORGUL)
	{
		allow_shatter = false;
	}

	/* Mega Hack -- Hitting Nazgul is REALY dangerous (ideas from Akhronath) */
	if (r_ptr->flags & RF_NAZGUL)
	{
		if ((!o_ptr->name2) && (!artifact_p(o_ptr)) && allow_shatter)
		{
			msg_print("Your weapon *DISINTEGRATES*!");
			*k = 0;

			inc_stack_size_ex(INVEN_WIELD + weap, -1, OPTIMIZE, NO_DESCRIBE);

			/* To stop attacking */
			*num = num_blow;
		}
		else if (o_ptr->name2)
		{
			if (mundane)
			{
				msg_print
				("The Ringwraith is IMPERVIOUS to the mundane weapon.");
				*k = 0;
			}

			/* 25% chance of getting destroyed */
			if (magik(25) && allow_shatter)
			{
				msg_print("Your weapon is destroyed!");
				
				inc_stack_size_ex(INVEN_WIELD + weap, -1, OPTIMIZE, NO_DESCRIBE);

				/* To stop attacking */
				*num = num_blow;
			}
		}
		else if (artifact_p(o_ptr))
		{
			if (mundane)
			{
				msg_print
				("The Ringwraith is IMPERVIOUS to the mundane weapon.");
				*k = 0;
			}

			apply_disenchant(INVEN_WIELD + weap);

			/* 1/1000 chance of getting destroyed */
			if (!rand_int(1000) && allow_shatter)
			{
				msg_print("Your weapon is destroyed!");

				inc_stack_size_ex(INVEN_WIELD + weap, -1, OPTIMIZE, NO_DESCRIBE);

				/* To stop attacking */
				*num = num_blow;
			}
		}

		/* If any damage is done, then 25% chance of getting the Black Breath */
		if (*k)
		{
			if (magik(25))
			{
				msg_print("Your foe calls upon your soul!");
				msg_print
				("You feel the Black Breath slowly draining you of life...");
				p_ptr->black_breath = true;
			}
		}
	}
}


/*
 * Player attacks a (poor, defenseless) creature        -RAK-
 *
 * If no "weapon" is available, then "punch" the monster one time.
 */
void py_attack(int y, int x, int max_blow)
{
	auto const &r_info = game->edit_data.r_info;

	int num = 0, k, bonus, chance;

	s32b special = 0;

	cave_type *c_ptr = &cave[y][x];

	monster_type *m_ptr = &m_list[c_ptr->m_idx];

	bool fear = false;

	bool mdeath = false;

	bool backstab = false;

	bool vorpal_cut = false;

	int chaos_effect = 0;

	bool stab_fleeing = false;

	bool do_quake = false;

	bool done_crit = false;

	bool drain_msg = true;

	int drain_result = 0, drain_heal = 0;

	int drain_left = MAX_VAMPIRIC_DRAIN;

	int weap;

	/* Disturb the player */
	disturb();

	if (r_info[p_ptr->body_monster].flags & RF_NEVER_BLOW)
	{
		msg_print("You cannot attack in this form!");
		return;
	}

	if (get_skill(SKILL_BACKSTAB))
	{
		if ((m_ptr->csleep) && (m_ptr->ml))
		{
			/* Can't backstab creatures that we can't see, right? */
			backstab = true;
		}
		else if ((m_ptr->monfear) && (m_ptr->ml))
		{
			stab_fleeing = true;
		}
	}

	/* Disturb the monster */
	m_ptr->csleep = 0;


	/* Extract monster name (or "it") */
	char m_name[80];
	monster_desc(m_name, m_ptr, 0);

	/* Auto-Recall if possible and visible */
	if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

	/* Track a new monster */
	if (m_ptr->ml) health_track(c_ptr->m_idx);

	/* Stop if friendly */
	if ((is_friend(m_ptr) >= 0) &&
	                !(p_ptr->stun || p_ptr->confused || p_ptr->image ||
	                  !(m_ptr->ml)))
	{
		// Only 'Stormbringer' can hit friendlies unless player forces attack.
		if (p_ptr->inventory[INVEN_WIELD].artifact_name == "'Stormbringer'")
		{
			msg_format("Your black blade greedily attacks %s!", m_name);
		}
		else
		{
			msg_format("You stop to avoid hitting %s.", m_name);
			return;
		}
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

	/* Handle player fear */
	if (p_ptr->afraid)
	{
		/* Message */
		if (m_ptr->ml)
			msg_format("You are too afraid to attack %s!", m_name);
		else
			msg_format("There is something scary in your way!");

		/* Done */
		return;
	}

	/* Monsters can use barehanded combat, but not weapon combat */
	if ((p_ptr->body_monster) &&
	                (!r_info[p_ptr->body_monster].body_parts[BODY_WEAPON]) &&
	                !(p_ptr->melee_style == SKILL_HAND))
	{
		incarnate_monster_attack(c_ptr->m_idx, y, x);
	}
	/* Otherwise use your weapon(s) */
	else
	{
		int weapons;
		if (p_ptr->melee_style == SKILL_MASTERY)
			weapons = r_info[p_ptr->body_monster].body_parts[BODY_WEAPON];
		else /* SKILL_HAND */
			weapons = 1;

		/* Attack with ALL the weapons !!!!! -- ooh that's gonna hurt YOU */
		for (weap = 0; weap < weapons; ++weap)
		{
			/* Monster is already dead ? oh :( */
			if (mdeath) break;

			/* Reset the blows counter */
			num = 0;

			/* Access the weapon */
			object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD + weap];

			/* Get race info */
			auto r_ptr = m_ptr->race();

			/* Calculate the "attack quality" */
			bonus = p_ptr->to_h + p_ptr->to_h_melee + o_ptr->to_h;
			chance = p_ptr->skill_thn + (bonus * BTH_PLUS_ADJ);

			auto const flags = object_flags(o_ptr);

			if (!(flags & TR_NEVER_BLOW))
			{
				int num_blow = p_ptr->num_blow;

				/* Restrict to max_blow(if max_blow >= 0) */
				if ((max_blow >= 0) &&
				                (num_blow > max_blow)) num_blow = max_blow;

				/* Attack once for each legal blow */
				while (num++ < num_blow)
				{
					/* Test for hit */
					if (test_hit_norm(chance, m_ptr->ac, m_ptr->ml))
					{
						/* Hack -- bare hands do one damage */
						k = 1;

						/* Select a chaotic effect (50% chance) */
						if ((flags & TR_CHAOTIC) && (rand_int(2) == 0))
						{
							if (randint(5) < 3)
							{
								/* Vampiric (20%) */
								chaos_effect = 1;
							}
							else if (rand_int(250) == 0)
							{
								/* Quake (0.12%) */
								chaos_effect = 2;
							}
							else if (rand_int(10))
							{
								/* Confusion (26.892%) */
								chaos_effect = 3;
							}
							else if (rand_int(2) == 0)
							{
								/* Teleport away (1.494%) */
								chaos_effect = 4;
							}
							else
							{
								/* Polymorph (1.494%) */
								chaos_effect = 5;
							}
						}

						/* Vampiric drain */
						if ((flags & TR_VAMPIRIC) || (chaos_effect == 1))
						{
							if (!
							                ((r_ptr->flags & RF_UNDEAD) ||
							                 (r_ptr->flags & RF_NONLIVING)))
								drain_result = m_ptr->hp;
							else
								drain_result = 0;
						}

						if ((flags & TR_VORPAL) && (randint(6) == 1))
							vorpal_cut = true;
						else
							vorpal_cut = false;

						/* Should we attack with hands or not ? */
						if (p_ptr->melee_style != SKILL_MASTERY)
						{
							py_attack_hand(&k, m_ptr, &special);
						}
						/* Handle normal weapon */
						else if (o_ptr->k_ptr)
						{
							k = damroll(o_ptr->dd, o_ptr->ds);
							k = tot_dam_aux(o_ptr, k, m_ptr, &special);

							if (backstab)
							{
								k += (k *
								      get_skill_scale(SKILL_BACKSTAB,
								                      100)) / 100;
							}
							else if (stab_fleeing)
							{
								k += (k * get_skill_scale(SKILL_BACKSTAB, 70)) /
								     100;
							}

							if ((p_ptr->impact && ((k > 50) || randint(7) == 1))
							                || (chaos_effect == 2))
							{
								do_quake = true;
							}

							k = critical_norm(o_ptr->weight, o_ptr->to_h, k, o_ptr->tval, &done_crit);

							/* Stunning blow */
							if (magik(get_skill(SKILL_STUN)) && (o_ptr->tval == TV_HAFTED) && (o_ptr->weight > 50) && done_crit)
							{
								if (!(r_ptr->spells & SF_BR_SOUN) && !(r_ptr->spells & SF_BR_WALL) && k)
								{
									int tmp;

									/* Get stunned */
									if (m_ptr->stunned)
									{
										msg_format("%^s is more dazed.", m_name);
										tmp = m_ptr->stunned + get_skill_scale(SKILL_STUN, 30) + 10;
									}
									else
									{
										msg_format("%^s is dazed.", m_name);
										tmp = get_skill_scale(SKILL_STUN, 60) + 20;
									}

									/* Apply stun */
									m_ptr->stunned = (tmp < 200) ? tmp : 200;
								}
							}

							if (vorpal_cut)
							{
								int step_k = k;

								msg_format("Your weapon cuts deep into %s!",
								           m_name);
								do
								{
									k += step_k;
								}
								while (randint(4) == 1);
							}

							if (praying_to(GOD_TULKAS))
							{
								if (magik(wisdom_scale(130) - m_ptr->level) && (p_ptr->grace > 1000))
								{
									msg_print("You feel the hand of Tulkas helping your blow.");
									k += (o_ptr->to_d + p_ptr->to_d_melee) * 2;
								}
								else k += o_ptr->to_d + p_ptr->to_d_melee;
							}
							else k += o_ptr->to_d;

							/* Project some more nasty stuff? */
							if (p_ptr->tim_project)
							{
								project(0, p_ptr->tim_project_rad, y, x, p_ptr->tim_project_dam, p_ptr->tim_project_gf, p_ptr->tim_project_flag | PROJECT_JUMP);
								if (!c_ptr->m_idx)
								{
									mdeath = true;
									break;
								}
							}

							do_nazgul(&k, &num, num_blow, weap, r_ptr, o_ptr);

						}

						/* Melkor can cast curse for you*/
						if (praying_to(GOD_MELKOR))
						{
							int lv = get_level_s(MELKOR_CURSE, 100);

							if (lv >= 10)
							{
								int chance = (wisdom_scale(30) * lv) / ((m_ptr->level < 1) ? 1 : m_ptr->level);

								if (chance < 1) chance = 1;
								if ((p_ptr->grace > 5000) && magik(chance))
								{
									do_melkor_curse(c_ptr->m_idx);
								}
							}
						}

						/* May it clone the monster ? */
						if ((flags & TR_CLONE) && magik(30))
						{
							msg_format("Oh no! Your weapon clones %^s!",
							           m_name);
							multiply_monster(c_ptr->m_idx, false, true);
						}

						/* Apply the player damage bonuses */
						k += p_ptr->to_d + p_ptr->to_d_melee;

						/* No negative damage */
						if (k < 0) k = 0;

						/* Message */
						if (!(backstab || stab_fleeing))
						{
							/* These monsters never have flavoured combat msgs */
							if (strchr("vwjmelX,.*", r_ptr->d_char))
							{
								msg_format("You hit %s.", m_name);
							}

							/* Print flavoured messages if requested */
							else
							{
								char buff[255];

								flavored_attack((100 * k) / m_ptr->maxhp, buff);
								msg_format(buff, m_name);
							}
						}
						else if (backstab)
						{
							char buf[80];

							monster_race_desc(buf, m_ptr->r_idx, m_ptr->ego);

							backstab = false;

							msg_format
							("You cruelly stab the helpless, sleeping %s!",
							 buf);
						}
						else
						{
							char buf[80];

							monster_race_desc(buf, m_ptr->r_idx, m_ptr->ego);

							msg_format("You backstab the fleeing %s!", buf);
						}

						/* Complex message */
						if (wizard)
						{
							msg_format("You do %d (out of %d) damage.", k,
							           m_ptr->hp);
						}

						if (special) attack_special(m_ptr, special, k);

						/* Damage, check for fear and death */
						if (mon_take_hit(c_ptr->m_idx, k, &fear, NULL))
						{
							/* Hack -- High-level warriors can spread their attacks out
							 * among weaker foes.
							 */
							if ((p_ptr->has_ability(AB_SPREAD_BLOWS)) && (num < num_blow) &&
							                (energy_use))
							{
								energy_use = energy_use * num / num_blow;
							}
							mdeath = true;
							break;
						}

						switch (is_friend(m_ptr))
						{
						case 1:
							msg_format("%^s gets angry!", m_name);
							change_side(m_ptr);
							break;
						case 0:
							msg_format("%^s gets angry!", m_name);
							m_ptr->status = MSTATUS_NEUTRAL_M;
							break;
						}

						touch_zap_player(m_ptr);

						/* Are we draining it?  A little note: If the monster is
						   dead, the drain does not work... */

						if (drain_result)
						{
							drain_result -= m_ptr->hp; 	/* Calculate the difference */

							if (drain_result > 0)	/* Did we really hurt it? */
							{
								drain_heal = damroll(4, (drain_result / 6));

								if (options->cheat_xtra)
								{
									msg_format("Draining left: %d", drain_left);
								}

								if (drain_left)
								{
									if (drain_heal < drain_left)
									{
										drain_left -= drain_heal;
									}
									else
									{
										drain_heal = drain_left;
										drain_left = 0;
									}

									if (drain_msg)
									{
										msg_format
										("Your weapon drains life from %s!",
										 m_name);
										drain_msg = false;
									}

									hp_player(drain_heal);
									/* We get to keep some of it! */
								}
							}
						}

						/* Confusion attack */
						if ((p_ptr->confusing) || (chaos_effect == 3))
						{
							/* Cancel glowing hands */
							if (p_ptr->confusing)
							{
								p_ptr->confusing = false;
								msg_print("Your hands stop glowing.");
							}

							/* Confuse the monster */
							if (r_ptr->flags & RF_NO_CONF)
							{
								msg_format("%^s is unaffected.", m_name);
							}
							else if (rand_int(100) < m_ptr->level)
							{
								msg_format("%^s is unaffected.", m_name);
							}
							else
							{
								msg_format("%^s appears confused.", m_name);
								m_ptr->confused +=
								        10 + rand_int(get_skill(SKILL_COMBAT)) / 5;
							}
						}

						else if (chaos_effect == 4)
						{
							msg_format("%^s disappears!", m_name);
							teleport_away(c_ptr->m_idx, 50);
							num = num_blow + 1; 	/* Can't hit it anymore! */
						}

						else if ((chaos_effect == 5) && cave_floor_bold(y, x) &&
						                (randint(90) > m_ptr->level))
						{
							if (!((r_ptr->flags & RF_UNIQUE) ||
							                (r_ptr->spells & SF_BR_CHAO) ||
							                (m_ptr->mflag & MFLAG_QUEST)))
							{
								/* Handle polymorph */
								if (do_poly_monster(y, x))
								{
									/* Polymorph succeeded */
									msg_format("%^s changes!", m_name);

									/* Hack -- Get new monster */
									m_ptr = &m_list[c_ptr->m_idx];

									/* Oops, we need a different name... */
									monster_desc(m_name, m_ptr, 0);

									/* Hack -- Get new race */
									r_ptr = m_ptr->race();

									fear = false;
								}
								else
								{
									msg_format("%^s resists.", m_name);
								}
							}
							else
							{
								msg_format("%^s is unaffected.", m_name);
							}
						}
					}

					/* Player misses */
					else
					{
						backstab = false; 	/* Clumsy! */

						/* Message */
						msg_format("You miss %s.", m_name);
					}
				}
			}
			else
			{
				msg_print("You can't attack with that weapon.");
			}
		}
	}

	/* Carried monster can attack too */
	if ((!mdeath) && m_list[c_ptr->m_idx].hp)
	{
		carried_monster_attack(c_ptr->m_idx, y, x);
	}

	/* Hack -- delay fear messages */
	if (fear && m_ptr->ml)
	{
		/* Message */
		msg_format("%^s flees in terror!", m_name);
	}

	/* Mega-Hack -- apply earthquake brand */
	if (do_quake)
	{
		/* Prevent destruction of quest levels and town */
		if (!is_quest(dun_level) && dun_level)
			earthquake(p_ptr->py, p_ptr->px, 10);
	}
}



bool player_can_enter(byte feature)
{
	auto const &r_info = game->edit_data.r_info;
	auto const &f_info = game->edit_data.f_info;

	/* Player can not walk through "walls" unless in Shadow Form */
	auto const pass_wall = (p_ptr->wraith_form || (race_flags_p(PR_SEMI_WRAITH)));

	/* Wall mimicry force the player to stay in walls */
	auto const only_wall = (p_ptr->mimic_extra & CLASS_WALL);

	/* Don't let the player kill himself with one keystroke */
	if (p_ptr->wild_mode)
	{
		if (feature == FEAT_DEEP_WATER)
		{
			int wt = weight_limit() / 2;

			if ((calc_total_weight() >= wt) && !(p_ptr->ffall))
			{
				return false;
			}
		}
		else if (feature == FEAT_SHAL_LAVA ||
		                feature == FEAT_DEEP_LAVA)
		{
			if (!(p_ptr->resist_fire ||
			                p_ptr->immune_fire ||
			                p_ptr->oppose_fire ||
			                p_ptr->ffall))
			{
				return false;
			}
		}
	}

	if (feature == FEAT_TREES)
	{
		if (p_ptr->fly ||
		    pass_wall ||
		    p_ptr->has_ability(AB_TREE_WALK) ||
		    (p_ptr->mimic_form == resolve_mimic_name("Ent")) ||
		    ((p_ptr->grace >= 9000) && praying_to(GOD_YAVANNA)))
		{
			return true;
		}
	}

	if ((p_ptr->climb) && (f_info[feature].flags & FF_CAN_CLIMB))
	{
		return true;
	}
	if ((p_ptr->fly) &&
	                ((f_info[feature].flags & FF_CAN_FLY) ||
	                 (f_info[feature].flags & FF_CAN_LEVITATE)))
	{
		return true;
	}
	else if (only_wall && (f_info[feature].flags & FF_FLOOR))
	{
		return false;
	}
	else if ((p_ptr->ffall) &&
	                (f_info[feature].flags & FF_CAN_LEVITATE))
	{
		return true;
	}
	else if ((pass_wall || only_wall) &&
	                (f_info[feature].flags & FF_CAN_PASS))
	{
		return true;
	}
	else if (f_info[feature].flags & FF_NO_WALK)
	{
		return false;
	}
	else if ((f_info[feature].flags & FF_WEB) &&
			((!(r_info[p_ptr->body_monster].flags & RF_SPIDER)) && (p_ptr->mimic_form != resolve_mimic_name("Spider"))))
	{
		return false;
	}

	return true;
}

/*
 * easy_open_door --
 *
 *	If there is a jammed/closed/locked door at the given location,
 *	then attempt to unlock/open it. Return true if an attempt was
 *	made (successful or not), otherwise return false.
 *
 *	The code here should be nearly identical to that in
 *	do_cmd_open_test() and do_cmd_open_aux().
 */

static bool easy_open_door(int y, int x)
{
	auto const &r_info = game->edit_data.r_info;

	int i, j;

	cave_type *c_ptr = &cave[y][x];

	auto r_ptr = &r_info[p_ptr->body_monster];


	if ((p_ptr->body_monster != 0) && !(r_ptr->flags & RF_OPEN_DOOR))
	{
		msg_print("You cannot open doors.");

		return false;
	}

	/* Must be a closed door */
	if (!((c_ptr->feat >= FEAT_DOOR_HEAD) && (c_ptr->feat <= FEAT_DOOR_TAIL)))
	{
		/* Nope */
		return false;
	}

	/* Jammed door */
	if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x08)
	{
		/* Stuck */
		msg_print("The door appears to be stuck.");
	}

	/* Locked door */
	else if (c_ptr->feat >= FEAT_DOOR_HEAD + 0x01)
	{
		/* Disarm factor */
		i = 100;

		/* Penalize some conditions */
		if (p_ptr->blind || no_lite()) i = i / 10;
		if (p_ptr->confused || p_ptr->image) i = i / 10;

		/* Extract the lock power */
		j = c_ptr->feat - FEAT_DOOR_HEAD;

		/* Extract the difficulty XXX XXX XXX */
		j = i - (j * 4);

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success */
		if (rand_int(100) < j)
		{
			/* Message */
			msg_print("You have picked the lock.");

			/* Open the door */
			cave_set_feat(y, x, FEAT_OPEN);

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);

			/* Experience */
			gain_exp(1);
		}

		/* Failure */
		else
		{
			/* Failure */
			flush_on_failure();

			/* Message */
			msg_print("You failed to pick the lock.");
		}
	}

	/* Closed door */
	else
	{
		/* Open the door */
		cave_set_feat(y, x, FEAT_OPEN);

		/* Update some things */
		p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_MON_LITE);
	}

	/* Result */
	return true;
}

/*
 * Move player in the given direction, with the given "pickup" flag.
 *
 * This routine should (probably) always induce energy expenditure.
 *
 * Note that moving will *always* take a turn, and will *always* hit
 * any monster which might be in the destination grid.  Previously,
 * moving into walls was "free" and did NOT hit invisible monsters.
 */
void move_player_aux(int dir, int do_pickup, int run)
{
	auto const &d_info = game->edit_data.d_info;
	auto const &r_info = game->edit_data.r_info;
	auto const &f_info = game->edit_data.f_info;

	int y, x, tmp;

	cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];

	monster_type *m_ptr;

	auto r_ptr = &r_info[p_ptr->body_monster];

	char m_name[80];

	bool oktomove = true;


	/* Hack - random movement */
	if (p_ptr->disembodied)
		tmp = dir;
	else if ((r_ptr->flags & RF_RAND_25) && (r_ptr->flags & RF_RAND_50))
	{
		if (randint(100) < 75)
			tmp = randint(9);
		else
			tmp = dir;
	}
	else if (r_ptr->flags & RF_RAND_50)
	{
		if (randint(100) < 50)
			tmp = randint(9);
		else
			tmp = dir;
	}
	else if (r_ptr->flags & RF_RAND_25)
	{
		if (randint(100) < 25)
			tmp = randint(9);
		else
			tmp = dir;
	}
	else
	{
		tmp = dir;
	}

	if ((c_ptr->feat == FEAT_ICE) && (!p_ptr->ffall && !p_ptr->fly))
	{
		if (magik(70 - p_ptr->lev))
		{
			tmp = randint(9);
			msg_print("You slip on the icy floor.");
		}
		else
			tmp = dir;
	}

	/* Find the result of moving */
	y = p_ptr->py + ddy[tmp];
	x = p_ptr->px + ddx[tmp];

	/* Examine the destination */
	c_ptr = &cave[y][x];

	/* Change oldpx and oldpy to place the player well when going back to big mode */
	if (p_ptr->wild_mode)
	{
		if (ddy[tmp] > 0) p_ptr->oldpy = 1;
		if (ddy[tmp] < 0) p_ptr->oldpy = MAX_HGT - 2;
		if (ddy[tmp] == 0) p_ptr->oldpy = MAX_HGT / 2;
		if (ddx[tmp] > 0) p_ptr->oldpx = 1;
		if (ddx[tmp] < 0) p_ptr->oldpx = MAX_WID - 2;
		if (ddx[tmp] == 0) p_ptr->oldpx = MAX_WID / 2;
	}

	/* Exit the area */
	if (!dun_level && !p_ptr->wild_mode && !is_quest(dun_level) &&
	                ((x == 0) || (x == cur_wid - 1) || (y == 0) || (y == cur_hgt - 1)))
	{
		/* Can the player enter the grid? */
		if (player_can_enter(c_ptr->mimic))
		{
			/* Hack: move to new area */
			if ((y == 0) && (x == 0))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = cur_wid - 2;
				ambush_flag = false;
			}

			else if ((y == 0) && (x == MAX_WID - 1))
			{
				p_ptr->wilderness_y--;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = 1;
				ambush_flag = false;
			}

			else if ((y == MAX_HGT - 1) && (x == 0))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x--;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = cur_wid - 2;
				ambush_flag = false;
			}

			else if ((y == MAX_HGT - 1) && (x == MAX_WID - 1))
			{
				p_ptr->wilderness_y++;
				p_ptr->wilderness_x++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = 1;
				ambush_flag = false;
			}

			else if (y == 0)
			{
				p_ptr->wilderness_y--;
				p_ptr->oldpy = cur_hgt - 2;
				p_ptr->oldpx = x;
				ambush_flag = false;
			}

			else if (y == cur_hgt - 1)
			{
				p_ptr->wilderness_y++;
				p_ptr->oldpy = 1;
				p_ptr->oldpx = x;
				ambush_flag = false;
			}

			else if (x == 0)
			{
				p_ptr->wilderness_x--;
				p_ptr->oldpx = cur_wid - 2;
				p_ptr->oldpy = y;
				ambush_flag = false;
			}

			else if (x == cur_wid - 1)
			{
				p_ptr->wilderness_x++;
				p_ptr->oldpx = 1;
				p_ptr->oldpy = y;
				ambush_flag = false;
			}

			p_ptr->leaving = true;

			return;
		}
	}

	/* Some hooks */
	{
		hook_move_in in = { y, x };
		if (process_hooks_new(HOOK_MOVE, &in, NULL)) {
			return; /* Prevent movement */
		}
	}

	if (p_ptr->dripping_tread > 0)
	{
		geomancy_random_floor(y, x, false);
		p_ptr->dripping_tread -= 1;
		if (p_ptr->dripping_tread == 0)
		{
			msg_print("You stop dripping raw elemental energies.");
		}
	}


	/* Get the monster */
	m_ptr = &m_list[c_ptr->m_idx];
	auto const mr_ptr = m_ptr->race();

	/* Hack -- attack monsters */
	if (c_ptr->m_idx && (m_ptr->ml || player_can_enter(c_ptr->feat)))
	{

		/* Attack -- only if we can see it OR it is not in a wall */
		if ((is_friend(m_ptr) > 0) &&
				!(p_ptr->confused || p_ptr->image || !(m_ptr->ml) || p_ptr->stun) &&
		                ((player_can_enter(cave[y][x].feat))))
		{
			m_ptr->csleep = 0;

			/* Extract monster name (or "it") */
			monster_desc(m_name, m_ptr, 0);

			/* Auto-Recall if possible and visible */
			if (m_ptr->ml) monster_race_track(m_ptr->r_idx, m_ptr->ego);

			/* Track a new monster */
			if (m_ptr->ml) health_track(c_ptr->m_idx);

			/* Is it Stormbringer? */
			bool stormbringer = p_ptr->inventory[INVEN_WIELD].artifact_name == "'Stormbringer'";

			/* displace? */
			if (stormbringer && (randint(1000) > 666))
			{
				py_attack(y, x, -1);
			}
			else if (cave_floor_bold(p_ptr->py, p_ptr->px) ||
			                (mr_ptr->flags & RF_PASS_WALL))
			{
				msg_format("You push past %s.", m_name);
				m_ptr->fy = p_ptr->py;
				m_ptr->fx = p_ptr->px;
				cave[p_ptr->py][p_ptr->px].m_idx = c_ptr->m_idx;
				c_ptr->m_idx = 0;
				update_mon(cave[p_ptr->py][p_ptr->px].m_idx, true);
			}
			else
			{
				msg_format("%^s is in your way!", m_name);
				energy_use = 0;
				oktomove = false;
			}

			/* now continue on to 'movement' */
		}
		else
		{
			py_attack(y, x, -1);
			oktomove = false;
		}
	}

	else if ((c_ptr->feat == FEAT_DARK_PIT) && !p_ptr->ffall)
	{
		msg_print("You can't cross the chasm.");
		running = 0;
		oktomove = false;
	}

	/* Player can't enter ? soo bad for him/her ... */
	else if (!player_can_enter(c_ptr->feat))
	{
		oktomove = false;

		/* Disturb the player */
		disturb();

		if (p_ptr->prob_travel)
		{
			if (passwall(tmp, true)) return;
		}

		/* Notice things in the dark */
		if (!(c_ptr->info & (CAVE_MARK)) && !(c_ptr->info & (CAVE_SEEN)))
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
				msg_print("You feel some rubble blocking your way.");
				c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}

			/* Closed door */
			else if (c_ptr->feat < FEAT_SECRET)
			{
				msg_print("You feel a closed door blocking your way.");
				c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}

			/* Wall (or secret door) */
			else
			{
				int const feat = c_ptr->mimic
					? c_ptr->mimic
					: f_info[c_ptr->feat].mimic
					;

				msg_format("You feel %s.", f_info[feat].block);
				c_ptr->info |= (CAVE_MARK);
				lite_spot(y, x);
			}
		}

		/* Notice things */
		else
		{
			/* Rubble */
			if (c_ptr->feat == FEAT_RUBBLE)
			{
				msg_print("There is rubble blocking your way.");

				if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
					energy_use = 0;
				/*
				 * Well, it makes sense that you lose time bumping into
				 * a wall _if_ you are confused, stunned or blind; but
				 * typing mistakes should not cost you a turn...
				 */
			}
			/* Closed doors */
			else if ((c_ptr->feat >= FEAT_DOOR_HEAD) && (c_ptr->feat <= FEAT_DOOR_TAIL))
			{
				if (easy_open_door(y, x)) return;
			}

			/* Wall (or secret door) */
			else
			{
				int feat;

				if (c_ptr->mimic) feat = c_ptr->mimic;
				else
					feat = f_info[c_ptr->feat].mimic;

				msg_format("There is %s.", f_info[feat].block);

				if (!(p_ptr->confused || p_ptr->stun || p_ptr->image))
					energy_use = 0;
			}
		}
	}

	/* Normal movement */
	if (oktomove)
	{
		int oy, ox;
		int feat;

		/* Rooted means no move */
		if (p_ptr->tim_roots) return;

		/* Save old location */
		oy = p_ptr->py;
		ox = p_ptr->px;

		/* Move the player */
		p_ptr->py = y;
		p_ptr->px = x;

		if (cave[p_ptr->py][p_ptr->px].mimic) feat = cave[p_ptr->py][p_ptr->px].mimic;
		else
			feat = cave[p_ptr->py][p_ptr->px].feat;

		/* Redraw new spot */
		lite_spot(p_ptr->py, p_ptr->px);

		/* Redraw old spot */
		lite_spot(oy, ox);

		/* Sound */
		/* sound(SOUND_WALK); */

		/* Check for new panel (redraw map) */
		verify_panel();

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		if (!run) p_ptr->window |= (PW_OVERHEAD);

		/* Some feature descs */
		if (f_info[cave[p_ptr->py][p_ptr->px].feat].text != DEFAULT_FEAT_TEXT)
		{
			/* Mega-hack for dungeon branches */
			if ((feat == FEAT_MORE) && c_ptr->special)
			{
				msg_format("There is %s", d_info[c_ptr->special].text.c_str());
			}
			else
			{
				msg_print(f_info[feat].text);
			}

			/* Flush message while running */
			if (running) msg_print(NULL);
		}

		/* Handle "objects" */
		carry(do_pickup);

		/* Handle "store doors" */
		if (c_ptr->feat == FEAT_SHOP)
		{
			/* Disturb */
			disturb();

			/* Hack -- Enter store */
			command_new = '_';
		}

		else if (cave[y][x].feat >= FEAT_ALTAR_HEAD &&
		                cave[y][x].feat <= FEAT_ALTAR_TAIL)
		{
			auto msg = fmt::format(
				"You see {}.",
				singular_prefix(f_info[cave[y][x].feat].name));

			msg_print(msg);

			/* Flush message while running */
			if (running)
			{
				msg_print(nullptr);
			}
		}

		/* Execute the inscription */
		else if (c_ptr->inscription)
		{
			/* Disturb */
			disturb();

			msg_format("There is an inscription here: %s",
			           inscription_info[c_ptr->inscription].text);
			if (inscription_info[c_ptr->inscription].when & INSCRIP_EXEC_WALK)
			{
				execute_inscription(c_ptr->inscription, p_ptr->py, p_ptr->px);
			}
		}
	}

	/* Update wilderness knowledge */
	if (p_ptr->wild_mode)
	{
		if (wizard) msg_format("y:%d, x:%d", p_ptr->py, p_ptr->px);

		/* Update the known wilderness */
		reveal_wilderness_around_player(p_ptr->py, p_ptr->px, 0, WILDERNESS_SEE_RADIUS);

		/* Walking the wild isnt meaningfull */
		p_ptr->did_nothing = true;
	}
}

void move_player(int dir, int do_pickup)
{
	move_player_aux(dir, do_pickup, 0);
}


/*
 * Hack -- Grid-based version of see_obstacle
 */
static int see_obstacle_grid(cave_type *c_ptr)
{
	auto const &f_info = game->edit_data.f_info;

	/* Hack -- Handle special cases XXX XXX */
	switch (c_ptr->feat)
	{
		/* Require levitation */
	case FEAT_DARK_PIT:
	case FEAT_DEEP_WATER:
	case FEAT_ICE:
		{
			if (p_ptr->ffall || p_ptr->fly) return false;
		}

		/* Require immunity */
	case FEAT_DEEP_LAVA:
	case FEAT_SHAL_LAVA:
		{
			if (p_ptr->invuln || p_ptr->immune_fire) return false;
		}
	}


	/* "Safe" floor grids aren't obstacles */
	if (f_info[c_ptr->feat].flags & FF_CAN_RUN) return false;

	/* Must be known to the player */
	if (!(c_ptr->info & (CAVE_MARK))) return false;

	/* Default */
	return true;
}


/*
 * Hack -- Check for a "known wall" or "dangerous" feature (see below)
 */
static int see_obstacle(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are not known walls */
	if (!in_bounds2(y, x)) return false;

	/* Analyse the grid */
	return (see_obstacle_grid(&cave[y][x]));
}


/*
 * Hack -- Check for an "unknown corner" (see below)
 */
static int see_nothing(int dir, int y, int x)
{
	/* Get the new location */
	y += ddy[dir];
	x += ddx[dir];

	/* Illegal grids are unknown */
	if (!in_bounds2(y, x)) return true;

	/* Memorized grids are always known */
	if (cave[y][x].info & (CAVE_MARK)) return false;

	/* Non-floor grids are unknown */
	if (!cave_floor_bold(y, x)) return true;

	/* Viewable door/wall grids are known */
	if (player_can_see_bold(y, x)) return false;

	/* Default */
	return true;
}





/*
 * The running algorithm:                       -CJS-
 *
 * In the diagrams below, the player has just arrived in the
 * grid marked as '@', and he has just come from a grid marked
 * as 'o', and he is about to enter the grid marked as 'x'.
 *
 * Of course, if the "requested" move was impossible, then you
 * will of course be blocked, and will stop.
 *
 * Overview: You keep moving until something interesting happens.
 * If you are in an enclosed space, you follow corners. This is
 * the usual corridor scheme. If you are in an open space, you go
 * straight, but stop before entering enclosed space. This is
 * analogous to reaching doorways. If you have enclosed space on
 * one side only (that is, running along side a wall) stop if
 * your wall opens out, or your open space closes in. Either case
 * corresponds to a doorway.
 *
 * What happens depends on what you can really SEE. (i.e. if you
 * have no light, then running along a dark corridor is JUST like
 * running in a dark room.) The algorithm works equally well in
 * corridors, rooms, mine tailings, earthquake rubble, etc, etc.
 *
 * These conditions are kept in static memory:
 * find_openarea         You are in the open on at least one
 * side.
 * find_breakleft        You have a wall on the left, and will
 * stop if it opens
 * find_breakright       You have a wall on the right, and will
 * stop if it opens
 *
 * To initialize these conditions, we examine the grids adjacent
 * to the grid marked 'x', two on each side (marked 'L' and 'R').
 * If either one of the two grids on a given side is seen to be
 * closed, then that side is considered to be closed. If both
 * sides are closed, then it is an enclosed (corridor) run.
 *
 * LL           L
 * @x          LxR
 * RR          @R
 *
 * Looking at more than just the immediate squares is
 * significant. Consider the following case. A run along the
 * corridor will stop just before entering the center point,
 * because a choice is clearly established. Running in any of
 * three available directions will be defined as a corridor run.
 * Note that a minor hack is inserted to make the angled corridor
 * entry (with one side blocked near and the other side blocked
 * further away from the runner) work correctly. The runner moves
 * diagonally, but then saves the previous direction as being
 * straight into the gap. Otherwise, the tail end of the other
 * entry would be perceived as an alternative on the next move.
 *
 * #.#
 * ##.##
 * .@x..
 * ##.##
 * #.#
 *
 * Likewise, a run along a wall, and then into a doorway (two
 * runs) will work correctly. A single run rightwards from @ will
 * stop at 1. Another run right and down will enter the corridor
 * and make the corner, stopping at the 2.
 *
 * #@x    1
 * ########### ######
 * 2        #
 * #############
 * #
 *
 * After any move, the function area_affect is called to
 * determine the new surroundings, and the direction of
 * subsequent moves. It examines the current player location
 * (at which the runner has just arrived) and the previous
 * direction (from which the runner is considered to have come).
 *
 * Moving one square in some direction places you adjacent to
 * three or five new squares (for straight and diagonal moves
 * respectively) to which you were not previously adjacent,
 * marked as '!' in the diagrams below.
 *
 * ...!   ...
 * .o@!   .o.!
 * ...!   ..@!
 * !!!
 *
 * You STOP if any of the new squares are interesting in any way:
 * for example, if they contain visible monsters or treasure.
 *
 * You STOP if any of the newly adjacent squares seem to be open,
 * and you are also looking for a break on that side. (that is,
 * find_openarea AND find_break).
 *
 * You STOP if any of the newly adjacent squares do NOT seem to be
 * open and you are in an open area, and that side was previously
 * entirely open.
 *
 * Corners: If you are not in the open (i.e. you are in a corridor)
 * and there is only one way to go in the new squares, then turn in
 * that direction. If there are more than two new ways to go, STOP.
 * If there are two ways to go, and those ways are separated by a
 * square which does not seem to be open, then STOP.
 *
 * Otherwise, we have a potential corner. There are two new open
 * squares, which are also adjacent. One of the new squares is
 * diagonally located, the other is straight on (as in the diagram).
 * We consider two more squares further out (marked below as ?).
 *
 * We assign "option" to the straight-on grid, and "option2" to the
 * diagonal grid, and "check_dir" to the grid marked 's'.
 *
 * .s
 * @x?
 * #?
 *
 * If they are both seen to be closed, then it is seen that no
 * benefit is gained from moving straight. It is a known corner.
 * To cut the corner, go diagonally, otherwise go straight, but
 * pretend you stepped diagonally into that next location for a
 * full view next time. Conversely, if one of the ? squares is
 * not seen to be closed, then there is a potential choice. We check
 * to see whether it is a potential corner or an intersection/room entrance.
 * If the square two spaces straight ahead, and the space marked with 's'
 * are both blank, then it is a potential corner and enter if find_examine
 * is set, otherwise must stop because it is not a corner.
 */




/*
 * Hack -- allow quick "cycling" through the legal directions
 */
static byte cycle[] = { 1, 2, 3, 6, 9, 8, 7, 4, 1, 2, 3, 6, 9, 8, 7, 4, 1 };

/*
 * Hack -- map each direction into the "middle" of the "cycle[]" array
 */
static byte chome[] = { 0, 8, 9, 10, 7, 0, 11, 6, 5, 4 };

/*
 * The direction we are running
 */
static byte find_current;

/*
 * The direction we came from
 */
static byte find_prevdir;

/*
 * We are looking for open area
 */
static bool find_openarea;

/*
 * We are looking for a break
 */
static bool find_breakright;
static bool find_breakleft;



/*
 * Initialize the running algorithm for a new direction.
 *
 * Diagonal Corridor -- allow diaginal entry into corridors.
 *
 * Blunt Corridor -- If there is a wall two spaces ahead and
 * we seem to be in a corridor, then force a turn into the side
 * corridor, must be moving straight into a corridor here. ???
 *
 * Diagonal Corridor    Blunt Corridor (?)
 *       # #                  #
 *       #x#                 @x#
 *       @p.                  p
 */
static void run_init(int dir)
{
	int row, col, deepleft, deepright;

	int i, shortleft, shortright;


	/* Save the direction */
	find_current = dir;

	/* Assume running straight */
	find_prevdir = dir;

	/* Assume looking for open area */
	find_openarea = true;

	/* Assume not looking for breaks */
	find_breakright = find_breakleft = false;

	/* Assume no nearby walls */
	deepleft = deepright = false;
	shortright = shortleft = false;

	/* Find the destination grid */
	row = p_ptr->py + ddy[dir];
	col = p_ptr->px + ddx[dir];

	/* Extract cycle index */
	i = chome[dir];

	/* Check for walls */
	if (see_obstacle(cycle[i + 1], p_ptr->py, p_ptr->px))
	{
		find_breakleft = true;
		shortleft = true;
	}
	else if (see_obstacle(cycle[i + 1], row, col))
	{
		find_breakleft = true;
		deepleft = true;
	}

	/* Check for walls */
	if (see_obstacle(cycle[i - 1], p_ptr->py, p_ptr->px))
	{
		find_breakright = true;
		shortright = true;
	}
	else if (see_obstacle(cycle[i - 1], row, col))
	{
		find_breakright = true;
		deepright = true;
	}

	/* Looking for a break */
	if (find_breakleft && find_breakright)
	{
		/* Not looking for open area */
		find_openarea = false;

		/* Hack -- allow angled corridor entry */
		if (dir & 0x01)
		{
			if (deepleft && !deepright)
			{
				find_prevdir = cycle[i - 1];
			}
			else if (deepright && !deepleft)
			{
				find_prevdir = cycle[i + 1];
			}
		}

		/* Hack -- allow blunt corridor entry */
		else if (see_obstacle(cycle[i], row, col))
		{
			if (shortleft && !shortright)
			{
				find_prevdir = cycle[i - 2];
			}
			else if (shortright && !shortleft)
			{
				find_prevdir = cycle[i + 2];
			}
		}
	}
}


/*
 * Update the current "run" path
 *
 * Return true if the running should be stopped
 */
static bool run_test()
{
	auto const &f_info = game->edit_data.f_info;

	int prev_dir, new_dir, check_dir = 0;

	int row, col;

	int i, max, inv;

	int option = 0, option2 = 0;

	/* Where we came from */
	prev_dir = find_prevdir;


	/* Range of newly adjacent grids */
	max = (prev_dir & 0x01) + 1;


	/* Look at every newly adjacent square. */
	for (i = -max; i <= max; i++)
	{
		/* New direction */
		new_dir = cycle[chome[prev_dir] + i];

		/* New location */
		row = p_ptr->py + ddy[new_dir];
		col = p_ptr->px + ddx[new_dir];

		/* Access grid */
		cave_type *c_ptr = &cave[row][col];


		/* Visible monsters abort running */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];

			/* Visible monster */
			if (m_ptr->ml) return true;
		}

		/* Visible objects abort running */
		for (auto const o_idx: c_ptr->o_idxs)
		{
			/* Acquire object */
			object_type * o_ptr = &o_list[o_idx];

			/* Visible object */
			if (o_ptr->marked) return true;
		}


		/* Assume unknown */
		inv = true;

		/* Check memorized grids */
		if (c_ptr->info & (CAVE_MARK))
		{
			bool notice = true;

			/*
			 * Examine the terrain -- conditional disturbance
			 * If we had more flags, we could make these customisable too
			 */
			switch (c_ptr->feat)
			{
			case FEAT_DEEP_LAVA:
			case FEAT_SHAL_LAVA:
				{
					/* Ignore */
					if (p_ptr->invuln || p_ptr->immune_fire) notice = false;

					/* Done */
					break;
				}

			case FEAT_DEEP_WATER:
			case FEAT_ICE:
				{
					/* Ignore */
					if (p_ptr->ffall || p_ptr->fly) notice = false;

					/* Done */
					break;
				}

				/* Open doors */
			case FEAT_OPEN:
			case FEAT_BROKEN:
				{
					/* Option -- ignore */
				        if (options->find_ignore_doors) notice = false;

					/* Done */
					break;
				}

				/*
				 * Stairs - too many of them, should find better ways to
				 * handle them (not scripting!, because it can be called
				 * from within the running algo) XXX XXX XXX
				 */
			case FEAT_LESS:
			case FEAT_MORE:
			case FEAT_QUEST_ENTER:
			case FEAT_QUEST_EXIT:
			case FEAT_QUEST_DOWN:
			case FEAT_QUEST_UP:
			case FEAT_SHAFT_UP:
			case FEAT_SHAFT_DOWN:
			case FEAT_WAY_LESS:
			case FEAT_WAY_MORE:
				/* XXX */
			case FEAT_BETWEEN:
			case FEAT_BETWEEN2:
				{
					/* Option -- ignore */
				        if (options->find_ignore_stairs) notice = false;

					/* Done */
					break;
				}
			}

			/* Check the "don't notice running" flag */
			if (f_info[c_ptr->feat].flags & FF_DONT_NOTICE_RUNNING)
			{
				notice = false;
			}

			/* Interesting feature */
			if (notice) return true;

			/* The grid is "visible" */
			inv = false;
		}

		/* Analyze unknown grids and floors */
		if (inv || cave_floor_bold(row, col))
		{
			/* Looking for open area */
			if (find_openarea)
			{
				/* Nothing */
			}

			/* The first new direction. */
			else if (!option)
			{
				option = new_dir;
			}

			/* Three new directions. Stop running. */
			else if (option2)
			{
				return true;
			}

			/* Two non-adjacent new directions.  Stop running. */
			else if (option != cycle[chome[prev_dir] + i - 1])
			{
				return true;
			}

			/* Two new (adjacent) directions (case 1) */
			else if (new_dir & 0x01)
			{
				check_dir = cycle[chome[prev_dir] + i - 2];
				option2 = new_dir;
			}

			/* Two new (adjacent) directions (case 2) */
			else
			{
				check_dir = cycle[chome[prev_dir] + i + 1];
				option2 = option;
				option = new_dir;
			}
		}

		/* Obstacle, while looking for open area */
		else
		{
			if (find_openarea)
			{
				if (i < 0)
				{
					/* Break to the right */
					find_breakright = true;
				}

				else if (i > 0)
				{
					/* Break to the left */
					find_breakleft = true;
				}
			}
		}
	}


	/* Looking for open area */
	if (find_openarea)
	{
		/* Hack -- look again */
		for (i = -max; i < 0; i++)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = p_ptr->py + ddy[new_dir];
			col = p_ptr->px + ddx[new_dir];

			/* Access grid */
			cave_type *c_ptr = &cave[row][col];

			/* Unknown grids or non-obstacle */
			if (!see_obstacle_grid(c_ptr))
			{
				/* Looking to break right */
				if (find_breakright)
				{
					return true;
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break left */
				if (find_breakleft)
				{
					return true;
				}
			}
		}

		/* Hack -- look again */
		for (i = max; i > 0; i--)
		{
			new_dir = cycle[chome[prev_dir] + i];

			row = p_ptr->py + ddy[new_dir];
			col = p_ptr->px + ddx[new_dir];

			/* Access grid */
			cave_type *c_ptr = &cave[row][col];

			/* Unknown grid or non-obstacle */
			if (!see_obstacle_grid(c_ptr))
			{
				/* Looking to break left */
				if (find_breakleft)
				{
					return true;
				}
			}

			/* Obstacle */
			else
			{
				/* Looking to break right */
				if (find_breakright)
				{
					return true;
				}
			}
		}
	}


	/* Not looking for open area */
	else
	{
		/* No options */
		if (!option)
		{
			return true;
		}

		/* One option */
		else if (!option2)
		{
			/* Primary option */
			find_current = option;

			/* No other options */
			find_prevdir = option;
		}

		/* Two options, examining corners */
		else if (options->find_examine && !options->find_cut)
		{
			/* Primary option */
			find_current = option;

			/* Hack -- allow curving */
			find_prevdir = option2;
		}

		/* Two options, pick one */
		else
		{
			/* Get next location */
			row = p_ptr->py + ddy[option];
			col = p_ptr->px + ddx[option];

			/* Don't see that it is closed off. */
			/* This could be a potential corner or an intersection. */
			if (!see_obstacle(option, row, col) || !see_obstacle(check_dir, row, col))
			{
				/* Can not see anything ahead and in the direction we */
				/* are turning, assume that it is a potential corner. */
				if (options->find_examine &&
				                see_nothing(option, row, col) &&
				                see_nothing(option2, row, col))
				{
					find_current = option;
					find_prevdir = option2;
				}

				/* STOP: we are next to an intersection or a room */
				else
				{
					return true;
				}
			}

			/* This corner is seen to be enclosed; we cut the corner. */
			else if (options->find_cut)
			{
				find_current = option2;
				find_prevdir = option2;
			}

			/* This corner is seen to be enclosed, and we */
			/* deliberately go the long way. */
			else
			{
				find_current = option;
				find_prevdir = option2;
			}
		}
	}


	/* About to hit a known wall, stop */
	if (see_obstacle(find_current, p_ptr->py, p_ptr->px))
	{
		return true;
	}


	/* Failure */
	return false;
}



/*
 * Take one step along the current "run" path
 */
void run_step(int dir)
{
	/* Start running */
	if (dir)
	{
		/* Hack -- do not start silly run */
		if (see_obstacle(dir, p_ptr->py, p_ptr->px) &&
		                (cave[p_ptr->py + ddy[dir]][p_ptr->px + ddx[dir]].feat != FEAT_TREES))
		{
			/* Message */
			msg_print("You cannot run in that direction.");

			/* Disturb */
			disturb();

			/* Done */
			return;
		}

		/* Calculate torch radius */
		p_ptr->update |= (PU_TORCH);

		/* Initialize */
		run_init(dir);
	}

	/* Keep running */
	else
	{
		/* Update run */
		if (run_test())
		{
			/* Disturb */
			disturb();

			/* Done */
			return;
		}
	}

	/* Decrease the run counter */
	if (--running <= 0) return;

	/* Take time */
	energy_use = 100;


	/* Move the player, using the "pickup" flag */
	move_player_aux(find_current, options->always_pickup, 1);
}


/*
 * Issue a pet command
 */
void do_cmd_pet()
{
	auto const &r_info = game->edit_data.r_info;

	int i = 0;

	int num = 0;

	int powers[36];

	char power_desc[36][80];

	bool flag;

	int ask;

	char choice;

	char out_val[160];

	int pets = 0, pet_ctr = 0;

	bool all_pets = false;

	monster_type *m_ptr;


	for (num = 0; num < 36; num++)
	{
		powers[num] = 0;
		strcpy(power_desc[num], "");
	}

	num = 0;

	if (p_ptr->confused)
	{
		msg_print("You are too confused to command your pets.");
		energy_use = 0;
		return;
	}

	/* Calculate pets */
	/* Process the monsters (backwards) */
	for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
	{
		/* Access the monster */
		m_ptr = &m_list[pet_ctr];

		if (m_ptr->status >= MSTATUS_FRIEND) pets++;
	}

	if (pets == 0)
	{
		msg_print("You have no pets/companions.");
		energy_use = 0;
		return;
	}
	else
	{
		strcpy(power_desc[num], "dismiss pets");
		powers[num++] = 1;
		strcpy(power_desc[num], "dismiss companions");
		powers[num++] = 10;
		strcpy(power_desc[num], "call pets");
		powers[num++] = 2;
		strcpy(power_desc[num], "follow me");
		powers[num++] = 6;
		strcpy(power_desc[num], "seek and destroy");
		powers[num++] = 3;
		if (p_ptr->pet_open_doors)
			strcpy(power_desc[num], "disallow open doors");
		else
			strcpy(power_desc[num], "allow open doors");
		powers[num++] = 4;
		if (p_ptr->pet_pickup_items)
			strcpy(power_desc[num], "disallow pickup items");
		else
			strcpy(power_desc[num], "allow pickup items");
		powers[num++] = 5;
		strcpy(power_desc[num], "give target to a friend");
		powers[num++] = 7;
		strcpy(power_desc[num], "give target to all friends");
		powers[num++] = 8;
		strcpy(power_desc[num], "friend forget target");
		powers[num++] = 9;
	}

	/* Nothing chosen yet */
	flag = false;

	/* Build a prompt (accept all spells) */
	if (num <= 26)
	{
		/* Build a prompt (accept all spells) */
		strnfmt(out_val, 78,
			"(Command %c-%c, ESC=exit) Select a command: ", I2A(0),
		        I2A(num - 1));
	}
	else
	{
		strnfmt(out_val, 78,
			"(Command %c-%c, ESC=exit) Select a command: ", I2A(0),
		        '0' + num - 27);
	}

	/* Save the screen */
	character_icky = true;
	Term_save();

	/* Show the list */
	{
		byte y = 1, x = 0;
		int ctr = 0;
		char dummy[80];

		strcpy(dummy, "");

		prt("", y++, x);

		while (ctr < num)
		{
			strnfmt(dummy, 80, "%c) %s", I2A(ctr), power_desc[ctr]);
			prt(dummy, y + ctr, x);
			ctr++;
		}

		if (ctr < 17)
		{
			prt("", y + ctr, x);
		}
		else
		{
			prt("", y + 17, x);
		}
	}

	/* Get a command from the user */
	while (!flag && get_com(out_val, &choice))
	{
		if (choice == '\r' && num == 1)
		{
			choice = 'a';
		}

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
			ask = false; 		/* Can't uppercase digits */

			i = choice - '0' + 26;
		}

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Verify it */
		if (ask)
		{
			char tmp_val[160];

			/* Prompt */
			strnfmt(tmp_val, 78, "Use %s? ", power_desc[i]);

			/* Belay that order */
			if (!get_check(tmp_val)) continue;
		}

		/* Stop the loop */
		flag = true;
	}

	/* Restore the screen */
	Term_load();
	character_icky = false;

	/* Abort if needed */
	if (!flag)
	{
		energy_use = 0;
		return;
	}

	switch (powers[i])
	{
		/* forget target */
	case 9:
		{
			monster_type *m_ptr;
			int ii, jj;

			msg_print("Select the friendly monster:");
			if (!tgt_pt(&ii, &jj)) return;

			if (cave[jj][ii].m_idx)
			{
				m_ptr = &m_list[cave[jj][ii].m_idx];

				if (m_ptr->status < MSTATUS_PET)
				{
					msg_print("You cannot give orders to this monster.");
					return;
				}

				m_ptr->target = -1;
			}
			break;
		}
		/* Give target to all */
	case 8:
		{
			monster_type *m_ptr;
			int ii, jj, i;


			msg_print("Select the target monster:");
			if (!tgt_pt(&ii, &jj)) return;

			if (cave[jj][ii].m_idx)
			{
				msg_print("Target selected.");

				for (i = m_max - 1; i >= 1; i--)
				{
					/* Access the monster */
					m_ptr = &m_list[i];

					if (!m_ptr->r_idx) continue;

					if (m_ptr->status < MSTATUS_PET) continue;

					m_ptr->target = cave[jj][ii].m_idx;
				}
			}
			else
			{
				msg_print("This is not a correct target.");
				return;
			}
			break;
		}
	case 1: 				/* Dismiss pets */
		{
			int Dismissed = 0;

			if (get_check("Dismiss all pets? ")) all_pets = true;

			/* Process the monsters (backwards) */
			for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
			{
				/* Access the monster */
				m_ptr = &m_list[pet_ctr];
				auto r_ptr = &r_info[m_ptr->r_idx];

				if ((!(r_ptr->flags & RF_NO_DEATH)) && ((m_ptr->status == MSTATUS_PET) || (m_ptr->status == MSTATUS_FRIEND)))	/* Get rid of it! */
				{
					bool checked = false;
					char command;
					bool delete_this = false;

					if (all_pets)
					{
						delete_this = true;
					}
					else
					{
						char friend_name[80], check_friend[80];
						monster_desc(friend_name, m_ptr, 0x80);
						strnfmt(check_friend, 80, "Dismiss %s? (Escape to cancel)", friend_name);

						while (!checked)
						{
							if (!get_com(check_friend, &command))
							{
								/* get out of loop */
								checked = true;
								pet_ctr = 0;
							}
							else switch (command)
							{
							case 'Y':
							case 'y':
								delete_this = true;
								checked = true;
								break;
							case 'n':
							case 'N':
								checked = true;
								break;
							default:
								bell();
								break;
							}
						}
					}

					if (delete_this)
					{
						delete_monster_idx(pet_ctr);
						Dismissed++;
					}
				}
			}

			msg_format("You have dismissed %d pet%s.", Dismissed,
			           (Dismissed == 1 ? "" : "s"));
			break;
		}
	case 10: 				/* Dismiss companions */
		{
			int Dismissed = 0;

			if (get_check("Dismiss all companions? ")) all_pets = true;

			/* Process the monsters (backwards) */
			for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
			{
				/* Access the monster */
				m_ptr = &m_list[pet_ctr];
				auto r_ptr = &r_info[m_ptr->r_idx];

				if ((!(r_ptr->flags & RF_NO_DEATH)) && ((m_ptr->status == MSTATUS_COMPANION)))	/* Get rid of it! */
				{
					bool delete_this = false;

					if (all_pets)
						delete_this = true;
					else
					{
						char friend_name[80], check_friend[80];
						monster_desc(friend_name, m_ptr, 0x80);
						strnfmt(check_friend, 80, "Dismiss %s? ", friend_name);

						if (get_check(check_friend))
							delete_this = true;
					}

					if (delete_this)
					{
						delete_monster_idx(pet_ctr);
						Dismissed++;
					}
				}
			}

			msg_format("You have dismissed %d companion%s.", Dismissed,
			           (Dismissed == 1 ? "" : "s"));
			break;
		}
		/* Call pets */
	case 2:
		{
			p_ptr->pet_follow_distance = 1;
			break;
		}
		/* "Seek and destroy" */
	case 3:
		{
			p_ptr->pet_follow_distance = 255;
			break;
		}
		/* flag - allow pets to open doors */
	case 4:
		{
			p_ptr->pet_open_doors = !p_ptr->pet_open_doors;
			break;
		}
		/* flag - allow pets to pickup items */
	case 5:
		{
			p_ptr->pet_pickup_items = !p_ptr->pet_pickup_items;

			/* Drop objects being carried by pets */
			if (!p_ptr->pet_pickup_items)
			{
				for (pet_ctr = m_max - 1; pet_ctr >= 1; pet_ctr--)
				{
					/* Access the monster */
					m_ptr = &m_list[pet_ctr];

					if (m_ptr->status >= MSTATUS_PET)
					{
						monster_drop_carried_objects(m_ptr);
					}
				}
			}

			break;
		}
		/* "Follow Me" */
	case 6:
		{
			p_ptr->pet_follow_distance = 6;
			break;
		}
	}
}

/*
 * Incarnate into a body
 */
void do_cmd_integrate_body()
{
	if (!p_ptr->disembodied)
	{
		msg_print("You are already in a body.");
		return;
	}

	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Incarnate in which body? ",
		      "You have no corpse to incarnate in.",
		      (USE_FLOOR),
		      object_filter::TVal(TV_CORPSE)))
	{
		return;
	}

	object_type *o_ptr = &o_list[0 - item];

	if (o_ptr->sval != SV_CORPSE_CORPSE)
	{
		msg_print("You must select a corpse.");
		return;
	}

	p_ptr->body_monster = o_ptr->pval2;
	p_ptr->chp = o_ptr->pval3;

	floor_item_increase(0 - item, -1);
	floor_item_describe(0 - item);
	floor_item_optimize(0 - item);

	msg_print("Your spirit is incarnated in your new body.");
	p_ptr->wraith_form = false;
	p_ptr->disembodied = false;
	do_cmd_redraw();
}

/*
 * Leave a body
 */
void do_cmd_leave_body(bool drop_body)
{
	auto const &r_info = game->edit_data.r_info;

	if (p_ptr->disembodied)
	{
		msg_print("You are already disembodied.");
		return;
	}

	for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		if (p_ptr->body_parts[i - INVEN_WIELD] &&
			p_ptr->inventory[i].k_ptr &&
			cursed_p(&p_ptr->inventory[i]))
		{
			msg_print("A cursed object is preventing you from leaving your body.");
			return;
		}
	}

	if (drop_body)
	{
		if (magik(25 + get_skill_scale(SKILL_POSSESSION, 25) + get_skill(SKILL_PRESERVATION)))
		{
			auto r_ptr = &r_info[p_ptr->body_monster];

			object_type forge;
			auto o_ptr = &forge;
			object_prep(o_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_CORPSE));
			o_ptr->number = 1;
			o_ptr->pval = 0;
			o_ptr->pval2 = p_ptr->body_monster;
			o_ptr->pval3 = p_ptr->chp;
			o_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 10) + 1;
			object_aware(o_ptr);
			object_known(o_ptr);

			/* Unique corpses are unique */
			if (r_ptr->flags & RF_UNIQUE)
			{
				o_ptr->name1 = 201;
			}

			drop_near(o_ptr, -1, p_ptr->py, p_ptr->px);
		}
		else
			msg_print
			("You do not manage to keep the corpse from rotting away.");
	}

	msg_print("Your spirit leaves your body.");
	p_ptr->disembodied = true;

	/* Turn into a lost soul(just for the picture) */
	p_ptr->body_monster = test_monster_name("Lost soul");
	do_cmd_redraw();
}


bool execute_inscription(byte i, byte y, byte x)
{
	cave_type *c_ptr = &cave[y][x];


	/* Not enough mana in the current grid */
	if (c_ptr->mana < inscription_info[i].mana)
	{
		return true;
	}

	/* Reduce the grid mana -- note: it can't be restored */
	c_ptr->mana -= inscription_info[i].mana;

	/* Analyse inscription type */
	switch (i)
	{
	case INSCRIP_LIGHT:
		{
			msg_print("The inscription shines in a bright light!");
			lite_room(y, x);

			break;
		}

	case INSCRIP_DARK:
		{
			msg_print("The inscription is enveloped in a dark aura!");
			unlite_room(y, x);

			break;
		}

	case INSCRIP_STORM:
		{
			msg_print("The inscription releases a powerful storm!");
			project(0, 3, y, x, damroll(10, 10),
			        GF_ELEC, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM |
			        PROJECT_KILL | PROJECT_JUMP);

			break;
		}

	case INSCRIP_PROTECTION:
		{
			return false;
		}

	case INSCRIP_DWARF_SUMMON:
		{
			int yy = y, xx = x;

			scatter(&yy, &xx, y, x, 3);
			place_monster_one(yy, xx, test_monster_name("Dwarven Warrior"),
			                  0, false, MSTATUS_FRIEND);

			break;
		}

	case INSCRIP_CHASM:
		{
			int ii = x, ij = y;

			cave_set_feat(ij, ii, FEAT_DARK_PIT);
			msg_print("A chasm appears in the floor!");

			cave_type *c_ptr = &cave[ij][ii];

			if (c_ptr->m_idx)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				auto const r_ptr = m_ptr->race();

				if (r_ptr->flags & RF_CAN_FLY)
				{
					msg_print("The monster simply flies over the chasm.");
				}
				else
				{
					if (!(r_ptr->flags & RF_UNIQUE))
					{
						msg_print("The monster falls in the chasm!");
						delete_monster_idx(c_ptr->m_idx);
					}
				}
			}

			if (!c_ptr->o_idxs.empty())
			{
				/* Copy list of objects since we're going to be manipulating the list */
				auto const object_idxs(c_ptr->o_idxs);

				/* Scan all objects in the grid */
				for (auto const this_o_idx: object_idxs)
				{
					bool plural = false;

					char o_name[80];

					/* Acquire object */
					object_type * o_ptr = &o_list[this_o_idx];

					if (o_ptr->number > 1) plural = true;

					/* Effect "observed" */
					if (o_ptr->marked)
					{
						object_desc(o_name, o_ptr, false, 0);
					}

					/* Artifacts get to resist */
					if (o_ptr->name1)
					{
						/* Observe the resist */
						if (o_ptr->marked)
						{
							msg_format("The %s %s simply fly over the chasm!",
							           o_name, (plural ? "are" : "is"));
						}
					}

					/* Kill it */
					else
					{
						/* Delete the object */
						delete_object_idx(this_o_idx);

						/* Redraw */
						lite_spot(ij, ii);
					}
				}
			}

			break;
		}

	case INSCRIP_BLACK_FIRE:
		{
			msg_print("The inscription releases a blast of hellfire!");
			project(0, 3, y, x, 200,
			        GF_HELL_FIRE, PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM |
			        PROJECT_KILL | PROJECT_JUMP);

			break;
		}
	}

	return true;
}


/*
 * Choose an inscription and engrave it
 */
void do_cmd_engrave()
{
	char buf[41] = "";

	byte i;

	strnfmt(buf, 41, "%s", inscription_info[cave[p_ptr->py][p_ptr->px].inscription].text);

	get_string("Engrave what? ", buf, 40);

	/* Silently do nothing when player his escape or enters an empty string */
	if (!buf[0]) return;

	for (i = 0; i < MAX_INSCRIPTIONS; i++)
	{
		if (equals(inscription_info[i].text, buf))
		{
			if (p_ptr->inscriptions[i])
			{
				cave[p_ptr->py][p_ptr->px].inscription = i;
			}
			else
			{
				msg_print("You can't use this inscription for now.");
			}
		}
	}

	/* Execute the inscription */
	if (inscription_info[cave[p_ptr->py][p_ptr->px].inscription].when & INSCRIP_EXEC_ENGRAVE)
	{
		execute_inscription(cave[p_ptr->py][p_ptr->px].inscription, p_ptr->py, p_ptr->px);
	}

	energy_use += 300;
}


/*
 * Let's do a spinning around attack:                   -- DG --
 *     aDb
 *     y@k
 *     ooT
 * Ah ... all of those will get hit.
 */
void do_spin()
{
	int i, j;


	msg_print("You start spinning around...");

	for (j = p_ptr->py - 1; j <= p_ptr->py + 1; j++)
	{
		for (i = p_ptr->px - 1; i <= p_ptr->px + 1; i++)
		{
			/* Avoid stupid bugs */
			if (in_bounds(j, i) && cave[j][i].m_idx)
				py_attack(j, i, 1);
		}
	}
}
