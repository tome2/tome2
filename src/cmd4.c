/* File: cmd4.c */

/* Purpose: Interface commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include "messages.h"

/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 */
void do_cmd_redraw(void)
{
	int j;

	term *old = Term;


	/* Hack -- react to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Combine and Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);


	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_POWERS |
	                  PU_SANITY | PU_BODY);

	/* Forget view */
	p_ptr->update |= (PU_UN_VIEW);

	/* Update view */
	p_ptr->update |= (PU_VIEW);

	/* Update monster light */
	p_ptr->update |= (PU_MON_LITE);

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER | PW_M_LIST);

	/* Window stuff */
	p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_MONSTER | PW_OBJECT);

	/* Hack -- update */
	handle_stuff();


	/* Redraw every window */
	for (j = 0; j < 8; j++)
	{
		/* Dead window */
		if (!angband_term[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Redraw */
		Term_redraw();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
	char	c;

	int	mode = 0;

	char	tmp[160];


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Forever */
	while (1)
	{
		/* keep mode below 7 */
		mode = (mode + 6) % 6;

		/* Display the player */
		display_player(mode);

		/* Prompt */
		if (mode == 0)
		{
			Term_putstr(14, 22, -1, TERM_WHITE,
			            "['t/T' to change tactics, 'e/E' to change movement]");
		}

		Term_putstr(4, 23, -1, TERM_WHITE,
		            "['c' to change name, 'f' to file, 'p' for previous, 'n' for next, or ESC]");

		/* Query */
		c = inkey();

		/* Exit */
		if (c == ESCAPE) break;

		/* Change name */
		if (c == 'c')
		{
			get_name();
		}

		/* File dump */
		else if (c == 'f')
		{
			strnfmt(tmp, 160, "%s.txt", player_name);
			if (get_string("Filename(you can post it to http://angband.oook.cz/): ", tmp, 80))
			{
				if (tmp[0] && (tmp[0] != ' '))
				{
					file_character(tmp, FALSE);
				}
			}
		}

		/* Toggle mode */
		else if (c == 'n')
		{
			mode++;
		}
		else if (c == 'p')
		{
			mode--;
		}

		else if (mode == 0)
		{
			/* Change tactic */
			if (c == 't')
			{
				(void)do_cmd_change_tactic( -1);
			}
			else if (c == 'T')
			{
				(void)do_cmd_change_tactic(1);
			}

			/* Change movement */
			else if (c == 'e')
			{
				do_cmd_change_movement( -1);
			}
			else if (c == 'E')
			{
				do_cmd_change_movement(1);
			}
			else
			{
				bell();
			}
		}
		/* Oops */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;


	/* Redraw everything */
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);

	handle_stuff();
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	cptr msg = format("> %s", message_str(0));

	/* Recall one message XXX XXX XXX */
	display_message(0, 0, strlen(msg), message_color(0), msg);
}


/*
 * Show previous messages to the user	-BEN-
 *
 * The screen format uses line 0 and (Term->hgt - 1) for headers and prompts,
 * skips line 1 and (Term->hgt - 2), and uses line 2 thru (Term->hgt - 3) for
 * old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only hilite the matching portions of the string.
 *
 * Now taking advantages of big-screen. -pav-
 */
void do_cmd_messages(void)
{
	int i, j, k, n;
	u32b q;
	int wid, hgt;

	char shower[80];
	char finder[80];

	/* Wipe finder */
	strcpy(finder, "");

	/* Wipe shower */
	strcpy(shower, "");


	/* Total messages */
	n = message_num();

	/* Start on first message */
	i = 0;

	/* Start at leftmost edge */
	q = 0;

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Retrieve current screen size */
		Term_get_size(&wid, &hgt);

		/* Dump up to 20 (or more in bigscreen) lines of messages */
		for (j = 0; (j < (hgt - 4)) && (i + j < n); j++)
		{
			cptr msg = message_str(i + j);
			byte color = message_color(i + j);

			/* Apply horizontal scroll */
			msg = (strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			display_message(0, (hgt - 3) - j, strlen(msg), color, msg);

			/* Hilite "shower" */
			if (shower[0])
			{
				cptr str = msg;

				/* Display matches */
				while ((str = strstr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str - msg, (hgt - 3) - j, len, TERM_YELLOW, shower);

					/* Advance */
					str += len;
				}
			}
		}

		/* Display header XXX XXX XXX */
		prt(format("Message Recall (%d-%d of %d), Offset %d",
		           i, i + j - 1, n, q), 0, 0);

		/* Display prompt (not very informative) */
		prt("[Press 'p' for older, 'n' for newer, ..., or ESCAPE]", hgt - 1, 0);

		/* Get a command */
		k = inkey();

		/* Exit on Escape */
		if (k == ESCAPE) break;

		/* Hack -- Save the old index */
		j = i;

		/* Horizontal scroll */
		if (k == '4')
		{
			/* Scroll left */
			q = (q >= ((u32b)wid / 2)) ? (q - wid / 2) : 0;

			/* Success */
			continue;
		}

		/* Horizontal scroll */
		if (k == '6')
		{
			/* Scroll right */
			q = q + wid / 2;

			/* Success */
			continue;
		}

		/* Hack -- handle show */
		if (k == '=')
		{
			/* Prompt */
			prt("Show: ", hgt - 1, 0);

			/* Get a "shower" string, or continue */
			if (!askfor_aux(shower, 80)) continue;

			/* Okay */
			continue;
		}

		/* Hack -- handle find */
		if (k == '/')
		{
			s16b z;

			/* Prompt */
			prt("Find: ", hgt - 1, 0);

			/* Get a "finder" string, or continue */
			if (!askfor_aux(finder, 80)) continue;

			/* Show it */
			strcpy(shower, finder);

			/* Scan messages */
			for (z = i + 1; z < n; z++)
			{
				cptr msg = message_str(z);

				/* Search for it */
				if (strstr(msg, finder))
				{
					/* New location */
					i = z;

					/* Done */
					break;
				}
			}
		}

		/* Recall 1 older message */
		if ((k == '8') || (k == '\n') || (k == '\r'))
		{
			/* Go newer if legal */
			if (i + 1 < n) i += 1;
		}

		/* Recall 10 older messages */
		if (k == '+')
		{
			/* Go older if legal */
			if (i + 10 < n) i += 10;
		}

		/* Recall one screen of older messages */
		if ((k == 'p') || (k == KTRL('P')) || (k == ' '))
		{
			/* Go older if legal */
			if (i + (hgt - 4) < n) i += (hgt - 4);
		}

		/* Recall one screen of newer messages */
		if ((k == 'n') || (k == KTRL('N')))
		{
			/* Go newer (if able) */
			i = (i >= (hgt - 4)) ? (i - (hgt - 4)) : 0;
		}

		/* Recall 10 newer messages */
		if (k == '-')
		{
			/* Go newer (if able) */
			i = (i >= 10) ? (i - 10) : 0;
		}

		/* Recall 1 newer messages */
		if (k == '2')
		{
			/* Go newer (if able) */
			i = (i >= 1) ? (i - 1) : 0;
		}

		/* Hack -- Error of some kind */
		if (i == j) bell();
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}



/*
 * Number of cheating options
 */
#define CHEAT_MAX 6

/*
 * Cheating options
 */
static option_type cheat_info[CHEAT_MAX] =
{
	{ &cheat_peek, FALSE, 0, 0, "cheat_peek", "Peek into object creation" },
	{ &cheat_hear, FALSE, 0, 1, "cheat_hear", "Peek into monster creation" },
	{ &cheat_room, FALSE, 0, 2, "cheat_room", "Peek into dungeon creation" },
	{ &cheat_xtra, FALSE, 0, 3, "cheat_xtra", "Peek into something else" },
	{ &cheat_know, FALSE, 0, 4, "cheat_know", "Know complete monster info" },
	{ &cheat_live, FALSE, 0, 5, "cheat_live", "Allow player to avoid death" }
};

/*
 * Interact with some options for cheating
 */
static void do_cmd_options_cheat(cptr info)
{
	char ch;

	int	i, k = 0, n = CHEAT_MAX;

	int dir;

	char buf[80];


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		/* Prompt XXX XXX XXX */
		strnfmt(buf, 80, "%s (RET to advance, y/n to set, ESC to accept) ", info);
		prt(buf, 0, 0);

		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			strnfmt(buf, 80, "%-48s: %s  (%s)",
			        cheat_info[i].o_desc,
			        (*cheat_info[i].o_var ? "yes" : "no "),
			        cheat_info[i].o_text);
			c_prt(a, buf, i + 2, 0);
		}

		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/*
		 * Hack -- Try to translate the key into a direction
		 * to allow the use of roguelike keys for navigation
		 */
		dir = get_keymap_dir(ch);
		if ((dir == 2) || (dir == 4) || (dir == 6) || (dir == 8)) ch = I2D(dir);


		/* Analyze */
		switch (ch)
		{
		case ESCAPE:
			{
				return;
			}

		case '-':
		case '8':
			{
				k = (n + k - 1) % n;

				break;
			}

		case ' ':
		case '\n':
		case '\r':
		case '2':
			{
				k = (k + 1) % n;

				break;
			}

		case 'y':
		case 'Y':
		case '6':
			{
				noscore |= (cheat_info[k].o_page * 256 + cheat_info[k].o_bit);
				(*cheat_info[k].o_var) = TRUE;
				k = (k + 1) % n;

				break;
			}

		case 'n':
		case 'N':
		case '4':
			{
				(*cheat_info[k].o_var) = FALSE;
				k = (k + 1) % n;

				break;
			}

		default:
			{
				bell();

				break;
			}
		}
	}
}


static option_type autosave_info[2] =
{
	{ &autosave_l, FALSE, 0, 6, "autosave_l", "Autosave when entering new levels" },
	{ &autosave_t, FALSE, 0, 7, "autosave_t", "Timed autosave" },
};

s16b toggle_frequency(s16b current)
{
	if (current == 0) return (50);
	if (current == 50) return (100);
	if (current == 100) return (250);
	if (current == 250) return (500);
	if (current == 500) return (1000);
	if (current == 1000) return (2500);
	if (current == 2500) return (5000);
	if (current == 5000) return (10000);
	if (current == 10000) return (25000);

	return (0);
}


/*
 * Interact with some options for cheating
 */
static void do_cmd_options_autosave(cptr info)
{
	char ch;

	int i, k = 0, n = 2;

	int dir;

	char buf[80];


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		/* Prompt XXX XXX XXX */
		strnfmt(buf, 80,
		        "%s (RET to advance, y/n to set, 'F' for frequency, ESC to accept) ",
		        info);
		prt(buf, 0, 0);

		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			strnfmt(buf, 80, "%-48s: %s  (%s)",
			        autosave_info[i].o_desc,
			        (*autosave_info[i].o_var ? "yes" : "no "),
			        autosave_info[i].o_text);
			c_prt(a, buf, i + 2, 0);
		}

		prt(format("Timed autosave frequency: every %d turns", autosave_freq), 5, 0);


		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/*
		 * Hack -- Try to translate the key into a direction
		 * to allow the use of roguelike keys for navigation
		 */
		dir = get_keymap_dir(ch);
		if ((dir == 2) || (dir == 4) || (dir == 6) || (dir == 8)) ch = I2D(dir);

		/* Analyze */
		switch (ch)
		{
		case ESCAPE:
			{
				return;
			}

		case '-':
		case '8':
			{
				k = (n + k - 1) % n;

				break;
			}

		case ' ':
		case '\n':
		case '\r':
		case '2':
			{
				k = (k + 1) % n;

				break;
			}

		case 'y':
		case 'Y':
		case '6':
			{

				(*autosave_info[k].o_var) = TRUE;
				k = (k + 1) % n;

				break;
			}

		case 'n':
		case 'N':
		case '4':
			{
				(*autosave_info[k].o_var) = FALSE;
				k = (k + 1) % n;

				break;
			}

		case 'f':
		case 'F':
			{
				autosave_freq = toggle_frequency(autosave_freq);
				prt(format("Timed autosave frequency: every %d turns",
				           autosave_freq), 5, 0);

				break;
			}

		default:
			{
				bell();

				break;
			}
		}
	}
}

/* Switch an option by only knowing its name */
bool_ change_option(cptr name, bool_ value)
{
	int i;

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		if (!strcmp(option_info[i].o_text, name))
		{
			bool_ old = (*option_info[i].o_var);

			(*option_info[i].o_var) = value;

			return old;
		}
	}

	cmsg_format(TERM_VIOLET, "Warning, change_option couldn't find option '%s'.", name);
	return FALSE;
}

/*
 * Interact with some options
 */
void do_cmd_options_aux(int page, cptr info, bool_ read_only)
{
	char ch;

	int i, k = 0, n = 0;

	int dir;

	int	opt[24];

	char buf[80];


	/* Lookup the options */
	for (i = 0; i < 24; i++) opt[i] = 0;

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		/* Notice options on this "page" */
		if (option_info[i].o_page == page) opt[n++] = i;
	}


	/* Clear screen */
	Term_clear();

	/* Interact with the player */
	while (TRUE)
	{
		/* Prompt XXX XXX XXX */
		strnfmt(buf, 80, "%s (RET to advance, y/n to set, ESC to accept) ", info);
		prt(buf, 0, 0);

		/* Display the options */
		for (i = 0; i < n; i++)
		{
			byte a = TERM_WHITE;

			/* Color current option */
			if (i == k) a = TERM_L_BLUE;

			/* Display the option text */
			strnfmt(buf, 80, "%-48s: %s  (%s)",
			        option_info[opt[i]].o_desc,
			        (*option_info[opt[i]].o_var ? "yes" : "no "),
			        option_info[opt[i]].o_text);
			c_prt(a, buf, i + 2, 0);
		}

		/* Hilite current option */
		move_cursor(k + 2, 50);

		/* Get a key */
		ch = inkey();

		/*
		 * Hack -- Try to translate the key into a direction
		 * to allow the use of roguelike keys for navigation
		 */
		dir = get_keymap_dir(ch);
		if ((dir == 2) || (dir == 4) || (dir == 6) || (dir == 8)) ch = I2D(dir);

		/* Analyze */
		switch (ch)
		{
		case ESCAPE:
			{
				return;
			}

		case '-':
		case '8':
			{
				k = (n + k - 1) % n;

				break;
			}

		case ' ':
		case '\n':
		case '\r':
		case '2':
			{
				k = (k + 1) % n;

				break;
			}

		case 'y':
		case 'Y':
		case '6':
			{
				if (read_only) break;

				(*option_info[opt[k]].o_var) = TRUE;
				k = (k + 1) % n;

				break;
			}

		case 'n':
		case 'N':
		case '4':
			{
				if (read_only) break;

				(*option_info[opt[k]].o_var) = FALSE;
				k = (k + 1) % n;

				break;
			}

		default:
			{
				bell();

				break;
			}
		}
	}
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
	int i, j, d;

	int y = 0;

	int x = 0;

	char ch;

	bool_ go = TRUE;

	u32b old_flag[8];


	/* Memorize old flags */
	for (j = 0; j < 8; j++)
	{
		/* Acquire current flags */
		old_flag[j] = window_flag[j];
	}


	/* Clear screen */
	Term_clear();

	/* Interact */
	while (go)
	{
		/* Prompt XXX XXX XXX */
		prt("Window Flags (<dir>, t, y, n, ESC) ", 0, 0);

		/* Display the windows */
		for (j = 0; j < 8; j++)
		{
			byte a = TERM_WHITE;

			cptr s = angband_term_name[j];

			/* Use color */
			if (j == x) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < 16; i++)
		{
			byte a = TERM_WHITE;

			cptr str = window_flag_desc[i];

			/* Use color */
			if (i == y) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < 8; j++)
			{
				byte a = TERM_WHITE;

				char c = '.';

				/* Use color */
				if ((i == y) && (j == x)) a = TERM_L_BLUE;

				/* Active flag */
				if (window_flag[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ch = inkey();

		/* Analyze */
		switch (ch)
		{
		case ESCAPE:
			{
				go = FALSE;

				break;
			}

		case 'T':
		case 't':
			{
				/* Clear windows */
				for (j = 0; j < 8; j++)
				{
					window_flag[j] &= ~(1L << y);
				}

				/* Clear flags */
				for (i = 0; i < 16; i++)
				{
					window_flag[x] &= ~(1L << i);
				}

				/* Fall through */
			}

		case 'y':
		case 'Y':
			{
				/* Ignore screen */
				if (x == 0) break;

				/* Set flag */
				window_flag[x] |= (1L << y);

				break;
			}

		case 'n':
		case 'N':
			{
				/* Clear flag */
				window_flag[x] &= ~(1L << y);

				break;
			}

		default:
			{
				d = get_keymap_dir(ch);

				x = (x + ddx[d] + 8) % 8;
				y = (y + ddy[d] + 16) % 16;

				if (!d) bell();

				break;
			}
		}
	}

	/* Notice changes */
	for (j = 0; j < 8; j++)
	{
		term *old = Term;

		/* Dead window */
		if (!angband_term[j]) continue;

		/* Ignore non-changes */
		if (window_flag[j] == old_flag[j]) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Erase */
		Term_clear();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Write all current options to the given preference file in the
 * lib/user directory. Modified from KAmband 1.8.
 */
static errr option_dump(cptr fname)
{
	int i, j;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return ( -1);


	/* Skip some lines */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic option dump\n\n");

	/* Dump options (skip cheat, adult, score) */
	for (i = 0; option_info[i].o_var != NULL; i++)
	{
		/* Require a real option */
		if (!option_info[i].o_text) continue;

		/* No birth options */
		if (option_info[i].o_page == 6) continue;

		/* Comment */
		fprintf(fff, "# Option '%s'\n", option_info[i].o_desc);

		/* Dump the option */
		if ((*option_info[i].o_var))
		{
			fprintf(fff, "Y:%s\n", option_info[i].o_text);
		}
		else
		{
			fprintf(fff, "X:%s\n", option_info[i].o_text);
		}

		/* Skip a line */
		fprintf(fff, "\n");
	}

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < 32; j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			fprintf(fff, "# Window '%s', Flag '%s'\n",
			        angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (window_flag[i] & (1L << j))
			{
				fprintf(fff, "W:%d:%d:1\n", i, j);
			}
			else
			{
				fprintf(fff, "W:%d:%d:0\n", i, j);
			}

			/* Skip a line */
			fprintf(fff, "\n");
		}
	}

	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


/*
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static void do_cmd_pref_file_hack(int row)
{
	char ftmp[80];


	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	strnfmt(ftmp, 80, "%s.prf", player_base);

	/* Ask for a file (or cancel) */
	if (!askfor_aux(ftmp, 80)) return;

	/* Process the given filename */
	if (process_pref_file(ftmp))
	{
		/* Mention failure */
		msg_format("Failed to load '%s'!", ftmp);
	}
	else
	{
		/* Mention success */
		msg_format("Loaded '%s'.", ftmp);
	}
}


/*
 * Set or unset various options.
 *
 * The user must use the "Ctrl-R" command to "adapt" to changes
 * in any options which control "visual" aspects of the game.
 */
void do_cmd_options(void)
{
	int k;


	/* Save the screen */
	screen_save();

	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Why are we here */
		prt("Options", 2, 0);

		/* Give some choices */
		prt("(1) User Interface Options", 4, 5);
		prt("(2) Disturbance Options", 5, 5);
		prt("(3) Game-Play Options", 6, 5);
		prt("(4) Efficiency Options", 7, 5);
		prt("(5) ToME Options", 8, 5);
		prt("(6) Birth Options(read only)", 9, 5);

		/* Special choices */
		prt("(D) Base Delay Factor", 10, 5);
		prt("(H) Hitpoint Warning", 11, 5);
		prt("(A) Autosave Options", 12, 5);

		/* Automatizer */
		prt("(T) Automatizer", 14, 5);


		/* Window flags */
		prt("(W) Window Flags", 16, 5);

		/* Cheating */
		prt("(C) Cheating Options", 18, 5);

		/* Dump */
		prt("(U) Dump Options setting", 20, 5);
		prt("(O) Load Options setting", 21, 5);

		/* Prompt */
		prt("Command: ", 22, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE) break;

		/* Analyze */
		switch (k)
		{
			/* Load a user pref file */
		case 'o':
		case 'O':
			{
				/* Ask for and load a user pref file */
				do_cmd_pref_file_hack(21);

				break;
			}

			/* Append options to a file */
		case 'u':
		case 'U':
			{
				char ftmp[80];

				/* Prompt */
				prt("Command: Append options to a file", 21, 0);

				/* Prompt */
				prt("File: ", 21, 0);

				/* Default filename */
				strnfmt(ftmp, 80, "%s.prf", player_base);

				/* Ask for a file */
				if (!askfor_aux(ftmp, 80)) continue;

				/* Dump the options */
				if (option_dump(ftmp))
				{
					/* Failure */
					msg_print("Failed!");
				}
				else
				{
					/* Success */
					msg_print("Done.");
				}

				break;
			}

			/* General Options */
		case '1':
			{
				/* Process the general options */
				do_cmd_options_aux(1, "User Interface Options", FALSE);

				break;
			}

			/* Disturbance Options */
		case '2':
			{
				/* Spawn */
				do_cmd_options_aux(2, "Disturbance Options", FALSE);

				break;
			}

			/* Inventory Options */
		case '3':
			{
				/* Spawn */
				do_cmd_options_aux(3, "Game-Play Options", FALSE);

				break;
			}

			/* Efficiency Options */
		case '4':
			{
				/* Spawn */
				do_cmd_options_aux(4, "Efficiency Options", FALSE);

				break;
			}

			/* ToME Options */
		case '5':
			{
				do_cmd_options_aux(5, "ToME Options", FALSE);

				break;
			}

			/* Birth Options - read only */
		case '6':
			{
				do_cmd_options_aux(6, "Birth Options(read only)", TRUE);

				break;
			}
			/* Cheating Options */
		case 'C':
			{
				/* Spawn */
				do_cmd_options_cheat("Cheaters never win");

				break;
			}

		case 't':
		case 'T':
			{
				do_cmd_automatizer();
				break;
			}

		case 'a':
		case 'A':
			{
				do_cmd_options_autosave("Autosave");

				break;
			}

			/* Window flags */
		case 'W':
		case 'w':
			{
				/* Spawn */
				do_cmd_options_win();

				break;
			}

			/* Hack -- Delay Speed */
		case 'D':
		case 'd':
			{
				/* Prompt */
				prt("Command: Base Delay Factor", 21, 0);

				/* Get a new value */
				while (1)
				{
					int msec = delay_factor * delay_factor * delay_factor;
					prt(format("Current base delay factor: %d (%d msec)",
					           delay_factor, msec), 22, 0);
					prt("Delay Factor (0-9 or ESC to accept): ", 23, 0);
					k = inkey();
					if (k == ESCAPE) break;
					if (isdigit(k)) delay_factor = D2I(k);
					else bell();
				}

				break;
			}

			/* Hack -- hitpoint warning factor */
		case 'H':
		case 'h':
			{
				/* Prompt */
				prt("Command: Hitpoint Warning", 18, 0);

				/* Get a new value */
				while (1)
				{
					prt(format("Current hitpoint warning: %d0%%",
					           hitpoint_warn), 22, 0);
					prt("Hitpoint Warning (0-9 or ESC to accept): ", 20, 0);
					k = inkey();
					if (k == ESCAPE) break;
					if (isdigit(k)) hitpoint_warn = D2I(k);
					else bell();
				}

				break;
			}

			/* Unknown option */
		default:
			{
				/* Oops */
				bell();

				break;
			}
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	screen_load();
}



/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 */
void do_cmd_pref(void)
{
	char buf[80];


	/* Default */
	strcpy(buf, "");

	/* Ask for a "user pref command" */
	if (!get_string("Pref: ", buf, 80)) return;

	/* Process that pref command */
	(void)process_pref_file_aux(buf);
}


/*
 * Hack -- append all current macros to the given file
 */
static errr macro_dump(cptr fname)
{
	int i;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return ( -1);


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic macro dump\n\n");

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		fprintf(fff, "# Macro '%d'\n\n", i);

		/* Extract the action */
		ascii_to_text(buf, macro__act[i]);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);

		/* Extract the action */
		ascii_to_text(buf, macro__pat[i]);

		/* Dump normal macros */
		fprintf(fff, "P:%s\n", buf);

		/* End the macro */
		fprintf(fff, "\n\n");
	}

	/* Start dumping */
	fprintf(fff, "\n\n\n\n");


	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}


/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "util.c".
 *
 * Note that both "flush()" calls are extremely important.
 */
static void do_cmd_macro_aux(char *buf, bool_ macro_screen)
{
	int i, n = 0;

	char tmp[1024];


	/* Flush */
	flush();

	/* Do not process macros */
	inkey_base = TRUE;

	/* First key */
	i = inkey();

	/* Read the pattern */
	while (i)
	{
		/* Save the key */
		buf[n++] = i;

		/* Do not process macros */
		inkey_base = TRUE;

		/* Do not wait for keys */
		inkey_scan = TRUE;

		/* Attempt to read a key */
		i = inkey();
	}

	/* Terminate */
	buf[n] = '\0';

	/* Flush */
	flush();


	if (macro_screen)
	{
		/* Convert the trigger */
		ascii_to_text(tmp, buf);

		/* Hack -- display the trigger */
		Term_addstr( -1, TERM_WHITE, tmp);
	}
}

/*
 * Hack -- ask for a keymap "trigger" (see below)
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux_keymap(char *buf)
{
	char tmp[1024];


	/* Flush */
	flush();


	/* Get a key */
	buf[0] = inkey();
	buf[1] = '\0';


	/* Convert to ascii */
	ascii_to_text(tmp, buf);

	/* Hack -- display the trigger */
	Term_addstr( -1, TERM_WHITE, tmp);


	/* Flush */
	flush();
}


/*
 * Hack -- append all keymaps to the given file
 */
static errr keymap_dump(cptr fname)
{
	int i;

	FILE *fff;

	char key[1024];
	char buf[1024];

	int mode;


	/* Keymap mode */
	mode = get_keymap_mode();

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Append to the file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return ( -1);


	/* Skip space */
	fprintf(fff, "\n\n");

	/* Start dumping */
	fprintf(fff, "# Automatic keymap dump\n\n");

	/* Dump them */
	for (i = 0; i < 256; i++)
	{
		cptr act;

		/* Loop up the keymap */
		act = keymap_act[mode][i];

		/* Skip empty keymaps */
		if (!act) continue;

		/* Encode the key */
		buf[0] = i;
		buf[1] = '\0';
		ascii_to_text(key, buf);

		/* Encode the action */
		ascii_to_text(buf, act);

		/* Dump the macro */
		fprintf(fff, "A:%s\n", buf);
		fprintf(fff, "C:%d:%s\n", mode, key);
	}

	/* Start dumping */
	fprintf(fff, "\n\n\n");


	/* Close */
	my_fclose(fff);

	/* Success */
	return (0);
}



/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * Could use some helpful instructions on this page.  XXX XXX XXX
 */
void do_cmd_macros(void)
{
	int i;

	char tmp[1024];

	char buf[1024];

	int mode;


	/* Keymap mode */
	mode = get_keymap_mode();

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save screen */
	Term_save();


	/* Process requests until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Describe */
		prt("Interact with Macros", 2, 0);


		/* Describe that action */
		prt("Current action (if any) shown below:", 20, 0);

		/* Analyze the current action */
		ascii_to_text(buf, macro__buf);

		/* Display the current action */
		prt(buf, 22, 0);


		/* Selections */
		prt("(1) Load a user pref file", 4, 5);
		prt("(2) Append macros to a file", 5, 5);
		prt("(3) Query a macro", 6, 5);
		prt("(4) Create a macro", 7, 5);
		prt("(5) Remove a macro", 8, 5);
		prt("(6) Append keymaps to a file", 9, 5);
		prt("(7) Query a keymap", 10, 5);
		prt("(8) Create a keymap", 11, 5);
		prt("(9) Remove a keymap", 12, 5);
		prt("(0) Enter a new action", 13, 5);

		/* Prompt */
		prt("Command: ", 16, 0);

		/* Get a command */
		i = inkey();

		/* Leave */
		if (i == ESCAPE) break;

		/* Load a 'macro' file */
		else if (i == '1')
		{
			/* Prompt */
			prt("Command: Load a user pref file", 16, 0);

			/* Prompt */
			prt("File: ", 18, 0);

			/* Default filename */
			strnfmt(tmp, 1024, "%s.prf", player_name);

			/* Ask for a file */
			if (!askfor_aux(tmp, 80)) continue;

			/* Process the given filename */
			if (0 != process_pref_file(tmp))
			{
				/* Prompt */
				msg_print("Could not load file!");
			}
		}

		/* Save macros */
		else if (i == '2')
		{
			/* Prompt */
			prt("Command: Append macros to a file", 16, 0);

			/* Prompt */
			prt("File: ", 18, 0);

			/* Default filename */
			strnfmt(tmp, 1024, "%s.prf", player_name);

			/* Ask for a file */
			if (!askfor_aux(tmp, 80)) continue;

			/* Dump the macros */
			(void)macro_dump(tmp);

			/* Prompt */
			msg_print("Appended macros.");
		}

		/* Query a macro */
		else if (i == '3')
		{
			int k;

			/* Prompt */
			prt("Command: Query a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(buf, TRUE);

			/* Acquire action */
			k = macro_find_exact(buf);

			/* Nothing found */
			if (k < 0)
			{
				/* Prompt */
				msg_print("Found no macro.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				strcpy(macro__buf, macro__act[k]);

				/* Analyze the current action */
				ascii_to_text(buf, macro__buf);

				/* Display the current action */
				prt(buf, 22, 0);

				/* Prompt */
				msg_print("Found a macro.");
			}
		}

		/* Create a macro */
		else if (i == '4')
		{
			/* Prompt */
			prt("Command: Create a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(buf, TRUE);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, macro__buf);

			/* Get an encoded action */
			if (askfor_aux(tmp, 80))
			{
				/* Convert to ascii */
				text_to_ascii(macro__buf, tmp);

				/* Link the macro */
				macro_add(buf, macro__buf);

				/* Prompt */
				msg_print("Added a macro.");
			}
		}

		/* Remove a macro */
		else if (i == '5')
		{
			/* Prompt */
			prt("Command: Remove a macro", 16, 0);

			/* Prompt */
			prt("Trigger: ", 18, 0);

			/* Get a macro trigger */
			do_cmd_macro_aux(buf, TRUE);

			/* Link the macro */
			macro_add(buf, buf);

			/* Prompt */
			msg_print("Removed a macro.");
		}

		/* Save keymaps */
		else if (i == '6')
		{
			/* Prompt */
			prt("Command: Append keymaps to a file", 16, 0);

			/* Prompt */
			prt("File: ", 18, 0);

			/* Default filename */
			strnfmt(tmp, 1024, "%s.prf", player_name);

			/* Ask for a file */
			if (!askfor_aux(tmp, 80)) continue;

			/* Dump the macros */
			(void)keymap_dump(tmp);

			/* Prompt */
			msg_print("Appended keymaps.");
		}

		/* Query a keymap */
		else if (i == '7')
		{
			cptr act;

			/* Prompt */
			prt("Command: Query a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(buf);

			/* Look up the keymap */
			act = keymap_act[mode][(byte)(buf[0])];

			/* Nothing found */
			if (!act)
			{
				/* Prompt */
				msg_print("Found no keymap.");
			}

			/* Found one */
			else
			{
				/* Obtain the action */
				strcpy(macro__buf, act);

				/* Analyze the current action */
				ascii_to_text(buf, macro__buf);

				/* Display the current action */
				prt(buf, 22, 0);

				/* Prompt */
				msg_print("Found a keymap.");
			}
		}

		/* Create a keymap */
		else if (i == '8')
		{
			/* Prompt */
			prt("Command: Create a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(buf);

			/* Clear */
			clear_from(20);

			/* Prompt */
			prt("Action: ", 20, 0);

			/* Convert to text */
			ascii_to_text(tmp, macro__buf);

			/* Get an encoded action */
			if (askfor_aux(tmp, 80))
			{
				/* Convert to ascii */
				text_to_ascii(macro__buf, tmp);

				/* Free old keymap */
				string_free(keymap_act[mode][(byte)(buf[0])]);

				/* Make new keymap */
				keymap_act[mode][(byte)(buf[0])] = string_make(macro__buf);

				/* Prompt */
				msg_print("Added a keymap.");
			}
		}

		/* Remove a keymap */
		else if (i == '9')
		{
			/* Prompt */
			prt("Command: Remove a keymap", 16, 0);

			/* Prompt */
			prt("Keypress: ", 18, 0);

			/* Get a keymap trigger */
			do_cmd_macro_aux_keymap(buf);

			/* Free old keymap */
			string_free(keymap_act[mode][(byte)(buf[0])]);

			/* Make new keymap */
			keymap_act[mode][(byte)(buf[0])] = NULL;

			/* Prompt */
			msg_print("Removed a keymap.");
		}

		/* Enter a new action */
		else if (i == '0')
		{
			/* Prompt */
			prt("Command: Enter a new action", 16, 0);

			/* Go to the correct location */
			Term_gotoxy(0, 22);

			/* Hack -- limit the value */
			tmp[80] = '\0';

			/* Get an encoded action */
			if (!askfor_aux(buf, 80)) continue;

			/* Extract an action */
			text_to_ascii(macro__buf, buf);
		}

		/* Oops */
		else
		{
			/* Oops */
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}

	/* Load screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
	int i;

	FILE *fff;

	char tmp[160];

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Interact with Visuals", 2, 0);

		/* Give some choices */
		prt("(1) Load a user pref file", 4, 5);
		prt("(2) Dump monster attr/chars", 5, 5);
		prt("(3) Dump object attr/chars", 6, 5);
		prt("(4) Dump feature attr/chars", 7, 5);
		prt("(5) (unused)", 8, 5);
		prt("(6) Change monster attr/chars", 9, 5);
		prt("(7) Change object attr/chars", 10, 5);
		prt("(8) Change feature attr/chars", 11, 5);
		prt("(9) (unused)", 12, 5);
		prt("(0) Reset visuals", 13, 5);

		/* Prompt */
		prt("Command: ", 15, 0);

		/* Prompt */
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		/* Load a 'pref' file */
		else if (i == '1')
		{
			/* Prompt */
			prt("Command: Load a user pref file", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Query */
			if (!askfor_aux(tmp, 70)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);
		}

		/* Dump monster attr/chars */
		else if (i == '2')
		{
			/* Prompt */
			prt("Command: Dump monster attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Monster attr/char definitions\n\n");

			/* Dump monsters */
			for (i = 0; i < max_r_idx; i++)
			{
				monster_race *r_ptr = &r_info[i];

				/* Skip non-entries */
				if (!r_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (r_name + r_ptr->name));

				/* Dump the monster attr/char info */
				fprintf(fff, "R:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(r_ptr->x_attr), (byte)(r_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped monster attr/chars.");
		}

		/* Dump object attr/chars */
		else if (i == '3')
		{
			/* Prompt */
			prt("Command: Dump object attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Object attr/char definitions\n\n");

			/* Dump objects */
			for (i = 0; i < max_k_idx; i++)
			{
				object_kind *k_ptr = &k_info[i];

				/* Skip non-entries */
				if (!k_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (k_name + k_ptr->name));

				/* Dump the object attr/char info */
				fprintf(fff, "K:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped object attr/chars.");
		}

		/* Dump feature attr/chars */
		else if (i == '4')
		{
			/* Prompt */
			prt("Command: Dump feature attr/chars", 15, 0);

			/* Prompt */
			prt("File: ", 17, 0);

			/* Default filename */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Feature attr/char definitions\n\n");

			/* Dump features */
			for (i = 0; i < max_f_idx; i++)
			{
				feature_type *f_ptr = &f_info[i];

				/* Skip non-entries */
				if (!f_ptr->name) continue;

				/* Dump a comment */
				fprintf(fff, "# %s\n", (f_name + f_ptr->name));

				/* Dump the feature attr/char info */
				fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
				        (byte)(f_ptr->x_attr), (byte)(f_ptr->x_char));
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped feature attr/chars.");
		}

		/* Modify monster attr/chars */
		else if (i == '6')
		{
			static int r = 0;

			/* Prompt */
			prt("Command: Change monster attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				monster_race *r_ptr = &r_info[r];

				byte da = (r_ptr->d_attr);
				char dc = (r_ptr->d_char);
				byte ca = (r_ptr->x_attr);
				char cc = (r_ptr->x_char);

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Monster = %d, Name = %-40.40s",
				                   r, (r_name + r_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3u / %3u", da, (dc & 0xFF)));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);
				if (use_bigtile)
				{
					if (da & 0x80)
						Term_putch(44, 19, 255, 255);
					else
						Term_putch(44, 19, 0, ' ');
				}

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3u / %3u", ca, (cc & 0xFF)));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);
				if (use_bigtile)
				{
					if (ca & 0x80)
						Term_putch(44, 20, 255, 255);
					else
						Term_putch(44, 20, 0, ' ');
				}

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C): ");

				/* Get a command */
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') r = (r + max_r_idx + 1) % max_r_idx;
				if (i == 'N') r = (r + max_r_idx - 1) % max_r_idx;
				if (i == 'a') r_ptr->x_attr = (byte)(ca + 1);
				if (i == 'A') r_ptr->x_attr = (byte)(ca - 1);
				if (i == 'c') r_ptr->x_char = (byte)(cc + 1);
				if (i == 'C') r_ptr->x_char = (byte)(cc - 1);
			}
		}

		/* Modify object attr/chars */
		else if (i == '7')
		{
			static int k = 0;

			/* Prompt */
			prt("Command: Change object attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				object_kind *k_ptr = &k_info[k];

				byte da = (byte)k_ptr->d_attr;
				char dc = (byte)k_ptr->d_char;
				byte ca = (byte)k_ptr->x_attr;
				char cc = (byte)k_ptr->x_char;

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Object = %d, Name = %-40.40s",
				                   k, (k_name + k_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3u / %3u", da, (dc & 0xFF)));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);
				if (use_bigtile)
				{
					if (da & 0x80)
						Term_putch(44, 19, 255, 255);
					else
						Term_putch(44, 19, 0, ' ');
				}

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3u / %3u", ca, (cc & 0xFF)));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);
				if (use_bigtile)
				{
					if (ca & 0x80)
						Term_putch(44, 20, 255, 255);
					else
						Term_putch(44, 20, 0, ' ');
				}

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C): ");

				/* Get a command */
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') k = (k + max_k_idx + 1) % max_k_idx;
				if (i == 'N') k = (k + max_k_idx - 1) % max_k_idx;
				if (i == 'a') k_info[k].x_attr = (byte)(ca + 1);
				if (i == 'A') k_info[k].x_attr = (byte)(ca - 1);
				if (i == 'c') k_info[k].x_char = (byte)(cc + 1);
				if (i == 'C') k_info[k].x_char = (byte)(cc - 1);
			}
		}

		/* Modify feature attr/chars */
		else if (i == '8')
		{
			static int f = 0;

			/* Prompt */
			prt("Command: Change feature attr/chars", 15, 0);

			/* Hack -- query until done */
			while (1)
			{
				feature_type *f_ptr = &f_info[f];

				byte da = (byte)f_ptr->d_attr;
				char dc = (byte)f_ptr->d_char;
				byte ca = (byte)f_ptr->x_attr;
				char cc = (byte)f_ptr->x_char;

				/* Label the object */
				Term_putstr(5, 17, -1, TERM_WHITE,
				            format("Terrain = %d, Name = %-40.40s",
				                   f, (f_name + f_ptr->name)));

				/* Label the Default values */
				Term_putstr(10, 19, -1, TERM_WHITE,
				            format("Default attr/char = %3u / %3u", da, (dc & 0xFF)));
				Term_putstr(40, 19, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 19, da, dc);
				if (use_bigtile)
				{
					if (da & 0x80)
						Term_putch(44, 19, 255, 255);
					else
						Term_putch(44, 19, 0, ' ');
				}

				/* Label the Current values */
				Term_putstr(10, 20, -1, TERM_WHITE,
				            format("Current attr/char = %3u / %3u", ca, (cc & 0xFF)));
				Term_putstr(40, 20, -1, TERM_WHITE, "<< ? >>");
				Term_putch(43, 20, ca, cc);
				if (use_bigtile)
				{
					if (ca & 0x80)
						Term_putch(44, 20, 255, 255);
					else
						Term_putch(44, 20, 0, ' ');
				}

				/* Prompt */
				Term_putstr(0, 22, -1, TERM_WHITE,
				            "Command (n/N/a/A/c/C/d): ");

				/* Get a command */
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') f = (f + max_f_idx + 1) % max_f_idx;
				if (i == 'N') f = (f + max_f_idx - 1) % max_f_idx;
				if (i == 'a') f_info[f].x_attr = (byte)(ca + 1);
				if (i == 'A') f_info[f].x_attr = (byte)(ca - 1);
				if (i == 'c') f_info[f].x_char = (byte)(cc + 1);
				if (i == 'C') f_info[f].x_char = (byte)(cc - 1);
				if (i == 'd')
				{
					f_info[f].x_char = f_ptr->d_char;
					f_info[f].x_attr = f_ptr->d_attr;
				}
			}
		}

		/* Reset visuals */
		else if (i == '0')
		{
			/* Reset */
			reset_visuals();

			/* Message */
			msg_print("Visual attr/char tables reset.");
		}

		/* Unknown option */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
	int i;

	FILE *fff;

	char tmp[160];

	char buf[1024];


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();


	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Interact with Colors", 2, 0);

		/* Give some choices */
		prt("(1) Load a user pref file", 4, 5);
		prt("(2) Dump colors", 5, 5);
		prt("(3) Modify colors", 6, 5);

		/* Prompt */
		prt("Command: ", 8, 0);

		/* Prompt */
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		/* Load a 'pref' file */
		if (i == '1')
		{
			/* Prompt */
			prt("Command: Load a user pref file", 8, 0);

			/* Prompt */
			prt("File: ", 10, 0);

			/* Default file */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Query */
			if (!askfor_aux(tmp, 70)) continue;

			/* Process the given filename */
			(void)process_pref_file(tmp);

			/* Mega-Hack -- react to changes */
			Term_xtra(TERM_XTRA_REACT, 0);

			/* Mega-Hack -- redraw */
			Term_redraw();
		}

		/* Dump colors */
		else if (i == '2')
		{
			/* Prompt */
			prt("Command: Dump colors", 8, 0);

			/* Prompt */
			prt("File: ", 10, 0);

			/* Default filename */
			strnfmt(tmp, 160, "user-%s.prf", ANGBAND_SYS);

			/* Get a filename */
			if (!askfor_aux(tmp, 70)) continue;

			/* Build the filename */
			path_build(buf, 1024, ANGBAND_DIR_USER, tmp);

			/* Append to the file */
			fff = my_fopen(buf, "a");

			/* Failure */
			if (!fff) continue;

			/* Start dumping */
			fprintf(fff, "\n\n");
			fprintf(fff, "# Color redefinitions\n\n");

			/* Dump colors */
			for (i = 0; i < 256; i++)
			{
				int kv = angband_color_table[i][0];
				int rv = angband_color_table[i][1];
				int gv = angband_color_table[i][2];
				int bv = angband_color_table[i][3];

				cptr name = "unknown";

				/* Skip non-entries */
				if (!kv && !rv && !gv && !bv) continue;

				/* Extract the color name */
				if (i < 16) name = color_names[i];

				/* Dump a comment */
				fprintf(fff, "# Color '%s'\n", name);

				/* Dump the monster attr/char info */
				fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
				        i, kv, rv, gv, bv);
			}

			/* All done */
			fprintf(fff, "\n\n\n\n");

			/* Close */
			my_fclose(fff);

			/* Message */
			msg_print("Dumped color redefinitions.");
		}

		/* Edit colors */
		else if (i == '3')
		{
			static byte a = 0;

			/* Prompt */
			prt("Command: Modify colors", 8, 0);

			/* Hack -- query until done */
			while (1)
			{
				cptr name;

				/* Clear */
				clear_from(10);

				/* Exhibit the normal colors */
				for (i = 0; i < 16; i++)
				{
					/* Exhibit this color */
					Term_putstr(i*4, 20, -1, a, "###");

					/* Exhibit all colors */
					Term_putstr(i*4, 22, -1, i, format("%3d", i));
				}

				/* Describe the color */
				name = ((a < 16) ? color_names[a] : "undefined");

				/* Describe the color */
				Term_putstr(5, 10, -1, TERM_WHITE,
				            format("Color = %d, Name = %s", a, name));

				/* Label the Current values */
				Term_putstr(5, 12, -1, TERM_WHITE,
				            format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
				                   angband_color_table[a][0],
				                   angband_color_table[a][1],
				                   angband_color_table[a][2],
				                   angband_color_table[a][3]));

				/* Prompt */
				Term_putstr(0, 14, -1, TERM_WHITE,
				            "Command (n/N/k/K/r/R/g/G/b/B): ");

				/* Get a command */
				i = inkey();

				/* All done */
				if (i == ESCAPE) break;

				/* Analyze */
				if (i == 'n') a = (byte)(a + 1);
				if (i == 'N') a = (byte)(a - 1);
				if (i == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
				if (i == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
				if (i == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
				if (i == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
				if (i == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
				if (i == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
				if (i == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
				if (i == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

				/* Hack -- react to changes */
				Term_xtra(TERM_XTRA_REACT, 0);

				/* Hack -- redraw */
				Term_redraw();
			}
		}

		/* Unknown option */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Take notes.  There are two ways this can happen, either in the message
 * recall or a file.
 */
void do_cmd_note(void)
{
	char buf[80];


	/* Default */
	strcpy(buf, "");

	if (!get_string("Note: ", buf, 60)) return;

	/* Ignore empty notes */
	if (!buf[0] || (buf[0] == ' ')) return;

	if (take_notes)
	{
		/* Add note to file */
		add_note(buf, ' ');
	}
	else
	{
		/* Add note to message recall */
		msg_format("Note: %s", buf);
	}
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
	/* Silly message */
	msg_format("You are playing %s made by %s (%s).",
	           get_version_string(),
	           modules[game_module_idx].meta.author.name,
		   modules[game_module_idx].meta.author.email);
}



/*
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
{
	"Looks like any other level.",
	"You feel there is something special about this level.",
	"You have a superb feeling about this level.",
	"You have an excellent feeling...",
	"You have a very good feeling...",
	"You have a good feeling...",
	"You feel strangely lucky...",
	"You feel your luck is turning...",
	"You like the look of this place...",
	"This level can't be all bad...",
	"What a boring place..."
};


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
	/* Verify the feeling */
	if (feeling < 0) feeling = 0;
	if (feeling > 10) feeling = 10;

	/* Feeling of the fate */
	if (fate_flag && !(dungeon_flags2 & DF2_SPECIAL) && !p_ptr->inside_quest)
	{
		msg_print("You feel that you will meet your fate here.");
	}

	/* Hooked feelings ? */
	if (process_hooks(HOOK_FEELING, "(d)", is_quest(dun_level)))
	{
		return;
	}

	/* No useful feeling in special levels */
	if (dungeon_flags2 & DF2_DESC)
	{
		char buf[1024];

		if ((get_dungeon_save(buf)) || (generate_special_feeling) || (dungeon_flags2 & DF2_DESC_ALWAYS))
		{
			if (!get_level_desc(buf)) msg_print("Someone forgot to describe this level!");
			else msg_print(buf);
			return;
		}
	}

	/* No useful feeling in quests */
	if (p_ptr->inside_quest)
	{
		msg_print("Looks like a typical quest level.");
		return;
	}

	/* Display feelings in the dungeon, nothing on the surface */
	if (dun_level)
	{
		/* This could be simplified with a correct p_ptr->town_num */
		int i, town_level = 0;
		dungeon_info_type *d_ptr = &d_info[dungeon_type];

		/* Is it a town level ? */
		for (i = 0; i < TOWN_DUNGEON; i++)
		{
			if (d_ptr->t_level[i] == dun_level) town_level = d_ptr->t_idx[i];
		}

		if (town_level)
			msg_print("You hear the sound of a market.");
		else
			msg_print(do_cmd_feeling_text[feeling]);
	}
	return;
}



/*
 * Encode the screen colors
 */
static char hack[17] = "dwsorgbuDWvyRGBU";


/*
 * Hack -- load a screen dump from a file
 */
void do_cmd_load_screen(void)
{
	int i, y, x;

	int wid, hgt;
	int len;

	byte a = 0;
	char c = ' ';

	bool_ okay = TRUE;

	FILE *fff;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

	/* Append to the file */
	fff = my_fopen(buf, "r");

	/* Oops */
	if (!fff) return;


	/* Retrieve the current screen size */
	Term_get_size(&wid, &hgt);

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Clear the screen */
	Term_clear();


	/* Load the screen */
	for (y = 0; okay; y++)
	{
		/* Get a line of data */
		if (my_fgets(fff, buf, 1024)) okay = FALSE;

		/* Stop on blank line */
		if (!buf[0]) break;

		/* Ignore off screen lines */
		if (y >= hgt) continue;

		/* Get width */
		len = strlen(buf);

		/* Truncate if it's longer than current screen width */
		if (len > wid) len = wid;

		/* Show each row */
		for (x = 0; x < len; x++)
		{
			/* Put the attr/char */
			Term_draw(x, y, TERM_WHITE, buf[x]);
		}
	}

	/* Dump the screen */
	for (y = 0; okay; y++)
	{
		/* Get a line of data */
		if (my_fgets(fff, buf, 1024)) okay = FALSE;

		/* Stop on blank line */
		if (!buf[0]) break;

		/* Ignore off screen lines */
		if (y >= hgt) continue;

		/* Get width */
		len = strlen(buf);

		/* Truncate if it's longer than current screen width */
		if (len > wid) len = wid;

		/* Dump each row */
		for (x = 0; x < len; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

			/* Look up the attr */
			for (i = 0; i < 16; i++)
			{
				/* Use attr matches */
				if (hack[i] == buf[x]) a = i;
			}

			/* Put the attr/char */
			Term_draw(x, y, a, c);
		}
	}


	/* Close it */
	my_fclose(fff);


	/* Message */
	msg_print("Screen dump loaded.");
	msg_print(NULL);


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}



/*
 * Redefinable "save_screen" action
 */
void (*screendump_aux)(void) = NULL;






/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
	/* Do we use a special screendump function ? */
	if (screendump_aux)
	{
		/* Dump the screen to a graphics file */
		(*screendump_aux)();
	}

	/* Dump the screen as text */
	else
	{
		int y, x;
		int wid, hgt;

		byte a = 0;
		char c = ' ';

		FILE *fff;

		char buf[1024];


		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_USER, "dump.txt");

		/* File type is "TEXT" */
		FILE_TYPE(FILE_TYPE_TEXT);

		/* Append to the file */
		fff = my_fopen(buf, "w");

		/* Oops */
		if (!fff) return;


		/* Retrieve the current screen size */
		Term_get_size(&wid, &hgt);

		/* Enter "icky" mode */
		character_icky = TRUE;

		/* Save the screen */
		Term_save();


		/* Dump the screen */
		for (y = 0; y < hgt; y++)
		{
			/* Dump each row */
			for (x = 0; x < wid; x++)
			{
				/* Get the attr/char */
				(void)(Term_what(x, y, &a, &c));

				/* Dump it */
				buf[x] = c;
			}

			/* Terminate */
			buf[x] = '\0';

			/* End the row */
			fprintf(fff, "%s\n", buf);
		}

		/* Skip a line */
		fprintf(fff, "\n");


		/* Dump the screen */
		for (y = 0; y < hgt; y++)
		{
			/* Dump each row */
			for (x = 0; x < wid; x++)
			{
				/* Get the attr/char */
				(void)(Term_what(x, y, &a, &c));

				/* Dump it */
				buf[x] = hack[a & 0x0F];
			}

			/* Terminate */
			buf[x] = '\0';

			/* End the row */
			fprintf(fff, "%s\n", buf);
		}

		/* Skip a line */
		fprintf(fff, "\n");


		/* Close it */
		my_fclose(fff);


		/* Message */
		msg_print("Screen dump saved.");
		msg_print(NULL);


		/* Restore the screen */
		Term_load();

		/* Leave "icky" mode */
		character_icky = FALSE;
	}
}


/*
 * Check the status of "artifacts"
 */
void do_cmd_knowledge_artifacts(void)
{
	int i, k, z, x, y;

	FILE *fff;

	char file_name[1024];

	char base_name[80];

	bool_ *okay, *okayk;


	/* Allocate the "okay" array */
	C_MAKE(okay, max_a_idx, bool_);
	C_MAKE(okayk, max_k_idx, bool_);

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the artifacts */
	for (k = 0; k < max_a_idx; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* Default */
		okay[k] = FALSE;

		/* Skip "empty" artifacts */
		if (!a_ptr->name) continue;

		/* Skip "uncreated" artifacts */
		if (!a_ptr->cur_num) continue;

		/* Assume okay */
		okay[k] = TRUE;
	}

	for (k = 0; k < max_k_idx; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Default */
		okayk[k] = FALSE;

		/* Skip "empty" artifacts */
		if (!(k_ptr->flags3 & TR3_NORM_ART)) continue;

		/* Skip "uncreated" artifacts */
		if (!k_ptr->artifact) continue;

		/* Assume okay */
		okayk[k] = TRUE;
	}

	/* Check the dungeon */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			s16b this_o_idx, next_o_idx = 0;

			/* Scan all objects in the grid */
			for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type * o_ptr;

				/* Acquire object */
				o_ptr = &o_list[this_o_idx];

				/* Acquire next object */
				next_o_idx = o_ptr->next_o_idx;

				/* Ignore random artifacts */
				if (o_ptr->tval == TV_RANDART) continue;

				/* Ignore non-artifacts */
				if (!artifact_p(o_ptr)) continue;

				/* Ignore known items */
				if (object_known_p(o_ptr)) continue;

				/* Note the artifact */
				if (k_info[o_ptr->k_idx].flags3 & TR3_NORM_ART)
				{
					okayk[o_ptr->k_idx] = FALSE;
				}
				else
				{
					okay[o_ptr->name1] = FALSE;
				}
			}
		}
	}

	/* Check monsters in the dungeon */
	for (i = 0; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		s16b this_o_idx, next_o_idx = 0;

		/* Scan all objects the monster carries */
		for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type * o_ptr;

			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Ignore random artifacts */
			if (o_ptr->tval == TV_RANDART) continue;

			/* Ignore non-artifacts */
			if (!artifact_p(o_ptr)) continue;

			/* Ignore known items */
			if (object_known_p(o_ptr)) continue;

			/* Note the artifact */
			if (k_info[o_ptr->k_idx].flags3 & TR3_NORM_ART)
			{
				okayk[o_ptr->k_idx] = FALSE;
			}
			else
			{
				okay[o_ptr->name1] = FALSE;
			}
		}
	}

	/* Check the p_ptr->inventory and equipment */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Ignore non-objects */
		if (!o_ptr->k_idx) continue;

		/* Ignore random artifacts */
		if (o_ptr->tval == TV_RANDART) continue;

		/* Ignore non-artifacts */
		if (!artifact_p(o_ptr)) continue;

		/* Ignore known items */
		if (object_known_p(o_ptr)) continue;

		/* Note the artifact */
		if (k_info[o_ptr->k_idx].flags3 & TR3_NORM_ART)
		{
			okayk[o_ptr->k_idx] = FALSE;
		}
		else
		{
			okay[o_ptr->name1] = FALSE;
		}
	}

	/* Scan the artifacts */
	for (k = 0; k < max_a_idx; k++)
	{
		artifact_type *a_ptr = &a_info[k];

		/* List "dead" ones */
		if (!okay[k]) continue;

		/* Paranoia */
		strcpy(base_name, "Unknown Artifact");

		/* Obtain the base object type */
		z = lookup_kind(a_ptr->tval, a_ptr->sval);

		/* Real object */
		if (z)
		{
			object_type forge;
			object_type *q_ptr;
			u32b f1, f2, f3, f4, f5, esp;

			/* Get local object */
			q_ptr = &forge;

			/* Create fake object */
			object_prep(q_ptr, z);

			/* Make it an artifact */
			q_ptr->name1 = k;

			/* Spell in it ? no ! */
			object_flags(q_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
			if (f5 & TR5_SPELL_CONTAIN)
				q_ptr->pval2 = -1;

			/* Describe the artifact */
			object_desc_store(base_name, q_ptr, FALSE, 0);
		}

		/* Hack -- Build the artifact name */
		fprintf(fff, "     The %s\n", base_name);
	}

	for (k = 0; k < max_k_idx; k++)
	{
		/* List "dead" ones */
		if (!okayk[k]) continue;

		/* Paranoia */
		strcpy(base_name, "Unknown Artifact");

		/* Real object */
		if (k)
		{
			object_type forge;
			object_type *q_ptr;

			/* Get local object */
			q_ptr = &forge;

			/* Create fake object */
			object_prep(q_ptr, k);

			/* Describe the artifact */
			object_desc_store(base_name, q_ptr, FALSE, 0);
		}

		/* Hack -- Build the artifact name */
		fprintf(fff, "     The %s\n", base_name);
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Artifacts Seen", 0, 0);

	/* Remove the file */
	fd_kill(file_name);

	C_FREE(okay, max_a_idx, bool_);
	C_FREE(okayk, max_k_idx, bool_);
}


/*
 * Check the status of traps
 */
void do_cmd_knowledge_traps(void)
{
	int k;

	FILE *fff;

	trap_type *t_ptr;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the traps */
	for (k = 0; k < max_t_idx; k++)
	{
		/* Get the trap */
		t_ptr = &t_info[k];

		/* Skip "empty" traps */
		if (!t_ptr->name) continue;

		/* Skip unidentified traps */
		if (!t_ptr->ident) continue;

		/* Hack -- Build the trap name */
		fprintf(fff, "     %s\n", t_name + t_ptr->name);
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Traps known", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Display known uniques
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 */
static void insert_sort_unique(int *sort_uniques, int *num, int r_idx)
{
	int i, j;

	monster_race *r_ptr = &r_info[r_idx];

	int level = r_ptr->level;


	/* Hack -- Morgoth is always at the bottom of the list */
	if (r_idx == 862) level = 20000;

	/* Find the place */
	for (i = 0; i < *num; i++)
	{
		monster_race *r2_ptr = &r_info[sort_uniques[i]];
		int level2 = r2_ptr->level;

		if (sort_uniques[i] == 862) level2 = 20000;

		if (level < level2) break;
	}

	/* Move the remaining items */
	for (j = *num - 1; j >= i; j--)
	{
		sort_uniques[j + 1] = sort_uniques[j];
	}

	/* Insert it */
	sort_uniques[i] = r_idx;
	(*num)++;
}


static void do_cmd_knowledge_uniques(void)
{
	int k;

	int *sort_uniques;

	int num = 0;

	FILE *fff;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	C_MAKE(sort_uniques, max_r_idx, int);

	/* Sort the monster races */
	for (k = 1; k < max_r_idx; k++)
	{
		monster_race *r_ptr = &r_info[k];

		/* Only print Uniques */
		if (r_ptr->flags1 & (RF1_UNIQUE) &&
		                !(r_ptr->flags7 & RF7_PET) &&
		                !(r_ptr->flags7 & RF7_NEUTRAL))
		{
			insert_sort_unique(sort_uniques, &num, k);
		}
	}

	/* Scan the monster races -- sorted */
	for (k = 0; k < num; k++)
	{
		monster_race *r_ptr = &r_info[sort_uniques[k]];

		/* Only print Uniques */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			bool_ dead = (r_ptr->max_num == 0);

			/* Only display "known" uniques */
			if (dead || cheat_know || r_ptr->r_sights)
			{
				/* Print a message */
				if (dead)
				{
					/* Don't print the unique's ASCII symbol
					 * if use_graphics is on. */
					if (use_graphics)
					{
						fprintf(fff, "[[[[[R%-70s is dead]\n",
					        	(r_name + r_ptr->name));
					}
					else
					{
						fprintf(fff, "[[[[[%c%c] [[[[[R%-68s is dead]\n",
							conv_color[r_ptr->d_attr],
							r_ptr->d_char,
					        	(r_name + r_ptr->name));
					}
				}
				else
				{
					/* Don't print the unique's ASCII symbol
					 * if use_graphics is on. */
					if (use_graphics)
					{
						fprintf(fff, "[[[[[w%-70s is alive]\n",
					        	(r_name + r_ptr->name));
					}
					else
					{
						fprintf(fff, "[[[[[%c%c] [[[[[w%-68s is alive]\n",
							conv_color[r_ptr->d_attr],
							r_ptr->d_char,
					        	(r_name + r_ptr->name));
					}
				}
			}
		}
	}

	C_FREE(sort_uniques, max_r_idx, int);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known Uniques", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


void plural_aux(char *name)
{
	int name_len = strlen(name);

	/* Hack -- Precedent must be pluralised for this one */
	if (strstr(name, "Disembodied hand"))
	{
		strcpy(name, "Disembodied hands that strangled people");
	}

	/* "someone of something" */
	else if (strstr(name, " of "))
	{
		cptr aider = strstr(name, " of ");
		char dummy[80];
		int i = 0;
		cptr ctr = name;

		while (ctr < aider)
		{
			dummy[i] = *ctr;
			ctr++;
			i++;
		}

		if (dummy[i - 1] == 's')
		{
			strcpy(&dummy[i], "es");
			i++;
		}
		else
		{
			strcpy(&dummy[i], "s");
		}

		strcpy(&dummy[i + 1], aider);
		strcpy(name, dummy);
	}

	/* Creeping coins */
	else if (strstr(name, "coins"))
	{
		char dummy[80];
		strcpy(dummy, "piles of ");
		strcat(dummy, name);
		strcpy(name, dummy);
		return;
	}

	/* Manes stay manes */
	else if (strstr(name, "Manes"))
	{
		return;
	}

	/* Broken plurals are, well, broken */
	else if (name[name_len - 1] == 'y')
	{
		strcpy(&name[name_len - 1], "ies");
	}
	else if (streq(&name[name_len - 4], "ouse"))
	{
		strcpy(&name[name_len - 4], "ice");
	}
	else if (streq(&name[name_len - 6], "kelman"))
	{
		strcpy(&name[name_len - 6], "kelmen");
	}
	else if (streq(&name[name_len - 2], "ex"))
	{
		strcpy(&name[name_len - 2], "ices");
	}
	else if (streq(&name[name_len - 3], "olf"))
	{
		strcpy(&name[name_len - 3], "olves");
	}

	/* Now begins sane cases */
	else if ((streq(&name[name_len - 2], "ch")) || (name[name_len - 1] == 's'))
	{
		strcpy(&name[name_len], "es");
	}
	else
	{
		strcpy(&name[name_len], "s");
	}
}


/*
 * Display current pets
 */
static void do_cmd_knowledge_pets(void)
{
	int i;

	FILE *fff;

	monster_type *m_ptr;

	int t_friends = 0;

	int t_levels = 0;

	int show_upkeep = 0;

	int upkeep_divider = 20;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	if (has_ability(AB_PERFECT_CASTING)) upkeep_divider = 15;

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Calculate "upkeep" for friendly monsters */
		if (m_ptr->status >= MSTATUS_PET)
		{
			char pet_name[80];
			monster_race *r_ptr = race_inf(m_ptr);

			t_friends++;
			t_levels += m_ptr->level;
			monster_desc(pet_name, m_ptr, 0x88);
			fprintf(fff, "%s%s (%s)\n",
			        (r_ptr->flags1 & RF1_UNIQUE) ? "#####G" : "",
			        pet_name,
			        (m_ptr->status < MSTATUS_COMPANION) ? "pet" : "companion");
		}
	}

	if (t_friends > 1 + (p_ptr->lev / (upkeep_divider)))
	{
		show_upkeep = (t_levels);

		if (show_upkeep > 100) show_upkeep = 100;
		else if (show_upkeep < 10) show_upkeep = 10;
	}


	fprintf(fff, "----------------------------------------------\n");
	fprintf(fff, "   Total: %d pet%s.\n", t_friends, (t_friends == 1 ? "" : "s"));
	fprintf(fff, "   Upkeep: %d%% mana.\n", show_upkeep);


	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Current Pets", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}



/*
 * Total kill count
 *
 * Note that the player ghosts are ignored.  XXX XXX XXX
 */
static void do_cmd_knowledge_kill_count(void)
{
	int k;

	FILE *fff;

	char file_name[1024];

	s32b Total = 0;


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	{
		/* Monsters slain */
		int kk;

		/* For all monsters */
		for (kk = 1; kk < max_r_idx; kk++)
		{
			monster_race *r_ptr = &r_info[kk];

			if (r_ptr->flags1 & (RF1_UNIQUE))
			{
				bool_ dead = (r_ptr->max_num == 0);

				if (dead)
				{
					Total++;
				}
			}
			else
			{
				s16b This = r_ptr->r_pkills;

				if (This > 0)
				{
					Total += This;
				}
			}
		}

		if (Total < 1)
		{
			fprintf(fff, "You have defeated no enemies yet.\n\n");
		}
		else if (Total == 1)
		{
			fprintf(fff, "You have defeated one enemy.\n\n");
		}
		else
		{
                  fprintf(fff, "You have defeated %ld enemies.\n\n", (long int) Total);
		}
	}

	Total = 0;

	/* Scan the monster races */
	for (k = 0; k < max_r_idx; k++)
	{
		monster_race *r_ptr = &r_info[k];

		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			bool_ dead = (r_ptr->max_num == 0);

			if (dead)
			{
				/* Print a message */
				fprintf(fff, "     %s\n",
				        (r_name + r_ptr->name));
				Total++;
			}
		}
		else
		{
			s16b This = r_ptr->r_pkills;

			if (This > 0)
			{
				if (This < 2)
				{
					if (strstr(r_name + r_ptr->name, "coins"))
					{
						fprintf(fff, "     1 pile of %s\n", (r_name + r_ptr->name));
					}
					else
					{
						fprintf(fff, "     1 %s\n", (r_name + r_ptr->name));
					}
				}
				else
				{
					char to_plural[80];
					strcpy(to_plural, (r_name + r_ptr->name));
					plural_aux(to_plural);
					fprintf(fff, "     %d %s\n", This, to_plural);
				}

				Total += This;
			}
		}
	}

	fprintf(fff, "----------------------------------------------\n");
	fprintf(fff, "   Total: %ld creature%s killed.\n", (long int) Total, (Total == 1 ? "" : "s"));

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Kill Count", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(void)
{
	int k;

	FILE *fff;

	char o_name[80];

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Scan the object kinds */
	for (k = 1; k < max_k_idx; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Hack -- skip artifacts */
		if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

		/* List known flavored objects */
		if (k_ptr->flavor && k_ptr->aware)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Create fake object */
			object_prep(i_ptr, k);

			/* Describe the object */
			object_desc_store(o_name, i_ptr, FALSE, 0);

			/* Print a message */
			fprintf(fff, "     %s\n", o_name);
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Known Objects", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * List recall depths
 */
static void do_cmd_knowledge_dungeons(void)
{
	int y;
	char file_name[1024];
	FILE *fff;

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Oops */
	if (fff == NULL) return;

	/* Scan all dungeons */
	for (y = 1; y < max_d_idx; y++)
	{
		/* The dungeon has a valid recall depth set */
		if (max_dlv[y])
		{
			/* Describe the recall depth */
			fprintf(fff, "       %c%s: Level %d (%d')\n",
			        (p_ptr->recall_dungeon == y) ? '*' : ' ',
			        d_name + d_info[y].name,
			        max_dlv[y], 50 * (max_dlv[y]));
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Recall Depths", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * List known towns
 */
void do_cmd_knowledge_towns(void)
{
	int i, j;
	char file_name[1024];
	FILE *fff;

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Oops */
	if (fff == NULL) return;

	/* Scan all dungeons */
	for (i = 0; i < max_d_idx; i++)
	{
		dungeon_info_type *d_ptr = &d_info[i];

		/* Scan all dungeon town slots */
		for (j = 0; j < TOWN_DUNGEON; j++)
		{
			int town_idx = d_ptr->t_idx[j];

			/* Ignore non-existent towns */
			if (!(town_info[town_idx].flags & (TOWN_REAL))) continue;

			/* Ignore unknown towns */
			if (!(town_info[town_idx].flags & (TOWN_KNOWN))) continue;

			/* Describe the dungeon town */
			fprintf(fff, "        %s: Level %d (%d')\n",
			        d_name + d_ptr->name,
			        d_ptr->t_level[j],
			        50 * d_ptr->t_level[j]);
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Dungeon Towns", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * List corruptions
 */
void do_cmd_knowledge_corruptions(void)
{
	FILE *fff;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Dump the corruptions to file */
	if (fff)
	{
		dump_corruptions(fff, TRUE, FALSE);
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Corruptions", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Helper function for do_cmd_knowledge_quests
 */
static void insert_sort_quest(int *order, int *num, int q_idx)
{
	int i, j;

	quest_type *q_ptr = &quest[q_idx];

	int level = q_ptr->level;


	/* Find the place */
	for (i = 0; i < *num; i++)
	{
		quest_type *q2_ptr = &quest[order[i]];
		int level2 = q2_ptr->level;

		if (level < level2) break;
	}

	/* Move the remaining items */
	for (j = *num - 1; j >= i; j--)
	{
		order[j + 1] = order[j];
	}

	/* Insert it */
	order[i] = q_idx;
	(*num)++;
}


/*
 * Print quest status of all active quests
 */
static void do_cmd_knowledge_quests(void)
{
	FILE *fff;

	char file_name[1024];

	int order[MAX_Q_IDX] = { };

	int num = 0;

	int i, j, z;


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	for (i = 0; i < MAX_Q_IDX; i++)
	{
		insert_sort_quest(order, &num, i);
	}

	for (z = 0; z < MAX_Q_IDX; z++)
	{
		i = order[z];

		/* Dynamic descriptions */
		if (quest[i].gen_desc != NULL)
		{
			if (!quest[i].gen_desc(fff))
			{
				continue;
			}
		}

		/* Fixed quests (only known ones) */
		else if (!quest[i].silent)
		{
			if (quest[i].status == QUEST_STATUS_TAKEN)
			{
				/* Print the quest info */
				fprintf(fff, "#####y%s (Danger level: %d)\n",
				        quest[i].name, quest[i].level);

				j = 0;
				while ((j < 10) && (quest[i].desc[j][0] != '\0'))
				{
					fprintf(fff, "%s\n", quest[i].desc[j++]);
				}
				fprintf(fff, "\n");
			}
			else if (quest[i].status == QUEST_STATUS_COMPLETED)
			{
				fprintf(fff , "#####G%s Completed - Unrewarded\n", quest[i].name);
				fprintf(fff, "\n");
			}
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Quest status", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Print fate status
 */
static void do_cmd_knowledge_fates(void)
{
	FILE *fff;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	dump_fates(fff);

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Fate status", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Print the note file
 */
void do_cmd_knowledge_notes(void)
{
	/* Spawn */
	show_notes_file();

	/* Done */
	return;
}


/*
 * Interact with "knowledge"
 */
void do_cmd_knowledge(void)
{
	int i;


	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Interact until done */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Ask for a choice */
		prt("Display current knowledge", 2, 0);

		/* Give some choices */
		prt("(1) Display known artifacts", 4, 5);
		prt("(2) Display known uniques", 5, 5);
		prt("(3) Display known objects", 6, 5);
		prt("(4) Display kill count", 7, 5);
		prt("(5) Display recall depths", 8, 5);
		prt("(6) Display corruptions", 9, 5);
		prt("(7) Display current pets", 10, 5);
		prt("(8) Display current quests", 11, 5);
		prt("(9) Display current fates", 12, 5);
		prt("(0) Display known traps", 13, 5);
		prt("(A) Display known dungeon towns", 14, 5);
		if (take_notes) prt("(B) Display notes", 15, 5);

		/* Prompt */
		prt("Command: ", 17, 0);

		/* Prompt */
		i = inkey();

		/* Done */
		if (i == ESCAPE) break;

		switch (i)
		{
			/* Artifacts */
		case '1':
			{
				do_cmd_knowledge_artifacts();

				break;
			}

			/* Uniques */
		case '2':
			{
				do_cmd_knowledge_uniques();

				break;
			}

			/* Objects */
		case '3':
			{
				do_cmd_knowledge_objects();

				break;
			}

			/* Kill count  */
		case '4':
			{
				do_cmd_knowledge_kill_count();

				break;
			}

			/* Recall depths */
		case '5':
			{
				do_cmd_knowledge_dungeons();

				break;
			}

			/* corruptions */
		case '6':
			{
				do_cmd_knowledge_corruptions();

				break;
			}

			/* Pets */
		case '7':
			{
				do_cmd_knowledge_pets();

				break;
			}

			/* Quests */
		case '8':
			{
				do_cmd_knowledge_quests();

				break;
			}

			/* Fates */
		case '9':
			{
				do_cmd_knowledge_fates();

				break;
			}

			/* Traps */
		case '0':
			{
				do_cmd_knowledge_traps();

				break;
			}

			/* Dungeon towns */
		case 'A':
		case 'a':
			{
				do_cmd_knowledge_towns();

				break;
			}

			/* Notes */
		case 'B':
		case 'b':
			{
				if (take_notes) do_cmd_knowledge_notes();
				else bell();

				break;
			}

			/* Unknown option */
		default:
			{
				bell();

				break;
			}
		}

		/* Flush messages */
		msg_print(NULL);
	}

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Check on the status of an active quest -KMW-
 * TODO: Spill out status when not a simple kill # monster.
 */
void do_cmd_checkquest(void)
{
	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Quest info */
	do_cmd_knowledge_quests();

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


/*
 * Change player's "tactic" setting
 */
void do_cmd_change_tactic(int i)
{
	p_ptr->tactic += i;
	if (p_ptr->tactic > 8) p_ptr->tactic = 0;
	if (p_ptr->tactic < 0) p_ptr->tactic = 8;

	p_ptr->update |= (PU_BONUS);
	update_stuff();
	prt("", 0, 0);
}


/*
 * Change player's "movement" setting
 */
void do_cmd_change_movement(int i)
{
	p_ptr->movement += i;
	if (p_ptr->movement > 8) p_ptr->movement = 0;
	if (p_ptr->movement < 0) p_ptr->movement = 8;

	p_ptr->update |= (PU_BONUS);
	update_stuff();
	prt("", 0, 0);
}


/*
 * Display the time and date
 */
void do_cmd_time()
{
	int day = bst(DAY, turn);

	int hour = bst(HOUR, turn);

	int min = bst(MINUTE, turn);

	int full = hour * 100 + min;

	char buf2[20];

	int start = 9999;

	int end = -9999;

	int num = 0;

	char desc[1024];

	char buf[1024];

	FILE *fff;


	/* Note */
	strcpy(desc, "It is a strange time.");

	/* Format time of the day */
	strnfmt(buf2, 20, get_day(bst(YEAR, turn) + START_YEAR));

	/* Display current date in the Elvish calendar */
	msg_format("This is %s of the %s year of the third age.",
	           get_month_name(day, wizard, FALSE), buf2);

	/* Message */
	msg_format("The time is %d:%02d %s.",
	           (hour % 12 == 0) ? 12 : (hour % 12),
	           min, (hour < 12) ? "AM" : "PM");

	/* Find the path */
	if (!rand_int(10) || p_ptr->image)
	{
		path_build(buf, 1024, ANGBAND_DIR_FILE, "timefun.txt");
	}
	else
	{
		path_build(buf, 1024, ANGBAND_DIR_FILE, "timenorm.txt");
	}

	/* Open this file */
	fff = my_fopen(buf, "rt");

	/* Oops */
	if (!fff) return;

	/* Find this time */
	while (!my_fgets(fff, buf, 1024))
	{
		/* Ignore comments */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Ignore invalid lines */
		if (buf[1] != ':') continue;

		/* Process 'Start' */
		if (buf[0] == 'S')
		{
			/* Extract the starting time */
			start = atoi(buf + 2);

			/* Assume valid for an hour */
			end = start + 59;

			/* Next... */
			continue;
		}

		/* Process 'End' */
		if (buf[0] == 'E')
		{
			/* Extract the ending time */
			end = atoi(buf + 2);

			/* Next... */
			continue;
		}

		/* Ignore incorrect range */
		if ((start > full) || (full > end)) continue;

		/* Process 'Description' */
		if (buf[0] == 'D')
		{
			num++;

			/* Apply the randomizer */
			if (!rand_int(num)) strcpy(desc, buf + 2);

			/* Next... */
			continue;
		}
	}

	/* Message */
	msg_print(desc);

	/* Close the file */
	my_fclose(fff);
}

/*
 * Macro recorder!
 * It records all keypresses and then put them in a macro
 * Not as powerful as the macro screen, but much easier for newbies
 */
char *macro_recorder_current = NULL;
void macro_recorder_start()
{
	msg_print("Starting macro recording, press this key again to stop. Note that if the action you want to record accepts the @ key, use it; it will remove your the need to inscribe stuff.");
	C_MAKE(macro_recorder_current, 1, char);
	macro_recorder_current[0] = '\0';
}

void macro_recorder_add(char c)
{
	char *old_macro_recorder_current = macro_recorder_current;

	if (macro_recorder_current == NULL) return;

	C_MAKE(macro_recorder_current, strlen(macro_recorder_current) + 1 + 1, char);
	sprintf(macro_recorder_current, "%s%c", old_macro_recorder_current, c);
	C_FREE(old_macro_recorder_current, strlen(old_macro_recorder_current) + 1, char);
}

void macro_recorder_stop()
{
	char *str, *macro;
	char buf[1024];

	/* Ok we remove the last key, because it is the key to stop recording */
	macro_recorder_current[strlen(macro_recorder_current) - 1] = '\0';

	/* Stop the recording */
	macro = macro_recorder_current;
	macro_recorder_current = NULL;

	/* Add it */
	if (get_check("Are you satisfied and want to create the macro? "))
	{
		prt("Trigger: ", 0, 0);

		/* Get a macro trigger */
		do_cmd_macro_aux(buf, FALSE);

		/* Link the macro */
		macro_add(buf, macro);

		/* Prompt */
		C_MAKE(str, (strlen(macro) + 1) * 3, char);
		ascii_to_text(str, macro);
		msg_format("Added a macro '%s'. If you want it to stay permanently, press @ now and dump macros to a file.", str);
		C_FREE(str, (strlen(macro) + 1) * 3, char);
	}

	/* Ok now rid of useless stuff */
	C_FREE(macro, strlen(macro) + 1, char);
}

void do_cmd_macro_recorder()
{
	if (macro_recorder_current == NULL)
		macro_recorder_start();
	else
		macro_recorder_stop();
}
