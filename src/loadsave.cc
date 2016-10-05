#include "loadsave.hpp"
#include "loadsave.h"

#include "artifact_type.hpp"
#include "birth.hpp"
#include "cave_type.hpp"
#include "dungeon_info_type.hpp"
#include "ego_item_type.hpp"
#include "game.hpp"
#include "init1.hpp"
#include "init2.hpp"
#include "levels.hpp"
#include "messages.hpp"
#include "modules.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_class.hpp"
#include "player_level_flag.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "hooks.hpp"
#include "skill_type.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "timer_type.hpp"
#include "town_type.hpp"
#include "trap_type.hpp"
#include "util.hpp"
#include "util.h"
#include "wilderness_map.hpp"
#include "variable.h"
#include "variable.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

#include <cassert>
#include <memory>

static u32b vernum; /* Version flag */
static FILE *fff; 	/* Local savefile ptr */

/*
 * Savefile version
 */
static byte sf_major;
static byte sf_minor;
static byte sf_patch;

/*
 * Savefile information
 */
static u32b sf_when;                   /* Time when savefile created */
static u16b sf_lives;                  /* Number of past "lives" with this file */
static u16b sf_saves;                  /* Number of "saves" during this life */

/**
 * Load/save flag
 */
enum class ls_flag_t {
	LOAD = 3,
	SAVE = 7
};

/**
 * Structure for loading/saving option values
 */
namespace {

struct option_value {
	std::string name;
	bool_ value;
};

} // namespace (anonymous)


/*
 * Basic byte-level reading from savefile. This provides a single point
 * of interface to the pseudoencryption that ToME (and Angband)
 * uses. I'm thinking about if it might be faster/better to modify all
 * the do_* functions to directly do this stuff -- it'd make the code
 * somewhat uglier to maintain, but concievably might be much faster. Or
 * is it better maybe to scrap the pseudoencryption entirely and adopt
 * some other means of obfuscation, should it still prove useful in any
 * way? -- Improv
 *
 * What's the point of encryption on savefiles anyway? If I wanted to
 * make a cheater savefile, I'd activate debug mode, and hack the game
 * not to save it. There's no point. -- takkaria
 */

static byte sf_get(void)
{
	byte c;

	/* Get a character, decode the value */
	c = getc(fff) & 0xFF;

	/* Return the value */
	return (c);
}


static void sf_put(byte v)
{
	(void)putc((int)v, fff);
}

/*
 * Size-aware read/write routines for the savefile, do all their
 * work through sf_get and sf_put.
 */
static void do_byte(byte *v, ls_flag_t flag)
{
	switch (flag)
	{
	case ls_flag_t::LOAD:
	{
		*v = sf_get();
		return;
	}
	case ls_flag_t::SAVE:
	{
		byte val = *v;
		sf_put(val);
		return;
	}
	}
}

static void do_char(char *c, ls_flag_t flag)
{
	do_byte((byte *) c, flag);
}

static void do_bool(bool_ *f, ls_flag_t flag)
{
	byte b = *f;
	do_byte(&b, flag);
	if (flag == ls_flag_t::LOAD)
	{
		*f = b;
	}
}

static void do_std_bool(bool *x, ls_flag_t flag)
{
	switch (flag)
	{
	case ls_flag_t::LOAD:
	{
		*x = (sf_get() != 0);
		return;
	}
	case ls_flag_t::SAVE:
	{
		byte val = (*x) ? 1 : 0;
		sf_put(val);
		return;
	}
	}
}

static void do_u16b(u16b *v, ls_flag_t flag)
{
	switch (flag)
	{
	case ls_flag_t::LOAD:
	{
		(*v) = sf_get();
		(*v) |= ((u16b)(sf_get()) << 8);
		return;
	}
	case ls_flag_t::SAVE:
	{
		u16b val;
		val = *v;
		sf_put((byte)(val & 0xFF));
		sf_put((byte)((val >> 8) & 0xFF));
		return;
	}
	}
}

static void do_s16b(s16b *ip, ls_flag_t flag)
{
	do_u16b((u16b *)ip, flag);
}

static void do_u32b(u32b *ip, ls_flag_t flag)
{
	switch(flag)
	{
	case ls_flag_t::LOAD:
	{
		(*ip) = sf_get();
		(*ip) |= ((u32b)(sf_get()) << 8);
		(*ip) |= ((u32b)(sf_get()) << 16);
		(*ip) |= ((u32b)(sf_get()) << 24);
		return;
	}
	case ls_flag_t::SAVE:
	{
		u32b val = *ip;
		sf_put((byte)(val & 0xFF));
		sf_put((byte)((val >> 8) & 0xFF));
		sf_put((byte)((val >> 16) & 0xFF));
		sf_put((byte)((val >> 24) & 0xFF));
		return;
	}
	}
}

static void do_s32b(s32b *ip, ls_flag_t flag)
{
	do_u32b((u32b *)ip, flag);
}

static void save_string(const char *str)
{
	while (*str)
	{
		do_byte((byte*)str, ls_flag_t::SAVE);
		str++;
	}
	do_byte((byte*)str, ls_flag_t::SAVE);
}

static void load_string(char *str, int max)
{
	int i;

	/* Read the string */
	for (i = 0; TRUE; i++)
	{
		byte tmp8u;

		/* Read a byte */
		do_byte(&tmp8u, ls_flag_t::LOAD);

		/* Collect string while legal */
		if (i < max) str[i] = tmp8u;

		/* End of string */
		if (!tmp8u) break;
	}
	/* Terminate */
	str[max - 1] = '\0';
}

static void do_string(char *str, int max, ls_flag_t flag)
/* Max is ignored for writing */
{
	switch(flag) {
	case ls_flag_t::LOAD:
	{
		load_string(str, max);
		return;
	}
	case ls_flag_t::SAVE:
	{
		save_string(str);
		return;
	}
	}
}

static void save_std_string(std::string const *s)
{
	// Length prefix.
	u32b saved_size = s->size();
	do_u32b(&saved_size, ls_flag_t::SAVE);
	// Save each character
	for (auto c: *s)
	{
		sf_put(c);
	}
}

static std::string load_std_string()
{
	// Length prefix.
	u32b saved_size;
	do_u32b(&saved_size, ls_flag_t::LOAD);
	// Convert to size_t
	std::size_t n = saved_size;
	// Make sure we reserve space rather than resizing as we go.
	std::string s;
	s.reserve(n);
	// Read each character
	for (std::size_t i = 0; i < n; i++)
	{
		s += sf_get();
	}
	// Done
	return s;
}


static void do_std_string(std::string &s, ls_flag_t flag)
{
	switch (flag)
	{
	case ls_flag_t::LOAD:
		s = load_std_string();
		break;
	case ls_flag_t::SAVE:
		save_std_string(&s);
		break;
	}
}

static void do_option_value(option_value *option_value, ls_flag_t flag)
{
	do_std_string(option_value->name, flag);
	do_bool(&option_value->value, flag);
}


namespace {

/**
 * Load/save flag set
 */
template<std::size_t Tiers> void do_flag_set(flag_set<Tiers> *flags, ls_flag_t flag)
{
	for (std::size_t i = 0; i < flags->size(); i++)
	{
		do_u32b(&(*flags)[i], flag);
	}
}

template<typename T, typename F> void do_vector(ls_flag_t flag, std::vector<T> &v, F f)
{
	u32b n = v.size();

	do_u32b(&n, flag);

	if (flag == ls_flag_t::LOAD)
	{
		v.clear(); // Make sure it's empty
		v.reserve(n);
		std::fill_n(std::back_inserter(v), n, T());
	}

	for (std::size_t i = 0; i < n; i++)
	{
		f(&v[i], flag);
	}
}

static void do_bytes(ls_flag_t flag, std::uint8_t *buf, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
	{
		do_byte(&buf[i], flag);
	}
};

static void do_seed(seed_t *seed, ls_flag_t flag)
{
	uint8_t buf[seed_t::n_bytes];

	if (flag == ls_flag_t::SAVE)
	{
		seed->to_bytes(buf);
	}

	do_bytes(flag, buf, sizeof(buf));

	if (flag == ls_flag_t::LOAD)
	{
		*seed = seed_t::from_bytes(buf);
	}
}

} // namespace (anonymous)


/*
 * Load/Save quick start data
 */
static void do_quick_start(ls_flag_t flag)
{
	do_s16b(&previous_char.race, flag);
	do_s16b(&previous_char.rmod, flag);
	do_s16b(&previous_char.pclass, flag);
	do_s16b(&previous_char.spec, flag);
	do_byte(&previous_char.quests, flag);
	do_byte(&previous_char.god, flag);
	do_s32b(&previous_char.grace, flag);
	do_s32b(&previous_char.au, flag);

	for (std::size_t i = 0; i < 6; i++)
	{
		do_s16b(&(previous_char.stat[i]), flag);
	}
	do_s16b(&previous_char.luck, flag);

	do_bool(&previous_char.quick_ok, flag);

	for (std::size_t i = 0; i < 4; i++)
	{
		do_string(previous_char.history[i], 60, flag);
	}
}

static void do_skill_modifier(skill_modifier *s, ls_flag_t flag)
{
	do_char(&s->basem, flag);
	do_u32b(&s->base, flag);
	do_char(&s->modm, flag);
	do_s16b(&s->mod, flag);
}

static void do_skill_modifiers(skill_modifiers *skill_modifiers, ls_flag_t flag)
{
	do_vector(flag, skill_modifiers->modifiers, do_skill_modifier);
}

static void do_player_level_flag(player_level_flag *lflag, ls_flag_t flag)
{
	do_flag_set(&lflag->oflags, flag);
	do_s16b(&lflag->pval, flag);
}

/*
 * The special saved subrace
 */
static void do_subrace(ls_flag_t flag)
{
	auto &race_mod_info = game->edit_data.race_mod_info;

	player_race_mod *sr_ptr = &race_mod_info[SUBRACE_SAVE];
	int i;

	do_std_string(sr_ptr->title, flag);
	do_std_string(sr_ptr->description, flag);

	do_bool(&sr_ptr->place, flag);

	for (i = 0; i < 6; i++)
	{
		do_s16b(&sr_ptr->ps.adj[i], flag);
	}

	do_char(&sr_ptr->luck, flag);
	do_s16b(&sr_ptr->mana, flag);

	do_s16b(&sr_ptr->ps.mhp, flag);
	do_s16b(&sr_ptr->ps.exp, flag);

	do_char(&sr_ptr->infra, flag);

	do_vector(flag, sr_ptr->ps.powers, do_s16b);

	for (i = 0; i < BODY_MAX; i++)
	{
		do_char(&sr_ptr->body_parts[i], flag);
	}

	do_flag_set(&sr_ptr->flags, flag);

	for (i = 0; i < PY_MAX_LEVEL + 1; i++)
	{
		do_player_level_flag(&sr_ptr->lflags[i], flag);
	}

	do_byte(&sr_ptr->g_attr, flag);
	do_char(&sr_ptr->g_char, flag);

	do_skill_modifiers(&sr_ptr->skill_modifiers, flag);
}


static void do_random_spell(random_spell *s_ptr, ls_flag_t flag)
{
	do_string(s_ptr->name, 30, flag);
	do_string(s_ptr->desc, 30, flag);
	do_s16b(&s_ptr->mana, flag);
	do_s16b(&s_ptr->fail, flag);
	do_u32b(&s_ptr->proj_flags, flag);
	do_byte(&s_ptr->GF, flag);
	do_byte(&s_ptr->radius, flag);
	do_byte(&s_ptr->dam_sides, flag);
	do_byte(&s_ptr->dam_dice, flag);
	do_byte(&s_ptr->level, flag);
	do_std_bool(&s_ptr->untried, flag);
}

static void do_rune_spell(rune_spell *s_ptr, ls_flag_t flag)
{
	do_string(s_ptr->name, 30, flag);
	do_s16b(&s_ptr->type, flag);
	do_s16b(&s_ptr->rune2, flag);
	do_s16b(&s_ptr->mana, flag);
}


/*
 * Show information on the screen, one line at a time.
 *
 * Avoid the top two lines, to avoid interference with "msg_print()".
 */
static void note(cptr msg)
{
	static int y = 2;

	/* Draw the message */
	prt(msg, y, 0);

	/* Advance one line (wrap if needed) */
	if (++y >= 24) y = 2;

	/* Flush it */
	Term_fresh();
}


/*
 * Misc. other data
 */
static char loaded_game_module[80];
static bool_ do_extra(ls_flag_t flag)
{
	auto const &d_info = game->edit_data.d_info;
	auto &s_info = game->s_info;

	do_string(player_name, 32, flag);

	do_string(died_from, 80, flag);

	for (std::size_t i = 0; i < 4; i++)
	{
		do_string(history[i], 60, flag);
	}

	/* Handle the special levels info */
	{
		byte tmp8u = d_info.size();
		u16b tmp16u = MAX_DUNGEON_DEPTH;

		do_byte(&tmp8u, flag);

		if (flag == ls_flag_t::LOAD)
		{
			if (tmp8u > d_info.size())
			{
				note(format("Too many dungeon types!", static_cast<int>(tmp8u)));
			}
		}

		do_u16b(&tmp16u, flag);

		if (flag == ls_flag_t::LOAD)
		{
			if (tmp16u > MAX_DUNGEON_DEPTH)
			{
				note(format("Too many (%d) max level by dungeon type!", static_cast<int>(tmp16u)));
			}
		}

		/* Load the special levels history */
		for (std::size_t i = 0; i < tmp8u; i++)
		{
			for (std::size_t j = 0; j < tmp16u; j++)
			{
				do_bool(&special_lvl[j][i], flag);
			}
		}
	}

	do_bool(&generate_special_feeling, flag);

	/* Load the quick start data */
	do_quick_start(flag);

	/* Load/save the special subrace */
	do_subrace(flag);

	/* Race/Class/Gender/Spells */
	do_s32b(&p_ptr->lives, flag);
	do_byte(&p_ptr->prace, flag);
	do_byte(&p_ptr->pracem, flag);
	do_byte(&p_ptr->pclass, flag);
	do_byte(&p_ptr->pspec, flag);
	do_byte(&p_ptr->mimic_form, flag);
	do_s16b(&p_ptr->mimic_level, flag);

	do_byte(&p_ptr->hitdie, flag);
	do_u16b(&p_ptr->expfact, flag);

	/* Dump the stats (maximum and current) */
	for (std::size_t i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_max[i], flag);
	for (std::size_t i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_cur[i], flag);
	for (std::size_t i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_cnt[i], flag);
	for (std::size_t i = 0; i < 6; ++i) do_s16b(&p_ptr->stat_los[i], flag);

	// Skills
	{
		do_s16b(&p_ptr->skill_points, flag);
		do_s16b(&p_ptr->skill_last_level, flag);
		do_s16b(&p_ptr->melee_style, flag);
		do_s16b(&p_ptr->use_piercing_shots, flag);

		u16b tmp16u = s_info.size();

		do_u16b(&tmp16u, flag);

		if ((flag == ls_flag_t::LOAD) && (tmp16u != s_info.size()))
		{
			quit("Too few/many skills");
		}

		for (std::size_t i = 0; i < tmp16u; i++)
		{
			do_s32b(&s_info[i].value, flag);
			do_s32b(&s_info[i].mod, flag);
			do_std_bool(&s_info[i].dev, flag);
			do_std_bool(&s_info[i].hidden, flag);
		}
	}

	// Abilities
	do_vector(flag, p_ptr->abilities, do_u16b);

	// Miscellaneous
	do_s16b(&p_ptr->luck_base, flag);
	do_s16b(&p_ptr->luck_max, flag);

	do_s32b(&p_ptr->au, flag);

	do_s32b(&p_ptr->max_exp, flag);
	do_s32b(&p_ptr->exp, flag);
	do_u16b(&p_ptr->exp_frac, flag);
	do_s16b(&p_ptr->lev, flag);

	do_s16b(&p_ptr->town_num, flag); 	/* -KMW- */

	/* Write arena and rewards information -KMW- */
	do_s16b(&p_ptr->inside_quest, flag);


	/* Save/load spellbinder */
	do_byte(&p_ptr->spellbinder.trigger, flag);
	do_vector(flag, p_ptr->spellbinder.spell_idxs, do_u32b);

	// Quests
	{
		byte tmp8u;

		// Number of quests
		if (flag == ls_flag_t::SAVE)
		{
			tmp8u = MAX_PLOTS;
		}
		do_byte(&tmp8u, flag);
		if ((flag == ls_flag_t::LOAD) && (tmp8u > MAX_PLOTS))
		{
			quit(format("Too many plots, %d %d", tmp8u, MAX_PLOTS));
		}

		// Quest status
		for (std::size_t i = 0; i < tmp8u; i++)
		{
			do_s16b(&plots[i], flag);
		}

		// Number of random quests
		if (flag == ls_flag_t::SAVE)
		{
			tmp8u = MAX_RANDOM_QUEST;
		}
		do_byte(&tmp8u, flag);
		if ((flag == ls_flag_t::LOAD) && (tmp8u > MAX_RANDOM_QUEST))
		{
			quit("Too many random quests");
		}

		// Random quest data
		for (std::size_t i = 0; i < tmp8u; i++)
		{
			do_byte(&random_quests[i].type, flag);
			do_s16b(&random_quests[i].r_idx, flag);
			do_std_bool(&random_quests[i].done, flag);
		}
	}

	do_s16b(&p_ptr->oldpx, flag);
	do_s16b(&p_ptr->oldpy, flag);

	do_s16b(&p_ptr->mhp, flag);
	do_s16b(&p_ptr->chp, flag);
	do_u16b(&p_ptr->chp_frac, flag);
	do_s16b(&p_ptr->hp_mod, flag);

	do_s16b(&p_ptr->msane, flag);
	do_s16b(&p_ptr->csane, flag);
	do_u16b(&p_ptr->csane_frac, flag);

	do_s16b(&p_ptr->msp, flag);
	do_s16b(&p_ptr->csp, flag);
	do_u16b(&p_ptr->csp_frac, flag);

	/* Gods */
	do_s32b(&p_ptr->grace, flag);
	do_s32b(&p_ptr->grace_delay, flag);
	do_bool(&p_ptr->praying, flag);
	do_s16b(&p_ptr->melkor_sacrifice, flag);
	do_byte(&p_ptr->pgod, flag);

	do_s16b(&p_ptr->max_plv, flag);

	// Max dungeon levels
	{
		byte tmp8u = d_info.size();

		do_byte(&tmp8u, flag);

		for (std::size_t i = 0; i < tmp8u; i++)
		{
			s16b tmp16s = max_dlv[i];

			do_s16b(&tmp16s, flag);

			if ((flag == ls_flag_t::LOAD) && (i <= d_info.size()))
			{
				max_dlv[i] = tmp16s;
			}
		}
	}

	/* Repair max player level??? */
	if ((flag == ls_flag_t::LOAD) && (p_ptr->max_plv < p_ptr->lev))
	{
		p_ptr->max_plv = p_ptr->lev;
	}

	/* Help */
	do_std_bool(&p_ptr->help.enabled, flag);
	for (std::size_t i = 0; i < HELP_MAX; i++)
	{
		do_std_bool(&p_ptr->help.activated[i], flag);
	}

	/* More info */
	do_s16b(&p_ptr->blind, flag);
	do_s16b(&p_ptr->paralyzed, flag);
	do_s16b(&p_ptr->confused, flag);
	do_s16b(&p_ptr->food, flag);
	do_s32b(&p_ptr->energy, flag);
	do_s16b(&p_ptr->fast, flag);
	do_s16b(&p_ptr->speed_factor, flag);
	do_s16b(&p_ptr->slow, flag);
	do_s16b(&p_ptr->afraid, flag);
	do_s16b(&p_ptr->cut, flag);
	do_s16b(&p_ptr->stun, flag);
	do_s16b(&p_ptr->poisoned, flag);
	do_s16b(&p_ptr->image, flag);
	do_s16b(&p_ptr->protevil, flag);
	do_s16b(&p_ptr->invuln, flag);
	do_s16b(&p_ptr->hero, flag);
	do_s16b(&p_ptr->shero, flag);
	do_s16b(&p_ptr->shield, flag);
	do_s16b(&p_ptr->shield_power, flag);
	do_s16b(&p_ptr->shield_power_opt, flag);
	do_s16b(&p_ptr->shield_power_opt2, flag);
	do_s16b(&p_ptr->shield_opt, flag);
	do_s16b(&p_ptr->blessed, flag);
	do_s16b(&p_ptr->control, flag);
	do_byte(&p_ptr->control_dir, flag);
	do_s16b(&p_ptr->tim_precognition, flag);
	do_s16b(&p_ptr->tim_thunder, flag);
	do_s16b(&p_ptr->tim_thunder_p1, flag);
	do_s16b(&p_ptr->tim_thunder_p2, flag);
	do_s16b(&p_ptr->tim_project, flag);
	do_s16b(&p_ptr->tim_project_dam, flag);
	do_s16b(&p_ptr->tim_project_gf, flag);
	do_s16b(&p_ptr->tim_project_rad, flag);
	do_s16b(&p_ptr->tim_project_flag, flag);

	do_s16b(&p_ptr->tim_magic_breath, flag);
	do_s16b(&p_ptr->tim_water_breath, flag);

	do_s16b(&p_ptr->tim_roots, flag);
	do_s16b(&p_ptr->tim_roots_ac, flag);
	do_s16b(&p_ptr->tim_roots_dam, flag);

	do_s16b(&p_ptr->tim_invis, flag);
	do_s16b(&p_ptr->word_recall, flag);
	do_byte(&p_ptr->recall_dungeon, flag);
	do_s16b(&p_ptr->see_infra, flag);
	do_s16b(&p_ptr->tim_infra, flag);
	do_s16b(&p_ptr->oppose_fire, flag);
	do_s16b(&p_ptr->oppose_cold, flag);
	do_s16b(&p_ptr->oppose_acid, flag);
	do_s16b(&p_ptr->oppose_elec, flag);
	do_s16b(&p_ptr->oppose_pois, flag);
	do_s16b(&p_ptr->oppose_cc, flag);

	do_s16b(&p_ptr->tim_esp, flag);
	do_s16b(&p_ptr->tim_wraith, flag);
	do_s16b(&p_ptr->tim_ffall, flag);
	do_s16b(&p_ptr->tim_fly, flag);
	do_s16b(&p_ptr->tim_poison, flag);
	do_s16b(&p_ptr->tim_invisible, flag);
	do_s16b(&p_ptr->tim_inv_pow, flag);
	do_s16b(&p_ptr->tim_mimic, flag);
	do_s16b(&p_ptr->lightspeed, flag);
	do_s16b(&p_ptr->tim_lite, flag);
	do_s16b(&p_ptr->tim_regen, flag);
	do_s16b(&p_ptr->tim_regen_pow, flag);
	do_s16b(&p_ptr->holy, flag);
	do_s16b(&p_ptr->immov_cntr, flag);
	do_s16b(&p_ptr->strike, flag);
	do_s16b(&p_ptr->tim_reflect, flag);
	do_s16b(&p_ptr->tim_deadly, flag);
	do_s16b(&p_ptr->prob_travel, flag);
	do_s16b(&p_ptr->disrupt_shield, flag);
	do_s16b(&p_ptr->parasite, flag);
	do_s16b(&p_ptr->parasite_r_idx, flag);
	do_s16b(&p_ptr->absorb_soul, flag);
	do_s32b(&p_ptr->inertia_controlled_spell, flag);
	do_s16b(&p_ptr->last_rewarded_level, flag);

	// Corruptions
	{
		u16b tmp16u = CORRUPTIONS_MAX;

		do_u16b(&tmp16u, flag);

		if (tmp16u > CORRUPTIONS_MAX)
		{
			quit("Too many corruptions");
		}

		for (std::size_t i = 0; i < tmp16u; i++)
		{
			do_bool(&p_ptr->corruptions[i], flag);
		}
	}

	do_bool(&p_ptr->corrupt_anti_teleport_stopped, flag);

	do_byte(&p_ptr->confusing, flag);
	do_bool(&p_ptr->black_breath, flag);
	do_bool(&fate_flag, flag);
	do_byte(&p_ptr->searching, flag);
	do_bool(&ambush_flag, flag);
	do_byte(&p_ptr->allow_one_death, flag);

	do_s16b(&no_breeds, flag);

	/* Auxilliary variables */
	do_u32b(&p_ptr->mimic_extra, flag);
	do_u32b(&p_ptr->antimagic_extra, flag);
	do_u32b(&p_ptr->music_extra, flag);
	do_u32b(&p_ptr->necro_extra, flag);
	do_u32b(&p_ptr->necro_extra2, flag);

	do_u16b(&p_ptr->body_monster, flag);
	do_bool(&p_ptr->disembodied, flag);

	/* Are we in astral mode? */
	do_bool(&p_ptr->astral, flag);

	// Powers
	{
		u16b tmp16u = POWER_MAX;

		do_u16b(&tmp16u, flag);

		if ((flag == ls_flag_t::LOAD) && (tmp16u != POWER_MAX))
		{
			quit("Too few/many powers!");
		}

		for (std::size_t i = 0; i < POWER_MAX; i++)
		{
			do_bool(&p_ptr->powers_mod[i], flag);
		}
	}

	/* The tactic */
	do_char(&p_ptr->tactic, flag);

	/* The movement */
	do_char(&p_ptr->movement, flag);

	/* The comapnions killed */
	do_s16b(&p_ptr->companion_killed, flag);

	/* The fate */
	do_bool(&p_ptr->no_mortal, flag);

	/* Random spells */
	do_vector(flag, p_ptr->random_spells, do_random_spell);

	/* Rune spells */
	do_vector(flag, p_ptr->rune_spells, do_rune_spell);

	/* Random seed for object flavors. */
	do_seed(&seed_flavor(), flag);

	/* Special stuff */
	do_u16b(&total_winner, flag);
	do_u16b(&has_won, flag);
	do_u16b(&noscore, flag);

	/* Write death */
	do_bool(&death, flag);

	/* Incompatible module? */
	if (flag == ls_flag_t::LOAD)
	{
		s32b ok;

		ok = module_savefile_loadable(loaded_game_module);

		/* Argh bad game module! */
		if (!ok)
		{
			note(format("Bad game module. Savefile was saved with module '%s' but game is '%s'.", loaded_game_module, game_module));
			return (FALSE);
		}
	}

	/* Level feeling */
	do_s16b(&feeling, flag);

	/* Turn of last "feeling" */
	do_s32b(&old_turn, flag);

	/* Current turn */
	do_s32b(&turn, flag);

	return TRUE;
}


/*
 * Read a monster
 */
static void do_monster(monster_type *m_ptr, ls_flag_t flag)
{
	/* Read the monster race */
	do_s16b(&m_ptr->r_idx, flag);

	do_u16b(&m_ptr->ego, flag);

	/* Read the other information */
	do_byte(&m_ptr->fy, flag);
	do_byte(&m_ptr->fx, flag);

	do_s32b(&m_ptr->hp, flag);
	do_s32b(&m_ptr->maxhp, flag);

	do_s16b(&m_ptr->csleep, flag);
	do_byte(&m_ptr->mspeed, flag);
	do_byte(&m_ptr->energy, flag);
	do_byte(&m_ptr->stunned, flag);
	do_byte(&m_ptr->confused, flag);
	do_byte(&m_ptr->monfear, flag);
	do_u32b(&m_ptr->smart, flag);
	do_s16b(&m_ptr->status, flag);
	do_s16b(&m_ptr->possessor, flag);
	do_byte(&m_ptr->speed, flag);
	do_byte(&m_ptr->level, flag);
	do_s16b(&m_ptr->ac, flag);
	do_s32b(&m_ptr->exp, flag);
	do_s16b(&m_ptr->target, flag);

	do_s16b(&m_ptr->bleeding, flag);
	do_s16b(&m_ptr->poisoned, flag);

	do_s32b(&m_ptr->mflag, flag);

	if (flag == ls_flag_t::LOAD)
	{
		m_ptr->mflag &= PERM_MFLAG_MASK;
	}

	/* Attacks */
	for (std::size_t i = 0; i < 4; i++)
	{
		do_byte(&m_ptr->blow[i].method, flag);
		do_byte(&m_ptr->blow[i].effect, flag);
		do_byte(&m_ptr->blow[i].d_dice, flag);
		do_byte(&m_ptr->blow[i].d_side, flag);
	}
}



/*
 * Determine if an item can be wielded/worn (e.g. helmet, sword, bow, arrow)
 */
static bool_ wearable_p(object_type *o_ptr)
{
	/* Valid "tval" codes */
	switch (o_ptr->tval)
	{
	case TV_WAND:
	case TV_STAFF:
	case TV_ROD:
	case TV_ROD_MAIN:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
	case TV_BOOMERANG:
	case TV_BOW:
	case TV_DIGGING:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_HELM:
	case TV_CROWN:
	case TV_SHIELD:
	case TV_CLOAK:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
	case TV_SCROLL:
	case TV_LITE:
	case TV_POTION:
	case TV_POTION2:
	case TV_AMULET:
	case TV_RING:
	case TV_HYPNOS:
	case TV_INSTRUMENT:
	case TV_DAEMON_BOOK:
	case TV_TRAPKIT:
	case TV_TOOL:
		{
			return (TRUE);
		}
	}

	/* Nope */
	return (FALSE);
}


/*
 * rd/wr an object
 */
static void do_item(object_type *o_ptr, ls_flag_t flag)
{
	byte old_dd;
	byte old_ds;

	/* Kind */
	do_s16b(&o_ptr->k_idx, flag);

	/* Location */
	do_byte(&o_ptr->iy, flag);
	do_byte(&o_ptr->ix, flag);

	/* Type/Subtype */
	do_byte(&o_ptr->tval, flag);
	do_byte(&o_ptr->sval, flag);

	/* Special pval */
	do_s32b(&o_ptr->pval, flag);

	/* Special pval */
	do_s16b(&o_ptr->pval2, flag);

	/* Special pval */
	do_s32b(&o_ptr->pval3, flag);

	do_byte(&o_ptr->discount, flag);
	do_byte(&o_ptr->number, flag);
	do_s32b(&o_ptr->weight, flag);

	do_byte(&o_ptr->name1, flag);
	do_s16b(&o_ptr->name2, flag);
	do_s16b(&o_ptr->name2b, flag);
	do_s16b(&o_ptr->timeout, flag);

	do_s16b(&o_ptr->to_h, flag);
	do_s16b(&o_ptr->to_d, flag);
	do_s16b(&o_ptr->to_a, flag);

	do_s16b(&o_ptr->ac, flag);

	/* We do special processing of this flag when reading */
	if (flag == ls_flag_t::LOAD)
	{
		do_byte(&old_dd, ls_flag_t::LOAD);
		do_byte(&old_ds, ls_flag_t::LOAD);
	}
	if (flag == ls_flag_t::SAVE)
	{
		do_byte(&o_ptr->dd, ls_flag_t::SAVE);
		do_byte(&o_ptr->ds, ls_flag_t::SAVE);
	}

	do_byte(&o_ptr->ident, flag);

	do_byte(&o_ptr->marked, flag);

	/* flags */
	do_flag_set(&o_ptr->art_flags, flag);

	/* obvious flags */
	do_flag_set(&o_ptr->art_oflags, flag);

	/* Monster holding object */
	do_s16b(&o_ptr->held_m_idx, flag);

	/* Special powers */
	do_byte(&o_ptr->xtra1, flag);
	do_s16b(&o_ptr->xtra2, flag);

	do_byte(&o_ptr->elevel, flag);
	do_s32b(&o_ptr->exp, flag);

	/* Read the pseudo-id */
	do_byte(&o_ptr->sense, flag);

	/* Read the found info */
	do_byte(&o_ptr->found, flag);
	do_s16b(&o_ptr->found_aux1, flag);
	do_s16b(&o_ptr->found_aux2, flag);
	do_s16b(&o_ptr->found_aux3, flag);
	do_s16b(&o_ptr->found_aux4, flag);

	// Inscription
	do_std_string(o_ptr->inscription, flag);

	// Artifact name
	do_std_string(o_ptr->artifact_name, flag);

	/* Stick any more shared code before this. The rest
	   of this function is reserved for ls_flag_t::LOAD's
	   cleanup functions */

	if (flag == ls_flag_t::SAVE) return;

	/*********** END OF ls_flag_t::SAVE ***************/

	/* Obtain the "kind" template */
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Obtain tval/sval from k_info */
	o_ptr->tval = k_ptr->tval;
	if (o_ptr->tval != TV_RANDART) o_ptr->sval = k_ptr->sval;


	/* Repair non "wearable" items */
	if (!wearable_p(o_ptr))
	{
		/* Acquire correct fields */
		o_ptr->to_h = k_ptr->to_h;
		o_ptr->to_d = k_ptr->to_d;
		o_ptr->to_a = k_ptr->to_a;

		/* Acquire correct fields */
		o_ptr->ac = k_ptr->ac;
		o_ptr->dd = k_ptr->dd;
		o_ptr->ds = k_ptr->ds;

		/* All done */
		return;
	}


	/* Paranoia */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Verify that artifact */
		if (!a_ptr->name) o_ptr->name1 = 0;
	}

	/* Paranoia */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr;

		/* Obtain the ego-item info */
		e_ptr = &e_info[o_ptr->name2];

		/* Verify that ego-item */
		if (!e_ptr->name) o_ptr->name2 = 0;
	}


	/* Acquire standard fields */
	o_ptr->ac = k_ptr->ac;
	o_ptr->dd = k_ptr->dd;
	o_ptr->ds = k_ptr->ds;

	/* Artifacts */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr;

		/* Obtain the artifact info */
		a_ptr = &a_info[o_ptr->name1];

		/* Acquire new artifact fields */
		o_ptr->ac = a_ptr->ac;
		o_ptr->dd = a_ptr->dd;
		o_ptr->ds = a_ptr->ds;

		/* Acquire new artifact weight */
		o_ptr->weight = a_ptr->weight;
	}

	/* Ego items */
	if (o_ptr->name2)
	{
		o_ptr->dd = old_dd;
		o_ptr->ds = old_ds;
	}

	if (!o_ptr->artifact_name.empty())	/* A random artifact */
	{
		o_ptr->dd = old_dd;
		o_ptr->ds = old_ds;
	}
}

static void do_cave_type(cave_type *c_ptr, ls_flag_t flag)
{
	do_u16b(&c_ptr->info, flag);
	do_byte(&c_ptr->feat, flag);
	do_byte(&c_ptr->mimic, flag);
	do_s16b(&c_ptr->special, flag);
	do_s16b(&c_ptr->special2, flag);
	do_s16b(&c_ptr->t_idx, flag);
	do_s16b(&c_ptr->inscription, flag);
	do_byte(&c_ptr->mana, flag);
	do_s16b(&c_ptr->effect, flag);
}

static void do_grid(ls_flag_t flag)
{
	for (int y = 0; y < cur_hgt; y++)
	{
		for (int x = 0; x < cur_wid; x++)
		{
			do_cave_type(&cave[y][x], flag);
		}
	}
}

static bool do_objects(ls_flag_t flag, bool no_companions)
{
	if (flag == ls_flag_t::SAVE)
	{
		// Compact everything before saving
		compact_objects(0);
		compact_monsters(0);
	}

	u16b n_objects = o_max;

	if (flag == ls_flag_t::SAVE)
	{
		u16b tmp16u = n_objects;

		if (no_companions)
		{
			for (int i = 1; i < o_max; i++)
			{
				object_type *o_ptr = &o_list[i];

				if (o_ptr->held_m_idx && (m_list[o_ptr->held_m_idx].status == MSTATUS_COMPANION))
				{
					tmp16u--;
				}
			}
		}

		do_u16b(&tmp16u, flag);
	}
	else
	{
		do_u16b(&n_objects, flag);
	}

	/* Verify maximum */
	if ((flag == ls_flag_t::LOAD) && (n_objects > max_o_idx))
	{
		note("Too many object entries!");
		return false;
	}

	/* Dungeon items */
	if (flag == ls_flag_t::SAVE)
	{
		for (std::size_t i = 1; i < n_objects; i++)
		{
			auto o_ptr = &o_list[i];
			// Skip objects held by companions when no_companions is set
			if (no_companions && o_ptr->held_m_idx && (m_list[o_ptr->held_m_idx].status == MSTATUS_COMPANION))
			{
				continue;
			}

			do_item(o_ptr, ls_flag_t::SAVE);
		}
	}
	else if (flag == ls_flag_t::LOAD)
	{
		for (int i = 1; i < n_objects; i++)
		{
			/* Get a new record */
			int o_idx = o_pop();

			/* Oops */
			if (i != o_idx)
			{
				note(format("Object allocation error (%d <> %d)", i, o_idx));
				return false;
			}

			/* Acquire place */
			auto o_ptr = &o_list[o_idx];

			/* Read the item */
			do_item(o_ptr, ls_flag_t::LOAD);

			/* Monster */
			if (o_ptr->held_m_idx)
			{
				/* Monster */
				monster_type *m_ptr = &m_list[o_ptr->held_m_idx];

				/* Place the object */
				m_ptr->hold_o_idxs.push_back(o_idx);
			}

			/* Dungeon */
			else
			{
				/* Access the item location */
				auto c_ptr = &cave[o_ptr->iy][o_ptr->ix];

				/* Place the object */
				c_ptr->o_idxs.push_back(o_idx);
			}
		}
	}

	return true;
}


static bool do_monsters(ls_flag_t flag, bool no_companions)
{
	auto &r_info = game->edit_data.r_info;

	u16b n_monsters = m_max;

	if (flag == ls_flag_t::SAVE)
	{
		u16b tmp16u = m_max;

		if (no_companions)
		{
			for (int i = 1; i < m_max; i++)
			{
				monster_type *m_ptr = &m_list[i];

				if (m_ptr->status == MSTATUS_COMPANION)
				{
					tmp16u--;
				}
			}
		}

		do_u16b(&tmp16u, flag);
	}
	else
	{
		do_u16b(&n_monsters, flag);
	}

	/* Validate */
	if ((flag == ls_flag_t::LOAD) && (n_monsters > max_m_idx))
	{
		note("Too many monster entries!");
		return false;
	}

	/* Load/save the monsters */

	if (flag == ls_flag_t::SAVE)
	{
		for (std::size_t i = 1; i < n_monsters; i++)
		{
			auto m_ptr = &m_list[i];

			// Skip companions when no_companions is set
			if (no_companions && m_ptr->status == MSTATUS_COMPANION)
			{
				continue;
			}

			do_monster(m_ptr, ls_flag_t::SAVE);
		}
	}
	else if (flag == ls_flag_t::LOAD)
	{
		for (int i = 1; i < n_monsters; i++)
		{
			/* Get a new record */
			int m_idx = m_pop();

			/* Oops */
			if (i != m_idx)
			{
				note(format("Monster allocation error (%d <> %d)", i, m_idx));
				return false;
			}

			/* Acquire monster */
			auto m_ptr = &m_list[m_idx];

			/* Read the monster */
			do_monster(m_ptr, ls_flag_t::LOAD);

			/* Place in grid */
			auto c_ptr = &cave[m_ptr->fy][m_ptr->fx];
			c_ptr->m_idx = m_idx;

			/* Controlled? */
			if (m_ptr->mflag & MFLAG_CONTROL)
			{
				p_ptr->control = m_idx;
			}

			/* Count as an alive member of race */
			auto r_ptr = &r_info[m_ptr->r_idx];
			r_ptr->cur_num++;
		}
	}

	/* Save/load pets */
	{
		u16b tmp16u = (flag == ls_flag_t::SAVE && !no_companions) ? max_m_idx : 0;

		do_u16b(&tmp16u, flag);

		if ((flag == ls_flag_t::LOAD) && (tmp16u > max_m_idx))
		{
			note("Too many monster entries!");
			return false;
		}

		for (std::size_t i = 1; i < tmp16u; i++)
		{
			/* Acquire monster */
			auto m_ptr = &km_list[i];

			/* Read the monster */
			do_monster(m_ptr, flag);
		}
	}

	return true;
}

/*
 * Handle dungeon
 *
 * The monsters/objects must be loaded in the same order
 * that they were stored, since the actual indexes matter.
 */
static bool_ do_dungeon(ls_flag_t flag, bool no_companions)
{
	/* Header info */
	do_s16b(&dun_level, flag);
	do_byte(&dungeon_type, flag);
	do_s16b(&num_repro, flag);
	do_s16b(&p_ptr->py, flag);
	do_s16b(&p_ptr->px, flag);
	do_s16b(&cur_hgt, flag);
	do_s16b(&cur_wid, flag);
	do_s16b(&max_panel_rows, flag);
	do_s16b(&max_panel_cols, flag);

	do_flag_set(&dungeon_flags, flag);

	/* Last teleportation */
	do_s16b(&last_teleportation_y, flag);
	do_s16b(&last_teleportation_y, flag);

	/* Spell effects */
	{
		u16b n_effects = MAX_EFFECTS;
		do_u16b(&n_effects, flag);

		if ((flag == ls_flag_t::LOAD) && (n_effects > MAX_EFFECTS))
		{
			quit("Too many spell effects");
		}

		for (std::size_t i = 0; i < n_effects; ++i)
		{
			do_s16b(&effects[i].type, flag);
			do_s16b(&effects[i].dam, flag);
			do_s16b(&effects[i].time, flag);
			do_u32b(&effects[i].flags, flag);
			do_s16b(&effects[i].cx, flag);
			do_s16b(&effects[i].cy, flag);
			do_s16b(&effects[i].rad, flag);
		}
	}

	/* To prevent bugs with evolving dungeons */
	for (std::size_t i = 0; i < 100; i++)
	{
		do_s16b(&floor_type[i], flag);
		do_s16b(&fill_type[i], flag);
	}

	if ((flag == ls_flag_t::LOAD) && (!dun_level && !p_ptr->inside_quest))
	{
		int xstart = 0;
		int ystart = 0;
		/* Init the wilderness */
		process_dungeon_file("w_info.txt", &ystart, &xstart, cur_hgt, cur_wid,
				     TRUE, FALSE);

		/* Init the town */
		xstart = 0;
		ystart = 0;
		init_flags = 0;
		process_dungeon_file("t_info.txt", &ystart, &xstart, cur_hgt, cur_wid,
				     TRUE, FALSE);
	}

	do_grid(flag);

	/*** Objects ***/
	if (!do_objects(flag, no_companions))
	{
		return FALSE;
	}

	/*** Monsters ***/
	if (!do_monsters(flag, no_companions))
	{
		return FALSE;
	}

	/*** Success ***/

	/* The dungeon is ready */
	if (flag == ls_flag_t::LOAD) character_dungeon = TRUE;

	/* Success */
	return (TRUE);
}

/* Save the current persistent dungeon -SC- */
void save_dungeon(void)
{
	char tmp[16];
	char name[1024], buf[5];

	/* Save only persistent dungeons */
	if (!get_dungeon_save(buf) || (!dun_level)) return;

	/* Construct filename */
	sprintf(tmp, "%s.%s", player_base, buf);
	path_build(name, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Open the file */
	fff = my_fopen(name, "wb");

	/* Save the dungeon */
	do_dungeon(ls_flag_t::SAVE, true);

	my_fclose(fff);
}

bool_ file_exist(cptr buf)
{
	int fd;
	bool_ result;

	/* Open savefile */
	fd = fd_open(buf, O_RDONLY);

	/* File exists */
	if (fd >= 0)
	{
		fd_close(fd);
		result = TRUE;
	}
	else
		result = FALSE;

	return result;
}


/*
 * Handle monster lore
 */
static void do_lore(std::size_t r_idx, ls_flag_t flag)
{
	auto &r_info = game->edit_data.r_info;

	auto r_ptr = &r_info[r_idx];

	do_s16b(&r_ptr->r_pkills, flag);
	do_s16b(&r_ptr->max_num, flag);
	do_bool(&r_ptr->on_saved, flag);
}




/*
 * Read a store
 */
static void do_store(store_type *str, ls_flag_t flag)
{
	// Store state
	do_s32b(&str->store_open, flag);
	do_u16b(&str->owner, flag);
	do_s32b(&str->last_visit, flag);

	// Items in store
	{
		byte num = str->stock.size();

		do_byte(&num, flag);

		if (flag == ls_flag_t::SAVE)
		{
			for (std::size_t i = 0; i < num; i++)
			{
				do_item(&str->stock[i], ls_flag_t::SAVE);
			}
		}
		else if (flag == ls_flag_t::LOAD)
		{
			for (std::size_t i = 0; i < num; i++)
			{
				object_type forge;
				object_wipe(&forge);
				do_item(&forge, ls_flag_t::LOAD);

				if ((str->stock.size() < STORE_INVEN_MAX) && (str->stock.size() < str->stock_size))
				{
					object_type stock_obj;
					object_copy(&stock_obj, &forge);
					str->stock.push_back(stock_obj);
				}
			}
		}
	}
}

/*
 * RNG state
 */
static void do_randomizer(ls_flag_t flag)
{
	std::string state;

        if (flag == ls_flag_t::SAVE)
        {
		state = get_complex_rng_state();
        }

	do_std_string(state, flag);

	if (flag == ls_flag_t::LOAD)
	{
		set_complex_rng_state(state);
	}

	/* Accept */
	if (flag == ls_flag_t::LOAD)
	{
		set_complex_rng();
	}
}

/*
 * Handle options
 *
 * Normal options are stored as a set of 256 bit flags,
 * plus a set of 256 bit masks to indicate which bit flags were defined
 * at the time the savefile was created.  This will allow new options
 * to be added, and old options to be removed, at any time, without
 * hurting old savefiles.
 *
 * The window options are stored in the same way, but note that each
 * window gets 32 options, and their order is fixed by certain defines.
 */
static void do_options(ls_flag_t flag)
{
	int i, n;

	u32b oflag[8];
	u32b mask[8];

	/*** Special info */

	/* Read "delay_factor" */
	do_byte(&options->delay_factor, flag);

	/* Read "hitpoint_warn" */
	do_byte(&options->hitpoint_warn, flag);

	/*** Cheating options ***/
	do_bool(&wizard, flag);
	do_bool(&options->cheat_peek, flag);
	do_bool(&options->cheat_hear, flag);
	do_bool(&options->cheat_room, flag);
	do_bool(&options->cheat_xtra, flag);
	do_bool(&options->cheat_live, flag);

	/*** Autosave options */
	do_bool(&options->autosave_l, flag);
	do_bool(&options->autosave_t, flag);
	do_s16b(&options->autosave_freq, flag);

	// Standard options
	{
		std::vector<option_value> option_values;

		// If we're saving we need to map to a vector of key-value pairs.
		if (flag == ls_flag_t::SAVE)
		{
			for (auto const &option: options->standard_options)
			{
				option_values.emplace_back(
				        option_value {
				                option.o_text,
				                *option.o_var
				        }
				);
			}
		}

		// Read/write the option values
		do_vector(flag, option_values, do_option_value);

		// If we're loading we need to set options based of the key-value pairs.
		if (flag == ls_flag_t::LOAD)
		{
			// Go through all the options that were loaded.
			for (auto const &option_value: option_values)
			{
				// We need to search through all the options
				// that are actually in the game; we'll ignore
				// saved options that are now gone.
				const option_type *found_option;
				for (auto const &option: options->standard_options)
				{
					if (option_value.name == option.o_text)
					{
						found_option = &option;
						break;
					}
				}

				// If we found the option, we'll set the value.
				if (found_option)
				{
					*(*found_option).o_var = option_value.value;
				}
			}
		}

	}

	if (flag == ls_flag_t::LOAD)
	{
		/*** Window Options ***/

		/* Read the window flags */
		for (n = 0; n < 8; n++) do_u32b(&oflag[n], flag);

		/* Read the window masks */
		for (n = 0; n < 8; n++) do_u32b(&mask[n], flag);

		/* Analyze the options */
		for (n = 0; n < 8; n++)
		{
			/* Analyze the options */
			for (i = 0; i < 32; i++)
			{
				/* Process valid flags */
				if (mask[n] & (1L << i))
				{
					/* Process valid flags */
					if (window_mask[n] & (1L << i))
					{
						/* Set */
						if (oflag[n] & (1L << i))
						{
							/* Set */
							window_flag[n] |= (1L << i);
						}

						/* Clear */
						else
						{
							/* Clear */
							window_flag[n] &= ~(1L << i);
						}
					}
				}
			}
		}
	}

	if (flag == ls_flag_t::SAVE)
	{
		/*** Window options ***/

		/* Dump the flags */
		for (i = 0; i < 8; i++) do_u32b(&window_flag[i], flag);

		/* Dump the masks */
		for (i = 0; i < 8; i++) do_u32b(&window_mask[i], flag);
	}
}


/*
 * Handle player inventory. Note that the inventory is
 * "re-sorted" later by "dungeon()".
 */
static bool_ do_inventory(ls_flag_t flag)
{
	if (flag == ls_flag_t::LOAD)
	{
		int slot = 0;

		object_type forge;
		object_type *q_ptr;

		/* No items */
		inven_cnt = 0;
		equip_cnt = 0;

		/* Read until done */
		while (1)
		{
			u16b n;

			/* Get the next item index */
			do_u16b(&n, ls_flag_t::LOAD);

			/* Nope, we reached the end */
			if (n == 0xFFFF) break;

			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Read the item */
			do_item(q_ptr, ls_flag_t::LOAD);

			/* Hack -- verify item */
			if (!q_ptr->k_idx) return (FALSE);

			/* Wield equipment */
			if (n >= INVEN_WIELD)
			{
				/* Copy object */
				object_copy(&p_ptr->inventory[n], q_ptr);

				/* Take care of item sets */
				if (q_ptr->name1)
				{
					wield_set(q_ptr->name1, a_info[q_ptr->name1].set, TRUE);
				}

				/* One more item */
				equip_cnt++;
			}

			/* Warning -- backpack is full */
			else if (inven_cnt == INVEN_PACK)
			{
				/* Oops */
				note("Too many items in the inventory!");

				/* Fail */
				return (FALSE);
			}

			/* Carry inventory */
			else
			{
				/* Get a slot */
				n = slot++;

				/* Copy object */
				object_copy(&p_ptr->inventory[n], q_ptr);

				/* One more item */
				inven_cnt++;
			}
		}
	}
	if (flag == ls_flag_t::SAVE)
	{
		for (u16b i = 0; i < INVEN_TOTAL; i++)
		{
			object_type *o_ptr = &p_ptr->inventory[i];
			if (!o_ptr->k_idx) continue;
			do_u16b(&i, flag);
			do_item(o_ptr, flag);
		}
		// Sentinel
		u16b sent = 0xFFFF;
		do_u16b(&sent, ls_flag_t::SAVE);
	}
	/* Success */
	return (TRUE);
}


static void do_message(message &msg, ls_flag_t flag)
{
	do_std_string(msg.text, flag);
	do_u32b(&msg.count, flag);
	do_byte(&msg.color, flag);
}


/*
 * Read the saved messages
 */
static void do_messages(ls_flag_t flag)
{
	/* Save/load number of messages */
	s16b num = message_num();
	do_s16b(&num, flag);

	/* Read the messages */
	for (int i = 0; i < num; i++)
	{
		message message;

		if (flag == ls_flag_t::SAVE)
		{
			message = message_at(i);
		}

		do_message(message, flag);

		if (flag == ls_flag_t::LOAD)
		{
			message_add(message);
		}
	}
}

/* Returns TRUE if we successfully load the dungeon */
bool_ load_dungeon(char *ext)
{
	char tmp[16];
	char name[1024];
	byte old_dungeon_type = dungeon_type;
	s16b old_dun = dun_level;

	/* Construct name */
	sprintf(tmp, "%s.%s", player_base, ext);
	path_build(name, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Open the file */
	fff = my_fopen(name, "rb");

	if (fff == NULL)
	{
		dun_level = old_dun;
		dungeon_type = old_dungeon_type;

		my_fclose(fff);
		return (FALSE);
	}

	/* Read the dungeon */
	if (!do_dungeon(ls_flag_t::LOAD, false))
	{
		dun_level = old_dun;
		dungeon_type = old_dungeon_type;

		my_fclose(fff);
		return (FALSE);
	}

	dun_level = old_dun;
	dungeon_type = old_dungeon_type;

	/* Done */
	my_fclose(fff);
	return (TRUE);
}

/*
 * Load/save timers.
 */
static void do_timers(ls_flag_t flag)
{
	timer_type *t_ptr;

	for (t_ptr = gl_timers; t_ptr != NULL; t_ptr = t_ptr->next)
	{
		do_bool(&t_ptr->enabled, flag);
		do_s32b(&t_ptr->delay, flag);
		do_s32b(&t_ptr->countdown, flag);
	}
}

/*
 * Load/save stores.
 */
static void do_stores(ls_flag_t flag)
{
	auto const &st_info = game->edit_data.st_info;

	// Indexes for "real" towns.
	std::vector<byte> reals;
	reals.reserve(max_towns);

	// Fill in the "real" towns if necessary.
	if (flag == ls_flag_t::SAVE)
	{
		for (int i = 1; i < max_towns; i++)
		{
			if (!(town_info[i].flags & TOWN_REAL))
			{
				continue;
			}
			reals.emplace_back(i);
		}
	}

	// Load/save
	do_vector(flag, reals, do_byte);

	/* Read the stores */
	u16b n_stores = st_info.size();
	do_u16b(&n_stores, flag);
	assert(n_stores <= st_info.size());

	for (auto const z: reals)
	{
		if (!town_info[z].stocked)
		{
			create_stores_stock(z);
		}

		for (int j = 0; j < n_stores; j++)
		{
			do_store(&town_info[z].store[j], flag);
		}
	}
}

/*
 * Monster memory
 */
static bool do_monster_lore(ls_flag_t flag)
{
	auto const &r_info = game->edit_data.r_info;

	u16b tmp16u = r_info.size();

	do_u16b(&tmp16u, flag);

	if ((flag == ls_flag_t::LOAD) && (tmp16u > r_info.size()))
	{
		note("Too many monster races!");
		return false;
	}

	for (std::size_t i = 0; i < tmp16u; i++)
	{
		do_lore(i, flag);
	}

	return true;
}


/*
 * Object memory
 */
static bool do_object_lore(ls_flag_t flag)
{
	u16b n_kinds = max_k_idx;

	do_u16b(&n_kinds, flag);

	if ((flag == ls_flag_t::LOAD) && (n_kinds > max_k_idx))
	{
		note("Too many object kinds!");
		return false;
	}

	for (std::size_t i = 0; i < n_kinds; i++)
	{
		object_kind *k_ptr = &k_info[i];
		do_bool(&k_ptr->aware, flag);
		do_bool(&k_ptr->tried, flag);
		do_bool(&k_ptr->artifact, flag);
	}

	return true;
}


static bool do_towns(ls_flag_t flag)
{
	auto &d_info = game->edit_data.d_info;

	u16b max_towns_ldsv = max_towns;

	do_u16b(&max_towns_ldsv, flag);

	if ((flag == ls_flag_t::LOAD) && (max_towns_ldsv > max_towns))
	{
		note("Too many towns!");
		return false;
	}

	if (flag == ls_flag_t::SAVE)
	{
		max_towns_ldsv = TOWN_RANDOM;
	}

	do_u16b(&max_towns_ldsv, flag);

	if ((flag == ls_flag_t::LOAD) && (max_towns_ldsv != TOWN_RANDOM))
	{
		note("Different random towns base!");
		return false;
	}

	for (std::size_t i = 0; i < max_towns; i++)
	{
		auto town = &town_info[i];

		do_bool(&town->destroyed, flag);

		if (i >= TOWN_RANDOM)
		{
			do_seed(&town->seed, flag);
			do_byte(&town->flags, flag);

			// Create stock if necessary
			if ((town_info->flags & TOWN_REAL) && (flag == ls_flag_t::LOAD))
			{
				create_stores_stock(i);
			}
		}
	}

	if (flag == ls_flag_t::SAVE)
	{
		max_towns_ldsv = d_info.size();
	}

	do_u16b(&max_towns_ldsv, flag);

	if ((flag == ls_flag_t::LOAD) && (max_towns_ldsv > d_info.size()))
	{
		note("Too many dungeon types!");
		return false;
	}

	// Town quest entrances
	u16b max_quests_ldsv = TOWN_DUNGEON;
	do_u16b(&max_quests_ldsv, flag);

	if ((flag == ls_flag_t::LOAD) && (max_quests_ldsv > TOWN_DUNGEON))
	{
		note("Too many town per dungeons!");
		return false;
	}

	for (std::size_t i = 0; i < max_towns_ldsv; i++)
	{
		for (std::size_t j = 0; j < max_quests_ldsv; j++)
		{
			do_s16b(&(d_info[i].t_idx[j]), flag);
			do_s16b(&(d_info[i].t_level[j]), flag);
		}
		do_s16b(&(d_info[i].t_num), flag);
	}

	return true;
}

static bool do_quests(ls_flag_t flag)
{
	u16b max_quests_ldsv = MAX_Q_IDX;

	do_u16b(&max_quests_ldsv, flag);

	if ((flag == ls_flag_t::LOAD) && (max_quests_ldsv != MAX_Q_IDX))
	{
		note("Invalid number of quests!");
		return false;
	}

	for (std::size_t i = 0; i < MAX_Q_IDX; i++)
	{
		auto q = &quest[i];

		do_s16b(&q->status, flag);
		for (auto &quest_data : q->data)
		{
			do_s32b(&quest_data, flag);
		}

		// Initialize the quest if necessary
		if ((flag == ls_flag_t::LOAD) && (q->init != NULL))
		{
			q->init();
		}
	}

	return true;
}

static bool do_wilderness(ls_flag_t flag)
{
	auto &wilderness = game->wilderness;

	// Player position and "mode" wrt. wilderness
	do_s32b(&p_ptr->wilderness_x, flag);
	do_s32b(&p_ptr->wilderness_y, flag);
	do_bool(&p_ptr->wild_mode, flag);
	do_bool(&p_ptr->old_wild_mode, flag);

	// Size of the wilderness
	u16b wild_x_size = wilderness.width();
	u16b wild_y_size = wilderness.height();
	do_u16b(&wild_x_size, flag);
	do_u16b(&wild_y_size, flag);

	if (flag == ls_flag_t::LOAD)
	{
		if ((wild_x_size > wilderness.width()) || (wild_y_size > wilderness.height()))
		{
			note("Wilderness is too big!");
			return false;
		}
	}

	// Save/load wilderness tile state
	for (std::size_t x = 0; x < wild_x_size; x++)
	{
		for (std::size_t y = 0; y < wild_y_size; y++)
		{
			auto w = &wilderness(x, y);
			do_seed(&w->seed, flag);
			do_u16b(&w->entrance, flag);
			do_bool(&w->known, flag);
		}
	}

	return true;
}

static bool do_randarts(ls_flag_t flag)
{
	u16b n_randarts = MAX_RANDARTS;

	do_u16b(&n_randarts, flag);

	if ((flag == ls_flag_t::LOAD) && (n_randarts > MAX_RANDARTS))
	{
		note("Too many random artifacts!");
		return false;
	}

	for (std::size_t i = 0; i < n_randarts; i++)
	{
		random_artifact *ra_ptr = &random_artifacts[i];

		do_string(ra_ptr->name_full, 80, flag);
		do_string(ra_ptr->name_short, 80, flag);
		do_byte(&ra_ptr->level, flag);
		do_byte(&ra_ptr->attr, flag);
		do_u32b(&ra_ptr->cost, flag);
		do_byte(&ra_ptr->activation, flag);
		do_byte(&ra_ptr->generated, flag);
	}

	return true;
}

static bool do_artifacts(ls_flag_t flag)
{
	u16b n_artifacts;

	if (flag == ls_flag_t::SAVE)
	{
		n_artifacts = max_a_idx;
	}

	do_u16b(&n_artifacts, flag);

	if ((flag == ls_flag_t::LOAD) && (n_artifacts > max_a_idx))
	{
		note("Too many artifacts!");
		return false;
	}

	for (std::size_t i = 0; i < n_artifacts; i++)
	{
		do_byte(&(&a_info[i])->cur_num, flag);
	}

	return true;
}

static bool do_fates(ls_flag_t flag)
{
	u16b n_fates = MAX_FATES;

	do_u16b(&n_fates, flag);

	if ((flag == ls_flag_t::LOAD) && (n_fates > MAX_FATES))
	{
		note("Too many fates!");
		return false;
	}

	for (std::size_t i = 0; i < n_fates; i++)
	{
		auto fate = &fates[i];
		do_byte(&fate->fate, flag);
		do_byte(&fate->level, flag);
		do_byte(&fate->serious, flag);
		do_s16b(&fate->o_idx, flag);
		do_s16b(&fate->e_idx, flag);
		do_s16b(&fate->a_idx, flag);
		do_s16b(&fate->v_idx, flag);
		do_s16b(&fate->r_idx, flag);
		do_s16b(&fate->count, flag);
		do_s16b(&fate->time, flag);
		do_bool(&fate->know, flag);
	}

	return true;
}

static bool do_traps(ls_flag_t flag)
{
	u16b n_traps = max_t_idx;

	do_u16b(&n_traps, flag);

	if ((flag == ls_flag_t::LOAD) && (n_traps > max_t_idx))
	{
		note("Too many traps!");
		return false;
	}

	for (std::size_t i = 0; i < n_traps; i++)
	{
		do_bool(&t_info[i].ident, flag);
	}

	return true;
}

static bool do_floor_inscriptions(ls_flag_t flag)
{
	u16b n_inscriptions = MAX_INSCRIPTIONS;
	do_u16b(&n_inscriptions, flag);

	if ((flag == ls_flag_t::LOAD) && (n_inscriptions > MAX_INSCRIPTIONS))
	{
		note("Too many inscriptions!");
		return false;
	}

	for (std::size_t i = 0; i < n_inscriptions; i++)
	{
		do_std_bool(&p_ptr->inscriptions[i], flag);
	}

	return true;
}

static bool do_player_hd(ls_flag_t flag)
{
	auto &player_hp = game->player_hp;

	u16b max_level = PY_MAX_LEVEL;

	do_u16b(&max_level, flag);

	if ((flag == ls_flag_t::LOAD) && (max_level > PY_MAX_LEVEL))
	{
		note("Too many hitpoint entries!");
		return false;
	}

	for (std::size_t i = 0; i < max_level; i++)
	{
		do_s16b(&player_hp[i], flag);
	}

	return true;
}


/*
 * Actually read the savefile
 */
static bool_ do_savefile_aux(ls_flag_t flag)
{
	auto &class_info = game->edit_data.class_info;
	auto const &race_info = game->edit_data.race_info;
	auto const &race_mod_info = game->edit_data.race_mod_info;

	/* Mention the savefile version */
	if (flag == ls_flag_t::LOAD)
	{
		if (vernum != SAVEFILE_VERSION)
		{
			note("Incompatible save file version");
			return FALSE;
		}
	}
	if (flag == ls_flag_t::SAVE)
	{
		sf_when = time((time_t *) 0); 	/* Note when file was saved */
		sf_saves++; 				/* Increment the saves ctr */
	}

	/* Handle version bytes */
	if (flag == ls_flag_t::LOAD)
	{
		/* Discard all this, we've already read it */
		u32b mt32b;
		do_u32b(&mt32b, flag);
	}
	if (flag == ls_flag_t::SAVE)
	{
		u32b saver;
		saver = SAVEFILE_VERSION;
		do_u32b(&saver, flag);
	}

	/* Time of last save */
	do_u32b(&sf_when, flag);

	/* Number of past lives */
	do_u16b(&sf_lives, flag);

	/* Number of times saved */
	do_u16b(&sf_saves, flag);

	/* Game module */
	if (flag == ls_flag_t::SAVE)
	{
		strcpy(loaded_game_module, game_module);
	}
	do_string(loaded_game_module, 80, flag);

	/* Timers */
	do_timers(flag);

	/* Read RNG state */
	do_randomizer(flag);

	/* Automatizer state */
	do_bool(&automatizer_enabled, flag);

	/* Then the options */
	do_options(flag);

	/* Then the "messages" */
	do_messages(flag);

	if (!do_monster_lore(flag))
	{
		return FALSE;
	}

	if (!do_object_lore(flag))
	{
		return FALSE;
	}

	if (!do_towns(flag))
	{
		return FALSE;
	}

	if (!do_quests(flag))
	{
		return FALSE;
	}

	if (!do_wilderness(flag))
	{
		return FALSE;
	}

	if (!do_randarts(flag))
	{
		return FALSE;
	}

	if (!do_artifacts(flag))
	{
		return FALSE;
	}

	if (!do_fates(flag))
	{
		return FALSE;
	}

	if (!do_traps(flag))
	{
		return FALSE;
	}

	if (!do_floor_inscriptions(flag))
	{
		return FALSE;
	}

	if (!do_extra(flag))
	{
		return FALSE;
	}

	if (!do_player_hd(flag))
	{
		return FALSE;
	}

	if (flag == ls_flag_t::LOAD)
	{
		// Make sure that the auxiliary pointers for player
		// class, race, etc. point to the right places.
		rp_ptr = &race_info[p_ptr->prace];
		rmp_ptr = &race_mod_info[p_ptr->pracem];
		cp_ptr = &class_info[p_ptr->pclass];
		spp_ptr = &class_info[p_ptr->pclass].spec[p_ptr->pspec];
	}

	/* Read the pet command settings */
	do_byte(&p_ptr->pet_follow_distance, flag);
	do_byte(&p_ptr->pet_open_doors, flag);
	do_byte(&p_ptr->pet_pickup_items, flag);

	/* Dripping Tread */
	do_s16b(&p_ptr->dripping_tread, flag);

	/* Read the inventory */
	if (!do_inventory(flag))
	{
		if (flag == ls_flag_t::LOAD)
		{
			note("Unable to read inventory");
			return FALSE;
		}
	}

	/* Stores */
	do_stores(flag);

	/* I'm not dead yet... */
	if (!death)
	{
		/* Dead players have no dungeon */
		if (flag == ls_flag_t::LOAD)
		{
			note("Restoring Dungeon...");
		}

		if ((flag == ls_flag_t::LOAD) && (!do_dungeon(ls_flag_t::LOAD, false)))
		{
			note("Error reading dungeon data");
			return FALSE;
		}

		if (flag == ls_flag_t::SAVE)
		{
			do_dungeon(ls_flag_t::SAVE, false);
		}
	}

	/* Success */
	return TRUE;
}



/*
 * Actually read the savefile
 */
static errr rd_savefile(void)
{
	errr err = 0;

	/* The savefile is a binary file */
	fff = my_fopen(savefile, "rb");

	/* Paranoia */
	if (!fff) return ( -1);

	/* Call the sub-function */
	err = !do_savefile_aux(ls_flag_t::LOAD);

	/* Check for errors */
	if (ferror(fff)) err = -1;

	/* Close the file */
	my_fclose(fff);

	/* Result */
	return (err);
}


/*
 * Attempt to Load a "savefile"
 *
 * On multi-user systems, you may only "read" a savefile if you will be
 * allowed to "write" it later, this prevents painful situations in which
 * the player loads a savefile belonging to someone else, and then is not
 * allowed to save his game when he quits.
 *
 * We return "TRUE" if the savefile was usable, and we set the global
 * flag "character_loaded" if a real, living, character was loaded.
 *
 * Note that we always try to load the "current" savefile, even if
 * there is no such file, so we must check for "empty" savefile names.
 */
bool_ load_player(void)
{
	errr err = 0;

	cptr what = "generic";

	/* Paranoia */
	turn = 0;

	/* Paranoia */
	death = FALSE;


	/* Allow empty savefile name */
	if (!savefile[0]) return (TRUE);


	/* XXX XXX XXX Fix this */

	/* Verify the existance of the savefile */
	if (!file_exist(savefile))
	{
		/* Give a message */
		msg_format("Savefile does not exist: %s", savefile);
		msg_print(NULL);

		/* Allow this */
		return (TRUE);
	}

	/* Okay */
	if (!err)
	{
		/* Open the savefile */
		int fd = fd_open(savefile, O_RDONLY);

		/* No file */
		if (fd < 0) err = -1;

		/* Message (below) */
		if (err) what = "Cannot open savefile";

		/* Close the file */
		if (!err) fd_close(fd);
	}

	/* Process file */
	if (!err)
	{
		/* Open the file XXX XXX XXX XXX Should use Angband file interface */
		fff = my_fopen(savefile, "rb");

		/* Read the first four bytes */
		do_u32b(&vernum, ls_flag_t::LOAD);

		my_fclose(fff);

	}

	/* Process file */
	if (!err)
	{

		/* Extract version */
		sf_major = VERSION_MAJOR;
		sf_minor = VERSION_MINOR;
		sf_patch = VERSION_PATCH;

		/* Clear screen */
		Term_clear();

		/* Attempt to load */
		err = rd_savefile();

		/* Message (below) */
		if (err) what = "Cannot parse savefile";
	}

	/* Paranoia */
	if (!err)
	{
		/* Invalid turn */
		if (!turn) err = -1;

		/* Message (below) */
		if (err) what = "Broken savefile";
	}


	/* Okay */
	if (!err)
	{
		/* Player is dead */
		if (death)
		{
			/* Player is no longer "dead" */
			death = FALSE;

			/* Cheat death (unless the character retired) */
			if (arg_wizard && !total_winner)
			{
				/* A character was loaded */
				character_loaded = TRUE;

				/* Done */
				return (TRUE);
			}

			/* Count lives */
			sf_lives++;

			/* Forget turns */
			turn = old_turn = 0;

			/* Done */
			return (TRUE);
		}

		/* A character was loaded */
		character_loaded = TRUE;

		/* Still alive */
		if (p_ptr->chp >= 0)
		{
			/* Reset cause of death */
			(void)strcpy(died_from, "(alive and well)");
		}

		/* Success */
		return (TRUE);
	}


	/* Message */
	msg_format("Error (%s) reading %d.%d.%d savefile.",
	           what, sf_major, sf_minor, sf_patch);
	msg_print(NULL);

	/* Oops */
	return (FALSE);
}



/*
 * Medium level player saver
 */
static bool_ save_player_aux(char *name)
{
	bool_ ok = FALSE;
	int fd = -1;
	int mode = 0644;

	/* No file yet */
	fff = NULL;

	/* Create the savefile */
	fd = fd_make(name, mode);

	/* File is okay */
	if (fd >= 0)
	{
		/* Close the "fd" */
		(void)fd_close(fd);

		/* Open the savefile */
		fff = my_fopen(name, "wb");

		/* Successful open */
		if (fff)
		{
			/* Write the savefile */
			if (do_savefile_aux(ls_flag_t::SAVE)) ok = TRUE;

			/* Attempt to close it */
			if (my_fclose(fff)) ok = FALSE;
		}

		/* "broken" savefile */
		if (!ok)
		{
			/* Remove "broken" files */
			(void)fd_kill(name);
		}
	}

	/* Failure */
	if (!ok) return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Attempt to save the player in a savefile
 */
bool_ save_player(void)
{
	int result = FALSE;
	char safe[1024];

	/* New savefile */
	strcpy(safe, savefile);
	strcat(safe, ".new");

	/* Remove it */
	fd_kill(safe);

	/* Attempt to save the player */
	if (save_player_aux(safe))
	{
		char temp[1024];

		/* Old savefile */
		strcpy(temp, savefile);
		strcat(temp, ".old");

		/* Remove it */
		fd_kill(temp);

		/* Preserve old savefile */
		fd_move(savefile, temp);

		/* Activate new savefile */
		fd_move(safe, savefile);

		/* Remove preserved savefile */
		fd_kill(temp);

		/* Hack -- Pretend the character was loaded */
		character_loaded = TRUE;

		/* Success */
		result = TRUE;
	}

	save_savefile_names();

	/* Return the result */
	return (result);
}
