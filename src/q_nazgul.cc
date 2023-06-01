#include "q_nazgul.hpp"

#include "cave.hpp"
#include "hook_chardump_in.hpp"
#include "hook_init_quest_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hook_wild_gen_in.hpp"
#include "hooks.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <cassert>

#define cquest (quest[QUEST_NAZGUL])

GENERATE_MONSTER_LOOKUP_FN(get_uvatha, "Uvatha the Horseman")

static bool quest_nazgul_gen_hook(void *, void *in_, void *)
{
	auto in = static_cast<struct hook_wild_gen_in const *>(in_);
	int m_idx, x = 1, y = 1, tries = 10000;

	if ((cquest.status != QUEST_STATUS_TAKEN) || in->small || (p_ptr->town_num != 1))
	{
		return false;
	}

	/* Find a good position */
	while (tries)
	{
		/* Get a random spot */
		y = randint(cur_hgt - 4) + 2;
		x = randint(cur_wid - 4) + 2;

		/* Is it a good spot ? */
		/* Not in player los */
		if ((!los(p_ptr->py, p_ptr->px, y, x)) && cave_empty_bold(y, x)) break;

		/* One less try */
		tries--;
	}

	/* Place the nazgul */
	int r_idx = get_uvatha();

	m_allow_special[r_idx] = true;
	m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_ENEMY);
	m_allow_special[r_idx] = false;

	if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;

	return false;
}

static bool quest_nazgul_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

	if (q_idx != QUEST_NAZGUL)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "I believe he will not come back! Thank you.", 8, 0);
	c_put_str(TERM_YELLOW, "Some time ago a ranger gave me this.", 9, 0);
	c_put_str(TERM_YELLOW, "I believe it will help you on your quest.", 10, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_ATHELAS));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->number = 6;
	inven_carry(q_ptr, false);

	/* End the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FINISH, quest_nazgul_finish_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_nazgul_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(f, "\n You saved Bree from a dreadful Nazgul.");
	}
	return false;
}

static bool quest_nazgul_forbid_hook(void *, void *in_, void *)
{
	struct hook_init_quest_in *in = static_cast<struct hook_init_quest_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_NAZGUL)
	{
		return false;
	}

	if (p_ptr->lev < 30)
	{
		c_put_str(TERM_WHITE, "I fear you are not ready for the next quest, come back later.", 8, 0);
		return true;
	}

	return false;
}

static bool quest_nazgul_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}

	if (r_idx != get_uvatha())
	{
		return false;
	}

	cquest.status = QUEST_STATUS_COMPLETED;

	del_hook_new(HOOK_MONSTER_DEATH, quest_nazgul_death_hook);
	process_hooks_restart = true;

	return false;
}

void quest_nazgul_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_nazgul_death_hook,  "nazgul_death",  NULL);
		add_hook_new(HOOK_WILD_GEN,      quest_nazgul_gen_hook,    "nazgul_gen",    NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_nazgul_finish_hook, "nazgul_finish", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP,  quest_nazgul_dump_hook,   "nazgul_dump", NULL);
	add_hook_new(HOOK_INIT_QUEST, quest_nazgul_forbid_hook, "nazgul_forbid", NULL);
}
