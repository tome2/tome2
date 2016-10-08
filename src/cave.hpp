#pragma once

#include "h-basic.h"
#include "cave_type_fwd.hpp"
#include "object_type_fwd.hpp"

extern int distance(int y1, int x1, int y2, int x2);
extern bool_ los(int y1, int x1, int y2, int x2);
extern bool_ cave_valid_bold(int y, int x);
extern bool_ no_lite(void);
extern void map_info_default(int y, int x, byte *ap, char *cp);
extern void move_cursor_relative(int row, int col);
extern void print_rel(char c, byte a, int y, int x);
extern void note_spot(int y, int x);
extern void lite_spot(int y, int x);
extern void prt_map(void);
extern void display_map(int *cy, int *cx);
extern void do_cmd_view_map(void);
extern errr vinfo_init(void);
extern void forget_view(void);
extern void update_view(void);
extern void forget_mon_lite(void);
extern void update_mon_lite(void);
extern void update_flow(void);
extern void map_area(void);
extern void wiz_lite(void);
extern void wiz_lite_extra(void);
extern void wiz_dark(void);
extern void cave_set_feat(int y, int x, int feat);
extern void place_floor(int y, int x);
extern void place_floor_convert_glass(int y, int x);
extern void place_filler(int y, int x);
extern void mmove2(int *y, int *x, int y1, int x1, int y2, int x2);
extern bool_ projectable(int y1, int x1, int y2, int x2);
extern void scatter(int *yp, int *xp, int y, int x, int d);
extern void health_track(int m_idx);
extern void monster_race_track(int r_idx, int ego);
extern void object_track(object_type *o_ptr);
extern void disturb();
extern void disturb_on_state();
extern void disturb_on_other();
extern int is_quest(int level);
extern int new_effect(int type, int dam, int time, int cy, int cx, int rad, s32b flags);
extern bool cave_floor_bold(int y, int x);
extern bool cave_floor_grid(cave_type const *c);
extern bool cave_plain_floor_bold(int y, int x);
extern bool cave_plain_floor_grid(cave_type const *c);
extern bool cave_sight_bold(int y, int x);
extern bool cave_sight_grid(cave_type const *c);
extern bool cave_clean_bold(int y, int x);
extern bool cave_empty_bold(int y, int x);
extern bool cave_naked_bold(int y, int x);
extern bool cave_naked_bold2(int y, int x);
extern bool cave_perma_bold(int y, int x);
extern bool cave_perma_grid(cave_type const *c);
extern bool player_has_los_bold(int y, int x);
extern bool player_can_see_bold(int y, int x);
