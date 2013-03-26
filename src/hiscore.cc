#include "hiscore.h"

#include <assert.h>

#include "angband.h"

int highscore_seek(int highscore_fd, int i)
{
	/* Seek for the requested record */
	return (fd_seek(highscore_fd, (huge)(i) * sizeof(high_score)));
}

errr highscore_read(int highscore_fd, high_score *score)
{
	/* Read the record, note failure */
	return (fd_read(highscore_fd, (char*)(score), sizeof(high_score)));
}

int highscore_write(int highscore_fd, high_score *score)
{
	/* Write the record, note failure */
	return (fd_write(highscore_fd, (char*)(score), sizeof(high_score)));
}

int highscore_where(int highscore_fd, high_score *score)
{
	int i;

	high_score the_score;

	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return ( -1);

	/* Go to the start of the highscore file */
	if (highscore_seek(highscore_fd, 0)) return ( -1);

	/* Read until we get to a higher score */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(highscore_fd, &the_score)) return (i);
		if (strcmp(the_score.pts, score->pts) < 0) return (i);
	}

	/* The "last" entry is always usable */
	return (MAX_HISCORES - 1);
}

int highscore_add(int highscore_fd, high_score *score)
{
	int i, slot;
	bool_ done = FALSE;

	high_score the_score, tmpscore;


	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return ( -1);

	/* Determine where the score should go */
	slot = highscore_where(highscore_fd, score);

	/* Hack -- Not on the list */
	if (slot < 0) return ( -1);

	/* Hack -- prepare to dump the new score */
	the_score = (*score);

	/* Slide all the scores down one */
	for (i = slot; !done && (i < MAX_HISCORES); i++)
	{
		/* Read the old guy, note errors */
		if (highscore_seek(highscore_fd, i)) return ( -1);
		if (highscore_read(highscore_fd, &tmpscore)) done = TRUE;

		/* Back up and dump the score we were holding */
		if (highscore_seek(highscore_fd, i)) return ( -1);
		if (highscore_write(highscore_fd, &the_score)) return ( -1);

		/* Hack -- Save the old score, for the next pass */
		the_score = tmpscore;
	}

	/* Return location used */
	return (slot);
}
