#include "q_invas.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "hook_chardump_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hook_monster_ai_in.hpp"
#include "hook_monster_ai_out.hpp"
#include "hook_stair_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-term.hpp"

#define cquest (quest[QUEST_INVASION])

static bool quest_invasion_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_INVASION)
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
	process_dungeon_file("maeglin.map", &ystart, &xstart, cur_hgt, cur_wid, true, true);

	for (x = 3; x < xstart; x++)
	{
		for (y = 3; y < ystart; y++)
		{
			if (cave[y][x].feat == FEAT_MARKER)
			{
				cquest.data[0] = y;
				cquest.data[1] = x;
				p_ptr->py = y;
				p_ptr->px = x;
				cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
			}
		}
	}

	return true;
}

static bool quest_invasion_ai_hook(void *, void *in_, void *out_)
{
	struct hook_monster_ai_in *in = static_cast<struct hook_monster_ai_in *>(in_);
	struct hook_monster_ai_out *out = static_cast<struct hook_monster_ai_out *>(out_);
	monster_type *m_ptr = in->m_ptr;
	s32b m_idx = in->m_idx;

	if (p_ptr->inside_quest != QUEST_INVASION)
	{
		return false;
	}

	/* Ugly but thats better than a call to test_monster_name which is SLOW */
	if (m_ptr->r_idx == 825)
	{
		/* Oups he fleed */
		if ((m_ptr->fy == cquest.data[0]) && (m_ptr->fx == cquest.data[1]))
		{
			delete_monster_idx(m_idx);

			cmsg_print(TERM_YELLOW, "Maeglin found the way to Gondolin! All hope is lost now!");
			cquest.status = QUEST_STATUS_FAILED;
			town_info[2].destroyed = true;
			return false;
		}

		/* Attack or flee ?*/
		if (distance(m_ptr->fy, m_ptr->fx, p_ptr->py, p_ptr->px) <= 2)
		{
			return false;
		}
		else
		{
			out->y = cquest.data[0];
			out->x = cquest.data[1];
			return true;
		}
	}

	return false;
}

static bool quest_invasion_turn_hook(void *, void *, void *)
{
	if (cquest.status != QUEST_STATUS_UNTAKEN) return false;
	if (p_ptr->lev < 45) return false;

	/* Wait until the end of the current quest */
	if (p_ptr->inside_quest) return ( false);

	/* Wait until the end of the astral mode */
	if (p_ptr->astral) return ( false);

	/* Ok give the quest */
	cmsg_print(TERM_YELLOW, "A Thunderlord appears in front of you and says:");
	cmsg_print(TERM_YELLOW, "'Hello, noble hero. I am Liron, rider of Tolan. Turgon, King of Gondolin sent me.'");
	cmsg_print(TERM_YELLOW, "'Gondolin is being invaded; he needs your help now or everything will be lost.'");
	cmsg_print(TERM_YELLOW, "'Please come quickly!'");

	cquest.status = QUEST_STATUS_TAKEN;

	quest_invasion_init_hook();
	del_hook_new(HOOK_END_TURN, quest_invasion_turn_hook);
	process_hooks_restart = true;

	return false;
}

static bool quest_invasion_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status == QUEST_STATUS_FAILED)
	{
		fprintf(f, "\n You abandoned Gondolin when it most needed you, thus causing its destruction.");
	}
	if ((cquest.status == QUEST_STATUS_FINISHED) || (cquest.status == QUEST_STATUS_REWARDED) || (cquest.status == QUEST_STATUS_COMPLETED))
	{
		fprintf(f, "\n You saved Gondolin from destruction.");
	}
	return false;
}

static bool quest_invasion_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_INVASION)
	{
		return false;
	}

	if (r_idx == test_monster_name("Maeglin, the Traitor of Gondolin"))
	{
		cmsg_print(TERM_YELLOW, "You did it! Gondolin will remain hidden.");
		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_invasion_death_hook);
		process_hooks_restart = true;

		return false;
	}

	return false;
}

static bool quest_invasion_stair_hook(void *, void *in_, void *)
{
	struct hook_stair_in *in = static_cast<struct hook_stair_in *>(in_);

	if (p_ptr->inside_quest != QUEST_INVASION) return false;

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS) return true;

	if (in->direction == STAIRS_UP)
	{
		if (cquest.status == QUEST_STATUS_FAILED)
		{
			cmsg_print(TERM_YELLOW, "The armies of Morgoth totally devastated Gondolin, leaving nothing but ruins...");
		}
		else if (cquest.status == QUEST_STATUS_COMPLETED)
		{
			cmsg_print(TERM_YELLOW, "Turgon appears before you and speaks:");
			cmsg_print(TERM_YELLOW, "'I will never be able to thank you enough.'");
			cmsg_print(TERM_YELLOW, "'My most powerful mages will cast a powerful spell for you, giving you extra life.'");

			p_ptr->hp_mod += 150;
			p_ptr->update |= (PU_HP);
			cquest.status = QUEST_STATUS_FINISHED;
		}
		else
		{
			/* Flush input */
			flush();

			if (!get_check("Really abandon the quest?")) return true;
			cmsg_print(TERM_YELLOW, "You flee away from Maeglin and his army...");
			cquest.status = QUEST_STATUS_FAILED;
			town_info[2].destroyed = true;
		}
		del_hook_new(HOOK_STAIR, quest_invasion_stair_hook);
		process_hooks_restart = true;
		return false;
	}

	return true;
}

void quest_invasion_init_hook()
{
	add_hook_new(HOOK_END_TURN,  quest_invasion_turn_hook, "invasion_turn", NULL);
	add_hook_new(HOOK_CHAR_DUMP, quest_invasion_dump_hook, "invasion_dump", NULL);
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_AI,    quest_invasion_ai_hook,    "invasion_ai",    NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_invasion_gen_hook,   "invasion_gen",   NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_invasion_death_hook, "invasion_death", NULL);
		add_hook_new(HOOK_STAIR,         quest_invasion_stair_hook, "invasion_stair", NULL);
	}
}
