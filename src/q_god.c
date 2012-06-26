#include "angband.h"

#include <assert.h>

#include "quark.h"

#define cquest (quest[QUEST_GOD])
#define cquest_quests_given (cquest.data[0])
#define cquest_relics_found (cquest.data[1])
#define cquest_dun_mindepth (cquest.data[2])
#define cquest_dun_maxdepth (cquest.data[3])
#define cquest_dun_minplev (cquest.data[4])
#define cquest_relic_gen_tries (cquest.data[5])
#define cquest_relic_generated (cquest.data[6])
#define cquest_dung_x (cquest.data[7])
#define cquest_dung_y (cquest.data[8])

/* d_idx of the god_quest (Lost Temple) dungeon */
#define DUNGEON_GOD 30
#define CHANCE_OF_GOD_QUEST 21

/*
 * Returns the direction of the compass that y2, x2 is from y, x
 * the return value will be one of the following: north, south,
 * east, west, north-east, south-east, south-west, north-west,
 * or "close" if it is within 2 tiles.
 *
 * The returned string is allocated with strdup().
 */
static char *compass(int y, int x, int y2, int x2)
{
	char compass_dir[64];

	// is it close to the north/south meridian?
	int y_diff = y2 - y;

	// determine if y2, x2 is to the north or south of y, x
	const char *y_axis;
	if ((y_diff > -3) && (y_diff < 3))
	{
		y_axis = 0;
	}
	else if (y2 > y)
	{
		y_axis = "south";
	}
	else
	{
		y_axis = "north";
	}

	// is it close to the east/west meridian?
	int x_diff = x2 - x;

	// determine if y2, x2 is to the east or west of y, x
	const char *x_axis;
	if ((x_diff > -3) && (x_diff < 3))
	{
		x_axis = 0;
	}
	else if (x2 > x)
	{
		x_axis = "east";
	}
	else
	{
		x_axis = "west";
	}

	// Maybe it is very close
	if ((!x_axis) && (!y_axis)) { strcpy(compass_dir, "close"); }
	// Maybe it is (almost) due N/S
	else if (!x_axis) { strcpy(compass_dir, y_axis); }
	// Maybe it is (almost) due E/W
	else if (!y_axis) { strcpy(compass_dir, x_axis); }
	//  or if it is neither
	else { sprintf(compass_dir, "%s-%s", y_axis, x_axis); }

	/* Return a copy */
	return strdup(compass_dir);
}

/* Returns a relative approximation of the 'distance' of y2, x2 from y, x. */
static cptr approximate_distance(int y, int x, int y2, int x2)
{
	//  how far to away to the north/south?
	int y_diff = abs(y2 - y);
	// how far to away to the east/west?
	int x_diff = abs(x2 - x);
	// find which one is the larger distance
	int most_dist = x_diff;
	if (y_diff > most_dist) {
		most_dist = y_diff;
	}

	// how far away then?
	if (most_dist >= 41) {
		return "a very long way";
	} else if (most_dist >= 25) {
		return "a long way";
	} else if (most_dist >= 8) {
		return "quite some way";
	} else {
		return "not very far";
	}
}

static int MAX_NUM_GOD_QUESTS()
{
	if (game_module_idx == MODULE_TOME)
	{
		return 5;
	}
	if (game_module_idx == MODULE_THEME)
	{
		return 7;
	}
	/* Uh, oh. */
	assert(FALSE);
	return 0;
}

static byte get_relic_num()
{
	int i;
	int sval_by_god[][2] = {
		{ GOD_ERU, 7 },
		{ GOD_MANWE, 8 },
		{ GOD_TULKAS, 9 },
		{ GOD_MELKOR, 10 },
		{ GOD_YAVANNA, 11 },
		{ GOD_AULE, 16 },
		{ GOD_VARDA, 17 },
		{ GOD_ULMO, 18 },
		{ GOD_MANDOS, 19 },
		{ -1, -1 },
	};

	for (i = 0; sval_by_god[i][1] != -1; i++)
	{
		if (p_ptr->pgod == sval_by_god[i][0])
		{
			int sval = sval_by_god[i][1];
			return sval;
		}
	}

	/* Uh, oh. */
	assert(FALSE);
}

static void get_home_coordinates(int *home1_y, int *home1_x, char **home1,
				 int *home2_y, int *home2_x, char **home2)
{
	/* Which are the waypoints? */
	if (p_ptr->pgod != GOD_MELKOR)
	{
		*home1 = "Bree";
		*home2 = "Minas Anor";
	}
	else
	{
		*home1 = "the Pits of Angband";
		*home2 = "the Land of Mordor";
	}

	/* Module specific locations */
	if (game_module_idx == MODULE_TOME)
	{
		if (p_ptr->pgod != GOD_MELKOR)
		{
			*home1_y = 21;
			*home1_x = 34;
			*home2_y = 56;
			*home2_x = 60;
		}
		else
		{
			*home1_y = 7;
			*home1_x = 34;
			*home2_y = 58;
			*home2_x = 65;
		}
	}
	else if (game_module_idx == MODULE_THEME)
	{
		if (p_ptr->pgod != GOD_MELKOR)
		{
			*home1_y = 21;
			*home1_x = 35;
			*home2_y = 56;
			*home2_x = 60;
		}
		else
		{
			*home1_y = 7;
			*home1_x = 11;
			*home2_y = 49;
			*home2_x = 70;
		}
	}
	else
	{
		assert(FALSE); /* Uh, oh */
	}
}

/* Print using cmsg_print. */
static void print_using_cmsg(cptr line, void *dummy)
{
	cmsg_print(TERM_YELLOW, line);
}

/* Print using print_hook. */
static void print_using_print_hook(cptr line, void *f_)
{
	FILE *f = (FILE *) f_;
	fprintf(f, "%s\n", line);
}

/* Show directions */
static void print_directions(bool_ feel_it, void (*pfunc)(cptr, void *), void *pfdata)
{
	int home1_y, home1_x;
	int home2_y, home2_x;
	char *home1 = NULL;
	char *home2 = NULL;
	char *home1_axis = NULL;
	char *home2_axis = NULL;
	cptr home1_distance = NULL;
	cptr home2_distance = NULL;
	cptr feel_it_str = feel_it ? ", I can feel it.'" : ".";
	char buf[256];

	get_home_coordinates(
		&home1_y, &home1_x, &home1,
		&home2_y, &home2_x, &home2);

	home1_axis = compass(home1_y, home1_x, cquest_dung_y, cquest_dung_x);
	home2_axis = compass(home2_y, home2_x, cquest_dung_y, cquest_dung_x);

	home1_distance = approximate_distance(home1_y, home1_x, cquest_dung_y, cquest_dung_x);
	home2_distance = approximate_distance(home2_y, home2_x, cquest_dung_y, cquest_dung_x);

	/* Build the message */
	if (!streq(home1_axis, "close"))
	{
		snprintf(buf, sizeof(buf),
			 "The temple lies %s to the %s of %s, ",
			 home1_distance,
			 home1_axis,
			 home1);
		pfunc(buf, pfdata);
	}
	else
	{
		snprintf(buf, sizeof(buf),
			 "The temple lies very close to %s, ",
			 home1);
		pfunc(buf, pfdata);
	}

	if (!streq(home2_axis, "close"))
	{
		snprintf(buf, sizeof(buf),
			 "and %s to the %s of %s%s",
			 home2_distance,
			 home2_axis,
			 home2,
			 feel_it_str);
		pfunc(buf, pfdata);
	}
	else
	{
		snprintf(buf, sizeof(buf),
			 "and very close to %s%s",
			 home2,
			 feel_it_str);
		pfunc(buf, pfdata);
	}

	/* Free dyanmically allocated strings */
	free(home1_axis);
	free(home2_axis);
}

bool_ quest_god_describe(FILE *fff)
{
	if (cquest.status == QUEST_STATUS_TAKEN)
	{
		fprintf(fff, "#####yGod quest " FMTs32b "!\n", cquest_quests_given);
		fprintf(fff, "Thou art to find the lost temple of thy God and\n");
		fprintf(fff, "to retrieve the lost part of the relic for thy God! \n");
		print_directions(FALSE, print_using_print_hook, fff);
		fprintf(fff, "\n");
	}

	return TRUE;
}

static void quest_god_place_rand_dung()
{
	int x = -1, y = -1, tries;

	/* erase old dungeon */
	if (cquest_quests_given > 0)
	{
		wild_map[cquest_dung_y][cquest_dung_x].entrance = 0;
		
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
	cquest_dung_x = x;
	cquest_dung_y = y;
}

static void quest_god_generate_relic()
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
	cquest_relic_generated = TRUE;

	/* Reset some variables */
	cquest_relic_gen_tries = 0;
}

static void quest_god_set_god_dungeon_attributes_eru()
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

static void quest_god_set_god_dungeon_attributes_manwe()
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

static void quest_god_set_god_dungeon_attributes_tulkas()
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

static void quest_god_set_god_dungeon_attributes_melkor()
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

static void quest_god_set_god_dungeon_attributes_yavanna()
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

static void quest_god_set_god_dungeon_attributes_aule()
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

static void quest_god_set_god_dungeon_attributes_varda()
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

static void quest_god_set_god_dungeon_attributes_ulmo()
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

static void quest_god_set_god_dungeon_attributes_mandos()
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

static bool_ quest_god_level_end_gen_hook(char *fmt)
{
	/* Check for dungeon */
	if ((dungeon_type != DUNGEON_GOD) ||
	    (cquest.status == QUEST_STATUS_UNTAKEN))
	{
		return FALSE;
	}

	/* if the relic has been created at this point, then it was
	   created on the *PREVIOUS* call of HOOK_LEVEL_END_GEN, and
	   therefore the player has caused another level generation in
	   the temple and hence failed the quest.*/

	else if ((cquest_relic_generated == TRUE) &&
		 (cquest.status != QUEST_STATUS_FAILED))
	{
		/* fail the quest, don't give another one, don't give
		 * this message again */
		cquest.status = QUEST_STATUS_FAILED;

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

	else if ((cquest_relic_gen_tries == 4) &&
		 (cquest_relic_generated == FALSE))
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
			cquest_relic_gen_tries = cquest_relic_gen_tries + 1;
		}
	}

	return FALSE;
}

static bool_ quest_god_player_level_hook(char *fmt)
{
	s32b gained = get_next_arg(fmt);

	if (gained <= 0)
	{
		return FALSE;
	}

	/* check player is worshipping a god, not already on a god quest. */
	if ((p_ptr->astral) ||
	    (p_ptr->pgod <= 0) ||
	    (cquest.status == QUEST_STATUS_TAKEN) ||
	    (cquest.status == QUEST_STATUS_FAILED) ||
	    (cquest_quests_given >= MAX_NUM_GOD_QUESTS()) ||
	    (magik(CHANCE_OF_GOD_QUEST) == FALSE) ||
	    ((dungeon_type == DUNGEON_GOD) &&
	     (dun_level > 0)) ||
	    (p_ptr->lev <= cquest_dun_minplev))
	{
		/* Don't let a player get quests with trickery */
		if (p_ptr->lev > cquest_dun_minplev)
		{
			cquest_dun_minplev = p_ptr->lev;
		}
		return FALSE;
	}
	else
	{
		/* This var will need resetting */
		cquest.status = QUEST_STATUS_TAKEN;
		cquest_relic_generated = FALSE;
		cquest_quests_given = cquest_quests_given + 1;
		
		/* actually place the dungeon in a random place */
		quest_god_place_rand_dung();
		
		/* God issues instructions */
		cmsg_format(TERM_L_BLUE, "The voice of %s booms in your head:", deity_info[p_ptr->pgod].name);
		
		cmsg_print(TERM_YELLOW, "'I have a task for thee.");
		cmsg_print(TERM_YELLOW, "Centuries ago an ancient relic of mine was broken apart.");
		cmsg_print(TERM_YELLOW, "The pieces of it have been lost in fallen temples.");
		cmsg_print(TERM_YELLOW, "Thou art to find my lost temple and retrieve a piece of the relic.");
		cmsg_print(TERM_YELLOW, "When thy task is done, thou art to lift it in the air and call upon my name.");
		cmsg_print(TERM_YELLOW, "I shall then come to reclaim what is mine!");
		
		print_directions(TRUE, print_using_cmsg, NULL);
		
		/* Prepare depth of dungeon. If this was
		 * generated in set_god_dungeon_attributes(),
		 * then we'd have trouble if someone levelled
		 * up in the dungeon! */
		cquest_dun_mindepth = p_ptr->lev*2/3;
		cquest_dun_maxdepth = cquest_dun_mindepth + 4;
	}

	return FALSE;
}

static bool_ quest_god_get_hook(char *fmt)
{
	s32b item;
	object_type *o_ptr = NULL;

	get_next_arg("d"); /* ignore first arg */
	item = get_next_arg("d");

	item = -item; /* Workaround */
	o_ptr = get_object(item);

	/* -- Is it the relic, and check to make sure the relic hasn't already been identified */
	if ((cquest.status == QUEST_STATUS_TAKEN) &&
	    (o_ptr->tval == TV_JUNK) &&
	    (o_ptr->sval == get_relic_num()) &&
	    (o_ptr->pval != TRUE) &&
	    (cquest_relics_found < cquest_quests_given))
	{
		cmsg_format(TERM_L_BLUE, "%s speaks to you:", deity_info[p_ptr->pgod].name);

		/* Is it the last piece of the relic? */
		if (cquest_quests_given == MAX_NUM_GOD_QUESTS())
		{
			cmsg_print(TERM_YELLOW, "'At last! Thou hast found all of the relic pieces.");

			/* reward player by increasing prayer skill */
			cmsg_print(TERM_YELLOW, "Thou hast done exceptionally well! I shall increase thy prayer skill even more!'");
			s_info[SKILL_PRAY].value += (10 * s_info[SKILL_PRAY].mod);
		}
		else
		{
			cmsg_print(TERM_YELLOW, "'Well done! Thou hast found part of the relic.");
			cmsg_print(TERM_YELLOW, "I shall surely ask thee to find more of it later!");
			cmsg_print(TERM_YELLOW, "I will take it from thee for now'");

			/* reward player by increasing prayer skill */
			cmsg_print(TERM_YELLOW, "'As a reward, I shall teach thee how to pray better'");
			s_info[SKILL_PRAY].value += (5 * s_info[SKILL_PRAY].mod);
		}

		/* Take the relic piece */
		inc_stack_size_ex(item, -1, OPTIMIZE, NO_DESCRIBE);

		/* relic piece has been identified */
		o_ptr->pval = TRUE;
		cquest_relics_found = cquest_relics_found + 1;

		/* Make sure quests can be given again if neccesary */
		cquest.status = QUEST_STATUS_UNTAKEN;

		/* Prevent further processing of 'take' action; we've
		   destroyed the item. */
		return TRUE;
	}

	return FALSE;
}

static bool_ quest_god_char_dump_hook(char *fmt)
{
	if (cquest_quests_given > 0)
	{
		int relics = cquest_relics_found;
		char relics_text[128];
		cptr append_text = "";

		snprintf(relics_text, sizeof(relics_text), "%d", relics);

		if (relics == MAX_NUM_GOD_QUESTS())
		{
			strcpy(relics_text, "all");
			append_text = " and pleased your god";
		}
		else
		{
			if (relics == 0)
			{
				strcpy(relics_text, "none");
			}
			if (cquest.status == QUEST_STATUS_FAILED)
			{
				append_text = " and failed in your quest";
			}
		}

		fprintf(hook_file, "\n You found %s of the relic pieces%s.",
			relics_text,
			append_text);
	}

	return FALSE;
}

static void set_god_dungeon_attributes()
{
	/* dungeon properties altered according to which god player is worshipping, */
	if (p_ptr->pgod == GOD_ERU)
	{
		quest_god_set_god_dungeon_attributes_eru();
	}
	else if (p_ptr->pgod == GOD_MANWE)
	{
		quest_god_set_god_dungeon_attributes_manwe();
	}
	else if (p_ptr->pgod == GOD_TULKAS)
	{
		quest_god_set_god_dungeon_attributes_tulkas();
	}
	else if (p_ptr->pgod == GOD_MELKOR)
	{
		quest_god_set_god_dungeon_attributes_melkor();
	}
	else if (p_ptr->pgod == GOD_YAVANNA)
	{
		quest_god_set_god_dungeon_attributes_yavanna();
	}
	else if (p_ptr->pgod == GOD_AULE)
	{
		quest_god_set_god_dungeon_attributes_aule();
	}
	else if (p_ptr->pgod == GOD_VARDA)
	{
		quest_god_set_god_dungeon_attributes_varda();
	}
	else if (p_ptr->pgod == GOD_ULMO)
	{
		quest_god_set_god_dungeon_attributes_ulmo();
	}
	else if (p_ptr->pgod == GOD_MANDOS)
	{
		quest_god_set_god_dungeon_attributes_mandos();
	}
	else
	{
		assert(FALSE);		/* Uh, oh! */
	}

	/* W: All dungeons are 5 levels deep, and created at 2/3 of
	 * the player clvl when the quest is given */
	{
		dungeon_info_type *d_ptr = &d_info[DUNGEON_GOD];
		d_ptr->mindepth = cquest_dun_mindepth;
		d_ptr->maxdepth = cquest_dun_maxdepth;
		d_ptr->min_plev = cquest_dun_minplev;
	}
}

static void quest_god_dungeon_setup(int d_idx)
{
	/* call the function to set the dungeon variables (dependant
	 * on pgod) the first time we enter the dungeon */
	if (d_idx != DUNGEON_GOD)
	{
		return;
	}

	set_god_dungeon_attributes();
}

static bool_ quest_god_enter_dungeon_hook(char *fmt)
{
	s32b d_idx = get_next_arg(fmt);
	quest_god_dungeon_setup(d_idx);
	return FALSE;
}

static bool_ quest_god_gen_level_begin_hook(char *fmt)
{
	quest_god_dungeon_setup(dungeon_type);
	return FALSE;
}

static bool_ quest_god_stair_hook(char *fmt)
{
	quest_god_dungeon_setup(dungeon_type);
	return FALSE;
}

static bool_ quest_god_birth_objects_hook(char *fmt)
{
	cquest_quests_given = 0;
	cquest_relics_found = 0;
	cquest_dun_mindepth = 1;
	cquest_dun_maxdepth = 4;
	cquest_dun_minplev = 0;
	cquest_relic_gen_tries = 0;
	cquest_relic_generated = FALSE;

	return FALSE;
}

bool_ quest_god_init_hook(int q)
{
	/* Only need hooks if the quest is unfinished. */
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) &&
	    (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_LEVEL_END_GEN,   quest_god_level_end_gen_hook,   "q_god_level_end_gen");
		add_hook(HOOK_ENTER_DUNGEON,   quest_god_enter_dungeon_hook,   "q_god_enter_dungeon");
		add_hook(HOOK_GEN_LEVEL_BEGIN, quest_god_gen_level_begin_hook, "q_god_gen_level_begin");
		add_hook(HOOK_STAIR,           quest_god_stair_hook,           "q_god_hook_stair");
		add_hook(HOOK_GET,             quest_god_get_hook,             "q_god_get");
		add_hook(HOOK_CHAR_DUMP,       quest_god_char_dump_hook,       "q_god_char_dump");
		add_hook(HOOK_PLAYER_LEVEL,    quest_god_player_level_hook,    "q_god_player_level");
	}

	/* Need this to re-initialize at birth; the quest data is
	 * zeroed which isn't quite right. */
	add_hook(HOOK_BIRTH_OBJECTS, quest_god_birth_objects_hook, "q_god_birth_objects");
	
	return FALSE;
}
