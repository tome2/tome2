/* File: notes.c */

/* Purpose: Note taking to a file */

/*
 * Copyright (c) 1989, 1999 James E. Wilson, Robert A. Koeneke,
 * Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * Show the notes file on the screen
 */
void show_notes_file(void)
{
	char basename[13];
	char buf[1024];
	char caption[10 + 13];

	/* Hack -- extract first 8 characters of name and append an extension */
	(void)strnfmt(basename, sizeof(basename), "%.8s.nte", player_base);
	basename[sizeof(basename) - 1] = '\0';

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_NOTE, basename);

	/* Use a caption, forcing direct access to the note file */
	sprintf(caption, "Note file %s", basename);

	/* Invoke show_file */
	(void)show_file(buf, caption, 0, 0);

	/* Done */
	return;
}

/*
 * Output a string to the notes file.
 * This is the only function that references that file.
 */
void output_note(char *final_note)
{
	FILE *fff;
	char basename[13];
	char buf[1024];

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Hack -- extract first 8 characters of name and append an extension */
	(void)strnfmt(basename, sizeof(basename), "%.8s.nte", player_base);
	basename[sizeof(basename) - 1] = '\0';

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_NOTE, basename);

	/* Open notes file */
	fff = my_fopen(buf, "a");

	/* Failure */
	if (!fff) return;

	/* Add note, and close note file */
	my_fputs(fff, final_note, 0);

	/* Close the handle */
	my_fclose(fff);

	/* Done */
	return;
}


/*
 * Add note to file using a string + character symbol
 * to specify its type so that the notes file can be
 * searched easily by external utilities.
 */
void add_note(char *note, char code)
{
	char buf[100];
	char final_note[100];
	char long_day[50];
	char depths[32];
	char *tmp;

	/* Get the first 60 chars - so do not have an overflow */
	tmp = C_WIPE(buf, 100, char);
	strncpy(buf, note, 60);

	/* Get date and time */
	sprintf(long_day, "%ld:%02ld %s, %s", (bst(HOUR, turn) % 12 == 0) ? 12 : (bst(HOUR, turn) % 12),
	        bst(MINUTE, turn), (bst(HOUR, turn) < 12) ? "AM" : "PM", get_month_name(bst(DAY, turn), FALSE,
	                        FALSE));

	/* Get depth  */
	if (!dun_level) strcpy(depths, "  Town");
	else if (depth_in_feet) sprintf(depths, "%4dft", dun_level * 50);
	else sprintf(depths, "Lev%3d", dun_level);

	/* Make note */
	sprintf(final_note, "%-20s %s %c: %s", long_day, depths, code, buf);

	/* Output to the notes file */
	output_note(final_note);
}


/*
 * Append a note to the notes file using a "type".
 */
void add_note_type(int note_number)
{
	char long_day[50], true_long_day[50];
	char buf[1024];
	time_t ct = time((time_t*)0);

	/* Get the date */
	strftime(true_long_day, 30, "%Y-%m-%d at %H:%M:%S", localtime(&ct));

	/* Get the date */
	sprintf(buf, "%ld", bst(YEAR, turn) + START_YEAR);
	sprintf(long_day, "%ld:%02ld %s the %s of III %s", (bst(HOUR, turn) % 12 == 0) ? 12 : (bst(HOUR, turn) % 12), bst(MINUTE, turn), (bst(HOUR, turn) < 12) ? "AM" : "PM", get_month_name(bst(DAY, turn), FALSE, FALSE), buf);

	/* Work out what to do */
	switch (note_number)
	{
	case NOTE_BIRTH:
		{
			/* Player has just been born */
			char player[100];

			/* Build the string containing the player information */
			sprintf(player, "the %s %s", get_player_race_name(p_ptr->prace, p_ptr->pracem), class_info[p_ptr->pclass].spec[p_ptr->pspec].title + c_name);

			/* Add in "character start" information */
			sprintf(buf,
			        "\n"
			        "================================================\n"
			        "%s %s\n"
			        "Born on %s\n"
			        "================================================\n",
			        player_name, player, true_long_day);

			break;
		}

	case NOTE_WINNER:
		{
			sprintf(buf,
			        "%s slew Morgoth on %s\n"
			        "Long live %s!\n"
			        "================================================",
			        player_name, long_day, player_name);

			break;
		}

	case NOTE_SAVE_GAME:
		{
			/* Saving the game */
			sprintf(buf, "\nSession end: %s", true_long_day);

			break;
		}

	case NOTE_ENTER_DUNGEON:
		{
			/* Entering the game after a break. */
			sprintf(buf,
			        "================================================\n"
			        "New session start: %s\n",
			        true_long_day);

			break;
		}

	default:
		return;
	}

	/* Output the notes to the file */
	output_note(buf);
}
