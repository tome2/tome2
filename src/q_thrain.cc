#include "q_thrain.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_info_type.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "hook_build_room1_in.hpp"
#include "hook_move_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "lua_bind.hpp"
#include "object_type.hpp"
#include "randart.hpp"
#include "messages.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <cassert>
#include <fmt/format.h>

#define cquest (quest[QUEST_THRAIN])

GENERATE_MONSTER_LOOKUP_FN(get_thrain, "Thrain, the King Under the Mountain")
GENERATE_MONSTER_LOOKUP_FN(get_dwar, "Dwar, Dog Lord of Waw")
GENERATE_MONSTER_LOOKUP_FN(get_hoarmurath, "Hoarmurath of Dir")

static bool quest_thrain_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	int r, x, y;
	monster_type *m_ptr;

	if ((cquest.status >= QUEST_STATUS_FINISHED) || (dun_level !=cquest.data[0]) || (dungeon_type != DUNGEON_DOL_GULDUR))
	{
		return false;
	}

	m_ptr = &m_list[m_idx];
	if ((m_ptr->r_idx != get_dwar()) && (m_ptr->r_idx != get_hoarmurath()))
	{
		return false;
	}

	cquest.data[2]++;

	if (cquest.data[2] < 2)
	{
		return false;
	}

	cmsg_print(TERM_YELLOW, "The magic hiding the room dissipates.");
	for (x = 0; x < cur_wid; x++)
	{
		for (y = 0; y < cur_hgt; y++)
		{
			cave_type *c_ptr = &cave[y][x];

			if (c_ptr->mimic != 61)
			{
				continue;
			}

			if (!(c_ptr->info & CAVE_FREE))
			{
				continue;
			}

			c_ptr->mimic = 0;
			lite_spot(y, x);
		}
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
		if (!m_ptr->r_idx)
		{
			continue;
		}

		/* Is it the princess? */
		if (m_ptr->r_idx == get_thrain())
		{
			int x = m_ptr->fx;
			int y = m_ptr->fy;
			int i, j;

			delete_monster_idx(r);

			/* Wipe the glass walls and create a stair */
			for (i = x - 1; i <= x + 1; i++)
			{
				for (j = y - 1; j <= y + 1; j++)
				{
					if (in_bounds(j, i)) cave_set_feat(j, i, FEAT_FLOOR);
				}
			}

			/* Get local object */
			object_type forge, *q_ptr;
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);
			object_prep(q_ptr, lookup_kind(TV_HELM, SV_DRAGON_HELM));
			q_ptr->number = 1;
			q_ptr->found = OBJ_FOUND_REWARD;
			create_artifact(q_ptr, false, true);
			q_ptr->artifact_name = "of Thrain";

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
			break;
		}
	}


	del_hook_new(HOOK_MONSTER_DEATH, quest_thrain_death_hook);
	process_hooks_restart = true;

	return false;
}

static bool quest_thrain_gen_hook(void *, void *in_, void *)
{
	struct hook_build_room1_in *in = static_cast<struct hook_build_room1_in *>(in_);
	s32b bx0 = in->x;
	s32b by0 = in->y;
	s32b x, y;
	int xstart;
	int ystart;
	int y2, x2, yval, xval;
	int y1, x1, xsize, ysize;

	if (dungeon_type != DUNGEON_DOL_GULDUR) return false;
	if (cquest.data[0] != dun_level) return false;
	if (cquest.data[1]) return false;
	if ((cquest.status < QUEST_STATUS_TAKEN) || (cquest.status >= QUEST_STATUS_FINISHED)) return false;

	/* Pick a room size */
	get_map_size("thrain.map", &ysize, &xsize);

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 2, ysize + 2, false, by0, bx0, &xval, &yval)) return false;

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
	reset_get_monster_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

	xstart = x1;
	ystart = y1;
	init_flags = INIT_CREATE_DUNGEON;
	process_dungeon_file("thrain.map", &ystart, &xstart, cur_hgt, cur_wid, true, true);

	for (x = x1; x < xstart; x++)
		for (y = y1; y < ystart; y++)
		{
			cave[y][x].info |= CAVE_ICKY | CAVE_ROOM | CAVE_FREE;
			if (cave[y][x].feat == FEAT_MARKER)
			{
				m_allow_special[get_thrain()] = true;
				int i = place_monster_one(y, x, get_thrain(), 0, false, MSTATUS_NEUTRAL);
				m_allow_special[get_thrain()] = false;

				if (i) m_list[i].mflag |= MFLAG_QUEST;
			}
		}

	/* Don't try another one for this generation */
	cquest.data[1] = 1;

	return true;
}

static bool quest_thrain_feeling_hook(void *, void *, void *)
{
	if (dungeon_type != DUNGEON_DOL_GULDUR) return false;
	if (cquest.data[0] != dun_level) return false;
	if (cquest.status != QUEST_STATUS_UNTAKEN) return false;

	cmsg_format(TERM_YELLOW, "You hear someone shouting under the torture.");
	cquest.status = QUEST_STATUS_TAKEN;
	cquest.init();

	return false;
}

static bool quest_thrain_move_hook(void *, void *in_, void *)
{
	struct hook_move_in *in = static_cast<struct hook_move_in *>(in_);
	s32b y = in->y;
	s32b x = in->x;
	cave_type *c_ptr = &cave[y][x];

	if (dungeon_type != DUNGEON_DOL_GULDUR) return false;
	if (cquest.data[0] != dun_level) return false;
	if ((cquest.status < QUEST_STATUS_TAKEN) || (cquest.status >= QUEST_STATUS_FINISHED)) return false;
	if (!(c_ptr->info & CAVE_FREE)) return false;
	if (c_ptr->mimic != 61) return false;

	cmsg_print(TERM_YELLOW, "The magic hiding the room dissipates.");
	for (x = 0; x < cur_wid; x++)
	{
		for (y = 0; y < cur_hgt; y++)
		{
			c_ptr = &cave[y][x];

			if (c_ptr->mimic != 61) continue;
			if (!(c_ptr->info & CAVE_FREE)) continue;

			c_ptr->mimic = 0;
			lite_spot(y, x);
		}
	}

	return false;
}

static bool quest_thrain_turn_hook(void *, void *, void *)
{
	cquest.data[1] = 0;
	cquest.data[2] = 0;
	return false;
}

void quest_thrain_init_hook()
{
	auto const &d_info = game->edit_data.d_info;
	auto &messages = game->messages;

	if (!cquest.data[0])
	{
		cquest.data[0] = rand_range(d_info[DUNGEON_DOL_GULDUR].mindepth + 1, d_info[DUNGEON_DOL_GULDUR].maxdepth - 1);
		if (wizard)
		{
			messages.add(fmt::format("Thrain lvl {}", cquest.data[0]), TERM_BLUE);
		}
	}
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MOVE, quest_thrain_move_hook, "thrain_move", NULL);
	}
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_LEVEL_REGEN,   quest_thrain_turn_hook,    "thrain_regen_lvl", NULL);
		add_hook_new(HOOK_NEW_LEVEL,     quest_thrain_turn_hook,    "thrain_new_lvl",   NULL);
		add_hook_new(HOOK_BUILD_ROOM1,   quest_thrain_gen_hook,     "thrain_gen",       NULL);
		add_hook_new(HOOK_FEELING,       quest_thrain_feeling_hook, "thrain_feel",      NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_thrain_death_hook,   "thrain_death",     NULL);
	}
}
