/* File: monster3.c */

/* Purpose: Monsters AI */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * Is the mon,ster in friendly state(pet, friend, ..)
 * -1 = enemy, 0 = neutral, 1 = friend
 */
int is_friend(monster_type *m_ptr)
{
	switch (m_ptr->status)
	{
		/* pets/friends/companion attacks monsters */
	case MSTATUS_NEUTRAL_P:
	case MSTATUS_FRIEND:
	case MSTATUS_PET:
	case MSTATUS_COMPANION:
		return 1;
		break;
	case MSTATUS_NEUTRAL_M:
	case MSTATUS_ENEMY:
		return -1;
		break;
	case MSTATUS_NEUTRAL:
		return 0;
		break;
	}

	/* OUPS */
	return (0);
}

/* Should they attack each others */
bool_ is_enemy(monster_type *m_ptr, monster_type *t_ptr)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx], *rt_ptr = &r_info[t_ptr->r_idx];
	int s1 = is_friend(m_ptr), s2 = is_friend(t_ptr);

	/* Monsters hates breeders */
	if ((m_ptr->status != MSTATUS_NEUTRAL) && (rt_ptr->flags4 & RF4_MULTIPLY) && (num_repro > MAX_REPRO * 2 / 3) && (r_ptr->d_char != rt_ptr->d_char)) return TRUE;
	if ((t_ptr->status != MSTATUS_NEUTRAL) && (r_ptr->flags4 & RF4_MULTIPLY) && (num_repro > MAX_REPRO * 2 / 3) && (r_ptr->d_char != rt_ptr->d_char)) return TRUE;

	/* No special conditions, lets test normal flags */
	if (s1 && s2 && (s1 == -s2)) return TRUE;

	/* Not ennemy */
	return (FALSE);
}

bool_ change_side(monster_type *m_ptr)
{
	monster_race *r_ptr = race_inf(m_ptr);

	/* neutrals and companions  */
	switch (m_ptr->status)
	{
	case MSTATUS_FRIEND:
		m_ptr->status = MSTATUS_ENEMY;
		if ((r_ptr->flags3 & RF3_ANIMAL) && (!(r_ptr->flags3 & RF3_EVIL)))
			inc_piety(GOD_YAVANNA, -m_ptr->level * 4);
		break;
	case MSTATUS_NEUTRAL_P:
		m_ptr->status = MSTATUS_NEUTRAL_M;
		break;
	case MSTATUS_NEUTRAL_M:
		m_ptr->status = MSTATUS_NEUTRAL_P;
		break;
	case MSTATUS_PET:
		m_ptr->status = MSTATUS_ENEMY;
		if ((r_ptr->flags3 & RF3_ANIMAL) && (!(r_ptr->flags3 & RF3_EVIL)))
			inc_piety(GOD_YAVANNA, -m_ptr->level * 4);
		break;
	case MSTATUS_COMPANION:
		return FALSE;
		break;
	}
	/* changed */
	return TRUE;
}

/* Multiply !! */
bool_ ai_multiply(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	int k, y, x, oy = m_ptr->fy, ox = m_ptr->fx;
	bool_ is_frien = (is_friend(m_ptr) > 0);

	/* Count the adjacent monsters */
	for (k = 0, y = oy - 1; y <= oy + 1; y++)
	{
		for (x = ox - 1; x <= ox + 1; x++)
		{
			if (cave[y][x].m_idx) k++;
		}
	}

	if (is_friend(m_ptr) > 0)
	{
		is_frien = TRUE;
	}
	else
	{
		is_frien = FALSE;
	}

	/* Hack -- multiply slower in crowded areas */
	if ((k < 4) && (!k || !rand_int(k * MON_MULT_ADJ)))
	{
		/* Try to multiply */
		if (multiply_monster(m_idx, (is_frien), FALSE))
		{
			/* Take note if visible */
			if (m_ptr->ml)
			{
				r_ptr->r_flags4 |= (RF4_MULTIPLY);
			}

			/* Multiplying takes energy */
			return TRUE;
		}
	}
	return FALSE;
}

/* Possessor incarnates */
bool_ ai_possessor(int m_idx, int o_idx)
{
	object_type *o_ptr = &o_list[o_idx];
	monster_type *m_ptr = &m_list[m_idx];
	int r_idx = m_ptr->r_idx, r2_idx = o_ptr->pval2;
	int i;
	monster_race *r_ptr = &r_info[r2_idx];
	char m_name[80], m_name2[80];

	monster_desc(m_name, m_ptr, 0x00);
	monster_race_desc(m_name2, r2_idx, 0);

	if (m_ptr->ml) msg_format("%^s incarnates into a %s!", m_name, m_name2);

	/* Remove the old one */
	delete_object_idx(o_idx);

	m_ptr->r_idx = r2_idx;
	m_ptr->ego = 0;

	/* No "damage" yet */
	m_ptr->stunned = 0;
	m_ptr->confused = 0;
	m_ptr->monfear = 0;

	/* No target yet */
	m_ptr->target = -1;

	/* Assume no sleeping */
	m_ptr->csleep = 0;

	/* Assign maximal hitpoints */
	if (r_ptr->flags1 & (RF1_FORCE_MAXHP))
	{
		m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
	}
	else
	{
		m_ptr->maxhp = damroll(r_ptr->hdice, r_ptr->hside);
	}

	/* And start out fully healthy */
	m_ptr->hp = m_ptr->maxhp;

	/* Some basic info */
	for (i = 0; i < 4; i++)
	{
		m_ptr->blow[i].method = r_ptr->blow[i].method;
		m_ptr->blow[i].effect = r_ptr->blow[i].effect;
		m_ptr->blow[i].d_dice = r_ptr->blow[i].d_dice;
		m_ptr->blow[i].d_side = r_ptr->blow[i].d_side;
	}
	m_ptr->ac = r_ptr->ac;
	m_ptr->level = r_ptr->level;
	m_ptr->speed = r_ptr->speed;
	m_ptr->exp = MONSTER_EXP(m_ptr->level);

	/* Extract the monster base speed */
	m_ptr->mspeed = m_ptr->speed;

	m_ptr->energy = 0;

	/* Hack -- Count the number of "reproducers" */
	if (r_ptr->flags4 & (RF4_MULTIPLY)) num_repro++;

	/* Hack -- Notice new multi-hued monsters */
	if (r_ptr->flags1 & (RF1_ATTR_MULTI)) shimmer_monsters = TRUE;

	/* Hack -- Count the monsters on the level */
	r_ptr->cur_num++;
	r_info[r_idx].cur_num--;

	m_ptr->possessor = r_idx;

	/* Update the monster */
	update_mon(m_idx, TRUE);

	return TRUE;
}

void ai_deincarnate(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	int r2_idx = m_ptr->possessor, r_idx = m_ptr->r_idx;
	monster_race *r_ptr = &r_info[r2_idx];
	int i;
	char m_name[80];

	monster_desc(m_name, m_ptr, 0x04);

	if (m_ptr->ml) msg_format("The soul of %s deincarnates!", m_name);

	m_ptr->r_idx = r2_idx;
	m_ptr->ego = 0;

	/* No "damage" yet */
	m_ptr->stunned = 0;
	m_ptr->confused = 0;
	m_ptr->monfear = 0;

	/* No target yet */
	m_ptr->target = -1;

	/* Assume no sleeping */
	m_ptr->csleep = 0;

	/* Assign maximal hitpoints */
	if (r_ptr->flags1 & (RF1_FORCE_MAXHP))
	{
		m_ptr->maxhp = maxroll(r_ptr->hdice, r_ptr->hside);
	}
	else
	{
		m_ptr->maxhp = damroll(r_ptr->hdice, r_ptr->hside);
	}

	/* And start out fully healthy */
	m_ptr->hp = m_ptr->maxhp;

	/* Some basic info */
	for (i = 0; i < 4; i++)
	{
		m_ptr->blow[i].method = r_ptr->blow[i].method;
		m_ptr->blow[i].effect = r_ptr->blow[i].effect;
		m_ptr->blow[i].d_dice = r_ptr->blow[i].d_dice;
		m_ptr->blow[i].d_side = r_ptr->blow[i].d_side;
	}
	m_ptr->ac = r_ptr->ac;
	m_ptr->level = r_ptr->level;
	m_ptr->speed = r_ptr->speed;
	m_ptr->exp = MONSTER_EXP(m_ptr->level);

	/* Extract the monster base speed */
	m_ptr->mspeed = m_ptr->speed;

	m_ptr->energy = 0;

	/* Hack -- Count the number of "reproducers" */
	if (r_ptr->flags4 & (RF4_MULTIPLY)) num_repro++;

	/* Hack -- Notice new multi-hued monsters */
	if (r_ptr->flags1 & (RF1_ATTR_MULTI)) shimmer_monsters = TRUE;

	/* Hack -- Count the monsters on the level */
	r_ptr->cur_num++;
	r_info[r_idx].cur_num--;

	m_ptr->possessor = 0;

	/* Update the monster */
	update_mon(m_idx, TRUE);
}

/* Returns if a new companion is allowed */
bool_ can_create_companion(void)
{
	int i, mcnt = 0;

	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status == MSTATUS_COMPANION) mcnt++;
	}

	if (mcnt < (1 + get_skill_scale(SKILL_LORE, 6))) return (TRUE);
	else return (FALSE);
}


/* Player controlled monsters */
bool_ do_control_walk(void)
{
	/* Get a "repeated" direction */
	if (p_ptr->control)
	{
		int dir;

		if (get_rep_dir(&dir))
		{
			/* Take a turn */
			energy_use = 100;

			/* Actually move the monster */
			p_ptr->control_dir = dir;
		}
		return TRUE;
	}
	else
		return FALSE;
}

bool_ do_control_inven(void)
{
	int monst_list[23];

	if (!p_ptr->control) return FALSE;
	screen_save();
	prt("Carried items", 0, 0);
	show_monster_inven(p_ptr->control, monst_list);
	inkey();
	screen_load();
	return TRUE;
}

bool_ do_control_pickup(void)
{
	int this_o_idx, next_o_idx = 0;
	monster_type *m_ptr = &m_list[p_ptr->control];
	cave_type *c_ptr;
	bool_ done = FALSE;

	if (!p_ptr->control) return FALSE;

	/* Scan all objects in the grid */
	c_ptr = &cave[m_ptr->fy][m_ptr->fx];
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Skip gold */
		if (o_ptr->tval == TV_GOLD) continue;

		/* Excise the object */
		excise_object_idx(this_o_idx);

		/* Forget mark */
		o_ptr->marked = FALSE;

		/* Forget location */
		o_ptr->iy = o_ptr->ix = 0;

		/* Memorize monster */
		o_ptr->held_m_idx = p_ptr->control;

		/* Build a stack */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Carry object */
		m_ptr->hold_o_idx = this_o_idx;
		done = TRUE;
	}
	if (done) msg_print("You pick up all objects on the floor.");
	return TRUE;
}

bool_ do_control_drop(void)
{
	monster_type *m_ptr = &m_list[p_ptr->control];

	if (!p_ptr->control) return FALSE;
	monster_drop_carried_objects(m_ptr);
	return TRUE;
}

bool_ do_control_magic(void)
{
	int power = -1;
	int num = 0, i;
	int powers[96];
	bool_ flag, redraw;
	int ask;
	char choice;
	char out_val[160];
	monster_race *r_ptr = &r_info[m_list[p_ptr->control].r_idx];
	int label;

	if (!p_ptr->control) return FALSE;

	if (get_check("Do you want to abandon the creature?"))
	{
		if (get_check("Abandon it permanently?"))
			delete_monster_idx(p_ptr->control);
		p_ptr->control = 0;
		return TRUE;
	}

	/* List the monster powers -- RF4_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags4 & BIT(i))
		{
			if (!monster_powers[i].power) continue;
			powers[num++] = i;
		}
	}

	/* List the monster powers -- RF5_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags5 & BIT(i))
		{
			if (!monster_powers[i + 32].power) continue;
			powers[num++] = i + 32;
		}
	}

	/* List the monster powers -- RF6_* */
	for (i = 0; i < 32; i++)
	{
		if (r_ptr->flags6 & BIT(i))
		{
			if (!monster_powers[i + 64].power) continue;
			powers[num++] = i + 64;
		}
	}

	if (!num)
	{
		msg_print("You have no powers you can use.");
		return (TRUE);
	}

	/* Nothing chosen yet */
	flag = FALSE;

	/* No redraw yet */
	redraw = FALSE;

	/* Get the last label */
	label = (num <= 26) ? I2A(num - 1) : I2D(num - 1 - 26);

	/* Build a prompt (accept all spells) */
	strnfmt(out_val, 78,
	        "(Powers a-%c, *=List, ESC=exit) Use which power of your golem? ",
	        label);

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

					label = (ctr < 26) ? I2A(ctr) : I2D(ctr - 26);

					strnfmt(dummy, 80, " %c) %s",
					        label, mp_ptr->name);

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

	/* Take a turn */
	if (flag)
	{
		energy_use = 100;
		monst_spell_monst_spell = power + 96;
	}
	return TRUE;
}

/* Finds the controlled monster and "reconnect" to it */
bool_ do_control_reconnect()
{
	int i;

	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->mflag & MFLAG_CONTROL)
		{
			p_ptr->control = i;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Turns a simple pet into a faithful companion
 */
void do_cmd_companion()
{
	monster_type *m_ptr;
	int ii, jj;

	if (!can_create_companion())
	{
		msg_print("You cannot have any more companions.");
		return;
	}

	if (!tgt_pt(&ii, &jj))
	{
		msg_print("You must target a pet.");
		return;
	}

	if (cave[jj][ii].m_idx)
	{
		char m_name[100];

		m_ptr = &m_list[cave[jj][ii].m_idx];

		monster_desc(m_name, m_ptr, 0);
		if (m_ptr->status == MSTATUS_PET)
		{
			m_ptr->status = MSTATUS_COMPANION;
			msg_format("%^s agrees to follow you.", m_name);
		}
		else
		{
			msg_format("%^s is not your pet!", m_name);
		}
	}
	else
		msg_print("You must target a pet.");
}

/*
 * List companions to the character sheet.
 */
void dump_companions(FILE *outfile)
{
	int i;
	int done_hdr = 0;

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status == MSTATUS_COMPANION)
		{
			char pet_name[80];

			/* Output the header if we haven't yet. */
			if (!done_hdr)
			{
				done_hdr = 1;
				fprintf(outfile, "\n\n  [Current companions]\n\n");
			}

			/* List the monster. */
			monster_desc(pet_name, m_ptr, 0x88);
			fprintf(outfile, "%s (level %d)\n", pet_name, m_ptr->level);
		}
	}
}
