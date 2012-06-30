/* File: cmd6.c */

/* Purpose: Object commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include "spell_type.h"

/*
 * Forward declare
 */
static bool_ activate_spell(object_type * o_ptr, byte choice);


/*
 * General function to find an item by its name
 */
cptr get_item_hook_find_obj_what;
bool_ get_item_hook_find_obj(int *item)
{
	int i;
	char buf[80];
	char buf2[100];

	strcpy(buf, "");
	if (!get_string(get_item_hook_find_obj_what, buf, 79))
		return FALSE;

	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		if (!item_tester_okay(o_ptr)) continue;

		object_desc(buf2, o_ptr, -1, 0);
		if (!strcmp(buf, buf2))
		{
			*item = i;
			return TRUE;
		}
	}
	return FALSE;
}


/*
 * This file includes code for eating food, drinking potions,
 * reading scrolls, aiming wands, using staffs, zapping rods,
 * and activating artifacts.
 *
 * In all cases, if the player becomes "aware" of the item's use
 * by testing it, mark it as "aware" and reward some experience
 * based on the object's level, always rounding up.  If the player
 * remains "unaware", mark that object "kind" as "tried".
 *
 * This code now correctly handles the unstacking of wands, staffs,
 * and rods.  Note the overly paranoid warning about potential pack
 * overflow, which allows the player to use and drop a stacked item.
 *
 * In all "unstacking" scenarios, the "used" object is "carried" as if
 * the player had just picked it up.  In particular, this means that if
 * the use of an item induces pack overflow, that item will be dropped.
 *
 * For simplicity, these routines induce a full "pack reorganization"
 * which not only combines similar items, but also reorganizes various
 * items to obey the current "sorting" method.  This may require about
 * 400 item comparisons, but only occasionally.
 *
 * There may be a BIG problem with any "effect" that can cause "changes"
 * to the p_ptr->inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to "disappear", moving the p_ptr->inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "o_ptr" to
 * no longer point at the correct item, with horrifying results.
 *
 * Note that food/potions/scrolls no longer use bit-flags for effects,
 * but instead use the "sval" (which is also used to sort the objects).
 */


/*
 * Determine the effects of eating a corpse. A corpse can be
 * eaten whole or cut into pieces for later.
 */
static void corpse_effect(object_type *o_ptr, bool_ cutting)
{
	monster_race *r_ptr = &r_info[o_ptr->pval2];

	/* Assume no bad effects */
	bool_ harmful = FALSE;

	byte method, effect, d_dice, d_side;

	int i, dam, idam = 0, mdam, brpow, brdam = 0;


	/* How much of the monster's breath attack remains */
	if (o_ptr->pval <= r_ptr->weight)
	{
		brpow = 0;
	}
	else
	{
		brpow = (o_ptr->pval - r_ptr->weight) / 5;
		if (brpow > (r_ptr->weight / 5)) brpow = r_ptr->weight / 5;
	}

	if (o_ptr->weight <= 0) o_ptr->weight = 1;
	if (o_ptr->pval <= 0) o_ptr->pval = 1;

	/*
	 * The breath is only discharged by accident or by slicing off pieces
	 * of meat, and only by corpses.
	 */
	if ((o_ptr->sval != SV_CORPSE_CORPSE) ||
	                (rand_int(o_ptr->weight / 5) && !cutting)) brpow = 0;

	/* Immediate effects - poison, acid, fire, etc. */
	if (!cutting)
	{
		for (i = 0; i < 4; i++)
		{
			/* skip empty blow slot */
			if (!r_ptr->blow[i].method) continue;

			method = r_ptr->blow[i].method;
			effect = r_ptr->blow[i].effect;
			d_dice = r_ptr->blow[i].d_dice;
			d_side = r_ptr->blow[i].d_side;
			dam = damroll(d_dice, d_side) * o_ptr->pval / o_ptr->weight / 2;
			idam = damroll(d_dice, d_side) *
			       ((o_ptr->weight / o_ptr->pval > 2) ?
			        o_ptr->weight / o_ptr->pval : 2);
			mdam = maxroll(d_dice, d_side) * 2;

			/* Analyse method */
			switch (method)
			{
				/* Methods that are meaningless after death */
			case RBM_BITE:
			case RBM_STING:
			case RBM_ENGULF:
			case RBM_DROOL:
			case RBM_SPIT:
			case RBM_GAZE:
			case RBM_WAIL:
			case RBM_BEG:
			case RBM_INSULT:
			case RBM_MOAN:
				{
					continue;
				}
			}

			/* Analyse effect */
			switch (effect)
			{
				/* Effects that are meaningless after death */
			case RBE_HURT:
			case RBE_UN_BONUS:
			case RBE_UN_POWER:
			case RBE_EAT_GOLD:
			case RBE_EAT_ITEM:
			case RBE_EAT_FOOD:
			case RBE_EAT_LITE:
			case RBE_ELEC:
			case RBE_COLD:
			case RBE_SHATTER:
				{
					break;
				}

			case RBE_POISON:
				{
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						set_poisoned(p_ptr->poisoned + dam + idam + 10);
						harmful = TRUE;
					}

					break;
				}

			case RBE_ACID:
				{
					/* Total Immunity */
					if (!(p_ptr->immune_acid || (dam <= 0)))
					{
						/* Resist the damage */
						if (p_ptr->resist_acid) dam = (dam + 2) / 3;
						if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

						/* Take damage */
						take_hit(dam, "acidic food");
						harmful = TRUE;
					}
					else
					{
						set_oppose_acid(p_ptr->oppose_acid + idam);
					}

					break;
				}

			case RBE_FIRE:
				{
					/* Totally immune */
					if (p_ptr->immune_fire || (dam <= 0))
					{
						/* Resist the damage */
						if (p_ptr->resist_fire) dam = (dam + 2) / 3;
						if (p_ptr->oppose_fire) dam = (dam + 2) / 3;

						/* Take damage */
						take_hit(dam, "a fiery meal");
						harmful = TRUE;
					}
					else
					{
						set_oppose_fire(p_ptr->oppose_fire + idam);
					}

					break;
				}

			case RBE_BLIND:
				{
					if (!p_ptr->resist_blind)
					{
						set_blind(p_ptr->blind + dam * 2 + idam * 2 + 20);
					}

					break;
				}

			case RBE_CONFUSE:
				{
					if (!p_ptr->resist_conf)
					{
						set_confused(p_ptr->confused + dam + idam + 10);
					}
					if (!p_ptr->resist_chaos && rand_int(mdam - dam))
					{
						set_image(p_ptr->image + dam * 10 + idam * 10 + 100);
					}

					break;
				}

			case RBE_HALLU:
				{
					if (!p_ptr->resist_chaos && rand_int(mdam - dam))
					{
						set_image(p_ptr->image + dam * 10 + idam * 10 + 50);
					}

					break;
				}

			case RBE_TERRIFY:
				{
					if (!p_ptr->resist_fear)
					{
						set_afraid(p_ptr->afraid + dam + idam + 10);
					}

					break;
				}

			case RBE_PARALYZE:
				{
					if (!p_ptr->free_act)
					{
						set_paralyzed(p_ptr->paralyzed + dam + idam + 10);
					}

					break;
				}

			case RBE_LOSE_STR:
				{
					do_dec_stat(A_STR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_INT:
				{
					do_dec_stat(A_INT, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_WIS:
				{
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_DEX:
				{
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CON:
				{
					do_dec_stat(A_CON, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CHR:
				{
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);

					break;
				}

				/* Don't eat Morgoth's corpse :) */
			case RBE_LOSE_ALL:
				{
					do_dec_stat(A_STR, STAT_DEC_NORMAL);
					do_dec_stat(A_INT, STAT_DEC_NORMAL);
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);
					do_dec_stat(A_CON, STAT_DEC_NORMAL);
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);
					o_ptr->pval = 1;

					break;
				}

			case RBE_SANITY:
				{
					msg_print("You feel your sanity slipping away!");
					take_sanity_hit(dam, "eating an insane monster");

					break;
				}

				/* Unlife is bad to eat */
			case RBE_EXP_10:
				{
					msg_print("A black aura surrounds the corpse!");

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) +
						         (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d / 10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}

					o_ptr->pval = 1;

					break;
				}

			case RBE_EXP_20:
				{
					msg_print("A black aura surrounds the corpse!");

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) +
						         (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d / 10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}

					o_ptr->pval = 1;

					break;
				}

			case RBE_EXP_40:
				{
					msg_print("A black aura surrounds the corpse!");

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) +
						         (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d / 10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}

					o_ptr->pval = 1;

					break;
				}

			case RBE_EXP_80:
				{
					msg_print("A black aura surrounds the corpse!");

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) +
						         (p_ptr->exp / 100) * MON_DRAIN_LIFE;

						if (p_ptr->hold_life)
						{
							msg_print("You feel your life slipping away!");
							lose_exp(d / 10);
						}
						else
						{
							msg_print("You feel your life draining away!");
							lose_exp(d);
						}
					}

					o_ptr->pval = 1;

					break;
				}
			}
		}
	} /* if (!cutting) */


	/*
	 * The organ that supplies breath attacks is not
	 * immediately emptied upon death, although some types
	 * of breath have no effect.
	 * AMHD's make rather risky meals, and deadly snacks.
	 */

	/* Acid */
	if (r_ptr->flags4 & RF4_BR_ACID && brpow > 0)
	{
		brdam = ((brpow / 3) > 1600 ? 1600 : (brpow / 3));

		msg_print("You are hit by a gush of acid!");

		/* Total Immunity */
		if (!(p_ptr->immune_acid || (brdam <= 0)))
		{
			/* Take damage */
			acid_dam(brdam, "a gush of acid");
			harmful = TRUE;
		}
		o_ptr->pval = 1;
	}
	else if (r_ptr->flags4 & RF4_BR_ACID)
	{
		set_oppose_acid(p_ptr->oppose_acid + rand_int(10) + 10);
	}

	/* Electricity */
	if (r_ptr->flags4 & RF4_BR_ELEC && brpow > 0)
	{
		brdam = ((brpow / 3) > 1600 ? 1600 : (brpow / 3));

		msg_print("You receive a heavy shock!");

		/* Total Immunity */
		if (!(p_ptr->immune_elec || (brdam <= 0)))
		{
			/* Take damage */
			elec_dam(brdam, "an electric shock");
			harmful = TRUE;
		}
		o_ptr->weight = o_ptr->weight - brpow;
		o_ptr->pval = o_ptr->weight;
	}
	else if (r_ptr->flags4 & RF4_BR_ELEC)
	{
		set_oppose_elec(p_ptr->oppose_elec + rand_int(10) + 10);
	}

	/* Fire */
	if (r_ptr->flags4 & RF4_BR_FIRE && brpow > 0)
	{
		brdam = ((brpow / 3) > 1600 ? 1600 : (brpow / 3));

		msg_print("Roaring flames engulf you!");

		/* Total Immunity */
		if (!(p_ptr->immune_fire || (brdam <= 0)))
		{
			/* Take damage */
			fire_dam(brdam, "an explosion");
			harmful = TRUE;
		}
		o_ptr->pval = 1;
	}
	else if (r_ptr->flags4 & RF4_BR_FIRE)
	{
		set_oppose_fire(p_ptr->oppose_fire + rand_int(10) + 10);
	}

	/* Cold */
	if (r_ptr->flags4 & RF4_BR_COLD && brpow > 0)
	{
		brdam = ((brpow / 3) > 1600 ? 1600 : (brpow / 3));

		msg_print("You are caught in a freezing liquid!");

		/* Total Immunity */
		if (!(p_ptr->immune_cold || (brdam <= 0)))
		{
			/* Take damage */
			cold_dam(brdam, "a chilling blast");
			harmful = TRUE;
		}
		o_ptr->weight = o_ptr->weight - brpow;
		o_ptr->pval = o_ptr->weight;
	}
	else if (r_ptr->flags4 & RF4_BR_COLD)
	{
		set_oppose_cold(p_ptr->oppose_cold + rand_int(10) + 10);
	}

	/* Poison */
	if (r_ptr->flags4 & RF4_BR_POIS && brpow > 0)
	{
		brdam = ((brpow / 3) > 800 ? 800 : (brpow / 3));

		msg_print("You are surrounded by toxic gases!");

		/* Resist the damage */
		if (p_ptr->resist_pois) brdam = (brdam + 2) / 3;
		if (p_ptr->oppose_pois) brdam = (brdam + 2) / 3;

		if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
		{
			(void)set_poisoned(p_ptr->poisoned + rand_int(brdam) + 10);
		}

		/* Take damage */
		take_hit(brdam, "toxic gases");
		o_ptr->weight = o_ptr->weight - brpow;
		o_ptr->pval = o_ptr->weight;
		harmful = TRUE;
	}

	/* Nether */
	if (r_ptr->flags4 & RF4_BR_NETH && brpow > 0)
	{
		brdam = ((brpow / 6) > 550 ? 550 : (brpow / 6));

		msg_print("A black aura surrounds the corpse!");

		if (p_ptr->resist_neth)
		{
			brdam *= 6;
			brdam /= (randint(6) + 6);
		}
		else
		{
			if (p_ptr->hold_life && (rand_int(100) < 75))
			{
				msg_print("You keep hold of your life force!");
			}
			else if (p_ptr->hold_life)
			{
				msg_print("You feel your life slipping away!");
				lose_exp(200 + (p_ptr->exp / 1000) * MON_DRAIN_LIFE);
			}
			else
			{
				msg_print("You feel your life draining away!");
				lose_exp(200 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
			}
		}

		/* Take damage */
		take_hit(brdam, "an unholy blast");
		harmful = TRUE;
		o_ptr->weight = o_ptr->weight - brpow;
		o_ptr->pval = o_ptr->weight;
	}

	/* Confusion */
	if (r_ptr->flags4 & RF4_BR_CONF && brpow > 0)
	{
		msg_print("A strange liquid splashes on you!");

		if (!p_ptr->resist_conf)
		{
			set_confused(p_ptr->confused + brdam + idam + 10);
		}
		o_ptr->weight = o_ptr->weight - brpow;
		o_ptr->pval = o_ptr->weight;
	}

	/* Chaos */
	if (r_ptr->flags4 & RF4_BR_CHAO && brpow > 0)
	{
		brdam = ((brpow / 6) > 600 ? 600 : (brpow / 6));

		msg_print("A swirling cloud surrounds you!");

		if (p_ptr->resist_chaos)
		{
			brdam *= 6;
			brdam /= (randint(6) + 6);
		}

		if (!p_ptr->resist_conf)
		{
			(void)set_confused(p_ptr->confused + rand_int(20) + 10);
		}

		if (!p_ptr->resist_chaos)
		{
			(void)set_image(p_ptr->image + randint(10));
		}

		if (!p_ptr->resist_neth && !p_ptr->resist_chaos)
		{
			if (p_ptr->hold_life && (rand_int(100) < 75))
			{
				msg_print("You keep hold of your life force!");
			}
			else if (p_ptr->hold_life)
			{
				msg_print("You feel your life slipping away!");
				lose_exp(500 + (p_ptr->exp / 1000) * MON_DRAIN_LIFE);
			}
			else
			{
				msg_print("You feel your life draining away!");
				lose_exp(5000 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
			}
		}

		/* Take damage */
		take_hit(brdam, "chaotic forces");
		o_ptr->pval = 1;
	}

	/* Disenchantment */
	if (r_ptr->flags4 & RF4_BR_DISE && brpow > 0)
	{
		brdam = ((brpow / 6) > 500 ? 500 : (brpow / 6));

		msg_print("You are blasted by raw mana!");

		if (p_ptr->resist_disen)
		{
			brdam *= 6;
			brdam /= (randint(6) + 6);
		}
		else
		{
			(void)apply_disenchant(0);
		}

		/* Take damage */
		take_hit(brdam, "raw mana");
		o_ptr->pval = 1;
	}

	/* Plasma */
	if (r_ptr->flags4 & RF4_BR_PLAS && brpow > 0)
	{
		brdam = ((brpow / 6) > 150 ? 150 : (brpow / 6));

		msg_print("Searing flames engulf the corpse!");

		/* Resist the damage */
		if (p_ptr->resist_fire || p_ptr->oppose_fire) brdam = (brdam + 2) / 3;

		if (!p_ptr->resist_sound)
		{
			int k = (randint((brdam > 40) ? 35 : (brdam * 3 / 4 + 5)));
			(void)set_stun(p_ptr->stun + k);
		}

		/* Take damage */
		take_hit(brdam, "an explosion");
		harmful = TRUE;
		o_ptr->pval = 1;
	}

	/* Hack -- Jellies are immune to acid only if they are already acidic */
	if (strchr("j", r_ptr->d_char) && (r_ptr->flags3 & RF3_IM_ACID))
	{
		dam = damroll(8, 8);

		/* Total Immunity */
		if (!(p_ptr->immune_acid || (dam <= 0)))
		{
			/* Resist the damage */
			if (p_ptr->resist_acid) dam = (dam + 2) / 3;
			if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

			/* Take damage */
			take_hit(dam, "acidic food");
		}
		harmful = TRUE;
	}

	/*
	 * Hack -- Jellies, kobolds, spiders, icky things, molds, and mushrooms
	 * are immune to poison because their body already contains
	 * poisonous chemicals.
	 */
	if (strchr("ijkmS,", r_ptr->d_char) && (r_ptr->flags3 & RF3_IM_POIS))
	{
		if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
		{
			set_poisoned(p_ptr->poisoned + rand_int(15) + 10);
		}
		harmful = TRUE;
	}

	/*
	 * Bad effects override good effects
	 * and hacked-up corpses lose intrinsics.
	 */
	if (!harmful && !cutting && (o_ptr->sval != SV_CORPSE_MEAT))
	{
		if (r_ptr->flags3 & RF3_IM_ACID)
		{
			set_oppose_acid(p_ptr->oppose_acid + rand_int(10) + 10);
		}
		if (r_ptr->flags3 & RF3_IM_ELEC)
		{
			set_oppose_elec(p_ptr->oppose_elec + rand_int(10) + 10);
		}
		if (r_ptr->flags3 & RF3_IM_FIRE)
		{
			set_oppose_fire(p_ptr->oppose_fire + rand_int(10) + 10);
		}
		if (r_ptr->flags3 & RF3_IM_COLD)
		{
			set_oppose_cold(p_ptr->oppose_cold + rand_int(10) + 10);
		}
		if (r_ptr->flags3 & RF3_IM_POIS)
		{
			set_oppose_pois(p_ptr->oppose_pois + rand_int(10) + 10);
		}
		if (r_ptr->flags3 & RF3_RES_NETH)
		{
			set_protevil(p_ptr->protevil + rand_int(25) + 3 * r_ptr->level);
		}
		if (r_ptr->flags3 & RF3_RES_PLAS)
		{
			set_oppose_fire(p_ptr->oppose_fire + rand_int(20) + 20);
		}
		if (r_ptr->flags2 & RF2_SHAPECHANGER)
		{
			/* DGDGDG			(void)set_mimic(20 , rand_int(MIMIC_VALAR)); */
		}

		if (r_ptr->flags3 & RF3_DEMON)
		{
			/* DGDGDG			(void)set_mimic(30 , MIMIC_DEMON); */
		}

		if (r_ptr->flags3 & RF3_UNDEAD)
		{
			/* DGDGDG			(void)set_mimic(30 , MIMIC_VAMPIRE); */
		}

		if (r_ptr->flags3 & RF3_NO_FEAR)
		{
			(void)set_afraid(0);
		}
		if (r_ptr->flags3 & RF3_NO_STUN)
		{
			(void)set_stun(0);
		}
		if (r_ptr->flags3 & RF3_NO_CONF)
		{
			(void)set_confused(0);
		}
		if (r_ptr->flags6 & RF6_S_THUNDERLORD)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_THUNDERLORD, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_DEMON)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_DEMON, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_KIN)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_KIN, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_HI_DEMON)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_DEMON, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_MONSTER)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, 0, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_MONSTERS)
		{
			int k;
			for (k = 0; k < 8; k++)
			{
				summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, 0, FALSE);
			}
		}
		if (r_ptr->flags6 & RF6_S_UNDEAD)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNDEAD, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_DRAGON)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_DRAGON, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_ANT)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANT, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_SPIDER)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_SPIDER, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_HOUND)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_HOUND, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_HYDRA)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_HYDRA, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_ANGEL)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_ANGEL, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_HI_DRAGON)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_DRAGON, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_HI_UNDEAD)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_HI_UNDEAD, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_WRAITH)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_WRAITH, FALSE);
		}
		if (r_ptr->flags6 & RF6_S_UNIQUE)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNIQUE, FALSE);
		}
	}
}


/*
 * Hook to determine if an object is eatable
 */
static bool_ item_tester_hook_eatable(object_type *o_ptr)
{
	/* Foods and, well, corpses are edible */
	if ((o_ptr->tval == TV_FOOD) || (o_ptr->tval == TV_CORPSE)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(void)
{
	int item, ident, lev, fval = 0;

	object_type *o_ptr;
	object_type *q_ptr, forge;

	monster_race *r_ptr;

	cptr q, s;

	bool_ destroy = TRUE;


	/* Restrict choices to food  */
	item_tester_hook = item_tester_hook_eatable;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Food full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Eat which item? ";
	s = "You have nothing to eat.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Sound */
	sound(SOUND_EAT);


	/* Take a turn */
	energy_use = 100;

	/* Identity not known yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Scripted foods */
	hook_eat_in in = { o_ptr };
	hook_eat_out out = { FALSE };

	if (process_hooks_ret(HOOK_EAT, "d", "(O)", o_ptr))
	{
		ident = process_hooks_return[0].num;
	}
	else if (process_hooks_new(HOOK_EAT, &in, &out))
	{
		ident = out.ident;
	}
	/* (not quite) Normal foods */
	else if (o_ptr->tval == TV_FOOD)
	{
		/* Analyze the food */
		switch (o_ptr->sval)
		{
		case SV_FOOD_GREAT_HEALTH:
			{
				p_ptr->hp_mod += 70;
				msg_print("As you eat it you begin to feel your life flow getting stronger.");
				ident = TRUE;
				p_ptr->update |= (PU_HP);

				break;
			}

		case SV_FOOD_POISON:
			{
				if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
				{
					if (set_poisoned(p_ptr->poisoned + rand_int(10) + 10))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_BLINDNESS:
			{
				if (!p_ptr->resist_blind)
				{
					if (set_blind(p_ptr->blind + rand_int(200) + 200))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_PARANOIA:
			{
				if (!p_ptr->resist_fear)
				{
					if (set_afraid(p_ptr->afraid + rand_int(10) + 10))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_CONFUSION:
			{
				if (!p_ptr->resist_conf)
				{
					if (set_confused(p_ptr->confused + rand_int(10) + 10))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_HALLUCINATION:
			{
				if (!p_ptr->resist_chaos)
				{
					if (set_image(p_ptr->image + rand_int(250) + 250))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_PARALYSIS:
			{
				if (!p_ptr->free_act)
				{
					if (set_paralyzed(p_ptr->paralyzed + rand_int(10) + 10))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_FOOD_WEAKNESS:
			{
				take_hit(damroll(6, 6), "poisonous food");
				(void)do_dec_stat(A_STR, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_SICKNESS:
			{
				take_hit(damroll(6, 6), "poisonous food");
				(void)do_dec_stat(A_CON, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_STUPIDITY:
			{
				take_hit(damroll(8, 8), "poisonous food");
				(void)do_dec_stat(A_INT, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_NAIVETY:
			{
				take_hit(damroll(8, 8), "poisonous food");
				(void)do_dec_stat(A_WIS, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_UNHEALTH:
			{
				take_hit(damroll(10, 10), "poisonous food");
				(void)do_dec_stat(A_CON, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_DISEASE:
			{
				take_hit(damroll(10, 10), "poisonous food");
				(void)do_dec_stat(A_STR, STAT_DEC_NORMAL);

				ident = TRUE;

				break;
			}

		case SV_FOOD_CURE_POISON:
			{
				if (set_poisoned(0)) ident = TRUE;

				break;
			}

		case SV_FOOD_CURE_BLINDNESS:
			{
				if (set_blind(0)) ident = TRUE;

				break;
			}

		case SV_FOOD_CURE_PARANOIA:
			{
				if (set_afraid(0)) ident = TRUE;

				break;
			}

		case SV_FOOD_CURE_CONFUSION:
			{
				if (set_confused(0)) ident = TRUE;

				break;
			}

		case SV_FOOD_CURE_SERIOUS:
			{
				if (hp_player(damroll(4, 8))) ident = TRUE;

				break;
			}

		case SV_FOOD_RESTORE_STR:
			{
				if (do_res_stat(A_STR, TRUE)) ident = TRUE;

				break;
			}

		case SV_FOOD_RESTORE_CON:
			{
				if (do_res_stat(A_CON, TRUE)) ident = TRUE;

				break;
			}

		case SV_FOOD_RESTORING:
			{
				if (do_res_stat(A_STR, TRUE)) ident = TRUE;
				if (do_res_stat(A_INT, TRUE)) ident = TRUE;
				if (do_res_stat(A_WIS, TRUE)) ident = TRUE;
				if (do_res_stat(A_DEX, TRUE)) ident = TRUE;
				if (do_res_stat(A_CON, TRUE)) ident = TRUE;
				if (do_res_stat(A_CHR, TRUE)) ident = TRUE;

				break;
			}

		case SV_FOOD_FORTUNE_COOKIE:
			{
				char rumour[80];

				msg_print("That tastes good.");
				msg_print("There is message in the cookie. It says:");
				msg_print(NULL);

				switch (randint(20))
				{
				case 1:
					{
						get_rnd_line("chainswd.txt", rumour);
						break;
					}

				case 2:
					{
						get_rnd_line("error.txt", rumour);
						break;
					}

				case 3:
				case 4:
				case 5:
					{
						get_rnd_line("death.txt", rumour);
						break;
					}

				default:
					{
						get_rnd_line("rumors.txt", rumour);
						break;
					}
				}

				msg_format("%s", rumour);
				msg_print(NULL);

				ident = TRUE;

				break;
			}


		case SV_FOOD_RATION:
		case SV_FOOD_BISCUIT:
		case SV_FOOD_JERKY:
			{
				msg_print("That tastes good.");

				ident = TRUE;

				break;
			}

		case SV_FOOD_SLIME_MOLD:
			{
				msg_print("That tastes good.");

				/* 2% chance of getting the mold power */
				if (magik(2))
				{
					ADD_POWER(p_ptr->powers_mod, PWR_GROW_MOLD);
					p_ptr->update |= PU_POWERS;
				}

				ident = TRUE;

				break;
			}

		case SV_FOOD_WAYBREAD:
			{
				msg_print("That tastes very good.");
				(void)set_poisoned(0);
				(void)hp_player(damroll(4, 8));
				set_food(PY_FOOD_MAX - 1);

				ident = TRUE;

				break;
			}

		case SV_FOOD_PINT_OF_ALE:
		case SV_FOOD_PINT_OF_WINE:
			{
				msg_print("That tastes good.");

				ident = TRUE;

				q_ptr = &forge;
				object_prep(q_ptr, lookup_kind(TV_BOTTLE, 1));
				q_ptr->number = 1;
				object_aware(q_ptr);
				object_known(q_ptr);
				q_ptr->ident |= IDENT_STOREB;
				(void)inven_carry(q_ptr, FALSE);

				break;
			}

		case SV_FOOD_ATHELAS:
			{
				msg_print("A fresh, clean essence rises, driving away wounds and poison.");

				(void)set_poisoned(0);
				(void)set_stun(0);
				(void)set_cut(0);
				if (p_ptr->black_breath)
				{
					msg_print("The hold of the Black Breath on you is broken!");
					p_ptr->black_breath = FALSE;
				}

				ident = TRUE;

				break;
			}
		}
	}

	/* Corpses... */
	else
	{
		r_ptr = &r_info[o_ptr->pval2];

		/* Analyse the corpse */
		switch (o_ptr->sval)
		{
		case SV_CORPSE_CORPSE:
			{
				bool_ no_meat = FALSE;

				/* Not all is edible. Apologies if messy. */

				/* Check weight -- they have to have some meat left */
				if (r_ptr->flags9 & RF9_DROP_SKELETON)
				{
					if (o_ptr->weight <= (r_ptr->weight * 3) / 5)
					{
						no_meat = TRUE;
					}
				}

				/* Non-skeletons are naturally have more allowances */
				else
				{
					if (o_ptr->weight <= (r_ptr->weight * 7) / 20)
					{
						no_meat = TRUE;
					}
				}

				/* Nothing left to eat */
				if (no_meat)
				{
					msg_print("There is not enough meat.");
					return;
				}


				/* Check freshness */
				if (!o_ptr->timeout) msg_print("Ugh! Raw meat!");
				else msg_print("That tastes good.");


				/* A pound of raw meat */
				o_ptr->pval -= 10;
				o_ptr->weight -= 10;

				/* Corpses still have meat on them */
				destroy = FALSE;

				ident = TRUE;

				break;
			}

		case SV_CORPSE_HEAD:
			{
				msg_print("You feel rather sick.");

				/* A pound of raw meat */
				o_ptr->pval -= 10;
				o_ptr->weight -= 10;

				/* Corpses still have meat on them */
				destroy = FALSE;

				ident = TRUE;

				break;
			}

		case SV_CORPSE_MEAT:
			{
				/* Just meat */
				if (!o_ptr->timeout) msg_print("You quickly swallow the meat.");
				else msg_print("That tastes good.");

				ident = TRUE;

				/* Those darn microorganisms */
				if (!o_ptr->timeout && (o_ptr->weight > o_ptr->pval) &&
				                !(p_ptr->resist_pois || p_ptr->oppose_pois))
				{
					set_poisoned(p_ptr->poisoned + rand_int(o_ptr->weight - o_ptr->pval) +
					             (o_ptr->weight - o_ptr->pval));
				}

				break;
			}
		}

		corpse_effect(o_ptr, FALSE);

		/* Less nutritious than food rations, but much more of it. */
		fval = (o_ptr->timeout) ? 2000 : 2500;

		/* Those darn microorganisms */
		if (!o_ptr->timeout && (o_ptr->weight - o_ptr->pval > 10) &&
		                !(p_ptr->resist_pois || p_ptr->oppose_pois))
		{
			set_poisoned(p_ptr->poisoned + rand_int(o_ptr->weight - o_ptr->pval) +
			             (o_ptr->weight - o_ptr->pval));
		}

		/* Partially cured */
		if (o_ptr->weight > o_ptr->timeout)
		{
			/* Adjust the "timeout" without overflowing */
			o_ptr->timeout = (o_ptr->timeout * ((100 * o_ptr->timeout) / o_ptr->weight)) / 100;
		}
	}


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* We have tried it */
	object_tried(o_ptr);

	/* The player is now aware of the object */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	if (!fval) fval = o_ptr->pval;

	/* Food can feed the player, in a different ways */

	/* Vampires */
	if ((PRACE_FLAG(PR1_VAMPIRE)) || (p_ptr->mimic_form == resolve_mimic_name("Vampire")))
	{
		/* Reduced nutritional benefit */
		/*		(void)set_food(p_ptr->food + (fval / 10)); -- No more */
		msg_print("Mere victuals hold scant sustenance for a being such as yourself.");

		/* Hungry */
		if (p_ptr->food < PY_FOOD_ALERT)
		{
			msg_print("Your hunger can only be satisfied with fresh blood!");
		}
	}

	else if (PRACE_FLAG(PR1_NO_FOOD))
	{
		if (PRACE_FLAG(PR1_UNDEAD))
		{
			msg_print("The food of mortals is poor sustenance for you.");
		}
		else
		{
			msg_print("Food is poor sustenance for you.");
		}
		set_food(p_ptr->food + ((fval) / 40));
	}

	/* Those living in fresh */
	else
	{
		(void)set_food(p_ptr->food + fval);
	}


	/* Destroy food? */
	if (destroy)
	{
		inc_stack_size(item, -1);
	}
}


/*
 * Cut a corpse up for convenient storage
 */
void do_cmd_cut_corpse(void)
{
	int item, meat = 0, not_meat = 0;

	object_type *o_ptr;

	object_type *i_ptr;

	object_type object_type_body;

	monster_race *r_ptr;

	cptr q, s;


	/* Restrict choices to corpses */
	item_tester_tval = TV_CORPSE;

	/* Get an item */
	q = "Hack up which corpse? ";
	s = "You have no corpses.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	r_ptr = &r_info[o_ptr->pval2];

	if ((o_ptr->sval != SV_CORPSE_CORPSE) && (o_ptr->sval != SV_CORPSE_HEAD))
	{
		msg_print ("You cannot split that.");
		return;
	}

	switch (o_ptr->sval)
	{
	case SV_CORPSE_CORPSE:
		{
			if (r_ptr->flags9 & RF9_DROP_SKELETON)
			{
				not_meat = (r_ptr->weight * 3) / 5;
			}
			else
			{
				not_meat = (r_ptr->weight * 7) / 20;
			}
			meat = r_ptr->weight + r_ptr->weight / 10 - not_meat;

			break;
		}

	case SV_CORPSE_HEAD:
		{
			not_meat = r_ptr->weight / 150;
			meat = r_ptr->weight / 30 + r_ptr->weight / 300 - not_meat;

			break;
		}
	}

	if ((o_ptr->weight <= not_meat) || (meat < 10))
	{
		msg_print("There is not enough meat.");
		return;
	}

	/* Hacking 10 pounds off */
	if (meat > 100) meat = 100;

	/* Take a turn */
	energy_use = 100;

	o_ptr->pval -= meat;
	o_ptr->weight -= meat;

	msg_print("You hack some meat off the corpse.");

	corpse_effect(o_ptr, TRUE);

	/* Get local object */
	i_ptr = &object_type_body;

	/* Make some meat */
	object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_MEAT));

	i_ptr->number = meat / 10;
	i_ptr->pval2 = o_ptr->pval2;

	/* Length of time before decay */
	i_ptr->pval = 1000 + rand_int(1000);

	if (inven_carry_okay(i_ptr))
	{
		inven_carry(i_ptr, TRUE);
	}
	else
	{
		drop_near(i_ptr, 0, p_ptr->py, p_ptr->px);
	}
}


/*
 * Use a potion to cure some meat
 *
 * Salt water works well.
 */
void do_cmd_cure_meat(void)
{
	int item, num, cure;

	object_type *o_ptr;

	object_type *i_ptr;

	cptr q, s;


	/* Restrict choices to corpses */
	item_tester_tval = TV_CORPSE;
	item_tester_hook = item_tester_hook_eatable;

	/* Get some meat */
	q = "Cure which meat? ";
	s = "You have no meat to cure.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Restrict choices to potions */
	item_tester_tval = TV_POTION;

	/* Get a potion */
	q = "Use which potion? ";
	s = "You have no potions to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	i_ptr = get_object(item);

	if (i_ptr->number > 1)
	{
		/* Get a number */
		get_count(1, i_ptr->number);

		/* Save it */
		num = command_arg;
	}
	else
	{
		num = 1;
	}

	if (num == 0) return;

	/* Take a turn */
	energy_use = 100;

	q = "You soak the meat.";
	s = "You soak the meat.";

	switch (i_ptr->sval)
	{
	case SV_POTION_SALT_WATER:
		{
			q = "You salt the meat.";
			cure = 200 * num;

			break;
		}

	case SV_POTION_POISON:
		{
			q = "You poison the meat.";
			cure = 0;
			o_ptr->pval /= 2;
			if (o_ptr->pval > o_ptr->weight) o_ptr->pval = o_ptr->weight;

			break;
		}

	case SV_POTION_CONFUSION:
		{
			cure = 80 * num;

			break;
		}

	case SV_POTION_SLOW_POISON:
		{
			cure = 20 * num;

			break;
		}

	case SV_POTION_CURE_POISON:
		{
			cure = 45 * num;

			break;
		}

	case SV_POTION_DEATH:
		{
			q = "You ruin the meat.";
			cure = 0;
			o_ptr->pval /= 10;
			if (o_ptr->pval > o_ptr->weight) o_ptr->pval = o_ptr->weight / 2;

			break;
		}

	default:
		{
			cure = 0;

			break;
		}
	}

	/* Message */
	if (object_known_p(i_ptr)) msg_print(q);
	else msg_print(s);

	/* The meat is already spoiling */
	if (((o_ptr->sval == SV_CORPSE_MEAT) && (o_ptr->weight > o_ptr->pval)) ||
	                (o_ptr->weight - o_ptr->pval > 10))
	{
		cure = (cure * o_ptr->pval) / (o_ptr->weight * 20);
	}

	/* Cure the meat */
	o_ptr->timeout += cure / o_ptr->number;

	if (o_ptr->timeout > o_ptr->pval) o_ptr->timeout = o_ptr->pval;

	/* Use up the potions */
	inc_stack_size(item, -num);
}


/*
 * Hook to determine if an object is quaffable
 */
static bool_ item_tester_hook_quaffable(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_POTION) || (o_ptr->tval == TV_POTION2)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


static bool_ quaff_potion(int tval, int sval, int pval, int pval2)
{
	int ident = FALSE;


	/* "Traditional" potions */
	if (tval == TV_POTION)
	{
		switch (sval)
		{
		case SV_POTION_WATER:
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SLIME_MOLD:
			{
				msg_print("You feel less thirsty.");
				ident = TRUE;

				break;
			}

		case SV_POTION_SLOWNESS:
			{
				if (set_slow(p_ptr->slow + randint(25) + 15)) ident = TRUE;

				break;
			}

		case SV_POTION_SALT_WATER:
			{
				msg_print("The potion makes you vomit!");
				(void)set_food(PY_FOOD_STARVE - 1);
				(void)set_poisoned(0);
				(void)set_paralyzed(p_ptr->paralyzed + 4);
				ident = TRUE;

				break;
			}

		case SV_POTION_POISON:
			{
				if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
				{
					if (set_poisoned(p_ptr->poisoned + rand_int(15) + 10))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_POTION_BLINDNESS:
			{
				if (!p_ptr->resist_blind)
				{
					if (set_blind(p_ptr->blind + rand_int(100) + 100))
					{
						ident = TRUE;
					}
				}

				break;
			}

			/* Booze */
		case SV_POTION_CONFUSION:
			{
				if (!((p_ptr->resist_conf) || (p_ptr->resist_chaos)))
				{
					if (set_confused(p_ptr->confused + rand_int(20) + 15))
					{
						ident = TRUE;
					}
					if (randint(2) == 1)
					{
						if (set_image(p_ptr->image + rand_int(150) + 150))
						{
							ident = TRUE;
						}
					}
					if (randint(13) == 1)
					{
						ident = TRUE;
						if (randint(3) == 1) lose_all_info();
						else wiz_dark();
						teleport_player(100);
						wiz_dark();
						msg_print("You wake up elsewhere with a sore head...");
						msg_print("You can't remember a thing, or how you got here!");
					}
				}

				break;
			}

		case SV_POTION_SLEEP:
			{
				if (!p_ptr->free_act)
				{
					if (set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_POTION_LOSE_MEMORIES:
			{
				if (!p_ptr->hold_life && (p_ptr->exp > 0))
				{
					msg_print("You feel your memories fade.");
					lose_exp(p_ptr->exp / 4);
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_RUINATION:
			{
				msg_print("Your nerves and muscles feel weak and lifeless!");
				take_hit(damroll(10, 10), "a potion of Ruination");
				(void)dec_stat(A_DEX, 25, TRUE);
				(void)dec_stat(A_WIS, 25, TRUE);
				(void)dec_stat(A_CON, 25, TRUE);
				(void)dec_stat(A_STR, 25, TRUE);
				(void)dec_stat(A_CHR, 25, TRUE);
				(void)dec_stat(A_INT, 25, TRUE);
				ident = TRUE;

				break;
			}

		case SV_POTION_DEC_STR:
			{
				if (do_dec_stat(A_STR, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DEC_INT:
			{
				if (do_dec_stat(A_INT, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DEC_WIS:
			{
				if (do_dec_stat(A_WIS, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DEC_DEX:
			{
				if (do_dec_stat(A_DEX, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DEC_CON:
			{
				if (do_dec_stat(A_CON, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DEC_CHR:
			{
				if (do_dec_stat(A_CHR, STAT_DEC_NORMAL)) ident = TRUE;

				break;
			}

		case SV_POTION_DETONATIONS:
			{
				msg_print("Massive explosions rupture your body!");
				take_hit(damroll(50, 20), "a potion of Detonation");
				(void)set_stun(p_ptr->stun + 75);
				(void)set_cut(p_ptr->cut + 5000);
				ident = TRUE;

				break;
			}

		case SV_POTION_DEATH:
			{
				msg_print("A feeling of Death flows through your body.");
				take_hit(5000, "a potion of Death");
				ident = TRUE;

				break;
			}

		case SV_POTION_INFRAVISION:
			{
				if (set_tim_infra(p_ptr->tim_infra + 100 + randint(100)))
				{
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_DETECT_INVIS:
			{
				if (set_tim_invis(p_ptr->tim_invis + 12 + randint(12)))
				{
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_SLOW_POISON:
			{
				if (set_poisoned(p_ptr->poisoned / 2)) ident = TRUE;

				break;
			}

		case SV_POTION_CURE_POISON:
			{
				if (set_poisoned(0)) ident = TRUE;

				break;
			}

		case SV_POTION_BOLDNESS:
			{
				if (set_afraid(0)) ident = TRUE;

				break;
			}

		case SV_POTION_SPEED:
			{
				if (!p_ptr->fast)
				{
					if (set_fast(randint(25) + 15, 10)) ident = TRUE;
				}
				else
				{
					(void)set_fast(p_ptr->fast + 5, 10);
				}

				break;
			}

		case SV_POTION_RESIST_HEAT:
			{
				if (set_oppose_fire(p_ptr->oppose_fire + randint(10) + 10))
				{
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_RESIST_COLD:
			{
				if (set_oppose_cold(p_ptr->oppose_cold + randint(10) + 10))
				{
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_HEROISM:
			{
				if (set_afraid(0)) ident = TRUE;
				if (set_hero(p_ptr->hero + randint(25) + 25)) ident = TRUE;
				if (hp_player(10)) ident = TRUE;

				break;
			}

		case SV_POTION_BESERK_STRENGTH:
			{
				if (set_afraid(0)) ident = TRUE;
				if (set_shero(p_ptr->shero + randint(25) + 25)) ident = TRUE;
				if (hp_player(30)) ident = TRUE;

				break;
			}

		case SV_POTION_CURE_LIGHT:
			{
				if (hp_player(damroll(2, 8))) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_cut(p_ptr->cut - 10)) ident = TRUE;

				break;
			}

		case SV_POTION_CURE_SERIOUS:
			{
				if (hp_player(damroll(4, 8))) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_confused(0)) ident = TRUE;
				if (set_cut((p_ptr->cut / 2) - 50)) ident = TRUE;

				break;
			}

		case SV_POTION_CURE_CRITICAL:
			{
				if (hp_player(damroll(6, 8))) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_confused(0)) ident = TRUE;
				if (set_poisoned(0)) ident = TRUE;
				if (set_stun(0)) ident = TRUE;
				if (set_cut(0)) ident = TRUE;

				break;
			}

		case SV_POTION_HEALING:
			{
				if (hp_player(300)) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_confused(0)) ident = TRUE;
				if (set_poisoned(0)) ident = TRUE;
				if (set_stun(0)) ident = TRUE;
				if (set_cut(0)) ident = TRUE;

				break;
			}

		case SV_POTION_STAR_HEALING:
			{
				if (hp_player(1200)) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_confused(0)) ident = TRUE;
				if (set_poisoned(0)) ident = TRUE;
				if (set_stun(0)) ident = TRUE;
				if (set_cut(0)) ident = TRUE;

				break;
			}

		case SV_POTION_LIFE:
			{
				msg_print("You feel life flow through your body!");
				restore_level();
				hp_player(5000);
				(void)set_poisoned(0);
				(void)set_blind(0);
				(void)set_confused(0);
				(void)set_image(0);
				(void)set_stun(0);
				(void)set_cut(0);
				(void)do_res_stat(A_STR, TRUE);
				(void)do_res_stat(A_CON, TRUE);
				(void)do_res_stat(A_DEX, TRUE);
				(void)do_res_stat(A_WIS, TRUE);
				(void)do_res_stat(A_INT, TRUE);
				(void)do_res_stat(A_CHR, TRUE);
				if (p_ptr->black_breath)
				{
					msg_print("The hold of the Black Breath on you is broken!");
				}
				p_ptr->black_breath = FALSE;
				ident = TRUE;

				break;
			}

		case SV_POTION_RESTORE_MANA:
			{
				if (p_ptr->csp < p_ptr->msp)
				{
					p_ptr->csp = p_ptr->msp;
					p_ptr->csp_frac = 0;
					msg_print("Your feel your head clear.");
					p_ptr->redraw |= (PR_MANA);
					p_ptr->window |= (PW_PLAYER);
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_RESTORE_EXP:
			{
				if (restore_level()) ident = TRUE;

				break;
			}

		case SV_POTION_RES_STR:
			{
				if (do_res_stat(A_STR, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_RES_INT:
			{
				if (do_res_stat(A_INT, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_RES_WIS:
			{
				if (do_res_stat(A_WIS, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_RES_DEX:
			{
				if (do_res_stat(A_DEX, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_RES_CON:
			{
				if (do_res_stat(A_CON, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_RES_CHR:
			{
				if (do_res_stat(A_CHR, TRUE)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_STR:
			{
				if (do_inc_stat(A_STR)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_INT:
			{
				if (do_inc_stat(A_INT)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_WIS:
			{
				if (do_inc_stat(A_WIS)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_DEX:
			{
				if (do_inc_stat(A_DEX)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_CON:
			{
				if (do_inc_stat(A_CON)) ident = TRUE;

				break;
			}

		case SV_POTION_INC_CHR:
			{
				if (do_inc_stat(A_CHR)) ident = TRUE;

				break;
			}

		case SV_POTION_AUGMENTATION:
			{
				if (do_inc_stat(A_STR)) ident = TRUE;
				if (do_inc_stat(A_INT)) ident = TRUE;
				if (do_inc_stat(A_WIS)) ident = TRUE;
				if (do_inc_stat(A_DEX)) ident = TRUE;
				if (do_inc_stat(A_CON)) ident = TRUE;
				if (do_inc_stat(A_CHR)) ident = TRUE;

				break;
			}

		case SV_POTION_ENLIGHTENMENT:
			{
				msg_print("An image of your surroundings forms in your mind...");
				wiz_lite();
				ident = TRUE;

				break;
			}

		case SV_POTION_STAR_ENLIGHTENMENT:
			{
				msg_print("You begin to feel more enlightened...");
				msg_print(NULL);
				wiz_lite_extra();
				(void)do_inc_stat(A_INT);
				(void)do_inc_stat(A_WIS);
				(void)detect_traps(DEFAULT_RADIUS);
				(void)detect_doors(DEFAULT_RADIUS);
				(void)detect_stairs(DEFAULT_RADIUS);
				(void)detect_treasure(DEFAULT_RADIUS);
				(void)detect_objects_gold(DEFAULT_RADIUS);
				(void)detect_objects_normal(DEFAULT_RADIUS);
				identify_pack();
				self_knowledge(NULL);
				ident = TRUE;

				break;
			}

		case SV_POTION_SELF_KNOWLEDGE:
			{
				msg_print("You begin to know yourself a little better...");
				msg_print(NULL);
				self_knowledge(NULL);
				ident = TRUE;

				break;
			}

		case SV_POTION_EXPERIENCE:
			{
				if (p_ptr->exp < PY_MAX_EXP)
				{
					s32b ee = (p_ptr->exp / 2) + 10;
					if (ee > 100000L) ee = 100000L;
					msg_print("You feel more experienced.");
					gain_exp(ee);
					ident = TRUE;
				}

				break;
			}

		case SV_POTION_RESISTANCE:
			{
				(void)set_oppose_acid(p_ptr->oppose_acid + randint(20) + 20);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint(20) + 20);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);
				(void)set_oppose_pois(p_ptr->oppose_pois + randint(20) + 20);
				ident = TRUE;

				break;
			}

		case SV_POTION_CURING:
			{
				if (hp_player(50)) ident = TRUE;
				if (set_blind(0)) ident = TRUE;
				if (set_poisoned(0)) ident = TRUE;
				if (set_confused(0)) ident = TRUE;
				if (set_stun(0)) ident = TRUE;
				if (set_cut(0)) ident = TRUE;
				if (set_image(0)) ident = TRUE;
				if (heal_insanity(50)) ident = TRUE;

				break;
			}

		case SV_POTION_INVULNERABILITY:
			{
				(void)set_invuln(p_ptr->invuln + randint(7) + 7);
				ident = TRUE;

				break;
			}

		case SV_POTION_NEW_LIFE:
			{
				do_cmd_rerate();
				ident = TRUE;

				break;
			}

		case SV_POTION_BLOOD:
			{
				msg_print("You feel the blood of life running through your veins!");
				ident = TRUE;
				p_ptr->allow_one_death++;

				break;
			}

		case SV_POTION_MUTATION:
			{
				/* In Theme, Melkor likes players who quaff
				   potions of corruption. */
				if (game_module_idx == MODULE_THEME)
				{
					GOD(GOD_MELKOR)
					{
						msg_print("Your quaffing of this potion pleases Melkor!");
						set_grace(p_ptr->grace + 2);
					}
				}

				msg_print("You feel the dark corruptions of Morgoth coming over you!");
				gain_random_corruption();
				ident = TRUE;
				break;
			}

		case SV_POTION_INVIS:
			{
				int t = 30 + randint(30);

				if (set_invis(p_ptr->tim_invis + t, 35))
				{
					ident = TRUE;
				}
				set_tim_invis(p_ptr->tim_invis + t);

				break;
			}

		case SV_POTION_LEARNING:
			{
				p_ptr->skill_points += rand_range(4, 10 + luck( -4, 4));
				cmsg_format(TERM_L_GREEN, "You can increase %d more skills.", p_ptr->skill_points);

				break;
			}

		default:
			{
				break;
			}
		}
	}

	/* "Duplicate" potions */
	else
	{
		switch (sval)
		{
		case SV_POTION2_MIMIC:
			{
				if (!p_ptr->mimic_form)
				{
					s32b time = get_mimic_random_duration(pval2);

					set_mimic(time, pval2, (p_ptr->lev * 2) / 3);

					/* Redraw title */
					p_ptr->redraw |= (PR_TITLE);

					/* Recalculate bonuses */
					p_ptr->update |= (PU_BONUS);

					ident = TRUE;
				}

				break;
			}

		case SV_POTION2_CURE_LIGHT_SANITY:
			{
				if (heal_insanity(damroll(4, 8))) ident = TRUE;

				break;
			}

		case SV_POTION2_CURE_SERIOUS_SANITY:
			{
				if (heal_insanity(damroll(8, 8))) ident = TRUE;

				break;
			}

		case SV_POTION2_CURE_CRITICAL_SANITY:
			{
				if (heal_insanity(damroll(12, 8))) ident = TRUE;

				break;
			}

		case SV_POTION2_CURE_SANITY:
			{
				if (heal_insanity(damroll(10, 100))) ident = TRUE;

				break;
			}

		default:
			{
				break;
			}
		}
	}

	return (ident);
}


/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(void)
{
	int item, ident, lev;

	object_type *o_ptr;

	object_type *q_ptr, forge;

	cptr q, s;


	/* Restrict choices to potions */
	item_tester_hook = item_tester_hook_quaffable;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Potion full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Quaff which potion? ";
	s = "You have no potions to quaff.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);


	/* Sound */
	sound(SOUND_QUAFF);


	/* Take a turn */
	energy_use = 100;

	/* Not identified yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Demon Breath corruption can spoil potions. */
	if (player_has_corruption(CORRUPT_DEMON_BREATH) && magik(9))
	{
		msg_print("Your demon breath spoils the potion!");
		ident = FALSE;
	}
	else
	{
		/* Normal potion handling */
		ident = quaff_potion(o_ptr->tval, o_ptr->sval, o_ptr->pval, o_ptr->pval2);
	}

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item has been tried */
	object_tried(o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	if (get_skill(SKILL_ALCHEMY))
	{
		if (item >= 0)
		{
			q_ptr = &forge;
			object_prep(q_ptr, lookup_kind(TV_BOTTLE, 1));
			q_ptr->number = 1;
			object_aware(q_ptr);
			object_known(q_ptr);

			q_ptr->ident |= IDENT_STOREB;

			(void)inven_carry(q_ptr, FALSE);
		}
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);


	/* Potions can feed the player */
	(void)set_food(p_ptr->food + o_ptr->pval);


	/* Destroy potion */
	inc_stack_size(item, -1);
}


/*
 * Drink from a fountain
 */
void do_cmd_drink_fountain(void)
{
	cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];

	bool_ ident;

	int tval, sval, pval = 0;

	int i;

	char ch;


	/* Is the fountain empty? */
	if (c_ptr->special2 <= 0)
	{
		msg_print("The fountain is dried out.");
		return;
	}

	/* We quaff or we fill ? */
	if (!get_com("Do you want to [Q]uaff or [F]ill from the fountain? ", &ch))
	{
		return;
	}

	if ((ch == 'F') || (ch == 'f'))
	{
		do_cmd_fill_bottle();

		return;
	}

	else if ((ch == 'Q') || (ch == 'q'))
	{
		if (c_ptr->special <= SV_POTION_LAST)
		{
			tval = TV_POTION;
			sval = c_ptr->special;
		}
		else
		{
			tval = TV_POTION2;
			sval = c_ptr->special - SV_POTION_LAST;
		}

		for (i = 0; i < max_k_idx; i++)
		{
			object_kind *k_ptr = &k_info[i];

			if (k_ptr->tval != tval) continue;
			if (k_ptr->sval != sval) continue;

			pval = k_ptr->pval;

			break;
		}

		ident = quaff_potion(tval, sval, pval, 0);

		c_ptr->special2--;

		if (c_ptr->special2 <= 0)
		{
			cave_set_feat(p_ptr->py, p_ptr->px, FEAT_EMPTY_FOUNTAIN);
		}

		if (ident) c_ptr->info |= CAVE_IDNT;
	}
}


/*
 * Fill an empty bottle
 */
void do_cmd_fill_bottle(void)
{
	cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];

	int tval, sval, item, amt = 1;

	object_type *q_ptr, *o_ptr, forge;

	cptr q, s;

	/* Is the fountain empty? */
	/*
	 * This check is redundant as it is done in do_cmd_drink_fountain()
	 * but I keep this because someone might want to call this directly.
	 * -- Kusunose
	 */
	if (c_ptr->special2 <= 0)
	{
		msg_print("The fountain has dried up.");
		return;
	}

	/* Determine the tval/sval of the potion */
	if (c_ptr->special <= SV_POTION_LAST)
	{
		tval = TV_POTION;
		sval = c_ptr->special;
	}
	else
	{
		tval = TV_POTION2;
		sval = c_ptr->special - SV_POTION_LAST;
	}

	/* Restrict choices to bottles */
	item_tester_tval = TV_BOTTLE;

	/* Get an item */
	q = "Fill which bottle? ";
	s = "You have no bottles to fill.";
	if (!get_item(&item, q, s, (USE_INVEN))) return;
	o_ptr = &p_ptr->inventory[item];

	/* Find out how many the player wants */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	if (amt > c_ptr->special2) amt = c_ptr->special2;

	/* Destroy bottles */
	inc_stack_size(item, -amt);

	/* Create the potion */
	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(tval, sval));
	q_ptr->number = amt;

	if (c_ptr->info & CAVE_IDNT)
	{
		object_aware(q_ptr);
		object_known(q_ptr);
	}

	inven_carry(q_ptr, TRUE);

	c_ptr->special2 -= amt;

	if (c_ptr->special2 <= 0)
	{
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_EMPTY_FOUNTAIN);
	}

	return;
}


/*
 * Curse the players armor
 */
bool_ curse_armor(void)
{
	object_type *o_ptr;

	char o_name[80];


	/* Curse the body armor */
	o_ptr = &p_ptr->inventory[INVEN_BODY];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, FALSE, 3);

	/* Attempt a saving throw for artifacts */
	if (((o_ptr->art_name) || artifact_p(o_ptr)) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format("A terrible black aura tries to surround your armour, "
		           "but your %s resists the effects!", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format("A terrible black aura blasts your %s!", o_name);

		/* Blast the armor */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_BLASTED;
		o_ptr->to_a = 0 - randint(5) - randint(5);
		o_ptr->to_h = 0;
		o_ptr->to_d = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;
		o_ptr->art_flags1 = 0;
		o_ptr->art_flags2 = 0;
		o_ptr->art_flags3 = 0;
		o_ptr->art_flags4 = 0;

		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	return (TRUE);
}


/*
 * Curse the players weapon
 */
bool_ curse_weapon(void)
{
	object_type *o_ptr;

	char o_name[80];


	/* Curse the weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Nothing to curse */
	if (!o_ptr->k_idx) return (FALSE);


	/* Describe */
	object_desc(o_name, o_ptr, FALSE, 3);

	/* Attempt a saving throw */
	if ((artifact_p(o_ptr) || o_ptr->art_name) && (rand_int(100) < 50))
	{
		/* Cool */
		msg_format("A terrible black aura tries to surround your weapon, "
		           "but your %s resists the effects!", o_name);
	}

	/* not artifact or failed save... */
	else
	{
		/* Oops */
		msg_format("A terrible black aura blasts your %s!", o_name);

		/* Shatter the weapon */
		o_ptr->name1 = 0;
		o_ptr->name2 = EGO_SHATTERED;
		o_ptr->to_h = 0 - randint(5) - randint(5);
		o_ptr->to_d = 0 - randint(5) - randint(5);
		o_ptr->to_a = 0;
		o_ptr->ac = 0;
		o_ptr->dd = 0;
		o_ptr->ds = 0;
		o_ptr->art_flags1 = 0;
		o_ptr->art_flags2 = 0;
		o_ptr->art_flags3 = 0;
		o_ptr->art_flags4 = 0;


		/* Curse it */
		o_ptr->ident |= (IDENT_CURSED);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}

	/* Notice */
	return (TRUE);
}


/*
 * Hook to determine if an object is readable
 */
static bool_ item_tester_hook_readable(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_SCROLL) || (o_ptr->tval == TV_PARCHMENT)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Read a scroll (from the pack or floor).
 *
 * Certain scrolls can be "aborted" without losing the scroll.  These
 * include scrolls with no effects but recharge or identify, which are
 * cancelled before use.  XXX Reading them still takes a turn, though.
 */
void do_cmd_read_scroll(void)
{
	int item, k, used_up, ident, lev;

	object_type *o_ptr;

	object_type *q_ptr, forge;

	cptr q, s;


	/* Check some conditions */
	if (p_ptr->blind)
	{
		msg_print("You can't see anything.");
		return;
	}

	if (no_lite())
	{
		msg_print("You have no light by which to read.");
		return;
	}

	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}


	/* Restrict choices to scrolls */
	item_tester_hook = item_tester_hook_readable;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Scroll full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Read which scroll? ";
	s = "You have no scrolls to read.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Take a turn */
	energy_use = 100;

	/* Not identified yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Assume the scroll will get used up */
	used_up = TRUE;

	/* Corruption */
	if (player_has_corruption(CORRUPT_BALROG_AURA) && magik(5))
	{
		msg_print("Your demon aura burns the scroll before you read it!");
		used_up = TRUE;
		ident = FALSE;
	}

	/* Scrolls */
	else if (o_ptr->tval == TV_SCROLL)
	{
		/* Analyze the scroll */
		switch (o_ptr->sval)
		{
		case SV_SCROLL_MASS_RESURECTION:
			{
				int k;

				ident = TRUE;
				msg_print("You feel the souls of the dead coming back "
				          "from the Halls of Mandos.");

				for (k = 0; k < max_r_idx; k++)
				{
					monster_race *r_ptr = &r_info[k];

					if (r_ptr->flags1 & RF1_UNIQUE &&
					                !(r_ptr->flags9 & RF9_SPECIAL_GENE))
					{
						r_ptr->max_num = 1;
					}
				}

				break;
			}

		case SV_SCROLL_DEINCARNATION:
			{
				if (!get_check("Do you really want to leave your body? "
				                "(beware, it'll be destroyed!) "))
				{
					used_up = FALSE;
					break;
				}

				do_cmd_leave_body(FALSE);

				ident = TRUE;
				used_up = TRUE;

				break;
			}

			/* original didn't set used_up flag ??? -- pelpel */
		case SV_SCROLL_RESET_RECALL:
			{
				if (!reset_recall(TRUE))
				{
					used_up = FALSE;
					break;
				}

				msg_format("Recall reset to %s at level %d.",
				           d_info[p_ptr->recall_dungeon].name + d_name,
				           max_dlv[p_ptr->recall_dungeon]);

				ident = TRUE;
				used_up = TRUE;

				break;
			}

		case SV_SCROLL_DIVINATION:
			{
				int i, count = 0;
				char buf[120];

				while (count < 1000)
				{
					count++;
					i = rand_int(MAX_FATES);
					if (!fates[i].fate) continue;
					if (fates[i].know) continue;

					msg_print("A message appears on the scroll. It says:");
					msg_print(NULL);

					fate_desc(buf, i);
					msg_format("%s", buf);

					msg_print(NULL);
					msg_print("The scroll disappears in a puff of smoke!");

					fates[i].know = TRUE;
					ident = TRUE;

					break;
				}

				break;
			}

		case SV_SCROLL_DARKNESS:
			{
				if (!(p_ptr->resist_blind) && !(p_ptr->resist_dark))
				{
					(void)set_blind(p_ptr->blind + 3 + randint(5));
				}
				if (unlite_area(10, 3)) ident = TRUE;

				break;
			}

		case SV_SCROLL_AGGRAVATE_MONSTER:
			{
				msg_print("There is a high-pitched humming noise.");
				aggravate_monsters(1);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_CURSE_ARMOR:
			{
				if (curse_armor()) ident = TRUE;

				break;
			}

		case SV_SCROLL_CURSE_WEAPON:
			{
				if (curse_weapon()) ident = TRUE;

				break;
			}

		case SV_SCROLL_SUMMON_MONSTER:
			{
				for (k = 0; k < randint(3); k++)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, dun_level, 0))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_SCROLL_SUMMON_MINE:
			{
				if (summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_MINE, FALSE))
				{
					ident = TRUE;
				}

				break;
			}

		case SV_SCROLL_SUMMON_UNDEAD:
			{
				for (k = 0; k < randint(3); k++)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, dun_level, SUMMON_UNDEAD))
					{
						ident = TRUE;
					}
				}

				break;
			}

		case SV_SCROLL_TRAP_CREATION:
			{
				if (trap_creation()) ident = TRUE;

				break;
			}

		case SV_SCROLL_PHASE_DOOR:
			{
				teleport_player(10);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_TELEPORT:
			{
				teleport_player(100);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_TELEPORT_LEVEL:
			{
				(void)teleport_player_level();

				ident = TRUE;

				break;
			}

		case SV_SCROLL_WORD_OF_RECALL:
			{
				if ((dungeon_flags2 & DF2_ASK_LEAVE) && !get_check("Leave this unique level forever? "))
				{
					used_up = FALSE;
				}
				else
				{
					recall_player(21, 15);

					ident = TRUE;
				}

				break;
			}

		case SV_SCROLL_IDENTIFY:
			{
				ident = TRUE;

				if (!ident_spell()) used_up = FALSE;

				break;
			}

		case SV_SCROLL_STAR_IDENTIFY:
			{
				ident = TRUE;

				if (!identify_fully()) used_up = FALSE;

				break;
			}

		case SV_SCROLL_REMOVE_CURSE:
			{
				if (remove_curse())
				{
					msg_print("You feel as if someone is watching over you.");
					ident = TRUE;
				}

				break;
			}

		case SV_SCROLL_STAR_REMOVE_CURSE:
			{
				remove_all_curse();

				ident = TRUE;

				break;
			}

		case SV_SCROLL_ENCHANT_ARMOR:
			{
				ident = TRUE;

				if (!enchant_spell(0, 0, 1, 0)) used_up = FALSE;

				break;
			}

		case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
			{
				if (!enchant_spell(1, 0, 0, 0)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
			{
				if (!enchant_spell(0, 1, 0, 0)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_ENCHANT_WEAPON_PVAL:
			{
				if (!enchant_spell(0, 0, 0, 1)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_STAR_ENCHANT_ARMOR:
			{
				if (!enchant_spell(0, 0, randint(3) + 2, 0)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_STAR_ENCHANT_WEAPON:
			{
				if (!enchant_spell(randint(3), randint(3), 0, 0)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_RECHARGING:
			{
				if (!recharge(60)) used_up = FALSE;

				ident = TRUE;

				break;
			}

		case SV_SCROLL_LIGHT:
			{
				if (lite_area(damroll(2, 8), 2)) ident = TRUE;

				break;
			}

		case SV_SCROLL_MAPPING:
			{
				map_area();

				ident = TRUE;

				break;
			}

		case SV_SCROLL_DETECT_GOLD:
			{
				if (detect_treasure(DEFAULT_RADIUS)) ident = TRUE;
				if (detect_objects_gold(DEFAULT_RADIUS)) ident = TRUE;

				break;
			}

		case SV_SCROLL_DETECT_ITEM:
			{
				if (detect_objects_normal(DEFAULT_RADIUS)) ident = TRUE;

				break;
			}

		case SV_SCROLL_DETECT_TRAP:
			{
				if (detect_traps(DEFAULT_RADIUS)) ident = TRUE;

				break;
			}

		case SV_SCROLL_DETECT_DOOR:
			{
				if (detect_doors(DEFAULT_RADIUS)) ident = TRUE;
				if (detect_stairs(DEFAULT_RADIUS)) ident = TRUE;

				break;
			}

		case SV_SCROLL_DETECT_INVIS:
			{
				if (detect_monsters_invis(DEFAULT_RADIUS)) ident = TRUE;

				break;
			}

		case SV_SCROLL_SATISFY_HUNGER:
			{
				if (set_food(PY_FOOD_MAX - 1)) ident = TRUE;

				break;
			}

		case SV_SCROLL_BLESSING:
			{
				if (set_blessed(p_ptr->blessed + randint(12) + 6)) ident = TRUE;

				break;
			}

		case SV_SCROLL_HOLY_CHANT:
			{
				if (set_blessed(p_ptr->blessed + randint(24) + 12)) ident = TRUE;

				break;
			}

		case SV_SCROLL_HOLY_PRAYER:
			{
				if (set_blessed(p_ptr->blessed + randint(48) + 24)) ident = TRUE;

				break;
			}

		case SV_SCROLL_MONSTER_CONFUSION:
			{
				if (p_ptr->confusing == 0)
				{
					msg_print("Your hands begin to glow.");
					p_ptr->confusing = TRUE;
					ident = TRUE;
				}

				break;
			}

		case SV_SCROLL_PROTECTION_FROM_EVIL:
			{
				k = 3 * p_ptr->lev;
				if (set_protevil(p_ptr->protevil + randint(25) + k)) ident = TRUE;

				break;
			}

		case SV_SCROLL_RUNE_OF_PROTECTION:
			{
				warding_glyph();

				ident = TRUE;

				break;
			}

		case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
			{
				if (destroy_doors_touch()) ident = TRUE;

				break;
			}

		case SV_SCROLL_STAR_DESTRUCTION:
			{
				/* Prevent destruction of quest levels and town */
				if (!is_quest(dun_level) && dun_level)
				{
					destroy_area(p_ptr->py, p_ptr->px, 15, TRUE, FALSE);
				}
				else
				{
					msg_print("The dungeon trembles...");
				}

				ident = TRUE;

				break;
			}

		case SV_SCROLL_DISPEL_UNDEAD:
			{
				if (dispel_undead(60)) ident = TRUE;

				break;
			}

		case SV_SCROLL_GENOCIDE:
			{
				(void)genocide(TRUE);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_MASS_GENOCIDE:
			{
				(void)mass_genocide(TRUE);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_ACQUIREMENT:
			{
				acquirement(p_ptr->py, p_ptr->px, 1, TRUE, FALSE);

				ident = TRUE;

				break;
			}

		case SV_SCROLL_STAR_ACQUIREMENT:
			{
				acquirement(p_ptr->py, p_ptr->px, randint(2) + 1, TRUE, FALSE);

				ident = TRUE;

				break;
			}

			/* ZAngband scrolls */
		case SV_SCROLL_FIRE:
			{
				fire_ball(GF_FIRE, 0, 150, 4);

				/*
				 * Note: "Double" damage since it is centered on
				 * the player ...
				 */
				if (!p_ptr->oppose_fire && !p_ptr->resist_fire &&
				                !p_ptr->immune_fire)
				{
					take_hit(50 + randint(50) + (p_ptr->sensible_fire) ? 20 : 0,
					         "a Scroll of Fire");
				}

				ident = TRUE;

				break;
			}


		case SV_SCROLL_ICE:
			{
				fire_ball(GF_ICE, 0, 175, 4);

				if (!p_ptr->oppose_cold && !p_ptr->resist_cold &&
				                !p_ptr->immune_cold)
				{
					take_hit(100 + randint(100), "a Scroll of Ice");
				}

				ident = TRUE;

				break;
			}

		case SV_SCROLL_CHAOS:
			{
				fire_ball(GF_CHAOS, 0, 222, 4);

				if (!p_ptr->resist_chaos)
				{
					take_hit(111 + randint(111), "a Scroll of Chaos");
				}

				ident = TRUE;

				break;
			}

		case SV_SCROLL_RUMOR:
			{
				char rumour[80];

				msg_print("There is message on the scroll. It says:");
				msg_print(NULL);

				/* Pick random text */
				switch (randint(20))
				{
				case 1:
					{
						get_rnd_line("chainswd.txt", rumour);

						break;
					}

				case 2:
					{
						get_rnd_line("error.txt", rumour);

						break;
					}

				case 3:
				case 4:
				case 5:
					{
						get_rnd_line("death.txt", rumour);

						break;
					}

				default:
					{
						get_rnd_line("rumors.txt", rumour);

						break;
					}
				}

				msg_format("%s", rumour);
				msg_print(NULL);

				msg_print("The scroll disappears in a puff of smoke!");

				ident = TRUE;

				break;
			}

		case SV_SCROLL_ARTIFACT:
			{
				ident = TRUE;

				if (!artifact_scroll()) used_up = FALSE;

				break;
			}

		case SV_SCROLL_STERILIZATION:
			{
				msg_print("A neutralising wave radiates from you!");
				set_no_breeders(randint(100) + 100);

				break;
			}

		default:
			{
				break;
			}
		}
	}

	/* Other readable items */
	else
	{
		/* Maps */
		if (o_ptr->sval >= 200)
		{
			int i, n;
			char buf[80], fil[20];

			strnfmt(fil, 20, "book-%d.txt", o_ptr->sval);

			n = atoi(get_line(fil, ANGBAND_DIR_FILE, buf, -1));

			/* Parse all the fields */
			for (i = 0; i < n; i += 4)
			{
				/* Grab the fields */
				int x = atoi(get_line(fil, ANGBAND_DIR_FILE, buf, i + 0));
				int y = atoi(get_line(fil, ANGBAND_DIR_FILE, buf, i + 1));
				int w = atoi(get_line(fil, ANGBAND_DIR_FILE, buf, i + 2));
				int h = atoi(get_line(fil, ANGBAND_DIR_FILE, buf, i + 3));

				reveal_wilderness_around_player(y, x, h, w);
			}
		}

		/* Normal parchements */
		else
		{
			/* Save screen */
			screen_save();

			/* Get the filename */
			q = format("book-%d.txt", o_ptr->sval);

			/* Peruse the help file */
			(void)show_file(q, NULL, 0, 0);

			/* Load screen */
			screen_load();

			if (o_ptr->sval >= 100)
			{
				inscription_info[o_ptr->sval - 100].know = TRUE;
			}

			used_up = FALSE;
		}
	}


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item was tried */
	object_tried(o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);


	/* Hack -- allow certain scrolls to be "preserved" */
	if (!used_up) return;

	sound(SOUND_SCROLL);

	/* Destroy scroll */
	inc_stack_size(item, -1);

	if (get_skill(SKILL_ALCHEMY))
	{
		if (item >= 0)
		{
			q_ptr = &forge;
			object_prep(q_ptr, lookup_kind(TV_SCROLL, SV_SCROLL_NOTHING));
			object_aware(q_ptr);
			object_known(q_ptr);

			q_ptr->ident |= IDENT_STOREB;

			(void)inven_carry(q_ptr, FALSE);
		}
	}
}



/* Set the 'stick mode' on */
void set_stick_mode(object_type *o_ptr)
{
	s32b bonus = o_ptr->pval3 & 0xFFFF;
	s32b max = o_ptr->pval3 >> 16;

	get_level_use_stick = bonus;
	get_level_max_stick = max;
}

/* Remove 'stick mode' */
void unset_stick_mode()
{
	get_level_use_stick = -1;
	get_level_max_stick = -1;
}


/*
 * Activate a device
 */
static void activate_stick(s16b s, bool_ *obvious, bool_ *use_charge)
{
	spell_type *spell = spell_at(s);
	casting_result ret;

	assert(obvious != NULL);
	assert(use_charge != NULL);

	ret = spell_type_produce_effect(spell, -1);

	switch (ret)
	{
	case NO_CAST:
		*use_charge = FALSE;
		*obvious = FALSE;
		break;
	case CAST_HIDDEN:
		*use_charge = TRUE;
		*obvious = FALSE;
		break;
	case CAST_OBVIOUS:
		*use_charge = TRUE;
		*obvious = TRUE;
		break;
	default:
		assert(FALSE);
	}
}


/*
 * Use a staff.                        -RAK-
 *
 * One charge of one staff disappears.
 *
 * Hack -- staffs of identify can be "cancelled".
 */
void do_cmd_use_staff(void)
{
	int item, ident, chance;

	bool_ obvious, use_charge;

	object_type *o_ptr;

	u32b f1, f2, f3, f4, f5, esp;

	cptr q, s;

	/* No magic */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}

	/* Restrict choices to wands */
	item_tester_tval = TV_STAFF;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Staff full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Use which staff? ";
	s = "You have no staff to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Mega-Hack -- refuse to use a pile from the ground */
	if ((item < 0) && (o_ptr->number > 1))
	{
		msg_print("You must first pick up the staffs.");
		return;
	}

	/* Enter device mode  */
	set_stick_mode(o_ptr);

	/* Take a turn */
	energy_use = 100;

	/* Not identified yet */
	ident = FALSE;

	/* get the chance */
	chance = spell_chance(o_ptr->pval2);

	/* Extract object flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Is it simple to use ? */
	if (f4 & TR4_EASY_USE)
	{
		chance /= 3;
	}

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if (magik(chance))
	{
		if (flush_failure) flush();
		msg_print("You failed to use the staff properly.");
		sound(SOUND_FAIL);

		/* Leave device mode  */
		unset_stick_mode();
		return;
	}

	/* Notice empty staffs */
	if (o_ptr->pval <= 0)
	{
		if (flush_failure) flush();
		msg_print("The staff has no charges left.");
		o_ptr->ident |= (IDENT_EMPTY);

		/* Leave device mode  */
		unset_stick_mode();
		return;
	}


	/* Sound */
	sound(SOUND_ZAP);


	/* Analyze the staff */
	activate_stick(o_ptr->pval2, &obvious, &use_charge);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Tried the item */
	object_tried(o_ptr);

	/* An identification was made */
	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);


	/* Hack -- some uses are "free" */
	if (!use_charge)
	{
		/* Leave device mode  */
		unset_stick_mode();

		return;
	}

	/* An identification was made */
	if (obvious)
	{
		object_aware(o_ptr);
	}

	/* Use a single charge */
	o_ptr->pval--;

	/* XXX Hack -- unstack if necessary */
	if ((item >= 0) && (o_ptr->number > 1))
	{
		object_type forge;
		object_type *q_ptr;

		/* Get local object */
		q_ptr = &forge;

		/* Obtain a local object */
		object_copy(q_ptr, o_ptr);

		/* Modify quantity */
		q_ptr->number = 1;

		/* Restore the charges */
		o_ptr->pval++;

		/* Unstack the used item */
		o_ptr->number--;
		item = inven_carry(q_ptr, FALSE);

		/* Message */
		msg_print("You unstack your staff.");
	}

	/* Describe charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(item);
	}

	/* Describe charges on the floor */
	else
	{
		floor_item_charges(0 - item);
	}

	/* Leave device mode  */
	unset_stick_mode();
}


/*
 * Aim a wand (from the pack or floor).
 *
 * Use a single charge from a single item.
 * Handle "unstacking" in a logical manner.
 *
 * For simplicity, you cannot use a stack of items from the
 * ground.  This would require too much nasty code.
 *
 * There are no wands which can "destroy" themselves, in the p_ptr->inventory
 * or on the ground, so we can ignore this possibility.  Note that this
 * required giving "wand of wonder" the ability to ignore destruction
 * by electric balls.
 *
 * All wands can be "cancelled" at the "Direction?" prompt for free.
 *
 * Note that the basic "bolt" wands do slightly less damage than the
 * basic "bolt" rods, but the basic "ball" wands do the same damage
 * as the basic "ball" rods.
 */
void do_cmd_aim_wand(void)
{
	bool_ obvious, use_charge;

	int item, ident, chance, sval;

	object_type *o_ptr;

	cptr q, s;

	u32b f1, f2, f3, f4, f5, esp;


	/* No magic */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}

	/* Restrict choices to wands */
	item_tester_tval = TV_WAND;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Wand full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Aim which wand? ";
	s = "You have no wand to aim.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);


	/* Mega-Hack -- refuse to aim a pile from the ground */
	if ((item < 0) && (o_ptr->number > 1))
	{
		msg_print("You must first pick up the wands.");
		return;
	}

	/* Take a turn */
	energy_use = 100;

	/* Not identified yet */
	ident = FALSE;

	/* Enter device mode  */
	set_stick_mode(o_ptr);

	/* get the chance */
	chance = spell_chance(o_ptr->pval2);

	/* Extract object flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Is it simple to use ? */
	if (f4 & TR4_EASY_USE)
	{
		chance /= 3;
	}

	/* Roll for usage */
	if (magik(chance))
	{
		if (flush_failure) flush();
		msg_print("You failed to use the wand properly.");
		sound(SOUND_FAIL);

		/* Leave device mode  */
		unset_stick_mode();
		return;
	}

	/* The wand is already empty! */
	if (o_ptr->pval <= 0)
	{
		if (flush_failure) flush();
		msg_print("The wand has no charges left.");
		o_ptr->ident |= (IDENT_EMPTY);

		/* Leave device mode  */
		unset_stick_mode();
		return;
	}


	/* Sound */
	sound(SOUND_ZAP);


	/* XXX Hack -- Extract the "sval" effect */
	sval = o_ptr->sval;

	/* Analyze the wand */
	activate_stick(o_ptr->pval2, &obvious, &use_charge);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Mark it as tried */
	object_tried(o_ptr);

	/* Hack -- some uses are "free" */
	if (!use_charge)
	{
		/* Leave device mode  */
		unset_stick_mode();

		return;
	}

	/* An identification was made */
	if (obvious)
	{
		object_aware(o_ptr);
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);


	/* Use a single charge */
	o_ptr->pval--;

	/* Describe the charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(item);
	}

	/* Describe the charges on the floor */
	else
	{
		floor_item_charges(0 - item);
	}

	/* Leave device mode  */
	unset_stick_mode();
}






/*
 * Activate (zap) a Rod
 *
 * Unstack fully charged rods as needed.
 *
 * Hack -- rods of perception/genocide can be "cancelled"
 * All rods can be cancelled at the "Direction?" prompt
 */


/*
 * Hook to determine if an object is zapable
 */
static bool_ item_tester_hook_zapable(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_ROD) || (o_ptr->tval == TV_ROD_MAIN)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Hook to determine if an object is attachable
 */
static bool_ item_tester_hook_attachable(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_ROD_MAIN) &&
	                (o_ptr->pval == SV_ROD_NOTHING)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Combine a rod and a rod tip
 */
void zap_combine_rod_tip(object_type *q_ptr, int tip_item)
{
	int item;

	object_type *o_ptr;

	cptr q, s;

	u32b f1, f2, f3, f4, f5, esp;
	s32b cost;


	/* No magic */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}

	/* Restrict choices to rods */
	item_tester_hook = item_tester_hook_attachable;

	/* Get an item */
	q = "Attach the rod tip with which rod? ";
	s = "You have no rod to attach to.";
	if (!get_item(&item, q, s, (USE_INVEN))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Examine the rod */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Calculate rod tip's mana cost */
	cost = q_ptr->pval;

	if (f4 & TR4_CHEAPNESS)
	{
		cost /= 2;
	}

	/*
	 * The rod must have at least the same mana capacity as the
	 * rod tip spell needs
	 */
	if (o_ptr->pval2 < cost)
	{
		msg_print("This rod doesn't have enough mana for the rod tip.");
		return;
	}

	/* Attach the tip to the rod */
	o_ptr->pval = q_ptr->sval;

	/* Destroy rod tip */
	inc_stack_size(tip_item, -1);
}


/*
 * Zap a rod, or attack a rod tip to a rod
 */
void do_cmd_zap_rod(void)
{
	int item, ident, chance, dir, lev;

	int cost;

	bool_ require_dir;

	object_type *o_ptr;

	object_kind *tip_ptr;

	u32b f1, f2, f3, f4, f5, esp;

	cptr q, s;

	/* Hack -- let perception get aborted */
	bool_ use_charge = TRUE;


	/* No magic */
	if (p_ptr->antimagic)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}


	/* Restrict choices to rods */
	item_tester_hook = item_tester_hook_zapable;

	/* Set up the extra finder */
	get_item_hook_find_obj_what = "Rod full name? ";
	get_item_extra_hook = get_item_hook_find_obj;

	/* Get an item */
	q = "Zap which rod? ";
	s = "You have no rod to zap.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EXTRA))) return;

	/* Get the item */
	o_ptr = get_object(item);


	/* "Zapping" a Rod Tip on rod of nothing will attach it */
	if (o_ptr->tval == TV_ROD)
	{
		if (item >= 0)
		{
			zap_combine_rod_tip(o_ptr, item);
			return;
		}
		else
		{
			msg_print("You can't zap a rod tip that's on the floor.");
			return;
		}
	}


	/* Non-directed rods */
	if (o_ptr->pval < SV_ROD_MIN_DIRECTION)
	{
		require_dir = FALSE;
	}

	/* Some rods always require direction */
	else
	{
		switch (o_ptr->pval)
		{
		case SV_ROD_DETECT_TRAP:
		case SV_ROD_HAVOC:
		case SV_ROD_HOME:
			{
				require_dir = FALSE;
				break;
			}

		default:
			{
				require_dir = TRUE;
				break;
			}
		}
	}

	/* Get a direction (unless KNOWN not to need it) */
	if (!object_aware_p(o_ptr) || require_dir)
	{
		/* Get a direction, allow cancel */
		if (!get_aim_dir(&dir)) return;
	}

	/* Take a turn */
	energy_use = 100;

	/* Examine the rod */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (f4 & TR4_FAST_CAST) energy_use /= 2;

	/* Not identified yet */
	ident = FALSE;

	/* Extract the item level */
	tip_ptr = &k_info[lookup_kind(TV_ROD, o_ptr->pval)];
	lev = k_info[lookup_kind(TV_ROD, o_ptr->pval)].level;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	if (chance <= 0)
	{
		chance = 1;
	}

	/* Is it simple to use ? */
	if (f4 & TR4_EASY_USE)
	{
		chance *= 10;
	}

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE))
	{
		/* Flush input if necessary */
		if (flush_failure) flush();

		/* Message */
		msg_print("You failed to use the rod properly.");

		sound(SOUND_FAIL);

		return;
	}

	/* Extract mana cost */
	cost = tip_ptr->pval;

	/* "Cheapness" ego halven the cost */
	if (f4 & TR4_CHEAPNESS) cost = cost / 2;

	/* A single rod is still charging */
	if (o_ptr->timeout < cost)
	{
		/* Flush input if necessary */
		if (flush_failure) flush();

		/* Message */
		msg_print("The rod does not have enough mana yet.");

		return;
	}

	/* Increase the timeout by the rod kind's pval. */
	o_ptr->timeout -= cost;

	/* Sound */
	sound(SOUND_ZAP);

	/* Analyze the rod */
	switch (o_ptr->pval)
	{
	case SV_ROD_HOME:
		{
			ident = TRUE;

			do_cmd_home_trump();

			break;
		}

	case SV_ROD_DETECT_TRAP:
		{
			if (detect_traps(DEFAULT_RADIUS)) ident = TRUE;

			break;
		}

	case SV_ROD_DETECT_DOOR:
		{
			if (detect_doors(DEFAULT_RADIUS)) ident = TRUE;
			if (detect_stairs(DEFAULT_RADIUS)) ident = TRUE;

			break;
		}

	case SV_ROD_IDENTIFY:
		{
			ident = TRUE;

			if (!ident_spell()) use_charge = FALSE;

			break;
		}

	case SV_ROD_RECALL:
		{
			if ((dungeon_flags2 & DF2_ASK_LEAVE) && !get_check("Leave this unique level forever? "))
			{
				use_charge = FALSE;
			}
			else
			{
				recall_player(21, 15);

				ident = TRUE;
			}

			break;
		}

	case SV_ROD_ILLUMINATION:
		{
			if (lite_area(damroll(2, 8), 2)) ident = TRUE;

			break;
		}

	case SV_ROD_MAPPING:
		{
			map_area();

			ident = TRUE;

			break;
		}

	case SV_ROD_DETECTION:
		{
			detect_all(DEFAULT_RADIUS);

			ident = TRUE;

			break;
		}

	case SV_ROD_PROBING:
		{
			probing();

			ident = TRUE;

			break;
		}

	case SV_ROD_CURING:
		{
			if (set_blind(0)) ident = TRUE;
			if (set_poisoned(0)) ident = TRUE;
			if (set_confused(0)) ident = TRUE;
			if (set_stun(0)) ident = TRUE;
			if (set_cut(0)) ident = TRUE;
			if (set_image(0)) ident = TRUE;

			break;
		}

	case SV_ROD_HEALING:
		{
			if (hp_player(500)) ident = TRUE;
			if (set_stun(0)) ident = TRUE;
			if (set_cut(0)) ident = TRUE;

			break;
		}

	case SV_ROD_RESTORATION:
		{
			if (restore_level()) ident = TRUE;
			if (do_res_stat(A_STR, TRUE)) ident = TRUE;
			if (do_res_stat(A_INT, TRUE)) ident = TRUE;
			if (do_res_stat(A_WIS, TRUE)) ident = TRUE;
			if (do_res_stat(A_DEX, TRUE)) ident = TRUE;
			if (do_res_stat(A_CON, TRUE)) ident = TRUE;
			if (do_res_stat(A_CHR, TRUE)) ident = TRUE;

			break;
		}

	case SV_ROD_SPEED:
		{
			if (!p_ptr->fast)
			{
				if (set_fast(randint(30) + 15, 10)) ident = TRUE;
			}
			else
			{
				(void)set_fast(p_ptr->fast + 5, 10);
			}

			break;
		}

	case SV_ROD_TELEPORT_AWAY:
		{
			if (teleport_monster(dir)) ident = TRUE;

			break;
		}

	case SV_ROD_DISARMING:
		{
			if (disarm_trap(dir)) ident = TRUE;

			break;
		}

	case SV_ROD_LITE:
		{
			msg_print("A line of blue shimmering light appears.");
			lite_line(dir);

			ident = TRUE;

			break;
		}

	case SV_ROD_SLEEP_MONSTER:
		{
			if (sleep_monster(dir)) ident = TRUE;

			break;
		}

	case SV_ROD_SLOW_MONSTER:
		{
			if (slow_monster(dir)) ident = TRUE;

			break;
		}

	case SV_ROD_DRAIN_LIFE:
		{
			if (drain_life(dir, 75)) ident = TRUE;

			break;
		}

	case SV_ROD_POLYMORPH:
		{
			if (poly_monster(dir)) ident = TRUE;

			break;
		}

	case SV_ROD_ACID_BOLT:
		{
			fire_bolt_or_beam(10, GF_ACID, dir, damroll(6, 8));

			ident = TRUE;

			break;
		}

	case SV_ROD_ELEC_BOLT:
		{
			fire_bolt_or_beam(10, GF_ELEC, dir, damroll(3, 8));

			ident = TRUE;

			break;
		}

	case SV_ROD_FIRE_BOLT:
		{
			fire_bolt_or_beam(10, GF_FIRE, dir, damroll(8, 8));

			ident = TRUE;

			break;
		}

	case SV_ROD_COLD_BOLT:
		{
			fire_bolt_or_beam(10, GF_COLD, dir, damroll(5, 8));

			ident = TRUE;

			break;
		}

	case SV_ROD_ACID_BALL:
		{
			fire_ball(GF_ACID, dir, 60, 2);

			ident = TRUE;

			break;
		}

	case SV_ROD_ELEC_BALL:
		{
			fire_ball(GF_ELEC, dir, 32, 2);

			ident = TRUE;

			break;
		}

	case SV_ROD_FIRE_BALL:
		{
			fire_ball(GF_FIRE, dir, 72, 2);

			ident = TRUE;

			break;
		}

	case SV_ROD_COLD_BALL:
		{
			fire_ball(GF_COLD, dir, 48, 2);

			ident = TRUE;

			break;
		}

	case SV_ROD_HAVOC:
		{
			call_chaos();

			ident = TRUE;

			break;
		}

	default:
		{
			process_hooks(HOOK_ZAP, "(d,d)", o_ptr->tval, o_ptr->sval);

			break;
		}
	}


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Tried the object */
	object_tried(o_ptr);

	/* Successfully determined the object function */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev >> 1)) / p_ptr->lev);
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Hack -- deal with cancelled zap */
	if (!use_charge)
	{
		o_ptr->timeout += cost;

		return;
	}
}




/*
 * Hook to determine if an object is activable
 */
static bool_ item_tester_hook_activate(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;


	/* Not known */
	if (!object_known_p(o_ptr)) return (FALSE);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Check activation flag */
	if (f3 & (TR3_ACTIVATE)) return (TRUE);

	/* Assume not */
	return (FALSE);
}



/*
 * Hack -- activate the ring of power
 */
int ring_of_power()
{
	char ch = 0, p = 0;

	int plev = p_ptr->lev;

	int timeout = 0;


	/* Select power to use */
	while (TRUE)
	{
		if (!get_com("[S]ummon a wraith, [R]ule the world or "
		                "[C]ast a powerful attack? ", &ch))
		{
			return (0);
		}

		if (ch == 'S' || ch == 's')
		{
			p = 1;
			break;
		}
		if (ch == 'R' || ch == 'r')
		{
			p = 2;
			break;
		}
		if (ch == 'C' || ch == 'c')
		{
			p = 3;
			break;
		}
	}

	/* Summon a Wraith */
	if (p == 1)
	{
		/* Rewrite this -- pelpel */
		if (summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
		                             (plev > 47 ? SUMMON_HI_UNDEAD_NO_UNIQUES : SUMMON_UNDEAD),
		                             (bool_)(((plev > 24) && (randint(3) == 1)) ? TRUE : FALSE)))
		{
			msg_print("Cold winds begin to blow around you, "
			          "carrying with them the stench of decay...");
			msg_print("Ancient, long-dead forms arise from the ground "
			          "to serve you!");
		}
		timeout = 200 + rand_int(200);
	}

	/* Rule the World -- only if we can really do so */
	else if (p == 2)
	{
		msg_print("The power of the ring destroys the world!");
		msg_print("The world changes!");

		autosave_checkpoint();

		/* Leaving */
		p_ptr->leaving = TRUE;
		timeout = 250 + rand_int(250);
	}

	/* Cast a powerful spell */
	else if (p == 3)
	{
		int dir;

		if (!get_aim_dir(&dir)) return (0);

		if (rand_int(3) == 0)
		{
			msg_print("You call the fire of Mount Doom!");
			fire_ball(GF_METEOR, dir, 600, 4);
		}
		else
		{
			msg_print("Your ring tries to take possession of your enemy's mind!");
			fire_bolt(GF_CHARM, dir, 600);
		}
		timeout = 300 + rand_int(300);
	}

	return (timeout);
}




/*
 * Enchant some bolts
 */
bool_ brand_bolts(void)
{
	int i;


	/* Use the first acceptable bolts */
	for (i = 0; i < INVEN_PACK; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-bolts */
		if (o_ptr->tval != TV_BOLT) continue;

		/* Skip artifacts and ego-items */
		if (o_ptr->art_name || artifact_p(o_ptr) || ego_item_p(o_ptr)) continue;

		/* Skip cursed/broken items */
		if (cursed_p(o_ptr)) continue;

		/* Randomize */
		if (rand_int(100) < 75) continue;

		/* Message */
		msg_print("Your bolts are covered in a fiery aura!");

		/* Ego-item */
		o_ptr->name2 = EGO_FLAME;

		/* Apply the ego */
		apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);

		/* Enchant */
		enchant(o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);

		/* Notice */
		return (TRUE);
	}

	/* Flush */
	if (flush_failure) flush();

	/* Fail */
	msg_print("The fiery enchantment failed.");

	/* Notice */
	return (TRUE);
}


/*
 * Objects in the p_ptr->inventory can now be activated, and
 * SOME of those may be able to stack (ego wands or something)
 * in any case, we can't know that it's impossible. *BUT* we'll
 * ignore it for now, and the timeout will be set on the entire stack
 * of objects. Reduces their utility, but oh well.
 *
 * Note that it always takes a turn to activate an object, even if
 * the user hits "escape" at the "direction" prompt.
 */
void do_cmd_activate(void)
{
	int item, lev, chance;

	char ch, spell_choice;

	object_type *o_ptr;

	u32b f1, f2, f3, f4, f5, esp;

	cptr q, s;


	/* Prepare the hook */
	item_tester_hook = item_tester_hook_activate;

	/* Get an item */
	command_wrk = USE_EQUIP;
	q = "Activate which item? ";
	s = "You have nothing to activate.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Extract object flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Wearable items have to be worn */
	if (!(f5 & TR5_ACTIVATE_NO_WIELD))
	{
		if (item < INVEN_WIELD)
		{
			msg_print("You must wear it to activate it.");
			return;
		}
	}

	/* Take a turn */
	energy_use = 100;

	/* Extract the item level */
	lev = k_info[o_ptr->k_idx].level;

	/* Hack -- Use artifact level instead */
	if (artifact_p(o_ptr))
	{
		if (o_ptr->tval == TV_RANDART)
		{
			lev = random_artifacts[o_ptr->sval].level;
		}
		else
		{
			lev = a_info[o_ptr->name1].level;
		}
	}

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* Hight level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	if (chance <= 0)
	{
		chance = 1;
	}

	/* Is it simple to use ? */
	if (f4 & TR4_EASY_USE)
	{
		chance *= 10;
	}

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE))
	{
		if (flush_failure) flush();
		msg_print("You failed to activate it properly.");
		sound(SOUND_FAIL);
		return;
	}

	/* Check the recharge */
	if (o_ptr->timeout)
	{
		/* Mage Staff of Spells -- Have another timeout in xtra2 */
		if (is_ego_p(o_ptr, EGO_MSTAFF_SPELL) && o_ptr->xtra2)
		{
			msg_print("It whines, glows and fades...");
			return;
		}

		/* Monster eggs */
		else if (o_ptr->tval == TV_EGG)
		{
			msg_print("You resume the development of the egg.");
			o_ptr->timeout = 0;

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);

			/* Success */
			return;
		}

		/* Normal activatable items */
		else
		{
			msg_print("It whines, glows and fades...");
			return;
		}
	}


	/* Activate the item */
	msg_print("You activate it...");

	/* Sound */
	sound(SOUND_ZAP);

	/* Lua hook ? -- go first to allow lua to override */
	if (process_hooks(HOOK_ACTIVATE, "(d)", item))
	{
		return;
	}

	/* New mostly unified activation code
	   This has to be early to allow artifacts to override normal items -- neil */

	if ( activation_aux(o_ptr, TRUE, item) == NULL )
	{
		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Success */
		return;
	}

	/* Mage Staff of Spells */
	if (is_ego_p(o_ptr, EGO_MSTAFF_SPELL))
	{
		while (TRUE)
		{
			if (!get_com("Use Spell [1] or [2]?", &ch))
			{
				return;
			}

			if (ch == '1')
			{
				spell_choice = 1;
				break;
			}

			if (ch == '2')
			{
				spell_choice = 2;
				break;
			}
		}

		if (spell_choice == 1)
		{
			/* Still need to check timeouts because there is another counter */
			if (o_ptr->timeout)
			{
				msg_print("The first spell is still charging!");
				return;
			}

			/* Cast spell 1 */
			activate_spell(o_ptr, spell_choice);
		}
		else if (spell_choice == 2)
		{
			/* Still need to check timeouts because there is another counter */
			if (o_ptr->xtra2)
			{
				msg_print("The second spell is still charging!");
				return;
			}

			/* Cast spell 2 */
			activate_spell(o_ptr, spell_choice);
		}

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Success */
		return;
	}

	/* Monster eggs */
	if (o_ptr->tval == TV_EGG)
	{
		msg_print("You stop the development of the egg.");
		o_ptr->timeout = -1;

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Success */
		return;
	}

	/* Musical instruments */
	if (o_ptr->tval == TV_INSTRUMENT)
	{
		/* Horns */
		if (o_ptr->sval == SV_HORN)
		{
			msg_format("Your instrument emits a loud sound!");

			aggravate_monsters(1);

			o_ptr->timeout = 100;
		}

		/* Success */
		return;
	}

	/* Mistake */
	msg_print("Oops.  That object cannot be activated.");
}

const char *activation_aux(object_type * o_ptr, bool_ doit, int item)
{
	static char buf[256];
	int plev = get_skill(SKILL_DEVICE);

	int i = 0, ii = 0, ij = 0, k, dir, dummy = 0;
	int chance;
	bool_ is_junkart = (o_ptr->tval == TV_RANDART);

	int spell = 0;

	/* Junkarts */
	if (is_junkart)
		spell = activation_info[o_ptr->pval2].spell;

	/* True Actifacts */
	if (!spell && o_ptr->name1)
		spell = a_info[o_ptr->name1].activate;

	/* Random and Alchemist Artifacts */
	if (!spell && o_ptr->art_name)
		spell = o_ptr->xtra2;

	/* Ego Items */
	if (!spell && o_ptr->name2)
		spell = e_info[o_ptr->name2].activate;

	/* Dual egos with the second ego having the activation */
	if (!spell && o_ptr->name2b)
		spell = e_info[o_ptr->name2b].activate;

	/* Intrinsic to item type (rings of Ice, etc) */
	if (!spell)
		spell = k_info[o_ptr->k_idx].activate;

	/* Complain about mis-configured .txt files? */
	if (!spell)
		return "Unknown!";

	/* Negative means a unified spell index */
	if (spell < 0)
	{
		struct spell_type *spell_ptr = spell_at(-spell);
		if (doit)
		{
			spell_type_produce_effect(spell_ptr, item);
			o_ptr->timeout = spell_type_activation_roll_timeout(spell_ptr);
		}
		else
		{
			spell_type_activation_description(spell_ptr, buf);
			return buf;
		}
	}
	else
	{
		/* Activate for attack */
		switch (spell)
		{
		case ACT_GILGALAD:
			{
				if (!doit) return "starlight (75) every 75+d75 turns";
				for (k = 1; k < 10; k++)
				{
					if (k - 5) fire_beam(GF_LITE, k, 75);
				}

				o_ptr->timeout = rand_int(75) + 75;

				break;
			}

		case ACT_CELEBRIMBOR:
			{
				if (!doit) return "temporary ESP (dur 20+d20) every 20+d50 turns";
				set_tim_esp(p_ptr->tim_esp + randint(20) + 20);

				o_ptr->timeout = rand_int(50) + 20;

				break;
			}

		case ACT_SKULLCLEAVER:
			{
				if (!doit) return "destruction every 200+d200 turns";
				destroy_area(p_ptr->py, p_ptr->px, 15, TRUE, FALSE);

				o_ptr->timeout = rand_int(200) + 200;

				break;
			}

		case ACT_HARADRIM:
			{
				if (!doit) return "berserk strength every 50+d50 turns";
				set_afraid(0);
				set_shero(p_ptr->shero + randint(25) + 25);
				hp_player(30);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}

		case ACT_FUNDIN:
			{
				if (!doit) return "dispel evil (x4) every 100+d100 turns";
				dispel_evil(p_ptr->lev * 4);

				o_ptr->timeout = rand_int(100) + 100;

				break;
			}

		case ACT_EOL:
			{
				if (!doit) return "mana bolt (9d8) 7+d7 turns";
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_MANA, dir, damroll(9, 8));

				o_ptr->timeout = rand_int(7) + 7;

				break;
			}

		case ACT_UMBAR:
			{
				if (!doit) return "magic arrow (10d10) every 20+d20 turns";
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_MISSILE, dir, damroll(10, 10));

				o_ptr->timeout = rand_int(20) + 20;

				break;
			}

		case ACT_NUMENOR:
			{
				/* Give full knowledge */
				/* Hack -- Maximal info */
				monster_race *r_ptr;
				cave_type *c_ptr;
				int x, y, m;

				if (!doit) return "analyze monster every 500+d200 turns";

				if (!tgt_pt(&x, &y)) break;

				c_ptr = &cave[y][x];
				if (!c_ptr->m_idx) break;

				r_ptr = &r_info[c_ptr->m_idx];

				/* Observe "maximal" attacks */
				for (m = 0; m < 4; m++)
				{
					/* Examine "actual" blows */
					if (r_ptr->blow[m].effect || r_ptr->blow[m].method)
					{
						/* Hack -- maximal observations */
						r_ptr->r_blows[m] = MAX_UCHAR;
					}
				}

				/* Hack -- maximal drops */
				r_ptr->r_drop_gold = r_ptr->r_drop_item =
				                             (((r_ptr->flags1 & (RF1_DROP_4D2)) ? 8 : 0) +
				                              ((r_ptr->flags1 & (RF1_DROP_3D2)) ? 6 : 0) +
				                              ((r_ptr->flags1 & (RF1_DROP_2D2)) ? 4 : 0) +
				                              ((r_ptr->flags1 & (RF1_DROP_1D2)) ? 2 : 0) +
				                              ((r_ptr->flags1 & (RF1_DROP_90)) ? 1 : 0) +
				                              ((r_ptr->flags1 & (RF1_DROP_60)) ? 1 : 0));

				/* Hack -- but only "valid" drops */
				if (r_ptr->flags1 & (RF1_ONLY_GOLD)) r_ptr->r_drop_item = 0;
				if (r_ptr->flags1 & (RF1_ONLY_ITEM)) r_ptr->r_drop_gold = 0;

				/* Hack -- observe many spells */
				r_ptr->r_cast_inate = MAX_UCHAR;
				r_ptr->r_cast_spell = MAX_UCHAR;

				/* Hack -- know all the flags */
				r_ptr->r_flags1 = r_ptr->flags1;
				r_ptr->r_flags2 = r_ptr->flags2;
				r_ptr->r_flags3 = r_ptr->flags3;
				r_ptr->r_flags4 = r_ptr->flags4;
				r_ptr->r_flags5 = r_ptr->flags5;
				r_ptr->r_flags6 = r_ptr->flags6;
				r_ptr->r_flags7 = r_ptr->flags7;
				r_ptr->r_flags8 = r_ptr->flags8;
				r_ptr->r_flags9 = r_ptr->flags9;

				o_ptr->timeout = rand_int(200) + 500;

				break;
			}

		case ACT_KNOWLEDGE:
			{
				if (!doit) return "whispers from beyond(sanity drain) 100+d200 turns";
				identify_fully();
				take_sanity_hit(damroll(10, 7), "the sounds of the dead");

				o_ptr->timeout = rand_int(200) + 100;

				break;
			}

		case ACT_UNDEATH:
			{
				if (!doit) return "ruination every 10+d10 turns";
				msg_print("The phial wells with dark light...");
				unlite_area(damroll(2, 15), 3);
				take_hit(damroll(10, 10), "activating The Phial of Undeath");
				(void)dec_stat(A_DEX, 25, STAT_DEC_PERMANENT);
				(void)dec_stat(A_WIS, 25, STAT_DEC_PERMANENT);
				(void)dec_stat(A_CON, 25, STAT_DEC_PERMANENT);
				(void)dec_stat(A_STR, 25, STAT_DEC_PERMANENT);
				(void)dec_stat(A_CHR, 25, STAT_DEC_PERMANENT);
				(void)dec_stat(A_INT, 25, STAT_DEC_PERMANENT);

				o_ptr->timeout = rand_int(10) + 10;

				break;
			}

		case ACT_THRAIN:
			{
				if (!doit) return "detection every 30+d30 turns";
				msg_print("The stone glows a deep green...");
				detect_all(DEFAULT_RADIUS);

				o_ptr->timeout = rand_int(30) + 30;

				break;
			}

		case ACT_BARAHIR:
			{
				if (!doit) return "dispel small life every 55+d55 turns";
				msg_print("You exterminate small life.");
				(void)dispel_monsters(4);

				o_ptr->timeout = rand_int(55) + 55;

				break;
			}

		case ACT_TULKAS:
			{
				if (!doit) return "haste self (75+d75 turns) every 150+d150 turns";
				msg_print("The ring glows brightly...");
				if (!p_ptr->fast)
				{
					(void)set_fast(randint(75) + 75, 10);
				}
				else
				{
					(void)set_fast(p_ptr->fast + 5, 10);
				}

				o_ptr->timeout = rand_int(150) + 150;

				break;
			}

		case ACT_NARYA:
			{
				if (!doit) return "healing (500) every 200+d100 turns";
				msg_print("The ring glows deep red...");
				hp_player(500);
				set_blind(0);
				set_confused(0);
				set_poisoned(0);
				set_stun(0);
				set_cut(0);

				o_ptr->timeout = rand_int(100) + 200;

				break;
			}

		case ACT_NENYA:
			{
				if (!doit) return "healing (800) every 100+d200 turns";
				msg_print("The ring glows bright white...");
				hp_player(800);
				set_blind(0);
				set_confused(0);
				set_poisoned(0);
				set_stun(0);
				set_cut(0);

				o_ptr->timeout = rand_int(200) + 100;

				break;
			}

		case ACT_VILYA:
			{
				if (!doit) return "greater healing (900) every 200+d200 turns";
				msg_print("The ring glows deep blue...");
				hp_player(900);
				set_blind(0);
				set_confused(0);
				set_poisoned(0);
				set_stun(0);
				set_cut(0);
				if (p_ptr->black_breath)
				{
					p_ptr->black_breath = FALSE;
					msg_print("The hold of the Black Breath on you is broken!");
				}

				o_ptr->timeout = rand_int(200) + 200;

				break;
			}

		case ACT_POWER:
			{
				if (!doit) return "powerful things";
				msg_print("The ring glows intensely black...");

				o_ptr->timeout = ring_of_power();

				break;
			}


			/* The Stone of Lore is perilous, for the sake of game balance. */
		case ACT_STONE_LORE:
			{
				if (!doit) return "perilous identify every turn";
				msg_print("The stone reveals hidden mysteries...");
				if (!ident_spell()) break;

				if (has_ability(AB_PERFECT_CASTING))
				{
					/* Sufficient mana */
					if (20 <= p_ptr->csp)
					{
						/* Use some mana */
						p_ptr->csp -= 20;
					}

					/* Over-exert the player */
					else
					{
						int oops = 20 - p_ptr->csp;

						/* No mana left */
						p_ptr->csp = 0;
						p_ptr->csp_frac = 0;

						/* Message */
						msg_print("You are too weak to control the stone!");

						/* Hack -- Bypass free action */
						(void)set_paralyzed(p_ptr->paralyzed +
						                    randint(5 * oops + 1));

						/* Confusing. */
						(void)set_confused(p_ptr->confused +
						                   randint(5 * oops + 1));
					}

					/* Redraw mana */
					p_ptr->redraw |= (PR_MANA);
				}

				take_hit(damroll(1, 12), "perilous secrets");

				/* Confusing. */
				if (rand_int(5) == 0)
				{
					(void)set_confused(p_ptr->confused + randint(10));
				}

				/* Exercise a little care... */
				if (rand_int(20) == 0)
				{
					take_hit(damroll(4, 10), "perilous secrets");
				}

				o_ptr->timeout = 1;

				break;
			}

		case ACT_RAZORBACK:
			{
				if (!doit) return "star ball (150) every 1000 turns";
				msg_print("Your armor is surrounded by lightning...");
				for (i = 0; i < 8; i++) fire_ball(GF_ELEC, ddd[i], 150, 3);

				o_ptr->timeout = 1000;

				break;
			}

		case ACT_BLADETURNER:
			{
				if (!doit) return "invulnerability (4+d8) every 800 turns";
				set_invuln(p_ptr->invuln + randint(8) + 4);

				o_ptr->timeout = 800;

				break;
			}

		case ACT_MEDIATOR:
			{
				if (!doit) return "breathe elements (300), berserk rage, bless, and resistance every 400 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe the elements.");
				fire_ball(GF_MISSILE, dir, 300, 4);
				msg_print("Your armor glows many colours...");
				(void)set_afraid(0);
				(void)set_shero(p_ptr->shero + randint(50) + 50);
				(void)hp_player(30);
				(void)set_blessed(p_ptr->blessed + randint(50) + 50);
				(void)set_oppose_acid(p_ptr->oppose_acid + randint(50) + 50);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint(50) + 50);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint(50) + 50);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint(50) + 50);
				(void)set_oppose_pois(p_ptr->oppose_pois + randint(50) + 50);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_BELEGENNON:
			{
				if (!doit) return ("heal (777), curing and heroism every 300 turns");
				msg_print("A heavenly choir sings...");
				(void)set_poisoned(0);
				(void)set_cut(0);
				(void)set_stun(0);
				(void)set_confused(0);
				(void)set_blind(0);
				(void)set_hero(p_ptr->hero + randint(25) + 25);
				(void)hp_player(777);

				o_ptr->timeout = 300;

				break;
			}

		case ACT_GORLIM:
			{
				if (!doit) return "rays of fear in every direction";
				turn_monsters(40 + p_ptr->lev);

				o_ptr->timeout = 3 * (p_ptr->lev + 10);

				break;
			}

		case ACT_COLLUIN:
			{
				if (!doit) return "resistance (20+d20 turns) every 111 turns";
				msg_print("Your cloak glows many colours...");
				(void)set_oppose_acid(p_ptr->oppose_acid + randint(20) + 20);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint(20) + 20);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);
				(void)set_oppose_pois(p_ptr->oppose_pois + randint(20) + 20);

				o_ptr->timeout = 111;

				break;
			}


		case ACT_BELANGIL:
			{
				if (!doit) return "frost ball (48) every 5+d5 turns";
				msg_print("Your dagger is covered in frost...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_COLD, dir, 48, 2);

				o_ptr->timeout = rand_int(5) + 5;

				break;
			}

		case ACT_ANGUIREL:
			{
				if (!doit) return "a getaway every 35 turns";
				switch (randint(13))
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					{
						teleport_player(10);

						break;
					}

				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					{
						teleport_player(222);

						break;
					}

				case 11:
				case 12:
					{
						(void)stair_creation();

						break;
					}

				default:
					{
						if (get_check("Leave this level? "))
						{
							autosave_checkpoint();

							/* Leaving */
							p_ptr->leaving = TRUE;
						}

						break;
					}
				}

				o_ptr->timeout = 35;

				break;
			}

		case ACT_ERU:
			{
				if (!doit) return "healing(7000), curing every 500 turns";
				msg_print("Your sword glows an intense white...");
				hp_player(7000);
				heal_insanity(50);
				set_blind(0);
				set_poisoned(0);
				set_confused(0);
				set_stun(0);
				set_cut(0);
				set_image(0);

				o_ptr->timeout = 500;

				break;
			}

		case ACT_DAWN:
			{
				if (!doit) return "summon the Legion of the Dawn every 500+d500 turns";
				msg_print("You summon the Legion of the Dawn.");
				(void)summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_DAWN, TRUE);

				o_ptr->timeout = 500 + randint(500);

				break;
			}

		case ACT_FIRESTAR:
			{
				if (!doit) return "large fire ball (72) every 100 turns";
				msg_print("Your morning star rages in fire...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_FIRE, dir, 72, 3);

				o_ptr->timeout = 100;

				break;
			}

		case ACT_TURMIL:
			{
				if (!doit) return "drain life (90) every 70 turns";
				msg_print("Your hammer glows white...");
				if (!get_aim_dir(&dir)) break;
				drain_life(dir, 90);

				o_ptr->timeout = 70;

				break;
			}

		case ACT_CUBRAGOL:
			{
				if (!doit) return "fire branding of bolts every 999 turns";
				msg_print("Your crossbow glows deep red...");
				(void)brand_bolts();

				o_ptr->timeout = 999;

				break;
			}

		case ACT_ELESSAR:
			{
				if (!doit) return "heal and cure black breath every 200 turns";
				if (p_ptr->black_breath)
				{
					msg_print("The hold of the Black Breath on you is broken!");
				}
				p_ptr->black_breath = FALSE;
				hp_player(100);

				o_ptr->timeout = 200;

				break;
			}

		case ACT_GANDALF:
			{
				if (!doit) return "restore mana every 666 turns";
				msg_print("Your mage staff glows deep blue...");
				if (p_ptr->csp < p_ptr->msp)
				{
					p_ptr->csp = p_ptr->msp;
					p_ptr->csp_frac = 0;
					msg_print("Your feel your head clear.");
					p_ptr->redraw |= (PR_MANA);
					p_ptr->window |= (PW_PLAYER);
				}

				o_ptr->timeout = 666;

				break;
			}

		case ACT_MARDA:
			{
				if (!doit) return "summon a thunderlord every 1000 turns";
				if (randint(3) == 1)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, ((plev * 3) / 2), SUMMON_THUNDERLORD))
					{
						msg_print("A Thunderlord comes from thin air!");
						msg_print("'I will burn you!'");
					}
				}
				else
				{
					if (summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
					                             SUMMON_THUNDERLORD, (bool_)(plev == 50 ? TRUE : FALSE)))
					{
						msg_print("A Thunderlord comes from thin air!");
						msg_print("'I will help you in your difficult task.'");
					}
				}

				o_ptr->timeout = 1000;

				break;
			}

		case ACT_PALANTIR:
			{
				if (!doit) return "clairvoyance every 100+d100 turns";
				msg_print("The stone glows a deep green...");
				wiz_lite_extra();
				(void)detect_traps(DEFAULT_RADIUS);
				(void)detect_doors(DEFAULT_RADIUS);
				(void)detect_stairs(DEFAULT_RADIUS);

				o_ptr->timeout = rand_int(100) + 100;

				break;
			}

		case ACT_EREBOR:
			{
				if (!doit) return "open a secret passage every 75 turns";
				msg_print("Your pick twists in your hands.");

				if (!get_aim_dir(&dir)) break;
				if (passwall(dir, TRUE))
				{
					msg_print("A passage opens, and you step through.");
				}
				else
				{
					msg_print("There is no wall there!");
				}

				o_ptr->timeout = 75;

				break;
			}

		case ACT_DRUEDAIN:
			{
				if (!doit) return "detection every 99 turns";
				msg_print("Your drum shows you the world.");
				detect_all(DEFAULT_RADIUS);

				o_ptr->timeout = 99;

				break;
			}

		case ACT_ROHAN:
			{
				if (!doit) return "heroism, berserker, and haste every 250 turns";
				msg_print("Your horn glows deep red.");
				set_afraid(0);
				set_shero(p_ptr->shero + damroll(5, 10) + 30);
				set_afraid(0);
				set_hero(p_ptr->hero + damroll(5, 10) + 30);
				set_fast(p_ptr->fast + damroll(5, 10) + 30, 10);
				hp_player(30);

				o_ptr->timeout = 250;

				break;
			}

		case ACT_HELM:
			{
				if (!doit) return "sound ball (300) every 300 turns";
				msg_print("Your horn emits a loud sound.");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_SOUND, dir, 300, 6);

				o_ptr->timeout = 300;

				break;
			}

		case ACT_BOROMIR:
			{
				if (!doit) return "mass human summoning every 1000 turns";
				msg_print("Your horn calls for help.");
				for (i = 0; i < 15; i++)
				{
					summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2), SUMMON_HUMAN, TRUE);
				}

				o_ptr->timeout = 1000;

				break;
			}

		case ACT_HURIN:
			{
				if (!doit) return "berserker and +10 to speed (50) every 100+d200 turns";
				if (!p_ptr->fast)
				{
					(void)set_fast(randint(50) + 50, 10);
				}
				else
				{
					(void)set_fast(p_ptr->fast + 5, 10);
				}
				hp_player(30);
				set_afraid(0);
				set_shero(p_ptr->shero + randint(50) + 50);

				o_ptr->timeout = rand_int(200) + 100;

				break;
			}

		case ACT_AXE_GOTHMOG:
			{
				if (!doit) return "fire ball (300) every 200+d200 turns";
				msg_print("Your lochaber axe erupts in fire...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_FIRE, dir, 300, 4);

				o_ptr->timeout = 200 + rand_int(200);

				break;
			}

		case ACT_MELKOR:
			{
				if (!doit) return "darkness ball (150) every 100 turns";
				msg_print("Your spear is covered of darkness...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_DARK, dir, 150, 3);

				o_ptr->timeout = 100;

				break;
			}

		case ACT_GROND:
			{
				if (!doit) return "alter reality every 100 turns";
				msg_print("Your hammer hits the floor...");
				alter_reality();

				o_ptr->timeout = 100;

				break;
			}

		case ACT_NATUREBANE:
			{
				if (!doit) return "dispel monsters (300) every 200+d200 turns";
				msg_print("Your axe glows blood red...");
				dispel_monsters(300);

				o_ptr->timeout = 200 + randint(200);

				break;
			}

		case ACT_NIGHT:
			{
				if (!doit) return "vampiric drain (3*100) every 250 turns";
				msg_print("Your axe emits a black aura...");
				if (!get_aim_dir(&dir)) break;
				for (i = 0; i < 3; i++)
				{
					if (drain_life(dir, 100)) hp_player(100);
				}

				o_ptr->timeout = 250;

				break;
			}

		case ACT_ORCHAST:
			{
				if (!doit) return "detect orcs every 10 turns";
				msg_print("Your weapon glows brightly...");
				(void)detect_monsters_xxx(RF3_ORC, DEFAULT_RADIUS);

				o_ptr->timeout = 10;

				break;
			}
		case ACT_SUNLIGHT:
			{
				if (!doit) return "beam of sunlight every 10 turns";

				if (!get_aim_dir(&dir)) break;
				msg_print("A line of sunlight appears.");
				lite_line(dir);

				o_ptr->timeout = 10;

				break;
			}

		case ACT_BO_MISS_1:
			{
				if (!doit) return "magic missile (2d6) every 2 turns";
				msg_print("It glows extremely brightly...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_MISSILE, dir, damroll(2, 6));

				o_ptr->timeout = 2;

				break;
			}

		case ACT_BA_POIS_1:
			{
				if (!doit) return "stinking cloud (12), rad. 3, every 4+d4 turns";
				msg_print("It throbs deep green...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_POIS, dir, 12, 3);

				o_ptr->timeout = rand_int(4) + 4;

				break;
			}

		case ACT_BO_ELEC_1:
			{
				if (!doit) return "lightning bolt (4d8) every 6+d6 turns";
				msg_print("It is covered in sparks...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_ELEC, dir, damroll(4, 8));

				o_ptr->timeout = rand_int(6) + 6;

				break;
			}

		case ACT_BO_ACID_1:
			{
				if (!doit) return "acid bolt (5d8) every 5+d5 turns";
				msg_print("It is covered in acid...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_ACID, dir, damroll(5, 8));

				o_ptr->timeout = rand_int(5) + 5;

				break;
			}

		case ACT_BO_COLD_1:
			{
				if (!doit) return "frost bolt (6d8) every 7+d7 turns";
				msg_print("It is covered in frost...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_COLD, dir, damroll(6, 8));

				o_ptr->timeout = rand_int(7) + 7;

				break;
			}

		case ACT_BO_FIRE_1:
			{
				if (!doit) return "fire bolt (9d8) every 8+d8 turns";
				msg_print("It is covered in fire...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_FIRE, dir, damroll(9, 8));

				o_ptr->timeout = rand_int(8) + 8;

				break;
			}

		case ACT_BA_COLD_1:
			{
				if (!doit) return "ball of cold (48) every 400 turns";
				msg_print("It is covered in frost...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_COLD, dir, 48, 2);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_BA_FIRE_1:
			{
				if (!doit) return "ball of fire (72) every 400 turns";
				msg_print("It glows an intense red...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_FIRE, dir, 72, 2);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_DRAIN_1:
			{
				if (!doit) return "drain life (100) every 100+d100 turns";
				msg_print("It glows black...");
				if (!get_aim_dir(&dir)) break;
				if (drain_life(dir, 100))

					o_ptr->timeout = rand_int(100) + 100;

				break;
			}

		case ACT_BA_COLD_2:
			{
				if (!doit) return "ball of cold (100) every 300 turns";
				msg_print("It glows an intense blue...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_COLD, dir, 100, 2);

				o_ptr->timeout = 300;

				break;
			}

		case ACT_BA_ELEC_2:
			{
				if (!doit) return "ball of lightning (100) every 500 turns";
				msg_print("It crackles with electricity...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_ELEC, dir, 100, 3);

				o_ptr->timeout = 500;

				break;
			}

		case ACT_DRAIN_2:
			{
				if (!doit) return "drain life (120) every 400 turns";
				msg_print("It glows black...");
				if (!get_aim_dir(&dir)) break;
				drain_life(dir, 120);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_VAMPIRE_1:
			{
				if (!doit) return "vampiric drain (3*50) every 400 turns";
				if (!get_aim_dir(&dir)) break;
				for (dummy = 0; dummy < 3; dummy++)
				{
					if (drain_life(dir, 50))
						hp_player(50);
				}

				o_ptr->timeout = 400;

				break;
			}

		case ACT_BO_MISS_2:
			{
				if (!doit) return "arrows (150) every 90+d90 turns";
				msg_print("It grows magical spikes...");
				if (!get_aim_dir(&dir)) break;
				fire_bolt(GF_ARROW, dir, 150);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BA_FIRE_2:
			{
				if (!doit) return "fire ball (120) every 225+d225 turns";
				msg_print("It glows deep red...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_FIRE, dir, 120, 3);

				o_ptr->timeout = rand_int(225) + 225;

				break;
			}

		case ACT_BA_COLD_3:
			{
				if (!doit) return "ball of cold (200) every 325+d325 turns";
				msg_print("It glows bright white...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_COLD, dir, 200, 3);

				o_ptr->timeout = rand_int(325) + 325;

				break;
			}

		case ACT_BA_ELEC_3:
			{
				if (!doit) return "Lightning Ball (250) every 425+d425 turns";
				msg_print("It glows deep blue...");
				if (!get_aim_dir(&dir)) break;
				fire_ball(GF_ELEC, dir, 250, 3);

				o_ptr->timeout = rand_int(425) + 425;

				break;
			}

		case ACT_WHIRLWIND:
			{
				int y = 0, x = 0;
				cave_type *c_ptr;
				monster_type *m_ptr;
				if (!doit) return "whirlwind attack every 250 turns";

				for (dir = 0; dir <= 9; dir++)
				{
					y = p_ptr->py + ddy[dir];
					x = p_ptr->px + ddx[dir];
					c_ptr = &cave[y][x];

					/* Get the monster */
					m_ptr = &m_list[c_ptr->m_idx];

					/* Hack -- attack monsters */
					if (c_ptr->m_idx && (m_ptr->ml || cave_floor_bold(y, x)))
					{
						py_attack(y, x, -1);
					}
				}

				o_ptr->timeout = 250;

				break;
			}

		case ACT_VAMPIRE_2:
			{
				if (!doit) return "vampiric drain (3*100) every 400 turns";
				if (!get_aim_dir(&dir)) break;
				for (dummy = 0; dummy < 3; dummy++)
				{
					if (drain_life(dir, 100))
						hp_player(100);
				}

				o_ptr->timeout = 400;

				break;
			}


		case ACT_CALL_CHAOS:
			{
				if (!doit) return "call chaos every 350 turns";
				msg_print("It glows in scintillating colours...");
				call_chaos();

				o_ptr->timeout = 350;

				break;
			}

		case ACT_ROCKET:
			{
				if (!doit) return "launch rocket (120+level) every 400 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You launch a rocket!");
				fire_ball(GF_ROCKET, dir, 120 + (plev), 2);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_DISP_EVIL:
			{
				if (!doit) return "dispel evil (level*5) every 300+d300 turns";
				msg_print("It floods the area with goodness...");
				dispel_evil(p_ptr->lev * 5);

				o_ptr->timeout = rand_int(300) + 300;

				break;
			}

		case ACT_DISP_GOOD:
			{
				if (!doit) return "dispel good (level*5) every 300+d300 turns";
				msg_print("It floods the area with evil...");
				dispel_good(p_ptr->lev * 5);

				o_ptr->timeout = rand_int(300) + 300;

				break;
			}

		case ACT_BA_MISS_3:
			{
				if (!doit) return "elemental breath (300) every 500 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe the elements.");
				fire_ball(GF_MISSILE, dir, 300, 4);

				o_ptr->timeout = 500;

				break;
			}

			/* Activate for other offensive action */

		case ACT_CONFUSE:
			{
				if (!doit) return "confuse monster every 15 turns";
				msg_print("It glows in scintillating colours...");
				if (!get_aim_dir(&dir)) break;
				confuse_monster(dir, 20);

				o_ptr->timeout = 15;

				break;
			}

		case ACT_SLEEP:
			{
				if (!doit) return "sleep nearby monsters every 55 turns";
				msg_print("It glows deep blue...");
				sleep_monsters_touch();

				o_ptr->timeout = 55;

				break;
			}

		case ACT_QUAKE:
			{
				if (!doit) return "earthquake (rad 10) every 50 turns";
				/* Prevent destruction of quest levels and town */
				if (!is_quest(dun_level) && dun_level)
				{
					earthquake(p_ptr->py, p_ptr->px, 10);
					o_ptr->timeout = 50;
				}

				break;
			}

		case ACT_TERROR:
			{
				if (!doit) return "terror every 3 * (level+10) turns";
				turn_monsters(40 + p_ptr->lev);

				o_ptr->timeout = 3 * (p_ptr->lev + 10);

				break;
			}

		case ACT_TELE_AWAY:
			{
				if (!doit) return "teleport away every 200 turns";
				if (!get_aim_dir(&dir)) break;
				(void)fire_beam(GF_AWAY_ALL, dir, plev);

				o_ptr->timeout = 200;

				break;
			}

		case ACT_BANISH_EVIL:
			{
				if (!doit) return "banish evil every 250+d250 turns";
				if (banish_evil(100))
				{
					msg_print("The power of the artifact banishes evil!");
				}

				o_ptr->timeout = 250 + randint(250);

				break;
			}

		case ACT_GENOCIDE:
			{
				if (!doit) return "genocide every 500 turns";
				msg_print("It glows deep blue...");
				(void)genocide(TRUE);

				o_ptr->timeout = 500;

				break;
			}

		case ACT_MASS_GENO:
			{
				if (!doit) return "mass genocide every 1000 turns";
				msg_print("It lets out a long, shrill note...");
				(void)mass_genocide(TRUE);

				o_ptr->timeout = 1000;

				break;
			}

			/* Activate for summoning / charming */

		case ACT_CHARM_ANIMAL:
			{
				if (!doit) return "charm animal every 300 turns";
				if (!get_aim_dir(&dir)) break;
				(void) charm_animal(dir, plev);

				o_ptr->timeout = 300;

				break;
			}

		case ACT_CHARM_UNDEAD:
			{
				if (!doit) return "enslave undead every 333 turns";
				if (!get_aim_dir(&dir)) break;
				(void)control_one_undead(dir, plev);

				o_ptr->timeout = 333;

				break;
			}

		case ACT_CHARM_OTHER:
			{
				if (!doit) return "charm monster every 400 turns";
				if (!get_aim_dir(&dir)) break;
				(void) charm_monster(dir, plev);

				o_ptr->timeout = 400;

				break;
			}

		case ACT_CHARM_ANIMALS:
			{
				if (!doit) return "animal friendship every 500 turns";
				(void) charm_animals(plev * 2);

				o_ptr->timeout = 500;

				break;
			}

		case ACT_CHARM_OTHERS:
			{
				if (!doit) return "mass charm every 750 turns";
				charm_monsters(plev * 2);

				o_ptr->timeout = 750;

				break;
			}

		case ACT_SUMMON_ANIMAL:
			{
				if (!doit) return "summon animal every 200+d300 turns";
				(void)summon_specific_friendly(p_ptr->py, p_ptr->px, plev, SUMMON_ANIMAL_RANGER, TRUE);

				o_ptr->timeout = 200 + randint(300);

				break;
			}

		case ACT_SUMMON_PHANTOM:
			{
				if (!doit) return "summon phantasmal servant every 200+d200 turns";
				msg_print("You summon a phantasmal servant.");
				(void)summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_PHANTOM, TRUE);

				o_ptr->timeout = 200 + randint(200);

				break;
			}

		case ACT_SUMMON_ELEMENTAL:
			{
				if (!doit) return "summon elemental every 750 turns";
				if (randint(3) == 1)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, ((plev * 3) / 2), SUMMON_ELEMENTAL))
					{
						msg_print("An elemental materialises...");
						msg_print("You fail to control it!");
					}
				}
				else
				{
					if (summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
					                             SUMMON_ELEMENTAL, (bool_)(plev == 50 ? TRUE : FALSE)))
					{
						msg_print("An elemental materialises...");
						msg_print("It seems obedient to you.");
					}
				}

				o_ptr->timeout = 750;

				break;
			}

		case ACT_SUMMON_DEMON:
			{
				if (!doit) return "summon demon every 666+d333 turns";
				if (randint(3) == 1)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, ((plev * 3) / 2), SUMMON_DEMON))
					{
						msg_print("The area fills with a stench of sulphur and brimstone.");
						msg_print("'NON SERVIAM! Wretch! I shall feast on thy mortal soul!'");
					}
				}
				else
				{
					if (summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
					                             SUMMON_DEMON, (bool_)(plev == 50 ? TRUE : FALSE)))
					{
						msg_print("The area fills with a stench of sulphur and brimstone.");
						msg_print("'What is thy bidding... Master?'");
					}
				}

				o_ptr->timeout = 666 + randint(333);

				break;
			}

		case ACT_SUMMON_UNDEAD:
			{
				if (!doit) return "summon undead every 666+d333 turns";
				if (randint(3) == 1)
				{
					if (summon_specific(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
					                    (plev > 47 ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD)))
					{
						msg_print("Cold winds begin to blow around you, carrying with them the stench of decay...");
						msg_print("'The dead arise... to punish you for disturbing them!'");
					}
				}
				else
				{
					if (summon_specific_friendly(p_ptr->py, p_ptr->px, ((plev * 3) / 2),
					                             (plev > 47 ? SUMMON_HI_UNDEAD_NO_UNIQUES : SUMMON_UNDEAD),
					                             (bool_)(((plev > 24) && (randint(3) == 1)) ? TRUE : FALSE)))
					{
						msg_print("Cold winds begin to blow around you, carrying with them the stench of decay...");
						msg_print("Ancient, long-dead forms arise from the ground to serve you!");
					}
				}

				o_ptr->timeout = 666 + randint(333);

				break;
			}

			/* Activate for healing */

		case ACT_CURE_LW:
			{
				if (!doit) return format("cure light wounds every %d turns", (is_junkart ? 50 : 10));
				(void)set_afraid(0);
				(void)hp_player(30);

				o_ptr->timeout = 10;

				break;
			}

		case ACT_CURE_MW:
			{
				if (!doit) return format("cure serious wounds every %s turns", (is_junkart? "75" : "3+d3"));
				msg_print("It radiates deep purple...");
				hp_player(damroll(4, 8));
				(void)set_cut((p_ptr->cut / 2) - 50);

				o_ptr->timeout = rand_int(3) + 3;

				break;
			}

		case ACT_CURE_POISON:
			{
				if (!doit) return "remove fear and cure poison every 5 turns";
				msg_print("It glows deep blue...");
				(void)set_afraid(0);
				(void)set_poisoned(0);

				o_ptr->timeout = 5;

				break;
			}

		case ACT_REST_LIFE:
			{
				if (!doit) return "restore life levels every 450 turns";
				msg_print("It glows a deep red...");
				restore_level();

				o_ptr->timeout = 450;

				break;
			}

		case ACT_REST_ALL:
			{
				if (!doit) return format("restore stats and life levels every %d turns", (is_junkart ? 200 : 750));
				msg_print("It glows a deep green...");
				(void)do_res_stat(A_STR, TRUE);
				(void)do_res_stat(A_INT, TRUE);
				(void)do_res_stat(A_WIS, TRUE);
				(void)do_res_stat(A_DEX, TRUE);
				(void)do_res_stat(A_CON, TRUE);
				(void)do_res_stat(A_CHR, TRUE);
				(void)restore_level();

				o_ptr->timeout = 750;

				break;
			}

		case ACT_CURE_700:
			{
				if (!doit) return format("heal 700 hit points every %d turns", (is_junkart ? 100 : 250));
				msg_print("It glows deep blue...");
				msg_print("You feel a warm tingling inside...");
				(void)hp_player(700);
				(void)set_cut(0);

				o_ptr->timeout = 250;

				break;
			}

		case ACT_CURE_1000:
			{
				if (!doit) return "heal 1000 hit points every 888 turns";
				msg_print("It glows a bright white...");
				msg_print("You feel much better...");
				(void)hp_player(1000);
				(void)set_cut(0);

				o_ptr->timeout = 888;

				break;
			}

		case ACT_ESP:
			{
				if (!doit) return "temporary ESP (dur 25+d30) every 200 turns";
				(void)set_tim_esp(p_ptr->tim_esp + randint(30) + 25);

				o_ptr->timeout = 200;

				break;
			}

		case ACT_BERSERK:
			{
				if (!doit) return "heroism and berserk (dur 50+d50) every 100+d100 turns";
				(void)set_shero(p_ptr->shero + randint(50) + 50);
				(void)set_blessed(p_ptr->blessed + randint(50) + 50);

				o_ptr->timeout = 100 + randint(100);

				break;
			}

		case ACT_PROT_EVIL:
			{
				if (!doit) return "protection from evil (dur level*3 + d25) every 225+d225 turns";
				msg_print("It lets out a shrill wail...");
				k = 3 * p_ptr->lev;
				(void)set_protevil(p_ptr->protevil + randint(25) + k);

				o_ptr->timeout = rand_int(225) + 225;

				break;
			}

		case ACT_RESIST_ALL:
			{
				if (!doit) return "resist elements (dur 40+d40) every 200 turns";
				msg_print("It glows many colours...");
				(void)set_oppose_acid(p_ptr->oppose_acid + randint(40) + 40);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint(40) + 40);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint(40) + 40);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint(40) + 40);
				(void)set_oppose_pois(p_ptr->oppose_pois + randint(40) + 40);

				o_ptr->timeout = 200;

				break;
			}

		case ACT_SPEED:
			{
				if (!doit) return "speed (dur 20+d20) every 250 turns";
				msg_print("It glows bright green...");
				if (!p_ptr->fast)
				{
					(void)set_fast(randint(20) + 20, 10);
				}
				else
				{
					(void)set_fast(p_ptr->fast + 5, 10);
				}

				o_ptr->timeout = 250;

				break;
			}

		case ACT_XTRA_SPEED:
			{
				if (!doit) return "speed (dur 75+d75) every 200+d200 turns";
				msg_print("It glows brightly...");
				if (!p_ptr->fast)
				{
					(void)set_fast(randint(75) + 75, 10);
				}
				else
				{
					(void)set_fast(p_ptr->fast + 5, 10);
				}

				o_ptr->timeout = rand_int(200) + 200;

				break;
			}

		case ACT_WRAITH:
			{
				if (!doit) return "wraith form (level/2 + d(level/2)) every 1000 turns";
				set_shadow(p_ptr->tim_wraith + randint(plev / 2) + (plev / 2));

				o_ptr->timeout = 1000;

				break;
			}

		case ACT_INVULN:
			{
				if (!doit) return "invulnerability (dur 8+d8) every 1000 turns";
				(void)set_invuln(p_ptr->invuln + randint(8) + 8);

				o_ptr->timeout = 1000;

				break;
			}

			/* Activate for general purpose effect (detection etc.) */

		case ACT_LIGHT:
			{
				if (!doit) return format("light area (dam 2d15) every %s turns", (is_junkart ? "100" : "10+d10"));
				msg_print("It wells with clear light...");
				lite_area(damroll(2, 15), 3);

				o_ptr->timeout = rand_int(10) + 10;

				break;
			}

		case ACT_MAP_LIGHT:
			{
				if (!doit) return "light (dam 2d15) & map area every 50+d50 turns";
				msg_print("It shines brightly...");
				map_area();
				lite_area(damroll(2, 15), 3);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}

		case ACT_DETECT_ALL:
			{
				if (!doit) return "detection every 55+d55 turns";
				msg_print("It glows bright white...");
				msg_print("An image forms in your mind...");
				detect_all(DEFAULT_RADIUS);

				o_ptr->timeout = rand_int(55) + 55;

				break;
			}

		case ACT_DETECT_XTRA:
			{
				if (!doit) return "detection, probing and identify true every 1000 turns";
				msg_print("It glows brightly...");
				detect_all(DEFAULT_RADIUS);
				probing();
				identify_fully();

				o_ptr->timeout = 1000;

				break;
			}

		case ACT_ID_FULL:
			{
				if (!doit) return "identify true every 750 turns";
				msg_print("It glows yellow...");
				identify_fully();

				o_ptr->timeout = 750;

				break;
			}

		case ACT_ID_PLAIN:
			{
				if (!doit) return "identify spell every 10 turns";
				if (!ident_spell()) break;

				o_ptr->timeout = 10;

				break;
			}

		case ACT_RUNE_EXPLO:
			{
				if (!doit) return "explosive rune every 200 turns";
				msg_print("It glows bright red...");
				explosive_rune();

				o_ptr->timeout = 200;

				break;
			}

		case ACT_RUNE_PROT:
			{
				if (!doit) return "rune of protection every 400 turns";
				msg_print("It glows light blue...");
				warding_glyph();

				o_ptr->timeout = 400;

				break;
			}

		case ACT_SATIATE:
			{
				if (!doit) return "satisfy hunger every 200 turns";
				(void)set_food(PY_FOOD_MAX - 1);

				o_ptr->timeout = 200;

				break;
			}

		case ACT_DEST_DOOR:
			{
				if (!doit) return "destroy doors and traps every 10 turns";
				msg_print("It glows bright red...");
				destroy_doors_touch();

				o_ptr->timeout = 10;

				break;
			}

		case ACT_STONE_MUD:
			{
				if (!doit) return "stone to mud every 5 turns";
				msg_print("It pulsates...");
				if (!get_aim_dir(&dir)) break;
				wall_to_mud(dir);

				o_ptr->timeout = 5;

				break;
			}

		case ACT_RECHARGE:
			{
				if (!doit) return "recharging every 70 turns";
				recharge(60);

				o_ptr->timeout = 70;

				break;
			}

		case ACT_ALCHEMY:
			{
				if (!doit) return "alchemy every 500 turns";
				msg_print("It glows bright yellow...");
				(void) alchemy();

				o_ptr->timeout = 500;

				break;
			}

		case ACT_DIM_DOOR:
			{
				if (!doit) return "dimension door every 100 turns";
				if (dungeon_flags2 & DF2_NO_TELEPORT)
				{
					msg_print("Not on special levels!");
					break;
				}

				msg_print("You open a Void Jumpgate. Choose a destination.");
				if (!tgt_pt(&ii, &ij)) break;

				p_ptr->energy -= 60 - plev;

				if (!cave_empty_bold(ij, ii) || (cave[ij][ii].info & CAVE_ICKY) ||
				                (distance(ij, ii, p_ptr->py, p_ptr->px) > plev + 2) ||
				                (!rand_int(plev * plev / 2)))
				{
					msg_print("You fail to exit the void correctly!");
					p_ptr->energy -= 100;
					get_pos_player(10, &ij, &ii);
				}

				cave_set_feat(p_ptr->py, p_ptr->px, FEAT_BETWEEN);
				cave_set_feat(ij, ii, FEAT_BETWEEN);
				cave[p_ptr->py][p_ptr->px].special = ii + (ij << 8);
				cave[ij][ii].special = p_ptr->px + (p_ptr->py << 8);

				o_ptr->timeout = 100;

				break;
			}

		case ACT_TELEPORT:
			{
				if (!doit) return format("teleport (range 100) every %d turns", (is_junkart? 100 : 45));
				msg_print("It twists space around you...");
				teleport_player(100);

				o_ptr->timeout = 45;

				break;
			}

		case ACT_RECALL:
			{
				if (!(dungeon_flags2 & DF2_ASK_LEAVE) || ((dungeon_flags2 & DF2_ASK_LEAVE) && !get_check("Leave this unique level forever? ")))
				{
					if (!doit) return "word of recall every 200 turns";
					msg_print("It glows soft white...");
					recall_player(20,15);

					o_ptr->timeout = 200;
				}

				break;
			}

		case ACT_DEATH:
			{
				if (!doit) return "death";
				take_hit(5000, "activating a death spell");

				/* Timeout is set before return */

				break;
			}

		case ACT_RUINATION:
			{
				if (!doit) return "Ruination";
				msg_print("Your nerves and muscles feel weak and lifeless!");

				take_hit(damroll(10, 10), "activating Ruination");
				(void)dec_stat(A_DEX, 25, TRUE);
				(void)dec_stat(A_WIS, 25, TRUE);
				(void)dec_stat(A_CON, 25, TRUE);
				(void)dec_stat(A_STR, 25, TRUE);
				(void)dec_stat(A_CHR, 25, TRUE);
				(void)dec_stat(A_INT, 25, TRUE);

				/* Timeout is set before return */

				break;
			}

		case ACT_DESTRUC:
			{
				if (!doit) return "Destruction every 100 turns";
				earthquake(p_ptr->py, p_ptr->px, 12);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNINT:
			{
				if (!doit) return "decreasing Intelligence";
				(void)dec_stat(A_INT, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNSTR:
			{
				if (!doit) return "decreasing Strength";
				(void)dec_stat(A_STR, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNCON:
			{
				if (!doit) return "decreasing Constitution";
				(void)dec_stat(A_CON, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNCHR:
			{
				if (!doit) return "decreasing Charisma";
				(void)dec_stat(A_CHR, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNDEX:
			{
				if (!doit) return "decreasing Dexterity";
				(void)dec_stat(A_DEX, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_UNWIS:
			{
				if (!doit) return "decreasing Wisdom";
				(void)dec_stat(A_WIS, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_STATLOSS:
			{
				if (!doit) return "stat loss";
				(void)dec_stat(A_STR, 15, FALSE);
				(void)dec_stat(A_INT, 15, FALSE);
				(void)dec_stat(A_WIS, 15, FALSE);
				(void)dec_stat(A_DEX, 15, FALSE);
				(void)dec_stat(A_CON, 15, FALSE);
				(void)dec_stat(A_CHR, 15, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_HISTATLOSS:
			{
				if (!doit) return "high stat loss";
				(void)dec_stat(A_STR, 25, FALSE);
				(void)dec_stat(A_INT, 25, FALSE);
				(void)dec_stat(A_WIS, 25, FALSE);
				(void)dec_stat(A_DEX, 25, FALSE);
				(void)dec_stat(A_CON, 25, FALSE);
				(void)dec_stat(A_CHR, 25, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_EXPLOSS:
			{
				if (!doit) return "experience loss";
				lose_exp(p_ptr->exp / 20);

				/* Timeout is set before return */

				break;
			}

		case ACT_HIEXPLOSS:
			{
				if (!doit) return "high experience loss";
				lose_exp(p_ptr->exp / 10);

				/* Timeout is set before return */

				break;
			}

		case ACT_SUMMON_MONST:
			{
				if (!doit) return "summon monster";
				summon_specific(p_ptr->py, p_ptr->px, max_dlv[dungeon_type], 0);

				/* Timeout is set before return */

				break;
			}

		case ACT_PARALYZE:
			{
				if (!doit) return "paralyze";
				set_paralyzed(p_ptr->paralyzed + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_HALLU:
			{
				if (!doit) return "hallucination every 10 turns";
				set_image(p_ptr->image + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_POISON:
			{
				if (!doit) return "poison";
				set_poisoned(p_ptr->poisoned + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_HUNGER:
			{
				if (!doit) return "create hunger";
				(void)set_food(PY_FOOD_WEAK);

				/* Timeout is set before return */

				break;
			}

		case ACT_STUN:
			{
				if (!doit) return "stun";
				set_stun(p_ptr->stun + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_CUTS:
			{
				if (!doit) return "cuts";
				set_cut(p_ptr->cut + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_PARANO:
			{
				if (!doit) return "confusion";
				set_confused(p_ptr->confused + 30 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_CONFUSION:
			{
				if (!doit) return "confusion";
				set_confused(p_ptr->confused + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_BLIND:
			{
				if (!doit) return "blindness";
				set_blind(p_ptr->blind + 20 + randint(10));

				/* Timeout is set before return */

				break;
			}

		case ACT_PET_SUMMON:
			{
				if (!doit) return "summon pet every 101 turns";
				summon_specific_friendly(p_ptr->py, p_ptr->px, max_dlv[dungeon_type], 0, FALSE);

				/* Timeout is set before return */
				/*FINDME*/

				break;
			}

		case ACT_CURE_PARA:
			{
				if (!doit) return "cure confusion every 500 turns";
				set_confused(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_HALLU:
			{
				if (!doit) return "cure hallucination every 100 turns";
				set_image(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_POIS:
			{
				if (!doit) return "cure poison every 100 turns";
				set_poisoned(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_HUNGER:
			{
				if (!doit) return "satisfy hunger every 100 turns";
				(void)set_food(PY_FOOD_MAX - 1);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_STUN:
			{
				if (!doit) return "cure stun every 100 turns";
				set_stun(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_CUTS:
			{
				if (!doit) return "cure cuts every 100 turns";
				set_cut(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_FEAR:
			{
				if (!doit) return "cure fear every 100 turns";
				set_afraid(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_CONF:
			{
				if (!doit) return "cure confusion every 100 turns";
				set_confused(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_BLIND:
			{
				if (!doit) return "cure blindness every 100 turns";
				set_blind(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_CURING:
			{
				if (!doit) return "curing every 110 turns";
				set_blind(0);
				set_poisoned(0);
				set_confused(0);
				set_stun(0);
				set_cut(0);
				set_image(0);

				/* Timeout is set before return */

				break;
			}

		case ACT_DARKNESS:
			{
				if (!doit) return "darkness";
				unlite_area(damroll(2, 10), 10);

				/* Timeout is set before return */

				break;
			}

		case ACT_LEV_TELE:
			{
				if (!doit) return "teleport level every 50 turns";
				teleport_player_level();

				/* Timeout is set before return */

				break;
			}

		case ACT_ACQUIREMENT:
			{
				if (!doit) return "acquirement every 3000 turns";
				acquirement(p_ptr->py, p_ptr->px, 1, FALSE, FALSE);

				/* Timeout is set before return */

				break;
			}

		case ACT_WEIRD:
			{
				if (!doit) return "something weird every 5 turns";
				/* It doesn't do anything */

				/* Timeout is set before return */

				break;
			}

		case ACT_AGGRAVATE:
			{
				if (!doit) return "aggravate";
				aggravate_monsters(1);

				/* Timeout is set before return */

				break;
			}

		case ACT_MUT:
			{
				if (!doit) return "gain corruption every 10 turns";
				gain_random_corruption();
				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_INSANITY:
			{
				if (!doit) return "cure insanity every 200 turns";
				heal_insanity(damroll(10, 10));

				/* Timeout is set before return */

				break;
			}

		case ACT_CURE_MUT:
			{
				msg_print("Ahah, you wish.");
				/* Timeout is set before return */

				break;
			}

		case ACT_LIGHT_ABSORBTION:
			{
				int y, x, light = 0, dir;
				cave_type *c_ptr;

				if (!doit) return "light absorption every 80 turns";

				for (y = p_ptr->py - 6; y <= p_ptr->py + 6; y++)
				{
					for (x = p_ptr->px - 6; x <= p_ptr->px + 6; x++)
					{
						if (!in_bounds(y, x)) continue;

						c_ptr = &cave[y][x];

						if (distance(y, x, p_ptr->py, p_ptr->px) > 6) continue;

						if (c_ptr->info & CAVE_GLOW)
						{
							light++;

							/* No longer in the array */
							c_ptr->info &= ~(CAVE_TEMP);

							/* Darken the grid */
							c_ptr->info &= ~(CAVE_GLOW);

							/* Hack -- Forget "boring" grids */
							if (cave_plain_floor_grid(c_ptr) &&
							                !(c_ptr->info & (CAVE_TRDT)))
							{
								/* Forget the grid */
								c_ptr->info &= ~(CAVE_MARK);

								/* Notice */
								note_spot(y, x);
							}

							/* Process affected monsters */
							if (c_ptr->m_idx)
							{
								/* Update the monster */
								update_mon(c_ptr->m_idx, FALSE);
							}

							/* Redraw */
							lite_spot(y, x);
						}
					}
				}

				if (!get_aim_dir(&dir)) return (FALSE);

				msg_print("The light around you is absorbed... "
				          "and released in a powerful bolt!");
				fire_bolt(GF_LITE, dir, damroll(light, p_ptr->lev));

				/* Timeout is set before return */

				break;
			}
			/* Horns of DragonKind (Note that these are new egos)*/
		case ACT_BA_FIRE_H:
			{
				if (!doit) return "large fire ball (300) every 100 turns";
				fire_ball(GF_FIRE, 5, 300, 7);

				o_ptr->timeout = 100;

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				break;
			}
		case ACT_BA_COLD_H:
			{
				if (!doit) return "large cold ball (300) every 100 turns";
				fire_ball(GF_COLD, 5, 300, 7);

				o_ptr->timeout = 100;

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				break;
			}
		case ACT_BA_ELEC_H:
			{
				if (!doit) return "large lightning ball (300) every 100 turns";
				fire_ball(GF_ELEC, 5, 300, 7);

				o_ptr->timeout = 100;

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				break;
			}
		case ACT_BA_ACID_H:
			{
				if (!doit) return "large acid ball (300) every 100 turns";
				fire_ball(GF_ACID, 5, 300, 7);

				o_ptr->timeout = 100;

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				break;
			}

		case ACT_SPIN:
			{
				if (!doit) return "spinning around every 50+d25 turns";
				do_spin();

				o_ptr->timeout = 50 + randint(25);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				/* Done */
				break;
			}
		case ACT_NOLDOR:
			{
				if (!doit) return "detect treasure every 10+d20 turns";
				detect_treasure(DEFAULT_RADIUS);

				o_ptr->timeout = 10 + randint(20);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				/* Done */
				break;
			}
		case ACT_SPECTRAL:
			{
				if (!doit) return "wraith-form every 50+d50 turns";
				if (!p_ptr->wraith_form)
				{
					set_shadow(20 + randint(20));
				}
				else
				{
					set_shadow(p_ptr->tim_wraith + randint(20));
				}

				o_ptr->timeout = 50 + randint(50);

				/* Window stuff */
				p_ptr->window |= PW_INVEN | PW_EQUIP;

				/* Done */
				break;
			}
		case ACT_JUMP:
			{
				if (!doit) return "phasing every 10+d10 turns";
				teleport_player(10);
				o_ptr->timeout = 10 + randint(10);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP);

				/* Done */
				break;
			}

		case ACT_DEST_TELE:
			{
				if (!doit) return "teleportation and destruction of the ring";
				if (!item)
				{
					msg_print("You can't activate this when it's there!");
				}
				if (get_check("This will destroy the ring. Do you wish to continue? "))
				{
					msg_print("The ring explodes into a space distortion.");
					teleport_player(200);

					/* It explodes, doesn't it ? */
					take_hit(damroll(2, 10), "an exploding ring");

					inc_stack_size_ex(item, -255, OPTIMIZE, NO_DESCRIBE);
				}

				break;
			}
			/*amulet of serpents dam 100, rad 2 timeout 40+d60 */
		case ACT_BA_POIS_4:
			{
				if (!doit) return "venom breathing every 40+d60 turns";
				/* Get a direction for breathing (or abort) */
				if (!get_aim_dir(&dir)) break;

				msg_print("You breathe venom...");
				fire_ball(GF_POIS, dir, 100, 2);

				o_ptr->timeout = rand_int(60) + 40;

				/* Window stuff */
				p_ptr->window |= PW_INVEN | PW_EQUIP;

				/* Done */
				break;
			}
			/*rings of X 50,50+d50 dur 20+d20 */
		case ACT_BA_COLD_4:
			{
				if (!doit) return "ball of cold and resist cold every 50+d50 turns";
				/* Get a direction for breathing (or abort) */
				if (!get_aim_dir(&dir)) break;

				fire_ball(GF_COLD, dir, 50, 2);
				(void)set_oppose_cold(p_ptr->oppose_cold + randint(20) + 20);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}

		case ACT_BA_FIRE_4:
			{
				if (!doit) return "ball of fire and resist fire every 50+d50 turns";
				/* Get a direction for breathing (or abort) */
				if (!get_aim_dir(&dir)) break;

				fire_ball(GF_FIRE, dir, 50, 2);
				(void)set_oppose_fire(p_ptr->oppose_fire + randint(20) + 20);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}
		case ACT_BA_ACID_4:
			{
				if (!doit) return "ball of acid and resist acid every 50+d50 turns";
				/* Get a direction for breathing (or abort) */
				if (!get_aim_dir(&dir)) break;

				fire_ball(GF_ACID, dir, 50, 2);
				(void)set_oppose_acid(p_ptr->oppose_acid + randint(20) + 20);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}

		case ACT_BA_ELEC_4:
			{
				if (!doit) return "ball of lightning and resist lightning every 50+d50 turns";
				/* Get a direction for breathing (or abort) */
				if (!get_aim_dir(&dir)) break;

				fire_ball(GF_ELEC, dir, 50, 2);
				(void)set_oppose_elec(p_ptr->oppose_elec + randint(20) + 20);

				o_ptr->timeout = rand_int(50) + 50;

				break;
			}

		case ACT_BR_ELEC:
			{
				if (!doit) return "breathe lightning (100) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe lightning.");
				fire_ball(GF_ELEC, dir, 100, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_COLD:
			{
				if (!doit) return "breathe frost (110) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe frost.");
				fire_ball(GF_COLD, dir, 110, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_FIRE:
			{
				if (!doit) return "breathe fire (200) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe fire.");
				fire_ball(GF_FIRE, dir, 200, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_ACID:
			{
				if (!doit) return "breathe acid (130) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe acid.");
				fire_ball(GF_ACID, dir, 130, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_POIS:
			{
				if (!doit) return "breathe poison gas (150) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe poison gas.");
				fire_ball(GF_POIS, dir, 150, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_MANY:
			{
				if (!doit) return "breathe multi-hued (250) every 60+d60 turns";
				if (!get_aim_dir(&dir)) break;
				chance = rand_int(5);
				msg_format("You breathe %s.",
				           ((chance == 1) ? "lightning" :
				            ((chance == 2) ? "frost" :
				             ((chance == 3) ? "acid" :
				              ((chance == 4) ? "poison gas" : "fire")))));
				fire_ball(((chance == 1) ? GF_ELEC :
				           ((chance == 2) ? GF_COLD :
				            ((chance == 3) ? GF_ACID :
				             ((chance == 4) ? GF_POIS : GF_FIRE)))),
				          dir, 250, 2);

				o_ptr->timeout = rand_int(60) + 60;

				break;
			}

		case ACT_BR_CONF:
			{
				if (!doit) return "breathe confusion (120) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe confusion.");
				fire_ball(GF_CONFUSION, dir, 120, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_SOUND:
			{
				if (!doit) return "breathe sound (130) every 90+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe sound.");
				fire_ball(GF_SOUND, dir, 130, 2);

				o_ptr->timeout = rand_int(90) + 90;

				break;
			}

		case ACT_BR_CHAOS:
			{
				if (!doit) return "breathe chaos/disenchant (220) every 60+d90 turns";
				if (!get_aim_dir(&dir)) break;
				chance = rand_int(2);
				msg_format("You breathe %s.",
				           ((chance == 1 ? "chaos" : "disenchantment")));
				fire_ball((chance == 1 ? GF_CHAOS : GF_DISENCHANT),
				          dir, 220, 2);

				o_ptr->timeout = rand_int(90) + 60;

				break;
			}

		case ACT_BR_SHARD:
			{
				if (!doit) return "breathe sound/shards (230) every 60+d90 turns";
				if (!get_aim_dir(&dir)) break;
				chance = rand_int(2);
				msg_format("You breathe %s.",
				           ((chance == 1 ? "sound" : "shards")));
				fire_ball((chance == 1 ? GF_SOUND : GF_SHARDS),
				          dir, 230, 2);

				o_ptr->timeout = rand_int(90) + 60;

				break;
			}

		case ACT_BR_BALANCE:
			{
				if (!doit) return "breathe balance (250) every 60+d90 turns";
				if (!get_aim_dir(&dir)) break;
				chance = rand_int(4);
				msg_format("You breathe %s.",
				           ((chance == 1) ? "chaos" :
				            ((chance == 2) ? "disenchantment" :
				             ((chance == 3) ? "sound" : "shards"))));
				fire_ball(((chance == 1) ? GF_CHAOS :
				           ((chance == 2) ? GF_DISENCHANT :
				            ((chance == 3) ? GF_SOUND : GF_SHARDS))),
				          dir, 250, 2);

				o_ptr->timeout = rand_int(90) + 60;

				break;
			}

		case ACT_BR_LIGHT:
			{
				if (!doit) return "breathe light/darkness (200) every 60+d90 turns";
				if (!get_aim_dir(&dir)) break;
				chance = rand_int(2);
				msg_format("You breathe %s.",
				           ((chance == 0 ? "light" : "darkness")));
				fire_ball((chance == 0 ? GF_LITE : GF_DARK), dir, 200, 2);

				o_ptr->timeout = rand_int(90) + 60;

				break;
			}
		case ACT_BR_POWER:
			{
				if (!doit) return "breathe the elements (300) every 60+d90 turns";
				if (!get_aim_dir(&dir)) break;
				msg_print("You breathe the elements.");
				fire_ball(GF_MISSILE, dir, 300, 3);

				o_ptr->timeout = rand_int(90) + 60;

				break;
			}
		case ACT_GROW_MOLD:
			{
				if (!doit) return "grow mushrooms every 50+d50 turns";
				msg_print("You twirl and spores fly everywhere!");
				for (i = 0; i < 8; i++)
					summon_specific_friendly(p_ptr->py, p_ptr->px, p_ptr->lev, SUMMON_BIZARRE1, FALSE);

				o_ptr->timeout = randint(50) + 50;

				break;
			}
		case ACT_MUSIC:
			/*
			 fall through to unknown, as music should be
			 handled by calling procedure.
			 */

		default:
			{
				msg_format("Unknown activation effect: %d.", spell);
				if ( !doit ) return "Unknown Activation";
				return NULL;
			}
		}
	}

	/* Set timeout for junkarts
	 * Note that I still need to set the timeouts for other
	 * (non-random) artifacts above 
	 */
	if (is_junkart && doit)
		o_ptr->timeout = activation_info[o_ptr->pval2].cost / 10;

	return NULL;
}


static bool_ activate_spell(object_type * o_ptr, byte choice)
{
	int mana = 0, gf = 0, mod = 0;

	rune_spell s_ptr;


	if (choice == 1)
	{
		gf = o_ptr->pval & 0xFFFF;
		mod = o_ptr->pval3 & 0xFFFF;
		mana = o_ptr->pval2 & 0xFF;
	}
	else if (choice == 2)
	{
		gf = o_ptr->pval >> 16;
		mod = o_ptr->pval3 >> 16;
		mana = o_ptr->pval2 >> 8;
	}

	s_ptr.type = gf;
	s_ptr.rune2 = 1 << mod;
	s_ptr.mana = mana;

	/* Execute */
	rune_exec(&s_ptr, 0);

	if (choice == 1) o_ptr->timeout = mana * 5;
	if (choice == 2) o_ptr->xtra2 = mana * 5;

	return (TRUE);
}
