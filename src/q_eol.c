#undef cquest
#define cquest (quest[QUEST_EOL])

bool quest_eol_gen_hook(char *fmt)
{
	int x, y;
	bool done = FALSE;
	int xsize = 50, ysize = 30, y0, x0;
	int m_idx = 0;

	if (p_ptr->inside_quest != QUEST_EOL) return FALSE;

	x0 = 2 + (xsize / 2);
	y0 = 2 + (ysize / 2);

	feat_wall_outer = FEAT_WALL_OUTER;
	feat_wall_inner = FEAT_WALL_INNER;

	for (y = 0; y < 100; y++)
	{
		floor_type[y] = FEAT_FLOOR;
		fill_type[y] = FEAT_WALL_OUTER;
	}

	/* Start with perm walls */
	for (y = 0; y < cur_hgt; y++)
	{
		for (x = 0; x < cur_wid; x++)
		{
			cave_set_feat(y, x, FEAT_PERM_SOLID);
		}
	}
	dun_level = quest[p_ptr->inside_quest].level;

	while (!done)
	{
		int grd, roug, cutoff;

		/* testing values for these parameters feel free to adjust*/
		grd = 2 ^ (randint(4));

		/* want average of about 16 */
		roug = randint(8) * randint(4);

		/* about size/2 */
		cutoff = randint(xsize / 4) + randint(ysize / 4) + randint(xsize / 4) + randint(ysize / 4);

		/* make it */
		generate_hmap(y0, x0, xsize, ysize, grd, roug, cutoff);

		/* Convert to normal format+ clean up*/
		done = generate_fracave(y0, x0, xsize, ysize, cutoff, FALSE, TRUE);
	}

	/* Place a few traps */
	for (x = xsize - 1; x >= 2; x--)
		for (y = ysize - 1; y >= 2; y--)
		{
			if (!cave_clean_bold(y, x)) continue;

			/* Place eol at the other end */
			if (!m_idx)
			{
				m_allow_special[test_monster_name("Eol, the Dark Elf")] = TRUE;
				m_idx = place_monster_one(y, x, test_monster_name("Eol, the Dark Elf"), 0, FALSE, MSTATUS_ENEMY);
				m_allow_special[test_monster_name("Eol, the Dark Elf")] = FALSE;
			}

			if (magik(18))
			{
				place_trap(y, x);
			}

			/* Place player at one end */
			p_ptr->py = y;
			p_ptr->px = x;
		}

	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);

	return TRUE;
}
bool quest_eol_finish_hook(char *fmt)
{
	object_type forge, *q_ptr;
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_EOL) return FALSE;

	c_put_str(TERM_YELLOW, "A tragedy, but the deed needed to be done.", 8, 0);
	c_put_str(TERM_YELLOW, "Accept my thanks, and that reward.", 9, 0);

	q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_LITE, SV_LITE_DWARVEN));
	q_ptr->found = OBJ_FOUND_REWARD;
	q_ptr->name2 = EGO_LITE_MAGI;
	apply_magic(q_ptr, 1, FALSE, FALSE, FALSE);
	q_ptr->number = 1;
	object_aware(q_ptr);
	object_known(q_ptr);
	q_ptr->ident |= IDENT_STOREB;
	(void)inven_carry(q_ptr, FALSE);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NIRNAETH;
	quest[*(quest[q_idx].plot)].init(*(quest[q_idx].plot));

	del_hook(HOOK_QUEST_FINISH, quest_eol_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool quest_eol_fail_hook(char *fmt)
{
	s32b q_idx;

	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_EOL) return FALSE;

	c_put_str(TERM_YELLOW, "You fled ! I did not think you would flee...", 8, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook(HOOK_QUEST_FAIL, quest_eol_fail_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool quest_eol_death_hook(char *fmt)
{
	s32b r_idx, m_idx;

	m_idx = get_next_arg(fmt);
	r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_EOL) return FALSE;

	if (r_idx == test_monster_name("Eol, the Dark Elf"))
	{
		cmsg_print(TERM_YELLOW, "Such a sad end...");
		cquest.status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_MONSTER_DEATH, quest_eol_death_hook);
		process_hooks_restart = TRUE;
		return (FALSE);
	}

	return FALSE;
}
bool quest_eol_stair_hook(char *fmt)
{
	monster_race *r_ptr = &r_info[test_monster_name("Eol, the Dark Elf")];
	cptr down;

	down = get_next_arg_str(fmt);

	if (p_ptr->inside_quest != QUEST_EOL) return FALSE;

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS) return TRUE;

	if (r_ptr->max_num)
	{
		if (!strcmp(down, "up"))
		{
			/* Flush input */
			flush();

			if (!get_check("Really abandon the quest?")) return TRUE;

			cmsg_print(TERM_YELLOW, "You flee away from Eol...");
			cquest.status = QUEST_STATUS_FAILED;
			del_hook(HOOK_STAIR, quest_eol_stair_hook);
			process_hooks_restart = TRUE;
			return (FALSE);
		}
	}

	return FALSE;
}
bool quest_eol_init_hook(int q)
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_eol_death_hook, "eol_death");
		add_hook(HOOK_GEN_QUEST, quest_eol_gen_hook, "eol_gen");
		add_hook(HOOK_STAIR, quest_eol_stair_hook, "eol_stair");
		add_hook(HOOK_QUEST_FAIL, quest_eol_fail_hook, "eol_fail");
		add_hook(HOOK_QUEST_FINISH, quest_eol_finish_hook, "eol_finish");
	}
	return (FALSE);
}
