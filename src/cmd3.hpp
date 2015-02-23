#pragma once

#include "angband.h"

extern void do_cmd_html_dump(void);
extern void cli_add(cptr active, cptr trigger, cptr descr);
extern void do_cmd_cli(void);
extern void do_cmd_cli_help(void);
extern void do_cmd_inven(void);
extern void do_cmd_equip(void);
extern void do_cmd_wield(void);
extern void do_cmd_takeoff(void);
extern void do_cmd_drop(void);
extern void do_cmd_destroy(void);
extern void do_cmd_observe(void);
extern void do_cmd_uninscribe(void);
extern void do_cmd_inscribe(void);
extern void do_cmd_refill(void);
extern void do_cmd_target(void);
extern void do_cmd_look(void);
extern void do_cmd_locate(void);
extern void do_cmd_query_symbol(void);
extern bool_ do_cmd_sense_grid_mana(void);
extern bool_ research_mon(void);
extern s32b portable_hole_weight(void);
extern void set_portable_hole_weight(void);
extern void do_cmd_portable_hole(void);
