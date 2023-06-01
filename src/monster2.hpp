#pragma once

#include "h-basic.hpp"
#include "monster_race_fwd.hpp"
#include "monster_type_fwd.hpp"
#include "object_type_fwd.hpp"
#include <memory>

s32b monster_exp(s16b level);
void monster_set_level(int m_idx, int level);
s32b modify_aux(s32b a, s32b b, char mod);
void monster_msg_simple(const char *s);
bool mego_ok(monster_race const *r_ptr, int ego);
void monster_check_experience(int m_idx, bool silent);
void monster_gain_exp(int m_idx, u32b exp);
std::shared_ptr<monster_race> race_info_idx(int r_idx, int ego);
void delete_monster_idx(int i);
void delete_monster(int y, int x);
void compact_monsters(int size);
void wipe_m_list();
s16b m_pop();
void get_mon_num_prep();
s16b get_mon_num(int level);
void monster_desc(char *desc, const monster_type *m_ptr, int mode);
void monster_race_desc(char *desc, int r_idx, int ego);
void update_mon(int m_idx, bool full);
void update_monsters(bool full);
void monster_carry(monster_type *m_ptr, int m_idx, object_type *q_ptr);
extern bool bypass_r_ptr_max_num ;
bool place_monster_aux(int y, int x, int r_idx, bool slp, bool grp, int status);
bool place_monster(int y, int x, bool slp, bool grp);
bool alloc_horde(int y, int x);
bool alloc_monster(int dis, bool slp);
extern int summon_specific_level;
bool summon_specific(int y1, int x1, int lev, int type);
void monster_swap(int y1, int x1, int y2, int x2);
bool multiply_monster(int m_idx, bool charm, bool clone);
extern bool hack_message_pain_may_silent;
void message_pain(int m_idx, int dam);
void update_smart_learn(int m_idx, int what);
bool summon_specific_friendly(int y1, int x1, int lev, int type, bool Group_ok);
extern bool place_monster_one_no_drop;
s16b place_monster_one(int y, int x, int r_idx, int ego, bool slp, int status);
s16b player_place(int y, int x);
void monster_drop_carried_objects(monster_type *m_ptr);
bool monster_dungeon(monster_race const *r_ptr);
bool monster_quest(monster_race const *r_ptr);
void reset_get_monster_hook();
void set_monster_aux_hook(int y, int x);
bool monster_can_cross_terrain(byte feat, std::shared_ptr<monster_race> r_ptr);
