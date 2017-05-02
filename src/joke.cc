#include "joke.hpp"

#include "monster2.hpp"
#include "options.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

static void gen_joke_place_monster(int r_idx)
{
	int try_;

	for (try_ = 0; try_ < 1000; try_++)
	{
		int x = randint(cur_hgt - 4) + 2;
		int y = randint(cur_wid - 4) + 2;

		if (place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_ENEMY))
		{
			return;
		}
	}
}

bool gen_joke_monsters(void *data, void *in, void *out)
{
	if (options->joke_monsters)
	{
		if ((dungeon_type == 20) &&
		    (dun_level == 72))
		{
			int r_idx = test_monster_name("Neil, the Sorceror");
			m_allow_special[r_idx] = TRUE;
			gen_joke_place_monster(r_idx);
			m_allow_special[r_idx] = FALSE;
		}
	}

	return FALSE;
}
