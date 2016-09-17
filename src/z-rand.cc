/* File: z-rand.c */

/* Purpose: a simple random number generator -BEN- */

#include "z-rand.hpp"

#include <assert.h>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>

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
 * The number of entries in the "randnor_table"
 */
#define RANDNOR_NUM	256

/*
 * The standard deviation of the "randnor_table"
 */
#define RANDNOR_STD	64

/*
 * The normal distribution table for the "randnor()" function (below)
 */
static s16b randnor_table[RANDNOR_NUM] =
{
	206, 613, 1022, 1430, 1838, 2245, 2652, 3058,
	3463, 3867, 4271, 4673, 5075, 5475, 5874, 6271,
	6667, 7061, 7454, 7845, 8234, 8621, 9006, 9389,
	9770, 10148, 10524, 10898, 11269, 11638, 12004, 12367,
	12727, 13085, 13440, 13792, 14140, 14486, 14828, 15168,
	15504, 15836, 16166, 16492, 16814, 17133, 17449, 17761,
	18069, 18374, 18675, 18972, 19266, 19556, 19842, 20124,
	20403, 20678, 20949, 21216, 21479, 21738, 21994, 22245,

	22493, 22737, 22977, 23213, 23446, 23674, 23899, 24120,
	24336, 24550, 24759, 24965, 25166, 25365, 25559, 25750,
	25937, 26120, 26300, 26476, 26649, 26818, 26983, 27146,
	27304, 27460, 27612, 27760, 27906, 28048, 28187, 28323,
	28455, 28585, 28711, 28835, 28955, 29073, 29188, 29299,
	29409, 29515, 29619, 29720, 29818, 29914, 30007, 30098,
	30186, 30272, 30356, 30437, 30516, 30593, 30668, 30740,
	30810, 30879, 30945, 31010, 31072, 31133, 31192, 31249,

	31304, 31358, 31410, 31460, 31509, 31556, 31601, 31646,
	31688, 31730, 31770, 31808, 31846, 31882, 31917, 31950,
	31983, 32014, 32044, 32074, 32102, 32129, 32155, 32180,
	32205, 32228, 32251, 32273, 32294, 32314, 32333, 32352,
	32370, 32387, 32404, 32420, 32435, 32450, 32464, 32477,
	32490, 32503, 32515, 32526, 32537, 32548, 32558, 32568,
	32577, 32586, 32595, 32603, 32611, 32618, 32625, 32632,
	32639, 32645, 32651, 32657, 32662, 32667, 32672, 32677,

	32682, 32686, 32690, 32694, 32698, 32702, 32705, 32708,
	32711, 32714, 32717, 32720, 32722, 32725, 32727, 32729,
	32731, 32733, 32735, 32737, 32739, 32740, 32742, 32743,
	32745, 32746, 32747, 32748, 32749, 32750, 32751, 32752,
	32753, 32754, 32755, 32756, 32757, 32757, 32758, 32758,
	32759, 32760, 32760, 32761, 32761, 32761, 32762, 32762,
	32763, 32763, 32763, 32764, 32764, 32764, 32764, 32765,
	32765, 32765, 32765, 32766, 32766, 32766, 32766, 32767,
};



/*
 * Generate a random integer number of NORMAL distribution
 *
 * The table above is used to generate a psuedo-normal distribution,
 * in a manner which is much faster than calling a transcendental
 * function to calculate a true normal distribution.
 *
 * Basically, entry 64*N in the table above represents the number of
 * times out of 32767 that a random variable with normal distribution
 * will fall within N standard deviations of the mean.  That is, about
 * 68 percent of the time for N=1 and 95 percent of the time for N=2.
 *
 * The table above contains a "faked" final entry which allows us to
 * pretend that all values in a normal distribution are strictly less
 * than four standard deviations away from the mean.  This results in
 * "conservative" distribution of approximately 1/32768 values.
 *
 * Note that the binary search takes up to 16 quick iterations.
 */
s16b randnor(int mean, int stand)
{
	s16b tmp;
	s16b offset;

	s16b low = 0;
	s16b high = RANDNOR_NUM;

	/* Paranoia */
	if (stand < 1) return (mean);

	/* Roll for probability */
	tmp = (s16b)rand_int(32768);

	/* Binary Search */
	while (low < high)
	{
		int mid = (low + high) >> 1;

		/* Move right if forced */
		if (randnor_table[mid] < tmp)
		{
			low = mid + 1;
		}

		/* Move left otherwise */
		else
		{
			high = mid;
		}
	}

	/* Convert the index into an offset */
	offset = (long)stand * (long)low / RANDNOR_STD;

	/* One half should be negative */
	if (rand_int(100) < 50) return (mean - offset);

	/* One half should be positive */
	return (mean + offset);
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
