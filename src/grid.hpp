#pragma once

#include <boost/multi_array.hpp>

/**
 * 2D grid of T's.
 */
template <typename T>
struct grid {

private:
	boost::multi_array<T, 2> impl;

public:
	/**
	 * Get reference to tile.
	 */
	T &operator ()(std::size_t x, std::size_t y)
	{
		return impl[y][x];
	}

	/**
	 * Get const reference to tile.
	 */
	T const &operator ()(std::size_t x, std::size_t y) const
	{
		return impl[y][x];
	}

	/**
	 * Resize grid. There is no guarantee whether
	 * existing elements will be preserved or not.
	 */
	void resize(std::size_t width, std::size_t height)
	{
		impl.resize(boost::extents[height][width]);
	}

	/**
	 * Get current width.
	 */
	std::size_t width() const
	{
		return impl.shape()[1];
	}

	/**
	 * Get current height.
	 */
	std::size_t height() const
	{
		return impl.shape()[0];
	}

	/**
	 * Set width. Same caveats apply as for resize().
	 */
	void width(std::size_t width)
	{
		resize(width, height());
	}

	/**
	 * Set height. Same caveats apply as for resize().
	 */
	void height(std::size_t height)
	{
		resize(width(), height);
	}

};
