#include "angband.h"
#include <assert.h>

static s32b abomination_calc()
{
	apply_flags(TR1_SPEED + TR1_STR + TR1_INT + TR1_WIS + TR1_DEX + TR1_CON + TR1_CHR, 0, 0, 0, 0, 0, -10, 0, 0, 0, 0);
	p_ptr->xtra_f3 |= TR3_AGGRAVATE;

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
		p_ptr->powers[POWER_INVISIBILITY] = TRUE;
	}
}

static s32b eagle_calc()
{
	p_ptr->ffall = TRUE;
	p_ptr->pspeed = p_ptr->pspeed + 2 + (p_ptr->mimic_level / 6);

	p_ptr->stat_add[A_STR] += -3;
	p_ptr->stat_add[A_DEX] +=  2 + (p_ptr->mimic_level / 15);
	p_ptr->stat_add[A_CON] +=  4 + (p_ptr->mimic_level / 20);
	p_ptr->stat_add[A_INT] += -1;
	p_ptr->stat_add[A_WIS] +=  1;
	p_ptr->stat_add[A_CHR] += -1;

	if (p_ptr->mimic_level >= 20)
	{
                p_ptr->xtra_f4 |= TR4_FLY;
                p_ptr->xtra_f3 |= TR3_SEE_INVIS;
	}
	
	if (p_ptr->mimic_level >= 25)
	{
                p_ptr->xtra_f2 |= TR2_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 30)
	{
                p_ptr->xtra_f2 |= TR2_RES_ELEC;
	}

	if (p_ptr->mimic_level >= 35)
	{
                p_ptr->xtra_f3 |= TR3_SH_ELEC;
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

	p_ptr->xtra_f2 |= TR2_FREE_ACT;
	p_ptr->xtra_f2 |= TR2_RES_FEAR;

	if (p_ptr->mimic_level >= 10)
	{
		p_ptr->xtra_f2 |= TR2_RES_COLD;
	}

	if (p_ptr->mimic_level >= 15)
	{
                p_ptr->xtra_f3 |= TR3_SEE_INVIS;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_f2 |= TR2_RES_DARK;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_f2 |= TR2_RES_CONF;
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

	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f2 |= TR2_RES_FEAR;
	p_ptr->xtra_f2 |= TR2_RES_DARK;

	if (p_ptr->mimic_level >= 40)
	{
                p_ptr->xtra_f4 |= TR4_CLIMB;
	}

	return 0;
}

static void spider_power()
{
	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->powers[POWER_WEB] = TRUE;
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

	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f2 |= TR2_RES_COLD;
	p_ptr->xtra_f2 |= TR2_FREE_ACT;
	p_ptr->xtra_f3 |= TR3_REGEN;
	p_ptr->xtra_f3 |= TR3_SEE_INVIS;
	p_ptr->xtra_f2 |= TR2_SENS_FIRE;

	return 0;
}

static void ent_power()
{
	p_ptr->powers[PWR_GROW_TREE] = TRUE;
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
	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f2 |= TR2_RES_SHARDS;
	p_ptr->xtra_f2 |= TR2_IM_COLD;
	p_ptr->xtra_f2 |= TR2_FREE_ACT;
	p_ptr->xtra_f3 |= TR3_REGEN;
	p_ptr->xtra_f3 |= TR3_SEE_INVIS;
	p_ptr->xtra_f2 |= TR2_SENS_FIRE;
	p_ptr->xtra_f3 |= TR3_FEATHER;

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

	p_ptr->xtra_f2 |= TR2_RES_POIS;
	if (p_ptr->mimic_level >= 25)
	{
            	p_ptr->xtra_f2 |= TR2_FREE_ACT;
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
		p_ptr->xtra_f2 |= TR2_RES_FEAR;
	}

	if (p_ptr->mimic_level >= 25)
	{
		p_ptr->xtra_f2 |= TR2_RES_CONF;
	}

	if (p_ptr->mimic_level >= 30)
	{
            	p_ptr->xtra_f2 |= TR2_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_f2 |= TR2_RES_NEXUS;
	}

	return 0;
}

static s32b bear_calc()
{
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
            	p_ptr->xtra_f2 |= TR2_FREE_ACT;
	}

	if (p_ptr->mimic_level >= 20)
	{
		p_ptr->xtra_f3 |= TR3_REGEN;
	}

	if (p_ptr->mimic_level >= 30)
	{
		p_ptr->xtra_f2 |= TR2_RES_CONF;
	}

	if (p_ptr->mimic_level >= 35)
	{
		p_ptr->xtra_f2 |= TR2_RES_NEXUS;
	}

	/* activate the skill */
	s_info[SKILL_BEAR].hidden = FALSE;

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

	p_ptr->xtra_f2 |= TR2_IM_ACID;
	p_ptr->xtra_f2 |= TR2_IM_FIRE;
	p_ptr->xtra_f2 |= TR2_IM_ELEC;
	p_ptr->xtra_f2 |= TR2_RES_DARK;
	p_ptr->xtra_f2 |= TR2_RES_CHAOS;
	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f2 |= TR2_HOLD_LIFE;
	p_ptr->xtra_f3 |= TR3_FEATHER;
	p_ptr->xtra_f3 |= TR3_REGEN;
	p_ptr->xtra_f3 |= TR3_SH_FIRE;
	p_ptr->xtra_f3 |= TR3_LITE1;

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

	p_ptr->xtra_f2 |= TR2_IM_FIRE;
	p_ptr->xtra_f2 |= TR2_IM_ELEC;
	p_ptr->xtra_f2 |= TR2_IM_ACID;
	p_ptr->xtra_f2 |= TR2_IM_COLD;
	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f2 |= TR2_RES_LITE;
	p_ptr->xtra_f2 |= TR2_RES_DARK;
	p_ptr->xtra_f2 |= TR2_RES_CHAOS;
	p_ptr->xtra_f2 |= TR2_HOLD_LIFE;
	p_ptr->xtra_f3 |= TR3_FEATHER;
	p_ptr->xtra_f3 |= TR3_REGEN;

	return 2; /* Add two blows */
}

static s32b fire_elemental_calc()
{
	p_ptr->stat_add[A_STR] += 5 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_DEX] += 5 + (p_ptr->mimic_level / 5);
	p_ptr->stat_add[A_WIS] += -5 - (p_ptr->mimic_level / 5);

	p_ptr->xtra_f2 |= TR2_IM_FIRE;
	p_ptr->xtra_f2 |= TR2_RES_POIS;
	p_ptr->xtra_f3 |= TR3_SH_FIRE;
	p_ptr->xtra_f3 |= TR3_LITE1;

	return 0;
}

/*
 * Mimicry forms
 */
mimic_form_type mimic_forms[MIMIC_FORMS_MAX] =
{
	{ /* 0 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		"Abomination", /* MUST be at index 0! */
		"Abominable Cloak",
		"Abominations are failed experiments of powerful wizards.",
		NULL /* no realm */,
		FALSE,
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
		FALSE,
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
		FALSE,
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
		FALSE,
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
		FALSE,
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
		FALSE,
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
		TRUE,
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
		FALSE,
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
		FALSE,
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
		FALSE,
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
		TRUE,
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
		TRUE,
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
		TRUE,
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
		TRUE,
		1, 101, {10, 10},
		fire_elemental_calc,
		NULL,
	},

};

/*
 * Is the mimicry form enabled for the current module?
 */
static bool_ mimic_form_enabled(mimic_form_type *f)
{
	int i;

	for (i = 0; f->modules[i] >= 0; i++)
	{
		if (f->modules[i] == game_module_idx)
		{
			return TRUE;
		}
	}

	return FALSE;
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
s16b resolve_mimic_name(cptr name)
{
	s16b i;

	for (i = 0; i < MIMIC_FORMS_MAX; i++)
	{
		mimic_form_type *mf_ptr = get_mimic_form(i);
		if (mimic_form_enabled(mf_ptr) && streq(mf_ptr->name, name))
		{
			return i;
		}
	}

	return -1;
}

/*
 * Find a random mimic form
 */
s16b find_random_mimic_shape(byte level, bool_ limit)
{
	int tries = 1000;

	while (tries > 0)
	{
		int mf_idx = 0;
		mimic_form_type *mf_ptr = NULL;

		tries = tries - 1;

		mf_idx = rand_int(MIMIC_FORMS_MAX);
		mf_ptr = get_mimic_form(mf_idx);

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
cptr get_mimic_name(s16b mf_idx)
{
	return get_mimic_form(mf_idx)->name;
}

/*
 * Get mimic object name
 */
cptr get_mimic_object_name(s16b mf_idx)
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
	mimic_form_type *mf_ptr = get_mimic_form(mf_idx);
	return rand_range(mf_ptr->duration.min, mf_ptr->duration.max);
}

/*
 * Calculate bonuses for player's current mimic form
 */
byte calc_mimic()
{
	mimic_form_type *mf_ptr = get_mimic_form(p_ptr->mimic_form);
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
