#pragma once

#include "h-basic.h"
#include "option_type.hpp"

#include <vector>

/**
 * Game options accessible via the '=' menu.
 */
struct options {

	//
	// Option Set 1 -- User Interface
	//
	bool_ rogue_like_commands;               /* Rogue-like commands */
	bool_ quick_messages;                    /* Activate quick messages */
	bool_ carry_query_flag;                  /* Prompt before picking things up */
	bool_ use_old_target;                    /* Use old target by default */
	bool_ always_pickup;                     /* Pick things up by default */
	bool_ always_repeat;                     /* Repeat obvious commands */
	bool_ ring_bell;                         /* Ring the bell (on errors, etc) */

	//
	// Option Set 2 -- Disturbance
	//
	bool_ find_ignore_stairs;                /* Run past stairs */
	bool_ find_ignore_doors;                 /* Run through open doors */
	bool_ find_cut;                          /* Run past known corners */
	bool_ find_examine;                      /* Run into potential corners */
	bool_ disturb_move;                      /* Disturb whenever any monster moves */
	bool_ disturb_near;                      /* Disturb whenever viewable monster moves */
	bool_ disturb_panel;                     /* Disturb whenever map panel changes */
	bool_ disturb_detect;                    /* Disturb whenever leaving trap-detected area */
	bool_ disturb_state;                     /* Disturn whenever player state changes */
	bool_ disturb_minor;                     /* Disturb whenever boring things happen */
	bool_ disturb_other;                     /* Disturb whenever various things happen */
	bool_ last_words;                        /* Get last words upon dying */
	bool_ small_levels;                      /* Allow unusually small dungeon levels */
	bool_ empty_levels;                      /* Allow empty 'arena' levels */
	bool_ confirm_stairs;                    /* Prompt before staircases... */
	bool_ wear_confirm;                      /* Confirm before putting on known cursed items */
	bool_ disturb_pets;                      /* Pets moving nearby disturb us */

	//
	// Option Set 3 -- Game-Play
	//
	bool_ auto_scum;                         /* Auto-scum for good levels */
	bool_ view_perma_grids;                  /* Map remembers all perma-lit grids */
	bool_ view_torch_grids;                  /* Map remembers all torch-lit grids */
	bool_ dungeon_align;                     /* Generate dungeons with aligned rooms */
	bool_ dungeon_stair;                     /* Generate dungeons with connected stairs */
	bool_ flow_by_sound;                     /* Monsters track new player location */
	bool_ smart_learn;                       /* Monsters learn from their mistakes */

	//
	// Option Set 4 -- Efficiency
	//
	bool_ view_reduce_lite;                  /* Reduce lite-radius when running */
	bool_ avoid_abort;                       /* Avoid checking for user abort */
	bool_ avoid_shimmer;                     /* Avoid processing extra shimmering */
	bool_ avoid_other;                       /* Avoid processing special colors */
	bool_ flush_failure;                     /* Flush input on any failure */
	bool_ flush_disturb;                     /* Flush input on disturbance */
	bool_ flush_command;                     /* Flush input before every command */
	bool_ fresh_before;                      /* Flush output before normal commands */
	bool_ fresh_after;                       /* Flush output after normal commands */
	bool_ fresh_message;                     /* Flush output after all messages */
	bool_ hilite_player;                     /* Hilite the player with the cursor */
	bool_ view_yellow_lite;                  /* Use special colors for torch-lit grids */
	bool_ view_bright_lite;                  /* Use special colors for 'viewable' grids */
	bool_ view_granite_lite;                 /* Use special colors for wall grids (slow) */
	bool_ view_special_lite;                 /* Use special colors for floor grids (slow) */
	bool_ center_player;                     /* Center view on player */

	//
	// Option Set 5 - ToME options
	//
	bool_ linear_stats;
	bool_ player_char_health;                /* Display the player as a special symbol when in bad health ? */
	bool_ ingame_help;                       /* In-game contextual help? */
	bool_ auto_more;                         /* Auto more */

	//
	// Option Set 6 - Birth options
	//
	bool_ always_small_level;
	bool_ autoroll;
	bool_ fate_option;
	bool_ ironman_rooms;
	bool_ joke_monsters;
	bool_ point_based;
	bool_ preserve;
	bool_ no_selling;

	//
	// Other options
	//

	bool_ cheat_peek;                      /* Peek into object creation */
	bool_ cheat_hear;                      /* Peek into monster creation */
	bool_ cheat_room;                      /* Peek into dungeon creation */
	bool_ cheat_xtra;                      /* Peek into something else */
	bool_ cheat_live;                      /* Allow player to avoid death */

	byte hitpoint_warn;                    /* Hitpoint warning (0 to 9) */

	byte delay_factor;                     /* Delay factor (0 to 9) */

	s16b autosave_freq;                    /* Autosave frequency */
	bool_ autosave_t;                      /* Timed autosave */
	bool_ autosave_l;                      /* Autosave before entering new levels */

	/**
	 * Option groups
	 */
	std::vector<option_type> standard_options = {
	        // User-Interface
	        { &rogue_like_commands, FALSE, 1,  0, "rogue_like_commands", "Rogue-like commands" },
	        { &quick_messages     , TRUE , 1,  1, "quick_messages"     , "Activate quick messages" },
	        { &carry_query_flag   , FALSE, 1,  3, "carry_query_flag"   , "Prompt before picking things up" },
	        { &use_old_target     , FALSE, 1,  4, "use_old_target"     , "Use old target by default" },
	        { &always_pickup      , FALSE, 1,  5, "always_pickup"      , "Pick things up by default" },
	        { &always_repeat      , TRUE , 1,  7, "always_repeat"      , "Repeat obvious commands" },
	        { &ring_bell          , FALSE, 1, 18, "ring_bell"          , "Audible bell (on errors, etc)" },
	        // Disturbance
	        { &find_ignore_stairs , FALSE, 2,  0, "find_ignore_stairs" , "Run past stairs" },
	        { &find_ignore_doors  , TRUE , 2,  1, "find_ignore_doors"  , "Run through open doors" },
	        { &find_cut           , FALSE, 2,  2, "find_cut"           , "Run past known corners" },
	        { &find_examine       , TRUE , 2,  3, "find_examine"       , "Run into potential corners" },
	        { &disturb_move       , FALSE, 2,  4, "disturb_move"       , "Disturb whenever any monster moves" },
	        { &disturb_near       , TRUE , 2,  5, "disturb_near"       , "Disturb whenever viewable monster moves" },
	        { &disturb_panel      , TRUE , 2,  6, "disturb_panel"      , "Disturb whenever map panel changes" },
	        { &disturb_detect     , TRUE , 2, 21, "disturb_detect"     , "Disturb whenever leaving trap-detected area" },
	        { &disturb_state      , TRUE , 2,  7, "disturb_state"      , "Disturb whenever player state changes" },
	        { &disturb_minor      , TRUE , 2,  8, "disturb_minor"      , "Disturb whenever boring things happen" },
	        { &disturb_other      , FALSE, 2,  9, "disturb_other"      , "Disturb whenever random things happen" },
	        { &last_words         , TRUE , 2, 12, "last_words"         , "Get last words when the character dies" },
	        { &wear_confirm       , TRUE , 2, 15, "confirm_wear"       , "Confirm to wear/wield known cursed items" },
	        { &confirm_stairs     , FALSE, 2, 16, "confirm_stairs"     , "Prompt before exiting a dungeon level" },
	        { &disturb_pets       , FALSE, 2, 17, "disturb_pets"       , "Disturb when visible pets move" },
	        // Game-Play
	        { &auto_scum          , TRUE , 3,  1, "auto_scum"          , "Auto-scum for good levels" },
	        { &view_perma_grids   , TRUE , 3,  6, "view_perma_grids"   , "Map remembers all perma-lit grids" },
	        { &view_torch_grids   , FALSE, 3,  7, "view_torch_grids"   , "Map remembers all torch-lit grids" },
	        { &dungeon_align      , TRUE , 3,  8, "dungeon_align"      , "Generate dungeons with aligned rooms" },
	        { &dungeon_stair      , TRUE , 3,  9, "dungeon_stair"      , "Generate dungeons with connected stairs" },
	        { &flow_by_sound      , FALSE, 3, 10, "flow_by_sound"      , "Monsters chase current location (v.slow)" },
	        { &smart_learn        , FALSE, 3, 14, "smart_learn"        , "Monsters learn from their mistakes" },
	        { &small_levels       , TRUE , 3, 17, "small_levels"       , "Allow unusually small dungeon levels" },
	        { &empty_levels       , TRUE , 3, 18, "empty_levels"       , "Allow empty 'arena' levels" },
	        // Efficiency
	        { &view_reduce_lite   , FALSE, 4,  0, "view_reduce_lite"   , "Reduce lite-radius when running" },
	        { &avoid_abort        , FALSE, 4,  2, "avoid_abort"        , "Avoid checking for user abort" },
	        { &avoid_shimmer      , FALSE, 4, 17, "avoid_shimmer"      , "Avoid extra shimmering (fast)" },
	        { &avoid_other        , FALSE, 4,  3, "avoid_other"        , "Avoid processing special colors (fast)" },
	        { &flush_failure      , TRUE , 4,  4, "flush_failure"      , "Flush input on various failures" },
	        { &flush_disturb      , FALSE, 4,  5, "flush_disturb"      , "Flush input whenever disturbed" },
	        { &flush_command      , FALSE, 4,  6, "flush_command"      , "Flush input before every command" },
	        { &fresh_before       , TRUE , 4,  7, "fresh_before"       , "Flush output before every command" },
	        { &fresh_after        , FALSE, 4,  8, "fresh_after"        , "Flush output after every command" },
	        { &fresh_message      , FALSE, 4,  9, "fresh_message"      , "Flush output after every message" },
	        { &hilite_player      , FALSE, 4, 11, "hilite_player"      , "Hilite the player with the cursor" },
	        { &view_yellow_lite   , FALSE, 4, 12, "view_yellow_lite"   , "Use special colors for torch-lit grids" },
	        { &view_bright_lite   , FALSE, 4, 13, "view_bright_lite"   , "Use special colors for 'viewable' grids" },
	        { &view_granite_lite  , FALSE, 4, 14, "view_granite_lite"  , "Use special colors for wall grids (slow)" },
	        { &view_special_lite  , FALSE, 4, 15, "view_special_lite"  , "Use special colors for floor grids (slow)" },
	        { &center_player      , FALSE, 4, 16, "center_player"      , "Center the view on the player (very slow)" },
	        // ToME options
	        { &ingame_help        , TRUE , 5,  1, "ingame_help"        , "Ingame contextual help" },
	        { &auto_more          , FALSE, 5,  4, "auto_more"          , "Automatically clear '-more-' prompts" },
	        { &player_char_health , TRUE , 5,  6, "player_char_health" , "Player char represent his/her health" },
	        { &linear_stats       , TRUE , 5,  7, "linear_stats"       , "Stats are represented in a linear way" },
	        // Birth Options
	        { &preserve           , TRUE , 6,  2, "preserve"           , "Preserve artifacts" },
	        { &autoroll           , TRUE , 6,  3, "autoroll"           , "Specify 'minimal' stats" },
	        { &point_based        , FALSE, 6, 17, "point_based"        , "Generate character using a point system" },
	        { &ironman_rooms      , FALSE, 6,  6, "ironman_rooms"      , "Always generate very unusual rooms" },
	        { &joke_monsters      , FALSE, 6, 14, "joke_monsters"      , "Allow use of some 'joke' monsters" },
	        { &always_small_level , FALSE, 6, 16, "always_small_level" , "Always make small levels" },
	        { &fate_option        , TRUE , 6, 18, "fate_option"        , "You can receive fates, good or bad" },
	        { &no_selling         , FALSE, 6, 20, "no_selling"         , "Items always sell for 0 gold" },
	};

	/*
	 * Cheating options
	 */
	std::vector<option_type> cheat_options = {
	        { &cheat_peek, FALSE, 0, 0, "cheat_peek", "Peek into object creation" },
	        { &cheat_hear, FALSE, 0, 1, "cheat_hear", "Peek into monster creation" },
	        { &cheat_room, FALSE, 0, 2, "cheat_room", "Peek into dungeon creation" },
	        { &cheat_xtra, FALSE, 0, 3, "cheat_xtra", "Peek into something else" },
	        { &cheat_live, FALSE, 0, 5, "cheat_live", "Allow player to avoid death" },
        };

	/**
	 * Autosave boolean options
	 */
	std::vector<option_type> autosave_options {
	        { &autosave_l, FALSE, 0, 6, "autosave_l", "Autosave when entering new levels" },
	        { &autosave_t, FALSE, 0, 7, "autosave_t", "Timed autosave" }
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
