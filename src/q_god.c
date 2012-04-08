#include "angband.h"
#include <assert.h>

/* d_idx of the god_quest (Lost Temple) dungeon */
#define DUNGEON_GOD 30
#define CHANCE_OF_GOD_QUEST 21

static int get_quests_given()
{
	return get_lua_int("god_quest.quests_given");
}

static void set_quests_given(int i)
{
	exec_lua(format("god_quest.quests_given = %d", i));
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

static int get_status()
{
	return exec_lua("return quest(GOD_QUEST).status");
}

static void set_status(int new_status)
{
	exec_lua(format("quest(GOD_QUEST).status = %d", new_status));
}

static int MAX_NUM_GOD_QUESTS()
{
	return get_lua_int("god_quest.MAX_NUM_GOD_QUESTS");
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

static bool_ get_relic_generated()
{
	return get_lua_int("god_quest.relic_generated");
}

static void set_relic_gen_tries(int v)
{
	exec_lua(format("god_quest.relic_gen_tries = %d", v));
}

static int get_relic_gen_tries()
{
	return get_lua_int("god_quest.relic_gen_tries");
}

static void set_player_y(int y)
{
	exec_lua(format("god_quest.player_y = %d", y));
}

static void set_player_x(int x)
{
	exec_lua(format("god_quest.player_x = %d", x));
}

static int get_dun_mindepth()
{
	return get_lua_int("god_quest.dun_mindepth");
}

static void set_dun_mindepth(int d)
{
	exec_lua(format("god_quest.dun_mindepth = %d", d));
}

static void set_dun_maxdepth(int d)
{
	exec_lua(format("god_quest.dun_maxdepth = %d", d));

}

static void set_dun_minplev(int p)
{
	exec_lua(format("god_quest.dun_minplev = %d", p));
}

static int get_dun_minplev()
{
	return get_lua_int("god_quest.dun_minplev");
}

static void setup_relic_number()
{
	exec_lua("setup_relic_number()");
}

static void msg_directions()
{
	exec_lua("msg_directions()");
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

void quest_god_set_god_dungeon_attributes_eru()
{
	/* The Eru temple is based on Meneltarma. */

	/* W: Not too many monsters (they'll be tough though, with big
	 * levels) */
	d_info[DUNGEON_GOD].min_m_alloc_level = 14;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 200;

	/* L: Dirt and grass. More dirt at bottom, more grass at
	 * top. rocky ground would be nice */
	d_info[DUNGEON_GOD].floor1 = 88;
	d_info[DUNGEON_GOD].floor2 = 89;
	d_info[DUNGEON_GOD].floor_percent1[0] = 70;
	d_info[DUNGEON_GOD].floor_percent2[0] = 30;
	d_info[DUNGEON_GOD].floor_percent1[1] = 10;
	d_info[DUNGEON_GOD].floor_percent2[1] = 90;

	/* A: Outer wall mountain chain. other walls granite */
	d_info[DUNGEON_GOD].fill_type1 = 97;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 57;
	d_info[DUNGEON_GOD].inner_wall = 97;
	d_info[DUNGEON_GOD].fill_method = 2;

	/* O: "At Meneltarma no weapon or tool had ever been borne"
	 * (but invaders would have left a small number) */
	d_info[DUNGEON_GOD].objs.treasure = 45;
	d_info[DUNGEON_GOD].objs.combat = 5;
	d_info[DUNGEON_GOD].objs.magic = 45;
	d_info[DUNGEON_GOD].objs.tools = 5;

	/* F: A large pillar, with stairs created at edges. (You can't
	 * climb a rock through the middle, can you?) */
	d_info[DUNGEON_GOD].flags1 =
		DF1_BIG | DF1_NO_DOORS | DF1_CIRCULAR_ROOMS |
		DF1_EMPTY | DF1_TOWER | DF1_FLAT | DF1_ADJUST_LEVEL_2;
	d_info[DUNGEON_GOD].flags2 =
		DF2_ADJUST_LEVEL_1_2 |
		DF2_NO_SHAFT |
		DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 50;

	/* M: We want evil or flying characters */
	d_info[DUNGEON_GOD].rules[0].mflags3 = RF3_EVIL;

	d_info[DUNGEON_GOD].rules[1].mode = 3;
	d_info[DUNGEON_GOD].rules[1].percent = 50;

	/* M: We want evil or flying characters */
	d_info[DUNGEON_GOD].rules[1].mflags7 = RF7_CAN_FLY;
}

void quest_god_set_god_dungeon_attributes_manwe()
{
	/* Manwe's lost temple is high in the clouds */

	/* W: Has average number of monsters. */
	d_info[DUNGEON_GOD].min_m_alloc_level = 18;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 160;

	/* L: floor will be 'cloud-like vapour' and pools of
	 * 'condensing water' */
	d_info[DUNGEON_GOD].floor1 = 208;
	d_info[DUNGEON_GOD].floor2 = 209;
	d_info[DUNGEON_GOD].floor_percent1[0] = 85;
	d_info[DUNGEON_GOD].floor_percent2[0] = 15;

	/* A: Outer wall is 'hail stone wall', inner wall 'dense
	 * fog'. FIlled at max smoothing, like islands. */
	d_info[DUNGEON_GOD].fill_type1 = 211;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 210;
	d_info[DUNGEON_GOD].inner_wall = 211;
	d_info[DUNGEON_GOD].fill_method = 4;

	/* O: Can't imagine Manwe having much treasure. Little need
	 * for tools in a cloud temple. lots of magical stuff
	 * though... */
	d_info[DUNGEON_GOD].objs.treasure = 15;
	d_info[DUNGEON_GOD].objs.combat = 25;
	d_info[DUNGEON_GOD].objs.magic = 55;
	d_info[DUNGEON_GOD].objs.tools = 5;

	/* F: It's open, goes up like a tower, give it a few
	 * interesting rooms, make the monsters hard(ish). */
	d_info[DUNGEON_GOD].flags1 =
		DF1_NO_DOORS | DF1_TOWER |
		DF1_CAVERN | DF1_ADJUST_LEVEL_2;
	d_info[DUNGEON_GOD].flags2 =
		DF2_NO_SHAFT | DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 20;
	d_info[DUNGEON_GOD].rules[1].mode = 3;
	d_info[DUNGEON_GOD].rules[1].percent = 20;
	d_info[DUNGEON_GOD].rules[2].mode = 3;
	d_info[DUNGEON_GOD].rules[2].percent = 20;
	d_info[DUNGEON_GOD].rules[3].mode = 3;
	d_info[DUNGEON_GOD].rules[3].percent = 20;
	d_info[DUNGEON_GOD].rules[4].mode = 3;
	d_info[DUNGEON_GOD].rules[4].percent = 20;

	/* M: We want air(poison-type) or flying characters. Orcs
	 * too. They would have ransacked his elf-loving temple :) */
	d_info[DUNGEON_GOD].rules[0].mflags2 = RF2_INVISIBLE;
	d_info[DUNGEON_GOD].rules[1].mflags3 = RF3_ORC | RF3_IM_POIS;
	d_info[DUNGEON_GOD].rules[2].mflags4 = RF4_BR_POIS | RF4_BR_GRAV;
	d_info[DUNGEON_GOD].rules[3].mflags5 = RF5_BA_POIS;
	d_info[DUNGEON_GOD].rules[4].mflags7 = RF7_CAN_FLY;
}

void quest_god_set_god_dungeon_attributes_tulkas()
{
	/* Tulkas dungeon is quite normal, possibly a bit boring to be
	 * honest. Maybe I should add something radical to it.  'The
	 * house of Tulkas in the midmost of Valmar was a house of
	 * mirth and revelry. It sprang into the air with many
	 * storeys, and had a tower of bronze and pillars of copper in
	 * a wide arcade'
	 */

	/* W: but with lots of monsters */
	d_info[DUNGEON_GOD].min_m_alloc_level = 20;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 120;

	/* L: floor is normal */
	d_info[DUNGEON_GOD].floor1 = 1;
	d_info[DUNGEON_GOD].floor_percent1[0] = 100;

	/* A: Granite walls */
	d_info[DUNGEON_GOD].fill_type1 = 56;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 58;
	d_info[DUNGEON_GOD].inner_wall = 57;
	d_info[DUNGEON_GOD].fill_method = 0;

	/* O: Loads of combat drops */
	d_info[DUNGEON_GOD].objs.treasure = 10;
	d_info[DUNGEON_GOD].objs.combat = 70;
	d_info[DUNGEON_GOD].objs.magic = 5;
	d_info[DUNGEON_GOD].objs.tools = 15;

	/* F: fairly standard */
	d_info[DUNGEON_GOD].flags1 = DF1_NO_DESTROY | DF1_ADJUST_LEVEL_2;
	d_info[DUNGEON_GOD].flags2 = DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 100;

	/* M: plenty demons please */
	d_info[DUNGEON_GOD].rules[0].mflags3 = RF3_DEMON | RF3_EVIL;
}

void quest_god_set_god_dungeon_attributes_melkor()
{
	/* Melkors dungeon will be dark, fiery and stuff */

	/* Many many monsters! (but prob ADJUST_LEVEL_1_2) */
	d_info[DUNGEON_GOD].min_m_alloc_level = 24;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 80;

	/* L: floor is dirt/mud/nether */
	d_info[DUNGEON_GOD].floor1 = 88;
	d_info[DUNGEON_GOD].floor2 = 94;
	d_info[DUNGEON_GOD].floor3 = 102;
	d_info[DUNGEON_GOD].floor_percent1[0] = 45;
	d_info[DUNGEON_GOD].floor_percent2[0] = 45;
	d_info[DUNGEON_GOD].floor_percent3[0] = 10;
	d_info[DUNGEON_GOD].floor_percent1[1] = 35;
	d_info[DUNGEON_GOD].floor_percent2[1] = 35;
	d_info[DUNGEON_GOD].floor_percent3[1] = 30;

	/* A: Granite walls to fill but glass walls for room
	 * perimeters (you can see the nasty monsters coming) */
	d_info[DUNGEON_GOD].fill_type1 = 188;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 188;
	d_info[DUNGEON_GOD].inner_wall = 57;
	d_info[DUNGEON_GOD].fill_method = 1;

	/* O: Even drops */
	d_info[DUNGEON_GOD].objs.treasure = 25;
	d_info[DUNGEON_GOD].objs.combat = 25;
	d_info[DUNGEON_GOD].objs.magic = 25;
	d_info[DUNGEON_GOD].objs.tools = 25;

	/* F: Small, lava rivers, nasty monsters hehehehehe */
	d_info[DUNGEON_GOD].flags1 = DF1_SMALL | DF1_LAVA_RIVERS | DF1_ADJUST_LEVEL_1;
	d_info[DUNGEON_GOD].flags2 = DF2_ADJUST_LEVEL_1_2 | DF2_ADJUST_LEVEL_PLAYER;

	/* R: No restrictions on monsters here */
	d_info[DUNGEON_GOD].rules[0].mode = 0;
	d_info[DUNGEON_GOD].rules[0].percent = 80;

	/* R: Apart from making sure we have some GOOD ones */
	d_info[DUNGEON_GOD].rules[1].mode = 3;
	d_info[DUNGEON_GOD].rules[1].percent = 20;

	/* M: */
	d_info[DUNGEON_GOD].rules[1].mflags3 = RF3_GOOD;
}

void quest_god_set_god_dungeon_attributes_yavanna()
{
	/* Yavannas dungeon will be very natural, tress and stuff. */

	d_info[DUNGEON_GOD].min_m_alloc_level = 22;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 100;

	/* L: floor is grass/flowers, plus dirt so not always
	 * regenerating quick! */
	d_info[DUNGEON_GOD].floor1 = 89;
	d_info[DUNGEON_GOD].floor2 = 199;
	d_info[DUNGEON_GOD].floor3 = 88;
	d_info[DUNGEON_GOD].floor_percent1[0] = 40;
	d_info[DUNGEON_GOD].floor_percent2[0] = 15;
	d_info[DUNGEON_GOD].floor_percent3[0] = 45;

	/* A: Tree walls to fill, small trees for inner walls */
	d_info[DUNGEON_GOD].fill_type1 = 96;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 202;
	d_info[DUNGEON_GOD].inner_wall = 96;
	d_info[DUNGEON_GOD].fill_method = 1;

	/* O: not much combat.. tools where ransackers have tried to
	 * chop trees down. */
	d_info[DUNGEON_GOD].objs.treasure = 20;
	d_info[DUNGEON_GOD].objs.combat = 10;
	d_info[DUNGEON_GOD].objs.magic = 30;
	d_info[DUNGEON_GOD].objs.tools = 40;

	/* F: Natural looking */
	d_info[DUNGEON_GOD].flags1 =
		DF1_NO_DOORS | DF1_WATER_RIVERS |
		DF1_NO_DESTROY | DF1_ADJUST_LEVEL_1 | 
		DF1_NO_RECALL;
	d_info[DUNGEON_GOD].flags2 =
		DF2_ADJUST_LEVEL_1_2 | DF2_NO_SHAFT |
		DF2_NO_GENO | DF2_ADJUST_LEVEL_PLAYER;

	/* R: Demons, Undead, non-living */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 100;

	/* M: */
	d_info[DUNGEON_GOD].rules[0].mflags3 =
		RF3_DEMON | RF3_UNDEAD | RF3_NONLIVING;
}

void quest_god_set_god_dungeon_attributes_aule()
{
	d_info[DUNGEON_GOD].min_m_alloc_level = 24;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 80;

	/* L: floor is dirt/mud/shallow water */
	d_info[DUNGEON_GOD].floor1 = 88;
	d_info[DUNGEON_GOD].floor2 = 94;
	d_info[DUNGEON_GOD].floor3 = 84;
	d_info[DUNGEON_GOD].floor_percent1[0] = 45;
	d_info[DUNGEON_GOD].floor_percent2[0] = 45;
	d_info[DUNGEON_GOD].floor_percent3[0] = 10;
	d_info[DUNGEON_GOD].floor_percent1[1] = 35;
	d_info[DUNGEON_GOD].floor_percent2[1] = 35;
	d_info[DUNGEON_GOD].floor_percent3[1] = 30;

	/* A: Grey mountains, inner walls are low hills */
	d_info[DUNGEON_GOD].fill_type1 = 216;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 216;
	d_info[DUNGEON_GOD].inner_wall = 213;
	d_info[DUNGEON_GOD].fill_method = 1;

	/* O: Weapons and tools only */
	d_info[DUNGEON_GOD].objs.treasure = 0;
	d_info[DUNGEON_GOD].objs.combat = 50;
	d_info[DUNGEON_GOD].objs.magic = 0;
	d_info[DUNGEON_GOD].objs.tools = 50;

	/* F: Small, no destroyed levels, min monster level = dungeon
	 * level */
	d_info[DUNGEON_GOD].flags1 = 
		DF1_SMALL | DF1_NO_DESTROY |
		DF1_ADJUST_LEVEL_1 | DF1_NO_STREAMERS;

	/* R: No restrictions on monsters here */
	d_info[DUNGEON_GOD].rules[0].mode = 0;
	d_info[DUNGEON_GOD].rules[0].percent = 80;
}

void quest_god_set_god_dungeon_attributes_varda()
{
	/* Varda lives with Manwe, so high in the clouds */

	/* W: Has average number of monsters. */
	d_info[DUNGEON_GOD].min_m_alloc_level = 18;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 160;

	/* L: floor will be grass and flowers */
	d_info[DUNGEON_GOD].floor1 = 89;
	d_info[DUNGEON_GOD].floor2 = 82;
	d_info[DUNGEON_GOD].floor_percent1[0] = 85;
	d_info[DUNGEON_GOD].floor_percent2[0] = 15;

	/* A: Outer wall is 'hail stone wall', inner wall 'dense
	 * fog'. Filled at max smoothing, like islands. */
	d_info[DUNGEON_GOD].fill_type1 = 211;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 210;
	d_info[DUNGEON_GOD].inner_wall = 211;
	d_info[DUNGEON_GOD].fill_method = 4;

	/* O: Varda likes magical items and tools, not much treasure
	 * or weapons */
	d_info[DUNGEON_GOD].objs.treasure = 15;
	d_info[DUNGEON_GOD].objs.combat = 5;
	d_info[DUNGEON_GOD].objs.magic = 55;
	d_info[DUNGEON_GOD].objs.tools = 25;

	/* F: It's open, goes up like a tower, give it a few
	 * interesting rooms, make the monsters hard(ish). */
	d_info[DUNGEON_GOD].flags1 =
		DF1_NO_DOORS | DF1_TOWER |
		DF1_CAVERN | DF1_ADJUST_LEVEL_1;
	d_info[DUNGEON_GOD].flags2 =
		DF2_NO_SHAFT | DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 20;
	d_info[DUNGEON_GOD].rules[1].mode = 3;
	d_info[DUNGEON_GOD].rules[1].percent = 20;
	d_info[DUNGEON_GOD].rules[2].mode = 3;
	d_info[DUNGEON_GOD].rules[2].percent = 20;
	d_info[DUNGEON_GOD].rules[3].mode = 3;
	d_info[DUNGEON_GOD].rules[3].percent = 20;
	d_info[DUNGEON_GOD].rules[4].mode = 3;
	d_info[DUNGEON_GOD].rules[4].percent = 20;

	/* M: We want air(poison-type) or flying characters. Orcs too. */
	d_info[DUNGEON_GOD].rules[0].mflags2 = RF2_INVISIBLE;
	d_info[DUNGEON_GOD].rules[1].mflags3 = RF3_ORC | RF3_IM_POIS;
	d_info[DUNGEON_GOD].rules[2].mflags4 = RF4_BR_POIS | RF4_BR_GRAV;
	d_info[DUNGEON_GOD].rules[3].mflags5 = RF5_BA_POIS;
	d_info[DUNGEON_GOD].rules[4].mflags7 = RF7_CAN_FLY;
}

void quest_god_set_god_dungeon_attributes_ulmo()
{
	/* Ulmo dungeon is basically Tulkas, except with acquatic creatures. */

	/* W: but with lots of monsters */
	d_info[DUNGEON_GOD].min_m_alloc_level = 20;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 120;

	/* L: floor is dirt */
	d_info[DUNGEON_GOD].floor1 = 88;
	d_info[DUNGEON_GOD].floor_percent1[0] = 100;

	/* A: Cheat: walls are water. */
	d_info[DUNGEON_GOD].fill_type1 = 187;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 238;
	d_info[DUNGEON_GOD].inner_wall = 84;
	d_info[DUNGEON_GOD].fill_method = 0;

	/* O: Lots of treasure, not much else. */
	d_info[DUNGEON_GOD].objs.treasure = 90;
	d_info[DUNGEON_GOD].objs.combat = 0;
	d_info[DUNGEON_GOD].objs.magic = 5;
	d_info[DUNGEON_GOD].objs.tools = 5;

	/* F: fairly standard */
	d_info[DUNGEON_GOD].flags1 = DF1_NO_DESTROY | DF1_ADJUST_LEVEL_2;
	d_info[DUNGEON_GOD].flags2 = DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 35;
	d_info[DUNGEON_GOD].rules[1].mode = 3;
	d_info[DUNGEON_GOD].rules[1].percent = 30;
	d_info[DUNGEON_GOD].rules[2].mode = 3;
	d_info[DUNGEON_GOD].rules[2].percent = 30;

	/* M: Aquatic creatures only. */
	d_info[DUNGEON_GOD].rules[0].mflags3 = RF7_CAN_FLY;
	d_info[DUNGEON_GOD].rules[1].mflags3 = RF7_AQUATIC;
	d_info[DUNGEON_GOD].rules[2].mflags3 = RF3_RES_WATE;
}

void quest_god_set_god_dungeon_attributes_mandos()
{
	/* Mandos dungeon is basically Tulkas, except with undead. */

	/* W: but with lots of monsters */
	d_info[DUNGEON_GOD].min_m_alloc_level = 20;
	d_info[DUNGEON_GOD].max_m_alloc_chance = 120;

	/* L: floor is normal */
	d_info[DUNGEON_GOD].floor1 = 1;
	d_info[DUNGEON_GOD].floor_percent1[0] = 100;

	/* A: Granite walls */
	d_info[DUNGEON_GOD].fill_type1 = 56;
	d_info[DUNGEON_GOD].fill_percent1[0] = 100;
	d_info[DUNGEON_GOD].outer_wall = 58;
	d_info[DUNGEON_GOD].inner_wall = 57;
	d_info[DUNGEON_GOD].fill_method = 0;

	/* O: Loads of combat drops */
	d_info[DUNGEON_GOD].objs.treasure = 10;
	d_info[DUNGEON_GOD].objs.combat = 70;
	d_info[DUNGEON_GOD].objs.magic = 5;
	d_info[DUNGEON_GOD].objs.tools = 15;

	/* F: fairly standard */
	d_info[DUNGEON_GOD].flags1 = DF1_NO_DESTROY | DF1_ADJUST_LEVEL_2;
	d_info[DUNGEON_GOD].flags2 = DF2_ADJUST_LEVEL_PLAYER;

	/* R: */
	d_info[DUNGEON_GOD].rules[0].mode = 3;
	d_info[DUNGEON_GOD].rules[0].percent = 100;

	/* M: vampires! */
	d_info[DUNGEON_GOD].rules[0].r_char[0] = 'V';
	d_info[DUNGEON_GOD].rules[0].r_char[1] = '\0';
	d_info[DUNGEON_GOD].rules[0].r_char[2] = '\0';
	d_info[DUNGEON_GOD].rules[0].r_char[3] = '\0';
	d_info[DUNGEON_GOD].rules[0].r_char[4] = '\0';
	d_info[DUNGEON_GOD].rules[0].mflags3 = RF3_UNDEAD | RF3_EVIL;
}

void quest_god_level_end_gen_hook()
{
	/* Check for dungeon */
	if ((dungeon_type != DUNGEON_GOD) ||
	    (get_status() == QUEST_STATUS_UNTAKEN))
	{
		return;
	}

	/* if the relic has been created at this point, then it was
	   created on the *PREVIOUS* call of HOOK_LEVEL_END_GEN, and
	   therefore the player has caused another level generation in
	   the temple and hence failed the quest.*/

	else if ((get_relic_generated() == TRUE) &&
		 (get_status() != QUEST_STATUS_FAILED))
	{
		/* fail the quest, don't give another one, don't give
		 * this message again */
		set_status(QUEST_STATUS_FAILED);

		/* God issues instructions */
		cmsg_format(TERM_L_BLUE, "The voice of %s booms in your head:", deity_info[p_ptr->pgod].name);

		cmsg_print(TERM_YELLOW, "'Thou art a fool!");
		cmsg_print(TERM_YELLOW, "I told thee to look carefully for the relic. It appears thou hast missed the");
		cmsg_print(TERM_YELLOW, "opportunity to claim it in my name, as I sense that those monsters who ");
		cmsg_print(TERM_YELLOW, "have overrun my temple have destroyed it themselves.");
		cmsg_print(TERM_YELLOW, "I shall not ask thee to do such a thing again, as thou hast failed me in this");
		cmsg_print(TERM_YELLOW, "simple task!'");
	}

	/* Force relic generation on 5th attempt if others have been
	 * unsuccessful. */

	else if ((get_relic_gen_tries() == 4) &&
		 (get_relic_generated() == FALSE))
	{
		quest_god_generate_relic();
	}

	else
	{
		/* 1/5 chance of generation */
		if (magik(20))
		{
			quest_god_generate_relic();
		}
		else
		{
			set_relic_gen_tries(get_relic_gen_tries() + 1);
		}
	}
}

void quest_god_player_level_hook(int gained)
{
	if (gained <= 0)
	{
		return;
	}

	/* check player is worshipping a god, not already on a god quest. */
	if ((p_ptr->astral) ||
	    (p_ptr->pgod <= 0) ||
	    (get_status() == QUEST_STATUS_TAKEN) ||
	    (get_status() == QUEST_STATUS_FAILED) ||
	    (get_quests_given() >= MAX_NUM_GOD_QUESTS()) ||
	    (magik(CHANCE_OF_GOD_QUEST) == FALSE) ||
	    ((dungeon_type == DUNGEON_GOD) &&
	     (dun_level > 0)) ||
	    (p_ptr->lev <= get_dun_minplev()))
	{
		/* Don't let a player get quests with trickery */
		if (p_ptr->lev > get_dun_minplev())
		{
			set_dun_minplev(p_ptr->lev);
		}
		return;
	}
	else
	{
		/* each god has different characteristics, so the quests are differnet depending on your god */
		setup_relic_number();
		
		/* This var will need resetting */
		set_relic_generated(FALSE);
		set_status(QUEST_STATUS_TAKEN);
		set_quests_given(get_quests_given() + 1);
		
		/* actually place the dungeon in a random place */
		quest_god_place_rand_dung();
		
		/* store the variables of the coords where the player was given the quest */
		if (p_ptr->wild_mode)
		{
			set_player_x(p_ptr->px);
			set_player_y(p_ptr->py);
		}
		else
		{
			set_player_x(p_ptr->wilderness_x);
			set_player_y(p_ptr->wilderness_y);
		}
		
		/* God issues instructions */
		cmsg_format(TERM_L_BLUE, "The voice of %s booms in your head:", deity_info[p_ptr->pgod].name);
		
		cmsg_print(TERM_YELLOW, "'I have a task for thee.");
		cmsg_print(TERM_YELLOW, "Centuries ago an ancient relic of mine was broken apart.");
		cmsg_print(TERM_YELLOW, "The pieces of it have been lost in fallen temples.");
		cmsg_print(TERM_YELLOW, "Thou art to find my lost temple and retrieve a piece of the relic.");
		cmsg_print(TERM_YELLOW, "When thy task is done, thou art to lift it in the air and call upon my name.");
		cmsg_print(TERM_YELLOW, "I shall then come to reclaim what is mine!");
		
		msg_directions();
		
		/* Prepare depth of dungeon. If this was
		 * generated in set_god_dungeon_attributes(),
		 * then we'd have trouble if someone levelled
		 * up in the dungeon! */
		set_dun_mindepth(p_ptr->lev*2/3);
		set_dun_maxdepth(get_dun_mindepth() + 4);
	}
}
