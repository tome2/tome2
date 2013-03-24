#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

/******* Plot Bounty Quest ********/
extern bool_ quest_bounty_init_hook(int q_idx);
extern bool_ quest_bounty_drop_item();
extern bool_ quest_bounty_get_item();
extern bool_ quest_bounty_describe(FILE *fff);

#ifdef __cplusplus
} // extern "C"
#endif
