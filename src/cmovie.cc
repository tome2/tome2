/* File: cmovie.c */

/* Purpose: play cmovie files -DarkGod-Improv- */

#include "angband.h"

void cmovie_clean_line(int y, char *abuf, char *cbuf)
{
	const byte *ap = Term->scr->a[y];
	const char *cp = Term->scr->c[y];

	byte a;
	char c;

	int x;
	int wid, hgt;
	int screen_wid, screen_hgt;


	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	/* Calculate the size of dungeon map area */
	screen_wid = wid - (COL_MAP + 1);
	screen_hgt = hgt - (ROW_MAP + 1);

	/* For the time being, assume 80 column display XXX XXX XXX */
	for (x = 0; x < wid; x++)
	{
		/* Convert dungeon map into default attr/chars */
		if (!character_icky &&
		                ((x - COL_MAP) >= 0) &&
		                ((x - COL_MAP) < screen_wid) &&
		                ((y - ROW_MAP) >= 0) &&
		                ((y - ROW_MAP) < screen_hgt))
		{
			/* Retrieve default attr/char */
			map_info_default(y + panel_row_prt, x + panel_col_prt, &a, &c);

			abuf[x] = conv_color[a & 0xf];

			if (c == '\0') cbuf[x] = ' ';
			else cbuf[x] = c;
		}

		else
		{
			abuf[x] = conv_color[ap[x] & 0xf];
			cbuf[x] = cp[x];
		}
	}

	/* Null-terminate the prepared strings */
	abuf[x] = '\0';
	cbuf[x] = '\0';
}
