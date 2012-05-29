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

#include "lua.h"
#include "tolua.h"
extern lua_State *L;

s16b can_spell_random(s16b spell_idx)
{
	return spell_at(spell_idx)->random_type;
}

magic_power *grab_magic_power(magic_power *m_ptr, int num)
{
	return (&m_ptr[num]);
}

bool_ lua_spell_success(magic_power *spell, int stat, char *oups_fct)
{
	int chance;
	int minfail = 0;

	/* Spell failure chance */
	chance = spell->fail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (p_ptr->lev - spell->min_lev);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[stat]] - 1);

	/* Not enough mana to cast */
	if (spell->mana_cost > p_ptr->csp)
	{
		chance += 5 * (spell->mana_cost - p_ptr->csp);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[stat]];

	/* Failure rate */
	chance = clamp_failure_chance(chance, minfail);

	/* Failed spell */
	if (rand_int(100) < chance)
	{
		if (flush_failure) flush();
		msg_format("You failed to concentrate hard enough!");
		sound(SOUND_FAIL);

		if (oups_fct != NULL)
			exec_lua(format("%s(%d)", oups_fct, chance));
		return (FALSE);
	}
	return (TRUE);
}

/*
 * Create objects
 */
object_type *new_object()
{
	object_type *o_ptr;
	MAKE(o_ptr, object_type);
	return (o_ptr);
}

void end_object(object_type *o_ptr)
{
	FREE(o_ptr, object_type);
}

static char *lua_item_tester_fct;
static bool_ lua_item_tester(object_type* o_ptr)
{
	int oldtop = lua_gettop(L);
	bool_ ret;

	lua_getglobal(L, lua_item_tester_fct);
	tolua_pushusertype(L, o_ptr, tolua_tag(L, "object_type"));
	lua_call(L, 1, 1);
	ret = lua_tonumber(L, -1);
	lua_settop(L, oldtop);
	return (ret);
}

void lua_set_item_tester(int tval, char *fct)
{
	if (tval)
	{
		item_tester_tval = tval;
	}
	else
	{
		lua_item_tester_fct = fct;
		item_tester_hook = lua_item_tester;
	}
}

char *lua_object_desc(object_type *o_ptr, int pref, int mode)
{
	static char buf[150];

	object_desc(buf, o_ptr, pref, mode);
	return (buf);
}

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

static char *summon_lua_okay_fct;
bool_ summon_lua_okay(int r_idx)
{
	int oldtop = lua_gettop(L);
	bool_ ret;

	lua_getglobal(L, lua_item_tester_fct);
	tolua_pushnumber(L, r_idx);
	lua_call(L, 1, 1);
	ret = lua_tonumber(L, -1);
	lua_settop(L, oldtop);
	return (ret);
}

bool_ lua_summon_monster(int y, int x, int lev, bool_ friend_, char *fct)
{
	summon_lua_okay_fct = fct;

	if (!friend_)
		return summon_specific(y, x, lev, SUMMON_LUA);
	else
		return summon_specific_friendly(y, x, lev, SUMMON_LUA, TRUE);
}

/*
 * Misc
 */
bool_ get_com_lua(cptr prompt, int *com)
{
	char c;

	if (!get_com(prompt, &c)) return (FALSE);
	*com = c;
	return (TRUE);
}

/* Spell schools */
s16b new_school(int i, cptr name, s16b skill)
{
	schools[i].name = string_make(name);
	schools[i].skill = skill;
	return (i);
}

school_type *grab_school_type(s16b num)
{
	return (&schools[num]);
}

/* Change this fct if I want to switch to learnable spells */
s32b lua_get_level(s32b s, s32b lvl, s32b max, s32b min, s32b bonus)
{
	s32b tmp;

	tmp = lvl - ((school_spells[s].skill_level - 1) * (SKILL_STEP / 10));

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
	if (lvl - ((school_spells[s].skill_level + 1) * SKILL_STEP) >= get_level_max_stick * SKILL_STEP)
	{
		lvl = (get_level_max_stick + school_spells[s].skill_level - 1) * SKILL_STEP;
	}

	/* / 10 because otherwise we can overflow a s32b and we can use a u32b because the value can be negative
	-- The loss of information should be negligible since 1 skill = 1000 internally
	*/
	lvl = lvl / 10;
	lvl = lua_get_level(s, lvl, max, min, 0);

	return lvl;
}

int get_mana(s32b s)
{
	spell_type *spell = spell_at(s);
	return get_level(s, spell->mana_range.max, spell->mana_range.min);
}

/** Returns spell chance of failure for spell */
s32b spell_chance(s32b s)
{
        spell_type *s_ptr = &school_spells[s];
	int level = get_level(s, 50, 1);

	/* Extract the base spell failure rate */
	if (get_level_use_stick > -1) {
		return lua_spell_device_chance(s_ptr->failure_rate, level, s_ptr->skill_level);
	} else {
		return lua_spell_chance(s_ptr->failure_rate, level, s_ptr->skill_level, get_mana(s), get_power(s), s_ptr->casting_stat);
	}
}

void get_level_school(s32b s, s32b max, s32b min, s32b *level, bool_ *na)
{
	if (level != NULL)
	{
		*level = exec_lua(format("local lvl, na = get_level_school(%d, %d, %d); return lvl", s, max, min));
	}

	if (na != NULL)
	{
		*na =  exec_lua(format("local lvl, na = get_level_school(%d, %d, %d); return (na == \"n/a\")", s, max, min));
	}
}

s32b get_level(s32b s, s32b max, s32b min)
{
	/** Ahah shall we use Magic device instead ? */
	if (get_level_use_stick > -1) {
		return get_level_device(s, max, min);
	} else {
		s32b level;
		get_level_school(s, max, min, &level, NULL);
		return level;
	}
}


s32b lua_spell_chance(s32b chance, int level, int skill_level, int mana, int cur_mana, int stat)
{
	int minfail;
	/* Reduce failure rate by "effective" level adjustment */
	chance -= 3 * (level - 1);

	/* Reduce failure rate by INT/WIS adjustment */
	chance -= 3 * (adj_mag_stat[p_ptr->stat_ind[stat]] - 1);

	/* Not enough mana to cast */
	if (chance < 0) chance = 0;
	if (mana > cur_mana)
	{
		chance += 15 * (mana - cur_mana);
	}

	/* Extract the minimum failure rate */
	minfail = adj_mag_fail[p_ptr->stat_ind[stat]];

	/*
	        * Non mage characters never get too good
	 */
	if (!(has_ability(AB_PERFECT_CASTING)))
	{
		if (minfail < 5) minfail = 5;
	}

	/* Hack -- Priest prayer penalty for "edged" weapons  -DGK */
	if ((forbid_non_blessed()) && (p_ptr->icky_wield)) chance += 25;

	/* Return the chance */
	return clamp_failure_chance(chance, minfail);
}

s32b lua_spell_device_chance(s32b chance, int level, int base_level)
{
	int minfail;

	/* Reduce failure rate by "effective" level adjustment */
	chance -= (level - 1);

	/* Extract the minimum failure rate */
	minfail = 15 - get_skill_scale(SKILL_DEVICE, 25);

	/* Return the chance */
	return clamp_failure_chance(chance, minfail);
}

/* Cave */
cave_type *lua_get_cave(int y, int x)
{
	return (&(cave[y][x]));
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

bool_ alloc_room(int by0, int bx0, int ysize, int xsize, int *y1, int *x1, int *y2, int *x2)
{
	int xval, yval, x, y;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 2, ysize + 2, FALSE, by0, bx0, &xval, &yval)) return FALSE;

	/* Get corner values */
	*y1 = yval - ysize / 2;
	*x1 = xval - xsize / 2;
	*y2 = yval + (ysize) / 2;
	*x2 = xval + (xsize) / 2;

	/* Place a full floor under the room */
	for (y = *y1 - 1; y <= *y2 + 1; y++)
	{
		for (x = *x1 - 1; x <= *x2 + 1; x++)
		{
			cave_type *c_ptr = &cave[y][x];
			cave_set_feat(y, x, floor_type[rand_int(100)]);
			c_ptr->info |= (CAVE_ROOM);
			c_ptr->info |= (CAVE_GLOW);
		}
	}
	return TRUE;
}


/* Files */
void lua_print_hook(cptr str)
{
	fprintf(hook_file, "%s", str);
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

list_type *lua_create_list(int size)
{
	list_type *l;
	cptr *list;

	MAKE(l, list_type);
	C_MAKE(list, size, cptr);
	l->list = list;
	return l;
}

void lua_delete_list(list_type *l, int size)
{
	int i;

	for (i = 0; i < size; i++)
		string_free(l->list[i]);
	C_FREE(l->list, size, cptr);
	FREE(l, list_type);
}

void lua_add_to_list(list_type *l, int idx, cptr str)
{
	l->list[idx] = string_make(str);
}

void lua_display_list(int y, int x, int h, int w, cptr title, list_type* list, int max, int begin, int sel, byte sel_color)
{
	display_list(y, x, h, w, title, list->list, max, begin, sel, sel_color);
}



int get_lua_int(cptr name)
{
	char buf[128];
	sprintf(buf, "return %s", name);
	return exec_lua(buf);
}

int get_lua_list_size(cptr list_var)
{
	char buf[128];
	sprintf(buf, "return getn(%s)", list_var);
	return exec_lua(buf);
}

void increase_mana(int delta)
{
	char buf[256];
	sprintf(buf, "increase_mana(%d)", delta);
	exec_lua(buf);
}

timer_type *TIMER_AGGRAVATE_EVIL = 0;

void timer_aggravate_evil_enable()
{
    	TIMER_AGGRAVATE_EVIL->enabled = TRUE;
}

void timer_aggravate_evil_callback()
{
	dispel_evil(0);
}

cptr get_spell_info(s32b s)
{
	spell_type *spell = spell_at(s);

	assert(spell->info_func != NULL);
	return spell->info_func();
}
