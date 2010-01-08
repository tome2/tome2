#undef cquest
#define cquest (quest[QUEST_SHROOM])

bool quest_shroom_speak_hook(char *fmt);

bool quest_shroom_town_gen_hook(char *fmt)
{
	int x = 1, y = 1, try = 10000;
	s32b small;

	small = get_next_arg(fmt);

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
		m_allow_special[test_monster_name("Grip, Farmer Maggot's dog")] = TRUE;
		place_monster_one(y, x, test_monster_name("Grip, Farmer Maggot's dog"), 0, FALSE, MSTATUS_ENEMY);
		m_allow_special[test_monster_name("Grip, Farmer Maggot's dog")] = FALSE;

		y = rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5);
		x = rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7);
		m_allow_special[test_monster_name("Wolf, Farmer Maggot's dog")] = TRUE;
		place_monster_one(y, x, test_monster_name("Wolf, Farmer Maggot's dog"), 0, FALSE, MSTATUS_ENEMY);
		m_allow_special[test_monster_name("Wolf, Farmer Maggot's dog")] = FALSE;

		y = rand_range((cur_hgt / 2) - 5, (cur_hgt / 2) + 5);
		x = rand_range((cur_wid / 2) - 7, (cur_wid / 2) + 7);
		m_allow_special[test_monster_name("Fang, Farmer Maggot's dog")] = TRUE;
		place_monster_one(y, x, test_monster_name("Fang, Farmer Maggot's dog"), 0, FALSE, MSTATUS_ENEMY);
		m_allow_special[test_monster_name("Fang, Farmer Maggot's dog")] = FALSE;

		msg_print("You hear frenzied barking.");
	}

	/* Generate maggot in town, in daylight */
	if ((bst(HOUR, turn) < 6) || (bst(HOUR, turn) >= 18) || (cquest.status > QUEST_STATUS_COMPLETED) || (small) || (p_ptr->town_num != 1)) return (FALSE);

	/* Find a good position */
	while (try)
	{
		/* Get a random spot */
		y = randint(20) + (cur_hgt / 2) - 10;
		x = randint(20) + (cur_wid / 2) - 10;

		/* Is it a good spot ? */
		/* Not in player los, and avoid shop grids */
		if (!los(p_ptr->py, p_ptr->px, y, x) && cave_empty_bold(y, x) &&
		                cave_plain_floor_bold(y, x)) break;

		/* One less try */
		try--;
	}

	/* Place Farmer Maggot */
	m_allow_special[test_monster_name("Farmer Maggot")] = TRUE;
	place_monster_one(y, x, test_monster_name("Farmer Maggot"), 0, FALSE, MSTATUS_ENEMY);
	m_allow_special[test_monster_name("Farmer Maggot")] = FALSE;

	return FALSE;
}
bool quest_shroom_death_hook(char *fmt)
{
	s32b r_idx, m_idx;

	m_idx = get_next_arg(fmt);
	r_idx = m_list[m_idx].r_idx;

	if (cquest.status > QUEST_STATUS_COMPLETED) return FALSE;

	if ((r_idx == test_monster_name("Wolf, Farmer Maggot's dog")) ||
	                (r_idx == test_monster_name("Grip, Farmer Maggot's dog")) ||
	                (r_idx == test_monster_name("Fang, Farmer Maggot's dog")))
	{
		msg_print("The dog yells a last time and drops dead on the grass.");
	}

	return FALSE;
}
bool quest_shroom_give_hook(char *fmt)
{
	object_type *o_ptr;
	monster_type *m_ptr;
	s32b m_idx, item;

	m_idx = get_next_arg(fmt);
	item = get_next_arg(fmt);

	o_ptr = &p_ptr->inventory[item];
	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != test_monster_name("Farmer Maggot")) return (FALSE);

	/* If one is dead .. its bad */
	if ((r_info[test_monster_name("Grip, Farmer Maggot's dog")].max_num == 0) ||
	                (r_info[test_monster_name("Wolf, Farmer Maggot's dog")].max_num == 0) ||
	                (r_info[test_monster_name("Fang, Farmer Maggot's dog")].max_num == 0))
	{
		cquest.status = QUEST_STATUS_FAILED_DONE;
		msg_print("My puppy!  My poor, defenceless puppy...");
		msg_print("YOU MURDERER!  Out of my sight!");
		delete_monster_idx(m_idx);

		del_hook(HOOK_GIVE, quest_shroom_give_hook);
		del_hook(HOOK_CHAT, quest_shroom_speak_hook);
		del_hook(HOOK_WILD_GEN, quest_shroom_town_gen_hook);
		process_hooks_restart = TRUE;
		return TRUE;
	}

	if ((o_ptr->tval != TV_FOOD) || (o_ptr->pval2 != 1)) return (FALSE);

	/* Take a mushroom */
	inven_item_increase(item, -1);
	inven_item_optimize(item);
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

		del_hook(HOOK_GIVE, quest_shroom_give_hook);
		process_hooks_restart = TRUE;
	}
	else
		msg_format("Oh thank you, but you still have %d mushrooms to bring back!", cquest.data[1] - cquest.data[0]);

	return TRUE;
}
bool quest_shroom_speak_hook(char *fmt)
{
	s32b m_idx = get_next_arg(fmt);

	if (m_list[m_idx].r_idx != test_monster_name("Farmer Maggot")) return (FALSE);

	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		cptr m_name;

		m_name = get_next_arg_str(fmt);

		msg_format("%^s asks your help.", m_name);
		exec_lua("ingame_help('monster_chat')");
	}
	else
	{
		/* If one is dead .. its bad */
		if ((r_info[test_monster_name("Grip, Farmer Maggot's dog")].max_num == 0) ||
		                (r_info[test_monster_name("Wolf, Farmer Maggot's dog")].max_num == 0) ||
		                (r_info[test_monster_name("Fang, Farmer Maggot's dog")].max_num == 0))
		{
			cquest.status = QUEST_STATUS_FAILED_DONE;
			msg_print("My puppy!  My poor, defenceless puppy...");
			msg_print("YOU MURDERER!  Out of my sight!");
			delete_monster_idx(m_idx);

			del_hook(HOOK_GIVE, quest_shroom_give_hook);
			del_hook(HOOK_CHAT, quest_shroom_speak_hook);
			del_hook(HOOK_WILD_GEN, quest_shroom_town_gen_hook);
			process_hooks_restart = TRUE;
			return TRUE;
		}
		msg_format("You still have %d mushrooms to bring back!", cquest.data[1] - cquest.data[0]);
	}
	return (TRUE);
}
bool quest_shroom_chat_hook(char *fmt)
{
	monster_type *m_ptr;
	s32b m_idx;

	m_idx = get_next_arg(fmt);

	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != test_monster_name("Farmer Maggot")) return (FALSE);

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
		/* If one is dead .. its bad */
		if ((r_info[test_monster_name("Grip, Farmer Maggot's dog")].max_num == 0) ||
		                (r_info[test_monster_name("Wolf, Farmer Maggot's dog")].max_num == 0) ||
		                (r_info[test_monster_name("Fang, Farmer Maggot's dog")].max_num == 0))
		{
			cquest.status = QUEST_STATUS_FAILED_DONE;
			msg_print("My puppy!  My poor, defenceless puppy...");
			msg_print("YOU MURDERER!  Out of my sight!");
			delete_monster_idx(m_idx);

			del_hook(HOOK_GIVE, quest_shroom_give_hook);
			del_hook(HOOK_CHAT, quest_shroom_speak_hook);
			del_hook(HOOK_WILD_GEN, quest_shroom_town_gen_hook);
			process_hooks_restart = TRUE;
			return TRUE;
		}
		msg_format("You still have %d mushrooms to bring back!", cquest.data[1] - cquest.data[0]);
	}

	return TRUE;
}
bool quest_shroom_init_hook(int q_idx)
{
	/* Get a number of 'shrooms */
	if (!cquest.data[1])
	{
		cquest.data[0] = 0;
		cquest.data[1] = rand_range(7, 14);
		if (wizard) message_add(MESSAGE_MSG, format("Shrooms number %d", cquest.data[1]), TERM_BLUE);
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_shroom_death_hook, "shroom_death");
		add_hook(HOOK_GIVE, quest_shroom_give_hook, "shroom_give");
		add_hook(HOOK_WILD_GEN, quest_shroom_town_gen_hook, "shroom_town_gen");
		add_hook(HOOK_CHAT, quest_shroom_chat_hook, "shroom_chat");
		add_hook(HOOK_MON_SPEAK, quest_shroom_speak_hook, "shroom_speak");
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook(HOOK_MON_SPEAK, quest_shroom_speak_hook, "shroom_speak");
		add_hook(HOOK_WILD_GEN, quest_shroom_town_gen_hook, "shroom_town_gen");
		add_hook(HOOK_CHAT, quest_shroom_chat_hook, "shroom_chat");
	}
	return (FALSE);
}
