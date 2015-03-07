#pragma once

#include "h-basic.h"

//
// Option Set 1 -- User Interface.
//
extern bool_ rogue_like_commands;
extern bool_ quick_messages;
extern bool_ carry_query_flag;
extern bool_ use_old_target;
extern bool_ always_pickup;
extern bool_ prompt_pickup_heavy;
extern bool_ always_repeat;
extern bool_ ring_bell;

//
// Option Set 2 -- Disturbance
//
extern bool_ find_ignore_stairs;
extern bool_ find_ignore_doors;
extern bool_ find_cut;
extern bool_ find_examine;
extern bool_ disturb_move;
extern bool_ disturb_near;
extern bool_ disturb_panel;
extern bool_ disturb_detect;
extern bool_ disturb_state;
extern bool_ disturb_minor;
extern bool_ disturb_other;
extern bool_ alert_hitpoint;
extern bool_ alert_failure;
extern bool_ last_words;
extern bool_ small_levels;
extern bool_ empty_levels;
extern bool_ always_small_level;
extern bool_ confirm_stairs;
extern bool_ wear_confirm;
extern bool_ disturb_pets;

//
// Option Set 3 -- Game-Play
//
extern bool_ auto_scum;
extern bool_ expand_look;
extern bool_ expand_list;
extern bool_ view_perma_grids;
extern bool_ view_torch_grids;
extern bool_ dungeon_align;
extern bool_ dungeon_stair;
extern bool_ flow_by_sound;
extern bool_ smart_learn;

//
// Option Set 4 -- Efficiency
//
extern bool_ view_reduce_lite;
extern bool_ avoid_abort;
extern bool_ avoid_shimmer;
extern bool_ avoid_other;
extern bool_ flush_failure;
extern bool_ flush_disturb;
extern bool_ flush_command;
extern bool_ fresh_before;
extern bool_ fresh_after;
extern bool_ fresh_message;
extern bool_ hilite_player;
extern bool_ view_yellow_lite;
extern bool_ view_bright_lite;
extern bool_ view_granite_lite;
extern bool_ view_special_lite;

//
// Option Set 5 - ToME options
//
extern bool_ linear_stats;
extern bool_ player_char_health;
extern bool_ option_ingame_help;
extern bool_ auto_more;
extern bool_ inventory_no_move;
