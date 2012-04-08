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
