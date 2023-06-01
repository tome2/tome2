/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "main.hpp"

#include "birth.hpp"
#include "config.hpp"
#include "dungeon.hpp"
#include "files.hpp"
#include "game.hpp"
#include "init2.hpp"
#include "modules.hpp"
#include "program_args.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-form.hpp"
#include "z-util.hpp"

#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::equals;
using boost::algorithm::ends_with;

/*
 * A hook for "quit()".
 *
 * Close down, then fall back into "quit()".
 */
static void quit_hook(const char *s)
{
	/* Scan windows */
	for (int j = ANGBAND_TERM_MAX - 1; j >= 0; j--)
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
 * Returns false if all the attempts fail.
 */
static void init_save_dir()
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

/*
 * Initialize and verify the file paths, and the score file.
 *
 * Use the ANGBAND_PATH environment var if possible, else use
 * DEFAULT_PATH, and in either case, branch off appropriately.
 *
 * First, we'll look for the ANGBAND_PATH environment variable,
 * and then look for the files in there.  If that doesn't work,
 * we'll try the DEFAULT_PATH constant.  So be sure that one of
 * these two things works...
 *
 * We must ensure that the path ends with "PATH_SEP" if needed,
 * since the "init_file_paths()" function will simply append the
 * relevant "sub-directory names" to the given path.
 */
static void init_file_paths_with_env()
{
	char path[1024];

	/* Get the environment variable */
	const char *tail = getenv("TOME_PATH");

	/* Use the angband_path, or a default */
	strcpy(path, tail ? tail : DEFAULT_PATH);

	/* Hack -- Add a path separator (only if needed) */
	if (!ends_with(path, PATH_SEP)) strcat(path, PATH_SEP);

	/* Initialize */
	init_file_paths(path);
}


/*
 * Simple "main" function for multiple platforms.
 *
 * Note the special "--" option which terminates the processing of
 * standard options.  All non-standard options (if any) are passed
 * directly to the platform initialization function.
 */
int main_real(int argc, char *argv[], char const *platform_sys, int (*init_platform)(int, char *[]), char const *platform_usage)
{
	int i;

	// Initialize game structure
	game = new Game();

	/* Get the file paths */
	init_file_paths_with_env();

	/* Initialize the player name */
	game->player_name = user_name();

	/* Make sure save directory exists */
	init_save_dir();

	/* Program arguments */
	program_args program_args;

	/* Process the command line arguments */
	bool args = true;
	for (i = 1; args && (i < argc); i++)
	{
		/* Require proper options */
		if (argv[i][0] != '-')
		{
			goto usage;
		}

		/* Analyze option */
		switch (argv[i][1])
		{
		case 'W':
		case 'w':
			{
				program_args.wizard = true;
				break;
			}

		case 'R':
		case 'r':
			{
				program_args.force_key_set = 'r';
				break;
			}

		case 'O':
		case 'o':
			{
				program_args.force_key_set = 'o';
				break;
			}

		case 'u':
		case 'U':
			{
				if (!argv[i][2])
				{
					goto usage;
				}
				program_args.player_name = &argv[i][2];
				break;
			}

		case 'M':
			{
				if (!argv[i][2])
				{
					goto usage;
				}
				program_args.module = &argv[i][2];
				break;
			}

		case 'h':
			{
				goto usage;
			}

		case '-':
			{
				if (argv[i][2] == 'h' && equals(argv[i] + 2, "help"))
				{
					goto usage;
				}
				else
				{
					argv[i] = argv[0];
					argc = argc - i;
					argv = argv + i;
					args = false;
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
				puts("  -w                 Request wizard mode");
				puts("  -o                 Request original keyset");
				puts("  -r                 Request rogue-like keyset");
				puts("  -u<who>            Use your <who> savefile");
				puts("  -M<which>          Use the <which> module");

				puts("  --                 Sub options");
				puts(platform_usage);

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

	/* If player name specified... */
	if (!program_args.player_name.empty())
	{
		game->player_name = program_args.player_name;
		no_begin_screen = true;
	}

	/* Process the player name */
	set_player_base(game->player_name);


	/* Install "quit" hook */
	quit_aux = quit_hook;

	/* Run the platform main initialization */
	if (init_platform(argc, argv))
	{
		quit("Unable to prepare any 'display module'!");
	}
	else
	{
		ANGBAND_SYS = platform_sys;

		/* Initialize */
		init_angband(program_args);

		/* Wait for response */
		pause_line(23);

		/* Play the game */
		play_game(program_args);

		/* Quit */
		quit(NULL);

	}
	/* Exit */
	return (0);
}
