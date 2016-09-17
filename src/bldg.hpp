#pragma once

#include "h-basic.h"
#include "store_action_type_fwd.hpp"
#include "store_type_fwd.hpp"

extern bool_ bldg_process_command(store_type const *s_ptr, store_action_type const *action);
extern void show_building(store_type const *s_ptr);
extern bool_ is_state(store_type const *s_ptr, int state);
extern void enter_quest(void);
