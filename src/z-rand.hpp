#pragma once

#include "h-basic.h"



/**** Available constants ****/


/*
 * Random Number Generator -- Degree of "complex" RNG -- see "misc.c"
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63




/**** Available Variables ****/


extern bool_ Rand_quick;
extern u32b Rand_value;
extern u16b Rand_place;
extern u32b Rand_state[RAND_DEG];


/**** Available Functions ****/


void Rand_state_init(u32b seed);
s32b Rand_mod(s32b m);
s16b randnor(int mean, int stand);
s32b damroll(s16b num, s16b sides);
s32b maxroll(s16b num, s16b sides);

/**
 * Evaluate to "true" p percent of the time.
 */
bool magik(s32b p);

/*
 * Generates a random long integer X where 0<=X<M.
 * The integer X falls along a uniform distribution.
 * For example, if M is 100, you get "percentile dice"
 */
s32b rand_int(s32b m);

/*
 * Generate a random long integer X where 1<=X<=M
 * Also, "correctly" handle the case of M<=1
 */
s32b randint(s32b m);

/*
 * Generates a random long integer X where A<=X<=B
 * The integer X falls along a uniform distribution.
 * Note: rand_range(0,N-1) == rand_int(N)
 */
s32b rand_range(s32b a, s32b b);

/*
 * Generate a random long integer X where A-D<=X<=A+D
 * The integer X falls along a uniform distribution.
 * Note: rand_spread(A,D) == rand_range(A-D,A+D)
 */
s32b rand_spread(s32b a, s32b d);
