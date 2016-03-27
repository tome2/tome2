#pragma once

#include "angband.h"
#include "ability_type_fwd.hpp"
#include "alloc_entry_fwd.hpp"
#include "artifact_type_fwd.hpp"
#include "birther.hpp"
#include "cave_type_fwd.hpp"
#include "deity_type.hpp"
#include "dungeon_info_type_fwd.hpp"
#include "effect_type.hpp"
#include "ego_item_type_fwd.hpp"
#include "fate.hpp"
#include "feature_type_fwd.hpp"
#include "hist_type_fwd.hpp"
#include "meta_class_type_fwd.hpp"
#include "monster_ego_fwd.hpp"
#include "monster_race_fwd.hpp"
#include "monster_type_fwd.hpp"
#include "object_kind_fwd.hpp"
#include "object_type_fwd.hpp"
#include "owner_type_fwd.hpp"
#include "player_class_fwd.hpp"
#include "player_defs.hpp"
#include "player_race_fwd.hpp"
#include "player_race_mod_fwd.hpp"
#include "player_spec_fwd.hpp"
#include "player_type_fwd.hpp"
#include "randart_gen_type_fwd.hpp"
#include "randart_part_type_fwd.hpp"
#include "random_artifact.hpp"
#include "random_quest.hpp"
#include "random_spell.hpp"
#include "rune_spell.hpp"
#include "school_type.hpp"
#include "set_type_fwd.hpp"
#include "skill_type_fwd.hpp"
#include "skills_defs.hpp"
#include "store_action_type_fwd.hpp"
#include "store_info_type_fwd.hpp"
#include "timer_type_fwd.hpp"
#include "town_type_fwd.hpp"
#include "tval_desc.hpp"
#include "vault_type_fwd.hpp"
#include "wilderness_map_fwd.hpp"
#include "wilderness_type_info_fwd.hpp"

extern int max_macrotrigger;
extern char *macro_template;
extern char *macro_modifier_chr;
extern char *macro_modifier_name[MAX_MACRO_MOD];
extern char *macro_trigger_name[MAX_MACRO_TRIG];
extern char *macro_trigger_keycode[2][MAX_MACRO_TRIG];
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte sf_major;
extern byte sf_minor;
extern byte sf_patch;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern bool_ character_dungeon;
extern bool_ character_loaded;
extern bool_ character_xtra;
extern u32b seed_flavor;
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
extern bool_ cheat_peek;
extern bool_ cheat_hear;
extern bool_ cheat_room;
extern bool_ cheat_xtra;
extern bool_ cheat_know;
extern bool_ cheat_live;
extern byte hitpoint_warn;
extern byte delay_factor;
extern s16b autosave_freq;
extern bool_ autosave_t;
extern bool_ autosave_l;
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
extern u32b option_flag[8];
extern u32b option_mask[8];
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
extern s16b player_hp[PY_MAX_LEVEL];
extern char player_name[32];
extern char player_base[32];
extern ability_type *ab_info;
extern skill_type *s_info;
extern vault_type *v_info;
extern feature_type *f_info;
extern object_kind *k_info;
extern artifact_type *a_info;
extern ego_item_type *e_info;
extern randart_part_type *ra_info;
extern randart_gen_type ra_gen[30];
extern monster_race *r_info;
extern monster_ego *re_info;
extern dungeon_info_type *d_info;
extern player_class *class_info;
extern meta_class_type *meta_class_info;
extern player_race *race_info;
extern player_race_mod *race_mod_info;
extern wilderness_type_info *wf_info;
extern int wildc2i[256];
extern store_info_type *st_info;
extern store_action_type *ba_info;
extern owner_type *ow_info;
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
extern u16b max_wild_x;
extern u16b max_wild_y;
extern wilderness_map **wild_map;
extern u16b old_max_s_idx;
extern u16b max_ab_idx;
extern u16b max_s_idx;
extern u16b max_r_idx;
extern u16b max_re_idx;
extern u16b max_k_idx;
extern u16b max_v_idx;
extern u16b max_f_idx;
extern u16b max_a_idx;
extern u16b max_e_idx;
extern u16b max_ra_idx;
extern u16b max_d_idx;
extern u16b max_o_idx;
extern u16b max_m_idx;
extern u16b max_t_idx;
extern u16b max_rp_idx;
extern u16b max_c_idx;
extern u16b max_mc_idx;
extern u16b max_rmp_idx;
extern u16b max_st_idx;
extern u16b max_ba_idx;
extern u16b max_ow_idx;
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
extern random_spell random_spells[MAX_SPELLS];
extern s16b spell_num;
extern rune_spell rune_spells[MAX_RUNES];
extern s16b rune_num;
extern fate fates[MAX_FATES];
extern byte dungeon_type;
extern s16b *max_dlv;
extern s16b doppleganger;
extern bool_ generate_encounter;
extern bool_ special_lvls;
extern bool_ *m_allow_special;
extern bool_ *k_allow_special;
extern bool_ *a_allow_special;
extern s16b plots[MAX_PLOTS];
extern random_quest random_quests[MAX_RANDOM_QUEST];
extern bool_ *special_lvl[MAX_DUNGEON_DEPTH];
extern bool_ generate_special_feeling;
extern u32b dungeon_flags1;
extern u32b dungeon_flags2;
extern birther previous_char;
extern int max_bg_idx;
extern s16b schools_count;
extern school_type schools[SCHOOLS_MAX];
extern int project_time;
extern s32b project_time_effect;
extern effect_type effects[MAX_EFFECTS];
extern char gen_skill_basem[MAX_SKILLS];
extern u32b gen_skill_base[MAX_SKILLS];
extern char gen_skill_modm[MAX_SKILLS];
extern s16b gen_skill_mod[MAX_SKILLS];
extern int max_bact;
extern bool_ automatizer_enabled;
extern s16b last_teleportation_y;
extern s16b last_teleportation_x;
extern cptr game_module;
extern s32b game_module_idx;
extern s32b VERSION_MAJOR;
extern s32b VERSION_MINOR;
extern s32b VERSION_PATCH;
extern s32b max_plev;
extern s32b DUNGEON_BASE;
extern s32b DUNGEON_DEATH;
extern s32b DUNGEON_ASTRAL;
extern s32b DUNGEON_ASTRAL_WILD_X;
extern s32b DUNGEON_ASTRAL_WILD_Y;
extern deity_type deity_info[MAX_GODS];
extern timer_type *gl_timers;
extern const char *get_version_string();
extern tval_desc tvals[];
extern hist_type *bg;
extern bool_ arg_wizard;
extern bool_ arg_force_original;
extern bool_ arg_force_roguelike;
