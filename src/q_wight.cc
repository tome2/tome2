#include "q_wight.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "hook_monster_death_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <cassert>

#define cquest (quest[QUEST_WIGHT])

GENERATE_MONSTER_LOOKUP_FN(get_wight_king, "The Wight-King of the Barrow-downs")

static bool quest_wight_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_WIGHT)
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
	reset_get_monster_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("wights.map", &ystart, &xstart, cur_hgt, cur_wid, true, true);

	for (x = 3; x < xstart; x++)
	{
		for (y = 3; y < ystart; y++)
		{
			if (cave[y][x].feat == FEAT_MARKER)
			{
				int m_idx = 0;

				m_allow_special[get_wight_king()] = true;
				m_idx = place_monster_one(y, x, get_wight_king(), 0, false, MSTATUS_ENEMY);
				m_allow_special[get_wight_king()] = false;

				if (m_idx)
				{
					int o_idx;

					/* Get local object */
					object_type forge, *q_ptr = &forge;

					m_list[m_idx].mflag |= MFLAG_QUEST;

					/* Prepare to make the  */
					object_prep(q_ptr, lookup_kind(TV_SOFT_ARMOR, SV_FILTHY_RAG));

					/* Name the rags */

					q_ptr->artifact_name = "of the Wight";

					q_ptr->art_flags |=
						TR_INT |
						TR_RES_BLIND |
						TR_SENS_FIRE |
						TR_RES_CONF |
						TR_IGNORE_ACID |
						TR_IGNORE_ELEC |
						TR_IGNORE_FIRE |
						TR_IGNORE_COLD |
						TR_SEE_INVIS |
						TR_CURSED |
						TR_HEAVY_CURSE;

					if (randint(2) == 1)
					{
						q_ptr->art_flags |= TR_SPELL;
						q_ptr->pval = 6;
					}
					else
					{
						q_ptr->art_flags |= TR_MANA;
						q_ptr->pval = 2;
					}

					/* Get new object */
					o_idx = o_pop();

					if (o_idx)
					{
						/* Get the item */
						object_type *o_ptr = &o_list[o_idx];

						/* Structure copy */
						object_copy(o_ptr, q_ptr);

						/* Build a stack */
						o_ptr->held_m_idx = m_idx;
						o_ptr->ix = 0;
						o_ptr->iy = 0;

						m_list[m_idx].hold_o_idxs.push_back(o_idx);
					}
				}
			}
		}
	}

	return true;
}

static bool quest_wight_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_WIGHT)
	{
		return false;
	}

	if (r_idx == get_wight_king())
	{
		cmsg_print(TERM_YELLOW, "Without their King the wights won't be able to do much.");

		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
		cave[p_ptr->py][p_ptr->px].special = 0;

		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_wight_death_hook);
		process_hooks_restart = true;

		return false;
	}

	return false;
}

static bool quest_wight_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_WIGHT)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "I heard about your noble deeds.", 8, 0);
	c_put_str(TERM_YELLOW, "Keep what you found ..  may it serve you well.", 9, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NAZGUL;
	quest[*(quest[q_idx].plot)].init();

	del_hook_new(HOOK_QUEST_FINISH, quest_wight_finish_hook);
	process_hooks_restart = true;

	return true;
}

void quest_wight_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_wight_death_hook,  "wight_death",  NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_wight_gen_hook,    "wight_gen",    NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_wight_finish_hook, "wight_finish", NULL);
	}
}
