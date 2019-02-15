#pragma once

#include "defines.h"
#include "h-basic.h"
#include "z-term.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char *ANGBAND_SYS;
extern char *ANGBAND_DIR_SAVE;
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
extern bool_ character_generated;
extern bool_ character_icky;
extern bool_ inkey_flag;
extern bool_ msg_flag;

#ifdef __cplusplus
} // extern "C"
#endif
