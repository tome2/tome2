/* File: main.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "angband.h"

/* Use runtime location of lib directory */
#ifdef ENABLE_BINRELOC
#include "prefix.h"
#endif


/*
 * Some machines have a "main()" function in their "main-xxx.c" file,
 * all the others use this file for their "main()" function.
 */


#if !defined(MACINTOSH) && !defined(WINDOWS) && !defined(ACORN)


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
 * Set the stack size (for the Amiga)
 */
#ifdef AMIGA
# include <dos.h>
__near long __stack = 32768L;
#endif


/*
 * Set the stack size and overlay buffer (see main-286.c")
 */
#ifdef USE_286
# include <dos.h>
extern unsigned _stklen = 32768U;
extern unsigned _ovrbuffer = 0x1500;
#endif


#ifdef PRIVATE_USER_PATH

/*
 * Check and create if needed the directory dirpath
 */
bool private_check_user_directory(cptr dirpath)
{
	/* Is this used anywhere else in *bands? */
	struct stat stat_buf;

	int ret;

	/* See if it already exists */
	ret = stat(dirpath, &stat_buf);

	/* It does */
	if (ret == 0)
	{
		/* Now we see if it's a directory */
		if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) return (TRUE);

		/*
		 * Something prevents us from create a directory with
		 * the same pathname
		 */
		return (FALSE);
	}

	/* No - this maybe the first time. Try to create a directory */
	else
	{
		/* Create the ~/.ToME directory */
		ret = mkdir(dirpath, 0700);

		/* An error occured */
		if (ret == -1) return (FALSE);

		/* Success */
		return (TRUE);
	}
}

/*
 * Check existence of ".ToME/" directory in the user's
 * home directory or try to create it if it doesn't exist.
 * Returns FALSE if all the attempts fail.
 */
static bool check_create_user_dir(void)
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

	return private_check_user_directory(dirpath) && private_check_user_directory(versionpath) && private_check_user_directory(savepath);
}

#endif /* PRIVATE_USER_PATH */


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
 *
 * Note that the "path" must be "Angband:" for the Amiga, and it
 * is ignored for "VM/ESA", so I just combined the two.
 */
static void init_stuff(void)
{
	char path[1024];

#if defined(AMIGA) || defined(VM)

	/* Hack -- prepare "path" */
	strcpy(path, "Angband:");

#else /* AMIGA / VM */

	cptr tail;

	/* Get the environment variable */
	tail = getenv("TOME_PATH");

	/* Use the angband_path, or a default */
#ifndef ENABLE_BINRELOC
	strcpy(path, tail ? tail : DEFAULT_PATH);
#else /* Runtime lookup of location */
	strcpy(path, br_strcat(DATADIR, "/tome/lib"));
#endif

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) strcat(path, PATH_SEP);

#endif /* AMIGA / VM */

	/* Initialize */
	init_file_paths(path);
}



/*
 * Handle a "-d<what>=<path>" option
 *
 * The "<what>" can be any string starting with the same letter as the
 * name of a subdirectory of the "lib" folder (i.e. "i" or "info").
 *
 * The "<path>" can be any legal path for the given system, and should
 * not end in any special path separator (i.e. "/tmp" or "~/.ang-info").
 */
static void change_path(cptr info)
{
	cptr s;

	/* Find equal sign */
	s = strchr(info, '=');

	/* Verify equal sign */
	if (!s) quit_fmt("Try '-d<what>=<path>' not '-d%s'", info);

	/* Analyze */
	switch (tolower(info[0]))
	{
	case 'a':
		{
			string_free(ANGBAND_DIR_APEX);
			ANGBAND_DIR_APEX = string_make(s + 1);
			break;
		}

	case 'f':
		{
			string_free(ANGBAND_DIR_FILE);
			ANGBAND_DIR_FILE = string_make(s + 1);
			break;
		}

	case 'h':
		{
			string_free(ANGBAND_DIR_HELP);
			ANGBAND_DIR_HELP = string_make(s + 1);
			break;
		}

	case 'i':
		{
			string_free(ANGBAND_DIR_INFO);
			ANGBAND_DIR_INFO = string_make(s + 1);
			break;
		}

	case 'u':
		{
			string_free(ANGBAND_DIR_USER);
			ANGBAND_DIR_USER = string_make(s + 1);
			break;
		}

	case 'x':
		{
			string_free(ANGBAND_DIR_XTRA);
			ANGBAND_DIR_XTRA = string_make(s + 1);
			break;
		}

#ifdef VERIFY_SAVEFILE

	case 'b':
	case 'd':
	case 'e':
	case 's':
		{
			quit_fmt("Restricted option '-d%s'", info);
		}

#else /* VERIFY_SAVEFILE */

	case 'b':
		{
			string_free(ANGBAND_DIR_BONE);
			ANGBAND_DIR_BONE = string_make(s + 1);
			break;
		}

	case 'd':
		{
			string_free(ANGBAND_DIR_DATA);
			ANGBAND_DIR_DATA = string_make(s + 1);
			break;
		}

	case 'e':
		{
			string_free(ANGBAND_DIR_EDIT);
			ANGBAND_DIR_EDIT = string_make(s + 1);
			break;
		}

	case 's':
		{
			string_free(ANGBAND_DIR_SAVE);
			ANGBAND_DIR_SAVE = string_make(s + 1);
			break;
		}

#endif /* VERIFY_SAVEFILE */

	default:
		{
			quit_fmt("Bad semantics in '-d%s'", info);
		}
	}
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

	bool done = FALSE;

	bool new_game = FALSE;

	int show_score = 0;

	cptr mstr = NULL;

	bool args = TRUE;

#ifdef CHECK_MEMORY_LEAKS
	GC_find_leak = 1;
#endif /* CHECK_MEMORY_LEAKS */


	/* Save the "program name" XXX XXX XXX */
	argv0 = argv[0];


#ifdef USE_286
	/* Attempt to use XMS (or EMS) memory for swap space */
	if (_OvrInitExt(0L, 0L))
	{
		_OvrInitEms(0, 0, 64);
	}
#endif


#ifdef SET_UID

	/* Default permissions on files */
	(void)umask(022);

#endif /* SET_UID */


	/* Get the file paths */
	init_stuff();


#ifdef SET_UID

	/* Get the user id (?) */
	player_uid = getuid();

#ifdef VMS
	/* Mega-Hack -- Factor group id */
	player_uid += (getgid() * 1000);
#endif

# ifdef SAFE_SETUID

# ifdef _POSIX_SAVED_IDS

	/* Save some info for later */
	player_euid = geteuid();
	player_egid = getegid();

# endif

# if 0	/* XXX XXX XXX */

	/* Redundant setting necessary in case root is running the game */
	/* If not root or game not setuid the following two calls do nothing */

	if (setgid(getegid()) != 0)
	{
		quit("setgid(): cannot set permissions correctly!");
	}

	if (setuid(geteuid()) != 0)
	{
		quit("setuid(): cannot set permissions correctly!");
	}

# endif  /* XXX XXX XXX */

# endif  /* SAFE_SETUID */

#endif /* SET_UID */


#ifdef SET_UID

	/* Please note that the game is still running in the game's permission */

	/* Initialize the "time" checker */
	if (check_time_init() || check_time())
	{
		quit("The gates to Angband are closed (bad time).");
	}

	/* Initialize the "load" checker */
	if (check_load_init() || check_load())
	{
		quit("The gates to Angband are closed (bad load).");
	}


	/*
	 * Become user -- This will be the normal state for the rest of the game.
	 *
	 * Put this here because it's totally irrelevant to single user operating
	 * systems, as witnessed by huge number of cases where these functions
	 * weren't used appropriately (at least in this variant).
	 *
	 * Whenever it is necessary to open/remove/move the files in the lib folder,
	 * this convention must be observed:
	 *
	 *    safe_setuid_grab();
	    *
	 *    fd_open/fd_make/fd_kill/fd_move which requires game's permission,
	 *    i.e. manipulating files under the lib directory
	 *
	 *    safe_setuid_drop();
	 *
	 * Please never ever make unmatched calls to these grab/drop functions.
	 *
	 * Please note that temporary files used by various information commands
	 * and ANGBAND_DIR_USER files shouldn't be manipulated this way, because
	 * they reside outside of the lib directory on multiuser installations.
	 * -- pelpel
	 */
	safe_setuid_drop();


	/* Acquire the "user name" as a default player name */
	user_name(player_name, player_uid);


#ifdef PRIVATE_USER_PATH

	/*
	 * On multiuser systems, users' private directories are
	 * used to store pref files, chardumps etc.
	 */
	{
		bool ret;

		/* Create a directory for the user's files */
		ret = check_create_user_dir();

		/* Oops */
		if (ret == FALSE) quit("Cannot create directory " PRIVATE_USER_PATH);
	}

#endif /* PRIVATE_USER_PATH */

#endif /* SET_UID */




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

		case 'F':
		case 'f':
			{
				arg_fiddle = TRUE;
				break;
			}

		case 'W':
		case 'w':
			{
				arg_wizard = TRUE;
				break;
			}

		case 'V':
		case 'v':
			{
				arg_sound = TRUE;
				break;
			}

		case 'G':
		case 'g':
			{
				arg_graphics = TRUE;
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

		case 'S':
		case 's':
			{
				show_score = atoi(&argv[i][2]);
				if (show_score <= 0) show_score = 10;
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
				force_module = string_make(&argv[i][2]);
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

				init_lua();
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

		case 'c':
		case 'C':
			{
				chg_to_txt(argv[i + 1], argv[i + 2]);

				return 0;
			}

		case 'd':
		case 'D':
			{
				change_path(&argv[i][2]);
				break;
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
				puts("  -f                 Request fiddle mode");
				puts("  -w                 Request wizard mode");
				puts("  -v                 Request sound mode");
				puts("  -g                 Request graphics mode");
				puts("  -o                 Request original keyset");
				puts("  -r                 Request rogue-like keyset");
				puts("  -H <list of files> Convert helpfile to html");
				puts("  -c f1 f2           Convert changelog f1 to nice txt f2");
				puts("  -s<num>            Show <num> high scores");
				puts("  -u<who>            Use your <who> savefile");
				puts("  -M<which>            Use the <which> module");
				puts("  -m<sys>            Force 'main-<sys>.c' usage");
				puts("  -d<def>            Define a 'lib' dir sub-path");

#ifdef USE_GTK
				puts("  -mgtk              To use GTK");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -b              Turn off software backing store");
# ifdef USE_GRAPHICS
				puts("  -- -s              Turn off smoothscaling graphics");
				puts("  -- -o              Requests \"old\" graphics");
				puts("  -- -g              Requests \"new\" graphics");
				puts("  -- -t              Enable transparency effect");
# endif  /* USE_GRAPHICS */
#endif /* USE_GTK */

#ifdef USE_GTK2
				puts("  -mgtk2             To use GTK2");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -b              Turn off software backing store");
# ifdef USE_GRAPHICS
				puts("  -- -s              Turn off smoothscaling graphics");
				puts("  -- -o              Requests \"old\" graphics");
				puts("  -- -g              Requests \"new\" graphics");
				puts("  -- -t              Enable transparency effect");
# endif  /* USE_GRAPHICS */
#endif /* USE_GTK2 */

#ifdef USE_XAW
				puts("  -mxaw              To use XAW");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -d<name>        Display to use");
# ifdef USE_GRAPHICS
				puts("  -- -s              Turn off smoothscaling graphics");
				puts("  -- -o              Requests \"old\" graphics");
# endif  /* USE_GRAPHICS */
#endif /* USE_XAW */

#ifdef USE_X11
				puts("  -mx11              To use X11");
				puts("  --                 Sub options");
				puts("  -- -n#             Number of terms to use");
				puts("  -- -d<name>        Display to use");
# ifdef USE_GRAPHICS
				puts("  -- -s              Turn off smoothscaling graphics");
				puts("  -- -o              Requests \"old\" graphics");
				puts("  -- -b              Requests double-width tiles");
# endif  /* USE_GRAPHICS */
#endif /* USE_X11 */

#ifdef USE_GCU
				puts("  -mgcu              To use curses");
				puts("  --                 Sub options");
				puts("  -- -b              Requests big screen");
#endif /* USE_GCU */

#ifdef USE_CAP
				puts("  -mcap              To use termcap");
#endif /* USE_CAP */

#ifdef USE_DOS
				puts("  -mdos              To use Allegro");
#endif /* USE_DOS */

#ifdef USE_IBM
				puts("  -mibm              To use IBM/PC console");
#endif /* USE_IBM */

#ifdef USE_EMX
				puts("  -memx              To use EMX");
#endif /* USE_EMX */

#ifdef USE_SLA
				puts("  -msla              To use SLang");
#endif /* USE_SLA */

#ifdef USE_LSL
				puts("  -mlsl              To use SVGALIB");
#endif /* USE_LSL */

#ifdef USE_AMI
				puts("  -mami              To use Amiga");
#endif /* USE_AMI */

#ifdef USE_VME
				puts("  -mvme              To use VM/ESA");
#endif /* USE_VME */

#ifdef USE_ISO
				puts("  -miso              To use ISO");
#endif /* USE_ISO */

#ifdef USE_SDL
				puts("  -msdl              To use SDL");
				puts("  --                 Sub options");
				puts("  -- -n #            Number of virtual consoles to use");
				puts("  -- -g              Request new graphics (16x16)");
				puts("  -- -o              Request old graphics (8x8)");
				puts("  -- -b              Requests double-width tiles");
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


	/* Install the zsock hooks we cannot do it later because main-net needs them */
	zsock_init();


#ifdef USE_GLU
	/* Attempt to use the "main-glu.c" support */
	if (!done && (!mstr || (streq(mstr, "glu"))))
	{
		extern errr init_glu(int, char**);
		if (0 == init_glu(argc, argv))
		{
			ANGBAND_SYS = "glu";
			done = TRUE;
		}
	}
#endif

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

#ifdef USE_GTK
	/* Attempt to use the "main-gtk.c" support */
	if (!done && (!mstr || (streq(mstr, "gtk"))))
	{
		extern errr init_gtk(int, char**);
		if (0 == init_gtk(argc, argv))
		{
			ANGBAND_SYS = "gtk";
			done = TRUE;
		}
	}
#endif

#ifdef USE_XAW
	/* Attempt to use the "main-xaw.c" support */
	if (!done && (!mstr || (streq(mstr, "xaw"))))
	{
		extern errr init_xaw(int, char**);
		if (0 == init_xaw(argc, argv))
		{
			ANGBAND_SYS = "xaw";
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

#ifdef USE_GLU
	/* Attempt to use the "main-glu.c" support */
	if (!done && (!mstr || (streq(mstr, "glu"))))
	{
		extern errr init_glu(int, char**);
		if (0 == init_glu(argc, argv))
		{
			ANGBAND_SYS = "glu";
			done = TRUE;
		}
	}
#endif

#ifdef USE_CAP
	/* Attempt to use the "main-cap.c" support */
	if (!done && (!mstr || (streq(mstr, "cap"))))
	{
		extern errr init_cap(int, char**);
		if (0 == init_cap(argc, argv))
		{
			ANGBAND_SYS = "cap";
			done = TRUE;
		}
	}
#endif


#ifdef USE_DOS
	/* Attempt to use the "main-dos.c" support */
	if (!done && (!mstr || (streq(mstr, "dos"))))
	{
		extern errr init_dos(void);
		if (0 == init_dos())
		{
			ANGBAND_SYS = "dos";
			done = TRUE;
		}
	}
#endif

#ifdef USE_IBM
	/* Attempt to use the "main-ibm.c" support */
	if (!done && (!mstr || (streq(mstr, "ibm"))))
	{
		extern errr init_ibm(void);
		if (0 == init_ibm())
		{
			ANGBAND_SYS = "ibm";
			done = TRUE;
		}
	}
#endif


#ifdef USE_EMX
	/* Attempt to use the "main-emx.c" support */
	if (!done && (!mstr || (streq(mstr, "emx"))))
	{
		extern errr init_emx(void);
		if (0 == init_emx())
		{
			ANGBAND_SYS = "emx";
			done = TRUE;
		}
	}
#endif


#ifdef USE_SLA
	/* Attempt to use the "main-sla.c" support */
	if (!done && (!mstr || (streq(mstr, "sla"))))
	{
		extern errr init_sla(void);
		if (0 == init_sla())
		{
			ANGBAND_SYS = "sla";
			done = TRUE;
		}
	}
#endif


#ifdef USE_LSL
	/* Attempt to use the "main-lsl.c" support */
	if (!done && (!mstr || (streq(mstr, "lsl"))))
	{
		extern errr init_lsl(void);
		if (0 == init_lsl())
		{
			ANGBAND_SYS = "lsl";
			done = TRUE;
		}
	}
#endif


#ifdef USE_AMI
	/* Attempt to use the "main-ami.c" support */
	if (!done && (!mstr || (streq(mstr, "ami"))))
	{
		extern errr init_ami(void);
		if (0 == init_ami())
		{
			ANGBAND_SYS = "ami";
			done = TRUE;
		}
	}
#endif


#ifdef USE_VME
	/* Attempt to use the "main-vme.c" support */
	if (!done && (!mstr || (streq(mstr, "vme"))))
	{
		extern errr init_vme(void);
		if (0 == init_vme())
		{
			ANGBAND_SYS = "vme";
			done = TRUE;
		}
	}
#endif


#ifdef USE_PARAGUI
	/* Attempt to use the "main-pgu.c" support */
	if (!done && (!mstr || (streq(mstr, "pgu"))))
	{
		extern errr init_pgu(int, char**);
		if (0 == init_pgu(argc, argv))
		{
			ANGBAND_SYS = "pgu";
			done = TRUE;
		}
	}
#endif

#ifdef USE_ISO
	/* Attempt to use the "main-iso.c" support */
	if (!done && (!mstr || (streq(mstr, "iso"))))
	{
		extern errr init_iso(int, char**);
		if (0 == init_iso(argc, argv))
		{
			ANGBAND_SYS = "iso";
			done = TRUE;
		}
	}
#endif

#ifdef USE_LUA_GUI
	/* Attempt to use the "main-lua.c" support */
	if (!done && (!mstr || (streq(mstr, "lua"))))
	{
		extern errr init_lua_gui(int, char**);
		if (0 == init_lua_gui(argc, argv))
		{
			ANGBAND_SYS = "lua";
			done = TRUE;
		}
	}
#endif

#ifdef USE_NET
	/* Attempt to use the "main-net.c" support */
	if (!done && (!mstr || (streq(mstr, "net"))))
	{
		extern errr init_net(int, char**);
		if (0 == init_net(argc, argv))
		{
			ANGBAND_SYS = "net";
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

#ifdef USE_DMY
	/* Attempt to use the "main-dmy.c" support */
	if (!done && (!mstr || (streq(mstr, "dmy"))))
	{
		extern errr init_dummy(int, char**);
		if (0 == init_dummy(argc, argv))
		{
			ANGBAND_SYS = "dmy";
			done = TRUE;
		}
	}
#endif

	/* Make sure we have a display! */
	if (!done) quit("Unable to prepare any 'display module'!");


	/* Catch nasty signals */
	signals_init();

	/* Initialize */
	init_angband();

	/* Hack -- If requested, display scores and quit */
	if (show_score > 0) display_scores(0, show_score);

	/* Wait for response */
	pause_line(23);

	/* Play the game */
	play_game(new_game);

#ifdef CHECK_MEMORY_LEAKS
	CHECK_LEAKS();
#endif

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#endif
