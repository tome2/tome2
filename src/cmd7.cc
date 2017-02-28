/*
 * Copyright (c) 1999 Dark God
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "cmd7.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd1.hpp"
#include "cmd5.hpp"
#include "cmd6.hpp"
#include "dungeon_flag.hpp"
#include "ego_item_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

/*
 * Describe class powers of Mindcrafters
 *
 * 'p' points to a 80 byte long buffer
 */
void mindcraft_info(char *p, int power)
{
	int plev = get_skill(SKILL_MINDCRAFT);


	/* Clear buffer */
	strcpy(p, "");

	/* Fill the buffer with requested power description */
	switch (power)
	{
	case 0:
		strnfmt(p, 80, " rad %d", DEFAULT_RADIUS);
		break;
	case 1:
		strnfmt(p, 80, " dam %dd%d", 3 + ((plev - 1) / 4), 3 + plev / 15);
		break;
	case 2:
		strnfmt(p, 80, " range %d", (plev < 25 ? 10 : plev + 2 + p_ptr->to_s * 3));
		break;
	case 3:
		strnfmt(p, 80, " range %d", plev * 5);
		break;
	case 4:
		strnfmt(p, 80, " power %d", plev * (plev < 30 ? 1 : 2));
		break;
	case 5:
		if (plev > 20)
			strnfmt(p, 80, " dam %dd8 rad %d", 8 + ((plev - 5) / 4), (plev - 20)/8 + 1);
		else
			strnfmt(p, 80, " dam %dd8", 8 + ((plev - 5) / 4));
		break;
	case 6:
		strnfmt(p, 80, " dur %d", plev);
		break;
	case 7:
		break;
	case 8:
		if (plev < 25)
			strnfmt(p, 80, " dam %d rad %d", (3 * plev) / 2, 2 + (plev / 10));
		else
			strnfmt(p, 80, " dam %d", plev * ((plev - 5) / 10 + 1));
		break;
	case 9:
		strnfmt(p, 80, " dur 11-%d", 10 + plev + plev / 2);
		break;
	case 10:
		strnfmt(p, 80, " dam %dd6 rad %d", plev / 2, 0 + (plev - 25) / 10);
		break;
	case 11:
		strnfmt(p, 80, " dam %d rad %d", plev * (plev > 39 ? 4 : 3), 3 + plev / 10);
		break;
	}
}


/*
 * Describe class powers of Mimics
 *
 * 'p' points to a 80 byte long buffer
 */
void mimic_info(char *p, int power)
{
	auto const &k_info = game->edit_data.k_info;

	int plev = get_skill(SKILL_MIMICRY);
	object_type *o_ptr = &p_ptr->inventory[INVEN_OUTER];

	/* Clear the buffer */
	strcpy(p, "");

	/* Fill the buffer with requested power description */
	switch (power)
	{
	case 0:
		strnfmt(p, 80, " dur %d", k_info[o_ptr->k_idx].pval2 + get_skill_scale(SKILL_MIMICRY, 1000));
		break;
	case 1:
		strnfmt(p, 80, " dur %d+d20", 10 + plev);
		break;
	case 2:
		strnfmt(p, 80, " dur 50+d%d", 50 + (2 * plev));
		break;
	case 3:
		strnfmt(p, 80, " dur 50+d%d", 50 + (2 * plev));
		break;
	case 4:
		strnfmt(p, 80, " dur 50+d%d", 50 + (2 * plev));
		break;
	}
}

/**
 * Show magic powers that user can choose from
 */
static void display_magic_powers(
	magic_power *powers,
	int max_powers,
	void (*power_info)(char *p, int power),
	int plev,
	int cast_stat,
	int y,
	int x)
{
	char psi_desc[80];
	magic_power spell;
	int i;
	int chance = 0;
	int minfail = 0;
	char comment[80];

	/* Display a list of spells */
	prt("", 1, x);
	prt("", y, x);
	put_str("Name", y, x + 5);
	put_str("Lv Mana Fail Info", y, x + 35);

	/* Dump the spells */
	for (i = 0; i < max_powers; i++)
	{
		/* Access the spell */
		spell = powers[i];
		if (spell.min_lev > plev)
		{
			break;
		}

		chance = spell.fail;
		/* Reduce failure rate by "effective" level adjustment */
		chance -= 3 * (plev - spell.min_lev);

		/* Reduce failure rate by INT/WIS adjustment */
		chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[cast_stat]] - 1);

		/* Not enough mana to cast */
		if (spell.mana_cost > p_ptr->csp)
		{
			chance += 5 * (spell.mana_cost - p_ptr->csp);
		}

		/* Extract the minimum failure rate */
		minfail = adj_mag_fail[p_ptr->stat_ind[cast_stat]];

		/* Failure rate */
		chance = clamp_failure_chance(chance, minfail);

		/* Get info */
		power_info(comment, i);

		/* Dump the spell --(-- */
		strnfmt(psi_desc, 80, "  %c) %-30s%2d %4d %3d%%%s",
			I2A(i), spell.name,
			spell.min_lev, spell.mana_cost, chance, comment);
		prt(psi_desc, y + i + 1, x);
	}

	/* Clear the bottom line */
	prt("", y + i + 1, x);
}

/*
 * Allow user to choose a magic power.
 *
 * If a valid spell is chosen, saves it in '*sn' and returns TRUE
 * If the user hits escape, returns FALSE, and set '*sn' to -1
 * If there are no legal choices, returns FALSE, and sets '*sn' to -2
 *
 * The "prompt" should be "cast", "recite", or "study"
 * The "known" should be TRUE for cast/pray, FALSE for study
 *
 * nb: This function has a (trivial) display bug which will be obvious
 * when you run it. It's probably easy to fix but I haven't tried,
 * sorry.
 */
static bool_ get_magic_power(int *sn, magic_power *powers, int max_powers,
                     void (*power_info)(char *p, int power), int plev, int cast_stat)
{
	int i;

	int num = 0;

	int y = 2;

	int x = 18;

	int info;

	char choice;

	char out_val[160];

	cptr p = "power";

	magic_power spell;

	bool_ flag;


	/* Assume cancelled */
	*sn = ( -1);

	/* Get the spell, if available */
	if (repeat_pull(sn))
	{
		/* Verify the spell */
		if (powers[*sn].min_lev <= plev)
		{
			/* Success */
			return (TRUE);
		}
	}

	/* Nothing chosen yet */
	flag = FALSE;

	/* Count number of powers that satisfies minimum plev requirement */
	for (i = 0; i < max_powers; i++)
	{
		if (powers[i].min_lev <= plev)
		{
			num++;
		}
	}

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%^ss %c-%c, ESC=exit, %c-%c=Info) Use which %s? ",
	        p, I2A(0), I2A(num - 1), toupper(I2A(0)), toupper(I2A(num - 1)), p);

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Show the list */
	display_magic_powers(powers, max_powers, power_info, plev, cast_stat, y, x);

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Note verify */
		info = (isupper(choice));

		/* Lowercase */
		if (info) choice = tolower(choice);

		/* Extract request */
		i = (islower(choice) ? A2I(choice) : -1);

		/* Totally Illegal */
		if ((i < 0) || (i >= num))
		{
			bell();
			continue;
		}

		/* Save the spell index */
		spell = powers[i];

		/* Provides info */
		if (info)
		{
			c_prt(TERM_L_BLUE, spell.desc, 1, 0);

			/* Restore the screen */
			inkey();
			Term_load();
			character_icky = FALSE;

			/* Redisplay choices */
			display_magic_powers(powers, max_powers, power_info, plev, cast_stat, y, x);
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
	Term_load();
	character_icky = FALSE;

	/* Abort if needed */
	if (!flag) return (FALSE);

	/* Save the choice */
	(*sn) = i;


	repeat_push(*sn);

	/* Success */
	return (TRUE);
}


/*
 * do_cmd_cast calls this function if the player's class
 * is 'mindcrafter'.
 */
void do_cmd_mindcraft(void)
{
	int n = 0, b = 0;

	int chance;

	int dir;

	int minfail = 0;

	int plev = get_skill(SKILL_MINDCRAFT);

	magic_power spell;


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


	/* not if confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* get power */
	if (!get_magic_power(&n, mindcraft_powers, MAX_MINDCRAFT_POWERS,
	                     mindcraft_info, plev, A_WIS)) return;

	spell = mindcraft_powers[n];

	/* Verify "dangerous" spells */
	if (spell.mana_cost > p_ptr->csp)
	{
		/* Warning */
		msg_print("You do not have enough mana to use this power.");

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}

	/* Spell failure chance */
	chance = spell.fail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (get_skill(SKILL_MINDCRAFT) - spell.min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_WIS]] - 1);

	/* Not enough mana to cast */
	if (spell.mana_cost > p_ptr->csp)
	{
		chance += 5 * (spell.mana_cost - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_WIS]];

	/* Failure rate */
	chance = clamp_failure_chance(chance, minfail);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		flush_on_failure();

		msg_format("You failed to concentrate hard enough!");

		if (randint(100) < (chance / 2))
		{
			/* Backfire */
			b = randint(100);
			if (b < 5)
			{
				msg_print("Oh, no! Your mind has gone blank!");
				lose_all_info();
			}
			else if (b < 15)
			{
				msg_print("Weird visions seem to dance before your eyes...");
				set_image(p_ptr->image + 5 + randint(10));
			}
			else if (b < 45)
			{
				msg_print("Your brain is addled!");
				set_confused(p_ptr->confused + randint(8));
			}
			else if (b < 90)
			{
				set_stun(p_ptr->stun + randint(8));
			}
			else
			{
				/* Mana storm */
				msg_print("Your mind unleashes its power in an uncontrollable storm!");
				project(1, 2 + plev / 10, p_ptr->py, p_ptr->px, plev * 2,
				        GF_MANA, PROJECT_JUMP | PROJECT_KILL | PROJECT_GRID | PROJECT_ITEM);
				p_ptr->csp = MAX(0, p_ptr->csp - plev * MAX(1, plev / 10));
			}
		}
	}

	/* Successful spells */
	else
	{
		/* spell code */
		switch (n)
		{
			/* Precog */
		case 0:
			{
				/* Magic mapping */
				if (plev > 44)
				{
					wiz_lite();
				}
				else if (plev > 19)
				{
					map_area();
				}

				/* Detection */
				if (plev < 30)
				{
					b = detect_monsters_normal(DEFAULT_RADIUS);
					if (plev > 14) b |= detect_monsters_invis(DEFAULT_RADIUS);
				}
				else
				{
					b = detect_all(DEFAULT_RADIUS);
				}

				/* Telepathy */
				if (plev > 24)
				{
					set_tim_esp(p_ptr->tim_esp + plev);

					/* If plvl >= 40, we should have permanent ESP */
				}

				if (!b) msg_print("You feel safe.");

				break;
			}

			/* Mindblast */
		case 1:
			{
				if (!get_aim_dir(&dir)) return;

				if (randint(100) < plev * 2)
				{
					fire_beam(GF_PSI, dir, damroll(3 + ((plev - 1) / 4), (3 + plev / 15)));
				}
				else
				{
					fire_ball(GF_PSI, dir, damroll(3 + ((plev - 1) / 4), (3 + plev / 15)), 0);
				}

				break;
			}

			/* Minor displace */
		case 2:
			{
				if (plev < 25)
				{
					teleport_player(10);
				}
				else
				{
					int ii, ij;

					if (dungeon_flags & DF_NO_TELEPORT)
					{
						msg_print("Not on special levels!");
						break;
					}

					msg_print("You open a Void Jumpgate. Choose a destination.");

					if (!tgt_pt(&ii, &ij)) return;
					p_ptr->energy -= 60 - plev;

					if (!cave_empty_bold(ij, ii) ||
					                (cave[ij][ii].info & CAVE_ICKY) ||
					                (distance(ij, ii, p_ptr->py, p_ptr->px) > plev + 2 + (p_ptr->to_s*3)) ||
					                (rand_int(plev * plev / 2) == 0))
					{
						msg_print("You fail to exit the void correctly!");
						p_ptr->energy -= 100;
						get_pos_player(10 + p_ptr->to_s / 2, &ij, &ii);
					}

					cave_set_feat(p_ptr->py, p_ptr->px, FEAT_BETWEEN);
					cave_set_feat(ij, ii, FEAT_BETWEEN);
					cave[p_ptr->py][p_ptr->px].special = ii + (ij << 8);
					cave[ij][ii].special = p_ptr->px + (p_ptr->py << 8);
				}

				break;
			}

			/* Major displace */
		case 3:
			{
				if (plev > 29) banish_monsters(plev);
				teleport_player(plev * 5);

				break;
			}

			/* Domination */
		case 4:
			{
				if (plev < 30)
				{
					if (!get_aim_dir(&dir)) return;
					fire_ball(GF_DOMINATION, dir, plev, 0);
				}
				else
				{
					charm_monsters(plev * 2);
				}

				break;
			}

			/* Fist of Force  ---  not 'true' TK  */
		case 5:
			{
				if (!get_aim_dir(&dir)) return;
				fire_ball(GF_SOUND, dir, damroll(8 + ((plev - 5) / 4), 8),
				          (plev > 20 ? (plev - 20) / 8 + 1 : 0));

				break;
			}

			/* Character Armour */
		case 6:
			{
				set_shield(p_ptr->shield + plev, plev, 0, 0, 0);
				if (plev > 14) set_oppose_acid(p_ptr->oppose_acid + plev);
				if (plev > 19) set_oppose_fire(p_ptr->oppose_fire + plev);
				if (plev > 24) set_oppose_cold(p_ptr->oppose_cold + plev);
				if (plev > 29) set_oppose_elec(p_ptr->oppose_elec + plev);
				if (plev > 34) set_oppose_pois(p_ptr->oppose_pois + plev);

				break;
			}

			/* Psychometry */
		case 7:
			{
				ident_spell();
				break;
			}

			/* Mindwave */
		case 8:
			{
				msg_print("Mind-warping forces emanate from your brain!");
				if (plev < 25)
				{
					project(0, 2 + plev / 10, p_ptr->py, p_ptr->px,
					        (plev*3) / 2, GF_PSI, PROJECT_KILL);
				}
				else
				{
					(void)mindblast_monsters(plev * ((plev - 5) / 10 + 1));
				}

				break;
			}

			/* Adrenaline */
		case 9:
			{
				set_afraid(0);
				set_stun(0);
				hp_player(plev);

				b = 10 + randint((plev * 3) / 2);

				if (plev < 35)
				{
					set_hero(p_ptr->hero + b);
				}
				else
				{
					set_shero(p_ptr->shero + b);
				}

				if (!p_ptr->fast)
				{
					/* Haste */
					(void)set_fast(b, plev / 5);
				}
				else
				{
					(void)set_fast(p_ptr->fast + b, plev / 5);
				}

				break;
			}

			/* Psychic Drain */
		case 10:
			{
				if (!get_aim_dir(&dir)) return;

				b = damroll(plev / 2, 6);

				if (fire_ball(GF_PSI_DRAIN, dir, b, 0 + (plev - 25) / 10))
				{
					p_ptr->energy -= randint(150);
				}

				break;
			}

			/* Telekinesis */
		case 11:
			{
				msg_print("A wave of pure physical force radiates out from your body!");
				project(0, 3 + plev / 10, p_ptr->py, p_ptr->px,
				        plev * (plev > 39 ? 4 : 3), GF_TELEKINESIS,
				        PROJECT_KILL | PROJECT_ITEM | PROJECT_GRID);

				break;
			}

		default:
			{
				msg_print("Zap?");

				break;
			}
		}
	}

	/* Take a turn */
	energy_use = 100;

	/* Sufficient mana */
	if (spell.mana_cost <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= spell.mana_cost;
	}

	/* Over-exert the player */
	else
	{
		int oops = spell.mana_cost - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(randint(5 * oops + 1));

		/* Damage WIS (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool_ perm = (rand_int(100) < 25);

			/* Message */
			msg_print("You have damaged your mind!");

			/* Reduce constitution */
			(void)dec_stat(A_WIS, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


static int get_mimic_chance(int mimic)
{
	s32b chance;

	chance = get_mimic_level(mimic);
	chance *= 3;

	chance -= get_skill_scale(SKILL_MIMICRY, 150);
	chance -= 3 * adj_mag_stat[p_ptr->stat_ind[A_DEX]];

	/* Return the chance */
	return clamp_failure_chance(chance, 2);
}


void do_cmd_mimic_lore()
{
	auto const &k_info = game->edit_data.k_info;

	int fail;

	object_type	*o_ptr;


	/* Player has to be able to see */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* No transformations when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}


	/* Already in a mimic form -- Allow cancelling */
	if (p_ptr->mimic_form)
	{
		msg_print("You morph back to your natural form!");

		set_mimic(0, 0, 0);
	}

	/* Not in mimic forms -- Allow transformations */
	else
	{
		o_ptr = &p_ptr->inventory[INVEN_OUTER];

		if ((o_ptr->tval != TV_CLOAK) || (o_ptr->sval != SV_MIMIC_CLOAK))
		{
			msg_print("You are not wearing any cloaks of mimicry.");
			return;
		}

		/* Calculate failure rate */
		fail = get_mimic_chance(o_ptr->pval2);

		if (fail > 75)
		{
			msg_print("You feel uneasy with this shape-change.");

			if (!get_check("Try it anyway? ")) return;
		}

		/* Fumble */
		if (randint(100) < fail)
		{
			msg_print("Your shape-change goes horribly wrong!");

			if (randint(100) < p_ptr->skill_sav)
			{
				msg_print("You manage to wrest your body back under control.");
				return;
			}

			set_mimic(30, resolve_mimic_name("Abomination"), get_skill(SKILL_MIMICRY));
		}

		/* Success */
		else
		{
			set_mimic(k_info[o_ptr->k_idx].pval2 + get_skill_scale(SKILL_MIMICRY, 1000), o_ptr->pval2, get_skill(SKILL_MIMICRY));
		}
	}


	/* Redraw title */
	p_ptr->redraw |= (PR_FRAME);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
}

static bool_ mimic_forbid_travel(void *, void *, void *)
{
	u32b value = p_ptr->mimic_extra >> 16;
	u32b att = p_ptr->mimic_extra & 0xFFFF;

	if(value > 0 && (att & CLASS_ARMS || att & CLASS_LEGS))
	{
		msg_print("You had best not travel with your extra limbs.");
		return TRUE;
	}

	return FALSE;
}

/*
 * do_cmd_cast calls this function if the player's class
 * is 'mimic'.
 */
void do_cmd_mimic(void)
{
	int n = 0, b = 0;

	int fail;

	int minfail = 0;

	int plev = get_skill(SKILL_MIMICRY);

	magic_power spell;

	static bool_ added_hooks = FALSE;
	if(!added_hooks)
	{
		add_hook_new(HOOK_FORBID_TRAVEL, mimic_forbid_travel, "mimic_forbid_travel", NULL);
		added_hooks = TRUE;
	}

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


	/* not if confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* get power */
	if (!get_magic_power(&n, mimic_powers, MAX_MIMIC_POWERS, mimic_info,
	                     plev, A_DEX)) return;

	spell = mimic_powers[n];

	/* Verify "dangerous" spells */
	if (spell.mana_cost > p_ptr->csp)
	{
		/* Warning */
		msg_print("You do not have enough mana to use this power.");

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}

	/* Spell failure chance */
	fail = spell.fail;

	/* Reduce failure rate by "effective" level adjustment */
	fail -= 3 * (plev - spell.min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	fail -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_DEX]] - 1);

	/* Not enough mana to cast */
	if (spell.mana_cost > p_ptr->csp)
	{
		fail += 5 * (spell.mana_cost - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_DEX]];

	/* Minimum failure rate */
	if (fail < minfail) fail = minfail;

	/* Stunning makes spells harder */
	if (p_ptr->stun > 50) fail += 25;
	else if (p_ptr->stun) fail += 15;

	/* Always a 5 percent chance of working */
	if (fail > 95) fail = 95;

	/* Failed spell */
	if (rand_int(100) < fail)
	{
		flush_on_failure();

		msg_format("You failed to concentrate hard enough!");

		if (randint(100) < (fail / 2))
		{
			/* Backfire */
			b = randint(100);

			if (b < 5)
			{
				msg_print("Oh, no! Your mind has gone blank!");
				lose_all_info();
			}
			else if (b < 15)
			{
				msg_print("Weird visions seem to dance before your eyes...");
				set_image(p_ptr->image + 5 + randint(10));
			}
			else if (b < 45)
			{
				msg_print("Your brain is addled!");
				set_confused(p_ptr->confused + randint(8));
			}
			else
			{
				set_stun(p_ptr->stun + randint(8));
			}
		}
	}

	/* Successful spells */
	else
	{
		/* spell code */
		switch (n)
		{
			/* Mimic */
		case 0:
			{
				do_cmd_mimic_lore();

				break;
			}

			/* Invisibility */
		case 1:
			{
				int ii = 10 + plev + randint(20) + p_ptr->to_s;

				set_invis(p_ptr->tim_invisible + ii, 50);
				set_tim_invis(p_ptr->tim_invisible + ii);

				break;
			}

			/* Legs Mimicry */
		case 2:
			{
				/* Extract the value and the flags */
				u32b value = p_ptr->mimic_extra >> 16;
				u32b att = p_ptr->mimic_extra & 0xFFFF;

				/* Clear useless things */
				att &= ~(CLASS_ARMS);
				att &= ~(CLASS_WALL);

				if (att & CLASS_LEGS)
				{
					value += 50 + randint(50 + (2 * plev));
				}
				else
				{
					msg_print("You mimic a new pair of legs.");

					value = 50 + randint(50 + (2 * plev));
					att |= (CLASS_LEGS);
				}

				if (value > 10000) value = 10000;

				p_ptr->mimic_extra = att + (value << 16);
				p_ptr->update |= (PU_BODY);

				break;
			}

			/* Wall Mimicry */
		case 3:
			{
				/* Extract the value and the flags */
				u32b value = p_ptr->mimic_extra >> 16;
				u32b att = p_ptr->mimic_extra & 0xFFFF;

				/* Clear useless things */
				att &= ~(CLASS_ARMS);
				att &= ~(CLASS_LEGS);

				if (att & CLASS_WALL)
				{
					value += 50 + randint(50 + (2 * plev));
				}
				else
				{
					msg_print("You grow an affinity for walls.");

					value = 50 + randint(50 + (2 * plev));
					att |= (CLASS_WALL);
				}

				if (value > 10000) value = 10000;

				p_ptr->mimic_extra = att + (value << 16);
				p_ptr->update |= (PU_BODY);

				break;
			}

		case 4:    /* Arms Mimicry */
			{
				/* Extract the value and the flags */
				u32b value = p_ptr->mimic_extra >> 16;
				u32b att = p_ptr->mimic_extra & 0xFFFF;

				/* Clear useless things */
				att &= ~(CLASS_LEGS);
				att &= ~(CLASS_WALL);

				if (att & CLASS_ARMS)
				{
					value += 50 + randint(50 + (2 * plev));
				}
				else
				{
					msg_print("You mimic a new pair of arms.");

					value = 50 + randint(50 + (2 * plev));
					att |= (CLASS_ARMS);
				}

				if (value > 10000) value = 10000;

				p_ptr->mimic_extra = att + (value << 16);
				p_ptr->update |= (PU_BODY);

				break;
			}

		default:
			{
				msg_print("Zap?");

				break;
			}
		}
	}


	/* Take a turn */
	energy_use = 100;

	/* Sufficient mana */
	if (spell.mana_cost <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= spell.mana_cost;
	}

	/* Over-exert the player */
	else
	{
		int oops = spell.mana_cost - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(randint(5 * oops + 1));

		/* Damage WIS (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool_ perm = (rand_int(100) < 25);

			/* Message */
			msg_print("You have damaged your mind!");

			/* Reduce constitution */
			(void)dec_stat(A_DEX, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


/*
 * do_cmd_cast calls this function if the player's class
 * is 'beastmaster'.
 */
void do_cmd_beastmaster(void)
{
	int plev = p_ptr->lev, i, num;

	monster_type *m_ptr;


	/* Process the monsters (backwards) */
	num = 0;
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		m_ptr = &m_list[i];

		if (m_ptr->status == MSTATUS_PET)
		{
			num++;
		}
	}

	if (num < plev * 2)
	{
		/* XXX XXX */
		if (rand_int(80-(plev) - p_ptr->stat_use[5]-p_ptr->to_s) < 20)
		{
			summon_specific_friendly(p_ptr->py, p_ptr->px, plev, rand_int(plev / 2), FALSE);
		}
	}
	else msg_print("You can't summon more pets");

	/* Take a turn */
	if (is_magestaff()) energy_use = 80;
	else energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}


/*
 * Command to ask favors from your god.
 */
void do_cmd_pray(void)
{
	if (p_ptr->pgod == GOD_NONE)
	{
		msg_print("Pray hard enough and your prayers might be answered.");
		return;
	}
	else
	{
		if (!p_ptr->praying)
			msg_format("You start praying to %s.", deity_info[p_ptr->pgod].name);
		else
			msg_format("You stop praying to %s.", deity_info[p_ptr->pgod].name);
		p_ptr->praying = !p_ptr->praying;

		/* Update stuffs */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
		                  PU_SANITY | PU_BODY);

		p_ptr->redraw |= PR_WIPE | PR_FRAME | PR_MAP;
		energy_use = 100;
	}
}


/*
 * Return percentage chance of spell failure.
 */
int spell_chance_random(random_spell const *rspell)
{
	int chance, minfail;


	/* Extract the base spell failure rate */
	chance = rspell->level + 10;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (get_skill(SKILL_THAUMATURGY) - rspell->level);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_INT]] - 1);

	/* Not enough mana to cast */
	if (rspell->mana > p_ptr->csp)
	{
		chance += 5 * (rspell->mana - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_INT]];

	/* Failure rate */
	return clamp_failure_chance(chance, minfail);
}




/*
 * Print a batch of spells.
 */
static void print_spell_batch(int batch, int max)
{
	auto const &random_spells = p_ptr->random_spells;

	prt(format("      %-30s Lev Fail Mana Damage ", "Name"), 1, 20);

	int i;
	for (i = 0; i < max; i++)
	{
		auto rspell = &random_spells[batch * 10 + i];

		char buff[80];

		if (rspell->untried)
		{
			strnfmt(buff, 80, "  %c) %-30s  (Spell untried)  ",
			        I2A(i), rspell->name);

		}
		else
		{
			strnfmt(buff, 80, "  %c) %-30s %3d %4d%% %3d %3dd%d ",
			        I2A(i), rspell->name,
			        rspell->level, spell_chance_random(rspell), rspell->mana,
			        rspell->dam_dice, rspell->dam_sides);
		}

		prt(buff, 2 + i, 20);
	}

	prt("", 2 + i, 20);
}



/*
 * List ten random spells and ask to pick one.
 */
static random_spell* select_spell_from_batch(std::size_t batch)
{
	auto &random_spells = p_ptr->random_spells;

	char tmp[160];
	char out_val[30];
	char which;
	random_spell* ret = nullptr;

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	int const mut_max = (random_spells.size() < (batch + 1) * 10)
	        ? random_spells.size() - batch * 10
	        : 10;

	strnfmt(tmp, 160, "(a-%c, A-%c to browse, / to rename, - to comment) Select a power: ",
	        I2A(mut_max - 1), I2A(mut_max - 1) - 'a' + 'A');

	prt(tmp, 0, 0);

	while (1)
	{
		/* Print power list */
		print_spell_batch(batch, mut_max);

		/* Get a command */
		which = inkey();

		/* Abort */
		if (which == ESCAPE)
		{
			/* No selection */
			ret = NULL;

			/* Leave the command loop */
			break;

		}

		/* Accept default */
		if (which == '\r')
		{
			/* There are no other choices */
			if (mut_max == 1)
			{
				ret = &random_spells[batch * 10];

				/* Leave the command loop */
				break;
			}

			/* Wait for next command */
			continue;
		}

		/* Rename */
		if (which == '/')
		{
			prt("Rename which power: ", 0, 0);
			which = tolower(inkey());

			if (isalpha(which) && (A2I(which) <= mut_max))
			{
				strcpy(out_val, random_spells[batch*10 + A2I(which)].name);
				if (get_string("Name this power: ", out_val, 29))
				{
					strcpy(random_spells[batch*10 + A2I(which)].name, out_val);
				}
				prt(tmp, 0, 0);
			}
			else
			{
				bell();
				prt(tmp, 0, 0);
			}

			/* Wait for next command */
			continue;
		}

		/* Comment */
		if (which == '-')
		{
			prt("Comment which power: ", 0, 0);
			which = tolower(inkey());

			if (isalpha(which) && (A2I(which) <= mut_max))
			{
				strcpy(out_val, random_spells[batch*10 + A2I(which)].desc);
				if (get_string("Comment this power: ", out_val, 29))
				{
					strcpy(random_spells[batch*10 + A2I(which)].desc, out_val);
				}
				prt(tmp, 0, 0);
			}
			else
			{
				bell();
				prt(tmp, 0, 0);
			}

			/* Wait for next command */
			continue;
		}

		if (isalpha(which) && isupper(which))
		{
			which = tolower(which);
			c_prt(TERM_L_BLUE, format("%s : %s", random_spells[batch*10 + A2I(which)].name, random_spells[batch*10 + A2I(which)].desc), 0, 0);
			inkey();
			prt(tmp, 0, 0);
			continue;
		}
		else if (isalpha(which) && (A2I(which) < mut_max))
		{
			/* Pick the power */
			ret = &random_spells[batch * 10 + A2I(which)];

			/* Leave the command loop */
			break;
		}
		else
		{
			bell();
		}
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;

	/* Return selection */
	return (ret);
}


/*
 * Pick a random spell from a menu
 */
static random_spell* select_spell()
{
	auto const &random_spells = p_ptr->random_spells;

	char tmp[160];
	char which;

	random_spell *ret;


	/* Too confused */
	if (p_ptr->confused)
	{
		msg_print("You can't use your powers while confused!");
		return NULL;
	}

	/* No spells available */
	if (random_spells.empty())
	{
		msg_print("There are no spells you can cast.");
		return NULL;
	}

	/* How many spells in the last batch? */
	int batch_max = (random_spells.size() - 1) / 10;

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	strnfmt(tmp, 160, "(a-%c) Select batch of powers: ", I2A(batch_max));

	prt(tmp, 0, 0);

	while (1)
	{
		which = inkey();

		if (which == ESCAPE)
		{
			Term_load();

			ret = NULL;

			break;
		}

		if (which == '\r')
		{
			if (batch_max == 0)
			{
				Term_load();

				ret = select_spell_from_batch(0);

				break;
			}

			continue;
		}

		which = tolower(which);
		if (isalpha(which) && (A2I(which) <= batch_max))
		{
			Term_load();

			ret = select_spell_from_batch(A2I(which));

			break;
		}
		else
		{
			bell();
		}
	}

	/* Leave "icky" mode */
	character_icky = FALSE;

	return (ret);
}


void do_cmd_powermage(void)
{
	random_spell *s_ptr;

	u32b proj_flags;

	int dir, chance;

	int ty = 0, tx = 0;


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


	s_ptr = select_spell();

	if (s_ptr == NULL) return;

	if (p_ptr->csp < s_ptr->mana)
	{
		msg_print("You do not have enough mana.");
		return;
	}

	/* Spell failure chance */
	chance = spell_chance_random(s_ptr);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		int insanity = (p_ptr->msane - p_ptr->csane) * 100 / p_ptr->msane;
		char sfail[80];

		/* Flush input if told so */
		flush_on_failure();

		/* Insane players can see something strange */
		if (rand_int(100) < insanity)
		{
			get_rnd_line("sfail.txt", sfail);
			msg_format("A cloud of %s appears above you.", sfail);
		}

		/* Normal failure messages */
		else
		{
			msg_print("You failed to get the spell off!");
		}

		/* Let time pass */
		if (is_magestaff()) energy_use = 80;
		else energy_use = 100;

		/* Mana is spent anyway */
		p_ptr->csp -= s_ptr->mana;

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->redraw |= (PR_FRAME);

		return;
	}


	p_ptr->csp -= s_ptr->mana;

	s_ptr->untried = false;
	proj_flags = s_ptr->proj_flags;

	/* Hack -- Spell needs a target */
	if ((s_ptr->proj_flags & PROJECT_BEAM) ||
	                (s_ptr->proj_flags & PROJECT_STOP))
	{
		if (!get_aim_dir(&dir)) return;

		/* Hack -- Use an actual "target" */
		if ((dir == 5) && target_okay())
		{
			tx = target_col;
			ty = target_row;

			/* Mega-Hack -- Beam spells should continue through
				 * the target; bolt spells should stop at the
				 * target. --dsb */
			if (s_ptr->proj_flags & PROJECT_BEAM)
				proj_flags |= PROJECT_THRU;
		}
		else
		{
			/* Use the given direction */
			ty = p_ptr->py + ddy[dir];
			tx = p_ptr->px + ddx[dir];

			/* Mega-Hack -- Both beam and bolt spells should
				 * continue through this fake target. --dsb */
			proj_flags |= PROJECT_THRU;
		}
	}

	if (s_ptr->proj_flags & PROJECT_BLAST)
	{
		ty = p_ptr->py;
		tx = p_ptr->px;
	}

	if (s_ptr->proj_flags & PROJECT_VIEWABLE)
	{
		project_hack(s_ptr->GF, damroll(s_ptr->dam_dice, s_ptr->dam_sides));
	}
	else if (s_ptr->proj_flags & PROJECT_METEOR_SHOWER)
	{
		project_meteor(s_ptr->radius, s_ptr->GF,
		               damroll(s_ptr->dam_dice, s_ptr->dam_sides),
		               s_ptr->proj_flags);
	}
	else
	{
		project(0, s_ptr->radius, ty, tx,
		        damroll(s_ptr->dam_dice, s_ptr->dam_sides),
		        s_ptr->GF, proj_flags);
	}

	/* Take a turn */
	if (is_magestaff()) energy_use = 80;
	else energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_FRAME);
}


/*
 * Brand some ammunition.  Used by Cubragol and a mage spell.  The spell was
 * moved here from cmd6.c where it used to be for Cubragol only.  I've also
 * expanded it to do either frost, fire or venom, at random. -GJW	-KMW-
 */
void brand_ammo(int brand_type, int bolts_only)
{
	int a;

	for (a = 0; a < INVEN_PACK; a++)
	{
		object_type *o_ptr = &p_ptr->inventory[a];

		if (bolts_only && (o_ptr->tval != TV_BOLT)) continue;

		if (!bolts_only && (o_ptr->tval != TV_BOLT) &&
		                (o_ptr->tval != TV_ARROW) && (o_ptr->tval != TV_SHOT))
			continue;

		if (!artifact_p(o_ptr) && !ego_item_p(o_ptr) &&
		                !cursed_p(o_ptr))
			break;
	}

	/* Enchant the ammo (or fail) */
	if ((a < INVEN_PACK) && (rand_int(100) < 50))
	{
		object_type *o_ptr = &p_ptr->inventory[a];
		const char *ammo_name;
		const char *aura_name;
		char msg[48];
		int aura_type, r;

		/* fire only */
		if (brand_type == 1) r = 0;

		/* cold only */
		else if (brand_type == 2) r = 99;

		/* No bias */
		else r = rand_int(100);

		if (r < 50)
		{
			aura_name = "fiery";
			aura_type = EGO_FLAME;
		}
		else
		{
			aura_name = "frosty";
			aura_type = EGO_FROST;
		}

		if (o_ptr->tval == TV_BOLT)
		{
			ammo_name = "bolts";
		}
		else if (o_ptr->tval == TV_ARROW)
		{
			ammo_name = "arrows";
		}
		else
		{
			ammo_name = "shots";
		}

		strnfmt(msg, 48, "Your %s are covered in a %s aura!",
		        ammo_name, aura_name);
		msg_print(msg);

		o_ptr->name2 = aura_type;

		/* Apply the ego */
		apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
		o_ptr->discount = 100;

		enchant(o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);
	}
	else
	{
		flush_on_failure();
		msg_print("The enchantment failed.");
	}
}


/*
 * From Kamband by Ivan Tkatchev
 */
void summon_monster(int sumtype)
{
	/* Take a turn */
	energy_use = 100;

	if (summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level + randint(5), sumtype, TRUE))
	{
		msg_print("You summon some help.");
	}
	else
	{
		msg_print("You called, but no help came.");
	}
}



/*
 * Use a class power of Possessor
 */
void do_cmd_possessor()
{
	auto const &r_info = game->edit_data.r_info;

	char ch, ext;


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


	while (TRUE)
	{
		if (!get_com("Use your [R]ace powers or your [I]ncarnating powers?", &ch))
		{
			ext = 0;
			break;
		}
		if ((ch == 'R') || (ch == 'r'))
		{
			ext = 1;
			break;
		}
		if ((ch == 'I') || (ch == 'i'))
		{
			ext = 2;
			break;
		}
	}

	if (ext == 1)
	{
		if (p_ptr->disembodied)
		{
			msg_print("You don't currently own a body to use.");
			return;
		}

		/* Do we have access to all the powers ? */
		bool use_great = (get_skill_scale(SKILL_POSSESSION, 100) >= r_info[p_ptr->body_monster].level);

		/* Select power */
		use_monster_power(p_ptr->body_monster, use_great);
		assert(p_ptr->csp >= 0); // Sanity check
	}
	else if (ext == 2)
	{
		if (p_ptr->disembodied)
		{
			do_cmd_integrate_body();
		}
		else
		{
			do_cmd_leave_body(TRUE);
		}
	}
	else
	{
		return;
	}

	/* Take a turn */
	energy_use = 100;
}


/*
 * Hook to determine if an object is contertible in an arrow/bolt
 */
static object_filter_t const &item_tester_hook_convertible()
{
	using namespace object_filter;
	static auto instance =
		Or(
			TVal(TV_JUNK),
			TVal(TV_SKELETON));
	return instance;
}


/*
 * do_cmd_cast calls this function if the player's class
 * is 'archer'.
 */
void do_cmd_archer(void)
{
	int ext = 0;
	char ch;

	object_type	forge;
	object_type *q_ptr;

	char com[80];


	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	if (p_ptr->blind)
	{
		msg_print("You are blind!");
		return;
	}


	if (get_skill(SKILL_ARCHERY) >= 20)
	{
		strnfmt(com, 80, "Create [S]hots, [A]rrows or [B]olts? ");
	}
	else if (get_skill(SKILL_ARCHERY) >= 10)
	{
		strnfmt(com, 80, "Create [S]hots or [A]rrows? ");
	}
	else
	{
		strnfmt(com, 80, "Create [S]hots? ");
	}

	while (TRUE)
	{
		if (!get_com(com, &ch))
		{
			ext = 0;
			break;
		}
		if ((ch == 'S') || (ch == 's'))
		{
			ext = 1;
			break;
		}
		if (((ch == 'A') || (ch == 'a')) && (get_skill(SKILL_ARCHERY) >= 10))
		{
			ext = 2;
			break;
		}
		if (((ch == 'B') || (ch == 'b')) && (get_skill(SKILL_ARCHERY) >= 20))
		{
			ext = 3;
			break;
		}
	}

	/* Prepare for object creation */
	q_ptr = &forge;

	/**********Create shots*********/
	if (ext == 1)
	{
		int x, y, dir;
		cave_type *c_ptr;

		if (!get_rep_dir(&dir)) return;
		y = p_ptr->py + ddy[dir];
		x = p_ptr->px + ddx[dir];
		c_ptr = &cave[y][x];
		if (c_ptr->feat == FEAT_RUBBLE)
		{
			/* Get local object */
			q_ptr = &forge;

			/* Hack -- Give the player some shots */
			object_prep(q_ptr, lookup_kind(TV_SHOT, m_bonus(2, dun_level)));
			if (!artifact_p(q_ptr))
				q_ptr->number = (byte)rand_range(15, 30);
			else
				q_ptr->number = 1;
			object_aware(q_ptr);
			object_known(q_ptr);
			q_ptr->ident |= IDENT_MENTAL;
			apply_magic(q_ptr, dun_level, TRUE, TRUE, (magik(20)) ? TRUE : FALSE);
			q_ptr->discount = 90;
			q_ptr->found = OBJ_FOUND_SELFMADE;

			(void)inven_carry(q_ptr, FALSE);

			msg_print("You make some ammo.");

			(void)wall_to_mud(dir);
			p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);
			p_ptr->window |= (PW_OVERHEAD);
		}
	}

	/**********Create arrows*********/
	else if (ext == 2)
	{
		int item;

		/* Get an item */
		if (!get_item(&item,
			      "Convert which item? ",
			      "You have no item to convert.",
			      (USE_INVEN | USE_FLOOR),
			      item_tester_hook_convertible())) return;

		/* Get local object */
		q_ptr = &forge;

		/* Hack -- Give the player some arrows */
		object_prep(q_ptr, lookup_kind(TV_ARROW, m_bonus(1, dun_level) + 1));
		q_ptr->number = (byte)rand_range(15, 25);
		if (!artifact_p(q_ptr))
			q_ptr->number = (byte)rand_range(15, 30);
		else
			q_ptr->number = 1;
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->ident |= IDENT_MENTAL;
		apply_magic(q_ptr, dun_level, TRUE, TRUE, (magik(20)) ? TRUE : FALSE);
		q_ptr->discount = 90;
		q_ptr->found = OBJ_FOUND_SELFMADE;

		msg_print("You make some ammo.");

		inc_stack_size(item, -1);

		(void)inven_carry(q_ptr, FALSE);
	}

	/**********Create bolts*********/
	else if (ext == 3)
	{
		int item;

		/* Get an item */
		if (!get_item(&item,
			      "Convert which item? ",
			      "You have no item to convert.",
			      (USE_INVEN | USE_FLOOR),
			      item_tester_hook_convertible())) return;

		/* Get local object */
		q_ptr = &forge;

		/* Hack -- Give the player some bolts */
		object_prep(q_ptr, lookup_kind(TV_BOLT, m_bonus(1, dun_level) + 1));
		q_ptr->number = (byte)rand_range(15, 25);
		if (!artifact_p(q_ptr))
			q_ptr->number = (byte)rand_range(15, 30);
		else
			q_ptr->number = 1;
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->ident |= IDENT_MENTAL;
		apply_magic(q_ptr, dun_level, TRUE, TRUE, (magik(20)) ? TRUE : FALSE);
		q_ptr->discount = 90;
		q_ptr->found = OBJ_FOUND_SELFMADE;

		msg_print("You make some ammo.");

		inc_stack_size(item, -1);

		(void)inven_carry(q_ptr, FALSE);
	}
}

/*
 * Control whether shots are allowed to pierce
 */
void do_cmd_set_piercing(void)
{
	char ch;
	char com[80];

	if ((get_skill(SKILL_BOW) <= 25) && (get_skill(SKILL_XBOW) <= 25) &&
	    (get_skill(SKILL_SLING) <= 25))
	{
		msg_print("You can't fire piercing shots yet.");
		return;
	}

	strnfmt(com, 80, "Allow shots to pierce? ");

	while (TRUE)
	{
		if (!get_com(com, &ch))
		{
			break;
		}
		if ((ch == 'Y') || (ch == 'y'))
		{
			p_ptr->use_piercing_shots = 1;
			msg_print("Piercing shots activated.");
			break;
		}
		if ((ch == 'N') || (ch == 'n'))
		{
			p_ptr->use_piercing_shots = 0;
			msg_print("Piercing shots deactivated.");
			break;
		}
	}
}
/*
 * Helper function to describe necro powers
 */
void necro_info(char *p, int power)
{
	int plev = get_skill(SKILL_NECROMANCY);

	strcpy(p, "");

	switch (power)
	{
	case 0:
		{
			if (p_ptr->to_s)
				strnfmt(p, 80, " power %dd%d+%d", 2 + (plev * 2 / 3), 4, (p_ptr->to_s * 2));
			else
				strnfmt(p, 80, " power %dd%d", 2 + (plev * 2 / 3), 4);
			break;
		}
	case 2:
		{
			strnfmt(p, 80, " dur d%d+%d", 100 + (plev * 4), 200 + (plev * 3));
			break;
		}
	case 3:
		{
			strnfmt(p, 80, " dur d%d+%d", 30 + (plev * 2), 50 + plev);
			break;
		}
	}
}


/*
 * Cast a Necromancy spell
 */
void do_cmd_necromancer(void)
{
	int n = 0, b = 0;
	int chance;
	int dir;
	int minfail = 0;
	int plev = get_skill(SKILL_NECROMANCY);
	magic_power spell;
	int to_s2 = p_ptr->to_s / 2;
	int mto_s2 = p_ptr->to_s / 2;


	if (mto_s2 == 0) mto_s2 = 1;

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

	/* not if confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* get power */
	if (!get_magic_power(&n, necro_powers, MAX_NECRO_POWERS, necro_info,
	                     get_skill(SKILL_NECROMANCY), A_CON)) return;

	spell = necro_powers[n];

	/* Verify "dangerous" spells */
	if (spell.mana_cost > p_ptr->csp)
	{
		/* Warning */
		msg_print("You do not have enough mana to use this power.");

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}

	/* Spell failure chance */
	chance = spell.fail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (plev - spell.min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_CON]] - 1);

	/* Not enough mana to cast */
	if (spell.mana_cost > p_ptr->csp)
	{
		chance += 5 * (spell.mana_cost - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_CON]];

	/* Failure rate */
	chance = clamp_failure_chance(chance, minfail);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		flush_on_failure();
		msg_format("You failed to concentrate hard enough!");

		if (randint(100) < (chance / 2))
		{
			/* Backfire */
			b = randint(100);
			if (b < 10)
			{
				msg_print("Oh, no! You become undead!");

				p_ptr->necro_extra |= CLASS_UNDEAD;
				p_ptr->necro_extra2 = 2 * plev;
				msg_format("You have to kill %d monster%s to be brought back to life.",
				           p_ptr->necro_extra2,
				           (p_ptr->necro_extra2 == 1) ? "" : "s");

				/* MEGA-HACK !!! */
				calc_hitpoints();

				/* Enforce maximum */
				p_ptr->chp = p_ptr->mhp;
				p_ptr->chp_frac = 0;

				/* Display the hitpoints */
				p_ptr->redraw |= (PR_FRAME);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER);
			}
			else if (b < 40)
			{
				msg_print("Suddenly you feel that you're in a bad situation...");
				summon_specific(p_ptr->py, p_ptr->px, max_dlv[dungeon_type],
				                (plev >= 30) ? SUMMON_HI_UNDEAD : SUMMON_UNDEAD);
			}
			else
			{
				msg_print("Your body is damaged by the horrible forces of the spell!");
				take_hit(damroll(5, plev), "using necromancy unwisely");
			}
		}
	}
	else
	{
		/* spell code */
		switch (n)
		{
			/* Horrify */
		case 0:
			{
				int dam = damroll(2 + (plev * 2 / 3), 4) + (p_ptr->to_s * 2);

				if (plev > 45)
				{
					project_hack(GF_STUN, dam);
					project_hack(GF_TURN_ALL, dam);
				}
				else if (plev > 35)
				{
					if (!get_aim_dir(&dir)) return;
					fire_ball(GF_STUN, dir, dam, 3 + (plev / 10));
					fire_ball(GF_TURN_ALL, dir, dam, 3 + (plev / 10));
				}
				else if (plev > 20)
				{
					if (!get_aim_dir(&dir)) return;
					fire_beam(GF_STUN, dir, dam);
					fire_beam(GF_TURN_ALL, dir, dam);
				}
				else
				{
					if (!get_aim_dir(&dir)) return;
					fire_bolt(GF_STUN, dir, dam);
					fire_bolt(GF_TURN_ALL, dir, dam);
				}

				break;
			}

			/* Raise Death */
		case 1:
			{
				fire_ball(GF_RAISE, 0, plev * 3, 1 + to_s2 + (plev / 10));

				break;
			}

			/* Conjures temporary weapon */
		case 2:
			{
				int dur = randint(100 + (plev * 4)) + 200 + (plev * 3);
				object_type forge, *o_ptr = &forge;
				int k_idx = test_item_name("& Necromantic Teeth~");

				k_allow_special[k_idx] = TRUE;

				object_prep(o_ptr, k_idx);
				apply_magic(o_ptr, plev * 2, TRUE, TRUE, TRUE);

				o_ptr->art_flags |= TR_TEMPORARY;
				o_ptr->timeout = dur;

				/* These objects are "storebought" */
				o_ptr->ident |= IDENT_MENTAL;
				o_ptr->number = 1;

				object_aware(o_ptr);
				object_known(o_ptr);
				(void)inven_carry(o_ptr, FALSE);

				k_allow_special[k_idx] = FALSE;

				break;
			}

			/* Absorb souls */
		case 3:
			{
				set_absorb_soul(randint(30 + (plev * 2)) + 50 + plev);
				break;
			}

			/* Vampirism */
		case 4:
			{
				int i;
				if (!get_aim_dir(&dir)) return;
				for (i = 0; i < 1 + to_s2 + (plev / 15); i++)
				{
					if (drain_life(dir, 100))
						hp_player(100);
				}

				break;
			}

			/* Death */
		case 5:
			{
				if (get_check("Using the Death word will leave you undead, with 1 DP. Do you *really* want to use it? "))
				{
					if (!get_aim_dir(&dir)) return;
					fire_bolt(GF_DEATH, dir, 1);

					p_ptr->necro_extra |= CLASS_UNDEAD;
					p_ptr->necro_extra2 = plev + (rand_int(plev / 2) - (plev / 4));
					msg_format("You have to kill %d monster%s to be brought back to life.", p_ptr->necro_extra2, (p_ptr->necro_extra2 == 1) ? "" : "s");

					/* MEGA-HACK !!! */
					calc_hitpoints();

					/* Enforce 1 DP */
					p_ptr->chp = p_ptr->mhp;
					p_ptr->chp_frac = 0;

					/* Display the hitpoints */
					p_ptr->redraw |= (PR_FRAME);

					/* Window stuff */
					p_ptr->window |= (PW_PLAYER);
				}

				break;
			}

		default:
			{
				msg_print("Zap?");

				break;
			}
		}
	}

	/* Take a turn */
	if (is_magestaff()) energy_use = 80;
	else energy_use = 100;

	/* Sufficient mana */
	if (spell.mana_cost <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= spell.mana_cost;
	}

	/* Over-exert the player */
	else
	{
		int oops = spell.mana_cost - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool_ perm = (rand_int(100) < 25);

			/* Message */
			msg_print("You have damaged your body!");

			/* Reduce constitution */
			(void)dec_stat(A_CON, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

/*
 * math.h(sqrt) is banned of angband so ... :)
 */
s32b sroot(s32b n)
{
	s32b i = n / 2;

	if (n < 2) return (n);

	while (1)
	{
		s32b err = (i - n / (i + 1)) / 2;

		if (!err) break;

		i -= err;
	}

	return ((n / i < i) ? (i - 1) : i);
}


void do_cmd_unbeliever_antimagic()
{
	if (get_skill(SKILL_ANTIMAGIC) < 20)
	{
		msg_print("You must have at least a level 20 antimagic skill "
		          "to be able to disrupt the magic continuum.");
		return;
	}

	if (p_ptr->antimagic_extra & CLASS_ANTIMAGIC)
	{
		p_ptr->antimagic_extra &= ~CLASS_ANTIMAGIC;
		msg_print("You stop disrupting the magic continuum.");
	}
	else
	{
		p_ptr->antimagic_extra |= CLASS_ANTIMAGIC;
		msg_print("You start disrupting the magic continuum.");
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
}


/*
 * Detect traps + kill traps
 */
void do_cmd_unbeliever()
{
	int ext = 0;

	char ch;


	/* Select what to do */
	while (TRUE)
	{
		if (!get_com("Disrupt [C]ontinuum or [D]estroy Doors", &ch))
		{
			ext = 0;
			break;
		}
		if ((ch == 'C') || (ch == 'c'))
		{
			ext = 1;
			break;
		}
		if ((ch == 'D') || (ch == 'd'))
		{
			ext = 2;
			break;
		}
	}

	switch (ext)
	{
		/* Disrupt Continuum */
	case 1:
		{
			do_cmd_unbeliever_antimagic();
			break;
		}

		/* Destroy Doors */
	case 2:
		{
			s16b skill = get_skill(SKILL_ANTIMAGIC);

			if (skill < 25)
			{
				msg_print("You cannot use your door destruction abilities yet.");
				break;
			}

			destroy_doors_touch();

			break;
		}
	}
}

/*
 * Hook to determine if an object is totemable
 */
static object_filter_t const &item_tester_hook_totemable()
{
	using namespace object_filter;
	static auto instance = And(
		TVal(TV_CORPSE),
		Or(
			SVal(SV_CORPSE_CORPSE),
			SVal(SV_CORPSE_SKELETON)));
	return instance;
}


/*
 * Summoners
 */
void do_cmd_summoner_extract()
{
	auto const &r_info = game->edit_data.r_info;

	object_type forge, *q_ptr;

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Use which corpse? ",
		      "You have no corpse to use.",
		      (USE_INVEN | USE_FLOOR),
		      item_tester_hook_totemable()))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	bool_ partial;
	if (r_info[o_ptr->pval2].flags & RF_UNIQUE)
	{
		partial = FALSE;
	}
	else
	{
		partial = get_check("Do you want to create a partial totem?");
	}

	int r = o_ptr->pval2;

	inc_stack_size(item, -1);

	if (magik(r_info[o_ptr->pval2].level - get_skill(SKILL_SUMMON)))
	{
		msg_print("You failed to extract a totem.");
		energy_use += 100;
		return;
	}

	/* Prepare for object creation */
	q_ptr = &forge;

	/* Create the object */
	object_prep(q_ptr, lookup_kind(TV_TOTEM, partial ? 1 : 2));
	q_ptr->pval = r;
	q_ptr->pval2 = 0;
	q_ptr->number = 1;
	q_ptr->found = OBJ_FOUND_SELFMADE;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_MENTAL;
	(void)inven_carry(q_ptr, FALSE);

	msg_print("You extract a totem from the dead corpse.");
	energy_use += 100;
}


void summon_true(int r_idx, int item)
{
	auto const &r_info = game->edit_data.r_info;

	int i, status, x = 1, y = 1, rx, ry = 0, chance;

	bool_ used;

	auto r_ptr = &r_info[r_idx];


	/* Uniques are less likely to be nice */
	if (r_ptr->flags & RF_UNIQUE)
	{
		/* Because it's unique, it will always be destroyed */
		used = TRUE;

		/* About twice as hard as non-uniques */
		chance = (get_skill(SKILL_SUMMON) * 70 / (r_ptr->level + 1));

		if (magik(chance))
		{
			status = MSTATUS_PET;
		}
		else
		{
			status = MSTATUS_ENEMY;
		}
	}

	/* Non-uniques are easier to handle */
	else
	{
		if (get_skill(SKILL_SUMMON) == 0)
		{
			used = TRUE;
		}
		else
		{
			/* It can be used multiple times */
			used = FALSE;

			/* But it is not 100% sure (note: skill > 0) */
			chance = (r_ptr->level * 25 / get_skill(SKILL_SUMMON));
			if (magik(chance)) used = TRUE;
		}

		chance = (get_skill(SKILL_SUMMON) * 130 / (r_ptr->level + 1));

		if (magik(chance))
		{
			status = MSTATUS_PET;
		}
		else
		{
			status = MSTATUS_ENEMY;
		}
	}

	/* Find a grid where the monster is summoned */
	for (i = 0; i < 40; i++)
	{
		rx = (rand_int(8) - 4) + p_ptr->px;
		ry = (rand_int(8) - 4) + p_ptr->py;
		if (in_bounds(ry, rx) && cave_empty_bold(ry, rx))
		{
			x = rx;
			y = ry;
			break;
		}
	}

	/* No room found */
	if (i == 40)
	{
		msg_print("The summoning fails due to lack of room.");
		return;
	}

	/* Summon the monster */
	bypass_r_ptr_max_num = TRUE;
	if (!(i = place_monster_one (y, x, r_idx, 0, 0, status)))
	{
		msg_print("The summoning fails.");
	}
	else
	{
		m_list[i].status = status;
		m_list[i].mflag |= MFLAG_NO_DROP;
	}
	bypass_r_ptr_max_num = FALSE;

	/* Destroy the totem if the used flag is set */
	if (used)
	{
		/* Eliminate the totem */
		inc_stack_size(item, -1);
	}

	/* Done */
	return;
}


void do_cmd_summoner_summon()
{
	int item, x = 1, y = 1, rx, ry, m_idx = 0, i;

	cptr q, s;

	monster_type *m_ptr;


	/* Which Totem? */
	q = "Summon from which Totem?";
	s = "There are no totems to summon from!";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR), object_filter::TVal(TV_TOTEM))) return;

	/* Access the item */
	object_type *o_ptr = get_object(item);

	/* Take a turn */
	energy_use = 100;

	/* True Totems have their own function. */
	if (o_ptr->sval == 2)
	{
		summon_true(o_ptr->pval, item);
		return;
	}

	/* Handle partial totems */

	/* Find a grid where the monster is summoned */
	for (i = 0; i < 40; i++)
	{
		rx = (rand_int(8) - 4) + p_ptr->px;
		ry = (rand_int(8) - 4) + p_ptr->py;
		if (in_bounds(ry, rx) && cave_empty_bold(ry, rx))
		{
			x = rx;
			y = ry;
			break;
		}
	}

	/* No room found */
	if (i == 40)
	{
		msg_print("The summoning fails due to lack of room.");
		return;
	}

	/* Summon the monster */
	bypass_r_ptr_max_num = TRUE;
	place_monster_one_no_drop = TRUE;
	m_idx = place_monster_one(y, x, o_ptr->pval, 0, 0, MSTATUS_PET);
	bypass_r_ptr_max_num = FALSE;

	/* Failure. */
	if (!m_idx)
	{
		msg_print("The summoning fails.");
	}

	/* Mark the monster as a "partial" ally */
	m_ptr = &m_list[m_idx];
	m_ptr->mflag |= MFLAG_PARTIAL | MFLAG_NO_DROP;
}


void do_cmd_summoner(void)
{
	int ext = 0;

	char ch;

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

	/* not if confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* not if blind */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Select what to do */
	while (TRUE)
	{
		if (!get_com("[E]xtract a totem, [S]ummon", &ch))
		{
			ext = 0;
			break;
		}
		if ((ch == 'E') || (ch == 'e'))
		{
			ext = 1;
			break;
		}
		if ((ch == 's') || (ch == 'S'))
		{
			ext = 2;
			break;
		}
	}

	switch (ext)
	{
	case 1:
		{
			do_cmd_summoner_extract();
			break;
		}

	case 2:
		{
			do_cmd_summoner_summon();
			break;
		}
	}
}


/*
 * Dodge Chance Feedback.
 */
void use_ability_blade(void)
{
	int chance = p_ptr->dodge_chance - ((dun_level * 5) / 6);

	if (chance < 0) chance = 0;
	if (wizard)
	{
		msg_format("You have exactly %d chances of dodging a level %d monster.", chance, dun_level);
	}

	if (chance < 5)
	{
		msg_format("You have almost no chance of dodging a level %d monster.", dun_level);
	}
	else if (chance < 10)
	{
		msg_format("You have a slight chance of dodging a level %d monster.", dun_level);
	}
	else if (chance < 20)
	{
		msg_format("You have a significant chance of dodging a level %d monster.", dun_level);
	}
	else if (chance < 40)
	{
		msg_format("You have a large chance of dodging a level %d monster.", dun_level);
	}
	else if (chance < 70)
	{
		msg_format("You have a high chance of dodging a level %d monster.", dun_level);
	}
	else
	{
		msg_format("You will usually dodge successfully a level %d monster.", dun_level);
	}

	return;
}

/*
 * Helper function to describe symbiotic powers
 */
void symbiotic_info(char *p, int power)
{
	int plev = get_skill(SKILL_SYMBIOTIC);

	strcpy(p, "");

	switch (power)
	{
	case 2:
		{
			strnfmt(p, 80, " power %d", plev * 3);
			break;
		}
	case 5:
		{
			strnfmt(p, 80, " heal %d%%", 15 + get_skill_scale(SKILL_SYMBIOTIC, 35));
			break;
		}
	}
}


/*
 * Cast a symbiotic spell
 */
void do_cmd_symbiotic(void)
{
	int n = 0;
	int chance;
	int minfail = 0;
	int plev = get_skill(SKILL_SYMBIOTIC);
	magic_power spell;

	/* Get the carried monster */
	object_type *o_ptr = &p_ptr->inventory[INVEN_CARRY];

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

	/* not if confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* get power */
	if (!get_magic_power(&n, symbiotic_powers, MAX_SYMBIOTIC_POWERS, symbiotic_info,
	                     get_skill(SKILL_SYMBIOTIC), A_INT)) return;

	spell = symbiotic_powers[n];

	/* Verify "dangerous" spells */
	if (spell.mana_cost > p_ptr->csp)
	{
		/* Warning */
		msg_print("You do not have enough mana to use this power.");

		/* Verify */
		if (!get_check("Attempt it anyway? ")) return;
	}

	/* Spell failure chance */
	chance = spell.fail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (plev - spell.min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_INT]] - 1);

	/* Not enough mana to cast */
	if (spell.mana_cost > p_ptr->csp)
	{
		chance += 5 * (spell.mana_cost - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_INT]];

	/* Failure rate */
	chance = clamp_failure_chance(chance, minfail);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		flush_on_failure();
		msg_format("You failed to concentrate hard enough!");
	}
	else
	{
		/* spell code */
		switch (n)
		{
		case 0:
			{
				int dir, x, y;
				cave_type *c_ptr;
				monster_type *m_ptr;
				object_type *q_ptr;
				object_type forge;

				msg_print("Hypnotise which pet?");
				if (!get_rep_dir(&dir)) return;
				y = p_ptr->py + ddy[dir];
				x = p_ptr->px + ddx[dir];
				c_ptr = &cave[y][x];
				if (c_ptr->m_idx)
				{
					m_ptr = &m_list[c_ptr->m_idx];
					auto const r_ptr = m_ptr->race();

					if (!(r_ptr->flags & RF_NEVER_MOVE))
					{
						msg_print("You can only hypnotise monsters that cannot move.");
					}
					else if (m_ptr->status < MSTATUS_PET)
					{
						msg_print("You can only hypnotise pets and companions.");
					}
					else if (r_ptr->flags & RF_SPECIAL_GENE)
					{
						msg_print("You cannot hypnotise this monster.");
					}
					else
					{
						/* TODO fix this hack hack hack hackity hack with ToME 3 flags */
						q_ptr = &forge;
						object_prep(q_ptr, lookup_kind(TV_HYPNOS, 1));
						q_ptr->number = 1;
						q_ptr->pval = m_ptr->r_idx;
						q_ptr->pval2 = m_ptr->hp;
						q_ptr->pval3 = m_ptr->maxhp;
						/* overflow alert */
						q_ptr->exp = m_ptr->exp;
						q_ptr->elevel = m_ptr->level;
						object_aware(q_ptr);
						object_known(q_ptr);

						q_ptr->ident |= IDENT_STOREB;

						drop_near(q_ptr, 0, y, x);

						delete_monster(y, x);
						health_who = 0;
					}
				}
				else
				{
					msg_print("There is no pet here !");
				}

				break;
			}

		case 1:
			{
				monster_type *m_ptr;
				int m_idx;
				int item, x, y, d;

				/* Get an item */
				if (!get_item(&item,
					      "Awaken which monster? ",
					      "You have no monster to awaken.",
					      (USE_FLOOR),
					      object_filter::TVal(TV_HYPNOS)))
				{
					return;
				}

				object_type *o_ptr = &o_list[0 - item];

				d = 2;
				while (d < 100)
				{
					scatter(&y, &x, p_ptr->py, p_ptr->px, d);

					if (cave_floor_bold(y, x) && (!cave[y][x].m_idx)) break;

					d++;
				}

				if (d >= 100) return;

				if ((m_idx = place_monster_one(y, x, o_ptr->pval, 0, FALSE, MSTATUS_PET)) == 0) return;

				/* TODO fix this hack hack hack hackity hack with ToME 3 flags */
				/* Have to be careful here; releasing the symbiote into a
                 * dungeon with leveled monsters will level the symbiote
                 * before we can get hold of it. We'll be nice and use the
                 * larger of the saved exp and the exp that the newly-generated
                 * monster starts with. */
				m_ptr = &m_list[m_idx];
				if (m_ptr->exp < o_ptr->exp)
				{
					m_ptr->exp = o_ptr->exp;
					monster_check_experience(m_idx, TRUE);
					if (m_ptr->level != o_ptr->elevel)
						cmsg_format(TERM_VIOLET, "ERROR: level-%d HYPNOS becomes level-%d symbiote", o_ptr->elevel, m_ptr->level);
				}
				m_ptr->hp = o_ptr->pval2;
				m_ptr->maxhp = o_ptr->pval3;

				floor_item_increase(0 - item, -1);
				floor_item_describe(0 - item);
				floor_item_optimize(0 - item);
				break;
			}

			/* Charm */
		case 2:
			{
				int dir;

				if (!get_aim_dir(&dir)) return;

				fire_bolt(GF_CHARM_UNMOVING, dir, plev * 3);

				break;
			}

			/* Life Share */
		case 3:
			{
				s32b percent1, percent2;

				if (!o_ptr->k_idx)
				{
					msg_print("You are not in symbiosis.");
					break;
				}

				percent1 = p_ptr->chp;
				percent1 = (percent1 * 100) / p_ptr->mhp;

				percent2 = o_ptr->pval2;
				percent2 = (percent2 * 100) / o_ptr->pval3;

				/* Now get the average */
				percent1 = (percent1 + percent2) / 2;

				/* And set the hp of monster & player to it */
				p_ptr->chp = (percent1 * p_ptr->mhp) / 100;
				o_ptr->pval2 = (percent1 * o_ptr->pval3) / 100;

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER);

				/* Display the monster hitpoints */
				p_ptr->redraw |= (PR_FRAME);

				break;
			}

			/* Minor Symbiotic Powers */
		case 4:
			{
				if (!o_ptr->k_idx)
				{
					msg_print("You are not in symbiosis.");
					break;
				}

				if (0 > use_symbiotic_power(o_ptr->pval, false))
					return;

				break;
			}

			/* Heal Symbiote */
		case 5:
			{
				int hp;

				if (!o_ptr->k_idx)
				{
					msg_print("You are not in symbiosis.");
					break;
				}

				hp = o_ptr->pval3 * (15 + get_skill_scale(SKILL_SYMBIOTIC, 35)) / 100;
				o_ptr->pval2 += hp;
				if (o_ptr->pval2 > o_ptr->pval3) o_ptr->pval2 = o_ptr->pval3;

				msg_format("%s is healed.", symbiote_name(true).c_str());

				/* Display the monster hitpoints */
				p_ptr->redraw |= (PR_FRAME);

				break;
			}


			/* Major Symbiotic Powers */
		case 6:
			{
				if (!o_ptr->k_idx)
				{
					msg_print("You are not in symbiosis.");
					break;
				}

				if(0 > use_symbiotic_power(o_ptr->pval, true))
					return;

				break;
			}

			/* Summon never-moving pet */
		case 7:
			{
				summon_specific_friendly(p_ptr->py, p_ptr->px, dun_level, SUMMON_MINE, FALSE);

				break;
			}

			/* Force Symbiosis */
		case 8:
			{
				int y, x;
				if (!tgt_pt(&x, &y)) return;

				cave_type *c_ptr = &cave[y][x];

				if (!c_ptr->m_idx) break;

				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				use_symbiotic_power(m_ptr->r_idx, true);

				break;
			}


		default:
			{
				msg_print("Zap?");

				break;
			}
		}
	}

	/* Take a turn */
	energy_use = 100;

	/* Sufficient mana */
	if (spell.mana_cost <= p_ptr->csp)
	{
		/* Use some mana */
		p_ptr->csp -= spell.mana_cost;
	}

	/* Over-exert the player */
	else
	{
		int oops = spell.mana_cost - p_ptr->csp;

		/* No mana left */
		p_ptr->csp = 0;
		p_ptr->csp_frac = 0;

		/* Message */
		msg_print("You faint from the effort!");

		/* Hack -- Bypass free action */
		(void)set_paralyzed(randint(5 * oops + 1));

		/* Damage CON (possibly permanently) */
		if (rand_int(100) < 50)
		{
			bool_ perm = (rand_int(100) < 25);

			/* Message */
			msg_print("You have damaged your body!");

			/* Reduce constitution */
			(void)dec_stat(A_CHR, 15 + randint(10), perm);
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

/*
 * Boulder creation .. sorry :)
 */
void do_cmd_create_boulder()
{
	int x, y, dir;
	cave_type *c_ptr;

	if (!get_rep_dir(&dir)) return;
	y = p_ptr->py + ddy[dir];
	x = p_ptr->px + ddx[dir];
	c_ptr = &cave[y][x];

	/* Granite -- How about other wall types? */
	if (((c_ptr->feat >= FEAT_WALL_EXTRA) && (c_ptr->feat <= FEAT_WALL_SOLID)) ||
	                ((c_ptr->feat >= FEAT_MAGMA_H) && (c_ptr->feat <= FEAT_QUARTZ_K)) ||
	                ((c_ptr->feat == FEAT_MAGMA) ||
	                 (c_ptr->feat == FEAT_QUARTZ)))
	{
		object_type forge;
		object_type *q_ptr;

		(void)wall_to_mud(dir);

		/* Get local object */
		q_ptr = &forge;

		/* Hack -- Give the player some shots */
		object_prep(q_ptr, lookup_kind(TV_JUNK, SV_BOULDER));
		q_ptr->number = (byte)rand_range(2, 5);
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->ident |= IDENT_MENTAL;
		q_ptr->discount = 90;
		q_ptr->found = OBJ_FOUND_SELFMADE;

		(void)inven_carry(q_ptr, FALSE);

		msg_print("You make some boulders.");

		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);
		p_ptr->window |= (PW_OVERHEAD);

		/* Take a turn */
		energy_use = 100;
	}
}

/*
 * Clamp failure chance
 */
extern int clamp_failure_chance(int chance, int minfail)
{
	if (minfail < 0) minfail = 0;

	/* Minimum failure rate */
	if (chance < minfail) chance = minfail;

	/* Stunning makes spells harder */
	if (p_ptr->stun > 50) chance += 25;
	else if (p_ptr->stun) chance += 15;

	/* Always a 5 percent chance of working */
	if (chance > 95) chance = 95;

	return chance;
}
