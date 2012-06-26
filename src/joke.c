#include "angband.h"

static void gen_joke_place_monster(r_idx)
{
	int try;

	for (try = 0; try < 1000; try++)
	{
		int x = randint(cur_hgt - 4) + 2;
		int y = randint(cur_wid - 4) + 2;

		if (place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_ENEMY))
		{
			return;
		}
	}
}

bool_ gen_joke_monsters(void *data, void *in, void *out)
{
	if (joke_monsters)
	{
		if ((dungeon_type == 20) &&
		    (dun_level == 72))
		{
			int r_idx = test_monster_name("Neil, the Sorceror");
			m_allow_special[r_idx + 1] = TRUE;
			gen_joke_place_monster(r_idx);
			m_allow_special[r_idx + 1] = FALSE;
		}
	}

	return FALSE;
}
