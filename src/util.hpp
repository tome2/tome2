#pragma once

#include "h-basic.h"
#include "timer_type_fwd.hpp"

#include <vector>
#include <string>

#define MAX_IGNORE_KEYMAPS 12

extern bool input_box(std::string const &text, int y, int x, std::string *buf, std::size_t max);
extern std::string input_box_auto(std::string const &title, std::size_t max);
extern bool input_box_auto(std::string const &prompt, std::string *buf, std::size_t max);
extern void draw_box(int y, int x, int h, int w);
extern void display_list(int y, int x, int h, int w, cptr title, std::vector<std::string> const &list, std::size_t begin, std::size_t sel, byte sel_color);
extern std::string get_player_race_name(int pr, int ps);
extern std::string get_day(s32b day);
extern s32b bst(s32b what, s32b t);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fgets(FILE *fff, char *buf, huge n);
extern errr my_fclose(FILE *fff);
extern errr fd_kill(cptr file);
extern errr fd_move(cptr file, cptr what);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_seek(int fd, huge n);
extern errr fd_read(int fd, char *buf, huge n);
extern errr fd_write(int fd, cptr buf, huge n);
extern errr fd_close(int fd);
extern void flush(void);
extern void flush_on_failure();
extern void move_cursor(int row, int col);
extern void text_to_ascii(char *buf, cptr str);
extern void ascii_to_text(char *buf, cptr str);
extern char inkey_scan(void);
extern void display_message(int x, int y, int split, byte color, cptr t);
extern void cmsg_print(byte color, cptr msg);
extern void msg_print(cptr msg);
extern void cmsg_format(byte color, cptr fmt, ...);
extern void msg_format(cptr fmt, ...);
extern void screen_save(void);
extern void screen_load(void);
extern void c_put_str(byte attr, cptr str, int row, int col);
extern void c_put_str(byte attr, std::string const &str, int row, int col);
extern void put_str(cptr str, int row, int col);
extern void put_str(std::string const &s, int row, int col);
extern void c_prt(byte attr, cptr str, int row, int col);
extern void c_prt(byte attr, std::string const &s, int row, int col);
extern void prt(std::string const &s, int row, int col);
extern void text_out_to_screen(byte a, cptr str);
extern void text_out_to_file(byte a, cptr str);
extern void text_out(cptr str);
extern void text_out_c(byte a, cptr str);
extern void clear_from(int row);
extern int ask_menu(cptr ask, const std::vector<std::string> &items);
extern bool askfor_aux(std::string *buf, std::size_t max_len);
extern bool_ askfor_aux(char *buf, int len);
extern bool_ askfor_aux_with_completion(char *buf, int len);
extern bool_ get_string(cptr prompt, char *buf, int len);
extern bool_ get_check(cptr prompt);
extern bool_ get_com(cptr prompt, char *command);
extern s32b get_quantity(cptr prompt, s32b max);
extern char request_command_ignore_keymaps[MAX_IGNORE_KEYMAPS];
extern bool_ request_command_inven_mode;
extern void request_command(int shopping);
extern bool_ is_a_vowel(int ch);
extern int get_keymap_dir(char ch);
extern byte count_bits(u32b array);
extern void strlower(char *buf);
extern int test_monster_name(cptr name);
extern int test_mego_name(cptr name);
extern int test_item_name(cptr name);
extern char msg_box_auto(std::string const &title);
extern timer_type *new_timer(void (*callback)(), s32b delay);
extern int get_keymap_mode();
extern void repeat_push(int what);
extern bool_ repeat_pull(int *what);
extern void repeat_check(void);
extern void get_count(int number, int max);
extern bool in_bounds(int y, int x);
extern bool in_bounds2(int y, int x);
extern bool panel_contains(int y, int x);
extern errr path_parse(char *buf, int max, cptr file);
extern void pause_line(int row);
extern std::string user_name();
