/*
 * Copyright (c) 1989, 1999 James E. Wilson, Robert A. Koeneke,
 * Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "notes.hpp"

#include "files.hpp"
#include "game.hpp"
#include "player_class.hpp"
#include "player_type.hpp"
#include "util.hpp"
#include "variable.hpp"

#include <boost/filesystem/path.hpp>
#include <fmt/format.h>

namespace fs = boost::filesystem;

/**
 * Get note path name
 */
static fs::path note_path()
{
	char buf[1024];
	path_build(buf, sizeof(buf), ANGBAND_DIR_NOTE, name_file_note(game->player_base).c_str());
	return fs::path(buf);
}

/*
 * Show the notes file on the screen
 */
void show_notes_file()
{
	auto p = note_path();

	/* Use a caption, forcing direct access to the note file */
	auto caption = fmt::format("Note file {}", p.filename().string().c_str());

	/* Invoke show_file */
	show_file(p.string().c_str(), caption.c_str());
}

/*
 * Output a string to the notes file.
 * This is the only function that references that file.
 */
void output_note(const char *final_note)
{
	/* Open notes file */
	FILE *fff = my_fopen(note_path().string().c_str(), "a");

	/* Failure */
	if (!fff) return;

	/* Add note, and close note file */
	fprintf(fff, "%s\n", final_note);

	/* Close the handle */
	my_fclose(fff);
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
	char turn_s[50];
	char depths[32];

	/* Get the first 60 chars - so do not have an overflow */
	memset(buf, 0, sizeof(buf));
	strncpy(buf, note, 60);

	/* Get date and time */
	sprintf(turn_s, "Turn % 12ld", static_cast<long int>(turn));

	/* Get depth  */
	if (!dun_level) strcpy(depths, "  Town");
	else sprintf(depths, "Lev%3d", dun_level);

	/* Make note */
	snprintf(final_note, sizeof(final_note), "%-20s %s %c: %s", turn_s, depths, code, buf);

	/* Output to the notes file */
	output_note(final_note);
}


/*
 * Append a note to the notes file using a "type".
 */
void add_note_type(int note_number)
{
	auto const &class_info = game->edit_data.class_info;

	char true_long_day[50];
	char buf[1024];
	time_t ct = time((time_t*)0);

	/* Get the date */
	strftime(true_long_day, 30, "%Y-%m-%d at %H:%M:%S", localtime(&ct));

	/* Work out what to do */
	switch (note_number)
	{
	case NOTE_BIRTH:
		{
			/* Player has just been born */
			char player[100];

			/* Build the string containing the player information */
			auto const player_race_name = get_player_race_name(p_ptr->prace, p_ptr->pracem);
			sprintf(player,
				"the %s %s",
				player_race_name.c_str(),
				class_info[p_ptr->pclass].spec[p_ptr->pspec].title);

			/* Add in "character start" information */
			sprintf(buf,
			        "\n"
			        "================================================\n"
			        "%s %s\n"
			        "Born on %s\n"
			        "================================================\n",
				game->player_name.c_str(), player, true_long_day);

			break;
		}

	case NOTE_WINNER:
		{
			sprintf(buf,
			        "%s slew Morgoth on %s\n"
			        "Long live %s!\n"
			        "================================================",
				game->player_name.c_str(), true_long_day, game->player_name.c_str());

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
