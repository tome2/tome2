#pragma once

#include <numeric>
#include <vector>

/**
 * A monoid is an algebraic structure with a single associative
 * binary operation ('append') and an identity element ('empty').
 *
 * See https://en.wikipedia.org/wiki/Monoid
 *
 * Shamelessly adapted from:
 *
 *    https://gist.github.com/evincarofautumn/2b5f004ca81e33c62ff0
 */
template<typename T, T append_(T const&, T const&), const T &empty_>
struct monoid {
	/* Access the type the monoid operates on */
	typedef T type;

	/* Append two T's */
	static T append(T const& a, T const& b) {
		return append_(a, b);
	}

	/* The value of an empty T */
	static constexpr T empty = empty_;
};

/**
 * mconcat :: (Monoid m mappend) -> [m] -> m
 * mconcat = fold mappend mempty
 */
template<typename M>
typename M::type mconcat(const std::vector<typename M::type>& xs) {
	return std::accumulate(std::begin(xs), std::end(xs), M::empty, M::append);
}
