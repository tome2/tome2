#undef cquest
#define cquest (quest[QUEST_POISON])

static int wild_locs[4][2] =
{
	{ 32, 49, },
	{ 32, 48, },
	{ 33, 48, },
	{ 34, 48, },
};

static bool create_molds_hook(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags4 & RF4_MULTIPLY) return FALSE;

	if (r_ptr->d_char == 'm') return TRUE;
	else if (r_ptr->d_char == ',') return TRUE;
	else if (r_ptr->d_char == 'e') return TRUE;
	else return FALSE;
}

bool quest_poison_gen_hook(char *fmt)
{
	int cy = 1, cx = 1, x, y, try = 10000, r_idx;
	bool (*old_get_mon_num_hook)(int r_idx);

	if (cquest.status != QUEST_STATUS_TAKEN) return FALSE;
	if (p_ptr->wilderness_y != wild_locs[cquest.data[0]][0]) return FALSE;
	if (p_ptr->wilderness_x != wild_locs[cquest.data[0]][1]) return FALSE;
	if (p_ptr->wild_mode) return FALSE;

	/* Find a good position */
	while (try)
	{
		/* Get a random spot */
		cy = randint(cur_hgt - 24) + 22;
		cx = randint(cur_wid - 34) + 32;

		/* Is it a good spot ? */
		if (cave_empty_bold(cy, cx)) break;

		/* One less try */
		try--;
	}

	/* Place the baddies */

	/* Backup the old hook */
	old_get_mon_num_hook = get_mon_num_hook;

	/* Require "okay" monsters */
	get_mon_num_hook = create_molds_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick a monster, using the level calculation */
	for (x = cx - 25; x <= cx + 25; x++)
		for (y = cy - 25; y <= cy + 25; y++)
		{
			if (!in_bounds(y, x)) continue;

			if (distance(cy, cx, y, x) > 25) continue;

			if (magik(80) && ((cave[y][x].feat == FEAT_DEEP_WATER) || (cave[y][x].feat == FEAT_SHAL_WATER))) cave_set_feat(y, x, FEAT_TAINTED_WATER);

			if (distance(cy, cx, y, x) > 10) continue;

			if (magik(60))
			{
				int m_idx;

				r_idx = get_mon_num(30);
				m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_ENEMY);

				/* Sometimes make it up some levels */
				if (magik(80) && m_idx)
				{
					monster_type *m_ptr = &m_list[m_idx];

					if (m_ptr->level < p_ptr->lev)
					{
						m_ptr->exp = MONSTER_EXP(m_ptr->level + randint(p_ptr->lev - m_ptr->level));
						monster_check_experience(m_idx, TRUE);
					}
				}
			}
		}

	/* Reset restriction */
	get_mon_num_hook = old_get_mon_num_hook;

	/* Prepare allocation table */
	get_mon_num_prep();

	return FALSE;
}
bool quest_poison_finish_hook(char *fmt)
{
	object_type forge, *q_ptr;
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_POISON) return FALSE;

	c_put_str(TERM_YELLOW, "The water is clean again! Thank you so much.", 8, 0);
	c_put_str(TERM_YELLOW, "The beautiful Mallorns are safe. Take this as a proof of our gratitude.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_DRAG_ARMOR, SV_DRAGON_BLUE));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->number = 1;
	q_ptr->name2 = EGO_ELVENKIND;
	apply_magic(q_ptr, 1, FALSE, FALSE, FALSE);
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_STOREB;
	(void)inven_carry(q_ptr, FALSE);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook(HOOK_QUEST_FINISH, quest_poison_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool quest_poison_dump_hook(char *fmt)
{
	if (cquest.status >= QUEST_STATUS_COMPLETED)
	{
		fprintf(hook_file, "\n You saved the beautiful Mallorns of Lothlorien.");
	}
	return (FALSE);
}
bool quest_poison_quest_hook(char *fmt)
{
	object_type forge, *q_ptr;
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_POISON) return FALSE;

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_POTION2, SV_POTION2_CURE_WATER));
	q_ptr->number = 99;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_STOREB;
	q_ptr->note = quark_add("quest");
	(void)inven_carry(q_ptr, FALSE);

	del_hook(HOOK_INIT_QUEST, quest_poison_quest_hook);
	process_hooks_restart = TRUE;

	return FALSE;
}
bool quest_poison_drop_hook(char *fmt)
{
	s32b mcnt = 0, i, x, y, o_idx;
	object_type *o_ptr;

	o_idx = get_next_arg(fmt);
	o_ptr = &p_ptr->inventory[o_idx];

	if (cquest.status != QUEST_STATUS_TAKEN) return FALSE;
	if (p_ptr->wilderness_y != wild_locs[cquest.data[0]][0]) return FALSE;
	if (p_ptr->wilderness_x != wild_locs[cquest.data[0]][1]) return FALSE;
	if (p_ptr->wild_mode) return FALSE;

	if (o_ptr->tval != TV_POTION2) return FALSE;
	if (o_ptr->sval != SV_POTION2_CURE_WATER) return FALSE;

	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		if (m_ptr->status <= MSTATUS_NEUTRAL) mcnt++;
	}

	if (mcnt < 10)
	{
		for (x = 1; x < cur_wid - 1; x++)
			for (y = 1; y < cur_hgt - 1; y++)
			{
				if (!in_bounds(y, x)) continue;

				if (cave[y][x].feat == FEAT_TAINTED_WATER) cave_set_feat(y, x, FEAT_SHAL_WATER);
			}

		cmsg_print(TERM_YELLOW, "Well done! The water seems to be clean now.");

		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook(HOOK_DROP, quest_poison_drop_hook);
		process_hooks_restart = TRUE;

		return FALSE;
	}
	else
	{
		msg_print("There are too many monsters left to cure the water.");
		return TRUE;
	}
	return FALSE;
}
bool quest_poison_init_hook(int q_idx)
{
	/* Get a place to place the poison */
	if (!cquest.data[1])
	{
		cquest.data[1] = TRUE;
		cquest.data[0] = rand_int(4);
		if (wizard) message_add(MESSAGE_MSG, format("Wilderness poison %d, %d", wild_locs[cquest.data[0]][0], wild_locs[cquest.data[0]][1]), TERM_BLUE);
	}

	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_DROP, quest_poison_drop_hook, "poison_drop");
		add_hook(HOOK_WILD_GEN, quest_poison_gen_hook, "poison_gen");
		add_hook(HOOK_QUEST_FINISH, quest_poison_finish_hook, "poison_finish");
	}
	if (cquest.status < QUEST_STATUS_COMPLETED)
	{
		add_hook(HOOK_INIT_QUEST, quest_poison_quest_hook, "poison_iquest");
	}
	add_hook(HOOK_CHAR_DUMP, quest_poison_dump_hook, "poison_dump");
	return (FALSE);
}
