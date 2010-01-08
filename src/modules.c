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

#ifdef PRIVATE_USER_PATH

static void module_reset_dir_aux(cptr *dir, cptr new_path)
{
	char buf[1024];

	/* Build the new path */
	strnfmt(buf, sizeof (buf), "%s%s%s", dir, PATH_SEP, new_path);

	string_free(*dir);
	*dir = string_make(buf);

	/* Make it if needed */
	if (!private_check_user_directory(*dir))
		quit(format("Unable to create module dir %s\n", *dir));
}

#endif

void module_reset_dir(cptr dir, cptr new_path)
{
	cptr *d = 0;
	char buf[1025];

	if (!strcmp(dir, "apex")) d = &ANGBAND_DIR_APEX;
	if (!strcmp(dir, "bone")) d = &ANGBAND_DIR_BONE;
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
#ifndef PRIVATE_USER_PATH
	if (!strcmp(dir, "save")) d = &ANGBAND_DIR_SAVE;
#else /* PRIVATE_USER_PATH */
	if (
#ifdef PRIVATE_USER_PATH_APEX
	    !strcmp(dir, "apex") ||
#endif
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

		/* Tell the savefile code that we must not use setuid */
		savefile_setuid = FALSE;
	}
	else
#endif /* PRIVATE_USER_PATH */
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
	cptr name;

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

		call_lua("get_module_name", "(d)", "s", i, &name);
		strnfmt(buf, 40, "%c%c%c %s", pre, ind, post, name);

		if (sel == i)
		{
			call_lua("get_module_desc", "(d)", "s", i, &name);
			print_desc_aux(name, 5, 0);

			c_put_str(TERM_L_BLUE, buf, 10 + (i / 4), 20 * (i % 4));
		}
		else
			put_str(buf, 10 + (i / 4), 20 * (i % 4));
	}
}

static void activate_module()
{
	/* Initialize the module table */
	call_lua("assign_current_module", "(s)", "", game_module);

	/* Do misc inits  */
	call_lua("get_module_info", "(s)", "d", "max_plev", &max_plev);
	call_lua("get_module_info", "(s)", "d", "death_dungeon", &DUNGEON_DEATH);

	call_lua("get_module_info", "(s)", "d", "random_artifact_weapon_chance", &RANDART_WEAPON);
	call_lua("get_module_info", "(s)", "d", "random_artifact_armor_chance", &RANDART_ARMOR);
	call_lua("get_module_info", "(s)", "d", "random_artifact_jewelry_chance", &RANDART_JEWEL);

	call_lua("get_module_info", "(s,d)", "d", "version", 1, &VERSION_MAJOR);
	call_lua("get_module_info", "(s,d)", "d", "version", 2, &VERSION_MINOR);
	call_lua("get_module_info", "(s,d)", "d", "version", 3, &VERSION_PATCH);
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

/* Did the player force a module on command line */
cptr force_module = NULL;

/* Display possible modules and select one */
bool select_module()
{
	s32b k, sel, max;

	/* Hack */
	use_color = TRUE;

	/* Init some lua */
	init_lua();

	/* Some ports need to separate the module scripts from the installed mods,
	   so we need to check for these in two different places */
	if(!tome_dofile_anywhere(ANGBAND_DIR_CORE, "mods_aux.lua", FALSE))
		tome_dofile_anywhere(ANGBAND_DIR_MODULES, "mods_aux.lua", TRUE);
	if(!tome_dofile_anywhere(ANGBAND_DIR_CORE, "modules.lua", FALSE))
		tome_dofile_anywhere(ANGBAND_DIR_MODULES, "modules.lua", TRUE);

	/* Grab the savefiles */
	call_lua("max_modules", "()", "d", &max);

	/* No need to bother the player if there is only one module */
	sel = -1;
	if (force_module)
		call_lua("find_module", "(s)", "d", force_module, &sel);
	if (max == 1)
		sel = 0;
	if (sel != -1)
	{
		cptr tmp;

		/* Process the module */
		call_lua("init_module", "(d)", "", sel);
		call_lua("get_module_name", "(d)", "s", sel, &tmp);
		game_module = string_make(tmp);

		activate_module();

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
			cptr tmp;

			if (islower(k)) x = A2I(k);
			else x = A2I(tolower(k)) + 26;

			if ((x < 0) || (x >= max)) continue;

			/* Process the module */
			call_lua("init_module", "(d)", "", x);
			call_lua("get_module_name", "(d)", "s", x, &tmp);
			game_module = string_make(tmp);

			activate_module();

			return (FALSE);
		}
	}

	/* Shouldnt happen */
	return (FALSE);
}
