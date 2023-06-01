#include "q_eol.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "hook_stair_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hook_quest_fail_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <cassert>

#define cquest (quest[QUEST_EOL])

GENERATE_MONSTER_LOOKUP_FN(get_eol, "Eol, the Dark Elf")

static bool quest_eol_gen_hook(void *, void *, void *)
{
	int x, y;
	int xsize = 50, ysize = 30, y0, x0;
	int m_idx = 0;

	if (p_ptr->inside_quest != QUEST_EOL)
	{
		return false;
	}

	x0 = 2 + (xsize / 2);
	y0 = 2 + (ysize / 2);

	feat_wall_outer = FEAT_WALL_OUTER;
	feat_wall_inner = FEAT_WALL_INNER;

	for (y = 0; y < 100; y++)
	{
		floor_type[y] = FEAT_FLOOR;
		fill_type[y] = FEAT_WALL_OUTER;
	}

	/* Start with perm walls */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_set_feat(y, x, FEAT_PERM_SOLID);
		}
	}
	dun_level = quest[p_ptr->inside_quest].level;

	bool done = false;
	while (!done)
	{
		int grd, roug, cutoff;

		/* testing values for these parameters feel free to adjust*/
		grd = 2 ^ (randint(4));

		/* want average of about 16 */
		roug = randint(8) * randint(4);

		/* about size/2 */
		cutoff = randint(xsize / 4) + randint(ysize / 4) + randint(xsize / 4) + randint(ysize / 4);

		/* make it */
		generate_hmap(y0, x0, xsize, ysize, grd, roug, cutoff);

		/* Convert to normal format+ clean up*/
		done = generate_fracave(y0, x0, xsize, ysize, cutoff, false, true);
	}

	/* Place a few traps */
	for (x = xsize - 1; x >= 2; x--)
		for (y = ysize - 1; y >= 2; y--)
		{
			if (!cave_clean_bold(y, x)) continue;

			/* Place eol at the other end */
			if (!m_idx)
			{
				// Find Eol's r_info entry
				int r_idx = get_eol();
				// "Summon" Eol
				m_allow_special[r_idx] = true;
				m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_ENEMY);
				m_allow_special[r_idx] = false;
				// Mark with the QUEST flag
				if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
			}

			/* Place player at one end */
			p_ptr->py = y;
			p_ptr->px = x;
		}

	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);

	return true;
}

static bool quest_eol_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

	if (q_idx != QUEST_EOL)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "A tragedy, but the deed needed to be done.", 8, 0);
	c_put_str(TERM_YELLOW, "Accept my thanks, and that reward.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_LITE, SV_LITE_DWARVEN));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->name2 = EGO_LITE_MAGI;
	apply_magic(q_ptr, 1, false, false, false);
	q_ptr->number = 1;
	inven_carry(q_ptr, false);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NIRNAETH;
	quest[*(quest[q_idx].plot)].init();

	del_hook_new(HOOK_QUEST_FINISH, quest_eol_finish_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_eol_fail_hook(void *, void *in_, void *)
{
	struct hook_quest_fail_in *in = static_cast<struct hook_quest_fail_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_EOL)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "You fled ! I did not think you would flee...", 8, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FAIL, quest_eol_fail_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_eol_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_EOL)
	{
		return false;
	}

	if (r_idx == get_eol())
	{
		cmsg_print(TERM_YELLOW, "Such a sad end...");
		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_eol_death_hook);
		process_hooks_restart = true;

		return false;
	}

	return false;
}

static bool quest_eol_stair_hook(void *, void *in_, void *)
{
	auto const &r_info = game->edit_data.r_info;

	struct hook_stair_in *in = static_cast<struct hook_stair_in *>(in_);
	auto r_ptr = &r_info[get_eol()];

	if (p_ptr->inside_quest != QUEST_EOL)
	{
		return false;
	}

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS)
	{
		return true;
	}

	if (r_ptr->max_num)
	{
		if (in->direction == STAIRS_UP)
		{
			/* Flush input */
			flush();

			if (!get_check("Really abandon the quest?"))
			{
				return true;
			}

			cmsg_print(TERM_YELLOW, "You flee away from Eol...");
			cquest.status = QUEST_STATUS_FAILED;

			del_hook_new(HOOK_STAIR, quest_eol_stair_hook);
			process_hooks_restart = true;
			return false;
		}
	}

	return false;
}

void quest_eol_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_eol_death_hook,  "eol_death",  NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_eol_gen_hook,    "eol_gen",    NULL);
		add_hook_new(HOOK_STAIR,         quest_eol_stair_hook,  "eol_stair",  NULL);
		add_hook_new(HOOK_QUEST_FAIL,    quest_eol_fail_hook,   "eol_fail",   NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_eol_finish_hook, "eol_finish", NULL);
	}
}
