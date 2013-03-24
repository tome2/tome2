#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_ quest_necro_init_hook(int q_idx);
bool_ quest_sauron_init_hook(int q_idx);
bool_ quest_morgoth_init_hook(int q_idx);

#ifdef __cplusplus
} // extern "C"
#endif
