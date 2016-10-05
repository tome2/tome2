#pragma once

#include "angband.h"
#include "ability_type_fwd.hpp"
#include "alloc_entry_fwd.hpp"
#include "artifact_type_fwd.hpp"
#include "birther.hpp"
#include "cave_type_fwd.hpp"
#include "deity_type.hpp"
#include "dungeon_flag_set.hpp"
#include "dungeon_info_type_fwd.hpp"
#include "effect_type.hpp"
#include "ego_item_type_fwd.hpp"
#include "fate.hpp"
#include "feature_type_fwd.hpp"
#include "hist_type_fwd.hpp"
#include "monster_ego_fwd.hpp"
#include "monster_race_fwd.hpp"
#include "monster_type_fwd.hpp"
#include "object_kind_fwd.hpp"
#include "object_type_fwd.hpp"
#include "options.hpp"
#include "player_class_fwd.hpp"
#include "player_defs.hpp"
#include "player_race_fwd.hpp"
#include "player_race_mod_fwd.hpp"
#include "player_spec_fwd.hpp"
#include "player_type_fwd.hpp"
#include "random_artifact.hpp"
#include "random_quest.hpp"
#include "school_type.hpp"
#include "set_type_fwd.hpp"
#include "skill_descriptor_fwd.hpp"
#include "skill_modifiers_fwd.hpp"
#include "skill_type_fwd.hpp"
#include "skills_defs.hpp"
#include "store_info_type_fwd.hpp"
#include "timer_type_fwd.hpp"
#include "town_type_fwd.hpp"
#include "trap_type_fwd.hpp"
#include "wilderness_type_info_fwd.hpp"
#include "seed.hpp"

extern int max_macrotrigger;
extern char *macro_template;
extern char *macro_modifier_chr;
extern char *macro_modifier_name[MAX_MACRO_MOD];
extern char *macro_trigger_name[MAX_MACRO_TRIG];
extern char *macro_trigger_keycode[2][MAX_MACRO_TRIG];
extern bool_ character_dungeon;
extern bool_ character_loaded;
extern bool_ character_xtra;
extern seed_t &seed_flavor();
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_wrk;
extern s16b command_new;
extern s32b energy_use;
extern bool_ create_up_stair;
extern bool_ create_down_stair;
extern bool_ create_up_shaft;
extern bool_ create_down_shaft;
extern bool_ alive;
extern bool_ death;
extern s16b running;
extern s16b resting;
extern s16b cur_hgt;
extern s16b cur_wid;
extern s16b dun_level;
extern s16b old_dun_level;
extern s16b num_repro;
extern s16b object_level;
extern s16b monster_level;
extern s32b turn;
extern s32b old_turn;
extern bool_ wizard;
extern u16b total_winner;
extern u16b has_won;
extern u16b noscore;
extern bool_ inkey_base;
extern s16b coin_type;
extern bool_ opening_chest;
extern bool_ shimmer_monsters;
extern bool_ shimmer_objects;
extern bool_ repair_monsters;
extern s16b inven_cnt;
extern s16b equip_cnt;
extern s16b o_max;
extern s16b o_cnt;
extern s16b m_max;
extern s16b m_cnt;
extern s16b hack_m_idx;
extern int total_friends;
extern s32b total_friend_levels;
extern int leaving_quest;
extern char summon_kin_type;
extern bool_ hack_mind;
extern bool_ is_autosave;
extern int artifact_bias;
extern FILE *text_out_file;
extern void (*text_out_hook)(byte a, cptr str);
extern int text_out_indent;
extern s16b feeling;
extern s16b rating;
extern bool_ good_item_flag;
extern s16b max_panel_rows, max_panel_cols;
extern s16b panel_row_min, panel_row_max;
extern s16b panel_col_min, panel_col_max;
extern s16b panel_col_prt, panel_row_prt;
extern byte feat_wall_outer;
extern byte feat_wall_inner;
extern s16b floor_type[100];
extern s16b fill_type[100];
extern s16b target_who;
extern s16b target_col;
extern s16b target_row;
extern s16b health_who;
extern s16b monster_race_idx;
extern s16b monster_ego_idx;
extern object_type *tracked_object;
extern char died_from[80];
extern char history[4][60];
extern s16b lite_n;
extern s16b lite_y[LITE_MAX];
extern s16b lite_x[LITE_MAX];
extern s16b view_n;
extern byte view_y[VIEW_MAX];
extern byte view_x[VIEW_MAX];
extern s16b temp_n;
extern byte temp_y[TEMP_MAX];
extern byte temp_x[TEMP_MAX];
extern s16b macro__num;
extern char **macro__pat;
extern char **macro__act;
extern bool_ *macro__cmd;
extern char *macro__buf;
extern u32b window_flag[ANGBAND_TERM_MAX];
extern u32b window_mask[ANGBAND_TERM_MAX];
extern cave_type **cave;
extern object_type *o_list;
extern monster_type *m_list;
extern monster_type *km_list;
extern u16b max_real_towns;
extern u16b max_towns;
extern town_type *town_info;
extern s16b alloc_kind_size;
extern alloc_entry *alloc_kind_table;
extern bool_ alloc_kind_table_valid;
extern s16b alloc_race_size;
extern alloc_entry *alloc_race_table;
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern char *keymap_act[KEYMAP_MODES][256];
extern player_type *p_ptr;
extern player_race *rp_ptr;
extern player_race_mod *rmp_ptr;
extern player_class *cp_ptr;
extern player_spec *spp_ptr;
extern char player_name[32];
extern char player_base[32];
extern ability_type *ab_info;
extern skill_type *s_info;
extern skill_descriptor *s_descriptors;
extern feature_type *f_info;
extern object_kind *k_info;
extern artifact_type *a_info;
extern ego_item_type *e_info;
extern monster_race *r_info;
extern monster_ego *re_info;
extern dungeon_info_type *d_info;
extern player_class *class_info;
extern player_race *race_info;
extern player_race_mod *race_mod_info;
extern trap_type *t_info;
extern wilderness_type_info *wf_info;
extern int wildc2i[256];
extern store_info_type *st_info;
extern set_type *set_info;
extern cptr DEFAULT_FEAT_TEXT;
extern cptr DEFAULT_FEAT_TUNNEL;
extern cptr DEFAULT_FEAT_BLOCK;
extern char *ANGBAND_DIR;
extern char *ANGBAND_DIR_MODULES;
extern char *ANGBAND_DIR_CORE;
extern char *ANGBAND_DIR_DNGN;
extern bool_ (*get_mon_num_hook)(int r_idx);
extern bool_ (*get_mon_num2_hook)(int r_idx);
extern bool_ (*get_obj_num_hook)(int k_idx);
extern u16b max_ab_idx;
extern u16b max_s_idx;
extern u16b max_r_idx;
extern u16b max_re_idx;
extern u16b max_k_idx;
extern u16b max_f_idx;
extern u16b max_a_idx;
extern u16b max_e_idx;
extern u16b max_d_idx;
extern u16b max_o_idx;
extern u16b max_m_idx;
extern u16b max_t_idx;
extern u16b max_rp_idx;
extern u16b max_c_idx;
extern u16b max_rmp_idx;
extern u16b max_st_idx;
extern u16b max_wf_idx;
extern u16b max_set_idx;
extern int init_flags;
extern bool_ ambush_flag;
extern bool_ fate_flag;
extern s16b no_breeds;
extern bool_ carried_monster_hit;
extern random_artifact random_artifacts[MAX_RANDARTS];
extern s32b RANDART_WEAPON;
extern s32b RANDART_ARMOR;
extern s32b RANDART_JEWEL;
extern fate fates[MAX_FATES];
extern byte dungeon_type;
extern s16b *max_dlv;
extern s16b doppleganger;
extern bool_ generate_encounter;
extern bool_ *m_allow_special;
extern bool_ *k_allow_special;
extern bool_ *a_allow_special;
extern s16b plots[MAX_PLOTS];
extern random_quest random_quests[MAX_RANDOM_QUEST];
extern bool_ *special_lvl[MAX_DUNGEON_DEPTH];
extern bool_ generate_special_feeling;
DECLARE_FLAG_ZERO_INTF(dungeon_flag_set, dungeon_flags);
extern birther previous_char;
extern int max_bg_idx;
extern s16b schools_count;
extern school_type schools[SCHOOLS_MAX];
extern int project_time;
extern s32b project_time_effect;
extern effect_type effects[MAX_EFFECTS];
extern skill_modifiers *gen_skill;
extern bool_ automatizer_enabled;
extern s16b last_teleportation_y;
extern s16b last_teleportation_x;
extern cptr game_module;
extern s32b game_module_idx;
extern s32b VERSION_MAJOR;
extern s32b VERSION_MINOR;
extern s32b VERSION_PATCH;
extern s32b DUNGEON_BASE;
extern s32b DUNGEON_DEATH;
extern s32b DUNGEON_ASTRAL;
extern s32b DUNGEON_ASTRAL_WILD_X;
extern s32b DUNGEON_ASTRAL_WILD_Y;
extern deity_type deity_info[MAX_GODS];
extern timer_type *gl_timers;
extern const char *get_version_string();
extern hist_type *bg;
extern bool_ arg_wizard;
extern bool_ arg_force_original;
extern bool_ arg_force_roguelike;
extern struct options *options;
