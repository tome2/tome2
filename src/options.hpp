#pragma once

#include "h-basic.hpp"
#include "option_type.hpp"

#include <vector>

/**
 * Game options accessible via the '=' menu.
 */
struct options {

	//
	// Option Set 1 -- User Interface
	//
	bool rogue_like_commands = false;       /* Rogue-like commands */
	bool quick_messages = true;             /* Activate quick messages */
	bool carry_query_flag = false;          /* Prompt before picking things up */
	bool use_old_target = false;            /* Use old target by default */
	bool always_pickup = false;             /* Pick things up by default */
	bool always_repeat = true;              /* Repeat obvious commands */
	bool ring_bell = false;                 /* Ring the bell (on errors, etc) */

	//
	// Option Set 2 -- Disturbance
	//
	bool find_ignore_stairs = false;        /* Run past stairs */
	bool find_ignore_doors = true;          /* Run through open doors */
	bool find_cut = false;                  /* Run past known corners */
	bool find_examine = true;               /* Run into potential corners */
	bool disturb_move = false;              /* Disturb whenever any monster moves */
	bool disturb_near = true;               /* Disturb whenever viewable monster moves */
	bool disturb_panel = true;              /* Disturb whenever map panel changes */
	bool disturb_state = true;              /* Disturn whenever player state changes */
	bool disturb_minor = true;              /* Disturb whenever boring things happen */
	bool disturb_other = false;             /* Disturb whenever various things happen */
	bool last_words = true;                 /* Get last words upon dying */
	bool wear_confirm = true;               /* Confirm before putting on known cursed items */
	bool confirm_stairs = false;            /* Prompt before staircases... */
	bool disturb_pets = false;              /* Pets moving nearby disturb us */

	//
	// Option Set 3 -- Game-Play
	//
	bool auto_scum = true;                  /* Auto-scum for good levels */
	bool view_perma_grids = true;           /* Map remembers all perma-lit grids */
	bool view_torch_grids = false;          /* Map remembers all torch-lit grids */
	bool dungeon_align = true;              /* Generate dungeons with aligned rooms */
	bool dungeon_stair = true;              /* Generate dungeons with connected stairs */
	bool flow_by_sound = false;             /* Monsters track new player location */
	bool smart_learn = false;               /* Monsters learn from their mistakes */
	bool small_levels = true;               /* Allow unusually small dungeon levels */
	bool empty_levels = true;               /* Allow empty 'arena' levels */

	//
	// Option Set 4 -- Efficiency
	//
	bool view_reduce_lite = false;          /* Reduce lite-radius when running */
	bool avoid_abort = false;               /* Avoid checking for user abort */
	bool avoid_shimmer = false;             /* Avoid processing extra shimmering */
	bool avoid_other = false;               /* Avoid processing special colors */
	bool flush_failure = true;              /* Flush input on any failure */
	bool flush_disturb = false;             /* Flush input on disturbance */
	bool flush_command = false;             /* Flush input before every command */
	bool fresh_before = true;               /* Flush output before normal commands */
	bool fresh_after = false;               /* Flush output after normal commands */
	bool fresh_message = false;             /* Flush output after all messages */
	bool hilite_player = false;             /* Hilite the player with the cursor */
	bool view_yellow_lite = false;          /* Use special colors for torch-lit grids */
	bool view_bright_lite = false;          /* Use special colors for 'viewable' grids */
	bool view_granite_lite = false;         /* Use special colors for wall grids (slow) */
	bool view_special_lite = false;         /* Use special colors for floor grids (slow) */
	bool center_player = false;             /* Center view on player */

	//
	// Option Set 5 - ToME options
	//
	bool ingame_help = true;                /* In-game contextual help? */
	bool auto_more = false;                 /* Auto more */
	bool player_char_health = true;         /* Display the player as a special symbol when in bad health ? */
	bool linear_stats = true;

	//
	// Option Set 6 - Birth options
	//
	bool preserve = true;                   /* Preserve artifacts */
	bool autoroll = true;                   /* Specify 'minimal' stats to roll */
	bool point_based = false;		 /* Generate character using a point system */
	bool ironman_rooms = false;		 /* Always generate very unusual rooms */
	bool joke_monsters = false;		 /* Allow 'joke' monsters */
	bool always_small_level = false;	 /* Force small levels */
	bool fate_option = true;		 /* Player can receive fates */
	bool no_selling = false;		 /* Player cannot sell items */

	//
	// Other options
	//

	bool cheat_peek = false;                /* Peek into object creation */
	bool cheat_hear = false;                /* Peek into monster creation */
	bool cheat_room = false;                /* Peek into dungeon creation */
	bool cheat_xtra = false;                /* Peek into something else */
	bool cheat_live = false;                /* Allow player to avoid death */

	byte hitpoint_warn = 0;                  /* Hitpoint warning (0 to 9) */

	byte delay_factor = 0;                   /* Delay factor (0 to 9) */

	s16b autosave_freq = 100;                /* Autosave frequency */
	bool autosave_t = false;                /* Timed autosave */
	bool autosave_l = false;                /* Autosave before entering new levels */

	/**
	 * Option groups
	 */
	std::vector<option_type> standard_options = {
	        // User-Interface
	        { &rogue_like_commands, 1,  0, "rogue_like_commands", "Rogue-like commands" },
	        { &quick_messages     , 1,  1, "quick_messages"     , "Activate quick messages" },
	        { &carry_query_flag   , 1,  3, "carry_query_flag"   , "Prompt before picking things up" },
	        { &use_old_target     , 1,  4, "use_old_target"     , "Use old target by default" },
	        { &always_pickup      , 1,  5, "always_pickup"      , "Pick things up by default" },
	        { &always_repeat      , 1,  7, "always_repeat"      , "Repeat obvious commands" },
	        { &ring_bell          , 1, 18, "ring_bell"          , "Audible bell (on errors, etc)" },
	        // Disturbance
	        { &find_ignore_stairs , 2,  0, "find_ignore_stairs" , "Run past stairs" },
	        { &find_ignore_doors  , 2,  1, "find_ignore_doors"  , "Run through open doors" },
	        { &find_cut           , 2,  2, "find_cut"           , "Run past known corners" },
	        { &find_examine       , 2,  3, "find_examine"       , "Run into potential corners" },
	        { &disturb_move       , 2,  4, "disturb_move"       , "Disturb whenever any monster moves" },
	        { &disturb_near       , 2,  5, "disturb_near"       , "Disturb whenever viewable monster moves" },
	        { &disturb_panel      , 2,  6, "disturb_panel"      , "Disturb whenever map panel changes" },
	        { &disturb_state      , 2,  7, "disturb_state"      , "Disturb whenever player state changes" },
	        { &disturb_minor      , 2,  8, "disturb_minor"      , "Disturb whenever boring things happen" },
	        { &disturb_other      , 2,  9, "disturb_other"      , "Disturb whenever random things happen" },
	        { &last_words         , 2, 12, "last_words"         , "Get last words when the character dies" },
	        { &wear_confirm       , 2, 15, "confirm_wear"       , "Confirm to wear/wield known cursed items" },
	        { &confirm_stairs     , 2, 16, "confirm_stairs"     , "Prompt before exiting a dungeon level" },
	        { &disturb_pets       , 2, 17, "disturb_pets"       , "Disturb when visible pets move" },
	        // Game-Play
	        { &auto_scum          , 3,  1, "auto_scum"          , "Auto-scum for good levels" },
	        { &view_perma_grids   , 3,  6, "view_perma_grids"   , "Map remembers all perma-lit grids" },
	        { &view_torch_grids   , 3,  7, "view_torch_grids"   , "Map remembers all torch-lit grids" },
	        { &dungeon_align      , 3,  8, "dungeon_align"      , "Generate dungeons with aligned rooms" },
	        { &dungeon_stair      , 3,  9, "dungeon_stair"      , "Generate dungeons with connected stairs" },
	        { &flow_by_sound      , 3, 10, "flow_by_sound"      , "Monsters chase current location (v.slow)" },
	        { &smart_learn        , 3, 14, "smart_learn"        , "Monsters learn from their mistakes" },
	        { &small_levels       , 3, 17, "small_levels"       , "Allow unusually small dungeon levels" },
	        { &empty_levels       , 3, 18, "empty_levels"       , "Allow empty 'arena' levels" },
	        // Efficiency
	        { &view_reduce_lite   , 4,  0, "view_reduce_lite"   , "Reduce lite-radius when running" },
	        { &avoid_abort        , 4,  2, "avoid_abort"        , "Avoid checking for user abort" },
	        { &avoid_shimmer      , 4, 17, "avoid_shimmer"      , "Avoid extra shimmering (fast)" },
	        { &avoid_other        , 4,  3, "avoid_other"        , "Avoid processing special colors (fast)" },
	        { &flush_failure      , 4,  4, "flush_failure"      , "Flush input on various failures" },
	        { &flush_disturb      , 4,  5, "flush_disturb"      , "Flush input whenever disturbed" },
	        { &flush_command      , 4,  6, "flush_command"      , "Flush input before every command" },
	        { &fresh_before       , 4,  7, "fresh_before"       , "Flush output before every command" },
	        { &fresh_after        , 4,  8, "fresh_after"        , "Flush output after every command" },
	        { &fresh_message      , 4,  9, "fresh_message"      , "Flush output after every message" },
	        { &hilite_player      , 4, 11, "hilite_player"      , "Hilite the player with the cursor" },
	        { &view_yellow_lite   , 4, 12, "view_yellow_lite"   , "Use special colors for torch-lit grids" },
	        { &view_bright_lite   , 4, 13, "view_bright_lite"   , "Use special colors for 'viewable' grids" },
	        { &view_granite_lite  , 4, 14, "view_granite_lite"  , "Use special colors for wall grids (slow)" },
	        { &view_special_lite  , 4, 15, "view_special_lite"  , "Use special colors for floor grids (slow)" },
	        { &center_player      , 4, 16, "center_player"      , "Center the view on the player (very slow)" },
	        // ToME options
	        { &ingame_help        , 5,  1, "ingame_help"        , "Ingame contextual help" },
	        { &auto_more          , 5,  4, "auto_more"          , "Automatically clear '-more-' prompts" },
	        { &player_char_health , 5,  6, "player_char_health" , "Player char represent his/her health" },
	        { &linear_stats       , 5,  7, "linear_stats"       , "Stats are represented in a linear way" },
	        // Birth Options
	        { &preserve           , 6,  2, "preserve"           , "Preserve artifacts" },
	        { &autoroll           , 6,  3, "autoroll"           , "Specify 'minimal' stats" },
	        { &point_based        , 6, 17, "point_based"        , "Generate character using a point system" },
	        { &ironman_rooms      , 6,  6, "ironman_rooms"      , "Always generate very unusual rooms" },
	        { &joke_monsters      , 6, 14, "joke_monsters"      , "Allow use of some 'joke' monsters" },
	        { &always_small_level , 6, 16, "always_small_level" , "Always make small levels" },
	        { &fate_option        , 6, 18, "fate_option"        , "You can receive fates, good or bad" },
	        { &no_selling         , 6, 20, "no_selling"         , "Items always sell for 0 gold" },
	};

	/*
	 * Cheating options
	 */
	std::vector<option_type> cheat_options = {
	        { &cheat_peek, 0, 0, "cheat_peek", "Peek into object creation" },
	        { &cheat_hear, 0, 1, "cheat_hear", "Peek into monster creation" },
	        { &cheat_room, 0, 2, "cheat_room", "Peek into dungeon creation" },
	        { &cheat_xtra, 0, 3, "cheat_xtra", "Peek into something else" },
	        { &cheat_live, 0, 5, "cheat_live", "Allow player to avoid death" },
        };

	/**
	 * Autosave boolean options
	 */
	std::vector<option_type> autosave_options {
		{ &autosave_l, 0, 6, "autosave_l", "Autosave when entering new levels" },
		{ &autosave_t, 0, 7, "autosave_t", "Timed autosave" }
        };

	/*
	 * Reset cheat options
	 */
	void reset_cheat_options();

	/**
	 * Convert delay_factor to milliseconds
	 */
	int delay_factor_ms() const
	{
		return delay_factor * delay_factor * delay_factor;
	}

};
