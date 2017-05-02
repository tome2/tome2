#pragma once

#include "h-basic.h"
#include "monster_race_fwd.hpp"
#include "object_type_fwd.hpp"
#include "player_race_mod_fwd.hpp"

#include <memory>

void do_rebirth();
void switch_subrace(std::size_t racem, bool_ copy_old);
void drop_from_wild();
bool_ set_roots(int v, s16b ac, s16b dam);
bool_ set_project(int v, s16b gf, s16b dam, s16b rad, s16b flag);
bool_ set_parasite(int v, int r);
bool_ set_disrupt_shield(int v);
bool_ set_prob_travel(int v);
bool_ set_absorb_soul(int v);
bool_ set_tim_breath(int v, bool_ magical);
bool_ set_tim_precognition(int v);
bool_ set_tim_deadly(int v);
bool_ set_tim_reflect(int v);
bool_ set_tim_thunder(int v, int p1, int p2);
bool_ set_strike(int v);
bool_ set_tim_regen(int v, int p);
bool_ set_tim_ffall(int v);
bool_ set_tim_fly(int v);
bool_ set_poison(int v);
bool_ set_holy(int v);
void set_grace(s32b v);
bool_ set_mimic(int v, int p, int level);
bool_ set_no_breeders(int v);
bool_ set_invis(int v,int p);
bool_ set_lite(int v);
bool_ set_blind(int v);
bool_ set_confused(int v);
bool_ set_poisoned(int v);
bool_ set_afraid(int v);
bool_ set_paralyzed(int v);
void dec_paralyzed();
bool_ set_image(int v);
bool_ set_fast(int v, int p);
bool_ set_light_speed(int v);
bool_ set_slow(int v);
bool_ set_shield(int v, int p, s16b o, s16b d1, s16b d2);
bool_ set_blessed(int v);
bool_ set_hero(int v);
bool_ set_shero(int v);
bool_ set_protevil(int v);
bool_ set_invuln(int v);
bool_ set_tim_invis(int v);
bool_ set_tim_infra(int v);
bool_ set_mental_barrier(int v);
bool_ set_oppose_acid(int v);
bool_ set_oppose_elec(int v);
bool_ set_oppose_fire(int v);
bool_ set_oppose_cold(int v);
bool_ set_oppose_pois(int v);
bool_ set_oppose_cc(int v);
bool_ set_stun(int v);
bool_ set_cut(int v);
bool_ set_food(int v);
void check_experience();
void check_experience_obj(object_type *o_ptr);
void gain_exp(s32b amount);
void lose_exp(s32b amount);
int get_coin_type(std::shared_ptr<monster_race const> r_ptr);
void monster_death(int m_idx);
bool_ mon_take_hit(int m_idx, int dam, bool_ *fear, cptr note);
bool_ change_panel(int dy, int dx);
void verify_panel();
bool_ target_okay();
bool_ target_set(int mode);
bool_ get_aim_dir(int *dp);
bool_ get_rep_dir(int *dp);
bool_ set_shadow(int v);
bool_ set_tim_esp(int v);
bool_ tgp_pt(int *x, int * y);
bool_ tgt_pt (int *x, int *y);
void do_poly_self();
bool_ curse_weapon();
bool_ curse_armor();
void make_wish();
void create_between_gate(int dist, int y, int x);

extern "C" {
	void resize_map();
	void resize_window();
}
