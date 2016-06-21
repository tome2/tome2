#pragma once

#include "flag_set.hpp"

constexpr std::size_t FF_MAX_TIERS = 1;

typedef flag_set<FF_MAX_TIERS> feature_flag_set;
