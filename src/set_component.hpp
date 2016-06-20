#pragma once

#include "h-basic.h"

#include <array>
#include <cstdint>

constexpr std::size_t SET_MAX_SIZE = 6;

struct set_component {
	bool_ present = FALSE;                   /* Is it actually wore ? */
	s16b a_idx = 0;                          /* What artifact? */
	std::array<s16b, SET_MAX_SIZE> pval;     /* Pval for each combination */
	std::array<u32b, SET_MAX_SIZE> flags1;   /* Flags */
	std::array<u32b, SET_MAX_SIZE> flags2;   /* Flags */
	std::array<u32b, SET_MAX_SIZE> flags3;   /* Flags */
	std::array<u32b, SET_MAX_SIZE> flags4;   /* Flags */
	std::array<u32b, SET_MAX_SIZE> flags5;   /* Flags */
	std::array<u32b, SET_MAX_SIZE> esp;      /* Flags */
};
