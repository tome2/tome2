#include "options.hpp"

//
// Option Set 1 -- User Interface
//
bool_ rogue_like_commands; 	/* Rogue-like commands */
bool_ quick_messages; 		/* Activate quick messages */
bool_ carry_query_flag; 		/* Prompt before picking things up */
bool_ use_old_target; 		/* Use old target by default */
bool_ always_pickup;              /* Pick things up by default */
bool_ prompt_pickup_heavy;        /* Don't pick up the corpses */
bool_ always_repeat;              /* Repeat obvious commands */
bool_ ring_bell; 				/* Ring the bell (on errors, etc) */

//
// Option Set 2 -- Disturbance
//
bool_ find_ignore_stairs; 	/* Run past stairs */
bool_ find_ignore_doors; 		/* Run through open doors */
bool_ find_cut; 				/* Run past known corners */
bool_ find_examine; 			/* Run into potential corners */
bool_ disturb_move; 			/* Disturb whenever any monster moves */
bool_ disturb_near; 			/* Disturb whenever viewable monster moves */
bool_ disturb_panel; 			/* Disturb whenever map panel changes */
bool_ disturb_detect; 		/* Disturb whenever leaving trap-detected area */
bool_ disturb_state; 			/* Disturn whenever player state changes */
bool_ disturb_minor; 			/* Disturb whenever boring things happen */
bool_ disturb_other; 			/* Disturb whenever various things happen */
bool_ alert_hitpoint; 		/* Alert user to critical hitpoints */
bool_ alert_failure; 		/* Alert user to various failures */
bool_ last_words; 		/* Get last words upon dying */
bool_ small_levels; 		/* Allow unusually small dungeon levels */
bool_ empty_levels; 		/* Allow empty 'arena' levels */
bool_ always_small_level;         /* Small levels */
bool_ confirm_stairs; 		/* Prompt before staircases... */
bool_ wear_confirm; 		/* Confirm before putting on known cursed items */
bool_ disturb_pets; 		/* Pets moving nearby disturb us */

//
// Option Set 3 -- Game-Play
//
bool_ auto_scum; 				/* Auto-scum for good levels */
bool_ expand_look; 			/* Expand the power of the look command */
bool_ expand_list; 			/* Expand the power of the list commands */
bool_ view_perma_grids; 		/* Map remembers all perma-lit grids */
bool_ view_torch_grids; 		/* Map remembers all torch-lit grids */
bool_ dungeon_align; 			/* Generate dungeons with aligned rooms */
bool_ dungeon_stair; 			/* Generate dungeons with connected stairs */
bool_ flow_by_sound; 			/* Monsters track new player location */
bool_ smart_learn; 			/* Monsters learn from their mistakes */

//
// Option Set 4 -- Efficiency
//
bool_ view_reduce_lite; 		/* Reduce lite-radius when running */
bool_ avoid_abort; 			/* Avoid checking for user abort */
bool_ avoid_shimmer; 			/* Avoid processing extra shimmering */
bool_ avoid_other; 			/* Avoid processing special colors */
bool_ flush_failure; 			/* Flush input on any failure */
bool_ flush_disturb; 			/* Flush input on disturbance */
bool_ flush_command; 			/* Flush input before every command */
bool_ fresh_before; 			/* Flush output before normal commands */
bool_ fresh_after; 			/* Flush output after normal commands */
bool_ fresh_message; 			/* Flush output after all messages */
bool_ hilite_player; 			/* Hilite the player with the cursor */
bool_ view_yellow_lite; 		/* Use special colors for torch-lit grids */
bool_ view_bright_lite; 		/* Use special colors for 'viewable' grids */
bool_ view_granite_lite; 		/* Use special colors for wall grids (slow) */
bool_ view_special_lite; 		/* Use special colors for floor grids (slow) */
