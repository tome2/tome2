/* File: cmovie.c */

/* Purpose: play cmovie files -DarkGod-Improv- */

#include "angband.h"

/*
 * Play a given cmovie
 */
s16b do_play_cmovie(cptr cmov_file)
{
	FILE *fff;

	int y, line = 0, x;
	int delay;

	char *s;

	char buf[1024];
	char cbuf[90];
	char ch;

	char mode = 0;


	/* Cmovie files are moved to the user directory on the multiuser systems */

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_CMOV, cmov_file);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Read the file */
	fff = my_fopen(buf, "r");

	/* Failure */
	if (!fff) return ( -1);

	/* Save screen */
	character_icky = TRUE;
	Term_save();
	Term_clear();

	/* Give some usefull info */
	prt("While viewing the movie you can press Escape to exit, t/Space to switch between", 0, 0);
	prt("fluid more and step by step mode and any other key to step a frame in step by", 1, 0);
	prt("step mode.", 2, 0);
	prt("You can press D to do an html screenshot of the current frame.", 3, 0);
	prt("You can also use + and - to speed up/down the playing speed.", 5, 0);
	prt("Press any key when ready.", 8, 0);

	inkey();

	Term_clear();

	line = -1;

	delay = 1;

	/* Init to white */
	for (x = 0; x < 80; x++)
	{
		cbuf[x] = 'w';
	}

	/* Parse */
	while (0 == my_fgets(fff, buf, 1024))
	{
		/* Do not wait */
		inkey_scan = TRUE;
		ch = inkey();

		/* Stop */
		if (ch == ESCAPE) break;

		/* Change mode */
		else if (ch == 't')
		{
			mode = FALSE;
		}
		else if (ch == ' ')
		{
			mode = TRUE;
		}

		/* Change speed */
		else if (ch == '+')
		{
			delay--;
			if (delay < 0) delay = 0;
		}
		else if (ch == '-')
		{
			delay++;
			if (delay > 5) delay = 5;
		}
		else if (ch == 'D')
		{
			do_cmd_html_dump();
		}

		line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') break;

		/* Clean screen */
		if (buf[0] == 'C')
		{
			Term_clear();

			/* Next */
			continue;
		}

		/* Displays a textbox */
		if (buf[0] == 'B')
		{
			int len = strlen(buf + 2);

			/* Clear the line */
			Term_erase(0, 0, 255);

			/* Display the message */
			c_put_str(TERM_VIOLET, "###", 0, 0);
			c_put_str(TERM_ORANGE, buf + 2, 0, 3);
			c_put_str(TERM_VIOLET, "###", 0, 3 + len);
			c_put_str(TERM_WHITE, "(more)", 0, 6 + len);

			/* Next */
			continue;
		}

		/* Wait a key */
		if (buf[0] == 'W')
		{
			inkey();

			/* Next */
			continue;
		}

		/* Sleep */
		if (buf[0] == 'S')
		{
			long msec;

			/* Scan for the values */
			if (1 != sscanf(buf + 2, "%ld:", &msec))
			{
				return ( -2);
			}

			if (!mode)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
			else
			{
				bool_ stop = FALSE;

				while (TRUE)
				{
					ch = inkey();

					/* Stop */
					if (ch == ESCAPE)
					{
						stop = TRUE;
						break;
					}
					/* Change mode */
					else if (ch == 't')
					{
						mode = FALSE;
						break;
					}
					/* Change mode */
					else if (ch == ' ')
					{
						if (mode) continue;
						mode = TRUE;
						break;
					}
					/* Change speed */
					else if (ch == '+')
					{
						delay--;
						if (delay < 0) delay = 0;
					}
					else if (ch == '-')
					{
						delay++;
						if (delay > 5) delay = 5;
					}
					else if (ch == 'D')
					{
						do_cmd_html_dump();
					}
					else break;
				}
				if (stop) break;
			}

			/* Next */
			continue;
		}

		/* Get color for the NEXT L line */
		if (buf[0] == 'E')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return ( -2);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return ( -2);

			/* Get the index */
			y = atoi(buf + 2);

			C_COPY(cbuf, s, 80, char);

			/* Next... */
			continue;
		}

		/* Print a line */
		if (buf[0] == 'L')
		{
			/* Find the colon before the name */
			s = strchr(buf + 2, ':');

			/* Verify that colon */
			if (!s) return ( -2);

			/* Nuke the colon, advance to the name */
			*s++ = '\0';

			/* Paranoia -- require a name */
			if (!*s) return ( -2);

			/* Get the index */
			y = atoi(buf + 2);

			for (x = 0; x < 80; x++)
			{
				Term_putch(x, y, color_char_to_attr(cbuf[x]), s[x]);

				/* Reinit to white */
				cbuf[x] = 'w';
			}
			Term_redraw_section(0, y, 79, y);

			/* Next... */
			continue;
		}

		/* Update 1 char */
		if (buf[0] == 'P')
		{
			int x, y, a, c;

			/* Scan for the values */
			if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
			                &x, &y, &c, &a))
			{
				a = 'w';
				if (3 != sscanf(buf + 2, "%d:%d:%d",
				                &x, &y, &c)) return ( -2);
			}

			Term_putch(x, y, color_char_to_attr(cbuf[x]), c);
			Term_redraw_section(x, y, x + 1, y + 1);

			/* Next... */
			continue;
		}
	}

	/* Load screen */
	Term_load();
	character_icky = FALSE;

	/* Close */
	my_fclose(fff);

	return (0);
}


/*
 * Start the recording of a cmovie
 */
void do_record_cmovie(cptr cmovie)
{
	char buf[1024];
	int fd = -1;
	int y;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_CMOV, cmovie);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Check for existing file */
	fd = fd_open(buf, O_RDONLY);

	/* Existing file */
	if (fd >= 0)
	{
		char out_val[160];

		/* Close the file */
		(void)fd_close(fd);

		/* Build query */
		(void)sprintf(out_val, "Replace existing file %s? ", cmovie);

		/* Ask */
		if (get_check(out_val)) fd = -1;
	}

	/* Be sure */
	if (!get_check("Ready to record(Press ctrl+D to enter a textual note while recording)?")) return;

	/* Open the non-existing file */
	if (fd < 0) movfile = my_fopen(buf, "w");

	/* Invalid file */
	if (movfile == NULL)
	{
		msg_format("Cmovie recording failed!");

		return;
	}

	/* First thing: Record clear screen then enable the recording */
	fprintf(movfile, "# Generated by %s\n",
	        get_version_string());
	fprintf(movfile, "C:\n");
	last_paused = 0;
	do_movies = 1;
	cmovie_init_second();

	/* Mega Hack, get the screen */
	for (y = 0; y < Term->hgt; y++)
	{
		cmovie_record_line(y);
	}
}


/*
 * Stop the recording
 */
void do_stop_cmovie()
{
	if (do_movies == 1)
	{
		do_movies = 0;
		my_fclose(movfile);
	}
}


/*
 * Start a cmovie
 */
void do_start_cmovie()
{
	char name[90], rname[94];


	/* Should never happen */
	if (do_movies == 1) return;

	/* Default */
	sprintf(name, "%s", player_base);

	if (get_string("Cmovie name: ", name, 80))
	{
		if (name[0] && (name[0] != ' '))
		{
			sprintf(rname, "%s.cmv", name);

			if (get_check("Record(y), Play(n)?")) do_record_cmovie(rname);
			else do_play_cmovie(rname);
		}
	}
}


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


/*
 * Write a record of a screen row into a cmovie file
 */
void cmovie_record_line(int y)
{
	char abuf[256];
	char cbuf[256];

	cmovie_clean_line(y, abuf, cbuf);

	/* Write a colour record */
	fprintf(movfile, "E:%d:%.80s\n", y, abuf);

	/* Write a char record */
	fprintf(movfile, "L:%d:%.80s\n", y, cbuf);
}


/*
 * Record a "text box"
 */
void do_cmovie_insert()
{
	char buf[81] = "";

	/* Dont record */
	do_movies = 2;

	while (get_string("Textbox(ESC to end): ", buf, 80))
	{
		fprintf(movfile, "B:%s\nW:\n", buf);
		buf[0] = '\0';
	}

	/* We reinit the time as to not count the time the user needed ot enter the text */
	cmovie_init_second();

	/* Continue recording */
	do_movies = 1;
}
