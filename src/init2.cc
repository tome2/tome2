#include "init2.hpp"
#include "init2.h"

#include "ability_type.hpp"
#include "alloc_entry.hpp"
#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cli_comm.hpp"
#include "dungeon_info_type.hpp"
#include "ego_item_type.hpp"
#include "files.hpp"
#include "feature_type.hpp"
#include "generate.hpp"
#include "gen_evol.hpp"
#include "gen_maze.hpp"
#include "hist_type.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "lua_bind.hpp"
#include "messages.hpp"
#include "meta_class_type.hpp"
#include "modules.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object_kind.hpp"
#include "owner_type.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "q_library.hpp"
#include "quark.hpp"
#include "randart.hpp"
#include "randart_part_type.hpp"
#include "set_type.hpp"
#include "skill_type.hpp"
#include "spells3.hpp"
#include "spells4.hpp"
#include "spells5.hpp"
#include "spells6.hpp"
#include "squeltch.hpp"
#include "store_action_type.hpp"
#include "store_info_type.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "tome/make_array.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "vault_type.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"

#include <cassert>
#include <type_traits>

/*
 * This file is used to initialise various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * The "init1.c" file is used only to parse the ascii template files.
 */



/*
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variable.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Mega-Hack -- support fat raw files under NEXTSTEP, using special
 * "suffixed" directories for the "ANGBAND_DIR_DATA" directory, but
 * requiring the directories to be created by hand by the user.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(char *path)
{
	char *tail;
	int   pathlen;

	assert(path != nullptr);

	/*** Free everything ***/

	/* Free the main path */
	free(ANGBAND_DIR);

	/* Free the sub-paths */
	free(ANGBAND_DIR_CORE);
	free(ANGBAND_DIR_DNGN);
	free(ANGBAND_DIR_DATA);
	free(ANGBAND_DIR_EDIT);
	free(ANGBAND_DIR_FILE);
	free(ANGBAND_DIR_HELP);
	free(ANGBAND_DIR_INFO);
	free(ANGBAND_DIR_MODULES);
	free(ANGBAND_DIR_NOTE);
	free(ANGBAND_DIR_SAVE);
	free(ANGBAND_DIR_PREF);
	free(ANGBAND_DIR_USER);
	free(ANGBAND_DIR_XTRA);


	/*** Prepare the "path" ***/

	pathlen = strlen(path);

	/* Hack -- save the main directory without trailing PATH_SEP if present */
	if (strlen(PATH_SEP) > 0 && pathlen > 0)
	{
		int seplen = strlen(PATH_SEP);

		if (strcmp(path + pathlen - seplen, PATH_SEP) == 0)
		{
			path[pathlen - seplen] = '\0';
			ANGBAND_DIR = strdup(path);
			path[pathlen - seplen] = *PATH_SEP;
		}
		else
		{
			ANGBAND_DIR = strdup(path);
		}
	}
	else
	{
		ANGBAND_DIR = strdup(path);
	}

	/* Prepare to append to the Base Path */
	tail = path + pathlen;



	/*** Build the sub-directory names ***/

	/* Build a path name */
	strcpy(tail, "core");
	ANGBAND_DIR_CORE = strdup(path);

	/* Build a path name */
	strcpy(tail, "dngn");
	ANGBAND_DIR_DNGN = strdup(path);

	/* Build a path name */
	strcpy(tail, "data");
	ANGBAND_DIR_DATA = strdup(path);

	/* Build a path name */
	strcpy(tail, "edit");
	ANGBAND_DIR_EDIT = strdup(path);

	/* Build a path name */
	strcpy(tail, "file");
	ANGBAND_DIR_FILE = strdup(path);

	/* Build a path name */
	strcpy(tail, "help");
	ANGBAND_DIR_HELP = strdup(path);

	/* Build a path name */
	strcpy(tail, "info");
	ANGBAND_DIR_INFO = strdup(path);

	/* Build a path name */
	strcpy(tail, "mods");
	ANGBAND_DIR_MODULES = strdup(path);

	/* Build a path name */
	strcpy(tail, "pref");
	ANGBAND_DIR_PREF = strdup(path);

	/* synchronize with module_reset_dir */
	{
		char user_path[1024];

		/* Get an absolute path from the file name */
		path_parse(user_path, 1024, PRIVATE_USER_PATH);
		strcat(user_path, USER_PATH_VERSION);
		ANGBAND_DIR_USER = strdup(user_path);
		ANGBAND_DIR_NOTE = strdup(user_path);

		/* Savefiles are in user directory */
		strcat(user_path, "/save");
		ANGBAND_DIR_SAVE = strdup(user_path);
	}

	/* Build a path name */
	strcpy(tail, "xtra");
	ANGBAND_DIR_XTRA = strdup(path);
}



/*
 * Hack -- help give useful error messages
 */
s16b error_idx;
s16b error_line;


/*
 * Standard error message text
 */
static cptr err_str[9] =
{
	NULL,
	"parse error",
	"obsolete file",
	"missing record header",
	"non-sequential records",
	"invalid flag specification",
	"undefined directive",
	"out of memory",
	"invalid skill chart"
};


/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, str);
	Term_fresh();
}


/*
 * Traits for data arrays
 */
namespace {

	struct f_info_traits {

		static constexpr char const *name = "f_info.txt";

		static void allocate()
		{
			f_info = make_array<feature_type>(max_f_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_f_info_txt(fp);
		}

	};

	struct k_info_traits {

		static constexpr char const *name = "k_info.txt";

		static void allocate()
		{
			k_info = make_array<object_kind>(max_k_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_k_info_txt(fp);
		};

	};

	struct set_info_traits {

		static constexpr char const *name = "set_info.txt";

		static void allocate()
		{
			set_info = make_array<set_type>(max_set_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_set_info_txt(fp);
		}

	};

	struct a_info_traits {

		static constexpr char const *name = "a_info.txt";

		static void allocate()
		{
			a_info = make_array<artifact_type>(max_a_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_a_info_txt(fp);
		}

	};

	struct s_info_traits {

		static constexpr char const *name = "s_info.txt";

		static void allocate()
		{
			s_info = make_array<skill_type>(max_s_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_s_info_txt(fp);
		}

	};

	struct ab_info_traits {

		static constexpr char const *name = "ab_info.txt";

		static void allocate()
		{
			ab_info = make_array<ability_type>(max_ab_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_ab_info_txt(fp);
		}

	};

	struct e_info_traits {

		static constexpr char const *name = "e_info.txt";

		static void allocate()
		{
			e_info = make_array<ego_item_type>(max_e_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_e_info_txt(fp);
		}

	};

	struct ra_info_traits {

		static constexpr char const *name = "ra_info.txt";

		static void allocate()
		{
			ra_info = make_array<randart_part_type>(max_ra_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_ra_info_txt(fp);
		}

	};

	struct r_info_traits {

		static constexpr char const *name = "r_info.txt";

		static void allocate()
		{
			r_info = make_array<monster_race>(max_r_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_r_info_txt(fp);
		}

	};

	struct re_info_traits {

		static constexpr char const *name = "re_info.txt";

		static void allocate()
		{
			re_info = make_array<monster_ego>(max_re_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_re_info_txt(fp);
		}

	};

	struct d_info_traits {

		static constexpr char const *name = "d_info.txt";

		static void allocate()
		{
			d_info = make_array<dungeon_info_type>(max_d_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_d_info_txt(fp);
		}

	};

	struct st_info_traits {

		static constexpr char const *name = "st_info.txt";

		static void allocate()
		{
			st_info = make_array<store_info_type>(max_st_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_st_info_txt(fp);
		}

	};

	struct ow_info_traits {

		static constexpr char const *name = "ow_info.txt";

		static void allocate()
		{
			ow_info = make_array<owner_type>(max_ow_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_ow_info_txt(fp);
		}

	};

	struct ba_info_traits {

		static constexpr char const *name = "ba_info.txt";

		static void allocate()
		{
			ba_info = make_array<store_action_type>(max_ba_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_ba_info_txt(fp);
		}

	};

	struct wf_info_traits {

		static constexpr char const *name = "wf_info.txt";

		static void allocate()
		{
			wf_info = make_array<wilderness_type_info>(max_wf_idx);
		}

		static errr parse(FILE *fp)
		{
			return init_wf_info_txt(fp);
		}

	};

	struct v_info_traits {

		static constexpr char const *name = "v_info.txt";

		static void allocate()
		{
			v_info = new vault_type[max_v_idx];
		}

		static errr parse(FILE *fp)
		{
			return init_v_info_txt(fp);
		}

	};

	struct p_info_traits {

		static constexpr char const *name = "p_info.txt";

		static void allocate()
		{
			race_info = make_array<player_race>(max_rp_idx);
			race_mod_info = make_array<player_race_mod>(max_rmp_idx);
			class_info = make_array<player_class>(max_c_idx);
			bg = make_array<hist_type>(max_bg_idx);
			meta_class_info = make_array<meta_class_type>(max_mc_idx);
			for (std::size_t i = 0; i < max_mc_idx; i++)
			{
				meta_class_info[i].classes = make_array<s16b>(max_c_idx);
			}
		}

		static errr parse(FILE *fp)
		{
			return init_player_info_txt(fp);
		}

	};

}

template<typename T> static errr init_x_info() {

	/* Allocate the data array */
	T::allocate();

	/* Build the filename */
	boost::filesystem::path path(ANGBAND_DIR_EDIT);
	path /= T::name;

	/* Open the file */
	FILE *fp = my_fopen(path.c_str(), "r");

	/* Parse it */
	if (!fp)
	{
		quit_fmt("Cannot open '%s' file.", T::name);
	}

	/* Parse the file */
	errr err = T::parse(fp);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		/* Error string */
		cptr oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of '%s'.", err, error_line, T::name);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_print(NULL);

		/* Quit */
		quit_fmt("Error in '%s' file.", T::name);
	}

	/* Success */
	return (0);
}

errr init_v_info()
{
	return init_x_info<v_info_traits>();
}

/*
 * Initialize the very basic arrays
 */
static void init_basic()
{
	/* Macro variables */
	macro__pat = make_array<char *>(MACRO_MAX);
	macro__act = make_array<char *>(MACRO_MAX);
	macro__cmd = make_array<bool_>(MACRO_MAX);

	/* Macro action buffer */
	macro__buf = make_array<char>(1024);

	/* Extended trigger macros */
	cli_info = make_array<cli_comm>(CLI_MAX);
}


/*
 * Initialise misc. values
 */
static errr init_misc(void)
{
	int xstart = 0;
	int ystart = 0;
	int i;

	/*** Prepare the various "bizarre" arrays ***/

	/* Initialize quark subsystem */
	quark_init();

	/* Initialize messages subsystem */
	message_init();

	/* Initialise the values */
	process_dungeon_file("misc.txt", &ystart, &xstart, 0, 0, TRUE, FALSE);

	/* Init the spell effects */
	for (i = 0; i < MAX_EFFECTS; i++)
		effects[i].time = 0;

	/* Initialize timers */
	TIMER_INERTIA_CONTROL =
		new_timer(meta_inertia_control_timer_callback,
			  10);
	TIMER_AGGRAVATE_EVIL =
		new_timer(timer_aggravate_evil_callback,
			  10);

	return 0;
}


/*
 * Initialise town array
 */
static errr init_towns(void)
{
	int i = 0, j = 0;

	/*** Prepare the Towns ***/

	/* Allocate the towns */
	town_info = make_array<town_type>(max_towns);

	for (i = 1; i < max_towns; i++)
	{
		if (i <= max_real_towns) town_info[i].flags |= (TOWN_REAL);

		/* Allocate the stores */
		town_info[i].store = make_array<store_type>(max_st_idx);

		/* Fill in each store */
		for (j = 0; j < max_st_idx; j++)
		{
			/* Access the store */
			store_type *st_ptr = &town_info[i].store[j];

			/* Know who we are */
			st_ptr->st_idx = j;

			/* Assume full stock */
			st_ptr->stock_size = 0;
		}
	}
	return 0;
}

void create_stores_stock(int t)
{
	int j;
	town_type *t_ptr = &town_info[t];

	if (t_ptr->stocked) return;

	for (j = 0; j < max_st_idx; j++)
	{
		store_type *st_ptr = &t_ptr->store[j];

		/* Assume full stock */
		st_ptr->stock_size = st_info[j].max_obj;

		/* Allocate the stock */
		st_ptr->stock = make_array<object_type>(st_ptr->stock_size);
	}
	t_ptr->stocked = TRUE;
}

/*
 * Initialise wilderness map array
 */
static errr init_wilderness(void)
{
	int i;

	/* Allocate the wilderness (two-dimension array) */
	wild_map = make_array<wilderness_map *>(max_wild_y);

	/* Init the other pointers */
	for (i = 0; i < max_wild_y; i++)
	{
		wild_map[i] = make_array<wilderness_map>(max_wild_x);
	}

	/* No encounter right now */
	generate_encounter = FALSE;

	return 0;
}

/*
 * Initialise some other arrays
 */
static errr init_other(void)
{
	int i, n;

	/*** Prepare the "dungeon" information ***/

	/* Allocate and Wipe the special gene flags */
	m_allow_special = make_array<bool_>(max_r_idx);
	k_allow_special = make_array<bool_>(max_k_idx);
	a_allow_special = make_array<bool_>(max_a_idx);


	/*** Prepare "vinfo" array ***/

	/* Used by "update_view()" */
	(void)vinfo_init();


	/* Allocate and Wipe the object list */
	o_list = make_array<object_type>(max_o_idx);

	/* Allocate and Wipe the monster list */
	m_list = new monster_type[max_m_idx];

	/* Allocate and Wipe the to keep monster list */
	km_list = new monster_type[max_m_idx];

	/* Allocate and Wipe the max dungeon level */
	max_dlv = make_array<s16b>(max_d_idx);

	/* Allocate and Wipe the special levels */
	for (i = 0; i < MAX_DUNGEON_DEPTH; i++)
	{
		special_lvl[i] = make_array<bool_>(max_d_idx);
	}

	/* Allocate and wipe each line of the cave */
	cave = new cave_type *[MAX_HGT];
	for (i = 0; i < MAX_HGT; i++)
	{
		/* Allocate one row of the cave */
		cave[i] = new cave_type[MAX_WID];
	}

	/*** Pre-allocate the basic "auto-inscriptions" ***/

	/* The "basic" feelings */
	(void)quark_add("cursed");
	(void)quark_add("broken");
	(void)quark_add("average");
	(void)quark_add("good");

	/* The "extra" feelings */
	(void)quark_add("excellent");
	(void)quark_add("worthless");
	(void)quark_add("special");
	(void)quark_add("terrible");

	/* Some extra strings */
	(void)quark_add("uncursed");
	(void)quark_add("on sale");


	/*** Prepare the options ***/

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_page;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (option_info[i].o_var)
		{
			/* Accept */
			option_mask[os] |= (1L << ob);

			/* Set */
			if (option_info[i].o_norm)
			{
				/* Set */
				option_flag[os] |= (1L << ob);
			}

			/* Clear */
			else
			{
				/* Clear */
				option_flag[os] &= ~(1L << ob);
			}
		}
	}

	/* Analyze the windows */
	for (n = 0; n < 8; n++)
	{
		/* Analyze the options */
		for (i = 0; i < 32; i++)
		{
			/* Accept */
			if (window_flag_desc[i])
			{
				/* Accept */
				window_mask[n] |= (1L << i);
			}
		}
	}


	/*
	 * Install the various level generators
	 */
	add_level_generator("dungeon", level_generate_dungeon);
	add_level_generator("maze", level_generate_maze);
	add_level_generator("life", level_generate_life);

	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void)format("%s (%s).", "Dark God <darkgod@t-o-m-e.net>", MAINTAINER);

	/* Success */
	return (0);
}



/*
 * Initialise some other arrays
 */
static errr init_alloc(void)
{
	int i, j;

	object_kind *k_ptr;

	monster_race *r_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH_MONSTER];

	s16b aux[MAX_DEPTH_MONSTER];

	/*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	memset(aux, 0, MAX_DEPTH_MONSTER * sizeof(s16b));

	/* Clear the "num" array */
	memset(num, 0, MAX_DEPTH_MONSTER * sizeof(s16b));

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < ALLOCATION_MAX; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH_MONSTER; i++)
	{
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town objects!");


	/*** Initialise object allocation info ***/

	/* Allocate the alloc_kind_table */
	alloc_kind_table = make_array<alloc_entry>(alloc_kind_size);

	/* Access the table entry */
	table = alloc_kind_table;

	/* Scan the objects */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < ALLOCATION_MAX; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				int p, x, y, z;

				/* Extract the base level */
				x = k_ptr->locale[j];

				/* Extract the base probability */
				p = (100 / k_ptr->chance[j]);

				/* Skip entries preceding our locale */
				y = (x > 0) ? num[x - 1] : 0;

				/* Skip previous entries at this locale */
				z = y + aux[x];

				/* Load the entry */
				table[z].index = i;
				table[z].level = x;
				table[z].prob1 = p;
				table[z].prob2 = p;
				table[z].prob3 = p;

				/* Another entry complete for this locale */
				aux[x]++;
			}
		}
	}


	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	memset(aux, 0, MAX_DEPTH_MONSTER * sizeof(s16b));

	/* Clear the "num" array */
	memset(num, 0, MAX_DEPTH_MONSTER * sizeof(s16b));

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity)
		{
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH_MONSTER; i++)
	{
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");


	/*** Initialise monster allocation info ***/

	/* Allocate the alloc_race_table */
	alloc_race_table = make_array<alloc_entry>(alloc_race_size);

	/* Access the table entry */
	table = alloc_race_table;

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x - 1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}


	/* Success */
	return (0);
}

/* Init the sets in a_info */
static void init_sets_aux()
{
	int i, j;

	for (i = 0; i < max_a_idx; i++)
		a_info[i].set = -1;
	for (i = 0; i < max_set_idx; i++)
	{
		for (j = 0; j < set_info[i].num; j++)
		{
			a_info[set_info[i].arts[j].a_idx].set = i;
		}
	}
}

/*
 * Mark guardians and their artifacts with SPECIAL_GENE flag
 */
static void init_guardians(void)
{
	int i;

	/* Scan dungeons */
	for (i = 0; i < max_d_idx; i++)
	{
		dungeon_info_type *d_ptr = &d_info[i];

		/* Mark the guadian monster */
		if (d_ptr->final_guardian)
		{
			monster_race *r_ptr = &r_info[d_ptr->final_guardian];

			r_ptr->flags9 |= RF9_SPECIAL_GENE;

			/* Mark the final artifact */
			if (d_ptr->final_artifact)
			{
				artifact_type *a_ptr = &a_info[d_ptr->final_artifact];

				a_ptr->flags4 |= TR4_SPECIAL_GENE;
			}

			/* Mark the final object */
			if (d_ptr->final_object)
			{
				object_kind *k_ptr = &k_info[d_ptr->final_object];

				k_ptr->flags4 |= TR4_SPECIAL_GENE;
			}

			/* Give randart if there are no final artifacts */
			if (!(d_ptr->final_artifact) && !(d_ptr->final_object))
			{
				r_ptr->flags7 |= RF7_DROP_RANDART;
			}
		}
	}
}

/*
 * Hack -- Explain a broken "lib" folder and quit (see below).
 *
 * XXX XXX XXX This function is "messy" because various things
 * may or may not be initialised, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 */
static void init_angband_aux(cptr why)
{
	/* Why */
	plog(why);

	/* Explain */
	plog("The 'lib' directory is probably missing or broken.");

	/* More details */
	plog("Perhaps the archive was not extracted correctly.");

	/* Explain */
	plog("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
}

/*
 * Hack -- main Angband initialisation entry point
 *
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialise all internal arrays, and
 * load the basic "user pref files".
 *
 * Note that we blindly assume that "news2.txt" exists. XXX
 *
 * Be very careful to keep track of the order in which things
 * are initialised, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialised until the
 * end of this function, when the default "user pref files"
 * are loaded and "Term_xtra(TERM_XTRA_REACT,0)" is called.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 *
 * Note that various things are initialised by this function,
 * including everything that was once done by "init_some_arrays".
 *
 * This initialisation involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialised first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 *
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 *
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
void init_angband(void)
{
	int fd = -1;

	int mode = FILE_MODE;

	FILE *fp;

	const char *news_file;

	char buf[1024];

	/* Init some VERY basic stuff, like macro arrays */
	init_basic();

	/* Select & init a module if needed */
	select_module();

	/*** Choose which news.txt file to use ***/

	/* Choose the news file */
	switch (time(NULL) % 2)
	{
	default:
		{
			news_file = "news.txt";
			break;
		}

	case 0:
		{
			news_file = "news2.txt";
			break;
		}
	}

	/*** Verify the "news" file ***/

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, news_file);

	/* Attempt to open the file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		char why[1024];

		/* Message */
		sprintf(why, "Cannot access the '%s' file!", buf);

		/* Crash and burn */
		init_angband_aux(why);
	}

	/* Close it */
	(void)fd_close(fd);


	/*** Display the "news" file ***/

	/* Clear screen */
	Term_clear();

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, news_file);

	/* Open the News file */
	fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, 1024))
		{
			/* Display and advance - we use display_message to parse colour codes XXX */
			display_message(0, i++, strlen(buf), TERM_WHITE, buf);
		}

		/* Close */
		my_fclose(fp);
	}

	/* Flush it */
	Term_fresh();


	/*** Verify (or create) the "high score" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "scores.raw");

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		/* Create a new high score file */
		fd = fd_make(buf, mode);

		/* Failure */
		if (fd < 0)
		{
			char why[1024];

			/* Message */
			sprintf(why, "Cannot create the '%s' file!", buf);

			/* Crash and burn */
			init_angband_aux(why);
		}
	}

	/* Close it */
	(void)fd_close(fd);


	/*** Initialise some arrays ***/

	/* Initialise misc. values */
	note("[Initialising values... (misc)]");
	if (init_misc()) quit("Cannot initialise misc. values");

	/* Initialise some other arrays */
	{
		note("[Initialising scripting... (script)]");

		/* Initialize schooled spells */
		schools_init();
		school_spells_init();
		init_school_books();

		/* Post-spell creation initialization */
		initialize_bookable_spells();

		/* Finish up the corruptions */
		init_corruptions();
	}

	/* Initialise skills info */
	note("[Initialising arrays... (skills)]");
	if (init_x_info<s_info_traits>()) quit("Cannot initialise skills");

	/* Initialise abilities info */
	note("[Initialising arrays... (abilities)]");
	if (init_x_info<ab_info_traits>()) quit("Cannot initialise abilities");

	/* Initialise player info */
	note("[Initialising arrays... (players)]");
	if (init_x_info<p_info_traits>()) quit("Cannot initialise players");

	/* Initialise feature info */
	note("[Initialising arrays... (features)]");
	if (init_x_info<f_info_traits>()) quit("Cannot initialise features");

	/* Initialise object info */
	note("[Initialising arrays... (objects)]");
	if (init_x_info<k_info_traits>()) quit("Cannot initialise objects");

	/* Initialise artifact info */
	note("[Initialising arrays... (artifacts)]");
	if (init_x_info<a_info_traits>()) quit("Cannot initialise artifacts");

	/* Initialise set info */
	note("[Initialising item sets... (sets)]");
	if (init_x_info<set_info_traits>()) quit("Cannot initialise item sets");
	init_sets_aux();

	/* Initialise ego-item info */
	note("[Initialising arrays... (ego-items)]");
	if (init_x_info<e_info_traits>()) quit("Cannot initialise ego-items");

	/* Initialise randart parts info */
	note("[Initialising arrays... (randarts)]");
	if (init_x_info<ra_info_traits>()) quit("Cannot initialise randarts");

	/* Initialise monster info */
	note("[Initialising arrays... (monsters)]");
	if (init_x_info<r_info_traits>()) quit("Cannot initialise monsters");

	/* Initialise ego monster info */
	note("[Initialising arrays... (ego monsters)]");
	if (init_x_info<re_info_traits>()) quit("Cannot initialise ego monsters");

	/* Initialise dungeon type info */
	note("[Initialising arrays... (dungeon types)]");
	if (init_x_info<d_info_traits>()) quit("Cannot initialise dungeon types");
	init_guardians();

	/* Initialise actions type info */
	note("[Initialising arrays... (action types)]");
	if (init_x_info<ba_info_traits>()) quit("Cannot initialise action types");

	/* Initialise owners type info */
	note("[Initialising arrays... (owners types)]");
	if (init_x_info<ow_info_traits>()) quit("Cannot initialise owners types");

	/* Initialise stores type info */
	note("[Initialising arrays... (stores types)]");
	if (init_x_info<st_info_traits>()) quit("Cannot initialise stores types");

	/* Initialise wilderness features array */
	note("[Initialising arrays... (wilderness features)]");
	if (init_x_info<wf_info_traits>()) quit("Cannot initialise wilderness features");

	/* Initialise wilderness map array */
	note("[Initialising arrays... (wilderness map)]");
	if (init_wilderness()) quit("Cannot initialise wilderness map");

	/* Initialise town array */
	note("[Initialising arrays... (towns)]");
	if (init_towns()) quit("Cannot initialise towns");

	/* Initialise some other arrays */
	note("[Initialising arrays... (other)]");
	if (init_other()) quit("Cannot initialise other stuff");

	/* Initialise some other arrays */
	note("[Initialising arrays... (alloc)]");
	if (init_alloc()) quit("Cannot initialise alloc stuff");

	/* Init random artifact names */
	build_prob(artifact_names_list);

	/* Initialize the automatizer */
	automatizer_init();

	/*** Load default user pref files ***/

	/* Initialise feature info */
	note("[Initialising user pref files...]");

	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "basic" system pref file */
	sprintf(buf, "pref-%s.prf", ANGBAND_SYS);

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "user.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" system pref file */
	sprintf(buf, "user-%s.prf", ANGBAND_SYS);

	/* Process that file */
	process_pref_file(buf);

	/* Done */
	note("[Initialisation complete]");
}
