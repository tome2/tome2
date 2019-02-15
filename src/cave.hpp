#pragma once

#include "h-basic.h"
#include "cave_type_fwd.hpp"
#include "object_type_fwd.hpp"

#include <boost/optional.hpp>

int distance(int y1, int x1, int y2, int x2);
bool_ los(int y1, int x1, int y2, int x2);
bool_ cave_valid_bold(int y, int x);
bool_ no_lite();
void map_info_default(int y, int x, byte *ap, char *cp);
void move_cursor_relative(int row, int col);
void print_rel(char c, byte a, int y, int x);
void note_spot(int y, int x);
void lite_spot(int y, int x);
void prt_map();
void display_map(int *cy, int *cx);
void do_cmd_view_map();
errr vinfo_init();
void forget_view();
void update_view();
void forget_mon_lite();
void update_mon_lite();
void update_flow();
void map_area();
void wiz_lite();
void wiz_lite_extra();
void wiz_dark();
void cave_set_feat(int y, int x, int feat);
void place_floor(int y, int x);
void place_floor_convert_glass(int y, int x);
void place_filler(int y, int x);
void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
bool_ projectable(int y1, int x1, int y2, int x2);
void scatter(int *yp, int *xp, int y, int x, int d);
void health_track(int m_idx);
void monster_race_track(int r_idx, int ego);
void object_track(object_type *o_ptr);
void disturb();
void disturb_on_state();
void disturb_on_other();
int is_quest(int level);
boost::optional<s16b> new_effect(int type, int dam, int time, int cy, int cx, int rad, s32b flags);
bool cave_floor_bold(int y, int x);
bool cave_floor_grid(cave_type const *c);
bool cave_plain_floor_bold(int y, int x);
bool cave_plain_floor_grid(cave_type const *c);
bool cave_sight_bold(int y, int x);
bool cave_sight_grid(cave_type const *c);
bool cave_clean_bold(int y, int x);
bool cave_empty_bold(int y, int x);
bool cave_naked_bold(int y, int x);
bool cave_naked_bold2(int y, int x);
bool cave_perma_bold(int y, int x);
bool cave_perma_grid(cave_type const *c);
bool player_has_los_bold(int y, int x);
bool player_can_see_bold(int y, int x);
