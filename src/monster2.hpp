#pragma once

#include "angband.h"

extern s32b monster_exp(s16b level);
extern void monster_set_level(int m_idx, int level);
extern s32b modify_aux(s32b a, s32b b, char mod);
extern void monster_msg_simple(cptr s);
extern bool_ mego_ok(monster_race const *r_ptr, int ego);
extern void monster_check_experience(int m_idx, bool_ silent);
extern void monster_gain_exp(int m_idx, u32b exp, bool_ silent);
extern monster_race* race_info_idx(int r_idx, int ego);
extern void delete_monster_idx(int i);
extern void delete_monster(int y, int x);
extern void compact_monsters(int size);
extern void wipe_m_list(void);
extern s16b m_pop(void);
extern errr get_mon_num_prep(void);
extern s16b get_mon_num(int level);
extern void monster_desc(char *desc, monster_type *m_ptr, int mode);
extern void monster_race_desc(char *desc, int r_idx, int ego);
extern void lore_do_probe(int m_idx);
extern void lore_treasure(int m_idx, int num_item, int num_gold);
extern void update_mon(int m_idx, bool_ full);
extern void update_monsters(bool_ full);
extern void monster_carry(monster_type *m_ptr, int m_idx, object_type *q_ptr);
extern bool_ bypass_r_ptr_max_num ;
extern bool_ place_monster_aux(int y, int x, int r_idx, bool_ slp, bool_ grp, int status);
extern bool_ place_monster(int y, int x, bool_ slp, bool_ grp);
extern bool_ alloc_horde(int y, int x);
extern bool_ alloc_monster(int dis, bool_ slp);
extern int summon_specific_level;
extern bool_ summon_specific(int y1, int x1, int lev, int type);
extern void monster_swap(int y1, int x1, int y2, int x2);
extern bool_ multiply_monster(int m_idx, bool_ charm, bool_ clone);
extern bool_ hack_message_pain_may_silent;
extern void message_pain(int m_idx, int dam);
extern void update_smart_learn(int m_idx, int what);
extern bool_ summon_specific_friendly(int y1, int x1, int lev, int type, bool_ Group_ok);
extern bool_ place_monster_one_no_drop;
extern s16b place_monster_one(int y, int x, int r_idx, int ego, bool_ slp, int status);
extern s16b player_place(int y, int x);
extern void monster_drop_carried_objects(monster_type *m_ptr);
extern bool_ monster_dungeon(int r_idx);
extern bool_ monster_quest(int r_idx);
extern void set_mon_num_hook(void);
extern void set_mon_num2_hook(int y, int x);
extern bool_ monster_can_cross_terrain(byte feat, monster_race *r_ptr);
