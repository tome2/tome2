#include "tome/squelch/tree_printer_fwd.hpp"
#include "tome/squelch/tree_printer.hpp"

#include "../z-term.h"

namespace squelch {

TreePrinter::TreePrinter() : m_indent(0)
{
	int wid, hgt;
	// Output window
	Term_get_size(&wid, &hgt);
	m_write_out_y = 1;
	m_write_out_x = 16;
	m_write_out_h = hgt - 4 - 1;
	m_write_out_w = wid - 1 - 15 - 1;
	// Set position
	reset();
	reset_scroll();
}

void TreePrinter::indent() {
	m_indent++;
}

void TreePrinter::dedent() {
	m_indent--;
}

void TreePrinter::reset() {
	m_write_x = 0;
	m_write_y = 0;
}

void TreePrinter::reset_scroll() {
	m_write_off_y = 0;
	m_write_off_x = 0;
}

void TreePrinter::scroll_up() {
	m_write_off_y--;
}

void TreePrinter::scroll_down() {
	m_write_off_y++;
}

void TreePrinter::scroll_left() {
	m_write_off_x++;
}

void TreePrinter::scroll_right() {
	m_write_off_x--;
}

void TreePrinter::write(uint8_t color, cptr line)
{
	cptr p = line;

	for (p = line; *p != '\0'; p++)
	{
		char c = *p;
		int x = m_write_x - m_write_off_x + 3*m_indent;
		int y = m_write_y - m_write_off_y;

		if (c != '\n')
		{
			if ((y >= 0) &&
			    (y < m_write_out_h) &&
			    (x >= 0) &&
			    (x < m_write_out_w))
			{
				Term_putch(x + m_write_out_x,
					   y + m_write_out_y,
					   color,
					   c);
			}

			m_write_x += 1;
		}
		else
		{
			m_write_x = 0;
			m_write_y += 1;
		}
	}
}

void TreePrinter::write(uint8_t color, std::string const &line)
{
	write(color, line.c_str());
}

} // namespace
