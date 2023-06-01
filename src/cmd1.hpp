#pragma once

#include "h-basic.hpp"
#include "monster_type_fwd.hpp"
#include "object_type_fwd.hpp"

void attack_special(monster_type *m_ptr, s32b special, int dam);
bool test_hit_fire(int chance, int ac, int vis);
bool test_hit_norm(int chance, int ac, int vis);
s16b critical_shot(int weight, int plus, int dam, int skill);
s16b critical_norm(int weight, int plus, int dam, int weapon_tval, bool *done_crit);
s16b tot_dam_aux(object_type *o_ptr, int tdam, monster_type *m_ptr, s32b *special);
void search();
void carry(int pickup);
void py_attack(int y, int x, int max_blow);
bool player_can_enter(byte feature);
void move_player(int dir, int do_pickup);
void move_player_aux(int dir, int do_pickup, int run);
void run_step(int dir);
void do_cmd_pet();
void do_cmd_integrate_body();
void do_cmd_leave_body(bool drop_body);
bool execute_inscription(byte i, byte y, byte x);
void do_cmd_engrave();
void do_spin();
