#include "q_rand.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "dungeon_info_type.hpp"
#include "game.hpp"
#include "generate.hpp"
#include "hook_build_room1_in.hpp"
#include "hook_chardump_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "lua_bind.hpp"
#include "messages.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

#include <cassert>
#include <fmt/format.h>

static int randquest_hero[] = { 20, 13, 15, 16, 9, 17, 18, 8, -1 };

/* Possible number(and layout) or random quests */
#define MAX_RANDOM_QUESTS_TYPES ((8 * 3) + (8 * 1))
static int random_quests_types[MAX_RANDOM_QUESTS_TYPES] =
{
	1, 5, 6, 7, 10, 11, 12, 14,          /* Princess type */
	1, 5, 6, 7, 10, 11, 12, 14,          /* Princess type */
	1, 5, 6, 7, 10, 11, 12, 14,          /* Princess type */
	20, 13, 15, 16, 9, 17, 18, 8,        /* Hero Sword Quest */
};

/* Enforce OoD monsters until this level */
#define RQ_LEVEL_CAP 49

// Generate lookup function
GENERATE_MONSTER_LOOKUP_FN(get_adventurer, "Adventurer")

void initialize_random_quests(int n)
{
	auto const &d_info = game->edit_data.d_info;
	auto &r_info = game->edit_data.r_info;

	int step, lvl, i, k;
	int old_type = dungeon_type;

	/* Zero out everything first */
	for (i = 0; i < MAX_RANDOM_QUEST; i++) random_quests[i].type = 0;

	if (n == 0) return;

	/* Factor dlev value by 1000 to keep precision */
	step = (98 * 1000) / n;

	lvl = step / 2;

	quest[QUEST_RANDOM].status = QUEST_STATUS_TAKEN;

	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = nullptr;

		int rl = (lvl / 1000) + 1;

		int min_level;

		int tries = 5000;

		random_quest *q_ptr = &random_quests[rl];

		/* Find the appropriate dungeon */
		for (std::size_t j = 0; j < d_info.size(); j++)
		{
			auto d_ptr = &d_info[j];

			if (!(d_ptr->flags & DF_PRINCIPAL)) continue;

			if ((d_ptr->mindepth <= rl) && (rl <= d_ptr->maxdepth))
			{
				dungeon_type = j;
				break;
			}
		}

		q_ptr->type = random_quests_types[rand_int(MAX_RANDOM_QUESTS_TYPES)];

		/* XXX XXX XXX Try until valid choice is found */
		while (tries)
		{
			bool_ ok;

			tries--;

			/* Random monster 5 - 10 levels out of depth */
			q_ptr->r_idx = get_mon_num(rl + 4 + randint(6));

			if (!q_ptr->r_idx) continue;

			r_ptr = &r_info[q_ptr->r_idx];

			/* Accept only monsters that can be generated */
			if (r_ptr->flags & RF_SPECIAL_GENE) continue;
			if (r_ptr->flags & RF_NEVER_GENE) continue;

			/* Accept only monsters that are not breeders */
			if (r_ptr->spells & SF_MULTIPLY) continue;

			/* Forbid joke monsters */
			if (r_ptr->flags & RF_JOKEANGBAND) continue;

			/* Accept only monsters that are not friends */
			if (r_ptr->flags & RF_PET) continue;

			/* Refuse nazguls */
			if (r_ptr->flags & RF_NAZGUL) continue;

			/* Accept only monsters that are not good */
			if (r_ptr->flags & RF_GOOD) continue;

			/* If module says a monster race is friendly, then skip */
			if (modules[game_module_idx].race_status != NULL)
			{
				s16b *status = (*modules[game_module_idx].race_status)(q_ptr->r_idx);
				if ((status != NULL) && (*status >= 0))
				{
					continue;
				}
			}

			/* Assume no explosion attacks */
			ok = TRUE;

			/* Reject monsters with exploding attacks */
			for (k = 0; k < 4; k++)
			{
				if (r_ptr->blow[k].method == RBM_EXPLODE) ok = FALSE;
			}
			if (!ok) continue;

			/* No mutliple uniques */
			if ((r_ptr->flags & RF_UNIQUE) &&
			                ((q_ptr->type != 1) || (r_ptr->max_num == -1))) continue;

			/* No single non uniques */
			if ((!(r_ptr->flags & RF_UNIQUE)) && (q_ptr->type == 1)) continue;

			/* Level restriction */
			min_level = (rl > RQ_LEVEL_CAP) ? RQ_LEVEL_CAP : rl;

			/* Accept monsters matching the level restriction */
			if (r_ptr->level > min_level) break;
		}

		/* Arg could not find anything ??? */
		if (!tries)
		{
			if (wizard)
			{
				message_add(format("Could not find quest monster on lvl %d", rl), TERM_RED);
			}
			q_ptr->type = 0;
		}
		else
		{
			if (r_ptr->flags & RF_UNIQUE)
			{
				r_ptr->max_num = -1;
			}

			q_ptr->done = false;

			if (wizard)
			{
				message_add(format("Quest for %d on lvl %d",
						   q_ptr->r_idx, rl), TERM_RED);
			}
		}

		lvl += step;
	}

	dungeon_type = old_type;
}

bool_ is_randhero(int level)
{
	int i;
	bool_ result = FALSE;

	for (i = 0; randquest_hero[i] != -1; i++)
	{
		if (random_quests[level].type == randquest_hero[i])
		{
			result = TRUE;
			break;
		}
	}

	return result;
}

static void do_get_new_obj(int y, int x)
{
	auto &k_info = game->edit_data.k_info;
	auto &a_info = game->edit_data.a_info;

	object_type *q_ptr[3], forge[3];
	int res, i;

	/* Create objects */
	std::vector<std::string> items;
	for (i = 0; i < 3; i++)
	{
		/* Get local object */
		q_ptr[i] = &forge[i];

		/* Wipe the object */
		object_wipe(q_ptr[i]);

		/* Make a great object */
		make_object(q_ptr[i], TRUE, TRUE, obj_theme::no_theme());
		q_ptr[i]->found = OBJ_FOUND_REWARD;

		char buf[100];
		object_desc(buf, q_ptr[i], 0, 0);
		items.push_back(buf);
	}


	while (TRUE)
	{
		res = ask_menu("Choose a reward to get(a-c to choose, ESC to cancel)?", items);

		/* Ok ? lets learn ! */
		if (res > -1)
		{
			/* Drop it in the dungeon */
			drop_near(q_ptr[res], -1, y + 1, x);

			cmsg_print(TERM_YELLOW, "There, Noble Hero. I put it there. Thanks again!");
			break;
		}
	}

	for (i = 0; i < 3; i++)
	{

		object_type *o_ptr = q_ptr[i];

		/* Check if there is any not chosen artifact */
		if (i != res && artifact_p(o_ptr))
		{
			/* Mega-Hack -- Preserve the artifact */
			if (o_ptr->tval == TV_RANDART)
			{
				random_artifacts[o_ptr->sval].generated = FALSE;
			}
			else if (k_info[o_ptr->k_idx].flags & TR_NORM_ART)
			{
				k_info[o_ptr->k_idx].artifact = FALSE;
			}
			else if (o_ptr->name1)
			{
				a_info[o_ptr->name1].cur_num = 0;
			}
		}
	}
}

static void princess_death(s32b m_idx, s32b r_idx)
{
	int r;

	cmsg_print(TERM_YELLOW, "O Great And Noble Hero, you saved me!");
	cmsg_print(TERM_YELLOW, "I am heading home now. I cannot reward you as I should, but please take this.");

	/* Look for the princess */
	for (r = m_max - 1; r >= 1; r--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[r];

		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;

		/* Is it the princess? */
		if (m_ptr->r_idx == 969)
		{
			int x = m_ptr->fx;
			int y = m_ptr->fy;
			int i, j;

			delete_monster_idx(r);

			/* Wipe the glass walls and create a stair */
			for (i = x - 1; i <= x + 1; i++)
				for (j = y - 1; j <= y + 1; j++)
				{
					if (in_bounds(j, i)) cave_set_feat(j, i, FEAT_FLOOR);
				}
			cave_set_feat(y, x, FEAT_MORE);

			do_get_new_obj(y, x);

			random_quests[dun_level].done = true;

			break;
		}
	}
}

static void hero_death(s32b m_idx, s32b r_idx)
{
	random_quests[dun_level].done = true;

	cmsg_print(TERM_YELLOW, "The adventurer steps out of the shadows and picks up his sword:");
	cmsg_print(TERM_YELLOW, "'Ah! My sword! My trusty sword! Thanks.");

	if (!can_create_companion())
	{
		cmsg_print(TERM_YELLOW, "I must go on my own way now.");
		cmsg_print(TERM_YELLOW, "But before I go, I can help your skills.'");
		cmsg_print(TERM_YELLOW, "He touches your forehead.");
		do_get_new_skill();
		return;
	}
	cmsg_print(TERM_YELLOW, "If you wish, I can help you in your adventures.'");

	/* Flush input */
	flush();

	if (get_check("Do you want him to join you? "))
	{
		int x, y, i;

		/* Look for a location */
		for (i = 0; i < 20; ++i)
		{
			/* Pick a distance */
			int d = (i / 15) + 1;

			/* Pick a location */
			scatter(&y, &x, p_ptr->py, p_ptr->px, d);

			/* Require "empty" floor grid */
			if (!cave_empty_bold(y, x)) continue;

			/* Hack -- no summon on glyph of warding */
			if (cave[y][x].feat == FEAT_GLYPH) continue;
			if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

			/* Nor on the between */
			if (cave[y][x].feat == FEAT_BETWEEN) continue;

			/* ... nor on the Pattern */
			if ((cave[y][x].feat >= FEAT_PATTERN_START) &&
			                (cave[y][x].feat <= FEAT_PATTERN_XTRA2))
				continue;

			/* Okay */
			break;
		}

		if (i < 20)
		{
			int r_idx = get_adventurer();

			m_allow_special[r_idx] = TRUE;
			int m_idx = place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_COMPANION);
			m_allow_special[r_idx] = FALSE;

			if (m_idx)
			{
				m_list[m_idx].exp = monster_exp(1 + (dun_level * 3 / 2));
				m_list[m_idx].status = MSTATUS_COMPANION;
				monster_check_experience(m_idx, TRUE);
			}
		}
		else
			msg_print("The adventurer suddenly seems afraid and flees...");
	}
	else
	{
		cmsg_print(TERM_YELLOW, "'As you wish, but I want to do something for you.'");
		cmsg_print(TERM_YELLOW, "He touches your forehead.");
		do_get_new_skill();
	}
}

static bool quest_random_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	int r_idx = m_list[m_idx].r_idx;

	if ((!(dungeon_flags & DF_PRINCIPAL)) ||
	    ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) ||
	    (!random_quests[dun_level].type) ||
	    (random_quests[dun_level].done) ||
	    (p_ptr->inside_quest) ||
	    (random_quests[dun_level].r_idx != r_idx) ||
	    (!(m_list[m_idx].mflag & MFLAG_QUEST)))
	{
		return false;
	}

	/* Killed enough ?*/
	quest[QUEST_RANDOM].data[0]++;
	if (quest[QUEST_RANDOM].data[0] == random_quests[dun_level].type)
	{
		if (is_randhero(dun_level))
		{
			hero_death(m_idx, r_idx);
		}
		else
		{
			princess_death(m_idx, r_idx);
		}
	}

	return false;
}

static bool quest_random_turn_hook(void *, void *, void *)
{
	quest[QUEST_RANDOM].data[0] = 0;
	quest[QUEST_RANDOM].data[1] = 0;
	return false;
}

static bool quest_random_feeling_hook(void *, void *, void *)
{
	auto const &r_info = game->edit_data.r_info;

	if ((!(dungeon_flags & DF_PRINCIPAL)) ||
	    ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) ||
	    (!random_quests[dun_level].type) ||
	    (random_quests[dun_level].done) ||
	    (p_ptr->inside_quest) ||
	    (!dun_level))
	{
		return false;
	}

	if (is_randhero(dun_level))
	{
		cmsg_format(TERM_YELLOW, "A strange man wrapped in a dark cloak steps out of the shadows:");
		cmsg_format(TERM_YELLOW, "'Oh, please help me! A horrible %s stole my sword! I'm nothing without it.'", r_info[random_quests[dun_level].r_idx].name);
	}
	else
	{
		cmsg_format(TERM_YELLOW, "You hear someone shouting: 'Leave me alone, stupid %s'", r_info[random_quests[dun_level].r_idx].name);
	}
	return false;
}

static bool quest_random_gen_hero_hook(void *, void *, void *)
{
	if ((!(dungeon_flags & DF_PRINCIPAL)) ||
	    ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) ||
	    (!random_quests[dun_level].type) ||
	    (random_quests[dun_level].done) ||
	    (p_ptr->inside_quest) ||
	    (!is_randhero(dun_level)))
	{
		return false;
	}

	int i = random_quests[dun_level].type;

	m_allow_special[random_quests[dun_level].r_idx] = TRUE;
	while (i)
	{
		int m_idx, y = rand_range(1, cur_hgt - 2), x = rand_range(1, cur_wid - 2);

		m_idx = place_monster_one(y, x, random_quests[dun_level].r_idx, 0, FALSE, MSTATUS_ENEMY);
		if (m_idx)
		{
			monster_type *m_ptr = &m_list[m_idx];
			m_ptr->mflag |= MFLAG_QUEST;
			i--;
		}
	}
	m_allow_special[random_quests[dun_level].r_idx] = FALSE;

	return false;
}

static bool quest_random_gen_hook(void *, void *in_, void *)
{
	struct hook_build_room1_in *in = static_cast<struct hook_build_room1_in *>(in_);
	s32b bx0 = in->x;
	s32b by0 = in->y;
	s32b x, y;
	int xstart;
	int ystart;
	int y2, x2, yval, xval;
	int y1, x1, xsize, ysize;

	if ((!(dungeon_flags & DF_PRINCIPAL)) ||
	    ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) ||
	    (!random_quests[dun_level].type)  ||
	    (random_quests[dun_level].done)  ||
	    (p_ptr->inside_quest)  ||
	    (quest[QUEST_RANDOM].data[1]) ||
	    (is_randhero(dun_level)))
	{
		return false;
	}

	/* Pick a room size */
	get_map_size(format("qrand%d.map", random_quests[dun_level].type), &ysize, &xsize);

	/* Try to allocate space for room.  If fails, exit */
	if (!room_alloc(xsize + 2, ysize + 2, FALSE, by0, bx0, &xval, &yval))
	{
		return false;
	}

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
	process_dungeon_file(format("qrand%d.map", random_quests[dun_level].type), &ystart, &xstart, cur_hgt, cur_wid, TRUE, TRUE);

	for (x = x1; x < xstart; x++)
	{
		for (y = y1; y < ystart; y++)
		{
			cave[y][x].info |= CAVE_ICKY | CAVE_ROOM;
			if (cave[y][x].feat == FEAT_MARKER)
			{
				m_allow_special[random_quests[dun_level].r_idx] = TRUE;
				int i = place_monster_one(y, x, random_quests[dun_level].r_idx, 0, FALSE, MSTATUS_ENEMY);
				m_allow_special[random_quests[dun_level].r_idx] = FALSE;
				if (i)
				{
					auto m_ptr = &m_list[i];
					m_ptr->mflag |= MFLAG_QUEST;
				}
			}
		}
	}

	/* Dont try another one for this generation */
	quest[QUEST_RANDOM].data[1] = 1;

	/* Boost level feeling a bit - a la pits */
	rating += 10;

	return true;
}

static bool quest_random_dump_hook(void *, void *in_, void *)
{
	static const char *number[] = { "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten" };
	struct hook_chardump_in *in = static_cast<struct hook_chardump_in *>(in_);
	FILE *f = in->file;
	int i, valid = 0, lscnt = 0, pcnt = 0;

	for (i = 0; i < MAX_RANDOM_QUEST; i++)
	{
		if (random_quests[i].type)
		{
			valid++;
			if (random_quests[i].done)
			{
				if (is_randhero(i))
					lscnt++;
				else
					pcnt++;
			}
		}
	}

	if (valid)
	{
		if (pcnt > 10)
			fprintf(f, "\n You have completed %d princess quests.", pcnt);
		else if (pcnt > 1)
			fprintf(f, "\n You have completed %s princess quests.", number[pcnt-2]);
		else if (pcnt == 1)
			fprintf(f, "\n You have completed one princess quest.");
		else
			fprintf(f, "\n You haven't completed a single princess quest.");

		if (lscnt > 10)
			fprintf(f, "\n You have completed %d lost sword quests.", lscnt);
		else if (lscnt > 1)
			fprintf(f, "\n You have completed %s lost sword quests.", number[lscnt-2]);
		else if (lscnt == 1)
			fprintf(f, "\n You have completed one lost sword quest.");
		else
			fprintf(f, "\n You haven't completed a single lost sword quest.");
	}

	return false;
}

std::string quest_random_describe()
{
	auto const &r_info = game->edit_data.r_info;

	// Only emit description if we're actually on a
	// random quest level.
	if ((!(dungeon_flags & DF_PRINCIPAL)) ||
	    ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) ||
	    (!random_quests[dun_level].type) ||
	    (random_quests[dun_level].done) ||
	    (p_ptr->inside_quest) ||
	    (!dun_level))
	{
		return "";
	}

	fmt::MemoryWriter w;

	if (!is_randhero(dun_level))
	{
		w.write("#####yCaptured princess!\n");
		w.write("A princess is being held prisoner and tortured here!\n");
		w.write("Save her from the horrible {}.\n", r_info[random_quests[dun_level].r_idx].name);
	}
	else
	{
		w.write("#####yLost sword!\n");
		w.write("An adventurer lost his sword to a bunch of {}!\n", r_info[random_quests[dun_level].r_idx].name);
		w.write("Kill them all to get it back.\n");
	}

	w.write("Number: {}, Killed: {}.",
		random_quests[dun_level].type,
		quest[QUEST_RANDOM].data[0]);

	return w.str();
}

void quest_random_init_hook()
{
	add_hook_new(HOOK_MONSTER_DEATH, quest_random_death_hook,    "rand_death",     NULL);
	add_hook_new(HOOK_NEW_LEVEL,     quest_random_turn_hook,     "rand_new_lvl",   NULL);
	add_hook_new(HOOK_LEVEL_REGEN,   quest_random_turn_hook,     "rand_regen_lvl", NULL);
	add_hook_new(HOOK_LEVEL_END_GEN, quest_random_gen_hero_hook, "rand_gen_hero",  NULL);
	add_hook_new(HOOK_BUILD_ROOM1,   quest_random_gen_hook,      "rand_gen",       NULL);
	add_hook_new(HOOK_FEELING,       quest_random_feeling_hook,  "rand_feel",      NULL);
	add_hook_new(HOOK_CHAR_DUMP,     quest_random_dump_hook,     "rand_dump",      NULL);
}
