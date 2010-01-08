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
 * Get a religion
 */
void follow_god(int god, bool silent)
{
	/* Poor unbelievers, i'm so mean ... BOUHAHAHA */
	if (get_skill(SKILL_ANTIMAGIC))
	{
		msg_print("Don't be silly; you don't believe in gods.");
		return;
	}

	/* Are we allowed ? */
	if (process_hooks(HOOK_FOLLOW_GOD, "(d,s)", god, "ask"))
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

		/* Anything to be done? */
		process_hooks(HOOK_FOLLOW_GOD, "(d,s)", god, "done");
	}
}

/*
 * Show religious info.
 */
bool show_god_info(bool ext)
{
	int pgod = p_ptr->pgod;
	int tmp;

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

		tmp = inkey();

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

/* Find a god by name */
int find_god(cptr name)
{
	int i;

	for (i = 0; i < max_gods; i++)
	{
		/* The name matches */
		if (streq(deity_info[i].name, name)) return (i);
	}
	return -1;
}
