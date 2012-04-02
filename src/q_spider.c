#undef cquest
#define cquest (quest[QUEST_SPIDER])

bool_ quest_spider_gen_hook(char *fmt)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_SPIDER) return FALSE;

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
	process_dungeon_file("spiders.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	return TRUE;
}
bool_ quest_spider_death_hook(char *fmt)
{
	int i, mcnt = 0;

	if (p_ptr->inside_quest != QUEST_SPIDER) return FALSE;

	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status <= MSTATUS_ENEMY) mcnt++;
	}

	if (mcnt <= 1)
	{
		cmsg_print(TERM_YELLOW, "The forest is now safer, thanks to you.");

		/* Yavanna LOVES saving forests */
		GOD(GOD_YAVANNA)
		{
			cmsg_print(TERM_L_GREEN, "You feel the gentle touch of Yavanna, as she smiles at you.");
			inc_piety(GOD_YAVANNA, 6000);
		}

		cquest.status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_MONSTER_DEATH, quest_spider_death_hook);
		process_hooks_restart = TRUE;
		return (FALSE);
	}

	return (FALSE);
}
bool_ quest_spider_finish_hook(char *fmt)
{
	object_type forge, *q_ptr;
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_SPIDER) return FALSE;

	c_put_str(TERM_YELLOW, "All of us praise your mighty deed in driving back the", 8, 0);
	c_put_str(TERM_YELLOW, "menace. Take this as a reward.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_POTION, SV_POTION_AUGMENTATION));
	q_ptr->number = 1;
	q_ptr->found = OBJ_FOUND_REWARD;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_STOREB;
	(void)inven_carry(q_ptr, FALSE);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_POISON;
	quest[*(quest[q_idx].plot)].init(*(quest[q_idx].plot));

	del_hook(HOOK_QUEST_FINISH, quest_spider_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool_ quest_spider_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_spider_death_hook, "spider_death");
		add_hook(HOOK_GEN_QUEST, quest_spider_gen_hook, "spider_gen");
		add_hook(HOOK_QUEST_FINISH, quest_spider_finish_hook, "spider_finish");
	}
	return (FALSE);
}
