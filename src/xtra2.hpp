#pragma once

#include "h-basic.hpp"
#include "monster_race_fwd.hpp"
#include "object_type_fwd.hpp"
#include "player_race_mod_fwd.hpp"

#include <memory>

void do_rebirth();
void switch_subrace(std::size_t racem, bool copy_old);
void drop_from_wild();
bool set_roots(int v, s16b ac, s16b dam);
bool set_project(int v, s16b gf, s16b dam, s16b rad, s16b flag);
bool set_parasite(int v, int r);
bool set_disrupt_shield(int v);
bool set_prob_travel(int v);
bool set_absorb_soul(int v);
bool set_tim_breath(int v, bool magical);
bool set_tim_precognition(int v);
bool set_tim_deadly(int v);
bool set_tim_reflect(int v);
bool set_tim_thunder(int v, int p1, int p2);
bool set_strike(int v);
bool set_tim_regen(int v, int p);
bool set_tim_ffall(int v);
bool set_tim_fly(int v);
bool set_poison(int v);
bool set_holy(int v);
void set_grace(s32b v);
bool set_mimic(int v, int p, int level);
bool set_no_breeders(int v);
bool set_invis(int v,int p);
bool set_lite(int v);
bool set_blind(int v);
bool set_confused(int v);
bool set_poisoned(int v);
bool set_afraid(int v);
bool set_paralyzed(int v);
void dec_paralyzed();
bool set_image(int v);
bool set_fast(int v, int p);
bool set_light_speed(int v);
bool set_slow(int v);
bool set_shield(int v, int p, s16b o, s16b d1, s16b d2);
bool set_blessed(int v);
bool set_hero(int v);
bool set_shero(int v);
bool set_protevil(int v);
bool set_invuln(int v);
bool set_tim_invis(int v);
bool set_tim_infra(int v);
bool set_mental_barrier(int v);
bool set_oppose_acid(int v);
bool set_oppose_elec(int v);
bool set_oppose_fire(int v);
bool set_oppose_cold(int v);
bool set_oppose_pois(int v);
bool set_oppose_cc(int v);
bool set_stun(int v);
bool set_cut(int v);
bool set_food(int v);
void check_experience();
void check_experience_obj(object_type *o_ptr);
void gain_exp(s32b amount);
void lose_exp(s32b amount);
int get_coin_type(std::shared_ptr<monster_race const> r_ptr);
void monster_death(int m_idx);
bool mon_take_hit(int m_idx, int dam, bool *fear, const char *note);
bool change_panel(int dy, int dx);
void verify_panel();
bool target_okay();
bool target_set(int mode);
bool get_aim_dir(int *dp);
bool get_rep_dir(int *dp);
bool set_shadow(int v);
bool set_tim_esp(int v);
bool tgp_pt(int *x, int * y);
bool tgt_pt (int *x, int *y);
void do_poly_self();
bool make_wish();
void create_between_gate(int dist, int y, int x);
void resize_map();
void resize_window();
