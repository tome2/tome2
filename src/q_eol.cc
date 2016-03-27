#include "q_eol.hpp"

#include "cave.hpp"
#include "cave_type.hpp"
#include "generate.hpp"
#include "hook_stair_in.hpp"
#include "hook_quest_finish_in.hpp"
#include "hook_quest_fail_in.hpp"
#include "hook_monster_death_in.hpp"
#include "hooks.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

#include <cassert>

#define cquest (quest[QUEST_EOL])

GENERATE_MONSTER_LOOKUP_FN(get_eol, "Eol, the Dark Elf")

static bool_ quest_eol_gen_hook(void *, void *, void *)
{
	int x, y;
	bool_ done = FALSE;
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

	cave_set_feat(p_ptr->py, p_ptr->px, FEAT_LESS);

	return TRUE;
}

static bool_ quest_eol_finish_hook(void *, void *in_, void *)
{
	struct hook_quest_finish_in *in = static_cast<struct hook_quest_finish_in *>(in_);
	s32b q_idx = in->q_idx;
	object_type forge, *q_ptr;

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

	del_hook_new(HOOK_QUEST_FINISH, quest_eol_finish_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

static bool_ quest_eol_fail_hook(void *, void *in_, void *)
{
	struct hook_quest_fail_in *in = static_cast<struct hook_quest_fail_in *>(in_);
	s32b q_idx = in->q_idx;

	if (q_idx != QUEST_EOL) return FALSE;

	c_put_str(TERM_YELLOW, "You fled ! I did not think you would flee...", 8, 0);

	/* Continue the plot */
	*(quest[q_idx].plot) = QUEST_NULL;

	del_hook_new(HOOK_QUEST_FAIL, quest_eol_fail_hook);
	process_hooks_restart = TRUE;

	return TRUE;
}

static bool_ quest_eol_death_hook(void *, void *in_, void *)
{
	struct hook_monster_death_in *in = static_cast<struct hook_monster_death_in *>(in_);
	s32b m_idx = in->m_idx;
	s32b r_idx = m_list[m_idx].r_idx;

	if (p_ptr->inside_quest != QUEST_EOL) return FALSE;

	if (r_idx == get_eol())
	{
		cmsg_print(TERM_YELLOW, "Such a sad end...");
		cquest.status = QUEST_STATUS_COMPLETED;

		del_hook_new(HOOK_MONSTER_DEATH, quest_eol_death_hook);
		process_hooks_restart = TRUE;

		return (FALSE);
	}

	return FALSE;
}

static bool_ quest_eol_stair_hook(void *, void *in_, void *)
{
	struct hook_stair_in *in = static_cast<struct hook_stair_in *>(in_);
	monster_race *r_ptr = &r_info[get_eol()];

	if (p_ptr->inside_quest != QUEST_EOL) return FALSE;

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS) return TRUE;

	if (r_ptr->max_num)
	{
		if (in->direction == STAIRS_UP)
		{
			/* Flush input */
			flush();

			if (!get_check("Really abandon the quest?")) return TRUE;

			cmsg_print(TERM_YELLOW, "You flee away from Eol...");
			cquest.status = QUEST_STATUS_FAILED;

			del_hook_new(HOOK_STAIR, quest_eol_stair_hook);
			process_hooks_restart = TRUE;
			return (FALSE);
		}
	}

	return FALSE;
}

bool_ quest_eol_init_hook(int q)
{
	if ((cquest.status >= QUEST_STATUS_TAKEN) && (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_MONSTER_DEATH, quest_eol_death_hook,  "eol_death",  NULL);
		add_hook_new(HOOK_GEN_QUEST,     quest_eol_gen_hook,    "eol_gen",    NULL);
		add_hook_new(HOOK_STAIR,         quest_eol_stair_hook,  "eol_stair",  NULL);
		add_hook_new(HOOK_QUEST_FAIL,    quest_eol_fail_hook,   "eol_fail",   NULL);
		add_hook_new(HOOK_QUEST_FINISH,  quest_eol_finish_hook, "eol_finish", NULL);
	}
	return (FALSE);
}
