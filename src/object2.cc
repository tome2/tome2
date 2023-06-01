/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "object2.hpp"

#include "alloc_entry.hpp"
#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "spell_type.hpp"
#include "device_allocation.hpp"
#include "dungeon_info_type.hpp"
#include "ego_flag.hpp"
#include "ego_item_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "mimic.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object_flag.hpp"
#include "object_flag_meta.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "randart.hpp"
#include "randart_part_type.hpp"
#include "skills.hpp"
#include "spells2.hpp"
#include "spells3.hpp"
#include "spells5.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "xtra1.hpp"
#include "z-rand.hpp"

#include <algorithm>
#include <cassert>
#include <type_traits>
#include <vector>

/*
 * Calculate the player's total inventory weight.
 */
s32b calc_total_weight()
{
	s32b total;
	for (int i = total = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		if (o_ptr->k_ptr)
		{
			total += o_ptr->weight * o_ptr->number;
		}
	}
	return total;
}

/*
 * Excise a dungeon object from any stacks
 */
void excise_object_idx(int o_idx)
{
	/* Function to remove from list */
	auto remove_it = [o_idx](std::vector<s16b> *v) -> void {
		v->erase(
			std::remove(
				v->begin(),
				v->end(),
				o_idx),
			v->end());
	};

	/* Object */
	object_type *o_ptr = &o_list[o_idx];

	/* Monster */
	if (o_ptr->held_m_idx)
	{
		/* Monster */
		monster_type *m_ptr = &m_list[o_ptr->held_m_idx];

		/* Remove object from list of held objects, if present. */
		remove_it(&m_ptr->hold_o_idxs);
	}
	/* Dungeon */
	else
	{
		/* Grid */
		cave_type *c_ptr = &cave[o_ptr->iy][o_ptr->ix];

		/* Remove object from list of objects in the grid, if present. */
		remove_it(&c_ptr->o_idxs);
	}
}


/*
 * Delete a dungeon object
 *
 * Handle "stacks" of objects correctly.
 */
void delete_object_idx(int o_idx)
{
	object_type *j_ptr;

	/* Excise */
	excise_object_idx(o_idx);

	/* Object */
	j_ptr = &o_list[o_idx];

	/* Dungeon floor */
	if (!(j_ptr->held_m_idx))
	{
		int y, x;

		/* Location */
		y = j_ptr->iy;
		x = j_ptr->ix;

		/* Visual update */
		lite_spot(y, x);
	}

	/* Wipe the object */
	object_wipe(j_ptr);

	/* Count objects */
	o_cnt--;
}


/*
 * Deletes all objects at given location
 */
void delete_object(int y, int x)
{
	/* Refuse "illegal" locations */
	if (!in_bounds(y, x)) return;

	/* Grid */
	cave_type *c_ptr = &cave[y][x];

	/* Scan all objects in the grid */
	for (auto const this_o_idx: c_ptr->o_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		/* Wipe the object */
		object_wipe(o_ptr);

		/* Count objects */
		o_cnt--;
	}

	/* Objects are gone */
	c_ptr->o_idxs.clear();

	/* Visual update */
	lite_spot(y, x);
}


/*
 * Move an object from index i1 to index i2 in the object list
 */
static void compact_objects_aux(int i1, int i2)
{
	/* Do nothing */
	if (i1 == i2) return;

	/* Acquire object */
	object_type *o_ptr = &o_list[i1];

	/* Monster */
	if (o_ptr->held_m_idx)
	{
		/* Acquire monster */
		monster_type *m_ptr = &m_list[o_ptr->held_m_idx];

		/* Repair monster */
		for (auto &hold_o_idx: m_ptr->hold_o_idxs)
		{
			if (hold_o_idx == i1)
			{
				hold_o_idx = i2;
			}
		}
	}

	/* Dungeon */
	else
	{
		/* Acquire grid */
		cave_type *c_ptr = &cave[o_ptr->iy][o_ptr->ix];

		/* Repair grid */
		for (auto &o_idx: c_ptr->o_idxs)
		{
			if (o_idx == i1)
			{
				o_idx = i2;
			}
		}
	}

	/* Structure copy */
	o_list[i2] = o_list[i1];

	/* Wipe the hole */
	object_wipe(o_ptr);
}


/*
 * Compact and Reorder the object list
 *
 * This function can be very dangerous, use with caution!
 *
 * When actually "compacting" objects, we base the saving throw on a
 * combination of object level, distance from player, and current
 * "desperation".
 *
 * After "compacting" (if needed), we "reorder" the objects into a more
 * compact order, and we reset the allocation info, and the "live" array.
 */
void compact_objects(int size)
{
	int i, y, x, num;
	int cur_lev, cur_dis, chance;

	/* Compact */
	if (size)
	{
		/* Message */
		msg_print("Compacting objects...");

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD);
	}


	/* Compact at least 'size' objects */
	for (num = 0, cur_lev = 1; num < size; cur_lev++)
	{
		/* Get closer each iteration (start at distance 12). Around level 100 distance-protect nothing. */
		cur_dis = 12 * (101 - cur_lev) / 100;

		/* Examine the objects */
		for (i = 1; i < o_max; i++)
		{
			object_type *o_ptr = &o_list[i];
			auto const &k_ptr = o_ptr->k_ptr;

			/* Skip dead objects */
			if (!o_ptr->k_ptr) continue;

			/* High level objects are "immune" as long as we're not desperate enough */
			if (k_ptr->level > cur_lev) continue;

			/* Monster owned objects */
			if (o_ptr->held_m_idx)
			{
				monster_type *m_ptr;

				/* Acquire monster */
				m_ptr = &m_list[o_ptr->held_m_idx];

				/* Monsters start with protecting objects well */
				chance = 100;

				/* Get the location */
				y = m_ptr->fy;
				x = m_ptr->fx;
			}
			/* Dungeon floor objects */
			else
			{
				/* Floor objects start with lower protection */
				chance = 90;

				/* Get the location */
				y = o_ptr->iy;
				x = o_ptr->ix;
			}

			/* Near enough objects are "immune", even if low level			*/
			/* (like, importantly, food rations after hitting a trap of drop items) */
			if ((cur_dis > 0) && (distance(p_ptr->py, p_ptr->px, y, x) < cur_dis)) continue;

			/* object protection goes down as we get vicious   */
			/* around level 200 only artifacts have protection */
			chance = chance - cur_lev / 2;

			/* Artifacts */
			if (artifact_p(o_ptr))
			{
				/* Artifacts are "immune if the level is lower     */
				/* than 300 + artifact level                       */
				if ( cur_lev < 300 + k_ptr->level )
					continue;

				/* That's 400 +  level for fixed artifacts */
				if ( (k_ptr->flags & TR_NORM_ART) && cur_lev < 400 + k_ptr->level )
					continue;

				/* Never protect if level is high enough; so we don't wipe a better artifact */
				chance = -1;

				/* rewind the level so we never wipe many    */
				/* artifacts of same level if one will do!!! */
				cur_lev--;
			}

			/* Maybe some code to spare the God relic here. But I'd rather raise its level to 150 */

			/* Apply the saving throw */
			if (rand_int(100) < chance) continue;

			/* Delete the object */
			delete_object_idx(i);

			/* Count it */
			num++;
		}
	}


	/* Excise dead objects (backwards!) */
	for (i = o_max - 1; i >= 1; i--)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip real objects */
		if (o_ptr->k_ptr)
		{
			continue;
		}

		/* Move last object into open hole */
		compact_objects_aux(o_max - 1, i);

		/* Compress "o_max" */
		o_max--;
	}
}



/*
 * Rescue artifacts from destruction if the "preserve" option is
 * turned on.
 */
void rescue_artifact(object_type *o_ptr)
{
	auto &a_info = game->edit_data.a_info;

	if (artifact_p(o_ptr) && !object_known_p(o_ptr))
	{
		/* Mega-Hack -- Preserve the artifact */
		if (o_ptr->tval == TV_RANDART)
		{
			game->random_artifacts[o_ptr->sval].generated = false;
		}
		else if (o_ptr->k_ptr->flags & TR_NORM_ART)
		{
			o_ptr->k_ptr->artifact = false;
		}
		else
		{
			a_info[o_ptr->name1].cur_num = 0;
		}
	}
}

/*
 * Delete all the items when player leaves the level
 *
 * Note -- we do NOT visually reflect these (irrelevant) changes
 *
 * Hack -- we clear the "c_ptr->o_idx" field for every grid,
 * and the "m_ptr->next_o_idx" field for every monster, since
 * we know we are clearing every object.  Technically, we only
 * clear those fields for grids/monsters containing objects,
 * and we clear it once for every such object.
 */
void wipe_o_list()
{
	/* Delete the existing objects */
	for (int i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		/* Skip dead objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Mega-Hack -- preserve artifacts */
		if (!character_dungeon || options->preserve)
		{
			rescue_artifact(o_ptr);
		}

		/* Monster */
		if (o_ptr->held_m_idx)
		{
			/* Monster */
			monster_type *m_ptr = &m_list[o_ptr->held_m_idx];

			/* Hack -- see above */
			m_ptr->hold_o_idxs.clear();
		}

		/* Dungeon */
		else
		{
			cave_type *c_ptr;

			/* Access location */
			int y = o_ptr->iy;
			int x = o_ptr->ix;

			/* Access grid */
			c_ptr = &cave[y][x];

			/* Hack -- see above */
			c_ptr->o_idxs.clear();
		}

		/* Wipe the object */
		object_wipe(o_ptr);
	}

	/* Reset "o_max" */
	o_max = 1;

	/* Reset "o_cnt" */
	o_cnt = 0;
}


/*
 * Acquires and returns the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop()
{
	int i;


	/* Initial allocation */
	if (o_max < max_o_idx)
	{
		/* Get next space */
		i = o_max;

		/* Expand object array */
		o_max++;

		/* Count objects */
		o_cnt++;

		/* Use this object */
		return (i);
	}


	/* Recycle dead objects */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr;

		/* Acquire object */
		o_ptr = &o_list[i];

		/* Skip live objects */
		if (o_ptr->k_ptr)
		{
			continue;
		}

		/* Count objects */
		o_cnt++;

		/* Use this object */
		return (i);
	}


	/* Warn the player (except during dungeon creation) */
	if (character_dungeon) msg_print("Too many objects!");

	/* Oops */
	return (0);
}



/*
 * Apply a "object restriction function" to the "object allocation table"
 */
errr get_obj_num_prep()
{
	auto &alloc = game->alloc;
	auto const &k_info = game->edit_data.k_info;

	/* Scan the allocation table */
	for (auto &&entry: alloc.kind_table)
	{
		/* Accept objects which pass the restriction, if any */
		if (!get_object_hook || (*get_object_hook)(k_info.at(entry.index).get()))
		{
			/* Accept this object */
			entry.prob2 = entry.prob1;
		}

		/* Do not use this object */
		else
		{
			/* Decline this object */
			entry.prob2 = 0;
		}
	}

	/* Success */
	return (0);
}



/*
 * Choose an object kind that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "object allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" object, in
 * a relatively efficient manner.
 *
 * It is (slightly) more likely to acquire an object of the given level
 * than one of a lower level.  This is done by choosing several objects
 * appropriate to the given level and keeping the "hardest" one.
 *
 * Note that if no objects are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
s16b get_obj_num(int level)
{
	auto &alloc = game->alloc;

	std::size_t i, j;
	int p;
	long value, total;


	/* Boost level */
	if (level > 0)
	{
		/* Occasional "boost" */
		if (rand_int(GREAT_OBJ) == 0)
		{
			/* What a bizarre calculation */
			level = 1 + (level * MAX_DEPTH / randint(MAX_DEPTH));
		}
	}


	/* Reset total */
	total = 0L;

	/* Process probabilities */
	for (i = 0; i < alloc.kind_table.size(); i++)
	{
		auto &entry = alloc.kind_table[i];

		/* Objects are sorted by depth */
		if (entry.level > level) break;

		/* Default */
		entry.prob3 = 0;

		/* Accept */
		entry.prob3 = entry.prob2;

		/* Total */
		total += entry.prob3;
	}

	/* No legal objects */
	if (total <= 0) return (0);

	/* Pick an object */
	value = rand_int(total);

	/* Find the object */
	for (i = 0; i < alloc.kind_table.size(); i++)
	{
		auto &entry = alloc.kind_table[i];

		/* Found the entry */
		if (value < entry.prob3) break;

		/* Decrement */
		value = value - entry.prob3;
	}

	/* Power boost */
	p = rand_int(100);

	/* Shorthand */
	auto &table = alloc.kind_table;

	/* Try for a "better" object once (50%) or twice (10%) */
	if (p < 60)
	{
		/* Save old */
		j = i;

		/* Pick a object */
		value = rand_int(total);

		/* Find the monster */
		for (i = 0; i < table.size(); i++)
		{

			/* Found the entry */
			if (value < table[i].prob3) break;

			/* Decrement */
			value = value - table[i].prob3;
		}

		/* Keep the "best" one */
		if (table[i].level < table[j].level) i = j;
	}

	/* Try for a "better" object twice (10%) */
	if (p < 10)
	{
		/* Save old */
		j = i;

		/* Pick a object */
		value = rand_int(total);

		/* Find the object */
		for (i = 0; i < table.size(); i++)
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
 * Known is true when the "attributes" of an object are "known".
 * These include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more.  In fact, the player is always "aware" of any
 * item of which he has full "knowledge".
 *
 * But having full knowledge of, say, one "wand of wonder", does not, by
 * itself, give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as fully "know" it.
 *
 * This routine also removes any inscriptions generated by "feelings".
 */
void object_known(object_type *o_ptr)
{
	auto previously_known = object_known_p(o_ptr);

	/* Mark the item as identified */
	o_ptr->identified = true;

	/* If the status changed, then we invoke the hook */
	if (!previously_known)
	{
		identify_hooks(o_ptr);
	}
}



/*
 * Determine if a given inventory item is "known"
 * Test One -- Check for special "known" tag
 * Test Two -- Check for "Easy Know" + "Aware"
 */
bool object_known_p(object_type const *o_ptr)
{
	return (o_ptr->identified ||
		(o_ptr->k_ptr && o_ptr->k_ptr->easy_know && o_ptr->k_ptr->aware));
}



/*
 * The player is now aware of the effects of the given object.
 */
void object_aware(object_type *o_ptr)
{
	/* Fully aware of the effects */
	o_ptr->k_ptr->aware = true;
}

/**
 * Is the player aware of the effects of the given object?
 */
bool object_aware_p(object_type const *o_ptr)
{
	return o_ptr->k_ptr && o_ptr->k_ptr->aware;
}


/* Return the value of the flags the object has... */
s32b flag_cost(object_type const *o_ptr, int plusses)
{
	auto const flags = object_flags(o_ptr);

	if (flags & TR_TEMPORARY)
	{
		return 0;
	}

	if (flags & TR_CURSE_NO_DROP)
	{
		return 0;
	}

	s32b total = 0;

	if (flags & TR_STR) total += (1000 * plusses);
	if (flags & TR_INT) total += (1000 * plusses);
	if (flags & TR_WIS) total += (1000 * plusses);
	if (flags & TR_DEX) total += (1000 * plusses);
	if (flags & TR_CON) total += (1000 * plusses);
	if (flags & TR_CHR) total += (250 * plusses);
	if (flags & TR_CHAOTIC) total += 10000;
	if (flags & TR_VAMPIRIC) total += 13000;
	if (flags & TR_STEALTH) total += (250 * plusses);
	if (flags & TR_INFRA) total += (150 * plusses);
	if (flags & TR_TUNNEL) total += (175 * plusses);
	if ((flags & TR_SPEED) && (plusses > 0))
		total += (10000 + (2500 * plusses));
	if ((flags & TR_BLOWS) && (plusses > 0))
		total += (10000 + (2500 * plusses));
	if (flags & TR_MANA) total += (1000 * plusses);
	if (flags & TR_SPELL) total += (2000 * plusses);
	if (flags & TR_SLAY_ANIMAL) total += 3500;
	if (flags & TR_SLAY_EVIL) total += 4500;
	if (flags & TR_SLAY_UNDEAD) total += 3500;
	if (flags & TR_SLAY_DEMON) total += 3500;
	if (flags & TR_SLAY_ORC) total += 3000;
	if (flags & TR_SLAY_TROLL) total += 3500;
	if (flags & TR_SLAY_GIANT) total += 3500;
	if (flags & TR_SLAY_DRAGON) total += 3500;
	if (flags & TR_KILL_DEMON) total += 5500;
	if (flags & TR_KILL_UNDEAD) total += 5500;
	if (flags & TR_KILL_DRAGON) total += 5500;
	if (flags & TR_VORPAL) total += 5000;
	if (flags & TR_IMPACT) total += 5000;
	if (flags & TR_BRAND_POIS) total += 7500;
	if (flags & TR_BRAND_ACID) total += 7500;
	if (flags & TR_BRAND_ELEC) total += 7500;
	if (flags & TR_BRAND_FIRE) total += 5000;
	if (flags & TR_BRAND_COLD) total += 5000;
	if (flags & TR_SUST_STR) total += 850;
	if (flags & TR_SUST_INT) total += 850;
	if (flags & TR_SUST_WIS) total += 850;
	if (flags & TR_SUST_DEX) total += 850;
	if (flags & TR_SUST_CON) total += 850;
	if (flags & TR_SUST_CHR) total += 250;
	if (flags & TR_INVIS) total += 3000;
	if (flags & TR_LIFE) total += (5000 * plusses);
	if (flags & TR_IM_ACID) total += 10000;
	if (flags & TR_IM_ELEC) total += 10000;
	if (flags & TR_IM_FIRE) total += 10000;
	if (flags & TR_IM_COLD) total += 10000;
	if (flags & TR_SENS_FIRE) total -= 100;
	if (flags & TR_REFLECT) total += 10000;
	if (flags & TR_FREE_ACT) total += 4500;
	if (flags & TR_HOLD_LIFE) total += 8500;
	if (flags & TR_RES_ACID) total += 1250;
	if (flags & TR_RES_ELEC) total += 1250;
	if (flags & TR_RES_FIRE) total += 1250;
	if (flags & TR_RES_COLD) total += 1250;
	if (flags & TR_RES_POIS) total += 2500;
	if (flags & TR_RES_FEAR) total += 2500;
	if (flags & TR_RES_LITE) total += 1750;
	if (flags & TR_RES_DARK) total += 1750;
	if (flags & TR_RES_BLIND) total += 2000;
	if (flags & TR_RES_CONF) total += 2000;
	if (flags & TR_RES_SOUND) total += 2000;
	if (flags & TR_RES_SHARDS) total += 2000;
	if (flags & TR_RES_NETHER) total += 2000;
	if (flags & TR_RES_NEXUS) total += 2000;
	if (flags & TR_RES_CHAOS) total += 2000;
	if (flags & TR_RES_DISEN) total += 10000;
	if (flags & TR_SH_FIRE) total += 5000;
	if (flags & TR_SH_ELEC) total += 5000;
	if (flags & TR_DECAY) total += 0;
	if (flags & TR_NO_TELE) total += 2500;
	if (flags & TR_NO_MAGIC) total += 2500;
	if (flags & TR_WRAITH) total += 250000;
	if (flags & TR_TY_CURSE) total -= 15000;
	if (flags & TR_EASY_KNOW) total += 0;
	if (flags & TR_HIDE_TYPE) total += 0;
	if (flags & TR_SHOW_MODS) total += 0;
	if (flags & TR_INSTA_ART) total += 0;
	if (flags & TR_LITE1) total += 750;
	if (flags & TR_LITE2) total += 1250;
	if (flags & TR_LITE3) total += 2750;
	if (flags & TR_SEE_INVIS) total += 2000;
	total += 12500 * ((flags & object_flags_esp()).count());
	if (flags & TR_SLOW_DIGEST) total += 750;
	if (flags & TR_REGEN) total += 2500;
	if (flags & TR_XTRA_MIGHT) total += 2250;
	if (flags & TR_XTRA_SHOTS) total += 10000;
	if (flags & TR_IGNORE_ACID) total += 100;
	if (flags & TR_IGNORE_ELEC) total += 100;
	if (flags & TR_IGNORE_FIRE) total += 100;
	if (flags & TR_IGNORE_COLD) total += 100;
	if (flags & TR_ACTIVATE) total += 100;
	if (flags & TR_DRAIN_EXP) total -= 12500;
	if (flags & TR_TELEPORT)
	{
		if (o_ptr->art_flags & TR_CURSED)
			total -= 7500;
		else
			total += 250;
	}
	if (flags & TR_AGGRAVATE) total -= 10000;
	if (flags & TR_BLESSED) total += 750;
	if (flags & TR_CURSED) total -= 5000;
	if (flags & TR_HEAVY_CURSE) total -= 12500;
	if (flags & TR_PERMA_CURSE) total -= 15000;
	if (flags & TR_FEATHER) total += 1250;
	if (flags & TR_FLY) total += 10000;
	if (flags & TR_NEVER_BLOW) total -= 15000;
	if (flags & TR_PRECOGNITION) total += 250000;
	if (flags & TR_BLACK_BREATH) total -= 12500;
	if (flags & TR_DG_CURSE) total -= 25000;
	if (flags & TR_CLONE) total -= 10000;
	if (flags & TR_LEVELS) total += o_ptr->elevel * 2000;

	/* Also, give some extra for activatable powers... */

	if ((!o_ptr->artifact_name.empty()) && (o_ptr->art_flags & TR_ACTIVATE))
	{
		int type = o_ptr->xtra2;

		if (type == ACT_SUNLIGHT) total += 250;
		else if (type == ACT_BO_MISS_1) total += 250;
		else if (type == ACT_BA_POIS_1) total += 300;
		else if (type == ACT_BO_ELEC_1) total += 250;
		else if (type == ACT_BO_ACID_1) total += 250;
		else if (type == ACT_BO_COLD_1) total += 250;
		else if (type == ACT_BO_FIRE_1) total += 250;
		else if (type == ACT_BA_COLD_1) total += 750;
		else if (type == ACT_BA_FIRE_1) total += 1000;
		else if (type == ACT_DRAIN_1) total += 500;
		else if (type == ACT_BA_COLD_2) total += 1250;
		else if (type == ACT_BA_ELEC_2) total += 1500;
		else if (type == ACT_DRAIN_2) total += 750;
		else if (type == ACT_VAMPIRE_1) total += 1000;
		else if (type == ACT_BO_MISS_2) total += 1000;
		else if (type == ACT_BA_FIRE_2) total += 1750;
		else if (type == ACT_BA_COLD_3) total += 2500;
		else if (type == ACT_BA_ELEC_3) total += 2500;
		else if (type == ACT_WHIRLWIND) total += 7500;
		else if (type == ACT_VAMPIRE_2) total += 2500;
		else if (type == ACT_CALL_CHAOS) total += 5000;
		else if (type == ACT_ROCKET) total += 5000;
		else if (type == ACT_DISP_EVIL) total += 4000;
		else if (type == ACT_DISP_GOOD) total += 3500;
		else if (type == ACT_BA_MISS_3) total += 5000;
		else if (type == ACT_CONFUSE) total += 500;
		else if (type == ACT_SLEEP) total += 750;
		else if (type == ACT_QUAKE) total += 600;
		else if (type == ACT_TERROR) total += 2500;
		else if (type == ACT_TELE_AWAY) total += 2000;
		else if (type == ACT_GENOCIDE) total += 10000;
		else if (type == ACT_MASS_GENO) total += 10000;
		else if (type == ACT_CHARM_ANIMAL) total += 7500;
		else if (type == ACT_CHARM_UNDEAD) total += 10000;
		else if (type == ACT_CHARM_OTHER) total += 10000;
		else if (type == ACT_CHARM_ANIMALS) total += 12500;
		else if (type == ACT_CHARM_OTHERS) total += 17500;
		else if (type == ACT_SUMMON_ANIMAL) total += 10000;
		else if (type == ACT_SUMMON_PHANTOM) total += 12000;
		else if (type == ACT_SUMMON_ELEMENTAL) total += 15000;
		else if (type == ACT_SUMMON_DEMON) total += 20000;
		else if (type == ACT_SUMMON_UNDEAD) total += 20000;
		else if (type == ACT_CURE_LW) total += 500;
		else if (type == ACT_CURE_MW) total += 750;
		else if (type == ACT_REST_LIFE) total += 7500;
		else if (type == ACT_REST_ALL) total += 15000;
		else if (type == ACT_CURE_700) total += 10000;
		else if (type == ACT_CURE_1000) total += 15000;
		else if (type == ACT_ESP) total += 1500;
		else if (type == ACT_BERSERK) total += 800;
		else if (type == ACT_PROT_EVIL) total += 5000;
		else if (type == ACT_RESIST_ALL) total += 5000;
		else if (type == ACT_SPEED) total += 15000;
		else if (type == ACT_XTRA_SPEED) total += 25000;
		else if (type == ACT_WRAITH) total += 25000;
		else if (type == ACT_INVULN) total += 25000;
		else if (type == ACT_LIGHT) total += 150;
		else if (type == ACT_MAP_LIGHT) total += 500;
		else if (type == ACT_DETECT_ALL) total += 1000;
		else if (type == ACT_DETECT_XTRA) total += 12500;
		else if (type == ACT_RUNE_EXPLO) total += 4000;
		else if (type == ACT_RUNE_PROT) total += 10000;
		else if (type == ACT_SATIATE) total += 2000;
		else if (type == ACT_DEST_DOOR) total += 100;
		else if (type == ACT_STONE_MUD) total += 1000;
		else if (type == ACT_RECHARGE) total += 1000;
		else if (type == ACT_ALCHEMY) total += 10000;
		else if (type == ACT_DIM_DOOR) total += 10000;
		else if (type == ACT_TELEPORT) total += 2000;
		else if (type == ACT_RECALL) total += 7500;
	}

	return total;
}



/*
 * Return the "real" price of a "known" item, not including discounts
 *
 * Wand and staffs get cost for each charge
 *
 * Armor is worth an extra 100 gold per bonus point to armor class.
 *
 * Weapons are worth an extra 100 gold per bonus point (AC,TH,TD).
 *
 * Missiles are only worth 5 gold per bonus point, since they
 * usually appear in groups of 20, and we want the player to get
 * the same amount of cash for any "equivalent" item.  Note that
 * missiles never have any of the "pval" flags, and in fact, they
 * only have a few of the available flags, primarily of the "slay"
 * and "brand" and "ignore" variety.
 *
 * Armor with a negative armor bonus is worthless.
 * Weapons with negative hit+damage bonuses are worthless.
 *
 * Every wearable item with a "pval" bonus is worth extra (see below).
 */
s32b object_value_real(object_type const *o_ptr)
{
	auto const &r_info = game->edit_data.r_info;
	auto const &k_info = game->edit_data.k_info;
	auto const &a_info = game->edit_data.a_info;
	auto const &e_info = game->edit_data.e_info;

	if (o_ptr->tval == TV_RANDART)
	{
		return game->random_artifacts[o_ptr->sval].cost;
	}

	/* Get the object kind */
	auto k_ptr = o_ptr->k_ptr;

	/* Hack -- "worthless" items */
	if (!k_ptr->cost)
	{
		return (0L);
	}

	/* Base cost */
	s32b value = k_ptr->cost;

	/* Extract some flags */
	auto const flags = object_flags(o_ptr);

	if (flags & TR_TEMPORARY) return (0L);

	if (o_ptr->art_flags)
	{
		value += flag_cost (o_ptr, o_ptr->pval);
	}
	/* Artifact */
	else if (o_ptr->name1)
	{
		auto a_ptr = &a_info[o_ptr->name1];

		/* Hack -- "worthless" artifacts */
		if (!a_ptr->cost) return (0L);

		/* Hack -- Use the artifact cost instead */
		value = a_ptr->cost;
	}

	/* Ego-Item */
	else if (o_ptr->name2)
	{
		auto e_ptr = &e_info[o_ptr->name2];

		/* Hack -- "worthless" ego-items */
		if (!e_ptr->cost) return (0L);

		/* Hack -- Reward the ego-item with a bonus */
		value += e_ptr->cost;

		if (o_ptr->name2b)
		{
			auto e_ptr = &e_info[o_ptr->name2b];

			/* Hack -- "worthless" ego-items */
			if (!e_ptr->cost) return (0L);

			/* Hack -- Reward the ego-item with a bonus */
			value += e_ptr->cost;
		}
	}

	/* Pay the spell */
	if (flags & TR_SPELL_CONTAIN)
	{
		if (o_ptr->pval2 != -1)
			value += 5000 + 500 * spell_type_skill_level(spell_at(o_ptr->pval2));
		else
			value += 5000;
	}

	/* Analyze pval bonus */
	switch (o_ptr->tval)
	{
	case TV_BOW:
	case TV_BOOMERANG:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
	case TV_AXE:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
	case TV_LITE:
	case TV_AMULET:
	case TV_RING:
	case TV_MSTAFF:
	case TV_INSTRUMENT:
		{
			/* No pval */
			if (!o_ptr->pval) break;

			/* Give credit for stat bonuses */
			if (flags & TR_STR) value += (o_ptr->pval * 200L);
			if (flags & TR_INT) value += (o_ptr->pval * 200L);
			if (flags & TR_WIS) value += (o_ptr->pval * 200L);
			if (flags & TR_DEX) value += (o_ptr->pval * 200L);
			if (flags & TR_CON) value += (o_ptr->pval * 200L);
			if (flags & TR_CHR) value += (o_ptr->pval * 200L);

			if (flags & TR_CRIT) value += (o_ptr->pval * 500L);

			/* Give credit for stealth and searching */
			if (flags & TR_STEALTH) value += (o_ptr->pval * 100L);

			/* Give credit for infra-vision and tunneling */
			if (flags & TR_INFRA) value += (o_ptr->pval * 50L);
			if (flags & TR_TUNNEL) value += (o_ptr->pval * 50L);

			/* Give credit for extra attacks */
			if (flags & TR_BLOWS) value += (o_ptr->pval * 2000L);

			/* Give credit for speed bonus */
			if (flags & TR_SPEED) value += (o_ptr->pval * 30000L);

			break;
		}
	}


	/* Analyze the item */
	switch (o_ptr->tval)
	{
		/* Eggs */
	case TV_EGG:
		{
			auto r_ptr = &r_info[o_ptr->pval2];

			/* Pay the monster level */
			value += r_ptr->level * 100;

			/* Done */
			break;
		}

		/* Wands/Staffs */
	case TV_WAND:
		{
			/* Par for the spell */
			value *= spell_type_skill_level(spell_at(o_ptr->pval2));
			/* Take the average of the base and max spell levels */
			value *= (((o_ptr->pval3 >> 16) & 0xFFFF) + (o_ptr->pval3 & 0xFFFF)) / 2;
			/* Hack */
			value /= 6;

			/* Pay extra for charges */
			value += ((value / 20) * o_ptr->pval) / o_ptr->number;

			/* Done */
			break;
		}
	case TV_STAFF:
		{
			/* Par for the spell */
			value *= spell_type_skill_level(spell_at(o_ptr->pval2));
			/* Take the average of the base and max spell levels */
			value *= (((o_ptr->pval3 >> 16) & 0xFFFF) + (o_ptr->pval3 & 0xFFFF)) / 2;
			/* Hack */
			value /= 6;

			/* Pay extra for charges */
			value += ((value / 20) * o_ptr->pval);

			/* Done */
			break;
		}
	case TV_BOOK:
		{
			if (o_ptr->sval == 255)
			{
				/* Pay extra for the spell */
				value = value * spell_type_skill_level(spell_at(o_ptr->pval));
			}
			/* Done */
			break;
		}

		/* Rods */
	case TV_ROD_MAIN:
		{
			s16b tip_idx;

			/* It's not combined */
			if (o_ptr->pval == 0) break;

			/* Look up the tip attached */
			tip_idx = lookup_kind(TV_ROD, o_ptr->pval);

			/* Paranoia */
			if (tip_idx > 0)
			{
				/* Add its cost */
				value += k_info.at(tip_idx)->cost;
			}

			/* Done */
			break;
		}

		/* Rings/Amulets */
	case TV_RING:
	case TV_AMULET:
		{
			/* Hack -- negative bonuses are bad */
			if (o_ptr->to_a < 0 && !value) return (0L);
			if (o_ptr->to_h < 0 && !value) return (0L);
			if (o_ptr->to_d < 0 && !value) return (0L);

			/* Give credit for bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Done */
			break;
		}

		/* Armor */
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CLOAK:
	case TV_CROWN:
	case TV_HELM:
	case TV_SHIELD:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
		{
			/* Hack -- negative armor bonus */
			if (o_ptr->to_a < 0 && !value) return (0L);

			/* Give credit for bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Done */
			break;
		}

		/* Bows/Weapons */
	case TV_BOW:
	case TV_BOOMERANG:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_SWORD:
	case TV_DAEMON_BOOK:
	case TV_AXE:
	case TV_POLEARM:
		{
			/* Hack -- negative hit/damage bonuses */
			if (o_ptr->to_h + o_ptr->to_d < 0 && !value) return (0L);

			/* Factor in the bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

			/* Hack -- Factor in extra damage dice */
			if ((o_ptr->dd > k_ptr->dd) && (o_ptr->ds == k_ptr->ds))
			{
				value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 100L;
			}

			/* Done */
			break;
		}

		/* Ammo */
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		{
			/* Hack -- negative hit/damage bonuses */
			if (o_ptr->to_h + o_ptr->to_d < 0 && !value) return (0L);

			/* Factor in the bonuses */
			value += ((o_ptr->to_h + o_ptr->to_d) * 5L);

			/* Hack -- Factor in extra damage dice */
			if ((o_ptr->dd > k_ptr->dd) && (o_ptr->ds == k_ptr->ds))
			{
				value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 5L;
			}

			/* Special attack (exploding arrow) */
			if (o_ptr->pval2 != 0) value *= 14;

			/* Done */
			break;
		}
	}

	/* Return the value */
	return (value);
}


/*
 * Return the price of an item including plusses (and charges)
 *
 * This function returns the "value" of the given item (qty one)
 */
s32b object_value(object_type const *o_ptr)
{
	/* Cursed items -- worthless */
	if (cursed_p(o_ptr))
	{
		return (0L);
	}

	/* Real value */
	s32b value = object_value_real(o_ptr);

	/* Apply discount (if any) */
	if (o_ptr->discount)
	{
		value -= (value * o_ptr->discount / 100L);
	}

	/* Return the final value */
	return value;
}



/*
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow wands/staffs (if they are known to have equal
 * charges) and rods (if fully charged) to combine.  They will unstack
 * (if necessary) when they are used.
 *
 * If permitted, we allow weapons/armor to stack, if fully "known".
 *
 * Missiles will combine if both stacks have the same "known" status.
 * This is done to make unidentified stacks of missiles useful.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests, and activatable items, never stack (for various reasons).
 */
bool object_similar(object_type const *o_ptr, object_type const *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Require identical object types */
	if (o_ptr->k_ptr != j_ptr->k_ptr)
	{
		return false;
	}

	/* Extract the flags */
	auto const o_flags = object_flags(o_ptr);
	auto const j_flags = object_flags(j_ptr);

	if ((o_flags & TR_SPELL_CONTAIN) || (j_flags & TR_SPELL_CONTAIN))
		return false;

	/* Analyze the items */
	switch (o_ptr->tval)
	{
		/* School Book */
	case TV_BOOK:
		{
			if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) return false;

			/* Beware artifatcs should not combibne with "lesser" thing */
			if (artifact_p(o_ptr) != artifact_p(j_ptr)) return false;

			/* Do not combine different ego or normal ones */
			if (ego_item_p(o_ptr) != ego_item_p(j_ptr)) return false;

			/* Random books should stack if they are identical */
			if ((o_ptr->sval == 255) && (j_ptr->sval == 255))
			{
				if (o_ptr->pval != j_ptr->pval)
					return false;
			}

			return true;
		}

	case TV_RANDART:
		{
			return false;
		}

	case TV_INSTRUMENT:
		{
			return false;
		}

	case TV_HYPNOS:
	case TV_EGG:
		{
			return false;
		}

		/* Totems */
	case TV_TOTEM:
		{
			if ((o_ptr->pval == j_ptr->pval) && (o_ptr->pval2 == j_ptr->pval2)) return true;
			return false;
		}

		/* Corpses*/
	case TV_CORPSE:
		{
			return false;
		}

		/* Food and Potions and Scrolls */
	case TV_POTION:
	case TV_POTION2:
		{
			if (o_ptr->pval2 != j_ptr->pval2) return false;

			/* Assume okay */
			break;
		}

	case TV_SCROLL:
		{
			if (o_ptr->pval != j_ptr->pval) return false;
			if (o_ptr->pval2 != j_ptr->pval2) return false;
			break;
		}

		/* Staffs */
	case TV_STAFF:
		{
			/* Require knowledge for both staffs. */
			if ((!object_known_p(o_ptr)) || (!object_known_p(j_ptr))) return (false);

			/* Require identical charges, since staffs are bulky. */
			if (o_ptr->pval != j_ptr->pval) return (false);

			/* Do not combine recharged ones with non recharged ones. */
			if ((o_flags & TR_RECHARGED) != (j_flags & TR_RECHARGED)) return (false);

			/* Do not combine different spells */
			if (o_ptr->pval2 != j_ptr->pval2) return (false);

			/* Do not combine different base levels */
			if (o_ptr->pval3 != j_ptr->pval3) return (false);

			/* Beware artifatcs should not combibne with "lesser" thing */
			if (o_ptr->name1 != j_ptr->name1) return (false);

			/* Do not combine different ego or normal ones */
			if (o_ptr->name2 != j_ptr->name2) return (false);

			/* Do not combine different ego or normal ones */
			if (o_ptr->name2b != j_ptr->name2b) return (false);

			/* Assume okay */
			break;
		}

		/* Wands */
	case TV_WAND:
		{
			/* Require knowledge for both wands. */
			if ((!object_known_p(o_ptr)) || !object_known_p(j_ptr)) return (false);

			/* Beware artifatcs should not combibne with "lesser" thing */
			if (o_ptr->name1 != j_ptr->name1) return (false);

			/* Do not combine recharged ones with non recharged ones. */
			if ((o_flags & TR_RECHARGED) != (j_flags & TR_RECHARGED)) return (false);

			/* Do not combine different spells */
			if (o_ptr->pval2 != j_ptr->pval2) return (false);

			/* Do not combine different base levels */
			if (o_ptr->pval3 != j_ptr->pval3) return (false);

			/* Do not combine different ego or normal ones */
			if (o_ptr->name2 != j_ptr->name2) return (false);

			/* Do not combine different ego or normal ones */
			if (o_ptr->name2b != j_ptr->name2b) return (false);

			/* Assume okay */
			break;
		}

		/* Rod Tips */
	case TV_ROD:
		{
			/* Probably okay */
			break;
		}

		/* Rods */
	case TV_ROD_MAIN:
		{
			return false;
			break;
		}

		/* Weapons and Armor */
	case TV_BOW:
	case TV_BOOMERANG:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
	case TV_DAEMON_BOOK:
		{
			/* Fall through */
		}

		/* Rings, Amulets, Lites */
	case TV_RING:
	case TV_AMULET:
	case TV_LITE:
		{
			/* Require full knowledge of both items */
			if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) return (false);

			/* Require identical "turns of light" */
			if (o_ptr->timeout != j_ptr->timeout) return false;

			/* Fall through */
		}

		/* Missiles */
	case TV_BOLT:
	case TV_ARROW:
	case TV_SHOT:
		{
			/* Require identical knowledge of both items */
			if (object_known_p(o_ptr) != object_known_p(j_ptr)) return (false);

			/* Require identical "bonuses" */
			if (o_ptr->to_h != j_ptr->to_h) return false;
			if (o_ptr->to_d != j_ptr->to_d) return false;
			if (o_ptr->to_a != j_ptr->to_a) return false;

			/* Require identical "pval" code */
			if (o_ptr->pval != j_ptr->pval) return false;

			/* Require identical exploding status code */
			if (o_ptr->pval2 != j_ptr->pval2) return false;

			/* Require identical "artifact" names */
			if (o_ptr->name1 != j_ptr->name1) return false;

			/* Require identical "ego-item" names */
			if (o_ptr->name2 != j_ptr->name2) return false;

			/* Do not combine different ego or normal ones */
			if (o_ptr->name2b != j_ptr->name2b) return false;

			/* Hack -- Never stack "powerful" items */
			/*
			   Why?!
			-- wilh
			*/
			/* #if 0 */
			if (o_ptr->xtra1 || j_ptr->xtra1) return false;
			/* #endif */

			/* Hack -- Never stack recharging items */
			if ((o_ptr->timeout || j_ptr->timeout) &&
			                (o_ptr->tval != TV_LITE)) return false;

			/* Require identical "values" */
			if (o_ptr->ac != j_ptr->ac) return false;
			if (o_ptr->dd != j_ptr->dd) return false;
			if (o_ptr->ds != j_ptr->ds) return false;

			/* Probably okay */
			break;
		}

		/* UHH ugly hack for the mushroom quest, sorry */
	case TV_FOOD:
		{
			if (o_ptr->pval2 != j_ptr->pval2) return false;
			break;
		}

		/* Various */
	default:
		{
			/* Require knowledge */
			if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) return (false);

			/* Probably okay */
			break;
		}
	}


	/* Hack -- Identical art_flags! */
	if (o_ptr->art_flags != j_ptr->art_flags)
	{
		return (false);
	}

	/* Hack -- require semi-matching "inscriptions" */
	if ((!o_ptr->inscription.empty()) && (!j_ptr->inscription.empty()) && (o_ptr->inscription != j_ptr->inscription))
	{
		return (false);
	}

	/* Maximal "stacking" limit */
	if (total >= MAX_STACK_SIZE) return (false);


	/* They match, so they must be similar */
	return true;
}


/*
 * Allow one item to "absorb" another, assuming they are similar
 */
void object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Add together the item counts */
	o_ptr->number = ((total < MAX_STACK_SIZE) ? total : (MAX_STACK_SIZE - 1));

	/* Hack -- blend "known" status */
	if (object_known_p(j_ptr)) object_known(o_ptr);

	/* Hack -- blend "inscriptions" */
	if (!j_ptr->inscription.empty())
	{
		o_ptr->inscription = j_ptr->inscription;
	}

	/* Hack -- could average discounts XXX XXX XXX */
	/* Hack -- save largest discount XXX XXX XXX */
	if (o_ptr->discount < j_ptr->discount) o_ptr->discount = j_ptr->discount;

	/* Hack -- if wands are stacking, combine the charges. -LM- */
	if (o_ptr->tval == TV_WAND)
	{
		o_ptr->pval += j_ptr->pval;
	}
}



/*
 * Find the index of the object_kind with the given tval and sval
 */
s16b lookup_kind(int tval, int sval)
{
	auto const &k_info = game->edit_data.k_info;

	for (auto const &k_entry: k_info)
	{
		auto const &k_ptr = k_entry.second;
		if ((k_ptr->tval == tval) && (k_ptr->sval == sval))
		{
			return k_entry.first;
		}
	}

	/* Oops */
	if (wizard) msg_format("No object (%d,%d)", tval, sval);

	/* Oops */
	return 0;
}


/*
 * Wipe an object clean.
 */
void object_wipe(object_type *o_ptr)
{
	/* Wipe the structure */
	*o_ptr = object_type();
}


/*
 * Prepare an object based on an existing object
 */
void object_copy(object_type *o_ptr, object_type *j_ptr)
{
	/* Copy the structure */
	*o_ptr = *j_ptr;
}


/*
 * Initialize the experience of an object which is a
 * "sentient" object.
 */
static void init_obj_exp(object_type *o_ptr, std::shared_ptr<object_kind const> k_ptr)
{
	o_ptr->elevel = (k_ptr->level / 10) + 1;
	o_ptr->exp = player_exp[o_ptr->elevel - 1];
}


/*
 * Prepare an object based on an object kind.
 */
void object_prep(object_type *o_ptr, int k_idx)
{
	auto const &k_info = game->edit_data.k_info;
	auto k_ptr = k_info.at(k_idx);

	/* Clear the record */
	object_wipe(o_ptr);

	/* Save the kind index */
	o_ptr->k_ptr = k_ptr;

	/* Efficiency -- tval/sval */
	o_ptr->tval = k_ptr->tval;
	o_ptr->sval = k_ptr->sval;

	/* Default "pval" */
	o_ptr->pval = k_ptr->pval;
	o_ptr->pval2 = k_ptr->pval2;

	/* Default number */
	o_ptr->number = 1;

	/* Default weight */
	o_ptr->weight = k_ptr->weight;

	/* Default magic */
	o_ptr->to_h = k_ptr->to_h;
	o_ptr->to_d = k_ptr->to_d;
	o_ptr->to_a = k_ptr->to_a;

	/* Default power */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Hack -- cursed items are always "cursed" */
	if (k_ptr->flags & TR_CURSED)
	{
		o_ptr->art_flags |= TR_CURSED;
	}

	/* Hack give a basic exp/exp level to an object that needs it */
	if (k_ptr->flags & TR_LEVELS)
	{
		init_obj_exp(o_ptr, k_ptr);
		o_ptr->pval2 = 1;        /* Start with one point */
		o_ptr->pval3 = 0;        /* No flags groups */
	}
}


/*
 * Help determine an "enchantment bonus" for an object.
 *
 * To avoid floating point but still provide a smooth distribution of bonuses,
 * we simply round the results of division in such a way as to "average" the
 * correct floating point value.
 *
 * This function has been changed.  It uses "randnor()" to choose values from
 * a normal distribution, whose mean moves from zero towards the max as the
 * level increases, and whose standard deviation is equal to 1/4 of the max,
 * and whose values are forced to lie between zero and the max, inclusive.
 *
 * Since the "level" rarely passes 100 before Morgoth is dead, it is very
 * rare to get the "full" enchantment on an object, even a deep levels.
 *
 * It is always possible (albeit unlikely) to get the "full" enchantment.
 *
 * A sample distribution of values from "m_bonus(10, N)" is shown below:
 *
 *   N       0     1     2     3     4     5     6     7     8     9    10
 * ---    ----  ----  ----  ----  ----  ----  ----  ----  ----  ----  ----
 *   0   66.37 13.01  9.73  5.47  2.89  1.31  0.72  0.26  0.12  0.09  0.03
 *   8   46.85 24.66 12.13  8.13  4.20  2.30  1.05  0.36  0.19  0.08  0.05
 *  16   30.12 27.62 18.52 10.52  6.34  3.52  1.95  0.90  0.31  0.15  0.05
 *  24   22.44 15.62 30.14 12.92  8.55  5.30  2.39  1.63  0.62  0.28  0.11
 *  32   16.23 11.43 23.01 22.31 11.19  7.18  4.46  2.13  1.20  0.45  0.41
 *  40   10.76  8.91 12.80 29.51 16.00  9.69  5.90  3.43  1.47  0.88  0.65
 *  48    7.28  6.81 10.51 18.27 27.57 11.76  7.85  4.99  2.80  1.22  0.94
 *  56    4.41  4.73  8.52 11.96 24.94 19.78 11.06  7.18  3.68  1.96  1.78
 *  64    2.81  3.07  5.65  9.17 13.01 31.57 13.70  9.30  6.04  3.04  2.64
 *  72    1.87  1.99  3.68  7.15 10.56 20.24 25.78 12.17  7.52  4.42  4.62
 *  80    1.02  1.23  2.78  4.75  8.37 12.04 27.61 18.07 10.28  6.52  7.33
 *  88    0.70  0.57  1.56  3.12  6.34 10.06 15.76 30.46 12.58  8.47 10.38
 *  96    0.27  0.60  1.25  2.28  4.30  7.60 10.77 22.52 22.51 11.37 16.53
 * 104    0.22  0.42  0.77  1.36  2.62  5.33  8.93 13.05 29.54 15.23 22.53
 * 112    0.15  0.20  0.56  0.87  2.00  3.83  6.86 10.06 17.89 27.31 30.27
 * 120    0.03  0.11  0.31  0.46  1.31  2.48  4.60  7.78 11.67 25.53 45.72
 * 128    0.02  0.01  0.13  0.33  0.83  1.41  3.24  6.17  9.57 14.22 64.07
 */
s16b m_bonus(int max, int level)
{
	int bonus, stand, extra, value;


	/* Paranoia -- enforce maximal "level" */
	if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;


	/* The "bonus" moves towards the max */
	bonus = ((max * level) / MAX_DEPTH);

	/* Hack -- determine fraction of error */
	extra = ((max * level) % MAX_DEPTH);

	/* Hack -- simulate floating point computations */
	if (rand_int(MAX_DEPTH) < extra) bonus++;


	/* The "stand" is equal to one quarter of the max */
	stand = (max / 4);

	/* Hack -- determine fraction of error */
	extra = (max % 4);

	/* Hack -- simulate floating point computations */
	if (rand_int(4) < extra) stand++;


	/* Choose an "interesting" value */
	value = randnor(bonus, stand);

	/* Enforce the minimum value */
	if (value < 0) return (0);

	/* Enforce the maximum value */
	if (value > max) return (max);

	/* Result */
	return (value);
}


/*
 * Tinker with the random artifact to make it acceptable
 * for a certain depth; also connect a random artifact to an 
 * object.
 */
static void finalize_randart(object_type* o_ptr, int lev)
{
	auto &random_artifacts = game->random_artifacts;

	/* Paranoia */
	if (o_ptr->tval != TV_RANDART)
	{
		return;
	}

	for (int i = 0; ; i++)
	{
		auto const r = rand_int(MAX_RANDARTS);

		if (!(random_artifacts[r].generated) || i > 2000)
		{
			auto ra_ptr = &random_artifacts[r];

			o_ptr->sval = r;
			o_ptr->pval2 = ra_ptr->activation;
			o_ptr->xtra2 = activation_info[ra_ptr->activation].spell;

			ra_ptr->level = lev;
			ra_ptr->generated = true;
			return;
		}
	}
}



/*
 * Cheat -- describe a created object for the user
 */
static void object_mention(object_type *o_ptr)
{
	char o_name[80];

	/* Describe */
	object_desc_store(o_name, o_ptr, false, 0);

	/* Artifact */
	if (artifact_p(o_ptr))
	{
		/* Silly message */
		msg_format("Artifact (%s)", o_name);
	}

	/* Random Artifact */
	else if (!o_ptr->artifact_name.empty())
	{
		msg_print("Random artifact");
	}

	/* Ego-item */
	else if (ego_item_p(o_ptr))
	{
		/* Silly message */
		msg_format("Ego-item (%s)", o_name);
	}

	/* Normal item */
	else
	{
		/* Silly message */
		msg_format("Object (%s)", o_name);
	}
}

static void random_artifact_power(object_type *o_ptr)
{
	// Shorthand
	auto flags = &o_ptr->art_flags;

	// Choose ability
	auto try_choose = [&flags](int choice) {
		switch (choice)
		{
		case 0:
			(*flags) |= (TR_FEATHER);
			break;
		case 1:
			(*flags) |= (TR_LITE1);
			break;
		case 2:
			(*flags) |= (TR_SEE_INVIS);
			break;
		case 3:
			(*flags) |= (ESP_ALL);
			break;
		case 4:
			(*flags) |= (TR_SLOW_DIGEST);
			break;
		case 5:
			(*flags) |= (TR_REGEN);
			break;
		case 6:
			(*flags) |= (TR_FREE_ACT);
			break;
		case 7:
			(*flags) |= (TR_HOLD_LIFE);
			break;
		}
	};

	// Save old values for comparison
	auto const old_flags = *flags;

	// Choose an ability; make sure we choose one that isn't already chosen
	for (int tries = 0; tries < 1000; tries++)
	{
		// Tentative choice
		int choice = rand_int(8);
		try_choose(choice);

		// If there's any difference, then we've chosen a non-overlapping power.
		if (*flags != old_flags)
		{
			break;
		}
	}
}

void random_artifact_resistance(object_type * o_ptr)
{
	auto const &a_info = game->edit_data.a_info;

	auto const art_flags = a_info[o_ptr->name1].flags;

	// Check flags of the 'protype' artifact
	auto give_resistance = bool(art_flags & TR_RANDOM_RESIST);
	auto give_power = bool(art_flags & TR_RANDOM_POWER);
	if (art_flags & TR_RANDOM_RES_OR_POWER)
	{
		if (randint(2) == 1)
		{
			give_resistance = true;
		}
		else
		{
			give_power = true;
		}
	}

	// Grant a power?
	if (give_power)
	{
		random_artifact_power(o_ptr);
	}

	artifact_bias = 0;

	if (give_resistance)
	{
		// Save the *combined* pre-existing flags on the object;
		// including any power we may have granted above.
		auto const flags_before = art_flags | o_ptr->art_flags;

		// We'll be a little generous here and make sure that the object
		// gets a resistance that it doesn't actually already have.
		for (int tries = 0; tries < 1000; tries++)
		{
			random_resistance(o_ptr, randint(22) + 16);
			// Picked up new resistance?
			if (flags_before != (art_flags | o_ptr->art_flags))
			{
				break;
			}
		}
	}
}


/*
 * Mega-Hack -- Attempt to create one of the "Special Objects"
 *
 * We are only called from "make_object()", and we assume that
 * "apply_magic()" is called immediately after we return.
 *
 * Note -- see "make_artifact()" and "apply_magic()"
 */
static bool make_artifact_special(object_type *o_ptr)
{
	auto const &k_info = game->edit_data.k_info;
	auto const &a_info = game->edit_data.a_info;

	/* No artifacts in the town */
	if (!dun_level) return false;

	/* Check the artifact list (just the "specials") */
	for (std::size_t i = 0; i < a_info.size(); i++)
	{
		auto a_ptr = &a_info[i];

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* Cannot generate non special ones */
		if (!(a_ptr->flags & TR_INSTA_ART)) continue;

		/* Cannot generate some artifacts because they can only exists in special dungeons/quests/... */
		if ((a_ptr->flags & TR_SPECIAL_GENE) && (!a_allow_special[i])) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > dun_level)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (a_ptr->level - dun_level) * 2;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* Artifact "rarity roll" */
		if (rand_int(a_ptr->rarity - luck( -(a_ptr->rarity / 2), a_ptr->rarity / 2)) != 0) continue;

		/* Find the base object */
		int k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);
		auto const &k_ptr = k_info.at(k_idx);

		/* XXX XXX Enforce minimum "object" level (loosely) */
		if (k_ptr->level > object_level)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (k_ptr->level - object_level) * 5;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* Assign the template */
		object_prep(o_ptr, k_idx);

		/* Mega-Hack -- mark the item as an artifact */
		o_ptr->name1 = i;

		/* Extract some flags */
		auto const flags = object_flags(o_ptr);

		/* Hack give a basic exp/exp level to an object that needs it */
		if (flags & TR_LEVELS)
		{
			init_obj_exp(o_ptr, k_ptr);
		}

		/* Success */
		return true;
	}

	/* Failure */
	return false;
}


/*
 * Attempt to change an object into an artifact
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(object_type *o_ptr)
{
	auto const &a_info = game->edit_data.a_info;

	/* No artifacts in the town */
	if (!dun_level) return false;

	/* Paranoia -- no "plural" artifacts */
	if (o_ptr->number != 1) return false;

	/* Check the artifact list (skip the "specials") */
	for (std::size_t i = 0; i < a_info.size(); i++)
	{
		auto a_ptr = &a_info[i];

		/* Skip "empty" items */
		if (!a_ptr->name) continue;

		/* Cannot make an artifact twice */
		if (a_ptr->cur_num) continue;

		/* Cannot generate special ones */
		if (a_ptr->flags & TR_INSTA_ART) continue;

		/* Cannot generate some artifacts because they can only exists in special dungeons/quests/... */
		if ((a_ptr->flags & TR_SPECIAL_GENE) && (!a_allow_special[i])) continue;

		/* Must have the correct fields */
		if (a_ptr->tval != o_ptr->tval) continue;
		if (a_ptr->sval != o_ptr->sval) continue;

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (a_ptr->level > dun_level)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (a_ptr->level - dun_level) * 2;

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0) continue;
		}

		/* We must make the "rarity roll" */
		if (rand_int(a_ptr->rarity - luck( -(a_ptr->rarity / 2), a_ptr->rarity / 2)) != 0) continue;

		/* Hack -- mark the item as an artifact */
		o_ptr->name1 = i;

		/* Hack: Some artifacts get random extra powers */
		random_artifact_resistance(o_ptr);

		/* Extract some flags */
		auto const flags = object_flags(o_ptr);

		/* Hack give a basic exp/exp level to an object that needs it */
		if (flags & TR_LEVELS)
		{
			init_obj_exp(o_ptr, o_ptr->k_ptr);
		}

		/* Success */
		return true;
	}

	/* Failure */
	return false;
}

/*
 * Attempt to change an object into an ego
 *
 * This routine should only be called by "apply_magic()"
 */
static bool make_ego_item(object_type *o_ptr, bool good)
{
	auto const &e_info = game->edit_data.e_info;

	bool ret = false;

	if (artifact_p(o_ptr) || o_ptr->name2) return false;

	std::vector<size_t> ok_ego;

	/* Grab the ok ego */
	for (size_t i = 0; i < e_info.size(); i++)
	{
		auto e_ptr = &e_info[i];
		bool ok = false;

		/* Skip "empty" items */
		if (e_ptr->name.empty())
		{
			continue;
		}

		/* Must have the correct fields */
		for (size_t j = 0; j < 6; j++)
		{
			if (e_ptr->tval[j] == o_ptr->tval)
			{
				if ((e_ptr->min_sval[j] <= o_ptr->sval) && (e_ptr->max_sval[j] >= o_ptr->sval)) ok = true;
			}

			if (ok) break;
		}
		if (!ok)
		{
			/* Doesnt count as a try*/
			continue;
		}

		/* Good should be good, bad should be bad */
		if (good && (!e_ptr->cost)) continue;
		if ((!good) && e_ptr->cost) continue;

		/* Must posses the good flags */
		auto k_ptr = o_ptr->k_ptr;
		if ((k_ptr->flags & e_ptr->need_flags) != e_ptr->need_flags)
		{
			continue;
		}
		if (k_ptr->flags & e_ptr->forbid_flags)
		{
			continue;
		}

		/* ok */
		ok_ego.push_back(i);
	}

	/* Now test them a few times */
	for (size_t i = 0; i < ok_ego.size() * 10; i++)
	{
		size_t j = ok_ego[rand_int(ok_ego.size())];
		auto e_ptr = &e_info[j];

		/* XXX XXX Enforce minimum "depth" (loosely) */
		if (e_ptr->level > dun_level)
		{
			/* Acquire the "out-of-depth factor" */
			int d = (e_ptr->level - dun_level);

			/* Roll for out-of-depth creation */
			if (rand_int(d) != 0)
			{
				continue;
			}
		}

		/* We must make the "rarity roll" */
		if (rand_int(e_ptr->mrarity - luck( -(e_ptr->mrarity / 2), e_ptr->mrarity / 2)) > e_ptr->rarity)
		{
			continue;
		}

		/* Hack -- mark the item as an ego */
		o_ptr->name2 = j;

		/* Success */
		ret = true;
		break;
	}

	/*
	 * Sometimes(rarely) tries for a double ego
	        * Also make sure we dont already have a name2b, wchih would mean a special ego item
	 */
	if (magik(7 + luck( -7, 7)) && (!o_ptr->name2b))
	{
		/* Now test them a few times */
		for (size_t i = 0; i < ok_ego.size() * 10; i++)
		{
			int j = ok_ego[rand_int(ok_ego.size())]; // Explicit int conversion to avoid warning
			auto e_ptr = &e_info[j];

			/* Cannot be a double ego of the same ego type */
			if (j == o_ptr->name2) continue;

			/* Cannot have 2 suffixes or 2 prefixes */
			if (e_info[o_ptr->name2].before && e_ptr->before) continue;
			if ((!e_info[o_ptr->name2].before) && (!e_ptr->before)) continue;

			/* XXX XXX Enforce minimum "depth" (loosely) */
			if (e_ptr->level > dun_level)
			{
				/* Acquire the "out-of-depth factor" */
				int d = (e_ptr->level - dun_level);

				/* Roll for out-of-depth creation */
				if (rand_int(d) != 0)
				{
					continue;
				}
			}

			/* We must make the "rarity roll" */
			if (rand_int(e_ptr->mrarity - luck( -(e_ptr->mrarity / 2), e_ptr->mrarity / 2)) > e_ptr->rarity)
			{
				continue;
			}

			/* Hack -- mark the item as an ego */
			o_ptr->name2b = j;

			/* Success */
			ret = true;
			break;
		}
	}

	/* Return */
	return (ret);
}


/*
 * Charge a new stick.
 */
void charge_stick(object_type *o_ptr)
{
	spell_type *spell = spell_at(o_ptr->pval2);
	o_ptr->pval = spell_type_roll_charges(spell);
}

/*
 * Apply magic to an item known to be a "weapon"
 *
 * Hack -- note special base damage dice boosting
 * Hack -- note special processing for weapon/digger
 * Hack -- note special rating boost for dragon scale mail
 */
static void a_m_aux_1(object_type *o_ptr, int level, int power)
{
	int tohit1 = randint(5) + m_bonus(5, level);
	int todam1 = randint(5) + m_bonus(5, level);

	int tohit2 = m_bonus(10, level);
	int todam2 = m_bonus(10, level);

	artifact_bias = 0;

	/* Very good */
	if (power > 1)
	{
		/* Make ego item */
		if (rand_int(RANDART_WEAPON) == 1) create_artifact(o_ptr, false, true);
		else make_ego_item(o_ptr, true);
	}
	else if (power < -1)
	{
		/* Make ego item */
		make_ego_item(o_ptr, false);
	}

	/* Good */
	if (power > 0)
	{
		/* Enchant */
		o_ptr->to_h += tohit1;
		o_ptr->to_d += todam1;

		/* Very good */
		if (power > 1)
		{
			/* Enchant again */
			o_ptr->to_h += tohit2;
			o_ptr->to_d += todam2;
		}
	}

	/* Cursed */
	else if (power < 0)
	{
		/* Penalize */
		o_ptr->to_h -= tohit1;
		o_ptr->to_d -= todam1;

		/* Very cursed */
		if (power < -1)
		{
			/* Penalize again */
			o_ptr->to_h -= tohit2;
			o_ptr->to_d -= todam2;
		}

		/* Cursed (if "bad") */
		if (o_ptr->to_h + o_ptr->to_d < 0)
		{
			o_ptr->art_flags |= TR_CURSED;
		}
	}

	/* Some special cases */
	switch (o_ptr->tval)
	{
	case TV_MSTAFF:
		{
			o_ptr->art_flags |= (TR_SPELL_CONTAIN | TR_WIELD_CAST);
			break;
		}
	case TV_BOLT:
	case TV_ARROW:
	case TV_SHOT:
		{
			if ((power == 1) && !o_ptr->name2)
			{
				if (randint(100) < 30)
				{
					/* Exploding missile */
					int power[27] = {GF_ELEC, GF_POIS, GF_ACID,
					                 GF_COLD, GF_FIRE, GF_PLASMA, GF_LITE,
					                 GF_DARK, GF_SHARDS, GF_SOUND,
					                 GF_CONFUSION, GF_FORCE, GF_INERTIA,
					                 GF_MANA, GF_METEOR, GF_ICE, GF_CHAOS,
					                 GF_NETHER, GF_NEXUS, GF_TIME,
					                 GF_GRAVITY, GF_KILL_WALL, GF_AWAY_ALL,
					                 GF_TURN_ALL, GF_NUKE, GF_STUN,
					                 GF_DISINTEGRATE};

					o_ptr->pval2 = power[rand_int(27)];
				}
			}
			break;
		}
	}
}


static void dragon_resist(object_type * o_ptr)
{
	do
	{
		artifact_bias = 0;

		if (randint(4) == 1)
			random_resistance(o_ptr, randint(14) + 4);
		else
			random_resistance(o_ptr, randint(22) + 16);
	}
	while (randint(2) == 1);
}


/*
 * Apply magic to an item known to be "armor"
 *
 * Hack -- note special processing for crown/helm
 * Hack -- note special processing for robe of permanence
 */
static void a_m_aux_2(object_type *o_ptr, int level, int power)
{
	int toac1 = randint(5) + m_bonus(5, level);

	int toac2 = m_bonus(10, level);

	artifact_bias = 0;

	/* Very good */
	if (power > 1)
	{
		/* Make ego item */
		if (rand_int(RANDART_ARMOR) == 1) create_artifact(o_ptr, false, true);
		else make_ego_item(o_ptr, true);
	}
	else if (power < -1)
	{
		/* Make ego item */
		make_ego_item(o_ptr, false);
	}

	/* Good */
	if (power > 0)
	{
		/* Enchant */
		o_ptr->to_a += toac1;

		/* Very good */
		if (power > 1)
		{
			/* Enchant again */
			o_ptr->to_a += toac2;
		}
	}

	/* Cursed */
	else if (power < 0)
	{
		/* Penalize */
		o_ptr->to_a -= toac1;

		/* Very cursed */
		if (power < -1)
		{
			/* Penalize again */
			o_ptr->to_a -= toac2;
		}

		/* Cursed (if "bad") */
		if (o_ptr->to_a < 0)
		{
			o_ptr->art_flags |= TR_CURSED;
		}
	}

	/* Analyze type */
	switch (o_ptr->tval)
	{
	case TV_CLOAK:
		{
			if (o_ptr->sval == SV_ELVEN_CLOAK)
			{
				o_ptr->pval = randint(4);        /* No cursed elven cloaks...? */
			}
			else if (o_ptr->sval == SV_MIMIC_CLOAK)
			{
				s32b mimic = find_random_mimic_shape(level, true);
				o_ptr->pval2 = mimic;
			}
			break;
		}
	case TV_DRAG_ARMOR:
		{
			/* Rating boost */
			rating += 30;

			/* Mention the item */
			if (options->cheat_peek || p_ptr->precognition)
			{
				object_mention(o_ptr);
			}

			break;
		}
	case TV_SHIELD:
		{
			if (o_ptr->sval == SV_DRAGON_SHIELD)
			{
				/* Rating boost */
				rating += 5;

				/* Mention the item */
				if (options->cheat_peek || p_ptr->precognition)
				{
					object_mention(o_ptr);
				}
				dragon_resist(o_ptr);
			}
			break;
		}
	case TV_HELM:
		{
			if (o_ptr->sval == SV_DRAGON_HELM)
			{
				/* Rating boost */
				rating += 5;

				/* Mention the item */
				if (options->cheat_peek || p_ptr->precognition)
				{
					object_mention(o_ptr);
				}
				dragon_resist(o_ptr);
			}
			break;
		}
	}
}



/*
 * Apply magic to an item known to be a "ring" or "amulet"
 *
 * Hack -- note special rating boost for ring of speed
 * Hack -- note special rating boost for amulet of the magi
 * Hack -- note special "pval boost" code for ring of speed
 * Hack -- note that some items must be cursed (or blessed)
 */
static void a_m_aux_3(object_type *o_ptr, int level, int power)
{

	artifact_bias = 0;

	/* Very good */
	if (power > 1)
	{
		/* Make ego item */
		if (rand_int(RANDART_JEWEL) == 1) create_artifact(o_ptr, false, true);
		else make_ego_item(o_ptr, true);
	}
	else if (power < -1)
	{
		/* Make ego item */
		make_ego_item(o_ptr, false);
	}

	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
	case TV_RING:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Strength, Constitution, Dexterity, Intelligence */
			case SV_RING_ATTACKS:
				{
					/* Stat bonus */
					o_ptr->pval = m_bonus(3, level);
					if (o_ptr->pval < 1) o_ptr->pval = 1;

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Critical hits */
			case SV_RING_CRIT:
				{
					/* Stat bonus */
					o_ptr->pval = m_bonus(10, level);
					if (o_ptr->pval < 1) o_ptr->pval = 1;

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}


			case SV_RING_STR:
			case SV_RING_CON:
			case SV_RING_DEX:
			case SV_RING_INT:
				{
					/* Stat bonus */
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Ring of Speed! */
			case SV_RING_SPEED:
				{
					/* Base speed (1 to 10) */
					o_ptr->pval = randint(5) + m_bonus(5, level);

					/* Super-charge the ring */
					while (rand_int(100) < 50) o_ptr->pval++;

					/* Cursed Ring */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse pval */
						o_ptr->pval = 0 - (o_ptr->pval);

						break;
					}

					/* Rating boost */
					rating += 25;

					/* Mention the item */
					if (options->cheat_peek || p_ptr->precognition)
					{
						object_mention(o_ptr);
					}

					break;
				}

			case SV_RING_LORDLY:
				{
					do
					{
					        random_resistance(o_ptr, randint(20) + 18);
					}
					while (randint(4) == 1);

					/* Bonus to armor class */
					o_ptr->to_a = 10 + randint(5) + m_bonus(10, level);
					rating += 5;
				}
				break;

				/* Flames, Acid, Ice */
			case SV_RING_FLAMES:
			case SV_RING_ACID:
			case SV_RING_ICE:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint(5) + m_bonus(10, level);
					break;
				}

				/* Weakness, Stupidity */
			case SV_RING_WEAKNESS:
			case SV_RING_STUPIDITY:
				{
					/* Cursed */
					o_ptr->art_flags |= TR_CURSED;

					/* Penalize */
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* WOE, Stupidity */
			case SV_RING_WOE:
				{
					/* Cursed */
					o_ptr->art_flags |= TR_CURSED;

					/* Penalize */
					o_ptr->to_a = 0 - (5 + m_bonus(10, level));
					o_ptr->pval = 0 - (1 + m_bonus(5, level));

					break;
				}

				/* Ring of damage */
			case SV_RING_DAMAGE:
				{
					/* Bonus to damage */
					o_ptr->to_d = 5 + randint(8) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse bonus */
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

					break;
				}

				/* Ring of Accuracy */
			case SV_RING_ACCURACY:
				{
					/* Bonus to hit */
					o_ptr->to_h = 5 + randint(8) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse tohit */
						o_ptr->to_h = 0 - (o_ptr->to_h);
					}

					break;
				}

				/* Ring of Protection */
			case SV_RING_PROTECTION:
				{
					/* Bonus to armor class */
					o_ptr->to_a = 5 + randint(8) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse toac */
						o_ptr->to_a = 0 - (o_ptr->to_a);
					}

					break;
				}

				/* Ring of Slaying */
			case SV_RING_SLAYING:
				{
					/* Bonus to damage and to hit */
					o_ptr->to_d = randint(7) + m_bonus(10, level);
					o_ptr->to_h = randint(7) + m_bonus(10, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse bonuses */
						o_ptr->to_h = 0 - (o_ptr->to_h);
						o_ptr->to_d = 0 - (o_ptr->to_d);
					}

					break;
				}
			}

			break;
		}

	case TV_AMULET:
		{
			/* Analyze */
			switch (o_ptr->sval)
			{
				/* Amulet of Trickery */
			case SV_AMULET_TRICKERY:
			case SV_AMULET_DEVOTION:
				{
					o_ptr->pval = 1 + m_bonus(3, level);

					/* Mention the item */
					if (options->cheat_peek || p_ptr->precognition)
					{
						object_mention(o_ptr);
					}
					break;
				}

			case SV_AMULET_WEAPONMASTERY:
				{
					o_ptr->pval = 1 + m_bonus(2, level);
					o_ptr->to_a = 1 + m_bonus(4, level);
					o_ptr->to_h = 1 + m_bonus(5, level);
					o_ptr->to_d = 1 + m_bonus(5, level);

					/* Mention the item */
					if (options->cheat_peek || p_ptr->precognition)
					{
						object_mention(o_ptr);
					}
					break;
				}

				/* Amulet of wisdom/charisma */
			case SV_AMULET_BRILLANCE:
			case SV_AMULET_CHARISMA:
			case SV_AMULET_WISDOM:
			case SV_AMULET_INFRA:
				{
					o_ptr->pval = 1 + m_bonus(5, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

				/* Amulet of the Serpents */
			case SV_AMULET_SERPENT:
				{
					o_ptr->pval = 1 + m_bonus(5, level);
					o_ptr->to_a = 1 + m_bonus(6, level);

					/* Cursed */
					if (power < 0)
					{
						/* Cursed */
						o_ptr->art_flags |= TR_CURSED;

						/* Reverse bonuses */
						o_ptr->pval = 0 - (o_ptr->pval);
					}

					break;
				}

			case SV_AMULET_NO_MAGIC:
			case SV_AMULET_NO_TELE:
				{
					if (power < 0)
					{
						o_ptr->art_flags |= TR_CURSED;
					}
					break;
				}

			case SV_AMULET_RESISTANCE:
				{
					if (randint(3) == 1) random_resistance(o_ptr, randint(34) + 4);
					if (randint(5) == 1) o_ptr->art_flags |= TR_RES_POIS;
				}
				break;

				/* Amulet of the Magi -- never cursed */
			case SV_AMULET_THE_MAGI:
				{
					o_ptr->pval = 1 + m_bonus(3, level);

					if (randint(3) == 1) o_ptr->art_flags |= TR_SLOW_DIGEST;

					/* Boost the rating */
					rating += 25;

					/* Mention the item */
					if (options->cheat_peek || p_ptr->precognition)
					{
						object_mention(o_ptr);
					}

					break;
				}

				/* Amulet of Doom -- always cursed */
			case SV_AMULET_DOOM:
				{
					/* Cursed */
					o_ptr->art_flags |= TR_CURSED;

					/* Penalize */
					o_ptr->pval = 0 - (randint(5) + m_bonus(5, level));
					o_ptr->to_a = 0 - (randint(5) + m_bonus(5, level));

					break;
				}
			}

			break;
		}
	}
}

/*
 * Randomized level
 */
static int randomized_level_in_range(range_type *range, int level)
{
	s32b r = range->max - range->min;

	/* The basic idea is to have a max possible level of half the dungeon level */
	if (r * 2 > level)
	{
		r = level / 2;
	}

	/* Randomize a bit */
	r = m_bonus(r, dun_level);

	/* get the result */
	return range->min + r;
}


/*
 * Get a random base level
 */
static int get_stick_base_level(byte tval, int level, int spl)
{
	spell_type *spell = spell_at(spl);
	device_allocation *device_allocation = spell_type_device_allocation(spell, tval);
	assert(device_allocation != NULL);
	return randomized_level_in_range(&device_allocation->base_level, level);
}

/*
 * Get a random max level
 */
static int get_stick_max_level(byte tval, int level, int spl)
{
	spell_type *spell = spell_at(spl);
	device_allocation *device_allocation = spell_type_device_allocation(spell, tval);
	assert(device_allocation != NULL);
	return randomized_level_in_range(&device_allocation->max_level, level);
}


/*
 * Apply magic to an item known to be "boring"
 *
 * Hack -- note the special code for various items
 */
static void a_m_aux_4(object_type *o_ptr, int level, int power)
{
	auto const &r_info = game->edit_data.r_info;

	s32b bonus_lvl, max_lvl;

	/* Very good */
	if (power > 1)
	{
		/* Make ego item */
		if ((rand_int(RANDART_JEWEL) == 1) && (o_ptr->tval == TV_LITE))
		{
			create_artifact(o_ptr, false, true);
		}
		else
		{
			make_ego_item(o_ptr, true);
		}
	}
	else if (power < -1)
	{
		/* Make ego item */
		make_ego_item(o_ptr, false);
	}

	/* Apply magic (good or bad) according to type */
	switch (o_ptr->tval)
	{
	case TV_BOOK:
		{
			/* Randomize random books */
			if (o_ptr->sval == 255)
			{
				int i = 0;

				/* Only random ones */
				if (magik(75))
				{
					i = get_random_spell(SKILL_MAGIC, level);
				}
				else
				{
					i = get_random_spell(SKILL_SPIRITUALITY, level);
				}

				/* Use globe of light(or the first one) */
				if (i == -1)
					o_ptr->pval = 0;
				else
					o_ptr->pval = i;
			}

			break;
		}

	case TV_LITE:
		{
		        auto const flags = object_flags(o_ptr);

			/* Hack -- random fuel */
			if (flags & TR_FUEL_LITE)
			{
				if (o_ptr->k_ptr->pval2 > 0)
				{
					o_ptr->timeout = randint(o_ptr->k_ptr->pval2);
				}
			}

			break;
		}

	case TV_CORPSE:
		{
			/* Hack -- choose a monster */
			int r_idx = get_mon_num(dun_level);
			auto r_ptr = &r_info[r_idx];

			if (!(r_ptr->flags & RF_UNIQUE))
			{
				o_ptr->pval2 = r_idx;
			}
			else
			{
				o_ptr->pval2 = 2;
			}

			o_ptr->pval3 = 0;
			break;
		}

	case TV_EGG:
		{
			/* Hack -- choose a monster */
			int r_idx, count = 0;
			bool OK = false;

			while ((!OK) && (count < 1000))
			{
				r_idx = get_mon_num(dun_level);
				auto r_ptr = &r_info[r_idx];

				if (r_ptr->flags & RF_HAS_EGG)
				{
					o_ptr->pval2 = r_idx;
					OK = true;
				}
				count++;
			}

			if (count == 1000)
			{
				o_ptr->pval2 = 940;  /* Blue fire-lizard */
			}

			auto r_ptr = &r_info[o_ptr->pval2];
			o_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 100) + 1;
			o_ptr->pval = r_ptr->weight * 3 + rand_int(r_ptr->weight) + 1;
			break;
		}

	case TV_HYPNOS:
		{
			/* Hack -- choose a monster */
			int r_idx = get_mon_num(dun_level);
			auto r_ptr = &r_info[r_idx];

			if (!(r_ptr->flags & RF_NEVER_MOVE))
				o_ptr->pval = r_idx;
			else
				o_ptr->pval = 20;

			r_idx = o_ptr->pval;
			r_ptr = &r_info[r_idx];

			o_ptr->pval3 = maxroll(r_ptr->hdice, r_ptr->hside);
			o_ptr->pval2 = o_ptr->pval2;
			o_ptr->exp = 0;
			o_ptr->elevel = r_ptr->level;
			break;
		}

	case TV_WAND:
		{
			auto k_ptr = o_ptr->k_ptr;

			/* Decide the spell, pval == -1 means to bypass spell selection */
			if (o_ptr->pval != -1)
			{
				auto spl = get_random_stick(TV_WAND, dun_level);
				if (spl == -1)
				{
					spl = MANATHRUST;
				}

				o_ptr->pval2 = spl;
			}
			/* Is the spell predefined by the object kind? */
			else if (k_ptr->pval == -1)
			{
				o_ptr->pval2 = k_ptr->pval2;
			}

			/* Ok now get a base level */
			bonus_lvl = get_stick_base_level(TV_WAND, dun_level, o_ptr->pval2);
			max_lvl = get_stick_max_level(TV_WAND, dun_level, o_ptr->pval2);
			o_ptr->pval3 = (max_lvl << 16) + (bonus_lvl & 0xFFFF);

			/* Hack -- charge wands */
			charge_stick(o_ptr);

			break;
		}

	case TV_STAFF:
		{
			auto k_ptr = o_ptr->k_ptr;

			/* Decide the spell, pval == -1 means to bypass spell selection */
			if (o_ptr->pval != -1)
			{
				int spl = get_random_stick(TV_STAFF, dun_level);

				if (spl == -1)
				{
					spl = GLOBELIGHT;
				}

				o_ptr->pval2 = spl;
			}
			/* Is the spell predefined by the object kind? */
			else if (k_ptr->pval == -1)
			{
				o_ptr->pval2 = k_ptr->pval2;
			}

			/* Ok now get a base level */
			bonus_lvl = get_stick_base_level(TV_STAFF, dun_level, o_ptr->pval2);
			max_lvl = get_stick_max_level(TV_STAFF, dun_level, o_ptr->pval2);
			o_ptr->pval3 = (max_lvl << 16) + (bonus_lvl & 0xFFFF);

			/* Hack -- charge staffs */
			charge_stick(o_ptr);

			break;
		}

	case TV_POTION:
		if (o_ptr->sval == SV_POTION_BLOOD)
		{
			/* Rating boost */
			rating += 25;
			/*  Mention the item */
			if (options->cheat_peek || p_ptr->precognition)
			{
				object_mention(o_ptr);
			}
		}
		break;
	case TV_POTION2:
		if (o_ptr->sval == SV_POTION2_MIMIC)
		{
			s32b mimic = find_random_mimic_shape(level, false);
			o_ptr->pval2 = mimic;
		}
		break;
	case TV_INSTRUMENT:
		{
			if (o_ptr->sval != SV_HORN)
			{
				/* Nothing */
			}
			else
			{
				if (is_ego_p(o_ptr, EGO_INST_DRAGONKIND))
				{
					switch (randint(4))
					{
					case 1:
						o_ptr->pval2 = GF_ELEC;
						break;
					case 2:
						o_ptr->pval2 = GF_FIRE;
						break;
					case 3:
						o_ptr->pval2 = GF_COLD;
						break;
					case 4:
						o_ptr->pval2 = GF_ACID;
						break;
					}
				}
			}
			break;
		}

	case TV_TOOL:
		{
			break;
		}

	}
}

/* Add a random glag to the ego item */
void add_random_ego_flag(object_type *o_ptr, ego_flag_set const &fego, bool *limit_blows)
{
	if (fego & ETR_SUSTAIN)
	{
		/* Make a random sustain */
		switch (randint(6))
		{
		case 1:
			o_ptr->art_flags |= TR_SUST_STR;
			break;
		case 2:
			o_ptr->art_flags |= TR_SUST_INT;
			break;
		case 3:
			o_ptr->art_flags |= TR_SUST_WIS;
			break;
		case 4:
			o_ptr->art_flags |= TR_SUST_DEX;
			break;
		case 5:
			o_ptr->art_flags |= TR_SUST_CON;
			break;
		case 6:
			o_ptr->art_flags |= TR_SUST_CHR;
			break;
		}
	}

	if (fego & ETR_OLD_RESIST)
	{
		/* Make a random resist, equal probabilities */
		switch (randint(11))
		{
		case 1:
			o_ptr->art_flags |= TR_RES_BLIND;
			break;
		case 2:
			o_ptr->art_flags |= TR_RES_CONF;
			break;
		case 3:
			o_ptr->art_flags |= TR_RES_SOUND;
			break;
		case 4:
			o_ptr->art_flags |= TR_RES_SHARDS;
			break;
		case 5:
			o_ptr->art_flags |= TR_RES_NETHER;
			break;
		case 6:
			o_ptr->art_flags |= TR_RES_NEXUS;
			break;
		case 7:
			o_ptr->art_flags |= TR_RES_CHAOS;
			break;
		case 8:
			o_ptr->art_flags |= TR_RES_DISEN;
			break;
		case 9:
			o_ptr->art_flags |= TR_RES_POIS;
			break;
		case 10:
			o_ptr->art_flags |= TR_RES_DARK;
			break;
		case 11:
			o_ptr->art_flags |= TR_RES_LITE;
			break;
		}
	}

	if (fego & ETR_ABILITY)
	{
		/* Choose an ability */
		switch (randint(8))
		{
		case 1:
			o_ptr->art_flags |= TR_FEATHER;
			break;
		case 2:
			o_ptr->art_flags |= TR_LITE1;
			break;
		case 3:
			o_ptr->art_flags |= TR_SEE_INVIS;
			break;
		case 4:
			o_ptr->art_flags |= ESP_ALL;
			break;
		case 5:
			o_ptr->art_flags |= TR_SLOW_DIGEST;
			break;
		case 6:
			o_ptr->art_flags |= TR_REGEN;
			break;
		case 7:
			o_ptr->art_flags |= TR_FREE_ACT;
			break;
		case 8:
			o_ptr->art_flags |= TR_HOLD_LIFE;
			break;
		}
	}

	if (fego & ETR_R_ELEM)
	{
		/* Make an acid/elec/fire/cold/poison resist */
		random_resistance(o_ptr, randint(14) + 4);
	}
	if (fego & ETR_R_LOW)
	{
		/* Make an acid/elec/fire/cold resist */
		random_resistance(o_ptr, randint(12) + 4);
	}

	if (fego & ETR_R_HIGH)
	{
		/* Make a high resist */
		random_resistance(o_ptr, randint(22) + 16);
	}
	if (fego & ETR_R_ANY)
	{
		/* Make any resist */
		random_resistance(o_ptr, randint(34) + 4);
	}

	if (fego & ETR_R_DRAGON)
	{
		/* Make "dragon resist" */
		dragon_resist(o_ptr);
	}

	if (fego & ETR_SLAY_WEAP)
	{
		/* Make a Weapon of Slaying */

		if (randint(3) == 1) /* double damage */
			o_ptr->dd *= 2;
		else
		{
			do
			{
				o_ptr->dd++;
			}
			while (randint(o_ptr->dd) == 1);
			do
			{
				o_ptr->ds++;
			}
			while (randint(o_ptr->ds) == 1);
		}
		if (randint(5) == 1)
		{
			o_ptr->art_flags |= TR_BRAND_POIS;
		}
		if (o_ptr->tval == TV_SWORD && (randint(3) == 1))
		{
			o_ptr->art_flags |= TR_VORPAL;
		}
	}

	if (fego & ETR_DAM_DIE)
	{
		/* Increase damage dice */
		o_ptr->dd++;
	}

	if (fego & ETR_DAM_SIZE)
	{
		/* Increase damage dice size */
		o_ptr->ds++;
	}

	if (fego & ETR_LIMIT_BLOWS)
	{
		/* Swap this flag */
		*limit_blows = !(*limit_blows);
	}

	if (fego & ETR_PVAL_M1)
	{
		/* Increase pval */
		o_ptr->pval++;
	}

	if (fego & ETR_PVAL_M2)
	{
		/* Increase pval */
		o_ptr->pval += m_bonus(2, dun_level);
	}

	if (fego & ETR_PVAL_M3)
	{
		/* Increase pval */
		o_ptr->pval += m_bonus(3, dun_level);
	}

	if (fego & ETR_PVAL_M5)
	{
		/* Increase pval */
		o_ptr->pval += m_bonus(5, dun_level);
	}
	if (fego & ETR_AC_M1)
	{
		/* Increase ac */
		o_ptr->to_a++;
	}

	if (fego & ETR_AC_M2)
	{
		/* Increase ac */
		o_ptr->to_a += m_bonus(2, dun_level);
	}

	if (fego & ETR_AC_M3)
	{
		/* Increase ac */
		o_ptr->to_a += m_bonus(3, dun_level);
	}

	if (fego & ETR_AC_M5)
	{
		/* Increase ac */
		o_ptr->to_a += m_bonus(5, dun_level);
	}

	if (fego & ETR_TH_M1)
	{
		/* Increase to hit */
		o_ptr->to_h++;
	}

	if (fego & ETR_TH_M2)
	{
		/* Increase to hit */
		o_ptr->to_h += m_bonus(2, dun_level);
	}

	if (fego & ETR_TH_M3)
	{
		/* Increase to hit */
		o_ptr->to_h += m_bonus(3, dun_level);
	}

	if (fego & ETR_TH_M5)
	{
		/* Increase to hit */
		o_ptr->to_h += m_bonus(5, dun_level);
	}

	if (fego & ETR_TD_M1)
	{
		/* Increase to dam */
		o_ptr->to_d++;
	}

	if (fego & ETR_TD_M2)
	{
		/* Increase to dam */
		o_ptr->to_d += m_bonus(2, dun_level);
	}

	if (fego & ETR_TD_M3)
	{
		/* Increase to dam */
		o_ptr->to_d += m_bonus(3, dun_level);
	}

	if (fego & ETR_TD_M5)
	{
		/* Increase to dam */
		o_ptr->to_d += m_bonus(5, dun_level);
	}

	if (fego & ETR_R_P_ABILITY)
	{
		/* Add a random pval-affected ability */
		/* This might cause boots with + to blows */
		switch (randint(5))
		{
		case 1:
			o_ptr->art_flags |= TR_STEALTH;
			break;
		case 2:
			o_ptr->art_flags |= TR_INFRA;
			break;
		case 3:
			o_ptr->art_flags |= TR_TUNNEL;
			break;
		case 4:
			o_ptr->art_flags |= TR_SPEED;
			break;
		case 5:
			o_ptr->art_flags |= TR_BLOWS;
			break;
		}
	}
	if (fego & ETR_R_STAT)
	{
		/* Add a random stat */
		switch (randint(6))
		{
		case 1:
			o_ptr->art_flags |= TR_STR;
			break;
		case 2:
			o_ptr->art_flags |= TR_INT;
			break;
		case 3:
			o_ptr->art_flags |= TR_WIS;
			break;
		case 4:
			o_ptr->art_flags |= TR_DEX;
			break;
		case 5:
			o_ptr->art_flags |= TR_CON;
			break;
		case 6:
			o_ptr->art_flags |= TR_CHR;
			break;
		}
	}

	if (fego & ETR_R_STAT_SUST)
	{
		/* Add a random stat and sustain it */
		switch (randint(6))
		{
		case 1:
			{
				o_ptr->art_flags |= TR_STR;
				o_ptr->art_flags |= TR_SUST_STR;
				break;
			}

		case 2:
			{
				o_ptr->art_flags |= TR_INT;
				o_ptr->art_flags |= TR_SUST_INT;
				break;
			}

		case 3:
			{
				o_ptr->art_flags |= TR_WIS;
				o_ptr->art_flags |= TR_SUST_WIS;
				break;
			}

		case 4:
			{
				o_ptr->art_flags |= TR_DEX;
				o_ptr->art_flags |= TR_SUST_DEX;
				break;
			}

		case 5:
			{
				o_ptr->art_flags |= TR_CON;
				o_ptr->art_flags |= TR_SUST_CON;
				break;
			}
		case 6:
			{
				o_ptr->art_flags |= TR_CHR;
				o_ptr->art_flags |= TR_SUST_CHR;
				break;
			}
		}
	}

	if (fego & ETR_R_IMMUNITY)
	{
		/* Give a random immunity */
		switch (randint(4))
		{
		case 1:
			{
				o_ptr->art_flags |= TR_IM_FIRE;
				o_ptr->art_flags |= TR_IGNORE_FIRE;
				break;
			}
		case 2:
			{
				o_ptr->art_flags |= TR_IM_ACID;
				o_ptr->art_flags |= TR_IGNORE_ACID;
				break;
			}
		case 3:
			{
				o_ptr->art_flags |= TR_IM_ELEC;
				o_ptr->art_flags |= TR_IGNORE_ELEC;
				break;
			}
		case 4:
			{
				o_ptr->art_flags |= TR_IM_COLD;
				o_ptr->art_flags |= TR_IGNORE_COLD;
				break;
			}
		}
	}
}

/*
 * Complete the "creation" of an object by applying "magic" to the item
 *
 * This includes not only rolling for random bonuses, but also putting the
 * finishing touches on ego-items and artifacts, giving charges to wands and
 * staffs, giving fuel to lites, and placing traps on chests.
 *
 * In particular, note that "Instant Artifacts", if "created" by an external
 * routine, must pass through this function to complete the actual creation.
 *
 * The base "chance" of the item being "good" increases with the "level"
 * parameter, which is usually derived from the dungeon level, being equal
 * to the level plus 10, up to a maximum of 75.  If "good" is true, then
 * the object is guaranteed to be "good".  If an object is "good", then
 * the chance that the object will be "great" (ego-item or artifact), also
 * increases with the "level", being equal to half the level, plus 5, up to
 * a maximum of 20.  If "great" is true, then the object is guaranteed to be
 * "great".  At dungeon level 65 and below, 15/100 objects are "great".
 *
 * If the object is not "good", there is a chance it will be "cursed", and
 * if it is "cursed", there is a chance it will be "broken".  These chances
 * are related to the "good" / "great" chances above.
 *
 * Otherwise "normal" rings and amulets will be "good" half the time and
 * "cursed" half the time, unless the ring/amulet is always good or cursed.
 *
 * If "okay" is true, and the object is going to be "great", then there is
 * a chance that an artifact will be created.  This is true even if both the
 * "good" and "great" arguments are false.  As a total hack, if "great" is
 * true, then the item gets 3 extra "attempts" to become an artifact.
 */
void apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great, boost::optional<int> force_power)
{
	auto &a_info = game->edit_data.a_info;
	auto const &e_info = game->edit_data.e_info;

	int i, rolls;
	auto k_ptr = o_ptr->k_ptr;

	/* Aply luck */
	lev += luck( -7, 7);

	/* Spell in it? No! */
	if (k_ptr->flags & TR_SPELL_CONTAIN)
	{
		o_ptr->pval2 = -1;
	}

	/* Important to do before all else, be sure to have the basic obvious flags set */
	o_ptr->art_oflags = k_ptr->oflags;

	/* No need to touch normal artifacts */
	if (k_ptr->flags & TR_NORM_ART)
	{
		/* Ahah! we tried to trick us !! */
		if (k_ptr->artifact ||
				((k_ptr->flags & TR_SPECIAL_GENE) &&
				 (!k_ptr->allow_special)))
		{
			object_prep(o_ptr, lookup_kind(k_ptr->btval, k_ptr->bsval));
			if (wizard) msg_print("We've been tricked!");
		}
		else
		{
			/* Arg I hate so much to do that ... but I see no other way */
			if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
			{
				s32b base_lvl, max_lvl;

				/* Is the spell predefined by the object kind? */
				if (k_ptr->pval == -1)
				{
					o_ptr->pval2 = k_ptr->pval2;
				}

				/* Determine a base and a max level */
				base_lvl = get_stick_base_level(o_ptr->tval, dun_level, o_ptr->pval2);
				max_lvl = get_stick_max_level(o_ptr->tval, dun_level, o_ptr->pval2);
				o_ptr->pval3 = (max_lvl << 16) + (base_lvl & 0xFFFF);

				/* Hack -- charge wands */
				charge_stick(o_ptr);
			}

			k_ptr->artifact = true;

			if (options->cheat_peek || p_ptr->precognition)
			{
				object_mention(o_ptr);
			}
		}

		return;
	}

	/* Maximum "level" for various things */
	if (lev > MAX_DEPTH - 1) lev = MAX_DEPTH - 1;

	/* Roll for power */
	int power = 0;
	{
		/* Base chance of being "good" */
		int f1 = lev + 10 + luck( -15, 15);

		/* Maximal chance of being "good" */
		if (f1 > 75) f1 = 75;

		/* Base chance of being "great" */
		int f2 = f1 / 2;

		/* Maximal chance of being "great" */
		if (f2 > 20) f2 = 20;

		/* Assume normal */
		power = 0;

		/* Roll for "good" */
		if (good || magik(f1))
		{
			/* Assume "good" */
			power = 1;

			/* Roll for "great" */
			if (great || magik(f2)) power = 2;
		}

		/* Roll for "cursed" */
		else if (magik(f1))
		{
			/* Assume "cursed" */
			power = -1;

			/* Roll for "broken" */
			if (magik(f2)) power = -2;
		}

		/* Override power with parameter? */
		if (auto power_override = force_power)
		{
			power = *power_override;
		}
	}

	/* Assume no rolls */
	rolls = 0;

	/* Get one roll if excellent */
	if (power >= 2) rolls = 1;

	/* Hack -- Get four rolls if forced great */
	if (great) rolls = 4;

	/* Hack -- Get no rolls if not allowed */
	if (!okay || o_ptr->name1) rolls = 0;

	/* Roll for artifacts if allowed */
	for (i = 0; i < rolls; i++)
	{
		/* Roll for an artifact */
		if (make_artifact(o_ptr))
		{
			break;
		}
	}

	/* Mega hack -- to lazy to do it properly with hooks :) */
	if ((o_ptr->name1 == ART_POWER) && (quest[QUEST_ONE].status < QUEST_STATUS_TAKEN))
	{
		o_ptr->name1 = 0;
		o_ptr->name2 = 0;
		o_ptr->name2b = 0;
		object_prep(o_ptr, lookup_kind(TV_RING, SV_RING_INVIS));
	}


	/* Hack -- analyze artifacts */
	if (o_ptr->name1)
	{
		auto a_ptr = &a_info[o_ptr->name1];

		/* Hack -- Mark the artifact as "created" */
		a_ptr->cur_num = 1;

		/* Extract the other fields */
		o_ptr->pval = a_ptr->pval;
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;
		o_ptr->to_a = a_ptr->to_a;
		o_ptr->to_h = a_ptr->to_h;
		o_ptr->to_d = a_ptr->to_d;
		o_ptr->weight = a_ptr->weight;
		o_ptr->number = 1;

		/* Mega-Hack -- increase the rating */
		rating += 10;

		/* Mega-Hack -- increase the rating again */
		if (a_ptr->cost > 50000L) rating += 10;

		/* Set the good item flag */
		good_item_flag = true;

		/* Cheat -- peek at the item */
		if (options->cheat_peek || p_ptr->precognition)
		{
			object_mention(o_ptr);
		}

		/* Spell in it? No! */
		if (a_ptr->flags & TR_SPELL_CONTAIN)
			o_ptr->pval2 = -1;

		/* Give a basic exp/exp level to an artifact that needs it */
		if (a_ptr->flags & TR_LEVELS)
		{
			init_obj_exp(o_ptr, k_ptr);
		}

		/* Done */
		return;
	}


	/* Apply magic */
	switch (o_ptr->tval)
	{
	case TV_RANDART:
		{
			finalize_randart(o_ptr, lev);
			break;
		}
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
	case TV_BOOMERANG:
	case TV_BOW:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		{
			if (power) a_m_aux_1(o_ptr, lev, power);
			break;
		}

	case TV_DAEMON_BOOK:
		{
			/* UGLY, but needed, depending of sval teh demon stuff are eitehr weapon or armor */
			if (o_ptr->sval == SV_DEMONBLADE)
			{
				if (power) a_m_aux_1(o_ptr, lev, power);
			}
			else
			{
				if (power) a_m_aux_2(o_ptr, lev, power);
			}
			break;
		}

	case TV_DRAG_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
	case TV_SHIELD:
	case TV_HELM:
	case TV_CROWN:
	case TV_CLOAK:
	case TV_GLOVES:
	case TV_BOOTS:
		{
			a_m_aux_2(o_ptr, lev, power);
			break;
		}

	case TV_RING:
	case TV_AMULET:
		{
			if (!power && (rand_int(100) < 50)) power = -1;
			a_m_aux_3(o_ptr, lev, power);
			break;
		}

	default:
		{
			a_m_aux_4(o_ptr, lev, power);
			break;
		}
	}

	if (!o_ptr->artifact_name.empty())
	{
		rating += 40;
	}

	/* Hack -- analyze ego-items */
	else if (o_ptr->name2)
	{
		int j;
		bool limit_blows = false;
		s16b e_idx;

		e_idx = o_ptr->name2;

		/* Ok now, THAT is truly ugly */
try_an_other_ego:
		auto e_ptr = &e_info[e_idx];

		/* Hack -- extra powers */
		for (j = 0; j < FLAG_RARITY_MAX; j++)
		{
			/* Rarity check */
			if (magik(e_ptr->rar[j]))
			{
				o_ptr->art_flags |= e_ptr->flags[j];
				o_ptr->art_oflags |= e_ptr->oflags[j];
				add_random_ego_flag(o_ptr, e_ptr->fego[j], &limit_blows);
			}
		}

		/* No insane number of blows */
		if (limit_blows && (o_ptr->art_flags & TR_BLOWS))
		{
			if (o_ptr->pval > 2) o_ptr->pval = randint(2);
		}

		/* get flags */
		auto const flags = object_flags(o_ptr);

		/* Hack -- obtain bonuses */
		if (e_ptr->max_to_h > 0) o_ptr->to_h += randint(e_ptr->max_to_h);
		if (e_ptr->max_to_h < 0) o_ptr->to_h -= randint( -e_ptr->max_to_h);
		if (e_ptr->max_to_d > 0) o_ptr->to_d += randint(e_ptr->max_to_d);
		if (e_ptr->max_to_d < 0) o_ptr->to_d -= randint( -e_ptr->max_to_d);
		if (e_ptr->max_to_a > 0) o_ptr->to_a += randint(e_ptr->max_to_a);
		if (e_ptr->max_to_a < 0) o_ptr->to_a -= randint( -e_ptr->max_to_a);

		/* Hack -- obtain pval */
		if (e_ptr->max_pval > 0) o_ptr->pval += randint(e_ptr->max_pval);
		if (e_ptr->max_pval < 0) o_ptr->pval -= randint( -e_ptr->max_pval);

		/* Hack -- apply rating bonus */
		rating += e_ptr->rating;

		if (o_ptr->name2b && (o_ptr->name2b != e_idx))
		{
			e_idx = o_ptr->name2b;
			goto try_an_other_ego;
		}

		/* Spell in it ? No! */
		if (flags & TR_SPELL_CONTAIN)
		{
			o_ptr->pval2 = -1;
		}

		/* Cheat -- describe the item */
		if (options->cheat_peek || p_ptr->precognition)
		{
			object_mention(o_ptr);
		}
	}


	/* Examine real objects */
	if (o_ptr->k_ptr)
	{
		auto k_ptr = o_ptr->k_ptr;

		/* Hack -- acquire "cursed" flag */
		if (k_ptr->flags & TR_CURSED)
		{
			o_ptr->art_flags |= TR_CURSED;
		}

		/* Extract some flags */
		auto const flags = object_flags(o_ptr);

		/* Hack give a basic exp/exp level to an object that needs it */
		if (flags & TR_LEVELS)
		{
			init_obj_exp(o_ptr, k_ptr);
		}

		/* Spell in it ? No! */
		if (flags & TR_SPELL_CONTAIN)
		{
			o_ptr->pval2 = -1;
		}

		/* Hacccccccckkkkk attack ! :) -- To prevent som ugly crashs */
		if ((o_ptr->tval == TV_MSTAFF) && (o_ptr->sval == SV_MSTAFF) && (o_ptr->pval < 0))
		{
			o_ptr->pval = 0;
		}

		/* Hack, cant be done in a_m_aux4 because the ego flags are not yet in place */
		if (o_ptr->tval == TV_ROD_MAIN)
		{
			/* Set the max mana and the current mana */
			o_ptr->pval2 = (flags & TR_CAPACITY) ? o_ptr->sval * 2 : o_ptr->sval;

			o_ptr->timeout = o_ptr->pval2;
		}
	}
}


/* The themed objects to use */
static obj_theme *match_theme = nullptr;

/*
 * XXX XXX XXX It relies on the fact that obj_theme is a four byte structure
 * for its efficient operation. A horrendous hack, I'd say.
 */
bool init_match_theme(obj_theme const &theme)
{
	if (match_theme == nullptr)
	{
		match_theme = new obj_theme(theme);
		return true;
	}
	else if (*match_theme != theme)
	{
		*match_theme = theme;
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Maga-Hack -- match certain types of object only.
 */
static bool kind_is_theme(obj_theme const *theme, object_kind const *k_ptr)
{
	assert(theme != nullptr);

	s32b prob = 0;

	/*
	 * Paranoia -- Prevent accidental "(Nothing)"
	 * that are results of uninitialised theme structs.
	 *
	 * Caution: Junks go into the allocation table.
	 */
	if (theme->treasure + theme->combat + theme->magic + theme->tools == 0)
	{
		return true;
	}


	/* Pick probability to use */
	switch (k_ptr->tval)
	{
	case TV_SKELETON:
	case TV_BOTTLE:
	case TV_JUNK:
	case TV_CORPSE:
	case TV_EGG:
		{
			/*
			 * Degree of junk is defined in terms of the other
			 * 4 quantities XXX XXX XXX
			 * The type of prob should be *signed* as well as
			 * larger than theme components, or we would see
			 * unexpected, well, junks.
			 */
			prob = 100 - (theme->treasure + theme->combat +
				      theme->magic + theme->tools);
			break;
		}
	case TV_CROWN:
		prob = theme->treasure;
		break;
	case TV_DRAG_ARMOR:
		prob = theme->treasure;
		break;
	case TV_AMULET:
		prob = theme->treasure;
		break;
	case TV_RING:
		prob = theme->treasure;
		break;

	case TV_SHOT:
		prob = theme->combat;
		break;
	case TV_ARROW:
		prob = theme->combat;
		break;
	case TV_BOLT:
		prob = theme->combat;
		break;
	case TV_BOOMERANG:
		prob = theme->combat;
		break;
	case TV_BOW:
		prob = theme->combat;
		break;
	case TV_HAFTED:
		prob = theme->combat;
		break;
	case TV_POLEARM:
		prob = theme->combat;
		break;
	case TV_SWORD:
		prob = theme->combat;
		break;
	case TV_AXE:
		prob = theme->combat;
		break;
	case TV_GLOVES:
		prob = theme->combat;
		break;
	case TV_HELM:
		prob = theme->combat;
		break;
	case TV_SHIELD:
		prob = theme->combat;
		break;
	case TV_SOFT_ARMOR:
		prob = theme->combat;
		break;
	case TV_HARD_ARMOR:
		prob = theme->combat;
		break;

	case TV_MSTAFF:
		prob = theme->magic;
		break;
	case TV_STAFF:
		prob = theme->magic;
		break;
	case TV_WAND:
		prob = theme->magic;
		break;
	case TV_ROD:
		prob = theme->magic;
		break;
	case TV_ROD_MAIN:
		prob = theme->magic;
		break;
	case TV_SCROLL:
		prob = theme->magic;
		break;
	case TV_PARCHMENT:
		prob = theme->magic;
		break;
	case TV_POTION:
		prob = theme->magic;
		break;
	case TV_POTION2:
		prob = theme->magic;
		break;
	case TV_RANDART:
		prob = theme->magic;
		break;
	case TV_BOOK:
		prob = theme->magic;
		break;
	case TV_SYMBIOTIC_BOOK:
		prob = theme->magic;
		break;
	case TV_MUSIC_BOOK:
		prob = theme->magic;
		break;
	case TV_DRUID_BOOK:
		prob = theme->magic;
		break;
	case TV_DAEMON_BOOK:
		prob = theme->magic;
		break;

	case TV_LITE:
		prob = theme->tools;
		break;
	case TV_CLOAK:
		prob = theme->tools;
		break;
	case TV_BOOTS:
		prob = theme->tools;
		break;
	case TV_SPIKE:
		prob = theme->tools;
		break;
	case TV_DIGGING:
		prob = theme->tools;
		break;
	case TV_FLASK:
		prob = theme->tools;
		break;
	case TV_FOOD:
		prob = theme->tools;
		break;
	case TV_TOOL:
		prob = theme->tools;
		break;
	case TV_INSTRUMENT:
		prob = theme->tools;
		break;
	}

	/* Roll to see if it can be made */
	if (rand_int(100) < prob)
	{
		return true;
	}

	/* Not a match */
	return false;
}

/*
 * Determine if an object must not be generated.
 */
bool kind_is_legal(object_kind const *k_ptr)
{
	if (!kind_is_theme(match_theme, k_ptr))
	{
		return false;
	}

	if (k_ptr->flags & TR_SPECIAL_GENE)
	{
		return k_ptr->allow_special;
	}

	/* No 2 times the same normal artifact */
	if ((k_ptr->flags & TR_NORM_ART) && (k_ptr->artifact))
	{
		return false;
	}

	if (k_ptr->tval == TV_CORPSE)
	{
		return (k_ptr->sval != SV_CORPSE_SKULL && k_ptr->sval != SV_CORPSE_SKELETON &&
			k_ptr->sval != SV_CORPSE_HEAD && k_ptr->sval != SV_CORPSE_CORPSE);
	}

	if (k_ptr->tval == TV_HYPNOS)
	{
		return false;
	}

	/* Used only for the Nazgul rings */
	if ((k_ptr->tval == TV_RING) && (k_ptr->sval == SV_RING_SPECIAL))
	{
		return false;
	}

	/* Assume legal */
	return true;
}


/*
 * Hack -- determine if a template is "good"
 */
static bool kind_is_good(object_kind const *k_ptr)
{
	if (!kind_is_legal(k_ptr))
	{
		return false;
	}

	/* Analyze the item type */
	switch (k_ptr->tval)
	{
		/* Armor -- Good unless damaged */
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
	case TV_DRAG_ARMOR:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
		{
			return k_ptr->to_a >= 0;
		}

		/* Weapons -- Good unless damaged */
	case TV_BOW:
	case TV_SWORD:
	case TV_AXE:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_DIGGING:
	case TV_MSTAFF:
	case TV_BOOMERANG:
		{
			if (k_ptr->to_h < 0) return false;
			if (k_ptr->to_d < 0) return false;
			return true;
		}

		/* Ammo -- Arrows/Bolts are good */
	case TV_BOLT:
	case TV_ARROW:
		{
			return true;
		}

		/* Rods - Silver and better are good */
	case TV_ROD_MAIN:
		{
			return (k_ptr->sval >= SV_ROD_SILVER);
		}

		/* Expensive rod tips are good */
	case TV_ROD:
		{
			return (k_ptr->cost >= 4500);
		}

		/* The Tomes are good */
	case TV_BOOK:
		{
			return (k_ptr->sval <= SV_BOOK_MAX_GOOD);
		}

		/* Rings -- Rings of Speed are good */
	case TV_RING:
		{
			return (k_ptr->sval == SV_RING_SPEED);
		}

		/* Amulets -- Some are good */
	case TV_AMULET:
		{
			if (k_ptr->sval == SV_AMULET_THE_MAGI) return true;
			if (k_ptr->sval == SV_AMULET_DEVOTION) return true;
			if (k_ptr->sval == SV_AMULET_WEAPONMASTERY) return true;
			if (k_ptr->sval == SV_AMULET_TRICKERY) return true;
			if (k_ptr->sval == SV_AMULET_RESISTANCE) return true;
			if (k_ptr->sval == SV_AMULET_REFLECTION) return true;
			if (k_ptr->sval == SV_AMULET_TELEPATHY) return true;
			return false;
		}
	}

	/* Assume not good */
	return false;
}

/*
* Determine if template is suitable for building a randart -- dsb
*/
bool kind_is_artifactable(object_kind const *k_ptr)
{
	auto const &ra_info = game->edit_data.ra_info;

	if (kind_is_good(k_ptr))
	{
		// Consider the item artifactable if there is at least one
		// randart power which could be added to the item.
		for (auto const &ra_ref: ra_info)
		{
			for (auto const &filter: ra_ref.kind_filter)
			{
				if (filter.tval != k_ptr->tval) continue;
				if (filter.min_sval > k_ptr->sval) continue;
				if (filter.max_sval < k_ptr->sval) continue;
				return true;
			}
		}
	}

	/* No match. Too bad. */
	return false;
}


/*
 * Attempt to make an object (normal or good/great)
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * We assume that the given object has been "wiped".
 *
 * To Watch: The allocation table caching is heavily relies on
 * an assumption that the SPECIAL_GENE objects should only be created
 * through the forge--object_prep()--apply_magic() sequence and
 * get_obj_num() should never be called for that purpose XXX XXX XXX
 */
bool make_object(object_type *j_ptr, bool good, bool great, obj_theme const &theme)
{
	auto &alloc = game->alloc;

	int invprob, base;

	/* Chance of "special object" */
	invprob = (good ? 10 - luck( -9, 9) : 1000);

	/* Base level for the object */
	base = (good ? (object_level + 10) : object_level);


	/* Generate a special object, or a normal object */
	if ((rand_int(invprob) != 0) || !make_artifact_special(j_ptr))
	{
		/* See if the theme has been changed */
		if (init_match_theme(theme))
		{
			/* Invalidate the cached allocation table */
			alloc.kind_table_valid = false;
		}

		/* Good objects */
		if (good)
		{
			/* Activate restriction */
			get_object_hook = kind_is_good;

			/* Prepare allocation table */
			get_obj_num_prep();
		}

		/* Normal objects -- only when the cache is invalidated */
		else if (!alloc.kind_table_valid)
		{
			/* Activate normal restriction */
			get_object_hook = kind_is_legal;

			/* Prepare allocation table */
			get_obj_num_prep();

			/* The table is synchronised */
			alloc.kind_table_valid = true;
		}

		/* Pick a random object */
		int k_idx = get_obj_num(base);

		/* Good objects */
		if (good)
		{
			/* Restore normal restriction */
			get_object_hook = kind_is_legal;

			/* Prepare allocation table */
			get_obj_num_prep();

			/* The table is synchronised */
			alloc.kind_table_valid = true;
		}

		/* Handle failure */
		if (!k_idx) return false;

		/* Prepare the object */
		object_prep(j_ptr, k_idx);
	}

	/* Apply magic (allow artifacts) */
	apply_magic(j_ptr, object_level, true, good, great);

	/* Hack -- generate multiple spikes/missiles */
	switch (j_ptr->tval)
	{
	case TV_SPIKE:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		{
			j_ptr->number = (byte)damroll(6, 7);
		}
	}

	/* hack, no multiple artifacts */
	if (artifact_p(j_ptr))
	{
		j_ptr->number = 1;
	}

	/* Notice "okay" out-of-depth objects */
	auto j_level = j_ptr->k_ptr->level;
	if (!cursed_p(j_ptr) && (j_level > dun_level))
	{
		/* Rating increase */
		rating += (j_level - dun_level);

		/* Cheat -- peek at items */
		if (options->cheat_peek || p_ptr->precognition)
		{
			object_mention(j_ptr);
		}
	}

	/* Success */
	return true;
}



/*
 * Attempt to place an object (normal or good/great) at the given location.
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * This routine requires a clean floor grid destination.
 */
void place_object(int y, int x, bool good, bool great, int where)
{
	auto const &d_info = game->edit_data.d_info;
	auto &a_info = game->edit_data.a_info;
	auto &random_artifacts = game->random_artifacts;

	s16b o_idx;

	cave_type *c_ptr;

	object_type forge;
	object_type *q_ptr;


	/* Paranoia -- check bounds */
	if (!in_bounds(y, x)) return;

	/* Require clean floor space */
	if (!cave_clean_bold(y, x)) return;


	/* Get local object */
	q_ptr = &forge;

	/* Wipe the object */
	object_wipe(q_ptr);

	/* Make an object (if possible) */
	if (!make_object(q_ptr, good, great, d_info[dungeon_type].objs)) return;

	if (where == OBJ_FOUND_VAULT)
	{
		q_ptr->found = OBJ_FOUND_VAULT;
		q_ptr->found_aux1 = dungeon_type;
		q_ptr->found_aux2 = level_or_feat(dungeon_type, dun_level);
	}
	else if (where == OBJ_FOUND_FLOOR)
	{
		q_ptr->found = OBJ_FOUND_FLOOR;
		q_ptr->found_aux1 = dungeon_type;
		q_ptr->found_aux2 = level_or_feat(dungeon_type, dun_level);
	}
	else if (where == OBJ_FOUND_SPECIAL)
	{
		q_ptr->found = OBJ_FOUND_SPECIAL;
	}
	else if (where == OBJ_FOUND_RUBBLE)
	{
		q_ptr->found = OBJ_FOUND_RUBBLE;
	}

	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[o_idx];

		/* Structure Copy */
		object_copy(o_ptr, q_ptr);

		/* Location */
		o_ptr->iy = y;
		o_ptr->ix = x;

		/* Acquire grid */
		c_ptr = &cave[y][x];

		/* Place the object */
		c_ptr->o_idxs.push_back(o_idx);

		/* Notice */
		note_spot(y, x);

		/* Redraw */
		lite_spot(y, x);
	}

	/* Object array overflow */
	else
	{
		/* Hack -- Preserve artifacts */
		if (q_ptr->name1)
		{
			a_info[q_ptr->name1].cur_num = 0;
		}
		else if (q_ptr->k_ptr->flags & TR_NORM_ART)
		{
			q_ptr->k_ptr->artifact = false;
		}
		else if (q_ptr->tval == TV_RANDART)
		{
			random_artifacts[q_ptr->sval].generated = false;
		}
	}
}


/*
 * XXX XXX XXX Do not use these hard-coded values.
 */
#define OBJ_GOLD_LIST   480     /* First "gold" entry */
#define MAX_GOLD        18      /* Number of "gold" entries */

/*
 * Make a treasure object
 *
 * The location must be a legal, clean, floor grid.
 */
bool make_gold(object_type *j_ptr)
{
	auto const &k_info = game->edit_data.k_info;

	/* Hack -- Pick a Treasure variety */
	int i = ((randint(object_level + 2) + 2) / 2) - 1;

	/* Apply "extra" magic */
	if (rand_int(GREAT_OBJ) == 0)
	{
		i += randint(object_level + 1);
	}

	/* Hack -- Creeping Coins only generate "themselves" */
	if (coin_type) i = coin_type;

	/* Do not create "illegal" Treasure Types */
	if (i >= MAX_GOLD) i = MAX_GOLD - 1;

	/* Prepare a gold object */
	object_prep(j_ptr, OBJ_GOLD_LIST + i);

	/* Hack -- Base coin cost */
	s32b const base = k_info.at(OBJ_GOLD_LIST + i)->cost;

	/* Determine how much the treasure is "worth" */
	j_ptr->pval = (base + (8L * randint(base)) + randint(8));
	
	/* Multiply value by 5 if selling is disabled */
	if (options->no_selling)
	{
		j_ptr->pval *= 5;
	}

	/* Success */
	return true;
}


/*
 * Places a treasure (Gold or Gems) at given location
 *
 * The location must be a legal, clean, floor grid.
 */
void place_gold(int y, int x)
{
	s16b o_idx;

	cave_type *c_ptr;

	object_type forge;
	object_type *q_ptr;


	/* Paranoia -- check bounds */
	if (!in_bounds(y, x)) return;

	/* Require clean floor space */
	if (!cave_clean_bold(y, x)) return;


	/* Get local object */
	q_ptr = &forge;

	/* Wipe the object */
	object_wipe(q_ptr);

	/* Make some gold */
	if (!make_gold(q_ptr)) return;


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		object_type *o_ptr;

		/* Acquire object */
		o_ptr = &o_list[o_idx];

		/* Copy the object */
		object_copy(o_ptr, q_ptr);

		/* Save location */
		o_ptr->iy = y;
		o_ptr->ix = x;

		/* Acquire grid */
		c_ptr = &cave[y][x];

		/* Place the object */
		c_ptr->o_idxs.push_back(o_idx);

		/* Notice */
		note_spot(y, x);

		/* Redraw */
		lite_spot(y, x);
	}
}


/*
 * Let an object fall to the ground at or near a location.
 *
 * The initial location is assumed to be "in_bounds()".
 *
 * This function takes a parameter "chance".  This is the percentage
 * chance that the item will "disappear" instead of drop.  If the object
 * has been thrown, then this is the chance of disappearance on contact.
 *
 * Hack -- this function uses "chance" to determine if it should produce
 * some form of "description" of the drop event (under the player).
 *
 * We check several locations to see if we can find a location at which
 * the object can combine, stack, or be placed.  Artifacts will try very
 * hard to be placed, including "teleporting" to a useful grid if needed.
 */
s16b drop_near(object_type *j_ptr, int chance, int y, int x)
{
	auto const &f_info = game->edit_data.f_info;
	auto &a_info = game->edit_data.a_info;
	auto &random_artifacts = game->random_artifacts;

	int i, k, d, s;

	int bs, bn;
	int by, bx;
	int dy, dx;
	int ty, tx;

	cave_type *c_ptr;

	char o_name[80];

	bool flag = false;
	bool done = false;

	bool plural = false;


	/* Extract plural */
	if (j_ptr->number != 1) plural = true;

	/* Describe object */
	object_desc(o_name, j_ptr, false, 0);


	/* Handle normal "breakage" */
	if ((!artifact_p(j_ptr)) && (rand_int(100) < chance))
	{
		/* Message */
		msg_format("The %s disappear%s.",
		           o_name, (plural ? "" : "s"));

		/* Debug */
		if (wizard) msg_print("(breakage)");

		/* Failure */
		return (0);
	}


	/* Score */
	bs = -1;

	/* Picker */
	bn = 0;

	/* Default */
	by = y;
	bx = x;

	/* Scan local grids */
	for (dy = -3; dy <= 3; dy++)
	{
		/* Scan local grids */
		for (dx = -3; dx <= 3; dx++)
		{
			bool comb = false;

			/* Calculate actual distance */
			d = (dy * dy) + (dx * dx);

			/* Ignore distant grids */
			if (d > 10) continue;

			/* Location */
			ty = y + dy;
			tx = x + dx;

			/* Skip illegal grids */
			if (!in_bounds(ty, tx)) continue;

			/* Require line of sight */
			if (!los(y, x, ty, tx)) continue;

			/* Obtain grid */
			c_ptr = &cave[ty][tx];

			/* Require floor space (or shallow terrain) -KMW- */
			if (!(f_info[c_ptr->feat].flags & FF_FLOOR)) continue;

			/* No objects */
			k = 0;

			/* Scan objects in that grid */
			for (auto const this_o_idx: c_ptr->o_idxs)
			{
				/* Acquire object */
				object_type *o_ptr = &o_list[this_o_idx];

				/* Check for possible combination */
				if (object_similar(o_ptr, j_ptr)) comb = true;

				/* Count objects */
				k++;
			}

			/* Add new object */
			if (!comb) k++;

			/* Paranoia */
			if (k > 23) continue;

			/* Calculate score */
			s = 1000 - (d + k * 5);

			/* Skip bad values */
			if (s < bs) continue;

			/* New best value */
			if (s > bs) bn = 0;

			/* Apply the randomizer to equivalent values */
			if ((++bn >= 2) && (rand_int(bn) != 0)) continue;

			/* Keep score */
			bs = s;

			/* Track it */
			by = ty;
			bx = tx;

			/* Okay */
			flag = true;
		}
	}


	/* Handle lack of space */
	if (!flag && (!artifact_p(j_ptr)))
	{
		/* Message */
		msg_format("The %s disappear%s.",
		           o_name, (plural ? "" : "s"));

		/* Debug */
		if (wizard) msg_print("(no floor space)");

		/* Failure */
		return (0);
	}


	/* Find a grid */
	for (i = 0; !flag; i++)
	{
		/* Bounce around */
		if (i < 1000)
		{
			ty = rand_spread(by, 1);
			tx = rand_spread(bx, 1);
		}

		/* Random locations */
		else
		{
			ty = rand_int(cur_hgt);
			tx = rand_int(cur_wid);
		}

		/* Grid */
		c_ptr = &cave[ty][tx];

		/* Require floor space (or shallow terrain) -KMW- */
		if ((c_ptr->feat != FEAT_FLOOR) &&
		                (c_ptr->feat != FEAT_SHAL_WATER) &&
		                (c_ptr->feat != FEAT_GRASS) &&
		                (c_ptr->feat != FEAT_DIRT) &&
		                (c_ptr->feat != FEAT_SHAL_LAVA)) continue;

		/* Bounce to that location */
		by = ty;
		bx = tx;

		/* Require floor space */
		if (!cave_clean_bold(by, bx)) continue;

		/* Okay */
		flag = true;
	}


	/* Grid */
	c_ptr = &cave[by][bx];

	/* Scan objects in that grid for combination */
	for (auto const this_o_idx: c_ptr->o_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr))
		{
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Success */
			done = true;

			/* Done */
			break;
		}
	}

	/* Get new object */
	s16b o_idx = 0;
	if (!done) o_idx = o_pop();

	/* Failure */
	if (!done && !o_idx)
	{
		/* Message */
		msg_format("The %s disappear%s.",
		           o_name, (plural ? "" : "s"));

		/* Debug */
		if (wizard) msg_print("(too many objects)");

		/* Hack -- Preserve artifacts */
		if (j_ptr->name1)
		{
			a_info[j_ptr->name1].cur_num = 0;
		}
		else if (j_ptr->k_ptr->flags & TR_NORM_ART)
		{
			j_ptr->k_ptr->artifact = false;
		}
		else if (j_ptr->tval == TV_RANDART)
		{
			random_artifacts[j_ptr->sval].generated = false;
		}

		/* Failure */
		return (0);
	}

	/* Stack */
	if (!done)
	{
		/* Structure copy */
		object_copy(&o_list[o_idx], j_ptr);

		/* Access new object */
		j_ptr = &o_list[o_idx];

		/* Locate */
		j_ptr->iy = by;
		j_ptr->ix = bx;

		/* No monster */
		j_ptr->held_m_idx = 0;

		/* Place the object */
		c_ptr->o_idxs.push_back(o_idx);

		/* Success */
		done = true;
	}

	/* Note the spot */
	note_spot(by, bx);

	/* Draw the spot */
	lite_spot(by, bx);

	/* Mega-Hack -- no message if "dropped" by player */
	/* Message when an object falls under the player */
	if (chance && (by == p_ptr->py) && (bx == p_ptr->px))
	{
		msg_print("You feel something roll beneath your feet.");
	}

	/* XXX XXX XXX */

	/* Result */
	return (o_idx);
}




/*
 * Scatter some "great" objects near the player
 */
void acquirement(int y1, int x1, int num, bool great)
{
	auto const &d_info = game->edit_data.d_info;

	object_type *i_ptr;
	object_type object_type_body;

	/* Acquirement */
	while (num--)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Make a good (or great) object (if possible) */
		if (!make_object(i_ptr, true, great, d_info[dungeon_type].objs)) continue;

		/* Drop the object */
		drop_near(i_ptr, -1, y1, x1);
	}
}


/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	if (!object_known_p(o_ptr)) return;

	/* Multiple charges */
	if (o_ptr->pval != 1)
	{
		/* Print a message */
		msg_format("You have %d charges remaining.", o_ptr->pval);
	}

	/* Single charge */
	else
	{
		/* Print a message */
		msg_format("You have %d charge remaining.", o_ptr->pval);
	}
}


/*
 * Describe an item in the inventory.
 */
void inven_item_describe(int item)
{
	object_type *o_ptr = &p_ptr->inventory[item];
	char o_name[80];

	/* Get a description */
	object_desc(o_name, o_ptr, true, 3);

	/* Print a message */
	msg_format("You have %s.", o_name);
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(int item, int num)
{
	object_type *o_ptr = &p_ptr->inventory[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number and weight */
	if (num)
	{
		/* Add the number */
		o_ptr->number += num;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate mana XXX */
		p_ptr->update |= (PU_MANA);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}


/*
 * Erase an inventory slot if it has no more items
 */
void inven_item_optimize(int item)
{
	auto const &a_info = game->edit_data.a_info;

	object_type *o_ptr = &p_ptr->inventory[item];

	/* Only optimize real items */
	if (!o_ptr->k_ptr)
	{
		return;
	}

	/* Only optimize empty items */
	if (o_ptr->number)
	{
		return;
	}

	/* The item is in the pack */
	if (item < INVEN_WIELD)
	{
		int i;

		/* One less item */
		inven_cnt--;

		/* Slide everything down */
		for (i = item; i < INVEN_PACK; i++)
		{
			/* Structure copy */
			p_ptr->inventory[i] = p_ptr->inventory[i + 1];
		}

		/* Erase the "final" slot */
		object_wipe(&p_ptr->inventory[i]);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/* The item is being wielded */
	else
	{
		/* One less item */
		equip_cnt--;

		/* Take care of item sets*/
		if (o_ptr->name1)
		{
			takeoff_set(p_ptr->inventory[item].name1, a_info[p_ptr->inventory[item].name1].set);
		}

		/* Erase the empty slot */
		object_wipe(&p_ptr->inventory[item]);

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Recalculate torch */
		p_ptr->update |= (PU_TORCH);

		/* Recalculate mana XXX */
		p_ptr->update |= (PU_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);
	}
}


/*
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
	object_type *o_ptr = &o_list[item];

	/* Require staff/wand */
	if ((o_ptr->tval != TV_STAFF) && (o_ptr->tval != TV_WAND)) return;

	/* Require known item */
	if (!object_known_p(o_ptr)) return;

	/* Multiple charges */
	if (o_ptr->pval != 1)
	{
		/* Print a message */
		msg_format("There are %d charges remaining.", o_ptr->pval);
	}

	/* Single charge */
	else
	{
		/* Print a message */
		msg_format("There is %d charge remaining.", o_ptr->pval);
	}
}



/*
 * Describe an item in the inventory.
 */
void floor_item_describe(int item)
{
	object_type *o_ptr = &o_list[item];
	char o_name[80];

	/* Get a description */
	object_desc(o_name, o_ptr, true, 3);

	/* Print a message */
	msg_format("You see %s.", o_name);
}


/*
 * Increase the "number" of an item on the floor
 */
void floor_item_increase(int item, int num)
{
	object_type *o_ptr = &o_list[item];

	/* Apply */
	num += o_ptr->number;

	/* Bounds check */
	if (num > 255) num = 255;
	else if (num < 0) num = 0;

	/* Un-apply */
	num -= o_ptr->number;

	/* Change the number */
	o_ptr->number += num;
}


/*
 * Optimize an item on the floor (destroy "empty" items)
 */
void floor_item_optimize(int item)
{
	object_type *o_ptr = &o_list[item];

	/* Paranoia -- be sure it exists */
	if (!o_ptr->k_ptr) return;

	/* Only optimize empty items */
	if (o_ptr->number) return;

	/* Delete the object */
	delete_object_idx(item);
}


/*
 * Increase stack size for item, describe and optimize.
 */
void inc_stack_size(int item, int delta) {
	inc_stack_size_ex(item, delta, OPTIMIZE, DESCRIBE);
}

/*
 * Increase stack size for item.
 */
void inc_stack_size_ex(int item, int delta, optimize_flag opt, describe_flag desc) {
	/* Pack item? */
	if (item >= 0)
	{
		inven_item_increase(item, delta);
		if (desc == DESCRIBE)
		{
			inven_item_describe(item);
		}
		if (opt == OPTIMIZE)
		{
			inven_item_optimize(item);
		}
	}

	/* Floor item? */
	else
	{
		floor_item_increase(0 - item, delta);
		if (desc == DESCRIBE)
		{
			floor_item_describe(0 - item);
		}
		if (opt == OPTIMIZE)
		{
			floor_item_optimize(0 - item);
		}
	}
}



/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(object_type const *o_ptr)
{
	if (o_ptr->tval == TV_GOLD) return false;

	/* Empty slot? */
	if (inven_cnt < INVEN_PACK) return true;

	/* Similar slot? */
	for (int j = 0; j < INVEN_PACK; j++)
	{
		object_type *j_ptr = &p_ptr->inventory[j];

		/* Skip non-objects */
		if (!j_ptr->k_ptr) continue;

		/* Check if the two items can be combined */
		if (object_similar(j_ptr, o_ptr)) return true;
	}

	/* Nope */
	return false;
}

/**
 * Find an empty slot in the player's inventory.
 */
static boost::optional<int> find_empty_slot()
{
	for (int i = 0; i < INVEN_PACK; i++)
	{
		auto o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_ptr)
		{
			return i;
		}
	}

	return boost::none;
}

/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using "object_similar()" and "object_absorb()", otherwise,
 * the item will be placed into the "proper" location in the inventory.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 *
 * Note that this code must remove any location/stack information
 * from the object once it is placed into the inventory.
 *
 * The "final" flag tells this function to bypass the "combine"
 * and "reorder" code until later.
 */
s16b inven_carry(object_type *o_ptr, bool final)
{
	// Auto-identify the item before we put it in the pack.
	object_aware(o_ptr);
	object_known(o_ptr);

	// Index of last item
	int n = -1;

	/* Not final */
	if (!final)
	{
		/* Check for combining */
		for (int j = 0; j < INVEN_PACK; j++)
		{
			auto j_ptr = &p_ptr->inventory[j];

			/* Skip non-objects */
			if (!j_ptr->k_ptr)
			{
				continue;
			}

			/* Hack -- track last item */
			n = j;

			/* Check if the two items can be combined */
			if (object_similar(j_ptr, o_ptr))
			{
				/* Combine the items */
				object_absorb(j_ptr, o_ptr);

				/* Recalculate bonuses */
				p_ptr->update |= (PU_BONUS);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN);

				/* Success */
				return (j);
			}
		}
	}

	/* Paranoia */
	if (inven_cnt > INVEN_PACK)
	{
		return ( -1);
	}

	/* Find an empty slot */
	auto i = find_empty_slot()
		.get_value_or(INVEN_PACK);

	/* Hack -- pre-reorder the pack */
	if (!final && (i < INVEN_PACK))
	{
		/* Get the "value" of the item */
		s32b o_value = object_value(o_ptr);

		/* Scan every occupied slot */
		int j;
		for (j = 0; j < INVEN_PACK; j++)
		{
			auto j_ptr = &p_ptr->inventory[j];

			/* Use empty slots */
			if (!j_ptr->k_ptr)
			{
				break;
			}

			/* Objects sort by decreasing type */
			if (o_ptr->tval > j_ptr->tval) break;
			if (o_ptr->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_aware_p(o_ptr)) continue;
			if (!object_aware_p(j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o_ptr->sval < j_ptr->sval) break;
			if (o_ptr->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_known_p(o_ptr)) continue;
			if (!object_known_p(j_ptr)) break;

			/* Hack:  otherwise identical rods sort by
			increasing recharge time --dsb */
			if (o_ptr->tval == TV_ROD_MAIN)
			{
				if (o_ptr->timeout > j_ptr->timeout) break;
				if (o_ptr->timeout < j_ptr->timeout) continue;
			}

			/* Determine the "value" of the pack item */
			s32b j_value = object_value(j_ptr);

			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Use that slot */
		i = j;

		/* Slide objects */
		for (int k = n; k >= i; k--)
		{
			/* Hack -- Slide the item */
			object_copy(&p_ptr->inventory[k + 1], &p_ptr->inventory[k]);
		}

		/* Wipe the empty slot */
		object_wipe(&p_ptr->inventory[i]);
	}


	/* Acquire a copy of the item */
	object_copy(&p_ptr->inventory[i], o_ptr);

	/* Access new object */
	o_ptr = &p_ptr->inventory[i];

	/* Clean out unused fields */
	o_ptr->iy = o_ptr->ix = 0;
	o_ptr->held_m_idx = 0;

	/* Count the items */
	inven_cnt++;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine and Reorder pack */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN);

	/* Return the slot */
	return (i);
}



/*
 * Take off (some of) a non-cursed equipment item
 *
 * Note that only one item at a time can be wielded per slot.
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 *
 * Return the inventory slot into which the item is placed.
 */
s16b inven_takeoff(int item, int amt, bool force_drop)
{
	int slot;

	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr;

	const char *act;

	char o_name[80];


	/* Get the item to take off */
	o_ptr = &p_ptr->inventory[item];

	/* Paranoia */
	if (amt <= 0) return ( -1);

	/* Verify */
	if (amt > o_ptr->number) amt = o_ptr->number;

	/* Get local object */
	q_ptr = &forge;

	/* Obtain a local object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = amt;

	/* Describe the object */
	object_desc(o_name, q_ptr, true, 3);

	/* Took off weapon */
	if (item == INVEN_WIELD)
	{
		act = "You were wielding";
	}

	/* Took off bow */
	else if (item == INVEN_BOW)
	{
		act = "You were holding";
	}

	/* Took off light */
	else if (item == INVEN_LITE)
	{
		act = "You were holding";
	}

	/* Took off ammo */
	else if (item == INVEN_AMMO)
	{
		act = "You were carrying in your quiver";
	}

	/* Took off tool */
	else if (item == INVEN_TOOL)
	{
		act = "You were using";
	}

	/* Took off something */
	else
	{
		act = "You were wearing";
	}

	/* Modify, Optimize */
	inc_stack_size_ex(item, -amt, OPTIMIZE, NO_DESCRIBE);

	if ((item == INVEN_CARRY) && (get_skill(SKILL_SYMBIOTIC)))
	{
		/* Drop the monster */
		o_ptr->pval2 = 0;
		msg_print("You carefully drop the poor monster on the floor.");
		drop_near(q_ptr, 0, p_ptr->py, p_ptr->px);
		slot = -1;
	}
	else if (force_drop)
	{
		drop_near(q_ptr, 0, p_ptr->py, p_ptr->px);
		slot = -1;
	}
	else
	{
		/* Carry the object */
		slot = inven_carry(q_ptr, false);
	}

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(slot));

	/* Return slot */
	return (slot);
}




/*
 * Drop (some of) a non-cursed inventory/equipment item
 *
 * The object will be dropped "near" the current location
 */
void inven_drop(int item, int amt, int dy, int dx, bool silent)
{
	object_type forge;
	object_type *q_ptr;

	object_type *o_ptr;

	char o_name[80];


	/* Access original object */
	o_ptr = &p_ptr->inventory[item];

	/* Error check */
	if (amt <= 0) return;

	/* Not too many */
	if (amt > o_ptr->number) amt = o_ptr->number;


	/* Take off equipment */
	if (item >= INVEN_WIELD)
	{
		/* Take off first */
		item = inven_takeoff(item, amt, false);

		/* Access original object */
		o_ptr = &p_ptr->inventory[item];
	}

	if (item > -1)
	{
		/* Get local object */
		q_ptr = &forge;

		/* Obtain local object */
		object_copy(q_ptr, o_ptr);

		/*
		 * Hack -- If rods or wands are dropped, the total maximum timeout or 
		 * charges need to be allocated between the two stacks.  If all the items 
		 * are being dropped, it makes for a neater message to leave the original 
		 * stack's pval alone. -LM-
		 */
		if (o_ptr->tval == TV_WAND)
		{
			if (o_ptr->tval == TV_WAND)
			{
				q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
				if (amt < o_ptr->number) o_ptr->pval -= q_ptr->pval;
			}
		}

		/* Modify quantity */
		q_ptr->number = amt;

		/* Describe local object */
		object_desc(o_name, q_ptr, true, 3);

		/* Message */
		if (!silent) msg_format("You drop %s (%c).", o_name, index_to_label(item));

		/* Drop it near the player */
		drop_near(q_ptr, 0, dy, dx);

		/* Modify, Describe, Optimize */
		inc_stack_size(item, -amt);
	}
}



/*
 * Combine items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void combine_pack()
{
	bool flag = false;

	/* Combine the pack (backwards) */
	for (int i = INVEN_PACK; i > 0; i--)
	{
		/* Get the item */
		auto o_ptr = &p_ptr->inventory[i];

		/* Skip empty items */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Scan the items above that item */
		for (int j = 0; j < i; j++)
		{
			/* Get the item */
			auto j_ptr = &p_ptr->inventory[j];

			/* Skip empty items */
			if (!j_ptr->k_ptr)
			{
				continue;
			}

			/* Can we drop "o_ptr" onto "j_ptr"? */
			if (object_similar(j_ptr, o_ptr))
			{
				/* Take note */
				flag = true;

				/* Add together the item counts */
				object_absorb(j_ptr, o_ptr);

				/* One object is gone */
				inven_cnt--;

				/* Slide everything down */
				int k;
				for (k = i; k < INVEN_PACK; k++)
				{
					/* Structure copy */
					p_ptr->inventory[k] = p_ptr->inventory[k + 1];
				}

				/* Erase the "final" slot */
				object_wipe(&p_ptr->inventory[k]);

				/* Window stuff */
				p_ptr->window |= (PW_INVEN);

				/* Done */
				break;
			}
		}
	}

	/* Message */
	if (flag)
	{
		msg_print("You combine some items in your pack.");
	}
}


/*
 * Reorder items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void reorder_pack()
{
	int i, j, k;
	s32b o_value;
	s32b j_value;
	object_type forge;
	object_type *q_ptr;
	object_type *j_ptr;
	object_type *o_ptr;
	bool flag = false;


	/* Re-order the pack (forwards) */
	for (i = 0; i < INVEN_PACK; i++)
	{
		/* Mega-Hack -- allow "proper" over-flow */
		if ((i == INVEN_PACK) && (inven_cnt == INVEN_PACK)) break;

		/* Get the item */
		o_ptr = &p_ptr->inventory[i];

		/* Skip empty slots */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* Get the "value" of the item */
		o_value = object_value(o_ptr);

		/* Scan every occupied slot */
		for (j = 0; j < INVEN_PACK; j++)
		{
			/* Get the item already there */
			j_ptr = &p_ptr->inventory[j];

			/* Use empty slots */
			if (!j_ptr->k_ptr)
			{
				break;
			}

			/* Objects sort by decreasing type */
			if (o_ptr->tval > j_ptr->tval) break;
			if (o_ptr->tval < j_ptr->tval) continue;

			/* Non-aware (flavored) items always come last */
			if (!object_aware_p(o_ptr)) continue;
			if (!object_aware_p(j_ptr)) break;

			/* Objects sort by increasing sval */
			if (o_ptr->sval < j_ptr->sval) break;
			if (o_ptr->sval > j_ptr->sval) continue;

			/* Unidentified objects always come last */
			if (!object_known_p(o_ptr)) continue;
			if (!object_known_p(j_ptr)) break;


			/* Hack:  otherwise identical rods sort by
			increasing recharge time --dsb */
			if (o_ptr->tval == TV_ROD_MAIN)
			{
				if (o_ptr->timeout > j_ptr->timeout) break;
				if (o_ptr->timeout < j_ptr->timeout) continue;
			}

			/* Determine the "value" of the pack item */
			j_value = object_value(j_ptr);



			/* Objects sort by decreasing value */
			if (o_value > j_value) break;
			if (o_value < j_value) continue;
		}

		/* Never move down */
		if (j >= i) continue;

		/* Take note */
		flag = true;

		/* Get local object */
		q_ptr = &forge;

		/* Save a copy of the moving item */
		object_copy(q_ptr, &p_ptr->inventory[i]);

		/* Slide the objects */
		for (k = i; k > j; k--)
		{
			/* Slide the item */
			object_copy(&p_ptr->inventory[k], &p_ptr->inventory[k - 1]);
		}

		/* Insert the moving item */
		object_copy(&p_ptr->inventory[j], q_ptr);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN);
	}

	/* Message */
	if (flag) msg_print("You reorder some items in your pack.");
}


/*
 * Let the floor carry an object
 */
s16b floor_carry(int y, int x, object_type *j_ptr)
{
	/* Scan objects in that grid for combination */
	for (auto const this_o_idx: cave[y][x].o_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr))
		{
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Done */
			return this_o_idx;
		}
	}

	/* The stack is already too large */
	if (cave[y][x].o_idxs.size() > 23)
	{
		return (0);
	}

	/* Make an object */
	s16b o_idx = o_pop();

	/* Success */
	if (o_idx)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[o_idx];

		/* Structure Copy */
		object_copy(o_ptr, j_ptr);

		/* Location */
		o_ptr->iy = y;
		o_ptr->ix = x;

		/* Forget monster */
		o_ptr->held_m_idx = 0;

		/* Place the object */
		cave[y][x].o_idxs.push_back(o_idx);

		/* Notice */
		note_spot(y, x);

		/* Redraw */
		lite_spot(y, x);
	}

	/* Result */
	return (o_idx);
}

/*
 * Notice a decaying object in the pack
 */
void pack_decay(int item)
{
	auto const &r_info = game->edit_data.r_info;

	object_type *o_ptr = &p_ptr->inventory[item];

	auto r_ptr = &r_info[o_ptr->pval2];

	object_type *i_ptr;
	object_type object_type_body;

	int amt = o_ptr->number;

	s16b m_type;
	s32b wt;

	byte known = o_ptr->name1;

	byte gone = 1;

	char desc[80];

	/* Player notices each decaying object */
	object_desc(desc, o_ptr, true, 3);
	msg_format("You feel %s decompose.", desc);

	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain local object */
	object_copy(i_ptr, o_ptr);

	/* Remember what creature we were */
	m_type = o_ptr->pval2;

	/* and how much we weighed */
	wt = r_ptr->weight;

	/* Get rid of decayed object */
	inc_stack_size_ex(item, -amt, OPTIMIZE, NO_DESCRIBE);

	if (i_ptr->tval == TV_CORPSE)
	{
		/* Monster must have a skull for its head to become one */
		if (i_ptr->sval == SV_CORPSE_HEAD)
		{
			/* Replace the head with a skull */
			object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKULL));
			i_ptr->weight = wt / 60 + rand_int(wt) / 600;

			/* Stay here */
			gone = 0;
		}

		/* Monster must have a skeleton for its corpse to become one */
		if ((i_ptr->sval == SV_CORPSE_CORPSE) && (r_ptr->flags & RF_DROP_SKELETON))
		{
			/* Replace the corpse with a skeleton */
			object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKELETON));
			i_ptr->weight = wt / 4 + rand_int(wt) / 40;

			/* Stay here */
			gone = 0;
		}

		/* Don't restore if the item is gone */
		if (!gone)
		{
			i_ptr->number = amt;
			i_ptr->pval2 = m_type;

			/* Should become "The skull of Farmer Maggot", not "A skull" */
			if (known)
			{
				object_aware(i_ptr);

				/* Named skeletons are artifacts */
				i_ptr->name1 = 201;
			}
			inven_carry(i_ptr, true);
		}
	}
}

/*
 *	Decay an object on the floor
 */
void floor_decay(int item)
{
	auto const &r_info = game->edit_data.r_info;

	object_type *o_ptr = &o_list[item];

	auto r_ptr = &r_info[o_ptr->pval2];

	object_type *i_ptr;
	object_type object_type_body;

	int amt = o_ptr->number;

	s16b m_type;
	s32b wt;

	byte known = o_ptr->name1;

	/* Assume we disappear */
	byte gone = 1;

	byte x = o_ptr->ix;
	byte y = o_ptr->iy;

	/* Maybe the player sees it */
	bool visible = player_can_see_bold(o_ptr->iy, o_ptr->ix);
	char desc[80];

	if (visible)
	{
		/* Player notices each decaying object */
		object_desc(desc, o_ptr, true, 3);
		msg_format("You see %s decompose.", desc);
	}


	/* Get local object */
	i_ptr = &object_type_body;

	/* Obtain local object */
	object_copy(i_ptr, o_ptr);

	/* Remember what creature we were */
	m_type = o_ptr->pval2;

	/* and how much we weighed */
	wt = r_ptr->weight;

	floor_item_increase(item, -amt);
	floor_item_optimize(item);

	if (i_ptr->tval == TV_CORPSE)
	{
		/* Monster must have a skull for its head to become one */
		if (i_ptr->sval == SV_CORPSE_HEAD)
		{
			/* Replace the head with a skull */
			object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKULL));
			i_ptr->weight = wt / 60 + rand_int(wt) / 600;

			/* Stay here */
			gone = 0;
		}

		/* Monster must have a skeleton for its corpse to become one */
		if ((i_ptr->sval == SV_CORPSE_CORPSE) && (r_ptr->flags & RF_DROP_SKELETON))
		{
			/* Replace the corpse with a skeleton */
			object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKELETON));
			i_ptr->weight = wt / 4 + rand_int(wt) / 40;

			/* Stay here */
			gone = 0;
		}

		/* Don't restore if the item is gone */
		if (!gone)
		{
			i_ptr->number = amt;
			i_ptr->pval2 = m_type;

			/* Should become "The skull of Farmer Maggot", not "A skull" */
			if (known)
			{
				object_aware(i_ptr);

				/* Named skeletons are artifacts */
				i_ptr->name1 = 201;
			}
			floor_carry(y, x, i_ptr);
		}
	}
}

/* Return the item be it on the floor or in inven */
object_type *get_object(int item)
{
	if (item >= 0)
	{
		assert(item < INVEN_TOTAL);
		return &p_ptr->inventory[item];
	}
	else
	{
		return &o_list[0 - item];
	}
}

