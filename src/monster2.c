/* File: monster2.c */

/* Purpose: misc code for monsters */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define MAX_HORROR 20
#define MAX_FUNNY 22
#define MAX_COMMENT 5

/* Monster gain a few levels ? */
void monster_check_experience(int m_idx, bool_ silent)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	char m_name[80];

	/* Get the name */
	monster_desc(m_name, m_ptr, 0);

	/* Gain levels while possible */
	while ((m_ptr->level < MONSTER_LEVEL_MAX) &&
	                (m_ptr->exp >= (u32b)(MONSTER_EXP(m_ptr->level + 1))))
	{
		/* Gain a level */
		m_ptr->level++;

		if (m_ptr->ml && (!silent)) cmsg_format(TERM_L_BLUE, "%^s gains a level.", m_name);

		/* Gain hp */
		if (magik(80))
		{
			m_ptr->maxhp += r_ptr->hside;
			m_ptr->hp += r_ptr->hside;
		}

		/* Gain speed */
		if (magik(40))
		{
			int speed = randint(2);
			m_ptr->speed += speed;
			m_ptr->mspeed += speed;
		}

		/* Gain ac */
		if (magik(50))
		{
			m_ptr->ac += (r_ptr->ac / 10) ? r_ptr->ac / 10 : 1;
		}

		/* Gain melee power */
		if (magik(30))
		{
			int i = rand_int(3), tries = 20;

			while ((tries--) && !m_ptr->blow[i].d_dice) i = rand_int(3);

			m_ptr->blow[i].d_dice++;
		}
	}
}

/* Monster gain some xp */
void monster_gain_exp(int m_idx, u32b exp, bool_ silent)
{
	monster_type *m_ptr = &m_list[m_idx];

	m_ptr->exp += exp;
	if (wizard)
	{
		char m_name[80];

		/* Get the name */
		monster_desc(m_name, m_ptr, 0);

		if (!silent) msg_format("%^s gains %ld exp.", m_name, exp);
	}

	monster_check_experience(m_idx, silent);
}

void monster_set_level(int m_idx, int level)
{
	monster_type *m_ptr = &m_list[m_idx];

	if (level > 150) level = 150;

	if (m_ptr->level < level)
	{
		m_ptr->exp = MONSTER_EXP(level);
		monster_check_experience(m_idx, TRUE);
	}
}

/* Will add, sub, .. */
s32b modify_aux(s32b a, s32b b, char mod)
{
	switch (mod)
	{
	case MEGO_ADD:
		return (a + b);
		break;
	case MEGO_SUB:
		return (a - b);
		break;
	case MEGO_FIX:
		return (b);
		break;
	case MEGO_PRC:
		return (a * b / 100);
		break;
	default:
		msg_format("WARNING, unmatching MEGO(%d).", mod);
		return (0);
	}
}

/* Is this ego ok for this monster ? */
bool_ mego_ok(int r_idx, int ego)
{
	monster_ego *re_ptr = &re_info[ego];
	monster_race *r_ptr = &r_info[r_idx];
	bool_ ok = FALSE;
	int i;

	/* needed flags */
	if (re_ptr->flags1 && ((re_ptr->flags1 & r_ptr->flags1) != re_ptr->flags1)) return FALSE;
	if (re_ptr->flags2 && ((re_ptr->flags2 & r_ptr->flags2) != re_ptr->flags2)) return FALSE;
	if (re_ptr->flags3 && ((re_ptr->flags3 & r_ptr->flags3) != re_ptr->flags3)) return FALSE;
	if (re_ptr->flags7 && ((re_ptr->flags7 & r_ptr->flags7) != re_ptr->flags7)) return FALSE;
	if (re_ptr->flags8 && ((re_ptr->flags8 & r_ptr->flags8) != re_ptr->flags8)) return FALSE;
	if (re_ptr->flags9 && ((re_ptr->flags9 & r_ptr->flags9) != re_ptr->flags9)) return FALSE;

	/* unwanted flags */
	if (re_ptr->hflags1 && (re_ptr->hflags1 & r_ptr->flags1)) return FALSE;
	if (re_ptr->hflags2 && (re_ptr->hflags2 & r_ptr->flags2)) return FALSE;
	if (re_ptr->hflags3 && (re_ptr->hflags3 & r_ptr->flags3)) return FALSE;
	if (re_ptr->hflags7 && (re_ptr->hflags7 & r_ptr->flags7)) return FALSE;
	if (re_ptr->hflags8 && (re_ptr->hflags8 & r_ptr->flags8)) return FALSE;
	if (re_ptr->hflags9 && (re_ptr->hflags9 & r_ptr->flags9)) return FALSE;

	/* Need good race -- IF races are specified */
	if (re_ptr->r_char[0])
	{
		for (i = 0; i < 5; i++)
		{
			if (r_ptr->d_char == re_ptr->r_char[i]) ok = TRUE;
		}
		if (!ok) return FALSE;
	}
	if (re_ptr->nr_char[0])
	{
		for (i = 0; i < 5; i++)
		{
			if (r_ptr->d_char == re_ptr->nr_char[i]) return (FALSE);
		}
	}

	/* Passed all tests ? */
	return TRUE;
}

/* Choose an ego type */
int pick_ego_monster(int r_idx)
{
	/* Assume no ego */
	int ego = 0, lvl;
	int tries = max_re_idx + 10;
	monster_ego *re_ptr;

	if ((!(dungeon_flags2 & DF2_ELVEN)) && (!(dungeon_flags2 & DF2_DWARVEN)))
	{
		/* No townspeople ego */
		if (!r_info[r_idx].level) return 0;

		/* First are we allowed to find an ego */
		if (!magik(MEGO_CHANCE)) return 0;

		/* Lets look for one */
		while (tries--)
		{
			/* Pick one */
			ego = rand_range(1, max_re_idx - 1);
			re_ptr = &re_info[ego];

			/*  No hope so far */
			if (!mego_ok(r_idx, ego)) continue;

			/* Not too much OoD */
			lvl = r_info[r_idx].level;
			MODIFY(lvl, re_ptr->level, 0);
			lvl -= ((dun_level / 2) + (rand_int(dun_level / 2)));
			if (lvl < 1) lvl = 1;
			if (rand_int(lvl)) continue;

			/* Each ego types have a rarity */
			if (rand_int(re_ptr->rarity)) continue;

			/* We finally got one ? GREAT */
			return ego;
		}
	}
	/* Bypass restrictions for themed townspeople */
	else
	{
		if (dungeon_flags2 & DF2_ELVEN)
			ego = test_mego_name("Elven");
		else if (dungeon_flags2 & DF2_DWARVEN)
			ego = test_mego_name("Dwarven");

		if (mego_ok(r_idx, ego))
			return ego;
	}

	/* Found none ? so sad, well no ego for the time being */
	return 0;
}

/*
 * Return a (monster_race*) with the combination of the monster
 * properties and the ego type
 */
monster_race* race_info_idx(int r_idx, int ego)
{
	static monster_race race;
	monster_ego *re_ptr = &re_info[ego];
	monster_race *r_ptr = &r_info[r_idx], *nr_ptr = &race;
	int i;

	/* No work needed */
	if (!ego) return r_ptr;

	/* Copy the base monster */
	COPY(nr_ptr, r_ptr, monster_race);

	/* Adjust the values */
	for (i = 0; i < 4; i++)
	{
		s32b j, k;

		j = modify_aux(nr_ptr->blow[i].d_dice, re_ptr->blow[i].d_dice, re_ptr->blowm[i][0]);
		if (j < 0) j = 0;
		k = modify_aux(nr_ptr->blow[i].d_side, re_ptr->blow[i].d_side, re_ptr->blowm[i][1]);
		if (k < 0) k = 0;

		nr_ptr->blow[i].d_dice = j;
		nr_ptr->blow[i].d_side = k;

		if (re_ptr->blow[i].method) nr_ptr->blow[i].method = re_ptr->blow[i].method;
		if (re_ptr->blow[i].effect) nr_ptr->blow[i].effect = re_ptr->blow[i].effect;
	}

	MODIFY(nr_ptr->hdice, re_ptr->hdice, 1);
	MODIFY(nr_ptr->hside, re_ptr->hside, 1);

	MODIFY(nr_ptr->ac, re_ptr->ac, 0);

	MODIFY(nr_ptr->sleep, re_ptr->sleep, 0);

	MODIFY(nr_ptr->aaf, re_ptr->aaf, 1);
	MODIFY(nr_ptr->speed, re_ptr->speed, 50);
	MODIFY(nr_ptr->mexp, re_ptr->mexp, 0);

	MODIFY(nr_ptr->weight, re_ptr->weight, 10);

	nr_ptr->freq_inate = (nr_ptr->freq_inate > re_ptr->freq_inate)
	                     ? nr_ptr->freq_inate : re_ptr->freq_inate;
	nr_ptr->freq_spell = (nr_ptr->freq_spell > re_ptr->freq_spell)
	                     ? nr_ptr->freq_spell : re_ptr->freq_spell;

	MODIFY(nr_ptr->level, re_ptr->level, 1);

	/* Take off some flags */
	nr_ptr->flags1 &= ~(re_ptr->nflags1);
	nr_ptr->flags2 &= ~(re_ptr->nflags2);
	nr_ptr->flags3 &= ~(re_ptr->nflags3);
	nr_ptr->flags4 &= ~(re_ptr->nflags4);
	nr_ptr->flags5 &= ~(re_ptr->nflags5);
	nr_ptr->flags6 &= ~(re_ptr->nflags6);
	nr_ptr->flags7 &= ~(re_ptr->nflags7);
	nr_ptr->flags8 &= ~(re_ptr->nflags8);
	nr_ptr->flags9 &= ~(re_ptr->nflags9);

	/* Add some flags */
	nr_ptr->flags1 |= re_ptr->mflags1;
	nr_ptr->flags2 |= re_ptr->mflags2;
	nr_ptr->flags3 |= re_ptr->mflags3;
	nr_ptr->flags4 |= re_ptr->mflags4;
	nr_ptr->flags5 |= re_ptr->mflags5;
	nr_ptr->flags6 |= re_ptr->mflags6;
	nr_ptr->flags7 |= re_ptr->mflags7;
	nr_ptr->flags8 |= re_ptr->mflags8;
	nr_ptr->flags9 |= re_ptr->mflags9;

	/* Change the char/attr is needed */
	if (re_ptr->d_char != MEGO_CHAR_ANY)
	{
		nr_ptr->d_char = re_ptr->d_char;
		nr_ptr->x_char = re_ptr->d_char;
	}
	if (re_ptr->d_attr != MEGO_CHAR_ANY)
	{
		nr_ptr->d_attr = re_ptr->d_attr;
		nr_ptr->x_attr = re_ptr->d_attr;
	}

	/* And finanly return a pointer to a fully working monster race */
	return nr_ptr;
}

static cptr horror_desc[MAX_HORROR] =
{
	"abominable",
	"abysmal",
	"appalling",
	"baleful",
	"blasphemous",

	"disgusting",
	"dreadful",
	"filthy",
	"grisly",
	"hideous",

	"hellish",
	"horrible",
	"infernal",
	"loathsome",
	"nightmarish",

	"repulsive",
	"sacrilegious",
	"terrible",
	"unclean",
	"unspeakable",
};

static cptr funny_desc[MAX_FUNNY] =
{
	"silly",
	"hilarious",
	"absurd",
	"insipid",
	"ridiculous",

	"laughable",
	"ludicrous",
	"far-out",
	"groovy",
	"postmodern",

	"fantastic",
	"dadaistic",
	"cubistic",
	"cosmic",
	"awesome",

	"incomprehensible",
	"fabulous",
	"amazing",
	"incredible",
	"chaotic",

	"wild",
	"preposterous",
};

static cptr funny_comments[MAX_COMMENT] =
{
	"Wow, cosmic, man!",
	"Rad!",
	"Groovy!",
	"Cool!",
	"Far out!"
};


int get_wilderness_flag(void)
{
	int x = p_ptr->wilderness_x;
	int y = p_ptr->wilderness_y;

	if (dun_level)
		return (RF8_DUNGEON);
	else
		return (1L << wf_info[wild_map[y][x].feat].terrain_idx);
}


/*
 * Delete a monster by index.
 *
 * When a monster is deleted, all of its objects are deleted.
 */
void delete_monster_idx(int i)
{
	int x, y, j;

	monster_type *m_ptr = &m_list[i];

	monster_race *r_ptr = race_inf(m_ptr);

	s16b this_o_idx, next_o_idx = 0;

	bool_ had_lite = FALSE;
	;


	/* Get location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Hack -- Reduce the racial counter */
	r_ptr->cur_num--;
	r_ptr->on_saved = FALSE;

	/* Hack -- count the number of "reproducers" */
	if (r_ptr->flags4 & (RF4_MULTIPLY)) num_repro--;

	/* XXX XXX XXX remove monster light source */
	if (r_ptr->flags9 & (RF9_HAS_LITE)) had_lite = TRUE;


	/* Hack -- remove target monster */
	if (i == target_who) target_who = 0;

	/* Hack -- remove tracked monster */
	if (i == health_who) health_track(0);

	/* Hack -- remove tracked monster */
	if (i == p_ptr->control) p_ptr->control = 0;

	for (j = m_max - 1; j >= 1; j--)
	{
		/* Access the monster */
		monster_type *t_ptr = &m_list[j];

		/* Ignore "dead" monsters */
		if (!t_ptr->r_idx) continue;

		if (t_ptr->target == i) t_ptr->target = -1;
	}

	/* Monster is gone */
	cave[y][x].m_idx = 0;


	/* Delete objects */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Hack -- efficiency */
		o_ptr->held_m_idx = 0;

		if ( p_ptr->preserve )
		{
			/* Hack -- Preserve unknown artifacts */
			if (artifact_p(o_ptr) && !object_known_p(o_ptr))
			{
				/* Mega-Hack -- Preserve the artifact */
				if (o_ptr->tval == TV_RANDART)
				{
					random_artifacts[o_ptr->sval].generated = FALSE;
				}
				else if (k_info[o_ptr->k_idx].flags3 & TR3_NORM_ART)
				{
					k_info[o_ptr->k_idx].artifact = FALSE;
				}
				else
				{
					a_info[o_ptr->name1].cur_num = 0;
				}
			}
		}
		/* Delete the object */
		delete_object_idx(this_o_idx);
	}

	/* Delete mind & special race if needed */
	if (m_ptr->sr_ptr)
		KILL(m_ptr->sr_ptr, monster_race);
	if (m_ptr->mind)
		KILL(m_ptr->mind, monster_mind);

	/* Wipe the Monster */
	m_ptr = WIPE(m_ptr, monster_type);

	/* Count monsters */
	m_cnt--;

	/* Do we survided our fate ? */
	if ((dungeon_type == DUNGEON_DEATH) && (!m_cnt))
	{
		msg_print("You overcome your fate, mortal!");

		dungeon_type = DUNGEON_WILDERNESS;
		dun_level = 0;

		p_ptr->leaving = TRUE;
	}

	/* Update monster light */
	if (had_lite) p_ptr->update |= (PU_MON_LITE);

	/* Update monster list window */
	p_ptr->window |= (PW_M_LIST);

	/* Visual update */
	lite_spot(y, x);
}


/*
 * Delete the monster, if any, at a given location
 */
void delete_monster(int y, int x)
{
	cave_type *c_ptr;

	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Check the grid */
	c_ptr = &cave[y][x];

	/* Delete the monster (if any) */
	if (c_ptr->m_idx) delete_monster_idx(c_ptr->m_idx);
}


/*
 * Move an object from index i1 to index i2 in the object list
 */
static void compact_monsters_aux(int i1, int i2)
{
	int y, x, j;

	cave_type *c_ptr;

	monster_type *m_ptr;

	s16b this_o_idx, next_o_idx = 0;


	/* Do nothing */
	if (i1 == i2) return;


	/* Old monster */
	m_ptr = &m_list[i1];

	/* Location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Cave grid */
	c_ptr = &cave[y][x];

	/* Update the cave */
	c_ptr->m_idx = i2;

	/* Repair objects being carried by monster */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type * o_ptr;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Reset monster pointer */
		o_ptr->held_m_idx = i2;
	}

	/* Hack -- Update the control */
	if (p_ptr->control == i1) p_ptr->control = i2;

	/* Hack -- Update the doppleganger */
	if (doppleganger == i1) doppleganger = i2;

	/* Hack -- Update the target */
	if (target_who == i1) target_who = i2;

	/* Hack -- Update the health bar */
	if (health_who == i1) health_track(i2);

	for (j = m_max - 1; j >= 1; j--)
	{
		/* Access the monster */
		monster_type *t_ptr = &m_list[j];

		/* Ignore "dead" monsters */
		if (!t_ptr->r_idx) continue;

		if (t_ptr->target == i1) t_ptr->target = i2;
	}

	/* Structure copy */
	COPY(&m_list[i2], &m_list[i1], monster_type);

	/* Delete mind & special race if needed */
	if (m_list[i1].sr_ptr)
		KILL(m_list[i1].sr_ptr, monster_race);
	if (m_list[i1].mind)
		KILL(m_list[i1].mind, monster_mind);

	/* Wipe the hole */
	m_ptr = WIPE(&m_list[i1], monster_type);
}


/*
 * Compact and Reorder the monster list
 *
 * This function can be very dangerous, use with caution!
 *
 * When actually "compacting" monsters, we base the saving throw
 * on a combination of monster level, distance from player, and
 * current "desperation".
 *
 * After "compacting" (if needed), we "reorder" the monsters into a more
 * compact order, and we reset the allocation info, and the "live" array.
 */
void compact_monsters(int size)
{
	int	i, num, cnt;
	int	cur_lev, cur_dis, chance;


	/* Message (only if compacting) */
	if (size) msg_print("Compacting monsters...");


	/* Compact at least 'size' objects */
	for (num = 0, cnt = 1; num < size; cnt++)
	{
		/* Get more vicious each iteration */
		cur_lev = 5 * cnt;

		/* Get closer each iteration */
		cur_dis = 5 * (20 - cnt);

		/* Check all the monsters */
		for (i = 1; i < m_max; i++)
		{
			monster_type *m_ptr = &m_list[i];

			monster_race *r_ptr = race_inf(m_ptr);

			/* Paranoia -- skip "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Hack -- High level monsters start out "immune" */
			if (m_ptr->level > cur_lev) continue;

			/* Ignore nearby monsters */
			if ((cur_dis > 0) && (m_ptr->cdis < cur_dis)) continue;

			/* Saving throw chance */
			chance = 90;

			/* Only compact "Quest" Monsters in emergencies */
			if ((m_ptr->mflag & MFLAG_QUEST) && (cnt < 1000)) chance = 100;

			/* Try not to compact Unique Monsters */
			if (r_ptr->flags1 & (RF1_UNIQUE)) chance = 99;

			/* All monsters get a saving throw */
			if (rand_int(100) < chance) continue;

			/* Delete the monster */
			delete_monster_idx(i);

			/* Count the monster */
			num++;
		}
	}


	/* Excise dead monsters (backwards!) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Get the i'th monster */
		monster_type *m_ptr = &m_list[i];

		/* Skip real monsters */
		if (m_ptr->r_idx) continue;

		/* Move last monster into open hole */
		compact_monsters_aux(m_max - 1, i);

		/* Compress "m_max" */
		m_max--;
	}
}

/*
 * Delete/Remove all the monsters when the player leaves the level
 *
 * This is an efficient method of simulating multiple calls to the
 * "delete_monster()" function, with no visual effects.
 */
void wipe_m_list(void)
{
	int i;

	/* Delete all the monsters */
	for (i = m_max - 1; i >= 1; i--)
	{
		monster_type *m_ptr = &m_list[i];

		monster_race *r_ptr = race_inf(m_ptr);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Mega-Hack -- preserve Unique's XXX XXX XXX */

		/* Hack -- Reduce the racial counter */
		r_ptr->cur_num--;

		/* Monster is gone */
		cave[m_ptr->fy][m_ptr->fx].m_idx = 0;

		/* Delete mind & special race if needed */
		if (m_ptr->sr_ptr)
			KILL(m_ptr->sr_ptr, monster_race);
		if (m_ptr->mind)
			KILL(m_ptr->mind, monster_mind);


		/* Wipe the Monster */
		m_ptr = WIPE(m_ptr, monster_type);
	}

	/* Reset "m_max" */
	m_max = 1;

	/* Reset "m_cnt" */
	m_cnt = 0;

	/* Hack -- reset "reproducer" count */
	num_repro = 0;

	/* Hack -- no more target */
	target_who = 0;

	/* Reset control */
	p_ptr->control = 0;

	/* Hack -- no more tracking */
	health_track(0);
}


/*
 * Acquires and returns the index of a "free" monster.
 *
 * This routine should almost never fail, but it *can* happen.
 */
s16b m_pop(void)
{
	int i;


	/* Normal allocation */
	if (m_max < max_m_idx)
	{
		/* Access the next hole */
		i = m_max;

		/* Expand the array */
		m_max++;

		/* Count monsters */
		m_cnt++;

		/* Return the index */
		return (i);
	}


	/* Recycle dead monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr;

		/* Acquire monster */
		m_ptr = &m_list[i];

		/* Skip live monsters */
		if (m_ptr->r_idx) continue;

		/* Count monsters */
		m_cnt++;

		/* Use this monster */
		return (i);
	}


	/* Warn the player (except during dungeon creation) */
	if (character_dungeon) msg_print("Too many monsters!");

	/* Try not to crash */
	return (0);
}




/*
 * Apply a "monster restriction function" to the "monster allocation table"
 */
errr get_mon_num_prep(void)
{
	int i;

	/* Scan the allocation table */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Get the entry */
		alloc_entry *entry = &alloc_race_table[i];

		/* Accept monsters which pass the restriction, if any */
		if ((!get_mon_num_hook || (*get_mon_num_hook)(entry->index)) &&
		                (!get_mon_num2_hook || (*get_mon_num2_hook)(entry->index)))
		{
			/* Accept this monster */
			entry->prob2 = entry->prob1;
		}

		/* Do not use this monster */
		else
		{
			/* Decline this monster */
			entry->prob2 = 0;
		}
	}

	/* Success */
	return (0);
}

/*
 * Some dungeon types restrict the possible monsters.
 * Return TRUE is the monster is OK and FALSE otherwise
 */
bool_ apply_rule(monster_race *r_ptr, byte rule)
{
	dungeon_info_type *d_ptr = &d_info[dungeon_type];

	if (d_ptr->rules[rule].mode == DUNGEON_MODE_NONE)
	{
		return TRUE;
	}
	else if ((d_ptr->rules[rule].mode == DUNGEON_MODE_AND) || (d_ptr->rules[rule].mode == DUNGEON_MODE_NAND))
	{
		int a;

		if (d_ptr->rules[rule].mflags1)
		{
			if ((d_ptr->rules[rule].mflags1 & r_ptr->flags1) != d_ptr->rules[rule].mflags1)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags2)
		{
			if ((d_ptr->rules[rule].mflags2 & r_ptr->flags2) != d_ptr->rules[rule].mflags2)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags3)
		{
			if ((d_ptr->rules[rule].mflags3 & r_ptr->flags3) != d_ptr->rules[rule].mflags3)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags4)
		{
			if ((d_ptr->rules[rule].mflags4 & r_ptr->flags4) != d_ptr->rules[rule].mflags4)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags5)
		{
			if ((d_ptr->rules[rule].mflags5 & r_ptr->flags5) != d_ptr->rules[rule].mflags5)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags6)
		{
			if ((d_ptr->rules[rule].mflags6 & r_ptr->flags6) != d_ptr->rules[rule].mflags6)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags7)
		{
			if ((d_ptr->rules[rule].mflags7 & r_ptr->flags7) != d_ptr->rules[rule].mflags7)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags8)
		{
			if ((d_ptr->rules[rule].mflags8 & r_ptr->flags8) != d_ptr->rules[rule].mflags8)
				return FALSE;
		}
		if (d_ptr->rules[rule].mflags9)
		{
			if ((d_ptr->rules[rule].mflags9 & r_ptr->flags9) != d_ptr->rules[rule].mflags9)
				return FALSE;
		}
		for (a = 0; a < 5; a++)
		{
			if (d_ptr->rules[rule].r_char[a] && (d_ptr->rules[rule].r_char[a] != r_ptr->d_char)) return FALSE;
		}

		/* All checks passed ? lets go ! */
		return TRUE;
	}
	else if ((d_ptr->rules[rule].mode == DUNGEON_MODE_OR) || (d_ptr->rules[rule].mode == DUNGEON_MODE_NOR))
	{
		int a;

		if (d_ptr->rules[rule].mflags1 && (r_ptr->flags1 & d_ptr->rules[rule].mflags1)) return TRUE;
		if (d_ptr->rules[rule].mflags2 && (r_ptr->flags2 & d_ptr->rules[rule].mflags2)) return TRUE;
		if (d_ptr->rules[rule].mflags3 && (r_ptr->flags3 & d_ptr->rules[rule].mflags3)) return TRUE;
		if (d_ptr->rules[rule].mflags4 && (r_ptr->flags4 & d_ptr->rules[rule].mflags4)) return TRUE;
		if (d_ptr->rules[rule].mflags5 && (r_ptr->flags5 & d_ptr->rules[rule].mflags5)) return TRUE;
		if (d_ptr->rules[rule].mflags6 && (r_ptr->flags6 & d_ptr->rules[rule].mflags6)) return TRUE;
		if (d_ptr->rules[rule].mflags7 && (r_ptr->flags7 & d_ptr->rules[rule].mflags7)) return TRUE;
		if (d_ptr->rules[rule].mflags8 && (r_ptr->flags8 & d_ptr->rules[rule].mflags8)) return TRUE;
		if (d_ptr->rules[rule].mflags9 && (r_ptr->flags9 & d_ptr->rules[rule].mflags9)) return TRUE;

		for (a = 0; a < 5; a++)
			if (d_ptr->rules[rule].r_char[a] == r_ptr->d_char) return TRUE;

		/* All checks failled ? Sorry ... */
		return FALSE;
	}

	/* Should NEVER happen */
	return FALSE;
}

bool_ restrict_monster_to_dungeon(int r_idx)
{
	dungeon_info_type *d_ptr = &d_info[dungeon_type];
	monster_race *r_ptr = &r_info[r_idx];

	/* Select a random rule */
	byte rule = d_ptr->rule_percents[rand_int(100)];

	/* Apply the rule */
	bool_ rule_ret = apply_rule(r_ptr, rule);

	/* Should the rule be right or not ? */
	if ((d_ptr->rules[rule].mode == DUNGEON_MODE_NAND) || (d_ptr->rules[rule].mode == DUNGEON_MODE_NOR)) rule_ret = !rule_ret;

	/* Rule ok ? */
	if (rule_ret) return TRUE;

	/* Not allowed */
	return FALSE;
}

/* Ugly hack, let summon unappropriate monsters */
bool_ summon_hack = FALSE;

/*
 * Choose a monster race that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "monster allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" monster, in
 * a relatively efficient manner.
 *
 * Note that "town" monsters will *only* be created in the town, and
 * "normal" monsters will *never* be created in the town, unless the
 * "level" is "modified", for example, by polymorph or summoning.
 *
 * There is a small chance (1/50) of "boosting" the given depth by
 * a small amount (up to four levels), except in the town.
 *
 * It is (slightly) more likely to acquire a monster of the given level
 * than one of a lower level.  This is done by choosing several monsters
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that if no monsters are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
s16b get_mon_num(int level)
{
	int	i, j, p;

	int	r_idx;

	long	value, total;

	monster_race	*r_ptr;

	alloc_entry	*table = alloc_race_table;

	int in_tome;

	/* Boost the level */
	if (level > 0)
	{
		/* Occasional "nasty" monster */
		if (rand_int(NASTY_MON) == 0)
		{
			/* Pick a level bonus */
			int d = level / 4 + 2;

			/* Boost the level */
			level += ((d < 5) ? d : 5);
		}

		/* Occasional "nasty" monster */
		if (rand_int(NASTY_MON) == 0)
		{
			/* Pick a level bonus */
			int d = level / 4 + 2;

			/* Boost the level */
			level += ((d < 5) ? d : 5);
		}
	}


	/* Reset total */
	total = 0L;

	/* Check whether this is ToME or a module */
	in_tome = strcmp(game_module, "ToME") == 0;

	/* Process probabilities */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Monsters are sorted by depth */
		if (table[i].level > level) break;

		/* Default */
		table[i].prob3 = 0;

		/* Access the "r_idx" of the chosen monster */
		r_idx = table[i].index;

		/* Access the actual race */
		r_ptr = &r_info[r_idx];

		/* Hack -- "unique" monsters must be "unique" */
		if ((r_ptr->flags1 & (RF1_UNIQUE)) &&
		                (r_ptr->cur_num >= r_ptr->max_num))
		{
			continue;
		}

		/* Depth Monsters never appear out of depth */
		if ((r_ptr->flags1 & (RF1_FORCE_DEPTH)) && (r_ptr->level > dun_level))
		{
			continue;
		}

		/* Depth Monsters never appear out of their depth */
		if ((r_ptr->flags9 & (RF9_ONLY_DEPTH)) && (r_ptr->level != dun_level))
		{
			continue;
		}

		if(in_tome)
		{
			/* Zangbandish monsters not allowed */
			if (r_ptr->flags8 & RF8_ZANGBAND) continue;

			/* Lovecraftian monsters not allowed */
			if (r_ptr->flags8 & RF8_CTHANGBAND) continue;
		}

		/* Joke monsters allowed ? or not ? */
		if (!joke_monsters && (r_ptr->flags8 & RF8_JOKEANGBAND)) continue;

		/* Some dungeon types restrict the possible monsters */
		if (!summon_hack && !restrict_monster_to_dungeon(r_idx) && dun_level) continue;

		/* Accept */
		table[i].prob3 = table[i].prob2;

		/* Total */
		total += table[i].prob3;
	}

	/* No legal monsters */
	if (total <= 0) return (0);


	/* Pick a monster */
	value = rand_int(total);

	/* Find the monster */
	for (i = 0; i < alloc_race_size; i++)
	{
		/* Found the entry */
		if (value < table[i].prob3) break;

		/* Decrement */
		value = value - table[i].prob3;
	}


	/* Power boost */
	p = rand_int(100);

	/* Try for a "harder" monster once (50%) or twice (10%) */
	if (p < 60)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = rand_int(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}

	/* Try for a "harder" monster twice (10%) */
	if (p < 10)
	{
		/* Save old */
		j = i;

		/* Pick a monster */
		value = rand_int(total);

		/* Find the monster */
		for (i = 0; i < alloc_race_size; i++)
		{
			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}


	/* Result */
	return (table[i].index);
}





/*
 * Build a string describing a monster in some way.
 *
 * We can correctly describe monsters based on their visibility.
 * We can force all monsters to be treated as visible or invisible.
 * We can build nominatives, objectives, possessives, or reflexives.
 * We can selectively pronominalize hidden, visible, or all monsters.
 * We can use definite or indefinite descriptions for hidden monsters.
 * We can use definite or indefinite descriptions for visible monsters.
 *
 * Pronominalization involves the gender whenever possible and allowed,
 * so that by cleverly requesting pronominalization / visibility, you
 * can get messages like "You hit someone.  She screams in agony!".
 *
 * Reflexives are acquired by requesting Objective plus Possessive.
 *
 * If no m_ptr arg is given (?), the monster is assumed to be hidden,
 * unless the "Assume Visible" mode is requested.
 *
 * If no r_ptr arg is given, it is extracted from m_ptr and r_info
 * If neither m_ptr nor r_ptr is given, the monster is assumed to
 * be neuter, singular, and hidden (unless "Assume Visible" is set),
 * in which case you may be in trouble... :-)
 *
 * I am assuming that no monster name is more than 70 characters long,
 * so that "char desc[80];" is sufficiently large for any result.
 *
 * Mode Flags:
 *   0x01 --> Objective (or Reflexive)
 *   0x02 --> Possessive (or Reflexive)
 *   0x04 --> Use indefinites for hidden monsters ("something")
 *   0x08 --> Use indefinites for visible monsters ("a kobold")
 *   0x10 --> Pronominalize hidden monsters
 *   0x20 --> Pronominalize visible monsters
 *   0x40 --> Assume the monster is hidden
 *   0x80 --> Assume the monster is visible
 *  0x100 --> Ignore insanity
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Genocide resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visible ("his") or "its"
 *   0x23 --> Reflexive, genderized if visible ("himself") or "itself"
 */
void monster_desc(char *desc, monster_type *m_ptr, int mode)
{
	cptr res;
	monster_race *r_ptr = race_inf(m_ptr);
	cptr b_name = (r_name + r_ptr->name);
	char silly_name[80], name[100];
	bool_ seen, pron;
	int insanity = (p_ptr->msane - p_ptr->csane) * 100 / p_ptr->msane;

	if (m_ptr->ego)
	{
		if (re_info[m_ptr->ego].before) sprintf(name, "%s %s", re_name + re_info[m_ptr->ego].name, b_name);
		else sprintf(name, "%s %s", b_name, re_name + re_info[m_ptr->ego].name);
	}
	else
	{
		sprintf(name, "%s", b_name);
	}

	/*
	 * Are we hallucinating? (Idea from Nethack...)
	 * insanity roll added by pelpel
	 */
	if (!(mode & 0x100) && (p_ptr->image || (rand_int(300) < insanity)))
	{
		if (rand_int(2) == 0)
		{
			monster_race *hallu_race;

			do
			{
				hallu_race = &r_info[randint(max_r_idx - 2)];
			}
			while (hallu_race->flags1 & RF1_UNIQUE);

			strcpy(silly_name, (r_name + hallu_race->name));
		}
		else
		{
			get_rnd_line("silly.txt", silly_name);
		}

		strcpy(name, silly_name);
	}

	/* Can we "see" it (exists + forced, or visible + not unforced) */
	seen = (m_ptr && ((mode & 0x80) || (!(mode & 0x40) && m_ptr->ml)));

	/* Sexed Pronouns (seen and allowed, or unseen and allowed) */
	pron = (m_ptr && ((seen && (mode & 0x20)) || (!seen && (mode & 0x10))));


	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || pron)
	{
		/* an encoding of the monster "sex" */
		int kind = 0x00;

		/* Extract the gender (if applicable) */
		if (r_ptr->flags1 & (RF1_FEMALE)) kind = 0x20;
		else if (r_ptr->flags1 & (RF1_MALE)) kind = 0x10;

		/* Ignore the gender (if desired) */
		if (!m_ptr || !pron) kind = 0x00;


		/* Assume simple result */
		res = "it";

		/* Brute force: split on the possibilities */
		switch (kind | (mode & 0x07))
		{
			/* Neuter, or unknown */
		case 0x00:
			res = "it";
			break;
		case 0x01:
			res = "it";
			break;
		case 0x02:
			res = "its";
			break;
		case 0x03:
			res = "itself";
			break;
		case 0x04:
			res = "something";
			break;
		case 0x05:
			res = "something";
			break;
		case 0x06:
			res = "something's";
			break;
		case 0x07:
			res = "itself";
			break;

			/* Male (assume human if vague) */
		case 0x10:
			res = "he";
			break;
		case 0x11:
			res = "him";
			break;
		case 0x12:
			res = "his";
			break;
		case 0x13:
			res = "himself";
			break;
		case 0x14:
			res = "someone";
			break;
		case 0x15:
			res = "someone";
			break;
		case 0x16:
			res = "someone's";
			break;
		case 0x17:
			res = "himself";
			break;

			/* Female (assume human if vague) */
		case 0x20:
			res = "she";
			break;
		case 0x21:
			res = "her";
			break;
		case 0x22:
			res = "her";
			break;
		case 0x23:
			res = "herself";
			break;
		case 0x24:
			res = "someone";
			break;
		case 0x25:
			res = "someone";
			break;
		case 0x26:
			res = "someone's";
			break;
		case 0x27:
			res = "herself";
			break;
		}

		/* Copy the result */
		(void)strcpy(desc, res);
	}


	/* Handle visible monsters, "reflexive" request */
	else if ((mode & 0x02) && (mode & 0x01))
	{
		/* The monster is visible, so use its gender */
		if (r_ptr->flags1 & (RF1_FEMALE)) strcpy(desc, "herself");
		else if (r_ptr->flags1 & (RF1_MALE)) strcpy(desc, "himself");
		else strcpy(desc, "itself");
	}


	/* Handle all other visible monster requests */
	else
	{
		/* It could be a Unique */
		if ((r_ptr->flags1 & (RF1_UNIQUE)) && !(p_ptr->image))
		{
			/* Start with the name (thus nominative and objective) */
			(void)strcpy(desc, name);
		}

		/* It could be an indefinite monster */
		else if (mode & 0x08)
		{
			/* XXX Check plurality for "some" */

			/* Indefinite monsters need an indefinite article */
			(void)strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ");
			(void)strcat(desc, name);
		}

		/* It could be a normal, definite, monster */
		else
		{
			/* Definite monsters need a definite article */
			if (m_ptr->status >= MSTATUS_PET)
				(void)strcpy(desc, "your ");
			else
				(void)strcpy(desc, "the ");

			(void)strcat(desc, name);
		}

		/* Handle the Possessive as a special afterthought */
		if (mode & 0x02)
		{
			/* XXX Check for trailing "s" */

			/* Simply append "apostrophe" and "s" */
			(void)strcat(desc, "'s");
		}
	}
}

void monster_race_desc(char *desc, int r_idx, int ego)
{
	monster_race *r_ptr = &r_info[r_idx];
	cptr b_name = (r_name + r_ptr->name);
	char name[80];

	if (ego)
	{
		if (re_info[ego].before) sprintf(name, "%s %s", re_name + re_info[ego].name, b_name);
		else sprintf(name, "%s %s", b_name, re_name + re_info[ego].name);
	}
	else
	{
		sprintf(name, "%s", b_name);
	}

	/* It could be a Unique */
	if (r_ptr->flags1 & RF1_UNIQUE)
	{
		/* Start with the name (thus nominative and objective) */
		(void)strcpy(desc, name);
	}

	/* It could be a normal, definite, monster */
	else
	{
		/* Definite monsters need a definite article */
		(void)strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ");

		(void)strcat(desc, name);
	}
}



/*
 * Learn about a monster (by "probing" it)
 */
void lore_do_probe(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Hack -- Memorize some flags */
	r_ptr->r_flags1 = r_ptr->flags1;
	r_ptr->r_flags2 = r_ptr->flags2;
	r_ptr->r_flags3 = r_ptr->flags3;

	/* Update monster recall window */
	if (monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}
}


/*
 * Take note that the given monster just dropped some treasure
 *
 * Note that learning the "GOOD"/"GREAT" flags gives information
 * about the treasure (even when the monster is killed for the first
 * time, such as uniques, and the treasure has not been examined yet).
 *
 * This "indirect" method is used to prevent the player from learning
 * exactly how much treasure a monster can drop from observing only
 * a single example of a drop.  This method actually observes how much
 * gold and items are dropped, and remembers that information to be
 * described later by the monster recall code.
 */
void lore_treasure(int m_idx, int num_item, int num_gold)
{
	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Note the number of things dropped */
	if (num_item > r_ptr->r_drop_item) r_ptr->r_drop_item = num_item;
	if (num_gold > r_ptr->r_drop_gold) r_ptr->r_drop_gold = num_gold;

	/* Hack -- memorize the good/great flags */
	if (r_ptr->flags1 & (RF1_DROP_GOOD)) r_ptr->r_flags1 |= (RF1_DROP_GOOD);
	if (r_ptr->flags1 & (RF1_DROP_GREAT)) r_ptr->r_flags1 |= (RF1_DROP_GREAT);

	/* Update monster recall window */
	if (monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}
}



void sanity_blast(monster_type * m_ptr, bool_ necro)
{
	bool_ happened = FALSE;
	int power = 100;

	if (!necro)
	{
		char m_name[80];
		monster_race *r_ptr;

		if (m_ptr != NULL) r_ptr = race_inf(m_ptr);
		else return;

		power = (m_ptr->level) + 10;

		if (m_ptr != NULL)
		{
			monster_desc(m_name, m_ptr, 0);

			if (!(r_ptr->flags1 & RF1_UNIQUE))
			{
				if (r_ptr->flags1 & RF1_FRIENDS)
					power /= 2;
			}
			else power *= 2;

			if (!hack_mind)
				return ;  /* No effect yet, just loaded... */

			if (!(m_ptr->ml))
				return ;  /* Cannot see it for some reason */

			if (!(r_ptr->flags2 & RF2_ELDRITCH_HORROR))
				return ;  /* oops */



			if ((is_friend(m_ptr) > 0) && (randint(8) != 1))
				return ;  /* Pet eldritch horrors are safe most of the time */


			if (randint(power) < p_ptr->skill_sav)
			{
				return ;  /* Save, no adverse effects */
			}


			if (p_ptr->image)
			{
				/* Something silly happens... */
				msg_format("You behold the %s visage of %s!",
				           funny_desc[(randint(MAX_FUNNY)) - 1], m_name);
				if (randint(3) == 1)
				{
					msg_print(funny_comments[randint(MAX_COMMENT) - 1]);
					p_ptr->image = (p_ptr->image + randint(m_ptr->level));
				}
				return ;  /* Never mind; we can't see it clearly enough */
			}

			/* Something frightening happens... */
			msg_format("You behold the %s visage of %s!",
			           horror_desc[(randint(MAX_HORROR)) - 1], m_name);

			r_ptr->r_flags2 |= RF2_ELDRITCH_HORROR;

		}

		/* Undead characters are 50% likely to be unaffected */
		if ((PRACE_FLAG(PR1_UNDEAD)) || (p_ptr->mimic_form == resolve_mimic_name("Vampire")))
		{
			if (randint(100) < (25 + (p_ptr->lev))) return;
		}
	}
	else
	{
		msg_print("Your sanity is shaken by reading the Necronomicon!");
	}
	if (randint(power) < p_ptr->skill_sav) /* Mind blast */
	{
		if (!p_ptr->resist_conf)
		{
			(void)set_confused(p_ptr->confused + rand_int(4) + 4);
		}
		if ((!p_ptr->resist_chaos) && (randint(3) == 1))
		{
			(void) set_image(p_ptr->image + rand_int(250) + 150);
		}
		return;
	}

	if (randint(power) < p_ptr->skill_sav) /* Lose int & wis */
	{
		do_dec_stat (A_INT, STAT_DEC_NORMAL);
		do_dec_stat (A_WIS, STAT_DEC_NORMAL);
		return;
	}


	if (randint(power) < p_ptr->skill_sav) /* Brain smash */
	{
		if (!p_ptr->resist_conf)
		{
			(void)set_confused(p_ptr->confused + rand_int(4) + 4);
		}
		if (!p_ptr->free_act)
		{
			(void)set_paralyzed(p_ptr->paralyzed + rand_int(4) + 4);
		}
		while (rand_int(100) > p_ptr->skill_sav)
			(void)do_dec_stat(A_INT, STAT_DEC_NORMAL);
		while (rand_int(100) > p_ptr->skill_sav)
			(void)do_dec_stat(A_WIS, STAT_DEC_NORMAL);
		if (!p_ptr->resist_chaos)
		{
			(void) set_image(p_ptr->image + rand_int(250) + 150);
		}
		return;
	}

	if (randint(power) < p_ptr->skill_sav) /* Permanent lose int & wis */
	{
		if (dec_stat(A_INT, 10, TRUE)) happened = TRUE;
		if (dec_stat(A_WIS, 10, TRUE)) happened = TRUE;
		if (happened)
			msg_print("You feel much less sane than before.");
		return;
	}


	if (randint(power) < p_ptr->skill_sav) /* Amnesia */
	{

		if (lose_all_info())
			msg_print("You forget everything in your utmost terror!");
		return;
	}

	p_ptr->update |= PU_BONUS;
	handle_stuff();
}


/*
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player, checking
 * for visibility (natural, infravision, see-invis, telepathy),
 * updating the monster visibility flag, redrawing or erasing the
 * monster when the visibility changes, and taking note of any
 * "visual" features of the monster (cold-blooded, invisible, etc).
 *
 * The only monster fields that are changed here are "cdis" (the
 * distance from the player), "los" (clearly visible to player),
 * and "ml" (visible to the player in any way).
 *
 * There are a few cases where the calling routine knows that the
 * distance from the player to the monster has not changed, and so
 * we have a special parameter "full" to request distance computation.
 * This lets many calls to this function run very quickly.
 *
 * Note that every time a monster moves, we must call this function
 * for that monster, and update distance.  Note that every time the
 * player moves, we must call this function for every monster, and
 * update distance.  Note that every time the player "state" changes
 * in certain ways (including "blindness", "infravision", "telepathy",
 * and "see invisible"), we must call this function for every monster.
 *
 * The routines that actually move the monsters call this routine
 * directly, and the ones that move the player, or notice changes
 * in the player state, call "update_monsters()".
 *
 * Routines that change the "illumination" of grids must also call
 * this function, since the "visibility" of some monsters may be
 * based on the illumination of their grid.
 *
 * Note that this function is called once per monster every time the
 * player moves, so it is important to optimize it for monsters which
 * are far away.  Note the optimization which skips monsters which
 * are far away and were completely invisible last turn.
 *
 * Note the optimized "inline" version of the "distance()" function.
 *
 * Note that only monsters on the current panel can be "visible",
 * and then only if they are (1) in line of sight and illuminated
 * by light or infravision, or (2) nearby and detected by telepathy.
 *
 * The player can choose to be disturbed by several things, including
 * "disturb_move" (monster which is viewable moves in some way), and
 * "disturb_near" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 *
 * Note the new "xtra" field which encodes several state flags such
 * as "detected last turn", and "detected this turn", and "currently
 * in line of sight", all of which are used for visibility testing.
 */
void update_mon(int m_idx, bool_ full)
{
	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = race_inf(m_ptr);

	/* The current monster location */
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	bool_ old_ml = m_ptr->ml;

	/* Seen at all */
	bool_ flag = FALSE;

	/* Seen by vision */
	bool_ easy = FALSE;

	/* Seen by telepathy */
	bool_ hard = FALSE;

	/* Various extra flags */
	bool_ do_empty_mind = FALSE;
	bool_ do_weird_mind = FALSE;
	bool_ do_invisible = FALSE;
	bool_ do_cold_blood = FALSE;


	/* Calculate distance */
	if (full)
	{
		int d, dy, dx;

		/* Distance components */
		dy = (p_ptr->py > fy) ? (p_ptr->py - fy) : (fy - p_ptr->py);
		dx = (p_ptr->px > fx) ? (p_ptr->px - fx) : (fx - p_ptr->px);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

		/* Save the distance (in a byte) */
		m_ptr->cdis = (d < 255) ? d : 255;
	}


	/* Process "distant" monsters */
	if (m_ptr->cdis > MAX_SIGHT)
	{
		/* Ignore unseen monsters */
		if (!m_ptr->ml) return;

		/* Detected */
		if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;
	}


	/* Process "nearby" monsters on the current "panel" */
	else if (panel_contains(fy, fx))
	{
		/* Normal line of sight, and player is not blind */
		if (player_has_los_bold(fy, fx) && !p_ptr->blind)
		{
			/* Use "infravision" */
			if (m_ptr->cdis <= (byte)(p_ptr->see_infra))
			{
				/* Infravision only works on "warm" creatures */
				/* Below, we will need to know that infravision failed */
				if (r_ptr->flags2 & (RF2_COLD_BLOOD)) do_cold_blood = TRUE;

				/* Infravision works */
				if (!do_cold_blood) easy = flag = TRUE;
			}

			/* Use "illumination" */
			if (player_can_see_bold(fy, fx))
			{
				/* Take note of invisibility */
				if (r_ptr->flags2 & (RF2_INVISIBLE)) do_invisible = TRUE;

				/* Visible, or detectable, monsters get seen */
				if (!do_invisible || p_ptr->see_inv) easy = flag = TRUE;
			}
		}

		/* Telepathy can see all "nearby" monsters with "minds" */
		if (p_ptr->telepathy)
		{
			/* Assume we cant see */
			bool_ can_esp = FALSE;

			/* Different ESP */
			if ((p_ptr->telepathy & ESP_ORC) && (r_ptr->flags3 & RF3_ORC)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_SPIDER) && (r_ptr->flags7 & RF7_SPIDER)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_TROLL) && (r_ptr->flags3 & RF3_TROLL)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_DRAGON) && (r_ptr->flags3 & RF3_DRAGON)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_GIANT) && (r_ptr->flags3 & RF3_GIANT)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_DEMON) && (r_ptr->flags3 & RF3_DEMON)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_UNDEAD) && (r_ptr->flags3 & RF3_UNDEAD)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_EVIL) && (r_ptr->flags3 & RF3_EVIL)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_ANIMAL) && (r_ptr->flags3 & RF3_ANIMAL)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_THUNDERLORD) && (r_ptr->flags3 & RF3_THUNDERLORD)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_GOOD) && (r_ptr->flags3 & RF3_GOOD)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_NONLIVING) && (r_ptr->flags3 & RF3_NONLIVING)) can_esp = TRUE;
			if ((p_ptr->telepathy & ESP_UNIQUE) && ((r_ptr->flags1 & RF1_UNIQUE) || (r_ptr->flags3 & RF3_UNIQUE_4))) can_esp = TRUE;
			if (p_ptr->telepathy & ESP_ALL) can_esp = TRUE;

			/* Only do this when we can really detect monster */
			if (can_esp)
			{
				/* Empty mind, no telepathy */
				if (r_ptr->flags2 & (RF2_EMPTY_MIND))
				{
					do_empty_mind = TRUE;
				}

				/* Weird mind, occasional telepathy */
				else if (r_ptr->flags2 & (RF2_WEIRD_MIND))
				{
					do_weird_mind = TRUE;
					if (rand_int(100) < 10)
					{
						hard = TRUE;
						flag = TRUE;
					}
				}

				/* Normal mind, allow telepathy */
				else
				{
					hard = TRUE;
					flag = TRUE;
				}
			}
		}

		/* Apply "detection" spells */
		if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;

		/* Hack -- Wizards have "perfect telepathy" */
		if (wizard) flag = TRUE;
	}


	/* The monster is now visible */
	if (flag)
	{
		/* It was previously unseen */
		if (!m_ptr->ml)
		{
			/* Mark as visible */
			m_ptr->ml = TRUE;

			/* Draw the monster */
			lite_spot(fy, fx);

			/* Update health bar as needed */
			if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Update monster list window */
			p_ptr->window |= (PW_M_LIST);

			/* Hack -- Count "fresh" sightings */
			if (r_ptr->r_sights < MAX_SHORT) r_ptr->r_sights++;

			/* Disturb on appearance */
			if (disturb_move)
			{
				if (disturb_pets || (is_friend(m_ptr) <= 0)) disturb(1, 0);
			}
		}

		/* Apply telepathy */
		if (hard)
		{
			/* Hack -- Memorize mental flags */
			if (r_ptr->flags2 & (RF2_SMART)) r_ptr->r_flags2 |= (RF2_SMART);
			if (r_ptr->flags2 & (RF2_STUPID)) r_ptr->r_flags2 |= (RF2_STUPID);
		}

		/* Memorize various observable flags */
		if (do_empty_mind) r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
		if (do_weird_mind) r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
		if (do_cold_blood) r_ptr->r_flags2 |= (RF2_COLD_BLOOD);
		if (do_invisible) r_ptr->r_flags2 |= (RF2_INVISIBLE);
	}

	/* The monster is not visible */
	else
	{
		/* It was previously seen */
		if (m_ptr->ml)
		{
			/* Mark as not visible */
			m_ptr->ml = FALSE;

			/* Erase the monster */
			lite_spot(fy, fx);

			/* Update health bar as needed */
			if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Update monster list window */
			p_ptr->window |= (PW_M_LIST);

			/* Disturb on disappearance*/
			if (disturb_move)
			{
				if (disturb_pets || (is_friend(m_ptr) <= 0)) disturb(1, 0);
			}
		}
	}


	/* The monster is now easily visible */
	if (easy)
	{

		if (m_ptr->ml != old_ml)
		{
			if (r_ptr->flags2 & RF2_ELDRITCH_HORROR)
			{
				sanity_blast(m_ptr, FALSE);
			}
		}

		/* Change */
		if (!(m_ptr->mflag & (MFLAG_VIEW)))
		{
			/* Mark as easily visible */
			m_ptr->mflag |= (MFLAG_VIEW);

			/* Disturb on appearance */
			if (disturb_near)
			{
				if (disturb_pets || (is_friend(m_ptr) <= 0)) disturb(1, 0);
			}

		}
	}

	/* The monster is not easily visible */
	else
	{
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW))
		{
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Update monster list window */
			p_ptr->window |= (PW_M_LIST);

			/* Disturb on disappearance */
			if (disturb_near)
			{
				if (disturb_pets || (is_friend(m_ptr) <= 0)) disturb(1, 0);
			}
		}
	}
}




/*
 * This function simply updates all the (non-dead) monsters (see above).
 */
void update_monsters(bool_ full)
{
	int i;

	/* Update each (live) monster */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Update the monster */
		update_mon(i, full);
	}
}


void monster_carry(monster_type *m_ptr, int m_idx, object_type *q_ptr)
{
	object_type *o_ptr;

	/* Get new object */
	int o_idx = o_pop();

	if (o_idx)
	{
		/* Get the item */
		o_ptr = &o_list[o_idx];

		/* Structure copy */
		object_copy(o_ptr, q_ptr);

		/* Build a stack */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		o_ptr->held_m_idx = m_idx;
		o_ptr->ix = 0;
		o_ptr->iy = 0;

		m_ptr->hold_o_idx = o_idx;
	}

	else
	{
		/* Hack -- Preserve artifacts */
		if (q_ptr->name1)
		{
			a_info[q_ptr->name1].cur_num = 0;
		}
		else if (k_info[q_ptr->k_idx].flags3 & TR3_NORM_ART)
		{
			k_info[q_ptr->k_idx].artifact = 0;
		}
		else if (q_ptr->tval == TV_RANDART)
		{
			random_artifacts[q_ptr->sval].generated = FALSE;
		}
	}
}


static int possible_randart[] =
{
	TV_MSTAFF,
	TV_BOOMERANG,
	TV_DIGGING,
	TV_HAFTED,
	TV_POLEARM,
	TV_AXE,
	TV_SWORD,
	TV_BOOTS,
	TV_GLOVES,
	TV_HELM,
	TV_CROWN,
	TV_SHIELD,
	TV_CLOAK,
	TV_SOFT_ARMOR,
	TV_HARD_ARMOR,
	TV_LITE,
	TV_AMULET,
	TV_RING,
	-1,
};


bool_ kind_is_randart(int k_idx)
{
	int max;
	object_kind *k_ptr = &k_info[k_idx];

	if (!kind_is_legal(k_idx)) return (FALSE);

	for (max = 0; possible_randart[max] != -1; max++)
	{
		if (k_ptr->tval == possible_randart[max]) return (TRUE);
	}
	return (FALSE);
}

/*
 * Attempt to place a monster of the given race at the given location.
 *
 * To give the player a sporting chance, any monster that appears in
 * line-of-sight and is extremely dangerous can be marked as
 * "FORCE_SLEEP", which will cause them to be placed with low energy,
 * which often (but not always) lets the player move before they do.
 *
 * This routine refuses to place out-of-depth "FORCE_DEPTH" monsters.
 *
 * XXX XXX XXX Use special "here" and "dead" flags for unique monsters,
 * remove old "cur_num" and "max_num" fields.
 *
 * XXX XXX XXX Actually, do something similar for artifacts, to simplify
 * the "preserve" mode, and to make the "what artifacts" flag more useful.
 *
 * This is the only function which may place a monster in the dungeon,
 * except for the savefile loading code.
 */
bool_ bypass_r_ptr_max_num = FALSE;
int place_monster_result = 0;
bool_ place_monster_one_no_drop = FALSE;
monster_race *place_monster_one_race = NULL;
s16b place_monster_one(int y, int x, int r_idx, int ego, bool_ slp, int status)
{
	int i;
	char dummy[5];
	bool_ add_level = FALSE;
	int min_level = 0, max_level = 0;

	cave_type *c_ptr;

	monster_type *m_ptr;

	monster_race *r_ptr = &r_info[r_idx];

	cptr name = (r_name + r_ptr->name);

	/* Grab the special race if needed */
	if (place_monster_one_race)
	{
		r_ptr = place_monster_one_race;
	}

	/* DO NOT PLACE A MONSTER IN THE SMALL SCALE WILDERNESS !!! */
	if (p_ptr->wild_mode)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Verify location */
	if (!in_bounds(y, x))
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Require empty space */
	if (!cave_empty_bold(y, x))
	{
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster(%d): EMPTY BOLD", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Require no monster free grid, or special permission */
	if ((cave[y][x].info & CAVE_FREE) && (!m_allow_special[r_idx]))
	{
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster(%d): CAVE_FREE", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Hack -- no creation on glyph of warding */
	if (cave[y][x].feat == FEAT_GLYPH)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}
	if (cave[y][x].feat == FEAT_MINOR_GLYPH)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Nor on the between */
	if (cave[y][x].feat == FEAT_BETWEEN)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Nor on the altars */
	if ((cave[y][x].feat >= FEAT_ALTAR_HEAD)
	                && (cave[y][x].feat <= FEAT_ALTAR_TAIL))
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Nor on the Pattern */
	if ((cave[y][x].feat >= FEAT_PATTERN_START)
	                && (cave[y][x].feat <= FEAT_PATTERN_XTRA2))
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Paranoia */
	if (!r_idx)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Paranoia */
	if (!r_ptr->name)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Are we allowed to continue ? */
	if (process_hooks(HOOK_NEW_MONSTER, "(d)", r_idx))
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Ego Uniques are NOT to be created */
	if ((r_ptr->flags1 & RF1_UNIQUE) && ego)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Now could we generate an Ego Monster */
	/* Grab the special race if needed */
	if (place_monster_one_race)
	{
		r_ptr = place_monster_one_race;
	}
	else
	{
		r_ptr = race_info_idx(r_idx, ego);
	}

	if (!monster_can_cross_terrain(cave[y][x].feat, r_ptr))
	{
		if (wizard) cmsg_print(TERM_L_RED, "WARNING: Refused monster: cannot cross terrain");
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Unallow some uniques to be generated outside of their quests/special levels/dungeons */
	if ((r_ptr->flags9 & RF9_SPECIAL_GENE) && (!m_allow_special[r_idx]))
	{
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster(%d): SPECIAL_GENE", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Disallow Spirits in The Void, now this *IS* an ugly hack, I hate to do it ... */
	if ((r_ptr->flags7 & RF7_SPIRIT) && (dungeon_type != DUNGEON_VOID))
	{
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster(%d): SPIRIT in non VOID", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Fully forbid it */
	if (r_ptr->flags9 & RF9_NEVER_GENE)
	{
		if (wizard) cmsg_print(TERM_L_RED, "WARNING: Refused monster: NEVER_GENE");
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Hack -- "unique" monsters must be "unique" */
	if ((r_ptr->flags1 & (RF1_UNIQUE)) && (r_ptr->max_num == -1) && (!m_allow_special[r_idx]))
	{
		/* Cannot create */
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster %d: unique not unique", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* The monster is already on an unique level */
	if (r_ptr->on_saved)
	{
		if (wizard) cmsg_print(TERM_L_RED, "WARNING: Refused monster: unique already on saved level");
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Hack -- "unique" monsters must be "unique" */
	if ((r_ptr->flags1 & (RF1_UNIQUE)) && (r_ptr->cur_num >= r_ptr->max_num) && (r_ptr->max_num != -1) && (!bypass_r_ptr_max_num))
	{
		/* Cannot create */
		if (wizard) cmsg_format(TERM_L_RED, "WARNING: Refused monster %d: cur_num >= max_num", r_idx);
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Depth monsters may NOT be created out of depth */
	if ((r_ptr->flags1 & (RF1_FORCE_DEPTH)) && (dun_level < r_ptr->level))
	{
		/* Cannot create */
		if (wizard) cmsg_print(TERM_L_RED, "WARNING: FORCE_DEPTH");
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}

	/* Powerful monster */
	if (r_ptr->level > dun_level)
	{
		/* Unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			/* Message for cheaters */
			if ((cheat_hear) || (p_ptr->precognition)) msg_format("Deep Unique (%s).", name);

			/* Boost rating by twice delta-depth */
			rating += (r_ptr->level - dun_level) * 2;
		}

		/* Normal monsters */
		else
		{
			/* Message for cheaters */
			if ((cheat_hear) || (p_ptr->precognition)) msg_format("Deep Monster (%s).", name);

			/* Boost rating by delta-depth */
			rating += (r_ptr->level - dun_level);
		}
	}

	/* Note the monster */
	else if (r_ptr->flags1 & (RF1_UNIQUE))
	{
		/* Unique monsters induce message */
		if ((cheat_hear) || (p_ptr->precognition)) msg_format("Unique (%s).", name);
	}


	/* Access the location */
	c_ptr = &cave[y][x];

	/* Make a new monster */
	c_ptr->m_idx = m_pop();
	hack_m_idx_ii = c_ptr->m_idx;

	/* Mega-Hack -- catch "failure" */
	if (!c_ptr->m_idx)
	{
		if (place_monster_one_race) KILL(place_monster_one_race, monster_race);
		return 0;
	}


	/* Get a new monster record */
	m_ptr = &m_list[c_ptr->m_idx];

	/* Save the race */
	m_ptr->r_idx = r_idx;
	m_ptr->ego = ego;

	/* No special, no mind */
	m_ptr->sr_ptr = place_monster_one_race;
	m_ptr->mind = NULL;

	/* Place the monster at the location */
	m_ptr->fy = y;
	m_ptr->fx = x;

	/* No "damage" yet */
	m_ptr->stunned = 0;
	m_ptr->confused = 0;
	m_ptr->monfear = 0;

	/* No target yet */
	m_ptr->target = -1;

	/* Unknown distance */
	m_ptr->cdis = 0;

	/* No flags */
	m_ptr->mflag = 0;

	/* Not visible */
	m_ptr->ml = FALSE;

	/* No objects yet */
	m_ptr->hold_o_idx = 0;

	m_ptr->status = status;

	/* Friendly? */
	if (m_ptr->status < MSTATUS_FRIEND && r_ptr->flags7 & RF7_PET)
	{
		m_ptr->status = MSTATUS_FRIEND;
	}
	if (m_ptr->status < MSTATUS_NEUTRAL && r_ptr->flags7 & RF7_NEUTRAL)
	{
		m_ptr->status = MSTATUS_NEUTRAL;
	}

	/* Assume no sleeping */
	m_ptr->csleep = 0;

	/* Enforce sleeping if needed */
	if (slp && r_ptr->sleep)
	{
		int val = r_ptr->sleep;
		m_ptr->csleep = ((val * 2) + randint(val * 10));
	}

	/* Generate the monster's inventory(if any) */
	/* Only if not fated to die */
	if ((dungeon_type != DUNGEON_DEATH) && (!place_monster_one_no_drop))
	{
		bool_ good = (r_ptr->flags1 & (RF1_DROP_GOOD)) ? TRUE : FALSE;
		bool_ great = (r_ptr->flags1 & (RF1_DROP_GREAT)) ? TRUE : FALSE;

		bool_ do_gold = (!(r_ptr->flags1 & (RF1_ONLY_ITEM)));
		bool_ do_item = (!(r_ptr->flags1 & (RF1_ONLY_GOLD)));
		bool_ do_mimic = (r_ptr->flags9 & (RF9_MIMIC));
		int j;

		int force_coin = get_coin_type(r_ptr);

		int dump_item = 0;
		int dump_gold = 0;
		object_type forge;
		object_type *q_ptr;

		int number = 0;

		/* Average dungeon and monster levels */
		object_level = (dun_level + r_ptr->level) / 2;

		/* Determine how much we can drop */
		if ((r_ptr->flags1 & (RF1_DROP_60)) && (rand_int(100) < 60)) number++;
		if ((r_ptr->flags1 & (RF1_DROP_90)) && (rand_int(100) < 90)) number++;
		if (r_ptr->flags1 & (RF1_DROP_1D2)) number += damroll(1, 2);
		if (r_ptr->flags1 & (RF1_DROP_2D2)) number += damroll(2, 2);
		if (r_ptr->flags1 & (RF1_DROP_3D2)) number += damroll(3, 2);
		if (r_ptr->flags1 & (RF1_DROP_4D2)) number += damroll(4, 2);
		if (r_ptr->flags9 & (RF9_MIMIC)) number = 1;

		/* Hack -- handle creeping coins */
		coin_type = force_coin;

		if (r_ptr->flags7 & RF7_DROP_RANDART)
		{
			int tries = 1000;
			obj_theme theme;
			int i;

			/* Get local object */
			q_ptr = &forge;

			theme.treasure = 101;
			theme.combat = 101;
			theme.magic = 101;
			theme.tools = 101;

			init_match_theme(theme);

			/* Apply restriction */
			get_obj_num_hook = kind_is_legal;

			/* Rebuild allocation table */
			get_obj_num_prep();

			i = 0;
			while (tries)
			{
				tries--;
				i = get_obj_num(dun_level);
				if (!i) continue;

				if (!kind_is_randart(i)) continue;
				break;
			}

			/* Invalidate the cached allocation table */
			alloc_kind_table_valid = FALSE;

			if (tries)
			{
				object_prep(q_ptr, i);
				create_artifact(q_ptr, FALSE, TRUE);
				q_ptr->found = OBJ_FOUND_MONSTER;
				q_ptr->found_aux1 = m_ptr->r_idx;
				q_ptr->found_aux2 = m_ptr->ego;
				q_ptr->found_aux3 = dungeon_type;
				q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);

				monster_carry(m_ptr, c_ptr->m_idx, q_ptr);
			}
		}

		/* Drop some objects */
		for (j = 0; j < number; j++)
		{
			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Make Gold */
			if ((!do_mimic) && do_gold && (!do_item || (rand_int(100) < 50)))
			{
				/* Make some gold */
				if (!make_gold(q_ptr)) continue;

				/* XXX XXX XXX */
				dump_gold++;
			}

			/* Make Object */
			else
			{
				/* Make an object */
				if (!do_mimic)
				{
					if (!make_object(q_ptr, good, great, r_ptr->drops)) continue;
				}
				else
				{
					/* Try hard for mimics */
					int tries = 1000;

					while (tries--)
					{
						if (make_object(q_ptr, good, great, r_ptr->drops)) break;
					}
					/* BAD */
					if (!tries) continue;
				}

				/* XXX XXX XXX */
				dump_item++;
			}

			q_ptr->found = OBJ_FOUND_MONSTER;
			q_ptr->found_aux1 = m_ptr->r_idx;
			q_ptr->found_aux2 = m_ptr->ego;
			q_ptr->found_aux3 = dungeon_type;
			q_ptr->found_aux4 = level_or_feat(dungeon_type, dun_level);
			monster_carry(m_ptr, c_ptr->m_idx, q_ptr);
		}

		/* Reset the object level */
		object_level = dun_level;

		/* Reset "coin" type */
		coin_type = 0;
	}
	place_monster_one_no_drop = FALSE;


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

	/* Hack -- small racial variety */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		/* Allow some small variation per monster */
		i = extract_energy[m_ptr->speed] / 10;
		if (i) m_ptr->mspeed += rand_spread(0, i);
	}


	if (dungeon_flags2 & DF2_ADJUST_LEVEL_1_2)
	{
		min_level = max_level = dun_level / 2;
		add_level = TRUE;
	}
	if (dungeon_flags1 & DF1_ADJUST_LEVEL_1)
	{
		if (!min_level) min_level = dun_level;
		max_level = dun_level;
		add_level = TRUE;
	}
	if (dungeon_flags1 & DF1_ADJUST_LEVEL_2)
	{
		if (!min_level) min_level = dun_level * 2;
		max_level = dun_level * 2;
		add_level = TRUE;
	}
	if (add_level) monster_set_level(c_ptr->m_idx, rand_range(min_level, max_level));

	/* Give a random starting energy */
	m_ptr->energy = (byte)rand_int(100);

	/* Force monster to wait for player */
	if (r_ptr->flags1 & (RF1_FORCE_SLEEP))
	{
		/* Monster is still being nice */
		m_ptr->mflag |= (MFLAG_NICE);

		/* Must repair monsters */
		repair_monsters = TRUE;
	}

	/* Hack -- see "process_monsters()" */
	if (c_ptr->m_idx < hack_m_idx)
	{
		/* Monster is still being born */
		m_ptr->mflag |= (MFLAG_BORN);
	}


	/* Update the monster */
	update_mon(c_ptr->m_idx, TRUE);


	/* Hack -- Count the number of "reproducers" */
	if (r_ptr->flags4 & (RF4_MULTIPLY)) num_repro++;


	/* Hack -- Notice new multi-hued monsters */
	if (r_ptr->flags1 & (RF1_ATTR_MULTI)) shimmer_monsters = TRUE;

	/* Hack -- we need to modify the REAL r_info, not the fake one */
	r_ptr = &r_info[r_idx];

	/* Hack -- Count the monsters on the level */
	r_ptr->cur_num++;

	/* Unique monsters on saved levels should be "marked" */
	if ((r_ptr->flags1 & RF1_UNIQUE) && get_dungeon_save(dummy))
	{
		r_ptr->on_saved = TRUE;
	}

	place_monster_one_race = NULL;

	/* Processs hooks */
	{
		hook_new_monster_end_in in = { m_ptr };
		process_hooks_new(HOOK_NEW_MONSTER_END, &in, NULL);
	}

	/* Success */
	place_monster_result = c_ptr->m_idx;
	return c_ptr->m_idx;
}

/*
 * Maximum size of a group of monsters
 */
#define GROUP_MAX	32


/*
 * Attempt to place a "group" of monsters around the given location
 */
static bool_ place_monster_group(int y, int x, int r_idx, bool_ slp, int status)
{
	monster_race *r_ptr = &r_info[r_idx];

	int old, n, i;
	int total = 0, extra = 0;

	int hack_n = 0;

	byte hack_y[GROUP_MAX];
	byte hack_x[GROUP_MAX];


	/* Pick a group size */
	total = randint(13);

	/* Hard monsters, small groups */
	if (r_ptr->level > dun_level)
	{
		extra = r_ptr->level - dun_level;
		extra = 0 - randint(extra);
	}

	/* Easy monsters, large groups */
	else if (r_ptr->level < dun_level)
	{
		extra = dun_level - r_ptr->level;
		extra = randint(extra);
	}

	/* Hack -- limit group reduction */
	if (extra > 12) extra = 12;

	/* Modify the group size */
	total += extra;

	/* Minimum size */
	if (total < 1) total = 1;

	/* Maximum size */
	if (total > GROUP_MAX) total = GROUP_MAX;


	/* Save the rating */
	old = rating;

	/* Start on the monster */
	hack_n = 1;
	hack_x[0] = x;
	hack_y[0] = y;

	/* Puddle monsters, breadth first, up to total */
	for (n = 0; (n < hack_n) && (hack_n < total); n++)
	{
		/* Grab the location */
		int hx = hack_x[n];
		int hy = hack_y[n];

		/* Check each direction, up to total */
		for (i = 0; (i < 8) && (hack_n < total); i++)
		{
			int mx = hx + ddx_ddd[i];
			int my = hy + ddy_ddd[i];

			/* Walls and Monsters block flow */
			if (!cave_empty_bold(my, mx)) continue;

			/* Attempt to place another monster */
			if (place_monster_one(my, mx, r_idx, pick_ego_monster(r_idx), slp, status))
			{
				/* Add it to the "hack" set */
				hack_y[hack_n] = my;
				hack_x[hack_n] = mx;
				hack_n++;
			}
		}
	}

	/* Hack -- restore the rating */
	rating = old;


	/* Success */
	return (TRUE);
}


/*
 * Hack -- help pick an escort type
 */
static int place_monster_idx = 0;

/*
 * Hack -- help pick an escort type
 */
static bool_ place_monster_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[place_monster_idx];

	monster_race *z_ptr = &r_info[r_idx];

	/* Hack - Escorts have to have the same dungeon flag */
	if (monster_dungeon(place_monster_idx) != monster_dungeon(r_idx)) return (FALSE);

	/* Require similar "race" */
	if (z_ptr->d_char != r_ptr->d_char) return (FALSE);

	/* Skip more advanced monsters */
	if (z_ptr->level > r_ptr->level) return (FALSE);

	/* Skip unique monsters */
	if (z_ptr->flags1 & (RF1_UNIQUE)) return (FALSE);

	/* Paranoia -- Skip identical monsters */
	if (place_monster_idx == r_idx) return (FALSE);

	/* Okay */
	return (TRUE);
}


/*
 * Attempt to place a monster of the given race at the given location
 *
 * Note that certain monsters are now marked as requiring "friends".
 * These monsters, if successfully placed, and if the "grp" parameter
 * is TRUE, will be surrounded by a "group" of identical monsters.
 *
 * Note that certain monsters are now marked as requiring an "escort",
 * which is a collection of monsters with similar "race" but lower level.
 *
 * Some monsters induce a fake "group" flag on their escorts.
 *
 * Note the "bizarre" use of non-recursion to prevent annoying output
 * when running a code profiler.
 *
 * Note the use of the new "monster allocation table" code to restrict
 * the "get_mon_num()" function to "legal" escort types.
 */
bool_ place_monster_aux(int y, int x, int r_idx, bool_ slp, bool_ grp, int status)
{
	int i;
	monster_race *r_ptr = &r_info[r_idx];
	bool_ (*old_get_mon_num_hook)(int r_idx);


	/* Place one monster, or fail */
	if (!place_monster_one(y, x, r_idx, pick_ego_monster(r_idx), slp, status)) return (FALSE);


	/* Require the "group" flag */
	if (!grp) return (TRUE);


	/* Friends for certain monsters */
	if (r_ptr->flags1 & (RF1_FRIENDS))
	{
		/* Attempt to place a group */
		(void)place_monster_group(y, x, r_idx, slp, status);
	}


	/* Escorts for certain monsters */
	if (r_ptr->flags1 & (RF1_ESCORT))
	{
		old_get_mon_num_hook = get_mon_num_hook;

		/* Set the escort index */
		place_monster_idx = r_idx;

		/* Set the escort hook */
		get_mon_num_hook = place_monster_okay;

		/* Prepare allocation table */
		get_mon_num_prep();

		/* Try to place several "escorts" */
		for (i = 0; i < 50; i++)
		{
			int nx, ny, z, d = 3;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d);

			/* Require empty grids */
			if (!cave_empty_bold(ny, nx)) continue;

			set_mon_num2_hook(ny, nx);

			/* Prepare allocation table */
			get_mon_num_prep();

			/* Pick a random race */
			z = get_mon_num(r_ptr->level);

			/* Reset restriction */
			get_mon_num2_hook = NULL;

			/* Prepare allocation table */
			get_mon_num_prep();

			/* Handle failure */
			if (!z) break;

			/* Place a single escort */
			place_monster_one(ny, nx, z, pick_ego_monster(z), slp, status);

			/* Place a "group" of escorts if needed */
			if ((r_info[z].flags1 & (RF1_FRIENDS)) ||
			                (r_ptr->flags1 & (RF1_ESCORTS)))
			{
				/* Place a group of monsters */
				(void)place_monster_group(ny, nx, z, slp, status);
			}
		}

		/* Reset restriction */
		get_mon_num_hook = old_get_mon_num_hook;

		/* Prepare allocation table */
		get_mon_num_prep();
	}

	/* Success */
	return (TRUE);
}


/*
 * Hack -- attempt to place a monster at the given location
 *
 * Attempt to find a monster appropriate to the "monster_level"
 */
bool_ place_monster(int y, int x, bool_ slp, bool_ grp)
{
	int r_idx;

	/* Set monster restriction */
	set_mon_num2_hook(y, x);

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick a monster */
	r_idx = get_mon_num(monster_level);

	/* Reset restriction */
	get_mon_num2_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster */
	if (place_monster_aux(y, x, r_idx, slp, grp, MSTATUS_ENEMY)) return (TRUE);

	/* Oops */
	return (FALSE);
}


bool_ alloc_horde(int y, int x)
{
	int r_idx = 0;
	monster_race * r_ptr = NULL;
	monster_type * m_ptr;
	int attempts = 1000;

	set_mon_num2_hook(y, x);

	/* Prepare allocation table */
	get_mon_num_prep();

	while (--attempts)
	{
		/* Pick a monster */
		r_idx = get_mon_num(monster_level);

		/* Handle failure */
		if (!r_idx) return (FALSE);

		r_ptr = &r_info[r_idx];

		if (!(r_ptr->flags1 & (RF1_UNIQUE))
		                && !(r_ptr->flags1 & (RF1_ESCORTS)))
			break;
	}

	get_mon_num2_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	if (attempts < 1) return FALSE;

	attempts = 1000;

	while (--attempts)
	{
		/* Attempt to place the monster */
		if (place_monster_aux(y, x, r_idx, FALSE, FALSE, MSTATUS_ENEMY)) break;
	}

	if (attempts < 1) return FALSE;


	m_ptr = &m_list[hack_m_idx_ii];

	summon_kin_type = r_ptr->d_char;

	for (attempts = randint(10) + 5; attempts; attempts--)
	{
		(void) summon_specific(m_ptr->fy, m_ptr->fx, dun_level, SUMMON_KIN);
	}

	return TRUE;
}

/*
 * Attempt to allocate a random monster in the dungeon.
 *
 * Place the monster at least "dis" distance from the player.
 *
 * Use "slp" to choose the initial "sleep" status
 *
 * Use "monster_level" for the monster level
 */
bool_ alloc_monster(int dis, bool_ slp)
{
	int	y, x;
	int attempts_left = 10000;

	/* Find a legal, distant, unoccupied, space */
	while (attempts_left--)
	{
		/* Pick a location */
		y = rand_int(cur_hgt);
		x = rand_int(cur_wid);

		/* Require empty floor grid (was "naked") */
		if (!cave_empty_bold(y, x)) continue;

		/* Accept far away grids */
		if (distance(y, x, p_ptr->py, p_ptr->px) > dis) break;
	}

	if (!attempts_left)
	{
		if (cheat_xtra || cheat_hear)
		{
			msg_print("Warning! Could not allocate a new monster. Small level?");
		}

		return (FALSE);
	}


	if (randint(5000) <= dun_level)
	{
		if (alloc_horde(y, x))
		{
			if ((cheat_hear) || (p_ptr->precognition)) msg_print("Monster horde.");
			return (TRUE);
		}
	}
	else
	{

		/* Attempt to place the monster, allow groups */
		if (place_monster(y, x, slp, TRUE)) return (TRUE);

	}

	/* Nope */
	return (FALSE);
}




/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;


/*
 * Hack -- help decide if a monster race is "okay" to summon
 */
bool_ summon_specific_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	bool_ okay = FALSE;

	/* Hack - Only summon dungeon monsters */
	if (!monster_dungeon(r_idx)) return (FALSE);

	/* Hack -- no specific type specified */
	if (!summon_specific_type) return (TRUE);

	/* Check our requirements */
	switch (summon_specific_type)
	{
	case SUMMON_ANT:
		{
			okay = ((r_ptr->d_char == 'a') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_SPIDER:
		{
			okay = ((r_ptr->d_char == 'S') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HOUND:
		{
			okay = (((r_ptr->d_char == 'C') || (r_ptr->d_char == 'Z')) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HYDRA:
		{
			okay = ((r_ptr->d_char == 'M') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_ANGEL:
		{
			okay = ((r_ptr->d_char == 'A') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_DEMON:
		{
			okay = ((r_ptr->flags3 & (RF3_DEMON)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_UNDEAD:
		{
			okay = ((r_ptr->flags3 & (RF3_UNDEAD)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_DRAGON:
		{
			okay = ((r_ptr->flags3 & (RF3_DRAGON)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HI_UNDEAD:
		{
			okay = ((r_ptr->d_char == 'L') ||
			        (r_ptr->d_char == 'V') ||
			        (r_ptr->d_char == 'W'));
			break;
		}

	case SUMMON_HI_DRAGON:
		{
			okay = (r_ptr->d_char == 'D');
			break;
		}

	case SUMMON_WRAITH:
		{
			okay = (r_ptr->d_char == 'W');
			break;
		}

	case SUMMON_GHOST:
		{
			okay = (r_ptr->d_char == 'G');
			break;
		}

	case SUMMON_UNIQUE:
		{
			okay = (r_ptr->flags1 & (RF1_UNIQUE)) ? TRUE : FALSE;
			break;
		}

	case SUMMON_BIZARRE1:
		{
			okay = ((r_ptr->d_char == 'm') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}
	case SUMMON_BIZARRE2:
		{
			okay = ((r_ptr->d_char == 'b') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}
	case SUMMON_BIZARRE3:
		{
			okay = ((r_ptr->d_char == 'Q') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_BIZARRE4:
		{
			okay = ((r_ptr->d_char == 'v') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_BIZARRE5:
		{
			okay = ((r_ptr->d_char == '$') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_BIZARRE6:
		{
			okay = (((r_ptr->d_char == '!') ||
			         (r_ptr->d_char == '?') ||
			         (r_ptr->d_char == '=') ||
			         (r_ptr->d_char == '$') ||
			         (r_ptr->d_char == '|')) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HI_DEMON:
		{
			okay = ((r_ptr->flags3 & (RF3_DEMON)) &&
			        (r_ptr->d_char == 'U') &&
			        !(r_ptr->flags1 & RF1_UNIQUE));
			break;
		}


	case SUMMON_KIN:
		{
			okay = ((r_ptr->d_char == summon_kin_type) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_DAWN:
		{
			okay = ((strstr((r_name + r_ptr->name), "the Dawn")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_ANIMAL:
		{
			okay = ((r_ptr->flags3 & (RF3_ANIMAL)) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_ANIMAL_RANGER:
		{
			okay = ((r_ptr->flags3 & (RF3_ANIMAL)) &&
			        (strchr("abcflqrwBCIJKMRS", r_ptr->d_char)) &&
			        !(r_ptr->flags3 & (RF3_DRAGON)) &&
			        !(r_ptr->flags3 & (RF3_EVIL)) &&
			        !(r_ptr->flags3 & (RF3_UNDEAD)) &&
			        !(r_ptr->flags3 & (RF3_DEMON)) &&
			        !(r_ptr->flags4 || r_ptr->flags5 || r_ptr->flags6) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HI_UNDEAD_NO_UNIQUES:
		{
			okay = (((r_ptr->d_char == 'L') ||
			         (r_ptr->d_char == 'V') ||
			         (r_ptr->d_char == 'W')) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_HI_DRAGON_NO_UNIQUES:
		{
			okay = ((r_ptr->d_char == 'D') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_NO_UNIQUES:
		{
			okay = (!(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_PHANTOM:
		{
			okay = ((strstr((r_name + r_ptr->name), "Phantom")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_ELEMENTAL:
		{
			okay = ((strstr((r_name + r_ptr->name), "lemental")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_THUNDERLORD:
		{
			okay = (r_ptr->flags3 & RF3_THUNDERLORD) ? TRUE : FALSE;
			break;
		}

	case SUMMON_BLUE_HORROR:
		{
			okay = ((strstr((r_name + r_ptr->name), "lue horror")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_BUG:
		{
			okay = ((strstr((r_name + r_ptr->name), "Software bug")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_RNG:
		{
			okay = ((strstr((r_name + r_ptr->name), "Random Number Generator")) &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}
	case SUMMON_MINE:
		{
			okay = (r_ptr->flags1 & RF1_NEVER_MOVE) ? TRUE : FALSE;
			break;
		}

	case SUMMON_HUMAN:
		{
			okay = ((r_ptr->d_char == 'p') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_SHADOWS:
		{
			okay = ((r_ptr->d_char == 'G') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	case SUMMON_QUYLTHULG:
		{
			okay = ((r_ptr->d_char == 'Q') &&
			        !(r_ptr->flags1 & (RF1_UNIQUE)));
			break;
		}

	}

	/* Result */
	return (okay);
}


/*
 * Place a monster (of the specified "type") near the given
 * location.  Return TRUE if a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: SUMMON_UNIQUE and SUMMON_WRAITH (XXX) will summon Unique's
 * Note: SUMMON_HI_UNDEAD and SUMMON_HI_DRAGON may summon Unique's
 * Note: None of the other summon codes will ever summon Unique's.
 *
 * This function has been changed.  We now take the "monster level"
 * of the summoning monster as a parameter, and use that, along with
 * the current dungeon level, to help determine the level of the
 * desired monster.  Note that this is an upper bound, and also
 * tends to "prefer" monsters of that level.  Currently, we use
 * the average of the dungeon and monster levels, and then add
 * five to allow slight increases in monster power.
 *
 * Note that we use the new "monster allocation table" creation code
 * to restrict the "get_mon_num()" function to the set of "legal"
 * monsters, making this function much faster and more reliable.
 *
 * Note that this function may not succeed, though this is very rare.
 */
int summon_specific_level = 0;
bool_ summon_specific(int y1, int x1, int lev, int type)
{
	int i, x, y, r_idx;
	bool_ Group_ok = TRUE;
	bool_ (*old_get_mon_num_hook)(int r_idx);

	/* Look for a location */
	for (i = 0; i < 20; ++i)
	{
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(&y, &x, y1, x1, d);

		/* Require "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Hack -- no summon on glyph of warding */
		if (cave[y][x].feat == FEAT_GLYPH) continue;
		if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

		/* Nor on the between */
		if (cave[y][x].feat == FEAT_BETWEEN) return (FALSE);

		/* ... nor on the Pattern */
		if ((cave[y][x].feat >= FEAT_PATTERN_START) &&
		                (cave[y][x].feat <= FEAT_PATTERN_XTRA2))
			continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 20) return (FALSE);

	/* Save the "summon" type */
	summon_specific_type = type;

	/* Backup the old hook */
	old_get_mon_num_hook = get_mon_num_hook;

	/* Require "okay" monsters */
	get_mon_num_hook = summon_specific_okay;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Pick a monster, using the level calculation */
	summon_hack = TRUE;
	r_idx = get_mon_num((dun_level + lev) / 2 + 5);
	summon_hack = FALSE;

	/* Reset restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();


	/* Handle failure */
	if (!r_idx) return (FALSE);


	if ((type == SUMMON_DAWN) || (type == SUMMON_BLUE_HORROR))
	{
		Group_ok = FALSE;
	}

	/* Attempt to place the monster (awake, allow groups) */
	if (!place_monster_aux(y, x, r_idx, FALSE, Group_ok, MSTATUS_ENEMY)) return (FALSE);
	if (summon_specific_level)
	{
		monster_set_level(place_monster_result, summon_specific_level);
		summon_specific_level = 0;
	}

	/* Success */
	return (TRUE);
}



bool_ summon_specific_friendly(int y1, int x1, int lev, int type, bool_ Group_ok)
{
	int i, x, y, r_idx;
	bool_ (*old_get_mon_num_hook)(int r_idx);

	/* Look for a location */
	for (i = 0; i < 20; ++i)
	{
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(&y, &x, y1, x1, d);

		/* Require "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Hack -- no summon on glyph of warding */
		if (cave[y][x].feat == FEAT_GLYPH) continue;
		if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

		/* Nor on the between */
		if (cave[y][x].feat == FEAT_BETWEEN) return (FALSE);

		/* ... nor on the Pattern */
		if ((cave[y][x].feat >= FEAT_PATTERN_START) &&
		                (cave[y][x].feat <= FEAT_PATTERN_XTRA2))
			continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 20) return (FALSE);

	old_get_mon_num_hook = get_mon_num_hook;

	/* Save the "summon" type */
	summon_specific_type = type;

	/* Require "okay" monsters */
	get_mon_num_hook = summon_specific_okay;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick a monster, using the level calculation */
	r_idx = get_mon_num((dun_level + lev) / 2 + 5);

	/* Reset restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Handle failure */
	if (!r_idx) return (FALSE);

	/* Attempt to place the monster (awake, allow groups) */
	if (!place_monster_aux(y, x, r_idx, FALSE, Group_ok, MSTATUS_PET)) return (FALSE);
	if (summon_specific_level)
	{
		monster_set_level(place_monster_result, summon_specific_level);
		summon_specific_level = 0;
	}

	/* Success */
	return (TRUE);
}


/*
 * Swap the players/monsters (if any) at two locations XXX XXX XXX
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	monster_type *m_ptr;
	cave_type *c_ptr1, *c_ptr2;

	c_ptr1 = &cave[y1][x1];
	c_ptr2 = &cave[y2][x2];

	/* Monsters */
	m1 = c_ptr1->m_idx;
	m2 = c_ptr2->m_idx;


	/* Update grids */
	c_ptr1->m_idx = m2;
	c_ptr2->m_idx = m1;


	/* Monster 1 */
	if (m1 > 0)
	{
		m_ptr = &m_list[m1];

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* Update monster */
		update_mon(m1, TRUE);
	}

	/* Player 1 */
	else if (m1 < 0)
	{
		/* Move player */
		p_ptr->py = y2;
		p_ptr->px = x2;

		/* Check for new panel (redraw map) */
		verify_panel();

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		/* It's probably not a good idea to recalculate the
		 * overhead view each turn.

		 p_ptr->window |= (PW_OVERHEAD);
		*/
	}

	/* Monster 2 */
	if (m2 > 0)
	{
		m_ptr = &m_list[m2];

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* Update monster */
		update_mon(m2, TRUE);
	}

	/* Player 2 */
	else if (m2 > 0)
	{
		/* Move player */
		p_ptr->py = y1;
		p_ptr->px = x1;

		/* Check for new panel (redraw map) */
		verify_panel();

		/* Update stuff */
		p_ptr->update |= (PU_VIEW | PU_FLOW | PU_MON_LITE);

		/* Update the monsters */
		p_ptr->update |= (PU_DISTANCE);

		/* Window stuff */
		/* It's probably not a good idea to recalculate the
		 * overhead view each turn.

		 p_ptr->window |= (PW_OVERHEAD);
		*/
	}


	/* Redraw */
	lite_spot(y1, x1);
	lite_spot(y2, x2);
}


/*
 * Hack -- help decide if a monster race is "okay" to summon
 */
static bool_ mutate_monster_okay(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	bool_ okay = FALSE;

	/* Hack - Only summon dungeon monsters */
	if (!monster_dungeon(r_idx)) return (FALSE);

	okay = ((r_ptr->d_char == summon_kin_type) && !(r_ptr->flags1 & (RF1_UNIQUE))
	        && (r_ptr->level >= dun_level));

	return okay;
}


/*
 * Let the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 */
bool_ multiply_monster(int m_idx, bool_ charm, bool_ clone)
{
	monster_type	*m_ptr = &m_list[m_idx];
	monster_race *r_ptr = race_inf(m_ptr);

	int i, y, x, new_race;

	bool_ result = FALSE;

	if (no_breeds)
	{
		msg_print("It tries to breed but it fails!");
		return FALSE;
	}

	/* Try up to 18 times */
	for (i = 0; i < 18; i++)
	{
		int d = 1;


		/* Pick a location */
		scatter(&y, &x, m_ptr->fy, m_ptr->fx, d);

		/* Require an "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		new_race = m_ptr->r_idx;

		/* It can mutate into a nastier monster */
		if ((rand_int(100) < 3) && (!clone))
		{
			bool_ (*old_get_mon_num_hook)(int r_idx);

			/* Backup the old hook */
			old_get_mon_num_hook = get_mon_num_hook;

			/* Require "okay" monsters */
			get_mon_num_hook = mutate_monster_okay;

			/* Prepare allocation table */
			get_mon_num_prep();

			summon_kin_type = r_ptr->d_char;

			/* Pick a monster, using the level calculation */
			new_race = get_mon_num(dun_level + 5);

			/* Reset restriction */
			get_mon_num_hook = old_get_mon_num_hook;

			/* Prepare allocation table */
			get_mon_num_prep();
		}

		/* Create a new monster (awake, no groups) */
		result = place_monster_aux(y, x, new_race, FALSE, FALSE, (charm) ? MSTATUS_PET : MSTATUS_ENEMY);

		/* Done */
		break;
	}

	if (clone && result) m_list[hack_m_idx_ii].smart |= SM_CLONED;

	/* Result */
	return (result);
}





/*
 * Dump a message describing a monster's reaction to damage
 *
 * Technically should attempt to treat "Beholder"'s as jelly's
 */
bool_ hack_message_pain_may_silent = FALSE;
void message_pain_hook(cptr fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	if (hack_message_pain_may_silent)
		monster_msg(buf);
	else
		msg_print(buf);
}

void message_pain(int m_idx, int dam)
{
	long oldhp, newhp, tmp;
	int percentage;
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = race_inf(m_ptr);
	char m_name[80];

	/* Get the monster name */
	monster_desc(m_name, m_ptr, 0);

	/* Notice non-damage */
	if (dam == 0)
	{
		message_pain_hook("%^s is unharmed.", m_name);
		return;
	}

	/* Note -- subtle fix -CFT */
	newhp = (long)(m_ptr->hp);
	oldhp = newhp + (long)(dam);
	tmp = (newhp * 100L) / oldhp;
	percentage = (int)(tmp);


	/* Jelly's, Mold's, Vortex's, Quthl's */
	if (strchr("jmvQ", r_ptr->d_char))
	{
		if (percentage > 95)
			message_pain_hook("%^s barely notices.", m_name);
		else if (percentage > 75)
			message_pain_hook("%^s flinches.", m_name);
		else if (percentage > 50)
			message_pain_hook("%^s squelches.", m_name);
		else if (percentage > 35)
			message_pain_hook("%^s quivers in pain.", m_name);
		else if (percentage > 20)
			message_pain_hook("%^s writhes about.", m_name);
		else if (percentage > 10)
			message_pain_hook("%^s writhes in agony.", m_name);
		else
			message_pain_hook("%^s jerks limply.", m_name);
	}

	/* Dogs and Hounds */
	else if (strchr("CZ", r_ptr->d_char))
	{
		if (percentage > 95)
			message_pain_hook("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			message_pain_hook("%^s snarls with pain.", m_name);
		else if (percentage > 50)
			message_pain_hook("%^s yelps in pain.", m_name);
		else if (percentage > 35)
			message_pain_hook("%^s howls in pain.", m_name);
		else if (percentage > 20)
			message_pain_hook("%^s howls in agony.", m_name);
		else if (percentage > 10)
			message_pain_hook("%^s writhes in agony.", m_name);
		else
			message_pain_hook("%^s yelps feebly.", m_name);
	}

	/* One type of monsters (ignore,squeal,shriek) */
	else if (strchr("FIKMRSXabclqrst", r_ptr->d_char))
	{
		if (percentage > 95)
			message_pain_hook("%^s ignores the attack.", m_name);
		else if (percentage > 75)
			message_pain_hook("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			message_pain_hook("%^s squeals in pain.", m_name);
		else if (percentage > 35)
			message_pain_hook("%^s shrieks in pain.", m_name);
		else if (percentage > 20)
			message_pain_hook("%^s shrieks in agony.", m_name);
		else if (percentage > 10)
			message_pain_hook("%^s writhes in agony.", m_name);
		else
			message_pain_hook("%^s cries out feebly.", m_name);
	}

	/* Another type of monsters (shrug,cry,scream) */
	else
	{
		if (percentage > 95)
			message_pain_hook("%^s shrugs off the attack.", m_name);
		else if (percentage > 75)
			message_pain_hook("%^s grunts with pain.", m_name);
		else if (percentage > 50)
			message_pain_hook("%^s cries out in pain.", m_name);
		else if (percentage > 35)
			message_pain_hook("%^s screams in pain.", m_name);
		else if (percentage > 20)
			message_pain_hook("%^s screams in agony.", m_name);
		else if (percentage > 10)
			message_pain_hook("%^s writhes in agony.", m_name);
		else
			message_pain_hook("%^s cries out feebly.", m_name);
	}
}



/*
 * Learn about an "observed" resistance.
 */
void update_smart_learn(int m_idx, int what)
{
	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = race_inf(m_ptr);


	/* Not allowed to learn */
	if (!smart_learn) return;

	/* Too stupid to learn anything */
	if (r_ptr->flags2 & (RF2_STUPID)) return;

	/* Not intelligent, only learn sometimes */
	if (!(r_ptr->flags2 & (RF2_SMART)) && (rand_int(100) < 50)) return;


	/* XXX XXX XXX */

	/* Analyze the knowledge */
	switch (what)
	{
	case DRS_ACID:
		if (p_ptr->resist_acid) m_ptr->smart |= (SM_RES_ACID);
		if (p_ptr->oppose_acid) m_ptr->smart |= (SM_OPP_ACID);
		if (p_ptr->immune_acid) m_ptr->smart |= (SM_IMM_ACID);
		break;

	case DRS_ELEC:
		if (p_ptr->resist_elec) m_ptr->smart |= (SM_RES_ELEC);
		if (p_ptr->oppose_elec) m_ptr->smart |= (SM_OPP_ELEC);
		if (p_ptr->immune_elec) m_ptr->smart |= (SM_IMM_ELEC);
		break;

	case DRS_FIRE:
		if (p_ptr->resist_fire) m_ptr->smart |= (SM_RES_FIRE);
		if (p_ptr->oppose_fire) m_ptr->smart |= (SM_OPP_FIRE);
		if (p_ptr->immune_fire) m_ptr->smart |= (SM_IMM_FIRE);
		break;

	case DRS_COLD:
		if (p_ptr->resist_cold) m_ptr->smart |= (SM_RES_COLD);
		if (p_ptr->oppose_cold) m_ptr->smart |= (SM_OPP_COLD);
		if (p_ptr->immune_cold) m_ptr->smart |= (SM_IMM_COLD);
		break;

	case DRS_POIS:
		if (p_ptr->resist_pois) m_ptr->smart |= (SM_RES_POIS);
		if (p_ptr->oppose_pois) m_ptr->smart |= (SM_OPP_POIS);
		break;


	case DRS_NETH:
		if (p_ptr->resist_neth) m_ptr->smart |= (SM_RES_NETH);
		break;

	case DRS_LITE:
		if (p_ptr->resist_lite) m_ptr->smart |= (SM_RES_LITE);
		break;

	case DRS_DARK:
		if (p_ptr->resist_dark) m_ptr->smart |= (SM_RES_DARK);
		break;

	case DRS_FEAR:
		if (p_ptr->resist_fear) m_ptr->smart |= (SM_RES_FEAR);
		break;

	case DRS_CONF:
		if (p_ptr->resist_conf) m_ptr->smart |= (SM_RES_CONF);
		break;

	case DRS_CHAOS:
		if (p_ptr->resist_chaos) m_ptr->smart |= (SM_RES_CHAOS);
		break;

	case DRS_DISEN:
		if (p_ptr->resist_disen) m_ptr->smart |= (SM_RES_DISEN);
		break;

	case DRS_BLIND:
		if (p_ptr->resist_blind) m_ptr->smart |= (SM_RES_BLIND);
		break;

	case DRS_NEXUS:
		if (p_ptr->resist_nexus) m_ptr->smart |= (SM_RES_NEXUS);
		break;

	case DRS_SOUND:
		if (p_ptr->resist_sound) m_ptr->smart |= (SM_RES_SOUND);
		break;

	case DRS_SHARD:
		if (p_ptr->resist_shard) m_ptr->smart |= (SM_RES_SHARD);
		break;


	case DRS_FREE:
		if (p_ptr->free_act) m_ptr->smart |= (SM_IMM_FREE);
		break;

	case DRS_MANA:
		if (!p_ptr->msp) m_ptr->smart |= (SM_IMM_MANA);
		break;

	case DRS_REFLECT:
		if (p_ptr->reflect) m_ptr-> smart |= (SM_IMM_REFLECT);
		break;
	}
}


/*
 * Place the player in the dungeon XXX XXX
 */
s16b player_place(int y, int x)
{
	/* Paranoia XXX XXX */
	if (cave[y][x].m_idx != 0) return (0);

	/* Save player location */
	p_ptr->py = y;
	p_ptr->px = x;

	/* Success */
	return ( -1);
}

/*
 * Drop all items carried by a monster
 */
void monster_drop_carried_objects(monster_type *m_ptr)
{
	s16b this_o_idx, next_o_idx = 0;
	object_type forge;
	object_type *o_ptr;
	object_type *q_ptr;


	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		q_ptr = &forge;

		/* Copy the object */
		object_copy(q_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Drop it */
		drop_near(q_ptr, -1, m_ptr->fy, m_ptr->fx);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;
}
