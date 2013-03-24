#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_ is_randhero(int level);
bool_ quest_random_init_hook(int q_idx);
bool_ quest_random_describe(FILE *fff);

#ifdef __cplusplus
} // extern "C"
#endif
