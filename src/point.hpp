#pragma once

#include <cstdint>

struct point {

private:
	std::uint8_t m_x;
	std::uint8_t m_y;

public:
	point(std::uint8_t x, std::uint8_t y)
		: m_x(x), m_y(y) {
	}

	std::uint8_t x() const {
		return m_x;
	}

	std::uint8_t y() const {
		return m_y;
	}

};
