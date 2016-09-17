#include "seed.hpp"

#include <random>

seed_t seed_t::system()
{
	seed_t seed;
	// Use system's random device for seeding.
	std::random_device random_device;
	std::uniform_int_distribution<std::uint8_t> distribution;
	// Extract the number of bytes we need.
	for (std::size_t i = 0; i < n_bytes; i++)
	{
		seed.m_data[i] = distribution(random_device);
	}
	// Done
	return seed;
}

seed_t seed_t::from_bytes(std::uint8_t bytes[n_bytes])
{
	seed_t seed;
	// Copy
	for (std::size_t i = 0; i < n_bytes; i++)
	{
		seed.m_data[i] = bytes[i];
	}
	// Done
	return seed;
}

void seed_t::to_bytes(std::uint8_t bytes[n_bytes]) const
{
	// Copy
	for (std::size_t i = 0; i < n_bytes; i++)
	{
		bytes[i] = m_data[i];
	}
}
void seed_t::to_uint32(std::uint32_t seed_seq_data[n_uint32]) const
{
	for (std::size_t i = 0; i < n_uint32; i++)
	{
		// Position in the byte-oriented data.
		std::size_t p = 4 * i;
		// Pack m_data[p + 0], ..., m_data[p + 3] into a single uint32_t
		seed_seq_data[i] = 0;
		seed_seq_data[i] |= (uint32_t(m_data[p + 0]) << (0 * 8));
		seed_seq_data[i] |= (uint32_t(m_data[p + 1]) << (1 * 8));
		seed_seq_data[i] |= (uint32_t(m_data[p + 2]) << (2 * 8));
		seed_seq_data[i] |= (uint32_t(m_data[p + 3]) << (3 * 8));
	}
}
