#ifndef H_267bf1c1_eada_4b18_9558_d96330fa7258
#define H_267bf1c1_eada_4b18_9558_d96330fa7258

#include "h-type.h"

#ifdef __cplusplus
extern "C" {
#endif

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

	char sex[2];                 /* Player Sex (string) */
	char p_r[3];                 /* Player Race (number) */
	char p_s[3];             /* Player Subrace (number) */
	char p_c[3];                 /* Player Class (number) */
	char p_cs[3];            /* Player Class spec (number) */

	char cur_lev[4];                 /* Current Player Level (number) */
	char cur_dun[4];                 /* Current Dungeon Level (number) */
	char max_lev[4];                 /* Max Player Level (number) */
	char max_dun[4];                 /* Max Dungeon Level (number) */

	char arena_number[4];         /* Arena level attained -KMW- */
	char inside_arena[4];    /* Did the player die in the arena? */
	char inside_quest[4];    /* Did the player die in a quest? */
	char exit_bldg[4];         /* Can the player exit arena? Goal obtained? -KMW- */

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

#ifdef __cplusplus
} // extern "C"
#endif

#endif
