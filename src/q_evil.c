#undef cquest
#define cquest (quest[QUEST_EVIL])

bool_ quest_evil_gen_hook(char *fmt)
{
	int x, y, i;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_EVIL) return FALSE;

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
	process_dungeon_file("evil.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	dungeon_flags2 |= DF2_NO_GENO;

	/* Place some random balrogs */
	for (i = 6; i > 0; )
	{
		int m_idx, flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			m_idx = place_monster_one(y, x, 996, 0, FALSE, MSTATUS_ENEMY);
			if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
			--i;
		}
	}

	process_hooks_restart = TRUE;

	return TRUE;
}

bool_ quest_evil_death_hook(char *fmt)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_EVIL) return FALSE;

	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status <= MSTATUS_ENEMY) mcnt++;
	}

	/* Nobody left ? */
	if (mcnt <= 1)
	{
		/* TODO: change to COMPLETED and remove NULL when mayor is added */
		quest[p_ptr->inside_quest].status = QUEST_STATUS_FINISHED;
		*(quest[p_ptr->inside_quest].plot) = QUEST_NULL;
		del_hook(HOOK_MONSTER_DEATH, quest_evil_death_hook);
		del_hook(HOOK_GEN_QUEST, quest_evil_gen_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "Khazad-Dum is safer now.");
		return (FALSE);
	}
	return FALSE;
}

bool_ quest_evil_finish_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_EVIL) return FALSE;

	c_put_str(TERM_YELLOW, "Thank you for saving us!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the cave as your house as a reward.", 9, 0);

	/* End the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	return TRUE;
}

bool_ quest_evil_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_evil_death_hook, "evil_monster_death");
		add_hook(HOOK_QUEST_FINISH, quest_evil_finish_hook, "evil_finish");
		add_hook(HOOK_GEN_QUEST, quest_evil_gen_hook, "evil_geb");
	}
	return (FALSE);
}
