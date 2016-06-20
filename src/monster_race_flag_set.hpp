#pragma once

#include "flag_set.hpp"

constexpr std::size_t RF_MAX_TIERS = 6;

typedef flag_set<RF_MAX_TIERS> monster_race_flag_set;
