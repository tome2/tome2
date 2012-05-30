/* File: gods.c */

/* Purpose: Deities code */

/*
 * Copyright (c) 2002 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * Add amt piety is god is god
 */
void inc_piety(int god, s32b amt)
{
	s32b old = p_ptr->grace;

	if ((god == GOD_ALL) || (god == p_ptr->pgod))
	{
		set_grace(p_ptr->grace + amt);
	
		if(amt > 0 && p_ptr->grace <= old)
			set_grace(300000);

		if(amt < 0 && p_ptr->grace >= old)
			set_grace(-300000);
	}
}

/*
 * Renounce to religion
 */
void abandon_god(int god)
{
	if ((god == GOD_ALL) || (god == p_ptr->pgod))
	{
		p_ptr->pgod = GOD_NONE;
		set_grace(0);
	}
}

/*
 * Check if god may be followed by player
 */
static bool_ may_follow_god(int god)
{
	if (god == GOD_MELKOR)
	{
		int i;

		/* Check if player has wielded The One Ring */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			if (p_ptr->inventory[i].name1 == ART_POWER)
			{
				msg_print("The One Ring has corrupted "
					  "you, and you are rejected.");
				return FALSE;
			}
		}
	}
	/* Default is to allow */
	return TRUE;
}

/*
 * Get a religion
 */
void follow_god(int god, bool_ silent)
{
	/* Poor unbelievers, i'm so mean ... BOUHAHAHA */
	if (get_skill(SKILL_ANTIMAGIC))
	{
		msg_print("Don't be silly; you don't believe in gods.");
		return;
	}

	/* Are we allowed ? */
	if (!may_follow_god(god))
		return;

	if (p_ptr->pgod == GOD_NONE)
	{
		p_ptr->pgod = god;

		/* Melkor offer Udun magic */
		GOD(GOD_MELKOR)
		{
			s_info[SKILL_UDUN].hidden = FALSE;
			if (!silent) msg_print("You feel the dark powers of Melkor in you.  You can now use the Udun skill.");
		}
	}
}

/*
 * Show religious info.
 */
bool_ show_god_info(bool_ ext)
{
	int pgod = p_ptr->pgod;

	deity_type *d_ptr;

	if (pgod < 0)
	{
		msg_print("You don't worship anyone.");
		msg_print(NULL);
		return FALSE;
	}
	else
	{
		int i;

		d_ptr = &deity_info[pgod];

		msg_print(NULL);

		character_icky = TRUE;
		Term_save();

		text_out(format("You worship %s. ", d_ptr->name));
		for (i = 0; (i < 10) && (strcmp(d_ptr->desc[i], "")); i++)
			text_out(d_ptr->desc[i]);
		text_out("\n");

		inkey();

		Term_load();
		character_icky = FALSE;
	}

	return TRUE;
}

/*
 * Rescale the wisdom value to a 0 <-> max range
 */
int wisdom_scale(int max)
{
	int i = p_ptr->stat_ind[A_WIS];

	return (i * max) / 37;
}

/*
 * Get deity info for a given god index.
 * Returns NULL for the "atheist" god.
 */
deity_type *god_at(byte god_idx)
{
	assert(god_idx >= 0);
	assert(god_idx < MAX_GODS);

	if (god_idx == 0)
	{
		return NULL;
	}

	return &deity_info[god_idx];
}

/*
 * Check if god is enabled for the current module
 */
bool_ god_enabled(struct deity_type *deity)
{
	int i;

	for (i = 0; deity->modules[i] != -1; i++)
	{
		if (deity->modules[i] == game_module_idx)
		{
			return TRUE;
		}
	}
	/* Not enabled */
	return FALSE;
}

/* Find a god by name */
int find_god(cptr name)
{
	int i;

	for (i = 0; i < MAX_GODS; i++)
	{
		/* The name matches and god is "enabled" for the
		   current module. */
		if (god_enabled(&deity_info[i]) &&
		    streq(deity_info[i].name, name))
		{
			return (i);
		}
	}
	return -1;
}
