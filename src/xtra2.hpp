#pragma once

#include "h-basic.h"
#include "monster_race_fwd.hpp"
#include "object_type_fwd.hpp"
#include "player_race_mod_fwd.hpp"

extern void do_rebirth(void);
extern void set_subrace_title(player_race_mod *rmp_ptr, cptr name);
extern void set_subrace_description(player_race_mod *rmp_ptr, cptr desc);
extern void switch_subrace(int racem, bool_ copy_old);
extern void drop_from_wild(void);
extern bool_ set_roots(int v, s16b ac, s16b dam);
extern bool_ set_project(int v, s16b gf, s16b dam, s16b rad, s16b flag);
extern bool_ set_parasite(int v, int r);
extern bool_ set_disrupt_shield(int v);
extern bool_ set_prob_travel(int v);
extern bool_ set_absorb_soul(int v);
extern bool_ set_tim_breath(int v, bool_ magical);
extern bool_ set_tim_precognition(int v);
extern bool_ set_tim_deadly(int v);
extern bool_ set_tim_reflect(int v);
extern bool_ set_tim_thunder(int v, int p1, int p2);
extern bool_ set_strike(int v);
extern bool_ set_tim_regen(int v, int p);
extern bool_ set_tim_ffall(int v);
extern bool_ set_tim_fly(int v);
extern bool_ set_poison(int v);
extern bool_ set_holy(int v);
extern void set_grace(s32b v);
extern bool_ set_mimic(int v, int p, int level);
extern bool_ set_no_breeders(int v);
extern bool_ set_invis(int v,int p);
extern bool_ set_lite(int v);
extern bool_ set_blind(int v);
extern bool_ set_confused(int v);
extern bool_ set_poisoned(int v);
extern bool_ set_afraid(int v);
extern bool_ set_paralyzed(int v);
extern void dec_paralyzed();
extern bool_ set_image(int v);
extern bool_ set_fast(int v, int p);
extern bool_ set_light_speed(int v);
extern bool_ set_slow(int v);
extern bool_ set_shield(int v, int p, s16b o, s16b d1, s16b d2);
extern bool_ set_blessed(int v);
extern bool_ set_hero(int v);
extern bool_ set_shero(int v);
extern bool_ set_protevil(int v);
extern bool_ set_protgood(int v);
extern bool_ set_protundead(int v);
extern bool_ set_invuln(int v);
extern bool_ set_tim_invis(int v);
extern bool_ set_tim_infra(int v);
extern bool_ set_mental_barrier(int v);
extern bool_ set_oppose_acid(int v);
extern bool_ set_oppose_elec(int v);
extern bool_ set_oppose_fire(int v);
extern bool_ set_oppose_cold(int v);
extern bool_ set_oppose_pois(int v);
extern bool_ set_oppose_ld(int v);
extern bool_ set_oppose_cc(int v);
extern bool_ set_oppose_ss(int v);
extern bool_ set_oppose_nex(int v);
extern bool_ set_stun(int v);
extern bool_ set_cut(int v);
extern bool_ set_food(int v);
extern void check_experience(void);
extern void check_experience_obj(object_type *o_ptr);
extern void gain_exp(s32b amount);
extern void lose_exp(s32b amount);
extern int get_coin_type(monster_race *r_ptr);
extern void monster_death(int m_idx);
extern bool_ mon_take_hit(int m_idx, int dam, bool_ *fear, cptr note);
extern bool_ change_panel(int dy, int dx);
extern void verify_panel(void);
extern bool_ target_okay(void);
extern bool_ target_set(int mode);
extern bool_ get_aim_dir(int *dp);
extern bool_ get_rep_dir(int *dp);
extern bool_ set_shadow(int v);
extern bool_ set_tim_esp(int v);
extern bool_ tgp_pt(int *x, int * y);
extern bool_ tgt_pt (int *x, int *y);
extern void do_poly_self(void);
extern bool_ curse_weapon(void);
extern bool_ curse_armor(void);
extern void make_wish(void);
extern void create_between_gate(int dist, int y, int x);

extern "C" {
	extern void resize_map(void);
	extern void resize_window(void);
}
