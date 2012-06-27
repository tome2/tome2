/* File: modules.c */

/* Purpose: T-engine modules */

/*
 * Copyright (c) 2003 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

static void module_reset_dir_aux(cptr *dir, cptr new_path)
{
	char buf[1024];

	/* Build the new path */
	strnfmt(buf, sizeof (buf), "%s%s%s", *dir, PATH_SEP, new_path);

	string_free(*dir);
	*dir = string_make(buf);

	/* Make it if needed */
	if (!private_check_user_directory(*dir))
		quit(format("Unable to create module dir %s\n", *dir));
}

void module_reset_dir(cptr dir, cptr new_path)
{
	cptr *d = 0;
	char buf[1025];

	if (!strcmp(dir, "core")) d = &ANGBAND_DIR_CORE;
	if (!strcmp(dir, "dngn")) d = &ANGBAND_DIR_DNGN;
	if (!strcmp(dir, "data")) d = &ANGBAND_DIR_DATA;
	if (!strcmp(dir, "edit")) d = &ANGBAND_DIR_EDIT;
	if (!strcmp(dir, "file")) d = &ANGBAND_DIR_FILE;
	if (!strcmp(dir, "help")) d = &ANGBAND_DIR_HELP;
	if (!strcmp(dir, "info")) d = &ANGBAND_DIR_INFO;
	if (!strcmp(dir, "scpt")) d = &ANGBAND_DIR_SCPT;
	if (!strcmp(dir, "patch")) d = &ANGBAND_DIR_PATCH;
	if (!strcmp(dir, "pref")) d = &ANGBAND_DIR_PREF;
	if (!strcmp(dir, "xtra")) d = &ANGBAND_DIR_XTRA;
	if (!strcmp(dir, "user")) d = &ANGBAND_DIR_USER;
	if (!strcmp(dir, "note")) d = &ANGBAND_DIR_NOTE;
	if (!strcmp(dir, "cmov")) d = &ANGBAND_DIR_CMOV;
	if (
	    !strcmp(dir, "user") ||
	    !strcmp(dir, "note") ||
	    !strcmp(dir, "cmov"))
	{
		char user_path[1024];
		/* copied from init_file_paths */
		path_parse(user_path, 1024, PRIVATE_USER_PATH);
		strcat(user_path, USER_PATH_VERSION);
		strnfmt(buf, 1024, "%s%s%s", user_path, PATH_SEP, new_path);
		string_free(*d);
		*d = string_make(buf);
		// Make it if needed */
		if (!private_check_user_directory(*d))
		{
			quit(format("Unable to create module dir %s\n", *d));
		}
	}
#ifdef PRIVATE_USER_PATH_DATA
	else if (!strcmp(dir, "data"))
	{
		module_reset_dir_aux(&ANGBAND_DIR_DATA, new_path);
	}
#endif
	else if (!strcmp(dir, "save"))
	{
		module_reset_dir_aux(&ANGBAND_DIR_SAVE, new_path);
	}
	else
	{
		/* Build the new path */
		strnfmt(buf, 1024, "%s%s%s%s%s", ANGBAND_DIR_MODULES, PATH_SEP, new_path, PATH_SEP, dir);

		string_free(*d);
		*d = string_make(buf);
	}
}

static void dump_modules(int sel, int max)
{
	int i;

	char buf[40], pre = ' ', post = ')';

	char ind;


	for (i = 0; i < max; i++)
	{
		ind = I2A(i % 26);
		if (i >= 26) ind = toupper(ind);

		if (sel == i)
		{
			pre = '[';
			post = ']';
		}
		else
		{
			pre = ' ';
			post = ')';
		}

		strnfmt(buf, 40, "%c%c%c %s", pre, ind, post, modules[i].meta.name);

		if (sel == i)
		{
			print_desc_aux(modules[i].meta.desc, 5, 0);

			c_put_str(TERM_L_BLUE, buf, 10 + (i / 4), 20 * (i % 4));
		}
		else
			put_str(buf, 10 + (i / 4), 20 * (i % 4));
	}
}

static void activate_module(int module_idx)
{
	module_type *module_ptr = &modules[module_idx];

	/* Initialize the module table */
	game_module_idx = module_idx;

	/* Do misc inits  */
	max_plev = module_ptr->max_plev;

	RANDART_WEAPON = module_ptr->randarts.weapon_chance;
	RANDART_ARMOR = module_ptr->randarts.armor_chance;
	RANDART_JEWEL = module_ptr->randarts.jewelry_chance;

	VERSION_MAJOR = module_ptr->meta.version.major;
	VERSION_MINOR = module_ptr->meta.version.minor;
	VERSION_PATCH = module_ptr->meta.version.patch;
	version_major = VERSION_MAJOR;
	version_minor = VERSION_MINOR;
	version_patch = VERSION_PATCH;

	/* Change window name if needed */
	if (strcmp(game_module, "ToME"))
	{
		strnfmt(angband_term_name[0], 79, "T-Engine: %s", game_module);
		Term_xtra(TERM_XTRA_RENAME_MAIN_WIN, 0);
	}

	/* Reprocess the player name, just in case */
	process_player_base();
}

static void init_module(module_type *module_ptr)
{
	/* Set up module directories? */
	cptr dir = module_ptr->meta.module_dir;
	if (dir) {
		module_reset_dir("core", dir);
		module_reset_dir("data", dir);
		module_reset_dir("dngn", dir);
		module_reset_dir("edit", dir); 
		module_reset_dir("file", dir);
		module_reset_dir("help", dir);
		module_reset_dir("note", dir);
		module_reset_dir("save", dir);
		module_reset_dir("scpt", dir);
		module_reset_dir("user", dir);
		module_reset_dir("pref", dir);
	}
}

bool_ module_savefile_loadable(cptr savefile_mod)
{
	return (strcmp(savefile_mod, modules[game_module_idx].meta.save_file_tag) == 0);
}

/* Did the player force a module on command line */
cptr force_module = NULL;

/* Find module index by name. Returns -1 if matching module not found */
int find_module(cptr name)
{
	int i = 0;

	for (i=0; i<MAX_MODULES; i++)
	{
		if (streq(name, modules[i].meta.name))
		{
			return i;
		}
	}

	return -1;
}

/* Display possible modules and select one */
bool_ select_module()
{
	s32b k, sel, max;

	/* How many modules? */
	max = MAX_MODULES;

	/* No need to bother the player if there is only one module */
	sel = -1;
	if (force_module) {
		/* Find module by name */
		sel = find_module(force_module);
	}
	/* Only a single choice */
	if (max == 1) {
		sel = 0;
	}
	/* No module selected */
	if (sel != -1)
	{
		/* Process the module */
		init_module(&modules[sel]);
		game_module = string_make(modules[sel].meta.name);

		activate_module(sel);

		return FALSE;
	}

	sel = 0;

	/* Preprocess the basic prefs, we need them to have movement keys */
	process_pref_file("pref.prf");

	while (TRUE)
	{
		/* Clear screen */
		Term_clear();

		/* Let the user choose */
		c_put_str(TERM_YELLOW, "Welcome to ToME, you must select a module to play,", 1, 12);
		c_put_str(TERM_YELLOW, "either ToME official module or third party ones.", 2, 13);
		put_str("Press 8/2/4/6 to move, Return to select and Esc to quit.", 4, 3);

		dump_modules(sel, max);

		k = inkey();

		if (k == ESCAPE)
		{
			quit(NULL);
		}
		if (k == '6')
		{
			sel++;
			if (sel >= max) sel = 0;
			continue;
		}
		else if (k == '4')
		{
			sel--;
			if (sel < 0) sel = max - 1;
			continue;
		}
		else if (k == '2')
		{
			sel += 4;
			if (sel >= max) sel = sel % max;
			continue;
		}
		else if (k == '8')
		{
			sel -= 4;
			if (sel < 0) sel = (sel + max - 1) % max;
			continue;
		}
		else if (k == '\r')
		{
			if (sel < 26) k = I2A(sel);
			else k = toupper(I2A(sel));
		}

		{
			int x;

			if (islower(k)) x = A2I(k);
			else x = A2I(tolower(k)) + 26;

			if ((x < 0) || (x >= max)) continue;

			/* Process the module */
			init_module(&modules[x]);
			game_module = string_make(modules[x].meta.name);

			activate_module(x);

			return (FALSE);
		}
	}

	/* Shouldnt happen */
	return (FALSE);
}

static bool_ dleft(byte c, cptr str, int y, int o)
{
	int i = strlen(str);
	int x = 39 - (strlen(str) / 2) + o;
	while (i > 0)
	{
		int a = 0;
		int time = 0;

		if (str[i-1] != ' ')
		{
			while (a < x + i - 1)
			{
				Term_putch(a - 1, y, c, 32);
				Term_putch(a, y, c, str[i-1]);
				time = time + 1;
				if (time >= 4)
				{
					Term_xtra(TERM_XTRA_DELAY, 1);
					time = 0;
				}
				Term_redraw_section(a - 1, y, a, y);
				a = a + 1;

				inkey_scan = TRUE;
				if (inkey()) {
					return TRUE;
				}
			}
		}

		i = i - 1;
	}
	return FALSE;
}

static bool_ dright(byte c, cptr str, int y, int o)
{
	int x = 39 - (strlen(str) / 2) + o;
	int i = 1;
	while (i <= strlen(str))
	{
		int a = 79;
		int time = 0;

		if (str[i-1] != ' ') {
			while (a >= x + i - 1)
			{
				Term_putch(a + 1, y, c, 32);
				Term_putch(a, y, c, str[i-1]);
				time = time + 1;
				if (time >= 4) {
					Term_xtra(TERM_XTRA_DELAY, 1);
					time = 0;
				}
				Term_redraw_section(a, y, a + 1, y);
				a = a - 1;

				inkey_scan = TRUE;
				if (inkey()) {
					return TRUE;
				}
			}
		}

		i = i + 1;
	}
	return FALSE;
}

typedef struct intro_text intro_text;
struct intro_text
{
	bool_ (*drop_func)(byte, cptr, int, int);
	byte color;
	cptr text;
	int y0;
	int x0;
};

static bool_ show_intro(intro_text intro_texts[])
{
	int i = 0;

	Term_clear();
	for (i = 0; ; i++)
	{
		intro_text *it = &intro_texts[i];
		if (it->drop_func == NULL)
		{
			break;
		}
		else if (it->drop_func(it->color, it->text, it->y0, it->x0))
		{
			/* Abort */
			return TRUE;
		}
	}

	/* Wait for key */
	Term_putch(0, 0, TERM_DARK, 32);
	inkey_scan = FALSE;
	inkey();

	/* Continue */
	return FALSE;
}

void tome_intro()
{
	intro_text intro1[] =
	{
		{ dleft , TERM_L_BLUE, "Art thou an adventurer,", 10, 0, },
		{ dright, TERM_L_BLUE, "One who passes through the waterfalls we call danger", 11, -1, },
		{ dleft , TERM_L_BLUE, "to find the true nature of the legends beyond them?", 12, 0, },
		{ dright, TERM_L_BLUE, "If this is so, then seeketh me.", 13, -1, },
		{ dleft , TERM_WHITE , "[Press any key to continue]", 23, -1, },
		{ NULL, }
	};
	intro_text intro2[] =
	{
		{ dleft , TERM_L_BLUE , "DarkGod", 8, 0, },
		{ dright, TERM_WHITE  , "in collaboration with", 9, -1, },
		{ dleft , TERM_L_GREEN, "Eru Iluvatar,", 10, 0, },
		{ dright, TERM_L_GREEN, "Manwe", 11, -1, },
		{ dleft , TERM_WHITE  , "and", 12, 0, },
		{ dright, TERM_L_GREEN, "All the T.o.M.E. contributors(see credits.txt)", 13, -1, },
		{ dleft , TERM_WHITE  , "present", 15, 1, },
		{ dright, TERM_YELLOW , "T.o.M.E.", 16, 0, },
		{ dleft , TERM_WHITE  , "[Press any key to continue]", 23, -1, },
		{ NULL, }
	};

	screen_save();

	/* Intro 1 */
	if (show_intro(intro1))
	{
		goto exit;
	}

	/* Intro 2 */
	if (show_intro(intro2))
	{
		goto exit;
	}

exit:
	screen_load();
}

void theme_intro()
{
	struct intro_text intro1[] =
	{
		{ dleft , TERM_L_BLUE , "Three Rings for the Elven-kings under the sky,", 10, 0, },
		{ dright, TERM_L_BLUE , "Seven for the Dwarf-lords in their halls of stone,", 11, -1, },
		{ dleft , TERM_L_BLUE , "Nine for Mortal Men doomed to die,", 12, 0, },
		{ dright, TERM_L_BLUE , "One for the Dark Lord on his dark throne", 13, -1, },
		{ dleft , TERM_L_BLUE , "In the land of Mordor, where the Shadows lie.", 14, 0, },
		{ dright, TERM_L_BLUE , "One Ring to rule them all, One Ring to find them,", 15, -1, },
		{ dleft , TERM_L_BLUE , "One Ring to bring them all and in the darkness bind them", 16, 0, },
		{ dright, TERM_L_BLUE , "In the land of Mordor, where the Shadows lie.", 17, -1, },
		{ dright, TERM_L_GREEN, "--J.R.R. Tolkien", 18, 0, },
		{ dleft , TERM_WHITE  , "[Press any key to continue]", 23, -1, },
		{ NULL, },
	};
	struct intro_text intro2[] =
	{
		{ dleft , TERM_L_BLUE , "furiosity", 8, 0, },
		{ dright, TERM_WHITE  , "in collaboration with", 9, -1, },
		{ dleft , TERM_L_GREEN, "DarkGod and all the ToME contributors,", 10, 0, },
		{ dright, TERM_L_GREEN, "module creators, t-o-m-e.net forum posters,", 11, -1, },
		{ dleft , TERM_WHITE  , "and", 12, 0, },
		{ dright, TERM_L_GREEN, "by the grace of the Valar", 13, -1, },
		{ dleft , TERM_WHITE  , "present", 15, 1, },
		{ dright, TERM_YELLOW , "Theme (a module for ToME)", 16, 0, },
		{ dleft , TERM_WHITE  , "[Press any key to continue]", 23, -1, },
		{ NULL, },
	};
	
       	screen_save();
 
	/* Intro 1 */
	if (show_intro(intro1))
	{
		goto exit;
	}

	/* Intro 2 */
	if (show_intro(intro2))
	{
		goto exit;
	}

exit:
	screen_load();
}

static bool_ auto_stat_gain_hook(void *data, void *in, void *out)
{
	while (p_ptr->last_rewarded_level * 5 <= p_ptr->lev)
	{
		do_inc_stat(A_STR);
		do_inc_stat(A_INT);
		do_inc_stat(A_WIS);
		do_inc_stat(A_DEX);
		do_inc_stat(A_CON);
		do_inc_stat(A_CHR);

		p_ptr->last_rewarded_level += 1;
	}

	return FALSE;
}

static bool_ drunk_takes_wine(void *data, void *in_, void *out)
{
	hook_give_in *in = (hook_give_in *) in_;
	monster_type *m_ptr = &m_list[in->m_idx];
	object_type *o_ptr = get_object(in->item);

	if ((m_ptr->r_idx == test_monster_name("Singing, happy drunk")) &&
	    (o_ptr->tval == TV_FOOD) &&
	    ((o_ptr->sval == 38) ||
	     (o_ptr->sval == 39)))
	{
		cmsg_print(TERM_YELLOW, "'Hic!'");

		/* Destroy item */
		inc_stack_size_ex(in->item, -1, OPTIMIZE, NO_DESCRIBE);

		/* Create empty bottle */
		{
			object_type forge;
			object_prep(&forge, lookup_kind(TV_BOTTLE,1));
			drop_near(&forge, 50, p_ptr->py, p_ptr->px);
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}

static bool_ hobbit_food(void *data, void *in_, void *out)
{
	hook_give_in *in = (hook_give_in *) in_;
	monster_type *m_ptr = &m_list[in->m_idx];
	object_type *o_ptr = get_object(in->item);

	if ((m_ptr->r_idx == test_monster_name("Scruffy-looking hobbit")) &&
	    (o_ptr->tval == TV_FOOD))
	{
		cmsg_print(TERM_YELLOW, "'Yum!'");

		inc_stack_size_ex(in->item, -1, OPTIMIZE, NO_DESCRIBE);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static bool_ smeagol_ring(void *data, void *in_, void *out)
{
	hook_give_in *in = (hook_give_in *) in_;
	monster_type *m_ptr = &m_list[in->m_idx];
	object_type *o_ptr = get_object(in->item);

	if ((m_ptr->r_idx == test_monster_name("Smeagol")) &&
	    (o_ptr->tval == TV_RING))
	{
		cmsg_print(TERM_YELLOW, "'MY... PRECIOUSSSSS!!!'");

		inc_stack_size_ex(in->item, -1, OPTIMIZE, NO_DESCRIBE);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static bool_ longbottom_leaf(void *data, void *in_, void *out_)
{
	hook_eat_in *in = (hook_eat_in *) in_;

	if ((in->o_ptr->tval == TV_FOOD) &&
	    (in->o_ptr->sval == 45))
	{
		msg_print("What a stress reliever!");
		heal_insanity(1000);
		return TRUE;
	}

	return FALSE;
}

static bool_ food_vessel(void *data, void *in_, void *out)
{
	hook_eat_in *in = (hook_eat_in *) in_;

	if (((in->o_ptr->tval == TV_FOOD) && (in->o_ptr->sval == 43)) ||
	    ((in->o_ptr->tval == TV_FOOD) && (in->o_ptr->sval == 44)))
	{
		object_type forge;

		object_prep(&forge, lookup_kind(TV_JUNK, 3));

		forge.ident |= IDENT_MENTAL | IDENT_KNOWN;
		inven_carry(&forge, FALSE);

		return TRUE;
	}

	return FALSE;
}

/*
 * Player must have appropriate keys to enter Erebor.
 */
static bool_ erebor_stair(void *data, void *in_, void *out_)
{
	hook_stair_in *in = (hook_stair_in *) in_;
	hook_stair_out *out = (hook_stair_out *) out_;

	if ((dungeon_type == 20) &&
	    (dun_level == 60) &&
	    (in->direction == STAIRS_DOWN))
	{
		int i, keys;

		keys = 0;
		for (i = 0; i < INVEN_TOTAL - 1; i++)
		{
			if ((p_ptr->inventory[i].name1 == 209) ||
			    (p_ptr->inventory[i].name1 == 210))
			{
				keys += 1;
			}
		}

		if (keys >= 2)
		{
			msg_print("The moon-letters on the map show you "
				  "the keyhole! You use the key to enter.");
			out->allow = TRUE;
		}
		else
		{
			msg_print("You have found a door, but you cannot "
				  "find a way to enter. Ask in Dale, perhaps?");
			out->allow = FALSE;
		}
	}

	return FALSE;
}

/*
 * Orthanc requires a key.
 */
static bool_ orthanc_stair(void *data, void *in_, void *out_)
{
	hook_stair_in *in = (hook_stair_in *) in_;
	hook_stair_out *out = (hook_stair_out *) out_;

	if ((dungeon_type == 36) &&
	    (dun_level == 39) &&
	    (in->direction == STAIRS_DOWN))
	{
		int i, keys;

		keys = 0;
		for (i = 0; i < INVEN_TOTAL - 1; i++)
		{
			if (p_ptr->inventory[i].name1 == 15)
			{
				keys += 1;
			}
		}

		if (keys >= 1)
		{
			msg_print("#BYou have the key to the tower of Orthanc! You may proceed.#w");
			out->allow = TRUE;
		}
		else
		{
			msg_print("#yYou may not enter Orthanc without the key to the gates!#w Rumours say the key was lost in the Mines of Moria...");
			out->allow = FALSE;
		}
	}

	return FALSE;
}

/*
 * Movement from Theme
 */
bool_ theme_push_past(void *data, void *in_, void *out_)
{
	hook_move_in *p = (hook_move_in *) in_;
	cave_type *c_ptr = &cave[p->y][p->x];

	if (c_ptr->m_idx > 0)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];
		monster_race *mr_ptr = race_inf(m_ptr);

		if (m_ptr->status >= MSTATUS_NEUTRAL)
		{
			if ((cave_floor_bold(p->y, p->x) == TRUE) ||
			    (mr_ptr->flags2 == RF2_PASS_WALL))
			{
				char buf[128];

				monster_desc(buf, m_ptr, 0);
				msg_print(format("You push past %s.", buf));

				m_ptr->fy = p_ptr->py;
				m_ptr->fx = p_ptr->px;
				cave[p_ptr->py][p_ptr->px].m_idx = c_ptr->m_idx;
				c_ptr->m_idx = 0;
			}
			else
			{
				char buf[128];

				monster_desc(buf, m_ptr, 0);
				msg_print(format("%s is in your way!", buf));
				energy_use = 0;

				return TRUE;
			}
		}
	}

	return FALSE;
}

/*
 * Check if monster race is in list. The list is terminated
 * with a -1.
 */
static bool_ race_in_list(int r_idx, int race_idxs[])
{
	int i;

	for (i = 0; race_idxs[i] >= 0; i++)
	{
		if (r_idx == race_idxs[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Monster racial alignment from Theme.
 */
static s16b *compute_monster_status(int r_idx)
{
	static s16b FRIEND_ = MSTATUS_FRIEND;
	static s16b *FRIEND = &FRIEND_;
	static s16b NEUTRAL_ = MSTATUS_NEUTRAL;
	static s16b *NEUTRAL = &NEUTRAL_;

	object_type *o_ptr = NULL;

	switch (p_ptr->prace)
	{
	case RACE_MAIA:
	{
		int good_race_idxs[] = {
			25, 29, 45, 97, 109,
			147, 225, 335, 346, 443,
			581, 629, 699, 853, 984,
			1007, 1017, -1
		};

		if (!(player_has_corruption(CORRUPT_BALROG_AURA)) &&
		    !(player_has_corruption(CORRUPT_BALROG_WINGS)) &&
		    !(player_has_corruption(CORRUPT_BALROG_STRENGTH)) &&
		    !(player_has_corruption(CORRUPT_BALROG_FORM)) &&
		    race_in_list(r_idx, good_race_idxs))
		{
			/* Good beings (except swans, GWoPs, Wyrm
			 * Spirits, and some joke uniques) are
			 * coaligned with Maiar */
			return FRIEND;
		}

		break;
	}

	case RACE_HUMAN:
	case RACE_DUNADAN:
	case RACE_DRUADAN:
	case RACE_ROHANKNIGHT:
	{
		int nonevil_humanoid_race_idxs[] = {
			43, 45, 46, 83, 93,
			97, 109, 110, 142, 147,
			216, 225, 293, 345, 346,
			693, 699, 937, 988, 997,
			998, 1000, -1
		};
		
		if (race_in_list(r_idx, nonevil_humanoid_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_ELF:
	case RACE_HOBBIT:
	case RACE_WOOD_ELF:
	{
		int nonevil_sentient_race_idxs[] = {
			43, 45, 46, 83, 93,
			97, 109, 110, 142, 147,
			216, 225, 293, 345, 346,
			693, 699, 937, 988, 997,
			998, 1000, 74, 103, 882,
			1017, -1
		};

		if (race_in_list(r_idx, nonevil_sentient_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_GNOME:
	{
		int gnomish_race_idxs[] = {
			103, 281, 680, 984, 1001,
			1003, 1007, 1011, 1014, 1016,
			-1
		};

		if (race_in_list(r_idx, gnomish_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_DWARF:
	case RACE_PETTY_DWARF:
	{
		int dwarvish_race_idxs[] = {
			111, 112, 179, 180, 181,
			182, -1
		};

		if (race_in_list(r_idx, dwarvish_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_ORC:
	{
		int low_orc_race_idxs[] = {
			87, 118, 126, 149, 244,
			251, 264, -1
		};

		if ((p_ptr->pgod == GOD_MELKOR) &&
		    race_in_list(r_idx, low_orc_race_idxs))
		{
			return FRIEND;
		}

		break;
	}

	case RACE_TROLL:
	{
		int low_troll_race_idxs[] = {
			297, 401, 403, 424, 454,
			491, 496, 509, 538, -1
		};

		if ((p_ptr->pgod == GOD_MELKOR) &&
		    race_in_list(r_idx, low_troll_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_HALF_OGRE:
	{
		int ogre_race_idxs[] = {
			262, 285, 415, 430, 479,
			745, 918, -1
		};

		if (race_in_list(r_idx, ogre_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_BEORNING:
	{
		/* Bears; not werebears. */
		int bear_race_idxs[] = {
			160, 173, 191, 854,
			855, 867, 873, -1
		};

		if (race_in_list(r_idx, bear_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	case RACE_DARK_ELF:
	{
		int dark_elven_race_idxs[] = {
			122, 178, 183, 226, 348,
			375, 400, 657, -1
		};

		if (race_in_list(r_idx, dark_elven_race_idxs))
		{
			return FRIEND;
		}

		break;
	}

	case RACE_ENT:
	{
		int plant_race_idxs[] = {
			248, 266, 317, 329, 396,
			-1
		};

		if (race_in_list(r_idx, plant_race_idxs))
		{
			return FRIEND;
		}

		/* And since the above is largely useless except out
		   in the wild...  If an Ent worships Yavanna,
		   lower-level animals are coaligned should make the
		   early game a bit easier for Ents. */

		if (p_ptr->pgod == GOD_YAVANNA)
		{
			int lower_animal_race_idxs[] = {
				 21,  23,  24,  25,  26,
				 27,  28,  29,  30,  31,
				 33,  35,  36,  37,  38,
				 39,  41,  49,  50,  52,
				 56,  57,  58,  59,  60,
				 61,  62,  69,  70,  75,
				 77,  78,  79,  86,  88,
				 89,  90,  95,  96, 105,
				106, 114, 119, 120, 121,
				123, 127, 134, 141, 143,
				151, 154, 155, 156, 160,
				161, 168, 171, 173, 174,
				175, 176, 187, 191, 196,
				197, 198, 210, 211, 213,
				230, 236, 250, 259, -1
			};

			if (race_in_list(r_idx, lower_animal_race_idxs))
			{
				return FRIEND;
			}
		}

		break;
	}

	case RACE_EAGLE:
	{
		int nonevil_nonneurtal_bird_race_idxs[] = {
			61, 141, 151, 279, -1
		};

		if (race_in_list(r_idx, nonevil_nonneurtal_bird_race_idxs))
		{
			return FRIEND;
		}

		break;
	}

	case RACE_DRAGON:
	{
		int hatchling_dragon_race_idxs[] = {
			163, 164, 165, 166, 167,
			204, 218, 219, 911, -1
		};

		if (race_in_list(r_idx, hatchling_dragon_race_idxs))
		{
			return FRIEND;
		}

		break;
	}

	case RACE_YEEK:
	{
		int yeek_race_idxs[] = {
			580, 583, 594, 653, 655,
			659, 661, -1
		};

		if (race_in_list(r_idx, yeek_race_idxs))
		{
			return NEUTRAL;
		}

		break;
	}

	};

	/* Oathbreakers are coaligned if player is wielding Anduril.
	   It's dirty, but it works, and it doesn't bother checking
	   demons and the races who can't wield weapons. */
	o_ptr = get_object(INVEN_WIELD);
	if (o_ptr != NULL &&
	    o_ptr->name1 == ART_ANDURIL)
	{
		switch (p_ptr->prace)
		{
		case RACE_HUMAN:
		case RACE_HALF_ELF:
		case RACE_ELF:
		case RACE_HOBBIT:
		case RACE_GNOME:
		case RACE_DWARF:
		case RACE_ORC:
		case RACE_TROLL:
		case RACE_DUNADAN:
		case RACE_HIGH_ELF:
		case RACE_HALF_OGRE:
		case RACE_BEORNING:
		case RACE_DRUADAN:
		case RACE_PETTY_DWARF:
		case RACE_DARK_ELF:
		case RACE_ENT:
		case RACE_ROHANKNIGHT:
		case RACE_YEEK:
		case RACE_WOOD_ELF:
		case RACE_MAIA:
		case RACE_EASTERLING:
		case RACE_DEMON:
		{
			int oathbreaker_race_idxs[] = {
				731, -1
			};

			if (race_in_list(r_idx, oathbreaker_race_idxs))
			{
				return FRIEND;
			}

			break;
		}
		}
	}

	/* No status override */
	return NULL;
}

static bool_ theme_level_end_gen(void *data, void *in, void *out)
{
	int i = 0;

	for (i = 0; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		int r_idx = m_ptr->r_idx;
		s16b *status = compute_monster_status(r_idx);
		if (status)
		{
			m_ptr->status = *status;
		}
	}

	return FALSE;
}

static bool_ theme_new_monster_end(void *data, void *in_, void *out)
{
	hook_new_monster_end_in *in = (hook_new_monster_end_in *) in_;
	s16b *status = compute_monster_status(in->m_ptr->r_idx);

	if (status)
	{
		in->m_ptr->status = *status;
	}

	return FALSE;
}

void init_hooks_module()
{
	/*
	 * Common hooks
	 */
	add_hook_new(HOOK_GIVE,
		     drunk_takes_wine,
		     "drunk_takes_wine",
		     NULL);

	add_hook_new(HOOK_LEVEL_END_GEN,
		     gen_joke_monsters,
		     "gen_joke_monsters",
		     NULL);

	/*
	 * Module-specific hooks
	 */
	switch (game_module_idx)
	{
	case MODULE_TOME:
	{
		break;
	}

	case MODULE_THEME:
	{
		timer_aggravate_evil_enable();

		add_hook_new(HOOK_PLAYER_LEVEL,
			     auto_stat_gain_hook,
			     "auto_stat_gain",
			     NULL);

		add_hook_new(HOOK_GIVE,
			     hobbit_food,
			     "hobbit_food",
			     NULL);

		add_hook_new(HOOK_GIVE,
			     smeagol_ring,
			     "smeagol_ring",
			     NULL);

		add_hook_new(HOOK_EAT,
			     longbottom_leaf,
			     "longbottom_leaf",
			     NULL);

		add_hook_new(HOOK_EAT,
			     food_vessel,
			     "food_vessel",
			     NULL);

		add_hook_new(HOOK_STAIR,
			     erebor_stair,
			     "erebor_stair",
			     NULL);

		add_hook_new(HOOK_STAIR,
			     orthanc_stair,
			     "orthanc_stair",
			     NULL);

		add_hook_new(HOOK_MOVE,
			     theme_push_past,
			     "__hook_push_past",
			     NULL);

		add_hook_new(HOOK_LEVEL_END_GEN,
			     theme_level_end_gen,
			     "theme_level_end_gen",
			     NULL);

		add_hook_new(HOOK_NEW_MONSTER_END,
			     theme_new_monster_end,
			     "theme_new_monster_end",
			     NULL);

		break;
	}

	default:
		assert(FALSE);
	}
}
