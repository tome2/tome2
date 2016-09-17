/* File: z-rand.c */

/* Purpose: a simple random number generator -BEN- */

#include "z-rand.hpp"

#include <assert.h>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <type_traits>

#include "pcg_random.hpp"
#include "seed.hpp"

/**
 * Choice of RNG; we use the "statistically most powerful" (per the
 * documentation) RNG. The "insecure" bit just means that the RNG
 * is definitely known to *not* be cryptographically secure.
 */
using rng_t = pcg64_once_insecure;

/**
 * Reseed the given RNG.
 */
static void reseed_rng(rng_t *rng, seed_t const &seed)
{
	assert(rng != nullptr);
	// Create a seed_seq from the seed data.
	std::uint32_t data[seed_t::n_uint32];
	std::seed_seq seed_seq(
	        std::begin(data),
	        std::end(data)
	);
	// Seed the RNG.
	rng->seed(seed_seq);
}

/**
 * Allocate a new RNG and initialize with the given seed.
 */
static rng_t *new_seeded_rng(seed_t const &seed)
{
	rng_t *rng = new rng_t;
	reseed_rng(rng, seed);
	return rng;
}

/**
 * The "quick" RNG is used for fixed-seed applications.
 */
static rng_t *quick_rng()
{
	// Note that the "quick_rng" will always be seeded explicitly
	// whenever it's used, so we don't need to do any seeding here.
	static rng_t *instance = new rng_t();
	return instance;
}

/**
 * The "complex" RNG is used when we really want non-deterministic
 * random numbers.
 */
static rng_t *complex_rng()
{
	static rng_t *instance = new_seeded_rng(seed_t::system());
	return instance;
}


/**
 * Current RNG.
 */
static rng_t *current_rng = nullptr;

/**
 * Get the current RNG.
 */
static rng_t *get_current_rng()
{
	// Do we need to initialize?
	if (current_rng == nullptr)
	{
		// We start with the complex RNG.
		current_rng = complex_rng();
	}

	return current_rng;
}

void set_quick_rng(seed_t const &seed)
{
	reseed_rng(quick_rng(), seed);
	current_rng = quick_rng();
}

void set_complex_rng()
{
	current_rng = complex_rng();
}

std::string get_complex_rng_state()
{
	std::stringstream s;
	s << *complex_rng();
	return s.str();
}

void set_complex_rng_state(std::string const &state)
{
	std::stringstream s(state);
	s >> *complex_rng();
}



/*
 * Stochastic rounding
 */
static double round_stochastic(double x)
{
	double n;
	double f = std::modf(x, &n);

	// Round up?
	if (f > 0.5)
	{
		return n + 1;
	}

	// Round down?
	if (f < 0.5)
	{
		return n - 1;
	}

	// Tie breaker is random; hence 'stochastic'.
	std::uniform_int_distribution<int> distribution(0, 1);
	if (distribution(*get_current_rng()) == 0)
	{
		return n - 1;
	}
	else
	{
		return n + 1;
	}
}


/*
 * Generate a random integer number of NORMAL distribution
 */
s16b randnor(int mean, int stand)
{
	// Get our own return type; we need it for limits and casting.
	using retval_t = std::result_of<decltype(&randnor)(int, int)>::type;

	// Degenerate case
	if (stand < 1)
	{
		return 0;
	}

	// Sample from normal distribution
	std::normal_distribution<double> distribution(mean, stand);
	double x = distribution(*get_current_rng());

	// Stochastic rounding to avoid rounding bias
	double rounded_x = round_stochastic(x);

	// Enforce limits of retval_t. Given that we're talking about a normal
	// distribution, we're usually very unlikely to actually hit these (given
	// reasonable values for 'mean' and 'stand' parameters), but in (very) rare
	// cases it's needed to avoid undefined behavior due to the conversion
	// we're going to do. This does introduce some (very minor) bias, but
	// it's really unavoidable since retval_t cannot represent all possible
	// values. We also assuming that a double can accurately represent all
	// values in the range of retval_t.
	double clipped_x = std::min(
	        static_cast<double>(std::numeric_limits<retval_t>::max()),
	        std::max(static_cast<double>(std::numeric_limits<retval_t>::min()),
	                rounded_x));

	// Done: We just need to convert to retval_t.
	return static_cast<retval_t>(clipped_x);
}



/*
 * Generates damage for "2d6" style dice rolls
 */
s32b damroll(s16b num, s16b sides)
{
	int i;
	s32b sum = 0;
	for (i = 0; i < num; i++) sum += randint(sides);
	return (sum);
}


/*
 * Same as above, but always maximal
 */
s32b maxroll(s16b num, s16b sides)
{
	return (num * sides);
}

bool magik(s32b p) {
	return rand_int(100) < p;
}

s32b rand_int(s32b m)
{
	/* Degenerate case */
	if (m < 1)
	{
		return 0;
	}
	/* Normal case */
	std::uniform_int_distribution<s32b> distribution(0, m - 1);
	return distribution(*get_current_rng());
}

s32b randint(s32b m)
{
	/* Degenerate case */
	if (m < 2)
	{
		return 1;
	}
	/* Normal case */
	std::uniform_int_distribution<s32b> distribution(1, m);
	return distribution(*get_current_rng());
}

s32b rand_range(s32b a, s32b b)
{
	/* Degenerate case */
	if (b < a)
	{
		return a;
	}
	/* Normal case */
	std::uniform_int_distribution<s32b> distribution(a, b);
	return distribution(*get_current_rng());
}

s32b rand_spread(s32b a, s32b d)
{
	return rand_range(a-d, a+d);
}
