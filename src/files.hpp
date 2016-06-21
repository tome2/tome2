#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"
#include "object_flag_set.hpp"

#include <string>
#include <vector>

extern void html_screenshot(cptr name);
extern void help_file_screenshot(cptr name);
extern object_flag_set player_flags();
extern void wipe_saved(void);
extern s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2);
extern void display_player(int mode);
extern cptr describe_player_location(void);
extern errr file_character(cptr name, bool_ full);
extern errr process_pref_file_aux(char *buf);
extern errr process_pref_file(cptr name);
extern void show_string(const char *lines, const char *title, int line = 0);
extern void show_file(cptr name, cptr what, int line = 0);
extern void do_cmd_help(void);
extern void process_player_base(void);
extern void get_name(void);
extern void do_cmd_suicide(void);
extern void autosave_checkpoint();
extern void close_game(void);
extern errr get_rnd_line(const char * file_name, char * output);
extern char *get_line(const char* fname, cptr fdir, char *linbuf, int line);
extern void race_legends(void);
extern void show_highclass(int building);
extern errr get_xtra_line(const char * file_name, monster_type *m_ptr, char * output);
extern void process_player_name(bool_ sf);
