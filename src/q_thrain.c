#undef cquest
#define cquest (quest[QUEST_THRAIN])

bool quest_thrain_death_hook(char *fmt)
{
	s32b r_idx, m_idx;
	int r, x, y;
	monster_type *m_ptr;

	m_idx = get_next_arg(fmt);
	r_idx = m_list[m_idx].r_idx;

	if ((cquest.status >= QUEST_STATUS_FINISHED) || (dun_level !=cquest.data[0]) || (dungeon_type != DUNGEON_DOL_GULDUR)) return (FALSE);
	m_ptr = &m_list[m_idx];
	if ((m_ptr->r_idx != test_monster_name("Dwar, Dog Lord of Waw")) && (m_ptr->r_idx != test_monster_name("Hoarmurath of Dir"))) return (FALSE);

	cquest.data[2]++;

	if (cquest.data[2] < 2) return (FALSE);

	cmsg_print(TERM_YELLOW, "The magic hiding the room dissipates.");
	for (x = 0; x < cur_wid; x++)
		for (y = 0; y < cur_hgt; y++)
		{
			cave_type *c_ptr = &cave[y][x];

			if (c_ptr->mimic != 61) continue;
			if (!(c_ptr->info & CAVE_FREE)) continue;

			c_ptr->mimic = 0;
			lite_spot(y, x);
		}

	cquest.status = QUEST_STATUS_FINISHED;
	cmsg_print(TERM_YELLOW, "Thrain speaks:");
	cmsg_print(TERM_YELLOW, "'Ah, at last you came to me!  But... I fear it is too late for me.");
	cmsg_print(TERM_YELLOW, "However your quest continues, you must beware for the Necromancer");
	cmsg_print(TERM_YELLOW, "is in fact Sauron, the Dark Lord! He stole the Ring of Durin and tortured");
	cmsg_print(TERM_YELLOW, "me... arrgh... please make him pay!'");

	/* Look for Thrain */
	for (r = m_max - 1; r >= 1; r--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[r];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Is it the princess? */
		if (m_ptr->r_idx == test_monster_name("Thrain, the King Under the Mountain"))
		{
			int x = m_ptr->fx;
			int y = m_ptr->fy;
			int i, j;
			object_type forge, *q_ptr;

			delete_monster_idx(r);

			/* Wipe the glass walls and create a stair */
			for (i = x - 1; i <= x + 1; i++)
				for (j = y - 1; j <= y + 1; j++)
				{
					if (in_bounds(j, i)) cave_set_feat(j, i, FEAT_FLOOR);
				}

			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);
			object_prep(q_ptr, lookup_kind(TV_HELM, SV_DRAGON_HELM));
			q_ptr->number = 1;
			q_ptr->found = OBJ_FOUND_REWARD;
			create_artifact(q_ptr, FALSE, TRUE);
			q_ptr->art_name = quark_add("of Thrain");

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
			break;
		}
	}


	del_hook(HOOK_MONSTER_DEATH, quest_thrain_death_hook);
	process_hooks_restart = TRUE;

	return (FALSE);
}

bool quest_thrain_gen_hook(char *fmt)
{
	s32b x, y, bx0, by0;
	int xstart;
	int ystart;
	int y2, x2, yval, xval;
	int y1, x1, xsize, ysize;
	monster_type *m_ptr;

	if (dungeon_type != DUNGEON_DOL_GULDUR) return (FALSE);
	if (cquest.data[0] != dun_level) return (FALSE);
	if (cquest.data[1]) return (FALSE);
	if ((cquest.status < QUEST_STATUS_TAKEN) || (cquest.status >= QUEST_STATUS_FINISHED)) return (FALSE);

	by0 = get_next_arg(fmt);
	bx0 = get_next_arg(fmt);

	/* Pick a room size */
	xsize = 0;
	ysize = 0;
	init_flags = INIT_GET_SIZE;
	process_dungeon_file_full = TRUE;
	process_dungeon_file(NULL, "thrain.map", &ysize, &xsize, cur_hgt, cur_wid, TRUE);
	process_dungeon_file_full = FALSE;

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 2, ysize + 2, FALSE, by0, bx0, &xval, &yval)) return FALSE;

	/* Get corner values */
	y1 = yval - ysize / 2;
	x1 = xval - xsize / 2;
	y2 = y1 + ysize - 1;
	x2 = x1 + xsize - 1;

	/* Place a full floor under the room */
	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x <= x2; x++)
		{
			cave_set_feat(y, x, floor_type[rand_int(100)]);
			cave[y][x].info |= (CAVE_ROOM|CAVE_GLOW);
		}
	}

	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, CAVE_ROOM | CAVE_GLOW);

	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	xstart = x1;
	ystart = y1;
	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file_full = TRUE;
	process_dungeon_file(NULL, "thrain.map", &ystart, &xstart, cur_hgt, cur_wid, TRUE);
	process_dungeon_file_full = FALSE;

	for (x = x1; x < xstart; x++)
		for (y = y1; y < ystart; y++)
		{
			cave[y][x].info |= CAVE_ICKY | CAVE_ROOM | CAVE_FREE;
			if (cave[y][x].feat == FEAT_MARKER)
			{
				int i;

				m_allow_special[test_monster_name("Thrain, the King Under the Mountain")] = TRUE;
				i = place_monster_one(y, x, test_monster_name("Thrain, the King Under the Mountain"), 0, FALSE, MSTATUS_NEUTRAL);
				m_allow_special[test_monster_name("Thrain, the King Under the Mountain")] = FALSE;
			}
			if (cave[y][x].m_idx)
			{
				m_ptr = &m_list[cave[y][x].m_idx];
				if ((m_ptr->r_idx == test_monster_name("Dwar, Dog Lord of Waw")) || (m_ptr->r_idx == test_monster_name("Hoarmurath of Dir")))
				{
					m_ptr->mflag |= MFLAG_QUEST;
				}
			}
		}

	/* Don't try another one for this generation */
	cquest.data[1] = 1;

	return (TRUE);
}
bool quest_thrain_feeling_hook(char *fmt)
{
	if (dungeon_type != DUNGEON_DOL_GULDUR) return (FALSE);
	if (cquest.data[0] != dun_level) return (FALSE);
	if (cquest.status != QUEST_STATUS_UNTAKEN) return (FALSE);

	cmsg_format(TERM_YELLOW, "You hear someone shouting under the torture.");
	cquest.status = QUEST_STATUS_TAKEN;
	cquest.init(QUEST_THRAIN);

	return (FALSE);
}
bool quest_thrain_move_hook(char *fmt)
{
	s32b y;
	s32b x;
	cave_type *c_ptr;

	y = get_next_arg(fmt);
	x = get_next_arg(fmt);
	c_ptr = &cave[y][x];

	if (dungeon_type != DUNGEON_DOL_GULDUR) return (FALSE);
	if (cquest.data[0] != dun_level) return (FALSE);
	if ((cquest.status < QUEST_STATUS_TAKEN) || (cquest.status >= QUEST_STATUS_FINISHED)) return (FALSE);
	if (!(c_ptr->info & CAVE_FREE)) return (FALSE);
	if (c_ptr->mimic != 61) return (FALSE);

	cmsg_print(TERM_YELLOW, "The magic hiding the room dissipates.");
	for (x = 0; x < cur_wid; x++)
		for (y = 0; y < cur_hgt; y++)
		{
			c_ptr = &cave[y][x];

			if (c_ptr->mimic != 61) continue;
			if (!(c_ptr->info & CAVE_FREE)) continue;

			c_ptr->mimic = 0;
			lite_spot(y, x);
		}

	return (FALSE);
}
bool quest_thrain_turn_hook(char *fmt)
{
	cquest.data[1] = 0;
	cquest.data[2] = 0;
	return (FALSE);
}
bool quest_thrain_init_hook(int q)
{
	if (!cquest.data[0])
	{
		cquest.data[0] = rand_range(d_info[DUNGEON_DOL_GULDUR].mindepth + 1, d_info[DUNGEON_DOL_GULDUR].maxdepth - 1);
		if (wizard) message_add(MESSAGE_MSG, format("Thrain lvl %d", cquest.data[0]), TERM_BLUE);
	}
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_MOVE, quest_thrain_move_hook, "thrain_move");
	}
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_LEVEL_REGEN, quest_thrain_turn_hook, "thrain_regen_lvl");
		add_hook(HOOK_NEW_LEVEL, quest_thrain_turn_hook, "thrain_new_lvl");
		add_hook(HOOK_BUILD_ROOM1, quest_thrain_gen_hook, "thrain_gen");
		add_hook(HOOK_FEELING, quest_thrain_feeling_hook, "thrain_feel");
		add_hook(HOOK_MONSTER_DEATH, quest_thrain_death_hook, "thrain_death");
	}
	return (FALSE);
}
