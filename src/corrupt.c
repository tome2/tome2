#include "angband.h"
#include <assert.h>

/**
 * Vampire corruption helpers
 */

static void subrace_add_power(player_race_mod *rmp_ptr, int power)
{
	int i;

	for (i=0; i<4; i++)
	{
		if (rmp_ptr->powers[i] == -1)
		{
			rmp_ptr->powers[i] = power;
			return;
		}
	}
}

static void player_gain_vampire_teeth()
{
	player_race_mod *rmp_ptr = NULL;

	switch_subrace(SUBRACE_SAVE, TRUE);

	rmp_ptr = &race_mod_info[SUBRACE_SAVE];
	subrace_add_power(rmp_ptr, PWR_VAMPIRISM);
	rmp_ptr->flags1 = rmp_ptr->flags1
		| PR1_VAMPIRE
		| PR1_UNDEAD
		| PR1_NO_SUBRACE_CHANGE;
}

static void player_gain_vampire_strength()
{
	player_race_mod *rmp_ptr = &race_mod_info[SUBRACE_SAVE];
	/* Apply the bonuses/penalities */
	rmp_ptr->r_mhp = rmp_ptr->r_mhp + 1;
	rmp_ptr->r_exp = rmp_ptr->r_exp + 100;

	rmp_ptr->r_adj[A_STR] = rmp_ptr->r_adj[A_STR] + 3;
	rmp_ptr->r_adj[A_INT] = rmp_ptr->r_adj[A_INT] + 2;
	rmp_ptr->r_adj[A_WIS] = rmp_ptr->r_adj[A_WIS] - 3;
	rmp_ptr->r_adj[A_DEX] = rmp_ptr->r_adj[A_DEX] - 2;
	rmp_ptr->r_adj[A_CON] = rmp_ptr->r_adj[A_CON] + 1;
	rmp_ptr->r_adj[A_CHR] = rmp_ptr->r_adj[A_CHR] - 4;

	/* be reborn! */
	do_rebirth();
	cmsg_print(TERM_L_DARK, "You feel death slipping inside.");
}

static void player_gain_vampire()
{
	player_race_mod *rmp_ptr = &race_mod_info[SUBRACE_SAVE];

	/* Be a Vampire and be proud of it */
	cptr title = get_subrace_title(SUBRACE_SAVE);
	if (streq(title, " ") || streq(title, "Vampire"))
	{
		title = "Vampire";
		rmp_ptr->place = FALSE;
		set_subrace_title(SUBRACE_SAVE, title);
	}
	else
	{
		char buf[512];
		sprintf(buf, "Vampire %s", title);
		set_subrace_title(SUBRACE_SAVE, buf);
	}

	/* Bonus/and .. not bonus :) */
	rmp_ptr->flags1 = rmp_ptr->flags1 | PR1_HURT_LITE;
	rmp_ptr->oflags2[2] = rmp_ptr->oflags2[2]
		| TR2_RES_POIS
		| TR2_RES_NETHER
		| TR2_RES_COLD
		| TR2_RES_DARK
		| TR2_HOLD_LIFE;
	rmp_ptr->oflags3[2] = rmp_ptr->oflags3[2]
		| TR3_LITE1;
}

/**
 * Corruptions
 */
corruption_type corruptions[CORRUPTIONS_MAX] =
{
	/*
	 * BALROG corruptions
	 */ 

	{ /* 0 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_ORANGE,
		NULL /* no group */,
		"Balrog Aura",
		"A corrupted wall of flames surrounds you.",
		"The wall of corrupted flames abandons you.",
		"  Surrounds you with a fiery aura\n"
		"  But it can burn scrolls when you read them",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},

	{ /* 1 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_ORANGE,
		NULL /* no group */,
		"Balrog Wings",
		"Wings of shadow grow in your back.",
		"The wings in your back fall apart.",
		"  Creates ugly, but working, wings allowing you to fly\n"
		"  But it reduces charisma by 4 and dexterity by 2",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},

	{ /* 2 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_ORANGE,
		NULL /* no group */,
		"Balrog Strength",
		"Your muscles get unnatural strength.",
		"Your muscles get weaker again.",
		"  Provides 3 strength and 1 constitution\n"
		"  But it reduces charisma by 1 and dexterity by 3",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},

	{ /* 3 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_YELLOW,
		NULL /* no group */,
		"Balrog Form",
		"You feel the might of a Balrog inside you.",
		"The presence of the Balrog seems to abandon you.",
		"  Allows you to turn into a Balrog at will\n"
		"  You need Balrog Wings, Balrog Aura and Balrog Strength to activate it",
		{ CORRUPT_BALROG_AURA, CORRUPT_BALROG_WINGS, CORRUPT_BALROG_STRENGTH },
		{ -1 },
		NULL,
		PWR_BALROG,
	},

	/*
	 * DEMON corruptions
	 */
	{ /* 4 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Demon Spirit",
		"Your spirit opens to corrupted thoughts.",
		"Your spirit closes again to the corrupted thoughts.",
		"  Increases your intelligence by 1\n"
		"  But reduce your charisma by 2",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},

	{ /* 5 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Demon Hide",
		"Your skin grows into a thick hide.",
		"Your skin returns to a natural state.",
		"  Increases your armour class by your level\n"
		"  Provides immunity to fire at level 40\n"
		"  But reduces speed by your level / 7",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},

	{ /* 6 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Demon Breath",
		"Your breath becomes mephitic.",
		"Your breath is once again normal.",
		"  Provides fire breath\n"
		"  But gives a small chance to spoil potions when you quaff them",
		{ -1 },
		{ -1 },
		NULL,
		PWR_BR_FIRE,
	},

	{ /* 7 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_L_RED,
		NULL /* no group */,
		"Demon Realm",
		"You feel more attuned to the demon realm.",
		"You lose your attunement to the demon realm.",
		"  Provides access to the demon school skill and the use of demonic equipment\n"
		"  You need Demon Spirit, Demon Hide and Demon Breath to activate it",
		{ CORRUPT_DEMON_SPIRIT, CORRUPT_DEMON_HIDE,  CORRUPT_DEMON_BREATH },
		{ -1 },
		NULL,
		-1,
	},

	/*
	 * Teleportation corruptions
	 */

	{ /* 8 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_GREEN,
		NULL /* no group */,
		"Random teleportation",
		"Space seems to fizzle around you.",
		"Space solidify again around you.",
		"  Randomly teleports you around",
		{ -1 },
		{ CORRUPT_ANTI_TELEPORT, -1 },
		NULL,
		-1,
	},

	{ /* 9 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_GREEN,
		NULL /* no group */,
		"Anti-teleportation",
		"Space continuum freezes around you.",
		"Space continuum can once more be altered around you.",
		"  Prevents all teleportations, be it of you or monsters",
		{ -1 },
		{  CORRUPT_RANDOM_TELEPORT, -1 },
		NULL,
		POWER_COR_SPACE_TIME,
	},

	/*
	 * Troll blood
	 */

	{ /* 10 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_GREEN,
		NULL /* no group */,
		"Troll Blood",
		"Your blood thickens, you sense corruption in it.",
		"Your blood returns to a normal state.",
		"  Troll blood flows in your veins, granting increased regeneration\n"
		"  It also enables you to feel the presence of other troll beings\n"
		"  But it will make your presence more noticeable and aggravating",
		{ -1 },
		{ -1 },
		NULL,
		-1,
	},
	
	/*
	 * The vampire corruption set
	 */

	{ /* 11 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_L_DARK,
		"Vampire",
		"Vampiric Teeth",
		"You grow vampiric teeth!",
		NULL, /* cannot lose */
		"  Your teeth allow you to drain blood to feed yourself\n"
		"  However your stomach now only accepts blood.",
		{ -1 },
		{ -1 },
		player_gain_vampire_teeth,
		-1,
	},

	{ /* 12 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_L_DARK,
		"Vampire",
		"Vampiric Strength",
		"Your body seems more dead than alive.",
		NULL, /* cannot lose */
		"  Your body seems somewhat dead\n"
		"  In this near undead state it has improved strength, constitution and intelligence\n"
		"  But reduced dexterity, wisdom and charisma.",
		{ CORRUPT_VAMPIRE_TEETH, -1 },
		{ -1 },
		player_gain_vampire_strength,
		-1,
	},

	{ /* 13 */
		{ MODULE_TOME, MODULE_THEME, -1 },
		TERM_L_DARK,
		"Vampire",
		"Vampire",
		"You die to be reborn in a Vampire form.",
		NULL, /* cannot lose */
		"  You are a Vampire. As such you resist cold, poison, darkness and nether.\n"
		"  Your life is sustained, but you cannot stand the light of the sun.",
		{ CORRUPT_VAMPIRE_STRENGTH, -1 },
		{ -1 },
		player_gain_vampire,
		-1,
	},

	/*
	 * Activatable corruptions (mutations)
	 */

	{ /* 14 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Ancalagon's Breath",
		"You gain the ability to spit acid.",
		"You lose the ability to spit acid.",
		"  Fires an acid ball.\n"
		"  Damage=level Radius 1+(level/30)\n"
		"  Level=9, Cost=9, Stat=DEX, Difficulty=15",
		{ -1 },
		{ -1 },
		NULL,
		PWR_SPIT_ACID,
	},

	{ /* 15 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Smaug's Breath",
		"You gain the ability to breathe fire.",
		"You lose the ability to breathe fire.",
		"  Fires a fire ball.\n"
		"  Damage=2*level Radius 1+(level/20)\n"
		"  Level=20, Cost=10, Stat=CON, Difficulty=18",
		{ -1 },
		{ -1 },
		NULL,
		PWR_BR_FIRE,
	},

	{ /* 16 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Glaurung's Gaze",
		"Your eyes look mesmerizing...",
		"Your eyes look uninteresting.",
		"  Tries to make a monster your pet.\n"
		"  Power=level\n"
		"  Level=12, Cost=12, Stat=CHR, Difficulty=18",
		{ -1 },
		{ -1 },
		NULL,
		PWR_HYPN_GAZE,
	},

	{ /* 17 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Saruman's Power",
		"You gain the ability to move objects telekinetically.",
		"You lose the ability to move objects telekinetically.",
		"  Move an object in line of sight to you.\n"
		"  Max weight equal to (level) pounds\n"
		"  Level=9, Cost=9, Stat=WIS, Difficulty=14",
		{ -1 },
		{ -1 },
		NULL,
		PWR_TELEKINES,
	},

	{ /* 18 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Teleport",
		"You gain the power of teleportation at will.",
		"You lose the power of teleportation at will.",
		"  Teleports the player at will.\n"
		"  Distance 10+4*level squares\n"
		"  Level=7, Cost=7, Stat=WIS, Difficulty=15",
		{ -1 },
		{ -1 },
		NULL,
		PWR_VTELEPORT,
	},

	{ /* 19 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Glaurung's Spell",
		"You gain the power of Mind Blast.",
		"You lose the power of Mind Blast.",
		"  Fires a mind blasting bolt (psi damage).\n"
		"  Psi Damage (3+(level-1)/5)d3\n"
		"  Level=5, Cost=3, Stat=WIS, Difficulty=15",
		{ -1 },
		{ -1 },
		NULL,
		PWR_MIND_BLST,
	},

	{ /* 20 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Vampiric Drain",
		"You become vampiric.",
		"You are no longer vampiric.",
		"  You can drain life from a foe like a vampire.\n"
		"  Drains (level+1d(level))*(level/10) hitpoints,\n"
		"  heals you and satiates you. Doesn't work on all monsters\n"
		"  Level=4, Cost=5, Stat=CON, Difficulty=9",
		{ -1 },
		{ -1 },
		NULL,
		PWR_VAMPIRISM,
	},

	{ /* 21 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Carcharoth's Nose",
		"You smell a metallic odour.",
		"You no longer smell a metallic odour.",
		"  You can detect nearby precious metal (treasure).\n"
		"  Radius 25\n"
		"  Level=3, Cost=2, Stat=INT, Difficulty=12",
		{ -1 },
		{ -1 },
		NULL,
		PWR_SMELL_MET,
	},

	{ /* 22 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Huan's Nose",
		"You smell filthy monsters.",
		"You no longer smell filthy monsters.",
		"  You can detect nearby monsters.\n"
		"  Radius 25\n"
		"  Level=5, Cost=4, Stat=INT, Difficulty=15",
		{ -1 },
		{ -1 },
		NULL,
		PWR_SMELL_MON,
	},

	{ /* 23 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Blink",
		"You gain the power of minor teleportation.",
		"You lose the power of minor teleportation.",
		"  You can teleport yourself short distances (10 squares).\n"
		"  Level=3, Cost=3, Stat=WIS, Difficulty=12",
		{ -1 },
		{ -1 },
		NULL,
		PWR_BLINK,
	},

	{ /* 24 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Eat Rock",
		"The walls look delicious.",
		"The walls look unappetizing.",
		"  You can consume solid rock with food benefit,\n"
		"  leaving an empty space behind.\n"
		"  Level=8, Cost=12, Stat=CON, Difficulty=18",
		{ -1 },
		{ -1 },
		NULL,
		PWR_EAT_ROCK,
	},

	{ /* 25 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Swap Position",
		"You feel like walking a mile in someone else's shoes.",
		"You feel like staying in your own shoes.",
		"  You can switch locations with another being,\n"
		"  unless it resists teleportation.\n"
		"  Level=15, Cost=12, Stat=DEX, Difficulty=16",
		{ -1 },
		{ -1 },
		NULL,
		PWR_SWAP_POS,
	},

	{ /* 26 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Shriek",
		"Your vocal cords get much tougher.",
		"Your vocal cords get much weaker.",
		"  Fires a sound ball and aggravates monsters.\n"
		"  Damage=level*4, Radius=8, centered on player\n"
		"  Level=4, Cost=4, Stat=CON, Difficulty=6",
		{ -1 },
		{ -1 },
		NULL,
		PWR_SHRIEK,
	},

	{ /* 27 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Illuminate",
		"You can light up rooms with your presence.",
		"You can no longer light up rooms with your presence.",
		"  You can emit bright light that illuminates an area.\n"
		"  Damage=2d(level/2) Radius=(level/10)+1\n"
		"  Level=3, Cost=2, Stat=INT, Difficulty=10",
		{ -1 },
		{ -1 },
		NULL,
		PWR_ILLUMINE,
	},

	{ /* 28 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Detect Curses",
		"You can feel evil magics.",
		"You can no longer feel evil magics.",
		"  You can feel the danger of evil magic.\n"
		"  It detects cursed items in the inventory\n"
		"  Level=7, Cost=14, Stat=WIS, Difficulty=14",
		{ -1 },
		{ -1 },
		NULL,
		PWR_DET_CURSE,
	},

	{ /* 29 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Berserk",
		"You feel a controlled rage.",
		"You no longer feel a controlled rage.",
		"  You can drive yourself into a berserk frenzy.\n"
		"  It grants super-heroism. Duration=10+1d(level)\n"
		"  Level=8, Cost=8, Stat=STR, Difficulty=14",
		{ -1 },
		{ -1 },
		NULL,
		PWR_BERSERK,
	},

	{ /* 30 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Midas touch",
		"You gain the Midas touch.",
		"You lose the Midas touch.",
		"  You can turn ordinary items to gold.\n"
		"  Turns a non-artifact object into 1/3 its value in gold\n"
		"  Level=10, Cost=5, Stat=INT, Difficulty=12",
		{ -1 },
		{ -1 },
		NULL,
		PWR_MIDAS_TCH,
	},

	{ /* 31 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Grow Mold",
		"You feel a sudden affinity for mold.",
		"You feel a sudden dislike for mold.",
		"  You can cause mold to grow near you.\n"
		"  Summons up to 8 molds around the player\n"
		"  Level=1, Cost=6, Stat=CON, Difficulty=14",
		{ -1 },
		{ -1 },
		NULL,
		PWR_GROW_MOLD,
	},

	{ /* 32 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Resist Elements",
		"You feel like you can protect yourself.",
		"You feel like you might be vulnerable.",
		"  You can harden yourself to the ravages of the elements.\n"
		"  Level-dependent chance of gaining resistances to the four \n"
		"  elements and poison. Duration=20 + d20\n"
		"  Level=10, Cost=12, Stat=CON, Difficulty=12",
		{ -1 },
		{ -1 },
		NULL,
		PWR_RESIST,
	},

	{ /* 33 */
		{ MODULE_THEME, -1 },
		TERM_RED,
		NULL /* no group */,
		"Earthquake",
		"You gain the ability to wreck the dungeon.",
		"You lose the ability to wreck the dungeon.",
		"  You can bring down the dungeon around your ears.\n"
		"  Radius=10, center on the player\n"
		"  Level=12, Cost=12, Stat=STR, Difficulty=16",
		{ -1 },
		{ -1 },
		NULL,
		PWR_EARTHQUAKE,
	},

};

/**
 * Initialize corruptions
 */
void init_corruptions()
{
	/* Nothing needed currently */
}

/*
 * Corruptions
 */
bool_ player_has_corruption(int corruption_idx)
{
	if (corruption_idx < 0)
	{
		return FALSE;
	}

	return (p_ptr->corruptions[corruption_idx]);
}

static bool_ player_can_gain_corruption(int corruption_idx)
{
	cptr r_name = rp_ptr->title + rp_name;
	bool_ allowed = TRUE; /* Allowed by default */

	assert(corruption_idx >= 0);

	if (corruption_idx == CORRUPT_TROLL_BLOOD)
	{
		/* Ok trolls should not get this one. never. */
		if (streq(r_name, "Troll"))
		{
			allowed = FALSE;
		}
	}

	/* Theme module adds additional restrictions for Maiar */

	if (game_module_idx == MODULE_THEME)
	{
		if (streq(r_name, "Maia"))
		{
			/* We use a whitelist of corruptions for Maiar */
			bool_ allow = FALSE;
			if ((corruption_idx == CORRUPT_BALROG_AURA) ||
			    (corruption_idx == CORRUPT_BALROG_WINGS) ||
			    (corruption_idx == CORRUPT_BALROG_STRENGTH) ||
			    (corruption_idx == CORRUPT_BALROG_FORM) ||
			    (corruption_idx == CORRUPT_DEMON_BREATH))
			{
				allow = TRUE;
			};

			/* Mix result into 'allowed' flag */
			allowed = allowed & allow;
		}
	}

	/* Result */
	return allowed;
}

static bool_ player_allow_corruption(int corruption_idx)
{
	int i;
	bool_ found = FALSE;
	corruption_type *c_ptr = NULL;

	assert(corruption_idx < CORRUPTIONS_MAX);
	c_ptr = &corruptions[corruption_idx];

	/* Must be allowed by module */
	for (i = 0; c_ptr->modules[i] >= 0; i++)
	{
		if (c_ptr->modules[i] == game_module_idx)
		{
			found = TRUE;
		}
	}
	
	if (!found)
	{
		return FALSE;
	}

	/* Vampire teeth is special */
	if (corruption_idx == CORRUPT_VAMPIRE_TEETH)
	{
		if (PRACE_FLAG(PR1_NO_SUBRACE_CHANGE))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

static void player_set_corruption(int c, bool_ set)
{
	p_ptr->corruptions[c] = set;
	p_ptr->redraw = p_ptr->redraw | PR_BASIC;
	p_ptr->update = p_ptr->update | PU_BONUS | PU_TORCH | PU_BODY | PU_POWERS;

}

void player_gain_corruption(int corruption_idx)
{
	corruption_type *c_ptr = NULL;
	assert(corruption_idx >= 0);
	assert(corruption_idx < CORRUPTIONS_MAX);
	c_ptr = &corruptions[corruption_idx];

	/* Set the player's corruption flag */
	player_set_corruption(corruption_idx, TRUE);

	/* Invoke callback if necessary */
	if (c_ptr->gain_callback)
	{
		c_ptr->gain_callback();
	}
}

static void player_lose_corruption(int corruption_idx)
{
	assert(corruption_idx >= 0);
	assert(corruption_idx < CORRUPTIONS_MAX);

	player_set_corruption(corruption_idx, FALSE);

	/* Currently no corruptions need any special handling when lost */
}

/*
 * Test if we have that corruption
 * We must:
 * 1) have it or be willing to get it
 * 2) have all its dependancies
 * 3) have none of its opposing corruptions
 * 4) pass the possible tests
 */
static bool_ test_depend_corrupt(s16b corrupt_idx, bool_ can_gain)
{
	s16b i;
	corruption_type *c_ptr = NULL;

	assert(corrupt_idx >= 0);
	assert(corrupt_idx < CORRUPTIONS_MAX);

	c_ptr = &corruptions[corrupt_idx];

	if (can_gain)
	{
		if (p_ptr->corruptions[corrupt_idx])
		{
			return FALSE;
		}
	} else {
		if (!p_ptr->corruptions[corrupt_idx])
		{
			return FALSE;
		}
	}

	/* Go through all dependencies */
	for (i=0; c_ptr->depends[i] >= 0; i++)
	{
		if (!test_depend_corrupt(c_ptr->depends[i], FALSE))
		{
			return FALSE;
		}
	}

	/* Go through all opposers */
	for (i=0; c_ptr->opposes[i] >= 0; i++)
	{
		if (test_depend_corrupt(c_ptr->opposes[i], FALSE))
		{
			return FALSE;
		}
	}

	/* are we even allowed to get it? */
	return player_can_gain_corruption(corrupt_idx);
}

void gain_random_corruption()
{
	s16b i, max;
	s16b pos[CORRUPTIONS_MAX];

	/* Get the list of all possible ones */
	max = 0;
	for (i=0; i < CORRUPTIONS_MAX; i++)
	{
		if (test_depend_corrupt(i, TRUE) &&
		    player_allow_corruption(i))
		{
			pos[max] = i;
			max = max + 1;
		}
	}

	/* Ok now get one of them */
	if (max > 0)
	{
		s16b ret = rand_int(max);
		int c_idx = pos[ret];
		assert(c_idx < CORRUPTIONS_MAX);

		player_gain_corruption(c_idx);
		cmsg_print(TERM_L_RED, corruptions[c_idx].get_text);
	}
}

static void remove_corruption(int c_idx)
{
	assert(c_idx >= 0 && c_idx < CORRUPTIONS_MAX);
	assert(corruptions[c_idx].lose_text);

	player_lose_corruption(c_idx);
	cmsg_print(TERM_L_RED, corruptions[c_idx].lose_text);
}

void lose_corruption()
{
	s16b i, max;
	s16b pos[CORRUPTIONS_MAX];

	/* Get the list of all possible ones */
	max = 0;
	for (i = 0; i < CORRUPTIONS_MAX; i++)
	{
		bool_ is_removable = (corruptions[i].lose_text != NULL);
		if (test_depend_corrupt(i, FALSE) && is_removable)
		{
			pos[max] = i;
			max = max + 1;
		}
	}
	
	/* Ok now get one of them */
	if (max > 0)
	{
		s16b ret = rand_int(max);
		int c_idx = pos[ret];

		/* Remove the corruption */
		remove_corruption(c_idx);

		/* Ok now lets see if it broke some dependencies */
		for (i = 0; i < max - 1; i++)
		{
			if (p_ptr->corruptions[pos[i]] != test_depend_corrupt(pos[i], FALSE))
			{
				remove_corruption(pos[i]);
			}
		}
	}
}


/*
 * Dump the corruption list
 */
void dump_corruptions(FILE *fff, bool_ color, bool_ header)
{
	int i;

	assert(fff != NULL);

	for (i = 0; i < CORRUPTIONS_MAX; i++)
	{
		corruption_type *c_ptr = &corruptions[i];

		if (header)
		{
			fprintf(fff, "\n Corruption list:\n");
			header = FALSE;
		}

		if (p_ptr->corruptions[i])
		{
			byte c = c_ptr->color;

			if (color)
			{
				fprintf(fff, "#####%c%s:\n", conv_color[c], c_ptr->name);
			}
			else
			{
				fprintf(fff, "%s:\n", c_ptr->name);
			}

			fprintf(fff, "%s\n", c_ptr->desc);
		}
	}
}

/*
 * Get the power granted by a corruption. Returns -1
 * if the given corruption does not grant a power.
 */
s16b get_corruption_power(int corruption_idx)
{
	corruption_type *c_ptr = NULL;

	assert(corruption_idx >= 0);
	assert(corruption_idx < CORRUPTIONS_MAX);

	c_ptr = &corruptions[corruption_idx];

	if ((c_ptr->power >= 0) && (c_ptr->power < POWER_MAX))
	{
		return c_ptr->power;
	}
	else
	{
		assert(c_ptr->power == -1); /* Sanity check: Should always be the case. */
		return -1;
	}
}
