#include "q_haunted.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

#define cquest (quest[QUEST_HAUNTED])

static bool quest_haunted_gen_hook(void *, void *, void *)
{
	auto const &f_info = game->edit_data.f_info;

	int x, y, i, m_idx;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_HAUNTED)
	{
		return false;
	}

	/* Just in case we didnt talk the the mayor */
	if (cquest.status == QUEST_STATUS_UNTAKEN)
		cquest.status = QUEST_STATUS_TAKEN;

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
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("haunted.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	dungeon_flags |= DF_NO_GENO;

	/* Place some ghosts */
	for (i = 12; i > 0; )
	{
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		auto const flags = f_info[cave[y][x].feat].flags;
		if (!(flags & FF_PERMANENT) && (flags & FF_FLOOR))
		{
			m_idx = place_monster_one(y, x, 477, 0, FALSE, MSTATUS_ENEMY);
			if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
			--i;
		}
	}

	/* Place some random monsters to haunt us */
	for (i = damroll(4, 4); i > 0; )
	{
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		auto const flags = f_info[cave[y][x].feat].flags;
		if (!(flags & FF_PERMANENT) && (flags & FF_FLOOR))
		{
			int monsters[22] = { 65, 100, 124, 125, 133, 231, 273, 327, 365, 416, 418,
			                     507, 508, 533, 534, 553, 554, 555, 577, 607, 622, 665};
			int monster = monsters[rand_int(22)];
			m_idx = place_monster_one(y, x, monster, 0, FALSE, MSTATUS_ENEMY);
			m_list[m_idx].mflag |= MFLAG_QUEST;
			--i;
		}
	}

	process_hooks_restart = TRUE;

	return true;
}

static bool quest_haunted_death_hook(void *, void *, void *)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_HAUNTED)
	{
		return false;
	}

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx)
		{
			continue;
		}

		if (m_ptr->status <= MSTATUS_ENEMY)
		{
			mcnt++;
		}
	}

	/* Nobody left ? */
	if (mcnt <= 1)
	{
		quest[p_ptr->inside_quest].status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_haunted_death_hook);
		del_hook_new(HOOK_GEN_QUEST,     quest_haunted_gen_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "Minas Anor is safer now.");
		return false;
	}

	return false;
}

static bool quest_haunted_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_HAUNTED)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "Thank you for saving us!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the building as your house as a reward.", 9, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_BETWEEN;

	return true;
}

void quest_haunted_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_haunted_death_hook,  "haunted_monster_death", NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_haunted_finish_hook, "haunted_finish",        NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_haunted_gen_hook,    "haunted_geb",           NULL);
	}
}
