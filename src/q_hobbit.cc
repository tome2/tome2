#include "q_hobbit.hpp"

#include "cave.hpp"
#include "game.hpp"
#include "hook_chardump_in.hpp"
#include "hook_chat_in.hpp"
#include "hook_give_in.hpp"
#include "hook_mon_speak_in.hpp"
#include "hook_wild_gen_in.hpp"
#include "hooks.hpp"
#include "messages.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-form.h"
#include "z-rand.hpp"
#include "z-term.h"

#include <cassert>

#define cquest (quest[QUEST_HOBBIT])

GENERATE_MONSTER_LOOKUP_FN(get_melinda_proudfoot, "Melinda Proudfoot")
GENERATE_MONSTER_LOOKUP_FN(get_merton_proudfoot, "Merton Proudfoot, the lost hobbit")

static bool quest_hobbit_town_gen_hook(void *, void *in_, void *)
{
	struct hook_wild_gen_in *in = static_cast<struct hook_wild_gen_in *>(in_);
	int x = 1, y = 1, tries = 10000;
	bool_ small = in->small;

	if ((turn < (cquest.data[1] + (DAY * 10L))) || (cquest.status > QUEST_STATUS_COMPLETED) || (small) || (p_ptr->town_num != 1))
	{
		return false;
	}

	/* Find a good position */
	while (tries)
	{
		/* Get a random spot */
		y = randint(20) + (cur_hgt / 2) - 10;
		x = randint(20) + (cur_wid / 2) - 10;

		/* Is it a good spot ? */
		/* Not in player los, and avoid shop grids */
		if (!los(p_ptr->py, p_ptr->px, y, x) && cave_empty_bold(y, x) &&
		                cave_plain_floor_bold(y, x)) break;

		/* One less try */
		tries--;
	}

	/* Place Melinda */
	int r_idx = get_melinda_proudfoot();
	m_allow_special[r_idx] = TRUE;
	place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_ENEMY);
	m_allow_special[r_idx] = FALSE;

	return false;
}

static bool quest_hobbit_gen_hook(void *, void *, void *)
{
	int x = 1, y = 1, tries = 10000;

	if ((cquest.status != QUEST_STATUS_TAKEN) || (dun_level != cquest.data[0]) || (dungeon_type != DUNGEON_MAZE))
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
		if (cave_empty_bold(y, x)) break;

		/* One less try */
		tries--;
	}

	/* Place the hobbit */
	int r_idx = get_merton_proudfoot();
	m_allow_special[r_idx] = TRUE;
	place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_FRIEND);
	m_allow_special[r_idx] = FALSE;

	return false;
}

static bool quest_hobbit_give_hook(void *, void *in_, void *)
{
	struct hook_give_in *in = static_cast<struct hook_give_in *>(in_);
	object_type *o_ptr;
	monster_type *m_ptr;
	s32b m_idx = in->m_idx;
	s32b item = in->item;

	o_ptr = &p_ptr->inventory[item];
	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != get_merton_proudfoot()) return false;

	if ((o_ptr->tval != TV_SCROLL) || (o_ptr->sval != SV_SCROLL_WORD_OF_RECALL)) return false;

	msg_print("'Oh, thank you, noble one!'");
	msg_print("Merton Proudfoot reads the scroll and is recalled to the safety of his home.");

	delete_monster_idx(m_idx);

	inc_stack_size_ex(item, -1, OPTIMIZE, NO_DESCRIBE);

	cquest.status = QUEST_STATUS_COMPLETED;

	del_hook_new(HOOK_GIVE, quest_hobbit_give_hook);
	process_hooks_restart = TRUE;

	return true;
}

static bool quest_hobbit_speak_hook(void *, void *in_, void *)
{
	struct hook_mon_speak_in *in = static_cast<struct hook_mon_speak_in *>(in_);
	s32b m_idx = in->m_idx;

	if (m_list[m_idx].r_idx != get_melinda_proudfoot())
	{
		return false;
	}

	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		msg_format("%^s begs for your help.", in->m_name);
	}
	return true;
}

static bool quest_hobbit_chat_hook(void *, void *in_, void *)
{
	struct hook_chat_in *in = static_cast<struct hook_chat_in *>(in_);
	s32b m_idx = in->m_idx;
	monster_type *m_ptr;

	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != get_melinda_proudfoot())
	{
		return false;
	}

	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		msg_print("Oh! Oh!");
		msg_print("My poor Merton, where is my poor Merton? He was playing near that dreadful");
		msg_print("maze and never been seen again! Could you find him for me?");

		cquest.status = QUEST_STATUS_TAKEN;
		quest[QUEST_HOBBIT].init();
	}
	else if (cquest.status == QUEST_STATUS_COMPLETED)
	{
		object_type forge, *q_ptr;

		msg_print("My Merton is back! You saved him, hero.");
		msg_print("Take this as a proof of my gratitude.  It was given to my family");
		msg_print("by a famed wizard, but it should serve you better than me.");

		q_ptr = &forge;
		object_prep(q_ptr, lookup_kind(TV_ROD, SV_ROD_RECALL));
		q_ptr->number = 1;
		q_ptr->found = OBJ_FOUND_REWARD;
		inven_carry(q_ptr, FALSE);

		cquest.status = QUEST_STATUS_FINISHED;

		del_hook_new(HOOK_MON_SPEAK, quest_hobbit_speak_hook);
		process_hooks_restart = TRUE;
		delete_monster_idx(m_idx);

		return true;
	}
	else
	{
		msg_print("Thanks again.");
	}

	return true;
}

static bool quest_hobbit_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(f, "\n You saved a young hobbit from an horrible fate.");
	}
	return false;
}

void quest_hobbit_init_hook()
{
	auto &messages = game->messages;

	/* Get a level to place the hobbit */
	if (!cquest.data[0])
	{
		cquest.data[0] = rand_range(26, 34);
		cquest.data[1] = turn;
		if (wizard)
		{
			messages.add(format("Hobbit level %d", cquest.data[0]), TERM_BLUE);
		}
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_GIVE,      quest_hobbit_give_hook,     "hobbit_give",     NULL);
		add_hook_new(HOOK_GEN_LEVEL, quest_hobbit_gen_hook,      "hobbit_gen",      NULL);
		add_hook_new(HOOK_WILD_GEN,  quest_hobbit_town_gen_hook, "hobbit_town_gen", NULL);
		add_hook_new(HOOK_CHAT,      quest_hobbit_chat_hook,     "hobbit_chat",     NULL);
		add_hook_new(HOOK_MON_SPEAK, quest_hobbit_speak_hook,    "hobbit_speak",    NULL);
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook_new(HOOK_MON_SPEAK, quest_hobbit_speak_hook,    "hobbit_speak",    NULL);
		add_hook_new(HOOK_WILD_GEN,  quest_hobbit_town_gen_hook, "hobbit_town_gen", NULL);
		add_hook_new(HOOK_CHAT,      quest_hobbit_chat_hook,     "hobbit_chat",     NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP, quest_hobbit_dump_hook, "hobbit_dump", NULL);
}
