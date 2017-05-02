#pragma once

#include <string>

#include "h-basic.h"
#include "random_spell_fwd.hpp"

extern byte spell_color(int type);
extern s16b poly_r_idx(int r_idx);
extern void get_pos_player(int dis, int *ny, int *nx);
extern bool_ teleport_player_bypass;
extern void teleport_player_directed(int rad, int dir);
extern void teleport_away(int m_idx, int dis);
extern void teleport_player(int dis);
extern void teleport_player_to(int ny, int nx);
extern void teleport_monster_to(int m_idx, int ny, int nx);
extern void teleport_player_level(void);
extern void recall_player(int d, int f);
extern void take_hit(int damage, cptr kb_str);
extern void take_sanity_hit(int damage, cptr hit_from);
extern void acid_dam(int dam, cptr kb_str);
extern void elec_dam(int dam, cptr kb_str);
extern void fire_dam(int dam, cptr kb_str);
extern void cold_dam(int dam, cptr kb_str);
extern bool_ dec_stat(int stat, int amount, int mode);
extern bool_ res_stat(int stat, bool_ full);
extern bool_ apply_disenchant(int mode);
extern bool_ project_m(int who, int r, int y, int x, int dam, int typ);
extern bool_ project(int who, int rad, int y, int x, int dam, int typ, int flg);
extern bool_ potion_smash_effect(int who, int y, int x, int o_sval);
extern void do_poly_self(void);
extern void corrupt_player(void);
extern std::string name_spell(random_spell const *);
extern void generate_spell(int plev);
extern bool_ unsafe;
extern s16b do_poly_monster(int y, int x);
