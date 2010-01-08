/* File: traps.c */

/* Purpose: handle traps */

/* the below copyright probably still applies, but it is heavily changed
 * copied, adapted & re-engineered by JK.
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

bool do_player_trap_call_out(void)
{
	s16b i, sn, cx, cy;
	s16b h_index = 0;
	s16b h_level = 0;
	monster_type *m_ptr;
	monster_race *r_ptr;
	char m_name[80];
	bool ident = FALSE;

	for (i = 1; i < m_max; i++)
	{
		m_ptr = &m_list[i];
		r_ptr = race_inf(m_ptr);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->level >= h_level)
		{
			h_level = m_ptr->level;
			h_index = i;
		}
	}

	/* if the level is empty of monsters, h_index will be 0 */
	if (!h_index) return (FALSE);

	m_ptr = &m_list[h_index];

	sn = 0;
	for (i = 0; i < 8; i++)
	{
		cx = p_ptr->px + ddx[i];
		cy = p_ptr->py + ddy[i];

		/* Skip non-empty grids */
		if (!cave_valid_bold(cy, cx)) continue;
		if (cave[cy][cx].feat == FEAT_GLYPH) continue;
		if ((cx == p_ptr->px) && (cy == p_ptr->py)) continue;
		sn++;

		/* Randomize choice */
		if (rand_int(sn) > 0) continue;
		cave[cy][cx].m_idx = h_index;
		cave[m_ptr->fy][m_ptr->fx].m_idx = 0;
		m_ptr->fx = cx;
		m_ptr->fy = cy;

		/* we do not change the sublevel! */
		ident = TRUE;
		update_mon(h_index, TRUE);
		monster_desc(m_name, m_ptr, 0x08);
		msg_format("You hear a rapid-shifting wail, and %s appears!", m_name);
		break;
	}

	return (ident);
}

static bool do_trap_teleport_away(object_type *i_ptr, s16b y, s16b x)
{
	bool ident = FALSE;
	char o_name[80];

	s16b o_idx = 0;
	object_type *o_ptr;
	cave_type *c_ptr;

	s16b x1;
	s16b y1;

	if (i_ptr == NULL) return (FALSE);

	if (i_ptr->name1 == ART_POWER) return (FALSE);

	while (o_idx == 0)
	{
		x1 = rand_int(cur_wid);
		y1 = rand_int(cur_hgt);

		/* Obtain grid */
		c_ptr = &cave[y1][x1];

		/* Require floor space (or shallow terrain) -KMW- */
		if (!(f_info[c_ptr->feat].flags1 & FF1_FLOOR)) continue;

		o_idx = drop_near(i_ptr, 0, y1, x1);
	}

	o_ptr = &o_list[o_idx];

	x1 = o_ptr->ix;
	y1 = o_ptr->iy;

	if (!p_ptr->blind)
	{
		note_spot(y, x);
		lite_spot(y, x);
		ident = TRUE;
		object_desc(o_name, i_ptr, FALSE, 0);
		if (player_has_los_bold(y1, x1))
		{
			lite_spot(y1, x1);
			msg_format("The %s suddenly stands elsewhere.", o_name);

		}
		else
		{
			msg_format("You suddenly don't see the %s any more!", o_name);
		}
	}
	else
	{
		msg_print("You hear something move.");
	}
	return (ident);
}

/*
 * this handles a trap that places walls around the player
 */
static bool player_handle_trap_of_walls(void)
{
	bool ident;

	s16b dx, dy, cx, cy;
	s16b sx = 0, sy = 0, sn, i;
	cave_type *cv_ptr;
	bool map[5][5] =
	        {
	                {FALSE, FALSE, FALSE, FALSE, FALSE},
	                {FALSE, FALSE, FALSE, FALSE, FALSE},
	                {FALSE, FALSE, FALSE, FALSE, FALSE},
	                {FALSE, FALSE, FALSE, FALSE, FALSE},
	                {FALSE, FALSE, FALSE, FALSE, FALSE}
	        };

	for (dy = -2; dy <= 2; dy++)
		for (dx = -2; dx <= 2; dx++)
		{
			/* Extract the location */
			cx = p_ptr->px + dx;
			cy = p_ptr->py + dy;

			if (!in_bounds(cy, cx)) continue;

			cv_ptr = &cave[cy][cx];

			if (cv_ptr->m_idx) continue;

			/* Lose room and vault */
			cv_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
			/* Lose light and knowledge */
			cv_ptr->info &= ~(CAVE_GLOW | CAVE_MARK);

			/* Skip the center */
			if (!dx && !dy) continue;

			/* test for dungeon level */
			if (randint(100) > 10 + max_dlv[dungeon_type]) continue;

			/* Damage this grid */
			map[2 + dx][2 + dy] = TRUE;
		}

	for (dy = -2; dy <= 2; dy++)
		for (dx = -2; dx <= 2; dx++)
		{
			/* Extract the location */
			cx = p_ptr->px + dx;
			cy = p_ptr->py + dy;

			/* Skip unaffected grids */
			if (!map[2 + dx][2 + dy]) continue;

			cv_ptr = &cave[cy][cx];

			if (cv_ptr->m_idx)
			{
				monster_type *m_ptr = &m_list[cv_ptr->m_idx];
				monster_race *r_ptr = race_inf(m_ptr);

				/* Most monsters cannot co-exist with rock */
				if ((!(r_ptr->flags2 & RF2_KILL_WALL)) &&
				                (!(r_ptr->flags2 & RF2_PASS_WALL)))
				{
					char m_name[80];

					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Access the grid */
							cy = p_ptr->py + ddy[i];
							cx = p_ptr->px + ddx[i];

							/* Skip non-empty grids */
							if (!cave_clean_bold(cy, cx)) continue;

							/* Hack -- no safety on glyph of warding */
							if (cave[cy][cx].feat == FEAT_GLYPH) continue;

							/* Important -- Skip "quake" grids */
							if (map[2 + (cx - p_ptr->px)][2 + (cy - p_ptr->py)]) continue;

							/* Count "safe" grids */
							sn++;

							/* Randomize choice */
							if (rand_int(sn) > 0) continue;

							/* Save the safe grid */
							sx = cx;
							sy = cy;

							ident = TRUE;

							break;  /* discontinue for loop - safe grid found */
						}
					}

					/* Describe the monster */
					monster_desc(m_name, m_ptr, 0);

					/* Scream in pain */
					msg_format("%^s wails out in pain!", m_name);

					/* Monster is certainly awake */
					m_ptr->csleep = 0;

					/* Apply damage directly */
					m_ptr->hp -= (sn ? damroll(4, 8) : 200);

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Message */
						msg_format("%^s is entombed in the rock!", m_name);

						/* Delete the monster */
						delete_monster_idx(cave[cy][cx].m_idx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						s16b m_idx = cave[cy][cx].m_idx;

						/* Update the new location */
						cave[sy][sx].m_idx = m_idx;

						/* Update the old location */
						cave[cy][cx].m_idx = 0;

						/* Move the monster */
						m_ptr->fy = sy;
						m_ptr->fx = sx;

						/* do not change fz */
						/* don't make rock on that square! */
						if ((sx >= (p_ptr->px - 2)) && (sx <= (p_ptr->px + 2)) &&
						                (sy >= (p_ptr->py - 2)) && (sy <= (p_ptr->py + 2)))
						{
							map[2 + (sx - p_ptr->px)][2 + (sy - p_ptr->py)] = FALSE;
						}

						/* Update the monster (new location) */
						update_mon(m_idx, TRUE);

						/* Redraw the old grid */
						lite_spot(cy, cx);

						/* Redraw the new grid */
						lite_spot(sy, sx);
					} /* if sn */
				} /* if monster can co-exist with rock */
			} /* if monster on square */
		}

	/* Examine the quaked region */
	for (dy = -2; dy <= 2; dy++)
		for (dx = -2; dx <= 2; dx++)
		{
			/* Extract the location */
			cx = p_ptr->px + dx;
			cy = p_ptr->py + dy;

			/* Skip unaffected grids */
			if (!map[2 + dx][2 + dy]) continue;

			/* Access the cave grid */
			cv_ptr = &cave[cy][cx];

			/* Paranoia -- never affect player */
			if (!dy && !dx) continue;

			/* Destroy location (if valid) */
			if ((cx < cur_wid) && (cy < cur_hgt) && cave_valid_bold(cy, cx))
			{
				bool floor = (f_info[cave[cy][cx].feat].flags1 & FF1_FLOOR);

				/* Delete any object that is still there */
				delete_object(cy, cx);

				if (floor)
				{
					cave_set_feat(cy, cx, FEAT_WALL_OUTER);
				}
				else
				{
					/* Clear previous contents, add floor */
					cave_set_feat(cy, cx, FEAT_FLOOR);
				}
			}
		}

	/* Mega-Hack -- Forget the view and lite */
	p_ptr->update |= PU_UN_VIEW;

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_DISTANCE);

	/* Update the health bar */
	p_ptr->redraw |= (PR_HEALTH);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
	handle_stuff();

	msg_print("Suddenly the cave shifts around you. The air is getting stale!");

	ident = TRUE;

	return (ident);
}


/*
 * this function handles arrow & dagger traps, in various types.
 * num = number of missiles
 * tval, sval = kind of missiles
 * dd,ds = damage roll for missiles
 * poison_dam = additional poison damage
 * name = name given if you should die from it...
 *
 * return value = ident (always TRUE)
 */
static bool player_handle_missile_trap(s16b num, s16b tval, s16b sval, s16b dd, s16b ds,
                                       s16b pdam, cptr name)
{
	object_type *o_ptr, forge;
	s16b i, k_idx = lookup_kind(tval, sval);
	char i_name[80];

	o_ptr = &forge;
	object_prep(o_ptr, k_idx);
	o_ptr->number = num;
	apply_magic(o_ptr, max_dlv[dungeon_type], FALSE, FALSE, FALSE);
	object_desc(i_name, o_ptr, TRUE, 0);

	msg_format("Suddenly %s hit%s you!", i_name,
	           ((num == 1) ? "" : "s"));

	for (i = 0; i < num; i++)
	{
		take_hit(damroll(dd, ds), name);

		redraw_stuff();

		if (pdam > 0)
		{
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				(void)set_poisoned(p_ptr->poisoned + pdam);
			}
		}
	}

	drop_near(o_ptr, -1, p_ptr->py, p_ptr->px);

	return TRUE;
}

/*
 * this function handles a "breath" type trap - acid bolt, lightning balls etc.
 */
static bool player_handle_breath_trap(s16b rad, s16b type, u16b trap)
{
	trap_type *t_ptr = &t_info[trap];
	bool ident;
	s16b my_dd, my_ds, dam;

	my_dd = t_ptr->dd;
	my_ds = t_ptr->ds;

	/* these traps gets nastier as levels progress */
	if (max_dlv[dungeon_type] > (2 * t_ptr->minlevel))
	{
		my_dd += (max_dlv[dungeon_type] / 15);
		my_ds += (max_dlv[dungeon_type] / 15);
	}
	dam = damroll(my_dd, my_ds);

	ident = project( -2, rad, p_ptr->py, p_ptr->px, dam, type, PROJECT_KILL | PROJECT_JUMP);

	return (ident);
}

/*
 * This function damages the player by a trap
 */
static void trap_hit(s16b trap)
{
	s16b dam;
	trap_type *t_ptr = &t_info[trap];

	dam = damroll(t_ptr->dd, t_ptr->ds);

	take_hit(dam, t_name + t_ptr->name);
}

/*
 * this function activates one trap type, and returns
 * a bool indicating if this trap is now identified
 */
bool player_activate_trap_type(s16b y, s16b x, object_type *i_ptr, s16b item)
{
	bool ident = FALSE;
	s16b trap;

	s16b k, l;

	trap = cave[y][x].t_idx;

	if (i_ptr != NULL)
	{
		trap = i_ptr->pval;
	}

	if ((i_ptr == NULL) && (cave[y][x].o_idx != 0))
	{
		i_ptr = &o_list[cave[y][x].o_idx];
	}

	switch (trap)
	{
		/* stat traps */
	case TRAP_OF_WEAKNESS_I:
		ident = do_dec_stat(A_STR, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_WEAKNESS_II:
		ident = do_dec_stat(A_STR, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_WEAKNESS_III:
		ident = do_dec_stat(A_STR, STAT_DEC_PERMANENT);
		break;
	case TRAP_OF_INTELLIGENCE_I:
		ident = do_dec_stat(A_INT, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_INTELLIGENCE_II:
		ident = do_dec_stat(A_INT, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_INTELLIGENCE_III:
		ident = do_dec_stat(A_INT, STAT_DEC_PERMANENT);
		break;
	case TRAP_OF_WISDOM_I:
		ident = do_dec_stat(A_WIS, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_WISDOM_II:
		ident = do_dec_stat(A_WIS, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_WISDOM_III:
		ident = do_dec_stat(A_WIS, STAT_DEC_PERMANENT);
		break;
	case TRAP_OF_FUMBLING_I:
		ident = do_dec_stat(A_DEX, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_FUMBLING_II:
		ident = do_dec_stat(A_DEX, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_FUMBLING_III:
		ident = do_dec_stat(A_DEX, STAT_DEC_PERMANENT);
		break;
	case TRAP_OF_WASTING_I:
		ident = do_dec_stat(A_CON, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_WASTING_II:
		ident = do_dec_stat(A_CON, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_WASTING_III:
		ident = do_dec_stat(A_CON, STAT_DEC_PERMANENT);
		break;
	case TRAP_OF_BEAUTY_I:
		ident = do_dec_stat(A_CHR, STAT_DEC_TEMPORARY);
		break;
	case TRAP_OF_BEAUTY_II:
		ident = do_dec_stat(A_CHR, STAT_DEC_NORMAL);
		break;
	case TRAP_OF_BEAUTY_III:
		ident = do_dec_stat(A_CHR, STAT_DEC_PERMANENT);
		break;

		/* Trap of Curse Weapon */
	case TRAP_OF_CURSE_WEAPON:
		{
			ident = curse_weapon();
			break;
		}

		/* Trap of Curse Armor */
	case TRAP_OF_CURSE_ARMOR:
		{
			ident = curse_armor();
			break;
		}

		/* Earthquake Trap */
	case TRAP_OF_EARTHQUAKE:
		{
			msg_print("As you touch the trap, the ground starts to shake.");
			earthquake(y, x, 10);
			ident = TRUE;
			break;
		}

		/* Poison Needle Trap */
	case TRAP_OF_POISON_NEEDLE:
		{
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				msg_print("You prick yourself on a poisoned needle.");
				(void)set_poisoned(p_ptr->poisoned + rand_int(15) + 10);
				ident = TRUE;
			}
			else
			{
				msg_print("You prick yourself on a needle.");
			}
			break;
		}

		/* Summon Monster Trap */
	case TRAP_OF_SUMMON_MONSTER:
		{
			msg_print("A spell hangs in the air.");
			for (k = 0; k < randint(3); k++)
			{
				ident |= summon_specific(y, x, max_dlv[dungeon_type], 0);
			}
			break;
		}

		/* Summon Undead Trap */
	case TRAP_OF_SUMMON_UNDEAD:
		{
			msg_print("A mighty spell hangs in the air.");
			for (k = 0; k < randint(3); k++)
			{
				ident |= summon_specific(y, x, max_dlv[dungeon_type],
				                         SUMMON_UNDEAD);
			}
			break;
		}

		/* Summon Greater Undead Trap */
	case TRAP_OF_SUMMON_GREATER_UNDEAD:
		{
			msg_print("An old and evil spell hangs in the air.");
			for (k = 0; k < randint(3); k++)
			{
				ident |= summon_specific(y, x, max_dlv[dungeon_type],
				                         SUMMON_HI_UNDEAD);
			}
			break;
		}

		/* Teleport Trap */
	case TRAP_OF_TELEPORT:
		{
			msg_print("The world whirls around you.");
			teleport_player(RATIO * 67);
			ident = TRUE;
			break;
		}

		/* Paralyzing Trap */
	case TRAP_OF_PARALYZING:
		{
			if (!p_ptr->free_act)
			{
				msg_print("You touch a poisoned part and can't move.");
				(void)set_paralyzed(p_ptr->paralyzed + rand_int(10) + 10);
				ident = TRUE;
			}
			else
			{
				msg_print("You prick yourself on a needle.");
			}
			break;
		}

		/* Explosive Device */
	case TRAP_OF_EXPLOSIVE_DEVICE:
		{
			msg_print("A hidden explosive device explodes in your face.");
			take_hit(damroll(5, 8), "an explosion");
			ident = TRUE;
			break;
		}

		/* Teleport Away Trap */
	case TRAP_OF_TELEPORT_AWAY:
		{
			int item, amt;
			object_type *o_ptr;

			/* teleport away all items */
			while (cave[y][x].o_idx != 0)
			{
				item = cave[y][x].o_idx;

				o_ptr = &o_list[item];

				amt = o_ptr->number;

				ident = do_trap_teleport_away(o_ptr, y, x);

				floor_item_increase(item, -amt);
				floor_item_optimize(item);
			}
			break;
		}

		/* Lose Memory Trap */
	case TRAP_OF_LOSE_MEMORY:
		{
			lose_exp(p_ptr->exp / 4);

			ident |= dec_stat(A_WIS, rand_int(20) + 10, STAT_DEC_NORMAL);
			ident |= dec_stat(A_INT, rand_int(20) + 10, STAT_DEC_NORMAL);

			if (!p_ptr->resist_conf)
			{
				ident |= set_confused(p_ptr->confused + rand_int(100) + 50);
			}

			if (ident)
			{
				msg_print("You suddenly don't remember what you were doing.");
			}
			else
			{
				msg_print("You feel an alien force probing your mind.");
			}
			break;
		}
		/* Bitter Regret Trap */
	case TRAP_OF_BITTER_REGRET:
		{
			msg_print("An age-old and hideous-sounding spell reverberates off the walls.");

			ident |= dec_stat(A_DEX, 25, TRUE);
			ident |= dec_stat(A_WIS, 25, TRUE);
			ident |= dec_stat(A_CON, 25, TRUE);
			ident |= dec_stat(A_STR, 25, TRUE);
			ident |= dec_stat(A_CHR, 25, TRUE);
			ident |= dec_stat(A_INT, 25, TRUE);
			break;
		}

		/* Bowel Cramps Trap */
	case TRAP_OF_BOWEL_CRAMPS:
		{
			msg_print("A wretched-smelling gas cloud upsets your stomach.");

			(void)set_food(PY_FOOD_STARVE - 1);
			(void)set_poisoned(0);

			if (!p_ptr->free_act)
			{
				(void)set_paralyzed(p_ptr->paralyzed + rand_int(dun_level) + 6);
			}
			ident = TRUE;
			break;
		}

		/* Blindness/Confusion Trap */
	case TRAP_OF_BLINDNESS_CONFUSION:
		{
			msg_print("A powerful magic protected this.");

			if (!p_ptr->resist_blind)
			{
				ident |= set_blind(p_ptr->blind + rand_int(100) + 100);
			}
			if (!p_ptr->resist_conf)
			{
				ident |= set_confused(p_ptr->confused + rand_int(20) + 15);
			}
			break;
		}

		/* Aggravation Trap */
	case TRAP_OF_AGGRAVATION:
		{
			msg_print("You hear a hollow noise echoing through the dungeons.");
			aggravate_monsters(1);
			break;
		}

		/* Multiplication Trap */
	case TRAP_OF_MULTIPLICATION:
		{
			msg_print("You hear a loud click.");
			for (k = -1; k <= 1; k++)
				for (l = -1; l <= 1; l++)
				{
					if ((in_bounds(p_ptr->py + l, p_ptr->px + k)) &&
					                (!cave[p_ptr->py + l][p_ptr->px + k].t_idx))
					{
						place_trap(p_ptr->py + l, p_ptr->px + k);
					}
				}
			ident = TRUE;
			break;
		}

		/* Steal Item Trap */
	case TRAP_OF_STEAL_ITEM:
		{
			/*
			 * please note that magical stealing is not so
			 * easily circumvented
			 */
			if (!p_ptr->paralyzed &&
			                (rand_int(160) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
			                                  p_ptr->lev)))
			{
				/* Saving throw message */
				msg_print("Your backpack seems to vibrate strangely!");
				break;
			}

			/* Find an item */
			for (k = 0; k < rand_int(10); k++)
			{
				char i_name[80];
				object_type *j_ptr, *q_ptr, forge;

				/* Pick an item */
				s16b i = rand_int(INVEN_PACK);

				/* Obtain the item */
				j_ptr = &p_ptr->inventory[i];

				/* Accept real items */
				if (!j_ptr->k_idx) continue;

				/* Don't steal artifacts  -CFT */
				if (artifact_p(j_ptr)) continue;

				/* Get a description */
				object_desc(i_name, j_ptr, FALSE, 3);

				/* Message */
				msg_format("%sour %s (%c) was stolen!",
				           ((j_ptr->number > 1) ? "One of y" : "Y"),
				           i_name, index_to_label(i));

				/* Create the item */
				q_ptr = &forge;
				object_copy(q_ptr, j_ptr);
				q_ptr->number = 1;

				/* Drop it somewhere */
				do_trap_teleport_away(q_ptr, y, x);

				inven_item_increase(i, -1);
				inven_item_optimize(i);
				ident = TRUE;
			}
			break;
		}

		/* Summon Fast Quylthulgs Trap */
	case TRAP_OF_SUMMON_FAST_QUYLTHULGS:
		{
			for (k = 0; k < randint(3); k++)
			{
				ident |= summon_specific(y, x, max_dlv[dungeon_type], SUMMON_QUYLTHULG);
			}

			if (ident)
			{
				msg_print("You suddenly have company.");
				(void)set_slow(p_ptr->slow + randint(25) + 15);
			}
			break;
		}

		/* Trap of Sinking */
	case TRAP_OF_SINKING:
		{
			msg_print("You fell through a trap door!");

			if (p_ptr->ffall)
			{
				if (dungeon_flags1 & DF1_TOWER)
				{
					msg_print("You float gently down to the previous level.");
				}
				else
				{
					msg_print("You float gently down to the next level.");
				}
			}
			else
			{
				take_hit(damroll(2, 8), "a trap door");
			}

			/* Still alive and autosave enabled */
			if (autosave_l && (p_ptr->chp >= 0))
			{
				is_autosave = TRUE;
				msg_print("Autosaving the game...");
				do_cmd_save_game();
				is_autosave = FALSE;
			}

			if (dungeon_flags1 & DF1_TOWER) dun_level--;
			else dun_level++;

			/* Leaving */
			p_ptr->leaving = TRUE;
			break;
		}

		/* Trap of Mana Drain */
	case TRAP_OF_MANA_DRAIN:
		{
			if (p_ptr->csp > 0)
			{
				p_ptr->csp = 0;
				p_ptr->csp_frac = 0;
				p_ptr->redraw |= (PR_MANA);
				msg_print("You sense a great loss.");
				ident = TRUE;
			}
			else if (p_ptr->msp == 0)
			{
				/* no sense saying this unless you never have mana */
				msg_format("Suddenly you feel glad you're a mere %s",
				           spp_ptr->title + c_name);
			}
			else
			{
				msg_print("Your head feels dizzy for a moment.");
			}
			break;
		}
		/* Trap of Missing Money */
	case TRAP_OF_MISSING_MONEY:
		{
			s32b gold = (p_ptr->au / 10) + randint(25);

			if (gold < 2) gold = 2;
			if (gold > 5000) gold = (p_ptr->au / 20) + randint(3000);
			if (gold > p_ptr->au) gold = p_ptr->au;

			p_ptr->au -= gold;
			if (gold <= 0)
			{
				msg_print("You feel something touching you.");
			}
			else if (p_ptr->au)
			{
				msg_print("Your purse feels lighter.");
				msg_format("%ld coins were stolen!", (long)gold);
				ident = TRUE;
			}
			else
			{
				msg_print("Your purse feels empty.");
				msg_print("All of your coins were stolen!");
				ident = TRUE;
			}
			p_ptr->redraw |= (PR_GOLD);
			break;
		}

		/* Trap of No Return */
	case TRAP_OF_NO_RETURN:
		{
			object_type *j_ptr;
			s16b j;

			for (j = 0; j < INVEN_WIELD; j++)
			{
				if (!p_ptr->inventory[j].k_idx) continue;

				j_ptr = &p_ptr->inventory[j];

				if ((j_ptr->tval == TV_SCROLL) &&
				                (j_ptr->sval == SV_SCROLL_WORD_OF_RECALL))
				{
					inven_item_increase(j, -j_ptr->number);
					inven_item_optimize(j);
					combine_pack();
					reorder_pack();
					if (!ident)
					{
						msg_print("A small fire works its way through your backpack. "
						          "Some scrolls are burnt.");
					}
					else
					{
						msg_print("The fire hasn't finished.");
					}
					ident = TRUE;
				}
				else if ((j_ptr->tval == TV_ROD_MAIN) &&
				                (j_ptr->pval == SV_ROD_RECALL))
				{
					j_ptr->timeout = 0;  /* a long time */
					if (!ident) msg_print("You feel the air stabilise around you.");
					ident = TRUE;
				}
			}
			if ((!ident) && (p_ptr->word_recall))
			{
				msg_print("You feel like staying around.");
				p_ptr->word_recall = 0;
				ident = TRUE;
			}
			break;
		}

		/* Trap of Silent Switching */
	case TRAP_OF_SILENT_SWITCHING:
		{
			s16b i, j, slot1, slot2;
			object_type *j_ptr, *k_ptr;
			u32b f1, f2, f3, f4, f5, esp;

			for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
			{
				j_ptr = &p_ptr->inventory[i];

				if (!j_ptr->k_idx) continue;

				/* Do not allow this trap to touch the One Ring */
				object_flags(j_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
				if(f3 & TR3_PERMA_CURSE) continue;

				slot1 = wield_slot(j_ptr);

				for (j = 0; j < INVEN_WIELD; j++)
				{
					k_ptr = &p_ptr->inventory[j];

					if (!k_ptr->k_idx) continue;

					/* Do not allow this trap to touch the One Ring */
					object_flags(k_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
					if(f3 & TR3_PERMA_CURSE) continue;

					/* this is a crude hack, but it prevent wielding 6 torches... */
					if (k_ptr->number > 1) continue;

					slot2 = wield_slot(k_ptr);

					/* a chance of 4 in 5 of switching something, then 2 in 5 to do it again */
					if ((slot1 == slot2) &&
					                (rand_int(100) < (80 - (ident * 40))))
					{
						object_type tmp_obj;

						tmp_obj = p_ptr->inventory[j];
						p_ptr->inventory[j] = p_ptr->inventory[i];
						p_ptr->inventory[i] = tmp_obj;
						ident = TRUE;
					}
				}
			}

			if (ident)
			{
				p_ptr->update |= (PU_BONUS);
				p_ptr->update |= (PU_TORCH);
				p_ptr->update |= (PU_MANA);
				msg_print("You somehow feel like another person.");
			}
			else
			{
				msg_print("You feel a lack of useful items.");
			}
			break;
		}

		/* Trap of Walls */
	case TRAP_OF_WALLS:
		{
			ident = player_handle_trap_of_walls();
			break;
		}

		/* Trap of Calling Out */
	case TRAP_OF_CALLING_OUT:
		{
			ident = do_player_trap_call_out();

			if (!ident)
			{
				/* Increase "afraid" */
				if (p_ptr->resist_fear)
				{
					msg_print("You feel as if you had a nightmare!");
				}
				else if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You remember having a nightmare!");
				}
				else
				{
					if (set_afraid(p_ptr->afraid + 3 + randint(40)))
					{
						msg_print("You have a vision of a powerful enemy.");
					}
				}
			}
			break;
		}

		/* Trap of Sliding */
	case TRAP_OF_SLIDING:
		break;

		/* Trap of Charges Drain */
	case TRAP_OF_CHARGES_DRAIN:
		{
			/* Find an item */
			for (k = 0; k < 10; k++)
			{
				s16b i = rand_int(INVEN_PACK);

				object_type *j_ptr = &p_ptr->inventory[i];

				/* Drain charged wands/staffs
				   Hack -- don't let artifacts get drained */
				if (((j_ptr->tval == TV_STAFF) ||
				                (j_ptr->tval == TV_WAND)) &&
				                (j_ptr->pval) &&
			                     !artifact_p(j_ptr))
				{
					ident = TRUE;
					j_ptr->pval = j_ptr->pval / (randint(4) + 1);

					/* 60% chance of only 1 */
					if (randint(10) > 3) break;
				}
			}

			if (ident)
			{
				/* Window stuff */
				p_ptr->window |= PW_INVEN;
				/* Combine / Reorder the pack */
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);

				msg_print("Your backpack seems to be turned upside down.");
			}
			else
			{
				msg_print("You hear a wail of great disappointment.");
			}
			break;
		}

		/* Trap of Stair Movement */
	case TRAP_OF_STAIR_MOVEMENT:
		{
			s16b cx, cy, i, j;
			s16b cnt = 0;
			s16b cnt_seen = 0;
			s16b tmps, tmpx;
			s16b tmpspecial, tmpspecial2;
			u32b tmpf;
			bool seen = FALSE;
			s16b index_x[20], index_y[20];  /* 20 stairs per level is enough? */
			cave_type *cv_ptr;

			if (max_dlv[dungeon_type] == 99)
			{
				/* no sense in relocating that stair! */
				msg_print("You have a feeling that this trap could be dangerous.");
				break;
			}

			for (cx = 0; cx < cur_wid; cx++)
				for (cy = 0; cy < cur_hgt; cy++)
				{
					cv_ptr = &cave[cy][cx];

					if ((cv_ptr->feat != FEAT_LESS) &&
					                (cv_ptr->feat != FEAT_MORE) &&
					                (cv_ptr->feat != FEAT_SHAFT_UP) &&
					                (cv_ptr->feat != FEAT_SHAFT_DOWN)) continue;

					index_x[cnt] = cx;
					index_y[cnt] = cy;
					cnt++;
				}

			if (cnt == 0)
			{
				if (wizard) msg_print("Executing moving stairs trap on level with no stairs!");
				break;
			}

			for (i = 0; i < cnt; i++)
			{
				seen = FALSE;

				for (j = 0; j < 10; j++) /* try 10 times to relocate */
				{
					cave_type *cv_ptr = &cave[index_y[i]][index_x[i]];
					cave_type *cv_ptr2;

					cx = rand_int(cur_wid);
					cy = rand_int(cur_hgt);

					if ((cx == index_x[i]) || (cy == index_y[i])) continue;

					cv_ptr2 = &cave[cy][cx];

					if (!cave_valid_bold(cy, cx) || cv_ptr2->o_idx != 0) continue;

					/* don't put anything in vaults */
					if (cv_ptr2->info & CAVE_ICKY) continue;

					tmpx = cv_ptr2->mimic;
					tmps = cv_ptr2->info;
					tmpf = cv_ptr2->feat;
					tmpspecial = cv_ptr2->special;
					tmpspecial2 = cv_ptr2->special2;
					cave[cy][cx].mimic = cv_ptr->mimic;
					cave[cy][cx].info = cv_ptr->info;
					cave[cy][cx].special = cv_ptr->special;
					cave[cy][cx].special2 = cv_ptr->special2;
					cave_set_feat(cy, cx, cv_ptr->feat);
					cv_ptr->mimic = tmpx;
					cv_ptr->info = tmps;
					cv_ptr->special = tmpspecial;
					cv_ptr->special2 = tmpspecial2;
					cave_set_feat(index_y[i], index_x[i], tmpf);

					/* if we are placing walls in rooms, make them rubble instead */
					if ((cv_ptr->info & CAVE_ROOM) &&
					                (cv_ptr->feat >= FEAT_WALL_EXTRA) &&
					                (cv_ptr->feat <= FEAT_PERM_SOLID))
					{
						cave_set_feat(index_y[i], index_x[i], FEAT_RUBBLE);
					}

					if (player_has_los_bold(cy, cx))
					{
						note_spot(cy, cx);
						lite_spot(cy, cx);
						seen = TRUE;
					}
					else
					{
						cv_ptr2->info &= ~CAVE_MARK;
					}

					if (player_has_los_bold(index_y[i], index_x[i]))
					{
						note_spot(index_y[i], index_x[i]);
						lite_spot(index_y[i], index_x[i]);
						seen = TRUE;
					}
					else
					{
						cv_ptr->info &= ~CAVE_MARK;
					}
					break;
				}

				if (seen) cnt_seen++;
			}

			ident = (cnt_seen > 0);

			if ((ident) && (cnt_seen > 1))
			{
				msg_print("You see some stairs move.");
			}
			else if (ident)
			{
				msg_print("You see a stair move.");
			}
			else
			{
				msg_print("You hear distant scraping noises.");
			}
			p_ptr->redraw |= PR_MAP;
			break;
		}

		/* Trap of New Trap */
	case TRAP_OF_NEW:
		{
			/* if we're on a floor or on a door, place a new trap */
			if ((item == -1) || (item == -2))
			{
				place_trap(y, x);
				if (player_has_los_bold(y, x))
				{
					note_spot(y, x);
					lite_spot(y, x);
				}
			}
			else
			{
				/* re-trap the chest */
				place_trap(y, x);
			}
			msg_print("You hear a noise, and then its echo.");
			ident = FALSE;
			break;
		}

		/* Trap of Acquirement */
	case TRAP_OF_ACQUIREMENT:
		{
			/* Get a nice thing */
			msg_print("You notice something falling off the trap.");
			acquirement(y, x, 1, TRUE, FALSE);

			/* If we're on a floor or on a door, place a new trap */
			if ((item == -1) || (item == -2))
			{
				place_trap(y, x);
				if (player_has_los_bold(y, x))
				{
					note_spot(y, x);
					lite_spot(y, x);
				}
			}
			else
			{
				/* Re-trap the chest */
				place_trap(y, x);
			}
			msg_print("You hear a noise, and then its echo.");

			/* Never known */
			ident = FALSE;
		}
		break;

		/* Trap of Scatter Items */
	case TRAP_OF_SCATTER_ITEMS:
		{
			s16b i, j;
			bool message = FALSE;

			for (i = 0; i < INVEN_PACK; i++)
			{

				if (!p_ptr->inventory[i].k_idx) continue;

				if (rand_int(10) < 3) continue;

				for (j = 0; j < 10; j++)
				{
					object_type tmp_obj, *j_ptr = &tmp_obj;
					s16b cx = x + 15 - rand_int(30);
					s16b cy = y + 15 - rand_int(30);

					if (!in_bounds(cy, cx)) continue;

					if (!cave_floor_bold(cy, cx)) continue;

					object_copy(j_ptr, &p_ptr->inventory[i]);
					inven_item_increase(i, -999);
					inven_item_optimize(i);

					p_ptr->notice |= (PN_COMBINE | PN_REORDER);

					(void)floor_carry(cy, cx, j_ptr);

					if (!message)
					{
						msg_print("You feel light-footed.");
						message = TRUE;
					}

					if (player_has_los_bold(cy, cx))
					{
						char i_name[80];

						object_desc(i_name, &tmp_obj, TRUE, 3);
						note_spot(cy, cx);
						lite_spot(cy, cx);
						ident = TRUE;
						msg_format("Suddenly %s appear%s!", i_name,
						           (j_ptr->number > 1) ? "" : "s");
					}
					break;
				}
			}
			ident = message;
			break;
		}

		/* Trap of Decay */
	case TRAP_OF_DECAY:
		break;

		/* Trap of Wasting Wands */
	case TRAP_OF_WASTING_WANDS:
		{
			s16b i;
			object_type *j_ptr;

			for (i = 0; i < INVEN_PACK; i++)
			{
				if (!p_ptr->inventory[i].k_idx) continue;

				j_ptr = &p_ptr->inventory[i];

				if ((j_ptr->tval == TV_WAND) && (rand_int(5) == 1))
				{
					if (object_known_p(j_ptr)) ident = TRUE;

					/* Create a Wand of Nothing */
					object_prep(j_ptr, lookup_kind(TV_WAND, SV_WAND_NOTHING));
					hack_apply_magic_power = -99;
					apply_magic(j_ptr, 0, FALSE, FALSE, FALSE);
					j_ptr->ident &= ~IDENT_KNOWN;
					p_ptr->notice |= (PN_COMBINE | PN_REORDER);
				}
				else if ((j_ptr->tval == TV_STAFF) && (rand_int(5) == 1))
				{
					if (object_known_p(j_ptr)) ident = TRUE;

					/* Create a Staff of Nothing */
					object_prep(j_ptr, lookup_kind(TV_STAFF, SV_STAFF_NOTHING));
					hack_apply_magic_power = -99;
					apply_magic(j_ptr, 0, FALSE, FALSE, FALSE);
					j_ptr->ident &= ~IDENT_KNOWN;
					p_ptr->notice |= (PN_COMBINE | PN_REORDER);
				}
			}
			if (ident)
			{
				msg_print("You have lost trust in your backpack!");
			}
			else
			{
				msg_print("You hear an echoing cry of rage.");
			}
			break;
		}

		/* Trap of Filling */
	case TRAP_OF_FILLING:
		{
			s16b nx, ny;

			for (nx = x - 8; nx <= x + 8; nx++)
				for (ny = y - 8; ny <= y + 8; ny++)
				{
					if (!in_bounds (ny, nx)) continue;

					if (rand_int(distance(ny, nx, y, x)) > 3)
					{
						place_trap(ny, nx);
					}
				}

			msg_print("The floor vibrates in a strange way.");
			ident = FALSE;
			break;
		}

	case TRAP_OF_DRAIN_SPEED:
		{
			object_type *j_ptr;
			s16b j, chance = 75;
			u32b f1, f2, f3, f4, f5, esp;

			for (j = 0; j < INVEN_TOTAL; j++)
			{
				/* don't bother the overflow slot */
				if (j == INVEN_PACK) continue;

				if (!p_ptr->inventory[j].k_idx) continue;

				j_ptr = &p_ptr->inventory[j];
				object_flags(j_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

				/* is it a non-artifact speed item? */
				if ((!j_ptr->name1) && (f1 & TR1_SPEED))
				{
					if (randint(100) < chance)
					{
						j_ptr->pval = j_ptr->pval / 2;
						if (j_ptr->pval == 0)
						{
							j_ptr->pval--;
						}
						chance /= 2;
						ident = TRUE;
					}
					inven_item_optimize(j);
				}
			}
			if (!ident)
			{
				msg_print("You feel some things in your pack vibrating.");
			}
			else
			{
				combine_pack();
				reorder_pack();
				msg_print("You suddenly feel you have time for self-reflection.");

				/* Recalculate bonuses */
				p_ptr->update |= (PU_BONUS);

				/* Recalculate mana */
				p_ptr->update |= (PU_MANA);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
			}
			break;
		}

		/*
		 * single missile traps
		 */
	case TRAP_OF_ARROW_I:
		ident = player_handle_missile_trap(1, TV_ARROW, SV_AMMO_NORMAL, 4, 8, 0, "Arrow Trap");
		break;
	case TRAP_OF_ARROW_II:
		ident = player_handle_missile_trap(1, TV_BOLT, SV_AMMO_NORMAL, 5, 8, 0, "Bolt Trap");
		break;
	case TRAP_OF_ARROW_III:
		ident = player_handle_missile_trap(1, TV_ARROW, SV_AMMO_HEAVY, 6, 8, 0, "Seeker Arrow Trap");
		break;
	case TRAP_OF_ARROW_IV:
		ident = player_handle_missile_trap(1, TV_BOLT, SV_AMMO_HEAVY, 8, 10, 0, "Seeker Bolt Trap");
		break;
	case TRAP_OF_POISON_ARROW_I:
		ident = player_handle_missile_trap(1, TV_ARROW, SV_AMMO_NORMAL, 4, 8, 10 + randint(20), "Poison Arrow Trap");
		break;
	case TRAP_OF_POISON_ARROW_II:
		ident = player_handle_missile_trap(1, TV_BOLT, SV_AMMO_NORMAL, 5, 8, 15 + randint(30), "Poison Bolt Trap");
		break;
	case TRAP_OF_POISON_ARROW_III:
		ident = player_handle_missile_trap(1, TV_ARROW, SV_AMMO_HEAVY, 6, 8, 30 + randint(50), "Poison Seeker Arrow Trap");
		break;
	case TRAP_OF_POISON_ARROW_IV:
		ident = player_handle_missile_trap(1, TV_BOLT, SV_AMMO_HEAVY, 8, 10, 40 + randint(70), "Poison Seeker Bolt Trap");
		break;
	case TRAP_OF_DAGGER_I:
		ident = player_handle_missile_trap(1, TV_SWORD, SV_BROKEN_DAGGER, 2, 8, 0, "Dagger Trap");
		break;
	case TRAP_OF_DAGGER_II:
		ident = player_handle_missile_trap(1, TV_SWORD, SV_DAGGER, 3, 8, 0, "Dagger Trap");
		break;
	case TRAP_OF_POISON_DAGGER_I:
		ident = player_handle_missile_trap(1, TV_SWORD, SV_BROKEN_DAGGER, 2, 8, 15 + randint(20), "Poison Dagger Trap");
		break;
	case TRAP_OF_POISON_DAGGER_II:
		ident = player_handle_missile_trap(1, TV_SWORD, SV_DAGGER, 3, 8, 20 + randint(30), "Poison Dagger Trap");
		break;

		/*
		 * multiple missile traps
		 * numbers range from 2 (level 0 to 14) to 10 (level 120 and up)
		 */
	case TRAP_OF_ARROWS_I:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_ARROW, SV_AMMO_NORMAL, 4, 8, 0, "Arrow Trap");
		break;
	case TRAP_OF_ARROWS_II:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_BOLT, SV_AMMO_NORMAL, 5, 8, 0, "Bolt Trap");
		break;
	case TRAP_OF_ARROWS_III:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_ARROW, SV_AMMO_HEAVY, 6, 8, 0, "Seeker Arrow Trap");
		break;
	case TRAP_OF_ARROWS_IV:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_BOLT, SV_AMMO_HEAVY, 8, 10, 0, "Seeker Bolt Trap");
		break;
	case TRAP_OF_POISON_ARROWS_I:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_ARROW, SV_AMMO_NORMAL, 4, 8, 10 + randint(20), "Poison Arrow Trap");
		break;
	case TRAP_OF_POISON_ARROWS_II:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_BOLT, SV_AMMO_NORMAL, 5, 8, 15 + randint(30), "Poison Bolt Trap");
		break;
	case TRAP_OF_POISON_ARROWS_III:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_ARROW, SV_AMMO_HEAVY, 6, 8, 30 + randint(50), "Poison Seeker Arrow Trap");
		break;
	case TRAP_OF_POISON_ARROWS_IV:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_BOLT, SV_AMMO_HEAVY, 8, 10, 40 + randint(70), "Poison Seeker Bolt Trap");
		break;
	case TRAP_OF_DAGGERS_I:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_SWORD, SV_BROKEN_DAGGER, 2, 8, 0, "Dagger Trap");
		break;
	case TRAP_OF_DAGGERS_II:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_SWORD, SV_DAGGER, 3, 8, 0, "Dagger Trap");
		break;
	case TRAP_OF_POISON_DAGGERS_I:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_SWORD, SV_BROKEN_DAGGER, 2, 8, 15 + randint(20), "Poison Dagger Trap");
		break;
	case TRAP_OF_POISON_DAGGERS_II:
		ident = player_handle_missile_trap(2 + (max_dlv[dungeon_type] / 15), TV_SWORD, SV_DAGGER, 3, 8, 20 + randint(30), "Poison Dagger Trap");
		break;

	case TRAP_OF_DROP_ITEMS:
		{
			s16b i;
			bool message = FALSE;

			for (i = 0; i < INVEN_PACK; i++)
			{
				object_type tmp_obj;

				if (!p_ptr->inventory[i].k_idx) continue;
				if (randint(100) < 80) continue;
				if (p_ptr->inventory[i].name1 == ART_POWER) continue;

				tmp_obj = p_ptr->inventory[i];

				/* drop carefully */
				drop_near(&tmp_obj, 0, y, x);
				inven_item_increase(i, -999);
				inven_item_optimize(i);
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);

				if (!message)
				{
					msg_print("You are startled by a sudden sound.");
					message = TRUE;
				}
				ident = TRUE;
			}
			if (!ident)
			{
				msg_print("You hear a sudden, strange sound.");
			}
			break;
		}

	case TRAP_OF_DROP_ALL_ITEMS:
		{
			s16b i;
			bool message = FALSE;

			for (i = 0; i < INVEN_PACK; i++)
			{
				object_type tmp_obj;

				if (!p_ptr->inventory[i].k_idx) continue;
				if (randint(100) < 10) continue;
				if (p_ptr->inventory[i].name1 == ART_POWER) continue;

				tmp_obj = p_ptr->inventory[i];

				/* drop carefully */
				drop_near(&tmp_obj, 0, y, x);
				inven_item_increase(i, -999);
				inven_item_optimize(i);
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);

				if (!message)
				{
					msg_print("You are greatly startled by a sudden sound.");
					message = TRUE;
				}
				ident = TRUE;
			}
			if (!ident)
			{
				msg_print("You hear a sudden, strange sound.");
			}
			break;
		}

	case TRAP_OF_DROP_EVERYTHING:
		{
			s16b i;
			bool message = FALSE;

			for (i = 0; i < INVEN_TOTAL; i++)
			{
				object_type tmp_obj;
				if (!p_ptr->inventory[i].k_idx) continue;
				if (randint(100) < 30) continue;
				if (p_ptr->inventory[i].name1 == ART_POWER) continue;

				tmp_obj = p_ptr->inventory[i];
				/* drop carefully */

				drop_near(&tmp_obj, 0, y, x);
				inven_item_increase(i, -999);
				inven_item_optimize(i);
				p_ptr->notice |= (PN_COMBINE | PN_REORDER);

				if (!message)
				{
					msg_print("You are completely startled by a sudden sound.");
					message = TRUE;
				}
				ident = TRUE;
			}
			if (!ident)
			{
				msg_print("You hear a sudden, strange sound.");
			}
			break;
		}

		/* Bolt Trap */
	case TRAP_G_ELEC_BOLT:
		ident = player_handle_breath_trap(1, GF_ELEC, TRAP_G_ELEC_BOLT);
		break;
	case TRAP_G_POIS_BOLT:
		ident = player_handle_breath_trap(1, GF_POIS, TRAP_G_POIS_BOLT);
		break;
	case TRAP_G_ACID_BOLT:
		ident = player_handle_breath_trap(1, GF_ACID, TRAP_G_ACID_BOLT);
		break;
	case TRAP_G_COLD_BOLT:
		ident = player_handle_breath_trap(1, GF_COLD, TRAP_G_COLD_BOLT);
		break;
	case TRAP_G_FIRE_BOLT:
		ident = player_handle_breath_trap(1, GF_FIRE, TRAP_G_FIRE_BOLT);
		break;
	case TRAP_OF_ELEC_BOLT:
		ident = player_handle_breath_trap(1, GF_ELEC, TRAP_OF_ELEC_BOLT);
		break;
	case TRAP_OF_POIS_BOLT:
		ident = player_handle_breath_trap(1, GF_POIS, TRAP_OF_POIS_BOLT);
		break;
	case TRAP_OF_ACID_BOLT:
		ident = player_handle_breath_trap(1, GF_ACID, TRAP_OF_ACID_BOLT);
		break;
	case TRAP_OF_COLD_BOLT:
		ident = player_handle_breath_trap(1, GF_COLD, TRAP_OF_COLD_BOLT);
		break;
	case TRAP_OF_FIRE_BOLT:
		ident = player_handle_breath_trap(1, GF_FIRE, TRAP_OF_FIRE_BOLT);
		break;
	case TRAP_OF_PLASMA_BOLT:
		ident = player_handle_breath_trap(1, GF_PLASMA, TRAP_OF_PLASMA_BOLT);
		break;
	case TRAP_OF_WATER_BOLT:
		ident = player_handle_breath_trap(1, GF_WATER, TRAP_OF_WATER_BOLT);
		break;
	case TRAP_OF_LITE_BOLT:
		ident = player_handle_breath_trap(1, GF_LITE, TRAP_OF_LITE_BOLT);
		break;
	case TRAP_OF_DARK_BOLT:
		ident = player_handle_breath_trap(1, GF_DARK, TRAP_OF_DARK_BOLT);
		break;
	case TRAP_OF_SHARDS_BOLT:
		ident = player_handle_breath_trap(1, GF_SHARDS, TRAP_OF_SHARDS_BOLT);
		break;
	case TRAP_OF_SOUND_BOLT:
		ident = player_handle_breath_trap(1, GF_SOUND, TRAP_OF_SOUND_BOLT);
		break;
	case TRAP_OF_CONFUSION_BOLT:
		ident = player_handle_breath_trap(1, GF_CONFUSION, TRAP_OF_CONFUSION_BOLT);
		break;
	case TRAP_OF_FORCE_BOLT:
		ident = player_handle_breath_trap(1, GF_FORCE, TRAP_OF_FORCE_BOLT);
		break;
	case TRAP_OF_INERTIA_BOLT:
		ident = player_handle_breath_trap(1, GF_INERTIA, TRAP_OF_INERTIA_BOLT);
		break;
	case TRAP_OF_MANA_BOLT:
		ident = player_handle_breath_trap(1, GF_MANA, TRAP_OF_MANA_BOLT);
		break;
	case TRAP_OF_ICE_BOLT:
		ident = player_handle_breath_trap(1, GF_ICE, TRAP_OF_ICE_BOLT);
		break;
	case TRAP_OF_CHAOS_BOLT:
		ident = player_handle_breath_trap(1, GF_CHAOS, TRAP_OF_CHAOS_BOLT);
		break;
	case TRAP_OF_NETHER_BOLT:
		ident = player_handle_breath_trap(1, GF_NETHER, TRAP_OF_NETHER_BOLT);
		break;
	case TRAP_OF_DISENCHANT_BOLT:
		ident = player_handle_breath_trap(1, GF_DISENCHANT, TRAP_OF_DISENCHANT_BOLT);
		break;
	case TRAP_OF_NEXUS_BOLT:
		ident = player_handle_breath_trap(1, GF_NEXUS, TRAP_OF_NEXUS_BOLT);
		break;
	case TRAP_OF_TIME_BOLT:
		ident = player_handle_breath_trap(1, GF_TIME, TRAP_OF_TIME_BOLT);
		break;
	case TRAP_OF_GRAVITY_BOLT:
		ident = player_handle_breath_trap(1, GF_GRAVITY, TRAP_OF_GRAVITY_BOLT);
		break;

		/* Ball Trap */
	case TRAP_OF_ELEC_BALL:
		ident = player_handle_breath_trap(3, GF_ELEC, TRAP_OF_ELEC_BALL);
		break;
	case TRAP_OF_POIS_BALL:
		ident = player_handle_breath_trap(3, GF_POIS, TRAP_OF_POIS_BALL);
		break;
	case TRAP_OF_ACID_BALL:
		ident = player_handle_breath_trap(3, GF_ACID, TRAP_OF_ACID_BALL);
		break;
	case TRAP_OF_COLD_BALL:
		ident = player_handle_breath_trap(3, GF_COLD, TRAP_OF_COLD_BALL);
		break;
	case TRAP_OF_FIRE_BALL:
		ident = player_handle_breath_trap(3, GF_FIRE, TRAP_OF_FIRE_BALL);
		break;
	case TRAP_OF_PLASMA_BALL:
		ident = player_handle_breath_trap(3, GF_PLASMA, TRAP_OF_PLASMA_BALL);
		break;
	case TRAP_OF_WATER_BALL:
		ident = player_handle_breath_trap(3, GF_WATER, TRAP_OF_WATER_BALL);
		break;
	case TRAP_OF_LITE_BALL:
		ident = player_handle_breath_trap(3, GF_LITE, TRAP_OF_LITE_BALL);
		break;
	case TRAP_OF_DARK_BALL:
		ident = player_handle_breath_trap(3, GF_DARK, TRAP_OF_DARK_BALL);
		break;
	case TRAP_OF_SHARDS_BALL:
		ident = player_handle_breath_trap(3, GF_SHARDS, TRAP_OF_SHARDS_BALL);
		break;
	case TRAP_OF_SOUND_BALL:
		ident = player_handle_breath_trap(3, GF_SOUND, TRAP_OF_SOUND_BALL);
		break;
	case TRAP_OF_CONFUSION_BALL:
		ident = player_handle_breath_trap(3, GF_CONFUSION, TRAP_OF_CONFUSION_BALL);
		break;
	case TRAP_OF_FORCE_BALL:
		ident = player_handle_breath_trap(3, GF_FORCE, TRAP_OF_FORCE_BALL);
		break;
	case TRAP_OF_INERTIA_BALL:
		ident = player_handle_breath_trap(3, GF_INERTIA, TRAP_OF_INERTIA_BALL);
		break;
	case TRAP_OF_MANA_BALL:
		ident = player_handle_breath_trap(3, GF_MANA, TRAP_OF_MANA_BALL);
		break;
	case TRAP_OF_ICE_BALL:
		ident = player_handle_breath_trap(3, GF_ICE, TRAP_OF_ICE_BALL);
		break;
	case TRAP_OF_CHAOS_BALL:
		ident = player_handle_breath_trap(3, GF_CHAOS, TRAP_OF_CHAOS_BALL);
		break;
	case TRAP_OF_NETHER_BALL:
		ident = player_handle_breath_trap(3, GF_NETHER, TRAP_OF_NETHER_BALL);
		break;
	case TRAP_OF_DISENCHANT_BALL:
		ident = player_handle_breath_trap(3, GF_DISENCHANT, TRAP_OF_DISENCHANT_BALL);
		break;
	case TRAP_OF_NEXUS_BALL:
		ident = player_handle_breath_trap(3, GF_NEXUS, TRAP_OF_NEXUS_BALL);
		break;
	case TRAP_OF_TIME_BALL:
		ident = player_handle_breath_trap(3, GF_TIME, TRAP_OF_TIME_BALL);
		break;
	case TRAP_OF_GRAVITY_BALL:
		ident = player_handle_breath_trap(3, GF_GRAVITY, TRAP_OF_GRAVITY_BALL);
		break;

		/* -SC- */
	case TRAP_OF_FEMINITY:
		{
			msg_print("Gas sprouts out... you feel yourself transmute.");
			p_ptr->psex = SEX_FEMALE;
			sp_ptr = &sex_info[p_ptr->psex];
			ident = TRUE;
			trap_hit(trap);
			break;
		}

	case TRAP_OF_MASCULINITY:
		{
			msg_print("Gas sprouts out... you feel yourself transmute.");
			p_ptr->psex = SEX_MALE;
			sp_ptr = &sex_info[p_ptr->psex];
			ident = TRUE;
			trap_hit(trap);
			break;
		}

	case TRAP_OF_NEUTRALITY:
		{
			msg_print("Gas sprouts out... you feel yourself transmute.");
			p_ptr->psex = SEX_NEUTER;
			sp_ptr = &sex_info[p_ptr->psex];
			ident = TRUE;
			trap_hit(trap);
			break;
		}

	case TRAP_OF_AGING:
		{
			msg_print("Colors are scintillating around you. "
			          "You see your past running before your eyes.");
			p_ptr->age += randint((rp_ptr->b_age + rmp_ptr->b_age) / 2);
			ident = TRUE;
			trap_hit(trap);
			break;
		}

	case TRAP_OF_GROWING:
		{
			s16b tmp;

			msg_print("Heavy fumes sprout out... you feel yourself transmute.");
			if (p_ptr->psex == SEX_FEMALE) tmp = rp_ptr->f_b_ht + rmp_ptr->f_b_ht;
			else tmp = rp_ptr->m_b_ht + rmp_ptr->m_b_ht;

			p_ptr->ht += randint(tmp / 4);
			ident = TRUE;
			trap_hit(trap);
			break;
		}

	case TRAP_OF_SHRINKING:
		{
			s16b tmp;

			msg_print("Heavy fumes sprout out... you feel yourself transmute.");
			if (p_ptr->psex == SEX_FEMALE) tmp = rp_ptr->f_b_ht + rmp_ptr->f_b_ht;
			else tmp = rp_ptr->m_b_ht + rmp_ptr->m_b_ht;

			p_ptr->ht -= randint(tmp / 4);
			if (p_ptr->ht <= tmp / 4) p_ptr->ht = tmp / 4;
			ident = TRUE;
			trap_hit(trap);
			break;
		}

		/* Trap of Divine Anger */
	case TRAP_OF_DIVINE_ANGER:
		{
			if (p_ptr->pgod == 0)
			{
				msg_format("Suddenly you feel glad you're a mere %s", spp_ptr->title + c_name);
			}
			else
			{
				cptr name;

				name = deity_info[p_ptr->pgod].name;
				msg_format("You feel you have angered %s.", name);
				inc_piety(p_ptr->pgod, -3000);
			}
			break;
		}

		/* Trap of Divine Wrath */
	case TRAP_OF_DIVINE_WRATH:
		{
			if (p_ptr->pgod == 0)
			{
				msg_format("Suddenly you feel glad you're a mere %s", spp_ptr->title + c_name);
			}
			else
			{
				cptr name;

				name = deity_info[p_ptr->pgod].name;

				msg_format("%s quakes in rage: ``Thou art supremely insolent, mortal!!''", name);
				inc_piety(p_ptr->pgod, -500 * p_ptr->lev);
			}
			break;
		}

		/* Trap of hallucination */
	case TRAP_OF_HALLUCINATION:
		{
			msg_print("Scintillating colors hypnotise you for a moment.");

			set_image(80);
		}
		break;

		/* Bolt Trap */
	case TRAP_OF_ROCKET:
		ident = player_handle_breath_trap(1, GF_ROCKET, trap);
		break;
	case TRAP_OF_NUKE_BOLT:
		ident = player_handle_breath_trap(1, GF_NUKE, trap);
		break;
	case TRAP_OF_HOLY_FIRE:
		ident = player_handle_breath_trap(1, GF_HOLY_FIRE, trap);
		break;
	case TRAP_OF_HELL_FIRE:
		ident = player_handle_breath_trap(1, GF_HELL_FIRE, trap);
		break;
	case TRAP_OF_PSI_BOLT:
		ident = player_handle_breath_trap(1, GF_PSI, trap);
		break;
	case TRAP_OF_PSI_DRAIN:
		ident = player_handle_breath_trap(1, GF_PSI_DRAIN, trap);
		break;

		/* Ball Trap */
	case TRAP_OF_NUKE_BALL:
		ident = player_handle_breath_trap(3, GF_NUKE, TRAP_OF_NUKE_BALL);
		break;
	case TRAP_OF_PSI_BALL:
		ident = player_handle_breath_trap(3, GF_PSI, TRAP_OF_NUKE_BALL);
		break;

	default:
		{
			msg_print(format("Executing unknown trap %d", trap));
		}
	}
	return ident;
}

void player_activate_door_trap(s16b y, s16b x)
{
	cave_type *c_ptr;
	bool ident = FALSE;

	c_ptr = &cave[y][x];

	/* Return if trap or door not found */
	if ((c_ptr->t_idx == 0) ||
	                !(f_info[c_ptr->feat].flags1 & FF1_DOOR)) return;

	/* Disturb */
	disturb(0, 0);

	/* Message */
	msg_print("You found a trap!");

	/* Pick a trap */
	pick_trap(y, x);

	/* Hit the trap */
	ident = player_activate_trap_type(y, x, NULL, -1);
	if (ident)
	{
		t_info[c_ptr->t_idx].ident = TRUE;
		msg_format("You identified that trap as %s.",
		           t_name + t_info[c_ptr->t_idx].name);
	}
}


/*
 * Places a random trap at the given location.
 *
 * The location must be a valid, empty, clean, floor grid.
 */
void place_trap(int y, int x)
{
	s16b trap;
	trap_type *t_ptr;
	int cnt;
	u32b flags;
	cave_type *c_ptr = &cave[y][x];
	dungeon_info_type *d_ptr = &d_info[dungeon_type];

	/* No traps in town or on first level */
	if (dun_level <= 1) return;

	/*
	 * Avoid open doors -- because DOOR flag is added to make much more
	 * important processing faster
	 */
	if (c_ptr->feat == FEAT_OPEN) return;
	if (c_ptr->feat == FEAT_BROKEN) return;

	/* Traps only appears on empty floor */
	if (!cave_floor_grid(c_ptr) &&
	                !(f_info[c_ptr->feat].flags1 & (FF1_DOOR))) return;

	/* Set flags */
	if (f_info[c_ptr->feat].flags1 & FF1_DOOR) flags = FTRAP_DOOR;
	else flags = FTRAP_FLOOR;

	/* Try 100 times */
	cnt = 100;
	while (cnt--)
	{
		trap = randint(max_t_idx - 1);
		t_ptr = &t_info[trap];

		/* No traps below their minlevel */
		if (t_ptr->minlevel > dun_level) continue;

		/* is this a correct trap now?   */
		if (!(t_ptr->flags & flags)) continue;

		/*
		 * Hack -- No trap door at the bottom of dungeon or in flat
		 * (non dungeon) places or on quest levels
		 */
		if ((trap == TRAP_OF_SINKING) &&
		    ((d_ptr->maxdepth == dun_level) ||
		     (dungeon_flags1 & DF1_FLAT) || (is_quest(dun_level))) )
		{
			continue;
		}

		/* How probable is this trap */
		if (rand_int(100) < t_ptr->probability)
		{
			c_ptr->t_idx = trap;
			break;
		}
	}

	return;
}


/*
 * Places a random trap on the given chest.
 *
 * The object must be a valid chest.
 */
void place_trap_object(object_type *o_ptr)
{
	s16b trap;
	trap_type *t_ptr;
	int cnt;

	/* No traps in town or on first level */
	if (dun_level <= 1)
	{
		/* empty chest were already looted, therefore known */
		o_ptr->ident |= IDENT_KNOWN;
		return;
	}

	/* Try 100 times */
	cnt = 100;
	while (cnt--)
	{
		trap = randint(max_t_idx - 1);
		t_ptr = &t_info[trap];

		/* no traps below their minlevel */
		if (t_ptr->minlevel > dun_level) continue;

		/* Is this a correct trap now? */
		if (!(t_ptr->flags & FTRAP_CHEST)) continue;

		/* How probable is this trap */
		if (rand_int(100) < t_ptr->probability)
		{
			o_ptr->pval = trap;
			break;
		}
	}

	return;
}

/* Dangerous trap placing function */
void wiz_place_trap(int y, int x, int idx)
{
	cave_type *c_ptr = &cave[y][x];

	/* Dangerous enough as it is... */
	if (!cave_floor_grid(c_ptr) && (!(f_info[c_ptr->feat].flags1 & FF1_DOOR))) return;

	c_ptr->t_idx = idx;
}

/*
 * Here begin monster traps code
 */

/*
 * Hook to determine if an object is a device
 */
static bool item_tester_hook_device(object_type *o_ptr)
{
	if (((o_ptr->tval == TV_ROD_MAIN) && (o_ptr->pval != 0)) ||
	                (o_ptr->tval == TV_STAFF) ||
	                (o_ptr->tval == TV_WAND)) return (TRUE);

	/* Assume not */
	return (FALSE);
}

/*
 * Hook to determine if an object is a potion
 */
static bool item_tester_hook_potion(object_type *o_ptr)
{
	if ((o_ptr->tval == TV_POTION) ||
	                (o_ptr->tval == TV_POTION2)) return (TRUE);

	/* Assume not */
	return (FALSE);
}

/*
 * The trap setting code for rogues -MWK-
 *
 * Also, it will fail or give weird results if the tvals are resorted!
 */
void do_cmd_set_trap(void)
{
	int item_kit, item_load, i;
	int num;

	object_type *o_ptr, *j_ptr, *i_ptr;

	cptr q, s, c;

	object_type object_type_body;

	u32b f1, f2, f3, f4, f5, esp;

	/* Check some conditions */
	if (p_ptr->blind)
	{
		msg_print("You can't see anything.");
		return;
	}
	if (no_lite())
	{
		msg_print("You don't dare to set a trap in the darkness.");
		return;
	}
	if (p_ptr->confused)
	{
		msg_print("You are too confused!");
		return;
	}

	/* Only set traps on clean floor grids */
	if (!cave_clean_bold(p_ptr->py, p_ptr->px))
	{
		msg_print("You cannot set a trap on this.");
		return;
	}

	/* Restrict choices to trapkits */
	item_tester_tval = TV_TRAPKIT;

	/* Get an item */
	q = "Use which trapping kit? ";
	s = "You have no trapping kits.";
	if (!get_item(&item_kit, q, s, USE_INVEN)) return;

	o_ptr = &p_ptr->inventory[item_kit];

	/* Trap kits need a second object */
	switch (o_ptr->sval)
	{
	case SV_TRAPKIT_BOW:
		item_tester_tval = TV_ARROW;
		break;
	case SV_TRAPKIT_XBOW:
		item_tester_tval = TV_BOLT;
		break;
	case SV_TRAPKIT_SLING:
		item_tester_tval = TV_SHOT;
		break;
	case SV_TRAPKIT_POTION:
		item_tester_hook = item_tester_hook_potion;
		break;
	case SV_TRAPKIT_SCROLL:
		item_tester_tval = TV_SCROLL;
		break;
	case SV_TRAPKIT_DEVICE:
		item_tester_hook = item_tester_hook_device;
		break;
	default:
		msg_print("Unknown trapping kit type!");
		break;
	}

	/* Get the second item */
	q = "Load with what? ";
	s = "You have nothing to load that trap with.";
	if (!get_item(&item_load, q, s, USE_INVEN)) return;

	/* Get the second object */
	j_ptr = &p_ptr->inventory[item_load];

	/* Assume a single object */
	num = 1;

	/* In some cases, take multiple objects to load */
	if (o_ptr->sval != SV_TRAPKIT_DEVICE)
	{
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		if ((f3 & TR3_XTRA_SHOTS) && (o_ptr->pval > 0)) num += o_ptr->pval;

		if (f2 & (TRAP2_AUTOMATIC_5 | TRAP2_AUTOMATIC_99)) num = 99;

		if (num > j_ptr->number) num = j_ptr->number;

		c = format("How many (1-%d)? ", num);

		/* Ask for number of items to use */
		num = get_quantity(c, num);
	}

	/* Canceled */
	if (!num) return;

	/* Take a turn */
	energy_use = 100;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain local object for trap content */
	object_copy(i_ptr, j_ptr);

	/* Set number */
	i_ptr->number = num;

	/* Drop it here */
	cave[p_ptr->py][p_ptr->px].special = floor_carry(p_ptr->py, p_ptr->px, i_ptr);

	/* Obtain local object for trap trigger kit */
	object_copy(i_ptr, o_ptr);

	/* Set number */
	i_ptr->number = 1;

	/* Drop it here */
	cave[p_ptr->py][p_ptr->px].special2 = floor_carry(p_ptr->py, p_ptr->px, i_ptr);

	/* Modify, Describe, Optimize */
	inven_item_increase(item_kit, -1);
	inven_item_describe(item_kit);
	inven_item_increase(item_load, -num);
	inven_item_describe(item_load);

	for (i = 0; i < INVEN_WIELD; i++)
	{
		if (inven_item_optimize(i)) break;
	}
	for (i = 0; i < INVEN_WIELD; i++)
	{
		inven_item_optimize(i);
	}

	/* Actually set the trap */
	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_MON_TRAP);
}

/*
 * Monster hitting a rod trap -MWK-
 *
 * Return TRUE if the monster died
 */
bool mon_hit_trap_aux_rod(int m_idx, object_type *o_ptr)
{
	int dam = 0, typ = 0;
	int rad = 0;
	monster_type *m_ptr = &m_list[m_idx];
	int y = m_ptr->fy;
	int x = m_ptr->fx;

	/* Depend on rod type */
	switch (o_ptr->pval)
	{
	case SV_ROD_DETECT_TRAP:
		m_ptr->smart |= SM_NOTE_TRAP;
		break;
	case SV_ROD_DETECTION:
		m_ptr->smart |= SM_NOTE_TRAP;
		break;
	case SV_ROD_ILLUMINATION:
		typ = GF_LITE_WEAK;
		dam = damroll(2, 15);
		rad = 3;
		lite_room(y, x);
		break;
	case SV_ROD_CURING:
		typ = GF_OLD_HEAL;
		dam = damroll(3, 4);  /* and heal conf? */
		break;
	case SV_ROD_HEALING:
		typ = GF_OLD_HEAL;
		dam = 300;
		break;
	case SV_ROD_SPEED:
		typ = GF_OLD_SPEED;
		dam = 50;
		break;
	case SV_ROD_TELEPORT_AWAY:
		typ = GF_AWAY_ALL;
		dam = MAX_SIGHT * 5;
		break;
	case SV_ROD_DISARMING:
		break;
	case SV_ROD_LITE:
		typ = GF_LITE_WEAK;
		dam = damroll(6, 8);
		break;
	case SV_ROD_SLEEP_MONSTER:
		typ = GF_OLD_SLEEP;
		dam = 50;
		break;
	case SV_ROD_SLOW_MONSTER:
		typ = GF_OLD_SLOW;
		dam = 50;
		break;
	case SV_ROD_DRAIN_LIFE:
		typ = GF_OLD_DRAIN;
		dam = 75;
		break;
	case SV_ROD_POLYMORPH:
		typ = GF_OLD_POLY;
		dam = 50;
		break;
	case SV_ROD_ACID_BOLT:
		typ = GF_ACID;
		dam = damroll(6, 8);
		break;
	case SV_ROD_ELEC_BOLT:
		typ = GF_ELEC;
		dam = damroll(3, 8);
		break;
	case SV_ROD_FIRE_BOLT:
		typ = GF_FIRE;
		dam = damroll(8, 8);
		break;
	case SV_ROD_COLD_BOLT:
		typ = GF_COLD;
		dam = damroll(5, 8);
		break;
	case SV_ROD_ACID_BALL:
		typ = GF_ACID;
		dam = 60;
		rad = 2;
		break;
	case SV_ROD_ELEC_BALL:
		typ = GF_ELEC;
		dam = 32;
		rad = 2;
		break;
	case SV_ROD_FIRE_BALL:
		typ = GF_FIRE;
		dam = 72;
		rad = 2;
		break;
	case SV_ROD_COLD_BALL:
		typ = GF_COLD;
		dam = 48;
		rad = 2;
		break;
	default:
		return (FALSE);
	}

	/* Actually hit the monster */
	if (typ) (void) project( -2, rad, y, x, dam, typ, PROJECT_KILL | PROJECT_ITEM | PROJECT_JUMP);
	return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
}

/*
 * Monster hitting a staff trap -MWK-
 *
 * Return TRUE if the monster died
 */
bool mon_hit_trap_aux_staff(int m_idx, object_type *o_ptr)
{
	/* Monster pointer and position */
	monster_type *m_ptr = &m_list[m_idx];
	int y = m_ptr->fy;
	int x = m_ptr->fx;

	/* sval and base level of the staff */
	int sval = o_ptr->sval;

	/* Damage amount, type, and radius */
	int dam = 0, typ = 0;
	int rad = 0;

	/* Depend on staff type */
	switch (sval)
	{
#if 0 /*must be tested*/
	case SV_STAFF_IDENTIFY:
	case SV_STAFF_MANA:
	case SV_STAFF_REMOVE_CURSES:
	case SV_STAFF_REVEAL_WAYS:
	case SV_STAFF_SENSE_MONSTER:
	case SV_STAFF_VISION:
	case SV_STAFF_DISARM:
		return (FALSE);

	case SV_STAFF_LIGHT:
		lite_room(y, x);
		typ = GF_LITE_WEAK;
		dam = damroll(2, 8);
		rad = 2;
		break;

	case SV_STAFF_SUMMON:
		for (k = 0; k < randint(4) ; k++)
			(void)summon_specific(y, x, dun_level, 0);
		return (FALSE);

	case SV_STAFF_TELEPORTATION:
		typ = GF_AWAY_ALL;
		dam = 100 + 2 * level;
		break;

	case SV_STAFF_HEALING:
		typ = GF_OLD_HEAL;
		dam = m_ptr->maxhp * (150 + 7 * level) / 1000;
		break;

	case SV_STAFF_SHAKE:
		earthquake(y, x, 4 + level / 5);  /* was 10 */
		return (FALSE);

	case SV_STAFF_RECOVERY:
		m_ptr->bleeding = 0;
		m_ptr->poisoned = 0;
		return (FALSE);

	case SV_STAFF_GENOCIDE:
		{
			monster_race *r_ptr = &r_info[m_ptr->r_idx];
			genocide_aux(FALSE, r_ptr->d_char);
			/* although there's no point in a multiple genocide trap... */
			return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
		}

	case SV_STAFF_SENSE_HIDDEN:
		m_ptr->smart |= SM_NOTE_TRAP;
		return (FALSE);

	case SV_STAFF_WISH:
		acquirement(y, x, randint(2) + 1, TRUE, FALSE);
		return (FALSE);

	case SV_STAFF_MITHRANDIR:
		typ = GF_HOLY_FIRE;
		dam = 50 + 6 * level;
		rad = 9;  /* instead of LOS */

		/* How to implement these ? */
	case SV_STAFF_FIERY_SHIELD:
	case SV_STAFF_WINGS_WIND:
	case SV_STAFF_PROBABILITY_TRAVEL:

#endif

	default:
		return (FALSE);
	}

	/* Actually hit the monster */
	(void) project( -2, rad, y, x, dam, typ, PROJECT_KILL | PROJECT_ITEM | PROJECT_JUMP);
	return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
}

/*
 * Monster hitting a scroll trap -MWK-
 *
 * Return TRUE if the monster died
 */
bool mon_hit_trap_aux_scroll(int m_idx, int sval)
{
	monster_type *m_ptr = &m_list[m_idx];
	int dam = 0, typ = 0;
	int rad = 0;
	int y = m_ptr->fy;
	int x = m_ptr->fx;
	int k;

	/* Depend on scroll type */
	switch (sval)
	{
	case SV_SCROLL_CURSE_ARMOR:
	case SV_SCROLL_CURSE_WEAPON:
	case SV_SCROLL_TRAP_CREATION:  /* these don't work :-( */
	case SV_SCROLL_WORD_OF_RECALL:  /* should these? */
	case SV_SCROLL_IDENTIFY:
	case SV_SCROLL_STAR_IDENTIFY:
	case SV_SCROLL_MAPPING:
	case SV_SCROLL_DETECT_GOLD:
	case SV_SCROLL_DETECT_ITEM:
	case SV_SCROLL_REMOVE_CURSE:
	case SV_SCROLL_STAR_REMOVE_CURSE:
	case SV_SCROLL_ENCHANT_ARMOR:
	case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
	case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
	case SV_SCROLL_STAR_ENCHANT_ARMOR:
	case SV_SCROLL_STAR_ENCHANT_WEAPON:
	case SV_SCROLL_RECHARGING:
	case SV_SCROLL_DETECT_DOOR:
	case SV_SCROLL_DETECT_INVIS:
	case SV_SCROLL_SATISFY_HUNGER:
	case SV_SCROLL_RUNE_OF_PROTECTION:
	case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
	case SV_SCROLL_PROTECTION_FROM_EVIL:
		return (FALSE);
	case SV_SCROLL_DARKNESS:
		unlite_room(y, x);
		typ = GF_DARK_WEAK;
		dam = 10;
		rad = 3;
		break;
	case SV_SCROLL_AGGRAVATE_MONSTER:
		aggravate_monsters(m_idx);
		return (FALSE);
	case SV_SCROLL_SUMMON_MONSTER:
		for (k = 0; k < randint(3) ; k++) summon_specific(y, x, dun_level, 0);
		return (FALSE);
	case SV_SCROLL_SUMMON_UNDEAD:
		for (k = 0; k < randint(3) ; k++) summon_specific(y, x, dun_level, SUMMON_UNDEAD);
		return (FALSE);
	case SV_SCROLL_PHASE_DOOR:
		typ = GF_AWAY_ALL;
		dam = 10;
		break;
	case SV_SCROLL_TELEPORT:
		typ = GF_AWAY_ALL;
		dam = 100;
		break;
	case SV_SCROLL_TELEPORT_LEVEL:
		delete_monster(y, x);
		return (TRUE);
	case SV_SCROLL_LIGHT:
		lite_room(y, x);
		typ = GF_LITE_WEAK;
		dam = damroll(2, 8);
		rad = 2;
		break;
	case SV_SCROLL_DETECT_TRAP:
		m_ptr->smart |= SM_NOTE_TRAP;
		return (FALSE);
	case SV_SCROLL_BLESSING:
		typ = GF_HOLY_FIRE;
		dam = damroll(1, 4);
		break;
	case SV_SCROLL_HOLY_CHANT:
		typ = GF_HOLY_FIRE;
		dam = damroll(2, 4);
		break;
	case SV_SCROLL_HOLY_PRAYER:
		typ = GF_HOLY_FIRE;
		dam = damroll(4, 4);
		break;
	case SV_SCROLL_MONSTER_CONFUSION:
		typ = GF_OLD_CONF;
		dam = damroll(5, 10);
		break;
	case SV_SCROLL_STAR_DESTRUCTION:
		destroy_area(y, x, 15, TRUE, FALSE);
		return (FALSE);
	case SV_SCROLL_DISPEL_UNDEAD:
		typ = GF_DISP_UNDEAD;
		rad = 5;
		dam = 60;
		break;
	case SV_SCROLL_GENOCIDE:
		{
			monster_race *r_ptr = &r_info[m_ptr->r_idx];
			genocide_aux(FALSE, r_ptr->d_char);
			/* although there's no point in a multiple genocide trap... */
			return (!(r_ptr->flags1 & RF1_UNIQUE));
		}
	case SV_SCROLL_MASS_GENOCIDE:
		for (k = 0; k < 8; k++)
			delete_monster(y + ddy[k], x + ddx[k]);
		delete_monster(y, x);
		return (TRUE);
	case SV_SCROLL_ACQUIREMENT:
		acquirement(y, x, 1, TRUE, FALSE);
		return (FALSE);
	case SV_SCROLL_STAR_ACQUIREMENT:
		acquirement(y, x, randint(2) + 1, TRUE, FALSE);
		return (FALSE);
	default:
		return (FALSE);
	}

	/* Actually hit the monster */
	(void) project( -2, rad, y, x, dam, typ, PROJECT_KILL | PROJECT_ITEM | PROJECT_JUMP);
	return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
}

/*
 * Monster hitting a wand trap -MWK-
 *
 * Return TRUE if the monster died
 */
bool mon_hit_trap_aux_wand(int m_idx, object_type *o_ptr)
{
	/* Monster pointer and position */
	monster_type *m_ptr = &m_list[m_idx];
	int y = m_ptr->fy;
	int x = m_ptr->fx;

	/* sval and bonus level of the wand */
	int sval = o_ptr->sval;

	/* Damage amount, type, and radius */
	int dam = 0, typ = 0;
	int rad = 0;

	/* Depend on wand type */
	switch (sval)
	{

#if 0 /* must be tested */

	case SV_WAND_MANATHRUST:
		typ = GF_MANA;
		dam = damroll(3 + level, 1 + 2 * level / 5 );
		break;

	case SV_WAND_FIREFLASH:
		typ = GF_FIRE;
		dam = 20 + level * 10;
		rad = 2 + level / 10;
		break;

	case SV_WAND_NOXIOUS_CLOUD:
		typ = GF_POIS;
		dam = 7 + 3 * level;
		rad = 3;
		break;

	case SV_WAND_THUNDERSTORM:
		typ = GF_ELEC;  /* GF_LITE, GF_SOUND ??? */
		dam = damroll(5 + level / 5, 10 + level / 2);
		break;

	case SV_WAND_DIG:
	case SV_WAND_THRAIN:
		typ = GF_KILL_WALL;
		dam = 20 + randint(30);
		break;

	case SV_WAND_STRIKE:
		typ = GF_FORCE;
		dam = 50 + level;
		break;

	case SV_WAND_TELEPORT_AWAY:
		typ = GF_AWAY_ALL;
		dam = MAX_SIGHT * 5;
		break;

	case SV_WAND_SUMMON_ANIMAL:
		summon_specific(y, x, dun_level, SUMMON_ANIMAL);  /* friendly ?*/
		return (FALSE);

	case SV_WAND_SLOW_MONSTER:
		typ = GF_OLD_SLOW;
		dam = 40 + 16 * level / 5;
		break;

	case SV_WAND_BANISHMENT:
		typ = GF_AWAY_ALL;
		dam = 40 + 16 * level / 5;
		rad = 9;  /* instead of LOS */
		break;

	case SV_WAND_CHARM:
		typ = GF_CHARM;
		dam = 10 + 3 * level;
		break;

	case SV_WAND_CONFUSE:
		typ = GF_OLD_CONF;
		dam = 10 + 3 * level;
		break;

	case SV_WAND_HEAL_MONSTER:
		typ = GF_OLD_HEAL;
		dam = 20 + 38 * level / 5;
		break;

	case SV_WAND_SPEED:
	case SV_WAND_HASTE_MONSTER:
		typ = GF_OLD_SPEED;
		dam = damroll(5, 10);
		break;

	case SV_WAND_STONE_PRISON:
		wall_stone(y, x);
		return (FALSE);

	case SV_WAND_DISPERSE_MAGIC:
		m_ptr->confused = 0;
		m_ptr->mspeed = 0;
		return (FALSE);

	case SV_WAND_ICE_STORM:
		typ = GF_COLD;
		dam = 80 + 4 * level;
		rad = 1 + 3 * level / 50;
		break;

	case SV_WAND_TIDAL_WAVE:
		typ = GF_WAVE;
		dam = 40 + 4 * level;
		rad = 6 + level / 5;
		break;

	case SV_WAND_FIREWALL:
		typ = GF_FIRE;
		dam = 40 + 3 * level;
		rad = 1;  /*instead of beam*/

		/* Not sure about these */
	case SV_WAND_MAGELOCK:
	case SV_WAND_DEMON_BLADE:
	case SV_WAND_POISON_BLOOD:

#endif

	default:
		return (FALSE);
	}

	/* Actually hit the monster */
	(void) project( -2, rad, y, x, dam, typ, PROJECT_KILL | PROJECT_ITEM | PROJECT_JUMP);
	return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
}

/*
 * Monster hitting a potions trap -MWK-
 *
 * Return TRUE if the monster died
 */
bool mon_hit_trap_aux_potion(int m_idx, object_type *o_ptr)
{
	monster_type *m_ptr = &m_list[m_idx];
	int dam = 0, typ = 0;
	int y = m_ptr->fy;
	int x = m_ptr->fx;
	int sval = o_ptr->sval;

	/* Depend on potion type */
	if (o_ptr->tval == TV_POTION)
	{
		switch (sval)
		{
			/* Nothing happens */
		case SV_POTION_WATER:
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SLIME_MOLD:
		case SV_POTION_SALT_WATER:
		case SV_POTION_DEC_STR:
		case SV_POTION_DEC_INT:
		case SV_POTION_DEC_WIS:
		case SV_POTION_DEC_DEX:
		case SV_POTION_DEC_CON:
		case SV_POTION_DEC_CHR:
		case SV_POTION_INFRAVISION:
		case SV_POTION_DETECT_INVIS:
		case SV_POTION_SLOW_POISON:
		case SV_POTION_CURE_POISON:
		case SV_POTION_RESIST_HEAT:
		case SV_POTION_RESIST_COLD:
		case SV_POTION_RESTORE_MANA:
		case SV_POTION_RESTORE_EXP:
		case SV_POTION_RES_STR:
		case SV_POTION_RES_INT:
		case SV_POTION_RES_WIS:
		case SV_POTION_RES_DEX:
		case SV_POTION_RES_CON:
		case SV_POTION_RES_CHR:
		case SV_POTION_INC_STR:
		case SV_POTION_INC_INT:
		case SV_POTION_INC_WIS:
		case SV_POTION_INC_DEX:
		case SV_POTION_INC_CON:
		case SV_POTION_INC_CHR:
		case SV_POTION_AUGMENTATION:
		case SV_POTION_RUINATION: 	/* ??? */
		case SV_POTION_ENLIGHTENMENT:
		case SV_POTION_STAR_ENLIGHTENMENT:
		case SV_POTION_SELF_KNOWLEDGE:
			return (FALSE);

		case SV_POTION_EXPERIENCE:
			if (m_ptr->level < MONSTER_LEVEL_MAX)
			{
				m_ptr->exp = MONSTER_EXP(m_ptr->level + 1);
				monster_check_experience(m_idx, FALSE);
			}
			return (FALSE);
		case SV_POTION_SLOWNESS:
			typ = GF_OLD_SLOW;
			dam = damroll(4, 6);
			break;
		case SV_POTION_POISON:
			typ = GF_POIS;
			dam = damroll(8, 6);
			break;
		case SV_POTION_CONFUSION:
			typ = GF_CONFUSION;
			dam = damroll(4, 6);
			break;
		case SV_POTION_BLINDNESS:
			typ = GF_DARK;
			dam = 10;
			break;
		case SV_POTION_SLEEP:
			typ = GF_OLD_SLEEP;
			dam = damroll (4, 6);
			break;
		case SV_POTION_LOSE_MEMORIES:
			typ = GF_OLD_CONF;
			dam = damroll(10, 10);
			break;
		case SV_POTION_DETONATIONS:
			typ = GF_DISINTEGRATE;
			dam = damroll(20, 20);
			break;
		case SV_POTION_DEATH:
			typ = GF_NETHER;
			dam = damroll(100, 20);
			break;
		case SV_POTION_BOLDNESS:
			m_ptr->monfear = 0;
			return (FALSE);
		case SV_POTION_SPEED:
			typ = GF_OLD_SPEED;
			dam = damroll(5, 10);
			break;
		case SV_POTION_HEROISM:
		case SV_POTION_BESERK_STRENGTH:
			m_ptr->monfear = 0;
			typ = GF_OLD_HEAL;
			dam = damroll(2, 10);
			break;
		case SV_POTION_CURE_LIGHT:
			typ = GF_OLD_HEAL;
			dam = damroll(3, 4);
			break;
		case SV_POTION_CURE_SERIOUS:
			typ = GF_OLD_HEAL;
			dam = damroll(4, 6);
			break;
		case SV_POTION_CURE_CRITICAL:
			typ = GF_OLD_HEAL;
			dam = damroll(6, 8);
			break;
		case SV_POTION_HEALING:
			typ = GF_OLD_HEAL;
			dam = 300;
			break;
		case SV_POTION_STAR_HEALING:
			typ = GF_OLD_HEAL;
			dam = 1000;
			break;
		case SV_POTION_LIFE:
			{
				monster_race *r_ptr = &r_info[m_ptr->r_idx];
				if (r_ptr->flags3 & RF3_UNDEAD)
				{
					typ = GF_HOLY_FIRE;
					dam = damroll(20, 20);
				}
				else
				{
					typ = GF_OLD_HEAL;
					dam = 5000;
				}
				break;
			}
		default:
			return (FALSE);

		}
	}
	else
	{}

	/* Actually hit the monster */
	(void) project_m( -2, 0, y, x, dam, typ);
	return (cave[y][x].m_idx == 0 ? TRUE : FALSE);
}

/*
 * Monster hitting a monster trap -MWK-
 * Returns True if the monster died, false otherwise
 */
bool mon_hit_trap(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
#if 0 /* DGDGDGDG */
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];
#endif

	object_type *kit_o_ptr, *load_o_ptr, *j_ptr;

	u32b f1, f2, f3, f4, f5, esp;

	object_type object_type_body;

	int mx = m_ptr->fx;
	int my = m_ptr->fy;

	int difficulty;
	int smartness;

	char m_name[80];

	bool notice = FALSE;
	bool disarm = FALSE;
	bool remove = FALSE;
	bool dead = FALSE;
	bool fear = FALSE;
	s32b special = 0;

	int dam, chance, shots;
	int mul = 0;
	int breakage = -1;

	int cost = 0;

	/* Get the trap objects */
	kit_o_ptr = &o_list[cave[my][mx].special2];
	load_o_ptr = &o_list[cave[my][mx].special];
	j_ptr = &object_type_body;

	/* Get trap properties */
	object_flags(kit_o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Can set off check */
	/* Ghosts only set off Ghost traps */
	if ((r_ptr->flags2 & RF2_PASS_WALL) && !(f2 & TRAP2_KILL_GHOST)) return (FALSE);

	/* Some traps are specialized to some creatures */
	if (f2 & TRAP2_ONLY_MASK)
	{
		bool affect = FALSE;
		if ((f2 & TRAP2_ONLY_DRAGON) && (r_ptr->flags3 & RF3_DRAGON)) affect = TRUE;
		if ((f2 & TRAP2_ONLY_DEMON) && (r_ptr->flags3 & RF3_DEMON)) affect = TRUE;
		if ((f2 & TRAP2_ONLY_UNDEAD) && (r_ptr->flags3 & RF3_UNDEAD)) affect = TRUE;
		if ((f2 & TRAP2_ONLY_EVIL) && (r_ptr->flags3 & RF3_EVIL)) affect = TRUE;
		if ((f2 & TRAP2_ONLY_ANIMAL) && (r_ptr->flags3 & RF3_ANIMAL)) affect = TRUE;

		/* Don't set it off if forbidden */
		if (!affect) return (FALSE);
	}

	/* Get detection difficulty */
	difficulty = 25;

	/* Some traps are well-hidden */
	if (f1 & TR1_STEALTH)
	{
		difficulty += 10 * (kit_o_ptr->pval);
	}

	/* Get monster smartness for trap detection */
	/* Higher level monsters are smarter */
	smartness = r_ptr->level;

	/* Smart monsters are better at detecting traps */
	if (r_ptr->flags2 & RF2_SMART) smartness += 10;

	/* Some monsters are great at detecting traps */
#if 0 /* DGDGDGDG */
	if (r_ptr->flags2 & RF2_NOTICE_TRAP) smartness += 20;
#endif
	/* Some monsters have already noticed one of out traps */
	if (m_ptr->smart & SM_NOTE_TRAP) smartness += 20;

	/* Stupid monsters are no good at detecting traps */
	if (r_ptr->flags2 & (RF2_STUPID | RF2_EMPTY_MIND)) smartness = -150;

	/* Check if the monster notices the trap */
	if (randint(300) > (difficulty - smartness + 150)) notice = TRUE;

	/* Disarm check */
	if (notice)
	{
		/* The next traps will be easier to spot! */
		m_ptr->smart |= SM_NOTE_TRAP;

		/* Tell the player about it */
#if 0 /* DGDGDGDG */
		if (m_ptr->ml) l_ptr->r_flags2 |= (RF2_NOTICE_TRAP & r_ptr->flags2);
#endif
		/* Get trap disarming difficulty */
		difficulty = (kit_o_ptr->ac + kit_o_ptr->to_a);

		/* Get monster disarming ability */
		/* Higher level monsters are better */
		smartness = r_ptr->level / 5;

		/* Some monsters are great at disarming */
#if 0 /* DGDGDGDG */
		if (r_ptr->flags2 & RF2_DISARM_TRAP) smartness += 20;
#endif
		/* After disarming one trap, the next is easier */
#if 0 /* DGDGDGDG */
		if (m_ptr->status & STATUS_DISARM_TRAP) smartness += 20;
#endif
		/* Smart monsters are better at disarming */
		if (r_ptr->flags2 & RF2_SMART) smartness *= 2;

		/* Stupid monsters never disarm traps */
		if (r_ptr->flags2 & RF2_STUPID) smartness = -150;

		/* Nonsmart animals never disarm traps */
		if ((r_ptr->flags3 & RF3_ANIMAL) && !(r_ptr->flags2 & RF2_SMART)) smartness = -150;

		/* Check if the monster disarms the trap */
		if (randint(120) > (difficulty - smartness + 80)) disarm = TRUE;
	}

	/* If disarmed, remove the trap and print a message */
	if (disarm)
	{
		remove = TRUE;

		/* Next time disarming will be easier */
#if 0 /* DGDGDGDG */
		m_ptr->status |= STATUS_DISARM_TRAP;
#endif
		if (m_ptr->ml)
		{
			/* Get the name */
			monster_desc(m_name, m_ptr, 0);

			/* Tell the player about it */
#if 0 /* DGDGDGDG */
			l_ptr->r_flags2 |= (RF2_DISARM_TRAP & r_ptr->flags2);
#endif
			/* Print a message */
			msg_format("%^s disarms a trap!", m_name);
		}
	}

	/* Otherwise, activate the trap! */
	else
	{
		/* Message for visible monster */
		if (m_ptr->ml)
		{
			/* Get the name */
			monster_desc(m_name, m_ptr, 0);

			/* Print a message */
			msg_format("%^s sets off a trap!", m_name);
		}
		else
		{
			/* No message if monster isn't visible ? */
		}

		/* Next time be more careful */
		if (randint(100) < 80) m_ptr->smart |= SM_NOTE_TRAP;

		/* Actually activate the trap */
		switch (kit_o_ptr->sval)
		{
		case SV_TRAPKIT_BOW:
		case SV_TRAPKIT_XBOW:
		case SV_TRAPKIT_SLING:
			{
				/* Get number of shots */
				shots = 1;
				if (f3 & TR3_XTRA_SHOTS) shots += kit_o_ptr->pval;
				if (shots <= 0) shots = 1;
				if (shots > load_o_ptr->number) shots = load_o_ptr->number;

				while (shots-- && !dead)
				{
					/* Total base damage */
					dam = damroll(load_o_ptr->dd, load_o_ptr->ds) + load_o_ptr->to_d + kit_o_ptr->to_d;

					/* Total hit probability */
					chance = (kit_o_ptr->to_h + load_o_ptr->to_h + 20) * BTH_PLUS_ADJ;

					/* Damage multiplier */
					if (kit_o_ptr->sval == SV_TRAPKIT_BOW) mul = 3;
					if (kit_o_ptr->sval == SV_TRAPKIT_XBOW) mul = 4;
					if (kit_o_ptr->sval == SV_TRAPKIT_SLING) mul = 2;
					if (f3 & TR3_XTRA_MIGHT) mul += kit_o_ptr->pval;
					if (mul < 0) mul = 0;

					/* Multiply damage */
					dam *= mul;

					/* Check if we hit the monster */
					if (test_hit_fire(chance, r_ptr->ac, TRUE))
					{
						/* Assume a default death */
						cptr note_dies = " dies.";

						/* Some monsters get "destroyed" */
						if ((r_ptr->flags3 & (RF3_DEMON)) ||
						                (r_ptr->flags3 & (RF3_UNDEAD)) ||
						                (r_ptr->flags2 & (RF2_STUPID)) ||
						                (strchr("Evg", r_ptr->d_char)))
						{
							/* Special note at death */
							note_dies = " is destroyed.";
						}

						/* Message if visible */
						if (m_ptr->ml)
						{
							/* describe the monster (again, just in case :-) */
							monster_desc(m_name, m_ptr, 0);

							/* Print a message */
							msg_format("%^s is hit by a missile.", m_name);
						}

						/* Apply slays, brand, critical hits */
						dam = tot_dam_aux(load_o_ptr, dam, m_ptr, &special);
						dam = critical_shot(load_o_ptr->weight, load_o_ptr->to_h, dam, SKILL_ARCHERY);

						/* No negative damage */
						if (dam < 0) dam = 0;

						/* Hit the monster, check for death */
						if (mon_take_hit(m_idx, dam, &fear, note_dies))
						{
							/* Dead monster */
							dead = TRUE;
						}

						/* No death */
						else
						{
							/* Message */
							message_pain(m_idx, dam);

							if (special) attack_special(m_ptr, special, dam);

							/* Take note */
							if (fear && m_ptr->ml)
							{
								/* Message */
								msg_format("%^s flees in terror!", m_name);
							}
						}

					}

					/* Exploding ammo */
					if (load_o_ptr->pval2 != 0)
					{
						int rad = 0;
						int dam = (damroll(load_o_ptr->dd, load_o_ptr->ds) + load_o_ptr->to_d)*2;
						int flag = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL |
						           PROJECT_JUMP;

						switch (load_o_ptr->sval)
						{
						case SV_AMMO_LIGHT:
							rad = 2;
							dam /= 2;
							break;
						case SV_AMMO_NORMAL:
							rad = 3;
							break;
						case SV_AMMO_HEAVY:
							rad = 4;
							dam *= 2;
							break;
						}

						project(0, rad, my, mx, dam, load_o_ptr->pval2, flag);

						breakage = 100;
					}
					else
					{
						breakage = breakage_chance(load_o_ptr);
					}

					/* Copy and decrease ammo */
					object_copy(j_ptr, load_o_ptr);

					j_ptr->number = 1;

					load_o_ptr->number--;

					if (load_o_ptr->number <= 0)
					{
						remove = TRUE;
						delete_object_idx(kit_o_ptr->next_o_idx);
						kit_o_ptr->next_o_idx = 0;
					}

					/* Drop (or break) near that location */
					drop_near(j_ptr, breakage, my, mx);

				}

				break;
			}

		case SV_TRAPKIT_POTION:
			{
				/* Get number of shots */
				shots = 1;
				if (f3 & TR3_XTRA_SHOTS) shots += kit_o_ptr->pval;
				if (shots <= 0) shots = 1;
				if (shots > load_o_ptr->number) shots = load_o_ptr->number;

				while (shots-- && !dead)
				{

					/* Message if visible */
					if (m_ptr->ml)
					{
						/* describe the monster (again, just in case :-) */
						monster_desc(m_name, m_ptr, 0);

						/* Print a message */
						msg_format("%^s is hit by fumes.", m_name);
					}

					/* Get the potion effect */
					dead = mon_hit_trap_aux_potion(m_idx, load_o_ptr);

					/* Copy and decrease ammo */
					object_copy(j_ptr, load_o_ptr);

					j_ptr->number = 1;

					load_o_ptr->number--;

					if (load_o_ptr->number <= 0)
					{
						remove = TRUE;
						delete_object_idx(kit_o_ptr->next_o_idx);
						kit_o_ptr->next_o_idx = 0;
					}
				}

				break;
			}

		case SV_TRAPKIT_SCROLL:
			{
				/* Get number of shots */
				shots = 1;
				if (f3 & TR3_XTRA_SHOTS) shots += kit_o_ptr->pval;
				if (shots <= 0) shots = 1;
				if (shots > load_o_ptr->number) shots = load_o_ptr->number;

				while (shots-- && !dead)
				{

					/* Message if visible */
					if (m_ptr->ml)
					{
						/* describe the monster (again, just in case :-) */
						monster_desc(m_name, m_ptr, 0);

						/* Print a message */
						msg_format("%^s activates a spell!", m_name);
					}

					/* Get the potion effect */
					dead = mon_hit_trap_aux_scroll(m_idx, load_o_ptr->sval);

					/* Copy and decrease ammo */
					object_copy(j_ptr, load_o_ptr);

					j_ptr->number = 1;

					load_o_ptr->number--;

					if (load_o_ptr->number <= 0)
					{
						remove = TRUE;
						delete_object_idx(kit_o_ptr->next_o_idx);
						kit_o_ptr->next_o_idx = 0;
					}
				}

				break;
			}

		case SV_TRAPKIT_DEVICE:
			{
				if (load_o_ptr->tval == TV_ROD_MAIN)
				{
					/* Extract mana cost of the rod tip */
					u32b tf1, tf2, tf3, tf4, tf5, tesp;
					object_kind *tip_o_ptr = &k_info[lookup_kind(TV_ROD, load_o_ptr->pval)];
					object_flags(load_o_ptr, &tf1, &tf2, &tf3, &tf4, &tf5, &tesp);
					cost = (tf4 & TR4_CHEAPNESS) ? tip_o_ptr->pval / 2 : tip_o_ptr->pval;
					if (cost <= 0) cost = 1;
				}

				/* Get number of shots */
				shots = 1;
				if (f3 & TR3_XTRA_SHOTS) shots += kit_o_ptr->pval;
				if (shots <= 0) shots = 1;

				if (load_o_ptr->tval == TV_ROD_MAIN)
				{
					if (shots > load_o_ptr->timeout / cost) shots = load_o_ptr->timeout / cost;
				}
				else
				{
					if (shots > load_o_ptr->pval) shots = load_o_ptr->pval;
				}

				while (shots-- && !dead)
				{
#if 0
					/* Message if visible */
					if (m_ptr->ml)
					{
						/* describe the monster (again, just in case :-) */
						monster_desc(m_name, m_ptr, 0);

						/* Print a message */
						msg_format("%^s is hit by some magic.", m_name);
					}
#endif
					/* Get the effect effect */
					switch (load_o_ptr->tval)
					{
					case TV_ROD_MAIN:
						dead = mon_hit_trap_aux_rod(m_idx, load_o_ptr);
						break;
					case TV_WAND:
						dead = mon_hit_trap_aux_wand(m_idx, load_o_ptr);
						break;
					case TV_STAFF:
						dead = mon_hit_trap_aux_staff(m_idx, load_o_ptr);
						break;
					}

					if (load_o_ptr->tval == TV_ROD_MAIN)
					{
						/* decrease stored mana (timeout) for rods */
						load_o_ptr->timeout -= cost;
					}
					else
					{
						/* decrease charges for wands and staves */
						load_o_ptr->pval--;
					}
				}

				break;
			}

		default:
			msg_print("oops! nonexistant trap!");

		}

		/* Non-automatic traps are removed */
		if (!(f2 & (TRAP2_AUTOMATIC_5 | TRAP2_AUTOMATIC_99)))
		{
			remove = TRUE;
		}
		else if (f2 & TRAP2_AUTOMATIC_5) remove = (randint(5) == 1);

	}

	/* Special trap effect -- teleport to */
	if ((f2 & TRAP2_TELEPORT_TO) && (!disarm) && (!dead))
	{
		teleport_monster_to(m_idx, p_ptr->py, p_ptr->px);
	}

	/* Remove the trap if inactive now */
	if (remove) place_floor(my, mx);

	/* did it die? */
	return (dead);
}

