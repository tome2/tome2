#pragma once

#include "alloc_entry_fwd.hpp"
#include "birther.hpp"
#include "cave_type_fwd.hpp"
#include "defines.hpp"
#include "deity_type.hpp"
#include "dungeon_flag_set.hpp"
#include "effect_type.hpp"
#include "fate.hpp"
#include "monster_race_fwd.hpp"
#include "monster_type_fwd.hpp"
#include "object_type_fwd.hpp"
#include "object_kind_fwd.hpp"
#include "options.hpp"
#include "player_class_fwd.hpp"
#include "player_defs.hpp"
#include "player_race_fwd.hpp"
#include "player_race_mod_fwd.hpp"
#include "player_spec_fwd.hpp"
#include "player_type_fwd.hpp"
#include "random_quest.hpp"
#include "school_type.hpp"
#include "skill_modifiers_fwd.hpp"
#include "skills_defs.hpp"
#include "timer_type_fwd.hpp"
#include "town_type_fwd.hpp"
#include "seed.hpp"
#include "z-term.hpp"

extern int max_macrotrigger;
extern char *macro_template;
extern char *macro_modifier_chr;
extern char *macro_modifier_name[MAX_MACRO_MOD];
extern char *macro_trigger_name[MAX_MACRO_TRIG];
extern char *macro_trigger_keycode[2][MAX_MACRO_TRIG];
extern bool character_dungeon;
extern bool character_loaded;
extern bool character_xtra;
seed_t &seed_flavor();
extern s16b command_cmd;
extern s16b command_arg;
extern s16b command_rep;
extern s16b command_dir;
extern s16b command_wrk;
extern s16b command_new;
extern s32b energy_use;
extern bool create_up_stair;
extern bool create_down_stair;
extern bool create_up_shaft;
extern bool create_down_shaft;
extern bool alive;
extern bool death;
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
extern bool wizard;
extern u16b total_winner;
extern u16b has_won;
extern u16b noscore;
extern bool inkey_base;
extern s16b coin_type;
extern bool shimmer_monsters;
extern bool shimmer_objects;
extern bool repair_monsters;
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
extern bool hack_mind;
extern bool is_autosave;
extern int artifact_bias;
extern FILE *text_out_file;
extern void (*text_out_hook)(byte a, const char *str);
extern int text_out_indent;
extern s16b feeling;
extern s16b rating;
extern bool good_item_flag;
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
extern bool *macro__cmd;
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
extern byte misc_to_attr[256];
extern char misc_to_char[256];
extern byte tval_to_attr[128];
extern char tval_to_char[128];
extern char *keymap_act[KEYMAP_MODES][256];
extern player_type *p_ptr;
extern player_race const *rp_ptr;
extern player_race_mod const *rmp_ptr;
extern player_class const *cp_ptr;
extern player_spec const *spp_ptr;
extern int wildc2i[256];
extern const char *DEFAULT_FEAT_TEXT;
extern const char *DEFAULT_FEAT_TUNNEL;
extern const char *DEFAULT_FEAT_BLOCK;
extern char *ANGBAND_DIR;
extern char *ANGBAND_DIR_MODULES;
extern bool (*get_monster_hook)(monster_race const *);
extern bool (*get_monster_aux_hook)(monster_race const *);
extern bool (*get_object_hook)(object_kind const *k_ptr);
extern u16b max_o_idx;
extern u16b max_m_idx;
extern int init_flags;
extern bool ambush_flag;
extern bool fate_flag;
extern s16b no_breeds;
extern bool carried_monster_hit;
extern s32b RANDART_WEAPON;
extern s32b RANDART_ARMOR;
extern s32b RANDART_JEWEL;
extern fate fates[MAX_FATES];
extern byte dungeon_type;
extern s16b *max_dlv;
extern s16b doppleganger;
extern bool generate_encounter;
extern bool *m_allow_special;
extern bool *a_allow_special;
extern s16b plots[MAX_PLOTS];
extern random_quest random_quests[MAX_RANDOM_QUEST];
extern s16b schools_count;
extern school_type schools[SCHOOLS_MAX];
extern int project_time;
extern s32b project_time_effect;
extern bool automatizer_enabled;
extern s16b last_teleportation_y;
extern s16b last_teleportation_x;
extern const char *game_module;
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
const char *get_version_string();
extern struct options *options;
extern const char *ANGBAND_SYS;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_DATA;
extern char *ANGBAND_DIR_EDIT;
extern char *ANGBAND_DIR_FILE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_NOTE;
extern char *ANGBAND_DIR_PREF;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_XTRA;
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][80];
extern byte angband_color_table[256][4];
extern bool character_generated;
extern bool character_icky;
extern bool inkey_flag;
extern bool msg_flag;
