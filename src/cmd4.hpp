#pragma once

#include "h-basic.h"

void macro_recorder_start(void);
void macro_recorder_add(char c);
void macro_recorder_stop(void);
void do_cmd_macro_recorder(void);
void do_cmd_redraw(void);
void do_cmd_change_name(void);
void do_cmd_message_one(void);
void do_cmd_messages(void);
void do_cmd_options(void);
void do_cmd_pref(void);
void do_cmd_macros(void);
void do_cmd_visuals(void);
void do_cmd_colors(void);
void do_cmd_note(void);
void do_cmd_version(void);
void do_cmd_feeling(void);
void do_cmd_load_screen(void);
void do_cmd_save_screen(void);
void do_cmd_knowledge(void);
void do_cmd_checkquest(void);
void do_cmd_change_tactic(int i);
void do_cmd_change_movement(int i);
void do_cmd_time(void);
void do_cmd_options_aux(int page, cptr info, bool_ read_only);
