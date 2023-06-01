/*
 * Copyright (c) 2003 DarkGod.
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "gen_evol.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "levels.hpp"
#include "player_type.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

/*
 * Generate a game of life level :) and make it evolve
 */
void evolve_level(bool noise)
{
	auto const &f_info = game->edit_data.f_info;

	/* Add a bit of noise */
	if (noise)
	{
		int cw = 0;
		int cf = 0;

		for (int i = 1; i < cur_wid - 1; i++)
		{
			for (int j = 1; j < cur_hgt - 1; j++)
			{
				if (f_info[cave[j][i].feat].flags & FF_WALL) cw++;
				if (f_info[cave[j][i].feat].flags & FF_FLOOR) cf++;
			}
		}

		for (int i = 1; i < cur_wid - 1; i++)
		{
			for (int j = 1; j < cur_hgt - 1; j++)
			{
				/* Access the grid */
				auto c_ptr = &cave[j][i];

				/* Permanent features should stay */
				if (f_info[c_ptr->feat].flags & FF_PERMANENT) continue;

				/* Avoid evolving grids with object or monster */
				if ((!c_ptr->o_idxs.empty()) || c_ptr->m_idx) continue;

				/* Avoid evolving player grid */
				if ((j == p_ptr->py) && (i == p_ptr->px)) continue;

				if (magik(7))
				{
					if (cw > cf)
					{
						place_floor(j, i);
					}
					else
					{
						place_filler(j, i);
					}
				}
			}
		}
	}

	for (int i = 1; i < cur_wid - 1; i++)
	{
		for (int j = 1; j < cur_hgt - 1; j++)
		{
			int x, y, c;
			cave_type *c_ptr;

			/* Access the grid */
			c_ptr = &cave[j][i];

			/* Permanent features should stay */
			if (f_info[c_ptr->feat].flags & FF_PERMANENT) continue;

			/* Avoid evolving grids with object or monster */
			if ((!c_ptr->o_idxs.empty()) || c_ptr->m_idx) continue;

			/* Avoid evolving player grid */
			if ((j == p_ptr->py) && (i == p_ptr->px)) continue;


			/* Reset tally */
			c = 0;

			/* Count number of surrounding walls */
			for (x = i - 1; x <= i + 1; x++)
			{
				for (y = j - 1; y <= j + 1; y++)
				{
					if ((x == i) && (y == j)) continue;
					if (f_info[cave[y][x].feat].flags & FF_WALL) c++;
				}
			}

			/*
			 * Changed these parameters a bit, so that it doesn't produce
			 * too open or too narrow levels -- pelpel
			 */
			/* Starved or suffocated */
			if ((c < 4) || (c >= 7))
			{
				if (f_info[c_ptr->feat].flags & FF_WALL)
				{
					place_floor(j, i);
				}
			}

			/* Spawned */
			else if ((c == 4) || (c == 5))
			{
				if (!(f_info[c_ptr->feat].flags & FF_WALL))
				{
					place_filler(j, i);
				}
			}
		}
	}

	/* Notice changes */
	p_ptr->update |= (PU_VIEW | PU_MONSTERS | PU_FLOW | PU_MON_LITE);
}


bool level_generate_life()
{
	int i, j;

	for (i = 1; i < cur_wid - 1; i++)
	{
		for (j = 1; j < cur_hgt - 1; j++)
		{
			cave[j][i].info = (CAVE_ROOM | CAVE_GLOW | CAVE_MARK);
			if (magik(45)) place_floor(j, i);
			else place_filler(j, i);
		}
	}

	evolve_level(false);
	evolve_level(false);
	evolve_level(false);

	/* Determine the character location */
	if (!new_player_spot(get_branch()))
	{
		return false;
	}

	return true;
}
