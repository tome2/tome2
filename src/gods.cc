/*
 * Copyright (c) 2002 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */
#include "gods.hpp"

#include "game.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "skill_type.hpp"
#include "stats.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

using boost::algorithm::equals;

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
static bool may_follow_god(int god)
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
				return false;
			}
		}
	}
	/* Default is to allow */
	return true;
}

/*
 * Get a religion
 */
void follow_god(int god, bool silent)
{
	auto &s_info = game->s_info;

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
		if (p_ptr->pgod == GOD_MELKOR)
		{
			s_info[SKILL_UDUN].hidden = false;
			if (!silent)
			{
				msg_print("You feel the dark powers of Melkor in you.  You can now use the Udun skill.");
			}
		}
	}
}

/*
 * Show religious info.
 */
bool show_god_info()
{
	int pgod = p_ptr->pgod;

	deity_type *d_ptr;

	if (pgod < 0)
	{
		msg_print("You don't worship anyone.");
		msg_print(NULL);
		return false;
	}
	else
	{
		int i;

		d_ptr = &deity_info[pgod];

		msg_print(NULL);

		screen_save_no_flush();

		text_out(format("You worship %s. ", d_ptr->name));
		for (i = 0; (i < 10) && (!equals(d_ptr->desc[i], "")); i++)
			text_out(d_ptr->desc[i]);
		text_out("\n");

		inkey();

		screen_load_no_flush();
	}

	return true;
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
bool god_enabled(struct deity_type *deity)
{
	int i;

	for (i = 0; deity->modules[i] != -1; i++)
	{
		if (deity->modules[i] == game_module_idx)
		{
			return true;
		}
	}
	/* Not enabled */
	return false;
}

/* Find a god by name */
int find_god(const char *name)
{
	int i;

	for (i = 0; i < MAX_GODS; i++)
	{
		/* The name matches and god is "enabled" for the
		   current module. */
		if (god_enabled(&deity_info[i]) &&
		    equals(deity_info[i].name, name))
		{
			return (i);
		}
	}
	return -1;
}

bool praying_to(int god)
{
	return (p_ptr->pgod == god) && p_ptr->praying;
}
