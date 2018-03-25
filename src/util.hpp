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
void display_list(int y, int x, int h, int w, const char *title, std::vector<std::string> const &list, std::size_t begin, std::size_t sel, byte sel_color);
std::string get_player_race_name(int pr, int ps);
std::string get_day(s32b day);
s32b bst(s32b what, s32b t);
FILE *my_fopen(const char *file, const char *mode);
errr my_fgets(FILE *fff, char *buf, unsigned long n);
errr my_fclose(FILE *fff);
errr fd_kill(const char *file);
errr fd_move(const char *file, const char *what);
int fd_make(const char *file, int mode);
int fd_open(const char *file, int flags);
errr fd_seek(int fd, unsigned long n);
errr fd_read(int fd, char *buf, unsigned long n);
errr fd_write(int fd, const char *buf, unsigned long n);
errr fd_close(int fd);
void flush();
void flush_on_failure();
void move_cursor(int row, int col);
void text_to_ascii(char *buf, const char *str);
void ascii_to_text(char *buf, const char *str);
char inkey_scan();
void display_message(int x, int y, int split, byte color, const char *t);
void cmsg_print(byte color, const char *msg);
void cmsg_print(byte color, std::string const &msg);
void msg_print(const char *msg);
void cmsg_format(byte color, const char *fmt, ...);
void msg_format(const char *fmt, ...);
void screen_save();
void screen_load();
void c_put_str(byte attr, const char *str, int row, int col);
void c_put_str(byte attr, std::string const &str, int row, int col);
void put_str(const char *str, int row, int col);
void put_str(std::string const &s, int row, int col);
void c_prt(byte attr, const char *str, int row, int col);
void c_prt(byte attr, std::string const &s, int row, int col);
void prt(std::string const &s, int row, int col);
void text_out_to_screen(byte a, const char *str);
void text_out_to_file(byte a, const char *str);
void text_out(const char *str);
void text_out(std::string const &str);
void text_out_c(byte a, const char *str);
void text_out_c(byte a, std::string const &str);
void clear_from(int row);
int ask_menu(const char *ask, const std::vector<std::string> &items);
bool askfor_aux(std::string *buf, std::size_t max_len);
bool_ askfor_aux(char *buf, int len);
bool_ askfor_aux_with_completion(char *buf, int len);
bool_ get_string(const char *prompt, char *buf, int len);
bool_ get_check(const char *prompt);
bool_ get_com(const char *prompt, char *command);
s32b get_quantity(const char *prompt, s32b max);
extern char request_command_ignore_keymaps[MAX_IGNORE_KEYMAPS];
extern bool_ request_command_inven_mode;
void request_command(int shopping);
bool_ is_a_vowel(int ch);
int get_keymap_dir(char ch);
byte count_bits(u32b array);
void strlower(char *buf);
int test_monster_name(const char *name);
int test_mego_name(const char *name);
int test_item_name(const char *name);
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
errr path_parse(char *buf, int max, const char *file);
void pause_line(int row);
std::string user_name();
