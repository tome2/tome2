#undef cquest
#define cquest (quest[QUEST_HAUNTED])

bool quest_haunted_gen_hook(char *fmt)
{
	int x, y, i;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_HAUNTED) return FALSE;

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
	process_dungeon_file(NULL, "haunted.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE);
	dungeon_flags2 |= DF2_NO_GENO;

	/* Place some ghosts */
	for (i = 12; i > 0; )
	{
		int flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			place_monster_one(y, x, 477, 0, FALSE, MSTATUS_ENEMY);
			--i;
		}
	}

	/* Place some random monsters to haunt us */
	for (i = damroll(4, 4); i > 0; )
	{
		int flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			int monsters[22] = { 65, 100, 124, 125, 133, 231, 273, 327, 365, 416, 418,
			                     507, 508, 533, 534, 553, 554, 555, 577, 607, 622, 665};
			int monster = monsters[rand_int(22)];
			place_monster_one(y, x, monster, 0, FALSE, MSTATUS_ENEMY);
			--i;
		}
	}

	/* Place some random traps */
	for (i = 10 + damroll(4, 4); i > 0; )
	{
		int flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			--i;
			place_trap(y, x);
		}
	}

	process_hooks_restart = TRUE;

	return TRUE;
}

bool quest_haunted_death_hook(char *fmt)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_HAUNTED) return FALSE;

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
		quest[p_ptr->inside_quest].status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_MONSTER_DEATH, quest_haunted_death_hook);
		del_hook(HOOK_GEN_QUEST, quest_haunted_gen_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "Minas Anor is safer now.");
		return (FALSE);
	}
	return FALSE;
}

bool quest_haunted_finish_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_HAUNTED) return FALSE;

	c_put_str(TERM_YELLOW, "Thank you for saving us!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the building as your house as a reward.", 9, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_BETWEEN;

	return TRUE;
}

bool quest_haunted_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_haunted_death_hook, "haunted_monster_death");
		add_hook(HOOK_QUEST_FINISH, quest_haunted_finish_hook, "haunted_finish");
		add_hook(HOOK_GEN_QUEST, quest_haunted_gen_hook, "haunted_geb");
	}
	return (FALSE);
}
