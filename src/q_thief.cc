#include "q_thief.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "game.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "skill_type.hpp"
#include "spells2.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

#define cquest (quest[QUEST_THIEVES])

static bool quest_thieves_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;
	bool_ again = TRUE;

	if (p_ptr->inside_quest != QUEST_THIEVES)
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
	process_dungeon_file("thieves.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	dungeon_flags |= DF_NO_GENO;

	/* Rip the inventory from the player */
	cmsg_print(TERM_YELLOW, "You feel a vicious blow on your head.");
	while (again)
	{
		again = FALSE;
		for (x = 0; x < INVEN_TOTAL; x++)
		{
			object_type *o_ptr = &p_ptr->inventory[x];

			if (!o_ptr->k_ptr)
			{
				continue;
			}

			if ((x >= INVEN_WIELD) && cursed_p(o_ptr))
			{
				continue;
			}

			inven_drop(x, 99, 4, 24, TRUE);

			/* Thats ugly .. but it works */
			again = TRUE;
			break;
		}
	}

	del_hook_new(HOOK_GEN_QUEST, quest_thieves_gen_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

static bool quest_thieves_hook(void *, void *, void *)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_THIEVES)
	{
		return false;
	}

	/* ALARM !!! */
	if ((cave[17][22].feat == FEAT_OPEN) ||
	                (cave[17][22].feat == FEAT_BROKEN))
	{
		cmsg_print(TERM_L_RED, "An alarm rings!");
		aggravate_monsters(0);
		cave_set_feat(14, 20, FEAT_OPEN);
		cave_set_feat(14, 16, FEAT_OPEN);
		cave_set_feat(14, 12, FEAT_OPEN);
		cave_set_feat(14, 8, FEAT_OPEN);
		cave_set_feat(20, 20, FEAT_OPEN);
		cave_set_feat(20, 16, FEAT_OPEN);
		cave_set_feat(20, 12, FEAT_OPEN);
		cave_set_feat(20, 8, FEAT_OPEN);
		msg_print("The door explodes.");
		cave_set_feat(17, 22, FEAT_FLOOR);
	}

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status < MSTATUS_FRIEND) mcnt++;
	}

	/* Nobody left ? */
	if (!mcnt)
	{
		msg_print("The magic hiding the stairs is now gone.");
		cave_set_feat(23, 4, FEAT_LESS);
		cave[23][4].special = 0;

		quest[p_ptr->inside_quest].status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_END_TURN, quest_thieves_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "You stopped the thieves and saved Bree!");
		return false;
	}
	return false;
}

static bool quest_thieves_finish_hook(void *, void *in_, void *)
{
	auto const &s_info = game->s_info;

	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_THIEVES)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "Thank you for killing the band of thieves!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the hideout as your house as a reward.", 9, 0);

	/* Continue the plot */

	/* 10% chance to randomly select, otherwise use the combat/magic skill ratio */
	if (magik(10) || (s_info[SKILL_COMBAT].value == s_info[SKILL_MAGIC].value))
	{
		*(quest[q_idx].plot) = (magik(50)) ? QUEST_TROLL : QUEST_WIGHT;
	}
	else
	{
		if (s_info[SKILL_COMBAT].value > s_info[SKILL_MAGIC].value)
		{
			*(quest[q_idx].plot) = QUEST_TROLL;
		}
		else
		{
			*(quest[q_idx].plot) = QUEST_WIGHT;
		}
	}
	quest[*(quest[q_idx].plot)].init();

	del_hook_new(HOOK_QUEST_FINISH, quest_thieves_finish_hook);
	process_hooks_restart = TRUE;

	return true;
}

static bool quest_thieves_feeling_hook(void *, void *, void *)
{
	if (p_ptr->inside_quest != QUEST_THIEVES)
	{
		return false;
	}

	msg_print("You wake up in a prison cell.");
	msg_print("All your possessions have been stolen!");

	del_hook_new(HOOK_FEELING, quest_thieves_feeling_hook);
	process_hooks_restart = TRUE;

	return true;
}

void quest_thieves_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_END_TURN,     quest_thieves_hook,         "thieves_end_turn", NULL);
		add_hook_new(HOOK_QUEST_FINISH, quest_thieves_finish_hook,  "thieves_finish",   NULL);
		add_hook_new(HOOK_GEN_QUEST,    quest_thieves_gen_hook,     "thieves_geb",      NULL);
		add_hook_new(HOOK_FEELING,      quest_thieves_feeling_hook, "thieves_feel",     NULL);
	}
}
