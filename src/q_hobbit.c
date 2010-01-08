#undef cquest
#define cquest (quest[QUEST_HOBBIT])

bool quest_hobbit_town_gen_hook(char *fmt)
{
	int x = 1, y = 1, try = 10000;
	s32b small;

	small = get_next_arg(fmt);

	if ((turn < (cquest.data[1] + (DAY * 10L))) || (cquest.status > QUEST_STATUS_COMPLETED) || (small) || (p_ptr->town_num != 1)) return (FALSE);

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

	/* Place Melinda */
	m_allow_special[test_monster_name("Melinda Proudfoot")] = TRUE;
	place_monster_one(y, x, test_monster_name("Melinda Proudfoot"), 0, FALSE, MSTATUS_ENEMY);
	m_allow_special[test_monster_name("Melinda Proudfoot")] = FALSE;

	return FALSE;
}
bool quest_hobbit_gen_hook(char *fmt)
{
	int x = 1, y = 1, try = 10000;

	if ((cquest.status != QUEST_STATUS_TAKEN) || (dun_level != cquest.data[0]) || (dungeon_type != DUNGEON_MAZE)) return FALSE;

	/* Find a good position */
	while (try)
	{
		/* Get a random spot */
		y = randint(cur_hgt - 4) + 2;
		x = randint(cur_wid - 4) + 2;

		/* Is it a good spot ? */
		if (cave_empty_bold(y, x)) break;

		/* One less try */
		try--;
	}

	/* Place the hobbit */
	m_allow_special[test_monster_name("Merton Proudfoot, the lost hobbit")] = TRUE;
	place_monster_one(y, x, test_monster_name("Merton Proudfoot, the lost hobbit"), 0, FALSE, MSTATUS_FRIEND);
	m_allow_special[test_monster_name("Merton Proudfoot, the lost hobbit")] = FALSE;

	return FALSE;
}
bool quest_hobbit_give_hook(char *fmt)
{
	object_type *o_ptr;
	monster_type *m_ptr;
	s32b m_idx, item;

	m_idx = get_next_arg(fmt);
	item = get_next_arg(fmt);

	o_ptr = &p_ptr->inventory[item];
	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != test_monster_name("Merton Proudfoot, the lost hobbit")) return (FALSE);

	if ((o_ptr->tval != TV_SCROLL) || (o_ptr->sval != SV_SCROLL_WORD_OF_RECALL)) return (FALSE);

	msg_print("'Oh, thank you, noble one!'");
	msg_print("Merton Proudfoot reads the scroll and is recalled to the safety of his home.");

	delete_monster_idx(m_idx);
	inven_item_increase(item, -1);
	inven_item_optimize(item);

	cquest.status = QUEST_STATUS_COMPLETED;

	del_hook(HOOK_GIVE, quest_hobbit_give_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool quest_hobbit_speak_hook(char *fmt)
{
	s32b m_idx = get_next_arg(fmt);

	if (m_list[m_idx].r_idx != test_monster_name("Melinda Proudfoot")) return (FALSE);

	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		cptr m_name;

		m_name = get_next_arg_str(fmt);

		msg_format("%^s begs for your help.", m_name);
	}
	return (TRUE);
}
bool quest_hobbit_chat_hook(char *fmt)
{
	monster_type *m_ptr;
	s32b m_idx;

	m_idx = get_next_arg(fmt);

	m_ptr = &m_list[m_idx];

	if (m_ptr->r_idx != test_monster_name("Melinda Proudfoot")) return (FALSE);

	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		msg_print("Oh! Oh!");
		msg_print("My poor Merton, where is my poor Merton? He was playing near that dreadful");
		msg_print("maze and never been seen again! Could you find him for me?");

		cquest.status = QUEST_STATUS_TAKEN;
		quest[QUEST_HOBBIT].init(QUEST_HOBBIT);
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
		object_aware(q_ptr);
		object_known(q_ptr);
		q_ptr->ident |= IDENT_STOREB;
		(void)inven_carry(q_ptr, FALSE);

		cquest.status = QUEST_STATUS_FINISHED;

		del_hook(HOOK_MON_SPEAK, quest_hobbit_speak_hook);
		process_hooks_restart = TRUE;
		delete_monster_idx(m_idx);

		return TRUE;
	}
	else
	{
		msg_print("Thanks again.");
	}

	return TRUE;
}
bool quest_hobbit_dump_hook(char *fmt)
{
	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(hook_file, "\n You saved a young hobbit from an horrible fate.");
	}
	return (FALSE);
}
bool quest_hobbit_init_hook(int q_idx)
{
	/* Get a level to place the hobbit */
	if (!cquest.data[0])
	{
		cquest.data[0] = rand_range(26, 34);
		cquest.data[1] = turn;
		if (wizard) message_add(MESSAGE_MSG, format("Hobbit level %d", cquest.data[0]), TERM_BLUE);
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_GIVE, quest_hobbit_give_hook, "hobbit_give");
		add_hook(HOOK_GEN_LEVEL, quest_hobbit_gen_hook, "hobbit_gen");
		add_hook(HOOK_WILD_GEN, quest_hobbit_town_gen_hook, "hobbit_town_gen");
		add_hook(HOOK_CHAT, quest_hobbit_chat_hook, "hobbit_chat");
		add_hook(HOOK_MON_SPEAK, quest_hobbit_speak_hook, "hobbit_speak");
	}
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		add_hook(HOOK_MON_SPEAK, quest_hobbit_speak_hook, "hobbit_speak");
		add_hook(HOOK_WILD_GEN, quest_hobbit_town_gen_hook, "hobbit_town_gen");
		add_hook(HOOK_CHAT, quest_hobbit_chat_hook, "hobbit_chat");
	}
	add_hook(HOOK_CHAR_DUMP, quest_hobbit_dump_hook, "hobbit_dump");
	return (FALSE);
}
