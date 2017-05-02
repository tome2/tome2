#include "q_spider.hpp"

#include "cave.hpp"
#include "gods.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"

#define cquest (quest[QUEST_SPIDER])

static bool quest_spider_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_SPIDER)
	{
		return false;
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
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("spiders.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	return true;
}

static bool quest_spider_death_hook(void *, void *, void *)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_SPIDER)
	{
		return false;
	}

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

	if (mcnt <= 1)
	{
		cmsg_print(TERM_YELLOW, "The forest is now safer, thanks to you.");

		/* Yavanna LOVES saving forests */
		if (p_ptr->pgod == GOD_YAVANNA)
		{
			cmsg_print(TERM_L_GREEN, "You feel the gentle touch of Yavanna, as she smiles at you.");
			inc_piety(GOD_YAVANNA, 6000);
		}

		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_spider_death_hook);
		process_hooks_restart = TRUE;

		return false;
	}

	return false;
}

static bool quest_spider_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	object_type forge, *q_ptr;
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_SPIDER)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "All of us praise your mighty deed in driving back the", 8, 0);
	c_put_str(TERM_YELLOW, "menace. Take this as a reward.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_POTION, SV_POTION_AUGMENTATION));
	q_ptr->number = 1;
	q_ptr->found = OBJ_FOUND_REWARD;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_STOREB;
	inven_carry(q_ptr, FALSE);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_POISON;
	quest[*(quest[q_idx].plot)].init();

	del_hook_new(HOOK_QUEST_FINISH, quest_spider_finish_hook);
	process_hooks_restart = TRUE;

	return true;
}

void quest_spider_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_spider_death_hook,  "spider_death",  NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_spider_gen_hook,    "spider_gen",    NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_spider_finish_hook, "spider_finish", NULL);
	}
}
