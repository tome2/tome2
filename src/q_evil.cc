#include "q_evil.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "hook_quest_finish_in.hpp"
#include "hook_quest_gen_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.h"

#define cquest (quest[QUEST_EVIL])

static bool quest_evil_gen_hook(void *, void *in_, void *)
{
	auto in = static_cast<hook_quest_gen_in *>(in_);
	auto const &f_info = game->edit_data.f_info;

	int x, y, i;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_EVIL)
	{
		return false;
	}

	/* Just in case we didnt talk the the mayor */
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		cquest.status = QUEST_STATUS_TAKEN;
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

	/* Set the correct monster hook */
	reset_get_monster_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("evil.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	in->dungeon_flags_ref |= DF_NO_GENO;

	/* Place some random balrogs */
	for (i = 6; i > 0; )
	{
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		auto const flags = f_info[cave[y][x].feat].flags;
		if (!(flags & FF_PERMANENT) && (flags & FF_FLOOR))
		{
			int m_idx = place_monster_one(y, x, 996, 0, FALSE, MSTATUS_ENEMY);
			if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
			--i;
		}
	}

	process_hooks_restart = TRUE;

	return true;
}

static bool quest_evil_death_hook(void *, void *, void *)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_EVIL)
	{
		return false;
	}

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status <= MSTATUS_ENEMY) mcnt++;
	}

	/* Nobody left ? */
	if (mcnt <= 1)
	{
		/* TODO: change to COMPLETED and remove NULL when mayor is added */
		quest[p_ptr->inside_quest].status = QUEST_STATUS_FINISHED;
		*(quest[p_ptr->inside_quest].plot) = QUEST_NULL;

		del_hook_new(HOOK_MONSTER_DEATH, quest_evil_death_hook);
		del_hook_new(HOOK_GEN_QUEST,     quest_evil_gen_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "Khazad-Dum is safer now.");
		return false;
	}
	return false;
}

static bool quest_evil_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_EVIL)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "Thank you for saving us!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the cave as your house as a reward.", 9, 0);

	/* End the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	return true;
}

void quest_evil_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_evil_death_hook,  "evil_monster_death", NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_evil_finish_hook, "evil_finish",        NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_evil_gen_hook,    "evil_geb",           NULL);
	}
}
