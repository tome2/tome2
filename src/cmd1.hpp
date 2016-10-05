#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"
#include "object_type_fwd.hpp"

extern void attack_special(monster_type *m_ptr, s32b special, int dam);
extern bool_ test_hit_fire(int chance, int ac, int vis);
extern bool_ test_hit_norm(int chance, int ac, int vis);
extern s16b critical_shot(int weight, int plus, int dam, int skill);
extern s16b critical_norm(int weight, int plus, int dam, int weapon_tval, bool_ *done_crit);
extern s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s32b *special);
extern void search(void);
extern void carry(int pickup);
extern void py_attack(int y, int x, int max_blow);
extern bool_ player_can_enter(byte feature);
extern void move_player(int dir, int do_pickup);
extern void move_player_aux(int dir, int do_pickup, int run);
extern void run_step(int dir);
extern void do_cmd_pet(void);
extern void do_cmd_integrate_body();
extern bool_ do_cmd_leave_body(bool_ drop_body);
extern bool_ execute_inscription(byte i, byte y, byte x);
extern void do_cmd_engrave(void);
extern void do_spin(void);
