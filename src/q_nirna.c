#undef cquest
#define cquest (quest[QUEST_NIRNAETH])

bool_ quest_nirnaeth_gen_hook(char *fmt)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_NIRNAETH) return FALSE;

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
	process_dungeon_file("nirnaeth.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	/* Count the number of monsters */
	cquest.data[0] = 0;
	cquest.data[1] = 0;
	for (x = 2; x < xstart; x++)
		for (y = 2; y < ystart; y++)
		{
			if (cave[y][x].m_idx) cquest.data[0]++;
		}

	return TRUE;
}
bool_ quest_nirnaeth_finish_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_NIRNAETH) return FALSE;

	/* Killed at least 2/3 of them ? better reward ! */
	if (cquest.data[1] >= (2 * cquest.data[0] / 3))
	{
		c_put_str(TERM_YELLOW, "Not only did you found a way out, but you also destroyed a good", 8, 0);
		c_put_str(TERM_YELLOW, "number of trolls! Thank you so much. Take this gold please.", 9, 0);
		c_put_str(TERM_YELLOW, "I also grant you access to the royal jewelry shop!", 10, 0);

		p_ptr->au += 200000;

		/* Redraw gold */
		p_ptr->redraw |= (PR_GOLD);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
	else
	{
		c_put_str(TERM_YELLOW, "I thank you for your efforts.", 8, 0);
		c_put_str(TERM_YELLOW, "I grant you access to the royal jewelry shop!", 9, 0);
	}

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook(HOOK_QUEST_FINISH, quest_nirnaeth_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool_ quest_nirnaeth_death_hook(char *fmt)
{
	if (p_ptr->inside_quest != QUEST_NIRNAETH) return FALSE;

	cquest.data[1]++;

	return FALSE;
}
bool_ quest_nirnaeth_stair_hook(char *fmt)
{
	if (p_ptr->inside_quest != QUEST_NIRNAETH) return FALSE;

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS) return (FALSE);

	cmsg_print(TERM_YELLOW, "You found a way out!");
	cquest.status = QUEST_STATUS_COMPLETED;
	del_hook(HOOK_STAIR, quest_nirnaeth_stair_hook);
	process_hooks_restart = TRUE;
	return (FALSE);
}
bool_ quest_nirnaeth_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_nirnaeth_death_hook, "nirnaeth_death");
		add_hook(HOOK_GEN_QUEST, quest_nirnaeth_gen_hook, "nirnaeth_gen");
		add_hook(HOOK_STAIR, quest_nirnaeth_stair_hook, "nirnaeth_stair");
		add_hook(HOOK_QUEST_FINISH, quest_nirnaeth_finish_hook, "nirnaeth_finish");
	}
	return (FALSE);
}
