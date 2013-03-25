#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

bool_ quest_library_init_hook(int q);
bool_ quest_library_describe(FILE *fff);
void quest_library_building(bool_ *paid, bool_ *recreate);
void initialize_bookable_spells();

#ifdef __cplusplus
} // extern "C"
#endif
