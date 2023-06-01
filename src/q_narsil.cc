#include "q_narsil.hpp"

#include "cave_type.hpp"
#include "hook_chardump_in.hpp"
#include "hook_identify_in.hpp"
#include "hook_move_in.hpp"
#include "hooks.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-term.hpp"

#define cquest (quest[QUEST_NARSIL])

static bool quest_narsil_move_hook(void *, void *in_, void *)
{
	struct hook_move_in *in = static_cast<struct hook_move_in *>(in_);
	s32b y = in->y;
	s32b x = in->x;
	cave_type *c_ptr = &cave[y][x];
	int i;
	object_type *o_ptr;

	if (cquest.status != QUEST_STATUS_TAKEN)
	{
		return false;
	}

	/* The castle of Aragorn */
	if ((c_ptr->feat != FEAT_SHOP) || (c_ptr->special != 14))
	{
		return false;
	}

	/* Look out for Narsil */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->k_ptr)
		{
			continue;
		}

		if (o_ptr->name1 == ART_NARSIL)
		{
			break;
		}
	}

	if (i == INVEN_TOTAL)
	{
		return false;
	}

	cmsg_print(TERM_YELLOW, "I heard that the broken sword had been found!");
	cmsg_print(TERM_YELLOW, "I thought it was only a rumor... until now.");
	cmsg_print(TERM_YELLOW, "What you have is really the sword that was broken.");
	cmsg_print(TERM_YELLOW, "I will get it reforged...");
	cmsg_print(TERM_L_BLUE, "Aragorn leaves for a few hours then comes back...");
	cmsg_print(TERM_YELLOW, "Here it is, Anduril, the sword that was forged and is");
	cmsg_print(TERM_YELLOW, "reforged again. Take it; you will surely need it for your quest.");

	object_prep(o_ptr, lookup_kind(TV_SWORD, SV_LONG_SWORD));
	o_ptr->name1 = ART_ANDURIL;
	apply_magic(o_ptr, -1, true, true, true);
	object_aware(o_ptr);
	object_known(o_ptr);
	inven_item_describe(i);
	inven_item_optimize(i);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_PLAYER | PW_INVEN);

	/* Continue the plot */
	cquest.status = QUEST_STATUS_FINISHED;

	del_hook_new(HOOK_MOVE, quest_narsil_move_hook);
	process_hooks_restart = true;

	return true;
}

static bool quest_narsil_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(f, "\n The sword that was broken is now reforged.");
	}
	return false;
}

static bool quest_narsil_identify_hook(void *, void *in_, void *)
{
	struct hook_identify_in *in = static_cast<struct hook_identify_in *>(in_);

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		if (in->o_ptr->name1 == ART_NARSIL)
		{
			cquest.status = QUEST_STATUS_TAKEN;

			for (int i = 0; i < 5; i++)
			{
				if (quest[QUEST_NARSIL].desc[i][0] != '\0')
				{
					cmsg_print(TERM_YELLOW, quest[QUEST_NARSIL].desc[i]);
				}
			}

			add_hook_new(HOOK_MOVE, quest_narsil_move_hook, "narsil_move", NULL);
			del_hook_new(HOOK_IDENTIFY, quest_narsil_identify_hook);
			process_hooks_restart = true;
		}
	}

	return false;
}

void quest_narsil_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MOVE, quest_narsil_move_hook, "narsil_move", NULL);
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook_new(HOOK_IDENTIFY, quest_narsil_identify_hook, "narsil_id", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP, quest_narsil_dump_hook, "narsil_dump", NULL);
}
