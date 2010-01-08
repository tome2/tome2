/* File: powers.c */

/* Purpose: Powers */

/*
 * Copyright (c) 2001 James E. Wilson, Robert A. Koeneke, DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * Note: return value indicates the amount of mana to use
 */
bool power_chance(power_type *x_ptr)
{
	bool use_hp = FALSE;
	int diff = x_ptr->diff;

	/* Always true ? */
	if (!x_ptr->cost) return TRUE;

	/* Not enough mana - use hp */
	if (p_ptr->csp < x_ptr->cost) use_hp = TRUE;

	/* Power is not available yet */
	if (p_ptr->lev < x_ptr->level)
	{
		msg_format("You need to attain level %d to use this power.", x_ptr->level);
		energy_use = 0;
		return (FALSE);
	}

	/* Too confused */
	else if (p_ptr->confused)
	{
		msg_print("You are too confused to use this power.");
		energy_use = 0;
		return (FALSE);
	}

	/* Risk death? */
	else if (use_hp && (p_ptr->chp < x_ptr->cost))
	{
		if (!(get_check("Really use the power in your weakened state? ")))
		{
			energy_use = 0;
			return (FALSE);
		}
	}

	/* Else attempt to do it! */

	if (p_ptr->stun)
	{
		diff += p_ptr->stun;
	}
	else if (p_ptr->lev > x_ptr->level)
	{
		int lev_adj = ((p_ptr->lev - x_ptr->level) / 3);
		if (lev_adj > 10) lev_adj = 10;
		diff -= lev_adj;
	}

	if (diff < 5) diff = 5;

	/* take time and pay the price */
	if (use_hp)
	{
		take_hit(((x_ptr->cost / 2) + (randint(x_ptr->cost / 2))),
		         "concentrating too hard");
	}
	else
	{
		p_ptr->csp -= (x_ptr->cost / 2 ) + (randint(x_ptr->cost / 2));
	}
	energy_use = 100;

	/* Redraw mana and hp */
	p_ptr->redraw |= (PR_HP | PR_MANA);

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Success? */
	if (randint(p_ptr->stat_cur[x_ptr->stat]) >=
	                ((diff / 2) + randint(diff / 2)))
	{
		return (TRUE);
	}

	if (flush_failure) flush();
	msg_print("You've failed to concentrate hard enough.");

	return (FALSE);
}

static void power_activate(int power)
{
	s16b plev = p_ptr->lev;
	char ch = 0;
	int amber_power = 0;
	int dir = 0, dummy;
	object_type *q_ptr;
	object_type forge;
	int ii = 0, ij = 0;
	/* char out_val[80]; */
	/* cptr p = "Power of the flame: "; */
	cptr q, s;
	power_type *x_ptr = &powers_type[power], x_ptr_foo;
	int x, y;
	cave_type *c_ptr;

	/* Break goi/manashield */
	if (p_ptr->invuln)
	{
		set_invuln(0);
	}
	if (p_ptr->disrupt_shield)
	{
		set_disrupt_shield(0);
	}


	if (!power_chance(x_ptr)) return;

	switch (power)
	{
	case PWR_BALROG:
		{
			set_mimic(p_ptr->lev / 2, resolve_mimic_name("Balrog"), p_ptr->lev);
		}
		break;
	case PWR_BEAR:
		{
			set_mimic(150 + (p_ptr->lev * 10) , resolve_mimic_name("Bear"), p_ptr->lev);
		}
		break;
	case PWR_COMPANION:
		{
			if (!can_create_companion())
			{
				msg_print("You cannot have more companions.");
			}
			else
			{
				monster_type *m_ptr;
				int ii, jj;

				msg_print("Select the friendly monster:");
				if (!tgt_pt(&ii, &jj)) return;

				if (cave[jj][ii].m_idx)
				{
					m_ptr = &m_list[cave[jj][ii].m_idx];

					if (m_ptr->status != MSTATUS_PET)
					{
						msg_print("You cannot convert this monster.");
						return;
					}

					m_ptr->status = MSTATUS_COMPANION;
				}
			}
		}
		break;
	case PWR_MERCHANT:
		/* Select power to use */
		while (TRUE)
		{
			if (!get_com("[A]ppraise item, [W]arp item or [I]dentify item? ", &ch))
			{
				amber_power = 0;
				break;
			}

			if (ch == 'A' || ch == 'a')
			{
				amber_power = 1;
				break;
			}

			if (ch == 'W' || ch == 'w')
			{
				amber_power = 2;
				break;
			}

			if (ch == 'I' || ch == 'i')
			{
				amber_power = 3;
				break;
			}
		}

		if (amber_power == 1)
		{
			x_ptr_foo.level = 5;
			x_ptr_foo.cost = 5;
			x_ptr_foo.stat = A_INT;
			x_ptr_foo.diff = 5;
			if (power_chance(&x_ptr_foo))
			{
				/* Appraise an object */
				int idx;
				cptr q, s;

				/* Get the item */
				q = "Appraise which item? ";
				s = "You have nothing to appraise.";
				if (get_item(&idx, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
				{
					object_type *o_ptr;
					char out_val[80], value[16];

					/* The item is in the pack */
					if (idx >= 0) o_ptr = &p_ptr->inventory[idx];
					/* The item is on the floor */
					else o_ptr = &o_list[0 - idx];

					/* Appraise it */
					sprintf(value, "%i au", (int)object_value(o_ptr));

					/* Inscribe the value */
					/* Get the original inscription */
					if (o_ptr->note)
					{
						strcpy(out_val, quark_str(o_ptr->note));
						strcat(out_val, " ");
					}
					else
						out_val[0] = '\0';

					strcat(out_val, value);

					/* Save the new inscription */
					o_ptr->note = quark_add(out_val);

					/* Combine the pack */
					p_ptr->notice |= (PN_COMBINE);

					/* Window stuff */
					p_ptr->window |= (PW_INVEN | PW_EQUIP);
				}
			}
		}
		if (amber_power == 2)
		{
			x_ptr_foo.level = 15;
			x_ptr_foo.cost = 10;
			x_ptr_foo.stat = A_INT;
			x_ptr_foo.diff = 7;
			if (power_chance(&x_ptr_foo))
			{
				int chest, item;
				cptr q1, s1, q2, s2;
				u32b flag = (USE_EQUIP | USE_INVEN | USE_FLOOR);
				object_type *o1_ptr = &p_ptr->inventory[0], *o2_ptr = &p_ptr->inventory[1];
				int ok = 0;

				q1 = "Select a chest! ";
				s1 = "You need a chest to warp items.";

				q2 = "Warp which item? ";
				s2 = "You have nothing to warp.";

				item_tester_tval = TV_CHEST;

				/* Get the chest */
				if (get_item(&chest, q1, s1, flag))
				{
					if (chest >= 0) o1_ptr = &p_ptr->inventory[chest];
					else o1_ptr = &o_list[0 - chest];

					/* Is the chest disarmed? */
					if (o1_ptr->pval > 0)
						msg_print("This chest may be trapped.");

					/* Is it ruined? */
					else if (k_info[o1_ptr->k_idx].level <= 0)
						msg_print("This chest is broken.");

					/* Is it empty? */
					else if (o1_ptr->pval2 >= (o1_ptr->sval % SV_CHEST_MIN_LARGE) * 2)
						msg_print("This chest is full.");

					else ok = 1;
				}

				/* Get the item */
				if (ok && get_item(&item, q2, s2, flag))
				{
					ok = 0;

					if (item >= 0) o2_ptr = &p_ptr->inventory[item];
					else o2_ptr = &o_list[0 - item];

					/* Is the item cursed? */
					if ((item >= INVEN_WIELD) && cursed_p(o2_ptr))
						msg_print("Hmmm, it seems to be cursed.");

					/* Is it the same chest? */
					if (item == chest)
						msg_print("You can't put a chest into itself.");

					/* Is it another chest? */
					if (o2_ptr->tval == TV_CHEST)
						msg_print("You can't put a chest into another one.");

					/* Try to use the power */
					else ok = 1;
				}

				if (ok)
				{
					int tmp, level;

					/* Calculate the level of objects */
					tmp = o1_ptr->pval;

					/* Get the level of the current object */
					/* Cursed items/cheap items always break */
					if (k_info[o2_ptr->k_idx].cost < 20) level = 0;
					/* Not-so-cheap items break 90% of the time */
					else if (k_info[o2_ptr->k_idx].cost < 100) level = 1;
					else level = k_info[o2_ptr->k_idx].level;

					/* Break some items */
					if (randint(10) > level)
						msg_print("The item disappeared!");
					else
					{
						level /= (o1_ptr->sval % SV_CHEST_MIN_LARGE) * 2;

						/* Increase the number of objects in
						 * the chest */
						o1_ptr->pval2++;

						/* Set the level of chest */
						tmp = tmp - level;
						o1_ptr->pval = tmp;
					}

					/* Destroy item */
					if (item >= 0)
					{
						inven_item_increase(item, -1);
						inven_item_describe(item);
						inven_item_optimize(item);
					}
					else
					{
						inven_item_increase(0 - item, -1);
						inven_item_describe(0 - item);
						inven_item_optimize(0 - item);
					}
				}
			}
		}
		if (amber_power == 3)
		{
			x_ptr_foo.level = 30;
			x_ptr_foo.cost = 20;
			x_ptr_foo.stat = A_INT;
			x_ptr_foo.diff = 7;
			if (power_chance(&x_ptr_foo))
			{
				ident_spell();
			}
		}
		break;
	case PWR_LAY_TRAP:
		{
			do_cmd_set_trap();
		}
		break;
	case PWR_MAGIC_MAP:
		{
			msg_print("You sense the world around you.");
			map_area();
		}
		break;

	case PWR_PASSWALL:
		{
			if (!get_aim_dir(&dir))
				break;
			if (passwall(dir, TRUE))
				msg_print("A passage opens, and you step through.");
			else
				msg_print("There is no wall there!");
		}
		break;

	case PWR_COOK_FOOD:
		{
			/* Get local object */
			q_ptr = &forge;

			/* Create the item */
			object_prep(q_ptr, 21);

			/* Drop the object from heaven */
			drop_near(q_ptr, -1, p_ptr->py, p_ptr->px);
			msg_print("You cook some food.");
		}
		break;

	case PWR_UNFEAR:
		{
			msg_print("You play tough.");
			(void)set_afraid(0);
		}
		break;

	case PWR_BERSERK:
		{
			msg_print("RAAAGH!");
			(void)set_afraid(0);

			(void)set_shero(p_ptr->shero + 10 + randint(plev));
			(void)hp_player(30);
		}
		break;

	case PWR_EXPL_RUNE:
		{
			msg_print("You carefully set an explosive rune...");
			explosive_rune();
		}
		break;

	case PWR_STM:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You bash at a stone wall.");
			(void)wall_to_mud(dir);
		}
		break;

	case PWR_ROHAN:
		/* Select power to use */
		while (TRUE)
		{
			if (!get_com("Use [F]lash aura or [L]ight speed jump? ", &ch))
			{
				amber_power = 0;
				break;
			}

			if (ch == 'F' || ch == 'f')
			{
				amber_power = 1;
				break;
			}

			if (ch == 'L' || ch == 'l')
			{
				amber_power = 2;
				break;
			}
		}

		if (amber_power == 1)
		{
			x_ptr_foo.level = 1;
			x_ptr_foo.cost = 9;
			x_ptr_foo.stat = A_CHR;
			x_ptr_foo.diff = 7;
			if (power_chance(&x_ptr_foo))
			{
				if (!(get_aim_dir(&dir))) break;
				msg_print("You flash a bright aura.");
				if (p_ptr->lev < 10)
					fire_bolt(GF_CONFUSION, dir, plev*2);
				else
					fire_ball(GF_CONFUSION, dir, plev*2, 2);
			}
		}
		if (amber_power == 2)
		{
			x_ptr_foo.level = 30;
			x_ptr_foo.cost = 30;
			x_ptr_foo.stat = A_WIS;
			x_ptr_foo.diff = 7;
			if (power_chance(&x_ptr_foo))
			{
				(void)set_light_speed(p_ptr->lightspeed + 3);
			}
		}
		break;


	case PWR_POIS_DART:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You throw a dart of poison.");
			fire_bolt(GF_POIS, dir, plev);
		}
		break;

	case PWR_DETECT_TD:
		{
			msg_print("You examine your surroundings.");
			(void)detect_traps(DEFAULT_RADIUS);
			(void)detect_doors(DEFAULT_RADIUS);
			(void)detect_stairs(DEFAULT_RADIUS);
		}
		break;

	case PWR_MAGIC_MISSILE:
		{
			if (!get_aim_dir(&dir)) break;
			msg_print("You cast a magic missile.");
			fire_bolt_or_beam(10, GF_MISSILE, dir,
			                  damroll(3 + ((plev - 1) / 5), 4));
		}
		break;

	case PWR_THUNDER:
		/* Select power to use */
		while (TRUE)
		{
			if (!get_com("Use [T]hunder strike, [R]ide the straight road, go [B]ack in town? ", &ch))
			{
				amber_power = 0;
				break;
			}

			if (ch == 'T' || ch == 't')
			{
				amber_power = 1;
				break;
			}

			if (ch == 'R' || ch == 'r')
			{
				amber_power = 2;
				break;
			}

			if (ch == 'B' || ch == 'b')
			{
				amber_power = 3;
				break;
			}

		}

		if (amber_power == 1)
		{
			x_ptr_foo.level = 1;
			x_ptr_foo.cost = p_ptr->lev;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(&x_ptr_foo))
			{
				if (!get_aim_dir(&dir)) break;
				msg_format("You conjure up thunder!");
				fire_beam(GF_ELEC, dir, p_ptr->lev * 2);
				fire_beam(GF_SOUND, dir, p_ptr->lev * 2);
				p_ptr->energy -= 100;
			}
		}
		if (amber_power == 2)
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No teleport on special levels ...");
				break;
			}
			x_ptr_foo.level = 3;
			x_ptr_foo.cost = 15;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(&x_ptr_foo))
			{
				msg_print("You enter the straight road and fly beside the world. Where to exit?");
				if (!tgt_pt(&ii, &ij)) return;
				p_ptr->energy -= 60 - plev;
				if (!cave_empty_bold(ij, ii) || (cave[ij][ii].info & CAVE_ICKY) ||
				                (distance(ij, ii, p_ptr->py, p_ptr->px) > plev*20 + 2) || !(cave[ij][ii].info & CAVE_MARK))
				{
					msg_print("You fail to exit correctly!");
					p_ptr->energy -= 100;
					teleport_player(10);
				}
				else teleport_player_to(ij, ii);
			}

		}
		if (amber_power == 3)
		{
			if (dungeon_flags2 & DF2_NO_TELEPORT)
			{
				msg_print("No recall on special levels..");
				break;
			}
			x_ptr_foo.level = 7;
			x_ptr_foo.cost = 30;
			x_ptr_foo.stat = A_CON;
			x_ptr_foo.diff = 6;
			if (power_chance(&x_ptr_foo))
			{
				if (dun_level == 0)
					msg_print("You are already in town!");
				else
				{
					msg_print("You enter the straight road and fly beside the world.");
					p_ptr->energy -= 100;
					p_ptr->word_recall = 1;
				}
			}
		}
		break;

	case PWR_GROW_TREE:
		{
			msg_print("You make the trees grow!");
			grow_trees((plev / 8 < 1) ? 1 : plev / 8);
		}
		break;

	case PWR_DEATHMOLD:
		do_cmd_immovable_special();
		break;

	case PWR_BR_COLD:
		{
			msg_print("You breathe cold...");
			if (get_aim_dir(&dir))
				fire_ball(GF_COLD, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_BR_CHAOS:
		{
			msg_print("You breathe chaos...");
			if (get_aim_dir(&dir))
				fire_ball(GF_CHAOS, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_BR_ELEM:
		{
			if (!get_aim_dir(&dir)) break;
			msg_format("You breathe the elements.");
			fire_ball(GF_MISSILE, dir, (p_ptr->lev)*2,
			          ((p_ptr->lev) / 15) + 1);
		}
		break;

	case PWR_SUMMON_MONSTER:
		{
			do_cmd_beastmaster();
		}
		break;

	case PWR_WRECK_WORLD:
		msg_print("The power of Eru Iluvatar flows through you!");
		msg_print("The world changes!");
		if (autosave_l)
		{
			is_autosave = TRUE;
			msg_print("Autosaving the game...");
			do_cmd_save_game();
			is_autosave = FALSE;
		}
		/* Leaving */
		p_ptr->leaving = TRUE;
		break;

	case PWR_VAMPIRISM:
		{
			/* Only works on adjacent monsters */
			if (!get_rep_dir(&dir)) break;    /* was get_aim_dir */
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];

			if (!(c_ptr->m_idx))
			{
				msg_print("You bite into thin air!");
				break;
			}

			msg_print("You grin and bare your fangs...");
			dummy = plev + randint(plev) * MAX(1, plev / 10);    /* Dmg */
			if (drain_life(dir, dummy))
			{
				if (p_ptr->food < PY_FOOD_FULL)
					/* No heal if we are "full" */
					(void)hp_player(dummy);
				else
					msg_print("You were not hungry.");
				/* Gain nutritional sustenance: 150/hp drained */
				/* A Food ration gives 5000 food points (by contrast) */
				/* Don't ever get more than "Full" this way */
				/* But if we ARE Gorged,  it won't cure us */
				dummy = p_ptr->food + MIN(5000, 100 * dummy);
				if (p_ptr->food < PY_FOOD_MAX)   /* Not gorged already */
					(void)set_food(dummy >= PY_FOOD_MAX ? PY_FOOD_MAX - 1 : dummy);
			}
			else
				msg_print("Yechh. That tastes foul.");
		}
		break;

	case PWR_SCARE:
		{
			msg_print("You emit an eldritch howl!");
			if (!get_aim_dir(&dir)) break;
			(void)fear_monster(dir, plev);
		}
		break;

	case PWR_REST_LIFE:
		{
			msg_print("You attempt to restore your lost energies.");
			(void)restore_level();
		}
		break;

	case PWR_HYPNO:
		{
			int dir, x, y;
			cave_type *c_ptr;
			monster_type *m_ptr;
			monster_race *r_ptr;
			object_type *q_ptr;
			object_type forge;

			msg_print("Hypnotize which pet?");
			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (c_ptr->m_idx)
			{
				m_ptr = &m_list[c_ptr->m_idx];
				r_ptr = race_inf(m_ptr);

				if ((r_ptr->flags1 & RF1_NEVER_MOVE) && (m_ptr->status == MSTATUS_PET) && (!(r_ptr->flags9 & RF9_SPECIAL_GENE)))
				{
					q_ptr = &forge;
					object_prep(q_ptr, lookup_kind(TV_HYPNOS, 1));
					q_ptr->number = 1;
					q_ptr->pval = m_ptr->r_idx;
					q_ptr->pval2 = m_ptr->hp;
					object_aware(q_ptr);
					object_known(q_ptr);

					q_ptr->ident |= IDENT_STOREB;

					drop_near(q_ptr, 0, y, x);

					delete_monster(y, x);
					health_who = 0;
				}
				else
					msg_print("You can only hypnotize monsters that can't move.");
			}
			else msg_print("There is no pet here!");
		}
		break;

	case PWR_UNHYPNO:
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
				scatter(&y, &x, p_ptr->py, p_ptr->px, d, 0);

				if (cave_floor_bold(y, x) && (!cave[y][x].m_idx)) break;

				d++;
			}

			if (d >= 100) return;

			if ((m_idx = place_monster_one(y, x, o_ptr->pval, 0, FALSE, MSTATUS_PET)) == 0) return;

			m_ptr = &m_list[m_idx];
			m_ptr->hp = o_ptr->pval2;

			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
		break;

	case PWR_NECRO:
		{
			do_cmd_necromancer();
			break;
		}
	case PWR_INCARNATE:
		{
			do_cmd_integrate_body();
			break;
		}

	case PWR_SPIT_ACID:
		{
			msg_print("You spit acid...");
			if (get_aim_dir(&dir))
				fire_ball(GF_ACID, dir, p_ptr->lev, 1 + (p_ptr->lev / 30));
		}
		break;

	case PWR_BR_FIRE:
		{
			msg_print("You breathe fire...");
			if (get_aim_dir(&dir))
				fire_ball(GF_FIRE, dir, p_ptr->lev * 2, 1 + (p_ptr->lev / 20));
		}
		break;

	case PWR_HYPN_GAZE:
		{
			msg_print("Your eyes look mesmerising...");
			if (get_aim_dir(&dir))
				(void) charm_monster(dir, p_ptr->lev);
		}
		break;

	case PWR_TELEKINES:
		{
			msg_print("You concentrate...");
			if (get_aim_dir(&dir))
				fetch(dir, p_ptr->lev * 10, TRUE);
		}
		break;

	case PWR_VTELEPORT:
		{
			msg_print("You concentrate...");
			teleport_player(10 + 4*(p_ptr->lev));
		}
		break;

	case PWR_MIND_BLST:
		{
			msg_print("You concentrate...");
			if (!get_aim_dir(&dir)) return;
			fire_bolt(GF_PSI, dir, damroll(3 + ((p_ptr->lev - 1) / 5), 3));
		}
		break;

	case PWR_RADIATION:
		{
			msg_print("Radiation flows from your body!");
			fire_ball(GF_NUKE, 0, (p_ptr->lev * 2), 3 + (p_ptr->lev / 20));
		}
		break;

	case PWR_SMELL_MET:
		{
			(void)detect_treasure(DEFAULT_RADIUS);
		}
		break;

	case PWR_SMELL_MON:
		{
			(void)detect_monsters_normal(DEFAULT_RADIUS);
		}
		break;

	case PWR_BLINK:
		{
			teleport_player(10);
		}
		break;

	case PWR_EAT_ROCK:
		{
			int x, y, ox, oy;
			cave_type *c_ptr;

			if (!get_rep_dir(&dir)) break;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (cave_floor_bold(y, x))
			{
				msg_print("You bite into thin air!");
				break;
			}
			else if ((f_info[c_ptr->feat].flags1 & FF1_PERMANENT) || (c_ptr->feat == FEAT_MOUNTAIN))
			{
				msg_print("Ouch!  This wall is harder than your teeth!");
				break;
			}
			else if (c_ptr->m_idx)
			{
				msg_print("There's something in the way!");
				break;
			}
			else if (c_ptr->feat == FEAT_TREES)
			{
				msg_print("You don't like the woody taste!");
				break;
			}
			else
			{
				if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
				                (c_ptr->feat <= FEAT_RUBBLE))
				{
					(void)set_food(p_ptr->food + 3000);
				}
				else if ((c_ptr->feat >= FEAT_MAGMA) &&
				                (c_ptr->feat <= FEAT_QUARTZ_K))
				{
					(void)set_food(p_ptr->food + 5000);
				}
				else if ((c_ptr->feat >= FEAT_SANDWALL) &&
				                (c_ptr->feat <= FEAT_SANDWALL_K))
				{
					(void)set_food(p_ptr->food + 500);
				}
				else
				{
					msg_print("This granite is very filling!");
					(void)set_food(p_ptr->food + 10000);
				}
			}
			(void)wall_to_mud(dir);

			oy = p_ptr->py;
			ox = p_ptr->px;

			p_ptr->py = y;
			p_ptr->px = x;

			lite_spot(p_ptr->py, p_ptr->px);
			lite_spot(oy, ox);

			verify_panel();

			p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);
			p_ptr->update |= (PU_DISTANCE);
			p_ptr->window |= (PW_OVERHEAD);
		}
		break;

	case PWR_SWAP_POS:
		{
			if (!get_aim_dir(&dir)) return;
			(void)teleport_swap(dir);
		}
		break;

	case PWR_SHRIEK:
		{
			(void)fire_ball(GF_SOUND, 0, 4 * p_ptr->lev, 8);
			(void)aggravate_monsters(0);
		}
		break;

	case PWR_ILLUMINE:
		{
			(void)lite_area(damroll(2, (p_ptr->lev / 2)), (p_ptr->lev / 10) + 1);
		}
		break;

	case PWR_DET_CURSE:
		{
			int i;

			for (i = 0; i < INVEN_TOTAL; i++)
			{
				object_type *o_ptr = &p_ptr->inventory[i];

				if (!o_ptr->k_idx) continue;
				if (!cursed_p(o_ptr)) continue;

				if (!o_ptr->sense) o_ptr->sense = SENSE_CURSED;
			}
		}
		break;

	case PWR_POLYMORPH:
		{
			do_poly_self();
		}
		break;

	case PWR_MIDAS_TCH:
		{
			(void)alchemy();
		}
		break;

	case PWR_GROW_MOLD:
		{
			int i;
			for (i = 0; i < 8; i++)
			{
				summon_specific_friendly(p_ptr->py, p_ptr->px, p_ptr->lev, SUMMON_BIZARRE1, FALSE);
			}
		}
		break;

	case PWR_RESIST:
		{
			int num = p_ptr->lev / 10;
			int dur = randint(20) + 20;

			if (rand_int(5) < num)
			{
				(void)set_oppose_acid(p_ptr->oppose_acid + dur);
				num--;
			}
			if (rand_int(4) < num)
			{
				(void)set_oppose_elec(p_ptr->oppose_elec + dur);
				num--;
			}
			if (rand_int(3) < num)
			{
				(void)set_oppose_fire(p_ptr->oppose_fire + dur);
				num--;
			}
			if (rand_int(2) < num)
			{
				(void)set_oppose_cold(p_ptr->oppose_cold + dur);
				num--;
			}
			if (num)
			{
				(void)set_oppose_pois(p_ptr->oppose_pois + dur);
				num--;
			}
		}
		break;

	case PWR_EARTHQUAKE:
		{
			/* Prevent destruction of quest levels and town */
			if (!is_quest(dun_level) && dun_level)
				earthquake(p_ptr->py, p_ptr->px, 10);
		}
		break;

	case PWR_EAT_MAGIC:
		{
			object_type * o_ptr;
			int lev, item;

			item_tester_hook = item_tester_hook_recharge;

			/* Get an item */
			q = "Drain which item? ";
			s = "You have nothing to drain.";
			if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) break;

			if (item >= 0)
			{
				o_ptr = &p_ptr->inventory[item];
			}
			else
			{
				o_ptr = &o_list[0 - item];
			}

			lev = k_info[o_ptr->k_idx].level;

			if (o_ptr->tval == TV_ROD_MAIN)
			{
				if (!o_ptr->timeout)
				{
					msg_print("You can't absorb energy from a discharged rod.");
				}
				else
				{
					p_ptr->csp += o_ptr->timeout;
					o_ptr->timeout = 0;
				}
			}
			else
			{
				if (o_ptr->pval > 0)
				{
					p_ptr->csp += o_ptr->pval * lev;
					o_ptr->pval = 0;
				}
				else
				{
					msg_print("There's no energy there to absorb!");
				}
				o_ptr->ident |= IDENT_EMPTY;
			}

			if (p_ptr->csp > p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
			}

			p_ptr->notice |= (PN_COMBINE | PN_REORDER);
			p_ptr->window |= (PW_INVEN);
		}
		break;

	case PWR_WEIGH_MAG:
		{
			report_magics();
		}
		break;

	case PWR_STERILITY:
		{
			/* Fake a population explosion. */
			msg_print("You suddenly have a headache!");
			take_hit(randint(30) + 30, "the strain of forcing abstinence");
			num_repro += MAX_REPRO;
		}
		break;

	case PWR_PANIC_HIT:
		{
			int x, y;

			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			if (cave[y][x].m_idx)
			{
				py_attack(y, x, -1);
				teleport_player(30);
			}
			else
			{
				msg_print("You don't see any monster in this direction.");
				msg_print(NULL);
			}
		}
		break;

	case PWR_DAZZLE:
		{
			stun_monsters(p_ptr->lev * 4);
			confuse_monsters(p_ptr->lev * 4);
			turn_monsters(p_ptr->lev * 4);
		}
		break;

	case PWR_DARKRAY:
		{
			if (!get_aim_dir(&dir)) return;
			fire_beam(GF_LITE, dir, 2*p_ptr->lev);
		}
		break;

	case PWR_RECALL:
		{
			if (!(dungeon_flags2 & DF2_ASK_LEAVE) || ((dungeon_flags2 & DF2_ASK_LEAVE) && !get_check("Leave this unique level forever? ")))
			{
				recall_player(21, 15);
			}
		}
		break;

	case PWR_BANISH:
		{
			int x, y;
			cave_type *c_ptr;
			monster_type *m_ptr;
			monster_race *r_ptr;

			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (!(c_ptr->m_idx))
			{
				msg_print("You sense no evil there!");
				break;
			}

			m_ptr = &m_list[c_ptr->m_idx];
			r_ptr = race_inf(m_ptr);

			if (r_ptr->flags3 & RF3_EVIL)
			{
				/* Delete the monster, rather than killing it. */
				delete_monster_idx(c_ptr->m_idx);
				msg_print("The evil creature vanishes in a puff of sulphurous smoke!");
			}
			else
			{
				msg_print("Your invocation is ineffectual!");
			}
		}
		break;

	case PWR_COLD_TOUCH:
		{
			int x, y;
			cave_type *c_ptr;

			if (!get_rep_dir(&dir)) return;
			y = p_ptr->py + ddy[dir];
			x = p_ptr->px + ddx[dir];
			c_ptr = &cave[y][x];
			if (!(c_ptr->m_idx))
			{
				msg_print("You wave your hands in the air.");
				break;
			}
			fire_bolt(GF_COLD, dir, 2 * (p_ptr->lev));
		}
		break;

	case PWR_LAUNCHER:
		{
			/* Gives a multiplier of 2 at first, up to 5 at 48th */
			p_ptr->throw_mult = 2 + (p_ptr->lev / 16);
			do_cmd_throw();
			p_ptr->throw_mult = 1;
		}
		break;

	case PWR_DODGE:
		use_ability_blade();
		break;

	default:
		if (!process_hooks(HOOK_ACTIVATE_POWER, "(d)", power))
		{
			msg_format("Warning power_activate() called with invalid power(%d).", power);
			energy_use = 0;
		}
		break;
	}

	p_ptr->redraw |= (PR_HP | PR_MANA);
	p_ptr->window |= (PW_PLAYER);
}

/*
 * Print a batch of power.
 */
static void print_power_batch(int *p, int start, int max, bool mode)
{
	char buff[80];
	power_type* spell;
	int i = start, j = 0;

	if (mode) prt(format("         %-31s Level Mana Fail", "Name"), 1, 20);

	for (i = start; i < (start + 20); i++)
	{
		if (i >= max) break;

		spell = &powers_type[p[i]];

		sprintf(buff, "  %c-%3d) %-30s  %5d %4d %s@%d", I2A(j), p[i] + 1, spell->name,
		        spell->level, spell->cost, stat_names[spell->stat], spell->diff);

		if (mode) prt(buff, 2 + j, 20);
		j++;
	}
	if (mode) prt("", 2 + j, 20);
	prt(format("Select a power (a-%c), +/- to scroll:", I2A(j - 1)), 0, 0);
}


/*
 * List powers and ask to pick one. 
 */

static power_type* select_power(int *x_idx)
{
	char which;
	int max = 0, i, start = 0;
	power_type* ret;
	bool mode = FALSE;
	int *p;

	C_MAKE(p, power_max, int);
	/* Count the max */
	for (i = 0; i < power_max; i++)
	{
		if (p_ptr->powers[i])
		{
			p[max++] = i;
		}
	}

	/* Exit if there aren't powers */
	if (max == 0)
	{
		*x_idx = -1;
		ret = NULL;
		msg_print("You don't have any special powers.");
	}
	else
	{
		character_icky = TRUE;
		Term_save();

		while (1)
		{
			print_power_batch(p, start, max, mode);
			which = inkey();

			if (which == ESCAPE)
			{
				*x_idx = -1;
				ret = NULL;
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
				if (start >= max) start -= 20;
				Term_load();
				character_icky = FALSE;
			}
			else if (which == '-')
			{
				start -= 20;
				if (start < 0) start += 20;
				Term_load();
				character_icky = FALSE;
			}
			else
			{
				which = tolower(which);
				if (start + A2I(which) >= max)
				{
					bell();
					continue;
				}
				if (start + A2I(which) < 0)
				{
					bell();
					continue;
				}

				*x_idx = p[start + A2I(which)];
				ret = &powers_type[p[start + A2I(which)]];
				break;
			}
		}
		Term_load();
		character_icky = FALSE;
	}

	C_FREE(p, power_max, int);

	return ret;
}

/* Ask & execute a power */
void do_cmd_power()
{
	int x_idx;
	power_type *x_ptr;
	bool push = TRUE;

	/* Get the skill, if available */
	if (repeat_pull(&x_idx))
	{
		if ((x_idx < 0) || (x_idx >= power_max)) return;
		x_ptr = &powers_type[x_idx];
		push = FALSE;
	}
	else if (!command_arg) x_ptr = select_power(&x_idx);
	else
	{
		x_idx = command_arg - 1;
		if ((x_idx < 0) || (x_idx >= power_max)) return;
		x_ptr = &powers_type[x_idx];
	}

	if (x_ptr == NULL) return;

	if (push) repeat_push(x_idx);

	if (p_ptr->powers[x_idx])
		power_activate(x_idx);
	else
		msg_print("You do not have access to this power.");
}
