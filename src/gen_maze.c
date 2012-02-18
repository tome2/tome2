/*
 * Maze dungeon generator
 */

/*
 * Copyright (c) 2003 DarkGod. And somebody who posted the algorith on
 * rec.games.roguelike.development. I can't remember teh name :( please mail me
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

/*
 * If we wasted static memory for this, it would look like:
 *
 * static char maze[(MAX_HGT / 2) + 2][(MAX_WID / 2) + 2];
 */
typedef signed char maze_row[(MAX_WID / 2) + 2];

void dig(maze_row *maze, int y, int x, int d)
{
	int k;
	int dy = 0, dx = 0;

	/*
	 * first, open the wall of the new cell
	 * in the direction we come from.
	 */
	switch (d)
	{
	case 0:
		{
			maze[y][x] |= 4;
			break;
		}

	case 1:
		{
			maze[y][x] |= 8;
			break;
		}

	case 2:
		{
			maze[y][x] |= 1;
			break;
		}

	case 3:
		{
			maze[y][x] |= 2;
			break;
		}
	}

	/*
	 * try to chage direction, here 50% times.
	 * with smaller values (say 25%) the maze
	 * is made of long straight corridors. with
	 * greaters values (say 75%) the maze is
	 * very "turny".
	 */
	if (rand_range(1, 100) < 50) d = rand_range(0, 3);

	for (k = 1; k <= 4; k++)
	{
		switch (d)
		{
		case 0:
			{
				dy = 0;
				dx = 1;
				break;
			}

		case 1:
			{
				dy = -1;
				dx = 0;
				break;
			}

		case 2:
			{
				dy = 0;
				dx = -1;
				break;
			}

		case 3:
			{
				dy = 1;
				dx = 0;
				break;
			}
		}

		if (maze[y + dy][x + dx] == 0)
		{
			/*
			 * now, open the wall of the new cell
			 * in the direction we go to.
			 */
			switch (d)
			{
			case 0:
				{
					maze[y][x] |= 1;
					break;
				}

			case 1:
				{
					maze[y][x] |= 2;
					break;
				}

			case 2:
				{
					maze[y][x] |= 4;
					break;
				}

			case 3:
				{
					maze[y][x] |= 8;
					break;
				}
			}

			dig(maze, y + dy, x + dx, d);
		}

		d = (d + 1) % 4;
	}
}


bool_ level_generate_maze()
{
	int i, j, d;
	int y, dy = 0;
	int x, dx = 0;
	int m_1 = 0, m_2 = 0;
	maze_row *maze;

	/* Allocate temporary memory */
	C_MAKE(maze, (MAX_HGT / 2) + 2, maze_row);

	/*
	 * the empty maze is:
	 *
	 * -1 -1 ... -1 -1
	 * -1  0      0 -1
	 *  .            .
	 *  .            .
	 * -1  0      0 -1
	 * -1 -1 ... -1 -1
	 *
	 *  -1 are so-called "sentinel value".
	 *   0 are empty cells.
	 *
	 *  walls are not represented, only cells.
	 *  at the end of the algorithm each cell
	 *  contains a value that is bit mask
	 *  representing surrounding walls:
	 *
	 *         bit #1
	 *
	 *        +------+
	 *        |      |
	 * bit #2 |      | bit #0
	 *        |      |
	 *        +------+
	 *
	 *         bit #3
	 *
	 * d is the direction you are digging
	 * to. d value is the bit number:
	 * d=0 --> go east
	 * d=1 --> go north
	 * etc
	 *
	 * you need only 4 bits per cell.
	 * this gives you a very compact
	 * maze representation.
	 *
	 */
	for (j = 0; j <= (cur_hgt / 2) + 1; j++)
	{
		for (i = 0; i <= (cur_wid / 2) + 1; i++)
		{
			maze[j][i] = -1;
		}
	}

	for (j = 1; j <= (cur_hgt / 2); j++)
	{
		for (i = 1; i <= (cur_wid / 2); i++)
		{
			maze[j][i] = 0;
		}
	}

	y = rand_range(1, (cur_hgt / 2));
	x = rand_range(1, (cur_wid / 2));
	d = rand_range(0, 3);

	dig(maze, y, x, d);

	maze[y][x] = 0;

	for (d = 0; d <= 3; d++)
	{
		switch (d)
		{
		case 0:
			{
				dy = 0;
				dx = 1;
				m_1 = 1;
				m_2 = 4;
				break;
			}

		case 1:
			{
				dy = -1;
				dx = 0;
				m_1 = 2;
				m_2 = 8;
				break;
			}

		case 2:
			{
				dy = 0;
				dx = -1;
				m_1 = 4;
				m_2 = 1;
				break;
			}

		case 3:
			{
				dy = 1;
				dx = 0;
				m_1 = 8;
				m_2 = 2;
				break;
			}
		}

		if ((maze[y + dy][x + dx] != -1) &&
		                ((maze[y + dy][x + dx] & m_2) != 0))
		{
			maze[y][x] |= m_1;
		}
	}

	/* Translate the maze bit array into a real dungeon map -- DG */
	for (j = 1; j <= (cur_hgt / 2) - 2; j++)
	{
		for (i = 1; i <= (cur_wid / 2) - 2; i++)
		{
			if (maze[j][i])
			{
				place_floor(j * 2, i * 2);
			}

			if (maze[j][i] & 1)
			{
				place_floor(j * 2, i * 2 + 1);
			}

			if (maze[j][i] & 8)
			{
				place_floor(j * 2 + 1, i * 2);
			}
		}
	}

	/* Free temporary memory */
	C_FREE(maze, (MAX_HGT / 2) + 2, maze_row);

	/* Determine the character location */
	if (!new_player_spot(get_branch()))
		return FALSE;

	return TRUE;
}
