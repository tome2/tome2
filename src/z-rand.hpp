#pragma once

#include "h-basic.h"
#include "seed_fwd.hpp"

#include <cassert>
#include <string>

/**** Available constants ****/


/*
 * Random Number Generator -- Degree of "complex" RNG -- see "misc.c"
 * This value is hard-coded at 63 for a wide variety of reasons.
 */
#define RAND_DEG 63




/**** Available Variables ****/


/**
 * Change to "quick" RNG, using the given seed.
 */
void set_quick_rng(seed_t const &seed);


/**
 * Change to "complex" RNG which uses the "non-deterministic" seed.
 */
void set_complex_rng();


/**
 * Get a copy of the state of the "complex" RNG.
 */
std::string get_complex_rng_state();


/**
 * Set the state of the "complex" RNG. The given array must have
 * been previously obtained via the get_complex_rng_state() function.
 */
void set_complex_rng_state(std::string const &state);

/**** Available Functions ****/


void Rand_state_init();
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

/**
 * Choose a random element in from the given container.
 * The container, C, must fulfill the Container concept
 * whose iterators fulfill the RandomIterator concept.
 **/
template <class C> typename C::const_iterator uniform_element(C const &c)
{
	assert(!c.empty());
	return std::next(std::cbegin(c), rand_int(c.size()));
}

/**
 * Choose a random element in from the given container.
 * The container, C, must fulfill the Container concept
 * whose iterators fulfill the RandomIterator concept.
 **/
template <class C> typename C::iterator uniform_element(C &c)
{
	assert(!c.empty());
	return std::next(std::begin(c), rand_int(c.size()));
}

/**
 * Shuffle contents of given random-access container.
 */
template <class C> void shuffle(C &c)
{
	if (c.empty())
	{
		return;
	}

	auto n = c.size();

	for (auto i = n - 1; i > 0; i--)
	{
		std::swap(c[i], c[rand_int(i + 1)]);
	}
}
