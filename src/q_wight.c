#undef cquest
#define cquest (quest[QUEST_WIGHT])

bool quest_wight_gen_hook(char *fmt)
{
	int x, y;
	int xstart = 2;
	int ystart = 2;

	if (p_ptr->inside_quest != QUEST_WIGHT) return FALSE;

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
	process_dungeon_file_full = TRUE;
	process_dungeon_file(NULL, "wights.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE);
	process_dungeon_file_full = FALSE;

	for (x = 3; x < xstart; x++)
		for (y = 3; y < ystart; y++)
		{
			if (cave[y][x].feat == FEAT_MARKER)
			{
				int m_idx = 0;

				m_allow_special[test_monster_name("The Wight-King of the Barrow-downs")] = TRUE;
				m_idx = place_monster_one(y, x, test_monster_name("The Wight-King of the Barrow-downs"), 0, FALSE, MSTATUS_ENEMY);
				m_allow_special[test_monster_name("The Wight-King of the Barrow-downs")] = FALSE;

				if (m_idx)
				{
					int o_idx;

					/* Get local object */
					object_type forge, *q_ptr = &forge;

					/* Prepare to make the  */
					object_prep(q_ptr, lookup_kind(TV_SOFT_ARMOR, SV_FILTHY_RAG));

					/* Name the rags */

					q_ptr->art_name = quark_add("of the Wight");

					q_ptr->art_flags1 |= ( TR1_INT | TR1_SEARCH );
					q_ptr->art_flags2 |= ( TR2_RES_BLIND | TR2_SENS_FIRE | TR2_RES_CONF );
					q_ptr->art_flags3 |= ( TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
					                       TR3_IGNORE_FIRE | TR3_IGNORE_COLD | TR3_SEE_INVIS);

					/* For game balance... */
					q_ptr->art_flags3 |= (TR3_CURSED | TR3_HEAVY_CURSE);
					q_ptr->ident |= IDENT_CURSED;

					if (randint(2) == 1)
					{
						q_ptr->art_flags1 |= (TR1_SPELL);
						q_ptr->pval = 6;
					}
					else
					{
						q_ptr->art_flags1 |= (TR1_MANA);
						q_ptr->pval = 2;
					}

					/* Get new object */
					o_idx = o_pop();

					if (o_idx)
					{
						/* Get the item */
						object_type *o_ptr = &o_list[o_idx];

						/* Structure copy */
						object_copy(o_ptr, q_ptr);

						/* Build a stack */
						o_ptr->next_o_idx = m_list[m_idx].hold_o_idx;

						o_ptr->held_m_idx = m_idx;
						o_ptr->ix = 0;
						o_ptr->iy = 0;

						m_list[m_idx].hold_o_idx = o_idx;
					}
				}
			}
		}

	return TRUE;
}
bool quest_wight_death_hook(char *fmt)
{
	s32b r_idx, m_idx;

	m_idx = get_next_arg(fmt);
	r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_WIGHT) return FALSE;

	if (r_idx == test_monster_name("The Wight-King of the Barrow-downs"))
	{
		cmsg_print(TERM_YELLOW, "Without their King the wights won't be able to do much.");

		cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);
		cave[p_ptr->py][p_ptr->px].special = 0;

		cquest.status = QUEST_STATUS_COMPLETED;
		del_hook(HOOK_MONSTER_DEATH, quest_wight_death_hook);
		process_hooks_restart = TRUE;
		return (FALSE);
	}

	return (FALSE);
}
bool quest_wight_finish_hook(char *fmt)
{
	s32b q_idx;
	q_idx = get_next_arg(fmt);

	if (q_idx != QUEST_WIGHT) return FALSE;

	c_put_str(TERM_YELLOW, "I heard about your noble deeds.", 8, 0);
	c_put_str(TERM_YELLOW, "Keep what you found ..  may it serve you well.", 9, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NAZGUL;
	quest[*(quest[q_idx].plot)].init(*(quest[q_idx].plot));

	del_hook(HOOK_QUEST_FINISH, quest_wight_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}
bool quest_wight_init_hook(int q_idx)
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MONSTER_DEATH, quest_wight_death_hook, "wight_death");
		add_hook(HOOK_GEN_QUEST, quest_wight_gen_hook, "wight_gen");
		add_hook(HOOK_QUEST_FINISH, quest_wight_finish_hook, "wight_finish");
	}
	return (FALSE);
}
