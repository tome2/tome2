#pragma once

#include "h-basic.h"

/**
 * Maximum number of quarks.
 */
constexpr int QUARK_MAX = 768;

void quark_init();
cptr quark_str(s16b num);
s16b quark_add(cptr str);
