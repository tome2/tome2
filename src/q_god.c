#include "angband.h"
#include <assert.h>

/* d_idx of the god_quest (Lost Temple) dungeon */
#define DUNGEON_GOD 30

static int get_quests_given()
{
	return get_lua_int("god_quest.quests_given");
}

static int get_dung_y()
{
	return get_lua_int("god_quest.dung_y");
}

static int get_dung_x()
{
	return get_lua_int("god_quest.dung_x");
}

static void set_dung_y(int y)
{
	exec_lua(format("god_quest.dung_y = %d", y));
}

static void set_dung_x(int x)
{
	exec_lua(format("god_quest.dung_x = %d", x));
}

static int get_relic_num()
{
	return get_lua_int("god_quest.relic_num");
}

static void set_relic_generated(bool_ v)
{
	switch (v)
	{
	case TRUE:
		exec_lua("god_quest.relic_generated = TRUE");
		break;
	case FALSE:
		exec_lua("god_quest.relic_generated = FALSE");
		break;
	default:
		assert(FALSE);
		break;
	}
}

static void set_relic_gen_tries(int v)
{
	exec_lua(format("god_quest.relic_gen_tries = %d", v));
}

void quest_god_place_rand_dung()
{
	int x = -1, y = -1, tries;

	/* erase old dungeon */
	if (get_quests_given() > 0)
	{
		wild_map[get_dung_y()][get_dung_x()].entrance = 0;
		
		/* erase old recall level */
		max_dlv[DUNGEON_GOD] = 0;
	}

	/* initialise tries variable */
	tries = 1000;
	while (tries > 0)
	{
		wilderness_map *w_ptr = NULL;
		wilderness_type_info *wf_ptr = NULL;
		tries = tries - 1;

		/* get grid coordinates, within a range which prevents
		 * dungeon being generated at the very edge of the
		 * wilderness (would crash the game). */
		x = rand_range(1, max_wild_x-2);
		y = rand_range(1, max_wild_y-2);

		/* Is there a town/dungeon/potentially impassable feature there, ? */
		w_ptr = &wild_map[y][x];
		wf_ptr = &wf_info[w_ptr->feat];

		if ((w_ptr->entrance != 0) ||
		    (wf_ptr->entrance != 0) ||
		    (wf_ptr->terrain_idx == TERRAIN_EDGE) ||
		    (wf_ptr->terrain_idx == TERRAIN_DEEP_WATER) ||
		    (wf_ptr->terrain_idx == TERRAIN_TREES) ||
		    (wf_ptr->terrain_idx == TERRAIN_SHALLOW_LAVA) ||
		    (wf_ptr->terrain_idx == TERRAIN_DEEP_LAVA) ||
		    (wf_ptr->terrain_idx == TERRAIN_MOUNTAIN))
		{
			/* try again */
		}
		else
		{
			/* either player, nor wall, then stop this 'while' */
			break;
		}
	}

	assert(x >= 0);
	assert(y >= 0);

	if (tries == 0)
	{
		/* Use Bree as last resort */
		x = 32;
		y = 19;
	}

	/* create god dungeon in that place */
	wild_map[y][x].entrance = 1000 + DUNGEON_GOD;

	/* set quest variables */
	set_dung_x(x);
	set_dung_y(y);
}

void quest_god_generate_relic()
{
	int tries = 1000, x = -1, y = -1;
	object_type relic;

	tries = 1000;

	while (tries > 0)
	{
		cave_type *c_ptr;
		tries = tries - 1;
		/* get grid coordinates from current height/width,
		 * minus one to prevent relic being generated in
		 * outside wall. (would crash the game) */
		y = randint(cur_hgt-1);
		x = randint(cur_wid-1);
		c_ptr = &cave[y][x];

		/* are the coordinates on a floor, not on a permanent feature (eg stairs), and not on a trap ? */
		if ((f_info[c_ptr->feat].flags1 & FF1_FLOOR) &&
		    (!(f_info[c_ptr->feat].flags1 & FF1_PERMANENT)) &&
		    (c_ptr->t_idx == 0))
		{
			break;
		}
	}

	/* create relic */
	object_prep(&relic, lookup_kind(TV_JUNK, get_relic_num()));

	/* inscribe it to prevent automatizer 'accidents' */
	relic.note = quark_add("quest");

	/* If no safe co-ords were found, put it in the players backpack */
	if (tries == 0)
	{
		/* explain it */
		cmsg_print(TERM_L_BLUE, "You luckily stumble across the relic on the stairs!");

		if (inven_carry_okay(&relic))
		{
			inven_carry(&relic, FALSE);
		}
		else
		{
			/* no place found, drop it on the stairs */
			drop_near(&relic, -1, p_ptr->py, p_ptr->px);
		}
	}
	else
	{
		/* drop it */
		drop_near(&relic, -1, y, x);
	}

	/* Only generate once! */
	set_relic_generated(TRUE);

	/* Reset some variables */
	set_relic_gen_tries(0);
}
