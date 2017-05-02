#pragma once

#include "h-basic.h"

void macro_recorder_start();
void macro_recorder_add(char c);
void macro_recorder_stop();
void do_cmd_macro_recorder();
void do_cmd_redraw();
void do_cmd_change_name();
void do_cmd_message_one();
void do_cmd_messages();
void do_cmd_options();
void do_cmd_pref();
void do_cmd_macros();
void do_cmd_visuals();
void do_cmd_colors();
void do_cmd_note();
void do_cmd_version();
void do_cmd_feeling();
void do_cmd_load_screen();
void do_cmd_save_screen();
void do_cmd_knowledge();
void do_cmd_checkquest();
void do_cmd_change_tactic(int i);
void do_cmd_change_movement(int i);
void do_cmd_time();
void do_cmd_options_aux(int page, cptr info, bool_ read_only);
