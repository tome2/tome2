/* File: cmd7.c */

/* Purpose: More Class commands */

/*
 * Copyright (c) 1999 Dark God
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


#include "angband.h"

#include "quark.h"

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
bool_ get_magic_power(int *sn, magic_power *powers, int max_powers,
                     void (*power_info)(char *p, int power), int plev, int cast_stat)
{
	int i;

	int num = 0;

	int y = 2;

	int x = 18;

	int minfail = 0;

	int chance = 0;

	int info;

	char choice;

	char out_val[160];

	char comment[80];

	cptr p = "power";

	magic_power spell;

	bool_ flag, redraw;


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

	/* No redraw yet */
	redraw = FALSE;

	/* Count number of powers that satisfies minimum plev requirement */
	for (i = 0; i < max_powers; i++)
	{
		if (powers[i].min_lev <= plev)
		{
			num++;
		}
	}

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(%^ss %c-%c, *=List, ESC=exit, %c-%c=Info) Use which %s? ",
	        p, I2A(0), I2A(num - 1), toupper(I2A(0)), toupper(I2A(num - 1)), p);

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?'))
		{
			/* Show the list */
			if (!redraw)
			{
				char psi_desc[80];

				/* Show list */
				redraw = TRUE;

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
					if (spell.min_lev > plev) break;

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

			/* Hide the list */
			else
			{
				/* Hide list */
				redraw = FALSE;

				/* Restore the screen */
				Term_load();
				character_icky = FALSE;
			}

			/* Redo asking */
			continue;
		}

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
			continue;
		}

		/* Stop the loop */
		flag = TRUE;
	}

	/* Restore the screen */
	if (redraw)
	{
		Term_load();
	}
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
		if (flush_failure) flush();

		msg_format("You failed to concentrate hard enough!");

		sound(SOUND_FAIL);

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
		sound(SOUND_ZAP);

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
					if (plev > 4) b |= detect_traps(DEFAULT_RADIUS);
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

					if (dungeon_flags2 & DF2_NO_TELEPORT)
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
		(void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

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
	p_ptr->redraw |= (PR_MANA);

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
	p_ptr->redraw |= (PR_TITLE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
}

static bool_ mimic_forbid_travel(char *fmt)
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
		add_hook(HOOK_FORBID_TRAVEL, mimic_forbid_travel, "mimic_forbid_travel");
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
		if (flush_failure) flush();

		msg_format("You failed to concentrate hard enough!");

		sound(SOUND_FAIL);

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
		sound(SOUND_ZAP);

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
		(void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

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
	p_ptr->redraw |= (PR_MANA);

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
 * Set of variables and functions to create an artifact
 */


/* LOG2 is a constant (compile-time) method of converting a single
 * set bit into a number. Works well, but for variable (runtime)
 * expressions, use a loop instead.. much smaller code*/
#define LOG2(x)   ( (x) & 0xFFFF? BLOG16(x) : BLOG16((x)>>16) + 16 )
#define BLOG16(x) ( (x) & 0xFF  ? BLOG8(x)  : BLOG8 ((x)>>8 ) + 8  )
#define BLOG8(x)  ( (x) & 0xF   ? BLOG4(x)  : BLOG4 ((x)>>4 ) + 4  )
#define BLOG4(x)  ( (x) & 0x3   ? BLOG2(x)  : BLOG2 ((x)>>2 ) + 2  )
#define BLOG2(x)  ( (x) & 0x1   ? 0         :                   1  )

int flags_select[32*5];
int activation_select;

/* Return true if the player is wielding the philosopher's stone
 */
bool_ alchemist_has_stone(void)
{
	if (p_ptr->inventory[INVEN_LITE].name1 == 209)
		return TRUE;
	else
		return FALSE;
}

/*
 Display a group of flags from a_select flags, and return
 the number of flags displayed (even invisible ones)
 */
int show_flags(byte group, int pval)
{
	int i, x, color = TERM_WHITE;
	int items = 0;

	char ttt[80];

	Term_clear();

	group++;  /* Adjust - no zero group */

	for ( i = 0 ; a_select_flags[i].group ; i++)
	{
		if (a_select_flags[i].group != group)
			continue;

		if (a_select_flags[i].xp == 0)
			break;
		else
		{
			sprintf(ttt, "%c) %s",
			        (items < 26) ? I2A(items) : ('0' + items - 26),
			        al_name + a_select_flags[i].desc);
			if ( wizard || alchemist_has_stone())
				sprintf(ttt, "%c) %s (exp %ld)",
				        (items < 26) ? I2A(items) : ('0' + items - 26),
				        al_name + a_select_flags[i].desc,
				        (long int) a_select_flags[i].xp);

			/* Note: Somebody is VERY clever, and it wasn't me. Text printed as
			 * TERM_DARK is actually printed as TERM_BLUE *SPACES* to prevent the
			 * player from using a 'cut-and-paste' enabled terminal to see
			 * what he shouldn't.  Thus, simply setting the color to TERM_DARK
			 * will entirely prevent the unspoiled player from knowing that it's
			 * even possible. */

			switch (flags_select[i])
			{
			case 1:
				color = TERM_YELLOW;
				break;   /* Flag was set by the player (just now)*/
			case 0:
				color = TERM_WHITE;
				break;   /* This flag can be set, player is 'aware' of it*/
			case - 1:
				color = TERM_L_GREEN;
				break; /* Flag is already set*/
			case - 2:
				color = TERM_DARK;
				break;     /* Invisible option */
			case - 3:
				color = TERM_RED;
				break;     /* Flag is set, but player isn't 'aware' of it */
			case - 4:
				color = TERM_L_DARK;
				break;  /* Flag is not set, player is 'aware', but it's beyond thier skill */
			default:
				color = TERM_DARK;
				break;     /* Just in Case*/
			}
		}
		/* For alchemists who have the stone, at least show all the flags... */
		if ((alchemist_has_stone() || wizard) && color == TERM_DARK)
			color = TERM_BLUE;

		if (items < 16) x = 5;
		else x = 45;
		c_prt(color, ttt, ((items < 16) ? items : items - 16) + 5, x);
		items++;

	}
	return items;
}

void show_levels(void)
{
	Term_clear();
	c_prt(TERM_WHITE, "[a] Stats, sustains, luck, speed, vision, etc.          ", 3, 10);
	c_prt(TERM_WHITE, "[b] Misc. (Auras, light, see invis, etc)                ", 4, 10);
	c_prt(TERM_WHITE, "[c] Weapon Branding                                     ", 5, 10);
	c_prt(TERM_WHITE, "[d] Resistances and Immunities                          ", 6, 10);
	c_prt(TERM_WHITE, "[e] ESP and Curses                                      ", 7, 10);
	c_prt(TERM_WHITE, "[f] Activation                                          ", 8, 10);
	c_prt(TERM_DARK , "[g] Abilities Gained                                    ", 9, 10);
	c_prt(TERM_WHITE, "[h] Display Required Essences and items                 ", 10, 10);
	c_prt(TERM_WHITE, "[i] Done! Finalize and commit changes.                  ", 11, 10);
	/*No need to return anything - if the valid selections change, it'll be a code level change.*/
}

s32b get_flags_exp(int pval, int oldpval)
{
	int i;
	s32b exp = 0;

	for (i = 0 ; a_select_flags[i].group ; i++ )
	{
		if (a_select_flags[i].xp == 0)
			break;
		else
		{
			if ( a_select_flags[i].group <= 5 && flags_select[i] )
			{
				s32b xp = a_select_flags[i].xp;
				int factor = 1, oldfactor = 0;

				/* don't even look at flags which the user can't set
				 * because they also can't change the pval when a pval-
				 * dependant flag is set, flags which they can't set
				 * cannot effect the exp in any way, whether their set or not
				 */
				if ( flags_select[i] < -1 )
					continue;
				if ( flags_select[i] == -1 )
					oldfactor = 1;

				if (a_select_flags[i].pval)
				{
					/* (1/4)x^2 + x
					 * I wanted something smaller than x^2 or x^x
					 * this is because although a ring of speed +10 is
					 * more than 10 times better than a ring of speed +1,
					 * I don't think it's 100 times better. More like 30.
					 * this function yields:
					 * 1=1 * 2=3 * 3=5 * 4=8 * 5=11 * 6=15 * 7=21
					 * 8=24 * 9=29 * 10=35 * 11=41 * 12=48 * 13=55
					 * 14=63 * 15=71 * 20=120 * 25=181 * 30=255
					 * which I think is acceptable.
					 * briefly, to get a +30 speed ring, it would be:
					 * 255*50000 or over 12 million experience
					 * points. For reference, a level 50 human requires
					 * 5 million xp. I'm sure it's doable, but it'd be
					 * *HARD*
					 * a speed+10 artifact would require 1.75 million.
					 * much more doable, but not too easily.
					 */
					factor = (pval * pval / 4 + pval);
					if ( flags_select[i] == -1 )
					{
						oldfactor = oldpval * oldpval / 4 + oldpval;
					}
				}
				exp += xp * factor - xp * oldfactor;
			}
			if ( a_select_flags[i].group == 88 && a_select_flags[i].flag == -activation_select )
			{
				exp += a_select_flags[i].xp;
			}
		}
	}
	if ( alchemist_has_stone() ) exp = exp / 4;
	return exp;
}

/* returns the 'real quantity' of items needed to empower
 * a particular flag to a particular pval.
 * Note that this routine returns zero for any flag that
 * doesn't require some sort of action.
 */
int calc_rqty(int i, int pval, int oldpval)
{
	/* return 0 if flag is greater than size of flags_select && ! activation */
	if ( a_select_flags[i].group > 5 )
	{
		if ( activation_select == a_select_flags[i].flag)
			return 1;
		else
			return 0;
	}

	/* return 0 if the flag wasn't set */
	if ( flags_select[i] < -1 || flags_select[i] == 0 )
		return 0;

	/* Return change in pval if the flag was already set */
	if ( flags_select[i] == -1 && a_select_flags[i].pval)
		return pval - oldpval;

	/* Return pval if the flag will be set this time */
	else if ( a_select_flags[i].pval )
		return pval;

	/* Return 0 if the flag is unknown */
	else if ( flags_select[i] == -1 )
		return 0;
	return 1;
}

/* Handle the various items that creating artifacts requires.
 * Mode = 0 to print a description,
 * 1 to use up the items
 * -1 to check to see if the items exist
 * Note that this function is called ONLY from the
 * other artifact item helper function.
 */


int check_artifact_items(int pval, int oldpval, int mode)
{
	int i, j, k, row = 1 , col = 15, rqty, orqty, trqty;
	bool_ good = TRUE;
	int temporary = -1;
	char ch;

	/* For temporary items, waive the item requirements,
	 * except for the corpse... */
	for ( j = 0 ; a_select_flags[j].group ; j++)
		if (a_select_flags[j].flag == 4*32 && flags_select[j] == 1 )
			temporary = j;
	/* Check for enough items */
	for (i = 0; a_select_flags[i].group ; i++)
	{
		/* For temporary items, ignore
		 everything except the one item
		 */
		if (temporary != -1 && i != temporary)
			continue;

		/* Calc quantity is done per flag, because
		 some have a pval, some don't, some where already
		 set at pval=2, etc
		 */
		rqty = orqty = calc_rqty(i, pval, oldpval);

		/* If no item is associated with this flag,
		 or this flag wasn't set or didn't change */
		if ( !a_select_flags[i].rtval || !rqty)
			continue;

		for ( k = 0 ; k < INVEN_WIELD ; k++ )
		{
			object_type *o_ptr = &p_ptr->inventory[k];

			/* Note here that an rsval of -1 (which is read is 0xff
			 for a byte..) matches anything. */
			if (o_ptr->tval == a_select_flags[i].rtval
			                && (o_ptr->sval == a_select_flags[i].rsval
			                    || a_select_flags[i].rsval == (byte) - 1 ) )
			{
				/* Corpse validation is COMPLICATED!
				 * But at least we don't have to do this twice.
				 */
				if ( a_select_flags[i].rtval == TV_CORPSE )
				{
					bool_ itemgood = TRUE;

					/*Specified race not this one */
					if ( o_ptr->pval2 != a_select_flags[i].rpval && a_select_flags[i].rpval)
						continue;

					/* Race flag (any monster who...)*/
					for ( j = 0 ; !a_select_flags[i].rpval && a_select_flags[i].rflag[j] && j < 6 && itemgood ; j++)
					{
						int flag = a_select_flags[i].rflag[j] / 32;
						u32b mask = 1 << (a_select_flags[i].rflag[j] % 32);

						switch (flag)
						{
						case 0:
							if ( !(r_info[o_ptr->pval2].flags1 & mask) ) itemgood = FALSE;
							break;
						case 1:
							if ( !(r_info[o_ptr->pval2].flags2 & mask) ) itemgood = FALSE;
							break;
						case 2:
							if ( !(r_info[o_ptr->pval2].flags3 & mask) ) itemgood = FALSE;
							break;
						case 3:
							if ( !(r_info[o_ptr->pval2].flags4 & mask) ) itemgood = FALSE;
							break;
						case 4:
							if ( !(r_info[o_ptr->pval2].flags5 & mask) ) itemgood = FALSE;
							break;
						case 5:
							if ( !(r_info[o_ptr->pval2].flags6 & mask) ) itemgood = FALSE;
							break;
						case 6:
							if ( !(r_info[o_ptr->pval2].flags7 & mask) ) itemgood = FALSE;
							break;
						case 7:
							if ( !(r_info[o_ptr->pval2].flags8 & mask) ) itemgood = FALSE;
							break;
						case 8:
							if ( !(r_info[o_ptr->pval2].flags9 & mask) ) itemgood = FALSE;
							break;
						default:
							msg_print("This code should never be hit!");
						}
					}
					if ( ! itemgood )
						continue;

				}
				/* Validate pval of good item */
				else if ( a_select_flags[i].rpval)
				{
					/* Must have matching signs */
					if ( (o_ptr->pval < 0) != (a_select_flags[i].rpval < 0))
						continue;
					/* Must be greater than */
					if ( abs(o_ptr->pval) < abs(a_select_flags[i].rpval))
						continue;
				}

				trqty = MIN(o_ptr->number, rqty);
				rqty -= trqty;

				if ( mode == 1 )
				{
					inc_stack_size_ex(k, -trqty, NO_OPTIMIZE, DESCRIBE);
				}
			}/* if p_ptr->inventory item is acceptable */

		} /*end of looping through the p_ptr->inventory*/

		if (rqty)
		{
			good = FALSE;
			/* Oops, we didn't have enough of this object
			 when actually creating the artifact.
			 unset this flag
			 */
			if ( mode == 1 )
			{
				flags_select[i] = -4;
			}
			/* we only return false for mode -1,
			 * for mode 0 we display stuff, and for
			 * mode 1 we want to continue destroying things
			 * even if the player is missing one small item,
			 * because there's no way to change things now.
			 * We may have already destroyed a unique corpse,
			 * or some other hard-to-find item.
			 */
			if ( mode == -1 )
				return FALSE;
		}

		/* Display a description of the required object, if needed */
		/* Note that the tests for good items HAVE to be in a different
		 place, because otherwise we don't know how many the player
		 has, as opposed to how many they need.
		 */
		if ( mode == 0 )
		{
			char *o_name = al_name + a_select_flags[i].item_desc;
			if (orqty > 1 && a_select_flags[i].pval && a_select_flags[i].item_descp)
				o_name = al_name + a_select_flags[i].item_descp;

			if ( rqty )
			{
				if ( orqty > 1 )
					c_prt(TERM_RED, format(" you are missing %d of the %d %s", rqty, orqty, o_name), row++, col);
				else if ( is_a_vowel(o_name[0]))
					c_prt(TERM_RED, format(" you are missing an %s", o_name), row++, col);
				else
					c_prt(TERM_RED, format(" you are missing a %s", o_name), row++, col);
			}
			else
			{
				if ( orqty > 1 )
					c_prt(TERM_GREEN, format(" you have the %d %s", orqty, o_name), row++, col);
				else if ( is_a_vowel(o_name[0]))
					c_prt(TERM_GREEN, format(" you have an %s", o_name), row++, col);
				else
					c_prt(TERM_GREEN, format(" you have a %s", o_name), row++, col);
			}

			if ( row > 21 )
			{
				row = 1;
				if (!good)
					(void)get_com("You are missing some items:", &ch);
				else
					(void)get_com("You have these needed items on hand:", &ch);
			}

		}

	} /* End of group associated with this a_select_flags entry */

	if ( mode == 0 )
	{
		while ( row < 22 )
			c_prt(TERM_GREEN, "                            ", row++, col);
		if (!good)
			(void)get_com("You are missing some items:", &ch);
		else
			(void)get_com("You have these needed items on hand:", &ch);
	}
	return good;
}

/* Display a list of required essences,
 * and/or use up the essences. */
bool_ artifact_display_or_use(int pval, int oldpval, bool_ use)
{
	int essence[MAX_BATERIE_SVAL];
	int essenceh[MAX_BATERIE_SVAL];
	int al_idx, i, j, k;
	bool_ enough;

	/* Temporary Items require only one item, and no essences. */
	for ( i = 0 ; a_select_flags[i].group ; i++)
		if ( a_select_flags[i].flag == 32*4)
		{
			if ( use )
				return check_artifact_items(pval, oldpval, 1);
			else
				return check_artifact_items(pval, oldpval, 0);
		}

	for ( i = 0 ; i < MAX_BATERIE_SVAL ; i++ )
		essence[i] = essenceh[i] = 0;

	/* Accumulate a list of required essences */
	for ( al_idx = 0; al_idx < max_al_idx ; al_idx++ )
		if ( alchemist_recipes[al_idx].tval == 0 )
			for ( i = 0 ; a_select_flags[i].group ; i++)
			{
				int rqty = calc_rqty(i, pval, oldpval);

				/* If the flag isn't being set, rqty will be zero */
				if ( !rqty)
					continue;

				if ( alchemist_recipes[al_idx].sval == a_select_flags[i].flag )
					essence[alchemist_recipes[al_idx].sval_essence] +=
					        alchemist_recipes[al_idx].qty * rqty;
			}

	/* The essence array now contains a list of all essences
	 * that will be consumed in the creation of this artifact */

	/* Check for existence of required quatities of essences. */
	for ( i = 0 ; i < INVEN_WIELD ; i++ )
	{
		for ( j = 0 ; j < MAX_BATERIE_SVAL ; j++)
			if ( p_ptr->inventory[i].tval == TV_BATERIE && p_ptr->inventory[i].sval == j + 1)
			{
				essenceh[j] += p_ptr->inventory[i].number;
			}
	}

	/* Check for enough essences */
	enough = TRUE;
	for ( i = 0 ; i < MAX_BATERIE_SVAL ; i++)
		if ( essenceh[i] < essence[i] )
		{
			enough = FALSE;
			break;
		}

	/* Check for items */
	if ( enough )
		enough = check_artifact_items(pval, oldpval, -1);


	/* Display recipe list if they don't have enough, or not enough exp */
	if (!enough || !use )
	{
		int row = 1 , col = 15;
		bool_ good = FALSE;
		char ch;

		/* display of list of required essences */
		/* Note: there are only 12 or so essences, so this list
		 * will ALWAYS fit on the screen */
		for ( i = 0 ; i < MAX_BATERIE_SVAL ; i++)
			if ( essence[i] )
			{
				int missing = -MIN(essenceh[i] - essence[i], 0);
				good = TRUE;
				if ( missing )
					c_prt(TERM_RED, format("%d of the required %d essences of %s",
					                       missing, essence[i],
					                       k_name + k_info[lookup_kind(TV_BATERIE, i + 1)].name ),
					      row++, col);
				else
					c_prt(TERM_GREEN, format("you have the needed %d essences of %s",
					                         essence[i],
					                         k_name + k_info[lookup_kind(TV_BATERIE, i + 1)].name ),
					      row++, col);
			}

		if (good)
		{
			/* blank the bottom row */
			c_prt(TERM_WHITE, "                              ", row++, col);

			/* and wait for a key */
			(void)get_com("You are currently missing:", &ch);
		}

		/* Display a list of needed items as well */
		check_artifact_items(pval, oldpval, 0);

		return FALSE;
	}

	/* If we get to this point in the code, then the player
	 * has the required essences and items in their p_ptr->inventory */

	/* If they do have enough, and they have enough exp, consume them */
	for (i = 0 ; i < MAX_BATERIE_SVAL ; i++)
		for ( k = 0 ; k < INVEN_WIELD && essence[i] > 0 ; k++)
			if (p_ptr->inventory[k].tval == TV_BATERIE
			                && p_ptr->inventory[k].sval == i + 1
			                && essence[i])
			{
				int num = p_ptr->inventory[k].number;

				inc_stack_size_ex(k, MAX( -essence[i], -num), NO_OPTIMIZE, DESCRIBE);

				essence[i] -= MIN(num, essence[i]);
			}

	/* Destroy the items needed */
	check_artifact_items(pval, oldpval, 1);

	return TRUE;
}


void display_activation_info(int num)
{
	object_type forge;
	int i;


	/* find the a_select_flags number of this activation type... */
	for ( i = 0 ; a_select_flags[i].group ; i++)
		if (a_select_flags[i].group == 88 && a_select_flags[i].flag == -num )
			break;

	object_wipe(&forge);
	forge.xtra2 = num;
	/* Print out various information about this activation... */
	/* min level, experience, required items (and essences)
	   full description (from activation_aux) */
	if (wizard)
		c_prt(TERM_WHITE, format("  number:%d          ", num), 5, 5);
	else
		c_prt(TERM_WHITE, "                                    ", 5, 5);
	c_prt(TERM_WHITE, format("  Level:%d                              ", a_select_flags[i].level), 6, 5);
	c_prt(TERM_WHITE, format("  Exp  :%d                              ", a_select_flags[i].xp), 7, 5);
	c_prt(TERM_WHITE, format("  Item :%s                              ", al_name + a_select_flags[i].item_desc), 8, 5);
	c_prt(TERM_WHITE, "                                                                  ", 9, 5);
	c_prt(TERM_WHITE, format("  %s  ", activation_aux(&forge, 0, 0)), 9, 5);
	c_prt(TERM_WHITE, "                                    ", 10, 5);
	inkey();
}

void select_an_activation(void)
{
	int i, lev, wid, hgt, begin = 0, sel = 0;
	u32b max;
	cptr act_list[150];  /* currently, ~127 hardcoded activations */
	int act_ref[150];
	char c;
	/* How do we want to do this? */
	/* Ideally, we let them select from a list, which includes all the activations that they've ecountered in any form.
	Problems with this idea include mainly the lack of any (current) place to store which activations they've seen, and
	that they'll not get credit for any seen before we start tracking it.

	So - list is everything. If they select one which they're to low-level for
	or if the explicitly request it, we'll display info about this item.
	We'll also get our descriptions from the activation_aux(ACT_CONSTANT) 
	function, because they are more complete, and include even lua-scripted ones.
	msg_print("Since the code to actually let you select one isn't here");
	msg_print("You will automatically get the activation 'Dawn'");
	activation_select = ACT_DAWN;
	*/

	/* Build a list of available activations at the player's level */
	lev = get_skill(SKILL_ALCHEMY);
	for ( i = max = 0 ; max < (sizeof(act_list) / sizeof(cptr)) && a_select_flags[i].group ; i++)
		if (a_select_flags[i].group == 88 && a_select_flags[i].level <= lev )
		{
			act_ref[max] = -a_select_flags[i].flag;  /* Activation number */
			act_list[max++] = al_name + a_select_flags[i].desc;  /* Description */
		}

	/* Select from that list, using the util.c function display_list to display the scrolled list */
	/* Note: I think that there is only one other place that uses this function. Should be more! */
	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		c_prt(TERM_WHITE, "Enter to select, ? for more information, 2 and 8 to scroll         ", 0, 0);
		display_list(1, 0, hgt - 2, wid - 2, "Select an Activation", act_list, max, begin, sel, TERM_L_GREEN);

		c = inkey();

		if (c == ESCAPE) break;
		else if (c == '8')
		{
			sel--;
			if (sel < 0)
			{
				sel = max - 1;
				begin = max - hgt;
				if (begin < 0) begin = 0;
			}
			if (sel < begin) begin = sel;
		}
		else if (c == '2')
		{
			sel++;
			if (sel >= (s32b)max)
			{
				sel = 0;
				begin = 0;
			}
			if (sel >= begin + hgt - 1) begin++;
		}
		else if (c == '?')
		{
			display_activation_info(act_ref[sel]);
		}
		else if (c == '\r')
		{
			display_activation_info(act_ref[sel]);
			activation_select = act_ref[sel];
			return;
		}
	}
	activation_select = 0;
}


/* Consume 'num' magic essences and return true.
 * If there aren't enough essences, return false */

bool_ magic_essence(int num)
{
	int i;
	int j = 0;

	for (i = 0; i < INVEN_WIELD; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Count the magic essences */
		if (o_ptr->k_idx && (o_ptr->tval == TV_BATERIE) && (o_ptr->sval == SV_BATERIE_MAGIC)) j += o_ptr->number;
	}

	/* Abort if not enough essences. */
	if (j < num) return FALSE;

	/* Consume them */
	i = 0;
	j = num;
	while (i < INVEN_WIELD)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		if (o_ptr->k_idx && (o_ptr->tval == TV_BATERIE) && (o_ptr->sval == SV_BATERIE_MAGIC))
		{
			/* This can lead to invalid object pointer for objects
			 * that come after the magic essences. Therefore, every
			 * artifactable object should come before the essences.
			 */
			j -= o_ptr->number;
			inc_stack_size(i, -num);
			num = j;
			if (num <= 0) break;
			/* Stay on this slot; do not increment i. */
		}
		else
		{
			/* Move on to the next slot. */
			i++;
		}
	}

	/* Sanity check. */
	if (num > 0)
	{
		msg_format("ERROR: Couldn't destroy %d essences!", num);
		return FALSE;
	}

	return TRUE;
}


void do_cmd_create_artifact(object_type *q_ptr)
{
	int max, i = 0, j, cur_set = 0, abord = FALSE, done = FALSE;
	int skill;
	s32b exp = 0;

	char out_val[160];
	char choice = 0;
	bool_ lockpval = FALSE;
	int pval;
	int oldpval;
	energy_use = 100;

	pval = q_ptr->pval;
	oldpval = pval;
	skill = get_skill(SKILL_ALCHEMY);

	if ( !pval )
		pval = 1;
	/* No activation added on this round */
	activation_select = 0;

	/* Save the current flags */
	for (i = 0 ; a_select_flags[i].group ; i++)
	{
		if ( a_select_flags[i].flag < 0 || a_select_flags[i].group > 5)
			continue;

		flags_select[i] = 0;

		switch (a_select_flags[i].flag / 32)
		{
		case 0:
			if (q_ptr->art_flags1 & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		case 1:
			if (q_ptr->art_flags2 & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		case 2:
			if (q_ptr->art_flags3 & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		case 3:
			if (q_ptr->art_flags4 & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		case 4:
			if (q_ptr->art_flags5 & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		case 5:
			if (q_ptr->art_esp & 1 << (a_select_flags[i].flag % 32)) flags_select[i] = -1;
			break;
		default:
			/*This will not be hit, inspite of activations, because of the <= 5 above...*/
			break;
		}
		/*
		 this would learn about ALL flags....
		 if(wizard)
		 alchemist_known_artifacts[a_select_flags[i].flag/32] = 0xffffffffL;
		 */

		/* Set various flags if they haven't *ID*'d an artifact with this flag set.*/
		if ( !(alchemist_known_artifacts[a_select_flags[i].flag / 32] & (1 << (a_select_flags[i].flag % 32)) ))
		{
			/* If this item has an ability that depends on pval which the player
			 * cannot set, don't allow them to change the pval either. */
			if ( a_select_flags[i].pval && flags_select[i])
				lockpval = TRUE;

			/* Set the color and set-ablitity of this flag */
			if ( flags_select[i] )
				flags_select[i] = -3;
			else
				flags_select[i] = -2;
			continue;
		}
		else if ( skill < a_select_flags[i].level )
		{
			/* If the alchemist has not passed the skill level for this flag,
			 Set this flag as unsettable.
			 */
			if ( flags_select[i])
				lockpval = TRUE;
			else
				flags_select[i] = -4;
		}
	}

	/* Save the screen */
	character_icky = TRUE;
	Term_save();
	Term_clear();


	/* Everlasting love ... ... nevermind :) */
	while ( !done && !abord)
	{
		c_prt((q_ptr->exp - exp > 0) ? TERM_L_GREEN : TERM_L_RED, format("Experience left: %ld", q_ptr->exp - exp), 2, 0);

		/* Display the menu, but don't display it if we just
		 * displayed a message (it erases the screen, creating a blink message */
		if ( cur_set < 6 || cur_set == 7 )
			show_levels();

		c_prt((q_ptr->exp - exp > 0) ? TERM_L_GREEN : TERM_L_RED, format("Experience left: %ld", q_ptr->exp - exp), 2, 0);

		prt("Enter to accept, Escape to abort", 1, 0);

		abord = !get_com("Play around with which group of powers?[a-g]", &choice);

		if ( choice == ESCAPE)
			abord = TRUE;

		if ( abord )
			continue;  /*or break, same diff */

		if ( isalpha(choice))
		{
			if (isupper(choice))
				choice = tolower(choice);
			cur_set = A2I(choice);
		}
		else
		{
			bell();
			continue;
		}

		if ( cur_set == 5 )
		{
			if (q_ptr->xtra2 && !activation_select
			                && !get_check("This item already activates! Choose a different activation?")) continue;
			select_an_activation();
			exp = get_flags_exp(pval, oldpval);
			continue;
		}
		if ( cur_set == 6 )
		{
			msg_print("This option is not available");
			continue;
		}
		if ( cur_set == 7 )
		{
			artifact_display_or_use(pval, oldpval, FALSE);
			continue;
		}
		if ( cur_set == 8 )
		{
			if (q_ptr->exp - exp < 0)
				msg_print("Not enough experience for the flags you've selected.");
			else
				done = TRUE;
			continue;
		}

		if (cur_set < 0 || cur_set > 4 )
		{
			bell();
			continue;
		}


		while (!done && !abord)
		{
			/* Chose the flags */
			exp = 0;
			max = show_flags(cur_set, pval);
			exp = get_flags_exp(pval, oldpval);
			c_prt((q_ptr->exp - exp > 0) ? TERM_L_GREEN : TERM_L_RED, format("Experience left: %ld", q_ptr->exp - exp), 2, 0);

			/* Build a prompt (accept all flags) */
			if (max <= 26)
			{
				/* Build a prompt (accept all flags) */
				strnfmt(out_val, 78, "(Flags %c-%c, I,D to change power level) Add/Remove which flag? ",
				        I2A(0), I2A(max - 1));
			}
			else
			{
				strnfmt(out_val, 78, "(Flags %c-%c, I,D to change power level) Add/Remove which flag? ",
				        I2A(0), '0' + max - 27);
			}
			c_prt(TERM_L_BLUE, format("Power(I/D to increase/decrease): %d", pval), 3, 0);

			/* Get a spell from the user */
			while (!(done = !get_com(out_val, &choice)))
			{
				if (choice == 'I')
				{
					if ( lockpval )
					{
						msg_print("You cannot do that - you don't know how!");
						continue;
					}
					if (q_ptr->exp - exp < 0)
					{
						msg_print("Not enough experience.  Decrease power or deselect flags.");
						continue;
					}
					pval++;
					break;
				}
				else if (choice == 'D')
				{
					if ( lockpval )
					{
						msg_print("You cannot do that - you don't know how!");
						continue;
					}
					pval--;
					if (pval < oldpval) pval = oldpval;
					break;
				}
				else if (choice == '\r' || choice == ESCAPE || choice == ' ')
				{
					done = TRUE;
					break;
				}
				else if (isalpha(choice))
				{
					/* Lowercase */
					if (isupper(choice)) choice = tolower(choice);

					/* Extract request */
					i = (islower(choice) ? A2I(choice) : -1);
				}
				else
				{
					i = D2I(choice) + 26;

					/* Illegal */
					if (i < 26) i = -1;
				}

				/* Totally Illegal */
				if ((i < 0) || (i >= max))
				{
					bell();
					continue;
				}
				else
				{
					/*Find the i'th flag in group cur_set...*/
					for ( j = 0 ; a_select_flags[j].group ; j++)
						if (a_select_flags[j].group == cur_set + 1)
							if (!i--) break;

					if ( flags_select[j] == -4 )
					{
						msg_format("You need at least %d skill in alchemy.",
						           a_select_flags[j].level);
						continue;
					}
					if ( flags_select[j] != 0 && flags_select[j] != 1)
					{
						bell();
						continue;
					}
					if (flags_select[j]) flags_select[j] = 0;
					else if (!flags_select[j])
					{
						if (q_ptr->exp - exp < 0)
						{
							msg_print("Not enough experience.  Decrease power or deselect flags.");
							continue;
						}
						flags_select[j] = 1;
					}
					break;
				}
			}
		}/*sub-screen select and redraw loop*/
		done = FALSE;
		Term_clear();
	}/* main screen (flag select screen) select and redraw loop*/

	/* Abort if not enough experience, or no flags added */
	if ( q_ptr->exp - exp < 0 || exp == 0 )
		abord = TRUE;

	/* Display the recipe, or use up the essences.
	 * Note that this has to be done before the screen
	 * is restored. This is because it's also called from
	 * within the loop to display the required items. */
	if ( !abord )
		if (!artifact_display_or_use(pval, oldpval, TRUE))
			abord = TRUE;

	/* Restore the screen */
	Term_load();
	character_icky = FALSE;

	/* Return if abort, or missing ingredients */
	if ( abord )
		return;

	/* Actually create the artifact */
	q_ptr->exp -= exp;
	q_ptr->art_flags4 &= ~TR4_ART_EXP;
	q_ptr->pval = pval;

	/* Just to be sure */
	q_ptr->art_flags3 |= ( TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
	                       TR3_IGNORE_FIRE | TR3_IGNORE_COLD );

	{
		int now = 0, before = 0;
		char dummy_name[80];
		char new_name[80];

		/* Apply the flags */
		for (i = 0; a_select_flags[i].group ; i++)
		{
			if (flags_select[i] < 0)
				before++;
			else if ( flags_select[i] == 1)
			{
				now++;
				switch (a_select_flags[i].flag / 32)
				{
				case 0:
					q_ptr->art_flags1 |= 1 << (a_select_flags[i].flag % 32);
					break;
				case 1:
					q_ptr->art_flags2 |= 1 << (a_select_flags[i].flag % 32);
					break;
				case 2:
					q_ptr->art_flags3 |= 1 << (a_select_flags[i].flag % 32);
					break;
				case 3:
					q_ptr->art_flags4 |= 1 << (a_select_flags[i].flag % 32);
					break;
				case 4:
					q_ptr->art_flags5 |= 1 << (a_select_flags[i].flag % 32);
					break;
				case 5:
					q_ptr->art_esp |= 1 << (a_select_flags[i].flag % 32);
					break;
				default:
					msg_print("error: this code can't ever be hit!");
				}
			}
		}

		if ( activation_select )
		{
			q_ptr->art_flags3 |= TR3_ACTIVATE;
			q_ptr->xtra2 = activation_select;
		}


		/* Set the 'show modifier' flag */
		q_ptr->art_flags3 |= TR3_SHOW_MODS;

		/* For temporary items, set a timeout.
		 * alchemist_skill^2 for now */
		if ( q_ptr->art_flags5 & TR5_TEMPORARY )
		{
			int lev = get_skill(SKILL_ALCHEMY);
			q_ptr->timeout = lev * lev * 3;
		}

		/* Describe the new artifact */
		object_out_desc(q_ptr, NULL, FALSE, TRUE);


		/* Name the new artifact */
		strcpy(dummy_name, "of an Alchemist");
		if (!(get_string("What do you want to call the artifact? ", dummy_name, 80)))
			strcpy(new_name, "of an Alchemist");
		else
		{
			if ((strncmp(dummy_name, "of ", 3) == 0) ||
			                (strncmp(dummy_name, "Of ", 3) == 0) ||
			                ((dummy_name[0] == '\'') &&
			                 (dummy_name[strlen(dummy_name) - 1] == '\'')))
			{
				strcpy(new_name, dummy_name);
			}
			else
			{
				strcpy(new_name, "called '");
				strcat(new_name, dummy_name);
				strcat(new_name, "'");
			}
		}
		/* Identify it fully */
		object_aware(q_ptr);
		object_known(q_ptr);

		/* Mark the item as fully known */
		q_ptr->ident |= (IDENT_MENTAL);
		q_ptr->ident |= IDENT_STOREB;  /* This will be used later on... */

		/* Save the inscription */
		q_ptr->art_name = quark_add(new_name);
		q_ptr->found = OBJ_FOUND_SELFMADE;

		done = FALSE;
		while (!done && get_com("Do you want to let this item continue to gain experience?", &choice))
		{
			switch (choice)
			{
			case 'y':
			case 'Y':
				if (magic_essence(get_skill(SKILL_ALCHEMY)))
					q_ptr->art_flags4 |= TR4_ART_EXP;
				else
					msg_format("Oh, NO! You don't have enough magic essences. You needed %d.", get_skill(SKILL_ALCHEMY));
				done = TRUE;
				break;
			case 'n':
			case 'N':
				q_ptr->exp = 0;
				done = TRUE;
				break;
			}
		}

		/* Cycle through the p_ptr->inventory, and optimize everything.
		 * This wasn't done earlier, because if we had, then
		 * things in the p_ptr->inventory would shift around, and q_ptr
		 * wouldn't point to the right thing. BUT, at this point
		 * we don't need q_ptr anymore, so optimizing the p_ptr->inventory
		 * becomes sane. Sticky bug to figure out, let me tell you.
		 * Note also that this is cycling backwards - this is so
		 * that the same effect doesn't cause us to skip items. */
		for ( i = INVEN_WIELD - 1 ; i >= 0 ; i-- )
			inven_item_optimize(i);
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}

/*
 * Test to see if this tval/sval combo is in the alchemists'
 * recipes as a createable item. Used to determine if we
 * should extract from it.
 */
bool_ alchemist_exists(int tval, int sval, int ego, int artifact)
{
	int al_idx;

	/* To prevent conflicts with recipes for ego-items.
	 * artifact not used, simplifies the loop below. */
	if ((tval == 1) || artifact)
		return FALSE;

	/*Search for recipes with this tval/sval combo as the final result*/
	for (al_idx = 0 ; al_idx < max_al_idx ; al_idx++)
	{
		int rtval = alchemist_recipes[al_idx].tval;
		int rsval = alchemist_recipes[al_idx].sval;

		/* Accept ego wands and staves since ego is extracted last */
		if (((!ego || tval == TV_WAND || tval == TV_STAFF) && rtval == tval && rsval == sval) ||
		                ( ego && rtval == 1 && rsval == ego))
		{
			return TRUE;
		}
	}
	return FALSE;
}


/*
 * Hook to determine if an object can have things extracted from it.
 */
bool_ item_tester_hook_extractable(object_type *o_ptr)
{

	/* No artifacts */
	if (artifact_p(o_ptr)) return (FALSE);

	/* No cursed things */
	if (cursed_p(o_ptr)) return (FALSE);

	/* If we REALLY wanted to rebalance alchemists,
	 * we'd test for 'fully identified this object kind' here.
	 */

	return ((o_ptr->tval == TV_ROD_MAIN && o_ptr->pval != 0)
	        || alchemist_exists(o_ptr->tval, o_ptr->sval, o_ptr->name2, o_ptr->name1));
}

/*
 * Hook to determine if an object is empowerable (NOT rechargeable)
 */
bool_ item_tester_hook_empower(object_type *o_ptr)
{
	int sval = -1;
	int lev = get_skill(SKILL_ALCHEMY);
	/* after level 25, can empower ego items to create artifacts
	 * and double ego items.
	 * after level 50, can empower artifacts to create powerful artifacts
	 */

	/* Never Empower a cursed item */
	if ( cursed_p(o_ptr))
	{
		return FALSE;
	}

	/* Allow finalizing a self created artifact */
	if (artifact_p(o_ptr)
	                && (o_ptr->art_flags4 & TR4_ART_EXP)
	                && !(o_ptr->art_flags4 & TR4_ULTIMATE))
		return TRUE;

	switch ( o_ptr->tval)
	{
		/* Empowerable objects: Traditional alchemist stuff */
	case TV_WAND:
		sval = SV_WAND_NOTHING;
		break;
	case TV_RING:
		sval = SV_RING_NOTHING;
		break;
	case TV_STAFF:
		sval = SV_STAFF_NOTHING;
		break;
	case TV_BOTTLE:
		sval = 1;
		break;
	case TV_AMULET:
		sval = SV_AMULET_NOTHING;
		break;
	case TV_SCROLL:
		sval = SV_SCROLL_NOTHING;
		break;
	case TV_ROD:
		sval = SV_ROD_NOTHING;
		break;
	case TV_ROD_MAIN:
		sval = -1;
		break;
	case TV_BOOK:
		sval = -1;
		break;

		/* Ego item stuff */
		/* Disallow ego dragon armour before you can create artifacts.*/
	case TV_DRAG_ARMOR:
		if ( lev < 25)
			return FALSE;
		/* FALL THROUGH! no break here. */

		/* weapons */

	case TV_DAEMON_BOOK:
	case TV_SWORD:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_AXE:
	case TV_MSTAFF:

		/* misc other items */
	case TV_BOW:
	case TV_BOOMERANG:
	case TV_INSTRUMENT:
	case TV_DIGGING:
	case TV_LITE:

		/* Ammo */
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:

		/* Armor of various sorts */
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:

		/* Disallow ANY creation of ego items below level 5*/
		if ( lev < 5)
			return FALSE;

		/* empowering an ego item creates an artifact or a
		 * double ego item, disallow below level 25 */
		if ( lev < 25 && o_ptr->name2)
			return FALSE;

		/* Disallow double-ego and artifact unless the character has
		 * the artifact creation ability. */
		if (!has_ability(AB_CREATE_ART) && 
		   (artifact_p(o_ptr) || (o_ptr->name2 && o_ptr->name2b)))
			return FALSE;

		/* Otherwise... */
		return TRUE;

	default:
		return FALSE;
	}

	/* Return to the traditional alchemist objects.
	 * All ego items and artifacts returning TRUE are accepted as artifactable
	 * at level 25. If we want double ego non wieldable items (Fireproof Staff
	 * of Plenty) the artifactable test in do_cmd_alchemist() must be changed,
	 * e.g. checking if the item is wearable.
	 * For now, we disallow non-wearable ego-items and artifacts here.
	 */

	if ((o_ptr->name2 || artifact_p(o_ptr)) &&
	                o_ptr->tval != TV_RING && o_ptr->tval != TV_AMULET)
		return FALSE;

	/* return true if it's a 'of nothing' item;
	 * does nothing for TV_ROD_MAIN and TV_BOOK
	 */ 
	return (sval == o_ptr->sval

	        /* or if it's artifactable */
	        || ((lev >= 50 || (lev >= 25 && !artifact_p(o_ptr))) &&
	            (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET))

	        /* or if it's egoable (note that normal egos start at level 5, wands and such start at 15) */
	        || (!o_ptr->name2 && lev >= 15));
}

/* Extract a rod tip from a rod */
void rod_tip_extract(object_type *o_ptr)
{
	object_type *q_ptr;
	object_type forge;

	/* Get local object */
	q_ptr = &forge;

	/* Paranoia, return if it's a rod of nothing */
	if (o_ptr->pval == SV_ROD_NOTHING)
		return;

	/* Extract the rod tip */
	object_prep(q_ptr, lookup_kind(TV_ROD, o_ptr->pval));

	q_ptr->number = o_ptr->number;

	object_aware(q_ptr);
	object_known(q_ptr);
	(void)inven_carry(q_ptr, FALSE);

	/* Remove it from the rod */
	o_ptr->pval = SV_ROD_NOTHING;

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);
}


/* Begin & finish an art */
void do_cmd_toggle_artifact(object_type *o_ptr)
{
	char o_name[80];

	if (!(o_ptr->art_flags4 & TR4_ART_EXP))
	{
		bool_ okay = TRUE;

		if ( !alchemist_has_stone())
		{
			msg_print("Creating an artifact will result into a permanent loss of 10 hp.");
			if (!get_check("Are you sure you want to do that?")) return;
		}

		if (!magic_essence(get_skill(SKILL_ALCHEMY)))
		{
			msg_format("You need %d magic essences.", get_skill(SKILL_ALCHEMY));
			return;
		}

		/* Description */
		object_desc(o_name, o_ptr, FALSE, 0);

		if (o_ptr->number > 1)
		{
			msg_print("Not enough energy to enchant more than one object!");
			msg_format("%d of your %s %s destroyed!", (o_ptr->number) - 1, o_name, (o_ptr->number > 2 ? "were" : "was"));
			o_ptr->number = 1;
		}
		okay = TRUE;

		if (!okay) return;

		/* he/she got warned */
		p_ptr->hp_mod -= 10;

		/* Ok toggle it */
		o_ptr->art_flags4 |= TR4_ART_EXP;
		o_ptr->name2 = 0;
		o_ptr->name2b = 0;
		o_ptr->art_name = quark_add("Becoming");

		/* Copy the object_kind flags to the artifact flags.
		 * Note that this is only needed so that flags set in the
		 * 'kind' area are visible when finalizing the artifact.
		 */
		{
			u32b f1, f2, f3, f4, f5, esp;

			object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

			o_ptr->art_flags1 |= f1;
			o_ptr->art_flags2 |= f2;
			o_ptr->art_flags3 |= f3;
			o_ptr->art_flags4 |= f4;
			o_ptr->art_flags5 |= f5;
			o_ptr->art_esp |= esp;
		}

		p_ptr->update |= (PU_HP);
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	}
	else
	{
		do_cmd_create_artifact(o_ptr);
	}
}

/*
 * Test to see if they have all the ingredients to create an item.
 * (doesn't count base item)
 * creates 'tocreate' items (may be -1, but no more than that!)
 * if tocreate=0, will return true if the player has enough
 * in their p_ptr->inventory to empower that item.
 */
bool_ alchemist_items_check(int tval, int sval, int ego, int tocreate, bool_ message)
{
	int al_idx, j;
	bool_ exists = FALSE;


	for ( al_idx = 0 ; al_idx < max_al_idx ; al_idx++ )
		if ((ego && alchemist_recipes[al_idx].sval == ego
		                && alchemist_recipes[al_idx].tval == 1 )
		                || (!ego && alchemist_recipes[al_idx].sval == sval
		                    && alchemist_recipes[al_idx].tval == tval))
		{
			exists = TRUE;
			/* Create the essences */
			if (tocreate > 0)
			{
				object_type forge;
				object_type *o_ptr = &forge;

				object_wipe(o_ptr);
				object_prep(o_ptr, lookup_kind(TV_BATERIE, alchemist_recipes[al_idx].sval_essence));
				o_ptr->number = alchemist_recipes[al_idx].qty * tocreate;
				/* Don't bother with apply_magic */

				/* Randomly decrease the number of essences created */
				if ( randint(3) == 1
				                && randint(52) > get_skill(SKILL_ALCHEMY)
				                && !alchemist_has_stone())
					o_ptr->number /= randint(2) + 1;
				if ( o_ptr->number == 0)
					continue;
				object_aware(o_ptr);
				object_known(o_ptr);
				if (inven_carry_okay(o_ptr))
				{
					int i;
					inven_carry(o_ptr, FALSE);
					for (i = 0; i < INVEN_WIELD ; i++)
						if (p_ptr->inventory[i].tval == o_ptr->tval && p_ptr->inventory[i].sval == o_ptr->sval)
						{
							if ( message )
								inven_item_describe(i);
							break;
						}

				}
				else
					drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);

				o_ptr->ident |= IDENT_STOREB;
			}
			else if ( tocreate < -1)
			{
				/*It's not valid to create more than one
				 * thing at a time, so if it's less than -1,
				 * it must be time to display a recipe
				 */
				msg_format("%d essences of %d",
				           alchemist_recipes[al_idx].qty,
				           al_idx);
			}
			else /* Destroy the essences (tocreate == -1)
				                              * or check for existence(tocreate == 0)*/
			{
				int rqty = alchemist_recipes[al_idx].qty;
				for (j = 0; j < INVEN_WIELD; j++)
				{
					object_type *o_ptr = &p_ptr->inventory[j];
					if (o_ptr->k_idx
					                && (o_ptr->tval == TV_BATERIE )
					                && (o_ptr->sval == alchemist_recipes[al_idx].sval_essence )
					                && (o_ptr->number >= rqty ))
					{
						/* At this point, the item is required, destroy it. */
						if ( tocreate )
						{
							inc_stack_size_ex(j, 0 - rqty, OPTIMIZE, message ? DESCRIBE : NO_DESCRIBE);
						}

						/* When we find enough of the item, break out of the
						 * 'search through the p_ptr->inventory' loop */
						break;
					}
				}
				if ( j == INVEN_WIELD)
					/* This ingredient was not found, cannot do recipe */
					return FALSE;
			}/*destroying items, or just checking for existence */
		}
	return exists;
}

/* This function lists all the ingredients
 * needed to create something.
 */
void alchemist_display_recipe(int tval, int sval, int ego)
{
	int al_idx;
	int row = 1, col = 15;
	char o_name[80];
	char ch;
	object_type *o_ptr, forge;

	/* Display the ingredients for a recipe */
	for ( al_idx = 0 ; al_idx < max_al_idx ; al_idx++ )
		if ((ego && alchemist_recipes[al_idx].sval == ego
		                && alchemist_recipes[al_idx].tval == 1 )
		                || (!ego && alchemist_recipes[al_idx].sval == sval
		                    && alchemist_recipes[al_idx].tval == tval))
		{
			int qty = alchemist_recipes[al_idx].qty;
			c_prt(TERM_GREEN,
			      format("     %d essence%s %s         ", qty,
			             qty > 1 ? "s" : "",
			             k_name + k_info[lookup_kind(TV_BATERIE, alchemist_recipes[al_idx].sval_essence)].name ),
			      row++, col);
		}

	c_prt(TERM_WHITE, "                                                 ", row++, col);

	if (!ego)
	{
		/* Find the name of that object */
		o_ptr = &forge;
		object_prep(o_ptr, lookup_kind(tval, sval));
		o_ptr->name2 = ego;
		hack_apply_magic_power = -99;
		apply_magic(o_ptr, get_skill(SKILL_ALCHEMY) * 2, FALSE, FALSE, FALSE);
		object_aware(o_ptr);
		object_known(o_ptr);
		/* the 0 mode means only the text, leaving off any numbers */
		object_desc(o_name, o_ptr, FALSE, 0);
	}
	else
	{
		/* Display the ego item name */
		strcpy(o_name, e_name + e_info[ego].name);
	}

	/* Display a short message about it, and wait for a key. */
	(void)get_com(format("ingredients needed to create a %s", o_name), &ch);

}

/*
 *
 * The alchemist_recipe_select was copied from
 * wiz_create_itemtype
 * and then changed quite a bit.
 *
 */

/*
 The select array is a simple array of 'use this char to select item x'
 It has 88 items (three columns of 20 each)
 selectitem is initilized with the reverse mappings:
 selectitem[selectchar[x]] == x is always true.
 */
char selectchar[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*():;,.<=>[]{}/=?+'~";
byte selectitem[256];

void strip_and_print(char *str, int color, int num)
{
	int row = 2 + (num % 20), col = 40 * (num / 20);
	int ch, max_len = 0;
	char buf[80];
	char *string;

	if (num > 60)
	{
		msg_print("Attempting to display too many items!");
		return;
	}
	ch = selectchar[num];
	if (selectitem[ch] != num)
	{
		int i;
		for ( i = 0 ; i < 256 ; i++)
			selectitem[i] = 0xff;
		for ( i = 0 ; selectchar[i] ; i++)
			selectitem[(byte)selectchar[i]] = i;
	}

	/* Skip past leading characters */
	while ((*str == ' ') || (*str == '&')) str++;

	/* Copy useful chars */
	for (string = buf; *str; str++)
		if (*str != '~') *string++ = *str;

	/* Terminate the new name */
	*string = '\0';

	/* strip the name down to size
	 if (76-col < (signed)max_len)
	 max_len = 76-col;
	 else
	 max_len = 30-6;*/
	max_len = 39;

	string = buf;
	if (strlen(string) > (unsigned)max_len)
		string = string + (strlen(string) - max_len);

	/* Print it */
	c_prt(color, format("[%c] %s", ch, string), row, col);
}

/* Display a list of recipes that need a particular essence.
 * Note that we display a list of essences first,
 * so in effect, this is the alchemist's recipe book.
 */
void alchemist_recipe_book(void)
{
	int num, max_num, i, al_idx, bat, kidx;
	int choice[61], choice2[61];
	int mod40;
	bool_ essence[MAX_BATERIE_SVAL + 1];
	char ch;

	/* Save and clear the screen */
	character_icky = TRUE;
	Term_save();

	while ( TRUE )
	{
		Term_clear();

		num = 0;

		/* Display bateries */

		/* start with assumption that the alchemist knows about no recipes */
		for (i = 0; i < MAX_BATERIE_SVAL + 1 ; i++)
			essence[i] = FALSE;

		/* cycle through all alchemist recipes */
		for (al_idx = 0 ; al_idx < max_al_idx ; al_idx++)
			/* if we aren't already going to display this essence */
			if (!essence[alchemist_recipes[al_idx].sval_essence])
			{

				/*Note that we don't display artifact recipes here...*/
				/*This is partially because artifacts often require exotic
				 ingredients as well */

				if (!alchemist_recipes[al_idx].tval)
					continue;

				if (alchemist_recipes[al_idx].tval == 1)
				{
					if (alchemist_known_egos[alchemist_recipes[al_idx].sval / 32]
					                & (1 << (alchemist_recipes[al_idx].sval % 32)) )
						essence[alchemist_recipes[al_idx].sval_essence] = TRUE;
					continue;
				}

				kidx = lookup_kind(alchemist_recipes[al_idx].tval, alchemist_recipes[al_idx].sval);
				if (alchemist_recipes[al_idx].tval != 1 && k_info[kidx].know)
					essence[alchemist_recipes[al_idx].sval_essence] = TRUE;

			}
		for (num = 0, i = 0; i < MAX_BATERIE_SVAL + 7 ; i++)
			if (essence[i] || i > MAX_BATERIE_SVAL)
			{
				int kidx = lookup_kind(TV_BATERIE, i);
				if (i > MAX_BATERIE_SVAL)
				{
					switch (i)
					{
					case (MAX_BATERIE_SVAL + 1): strip_and_print("Scrolls", TERM_WHITE, num);
						break;
					case (MAX_BATERIE_SVAL + 2): strip_and_print("Potions", TERM_WHITE, num);
						break;
					case (MAX_BATERIE_SVAL + 3): strip_and_print("Wands", TERM_WHITE, num);
						break;
					case (MAX_BATERIE_SVAL + 4): strip_and_print("Rings", TERM_WHITE, num);
						break;
					case (MAX_BATERIE_SVAL + 5): strip_and_print("Staves", TERM_WHITE, num);
						break;
					case (MAX_BATERIE_SVAL + 6): strip_and_print("Amulets", TERM_WHITE, num);
						break;
					default:
						continue;
					}
				}
				else
					/* add this essence to the list*/
					strip_and_print(k_name + k_info[kidx].name, TERM_WHITE, num);

				choice[num++] = i;
			}
		max_num = num;
		if ( max_num == 0)
		{
			/*Note that this should never actually happen, as any skill
			 at alchemy automatically gets you some recipes, and this
			 procedure shouldn't be called for players without alchemist skill
			 */
			msg_print("You don't know any recipes!");
			msg_print("You can't be an alchemist without recipes!");
			break;
		}

		while (num == 0xff || num >= max_num)
		{
			ch = selectchar[max_num - 1];
			/* Choose! */
			if ( max_num == 0 ||
			                !get_com(format("Which Type of Recipe?[a-%c]", selectchar[max_num - 1]), &ch))
				break;

			/* Analyze choice - note that the cast to byte prevents overflow*/
			num = selectitem[(byte)ch];

		}
		/* This break, and the break for no recipes above,
		 are the only exits from this procedure.
		 */
		if ( num == 0xff || num >= max_num)
			break;

		/* Save the baterie index */
		bat = choice[num];
		num = 0;

		/*Display the 'type of object' recipe screen*/
		if (bat > MAX_BATERIE_SVAL)
		{
			int tval;
			switch (bat)
			{
			case MAX_BATERIE_SVAL + 1:
				tval = TV_SCROLL;
				break;
			case MAX_BATERIE_SVAL + 2:
				tval = TV_POTION;
				break;
			case MAX_BATERIE_SVAL + 3:
				tval = TV_WAND;
				break;
			case MAX_BATERIE_SVAL + 4:
				tval = TV_RING;
				break;
			case MAX_BATERIE_SVAL + 5:
				tval = TV_STAFF;
				break;
			case MAX_BATERIE_SVAL + 6:
				tval = TV_AMULET;
				break;
			}
			Term_load();
			alchemist_recipe_select(&tval, 0, FALSE, TRUE);
			Term_save();
			continue;
		}
		mod40 = 0;
		while ( TRUE )
		{
			int skipped;

			Term_clear();
			num = 0;

			if (mod40)
			{
				strip_and_print("--MORE--", TERM_WHITE, num);
				choice[num] = -2;
				choice2[num++] = 0;
			}

			/* Display all items made with this essence */
			for ( al_idx = 0 , skipped = 0 ; al_idx < max_al_idx ; al_idx++)
				if ( alchemist_recipes[al_idx].sval_essence == bat)
				{
					int sval = alchemist_recipes[al_idx].sval;
					int tval = alchemist_recipes[al_idx].tval;
					char names[200] = "";

					if (alchemist_recipes[al_idx].tval == 1)
					{
						/* Ego items */
						ego_item_type *e_ptr = &e_info[sval];
						int j, k;

						if ( !(alchemist_known_egos[sval / 32] & (1 << (sval % 32))))
							continue;

						for ( j = 0 ; j < 6 && e_ptr->tval[j] ; j ++ )
						{
							if ( j > 0 && e_ptr->tval[j] == e_ptr->tval[j - 1])
								continue;
							for ( k = 0; tvals[k].tval; k++)
								if (tvals[k].tval == e_ptr->tval[j])
								{
									strcat(names, tvals[k].desc);
									strcat(names, ", ");
									break;
								}
						}
						strcat(names, e_name + e_ptr->name);
					}
					else
					{
						/* Normal Items */
						int kidx = lookup_kind(tval, sval);
						int k;
						if ( !k_info[kidx].know )
							continue;

						for ( k = 0; tvals[k].tval; k++)
							if (tvals[k].tval == tval)
							{
								strcat(names, tvals[k].desc);
								break;
							}
						strcat(names, " of ");
						strcat(names, k_name + k_info[kidx].name);

					}

					/*Skip the first mod40 pages of recipes*/
					if (skipped++ < mod40*38)
						continue;

					/* add this object kind to the list*/
					strip_and_print(names, TERM_WHITE, num);
					choice[num] = tval;
					choice2[num++] = sval;
					if (num > 38)
					{
						strip_and_print("--MORE--", TERM_WHITE, num);
						choice[num] = -1;
						choice2[num++] = 0;
						break;
					}

				}/*Loop through tidx/sidx*/

			max_num = num;
			while (num == 0xff || num >= max_num)
			{
				ch = selectchar[max_num - 1];
				/* Choose! */
				if ( max_num == 0 || !get_com(
				                        format("Examine which recipe?[%c-%c]", selectchar[0], ch)
				                        , &ch))
				{
					break;
				}

				/* Analyze choice */
				num = selectitem[(byte)ch];
			}

			if ( choice[num] < 0)
			{
				if (choice[num] < -1)
					mod40--;
				else
					mod40++;
				continue;
			}

			if ( num == 0xff || num >= max_num)
				break;

			/* Display the recipe */
			if (choice[num] == 1)
				alchemist_display_recipe(0, 0, choice2[num]);
			else
				alchemist_display_recipe(choice[num], choice2[num], 0);
		}
		/*
		 break is at top of loop, after essence list
		 if( num < 0 || num >= max_num)
		 break;
		 */

	}/*show recipes*/

	/* Restore screen contents */
	Term_load();
	character_icky = FALSE;
}

/* Display a list of known recipies that can be made with
 * materials on hand (including the passed tval). Also
 * calls the recipe_display function, if requested by the
 * player or there aren't enough essences to make the
 * requested object.
 *
 * Note: sval is ignored if !ego, tval is the only determinant
 * of what recipies are available otherwise.
 *
 * This function needs to be able to scroll a list, because
 * there are SO MANY potions. :)
 */
int alchemist_recipe_select(int *tval, int sval, int ego, bool_ recipe)
{
	int i, mod40 = 0, num, max_num = 0;

	cptr tval_desc2 = "";
	char ch;
	bool_ done = FALSE;

	int choice[60];
	int validc[60];

	char *string;


	/* Save and clear the screen */
	character_icky = TRUE;
	Term_save();
	Term_clear();

	/* Base object type chosen, fill in tval */
	for ( num = 0 ; num < 40 ; num ++)
		if (tvals[num].tval == *tval)
		{
			tval_desc2 = tvals[num].desc;
		}

	while (!done)
	{
		Term_clear();
		if (ego)
		{
			/* Find matching ego items */
			for (num = 0, i = 1; (num < 40) && (i < max_e_idx) ; i++)
			{
				int j;
				ego_item_type *e_ptr = &e_info[i];

				/* Skip if unknown ego type */
				if ( !(alchemist_known_egos[i / 32] & (1 << (i % 32))))
					continue;

				/* search in permitted tvals/svals for allowed egos */
				for ( j = 0 ; j < 6 ; j ++ )
					if ( e_ptr->tval[j] == *tval
					                && sval >= e_ptr->min_sval[j]
					                && sval <= e_ptr->max_sval[j])
					{
						int color = TERM_GREEN;

						/*Reject if not opposite end of name
						 prefixes only on postfix egos,
						 postfixes only on prefix egos.
						 */
						if (ego != -1 && e_ptr->before == e_info[ego].before)
							continue;

						/*Color it red of the alchemist doesn't have the essences to create it*/
						if (!alchemist_items_check(*tval, 0, i, 0, TRUE))
							color = TERM_RED;

						/* add this ego to the list*/
						strip_and_print(e_name + e_info[i].name, color, num);
						validc[num] = color;
						choice[num++] = i;
						break;
					}
			}
		}
		else
		{
			char skipped = 0;
			num = 0;
			if (mod40 != 0)
			{
				strip_and_print("--MORE--", TERM_WHITE, num);
				validc[num] = TERM_WHITE;
				choice[num++] = -1;
			}

			for (i = 1; (num < 39) && (i < max_k_idx); i++)
			{
				object_kind *k_ptr = &k_info[i];

				/* Analyze matching items */
				if (k_ptr->tval == *tval || (k_ptr->tval == TV_POTION2 && *tval == TV_POTION))
				{
					char color = TERM_GREEN;
					/* Hack -- Skip instant artifacts */
					if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

					/*Don't display recipes that the alchemist doesn't know about*/
					if (!k_ptr->know && !wizard) continue;

					/*Skip recipes that are somehow known, but don't exist*/
					if (!alchemist_exists(k_ptr->tval, k_ptr->sval, 0, 0))
						continue;

					/* Skip the first 39 if they hit 'more' */
					if (skipped++ < mod40*39)
						continue;

					/* Color 'unable to create' items different */
					if (!alchemist_items_check(k_ptr->tval, k_ptr->sval, 0, 0, TRUE))
						color = TERM_RED;

					/* Acquire the "name" of object "i" */
					/* and print it in it's place */
					strip_and_print(k_name + k_ptr->name, color, num);

					/* Remember the object index */
					validc[num] = color;
					choice[num++] = i;
				}
			}
			if (num == 39)
			{
				strip_and_print("--MORE--", TERM_WHITE, num);
				validc[num] = TERM_WHITE;
				choice[num++] = -1;
			}
		}

		/* We need to know the maximal possible remembered object_index */
		max_num = num;
		string = "What Kind of %s? (* to see recipe) [%c-%c,*]";
		num = 0xff;

		/* Pretend they're all undoable if we where called to display recipes */
		if (recipe)
		{
			for ( num = 0 ; num < max_num ; num++)
				if (validc[num] != TERM_WHITE) validc[num] = TERM_RED;
			string = "show which %s recipe? [%c-%c]";
		}

		while (num == 0xff || num >= max_num)
		{
			ch = selectchar[max_num - 1];
			/* Choose! */
			if ( max_num == 0 || !get_com(format(string, tval_desc2, selectchar[0], ch), &ch))
			{
				break;
			}

			/* Extra breaks for recipe */
			if (recipe && (ch == '\r' || ch == ' ' || ch == ESCAPE ))
				break;

			/* Analyze choice */
			num = selectitem[(byte)ch];

			/* Pretend that we don't have enough essences for anything */
			if (ch == '*' )
			{
				for ( num = 0 ; num < max_num ; num++)
					if (validc[num] != TERM_WHITE) validc[num] = TERM_RED;
				string = "Show which %s recipe? [%c-%c]";
			}
		}
		if ( num == 0xff || max_num == 0 || num >= max_num)
			break;

		if ( validc[num] == TERM_WHITE )
		{
			if (num == 0)
				mod40--;
			else
				mod40++;
			if ( mod40 < 0)
				mod40 = 0;
			continue;
		}

		/* If we don't have enough essences, or user asked for recipes */
		if ( validc[num] != TERM_GREEN )
		{
			/* Display the recipe */
			if (ego)
				alchemist_display_recipe(*tval, sval, choice[num]);
			else
				alchemist_display_recipe(k_info[choice[num]].tval, k_info[choice[num]].sval, 0);
		}
		else
			done = TRUE;

	}/*while(!done)*/

	/* Restore screen contents */
	Term_load();
	character_icky = FALSE;

	/* User abort, or no choices */
	if (max_num == 0 || num == 0xff || num >= max_num)
	{
		if (max_num == 0)
			msg_print("You don't know of anything you can make using that.");
		return ( -1);
	}
	if ( validc[num] != TERM_GREEN )
		return ( -1);

	/* And return successful */
	if ( ego )
		return choice[num];

	/* Set the tval, should be the same unless they selected a potion2 */
	if (*tval != k_info[choice[num]].tval && *tval != TV_POTION)
		msg_print("Coding error: tval != TV_POTION");
	*tval = k_info[choice[num]].tval;
	return ( k_info[choice[num]].sval );
}

/* Set the 'known' flags for all objects with a level <= lev
 * This lets the budding alchemist create basic items.
 */
void alchemist_learn_all(int lev)
{
	int i;

	if ( !get_skill(SKILL_ALCHEMY) )
		return;

	/* msg_format("You learn about level %d items",lev); */

	for ( i = 0 ; i < max_k_idx ; i++ )
		if ( k_info[i].level <= lev )
			if (alchemist_exists(k_info[i].tval, k_info[i].sval, 0, 0))
				k_info[i].know = TRUE;
}

void alchemist_learn_ego(int ego)
{
	char *name;
	int i;

	/* some Paranoia*/
	if ( !ego || ego >= max_e_idx )
		return;

	/* Get the ego items name */
	name = e_name + e_info[ego].name;
	while (strchr(name, ' '))
		name = strchr(name, ' ') + 1;

	/* Don't learn about egos without recipes, and
	 * always learn about the passed ego item. */
	if (alchemist_exists(0, 0, ego, 0))
	{
		alchemist_known_egos[ego / 32] |= (1 << (ego % 32));
		/* msg_format("You learn about '%s' ego items.",e_name+e_info[ego].name); */
	}
	else
	{
		return;
	}

	/* Don't mass learn about egos that have no name. */
	if ( name[0] == 0 )
	{
		return;
	}

	/* Look through all ego's for matching name */
	/* Note that the original ego is marked here too */
	for ( i = 0 ; i < max_e_idx ; i++ )
		if ( strstr(e_name + e_info[i].name, name) != NULL        /*Last word of name exists in this ego's name*/
		                && alchemist_exists(0, 0, i, 0)                      /*There exists a recipe for this*/
		                && !(alchemist_known_egos[i / 32] & (1 << (i % 32)) ) ) /*Not already known*/
			/*&& (e_name+e_info[i].name)[0])non-blank name*/
		{
			alchemist_known_egos[i / 32] |= (1 << (i % 32));
			/* msg_format("You learn about '%s' ego items.",e_name+e_info[i].name); */
		}

	return;
}

/* Alchemist has learned about a new item.
 * Learn about not only it, but ALL egos with the
 * same name.
 */
int alchemist_learn_object(object_type *o_ptr)
{

	/* Allow alchemist to create this item,
	 and.. learn about it even if the player
	 doesn't currently have the alchemy skill
	 */
	k_info[o_ptr->k_idx].know = TRUE;

	/* Not Paranoia, identify_fully calls this always */
	if ( !get_skill(SKILL_ALCHEMY) )
		return FALSE;

	if ( artifact_p(o_ptr) )
	{
		char o_name[80];
		u32b f1, f2, f3, f4, f5, esp;
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Randarts and normal artifacts both*/
		alchemist_known_artifacts[0] |= f1;
		alchemist_known_artifacts[1] |= f2;
		alchemist_known_artifacts[2] |= f3;
		alchemist_known_artifacts[3] |= f4;
		alchemist_known_artifacts[4] |= f5;
		alchemist_known_artifacts[5] |= esp;

		object_desc(o_name, o_ptr, 1, 0);
		msg_format("You learn all about the abilities of %s!", o_name);
	}
	if (o_ptr->name2)
		alchemist_learn_ego(o_ptr->name2);

	if (o_ptr->name2b)
		alchemist_learn_ego(o_ptr->name2b);

	return (TRUE);
}

/* Alchemist has gained a level - set the ego flags
 * for all egos <= lev/4.
 */
void alchemist_gain_level(int lev)
{
	object_type forge;
	object_type *o_ptr = &forge;

	if ( lev == 0)
	{
		/* Learn about potions of Detonation */
		k_info[417].know = TRUE;
	}
	if ( lev == 5)
	{
		int ego;
		int egos[] = {
		        7/*armor of resist fire*/
		        , 18/*shield of resist fire*/
		        , 74/*shocking weapon*/
		        , 75/*fiery weapon*/
		        , 76/*frozen weapon*/
		        , 77/*Venomous weapon*/
		        , 78/*Chaotic weapon*/
		        , 115/*projectile of venom*/
		        , 116/*projectile of Acid*/
		        , 122/*projectile of flame*/
		        , 123/*projectile of frost*/
		        , 137/*Lite of fearlessness*/
		        , 0 /*terminator*/
		};
		object_wipe(o_ptr);
		/* learn about some basic ego items */
		/* Note that this is just to get you started. */
		for ( ego = 0 ; egos[ego] ; ego++)
		{
			o_ptr->name2 = egos[ego];
			alchemist_learn_object(o_ptr);
		}
		msg_print("You recall your old master teaching you about elemental item infusing.");
	}
	if ( lev == 10)
	{
		/*For 'hard rooms' Players only, learn about diggers.*/
		if (ironman_rooms)
		{
			msg_print("There's gotta be an easier way to get into all these vaults!");
			object_wipe(o_ptr);
			o_ptr->name2 = 101;  /* Ego item, 'of digging' */
			alchemist_learn_object(o_ptr);
		}
	}
	if ( lev == 25)
	{
		msg_print("You recall your old master reminiscing about legendary infusings");
		msg_print("and the Philosophers' stone.");

		/* No auto-learn on artifacts - by this level, you'll have *ID*'d several */
	}
	if ( lev == 25)
	{
		msg_print("You wonder about shocking daggers of slay evil.");
	}
	if ( lev == 50)
	{
		/* learn about Temporary item creation */
		/* Note that this is the ONLY way to learn this,
		 because spells which create a temporary item
		 also fully ID it. */
		alchemist_known_artifacts[4] |= TR5_TEMPORARY;
		msg_print("It suddenly occurs to you that artifacts don't *HAVE* to be permanent...");
	}

	/* Every Four Levels, learn about items that are
	 * less than that.
	 * Note that this isn't a significant effect after the
	 * first few levels, as the level at which you are learning
	 * things here quickly drops behind the level at which you
	 * are finding items.
	 */
	if ( (lev & 0x3) != 0 )
		return;
	lev = (lev >> 2) + 1;
	alchemist_learn_all(lev);

}

/* This, in combination with some code in loadsave.c,
 insures that alchemist_gain_level is called EXACTLY
 once with each possible value during the characters
 lifetime.
 */
void alchemist_check_level()
{
	u32b lev = get_skill(SKILL_ALCHEMY);
	if ( alchemist_gained > lev )
		return;
	/*Paranoia*/
	if ( !lev )
		return;
	while ( alchemist_gained <= lev )
		alchemist_gain_level(alchemist_gained++);
}

/*
 * do_cmd_cast calls this function if the player's class
 * is 'alchemist'.
 */
void do_cmd_alchemist(void)
{
	int item, ext = 0;
	int value, basechance;
	int askill;
	bool_ repeat = 0;
	char ch;

	object_type *o_ptr, *q_ptr;
	object_type forge, forge2;
	byte carry_o_ptr = FALSE;

	cptr q, s;

	/* With the new skill system, we can no longer depend on
	 * check_exp to handle the changes and learning involved in
	 * gaining levels.
	 * So we'll have to check for it here.
	 */
	alchemist_check_level();
	askill = get_skill(SKILL_ALCHEMY);


	q_ptr = &forge;

	o_ptr = &p_ptr->inventory[INVEN_HANDS];
	if ((o_ptr->tval != TV_GLOVES) || (o_ptr->sval != SV_SET_OF_LEATHER_GLOVES))
	{
		msg_print("You must wear gloves in order to do alchemy.");
		return;
	}

	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	while (TRUE)
	{
		if (!get_com("[P]ower, [R]echarge or [L]eech an item, [E]xtract essences, or recipe [B]ook?", &ch))
		{
			ext = 0;
			break;
		}
		if (ch == ' ' )
		{
			ext = 0;
			break;
		}
		if (ch == 'P' || ch == 'p')
		{
			ext = 1;
			break;
		}
		if (ch == 'E' || ch == 'e')
		{
			ext = 2;
			break;
		}
		if (ch == 'R' || ch == 'r')
		{
			ext = 3;
			break;
		}
		if (ch == 'L' || ch == 'l')
		{
			ext = 2;
			repeat = 1;
			break;
		}
		if (ch == 'B' || ch == 'b')
		{
			ext = 4;
			break;
		}
	}

	/**********Add a power*********/
	if (ext == 1)
	{
		int i, qty, tval, sval = 0, ego = 0;
		char o_name[200];

		/* Get an item */
		q = "Empower which item? ";
		s = "You have no empowerable items.";
		item_tester_hook = item_tester_hook_empower;

		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

		/* Get the item */
		o_ptr = get_object(item);

		/* Create an artifact from an ego or double ego item,
		 * from a previous artifact, or finish an artifact
		 */
		if ((askill >= 25) && (artifact_p(o_ptr) || o_ptr->name2) && has_ability(AB_CREATE_ART))
		{
			if (get_check("Create an artifact?"))
			{
				do_cmd_toggle_artifact(o_ptr);
				return;
			}
			/* Don't change artifacts or double ego items further */
			else if (artifact_p(o_ptr) || (o_ptr->name2 && o_ptr->name2b))
				return;
		}
		/*Ok, now we have the item, so we can now pick recipes.
		 Note: No recipe is known unless we have 'extracted' from
		 that object type. I.E. the 'know' flag (also greater identify)
		 is set.
		 */

		/* Here we're not setting what kind of ego item it IS,
		 * where' just deciding that it CAN be an ego item */
		if ( o_ptr->name2 ) /* creating a DUAL ego */
			ego = TRUE;
		if ( o_ptr->tval < 40 && o_ptr->tval != TV_BOTTLE)
			ego = TRUE;
		if ( o_ptr->tval == TV_ROD_MAIN || o_ptr->tval == TV_DAEMON_BOOK || o_ptr->tval == TV_BOOK)
			ego = TRUE;

		sval = o_ptr->sval;
		if (!ego)
		{
			switch ( o_ptr->tval)
			{
			case TV_WAND:
				sval = SV_WAND_NOTHING;
				break;
			case TV_RING:
				sval = SV_RING_NOTHING;
				break;
			case TV_STAFF:
				sval = SV_STAFF_NOTHING;
				break;
			case TV_BOTTLE:
				sval = 1;
				break;
			case TV_AMULET:
				sval = SV_AMULET_NOTHING;
				break;
			case TV_SCROLL:
				sval = SV_SCROLL_NOTHING;
				break;
			case TV_ROD:
				sval = SV_ROD_NOTHING;
				break;
			}
		}
		if ( o_ptr->sval != sval )
			ego = TRUE;

		tval = o_ptr->tval;
		sval = o_ptr->sval;

		/*HACK - bottles don't have the same tval as potions*/
		/*Everything else will have the same tval after empowering*/
		if (tval == TV_BOTTLE) tval = TV_POTION;
		if (ego)
			if (o_ptr->name2)
				ego = alchemist_recipe_select(&tval, sval, o_ptr->name2, FALSE);
			else
				ego = alchemist_recipe_select(&tval, sval, -1, FALSE);
		else
			sval = alchemist_recipe_select(&tval, 0, 0, FALSE);

		if ( sval < 0 || ego < 0)
			return;

		/* Check to make sure we have enough essences */
		/* theoretically this is taken care of by recipe_select*/
		/* but we'll double check just for paranoia. */
		if (!alchemist_items_check(tval, sval, ego, 0, TRUE))
		{
			msg_print("You do not have enough essences.");
			return;
		}

		/* Take a turn */
		energy_use = 100;

		/* Use up the essences */
		(void)alchemist_items_check(tval, sval, ego, -1, TRUE);

		/* Enchant stacks of ammunition at a time */
		if ( o_ptr->tval == TV_SHOT || o_ptr->tval == TV_ARROW || o_ptr->tval == TV_BOLT )
		{
			qty = 1;
			while (qty < o_ptr->number && alchemist_items_check(tval, sval, ego, -1, FALSE))
				qty++;
		}
		else
			qty = 1;

		/* Copy the object */
		q_ptr = &forge;
		object_copy(q_ptr, o_ptr);

		if ( o_ptr->tval == TV_WAND)
		{
			/* distribute charges on wands */
			q_ptr->pval = o_ptr->pval / o_ptr->number;
			o_ptr->pval -= q_ptr->pval;
		}

		o_ptr = q_ptr;
		o_ptr->number = qty;
		carry_o_ptr = TRUE;

		/* Destroy the initial object */
		inc_stack_size(item, -qty);


		if ( ego )
		{
			int pval, pval2;
			s32b pval3;

			pval = o_ptr->pval;
			pval2 = o_ptr->pval2;
			pval3 = o_ptr->pval3;

			if (o_ptr->name2)
				o_ptr->name2b = ego;
			else
				o_ptr->name2 = ego;
			o_ptr->pval = randint(e_info[ego].max_pval - 1) + 1;
			/* dilemma - how to prevent creation of cursed items,
			 * without allowing the creation of artifacts?
			 * We can't, unless we want to finalize the ego flags ourselves.
			 */
			apply_magic(o_ptr, askill * 2, FALSE, FALSE, FALSE);
			/* Remember what the old pval was, so that we can re-apply it. */
			if ( o_ptr->tval == TV_WAND
			                || o_ptr->tval == TV_RING
			                || o_ptr->tval == TV_AMULET
			                || o_ptr->tval == TV_STAFF)
			{
				o_ptr->pval = pval;
				o_ptr->pval2 = pval2;
				o_ptr->pval3 = pval3;
			}
			else if (o_ptr->tval == TV_ROD_MAIN)
			{
				o_ptr->pval = pval;
			}
			else if ((o_ptr->tval == TV_BOOK) && (o_ptr->sval == 255))
			{
				o_ptr->pval = pval;
			}
			else if (o_ptr->tval == TV_SHOT
			                || o_ptr->tval == TV_ARROW
			                || o_ptr->tval == TV_BOLT)
			{
				o_ptr->pval2 = pval2;
			}
			else if (o_ptr->tval == TV_INSTRUMENT)
			{
				o_ptr->pval2 = pval2;
			}

			/* Calculate failure rate, lev=val/2500+5 */
			value = MIN(e_info[o_ptr->name2].cost, 50000);
			if (o_ptr->name2b) value += MIN(e_info[o_ptr->name2b].cost, 50000);
			basechance = (value / 1000 + 5 - get_skill_scale(SKILL_ALCHEMY, 100) ) * 10;
			if ( basechance < 0) basechance = 0;
			if ( basechance > 100) basechance = 100;

			value = object_value_real(o_ptr);

		}
		else /* not an ego item */
		{
			o_ptr = &forge;
			object_wipe(o_ptr);
			object_prep(o_ptr, lookup_kind(tval, sval));
			hack_apply_magic_power = -99;
			apply_magic(o_ptr, askill * 2, FALSE, FALSE, FALSE);
			if ( o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF)
				o_ptr->pval = 0;
			value = object_value_real(o_ptr);

			basechance = k_info[o_ptr->k_idx].level - askill * 2;
			basechance *= 10;

			/* Can't fail more that 100% of the time... */
			if (basechance > 100)
				basechance = 100;
			/* Always success in creation of potion of detonations */
			if (o_ptr->tval == TV_POTION && o_ptr->sval == SV_POTION_DETONATIONS)
			{
				basechance /= 10;
			}
		}

		/* Use up gold to create items */
		/* this has the effect of making the alchemist
		 chronically short of funds, unless he finds the
		 philosopher's stone. It also means the easiest
		 things to make are 'bad', like a potion of
		 detonations...
		 */
		/* Problem - to restrictive. We need something
		 which requires less money. But at the same time,
		 we don't want an 'easy cash' situation. Maybe something
		 like '10% * level difference', meaning at skill level 5,
		 level one items are free? But egos are frequently level
		 zero! Maybe egos are forced to level 25? with a cost ceiling?
		 I mean, Potions and scrolls are really the problem causing the
		 'easy cash' situation, it's ego items. Ego items require
		 relatively few essences, and the rewards are HUGE. Most powerful
		 potions and scrolls require rare essences. Maybe force all egos
		 to require a magic essence? But then you'd get lots of magic
		 from distilling them. Maybe consumed in the creation? then when
		 you got a powerful item, you could make one ego item...
		 But if making things doesn't take gold, what about the cash
		 does the Philosopher's stone do?
		 Time*/

		/* 0% failure if you have the stone */
		if ( alchemist_has_stone())
			basechance = 0;

		if (basechance > 0 && value)
		{
			char string[80];
			string[0] = '0';
			string[1] = 0;

			msg_format("The chance of success is only %d%%!", 100-basechance);
			get_string("How much gold do you want to add?", string, 50);
			i = atoi(string);
			/* Note: don't trust the user to enter a positive number... */
			if ( i < 0)
				i = 0;
			if ( i > p_ptr->au)
				i = p_ptr->au;

			if (i)
			{
				basechance = basechance - (i * 20) / value;
				msg_format("The chance of success improved to %d%%.", 100-basechance);
			}

			if (randint(100) < basechance )
				/*creation failed, even with the extra gold...*/
				carry_o_ptr = FALSE;

			/* Redraw gold */
			p_ptr->au -= i;
			p_ptr->redraw |= (PR_GOLD);
		}

		/* Set fully identified
		 * After all, the player just made it...
		 */
		object_aware(o_ptr);
		object_known(o_ptr);
		o_ptr->ident |= IDENT_MENTAL;
		o_ptr->found = OBJ_FOUND_SELFMADE;

		object_desc(o_name, o_ptr, FALSE, 0);

		if ( carry_o_ptr)
		{
			msg_format("You have successfully created %s %s",
			           (o_ptr->number > 1 ? "some" : (is_a_vowel(o_name[0]) ? "an" : "a")),
			           o_name);

			if (inven_carry_okay(o_ptr))
				inven_carry(o_ptr, FALSE);
			else
			{
				drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);
				msg_format("You drop the %s", o_name);
			}
			carry_o_ptr = FALSE;
		}
		else /* don't carry, or in other words... */
		{
			int level = k_info[o_ptr->k_idx].level;
			if (o_ptr->name1) /* created ego item */
				level += e_info[o_ptr->name2].level;

			msg_format("Your attempt backfires! Your %s explodes!", o_name);
			take_hit(damroll(3, level - askill ) , "Alchemical Explosion");
			p_ptr->redraw |= (PR_HP);
		}

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

		/* Optimize the entire p_ptr->inventory - needed because we
		 don't know how many essences where used, and we may
		 have 'used up' a wielded item as well.
		 */
		for ( item = 0 ; item < INVEN_TOTAL ; item++ )
			inven_item_optimize(item);

		/**********Extract a power*********/
	}
	else if (ext == 2)
	{
		int ego;
		bool_ discharge_stick = FALSE;

		/* s_ptr holds the empty items */
		object_type *s_ptr = NULL;
		bool_ carry_s_ptr = FALSE;

		item_tester_hook = item_tester_hook_extractable;

		/* Get an item */
		q = "Extract from which item? ";
		s = "You have no item to extract power from.";
		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

		/* Get the item */
		o_ptr = get_object(item);

		/* This is to prevent creating magic essences by extracting
		 * from a recharged wand of dragon breath or something.
		 */
		if (( o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF )
		                && o_ptr->art_flags4 & TR4_RECHARGED)
		{
			msg_print("You cannot extract essences after it's been magically recharged.");
			return;
		}

		/* Take a turn */
		energy_use = 100;

		/* Handle Rods before the loop, since they don't stack */
		if (o_ptr->tval == TV_ROD_MAIN && o_ptr->pval != SV_ROD_NOTHING)
		{
			rod_tip_extract(o_ptr);
			return;
		}

		do
		{ /* Repeat (for leech command) */

			/* Create the items.
			 * we don't care if they drop to the ground,
			 * and if no action was taken, return
			 */
			ego = 0;
			if ( o_ptr->name2)
				ego = o_ptr->name2;

			/* For ego staves and wands (not of nothing), discharge before extracting the ego */
			discharge_stick = (o_ptr->pval > 0 &&
			                   ((o_ptr->tval == TV_STAFF && o_ptr->sval != SV_STAFF_NOTHING) ||
			                    (o_ptr->tval == TV_WAND && o_ptr->sval != SV_WAND_NOTHING)));
			if (discharge_stick)
				ego = 0;

			if (!alchemist_items_check(o_ptr->tval, o_ptr->sval, ego, 1, TRUE))
			{
				msg_print("You cannot extract anything from that item.");
				return;
			}

			if (o_ptr->name2b && !alchemist_items_check(o_ptr->tval, o_ptr->sval, o_ptr->name2b, 1, TRUE))
			{
				/* do nothing - if the second ego can't be extracted
				 because there is no recipe for it, simply destroy it
				 */
			}

			/* Once in three times, learn how to make the item */
			/* Sorry for the complicated if! Basically, if it's an
			 * unknown regular item or an unknown ego item, there's
			 * a one in 3 chance that it'll be id'd */
			if (((!ego && !k_info[o_ptr->k_idx].know)
			                || (ego && !(alchemist_known_egos[ego / 32] & (1 << (ego % 32)))))
			                && randint(3) == 1)
			{
				msg_print("While destroying it, you gain insight into this item.");
				/* If over level 10, the player has a chance of 'greater ID'
				 * on extracted items
				 */
				if (askill > 9)
					object_out_desc(o_ptr, NULL, FALSE, TRUE);
				alchemist_learn_object(o_ptr);
			}

			/* Always learn what kind of thing it is */
			object_known(o_ptr);
			object_aware(o_ptr);

			/* If it's a wand or staff with charges (but not of nothing),
			 * decrease number of charges, unstacking if needed.
			 * Otherwise, create the 'of nothing' item and destroy the old one.
			 */
			if (discharge_stick)
			{
				/* Unstack staves */
				if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
				{
					/* Create one local copy of the staff */
					q_ptr = &forge2;
					object_copy(q_ptr, o_ptr);

					/* Modify quantity */
					q_ptr->number = 1;

					/* Unstack the copied staff */
					o_ptr->number--;

					/* Use the local copy of the staff */
					o_ptr = q_ptr;
					carry_o_ptr = TRUE;
				}
				/* remove one charge */
				o_ptr->pval--;
			}
			else
			{
				/* Create the empty, plain item */
				/* If the item was already created, increase the number */
				if (carry_s_ptr)
				{
					s_ptr->number++;
				}
				else
				{
					/* Otherwise we must create a local copy of the empty item */
					int tval, sval;
					bool_ create_item = TRUE;

					tval = o_ptr->tval;
					if ( !ego && (tval == TV_POTION || tval == TV_POTION2))
						tval = TV_BOTTLE;

					sval = o_ptr->sval;

					if (!ego)
					{
						switch ( tval)
						{
						case TV_WAND:
							sval = SV_WAND_NOTHING;
							break;
						case TV_RING:
							sval = SV_RING_NOTHING;
							break;
						case TV_STAFF:
							sval = SV_STAFF_NOTHING;
							break;
						case TV_BOTTLE:
							sval = 1;
							break;
						case TV_AMULET:
							sval = SV_AMULET_NOTHING;
							break;
						case TV_SCROLL:
							sval = SV_SCROLL_NOTHING;
							break;
						case TV_ROD:
							sval = SV_ROD_NOTHING;
							break;
						default:
							create_item = FALSE;
						}
					}

					if (create_item)
					{
						/* Create the empty item */
						s_ptr = &forge;
						object_wipe(s_ptr);
						object_prep(s_ptr, lookup_kind(tval, sval));
						s_ptr->number = 1;

						/* Force creation of non ego non cursed */
						hack_apply_magic_power = -99;
						apply_magic(s_ptr, 0, FALSE, FALSE, FALSE);

						/* Hack -- remove possible curse */
						if (cursed_p(s_ptr))
						{
							s_ptr->art_flags3 &= ~(TR3_CURSED | TR3_HEAVY_CURSE);
							s_ptr->ident &= ~(IDENT_CURSED);
						}

						/* Restore pvals (e.g. charges ==0) of the item */
						if (ego && ((tval == TV_WAND) || (tval == TV_STAFF) ||
						                (tval == TV_RING) || (tval == TV_AMULET)))
						{
							s_ptr->pval = o_ptr->pval;
							s_ptr->pval2 = o_ptr->pval2;
							s_ptr->pval3 = o_ptr->pval3;
						}
						/* Restore the spell stored in a random book */
						else if ((o_ptr->tval == TV_BOOK) && (o_ptr->sval == 255))
						{
							s_ptr->pval = o_ptr->pval;
						}
						/* Restore the type of explosive ammo */
						else if (o_ptr->tval == TV_SHOT || o_ptr->tval == TV_ARROW
						                || o_ptr->tval == TV_BOLT)
						{
							s_ptr->pval2 = o_ptr->pval2;
						}
						/* Restore the music stored in an instrument */
						else if (o_ptr->tval == TV_INSTRUMENT)
						{
							s_ptr->pval2 = o_ptr->pval2;
						}

						object_aware(s_ptr);
						object_known(s_ptr);
						s_ptr->ident |= IDENT_STOREB;

						/* The empty item will be added to the p_ptr->inventory later */
						carry_s_ptr = TRUE;
					}
				}

				/* Now, we can delete the original (leeched) object.
				 * Is o_ptr an p_ptr->inventory / floor item or a local copy?
				 */
				if (!carry_o_ptr)
				{
					/* Break the leech-loop if it was the last item */
					if (o_ptr->number == 1)
						repeat = 0;

					inc_stack_size(item, -1);
				}
				else
				{
					/* Forget the local object */
					carry_o_ptr = FALSE;

					/* reset o_ptr to the original stack,
					 * which contains at least another item */
					o_ptr = get_object(item);
				}
			}
		}
		while ( repeat == 1);

		/* If we carry empty items, add them to the p_ptr->inventory */
		if (carry_s_ptr)
			inven_carry(s_ptr, TRUE);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

		/******* Recharge an item *******/
	}
	else if (ext == 3)
	{
		int item;

		cptr q, s;

		item_tester_hook = item_tester_hook_recharge;

		/* Get an item */
		q = "Recharge which item? ";
		s = "You have no rechargable items.";
		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR ))) return;

		/* Get the item */
		o_ptr = get_object(item);

		/* Make sure we have enough essences to recharge this */
		if (!alchemist_items_check(o_ptr->tval, o_ptr->sval, 0, 0, TRUE))
		{
			msg_print("You don't have the essences to recharge this item.");
			return;
		}

		/* Take a turn */
		energy_use = 100;

		/* Destroy the essences */
		(void)alchemist_items_check(o_ptr->tval, o_ptr->sval, 0, -1, TRUE);

		if ((o_ptr->tval == TV_STAFF) && (o_ptr->number > 1))
		{
			/* Unstack staves */
			/* Get local object */
			q_ptr = &forge2;

			/* Obtain a local object */
			object_copy(q_ptr, o_ptr);

			/* Modify quantity */
			q_ptr->number = 1;

			/* Unstack the used item */
			o_ptr->number--;

			o_ptr = q_ptr;
			carry_o_ptr = TRUE;
		}
		o_ptr->pval++;
	}
	else if ( ext == 4)
	{
		alchemist_recipe_book();
	}
	/* Just in case - */
	if (carry_o_ptr)
	{
		/* the o_ptr item was probably an unstacked staff
		 * Anyway, we need to add it to the p_ptr->inventory */
		if (inven_carry_okay(o_ptr))
			inven_carry(o_ptr, TRUE);
		else
			drop_near(o_ptr, 0, p_ptr->py, p_ptr->px);
	}
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

		p_ptr->redraw |= PR_PIETY | PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP;
		energy_use = 100;
	}
}


/*
 * Return percentage chance of spell failure.
 */
int spell_chance_random(random_spell* rspell)
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
	char buff[80];

	random_spell* rspell;

	int i;


	prt(format("      %-30s Lev Fail Mana Damage ", "Name"), 1, 20);

	for (i = 0; i < max; i++)
	{
		rspell = &random_spells[batch * 10 + i];

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
static random_spell* select_spell_from_batch(int batch)
{
	char tmp[160];

	char out_val[30];

	char which;

	int mut_max = 10;

	random_spell* ret;


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	if (spell_num < (batch + 1) * 10)
	{
		mut_max = spell_num - batch * 10;
	}

	strnfmt(tmp, 160, "(a-%c, A-%cto browse, / to rename, - to comment) Select a power: ",
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
	char tmp[160];

	char which;

	int batch_max = (spell_num - 1) / 10;

	random_spell *ret;


	/* Too confused */
	if (p_ptr->confused)
	{
		msg_print("You can't use your powers while confused!");
		return NULL;
	}

	/* No spells available */
	if (spell_num == 0)
	{
		msg_print("There are no spells you can cast.");
		return NULL;
	}

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
		if (flush_failure) flush();

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

		sound(SOUND_FAIL);

		/* Let time pass */
		if (is_magestaff()) energy_use = 80;
		else energy_use = 100;

		/* Mana is spent anyway */
		p_ptr->csp -= s_ptr->mana;

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->redraw |= (PR_MANA);

		return;
	}


	p_ptr->csp -= s_ptr->mana;

	s_ptr->untried = FALSE;
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
	p_ptr->redraw |= (PR_MANA);
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
		char *ammo_name;
		char *aura_name;
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
		if (flush_failure) flush();
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

	if (p_ptr->inside_arena)
	{
		msg_print("This place seems devoid of life.");
		msg_print(NULL);
		return;
	}

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
		bool_ use_great = FALSE;

		if (p_ptr->disembodied)
		{
			msg_print("You don't currently own a body to use.");
			return;
		}

		/* Do we have access to all the powers ? */
		if (get_skill_scale(SKILL_POSSESSION, 100) >= r_info[p_ptr->body_monster].level)
			use_great = TRUE;

		use_symbiotic_power(p_ptr->body_monster, use_great, FALSE, FALSE);

		if (p_ptr->csp < 0)
		{
			msg_print("You lose control of your body!");
			if (!do_cmd_leave_body(FALSE))
			{
				cmsg_print(TERM_VIOLET,
				           "You are forced back into your body by your cursed items, "
				           "you suffer a system shock!");

				p_ptr->chp = 1;

				/* Display the hitpoints */
				p_ptr->redraw |= (PR_HP);
			}
		}
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
static bool_ item_tester_hook_convertible(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_JUNK) || (o_ptr->tval == TV_SKELETON)) return TRUE;

	/* Assume not */
	return (FALSE);
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

		cptr q, s;

		item_tester_hook = item_tester_hook_convertible;

		/* Get an item */
		q = "Convert which item? ";
		s = "You have no item to convert.";
		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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

		cptr q, s;

		item_tester_hook = item_tester_hook_convertible;

		/* Get an item */
		q = "Convert which item? ";
		s = "You have no item to convert.";
		if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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
		if (flush_failure) flush();
		msg_format("You failed to concentrate hard enough!");
		sound(SOUND_FAIL);

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
				p_ptr->redraw |= (PR_HP);

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
		sound(SOUND_ZAP);

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

				o_ptr->art_flags5 |= TR5_TEMPORARY;
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
					p_ptr->redraw |= (PR_HP);

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
		(void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

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
	p_ptr->redraw |= (PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
}

/* Runecrafters -- Move this into variable.c XXX XXX XXX */
static s32b rune_combine = 0;

/*
 * Hook to determine if an object is "runestone"
 */
static bool_ item_tester_hook_runestone(object_type *o_ptr)
{
	if (o_ptr->tval != TV_RUNE2) return (FALSE);

	if (o_ptr->sval != RUNE_STONE) return (FALSE);

	if (o_ptr->pval != 0) return (FALSE);

	/* Assume yes */
	return (TRUE);
}


static bool_ item_tester_hook_runestone_full(object_type *o_ptr)
{
	if (o_ptr->tval != TV_RUNE2) return (FALSE);

	if (o_ptr->sval != RUNE_STONE) return (FALSE);

	if (o_ptr->pval == 0) return (FALSE);

	/* Assume yes */
	return (TRUE);
}


/*
 * Hook to determine if an object is "rune-able"
 */
static bool_ item_tester_hook_runeable1(object_type *o_ptr)
{
	if (o_ptr->tval != TV_RUNE1) return (FALSE);

	/* Assume yes */
	return (TRUE);
}


/*
 * Hook to determine if an object is "rune-able"
 */
static bool_ item_tester_hook_runeable2(object_type *o_ptr)
{
	if (o_ptr->tval != TV_RUNE2) return (FALSE);

	if (o_ptr->sval == RUNE_STONE) return (FALSE);

	if (rune_combine & BIT(o_ptr->sval)) return (FALSE);

	/* Assume yes */
	return (TRUE);
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


/*
 * Damage formula, for runes
 */
void rune_calc_power(s32b *power, s32b *powerdiv)
{
	/* Not too weak power(paranoia) */
	*power = (*power < 1) ? 1 : *power;
	*power += 3;

	*power = 37 * sroot(*power) / 10;

	/* To reduce the high level power, while increasing the low levels */
	*powerdiv = *power / 3;
	if (*powerdiv < 1) *powerdiv = 1;

	/* Use the spell multiplicator */
	*power *= (p_ptr->to_s / 2) ? (p_ptr->to_s / 2) : 1;
}


/*
 * Return percentage chance of runespell failure.
 */
int spell_chance_rune(rune_spell* spell)
{
	int chance, minfail;

	s32b power = spell->mana, power_rune = 0, powerdiv = 0;


	if (spell->rune2 & RUNE_POWER_SURGE)
	{
		power_rune += 4;
	}
	if (spell->rune2 & RUNE_ARMAGEDDON)
	{
		power_rune += 3;
	}
	if (spell->rune2 & RUNE_SPHERE)
	{
		power_rune += 2;
	}
	if (spell->rune2 & RUNE_RAY)
	{
		power_rune += 1;
	}

	rune_calc_power(&power, &powerdiv);

	chance = (5 * power_rune) + (power);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[A_DEX]] - 1);

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[A_DEX]];

	/* Return the chance */
	return clamp_failure_chance(chance, minfail);
}


/*
 * Combine the Runes
 */
int rune_exec(rune_spell *spell, int cost)
{
	int dir, power_rune = 0, mana_used, plev = get_skill(SKILL_RUNECRAFT);

	int chance;

	s32b power, powerdiv;

	int rad = 0, ty = -1, tx = -1, dam = 0, flg = 0;


	if (spell->rune2 & RUNE_POWER_SURGE)
	{
		power_rune += 4;
	}
	if (spell->rune2 & RUNE_ARMAGEDDON)
	{
		power_rune += 3;
	}
	if (spell->rune2 & RUNE_SPHERE)
	{
		power_rune += 2;
	}
	if (spell->rune2 & RUNE_RAY)
	{
		power_rune += 1;
	}


	power = spell->mana;

	if (cost && ((power * cost / 100) > p_ptr->csp - (power_rune * (plev / 5))))
	{
		power = p_ptr->csp - (power_rune * (plev / 5));
		mana_used = power + (power_rune * (plev / 5));
	}
	else
	{
		mana_used = (power * cost / 100) + (power_rune * (plev / 5));
	}

	rune_calc_power(&power, &powerdiv);

	dam = damroll(powerdiv, power);

	if (wizard) msg_format("Rune %dd%d = dam %d", powerdiv, power, dam);

	/* Extract the base spell failure rate */
	chance = spell_chance_rune(spell);

	/* Failure ? */
	if (rand_int(100) < chance)
	{
		int insanity = (p_ptr->msane - p_ptr->csane) * 100 / p_ptr->msane;
		char sfail[80];

		/* Flush input if told so */
		if (flush_failure) flush();

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

		sound(SOUND_FAIL);

		if (is_magestaff()) energy_use = 80;
		else energy_use = 100;

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->redraw |= (PR_MANA);
		return (mana_used);
	}

	if (spell->rune2 & RUNE_POWER_SURGE)
	{
		flg |= (PROJECT_VIEWABLE);
		ty = p_ptr->py;
		tx = p_ptr->px;
	}

	if (spell->rune2 & RUNE_ARMAGEDDON)
	{
		flg |= (PROJECT_THRU);
		flg |= (PROJECT_KILL);
		flg |= (PROJECT_ITEM);
		flg |= (PROJECT_GRID);
		flg |= (PROJECT_METEOR_SHOWER);
		rad = (power / 8 == 0) ? 1 : power / 8;
		rad = (rad > 10) ? 10 : rad;
		ty = p_ptr->py;
		tx = p_ptr->px;
	}

	if (spell->rune2 & RUNE_SPHERE)
	{
		flg |= (PROJECT_THRU);
		flg |= (PROJECT_KILL);
		flg |= (PROJECT_ITEM);
		flg |= (PROJECT_GRID);
		rad = (power / 8 == 0) ? 1 : power / 8;
		rad = (rad > 10) ? 10 : rad;
		ty = p_ptr->py;
		tx = p_ptr->px;
	}

	if (spell->rune2 & RUNE_RAY)
	{
		flg |= (PROJECT_THRU);
		flg |= (PROJECT_KILL);
		flg |= (PROJECT_BEAM);
		ty = -1;
		tx = -1;
	}
	if (spell->rune2 & RUNE_ARROW)
	{
		flg |= (PROJECT_THRU);
		flg |= (PROJECT_STOP);
		flg |= (PROJECT_KILL);
		ty = -1;
		tx = -1;
	}
	if (spell->rune2 & RUNE_SELF)
	{
		flg |= (PROJECT_THRU);
		flg |= (PROJECT_STOP);
		flg |= (PROJECT_KILL);
		ty = p_ptr->py;
		tx = p_ptr->px;
		unsafe = TRUE;
	}

	if ((ty == -1) && (tx == -1))
	{
		if (!get_aim_dir(&dir)) return (mana_used);

		/* Use the given direction */
		tx = p_ptr->px + ddx[dir];
		ty = p_ptr->py + ddy[dir];

		/* Hack -- Use an actual "target" */
		if ((dir == 5) && target_okay())
		{
			tx = target_col;
			ty = target_row;
		}
	}

	if (flg & PROJECT_VIEWABLE)
	{
		project_hack(spell->type, dam);
	}
	else if (flg & PROJECT_METEOR_SHOWER)
	{
		project_meteor(rad, spell->type, dam, flg);
	}
	else project(0, rad, ty, tx, dam, spell->type, flg);

	if (unsafe) unsafe = FALSE;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);

	return (mana_used);
}


/*
 * Test if all runes needed at in the player p_ptr->inventory
 */
bool_ test_runespell(rune_spell *spell)
{
	int i;

	object_type *o_ptr;

	bool_ typeok = FALSE;

	int rune2 = 0;


	for (i = 0; i < INVEN_WIELD; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_idx) continue;

		/* Does the rune1(type) match ? */
		if ((o_ptr->tval == TV_RUNE1) && (o_ptr->sval == spell->type))
		{
			typeok = TRUE;
		}

		if ((o_ptr->tval == TV_RUNE2) && (o_ptr->sval != RUNE_STONE))
		{
			/* Add it to the list */
			rune2 |= 1 << o_ptr->sval;
		}
	}

	/* Need all runes to be present */
	return (typeok && ((rune2 & spell->rune2) == spell->rune2));
}


/*
 * Ask for rune, rune2 and mana
 */
bool_ get_runespell(rune_spell *spell)
{
	int item, power_rune = 0, rune2 = 0, plev = get_skill(SKILL_RUNECRAFT);

	s32b power;

	int type = 0;

	object_type *o_ptr;

	cptr q, s;

	bool_ OK = FALSE;


	rune_combine = 0;

	/* Restrict choices to unused runes */
	item_tester_hook = item_tester_hook_runeable1;

	/* Get an item */
	q = "Use which rune? ";
	s = "You have no rune to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return FALSE;

	/* Get the item */
	o_ptr = get_object(item);
	type = o_ptr->sval;

	while (1)
	{
		/* Restrict choices to unused secondary runes */
		item_tester_hook = item_tester_hook_runeable2;

		OK = !get_item(&item, q, s, (USE_INVEN | USE_FLOOR));

		if (OK) break;

		/* Get the item */
		o_ptr = get_object(item);

		rune_combine |= 1 << o_ptr->sval;
		rune2 |= 1 << o_ptr->sval;
	}

	if (!rune2)
	{
		msg_print("You have not selected a second rune!");
		return (FALSE);
	}

	power = get_quantity("Which amount of Mana?",
	                     p_ptr->csp - (power_rune * (plev / 5)));
	if (power < 1) power = 1;

	spell->mana = power;
	spell->type = type;
	spell->rune2 = rune2;

	return (TRUE);
}


void do_cmd_rune(void)
{
	rune_spell spell;


	/* Require some mana */
	if (p_ptr->csp <= 0)
	{
		msg_print("You have no mana!");
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	if (!get_runespell(&spell)) return;

	/* Execute at normal mana cost */
	p_ptr->csp -= rune_exec(&spell, 100);

	/* Safety :) */
	if (p_ptr->csp < 0) p_ptr->csp = 0;

	/* Take a turn */
	if (is_magestaff()) energy_use = 80;
	else energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


/*
 * Print a batch of runespells.
 */
static void print_runespell_batch(int batch, int max)
{
	char buff[80];

	rune_spell* spell;

	int i;

	s32b power, powerdiv;

	int p, dp;


	prt(format("      %-30s Fail Mana Power", "Name"), 1, 20);

	for (i = 0; i < max; i++)
	{
		spell = &rune_spells[batch * 10 + i];

		power = spell->mana;
		rune_calc_power(&power, &powerdiv);
		p = power;
		dp = powerdiv;

		strnfmt(buff, 80, "  %c) %-30s %4d%% %4d %dd%d ", I2A(i), spell->name,
		        spell_chance_rune(spell), spell->mana, dp, p);

		prt(buff, 2 + i, 20);
	}
	prt("", 2 + i, 20);
}



/*
 * List ten random spells and ask to pick one.
 */

static rune_spell* select_runespell_from_batch(int batch, int *s_idx)
{
	char tmp[160];

	char out_val[30];

	char which;

	int mut_max = 10;

	rune_spell* ret;


	character_icky = TRUE;

	if (rune_num < (batch + 1) * 10)
	{
		mut_max = rune_num - batch * 10;
	}

	strnfmt(tmp, 160, "(a-%c, * to list, / to rename, - to comment) Select a power: ",
	        I2A(mut_max - 1));

	prt(tmp, 0, 0);

	while (1)
	{
		Term_save();

		print_runespell_batch(batch, mut_max);

		which = inkey();

		Term_load();

		if (which == ESCAPE)
		{
			*s_idx = -1;
			ret = NULL;
			break;
		}
		else if ((which == '*') || (which == '?') || (which == ' '))
		{
			print_runespell_batch(batch, mut_max);
		}
		else if ((which == '\r') && (mut_max == 1))
		{
			*s_idx = batch * 10;
			ret = &rune_spells[batch * 10];
			break;
		}
		else if (which == '/')
		{
			prt("Rename which power: ", 0, 0);
			which = tolower(inkey());

			if (isalpha(which) && (A2I(which) <= mut_max))
			{
				strcpy(out_val, rune_spells[batch*10 + A2I(which)].name);
				if (get_string("Name this power: ", out_val, 29))
				{
					strcpy(rune_spells[batch*10 + A2I(which)].name, out_val);
				}
				prt(tmp, 0, 0);
			}
			else
			{
				bell();
				prt(tmp, 0, 0);
			}
		}
		else
		{
			which = tolower(which);
			if (isalpha(which) && (A2I(which) < mut_max))
			{
				*s_idx = batch * 10 + A2I(which);
				ret = &rune_spells[batch * 10 + A2I(which)];
				break;
			}
			else
			{
				bell();
			}
		}
	}

	character_icky = FALSE;

	return (ret);
}


/*
 * Pick a random spell from a menu
 */

rune_spell* select_runespell(int *s_idx)
{
	char tmp[160];

	char which;

	int batch_max = (rune_num - 1) / 10;

	if (rune_num == 0)
	{
		msg_print("There are no runespells you can cast.");
		return (NULL);
	}

	character_icky = TRUE;
	Term_save();

	strnfmt(tmp, 160, "(a-%c) Select batch of powers: ", I2A(batch_max));

	prt(tmp, 0, 0);

	while (1)
	{
		which = inkey();

		if (which == ESCAPE)
		{
			Term_load();
			character_icky = FALSE;
			return (NULL);
		}
		else if ((which == '\r') && (batch_max == 0))
		{
			Term_load();
			character_icky = FALSE;
			return (select_runespell_from_batch(0, s_idx));

		}
		else
		{
			which = tolower(which);
			if (isalpha(which) && (A2I(which) <= batch_max))
			{
				Term_load();
				character_icky = FALSE;
				return (select_runespell_from_batch(A2I(which), s_idx));
			}
			else
			{
				bell();
			}
		}
	}
}


/*
 * Cast a memorized runespell
 * Note that the only limits are antimagic & conf, NOT blind
 */
void do_cmd_rune_cast()
{
	rune_spell *s_ptr;

	int s_idx;


	/* Require some mana */
	if (p_ptr->csp <= 0)
	{
		msg_print("You have no mana!");
		return;
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

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	s_ptr = select_runespell(&s_idx);

	if (s_ptr == NULL) return;

	/* Need the runes */
	if (!test_runespell(s_ptr))
	{
		msg_print("You lack some essential rune(s) for this runespell!");
		return;
	}

	/* Execute at normal mana cost */
	p_ptr->csp -= rune_exec(s_ptr, 100);

	/* Safety :) */
	if (p_ptr->csp < 0) p_ptr->csp = 0;

	/* Take a turn */
	if (is_magestaff()) energy_use = 80;
	else energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


/*
 * Cast a runespell from a carved runestone
 */
void do_cmd_runestone()
{
	rune_spell s_ptr;

	object_type *o_ptr;

	cptr q, s;

	int item;


	/* Require some mana */
	if (p_ptr->csp <= 0)
	{
		msg_print("You have no mana!");
		return;
	}

	/* Require lite */
	if (p_ptr->blind || no_lite())
	{
		msg_print("You cannot see!");
		return;
	}

	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
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

	/* Restrict choices to unused runes */
	item_tester_hook = item_tester_hook_runestone_full;

	/* Get an item */
	q = "Cast from which runestone? ";
	s = "You have no runestone to cast from.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	s_ptr.type = o_ptr->pval;
	s_ptr.rune2 = o_ptr->pval2;
	s_ptr.mana = o_ptr->pval3;

	/* Execute less mana */
	p_ptr->csp -= rune_exec(&s_ptr, 75);

	/* Safety :) */
	if (p_ptr->csp < 0) p_ptr->csp = 0;

	/* Take a turn */
	energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


/*
 * Add a runespell to the list
 */
void do_cmd_rune_add_mem()
{
	rune_spell s_ptr;

	rune_spell *ds_ptr = &rune_spells[rune_num];


	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}


	if (rune_num >= MAX_RUNES)
	{
		msg_print("You have already learn the maximun number of runespells!");
		return;
	}

	if (!get_runespell(&s_ptr)) return;

	ds_ptr->type = s_ptr.type;
	ds_ptr->rune2 = s_ptr.rune2;
	ds_ptr->mana = s_ptr.mana;
	strcpy(ds_ptr->name, "Unnamed Runespell");

	get_string("Name this runespell: ", ds_ptr->name, 29);

	rune_num++;

	/* Take a turn */
	energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


/*
 * Carve a runespell onto a Runestone
 */
void do_cmd_rune_carve()
{
	rune_spell s_ptr;

	object_type *o_ptr;

	cptr q, s;

	int item, i;

	char out_val[80];


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

	if (!get_check("Beware, this will destroy the involved runes, continue?"))
	{
		return;
	}

	if (!get_runespell(&s_ptr)) return;

	/* Restrict choices to unused runes */
	item_tester_hook = item_tester_hook_runestone;

	/* Get an item */
	q = "Use which runestone? ";
	s = "You have no runestone to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	o_ptr->pval = s_ptr.type;
	o_ptr->pval2 = s_ptr.rune2;
	o_ptr->pval3 = s_ptr.mana;

	/* Start with nothing */
	strcpy(out_val, "");

	/* Use old inscription */
	if (o_ptr->note)
	{
		/* Start with the old inscription */
		strcpy(out_val, quark_str(o_ptr->note));
	}

	/* Get a new inscription (possibly empty) */
	if (get_string("Name this runestone: ", out_val, 80))
	{
		/* Save the inscription */
		o_ptr->note = quark_add(out_val);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}

	/* Delete the runes */
	for (i = 0; i < INVEN_WIELD; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		if (o_ptr->k_idx)
		{
			bool_ do_del = FALSE;

			if ((o_ptr->tval == TV_RUNE1) && (o_ptr->sval == s_ptr.type)) do_del = TRUE;
			if ((o_ptr->tval == TV_RUNE2) && (BIT(o_ptr->sval) & s_ptr.rune2)) do_del = TRUE;

			if (do_del)
			{
				inc_stack_size_ex(i, -1, OPTIMIZE, NO_DESCRIBE);
			}
		}
	}

	/* Take a turn -- Carving takes a LONG time */
	energy_use = 400;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


/*
 * Remove a runespell
 */
void do_cmd_rune_del()
{
	rune_spell *s_ptr;

	int s_idx;

	int i;


	/* Not when confused */
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	s_ptr = select_runespell(&s_idx);

	if (s_ptr == NULL) return;

	/* Delete and move */
	for (i = s_idx + 1; i < rune_num; i++)
	{
		rune_spells[i - 1].type = rune_spells[i].type;
		rune_spells[i - 1].rune2 = rune_spells[i].rune2;
		rune_spells[i - 1].mana = rune_spells[i].mana;
		strcpy(rune_spells[i - 1].name, rune_spells[i].name);
	}
	rune_num--;

	/* Take a turn */
	energy_use = 100;

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);
	p_ptr->redraw |= (PR_MANA);
}


void do_cmd_rune_add()
{
	int ext = 0;

	char ch;


	/* Select what to do */
	while (TRUE)
	{
		if (!get_com("Add to [M]emory(need runes to cast) or "
		                "Carve a [R]unestone(less mana to cast)", &ch))
		{
			ext = 0;
			break;
		}
		if ((ch == 'M') || (ch == 'm'))
		{
			ext = 1;
			break;
		}
		if ((ch == 'R') || (ch == 'r'))
		{
			ext = 2;
			break;
		}
	}

	switch (ext)
	{
		/* Create a Spell in memory */
	case 1:
		{
			do_cmd_rune_add_mem();
			break;
		}

		/* Carve a Runestone */
	case 2:
		{
			do_cmd_rune_carve();
			break;
		}
	}
}


void do_cmd_runecrafter()
{
	int ext = 0;

	char ch;


	/* Select what to do */
	while (TRUE)
	{
		if (!get_com("Rune Spell:[C]reate, [D]elete, C[a]st, D[i]rectly Cast "
		                "or Use [R]unestone", &ch))
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
		if ((ch == 'A') || (ch == 'a'))
		{
			ext = 3;
			break;
		}
		if ((ch == 'I') || (ch == 'i'))
		{
			ext = 4;
			break;
		}
		if ((ch == 'R') || (ch == 'r'))
		{
			ext = 5;
			break;
		}
	}

	switch (ext)
	{
		/* Create a Spell */
	case 1:
		{
			do_cmd_rune_add();
			break;
		}

		/* Delete a Spell */
	case 2:
		{
			do_cmd_rune_del();
			break;
		}

		/* Cast a Spell */
	case 3:
		{
			do_cmd_rune_cast();
			break;
		}

		/* Directly Cast a Spell */
	case 4:
		{
			do_cmd_rune();
			break;
		}

		/* Cast a Runestone */
	case 5:
		{
			do_cmd_runestone();
			break;
		}
	}
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
		if (!get_com("Disrupt [C]ontinuum or [D]etect Traps", &ch))
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

		/* Detect Traps */
	case 2:
		{
			s16b skill = get_skill(SKILL_ANTIMAGIC);

			if (skill < 25)
			{
				msg_print("You cannot use your detection abilities yet.");
				break;
			}

			detect_traps(DEFAULT_RADIUS);

			if (skill >= 35) destroy_doors_touch();

			break;
		}
	}
}

/*
 * Hook to determine if an object is totemable
 */
static bool_ item_tester_hook_totemable(object_type *o_ptr)
{
	/* Only full corpse */
	if ((o_ptr->tval == TV_CORPSE) &&
	                ((o_ptr->sval == SV_CORPSE_CORPSE) || (o_ptr->sval == SV_CORPSE_SKELETON)))
	{
		return (TRUE);
	}

	/* Assume not */
	return (FALSE);
}


/*
 * Summoners
 */
void do_cmd_summoner_extract()
{
	object_type *o_ptr, forge, *q_ptr;

	cptr q, s;

	int item, r;

	bool_ partial;


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

	item_tester_hook = item_tester_hook_totemable;

	/* Get an item */
	q = "Use which corpse? ";
	s = "You have no corpse to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);


	if (r_info[o_ptr->pval2].flags1 & RF1_UNIQUE)
	{
		partial = FALSE;
	}
	else
	{
		partial = get_check("Do you want to create a partial totem?");
	}

	r = o_ptr->pval2;

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
	int i, status, x = 1, y = 1, rx, ry = 0, chance;

	bool_ used;

	monster_race *r_ptr = &r_info[r_idx];


	/* Uniques are less likely to be nice */
	if (r_ptr->flags1 & (RF1_UNIQUE))
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

	object_type *o_ptr;

	monster_type *m_ptr;


	/* Which Totem? */
	item_tester_tval = TV_TOTEM;

	q = "Summon from which Totem?";
	s = "There are no totems to summon from!";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Access the item */
	o_ptr = get_object(item);

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
 * Fighters may invoke The Rush.
 */
void do_cmd_blade(void)
{
	/* Are we already Rushed? */
	if (p_ptr->rush)
	{
		msg_format("You have %d turns of The Rush remaining", p_ptr->rush);
		return;
	}

	/* Are you sure? */
	if (!get_check("Are you sure you want to invoke The Rush?")) return;

	/* Let's Rush! */
	set_rush(2 + p_ptr->lev / 2 + randint(p_ptr->lev / 2));
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
		if (flush_failure) flush();
		msg_format("You failed to concentrate hard enough!");
		sound(SOUND_FAIL);
	}
	else
	{
		sound(SOUND_ZAP);

		/* spell code */
		switch (n)
		{
		case 0:
			{
				int dir, x, y;
				cave_type *c_ptr;
				monster_type *m_ptr;
				monster_race *r_ptr;
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
					r_ptr = race_inf(m_ptr);

					if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
					{
						msg_print("You can only hypnotise monsters that cannot move.");
					}
					else if (m_ptr->status < MSTATUS_PET)
					{
						msg_print("You can only hypnotise pets and companions.");
					}
					else if (r_ptr->flags9 & RF9_SPECIAL_GENE)
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
				object_type *o_ptr;

				cptr q, s;

				/* Restrict choices to monsters */
				item_tester_tval = TV_HYPNOS;

				/* Get an item */
				q = "Awaken which monster? ";
				s = "You have no monster to awaken.";
				if (!get_item(&item, q, s, (USE_FLOOR))) return;

				o_ptr = &o_list[0 - item];

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

				/* Redraw */
				p_ptr->redraw |= (PR_HP);

				/* Window stuff */
				p_ptr->window |= (PW_PLAYER);

				/* Display the monster hitpoints */
				p_ptr->redraw |= (PR_MH);

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

				if (0 > use_symbiotic_power(o_ptr->pval, FALSE, FALSE, TRUE))
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

				msg_format("%s is healed.", symbiote_name(TRUE));

				/* Display the monster hitpoints */
				p_ptr->redraw |= (PR_MH);

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

				if(0 > use_symbiotic_power(o_ptr->pval, TRUE, FALSE, TRUE))
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
				cave_type *c_ptr;
				monster_type *m_ptr;

				if (!tgt_pt(&x, &y)) return;

				c_ptr = &cave[y][x];

				if (!c_ptr->m_idx) break;

				m_ptr = &m_list[c_ptr->m_idx];
				use_symbiotic_power(m_ptr->r_idx, TRUE, FALSE, TRUE);

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
		(void)set_paralyzed(p_ptr->paralyzed + randint(5 * oops + 1));

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
	p_ptr->redraw |= (PR_MANA);

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
