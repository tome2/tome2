#ifndef H_3d6cc652_c674_4a84_911d_e8ec35cc992a
#define H_3d6cc652_c674_4a84_911d_e8ec35cc992a

#include <boost/noncopyable.hpp>
#include <cstdint>

namespace squelch {

/**
 * Printing trees.
 */
class TreePrinter : boost::noncopyable
{
public:
	TreePrinter();

	void indent();

	void dedent();

	void reset();

	void reset_scroll();

	void scroll_up();

	void scroll_down();

	void scroll_left();

	void scroll_right();

	void write(uint8_t color, const char *line);

private:
	int m_indent;
	int m_write_out_y;
	int m_write_out_x;
	int m_write_out_h;
	int m_write_out_w;
	int m_write_y;
	int m_write_x;
	int m_write_off_x;
	int m_write_off_y;
};

} // namespace

#endif
