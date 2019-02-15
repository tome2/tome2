#pragma once

#include "h-basic.hpp"
#include "monster_type_fwd.hpp"
#include "object_flag_set.hpp"

#include <boost/filesystem.hpp>
#include <string>
#include <string_view>
#include <vector>

std::string name_file_note(std::string_view);
std::string name_file_pref(std::string_view);
std::string name_file_save();
std::string name_file_save(std::string_view);

boost::filesystem::path name_file_dungeon_save(std::string const &ext);

void html_screenshot(const char *name);
void help_file_screenshot(const char *name);
object_flag_set player_flags();
void wipe_saved();
s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2);
void display_player(int mode);
std::string describe_player_location();
errr file_character(const char *name);
errr process_pref_file_aux(char *buf);
errr process_pref_file(std::string const &name);
void show_string(const char *lines, const char *title, int line = 0);
void show_file(const char *name, const char *what, int line = 0);
void do_cmd_help();
void get_name();
void do_cmd_suicide();
void autosave_checkpoint();
void close_game();
errr get_rnd_line(const char * file_name, char * output);
char *get_line(const char* fname, const char *fdir, char *linbuf, int line);
void race_legends();
void show_highclass(int building);
errr get_xtra_line(const char * file_name, monster_type *m_ptr, char * output);
std::string process_player_name(std::string const &);
void set_player_base(std::string const &name);

void do_cmd_save_game();
void predict_score_gui(bool *initialized, bool *game_in_progress);
