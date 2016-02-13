#pragma once

#include "h-basic.h"

#include <string>

extern bool_ quest_bounty_init_hook(int q_idx);
extern bool_ quest_bounty_drop_item();
extern bool_ quest_bounty_get_item();
extern std::string quest_bounty_describe();
