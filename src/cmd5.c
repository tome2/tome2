/* File: cmd5.c */

/* Purpose: Class commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


#include "angband.h"
#include "lua/lua.h"
#include "tolua.h"

extern lua_State *L;


/* Maximum number of tries for teleporting */
#define MAX_TRIES 300

bool is_school_book(object_type *o_ptr)
{
	if (o_ptr->tval == TV_BOOK)
	{
		return TRUE;
	}
	else if (o_ptr->tval == TV_DAEMON_BOOK)
	{
		return TRUE;
	}
	else if (o_ptr->tval == TV_INSTRUMENT)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* Does it contains a schooled spell ? */
static bool hook_school_spellable(object_type *o_ptr)
{
	if (is_school_book(o_ptr))
		return TRUE;
	else
	{
		u32b f1, f2, f3, f4, f5, esp;

		/* Extract object flags */
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		if ((f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 != -1))
			return TRUE;
	}
	return FALSE;
}

/* Is it a book */
bool item_tester_hook_browsable(object_type *o_ptr)
{
	if (hook_school_spellable(o_ptr)) return TRUE;
	if (o_ptr->tval >= TV_BOOK) return TRUE;
	return FALSE;
}

/*
 * Are we using a mage staff
 */
bool is_magestaff()
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

	if (is_school_book(o_ptr))
		browse_school_spell(o_ptr->sval, o_ptr->pval, o_ptr);
	else if (f5 & TR5_SPELL_CONTAIN && o_ptr->pval2 != -1)
		browse_school_spell(255, o_ptr->pval2, o_ptr);
}

void do_cmd_browse(void)
{
	int item;

	cptr q, s;

	object_type *o_ptr;

	/* Restrict choices to "useful" books */
	item_tester_hook = item_tester_hook_browsable;

	/* Get an item */
	q = "Browse which book? ";
	s = "You have no books that you can read.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	do_cmd_browse_aux(o_ptr);
}

void do_poly_wounds(void)
{
	/* Changed to always provide at least _some_ healing */
	s16b wounds = p_ptr->cut;

	s16b hit_p = (p_ptr->mhp - p_ptr->chp);

	s16b change = damroll(p_ptr->lev, 5);

	bool Nasty_effect = (randint(5) == 1);


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

		if ((power > rand_int(5)) && (rand_int(4) == 0))
		{
			/* sex change */
			power -= 2;

			if (p_ptr->psex == SEX_MALE)
			{
				p_ptr->psex = SEX_FEMALE;
				sp_ptr = &sex_info[p_ptr->psex];
				strcpy(effect_msg, "female");
			}
			else
			{
				p_ptr->psex = SEX_MALE;
				sp_ptr = &sex_info[p_ptr->psex];
				strcpy(effect_msg, "male");
			}
		}

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

			lose_corruption(0);
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
			           ((is_a_vowel(rp_name[race_info[new_race].title])) ? "n" : ""),
			           race_info[new_race].title + rp_name);
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

		/* Calculate the height/weight */
		get_height_weight();


		check_experience();
		p_ptr->max_plv = p_ptr->lev;

		p_ptr->redraw |= (PR_BASIC);

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
		(void) gain_random_corruption(0);
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
 * Brand the current weapon
 */
void brand_weapon(int brand_type)
{
	object_type *o_ptr;

	cptr act = NULL;

	char o_name[80];


	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/*
	 * You can never modify artifacts / ego-items
	 * You can never modify cursed items
	 *
	 * TY: You _can_ modify broken items (if you're silly enough)
	 */
	if (!o_ptr->k_idx || artifact_p(o_ptr) || ego_item_p(o_ptr) ||
	                o_ptr->art_name || cursed_p(o_ptr))
	{
		if (flush_failure) flush();

		msg_print("The Branding failed.");

		return;
	}


	/* Save the old name */
	object_desc(o_name, o_ptr, FALSE, 0);

	switch (brand_type)
	{
	case 6:
		{
			act = "glows with godly power.";
			o_ptr->name2 = EGO_BLESS_BLADE;
			o_ptr->pval = randint(4);

			break;
		}
	case 5:
		{
			act = "seems very powerful.";
			o_ptr->name2 = EGO_EARTHQUAKES;
			o_ptr->pval = randint(3);

			break;
		}
	case 4:
		{
			act = "seems very unstable now.";
			o_ptr->name2 = EGO_DRAGON;
			o_ptr->pval = randint(2);

			break;
		}
	case 3:
		{
			act = "thirsts for blood!";
			o_ptr->name2 = EGO_VAMPIRIC;

			break;
		}
	case 2:
		{
			act = "is coated with poison.";
			o_ptr->name2 = EGO_BRAND_POIS;

			break;
		}
	case 1:
		{
			act = "is engulfed in raw chaos!";
			o_ptr->name2 = EGO_CHAOTIC;

			break;
		}
	default:
		{
			if (rand_int(100) < 25)
			{
				act = "is covered in a fiery shield!";
				o_ptr->name2 = EGO_BRAND_FIRE;
			}
			else
			{
				act = "glows deep, icy blue!";
				o_ptr->name2 = EGO_BRAND_COLD;
			}
		}
	}

	/* Apply the ego */
	apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
	o_ptr->discount = 100;

	msg_format("Your %s %s", o_name, act);

	enchant(o_ptr, rand_int(3) + 4, ENCH_TOHIT | ENCH_TODAM);
}

/*
 * Fetch an item (teleport it right underneath the caster)
 */
void fetch(int dir, int wgt, bool require_los)
{
	int ty, tx, i;

	bool flag;

	cave_type *c_ptr;

	object_type *o_ptr;

	char o_name[80];


	/* Check to see if an object is already there */
	if (cave[p_ptr->py][p_ptr->px].o_idx)
	{
		msg_print("You can't fetch when you're already standing on something.");
		return;
	}

	/* Use a target */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;

		if (distance(p_ptr->py, p_ptr->px, ty, tx) > MAX_RANGE)
		{
			msg_print("You can't fetch something that far away!");
			return;
		}

		c_ptr = &cave[ty][tx];

		if (!c_ptr->o_idx)
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
		ty = p_ptr->py;  /* Where to drop the item */
		tx = p_ptr->px;
		flag = FALSE;

		while (1)
		{
			ty += ddy[dir];
			tx += ddx[dir];
			c_ptr = &cave[ty][tx];

			if ((distance(p_ptr->py, p_ptr->px, ty, tx) > MAX_RANGE) ||
			                !cave_floor_bold(ty, tx)) return;

			if (c_ptr->o_idx) break;
		}
	}

	o_ptr = &o_list[c_ptr->o_idx];

	if (o_ptr->weight > wgt)
	{
		/* Too heavy to 'fetch' */
		msg_print("The object is too heavy.");
		return;
	}

	i = c_ptr->o_idx;
	c_ptr->o_idx = o_ptr->next_o_idx;
	cave[p_ptr->py][p_ptr->px].o_idx = i;  /* 'move' it */
	o_ptr->next_o_idx = 0;
	o_ptr->iy = p_ptr->py;
	o_ptr->ix = p_ptr->px;

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
 * Like all the random effect codes, this is *ugly*,
 * and there is not a single line of comment, so I can't tell
 * some fall throughs are really intended. Well, I know it's
 * intended to be bizarre :) -- pelpel
 */
void wild_magic(int spell)
{
	int counter = 0;
	int type = SUMMON_BIZARRE1 - 1 + randint(6);

	if (type < SUMMON_BIZARRE1) type = SUMMON_BIZARRE1;
	else if (type > SUMMON_BIZARRE6) type = SUMMON_BIZARRE6;

	switch (randint(spell) + randint(8) + 1)
	{
	case 1:
	case 2:
	case 3:
		{
			teleport_player(10);

			break;
		}

	case 4:
	case 5:
	case 6:
		{
			teleport_player(100);

			break;
		}

	case 7:
	case 8:
		{
			teleport_player(200);

			break;
		}

	case 9:
	case 10:
	case 11:
		{
			unlite_area(10, 3);

			break;
		}

	case 12:
	case 13:
	case 14:
		{
			lite_area(damroll(2, 3), 2);

			break;
		}

	case 15:
		{
			destroy_doors_touch();

			break;
		}

	case 16:
	case 17:
		{
			wall_breaker();

			/* I don't think this is a fall through -- pelpel */
			break;
		}

	case 18:
		{
			sleep_monsters_touch();

			break;
		}

	case 19:
	case 20:
		{
			trap_creation();

			break;
		}

	case 21:
	case 22:
		{
			door_creation();

			break;
		}

	case 23:
	case 24:
	case 25:
		{
			aggravate_monsters(1);

			break;
		}

	case 26:
		{
			/* Prevent destruction of quest levels and town */
			if (!is_quest(dun_level) && dun_level)
				earthquake(p_ptr->py, p_ptr->px, 5);

			break;
		}

	case 27:
	case 28:
		{
			break;
		}

	case 29:
	case 30:
		{
			apply_disenchant(0);

			break;
		}

	case 31:
		{
			lose_all_info();

			break;
		}

	case 32:
		{
			fire_ball(GF_CHAOS, 0, spell + 5, 1 + (spell / 10));

			break;
		}

	case 33:
		{
			wall_stone(p_ptr->py, p_ptr->px);

			break;
		}

	case 34:
	case 35:
		{
			while (counter++ < 8)
			{
				(void) summon_specific(p_ptr->py, p_ptr->px, (dun_level * 3) / 2, type);
			}

			break;
		}

	case 36:
	case 37:
		{
			activate_hi_summon();

			break;
		}

	case 38:
		{
			summon_cyber();

			/* I don't think this is a fall through -- pelpel */
			break;
		}

	default:
		{
			activate_ty_curse();
		}
	}

	return;
}


/*
 * Hack -- Determine if the player is wearing an artefact ring
 * specified by art_type, that should be an index into a_info
 */
bool check_ring(int art_type)
{
	int i;


	/* We are only interested in ring slots */
	i = INVEN_RING;

	/* Scan the list of rings until we reach the end */
	while (p_ptr->body_parts[i - INVEN_WIELD] == INVEN_RING)
	{
		/* Found the ring we were looking for */
		if (p_ptr->inventory[i].k_idx && (p_ptr->inventory[i].name1 == art_type))
		{
			return (TRUE);
		}

		/* Next item */
		i++;
	}

	/* Found nothing */
	return (FALSE);
}

/*
 * Return the symbiote's name or description.
 */
cptr symbiote_name(bool capitalize)
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
			strncpy(buf, r_name + r_ptr->name, sizeof(buf));
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
			strncpy(buf + 5, r_name + r_ptr->name, sizeof(buf) - 5);
		}
	}

	/* Just in case... */
	buf[sizeof(buf) - 1] = '\0';
	if (capitalize) buf[0] = toupper(buf[0]);
	return buf;
}

/*
 * Use a power of the monster in symbiosis
 */
int use_symbiotic_power(int r_idx, bool great, bool only_number, bool no_cost)
{
	int power = -1;

	int num = 0, dir = 0 , i;

	int powers[96];

	bool flag, redraw;

	int ask, plev = p_ptr->lev;

	char choice;

	char out_val[160];

	monster_race *r_ptr = &r_info[r_idx];

	int rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	int x = p_ptr->px, y = p_ptr->py, k;

	int rad;

	int label;


	/* List the monster powers -- RF4_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags4 & BIT(i))
		{
			if (monster_powers[i].great && (!great)) continue;
			if (!monster_powers[i].power) continue;
			powers[num++] = i;
		}
	}

	/* List the monster powers -- RF5_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags5 & BIT(i))
		{
			if (monster_powers[i + 32].great && (!great)) continue;
			if (!monster_powers[i + 32].power) continue;
			powers[num++] = i + 32;
		}
	}

	/* List the monster powers -- RF6_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags6 & BIT(i))
		{
			if (monster_powers[i + 64].great && (!great)) continue;
			if (!monster_powers[i + 64].power) continue;
			powers[num++] = i + 64;
		}
	}

	if (!num)
	{
		msg_print("You have no powers you can use.");
		return (0);
	}

	if (only_number) return (num);

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Get the last label */
	label = (num <= 26) ? I2A(num - 1) : I2D(num - 1 - 26);

	/* Build a prompt (accept all spells) */
	/* Mega Hack -- if no_cost is false, we're actually a Possessor -dsb */
	strnfmt(out_val, 78,
	        "(Powers a-%c, *=List, ESC=exit) Use which power of your %s? ",
	        label, (no_cost ? "symbiote" : "body"));

	/* Get a spell from the user */
	while (!flag && get_com(out_val, &choice))
	{
		/* Request redraw */
		if ((choice == ' ') || (choice == '*') || (choice == '?'))
		{
			/* Show the list */
			if (!redraw)
			{
				byte y = 1, x = 0;
				int ctr = 0;
				char dummy[80];

				strcpy(dummy, "");

				/* Show list */
				redraw = TRUE;

				/* Save the screen */
				character_icky = TRUE;
				Term_save();

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
	if (redraw)
	{
		Term_load();
		character_icky = FALSE;
	}

	/* Abort if needed */
	if (!flag)
	{
		energy_use = 0;
		return -1;
	}

	/* 'Powerful' monsters have wider radii */
	if (r_ptr->flags2 & RF2_POWERFUL)
	{
		rad = 1 + (p_ptr->lev / 15);
	}
	else
	{
		rad = 1 + (p_ptr->lev / 20);
	}


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
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ROCKET, dir, p_ptr->lev * 12, 1 + (p_ptr->lev / 20));

			break;
		}

		/* ARROW_1 */
	case 4:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(1, 6));

			break;
		}

		/* ARROW_2 */
	case 5:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(3, 6));

			break;
		}

		/* ARROW_3 */
	case 6:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(5, 6));

			break;
		}

		/* ARROW_4 */
	case 7:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ARROW, dir, damroll(7, 6));

			break;
		}

		/* BR_ACID */
	case 8:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_ELEC */
	case 9:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_FIRE */
	case 10:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_COLD */
	case 11:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_POIS */
	case 12:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_NETH */
	case 13:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_LITE */
	case 14:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_LITE, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_DARK */
	case 15:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DARK, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_CONF */
	case 16:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CONFUSION, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_SOUN */
	case 17:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SOUND, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_CHAO */
	case 18:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 7, rad);

			break;
		}

		/* BR_DISE */
	case 19:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISENCHANT, dir, p_ptr->lev * 7, rad);

			break;
		}

		/* BR_NEXU */
	case 20:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NEXUS, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BR_TIME */
	case 21:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_TIME, dir, p_ptr->lev * 3, rad);

			break;
		}

		/* BR_INER */
	case 22:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_INERTIA, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_GRAV */
	case 23:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_GRAVITY, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_SHAR */
	case 24:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_SHARDS, dir, p_ptr->lev * 8, rad);

			break;
		}

		/* BR_PLAS */
	case 25:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_PLASMA, dir, p_ptr->lev * 3, rad);

			break;
		}

		/* BR_WALL */
	case 26:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FORCE, dir, p_ptr->lev * 4, rad);

			break;
		}

		/* BR_MANA */
	case 27:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, p_ptr->lev * 5, rad);

			break;
		}

		/* BA_NUKE */
	case 28:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

		/* BR_NUKE */
	case 29:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NUKE, dir, p_ptr->lev * 8, 1 + (p_ptr->lev / 20));

			break;
		}

		/* BA_CHAO */
	case 30:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_CHAOS, dir, p_ptr->lev * 4, 2);

			break;
		}

		/* BR_DISI */
	case 31:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_DISINTEGRATE, dir, p_ptr->lev * 5, 1 + (p_ptr->lev / 20));

			break;
		}


		/**** RF5 (bit position + 32) ****/

		/* BA_ACID */
	case 32:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ACID, dir, randint(p_ptr->lev * 6) + 20, 2);

			break;
		}

		/* BA_ELEC */
	case 33:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_ELEC, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_FIRE */
	case 34:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_FIRE, dir, randint(p_ptr->lev * 7) + 20, 2);

			break;
		}

		/* BA_COLD */
	case 35:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_COLD, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_POIS */
	case 36:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_POIS, dir, damroll(12, 2), 2);

			break;
		}

		/* BA_NETH */
	case 37:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_NETHER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

		/* BA_WATE */
	case 38:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_WATER, dir, randint(p_ptr->lev * 4) + 20, 2);

			break;
		}

		/* BA_MANA */
	case 39:
		{
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_MANA, dir, randint(p_ptr->lev * 3) + 20, 2);

			break;
		}

		/* BA_DARK */
	case 40:
		{
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
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8));

			break;
		}

		/* CAUSE_2 */
	case 45:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(8, 8));

			break;
		}

		/* CAUSE_3 */
	case 46:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(10, 15));

			break;
		}

		/* CAUSE_4 */
	case 47:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(15, 15));

			break;
		}

		/* BO_ACID */
	case 48:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ACID, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_ELEC */
	case 49:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ELEC, dir, damroll(4, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_FIRE */
	case 50:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_FIRE, dir, damroll(9, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_COLD */
	case 51:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_COLD, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_POIS */
	case 52:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_POIS, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_NETH */
	case 53:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_NETHER, dir, damroll(5, 5) + (p_ptr->lev / 3));

			break;
		}

		/* BO_WATE */
	case 54:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_WATER, dir, damroll(10, 10) + (p_ptr->lev / 3));

			break;
		}

		/* BO_MANA */
	case 55:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MANA, dir, damroll(3, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_PLAS */
	case 56:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_PLASMA, dir, damroll(8, 8) + (p_ptr->lev / 3));

			break;
		}

		/* BO_ICEE */
	case 57:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_ICE, dir, damroll(6, 6) + (p_ptr->lev / 3));

			break;
		}

		/* MISSILE */
	case 58:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_MISSILE, dir, damroll(2, 6) + (p_ptr->lev / 3));

			break;
		}

		/* SCARE */
	case 59:
		{
			if (!get_aim_dir(&dir)) break;

			fear_monster(dir, plev);

			break;
		}

		/* BLIND */
	case 60:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(1, 8) + (p_ptr->lev / 3));

			break;
		}

		/* CONF */
	case 61:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_CONFUSION, dir, damroll(7, 8) + (p_ptr->lev / 3));

			break;
		}

		/* SLOW */
	case 62:
		{
			if (!get_aim_dir(&dir)) break;

			fire_bolt(GF_OLD_SLOW, dir, damroll(6, 8) + (p_ptr->lev / 3));

			break;
		}

		/* HOLD */
	case 63:
		{
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
			for (k = 0; k < 4; k++)
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
			if (!get_aim_dir(&dir)) break;

			fire_ball(GF_RAISE, dir, 1, 0);

			break;
		}

		/* 77 S_BUG -- Not available, well we do that anyway ;) */

		/* 78 S_RNG -- Not available, who dares? */

		/* S_THUNDERLORD */
	case 79:
		{
			for (k = 0; k < 1; k++)
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

			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_KIN, TRUE);
			}

			break;
		}

		/* S_HI_DEMON */
	case 81:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DEMON, TRUE);
			}

			break;
		}

		/* S_MONSTER */
	case 82:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

		/* S_MONSTERS */
	case 83:
		{
			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, 0, TRUE);
			}

			break;
		}

		/* S_ANT */
	case 84:
		{
			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANT, TRUE);
			}

			break;
		}

		/* S_SPIDER */
	case 85:
		{
			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_SPIDER, TRUE);
			}

			break;
		}

		/* S_HOUND */
	case 86:
		{
			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HOUND, TRUE);
			}

			break;
		}

		/* S_HYDRA */
	case 87:
		{
			for (k = 0; k < 6; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HYDRA, TRUE);
			}

			break;
		}

		/* S_ANGEL */
	case 88:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_ANGEL, TRUE);
			}

			break;
		}

		/* S_DEMON */
	case 89:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DEMON, TRUE);
			}

			break;
		}

		/* S_UNDEAD */
	case 90:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_UNDEAD, TRUE);
			}

			break;
		}

		/* S_DRAGON */
	case 91:
		{
			for (k = 0; k < 1; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_DRAGON, TRUE);
			}

			break;
		}

		/* S_HI_UNDEAD */
	case 92:
		{
			for (k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_UNDEAD_NO_UNIQUES, TRUE);
			}

			break;
		}

		/* S_HI_DRAGON */
	case 93:
		{
			for (k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_HI_DRAGON_NO_UNIQUES, TRUE);
			}

			break;
		}

		/* S_WRAITH */
	case 94:
		{
			for (k = 0; k < 8; k++)
			{
				summon_specific_friendly(y, x, rlev, SUMMON_WRAITH, TRUE);
			}

			break;
		}

		/* 95 S_UNIQUE -- Not available */
	}

	/* Take some SP */
	if (!no_cost)
	{
		int chance, pchance;

		chance = (monster_powers[power].mana + r_ptr->level);
		pchance = adj_str_wgt[p_ptr->stat_ind[A_WIS]] / 2 + get_skill(SKILL_POSSESSION);

		if (rand_int(chance) >= pchance)
		{
			int m = monster_powers[power].mana / 10;

			if (m > p_ptr->msp) m = p_ptr->msp;
			if (!m) m = 1;

			p_ptr->csp -= m;
		}
	}

	/* Redraw mana */
	p_ptr->redraw |= (PR_MANA);

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
static object_type *hack_force_spell_obj = NULL;
bool get_item_hook_find_spell(int *item)
{
	int i, spell;
	char buf[80];
	char buf2[100];

	strcpy(buf, "Manathrust");
	if (!get_string("Spell name? ", buf, 79))
		return FALSE;
	sprintf(buf2, "return find_spell(\"%s\")", buf);
	spell = exec_lua(buf2);
	if (spell == -1) return FALSE;
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];
		u32b f1, f2, f3, f4, f5, esp;

		/* Must we wield it ? */
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
		if ((wield_slot(o_ptr) != -1) && (i < INVEN_WIELD) && (f5 & TR5_WIELD_CAST)) continue;

		/* Is it a non-book? */
		if (!is_school_book(o_ptr))
		{
			u32b f1, f2, f3, f4, f5, esp;

			/* Extract object flags */
			object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

			if ((f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 == spell))
			{
				*item = i;
				hack_force_spell = spell;
				hack_force_spell_obj = o_ptr;
				return TRUE;
			}
		}
		/* A random book ? */
		else if ((o_ptr->sval == 255) && (o_ptr->pval == spell))
		{
			*item = i;
			hack_force_spell = spell;
			hack_force_spell_obj = o_ptr;
			return TRUE;
		}
		/* A normal book */
		else if (o_ptr->sval != 255)
		{
			sprintf(buf2, "return spell_in_book(%d, %d)", o_ptr->sval, spell);
			if (exec_lua(buf2))
			{
				*item = i;
				hack_force_spell = spell;
				hack_force_spell_obj = o_ptr;
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 * Get a spell from a book
 */
u32b get_school_spell(cptr do_what, cptr check_fct, s16b force_book)
{
	int i, item;
	u32b spell = -1;
	int num = 0;
	s32b where = 1;
	int ask;
	bool flag, redraw;
	char choice;
	char out_val[160];
	char buf2[40];
	char buf3[40];
	object_type *o_ptr, forge;
	int tmp;
	int sval, pval;
	u32b f1, f2, f3, f4, f5, esp;

	hack_force_spell = -1;
	hack_force_spell_obj = NULL;

	/* Ok do we need to ask for a book ? */
	if (!force_book)
	{
		get_item_extra_hook = get_item_hook_find_spell;
		item_tester_hook = hook_school_spellable;
		sprintf(buf2, "You have no book to %s from", do_what);
		sprintf(buf3, "%s from which book?", do_what);
		if (!get_item(&item, buf3, buf2, USE_INVEN | USE_EQUIP | USE_EXTRA )) return -1;

		/* Get the item (in the pack) */
		if (item >= 0)
		{
			o_ptr = &p_ptr->inventory[item];
		}

		/* Get the item (on the floor) */
		else
		{
			o_ptr = &o_list[0 - item];
		}

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

	/* No redraw yet */
	redraw = FALSE;

	/* Show choices */
	if (show_choices)
	{
		/* Window stuff */
		window_stuff();
	}

	/* No spell to cast by default */
	spell = -1;

	/* Is it a random book, or something else ? */
	if (is_school_book(o_ptr))
	{
		sval = o_ptr->sval;
		pval = o_ptr->pval;
	}
	else
	{
		sval = 255;
		pval = o_ptr->pval2;
	}

	if (hack_force_spell == -1)
	{
		num = exec_lua(format("return book_spells_num(%d)", sval));

		/* Build a prompt (accept all spells) */
		strnfmt(out_val, 78, "(Spells %c-%c, Descs %c-%c, *=List, ESC=exit) %^s which spell? ",
		        I2A(0), I2A(num - 1), I2A(0) - 'a' + 'A', I2A(num - 1) - 'a' + 'A', do_what);

		/* Get a spell from the user */
		while (!flag && get_com(out_val, &choice))
		{
			/* Request redraw */
			if (((choice == ' ') || (choice == '*') || (choice == '?')))
			{
				/* Show the list */
				if (!redraw)
				{
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					character_icky = TRUE;
					Term_save();

					/* Display a list of spells */
					call_lua("print_book", "(d,d,O)", "d", sval, pval, o_ptr, &where);
				}

				/* Hide the list */
				else
				{
					/* Hide list */
					redraw = FALSE;
					where = 1;

					/* Restore the screen */
					Term_load();
					character_icky = FALSE;
				}

				/* Redo asking */
				continue;
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
				/* Show the list */
				if (!redraw)
				{
					/* Show list */
					redraw = TRUE;

					/* Save the screen */
					character_icky = TRUE;
					Term_load();
					Term_save();

				}
				/* Rstore the screen */
				else
				{
					/* Restore the screen */
					Term_load();
				}

				/* Display a list of spells */
				call_lua("print_book", "(d,d,O)", "d", sval, pval, o_ptr, &where);
				exec_lua(format("print_spell_desc(spell_x(%d, %d, %d), %d)", sval, pval, i, where));
			}
			else
			{
				s32b ok;

				/* Save the spell index */
				spell = exec_lua(format("return spell_x(%d, %d, %d)", sval, pval, i));

				/* Do we need to do some pre test */
				call_lua(check_fct, "(d,O)", "d", spell, o_ptr, &ok);

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
		s32b ok;

		/* Require "okay" spells */
		call_lua(check_fct, "(d, O)", "d", hack_force_spell, hack_force_spell_obj, &ok);
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
	if (redraw)
	{
		Term_load();
		character_icky = FALSE;
	}


	/* Show choices */
	if (show_choices)
	{
		/* Window stuff */
		window_stuff();
	}


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

	spell = get_school_spell("cast", "is_ok_spell", 0);

	/* Actualy cast the choice */
	if (spell != -1)
	{
		exec_lua(format("cast_school_spell(%d, spell(%d))", spell, spell));
	}
}

void browse_school_spell(int book, int pval, object_type *o_ptr)
{
	int i;
	int num = 0, where = 1;
	int ask;
	char choice;
	char out_val[160];

	/* Show choices */
	if (show_choices)
	{
		/* Window stuff */
		window_stuff();
	}

	num = exec_lua(format("return book_spells_num(%d)", book));

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78, "(Spells %c-%c, ESC=exit) cast which spell? ",
	        I2A(0), I2A(num - 1));

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Display a list of spells */
	call_lua("print_book", "(d,d,O)", "d", book, pval, o_ptr, &where);

	/* Get a spell from the user */
	while (get_com(out_val, &choice))
	{
		/* Display a list of spells */
		call_lua("print_book", "(d,d,O)", "d", book, pval, o_ptr, &where);

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
		call_lua("print_book", "(d,d,O)", "d", book, pval, o_ptr, &where);
		exec_lua(format("print_spell_desc(spell_x(%d, %d, %d), %d)", book, pval, i, where));
	}


	/* Restore the screen */
	Term_load();
	character_icky = FALSE;

	/* Show choices */
	if (show_choices)
	{
		/* Window stuff */
		window_stuff();
	}
}

/* Can it contains a schooled spell ? */
static bool hook_school_can_spellable(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	/* Extract object flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if ((f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 == -1))
		return TRUE;
	return FALSE;
}

/*
 * Copy a spell from a bok to an object
 */
void do_cmd_copy_spell()
{
	int spell = get_school_spell("copy", "is_ok_spell", 0);
	int item;
	object_type *o_ptr;

	if (spell == -1) return;

	/* Spells that cannot be randomly created cannot be copied */
	if (exec_lua(format("return can_spell_random(%d)", spell)) == FALSE)
	{
		msg_print("This spell cannot be copied.");
		return;
	}

	item_tester_hook = hook_school_can_spellable;
	if (!get_item(&item, "Copy to which object? ", "You have no object to copy to.", (USE_INVEN | USE_EQUIP))) return;
	o_ptr = get_object(item);

	msg_print("You copy the spell!");
	o_ptr->pval2 = spell;
	inven_item_describe(item);
}

/*
 * Finds a spell by name, optimized for speed
 */
int find_spell(char *name)
{
	int oldtop, spell;
	oldtop = lua_gettop(L);

	lua_getglobal(L, "find_spell");
	tolua_pushstring(L, name);

	/* Call the function */
	if (lua_call(L, 1, 1))
	{
		cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling 'find_spell'.");
		lua_settop(L, oldtop);
		return -1;
	}

	spell = tolua_getnumber(L, -(lua_gettop(L) - oldtop), -1);

	lua_settop(L, oldtop);

	return spell;
}
