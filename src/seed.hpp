#pragma once

#include <array>
#include <cstdint>

class seed_t {

public:
	// Number of seed bytes.
	static constexpr std::size_t n_bytes = 64;

	// Sanity check; we're relying on converting to uint32_t elsewhere,
	// so let's just keep it as easy as possible.
	static_assert(n_bytes % 4 == 0, "n_bytes must be multiple of 4");

	// Number of uint32_t's required to store the seed.
	static constexpr std::size_t n_uint32 = n_bytes / 4;

private:
	std::array<std::uint8_t, n_bytes> m_data;

	// Default constructor is private. Use the static
	// factory functions instead.
	seed_t()
	{
		// Factory functions do explicit initialization.
	};

public:

	/**
	 * Create a seed from system entropy.
	 */
	static seed_t system();

	/**
	 * Create a seed from the given bytes.
	 */
	static seed_t from_bytes(std::uint8_t bytes[n_bytes]);

	/**
	 * Convert seed to bytes.
	 */
	void to_bytes(std::uint8_t bytes[n_bytes]) const;

	/**
	 * Convert seed to uint32_t's suitable for seed_seq.
	 */
	void to_uint32(std::uint32_t seed_seq_data[n_uint32]) const;

};
