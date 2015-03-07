/* File: main.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "angband.h"
#include "birth.h"
#include "dungeon.h"
#include "files.h"
#include "init2.h"
#include "modules.h"
#include "script.h"
#include "util.h"
#include "variable.h"



/*
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */


#if !defined(MACINTOSH) && !defined(WINDOWS)


/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(cptr s)
{
	int j;

	/* Scan windows */
	for (j = 8 - 1; j >= 0; j--)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Nuke it */
		term_nuke(angband_term[j]);
	}
}



/*
 * Check existence of ".ToME/" directory in the user's
 * home directory or try to create it if it doesn't exist.
 * Returns FALSE if all the attempts fail.
 */
static void init_save_dir(void)
{
	char dirpath[1024];
	char versionpath[1024];
	char savepath[1024];

	/* Get an absolute path from the filename */
	path_parse(dirpath, 1024, PRIVATE_USER_PATH);
	strcpy(versionpath, dirpath);
	strcat(versionpath, USER_PATH_VERSION);
	strcpy(savepath, versionpath);
	strcat(savepath, "/save");

	if (!private_check_user_directory(dirpath))
	{
		quit_fmt("Cannot create directory '%s'", dirpath);
	}

	if (!private_check_user_directory(versionpath))
	{
		quit_fmt("Cannot create directory '%s'", versionpath);
	}

	if (!private_check_user_directory(savepath))
	{
		quit_fmt("Cannot create directory '%s'", savepath);
	}
}


static void init_player_name()
{
	/* Get the user id (?) */
	int player_uid = getuid();

	/* Acquire the "user name" as a default player name */
	user_name(player_name, player_uid);
}



/*
 * Simple "main" function for multiple platforms.
 *
 * Note the special "--" option which terminates the processing of
 * standard options.  All non-standard options (if any) are passed
 * directly to the "init_xxx()" function.
 */
int main(int argc, char *argv[])
{
	int i;

	bool_ done = FALSE;

	bool_ new_game = FALSE;

	cptr mstr = NULL;

	bool_ args = TRUE;

	/* Get the file paths */
	init_file_paths_with_env();

	/* Initialize the player name */
	init_player_name();

	/* Make sure save directory exists */
	init_save_dir();


	/* Process the command line arguments */
	for (i = 1; args && (i < argc); i++)
	{
		/* Require proper options */
		if (argv[i][0] != '-') goto usage;

		/* Analyze option */
		switch (argv[i][1])
		{
		case 'N':
		case 'n':
			{
				new_game = TRUE;
				break;
			}

		case 'W':
		case 'w':
			{
				arg_wizard = TRUE;
				break;
			}

		case 'R':
		case 'r':
			{
				arg_force_roguelike = TRUE;
				break;
			}

		case 'O':
		case 'o':
			{
				arg_force_original = TRUE;
				break;
			}

		case 'u':
		case 'U':
			{
				if (!argv[i][2]) goto usage;
				strcpy(player_name, &argv[i][2]);
				strcpy(player_base, &argv[i][2]);
				no_begin_screen = TRUE;
				break;
			}

		case 'm':
			{
				if (!argv[i][2]) goto usage;
				mstr = &argv[i][2];
				break;
			}

		case 'M':
			{
				if (!argv[i][2]) goto usage;
				force_module = &argv[i][2];
				break;
			}

		case 'h':
			{
				goto usage;
				break;
			}

		case 'H':
			{
				char *s;
				int j;

				init_lua_init();

				for (j = i + 1; j < argc; j++)
				{
					s = argv[j];

					while (*s != '.') s++;
					*s = '\0';
					s++;
					txt_to_html("head.aux", "foot.aux", argv[j], s, FALSE, FALSE);
				}

				return 0;
			}

		case '-':
			{
				if (argv[i][2] == 'h' && !strcmp((argv[i] + 2), "help"))
					goto usage;
				else
				{
					argv[i] = argv[0];
					argc = argc - i;
					argv = argv + i;
					args = FALSE;
					break;
				}
			}

		default:
usage:
			{
				int j;

				/* Dump usage information */
				for (j = 0; j < argc; j++) printf("%s ", argv[j]);
				printf("\n");
				puts("Usage: tome [options] [-- subopts]");
				puts("  -h                 This help");
				puts("  -n                 Start a new character");
				puts("  -w                 Request wizard mode");
				puts("  -o                 Request original keyset");
				puts("  -r                 Request rogue-like keyset");
				puts("  -H <list of files> Convert helpfile to html");
				puts("  -u<who>            Use your <who> savefile");
				puts("  -M<which>            Use the <which> module");
				puts("  -m<sys>            Force 'main-<sys>.c' usage");

#ifdef USE_GTK2
				puts("  -mgtk2             To use GTK2");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -b              Turn off software backing store");
#endif /* USE_GTK2 */

#ifdef USE_X11
				puts("  -mx11              To use X11");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -d<name>        Display to use");
#endif /* USE_X11 */

#ifdef USE_GCU
				puts("  -mgcu              To use curses");
				puts("  --                 Sub options");
				puts("  -- -b              Requests big screen");
#endif /* USE_GCU */

#ifdef USE_SDL
				puts("  -msdl              To use SDL");
				puts("  --                 Sub options");
				puts("  -- -n #            Number of virtual consoles to use");
				puts("  -- -w #            Request screen width in pixels");
				puts("  -- -h #            Request screen height in pixels");
				puts("  -- -bpp #          Request screen color depth in bits");
				puts("  -- -fs             Start with full-screen display");
				puts("  -- -s #            Request font size");
				puts("  -- -f <font>       Request true-type font by name");
#endif /* USE_SDL */

				/* Actually abort the process */
				quit(NULL);
			}
		}
	}

	/* Hack -- Forget standard args */
	if (args)
	{
		argc = 1;
		argv[1] = NULL;
	}


	/* Process the player name */
	process_player_name(TRUE);


	/* Install "quit" hook */
	quit_aux = quit_hook;


#ifdef USE_GTK2
	/* Attempt to use the "main-gtk2.c" support */
	if (!done && (!mstr || (streq(mstr, "gtk2"))))
	{
		extern errr init_gtk2(int, char**);
		if (0 == init_gtk2(argc, argv))
		{
			ANGBAND_SYS = "gtk2";
			done = TRUE;
		}
	}
#endif

#ifdef USE_X11
	/* Attempt to use the "main-x11.c" support */
	if (!done && (!mstr || (streq(mstr, "x11"))))
	{
		extern errr init_x11(int, char**);
		if (0 == init_x11(argc, argv))
		{
			ANGBAND_SYS = "x11";
			done = TRUE;
		}
	}
#endif

#ifdef USE_GCU
	/* Attempt to use the "main-gcu.c" support */
	if (!done && (!mstr || (streq(mstr, "gcu"))))
	{
		extern errr init_gcu(int, char**);
		if (0 == init_gcu(argc, argv))
		{
			ANGBAND_SYS = "gcu";
			done = TRUE;
		}
	}
#endif

#ifdef USE_SDL
	/* Attempt to use the "main-sdl.c" support */
	if (!done && (!mstr || (streq(mstr, "sdl"))))
	{
		extern errr init_sdl(int, char**);
		if (0 == init_sdl(argc, argv))
		{
			ANGBAND_SYS = "sdl";
			done = TRUE;
		}
	}
#endif

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");


	/* Initialize */
	init_angband();

	/* Wait for response */
	pause_line(23);

	/* Play the game */
	play_game(new_game);

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#endif
