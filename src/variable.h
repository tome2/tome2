#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cptr ANGBAND_SYS;
extern cptr ANGBAND_KEYBOARD;
extern char *ANGBAND_DIR_MODULES;
extern char *ANGBAND_DIR_SAVE;
extern char *ANGBAND_DIR_CORE;
extern char *ANGBAND_DIR_DNGN;
extern char *ANGBAND_DIR_DATA;
extern char *ANGBAND_DIR_EDIT;
extern char *ANGBAND_DIR_FILE;
extern char *ANGBAND_DIR_HELP;
extern char *ANGBAND_DIR_INFO;
extern char *ANGBAND_DIR_NOTE;
extern char *ANGBAND_DIR_PREF;
extern char *ANGBAND_DIR_USER;
extern char *ANGBAND_DIR_XTRA;
extern term *angband_term[ANGBAND_TERM_MAX];
extern char angband_term_name[ANGBAND_TERM_MAX][80];
extern byte angband_color_table[256][4];
extern bool_ arg_wizard;
extern bool_ arg_force_original;
extern bool_ arg_force_roguelike;
extern bool_ character_generated;
extern bool_ character_icky;
extern bool_ inkey_flag;
extern bool_ msg_flag;
extern char player_name[32];
extern char player_base[32];
extern char savefile[1024];

#ifdef __cplusplus
} // extern "C"
#endif
