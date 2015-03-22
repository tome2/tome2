#pragma once

#include "angband.h"

extern void quest_fireproof_building(bool_ *paid, bool_ *recreate);
extern bool_ quest_fireproof_init_hook(int q);
extern bool_ quest_fireproof_describe(FILE *fff);
