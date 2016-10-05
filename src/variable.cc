/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "variable.hpp"
#include "variable.h"

#include "cli_comm_fwd.hpp"
#include "player_type.hpp"
#include "skill_modifiers.hpp"
#include "util.hpp"


int max_macrotrigger = 0;
char *macro_template = NULL;
char *macro_modifier_chr;
char *macro_modifier_name[MAX_MACRO_MOD];
char *macro_trigger_name[MAX_MACRO_TRIG];
char *macro_trigger_keycode[2][MAX_MACRO_TRIG];

/*
 * Run-time aruments
 */
bool_ arg_wizard; 			/* Command arg -- Request wizard mode */
bool_ arg_force_original; 	/* Command arg -- Request original keyset */
bool_ arg_force_roguelike; 	/* Command arg -- Request roguelike keyset */

/*
 * Various things
 */

bool_ character_generated; 	/* The character exists */
bool_ character_dungeon; 		/* The character has a dungeon */
bool_ character_loaded; 		/* The character was loaded from a savefile */

bool_ character_icky; 		/* The game is in an icky full screen mode */
bool_ character_xtra; 		/* The game is in an icky startup mode */

seed_t &seed_flavor()
{
	static seed_t *instance = new seed_t(seed_t::system());
	return *instance;
}

s16b command_cmd; 		/* Current "Angband Command" */

s16b command_arg; 		/* Gives argument of current command */
s16b command_rep; 		/* Gives repetition of current command */
s16b command_dir; 		/* Gives direction of current command */

s16b command_wrk; 		/* See "cmd1.c" */

s16b command_new; 		/* Command chaining from inven/equip view */

s32b energy_use;                 /* Energy use this turn */

bool_ create_up_stair; 	/* Auto-create "up stairs" */
bool_ create_down_stair; 	/* Auto-create "down stairs" */

bool_ create_up_shaft;  /* Auto-create "up shaft" */
bool_ create_down_shaft;        /* Auto-create "down shaft" */

bool_ msg_flag; 			/* Used in msg_print() for "buffering" */

bool_ alive; 				/* True if game is running */

bool_ death; 				/* True if player has died */

s16b running; 			/* Current counter for running, if any */
s16b resting; 			/* Current counter for resting, if any */

s16b cur_hgt; 			/* Current dungeon height */
s16b cur_wid; 			/* Current dungeon width */
s16b dun_level; 			/* Current dungeon level */
s16b old_dun_level;              /* Old dungeon level */
s16b num_repro; 			/* Current reproducer count */
s16b object_level; 		/* Current object creation level */
s16b monster_level; 		/* Current monster creation level */

s32b turn; 				/* Current game turn */
s32b old_turn; 			/* Turn when level began (feelings) */

bool_ wizard; 			/* Is the player currently in Wizard mode? */

u16b total_winner; 		/* Semi-Hack -- Game has been won */
u16b has_won;               /* Semi-Hack -- Game has been won */

u16b noscore; 			/* Track various "cheating" conditions */

bool_ inkey_base; 		/* See the "inkey()" function */

s16b coin_type; 			/* Hack -- force coin type */

bool_ opening_chest; 		/* Hack -- prevent chest generation */

bool_ shimmer_monsters;           /* Hack -- optimize multi-hued monsters */
bool_ shimmer_objects;            /* Hack -- optimize multi-hued objects */

bool_ repair_monsters; 	/* Hack -- optimize detect monsters */

bool_ hack_mind;
int artifact_bias;
bool_ is_autosave = FALSE;

s16b inven_cnt; 			/* Number of items in inventory */
s16b equip_cnt; 			/* Number of items in equipment */

s16b o_max = 1; 			/* Number of allocated objects */
s16b o_cnt = 0; 			/* Number of live objects */

s16b m_max = 1; 			/* Number of allocated monsters */
s16b m_cnt = 0; 			/* Number of live monsters */

s16b hack_m_idx = 0; 	/* Hack -- see "process_monsters()" */
char summon_kin_type;    /* Hack, by Julian Lighton: summon 'relatives' */

int total_friends = 0;
s32b total_friend_levels = 0;

int leaving_quest = 0;



/*
 * Hack - the destination file for text_out_to_file.
 */
FILE *text_out_file = NULL;


/*
 * Hack -- function hook to output (colored) text to the
 * screen or to a file.
 */
void (*text_out_hook)(byte a, cptr str) = text_out_to_screen;


/*
 * Hack -- Indentation for the text when using text_out().
 */
int text_out_indent = 0;


/*
 * Options
 */
struct options *options = nullptr;

/*
 * Dungeon variables
 */

s16b feeling; 			/* Most recent feeling */
s16b rating; 			/* Level's current rating */

bool_ good_item_flag; 		/* True if "Artifact" on this level */

/*
 * Dungeon size info
 */

s16b max_panel_rows, max_panel_cols;
s16b panel_row_min, panel_row_max;
s16b panel_col_min, panel_col_max;
s16b panel_col_prt, panel_row_prt;

/*
 * Dungeon graphics info
 * Why the first two are byte and the rest s16b???
 */
byte feat_wall_outer = FEAT_WALL_OUTER; 	/* Outer wall of rooms */
byte feat_wall_inner = FEAT_WALL_INNER; 	/* Inner wall of rooms */
s16b floor_type[100]; 	/* Dungeon floor */
s16b fill_type[100]; 	/* Dungeon filler */

/*
 * Targetting variables
 */
s16b target_who;
s16b target_col;
s16b target_row;

/*
 * Health bar variable -DRS-
 */
s16b health_who;

/*
 * Monster race to track
 */
s16b monster_race_idx;
s16b monster_ego_idx;

/*
 * Object to track
 */
object_type *tracked_object;



/*
 * Current player's character name
 */
char player_name[32];

/*
 * Stripped version of "player_name"
 */
char player_base[32];

/*
 * What killed the player
 */
char died_from[80];

/*
 * Hack -- Textual "history" for the Player
 */
char history[4][60];

/*
 * Buffer to hold the current savefile name
 */
char savefile[1024];


/*
 * Array of grids lit by player lite (see "cave.c")
 */
s16b lite_n;
s16b lite_y[LITE_MAX];
s16b lite_x[LITE_MAX];

/*
 * Array of grids viewable to the player (see "cave.c")
 */
s16b view_n;
byte view_y[VIEW_MAX];
byte view_x[VIEW_MAX];

/*
 * Array of grids for use by various functions (see "cave.c")
 */
s16b temp_n;
byte temp_y[TEMP_MAX];
byte temp_x[TEMP_MAX];


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
char **macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
char **macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool_ *macro__cmd;

/*
 * Current macro action [1024]
 */
char *macro__buf;


/*
 * The array of window options
 */
u32b window_flag[8];
u32b window_mask[8];


/*
 * The array of window pointers
 */
term *angband_term[ANGBAND_TERM_MAX];


/*
 * Standard window names
 */
char angband_term_name[ANGBAND_TERM_MAX][80] =
{
	"ToME",
	"Mirror",
	"Recall",
	"Choice",
	"Term-4",
	"Term-5",
	"Term-6",
	"Term-7"
};


/*
 * Global table of color definitions
 */
byte angband_color_table[256][4] =
{
	{0x00, 0x00, 0x00, 0x00}, 	/* TERM_DARK */
	{0x00, 0xFF, 0xFF, 0xFF}, 	/* TERM_WHITE */
	{0x00, 0x80, 0x80, 0x80}, 	/* TERM_SLATE */
	{0x00, 0xFF, 0x80, 0x00}, 	/* TERM_ORANGE */
	{0x00, 0xC0, 0x00, 0x00}, 	/* TERM_RED */
	{0x00, 0x00, 0x80, 0x40}, 	/* TERM_GREEN */
	{0x00, 0x00, 0x00, 0xFF}, 	/* TERM_BLUE */
	{0x00, 0x80, 0x40, 0x00}, 	/* TERM_UMBER */
	{0x00, 0x40, 0x40, 0x40}, 	/* TERM_L_DARK */
	{0x00, 0xC0, 0xC0, 0xC0}, 	/* TERM_L_WHITE */
	{0x00, 0xFF, 0x00, 0xFF}, 	/* TERM_VIOLET */
	{0x00, 0xFF, 0xFF, 0x00}, 	/* TERM_YELLOW */
	{0x00, 0xFF, 0x00, 0x00}, 	/* TERM_L_RED */
	{0x00, 0x00, 0xFF, 0x00}, 	/* TERM_L_GREEN */
	{0x00, 0x00, 0xFF, 0xFF}, 	/* TERM_L_BLUE */
	{0x00, 0xC0, 0x80, 0x40}	/* TERM_L_UMBER */
};


/*
 * The array of "cave grids" [MAX_WID][MAX_HGT].
 */
cave_type **cave = nullptr;

/*
 * The array of dungeon items [max_o_idx]
 */
object_type *o_list;

/*
 * The array of dungeon monsters [max_m_idx]
 */
monster_type *m_list;

/*
 * The array of to keep monsters [max_m_idx]
 */
monster_type *km_list;


/*
 * Maximum number of towns
 */
u16b max_towns;
u16b max_real_towns;

/*
 * The towns [max_towns]
 */
town_type *town_info;

/*
 * The size of "alloc_kind_table" (at most max_k_idx * ALLOCATIONS_MAX)
 */
s16b alloc_kind_size;

/*
 * The entries in the "kind allocator table"
 */
alloc_entry *alloc_kind_table;

/*
 * The flag to tell if alloc_kind_table contains valid entries
 * for normal (i.e. kind_is_legal) object allocation
 */
bool_ alloc_kind_table_valid = FALSE;


/*
 * The size of "alloc_race_table" (at most max_r_idx)
 */
s16b alloc_race_size;

/*
 * The entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;


/*
 * Specify attr/char pairs for visual special effects
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte misc_to_attr[256];
char misc_to_char[256];


/*
 * Specify attr/char pairs for inventory items (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];
char tval_to_char[128];


/*
 * Keymaps for each "mode" associated with each keypress.
 */
char *keymap_act[KEYMAP_MODES][256];



/*** Player information ***/

/*
 * Pointer to the player info
 */
player_type *p_ptr = nullptr;

/*
 * Pointer to the player tables
 * (sex, race, race mod, class, magic)
 */
player_race const *rp_ptr;
player_race_mod const *rmp_ptr;
player_class const *cp_ptr;
player_spec const *spp_ptr;


/*
 * The terrain feature arrays
 */
feature_type *f_info;

/*
 * The object kind arrays
 */
object_kind *k_info;

/*
 * The artifact arrays
 */
artifact_type *a_info;

/*
 * The ego-item arrays
 */
ego_item_type *e_info;

/* jk */
/* the trap-arrays */
trap_type *t_info;

/*
 * The monster race arrays
 */
monster_race *r_info;

/*
 * The wilderness features arrays
 */
wilderness_type_info *wf_info;
int wildc2i[256];

/*
 * Default texts for feature information.
 */
cptr DEFAULT_FEAT_TEXT = "a wall blocking your way";
cptr DEFAULT_FEAT_TUNNEL = "You cannot tunnel through that.";
cptr DEFAULT_FEAT_BLOCK = DEFAULT_FEAT_TEXT;

/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
char *ANGBAND_DIR;

/*
 * Core lua system
 * These files are portable between platforms
 */
char *ANGBAND_DIR_CORE;

/*
 * Textual dungeon level definition files
 * These files are portable between platforms
 */
char *ANGBAND_DIR_DNGN;

/*
 * Binary image files for the "*_info" arrays (binary)
 * These files are not portable between platforms
 */
char *ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
char *ANGBAND_DIR_EDIT;

/*
 * Various extra files (ascii)
 * These files may be portable between platforms
 */
char *ANGBAND_DIR_FILE;

/*
 * Help files (normal) for the online help (ascii)
 * These files are portable between platforms
 */
char *ANGBAND_DIR_HELP;

/*
 * Help files (spoilers) for the online help (ascii)
 * These files are portable between platforms
 */
char *ANGBAND_DIR_INFO;

/*
 * Modules, those subdirectories are half-mirrors of lib/
 */
char *ANGBAND_DIR_MODULES;

/*
 * Textual template files for the plot files (ascii)
 * These files are portable between platforms
 */
char *ANGBAND_DIR_NOTE;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
char *ANGBAND_DIR_SAVE;

/*
 * Default "preference" files (ascii)
 * These files are rarely portable between platforms
 */
char *ANGBAND_DIR_PREF;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
char *ANGBAND_DIR_USER;

/*
 * Various extra files (binary)
 * These files are rarely portable between platforms
 */
char *ANGBAND_DIR_XTRA;



/*
 * Hack -- function hooks to restrict "get_mon_num_prep()" function
 */
bool_ (*get_mon_num_hook)(int r_idx);
bool_ (*get_mon_num2_hook)(int r_idx);


/*
 * Hack -- function hook to restrict "get_obj_num_prep()" function
 */
bool_ (*get_obj_num_hook)(int k_idx);

/*
 * Devices
 */
s32b get_level_max_stick = -1;
s32b get_level_use_stick = -1;

/*
 * Maximum number of monsters in r_info.txt
 */
u16b max_r_idx;

/*
 * Maximum number of items in k_info.txt
 */
u16b max_k_idx;

/*
 * Maximum number of terrain features in f_info.txt
 */
u16b max_f_idx;

/*
 * Maximum number of artifacts in a_info.txt
 */
u16b max_a_idx;

/*
 * Maximum number of ego-items in e_info.txt
 */
u16b max_e_idx;

/*
 * Maximum number of objects in the level
 */
u16b max_o_idx;

/*
 * Maximum number of monsters in the level
 */
u16b max_m_idx;

/*
 * Maximum number of traps in tr_info.txt
 */
u16b max_t_idx;

/*
 * Maximum number of wilderness features in wf_info.txt
 */
u16b max_wf_idx;

/*
 * Flags for initialization
 */
int init_flags;

/* True if on an ambush */
bool_ ambush_flag;

/* True if on fated level */
bool_ fate_flag;

/* No breeders */
s16b no_breeds;

/* Carried monsters can't take the damage if this is them which attack the player */
bool_ carried_monster_hit = FALSE;

/*
 * Random artifacts.
 */
random_artifact random_artifacts[MAX_RANDARTS];
/* These three used to be constants but now are set by modules */
s32b RANDART_WEAPON;
s32b RANDART_ARMOR;
s32b RANDART_JEWEL;

/*
 * Fate.
 */
fate fates[MAX_FATES];

/*
 * Which dungeon ?
 * 0 = Wilderness
 * 1 = Mirkwood
 * 2 = Mordor
 * 3 = Angband
 * 4 = Barrow Downs
 * 5 = Mount Doom
 * 6 = Nether Realm
 * etc. (see d_info.txt)
 */
byte dungeon_type;
s16b *max_dlv;

/* The Doppleganger index in m_list */
s16b doppleganger;

/* To allow wilderness encounters */
bool_ generate_encounter = FALSE;

/*
 * Such an ugly hack ...
 */
bool_ *m_allow_special;
bool_ *k_allow_special;
bool_ *a_allow_special;

/*
 * Plots
 */
s16b plots[MAX_PLOTS];

/*
 * Random quest
 */
random_quest random_quests[MAX_RANDOM_QUEST];

/*
 * Special levels
 */
bool_ *special_lvl[MAX_DUNGEON_DEPTH];
bool_ generate_special_feeling = FALSE;

/*
 * Dungeon flags
 */
DECLARE_FLAG_ZERO_IMPL(dungeon_flag_set, dungeon_flags);

/*
 * The last character displayed
 */
birther previous_char;

/*
 * The spell list of schools
 */
s16b schools_count = 0;
school_type schools[SCHOOLS_MAX];

/*
 * Lasting spell effects
 */
int project_time = 0;
s32b project_time_effect = 0;
effect_type effects[MAX_EFFECTS];

/*
 * Table of "cli" macros.
 */
cli_comm *cli_info;
int cli_total = 0;

/*
 * Automatizer enabled status
 */
bool_ automatizer_enabled = FALSE;
bool_ automatizer_create = FALSE;

/*
 * Location of the last teleportation thath affected the level
 */
s16b last_teleportation_y = -1;
s16b last_teleportation_x = -1;

/*
 * The current game module
 */
cptr game_module;
s32b game_module_idx;
s32b VERSION_MAJOR;
s32b VERSION_MINOR;
s32b VERSION_PATCH;

/*
 * Some module info
 */
s32b DUNGEON_BASE = 4;
s32b DUNGEON_DEATH = 28;
s32b DUNGEON_ASTRAL = 8;
s32b DUNGEON_ASTRAL_WILD_X = 45;
s32b DUNGEON_ASTRAL_WILD_Y = 19;

/*
 * Timers
 */
timer_type *gl_timers = NULL;


/**
 * Get the version string.
 */
const char *get_version_string()
{
	static char version_str[80];
	static bool_ initialized = 0;
	if (!initialized) {
		sprintf(version_str, "%s %ld.%ld.%ld%s",
		        game_module,
			(long int) VERSION_MAJOR,
			(long int) VERSION_MINOR,
			(long int) VERSION_PATCH, IS_CVS);
		initialized = TRUE;
	}
	return version_str;
}
