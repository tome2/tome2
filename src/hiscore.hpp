#pragma once

#include "h-basic.hpp"

/**
 * Maximum number of high scores in the high score file
 */
constexpr int MAX_HISCORES = 100;

/*
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score
{
	char what[8];                 /* Version info (string) */

	char pts[10];                 /* Total Score (number) */

	char gold[10];                 /* Total Gold (number) */

	char turns[10];                 /* Turns Taken (number) */

	char day[10];                 /* Time stamp (string) */

	char who[16];                 /* Player Name (string) */

	char unused_1[8]; /* Kept for compatibility only */

	char p_r[3];                 /* Player Race (number) */
	char p_s[3];             /* Player Subrace (number) */
	char p_c[3];                 /* Player Class (number) */
	char p_cs[3];            /* Player Class spec (number) */

	char cur_lev[4];                 /* Current Player Level (number) */
	char cur_dun[4];                 /* Current Dungeon Level (number) */
	char max_lev[4];                 /* Max Player Level (number) */
	char max_dun[4];                 /* Max Dungeon Level (number) */

	char unused_2[4]; /* Kept for compatibility only */
	char unused_3[4]; /* Kept for compatibility only */
	char inside_quest[4];    /* Did the player die in a quest? */
	char unused_4[4]; /* Kept for compatibility only */

	char how[32];                 /* Method of death (string) */
};

/*
 * Seek score 'i' in the highscore file
 */
int highscore_seek(int highscore_fd, int i);

/*
 * Read one score from the highscore file
 */
errr highscore_read(int highscore_fd, high_score *score);

/*
 * Write one score to the highscore file
 */
int highscore_write(int highscore_fd, high_score *score);

/*
 * Determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
int highscore_where(int highscore_fd, high_score *score);

/*
 * Place an entry into the high score file. Return the location (0 is
 * best) or -1 on "failure"
 */
int highscore_add(int highscore_fd, high_score *score);
