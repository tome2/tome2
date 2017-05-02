#include "q_betwen.hpp"

#include "cave.hpp"
#include "dungeon_flag.hpp"
#include "cave_type.hpp"
#include "hook_chardump_in.hpp"
#include "hook_init_quest_in.hpp"
#include "hook_move_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"

#define cquest (quest[QUEST_BETWEEN])

static bool_ quest_between_move_hook(void *, void *in_, void *)
{
	struct hook_move_in *in = static_cast<struct hook_move_in *>(in_);
	s32b y = in->y;
	s32b x = in->x;
	cave_type *c_ptr;

	c_ptr = &cave[y][x];

	if (cquest.status != QUEST_STATUS_TAKEN) return FALSE;

	/* The tower of Turgon */
	if ((c_ptr->feat == FEAT_SHOP) && (c_ptr->special == 27))
	{
		cmsg_print(TERM_YELLOW, "Turgon is there.");
		cmsg_print(TERM_YELLOW, "'Ah, thank you, noble hero! Now please return to Minas Anor to finish the link.'");

		cquest.status = QUEST_STATUS_COMPLETED;

		return TRUE;
	}

	/* Only 1 ambush */
	if (cquest.data[0]) return (FALSE);

	if (!p_ptr->wild_mode)
	{
		if (p_ptr->wilderness_y > 19) return (FALSE);
	}
	else
	{
		if (p_ptr->py > 19) return (FALSE);
	}

	/* Mark as entered */
	cquest.data[0] = TRUE;

	p_ptr->wild_mode = FALSE;
	p_ptr->inside_quest = QUEST_BETWEEN;
	p_ptr->leaving = TRUE;

	cmsg_print(TERM_YELLOW, "Looks like a full wing of thunderlords ambushes you!");
	cmsg_print(TERM_YELLOW, "Trone steps forth and speaks: 'The secret of the Void Jumpgates");
	cmsg_print(TERM_YELLOW, "will not be used by any but the thunderlords!'");

	return FALSE;
}

static bool_ quest_between_gen_hook(void *, void *, void *)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_BETWEEN) return FALSE;

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
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("between.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	/* Otherwise instadeath */
	energy_use = 0;

	dungeon_flags |= DF_NO_GENO;

	return TRUE;
}

static bool_ quest_between_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

	if (q_idx != QUEST_BETWEEN) return FALSE;

	c_put_str(TERM_YELLOW, "Ah you finally arrived, I hope your travel wasn't too hard.", 8, 0);
	c_put_str(TERM_YELLOW, "As a reward you can freely use the Void Jumpgates for quick travel.", 9, 0);
	c_put_str(TERM_YELLOW, "Oh and take that horn, it shall serve you well.", 10, 0);

	/* prepare the reward */
	q_ptr = &forge;
	object_prep(q_ptr, test_item_name("& Golden Horn~ of the Thunderlords"));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->number = 1;


	/* Mega-Hack -- Actually create the Golden Horn of the Thunderlords */
	k_allow_special[test_item_name("& Golden Horn~ of the Thunderlords")] = TRUE;
	apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);
	k_allow_special[test_item_name("& Golden Horn~ of the Thunderlords")] = FALSE;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->discount = 100;
	q_ptr->ident |= IDENT_STOREB;
	inven_carry(q_ptr, FALSE);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FINISH, quest_between_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

static bool_ quest_between_death_hook(void *, void *, void *)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_BETWEEN) return FALSE;

	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status <= MSTATUS_NEUTRAL) mcnt++;
	}

	if (mcnt < 2)
	{
		cmsg_print(TERM_YELLOW, "You can escape now.");
		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
		cave[p_ptr->py][p_ptr->px].special = 0;

		return FALSE;
	}


	return FALSE;
}

static bool_ quest_between_dump_hook(void *, void *in_, void *)
{
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;

	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(f, "\n You established a permanent void jumpgates liaison between Minas Anor and Gondolin,");
		fprintf(f, "\n  thus allowing the last alliance to exist.");
	}
	return (FALSE);
}

static bool_ quest_between_forbid_hook(void *, void *in_, void *)
{
	hook_init_quest_in *in = static_cast<struct hook_init_quest_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_BETWEEN) return (FALSE);

	if (p_ptr->lev < 45)
	{
		c_put_str(TERM_WHITE, "I fear you are not ready for the next quest, come back later.", 8, 0);
		return (TRUE);
	}
	return (FALSE);
}

bool_ quest_between_init_hook()
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MOVE,          quest_between_move_hook,   "between_move",   NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_between_gen_hook,    "between_gen",    NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_between_finish_hook, "between_finish", NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_between_death_hook,  "between_death",  NULL);
	}
	add_hook_new(HOOK_CHAR_DUMP,  quest_between_dump_hook,   "between_dump",   NULL);
	add_hook_new(HOOK_INIT_QUEST, quest_between_forbid_hook, "between_forbid", NULL);
	return (FALSE);
}
