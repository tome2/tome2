#include "mimic.hpp"

#include "game.hpp"
#include "object_flag.hpp"
#include "player_type.hpp"
#include "skill_type.hpp"
#include "stats.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

using boost::algorithm::equals;

/**
 * Mimicry forms
 */
typedef struct mimic_duration_type mimic_duration_type;
struct mimic_duration_type
{
	s16b min;
	s16b max;
};

typedef struct mimic_form_type mimic_form_type;
struct mimic_form_type
{
	int modules[3]; /* Modules where this mimicry form is available; terminated with a -1 entry */
	const char *name;     /* Name of mimicry form */
	const char *obj_name; /* Object mimicry form name */
	const char *desc;     /* Description */
	const char *realm;    /* Realm of mimicry */
	bool limit;   /* If true, the form is not available except through special means */
	byte level;
	byte rarity;
	mimic_duration_type duration;
	s32b (*calc)();  /* Callback to calculate bonuses; return number of blows to add */
	void (*power)(); /* Callback to calculate powers */
};

static s32b abomination_calc()
{
	apply_flags(TR_SPEED | TR_STR | TR_INT | TR_WIS | TR_DEX | TR_CON | TR_CHR, -10, 0, 0, 0, 0);
	p_ptr->xtra_flags |= TR_AGGRAVATE;

	return 0;
}

static s32b mouse_calc()
{
	/* Mice run! */
	p_ptr->pspeed += 5 + (p_ptr->mimic_level / 7);

	/* They can crawl under your armor to hit you ;) */
	p_ptr->to_h = p_ptr->to_h + 10 + (p_ptr->mimic_level / 5);
	p_ptr->dis_to_h = p_ptr->dis_to_h + 10 + (p_ptr->mimic_level / 5);

	/* But they are not very powerfull */
	p_ptr->to_d = p_ptr->to_d / 5;
	p_ptr->dis_to_d = p_ptr->dis_to_d / 5;

	/* But they are stealthy */
	p_ptr->skill_stl = p_ptr->skill_stl + 10 + (p_ptr->mimic_level / 5);

	/* Stat mods */
	p_ptr->stat_add[A_STR] += -5;
	p_ptr->stat_add[A_DEX] +=  3;
	p_ptr->stat_add[A_CON] +=  1;

	return 0;
}

static void mouse_power()
{
	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->powers.insert(POWER_INVISIBILITY);
	}
}

static s32b eagle_calc()
{
	p_ptr->ffall = true;
	p_ptr->pspeed = p_ptr->pspeed + 2 + (p_ptr->mimic_level / 6);

	p_ptr->stat_add[A_STR] += -3;
	p_ptr->stat_add[A_DEX] +=  2 + (p_ptr->mimic_level / 15);
	p_ptr->stat_add[A_CON] +=  4 + (p_ptr->mimic_level / 20);
	p_ptr->stat_add[A_INT] += -1;
	p_ptr->stat_add[A_WIS] +=  1;
	p_ptr->stat_add[A_CHR] += -1;

	if (p_ptr->mimic_level >= 20)
	{
		p_ptr->xtra_flags |= TR_FLY;
		p_ptr->xtra_flags |= TR_SEE_INVIS;
	}
	
	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->xtra_flags |= TR_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_flags |= TR_RES_ELEC;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_flags |= TR_SH_ELEC;
	}

	return 0;
}

static s32b wolf_calc()
{
	p_ptr->stat_add[A_STR] +=  2 + (p_ptr->mimic_level / 20);
	p_ptr->stat_add[A_DEX] +=  3 + (p_ptr->mimic_level / 20);
	p_ptr->stat_add[A_INT] += -3;
	p_ptr->stat_add[A_CHR] += -2;

	p_ptr->pspeed = p_ptr->pspeed + 10 + (p_ptr->mimic_level / 5);

	p_ptr->xtra_flags |= TR_FREE_ACT;
	p_ptr->xtra_flags |= TR_RES_FEAR;

	if (p_ptr->mimic_level >= 10)
	{
		p_ptr->xtra_flags |= TR_RES_COLD;
	}

	if (p_ptr->mimic_level >= 15)
	{
		p_ptr->xtra_flags |= TR_SEE_INVIS;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_flags |= TR_RES_DARK;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_flags |= TR_RES_CONF;
	}

	return 0;
}

static s32b spider_calc()
{
	p_ptr->stat_add[A_STR] += -4;
	p_ptr->stat_add[A_DEX] +=  1 + (p_ptr->mimic_level / 8);
	p_ptr->stat_add[A_INT] +=  1 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_WIS] +=  1 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_CON] += -5;
	p_ptr->stat_add[A_CHR] += -10;

	p_ptr->pspeed = p_ptr->pspeed + 5;

	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_RES_FEAR;
	p_ptr->xtra_flags |= TR_RES_DARK;

	if (p_ptr->mimic_level >= 40)
	{
		p_ptr->xtra_flags |= TR_CLIMB;
	}

	return 0;
}

static void spider_power()
{
	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->powers.insert(POWER_WEB);
	}
}

static s32b ent_calc()
{
	p_ptr->pspeed = p_ptr->pspeed - 5 - (p_ptr->mimic_level / 10);

	p_ptr->to_a     = p_ptr->to_a     + 10 + p_ptr->mimic_level;
	p_ptr->dis_to_a = p_ptr->dis_to_a + 10 + p_ptr->mimic_level;

	p_ptr->stat_add[A_STR] += p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_INT] += - (p_ptr->mimic_level / 7);
	p_ptr->stat_add[A_WIS] += - (p_ptr->mimic_level / 7);
	p_ptr->stat_add[A_DEX] += -4;
	p_ptr->stat_add[A_CON] += p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_CHR] += -7;

	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_RES_COLD;
	p_ptr->xtra_flags |= TR_FREE_ACT;
	p_ptr->xtra_flags |= TR_REGEN;
	p_ptr->xtra_flags |= TR_SEE_INVIS;
	p_ptr->xtra_flags |= TR_SENS_FIRE;

	return 0;
}

static void ent_power()
{
	p_ptr->powers.insert(PWR_GROW_TREE);
}

static s32b vapour_calc()
{
	p_ptr->pspeed = p_ptr->pspeed + 5;

	/* Try to hit a cloud! */
	p_ptr->to_a     = p_ptr->to_a     + 40 + p_ptr->mimic_level;
	p_ptr->dis_to_a = p_ptr->dis_to_a + 40 + p_ptr->mimic_level;

	/* Try to hit WITH a cloud! */
	p_ptr->to_h     = p_ptr->to_h     - 40;
	p_ptr->dis_to_h = p_ptr->dis_to_h - 40;

	/* Stat mods */
	p_ptr->stat_add[A_STR] += -4;
	p_ptr->stat_add[A_DEX] += 5;
	p_ptr->stat_add[A_CON] += -4;
	p_ptr->stat_add[A_CHR] += -10;

	/* But they are stealthy */
	p_ptr->skill_stl = p_ptr->skill_stl + 10 + (p_ptr->mimic_level / 5);
	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_RES_SHARDS;
	p_ptr->xtra_flags |= TR_IM_COLD;
	p_ptr->xtra_flags |= TR_FREE_ACT;
	p_ptr->xtra_flags |= TR_REGEN;
	p_ptr->xtra_flags |= TR_SEE_INVIS;
	p_ptr->xtra_flags |= TR_SENS_FIRE;
	p_ptr->xtra_flags |= TR_FEATHER;

	return 0;
}

static s32b serpent_calc()
{
	p_ptr->pspeed = p_ptr->pspeed + 10 + (p_ptr->mimic_level / 6);

	p_ptr->to_a     = p_ptr->to_a     + 3 + (p_ptr->mimic_level / 8);
	p_ptr->dis_to_a = p_ptr->dis_to_a + 3 + (p_ptr->mimic_level / 8);

	p_ptr->stat_add[A_STR] += p_ptr->mimic_level / 8;
	p_ptr->stat_add[A_INT] += -6;
	p_ptr->stat_add[A_WIS] += -6;
	p_ptr->stat_add[A_DEX] += -4;
	p_ptr->stat_add[A_CON] += p_ptr->mimic_level / 7;
	p_ptr->stat_add[A_CHR] += -6;

	p_ptr->xtra_flags |= TR_RES_POIS;
	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->xtra_flags |= TR_FREE_ACT;
	}

	return 0;
}

static s32b mumak_calc()
{
	p_ptr->pspeed = p_ptr->pspeed - 5 - (p_ptr->mimic_level / 10);

	p_ptr->to_a     = p_ptr->to_a     + 10 + (p_ptr->mimic_level / 6);
	p_ptr->dis_to_a = p_ptr->dis_to_a + 10 + (p_ptr->mimic_level / 6);
	p_ptr->to_d     = p_ptr->to_d     + 5 + ((p_ptr->mimic_level * 2) / 3);
	p_ptr->dis_to_d = p_ptr->dis_to_d + 5 + ((p_ptr->mimic_level * 2) / 3);

	p_ptr->stat_add[A_STR] += p_ptr->mimic_level / 4;
	p_ptr->stat_add[A_INT] += -8;
	p_ptr->stat_add[A_WIS] += -4;
	p_ptr->stat_add[A_DEX] += -5;
	p_ptr->stat_add[A_CON] += p_ptr->mimic_level / 3;
	p_ptr->stat_add[A_CHR] += -10;

	if (p_ptr->mimic_level >= 10)
	{
		p_ptr->xtra_flags |= TR_RES_FEAR;
	}

	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->xtra_flags |= TR_RES_CONF;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_flags |= TR_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_flags |= TR_RES_NEXUS;
	}

	return 0;
}

static s32b bear_calc()
{
	auto &s_info = game->s_info;

	p_ptr->pspeed = p_ptr->pspeed - 5 + (p_ptr->mimic_level / 5);

	p_ptr->to_a     = p_ptr->to_a     + 5 + ((p_ptr->mimic_level * 2) / 3);
	p_ptr->dis_to_a = p_ptr->dis_to_a + 5 + ((p_ptr->mimic_level * 2) / 3);

	p_ptr->stat_add[A_STR] += p_ptr->mimic_level / 11;
	p_ptr->stat_add[A_INT] += p_ptr->mimic_level / 11;
	p_ptr->stat_add[A_WIS] += p_ptr->mimic_level / 11;
	p_ptr->stat_add[A_DEX] += -1;
	p_ptr->stat_add[A_CON] += p_ptr->mimic_level / 11;
	p_ptr->stat_add[A_CHR] += -10;

	if (p_ptr->mimic_level >= 10)
	{
		p_ptr->xtra_flags |= TR_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 20)
	{
		p_ptr->xtra_flags |= TR_REGEN;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_flags |= TR_RES_CONF;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_flags |= TR_RES_NEXUS;
	}

	/* activate the skill */
	s_info[SKILL_BEAR].hidden = false;

	return 0;
}

static s32b balrog_calc()
{
	p_ptr->stat_add[A_STR] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_INT] += p_ptr->mimic_level / 10;
	p_ptr->stat_add[A_WIS] += - ( 5 + p_ptr->mimic_level / 10);
	p_ptr->stat_add[A_DEX] += p_ptr->mimic_level / 10;
	p_ptr->stat_add[A_CON] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_CHR] += - ( 5 + p_ptr->mimic_level / 10);

	p_ptr->xtra_flags |= TR_IM_ACID;
	p_ptr->xtra_flags |= TR_IM_FIRE;
	p_ptr->xtra_flags |= TR_IM_ELEC;
	p_ptr->xtra_flags |= TR_RES_DARK;
	p_ptr->xtra_flags |= TR_RES_CHAOS;
	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_HOLD_LIFE;
	p_ptr->xtra_flags |= TR_FEATHER;
	p_ptr->xtra_flags |= TR_REGEN;
	p_ptr->xtra_flags |= TR_SH_FIRE;
	p_ptr->xtra_flags |= TR_LITE1;

	return 1; /* Adds a blow */
}

static s32b maia_calc()
{
	p_ptr->stat_add[A_STR] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_INT] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_WIS] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_DEX] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_CON] += 5 + p_ptr->mimic_level / 5;
	p_ptr->stat_add[A_CHR] += 5 + p_ptr->mimic_level / 5;

	p_ptr->xtra_flags |= TR_IM_FIRE;
	p_ptr->xtra_flags |= TR_IM_ELEC;
	p_ptr->xtra_flags |= TR_IM_ACID;
	p_ptr->xtra_flags |= TR_IM_COLD;
	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_RES_LITE;
	p_ptr->xtra_flags |= TR_RES_DARK;
	p_ptr->xtra_flags |= TR_RES_CHAOS;
	p_ptr->xtra_flags |= TR_HOLD_LIFE;
	p_ptr->xtra_flags |= TR_FEATHER;
	p_ptr->xtra_flags |= TR_REGEN;

	return 2; /* Add two blows */
}

static s32b fire_elemental_calc()
{
	p_ptr->stat_add[A_STR] += 5 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_DEX] += 5 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_WIS] += -5 - (p_ptr->mimic_level / 5);

	p_ptr->xtra_flags |= TR_IM_FIRE;
	p_ptr->xtra_flags |= TR_RES_POIS;
	p_ptr->xtra_flags |= TR_SH_FIRE;
	p_ptr->xtra_flags |= TR_LITE1;

	return 0;
}

/*
 * Mimicry forms
 */
static mimic_form_type mimic_forms[MIMIC_FORMS_MAX] =
{
	{ /* 0 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Abomination", /* MUST be at index 0! */
		"Abominable Cloak",
		"Abominations are failed experiments of powerful wizards.",
		NULL /* no realm */,
		false,
		1, 101, {20, 100},
		abomination_calc,
		NULL,
	},

	/*
	 * Nature forms
	 */

	{ /* 1 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Mouse",
		"Mouse Fur",
		"Mice are small, fast and very stealthy",
		"nature",
		false,
		1, 10, {20, 40},
		mouse_calc,
		mouse_power,
	},

	{ /* 2 */
		{ MODULE_TOME, -1 },
		"Eagle",
		"Feathers Cloak",
		"Eagles are master of the air, good hunters with excellent vision.",
		"nature",
		false,
		10, 30, {10, 50},
		eagle_calc,
		NULL,
	},

	{ /* 3 */
		{ MODULE_THEME, -1 },
		"Eagle",
		"Feathered Cloak",
		"Eagles are master of the air, good hunters with excellent vision.",
		"nature",
		false,
		10, 30, {10, 50},
		eagle_calc,
		NULL,
	},

	{ /* 4 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Wolf",
		"Wolf Pelt",
		"Wolves are masters of movement, strong and have excellent eyesight.",
		"nature",
		false,
		20, 40, {10, 50},
		wolf_calc,
		NULL,
	},

	{ /* 5 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Spider",
		"Spider Web",
		"Spiders are clever and become good climbers.",
		"nature",
		false,
		25, 50, {10, 50},
		spider_calc,
		spider_power,
	},

	{ /* 6 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Elder Ent",
		"Entish Bark",
		"Ents are powerful tree-like beings dating from the dawn of time.",
		"nature",
		true,
		40, 60, {10, 30},
		ent_calc,
		ent_power,
	},

	{ /* 7 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Vapour",
		"Cloak of Mist",
		"A sentient cloud, darting around",
		"nature",
		false,
		15, 10, {10, 40},
		vapour_calc,
		NULL,
	},

	{ /* 8 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Serpent",
		"Snakeskin Cloak",
		"Serpents are fast, lethal predators.",
		"nature",
		false,
		30, 25, {15, 20},
		serpent_calc,
		NULL,
	},

	{ /* 9 */ 
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Mumak",
		"Mumak Hide",
		"A giant, elaphantine form.",
		"nature",
		false,
		40, 40, {15, 20},
		mumak_calc,
		NULL,
	},

	/*
	 * Extra shapes
	 */

	{ /* 10 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Bear",
		NULL,
		"A fierce, terrible bear.",
		NULL /* no realm */,
		true,
		1, 101, {50, 200},
		bear_calc,
		NULL,
	},

	{ /* 11 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Balrog",
		NULL,
		"A corrupted maia.",
		NULL /* no realm */,
		true,
		1, 101, {30, 70},
		balrog_calc,
		NULL,
	},

	{ /* 12 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Maia",
		NULL,
		"A near god-like being.",
		NULL /* no realm */,
		true,
		1, 101, {30, 70},
		maia_calc,
		NULL,
	},

	{ /* 13 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Fire Elem.",
		NULL,
		"A towering column of flames",
		NULL /* no realm */,
		true,
		1, 101, {10, 10},
		fire_elemental_calc,
		NULL,
	},

};

/*
 * Is the mimicry form enabled for the current module?
 */
static bool mimic_form_enabled(mimic_form_type const *f)
{
	for (int i = 0; f->modules[i] >= 0; i++)
	{
		if (f->modules[i] == game_module_idx)
		{
			return true;
		}
	}

	return false;
}

/*
 * Get a mimic form by index
 */
static mimic_form_type *get_mimic_form(int mf_idx)
{
	assert(mf_idx >= 0);
	assert(mf_idx < MIMIC_FORMS_MAX);
	return &mimic_forms[mf_idx];
}

/*
 * Find a mimic by name
 */
s16b resolve_mimic_name(const char *name)
{
	for (s16b i = 0; i < MIMIC_FORMS_MAX; i++)
	{
		auto const mf_ptr = get_mimic_form(i);
		if (mimic_form_enabled(mf_ptr) && equals(mf_ptr->name, name))
		{
			return i;
		}
	}

	return -1;
}

/*
 * Find a random mimic form
 */
s16b find_random_mimic_shape(byte level, bool limit)
{
	int tries = 1000;

	while (tries > 0)
	{
		tries = tries - 1;

		int mf_idx = rand_int(MIMIC_FORMS_MAX);
		auto const mf_ptr = get_mimic_form(mf_idx);

		if (mimic_form_enabled(mf_ptr))
		{
			if (limit >= mf_ptr->limit)
			{
				if ((rand_int(mf_ptr->level * 3) < level) &&
				    (mf_ptr->rarity < 100) &&
				    (magik(100 - mf_ptr->rarity)))
				{
					return mf_idx;
				}
			}
		}
	}
	/* Abomination */
	return 0;
}

/*
 * Get mimic name
 */
const char *get_mimic_name(s16b mf_idx)
{
	return get_mimic_form(mf_idx)->name;
}

/*
 * Get mimic object name
 */
const char *get_mimic_object_name(s16b mf_idx)
{
	return get_mimic_form(mf_idx)->obj_name;
}

/*
 * Get mimic object level
 */
byte get_mimic_level(s16b mf_idx)
{
	return get_mimic_form(mf_idx)->level;
}

/*
 * Get a random duration for the given mimic form
 */
s32b get_mimic_random_duration(s16b mf_idx)
{
	auto const mf_ptr = get_mimic_form(mf_idx);
	return rand_range(mf_ptr->duration.min, mf_ptr->duration.max);
}

/*
 * Calculate bonuses for player's current mimic form
 */
byte calc_mimic()
{
	auto const mf_ptr = get_mimic_form(p_ptr->mimic_form);
	if (mf_ptr->calc != NULL)
	{
		return mf_ptr->calc();
	}
	else
	{
		return 0;
	}
}

/*
 * Calculate powers for player's current mimic form
 */
void calc_mimic_power()
{
	mimic_form_type *mf_ptr = get_mimic_form(p_ptr->mimic_form);
	if (mf_ptr->power != NULL)
	{
		mf_ptr->power();
	}
}
