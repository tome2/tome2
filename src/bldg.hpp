#pragma once

#include "store_action_type_fwd.hpp"
#include "store_type_fwd.hpp"

bool bldg_process_command(store_type const *s_ptr, store_action_type const *action);
void show_building(store_type const *s_ptr);
bool is_state(store_type const *s_ptr, int state);
void enter_quest();
