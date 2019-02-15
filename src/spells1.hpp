#pragma once

#include <string>

#include "h-basic.hpp"
#include "random_spell_fwd.hpp"

byte spell_color(int type);
s16b poly_r_idx(int r_idx);
void get_pos_player(int dis, int *ny, int *nx);
extern bool_ teleport_player_bypass;
void teleport_player_directed(int rad, int dir);
void teleport_away(int m_idx, int dis);
void teleport_player(int dis);
void teleport_player_to(int ny, int nx);
void teleport_monster_to(int m_idx, int ny, int nx);
void teleport_player_level();
void recall_player(int d, int f);
void take_hit(int damage, const char *kb_str);
void take_hit(int damage, std::string const &kb_str);
void take_sanity_hit(int damage, const char *hit_from);
void acid_dam(int dam, const char *kb_str);
void elec_dam(int dam, const char *kb_str);
void fire_dam(int dam, const char *kb_str);
void cold_dam(int dam, const char *kb_str);
bool_ dec_stat(int stat, int amount, int mode);
bool_ res_stat(int stat, bool_ full);
bool_ apply_disenchant(int mode);
bool_ project_m(int who, int r, int y, int x, int dam, int typ);
bool_ project(int who, int rad, int y, int x, int dam, int typ, int flg);
bool_ potion_smash_effect(int who, int y, int x, int o_sval);
void do_poly_self();
void corrupt_player();
std::string name_spell(random_spell const *);
void generate_spell(int plev);
s16b do_poly_monster(int y, int x);
