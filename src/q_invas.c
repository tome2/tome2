#undef cquest
#define cquest (quest[QUEST_INVASION])

bool_ quest_invasion_gen_hook(char *fmt)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_INVASION) return FALSE;

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
	process_dungeon_file("maeglin.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	for (x = 3; x < xstart; x++)
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

	return TRUE;
}
bool_ quest_invasion_ai_hook(char *fmt)
{
	monster_type *m_ptr;
	s32b m_idx;

	m_idx = get_next_arg(fmt);
	m_ptr = &m_list[m_idx];

	if (p_ptr->inside_quest != QUEST_INVASION) return FALSE;

	/* Ugly but thats better than a call to test_monster_name which is SLOW */
	if (m_ptr->r_idx == 825)
	{
		/* Oups he fleed */
		if ((m_ptr->fy == cquest.data[0]) && (m_ptr->fx == cquest.data[1]))
		{
			delete_monster_idx(m_idx);

			cmsg_print(TERM_YELLOW, "Maeglin found the way to Gondolin! All hope is lost now!");
			cquest.status = QUEST_STATUS_FAILED;
			town_info[2].destroyed = TRUE;
			return (FALSE);
		}

		/* Attack or flee ?*/
		if (distance(m_ptr->fy, m_ptr->fx, p_ptr->py, p_ptr->px) <= 2)
		{
			return (FALSE);
		}
		else
		{
			process_hooks_return[0].num = cquest.data[0];
			process_hooks_return[1].num = cquest.data[1];
			return (TRUE);
		}
	}

	return (FALSE);
}
bool_ quest_invasion_turn_hook(char *fmt)
{
	bool_ old_quick_messages = quick_messages;

	if (cquest.status != QUEST_STATUS_UNTAKEN) return (FALSE);
	if (p_ptr->lev < 45) return (FALSE);

	/* Wait until the end of the current quest */
	if (p_ptr->inside_quest) return ( FALSE);

	/* Wait until the end of the astral mode */
	if (p_ptr->astral) return ( FALSE);

	/* Ok give the quest */
	quick_messages = FALSE;
	cmsg_print(TERM_YELLOW, "A Thunderlord appears in front of you and says:");
	cmsg_print(TERM_YELLOW, "'Hello, noble hero. I am Liron, rider of Tolan. Turgon, King of Gondolin sent me.'");
	cmsg_print(TERM_YELLOW, "'Gondolin is being invaded; he needs your help now or everything will be lost.'");
	cmsg_print(TERM_YELLOW, "'Please come quickly!'");

	cquest.status = QUEST_STATUS_TAKEN;

	quick_messages = old_quick_messages;

	quest_invasion_init_hook(QUEST_INVASION);
	del_hook(HOOK_END_TURN, quest_invasion_turn_hook);
	process_hooks_restart = TRUE;
	return (FALSE);
}
bool_ quest_invasion_dump_hook(char *fmt)
{
	if (cquest.status == QUEST_STATUS_FAILED)
	{
		fprintf(hook_file, "\n You abandoned Gondolin when it most needed you, thus causing its destruction.");
	}
	if ((cquest.status == QUEST_STATUS_FINISHED) || (cquest.status == QUEST_STATUS_REWARDED) || (cquest.status == QUEST_STATUS_COMPLETED))
	{
		fprintf(hook_file, "\n You saved Gondolin from destruction.");
	}
	return (FALSE);
}
bool_ quest_invasion_death_hook(char *fmt)
{
	s32b r_idx, m_idx;

	m_idx = get_next_arg(fmt);
	r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_INVASION) return FALSE;

	if (r_idx == test_monster_name("Maeglin, the Traitor of Gondolin"))
	{
		cmsg_print(TERM_YELLOW, "You did it! Gondolin will remain hidden.");
		cquest.status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_MONSTER_DEATH, quest_invasion_death_hook);
		process_hooks_restart = TRUE;
		return (FALSE);
	}

	return FALSE;
}
bool_ quest_invasion_stair_hook(char *fmt)
{
	cptr down;

	down = get_next_arg_str(fmt);

	if (p_ptr->inside_quest != QUEST_INVASION) return FALSE;

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS) return TRUE;

	if (!strcmp(down, "up"))
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

			if (!get_check("Really abandon the quest?")) return TRUE;
			cmsg_print(TERM_YELLOW, "You flee away from Maeglin and his army...");
			cquest.status = QUEST_STATUS_FAILED;
			town_info[2].destroyed = TRUE;
		}
		del_hook(HOOK_STAIR, quest_invasion_stair_hook);
		process_hooks_restart = TRUE;
		return (FALSE);
	}

	return TRUE;
}
bool_ quest_invasion_init_hook(int q_idx)
{
	add_hook(HOOK_END_TURN, quest_invasion_turn_hook, "invasion_turn");
	add_hook(HOOK_CHAR_DUMP, quest_invasion_dump_hook, "invasion_dump");
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_AI, quest_invasion_ai_hook, "invasion_ai");
		add_hook(HOOK_GEN_QUEST, quest_invasion_gen_hook, "invasion_gen");
		add_hook(HOOK_MONSTER_DEATH, quest_invasion_death_hook, "invasion_death");
		add_hook(HOOK_STAIR, quest_invasion_stair_hook, "invasion_stair");
	}
	return (FALSE);
}
