#include "q_ultrag.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "game.hpp"
#include "hook_chardump_in.hpp"
#include "hook_move_in.hpp"
#include "hook_stair_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-term.hpp"

#define cquest (quest[QUEST_ULTRA_GOOD])

static std::shared_ptr<object_kind> get_flame_imperishable()
{
	static auto &k_info = game->edit_data.k_info;
	static auto &k_flame_imperishable = k_info[test_item_name("& Flame~ Imperishable")];

	return k_flame_imperishable;
}

static bool quest_ultra_good_move_hook(void *, void *in_, void *)
{
	struct hook_move_in *in = static_cast<struct hook_move_in *>(in_);
	s32b y = in->y;
	s32b x = in->x;
	cave_type *c_ptr = &cave[y][x];

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		if (quest[QUEST_MORGOTH].status < QUEST_STATUS_FINISHED)
		{
			return false;
		}

		/* The mirror of Galadriel */
		if ((c_ptr->feat != FEAT_SHOP) || (c_ptr->special != 23))
		{
			return false;
		}

		auto old_quick_messages = options->quick_messages;
		options->quick_messages = false;

		cmsg_print(TERM_L_BLUE, "You meet Galadriel.");
		cmsg_print(TERM_YELLOW, "'I still cannot believe this is all over.'");
		cmsg_print(TERM_YELLOW, "'Morgoth's reign of terror is over at last!'");
		cmsg_print(TERM_YELLOW, "'His spirit has been banished to the Void where he cannot do much harm.'");
		cmsg_print(TERM_YELLOW, "'We can never thank you enough, hero!'");
		cmsg_print(TERM_L_BLUE, "Although everything seems all right, Galadriel seems a little subdued.");
		cmsg_print(TERM_YELLOW, "'The spirit of Morgoth is not destroyed however -- only banished.'");
		cmsg_print(TERM_YELLOW, "'He can still control his allies left on Arda.'");
		cmsg_print(TERM_YELLOW, "'Maybe... maybe there could be a way to remove the threat of evil forever.'");
		cmsg_print(TERM_YELLOW, "'Somebody would have to go into the Void to do it.'");
		cmsg_print(TERM_YELLOW, "'But going there is certain death; we cannot ask it of anyone.'");
		cmsg_print(TERM_YELLOW, "'But you may choose, of your own free will, to attempt it...'");
		cmsg_print(TERM_L_BLUE, "Galadriel plainly presents the choice that now lies before you:");

		cmsg_print(TERM_YELLOW, "'You have earned the right to make whatever you wish of your future.'");
		cmsg_print(TERM_YELLOW, "'Become a ruler of Arda if you so desire; reign long, enjoying'");
		cmsg_print(TERM_YELLOW, "'the adulation of all, and have a happy life. Or, you can turn your'");
		cmsg_print(TERM_YELLOW, "'back on safety. Enter the Void, alone, to fight a hopeless battle'");
		cmsg_print(TERM_YELLOW, "'and face certain death.'");

		/* This is SO important a question that flush pending inputs */
		flush();

		if (!get_check("Will you stay on Arda and lead a happy life?"))
		{
			cmsg_print(TERM_YELLOW, "'So be it. I will open a portal to the Void.'");
			cmsg_print(TERM_YELLOW, "'But you must know this: the portal can only lead one way.'");
			cmsg_print(TERM_YELLOW, "'It will close once you enter, so as not to permit the horrors'");
			cmsg_print(TERM_YELLOW, "'that lurk in the Void to enter Arda. Your only way to come back'");
			cmsg_print(TERM_YELLOW, "'is to defeat the spirit of Morgoth, known as Melkor.'");
			cmsg_print(TERM_YELLOW, "'You will not be able to recall back either.'");
			cmsg_print(TERM_YELLOW, "'You can still choose to retire; it is not too late'");
			cmsg_print(TERM_YELLOW, "'to save your life.'");
			cmsg_print(TERM_YELLOW, "'One last thing: It is quite certain that Melkor will have erected'");
			cmsg_print(TERM_YELLOW, "'powerful magical barriers around him. You certainly will'");
			cmsg_print(TERM_YELLOW, "'need to find a way to break them to get to him.'");

			/* Create the entrance */
			cave_set_feat(y - 5, x, FEAT_MORE);
			cave[y - 5][x].special = 11;

			/* Continue the plot */
			cquest.status = QUEST_STATUS_TAKEN;
			cquest.init();
		}

		options->quick_messages = old_quick_messages;

		return true;
	}

	return false;
}

static bool quest_ultra_good_stair_hook(void *, void *in_, void *)
{
	struct hook_stair_in *in = static_cast<struct hook_stair_in *>(in_);
	stairs_direction dir = in->direction;

	if (dungeon_type != DUNGEON_VOID)
	{
		return false;
	}

	/* Cant leave */
	if ((dir == STAIRS_UP) && (dun_level == 128))
	{
		cmsg_print(TERM_YELLOW, "The portal to Arda is now closed.");
		return true;
	}

	/* there is no coming back */
	if ((dir == STAIRS_UP) && (dun_level == 150))
	{
		cmsg_print(TERM_YELLOW, "The barrier seems to be impenetrable from this side.");
		cmsg_print(TERM_YELLOW, "You will have to move on.");
		return true;
	}

	/* Cant enter without the flame imperishable */
	if ((dir == STAIRS_DOWN) && (dun_level == 149))
	{
		int i;
		bool ultimate = false;

		/* Now look for an ULTIMATE artifact, that is, one imbued with the flame */
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			auto o_ptr = &p_ptr->inventory[i];

			if (!o_ptr->k_ptr)
			{
				continue;
			}

			auto const flags = object_flags(o_ptr);

			if (flags & TR_ULTIMATE)
			{
				ultimate = true;
				break;
			}
		}

		if (!ultimate)
		{
			cmsg_print(TERM_YELLOW, "It seems the level is protected by an impassable barrier of pure magic.");
			cmsg_print(TERM_YELLOW, "Only the most powerful magic could remove it. You will need to use");
			cmsg_print(TERM_YELLOW, "the Flame Imperishable to pass. The source of Eru Iluvatar's own power.");
			return true;
		}
		else
		{
			cmsg_print(TERM_YELLOW, "The power of the Flame Imperishable shatters the magical barrier.");
			cmsg_print(TERM_YELLOW, "The way before you is free.");
		}
	}

	return false;
}

static bool quest_ultra_good_recall_hook(void *, void *, void *)
{
	if ((dungeon_type != DUNGEON_VOID) && (dungeon_type != DUNGEON_NETHER_REALM))
	{
		return false;
	}

	cmsg_print(TERM_YELLOW, "You cannot recall. The portal to Arda is closed.");
	return true;
}

static bool quest_ultra_good_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;

	monster_type *m_ptr = &m_list[m_idx];

	/* Melkor is dead! */
	if (m_ptr->r_idx == 1044)
	{
		/* Total winner */
		total_winner = WINNER_ULTRA;
		has_won = WINNER_ULTRA;
		quest[QUEST_ULTRA_GOOD].status = QUEST_STATUS_FINISHED;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_FRAME);

		/* Congratulations */
		cmsg_print(TERM_L_GREEN, "****** CONGRATULATIONS ******");
		cmsg_print(TERM_L_GREEN, "You have done more than the impossible. You ended the threat of");
		cmsg_print(TERM_L_GREEN, "Melkor forever. Thanks to you, Arda will live in eternal peace.");
		cmsg_print(TERM_L_GREEN, "You feel the spirit of Eru touching you. You feel your spirit rising!");
		cmsg_print(TERM_L_GREEN, "Before you, a portal to Arda opens. You can now come back to your world");
		cmsg_print(TERM_L_GREEN, "and live happily ever after.");
		cmsg_print(TERM_L_GREEN, "What you do now is up to you, but your deeds shall ever be remembered.");
		cmsg_print(TERM_L_GREEN, "You may retire (commit suicide) when you are ready.");

		/* Create the entrance */
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_MORE);

		/* Remove now used hook */
		del_hook_new(HOOK_MONSTER_DEATH, quest_ultra_good_death_hook);
		process_hooks_restart = true;

		/* End plot */
		*(quest[QUEST_ULTRA_GOOD].plot) = QUEST_NULL;
	}

	/* Tik'svvrzllat is dead! */
	if (m_ptr->r_idx == 1032)
	{
		int i;

		/* Get local object */
		object_type forge, *q_ptr = &forge;

		/* Mega-Hack -- Prepare to make the Flame Imperishable */
		object_prep(q_ptr, lookup_kind(TV_JUNK, 255));

		/* Mega-Hack -- Actually create the Flame Imperishable */
		get_flame_imperishable()->allow_special = true;
		apply_magic(q_ptr, -1, true, true, true);
		get_flame_imperishable()->allow_special = false;

		/* Identify it fully */
		object_aware(q_ptr);
		object_known(q_ptr);

		/* Find a space */
		for (i = 0; i < INVEN_PACK; i++)
		{
			/* Skip non-objects */
			if (!p_ptr->inventory[i].k_ptr)
			{
				break;
			}
		}
		/* Arg, no space ! */
		if (i == INVEN_PACK)
		{
			char o_name[200];

			object_desc(o_name, &p_ptr->inventory[INVEN_PACK - 1], false, 0);

			/* Drop the item */
			inven_drop(INVEN_PACK - 1, 99, p_ptr->py, p_ptr->px, false);

			cmsg_format(TERM_VIOLET, "You feel the urge to drop your %s to make room in your inventory.", o_name);
		}

		/* Carry it */
		cmsg_format(TERM_VIOLET, "You feel the urge to pick up the Flame Imperishable.");
		inven_carry(q_ptr, false);
	}

	return false;
}

static bool quest_ultra_good_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (quest[QUEST_ULTRA_GOOD].status >= QUEST_STATUS_TAKEN)
	{
		/* Ultra winner ! */
		if (total_winner == WINNER_ULTRA)
		{
			fprintf(f, "\n You destroyed Melkor forever and have been elevated to the status of Vala by Eru Iluvatar.");
			fprintf(f, "\n Arda will forever be free.");
		}
		else
		{
			/* Tried and failed */
			if (death)
			{
				fprintf(f, "\n You tried to destroy Melkor forever, but died in the attempt.");
				fprintf(f, "\n Arda will be quiet, but not free from evil.");
			}
		}
	}

	return false;
}


void quest_ultra_good_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_STAIR,         quest_ultra_good_stair_hook,  "ultrag_stair",  NULL);
		add_hook_new(HOOK_RECALL,        quest_ultra_good_recall_hook, "ultrag_recall", NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_ultra_good_death_hook,  "ultrag_death",  NULL);
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook_new(HOOK_MOVE, quest_ultra_good_move_hook, "ultrag_move", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP, quest_ultra_good_dump_hook, "ultrag_dump", NULL);
}
