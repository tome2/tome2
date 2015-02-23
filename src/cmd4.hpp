#pragma once

#include "angband.h"

extern void macro_recorder_start(void);
extern void macro_recorder_add(char c);
extern void macro_recorder_stop(void);
extern void do_cmd_macro_recorder(void);
extern void do_cmd_redraw(void);
extern void do_cmd_change_name(void);
extern void do_cmd_message_one(void);
extern void do_cmd_messages(void);
extern void do_cmd_options(void);
extern void do_cmd_pref(void);
extern void do_cmd_macros(void);
extern void do_cmd_visuals(void);
extern void do_cmd_colors(void);
extern void do_cmd_note(void);
extern void do_cmd_version(void);
extern void do_cmd_feeling(void);
extern void do_cmd_load_screen(void);
extern void do_cmd_save_screen(void);
extern void do_cmd_knowledge(void);
extern void do_cmd_checkquest(void);
extern void do_cmd_change_tactic(int i);
extern void do_cmd_change_movement(int i);
extern void do_cmd_time(void);
extern void do_cmd_options_aux(int page, cptr info, bool_ read_only);
