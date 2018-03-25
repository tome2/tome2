#pragma once

#include "h-basic.h"
#include "timer_type_fwd.hpp"

#include <vector>
#include <string>

#define MAX_IGNORE_KEYMAPS 12

bool input_box(std::string const &text, int y, int x, std::string *buf, std::size_t max);
std::string input_box_auto(std::string const &title, std::size_t max);
bool input_box_auto(std::string const &prompt, std::string *buf, std::size_t max);
void draw_box(int y, int x, int h, int w);
void display_list(int y, int x, int h, int w, cptr title, std::vector<std::string> const &list, std::size_t begin, std::size_t sel, byte sel_color);
std::string get_player_race_name(int pr, int ps);
std::string get_day(s32b day);
s32b bst(s32b what, s32b t);
FILE *my_fopen(cptr file, cptr mode);
errr my_fgets(FILE *fff, char *buf, huge n);
errr my_fclose(FILE *fff);
errr fd_kill(cptr file);
errr fd_move(cptr file, cptr what);
int fd_make(cptr file, int mode);
int fd_open(cptr file, int flags);
errr fd_seek(int fd, huge n);
errr fd_read(int fd, char *buf, huge n);
errr fd_write(int fd, cptr buf, huge n);
errr fd_close(int fd);
void flush();
void flush_on_failure();
void move_cursor(int row, int col);
void text_to_ascii(char *buf, cptr str);
void ascii_to_text(char *buf, cptr str);
char inkey_scan();
void display_message(int x, int y, int split, byte color, cptr t);
void cmsg_print(byte color, cptr msg);
void cmsg_print(byte color, std::string const &msg);
void msg_print(cptr msg);
void cmsg_format(byte color, cptr fmt, ...);
void msg_format(cptr fmt, ...);
void screen_save();
void screen_load();
void c_put_str(byte attr, cptr str, int row, int col);
void c_put_str(byte attr, std::string const &str, int row, int col);
void put_str(cptr str, int row, int col);
void put_str(std::string const &s, int row, int col);
void c_prt(byte attr, cptr str, int row, int col);
void c_prt(byte attr, std::string const &s, int row, int col);
void prt(std::string const &s, int row, int col);
void text_out_to_screen(byte a, cptr str);
void text_out_to_file(byte a, cptr str);
void text_out(cptr str);
void text_out_c(byte a, cptr str);
void text_out_c(byte a, std::string const &str);
void clear_from(int row);
int ask_menu(cptr ask, const std::vector<std::string> &items);
bool askfor_aux(std::string *buf, std::size_t max_len);
bool_ askfor_aux(char *buf, int len);
bool_ askfor_aux_with_completion(char *buf, int len);
bool_ get_string(cptr prompt, char *buf, int len);
bool_ get_check(cptr prompt);
bool_ get_com(cptr prompt, char *command);
s32b get_quantity(cptr prompt, s32b max);
extern char request_command_ignore_keymaps[MAX_IGNORE_KEYMAPS];
extern bool_ request_command_inven_mode;
void request_command(int shopping);
bool_ is_a_vowel(int ch);
int get_keymap_dir(char ch);
byte count_bits(u32b array);
void strlower(char *buf);
int test_monster_name(cptr name);
int test_mego_name(cptr name);
int test_item_name(cptr name);
char msg_box_auto(std::string const &title);
timer_type *new_timer(void (*callback)(), s32b delay);
int get_keymap_mode();
void repeat_push(int what);
bool_ repeat_pull(int *what);
void repeat_check();
void get_count(int number, int max);
bool in_bounds(int y, int x);
bool in_bounds2(int y, int x);
bool panel_contains(int y, int x);
errr path_parse(char *buf, int max, cptr file);
void pause_line(int row);
std::string user_name();
