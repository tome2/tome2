#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

void do_cmd_save_game();
void predict_score_gui(bool_ *initialized, bool_ *game_in_progress);

#ifdef __cplusplus
} // extern "C"
#endif
