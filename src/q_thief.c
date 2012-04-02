#undef cquest
#define cquest (quest[QUEST_THIEVES])

bool_ quest_thieves_gen_hook(char *fmt)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;
	bool_ again = TRUE;

	if (p_ptr->inside_quest != QUEST_THIEVES) return FALSE;

	/* Just in case we didnt talk the the mayor */
	if (cquest.status == QUEST_STATUS_UNTAKEN)
		cquest.status = QUEST_STATUS_TAKEN;

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
	process_dungeon_file("thieves.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	dungeon_flags2 |= DF2_NO_GENO;

	/* Rip the inventory from the player */
	cmsg_print(TERM_YELLOW, "You feel a vicious blow on your head.");
	while (again)
	{
		again = FALSE;
		for (x = 0; x < INVEN_TOTAL; x++)
		{
			object_type *o_ptr = &p_ptr->inventory[x];

			if (!o_ptr->k_idx) continue;

			if ((x >= INVEN_WIELD) && cursed_p(o_ptr)) continue;

			inven_drop(x, 99, 4, 24, TRUE);

			/* Thats ugly .. but it works */
			again = TRUE;
			break;
		}
	}

	del_hook(HOOK_GEN_QUEST, quest_thieves_gen_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool_ quest_thieves_hook(char *fmt)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_THIEVES) return FALSE;

	/* ALARM !!! */
	if ((cave[17][22].feat == FEAT_OPEN) ||
	                (cave[17][22].feat == FEAT_BROKEN))
	{
		cmsg_print(TERM_L_RED, "An alarm rings!");
		aggravate_monsters(0);
		cave_set_feat(14, 20, FEAT_OPEN);
		cave_set_feat(14, 16, FEAT_OPEN);
		cave_set_feat(14, 12, FEAT_OPEN);
		cave_set_feat(14, 8, FEAT_OPEN);
		cave_set_feat(20, 20, FEAT_OPEN);
		cave_set_feat(20, 16, FEAT_OPEN);
		cave_set_feat(20, 12, FEAT_OPEN);
		cave_set_feat(20, 8, FEAT_OPEN);
		msg_print("The door explodes.");
		cave_set_feat(17, 22, FEAT_FLOOR);
	}

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status < MSTATUS_FRIEND) mcnt++;
	}

	/* Nobody left ? */
	if (!mcnt)
	{
		msg_print("The magic hiding the stairs is now gone.");
		cave_set_feat(23, 4, FEAT_LESS);
		cave[23][4].special = 0;

		quest[p_ptr->inside_quest].status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_END_TURN, quest_thieves_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "You stopped the thieves and saved Bree!");
		return (FALSE);
	}
	return FALSE;
}
bool_ quest_thieves_finish_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_THIEVES) return FALSE;

	c_put_str(TERM_YELLOW, "Thank you for killing the band of thieves!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the hideout as your house as a reward.", 9, 0);

	/* Continue the plot */

	/* 10% chance to randomly select, otherwise use the combat/magic skill ratio */
	if (magik(10) || (s_info[SKILL_COMBAT].value == s_info[SKILL_MAGIC].value))
	{
		*(quest[q_idx].plot) = (magik(50)) ? QUEST_TROLL : QUEST_WIGHT;
	}
	else
	{
		if (s_info[SKILL_COMBAT].value > s_info[SKILL_MAGIC].value)
			*(quest[q_idx].plot) = QUEST_TROLL;
		else
			*(quest[q_idx].plot) = QUEST_WIGHT;
	}
	quest[*(quest[q_idx].plot)].init(*(quest[q_idx].plot));

	del_hook(HOOK_QUEST_FINISH, quest_thieves_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

bool_ quest_thieves_feeling_hook(char *fmt)
{
	if (p_ptr->inside_quest != QUEST_THIEVES) return FALSE;

	msg_print("You wake up in a prison cell.");
	msg_print("All your possessions have been stolen!");

	del_hook(HOOK_FEELING, quest_thieves_feeling_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

bool_ quest_thieves_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_END_TURN, quest_thieves_hook, "thieves_end_turn");
		add_hook(HOOK_QUEST_FINISH, quest_thieves_finish_hook, "thieves_finish");
		add_hook(HOOK_GEN_QUEST, quest_thieves_gen_hook, "thieves_geb");
		add_hook(HOOK_FEELING, quest_thieves_feeling_hook, "thieves_feel");
	}
	return (FALSE);
}
