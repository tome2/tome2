#include "q_poison.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "game.hpp"
#include "hook_chardump_in.hpp"
#include "hook_drop_in.hpp"
#include "hook_init_quest_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "messages.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <fmt/format.h>

#define cquest (quest[QUEST_POISON])

static int wild_locs[4][2] =
{
	{ 32, 49, },
	{ 32, 48, },
	{ 33, 48, },
	{ 34, 48, },
};

static bool create_molds_hook(monster_race const *r_ptr)
{
	if (r_ptr->spells & SF_MULTIPLY) return false;

	if (r_ptr->d_char == 'm') return true;
	if (r_ptr->d_char == ',') return true;
	if (r_ptr->d_char == 'e') return true;

	return false;
}

static bool quest_poison_gen_hook(void *, void *, void *)
{
	int cy = 1, cx = 1, x, y, tries = 10000, r_idx;
	bool (*old_get_monster_hook)(monster_race const *);

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}
	if (p_ptr->wilderness_y != wild_locs[cquest.data[0]][0])
	{
		return false;
	}
	if (p_ptr->wilderness_x != wild_locs[cquest.data[0]][1])
	{
		return false;
	}
	if (p_ptr->wild_mode)
	{
		return false;
	}

	/* Find a good position */
	while (tries)
	{
		/* Get a random spot */
		cy = randint(cur_hgt - 24) + 22;
		cx = randint(cur_wid - 34) + 32;

		/* Is it a good spot ? */
		if (cave_empty_bold(cy, cx))
		{
			break;
		}

		/* One less try */
		tries--;
	}

	/* Place the baddies */

	/* Backup the old hook */
	old_get_monster_hook = get_monster_hook;

	/* Require "okay" monsters */
	get_monster_hook = create_molds_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick a monster, using the level calculation */
	for (x = cx - 25; x <= cx + 25; x++)
	{
		for (y = cy - 25; y <= cy + 25; y++)
		{
			if (!in_bounds(y, x))
			{
				continue;
			}

			if (distance(cy, cx, y, x) > 25)
			{
				continue;
			}

			if (magik(80) && ((cave[y][x].feat == FEAT_DEEP_WATER) || (cave[y][x].feat == FEAT_SHAL_WATER)))
			{
				cave_set_feat(y, x, FEAT_TAINTED_WATER);
			}

			if (distance(cy, cx, y, x) > 10)
			{
				continue;
			}

			if (magik(60))
			{
				int m_idx;

				r_idx = get_mon_num(30);
				m_idx = place_monster_one(y, x, r_idx, 0, false, MSTATUS_ENEMY);
				if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;

				/* Sometimes make it up some levels */
				if (magik(80) && m_idx)
				{
					monster_type *m_ptr = &m_list[m_idx];

					if (m_ptr->level < p_ptr->lev)
					{
						m_ptr->exp = monster_exp(m_ptr->level + randint(p_ptr->lev - m_ptr->level));
						monster_check_experience(m_idx, true);
					}
				}
			}
		}
	}

	/* Reset restriction */
	get_monster_hook = old_get_monster_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	return false;
}

static bool quest_poison_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

	if (q_idx != QUEST_POISON)
	{
		return false;
	}

	c_put_str(TERM_YELLOW, "The water is clean again! Thank you so much.", 8, 0);
	c_put_str(TERM_YELLOW, "The beautiful Mallorns are safe. Take this as a proof of our gratitude.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_DRAG_ARMOR, SV_DRAGON_BLUE));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->number = 1;
	q_ptr->name2 = EGO_ELVENKIND;
	apply_magic(q_ptr, 1, false, false, false);

	inven_carry(q_ptr, false);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FINISH, quest_poison_finish_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_poison_dump_hook(void *, void *in_, void *)
{
	hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(f, "\n You saved the beautiful Mallorns of Lothlorien.");
	}
	return false;
}

static bool quest_poison_quest_hook(void *, void *in_, void *)
{
	struct hook_init_quest_in *in = static_cast<struct hook_init_quest_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

	if (q_idx != QUEST_POISON)
	{
		return false;
	}

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_POTION2, SV_POTION2_CURE_WATER));
	q_ptr->number = 99;
	q_ptr->inscription = "quest";

	inven_carry(q_ptr, false);

	del_hook_new(HOOK_INIT_QUEST, quest_poison_quest_hook);
	process_hooks_restart = true;

	return false;
}

static bool quest_poison_drop_hook(void *, void *in_, void *)
{
	struct hook_drop_in *in = static_cast<struct hook_drop_in *>(in_);
	s32b mcnt = 0, i, x, y;
	s32b o_idx = in->o_idx;
	object_type *o_ptr = &p_ptr->inventory[o_idx];

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}
	if (p_ptr->wilderness_y != wild_locs[cquest.data[0]][0])
	{
		return false;
	}
	if (p_ptr->wilderness_x != wild_locs[cquest.data[0]][1])
	{
		return false;
	}
	if (p_ptr->wild_mode)
	{
		return false;
	}

	if (o_ptr->tval != TV_POTION2)
	{
		return false;
	}
	if (o_ptr->sval != SV_POTION2_CURE_WATER)
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

		if (m_ptr->status <= MSTATUS_NEUTRAL)
		{
			mcnt++;
		}
	}

	if (mcnt < 10)
	{
		for (x = 1; x < cur_wid - 1; x++)
		{
			for (y = 1; y < cur_hgt - 1; y++)
			{
				if (!in_bounds(y, x))
				{
					continue;
				}

				if (cave[y][x].feat == FEAT_TAINTED_WATER)
				{
					cave_set_feat(y, x, FEAT_SHAL_WATER);
				}
			}
		}

		cmsg_print(TERM_YELLOW, "Well done! The water seems to be clean now.");

		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_DROP, quest_poison_drop_hook);
		process_hooks_restart = true;

		return false;
	}
	else
	{
		msg_print("There are too many monsters left to cure the water.");
		return true;
	}
	return false;
}

void quest_poison_init_hook()
{
	auto &messages = game->messages;

	/* Get a place to place the poison */
	if (!cquest.data[1])
	{
		cquest.data[1] = true;
		cquest.data[0] = rand_int(4);
		if (wizard)
		{
			messages.add(fmt::format("Wilderness poison {}, {}", wild_locs[cquest.data[0]][0], wild_locs[cquest.data[0]][1]), TERM_BLUE);
		}
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_DROP,         quest_poison_drop_hook,   "poison_drop",   NULL);
		add_hook_new(HOOK_WILD_GEN,     quest_poison_gen_hook,    "poison_gen",    NULL);
		add_hook_new(HOOK_QUEST_FINISH, quest_poison_finish_hook, "poison_finish", NULL);
	}
	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		add_hook_new(HOOK_INIT_QUEST, quest_poison_quest_hook, "poison_iquest", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP, quest_poison_dump_hook, "poison_dump", NULL);
}
