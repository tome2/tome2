/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "melee1.hpp"

#include "cave.hpp"
#include "cmd5.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "store.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::iequals;

/*
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (rand_int(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
	{
		while (rand_int(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}





/*
 * Determine if a monster attack against the player succeeds.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int check_hit(int power, int level)
{
	int i, k, ac;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Always miss or hit */
	if (k < 10) return (k < 5);

	/* Calculate the "attack quality" */
	i = (power + (level * 3));

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Power and Level compete against Armor */
	if ((i > 0) && (randint(i - luck( -10, 10)) > ((ac * 3) / 4))) return true;

	/* Assume miss */
	return false;
}



/*
 * Hack -- possible "insult" messages
 */
static const char *desc_insult[] =
{
	"insults you!",
	"insults your mother!",
	"jumps around you!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obnoxious gestures!",
	"pokes you!!!"
};



/*
 * Hack -- possible "insult" messages
 */
static const char *desc_moan[] =
{
	"seems sad about something.",
	"asks if you have seen his dogs.",
	"tells you to get off his land.",
	"mumbles something about mushrooms.",

	/* Mathilde's sentence */
	"giggles at you.",
	"asks you if you want to giggle with her.",
	"says she is always happy."
};


/*
 * Get the "power" of an attack of given effect type.
 */
int get_attack_power(int effect)
{
	switch (effect)
	{
	case RBE_HURT:
		return 60;
	case RBE_POISON:
		return 5;
	case RBE_UN_BONUS:
		return 20;
	case RBE_UN_POWER:
		return 15;
	case RBE_EAT_GOLD:
		return 5;
	case RBE_EAT_ITEM:
		return 5;
	case RBE_EAT_FOOD:
		return 5;
	case RBE_EAT_LITE:
		return 5;
	case RBE_ACID:
		return 0;
	case RBE_ELEC:
		return 10;
	case RBE_FIRE:
		return 10;
	case RBE_COLD:
		return 10;
	case RBE_BLIND:
		return 2;
	case RBE_CONFUSE:
		return 10;
	case RBE_TERRIFY:
		return 10;
	case RBE_PARALYZE:
		return 2;
	case RBE_LOSE_STR:
		return 0;
	case RBE_LOSE_DEX:
		return 0;
	case RBE_LOSE_CON:
		return 0;
	case RBE_LOSE_INT:
		return 0;
	case RBE_LOSE_WIS:
		return 0;
	case RBE_LOSE_CHR:
		return 0;
	case RBE_LOSE_ALL:
		return 2;
	case RBE_SHATTER:
		return 60;
	case RBE_EXP_10:
		return 5;
	case RBE_EXP_20:
		return 5;
	case RBE_EXP_40:
		return 5;
	case RBE_EXP_80:
		return 5;
	case RBE_DISEASE:
		return 5;
	case RBE_TIME:
		return 5;
	case RBE_SANITY:
		return 60;
	case RBE_HALLU:
		return 10;
	case RBE_PARASITE:
		return 5;
	case RBE_ABOMINATION:
		return 30;
	}
	/* Unknown effects have no power */
	return 0;
}

/*
 * Attack the player via physical attacks.
 */
bool carried_make_attack_normal(int r_idx)
{
	auto const &r_info = game->edit_data.r_info;

	auto r_ptr = &r_info[r_idx];

	int ap_cnt;

	int k, tmp, ac, rlev;
	int do_cut, do_stun;

	char ddesc[80] = "your symbiote";
	auto sym_name = symbiote_name(true);

	bool alive = true;

	/* Not allowed to attack */
	if (r_ptr->flags & RF_NEVER_BLOW) return false;

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

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


		/* Hack -- no more attacks */
		if (!method) break;


		/* Stop if player is dead or gone */
		if (!alive || death) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Extract the attack "power" */
		power = get_attack_power(effect);


		/* Monster hits player */
		if (!effect || check_hit(power, rlev))
		{
			/* Always disturbing */
			disturb();

			/* Hack -- Apply "protection from evil" */
			if ((p_ptr->protevil > 0) &&
			                (r_ptr->flags & RF_EVIL) &&
			                (p_ptr->lev >= rlev) &&
			                ((rand_int(100) + p_ptr->lev) > 50))
			{
				/* Message */
				msg_format("%s is repelled.", sym_name.c_str());

				/* Hack -- Next attack */
				continue;
			}

			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touches you.";
					break;
				}

			case RBM_PUNCH:
				{
					act = "punches you.";
					do_stun = 1;
					break;
				}

			case RBM_KICK:
				{
					act = "kicks you.";
					do_stun = 1;
					break;
				}

			case RBM_CLAW:
				{
					act = "claws you.";
					do_cut = 1;
					break;
				}

			case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					break;
				}

			case RBM_STING:
				{
					act = "stings you.";
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's you.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulfs you.";
					break;
				}

			case RBM_CHARGE:
				{
					act = "charges you.";
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawls on you.";
					break;
				}

			case RBM_DROOL:
				{
					act = "drools on you.";
					break;
				}

			case RBM_SPIT:
				{
					act = "spits on you.";
					break;
				}

			case RBM_EXPLODE:
				{
					act = "explodes.";
					break;
				}

			case RBM_GAZE:
				{
					act = "gazes at you.";
					break;
				}

			case RBM_WAIL:
				{
					act = "wails at you.";
					break;
				}

			case RBM_SPORE:
				{
					act = "releases spores at you.";
					break;
				}

			case RBM_XXX4:
				{
					act = "projects XXX4's at you.";
					break;
				}

			case RBM_BEG:
				{
					act = "begs you for money.";
					break;
				}

			case RBM_INSULT:
				{
					act = desc_insult[rand_int(8)];
					break;
				}

			case RBM_MOAN:
				{
					act = desc_moan[rand_int(4)];
					break;
				}

			case RBM_SHOW:
				{
					if (randint(3) == 1)
						act = "sings 'We are a happy family.'";
					else
						act = "sings 'I love you, you love me.'";
					break;
				}
			}

			/* Message */
			if (act) msg_format("%s %s", sym_name.c_str(), act);


			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					/* Hack -- No damage */
					damage = 0;

					break;
				}

			case RBE_HURT:
				{
					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					break;
				}

			case RBE_ABOMINATION:
				{
					/* Morph, but let mimicry skill have a chance to stop this */
					if (magik(60 - get_skill(SKILL_MIMICRY)))
					{
						/* Message */
						cmsg_print(TERM_VIOLET, "You feel the dark powers twisting your body!");

						set_mimic(damage, resolve_mimic_name("Abomination"), 50);
					}
					else
					{
						/* Message */
						cmsg_print(TERM_VIOLET, "You feel the dark powers trying to twisting your body, but they fail.");
					}

					break;
				}

			case RBE_SANITY:
				{
					take_sanity_hit(damage, ddesc);
					break;
				}

			case RBE_POISON:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						set_poisoned(p_ptr->poisoned + randint(rlev) + 5);
					}

					break;
				}

			case RBE_UN_BONUS:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Allow complete resist */
					if (!p_ptr->resist_disen)
					{
						/* Apply disenchantment */
						apply_disenchant(0);
					}

					break;
				}

			case RBE_UN_POWER:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}

			case RBE_EAT_GOLD:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}

			case RBE_EAT_ITEM:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}

			case RBE_EAT_FOOD:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}

			case RBE_EAT_LITE:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}

			case RBE_ACID:
				{
					/* Message */
					msg_print("You are covered in acid!");

					/* Special damage */
					carried_monster_hit = true;
					acid_dam(damage, ddesc);

					break;
				}

			case RBE_ELEC:
				{
					/* Message */
					msg_print("You are struck by electricity!");

					/* Special damage */
					carried_monster_hit = true;
					elec_dam(damage, ddesc);


					break;
				}

			case RBE_FIRE:
				{
					/* Message */
					msg_print("You are enveloped in flames!");

					/* Special damage */
					carried_monster_hit = true;
					fire_dam(damage, ddesc);


					break;
				}

			case RBE_COLD:
				{
					/* Message */
					msg_print("You are covered with frost!");

					/* Special damage */
					carried_monster_hit = true;
					cold_dam(damage, ddesc);


					break;
				}

			case RBE_BLIND:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Increase "blind" */
					if (!p_ptr->resist_blind)
					{
						set_blind(p_ptr->blind + 10 + randint(rlev));
					}


					break;
				}

			case RBE_CONFUSE:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Increase "confused" */
					if (!p_ptr->resist_conf)
					{
						set_confused(p_ptr->confused + 3 + randint(rlev));
					}


					break;
				}

			case RBE_TERRIFY:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Increase "afraid" */
					if (p_ptr->resist_fear)
					{
						msg_print("You stand your ground!");
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You stand your ground!");
					}
					else
					{
						set_afraid(p_ptr->afraid + 3 + randint(rlev));
					}


					break;
				}

			case RBE_PARALYZE:
				{
					/* Hack -- Prevent perma-paralysis via damage */
					if (p_ptr->paralyzed && (damage < 1)) damage = 1;

					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Increase "paralyzed" */
					if (p_ptr->free_act)
					{
						msg_print("You are unaffected!");
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
					}
					else
					{
						set_paralyzed(3 + randint(rlev));
					}


					break;
				}

			case RBE_LOSE_STR:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_STR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_INT:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_INT, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_WIS:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_DEX:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CON:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_CON, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CHR:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_ALL:
				{
					/* Damage (physical) */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Damage (stats) */
					do_dec_stat(A_STR, STAT_DEC_NORMAL);
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);
					do_dec_stat(A_CON, STAT_DEC_NORMAL);
					do_dec_stat(A_INT, STAT_DEC_NORMAL);
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_SHATTER:
				{
					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Radius 8 earthquake centered at the monster */
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
							earthquake(p_ptr->py, p_ptr->px, 8);
					}

					break;
				}

			case RBE_EXP_10:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 95))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_20:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 90))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_40:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 75))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_80:
				{
					/* Take damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_DISEASE:
				{
					/* Take some damage */
					carried_monster_hit = true;
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						set_poisoned(p_ptr->poisoned + randint(rlev) + 5);
					}

					/* Damage CON (10% chance)*/
					if (randint(100) < 11)
					{
						/* 1% chance for perm. damage */
						bool perm = (randint(10) == 1);
						dec_stat(A_CON, randint(10), perm);
					}

					break;
				}
			case RBE_PARASITE:
				{
					if (!p_ptr->parasite) set_parasite(damage, r_idx);

					break;
				}
			case RBE_HALLU:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "image" */
					if (!p_ptr->resist_chaos)
					{
						set_image(p_ptr->image + 3 + randint(rlev / 2));
					}
					break;

				}
			case RBE_TIME:
				{
					switch (randint(10))
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						{
							msg_print("You feel life has clocked back.");
							lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
							break;
						}

					case 6:
					case 7:
					case 8:
					case 9:
						{
							int stat = rand_int(6);

							switch (stat)
							{
							case A_STR:
								act = "strong";
								break;
							case A_INT:
								act = "bright";
								break;
							case A_WIS:
								act = "wise";
								break;
							case A_DEX:
								act = "agile";
								break;
							case A_CON:
								act = "hardy";
								break;
							case A_CHR:
								act = "beautiful";
								break;
							}

							msg_format("You're not as %s as you used to be...", act);

							p_ptr->stat_cur[stat] = (p_ptr->stat_cur[stat] * 3) / 4;
							if (p_ptr->stat_cur[stat] < 3) p_ptr->stat_cur[stat] = 3;
							p_ptr->update |= (PU_BONUS);
							break;
						}

					case 10:
						{
							msg_print("You're not as powerful as you used to be...");

							for (k = 0; k < 6; k++)
							{
								p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
								if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
							}
							p_ptr->update |= (PU_BONUS);
							break;
						}
					}
					carried_monster_hit = true;
					take_hit(damage, ddesc);
					break;
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (rand_int(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
				case 0:
					k = 0;
					break;
				case 1:
					k = randint(5);
					break;
				case 2:
					k = randint(5) + 5;
					break;
				case 3:
					k = randint(20) + 20;
					break;
				case 4:
					k = randint(50) + 50;
					break;
				case 5:
					k = randint(100) + 100;
					break;
				case 6:
					k = 300;
					break;
				default:
					k = 500;
					break;
				}

				/* Apply the cut */
				if (k) set_cut(p_ptr->cut + k);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
				case 0:
					k = 0;
					break;
				case 1:
					k = randint(5);
					break;
				case 2:
					k = randint(10) + 10;
					break;
				case 3:
					k = randint(20) + 20;
					break;
				case 4:
					k = randint(30) + 30;
					break;
				case 5:
					k = randint(40) + 40;
					break;
				case 6:
					k = 100;
					break;
				default:
					k = 200;
					break;
				}

				/* Apply the stun */
				if (k) set_stun(p_ptr->stun + k);
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

				/* Disturbing */
				disturb();

				/* Message */
				msg_format("%s misses you.", sym_name.c_str());

				break;
			}
		}
	}
	/* Assume we attacked */
	return true;
}

/*
 * Give unprotected player the Black Breath with a 1 in (chance) probability
 *
 */
void black_breath_attack(int chance)
{
	if (randint(chance) == 1)
	{
		 msg_print("Your foe calls upon your soul!");
		 msg_print("You feel the Black Breath slowly draining you of life...");
		 p_ptr->black_breath = true;
	}
}

/*
 * Attack the player via physical attacks.
 */
bool make_attack_normal(int m_idx, byte divis)
{
	monster_type *m_ptr = &m_list[m_idx];

	int ap_cnt;

	int i, j, k, tmp, ac, rlev;
	int do_cut, do_stun;

	s32b gold;

	object_type *o_ptr;

	char o_name[80];

	char m_name[80];

	char ddesc[80];

	bool blinked;
	bool touched = false;
	bool fear = false;
	bool alive = true;
	bool explode = false;

	/* Not allowed to attack? */
	auto r_ptr = m_ptr->race();
	if (r_ptr->flags & RF_NEVER_BLOW) return false;

	/* ...nor if friendly */
	if (is_friend(m_ptr) >= 0)
	{
		if (p_ptr->control == m_idx) swap_position(m_ptr->fy, m_ptr->fx);
		return false;
	}

	/* Cannot attack the player if mortal and player fated to never die by the ... */
	if ((r_ptr->flags & RF_MORTAL) && (p_ptr->no_mortal)) return false;

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Extract the effective monster level */
	rlev = ((m_ptr->level >= 1) ? m_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, m_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, m_ptr, 0x88);


	/* Assume no blink */
	blinked = false;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = m_ptr->blow[ap_cnt].effect;
		int method = m_ptr->blow[ap_cnt].method;
		int d_dice = m_ptr->blow[ap_cnt].d_dice;
		int d_side = m_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;


		/* Stop if player is dead or gone */
		if (!alive || death) break;

		/* Handle "leaving" */
		if (p_ptr->leaving) break;

		/* Extract the attack "power" */
		switch (effect)
		{
		case RBE_HURT:
			power = 60;
			break;
		case RBE_POISON:
			power = 5;
			break;
		case RBE_UN_BONUS:
			power = 20;
			break;
		case RBE_UN_POWER:
			power = 15;
			break;
		case RBE_EAT_GOLD:
			power = 5;
			break;
		case RBE_EAT_ITEM:
			power = 5;
			break;
		case RBE_EAT_FOOD:
			power = 5;
			break;
		case RBE_EAT_LITE:
			power = 5;
			break;
		case RBE_ACID:
			power = 0;
			break;
		case RBE_ELEC:
			power = 10;
			break;
		case RBE_FIRE:
			power = 10;
			break;
		case RBE_COLD:
			power = 10;
			break;
		case RBE_BLIND:
			power = 2;
			break;
		case RBE_CONFUSE:
			power = 10;
			break;
		case RBE_TERRIFY:
			power = 10;
			break;
		case RBE_PARALYZE:
			power = 2;
			break;
		case RBE_LOSE_STR:
			power = 0;
			break;
		case RBE_LOSE_DEX:
			power = 0;
			break;
		case RBE_LOSE_CON:
			power = 0;
			break;
		case RBE_LOSE_INT:
			power = 0;
			break;
		case RBE_LOSE_WIS:
			power = 0;
			break;
		case RBE_LOSE_CHR:
			power = 0;
			break;
		case RBE_LOSE_ALL:
			power = 2;
			break;
		case RBE_SHATTER:
			power = 60;
			break;
		case RBE_EXP_10:
			power = 5;
			break;
		case RBE_EXP_20:
			power = 5;
			break;
		case RBE_EXP_40:
			power = 5;
			break;
		case RBE_EXP_80:
			power = 5;
			break;
		case RBE_DISEASE:
			power = 5;
			break;
		case RBE_TIME:
			power = 5;
			break;
		case RBE_SANITY:
			power = 60;
			break;
		case RBE_HALLU:
			power = 10;
			break;
		case RBE_PARASITE:
			power = 5;
			break;
		case RBE_ABOMINATION:
			power = 20;
			break;
		}


		/* Monster hits player */
		if (!effect || check_hit(power, rlev))
		{
			int chance = p_ptr->dodge_chance - ((rlev * 5) / 6);

			/* Always disturbing */
			disturb();

			if ((chance > 0) && magik(chance))
			{
				char m_poss[80];
				monster_desc(m_poss, m_ptr, 0x06);
				msg_format("You dodge %s attack!", m_poss);
				continue;
			}

			/* Eru can help you */
			if (praying_to(GOD_ERU))
			{
				s32b chance = p_ptr->grace;

				if (chance > 50000) chance = 50000;
				chance -= rlev * 300;

				if ((randint(100000) < chance) && (r_ptr->flags & RF_EVIL))
				{
					/* Message */
					msg_format("The hand of Eru Iluvatar stops %s blow.", m_name);

					/* Hack -- Next attack */
					continue;
				}
			}

			/* Hack -- Apply "protection from evil" */
			if ((p_ptr->protevil > 0) &&
			                (r_ptr->flags & RF_EVIL) &&
			                (p_ptr->lev >= rlev) &&
			                ((rand_int(100) + p_ptr->lev) > 50))
			{
				/* Message */
				msg_format("%^s is repelled.", m_name);

				/* Hack -- Next attack */
				continue;
			}

			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Describe the attack method */
			switch (method)
			{
			case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					touched = true;
					break;
				}

			case RBM_TOUCH:
				{
					act = "touches you.";
					touched = true;
					break;
				}

			case RBM_PUNCH:
				{
					act = "punches you.";
					touched = true;
					do_stun = 1;
					break;
				}

			case RBM_KICK:
				{
					act = "kicks you.";
					touched = true;
					do_stun = 1;
					break;
				}

			case RBM_CLAW:
				{
					act = "claws you.";
					touched = true;
					do_cut = 1;
					break;
				}

			case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					touched = true;
					break;
				}

			case RBM_STING:
				{
					act = "stings you.";
					touched = true;
					break;
				}

			case RBM_XXX1:
				{
					act = "XXX1's you.";
					break;
				}

			case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					touched = true;
					break;
				}

			case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					touched = true;
					break;
				}

			case RBM_ENGULF:
				{
					act = "engulfs you.";
					touched = true;
					break;
				}

			case RBM_CHARGE:
				{
					act = "charges you.";
					touched = true;
					break;
				}

			case RBM_CRAWL:
				{
					act = "crawls on you.";
					touched = true;
					break;
				}

			case RBM_DROOL:
				{
					act = "drools on you.";
					break;
				}

			case RBM_SPIT:
				{
					act = "spits on you.";
					break;
				}

			case RBM_EXPLODE:
				{
					act = "explodes.";
					explode = true;
					break;
				}

			case RBM_GAZE:
				{
					act = "gazes at you.";
					break;
				}

			case RBM_WAIL:
				{
					act = "wails at you.";
					break;
				}

			case RBM_SPORE:
				{
					act = "releases spores at you.";
					break;
				}

			case RBM_XXX4:
				{
					act = "projects XXX4's at you.";
					break;
				}

			case RBM_BEG:
				{
					act = "begs you for money.";
					break;
				}

			case RBM_INSULT:
				{
					act = desc_insult[rand_int(8)];
					break;
				}

			case RBM_MOAN:
				{
					if (strstr(r_ptr->name, "Mathilde, the Science Student"))
						act = desc_moan[rand_int(3) + 4];
					else
						act = desc_moan[rand_int(4)];
					break;
				}

			case RBM_SHOW:
				{
					if (randint(3) == 1)
						act = "sings 'We are a happy family.'";
					else
						act = "sings 'I love you, you love me.'";
					break;
				}
			}

			/* Message */
			if (act) msg_format("%^s %s", m_name, act);

			/* The undead can give the player the Black Breath with
			 * a successful blow. Uniques have a better chance. -LM-
			 * Nazgul have a 25% chance
			 */
			if (r_ptr->flags & RF_NAZGUL)
			{
				black_breath_attack(4);
			}
			else if ((m_ptr->level >= 35) && (r_ptr->flags & RF_UNDEAD) &&
					    (r_ptr->flags & RF_UNIQUE))
			{
				black_breath_attack(300 - m_ptr->level);
			}
			else if ((m_ptr->level >= 40) && (r_ptr->flags & RF_UNDEAD))
			{
				black_breath_attack(450 - m_ptr->level);
			}

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			/* Sometime reduce the damage */
			damage /= divis;

			/* Apply appropriate damage */
			switch (effect)
			{
			case 0:
				{
					/* Hack -- No damage */
					damage = 0;

					break;
				}

			case RBE_HURT:
				{
					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(damage, ddesc);

					break;
				}

			case RBE_ABOMINATION:
				{
					/* Morph, but let mimicry skill have a chance to stop this */
					if (magik(60 - get_skill(SKILL_MIMICRY)))
					{
						/* Message */
						cmsg_print(TERM_VIOLET, "You feel the dark powers twisting your body!");

						set_mimic(damage, resolve_mimic_name("Abomination"), 50);
					}
					else
					{
						/* Message */
						cmsg_print(TERM_VIOLET, "You feel the dark powers trying to twisting your body, but they fail.");
					}

					break;
				}

			case RBE_SANITY:
				{
					take_sanity_hit(damage, ddesc);
					break;
				}

			case RBE_POISON:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						set_poisoned(p_ptr->poisoned + randint(rlev) + 5);
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_POIS);

					break;
				}

			case RBE_UN_BONUS:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Allow complete resist */
					if (!p_ptr->resist_disen)
					{
						apply_disenchant(0);
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_DISEN);

					break;
				}

			case RBE_UN_POWER:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_ptr)
						{
							continue;
						}

						/* Drain charged wands/staffs
						   Hack -- don't let artifacts get drained */
						if (((o_ptr->tval == TV_STAFF) ||
						                (o_ptr->tval == TV_WAND)) &&
						                (o_ptr->pval) &&
					                     !artifact_p(o_ptr))
						{
							/* Message */
							msg_print("Energy drains from your pack!");

							/* Heal */
							j = rlev;
							m_ptr->hp += j * o_ptr->pval * o_ptr->number;
							if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

							/* Redraw (later) if needed */
							if (health_who == m_idx) p_ptr->redraw |= (PR_FRAME);

							/* Uncharge */
							o_ptr->pval = 0;

							/* Combine / Reorder the pack */
							p_ptr->notice |= (PN_COMBINE | PN_REORDER);

							/* Window stuff */
							p_ptr->window |= (PW_INVEN);

							/* Done */
							break;
						}
					}

					break;
				}

			case RBE_EAT_GOLD:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->paralyzed &&
					                (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
					                                  p_ptr->lev)))
					{
						/* Saving throw message */
						msg_print("You quickly protect your money pouch!");

						/* Occasional blink anyway */
						if (rand_int(3)) blinked = true;
					}

					/* Eat gold */
					else
					{
						gold = (p_ptr->au / 10) + randint(25);
						if (gold < 2) gold = 2;
						if (gold > 5000) gold = (p_ptr->au / 20) + randint(3000);
						if (gold > p_ptr->au) gold = p_ptr->au;
						p_ptr->au -= gold;
						if (gold <= 0)
						{
							msg_print("Nothing was stolen.");
						}
						else if (p_ptr->au)
						{
							msg_print("Your purse feels lighter.");
							msg_format("%ld coins were stolen!", (long)gold);
						}
						else
						{
							msg_print("Your purse feels lighter.");
							msg_print("All of your coins were stolen!");
						}

						while (gold > 0)
						{
							object_type forge, *j_ptr = &forge;

							/* Wipe the object */
							object_wipe(j_ptr);

							/* Prepare a gold object */
							object_prep(j_ptr, lookup_kind(TV_GOLD, 9));

							/* Determine how much the treasure is "worth" */
							j_ptr->pval = (gold >= 15000) ? 15000 : gold;

							monster_carry(m_ptr, m_idx, j_ptr);

							gold -= 15000;
						}

						/* Redraw gold */
						p_ptr->redraw |= (PR_FRAME);

						/* Window stuff */
						p_ptr->window |= (PW_PLAYER);

						/* Blink away */
						blinked = true;
					}

					break;
				}

			case RBE_EAT_ITEM:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->paralyzed &&
					                (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
					                                  p_ptr->lev)))
					{
						/* Saving throw message */
						msg_print("You grab hold of your backpack!");

						/* Occasional "blink" anyway */
						blinked = true;

						/* Done */
						break;
					}

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_ptr)
						{
							continue;
						}

						/* Skip artifacts */
						if (artifact_p(o_ptr))
						{
							continue;
						}

						/* Get a description */
						object_desc(o_name, o_ptr, false, 3);

						/* Message */
						msg_format("%sour %s (%c) was stolen!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Copy into inventory of monster */
						{
							s16b o_idx;

							/* Make an object */
							o_idx = o_pop();

							/* Success */
							if (o_idx)
							{
								object_type *j_ptr;

								/* Get new object */
								j_ptr = &o_list[o_idx];

								/* Copy object */
								object_copy(j_ptr, o_ptr);

								/* Modify number */
								j_ptr->number = 1;

								/* Hack -- If a wand, allocate total
								 * maximum timeouts or charges between those
								 * stolen and those missed. -LM-
								 */
								if (o_ptr->tval == TV_WAND)
								{
									j_ptr->pval = o_ptr->pval / o_ptr->number;
									o_ptr->pval -= j_ptr->pval;
								}

								/* Forget mark */
								j_ptr->marked = false;

								/* Memorize monster */
								j_ptr->held_m_idx = m_idx;

								/* Build stack */
								m_ptr->hold_o_idxs.push_back(o_idx);
							}
						}

						/* Steal the items */
						inc_stack_size_ex(i, -1, OPTIMIZE, NO_DESCRIBE);

						/* Blink away */
						blinked = true;

						/* Done */
						break;
					}

					break;
				}

			case RBE_EAT_FOOD:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Steal some food */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item from the pack */
						i = rand_int(INVEN_PACK);

						/* Get the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_ptr)
						{
							continue;
						}

						/* Skip non-food objects */
						if (o_ptr->tval != TV_FOOD) continue;

						/* Get a description */
						object_desc(o_name, o_ptr, false, 0);

						/* Message */
						msg_format("%sour %s (%c) was eaten!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Steal the items */
						inc_stack_size_ex(i, -1, OPTIMIZE, NO_DESCRIBE);

						/* Done */
						break;
					}

					break;
				}

			case RBE_EAT_LITE:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Access the lite */
					o_ptr = &p_ptr->inventory[INVEN_LITE];

					/* Drain fuel */
					if ((o_ptr->pval > 0) && (!artifact_p(o_ptr)))
					{
						/* Reduce fuel */
						o_ptr->pval -= (250 + randint(250));
						if (o_ptr->pval < 1) o_ptr->pval = 1;

						/* Notice */
						if (!p_ptr->blind)
						{
							msg_print("Your light dims.");
						}

						/* Window stuff */
						p_ptr->window |= (PW_EQUIP);
					}

					break;
				}

			case RBE_ACID:
				{
					/* Message */
					msg_print("You are covered in acid!");

					/* Special damage */
					acid_dam(damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_ACID);

					break;
				}

			case RBE_ELEC:
				{
					/* Message */
					msg_print("You are struck by electricity!");

					/* Special damage */
					elec_dam(damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_ELEC);

					break;
				}

			case RBE_FIRE:
				{
					/* Message */
					msg_print("You are enveloped in flames!");

					/* Special damage */
					fire_dam(damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FIRE);

					break;
				}

			case RBE_COLD:
				{
					/* Message */
					msg_print("You are covered with frost!");

					/* Special damage */
					cold_dam(damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_COLD);

					break;
				}

			case RBE_BLIND:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "blind" */
					if (!p_ptr->resist_blind)
					{
						set_blind(p_ptr->blind + 10 + randint(rlev));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_BLIND);

					break;
				}

			case RBE_CONFUSE:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "confused" */
					if (!p_ptr->resist_conf)
					{
						set_confused(p_ptr->confused + 3 + randint(rlev));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_CONF);

					break;
				}

			case RBE_TERRIFY:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "afraid" */
					if (p_ptr->resist_fear)
					{
						msg_print("You stand your ground!");
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You stand your ground!");
					}
					else
					{
						set_afraid(p_ptr->afraid + 3 + randint(rlev));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FEAR);

					break;
				}

			case RBE_PARALYZE:
				{
					/* Hack -- Prevent perma-paralysis via damage */
					if (p_ptr->paralyzed && (damage < 1)) damage = 1;

					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "paralyzed" */
					if (p_ptr->free_act)
					{
						msg_print("You are unaffected!");
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print("You resist the effects!");
					}
					else
					{
						set_paralyzed(3 + randint(rlev));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FREE);

					break;
				}

			case RBE_LOSE_STR:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_STR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_INT:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_INT, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_WIS:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_DEX:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CON:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_CON, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_CHR:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stat) */
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_LOSE_ALL:
				{
					/* Damage (physical) */
					take_hit(damage, ddesc);

					/* Damage (stats) */
					do_dec_stat(A_STR, STAT_DEC_NORMAL);
					do_dec_stat(A_DEX, STAT_DEC_NORMAL);
					do_dec_stat(A_CON, STAT_DEC_NORMAL);
					do_dec_stat(A_INT, STAT_DEC_NORMAL);
					do_dec_stat(A_WIS, STAT_DEC_NORMAL);
					do_dec_stat(A_CHR, STAT_DEC_NORMAL);

					break;
				}

			case RBE_SHATTER:
				{
					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(damage, ddesc);

					/* Radius 8 earthquake centered at the monster */
					if (damage > 23)
					{
						/* Prevent destruction of quest levels and town */
						if (!is_quest(dun_level) && dun_level)
							earthquake(m_ptr->fy, m_ptr->fx, 8);
					}

					break;
				}

			case RBE_EXP_10:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 95))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_20:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 90))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_40:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 75))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_EXP_80:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p_ptr->exp / 100) * MON_DRAIN_LIFE;
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
					break;
				}

			case RBE_DISEASE:
				{
					/* Take some damage */
					take_hit(damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						set_poisoned(p_ptr->poisoned + randint(rlev) + 5);
					}

					/* Damage CON (10% chance)*/
					if (randint(100) < 11)
					{
						/* 1% chance for perm. damage */
						bool perm = (randint(10) == 1);
						dec_stat(A_CON, randint(10), perm);
					}

					break;
				}
			case RBE_HALLU:
				{
					/* Take damage */
					take_hit(damage, ddesc);

					/* Increase "image" */
					if (!p_ptr->resist_chaos)
					{
						set_image(p_ptr->image + 3 + randint(rlev / 2));
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_CHAOS);

					break;

				}
			case RBE_TIME:
				{
					switch (randint(10))
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						{
							msg_print("You feel life has clocked back.");
							lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
							break;
						}

					case 6:
					case 7:
					case 8:
					case 9:
						{
							int stat = rand_int(6);

							switch (stat)
							{
							case A_STR:
								act = "strong";
								break;
							case A_INT:
								act = "bright";
								break;
							case A_WIS:
								act = "wise";
								break;
							case A_DEX:
								act = "agile";
								break;
							case A_CON:
								act = "hardy";
								break;
							case A_CHR:
								act = "beautiful";
								break;
							}

							msg_format("You're not as %s as you used to be...", act);

							p_ptr->stat_cur[stat] = (p_ptr->stat_cur[stat] * 3) / 4;
							if (p_ptr->stat_cur[stat] < 3) p_ptr->stat_cur[stat] = 3;
							p_ptr->update |= (PU_BONUS);
							break;
						}

					case 10:
						{
							msg_print("You're not as powerful as you used to be...");

							for (k = 0; k < 6; k++)
							{
								p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
								if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
							}
							p_ptr->update |= (PU_BONUS);
							break;
						}
					}
					take_hit(damage, ddesc);
					break;
				}
			case RBE_PARASITE:
				{
					if (!p_ptr->parasite) set_parasite(damage, m_ptr->r_idx);

					break;
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (rand_int(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
				case 0:
					k = 0;
					break;
				case 1:
					k = randint(5);
					break;
				case 2:
					k = randint(5) + 5;
					break;
				case 3:
					k = randint(20) + 20;
					break;
				case 4:
					k = randint(50) + 50;
					break;
				case 5:
					k = randint(100) + 100;
					break;
				case 6:
					k = 300;
					break;
				default:
					k = 500;
					break;
				}

				/* Apply the cut */
				if (k) set_cut(p_ptr->cut + k);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
				case 0:
					k = 0;
					break;
				case 1:
					k = randint(5);
					break;
				case 2:
					k = randint(10) + 10;
					break;
				case 3:
					k = randint(20) + 20;
					break;
				case 4:
					k = randint(30) + 30;
					break;
				case 5:
					k = randint(40) + 40;
					break;
				case 6:
					k = 100;
					break;
				default:
					k = 200;
					break;
				}

				/* Apply the stun */
				if (k) set_stun(p_ptr->stun + k);
			}

			if (explode)
			{
				if (mon_take_hit(m_idx, m_ptr->hp + 1, &fear, NULL))
				{
					blinked = false;
					alive = false;
				}
			}

			if (touched)
			{
				if (p_ptr->sh_fire && alive)
				{
					if (!(r_ptr->flags & RF_IM_FIRE))
					{
						msg_format("%^s is suddenly very hot!", m_name);
						if (mon_take_hit(m_idx, damroll(2, 6), &fear,
						                 " turns into a pile of ash."))
						{
							blinked = false;
							alive = false;
						}
					}
				}

				if (p_ptr->sh_elec && alive)
				{
					if (!(r_ptr->flags & RF_IM_ELEC))
					{
						msg_format("%^s gets zapped!", m_name);
						if (mon_take_hit(m_idx, damroll(2, 6), &fear,
						                 " turns into a pile of cinder."))
						{
							blinked = false;
							alive = false;
						}
					}
				}

				if (p_ptr->shield && (p_ptr->shield_opt & SHIELD_COUNTER) && alive)
				{
					msg_format("%^s gets bashed by your mystic shield!", m_name);
					if (mon_take_hit(m_idx, damroll(p_ptr->shield_power_opt, p_ptr->shield_power_opt2), &fear,
					                 " is bashed by your mystic shield."))
					{
						blinked = false;
						alive = false;
					}
				}

				if (p_ptr->shield && (p_ptr->shield_opt & SHIELD_FIRE) && alive)
				{
					if (!(r_ptr->flags & RF_IM_FIRE))
					{
						msg_format("%^s gets burned by your fiery shield!", m_name);
						if (mon_take_hit(m_idx, damroll(p_ptr->shield_power_opt, p_ptr->shield_power_opt2), &fear,
						                 " is burned by your fiery shield."))
						{
							blinked = false;
							alive = false;
						}
					}
				}

				if (p_ptr->shield && (p_ptr->shield_opt & SHIELD_GREAT_FIRE) && alive)
				{
					msg_format("%^s gets burned by your fiery shield!", m_name);
					if (mon_take_hit(m_idx, damroll(p_ptr->shield_power_opt, p_ptr->shield_power_opt2), &fear,
					                 " is burned by your fiery shield."))
					{
						blinked = false;
						alive = false;
					}
				}

				if (p_ptr->shield && (p_ptr->shield_opt & SHIELD_FEAR) && alive)
				{
					int tmp;

					if ((!(r_ptr->flags & RF_UNIQUE)) && (damroll(p_ptr->shield_power_opt, p_ptr->shield_power_opt2) - m_ptr->level > 0))
					{
						msg_format("%^s gets scared away!", m_name);

						/* Increase fear */
						tmp = m_ptr->monfear + p_ptr->shield_power_opt;
						fear = true;

						/* Set fear */
						m_ptr->monfear = (tmp < 200) ? tmp : 200;
					}
				}

				touched = false;
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

				/* Visible monsters */
				if (m_ptr->ml)
				{
					/* Disturbing */
					disturb();

					/* Message */
					msg_format("%^s misses you.", m_name);
				}

				break;
			}
		}
	}


	/* Blink away */
	if (blinked)
	{
		msg_print("The thief flees laughing!");
		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}

	/* Fear */
	if (m_ptr->ml && fear)
	{
		msg_format("%^s flees in terror!", m_name);
	}

	/* Assume we attacked */
	return true;
}


