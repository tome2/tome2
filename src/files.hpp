#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"
#include "object_flag_set.hpp"

#include <string>
#include <vector>

void html_screenshot(cptr name);
void help_file_screenshot(cptr name);
object_flag_set player_flags();
void wipe_saved(void);
s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2);
void display_player(int mode);
std::string describe_player_location();
errr file_character(cptr name, bool_ full);
errr process_pref_file_aux(char *buf);
errr process_pref_file(cptr name);
void show_string(const char *lines, const char *title, int line = 0);
void show_file(cptr name, cptr what, int line = 0);
void do_cmd_help(void);
void process_player_base(void);
void get_name(void);
void do_cmd_suicide(void);
void autosave_checkpoint();
void close_game(void);
errr get_rnd_line(const char * file_name, char * output);
char *get_line(const char* fname, cptr fdir, char *linbuf, int line);
void race_legends(void);
void show_highclass(int building);
errr get_xtra_line(const char * file_name, monster_type *m_ptr, char * output);
void process_player_name(bool_ sf);
