#include "q_main.hpp"

#include "hook_chardump_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hook_new_monster_in.hpp"
#include "hooks.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"

#include <cassert>

GENERATE_MONSTER_LOOKUP_FN(get_necromancer, "The Necromancer of Dol Guldur")
GENERATE_MONSTER_LOOKUP_FN(get_sauron, "Sauron, the Sorcerer")
GENERATE_MONSTER_LOOKUP_FN(get_morgoth, "Morgoth, Lord of Darkness")

static void quest_describe(int q_idx)
{
	int i = 0;

	while ((i < 10) && (quest[q_idx].desc[i][0] != '\0'))
	{
		cmsg_print(TERM_YELLOW, quest[q_idx].desc[i++]);
	}
}

static bool_ quest_main_monsters_hook(void *, void *in_, void *)
{
	struct hook_new_monster_in *in = static_cast<struct hook_new_monster_in *>(in_);
	s32b r_idx = in->r_idx;

	/* Sauron */
	if (r_idx == get_sauron())
	{
		/* No Sauron until Necromancer dies */
		if (r_info[get_necromancer()].max_num) return TRUE;
	}
	/* Morgoth */
	else if (r_idx == get_morgoth())
	{
		/* No Morgoth until Sauron dies */
		if (r_info[get_sauron()].max_num) return TRUE;
	}
	return FALSE;
}

static bool_ quest_morgoth_hook(void *, void *, void *)
{
	monster_race *r_ptr = &r_info[get_morgoth()];

	/* Need to kill him */
	if (!r_ptr->max_num)
	{
		/* Total winner */
		total_winner = WINNER_NORMAL;
		has_won = WINNER_NORMAL;
		quest[QUEST_MORGOTH].status = QUEST_STATUS_FINISHED;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_FRAME);

		/* Congratulations */
		if (quest[QUEST_ONE].status == QUEST_STATUS_FINISHED)
		{
			cmsg_print(TERM_L_GREEN, "*** CONGRATULATIONS ***");
			cmsg_print(TERM_L_GREEN, "You have banished Morgoth's foul spirit from Ea, and as you watch, a cleansing");
			cmsg_print(TERM_L_GREEN, "wind roars through the dungeon, dispersing the nether mists around where the");
			cmsg_print(TERM_L_GREEN, "body fell. You feel thanks, and a touch of sorrow, from the Valar");
			cmsg_print(TERM_L_GREEN, "for your deed. You will be forever heralded, your deed forever legendary.");
			cmsg_print(TERM_L_GREEN, "You may retire (commit suicide) when you are ready.");
		}
		else
		{
			cmsg_print(TERM_VIOLET, "*** CONGRATULATIONS ***");
			cmsg_print(TERM_VIOLET, "You have banished Morgoth from Arda, and made Ea a safer place.");
			cmsg_print(TERM_VIOLET, "As you look down at the dispersing mists around Morgoth, a sudden intuition");
			cmsg_print(TERM_VIOLET, "grasps you. Fingering the One Ring, you gather the nether mists around");
			cmsg_print(TERM_VIOLET, "yourself, and inhale deeply their seductive power.");
			cmsg_print(TERM_VIOLET, "You will be forever feared, your orders forever obeyed.");
			cmsg_print(TERM_VIOLET, "You may retire (commit suicide) when you are ready.");
		}

		/* Continue the plot(maybe) */
		del_hook_new(HOOK_MONSTER_DEATH, quest_morgoth_hook);
		process_hooks_restart = TRUE;

		/* Either ultra good if the one Ring is destroyed, or ultra evil if used */
		if (quest[QUEST_ONE].status == QUEST_STATUS_FINISHED)
			*(quest[QUEST_MORGOTH].plot) = QUEST_ULTRA_GOOD;
		else
			*(quest[QUEST_MORGOTH].plot) = QUEST_ULTRA_EVIL;
		quest[*(quest[QUEST_MORGOTH].plot)].init(*(quest[QUEST_MORGOTH].plot));
	}
	return (FALSE);
}

static bool_ quest_morgoth_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (quest[QUEST_MORGOTH].status >= QUEST_STATUS_COMPLETED)
	{
		if (quest[QUEST_ONE].status == QUEST_STATUS_FINISHED)
			fprintf(f, "\n You saved Arda and became a famed hero.");
		else
			fprintf(f, "\n You became a new force of darkness and enslaved all free people.");
	}
	return (FALSE);
}

bool_ quest_morgoth_init_hook(int q_idx)
{
	if ((quest[QUEST_MORGOTH].status >= QUEST_STATUS_TAKEN) && (quest[QUEST_MORGOTH].status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_morgoth_hook, "morgoth_death", NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP,   quest_morgoth_dump_hook,  "morgoth_dump",     NULL);
	add_hook_new(HOOK_NEW_MONSTER, quest_main_monsters_hook, "main_new_monster", NULL);
	return (FALSE);
}

static bool_ quest_sauron_hook(void *, void *, void *)
{
	monster_race *r_ptr = &r_info[get_sauron()];

	/* Need to kill him */
	if (!r_ptr->max_num)
	{
		cmsg_print(TERM_YELLOW, "Well done! You are on the way to slaying Morgoth...");
		quest[QUEST_SAURON].status = QUEST_STATUS_FINISHED;

		quest[QUEST_MORGOTH].status = QUEST_STATUS_TAKEN;
		quest_describe(QUEST_MORGOTH);

		del_hook_new(HOOK_MONSTER_DEATH, quest_sauron_hook);
		add_hook_new(HOOK_MONSTER_DEATH, quest_morgoth_hook, "morgort_death", NULL);
		*(quest[QUEST_SAURON].plot) = QUEST_MORGOTH;
		quest_morgoth_init_hook(QUEST_MORGOTH);

		process_hooks_restart = TRUE;
	}
	return (FALSE);
}

static bool_ quest_sauron_resurect_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	if ((r_ptr->flags & RF_NAZGUL) && r_info[get_sauron()].max_num)
	{
		msg_format("Somehow you feel %s is not totally destroyed...", (r_ptr->flags & RF_FEMALE ? "she" : "he"));
		r_ptr->max_num = 1;
	}
	else if ((m_ptr->r_idx == get_sauron()) && (quest[QUEST_ONE].status < QUEST_STATUS_FINISHED))
	{
		msg_print("Sauron will not be permanently defeated until the One Ring is either destroyed or used...");
		r_ptr->max_num = 1;
	}
	return FALSE;
}

bool_ quest_sauron_init_hook(int q_idx)
{
	if ((quest[QUEST_SAURON].status >= QUEST_STATUS_TAKEN) && (quest[QUEST_SAURON].status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_sauron_hook, "sauron_death", NULL);
	}
	add_hook_new(HOOK_NEW_MONSTER,   quest_main_monsters_hook,   "main_new_monster",      NULL);
	add_hook_new(HOOK_MONSTER_DEATH, quest_sauron_resurect_hook, "sauron_resurect_death", NULL);
	return (FALSE);
}

static bool_ quest_necro_hook(void *, void *, void *)
{
	monster_race *r_ptr = &r_info[get_necromancer()];

	/* Need to kill him */
	if (!r_ptr->max_num)
	{
		cmsg_print(TERM_YELLOW, "You see the spirit of the necromancer rise and flee...");
		cmsg_print(TERM_YELLOW, "It looks like it was indeed Sauron...");
		cmsg_print(TERM_YELLOW, "You should report that to Galadriel as soon as possible.");

		quest[QUEST_NECRO].status = QUEST_STATUS_FINISHED;

		*(quest[QUEST_NECRO].plot) = QUEST_ONE;
		quest[*(quest[QUEST_NECRO].plot)].init(*(quest[QUEST_NECRO].plot));

		del_hook_new(HOOK_MONSTER_DEATH, quest_necro_hook);
		process_hooks_restart = TRUE;
	}
	return (FALSE);
}

bool_ quest_necro_init_hook(int q_idx)
{
	if ((quest[QUEST_NECRO].status >= QUEST_STATUS_TAKEN) && (quest[QUEST_NECRO].status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_necro_hook, "necro_death", NULL);
	}
	add_hook_new(HOOK_NEW_MONSTER, quest_main_monsters_hook, "main_new_monster", NULL);
	return (FALSE);
}
