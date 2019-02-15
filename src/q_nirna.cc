#include "q_nirna.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-term.hpp"

#define cquest (quest[QUEST_NIRNAETH])

static bool quest_nirnaeth_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_NIRNAETH)
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
	process_dungeon_file("nirnaeth.map", &ystart, &xstart, cur_hgt, cur_wid, true, true);

	/* Count the number of monsters */
	cquest.data[0] = 0;
	cquest.data[1] = 0;
	for (x = 2; x < xstart; x++)
	{
		for (y = 2; y < ystart; y++)
		{
			if (cave[y][x].m_idx) cquest.data[0]++;
		}
	}

	return true;
}

static bool quest_nirnaeth_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_NIRNAETH)
	{
		return false;
	}

	/* Killed at least 2/3 of them ? better reward ! */
	if (cquest.data[1] >= (2 * cquest.data[0] / 3))
	{
		c_put_str(TERM_YELLOW, "Not only did you found a way out, but you also destroyed a good", 8, 0);
		c_put_str(TERM_YELLOW, "number of trolls! Thank you so much. Take this gold please.", 9, 0);
		c_put_str(TERM_YELLOW, "I also grant you access to the royal jewelry shop!", 10, 0);

		p_ptr->au += 200000;

		/* Redraw gold */
		p_ptr->redraw |= (PR_FRAME);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
	else
	{
		c_put_str(TERM_YELLOW, "I thank you for your efforts.", 8, 0);
		c_put_str(TERM_YELLOW, "I grant you access to the royal jewelry shop!", 9, 0);
	}

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FINISH, quest_nirnaeth_finish_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_nirnaeth_death_hook(void *, void *, void *)
{
	if (p_ptr->inside_quest != QUEST_NIRNAETH) return false;

	cquest.data[1]++;

	return false;
}

static bool quest_nirnaeth_stair_hook(void *, void *, void *)
{
	if (p_ptr->inside_quest != QUEST_NIRNAETH)
	{
		return false;
	}

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS)
	{
		return false;
	}

	cmsg_print(TERM_YELLOW, "You found a way out!");
	cquest.status = QUEST_STATUS_COMPLETED;

	del_hook_new(HOOK_STAIR, quest_nirnaeth_stair_hook);
	process_hooks_restart = true;
	return false;
}

void quest_nirnaeth_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_nirnaeth_death_hook,  "nirnaeth_death",  NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_nirnaeth_gen_hook,    "nirnaeth_gen",    NULL);
		add_hook_new(HOOK_STAIR,         quest_nirnaeth_stair_hook,  "nirnaeth_stair",  NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_nirnaeth_finish_hook, "nirnaeth_finish", NULL);
	}
}
