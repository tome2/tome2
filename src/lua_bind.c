/* File: lua_bind.c */

/* Purpose: various lua bindings */

/*
 * Copyright (c) 2001 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include <assert.h>

#include "spell_type.h"

/*
 * Monsters
 */

void find_position(int y, int x, int *yy, int *xx)
{
	int attempts = 500;

	do
	{
		scatter(yy, xx, y, x, 6);
	}
	while (!(in_bounds(*yy, *xx) && cave_floor_bold(*yy, *xx)) && --attempts);
}

/*
 * Misc
 */

/* Change this fct if I want to switch to learnable spells */
s32b lua_get_level(spell_type *spell, s32b lvl, s32b max, s32b min, s32b bonus)
{
	s32b tmp;

	tmp = lvl - ((spell_type_skill_level(spell) - 1) * (SKILL_STEP / 10));

	if (tmp >= (SKILL_STEP / 10)) /* We require at least one spell level */
		tmp += bonus;

	tmp = (tmp * (max * (SKILL_STEP / 10)) / (SKILL_MAX / 10));

	if (tmp < 0) /* Shift all negative values, so they map to appropriate integer */
		tmp -= SKILL_STEP / 10 - 1;

	/* Now, we can safely divide */
	lvl = tmp / (SKILL_STEP / 10);

	if (lvl < min)
		lvl = min;

	return lvl;
}

/** This is the function to use when casting through a stick */
s32b get_level_device(s32b s, s32b max, s32b min)
{
	int lvl;
	spell_type *spell = spell_at(s);

	/* No max specified ? assume 50 */
	if (max <= 0) {
		max = 50;
	}
	/* No min specified ? */
	if (min <= 0) {
		min = 1;
	}

	lvl = s_info[SKILL_DEVICE].value;
	lvl = lvl + (get_level_use_stick * SKILL_STEP);

	/* Sticks are limited */
	if (lvl - ((spell_type_skill_level(spell) + 1) * SKILL_STEP) >= get_level_max_stick * SKILL_STEP)
	{
		lvl = (get_level_max_stick + spell_type_skill_level(spell) - 1) * SKILL_STEP;
	}

	/* / 10 because otherwise we can overflow a s32b and we can use a u32b because the value can be negative
	-- The loss of information should be negligible since 1 skill = 1000 internally
	*/
	lvl = lvl / 10;
	lvl = lua_get_level(spell, lvl, max, min, 0);

	return lvl;
}

int get_mana(s32b s)
{
	spell_type *spell = spell_at(s);
	range_type mana_range;
	spell_type_mana_range(spell, &mana_range);
	return get_level(s, mana_range.max, mana_range.min);
}

/** Returns spell chance of failure for spell */
s32b spell_chance(s32b s)
{
        spell_type *s_ptr = spell_at(s);
	int level = get_level(s, 50, 1);

	/* Extract the base spell failure rate */
	if (get_level_use_stick > -1)
	{
		int minfail;
		s32b chance = spell_type_failure_rate(s_ptr);

		/* Reduce failure rate by "effective" level adjustment */
		chance -= (level - 1);

		/* Extract the minimum failure rate */
		minfail = 15 - get_skill_scale(SKILL_DEVICE, 25);

		/* Return the chance */
		return clamp_failure_chance(chance, minfail);
	}
	else
	{
		s32b chance = spell_type_failure_rate(s_ptr);
		int mana = get_mana(s);
		int cur_mana = get_power(s);
		int stat = spell_type_casting_stat(s_ptr);
		int stat_ind = p_ptr->stat_ind[stat];
		int minfail;

		/* Reduce failure rate by "effective" level adjustment */
		chance -= 3 * (level - 1);

		/* Reduce failure rate by INT/WIS adjustment */
		chance -= 3 * (adj_mag_stat[stat_ind] - 1);

		/* Not enough mana to cast */
		if (chance < 0) chance = 0;
		if (mana > cur_mana)
		{
			chance += 15 * (mana - cur_mana);
		}

		/* Extract the minimum failure rate */
		minfail = adj_mag_fail[stat_ind];

		/* Must have Perfect Casting to get below 5% */
		if (!(has_ability(AB_PERFECT_CASTING)))
		{
			if (minfail < 5) minfail = 5;
		}

		/* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
		if ((forbid_non_blessed()) && (p_ptr->icky_wield)) chance += 25;

		/* Return the chance */
		return clamp_failure_chance(chance, minfail);
	}
}

s32b get_level(s32b s, s32b max, s32b min)
{
	/** Ahah shall we use Magic device instead ? */
	if (get_level_use_stick > -1) {
		return get_level_device(s, max, min);
	} else {
		s32b level;
		bool_ notused;
		get_level_school(s, max, min, &level, &notused);
		return level;
	}
}

void set_target(int y, int x)
{
	target_who = -1;
	target_col = x;
	target_row = y;
}

void get_target(int dir, int *y, int *x)
{
	int ty, tx;

	/* Use the given direction */
	tx = p_ptr->px + (ddx[dir] * 100);
	ty = p_ptr->py + (ddy[dir] * 100);

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay())
	{
		tx = target_col;
		ty = target_row;
	}
	*y = ty;
	*x = tx;
}

/* Level gen */
void get_map_size(char *name, int *ysize, int *xsize)
{
	*xsize = 0;
	*ysize = 0;
	init_flags = INIT_GET_SIZE;
	process_dungeon_file(name, ysize, xsize, cur_hgt, cur_wid, TRUE, TRUE);
}

void load_map(char *name, int *y, int *x)
{
	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file(name, y, x, cur_hgt, cur_wid, TRUE, TRUE);
}

/*
 * Finds a good random bounty monster
 * Im too lazy to write it in lua since the lua API for monsters is not very well yet
 */

/*
 * Hook for bounty monster selection.
 */
static bool_ lua_mon_hook_bounty(int r_idx)
{
	monster_race* r_ptr = &r_info[r_idx];


	/* Reject uniques */
	if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);

	/* Reject those who cannot leave anything */
	if (!(r_ptr->flags9 & RF9_DROP_CORPSE)) return (FALSE);

	/* Accept only monsters that can be generated */
	if (r_ptr->flags9 & RF9_SPECIAL_GENE) return (FALSE);
	if (r_ptr->flags9 & RF9_NEVER_GENE) return (FALSE);

	/* Reject pets */
	if (r_ptr->flags7 & RF7_PET) return (FALSE);

	/* Reject friendly creatures */
	if (r_ptr->flags7 & RF7_FRIENDLY) return (FALSE);

	/* Accept only monsters that are not breeders */
	if (r_ptr->flags4 & RF4_MULTIPLY) return (FALSE);

	/* Forbid joke monsters */
	if (r_ptr->flags8 & RF8_JOKEANGBAND) return (FALSE);

	/* Accept only monsters that are not good */
	if (r_ptr->flags3 & RF3_GOOD) return (FALSE);

	/* The rest are acceptable */
	return (TRUE);
}

int lua_get_new_bounty_monster(int lev)
{
	int r_idx;

	/*
	 * Set up the hooks -- no bounties on uniques or monsters
	 * with no corpses
	 */
	get_mon_num_hook = lua_mon_hook_bounty;
	get_mon_num_prep();

	/* Set up the quest monster. */
	r_idx = get_mon_num(lev);

	/* Undo the filters */
	get_mon_num_hook = NULL;
	get_mon_num_prep();

	return r_idx;
}

/*
 * Some misc functions
 */
char *lua_input_box(cptr title, int max)
{
	static char buf[80];
	int wid, hgt;

	strcpy(buf, "");
	Term_get_size(&wid, &hgt);
	if (!input_box(title, hgt / 2, wid / 2, buf, (max > 79) ? 79 : max))
		return "";
	return buf;
}

char lua_msg_box(cptr title)
{
	int wid, hgt;

	Term_get_size(&wid, &hgt);
	return msg_box(title, hgt / 2, wid / 2);
}



void increase_mana(int delta)
{
	p_ptr->csp += delta;
	p_ptr->redraw |= PR_MANA;

	if (p_ptr->csp < 0)
	{
		p_ptr->csp = 0;
	}
	if (p_ptr->csp > p_ptr->msp)
	{
		p_ptr->csp = p_ptr->msp;
	}
}

timer_type *TIMER_AGGRAVATE_EVIL = 0;

void timer_aggravate_evil_enable()
{
    	TIMER_AGGRAVATE_EVIL->enabled = TRUE;
}

void timer_aggravate_evil_callback()
{
	if ((p_ptr->prace == RACE_MAIA) &&
	    (!player_has_corruption(CORRUPT_BALROG_AURA)) &&
	    (!player_has_corruption(CORRUPT_BALROG_WINGS)) &&
	    (!player_has_corruption(CORRUPT_BALROG_STRENGTH)) &&
	    (!player_has_corruption(CORRUPT_BALROG_FORM)))
	{
		dispel_evil(0);
	}
}
