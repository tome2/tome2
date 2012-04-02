#undef cquest
#define cquest (quest[QUEST_DRAGONS])

bool_ quest_dragons_gen_hook(char *fmt)
{
	int x, y, i;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_DRAGONS) return FALSE;

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
	process_dungeon_file("dragons.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, FALSE);
	dungeon_flags2 |= DF2_NO_GENO;

	/* Place some columns */
	for (i = 35; i > 0; )
	{
		int flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		/* Bar columns on even squares so the whole level is guaranteed to be
		   accessible */
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags % 2) && !(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			--i;
			cave_set_feat(y, x, FEAT_MOUNTAIN);
		}
	}

	/* Place some random dragons */
	for (i = 25; i > 0; )
	{
		int m_idx, flags;
		y = rand_int(21) + 3;
		x = rand_int(31) + 3;
		flags = f_info[cave[y][x].feat].flags1;
		if (!(flags & FF1_PERMANENT) && (flags & FF1_FLOOR))
		{
			/*                       blue, white, red, black, bronze, gold, green, multi-hued */
			int baby_dragons[8] = {163, 164, 167, 166, 218, 219, 165, 204};
			int young_dragons[8] = {459, 460, 563, 546, 462, 559, 461, 556};
			int mature_dragons[8] = {560, 549, 589, 592, 562, 590, 561, 593};
			int happy_dragons[8] = {601, 617, 644, 624, 602, 645, 618, 675};

			int chance, dragon, color;

			color = rand_int(8);
			chance = rand_int(100);
			if (chance == 0)
				dragon = happy_dragons[color];
			else if (chance < 33)
				dragon = baby_dragons[color];
			else if (chance < 66)
				dragon = young_dragons[color];
			else
				dragon = mature_dragons[color];

			--i;
			m_idx = place_monster_one(y, x, dragon, 0, magik(33), MSTATUS_ENEMY);
			if (m_idx) m_list[m_idx].mflag |= MFLAG_QUEST;
		}
	}

	process_hooks_restart = TRUE;

	return TRUE;
}

bool_ quest_dragons_death_hook(char *fmt)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_DRAGONS) return FALSE;

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
		del_hook(HOOK_MONSTER_DEATH, quest_dragons_death_hook);
		del_hook(HOOK_GEN_QUEST, quest_dragons_gen_hook);
		process_hooks_restart = TRUE;

		cmsg_print(TERM_YELLOW, "Gondolin is safer now.");
		return (FALSE);
	}
	return FALSE;
}

bool_ quest_dragons_finish_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_DRAGONS) return FALSE;

	c_put_str(TERM_YELLOW, "Thank you for killing the dragons!", 8, 0);
	c_put_str(TERM_YELLOW, "You can use the cave as your house as a reward.", 9, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_EOL;

	return TRUE;
}

bool_ quest_dragons_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_dragons_death_hook, "dragons_monster_death");
		add_hook(HOOK_QUEST_FINISH, quest_dragons_finish_hook, "dragons_finish");
		add_hook(HOOK_GEN_QUEST, quest_dragons_gen_hook, "dragons_geb");
	}
	return (FALSE);
}
