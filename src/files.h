#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern bool_ txt_to_html(cptr head, cptr food, cptr base, cptr ext, bool_ force, bool_ recur);
extern void process_player_name(bool_ sf);
extern void do_cmd_save_game(void);
extern void predict_score_gui(bool_ *initialized, bool_ *game_in_progress);

#ifdef __cplusplus
} // extern "C"
#endif
