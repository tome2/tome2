#include "q_shroom.h"

#include "cave.hpp"
#include "hooks.h"
#include "messages.h"
#include "monster2.hpp"
#include "object2.hpp"
#include "util.hpp"

#include <cassert>

#define cquest (quest[QUEST_SHROOM])

static bool_ quest_shroom_speak_hook(void *, void *, void *);
static bool_ quest_shroom_chat_hook(void *, void *, void *);

GENERATE_MONSTER_LOOKUP_FN(get_grip, "Grip, Farmer Maggot's dog")
GENERATE_MONSTER_LOOKUP_FN(get_wolf, "Wolf, Farmer Maggot's dog")
GENERATE_MONSTER_LOOKUP_FN(get_fang, "Fang, Farmer Maggot's dog")
GENERATE_MONSTER_LOOKUP_FN(get_farmer_maggot, "Farmer Maggot")

static bool_ quest_shroom_town_gen_hook(void *, void *in_, void *)
{
	struct hook_wild_gen_in *in = static_cast<struct hook_wild_gen_in *>(in_);
	int m_idx, x = 1, y = 1, tries = 10000;
	bool_ small = in->small;

	/* Generate the shrooms field */
	if ((!small) && (p_ptr->wilderness_y == 21) && (p_ptr->wilderness_x == 33))
	{
		/* Create the field */
		for (x = (cur_wid / 2) - 7; x <= (cur_wid / 2) + 7; x++)
			for (y = (cur_hgt / 2) - 5; y <= (cur_hgt / 2) + 5; y++)
				cave_set_feat(y, x, 181);

		/* Throw in some 'shrooms */
		for (x = 0; x < (cquest.data[1] - cquest.data[0]); x++)
		{
			object_type forge, *q_ptr = &forge;

			object_prep(q_ptr, lookup_kind(TV_FOOD, rand_range(1, 18)));
			q_ptr->number = 1;
			/* Mark them */
			q_ptr->pval2 = 1;
			drop_near(q_ptr, -1, rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5), rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7));
		}

		/* Throw in some dogs ;) */
		y = rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5);
		x = rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7);
		m_allow_special[get_grip()] = TRUE;
		m_idx = place_monster_one(y, x, get_grip(), 0, FALSE, MSTATUS_ENEMY);
		if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
		m_allow_special[get_grip()] = FALSE;

		y = rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5);
		x = rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7);
		m_allow_special[get_wolf()] = TRUE;
		m_idx = place_monster_one(y, x, get_wolf(), 0, FALSE, MSTATUS_ENEMY);
		if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
		m_allow_special[get_wolf()] = FALSE;

		y = rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5);
		x = rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7);
		m_allow_special[get_fang()] = TRUE;
		m_idx = place_monster_one(y, x, get_fang(), 0, FALSE, MSTATUS_ENEMY);
		if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
		m_allow_special[get_fang()] = FALSE;

		msg_print("You hear frenzied barking.");
	}

	/* Generate maggot in town, in daylight */
	if ((bst(HOUR, turn) < 6) || (bst(HOUR, turn) >= 18) || (cquest.status > QUEST_STATUS_COMPLETED) || (small) || (p_ptr->town_num != 1)) return (FALSE);

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

	/* Place Farmer Maggot */
	m_allow_special[get_farmer_maggot()] = TRUE;
	place_monster_one(y, x, get_farmer_maggot(), 0, FALSE, MSTATUS_ENEMY);
	m_allow_special[get_farmer_maggot()] = FALSE;

	return FALSE;
}

static bool_ quest_shroom_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);	
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (cquest.status > QUEST_STATUS_COMPLETED) return FALSE;

	if ((r_idx == get_wolf()) ||
			(r_idx == get_grip()) ||
			(r_idx == get_fang()))
	{
		msg_print("The dog yells a last time and drops dead on the grass.");
	}

	return FALSE;
}

static bool_ quest_shroom_give_hook(void *, void *in_, void *)
{
	struct hook_give_in *in = static_cast<struct hook_give_in *>(in_);
	object_type *o_ptr;
	monster_type *m_ptr;

	s32b m_idx = in->m_idx;
	s32b item = in->item;

	o_ptr = &p_ptr->inventory[item];
	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != get_farmer_maggot()) return (FALSE);

	/* If one is dead .. its bad */
	if ((r_info[get_grip()].max_num == 0) ||
			(r_info[get_wolf()].max_num == 0) ||
			(r_info[get_fang()].max_num == 0))
	{
		cquest.status = QUEST_STATUS_FAILED_DONE;
		msg_print("My puppy!  My poor, defenceless puppy...");
		msg_print("YOU MURDERER!  Out of my sight!");
		delete_monster_idx(m_idx);

		del_hook_new(HOOK_GIVE, quest_shroom_give_hook);
		del_hook_new(HOOK_CHAT, quest_shroom_speak_hook);
		del_hook_new(HOOK_WILD_GEN, quest_shroom_town_gen_hook);
		process_hooks_restart = TRUE;
		return TRUE;
	}

	if ((o_ptr->tval != TV_FOOD) || (o_ptr->pval2 != 1)) return (FALSE);

	/* Take a mushroom */
	inc_stack_size_ex(item, -1, OPTIMIZE, NO_DESCRIBE);
	cquest.data[0]++;

	if (cquest.data[0] == cquest.data[1])
	{
		object_type forge, *q_ptr;

		msg_print("Oh thank you!");
		msg_print("Take my sling and those mushrooms, may they help you!");
		msg_print("Farmer Maggot heads back to his house.");

		/* Mushrooms */
		q_ptr = &forge;
		object_prep(q_ptr, lookup_kind(TV_FOOD, SV_FOOD_CURE_SERIOUS));
		q_ptr->found = OBJ_FOUND_REWARD;
		q_ptr->number = rand_range(15, 20);
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->discount = 100;
		q_ptr->ident |= IDENT_STOREB;
		if (inven_carry_okay(q_ptr))
			inven_carry(q_ptr, FALSE);
		else
			drop_near(q_ptr, 0, p_ptr->py, p_ptr->px);

		/* The sling of farmer maggot */
		q_ptr = &forge;
		object_prep(q_ptr, lookup_kind(TV_BOW, SV_SLING));
		q_ptr->found = OBJ_FOUND_REWARD;
		q_ptr->number = 1;
		q_ptr->name1 = 149;
		apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->discount = 100;
		q_ptr->ident |= IDENT_STOREB;
		(void)inven_carry(q_ptr, FALSE);

		delete_monster_idx(m_idx);

		cquest.status = QUEST_STATUS_FINISHED;

		del_hook_new(HOOK_GIVE, quest_shroom_give_hook);
		process_hooks_restart = TRUE;
	}
	else
		msg_format("Oh thank you, but you still have %d mushrooms to bring back!", cquest.data[1] - cquest.data[0]);

	return TRUE;
}

static void check_dogs_alive(s32b m_idx)
{
	if ((r_info[get_grip()].max_num == 0) ||
	    (r_info[get_wolf()].max_num == 0) ||
	    (r_info[get_fang()].max_num == 0))
	{
		cquest.status = QUEST_STATUS_FAILED_DONE;
		msg_print("My puppy!  My poor, defenceless puppy...");
		msg_print("YOU MURDERER!  Out of my sight!");
		delete_monster_idx(m_idx);

		del_hook_new(HOOK_GIVE, quest_shroom_give_hook);
		del_hook_new(HOOK_MON_SPEAK, quest_shroom_speak_hook);
		del_hook_new(HOOK_CHAT, quest_shroom_chat_hook);
		del_hook_new(HOOK_WILD_GEN, quest_shroom_town_gen_hook);
		process_hooks_restart = TRUE;
	}
	else
	{
		msg_format("You still have %d mushrooms to bring back!", cquest.data[1] - cquest.data[0]);
	}
}

static bool_ quest_shroom_speak_hook(void *, void *in_, void *)
{
	struct hook_mon_speak_in *in = static_cast<struct hook_mon_speak_in *>(in_);
	s32b m_idx = in->m_idx;

	if (m_list[m_idx].r_idx != get_farmer_maggot()) return (FALSE);

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		msg_format("%^s asks your help.", in->m_name);
		process_hooks_new(HOOK_MON_ASK_HELP, NULL, NULL);
	}
	else
	{
		check_dogs_alive(m_idx);
	}
	return (TRUE);
}

static bool_ quest_shroom_chat_hook(void *, void *in_, void *)
{
	struct hook_chat_in *in = static_cast<struct hook_chat_in *>(in_);
	s32b m_idx = in->m_idx;
	monster_type *m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != get_farmer_maggot()) return (FALSE);

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		msg_print("My mushrooms, my mushrooms!");
		msg_print("The rain, a dark horrible rain, began so I had to return to my home.");
		msg_print("But when I came back my dogs were all mad and didn't let me near the field.");
		msg_print("Could you please bring me back all the mushrooms that have grown in my field");
		msg_print("to the west of Bree? Please try to not harm my dogs. They are so lovely...");

		cquest.status = QUEST_STATUS_TAKEN;
		quest[QUEST_SHROOM].init(QUEST_SHROOM);
	}
	else
	{
		check_dogs_alive(m_idx);
	}

	return TRUE;
}

bool_ quest_shroom_init_hook(int q_idx)
{
	/* Get a number of 'shrooms */
	if (!cquest.data[1])
	{
		cquest.data[0] = 0;
		cquest.data[1] = rand_range(7, 14);
		if (wizard)
		{
			message_add(format("Shrooms number %d", cquest.data[1]), TERM_BLUE);
		}
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_shroom_death_hook,    "shroom_death",    NULL);
		add_hook_new(HOOK_GIVE,          quest_shroom_give_hook,     "shroom_give",     NULL);
		add_hook_new(HOOK_WILD_GEN,      quest_shroom_town_gen_hook, "shroom_town_gen", NULL);
		add_hook_new(HOOK_CHAT,          quest_shroom_chat_hook,     "shroom_chat",     NULL);
		add_hook_new(HOOK_MON_SPEAK,     quest_shroom_speak_hook,    "shroom_speak",    NULL);
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook_new(HOOK_MON_SPEAK, quest_shroom_speak_hook,    "shroom_speak",    NULL);
		add_hook_new(HOOK_WILD_GEN,  quest_shroom_town_gen_hook, "shroom_town_gen", NULL);
		add_hook_new(HOOK_CHAT,      quest_shroom_chat_hook,     "shroom_chat",     NULL);
	}
	return (FALSE);
}
