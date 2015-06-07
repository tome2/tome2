#pragma once

#include "h-basic.h"

extern bool_ quest_bounty_init_hook(int q_idx);
extern bool_ quest_bounty_drop_item();
extern bool_ quest_bounty_get_item();
extern bool_ quest_bounty_describe(FILE *fff);
