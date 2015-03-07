/* File: externs.h */

/* Purpose: extern declarations (variables and functions) */

/*
 * Note that some files have their own header files
 * (z-util.h, z-form.h, term.h, random.h)
 */

/*
 * Options for inc_stack_size_ex
 */
typedef enum { OPTIMIZE, NO_OPTIMIZE } optimize_flag;
typedef enum { DESCRIBE, NO_DESCRIBE } describe_flag;


/*
 * Automatically generated "variable" declarations
 */

extern int max_macrotrigger;
extern char *macro_template;
extern char *macro_modifier_chr;
extern char *macro_modifier_name[MAX_MACRO_MOD];
extern char *macro_trigger_name[MAX_MACRO_TRIG];
extern char *macro_trigger_keycode[2][MAX_MACRO_TRIG];

/* tables.c */
extern s16b ddd[9];
extern s16b ddx[10];
extern s16b ddy[10];
extern s16b ddx_ddd[9];
extern s16b ddy_ddd[9];
extern char hexsym[16];
extern byte adj_val_min[];
extern byte adj_val_max[];
extern byte adj_mag_study[];
extern byte adj_mag_mana[];
extern byte adj_mag_fail[];
extern byte adj_mag_stat[];
extern byte adj_chr_gold[];
extern byte adj_int_dev[];
extern byte adj_wis_sav[];
extern byte adj_dex_dis[];
extern byte adj_int_dis[];
extern byte adj_dex_ta[];
extern byte adj_str_td[];
extern byte adj_dex_th[];
extern byte adj_str_th[];
extern byte adj_str_wgt[];
extern byte adj_str_hold[];
extern byte adj_str_dig[];
extern byte adj_str_blow[];
extern byte adj_dex_blow[];
extern byte adj_dex_safe[];
extern byte adj_con_fix[];
extern byte adj_con_mhp[];
extern byte blows_table[12][12];
extern byte extract_energy[300];
extern s32b player_exp[PY_MAX_LEVEL];
extern player_sex sex_info[MAX_SEXES];
extern cptr color_names[16];
extern cptr stat_names[6];
extern cptr stat_names_reduced[6];
extern cptr window_flag_desc[32];
extern option_type option_info[];
extern martial_arts bear_blows[MAX_BEAR];
extern martial_arts ma_blows[MAX_MA];
extern magic_power mindcraft_powers[MAX_MINDCRAFT_POWERS];
extern magic_power necro_powers[MAX_NECRO_POWERS];
extern magic_power mimic_powers[MAX_MIMIC_POWERS];
extern magic_power symbiotic_powers[MAX_SYMBIOTIC_POWERS];
extern cptr deity_rarity[2];
extern cptr deity_niceness[10];
extern cptr deity_standing[11];
extern move_info_type move_info[9];
extern tactic_info_type tactic_info[9];
extern activation activation_info[MAX_T_ACT];
extern inscription_info_type inscription_info[MAX_INSCRIPTIONS];
extern cptr sense_desc[];
extern flags_group flags_groups[MAX_FLAG_GROUP];
extern power_type powers_type[POWER_MAX];
extern cptr artifact_names_list;
extern monster_power monster_powers[96];
extern tval_desc tval_descs[];
extern between_exit between_exits[MAX_BETWEEN_EXITS];
extern int month_day[9];
extern cptr month_name[9];
extern cli_comm *cli_info;
extern int cli_total;
extern quest_type quest[MAX_Q_IDX];
extern int max_body_part[BODY_MAX];
extern gf_name_type gf_names[];
extern module_type modules[MAX_MODULES];


/* variable.c */
extern byte version_major;
extern byte version_minor;
extern byte version_patch;
extern byte sf_major;
extern byte sf_minor;
extern byte sf_patch;
extern u32b sf_when;
extern u16b sf_lives;
extern u16b sf_saves;
extern bool_ arg_wizard;
extern bool_ arg_force_original;
extern bool_ arg_force_roguelike;
extern bool_ character_generated;
extern bool_ character_dungeon;
extern bool_ character_loaded;
extern bool_ character_icky;
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
extern bool_ msg_flag;
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
extern bool_ inkey_flag;
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
extern s16b hack_m_idx_ii;
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
extern bool_ rogue_like_commands;
extern bool_ quick_messages;
extern bool_ carry_query_flag;
extern bool_ always_pickup;
extern bool_ prompt_pickup_heavy;
extern bool_ always_repeat;
extern bool_ use_old_target;
extern bool_ hilite_player;
extern bool_ ring_bell;
extern bool_ find_ignore_stairs;
extern bool_ find_ignore_doors;
extern bool_ find_cut;
extern bool_ find_examine;
extern bool_ disturb_near;
extern bool_ disturb_move;
extern bool_ disturb_panel;
extern bool_ disturb_detect;
extern bool_ disturb_state;
extern bool_ disturb_minor;
extern bool_ disturb_other;
extern bool_ avoid_abort;
extern bool_ avoid_shimmer;
extern bool_ avoid_other;
extern bool_ flush_disturb;
extern bool_ flush_failure;
extern bool_ flush_command;
extern bool_ fresh_before;
extern bool_ fresh_after;
extern bool_ fresh_message;
extern bool_ alert_hitpoint;
extern bool_ alert_failure;
extern bool_ view_yellow_lite;
extern bool_ view_bright_lite;
extern bool_ view_granite_lite;
extern bool_ view_special_lite;
extern bool_ wear_confirm;
extern bool_ confirm_stairs;
extern bool_ disturb_pets;
extern bool_ view_perma_grids;
extern bool_ view_torch_grids;
extern bool_ flow_by_sound;
extern bool_ view_reduce_lite;
extern bool_ auto_scum;
extern bool_ expand_look;
extern bool_ expand_list;
extern bool_ dungeon_align;
extern bool_ dungeon_stair;
extern bool_ smart_learn;
extern bool_ testing_stack;
extern bool_ testing_carry;
extern bool_ cheat_peek;
extern bool_ cheat_hear;
extern bool_ cheat_room;
extern bool_ cheat_xtra;
extern bool_ cheat_know;
extern bool_ cheat_live;
extern bool_ last_words;              /* Zangband options */
extern bool_ small_levels;
extern bool_ empty_levels;
extern bool_ always_small_level;
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
extern char player_name[32];
extern char player_base[32];
extern char died_from[80];
extern char history[4][60];
extern char savefile[1024];
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
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][80];
extern byte angband_color_table[256][4];
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
extern player_sex *sp_ptr;
extern player_race *rp_ptr;
extern player_race_mod *rmp_ptr;
extern player_class *cp_ptr;
extern player_spec *spp_ptr;
extern u32b alchemist_known_egos[32];
extern alchemist_recipe *alchemist_recipes;
extern u32b alchemist_known_artifacts[6];
extern u32b alchemist_gained;
extern s16b player_hp[PY_MAX_LEVEL];

extern artifact_select_flag *a_select_flags;

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
extern trap_type *t_info;
extern wilderness_type_info *wf_info;
extern int wildc2i[256];
extern store_info_type *st_info;
extern store_action_type *ba_info;
extern owner_type *ow_info;
extern set_type *set_info;

extern cptr DEFAULT_FEAT_TEXT;
extern cptr DEFAULT_FEAT_TUNNEL;
extern cptr DEFAULT_FEAT_BLOCK;

extern cptr ANGBAND_SYS;
extern cptr ANGBAND_KEYBOARD;
extern cptr ANGBAND_GRAF;
extern char *ANGBAND_DIR;
extern char *ANGBAND_DIR_MODULES;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_CORE;
extern char *ANGBAND_DIR_DNGN;
extern char *ANGBAND_DIR_DATA;
extern char *ANGBAND_DIR_EDIT;
extern char *ANGBAND_DIR_FILE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_NOTE;
extern char *ANGBAND_DIR_PREF;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_XTRA;
extern bool_ item_tester_full;
extern byte item_tester_tval;
extern bool_ (*item_tester_hook)(object_type *o_ptr);
extern bool_ (*get_mon_num_hook)(int r_idx);
extern bool_ (*get_mon_num2_hook)(int r_idx);
extern bool_ (*get_obj_num_hook)(int k_idx);
extern u16b max_wild_x;
extern u16b max_wild_y;
extern wilderness_map **wild_map;
extern u16b old_max_s_idx;
extern u16b max_ab_idx;
extern u16b max_s_idx;
extern u16b max_al_idx;
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
extern s16b max_set_idx;
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
extern bool_ autoroll;
extern bool_ point_based;
extern bool_ preserve, special_lvls, ironman_rooms;
extern bool_ inventory_no_move;
extern bool_ *m_allow_special;
extern bool_ *k_allow_special;
extern bool_ *a_allow_special;
extern bool_ rand_birth;
extern bool_ joke_monsters;
extern bool_ center_player;
extern s16b plots[MAX_PLOTS];
extern random_quest random_quests[MAX_RANDOM_QUEST];
extern bool_ fate_option;
extern bool_ *special_lvl[MAX_DUNGEON_DEPTH];
extern bool_ generate_special_feeling;
extern bool_ auto_more;
extern u32b dungeon_flags1;
extern u32b dungeon_flags2;
extern birther previous_char;
extern hist_type *bg;
extern int max_bg_idx;
extern s32b extra_savefile_parts;
extern bool_ player_char_health;
extern s16b schools_count;
extern school_type schools[SCHOOLS_MAX];
extern int project_time;
extern s32b project_time_effect;
extern effect_type effects[MAX_EFFECTS];
extern char gen_skill_basem[MAX_SKILLS];
extern u32b gen_skill_base[MAX_SKILLS];
extern char gen_skill_modm[MAX_SKILLS];
extern s16b gen_skill_mod[MAX_SKILLS];
extern bool_ linear_stats;
extern int max_bact;
extern bool_ option_ingame_help;
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

/* birth.c */
extern bool_ no_begin_screen;

/* dungeon.c */
extern void play_game(bool_ new_game);
extern void sense_inventory();

/* files.c */
extern void html_screenshot(cptr name);
extern void help_file_screenshot(cptr name);
extern void player_flags(u32b* f1, u32b* f2, u32b* f3, u32b* f4, u32b* f5, u32b* esp);
extern void wipe_saved(void);
extern s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2);
extern void display_player(int mode);
extern cptr describe_player_location(void);
extern errr file_character(cptr name, bool_ full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern bool_ txt_to_html(cptr head, cptr food, cptr base, cptr ext, bool_ force, bool_ recur);
extern bool_ show_file(cptr name, cptr what, int line, int mode);
extern void do_cmd_help(void);
extern void process_player_base(void);
extern void process_player_name(bool_ sf);
extern void get_name(void);
extern void do_cmd_suicide(void);
extern void do_cmd_save_game(void);
extern void autosave_checkpoint();
extern void predict_score_gui(bool_ *initialized, bool_ *game_in_progress);
extern void close_game(void);
extern errr get_rnd_line(const char * file_name, char * output);
extern char *get_line(const char* fname, cptr fdir, char *linbuf, int line);
extern void race_legends(void);
extern void show_highclass(int building);
extern errr get_xtra_line(const char * file_name, monster_type *m_ptr, char * output);

/* gen_maze.c */
extern bool_ level_generate_maze();

/* gen_life.c */
extern bool_ level_generate_life();
extern void evolve_level(bool_ noise);

/* generate.c */
extern bool_ new_player_spot(int branch);
extern void add_level_generator(cptr name, bool_ (*generator)(), bool_ stairs, bool_ monsters, bool_ objects, bool_ miscs);
extern bool_ level_generate_dungeon();
extern bool_ generate_fracave(int y0, int x0,int xsize,int ysize,int cutoff,bool_ light,bool_ room);
extern void generate_hmap(int y0, int x0,int xsiz,int ysiz,int grd,int roug,int cutoff);
extern bool_ room_alloc(int x,int y,bool_ crowded,int by0,int bx0,int *xx,int *yy);
extern void generate_grid_mana(void);
extern byte calc_dungeon_type(void);
extern void generate_cave(void);
extern void build_rectangle(int y1, int x1, int y2, int x2, int feat, int info);


/* wild.c */
extern int generate_area(int y, int x, bool_ border, bool_ corner, bool_ refresh);
extern void wilderness_gen(int refresh);
extern void wilderness_gen_small(void);
extern void reveal_wilderness_around_player(int y, int x, int h, int w);
extern void town_gen(int t_idx);


/* init1.c */
extern int color_char_to_attr(char c);
extern byte conv_color[16];
extern errr init_player_info_txt(FILE *fp, char *buf);
extern errr init_ab_info_txt(FILE *fp, char *buf);
extern errr init_s_info_txt(FILE *fp, char *buf);
extern errr init_set_info_txt(FILE *fp, char *buf);
extern errr init_v_info_txt(FILE *fp, char *buf);
extern errr init_f_info_txt(FILE *fp, char *buf);
extern errr init_k_info_txt(FILE *fp, char *buf);
extern errr init_a_info_txt(FILE *fp, char *buf);
extern errr init_al_info_txt(FILE *fp, char *buf);
extern errr init_ra_info_txt(FILE *fp, char *buf);
extern errr init_e_info_txt(FILE *fp, char *buf);
extern errr init_r_info_txt(FILE *fp, char *buf);
extern errr init_re_info_txt(FILE *fp, char *buf);
extern errr grab_one_dungeon_flag(u32b *flags1, u32b *flags2, cptr what);
extern errr init_d_info_txt(FILE *fp, char *buf);
extern errr init_t_info_txt(FILE *fp, char *buf);
extern errr init_ba_info_txt(FILE *fp, char *buf);
extern errr init_st_info_txt(FILE *fp, char *buf);
extern errr init_ow_info_txt(FILE *fp, char *buf);
extern errr init_wf_info_txt(FILE *fp, char *buf);
extern errr process_dungeon_file(cptr name, int *yval, int *xval, int ymax, int xmax, bool_ init, bool_ full);

/* init2.c */
extern void init_corruptions();
extern void create_stores_stock(int t);
extern errr init_v_info(void);
extern void init_file_paths(char *path);
extern void init_file_paths_with_env();
extern void init_angband(void);
extern errr init_buildings(void);
extern s16b error_idx;
extern s16b error_line;

/* joke.c */
extern bool_ gen_joke_monsters(void *data, void *in, void *out);

/* loadsave.c */
extern bool_ file_exist(cptr buf);
extern bool_ load_dungeon(char *ext);
extern void save_dungeon(void);
extern bool_ save_player(void);
extern bool_ load_player(void);

/* melee1.c */
/* melee2.c */
extern int monst_spell_monst_spell;
extern bool_ mon_take_hit_mon(int s_idx, int m_idx, int dam, bool_ *fear, cptr note);
extern void mon_handle_fear(monster_type *m_ptr, int dam, bool_ *fear);
extern int check_hit2(int power, int level, int ac);
extern int get_attack_power(int effect);
extern bool_ carried_make_attack_normal(int r_idx);
extern bool_ make_attack_normal(int m_idx, byte divis);
extern void process_monsters(void);
extern void curse_equipment(int chance, int heavy_chance);
extern void curse_equipment_dg(int chance, int heavy_chance);

/* object1.c */
/* object2.c */
extern byte get_item_letter_color(object_type *o_ptr);
extern void describe_device(object_type *o_ptr);
extern void inc_stack_size(int item, int delta);
extern void inc_stack_size_ex(int item, int delta, optimize_flag opt, describe_flag desc);
extern void object_pickup(int this_o_idx);
extern int get_slot(int slot);
extern bool_ apply_flags_set(s16b a_idx, s16b set_idx, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern bool_ apply_set(s16b a_idx, s16b set_idx);
extern bool_ takeoff_set(s16b a_idx, s16b set_idx);
extern bool_ wield_set(s16b a_idx, s16b set_idx, bool_ silent);
extern object_type *get_object(int item);
extern s32b calc_total_weight(void);
extern void add_random_ego_flag(object_type *o_ptr, int fego, bool_ *limit_blows);
extern bool_ info_spell;
extern char spell_txt[50];
extern void init_match_theme(obj_theme theme);
extern bool_ kind_is_artifactable(int k_idx);
extern bool_ kind_is_legal(int k_idx);
extern bool_ verify(cptr prompt, int item);
extern void flavor_init(void);
extern void reset_visuals(void);
extern int object_power(object_type *o_ptr);
extern bool_ object_flags_no_set;
extern void object_flags(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_flags_known(object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);
extern void object_desc(char *buf, object_type *o_ptr, int pref, int mode);
extern void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode);
extern bool_ object_out_desc(object_type *o_ptr, FILE *fff, bool_ trim_down, bool_ wait_for_it);
extern char index_to_label(int i);
extern s16b wield_slot_ideal(object_type *o_ptr, bool_ ideal);
extern s16b wield_slot(object_type *o_ptr);
extern cptr describe_use(int i);
extern void inven_item_charges(int item);
extern void inven_item_describe(int item);
extern void inven_item_increase(int item, int num);
extern bool_ inven_item_optimize(int item);
extern void floor_item_charges(int item);
extern void floor_item_describe(int item);
extern void floor_item_increase(int item, int num);
extern void floor_item_optimize(int item);
extern bool_ inven_carry_okay(object_type *o_ptr);
extern s16b inven_carry(object_type *o_ptr, bool_ final);
extern s16b inven_takeoff(int item, int amt, bool_ force_drop);
extern void inven_drop(int item, int amt, int dy, int dx, bool_ silent);
extern bool_ item_tester_okay(object_type *o_ptr);
extern void display_inven(void);
extern void display_equip(void);
extern void show_inven();
extern void show_equip();
extern void toggle_inven_equip(void);
extern bool_ (*get_item_extra_hook)(int *cp);
extern bool_ get_item(int *cp, cptr pmt, cptr str, int mode);
extern void excise_object_idx(int o_idx);
extern void delete_object_idx(int o_idx);
extern void delete_object(int y, int x);
extern void compact_objects(int size);
extern void wipe_o_list(void);
extern s16b o_pop(void);
extern errr get_obj_num_prep(void);
extern s16b get_obj_num(int level);
extern void object_known(object_type *o_ptr);
extern void object_aware(object_type *o_ptr);
extern void object_tried(object_type *o_ptr);
extern s32b object_value(object_type *o_ptr);
extern s32b object_value_real(object_type *o_ptr);
extern bool_ object_similar(object_type *o_ptr, object_type *j_ptr);
extern void object_absorb(object_type *o_ptr, object_type *j_ptr);
extern s16b lookup_kind(int tval, int sval);
extern void object_wipe(object_type *o_ptr);
extern void object_prep(object_type *o_ptr, int k_idx);
extern void object_copy(object_type *o_ptr, object_type *j_ptr);
extern int hack_apply_magic_power;
extern void apply_magic(object_type *o_ptr, int lev, bool_ okay, bool_ good, bool_ great);
extern bool_ make_object(object_type *j_ptr, bool_ good, bool_ great, obj_theme theme);
extern void place_object(int y, int x, bool_ good, bool_ great, int where);
extern bool_ make_gold(object_type *j_ptr);
extern void place_gold(int y, int x);
extern void process_objects(void);
extern s16b drop_near(object_type *o_ptr, int chance, int y, int x);
extern void acquirement(int y1, int x1, int num, bool_ great, bool_ known);
extern void pick_trap(int y, int x);
extern cptr item_activation(object_type *o_ptr,byte num);
extern void combine_pack(void);
extern void reorder_pack(void);
extern void random_artifact_resistance (object_type * o_ptr);
extern s16b floor_carry(int y, int x, object_type *j_ptr);
extern void pack_decay(int item);
extern void floor_decay(int item);
extern void py_pickup_floor(int pickup);
extern s16b m_bonus(int max, int level);
extern void object_gain_level(object_type *o_ptr);
extern s32b flag_cost(object_type * o_ptr, int plusses);

/* main.c */
extern bool_ private_check_user_directory(cptr dirpath);

/* levels.c */
extern bool_ get_dungeon_generator(char *buf);
extern bool_ get_level_desc(char *buf);
extern void get_level_flags(void);
extern bool_ get_dungeon_name(char *buf);
extern bool_ get_dungeon_special(char *buf);
extern bool_ get_command(const char *file, char comm, char *param);
extern int get_branch(void);
extern int get_fbranch(void);
extern int get_flevel(void);
extern bool_ get_dungeon_save(char *buf);

/* wizard1.c */
extern void do_cmd_spoilers();

/* wizard2.c */
extern void do_cmd_wiz_cure_all(void);
extern void do_cmd_wiz_named_friendly(int r_idx, bool_ slp);
extern void do_cmd_debug();
extern tval_desc2 tvals[];

/*
 * Hack -- conditional (or "bizarre") externs
 */

#ifdef SET_UID
/* util.c */
extern void user_name(char *buf, int id);
#endif

/* script.c */
extern void init_lua_init(void);

/* modules.cc */
extern cptr force_module;

/* lua_bind.c */

extern s32b lua_get_level(struct spell_type *spell, s32b lvl, s32b max, s32b min, s32b bonus);
extern int get_mana(s32b s);
extern s32b get_power(s32b s);
extern s32b get_level(s32b s, s32b max, s32b min);
extern void get_level_school(struct spell_type *spell, s32b max, s32b min, s32b *level, bool_ *na);

extern s32b get_level_max_stick;
extern s32b get_level_use_stick;

extern void set_target(int y, int x);
extern void get_target(int dir, int *y, int *x);

extern void get_map_size(const char *name, int *ysize, int *xsize);
extern void load_map(const char *name, int *y, int *x);

extern int lua_get_new_bounty_monster(int lev);

extern char *lua_input_box(cptr title, int max);
extern char lua_msg_box(cptr title);

extern void increase_mana(int delta);

extern timer_type *TIMER_AGGRAVATE_EVIL;

void timer_aggravate_evil_enable();
void timer_aggravate_evil_callback();
