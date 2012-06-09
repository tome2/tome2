/* File: help.c */

/* Purpose: ingame help */

/*
 * Copyright (c) 2001 DarkGod
 * Copyright (c) 2012 Bardur Arantsson
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define DESC_MAX 14
#define TRIGGERED_HELP_MAX 19

#define HELP_VOID_JUMPGATE 0
#define HELP_FOUNTAIN      1
#define HELP_FOUND_OBJECT  2
#define HELP_FOUND_ALTAR   3
#define HELP_FOUND_STAIR   4
#define HELP_GET_ESSENCE   5
#define HELP_GET_RUNE      6
#define HELP_GET_ROD       7
#define HELP_GET_ROD_TIP   8
#define HELP_GET_TRAP_KIT  9
#define HELP_GET_DEVICE   10
#define HELP_WILDERNESS   11
#define HELP_GAME_TOME    12
#define HELP_GAME_THEME   13
#define HELP_1ST_LEVEL    14
#define HELP_20TH_LEVEL   15
#define HELP_ID_SPELL_ITM 16
#define HELP_MELEE_SKILLS 17
#define HELP_MON_ASK_HELP 18

/**
 * Game started?
 */
static bool_ game_started = FALSE;

/**
 * Struct for help triggered by a boolean condition
 */
typedef struct triggered_help_type triggered_help_type;
struct triggered_help_type
{
	/* Help item index; see HELP_* constants above */
	int help_index;
	/* Hook type */
	int hook_type;
	/* Trigger function */
	bool_ (*trigger_func)(void *in, void *out);
	/* Description; NULL terminated */
	cptr desc[DESC_MAX];
};

/**
 * Struct for context-sensitive help
 */
typedef struct context_help_type context_help_type;
struct context_help_type
{
	cptr key;       /* Lookup key */
	cptr file_name; /* Name of help file */
	int anchor;     /* Anchor in file */
};

/**
 * Race help files.
 */
context_help_type race_table[] =
{
	/* ToME */
	{ "Beorning",    "r_beorn.txt",   0 },
	{ "DeathMold",   "r_deathm.txt",  0 },
	{ "Dark-Elf",    "r_drkelf.txt",  0 },
	{ "Dunadan",     "r_dunad.txt",   0 },
	{ "Dwarf",       "r_dwarf.txt",   0 },
	{ "Elf",         "r_elf.txt",     0 },
	{ "Ent",         "r_ent.txt",     0 },
	{ "Gnome",       "r_gnome.txt",   0 },
	{ "Half-Elf",    "r_hafelf.txt",  0 },
	{ "Half-Ogre",   "r_hafogr.txt",  0 },
	{ "High-Elf",    "r_hielf.txt",   0 },
	{ "Hobbit",      "r_hobbit.txt",  0 },
	{ "Human",       "r_human.txt",   0 },
	{ "Kobold",      "r_kobold.txt",  0 },
	{ "Maia",        "r_maia.txt",    0 },
	{ "Orc",         "r_orc.txt",     0 },
	{ "Petty-Dwarf", "r_pettyd.txt",  0 },
	{ "RohanKnight", "r_rohank.txt",  0 },
	{ "Thunderlord", "r_thlord.txt",  0 },
	{ "Troll",       "r_troll.txt",   0 },
	{ "Wood-Elf",    "r_wodelf.txt",  0 },
	{ "Yeek",        "r_yeek.txt",    0 },
	/* Theme */
	{ "Dragon",      "r_dragon.txt",  0 },
	{ "Druadan",     "r_druadan.txt", 0 },
	{ "Eagle",       "r_eagle.txt",   0 },
	{ "Easterling",  "r_easterl.txt", 0 },
	{ "Demon",       "r_demon.txt",   0 },
	/* End of list */
	{ NULL,          NULL,            0 },
};

/**
 * Subrace help files.
 */
context_help_type subrace_table[] =
{
	/* ToME */
	{ "Barbarian",  "rm_barb.txt",    0 },
	{ "Classical",  "rm_class.txt",   0 },
	{ "Corrupted",  "rm_corru.txt",   0 },
	{ "Hermit",     "rm_herm.txt",    0 },
	{ "LostSoul",   "rm_lsoul.txt",   0 },
	{ "Skeleton",   "rm_skel.txt",    0 },
	{ "Spectre",    "rm_spec.txt",    0 },
	{ "Vampire",    "rm_vamp.txt",    0 },
	{ "Zombie",     "rm_zomb.txt",    0 },
	/* Theme */
	{ "Red",        "rm_red.txt",     0 },
	{ "Black",      "rm_black.txt",   0 },
	{ "Green",      "rm_green.txt",   0 },
	{ "Blue",       "rm_blue.txt",    0 },
	{ "White",      "rm_white.txt",   0 },
	{ "Ethereal",   "rm_ether.txt",   0 },
	{ "(Narrog)",   "rm_narrog.txt",  0 },
	{ "(Aewrog)",   "rm_aewrog.txt",  0 },
	{ "(Hurog)",    "rm_hurog.txt",   0 },
	{ "(Sarnrog)",  "rm_sarnrog.txt", 0 },
	{ "(Caborrog)", "rm_cabrog.txt",  0 },
	{ "(Draugrog)", "rm_drarog.txt",  0 },
	{ "(Lygrog)",   "rm_lygrog.txt",  0 },
	{ "(Limrog)",   "rm_limrog.txt",  0 },
	{ "(Rawrog)",   "rm_rawrog.txt",  0 },
	{ "(Adanrog)",  "rm_adanrog.txt", 0 },
	/* End of list */
	{ NULL,         NULL,             0 },
};

/**
 * Class help files
 */
context_help_type class_table[] =
{
	/* ToME */
	{ "Alchemist",      "c_alchem.txt",   0 },
	{ "Archer",         "c_archer.txt",   0 },
	{ "Assassin",       "c_assass.txt",   0 },
	{ "Axemaster",      "c_axemas.txt",   0 },
	{ "Bard",           "c_bard.txt",     0 },
	{ "Dark-Priest",    "c_pr_drk.txt",   0 },
	{ "Demonologist",   "c_demono.txt",   0 },
	{ "Druid",          "c_druid.txt",    0 },
	{ "Geomancer",      "c_geoman.txt",   0 },
	{ "Haftedmaster",   "c_hafted.txt",   0 },
	{ "Loremaster",     "c_lorema.txt",   0 },
	{ "Mage",           "c_mage.txt",     0 },
	{ "Mimic",          "c_mimic.txt",    0 },
	{ "Mindcrafter",    "c_mindcr.txt",   0 },
	{ "Monk",           "c_monk.txt",     0 },
	{ "Necromancer",    "c_necro.txt",    0 },
	{ "Paladin",        "c_palad.txt",    0 },
	{ "Polearmmaster",  "c_polear.txt",   0 },
	{ "Possessor",      "c_posses.txt",   0 },
	{ "Priest",         "c_priest.txt",   0 },
	{ "Priest(Eru)",    "c_pr_eru.txt",   0 },
	{ "Priest(Manwe)",  "c_pr_man.txt",   0 },
	{ "Ranger",         "c_ranger.txt",   0 },
	{ "Rogue",          "c_rogue.txt",    0 },
	{ "Runecrafter",    "c_runecr.txt",   0 },
	{ "Sorceror",       "c_sorcer.txt",   0 },
	{ "Summoner",       "c_summon.txt",   0 },
	{ "Swordmaster",    "c_swordm.txt",   0 },
	{ "Symbiant",       "c_symbia.txt",   0 },
	{ "Thaumaturgist",  "c_thaum.txt",    0 },
	{ "Unbeliever",     "c_unbel.txt",    0 },
	{ "Warper",         "c_warper.txt",   0 },
	{ "Warrior",        "c_warrio.txt",   0 },
	/* Theme */
	{ "Ascetic",        "c_ascet.txt",    0 },
	{ "Clairvoyant",    "c_clairv.txt",   0 },
	{ "Mercenary",      "c_mercen.txt",   0 },
	{ "Pacifist",       "c_pacif.txt",    0 },
	{ "Peace-mage",     "c_peacemag.txt", 0 },
	{ "Priest(Mandos)", "c_pr_mand.txt",  0 },
	{ "Priest(Ulmo)",   "c_pr_ulmo.txt",  0 },
	{ "Priest(Varda)",  "c_pr_varda.txt", 0 },
	{ "Sniper",         "c_sniper.txt",   0 },
	{ "Stonewright",    "c_stonewr.txt",  0 },
	{ "Trapper",        "c_trapper.txt",  0 },
	{ "Wainrider",      "c_wainrid.txt",  0 },
	{ "War-mage",       "c_warmage.txt",  0 },
	/* End of list */
	{ NULL,             NULL,             0 },
};

/**
 * God help files
 */
context_help_type god_table[] =
{
	/* ToME */
	{ "Eru Iluvatar",      "g_eru.txt",    0 },
	{ "Manwe Sulimo",      "g_manwe.txt",  0 },
	{ "Tulkas",            "g_tulkas.txt", 0 },
	{ "Melkor Bauglir",    "g_melkor.txt", 0 },
	{ "Yavanna Kementari", "g_yavann.txt", 0 },
	/* Theme */
	{ "Aule the Smith",    "g_aule.txt",   0 },
	{ "Mandos",            "g_mandos.txt", 0 },
	{ "Ulmo",              "g_ulmo.txt",   0 },
	{ "Varda Elentari",    "g_varda.txt",  0 },
	/* End of list */
	{ NULL,                NULL,           0 },
};

/**
 * Skill help files
 */
context_help_type skill_table[] =
{
	{ "Air",                 "skills.txt", 27 },
	{ "Alchemy",             "skills.txt", 49 },
	{ "Antimagic",           "skills.txt", 50 },
	{ "Archery",             "skills.txt",  8 },
	{ "Axe-mastery",         "skills.txt",  5 },
	{ "Backstab",            "skills.txt", 18 },
	{ "Barehand-combat",     "skills.txt", 13 },
	{ "Boomerang-mastery",   "skills.txt", 12 },
	{ "Boulder-throwing",    "skills.txt", 58 },
	{ "Bow-mastery",         "skills.txt", 10 },
	{ "Combat",              "skills.txt",  1 },
	{ "Conveyance",          "skills.txt", 30 },
	{ "Corpse-preservation", "skills.txt", 44 },
	{ "Critical-hits",       "skills.txt",  4 },
	{ "Crossbow-mastery",    "skills.txt", 11 },
	{ "Demonology",          "skills.txt", 52 },
	{ "Disarming",           "skills.txt", 16 },
	{ "Divination",          "skills.txt", 31 },
	{ "Dodging",             "skills.txt", 20 },
	{ "Druidistic",          "skills.txt", 40 },
	{ "Earth",               "skills.txt", 28 },
	{ "Fire",                "skills.txt", 25 },
	{ "Geomancy",            "skills.txt", 60 },
	{ "Hafted-mastery",      "skills.txt",  6 },
	{ "Magic",               "skills.txt", 21 },
	{ "Magic-Device",        "skills.txt", 54 },
	{ "Mana",                "skills.txt", 24 },
	{ "Meta",                "skills.txt", 29 },
	{ "Mimicry",             "skills.txt", 47 },
	{ "Mind",                "skills.txt", 33 },
	{ "Mindcraft",           "skills.txt", 41 },
	{ "Monster-lore",        "skills.txt", 42 },
	{ "Music",               "skills.txt", 59 },
	{ "Nature",              "skills.txt", 34 },
	{ "Necromancy",          "skills.txt", 35 },
	{ "Polearm-mastery",     "skills.txt",  7 },
	{ "Possession",          "skills.txt", 45 },
	{ "Prayer",              "skills.txt", 39 },
	{ "Runecraft",           "skills.txt", 36 },
	{ "Sling-mastery",       "skills.txt",  9 },
	{ "Sneakiness",          "skills.txt", 14 },
	{ "Spell-power",         "skills.txt", 22 },
	{ "Spirituality",        "skills.txt", 38 },
	{ "Sorcery",             "skills.txt", 23 },
	{ "Stealing",            "skills.txt", 19 },
	{ "Stealth",             "skills.txt", 15 },
	{ "Stunning-blows",      "skills.txt", 53 },
	{ "Summoning",           "skills.txt", 43 },
	{ "Sword-mastery",       "skills.txt",  3 },
	{ "Symbiosis",           "skills.txt", 46 },
	{ "Temporal",            "skills.txt", 32 },
	{ "Thaumaturgy",         "skills.txt", 37 },
	{ "Udun",                "skills.txt", 48 },
	{ "Weaponmastery",       "skills.txt",  2 },
	{ "Water",               "skills.txt", 26 },
	{ NULL,                  NULL,          0 },
};

/**
 * Ability help files
 */
context_help_type ability_table[] =
{
	{ "Spread blows",        "ability.txt",  2 },
	{ "Tree walking",        "ability.txt",  3 },
	{ "Perfect casting",     "ability.txt",  4 },
	{ "Extra Max Blow(1)",   "ability.txt",  5 },
	{ "Extra Max Blow(2)",   "ability.txt",  6 },
	{ "Ammo creation",       "ability.txt",  7 },
	{ "Touch of death",      "ability.txt",  8 },
	{ "Artifact Creation",   "ability.txt",  9 },
	{ "Far reaching attack", "ability.txt", 10 },
	{ "Trapping",            "ability.txt", 11 },
	{ "Undead Form",         "ability.txt", 12 },
	{ NULL,                  NULL,           0 },
};

/**
 * Trigger functions
 */
static bool_ trigger_void_jumpgate(void *in, void *out) {
	hook_move_in *p = (hook_move_in *) in;
	return cave[p->y][p->x].feat == FEAT_BETWEEN;
}

static bool_ trigger_fountain(void *in, void *out) {
	hook_move_in *p = (hook_move_in *) in;
	return cave[p->y][p->x].feat == FEAT_FOUNTAIN;
}

static bool_ trigger_found_object(void *in, void *out) {
	hook_move_in *p = (hook_move_in *) in;
	return cave[p->y][p->x].o_idx != 0;
}

static bool_ trigger_found_altar(void *in, void *out) {
	hook_move_in *p = (hook_move_in *) in;
	return ((cave[p->y][p->x].feat >= FEAT_ALTAR_HEAD) &&
		(cave[p->y][p->x].feat <= FEAT_ALTAR_TAIL));
}

static bool_ trigger_found_stairs(void *in, void *out) {
	hook_move_in *p = (hook_move_in *) in;
	return (cave[p->y][p->x].feat == FEAT_MORE);
}

static bool_ trigger_get_essence(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return (g->o_ptr->tval == TV_BATERIE);
}

static bool_ trigger_get_rune(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return ((g->o_ptr->tval == TV_RUNE1) ||
		(g->o_ptr->tval == TV_RUNE2));
}

static bool_ trigger_get_rod(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return (g->o_ptr->tval == TV_ROD_MAIN);
}

static bool_ trigger_get_rod_tip(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return (g->o_ptr->tval == TV_ROD);
}

static bool_ trigger_get_trap_kit(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return (g->o_ptr->tval == TV_TRAPKIT);
}

static bool_ trigger_get_magic_device(void *in, void *out) {
	hook_get_in *g = (hook_get_in *) in;
	return ((g->o_ptr->tval == TV_WAND) ||
		(g->o_ptr->tval == TV_STAFF));
}

static bool_ trigger_end_turn_wilderness(void *in, void *out) {
	return (((p_ptr->wilderness_x != 34) ||
		 (p_ptr->wilderness_y != 21)) &&
		(!p_ptr->astral));
}

static bool_ trigger_game_theme(void *in, void *out) {
	return (game_module_idx == MODULE_THEME);
}

static bool_ trigger_game_tome(void *in, void *out) {
	return (game_module_idx == MODULE_TOME);
}

static bool_ trigger_1st_level(void *in, void *out) {
	return (p_ptr->lev > 1);
}

static bool_ trigger_20th_level(void *in, void *out) {
	return (p_ptr->lev >= 20);
}

static bool_ trigger_identify_spell_item(void *in_, void *out) {
	hook_identify_in *in = (hook_identify_in *) in_;

	if (in->mode == IDENT_FULL)
	{
		u32b f1, f2, f3, f4, f5, esp;
		object_flags(in->o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
		if (f5 & TR5_SPELL_CONTAIN)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static bool_ trigger_melee_skills(void *in, void *out) {
	return (game_started && (get_melee_skills() > 1));
}

static bool_ trigger_always(void *in, void *out) {
	return TRUE;
}

/**
 * Trigger-based help items
 */
static triggered_help_type triggered_help[TRIGGERED_HELP_MAX] =
{
	{ HELP_VOID_JUMPGATE,
	  HOOK_MOVE,
	  trigger_void_jumpgate,
	  { "Void Jumpgates can be entered by pressing the > key. They will transport",
	    "you to another jumpgate, but beware of the cold damage that might kill you.",
	    NULL }
	},
	{ HELP_FOUNTAIN,
	  HOOK_MOVE,
	  trigger_fountain,
	  { "Fountains are always magical. You can quaff from them by pressing H.",
	    "Beware that unlike potions they cannot be identified.",
	    NULL }
	},
	{ HELP_FOUND_OBJECT,
	  HOOK_MOVE,
	  trigger_found_object,
	  { "So you found your first item! Nice, eh? Now when you stumble across",
	    "objects, you can pick them up by pressing g, and if you are wondering",
	    "what they do, press I (then *, then the letter for the item) to get",
	    "some basic information. You may also want to identify them with scrolls,",
	    "staves, rods or spells.",
	    NULL }
	},
	{ HELP_FOUND_ALTAR,
	  HOOK_MOVE,
	  trigger_found_altar,
	  { "Altars are the way to reach the Valar, powers of the world,",
	    "usualy called Gods. You can press O to become a follower.",
	    "Beware that once you follow a god, you are not allowed to change.",
	    "For an exact description of what gods do and want, read the documentation.",
	    NULL }
	},
	{ HELP_FOUND_STAIR,
	  HOOK_MOVE,
	  trigger_found_stairs,
	  { "Ah, this is a stair, or a way into something. Press > to enter it.",
	    "But be ready to fight what lies within, for it might not be too friendly.",
	    NULL }
	},
	{ HELP_GET_ESSENCE,
	  HOOK_GET,
	  trigger_get_essence,
	  { "Ah, an essence! Those magical containers stores energies. They are used",
	    "with the Alchemy skill to create or modify the powers of items.",
	    NULL }
	},
	{ HELP_GET_RUNE,
	  HOOK_GET,
	  trigger_get_rune,
	  { "Ah, a rune! Runes are used with the Runecraft skill to allow you to",
	    "create spells on your own.",
	    NULL
	  }
	},
	{ HELP_GET_ROD,
	  HOOK_GET,
	  trigger_get_rod,
	  { "This is a rod. You will need to attach a rod tip to it before you",
	    "can use it. This main part of the rod may give the rod bonuses",
	    "like quicker charging time, or a larger capacity for charges.",
	    NULL
	  }
	},
	{ HELP_GET_ROD_TIP,
	  HOOK_GET,
	  trigger_get_rod_tip,
	  { "You've found a rod-tip! You will need to attach it to a rod base",
	    "before you can use it. Once it has been attatched (use the 'z' key)",
	    "you cannot unattach it! The rod tip will determine the effect of",
	    "the rod. To use your rod, 'z'ap it once it has been assembled.",
	    NULL
	  }
	},
	{ HELP_GET_TRAP_KIT,
	  HOOK_GET,
	  trigger_get_trap_kit,
	  { "Ooooh, a trapping kit. If you have ability in the trapping skill,",
	    "you can lay this trap (via the 'm' key) to harm unsuspecting foes.",
	    "You'll generally need either some ammo or magic device depending",
	    "on the exact type of trap kit.",
	    NULL
	  }
	},
	{ HELP_GET_DEVICE,
	  HOOK_GET,
	  trigger_get_magic_device,
	  { "You've found a magical device, either a staff or a wand. Each staff",
	    "contains a spell, often from one of the primary magic schools. There",
	    "is a lot of information you can find about this object if you identify",
	    "it and 'I'nspect it. Check the help file on Magic for more about these.",
	    NULL
	  }
	},
	{ HELP_WILDERNESS,
	  HOOK_END_TURN,
	  trigger_end_turn_wilderness,
	  { "Ahh wilderness travel... The overview mode will allow you to travel",
	    "fast, but that comes to the cost of GREATLY increased food consumption.",
	    "So you should bring lots of food and really watch your hunger status.",
	    "To enter the overview mode, press < while in the wilderness.",
	    NULL
	  }
	},
	{ HELP_GAME_TOME,
	  HOOK_END_TURN,
	  trigger_game_tome,
	  { "Welcome to ToME! I am the spirit of knowledge and my task is to help you",
	    "to get used to how to play. I have prepared a #vparchment#y for you to #vread#y.",
	    "Press r, then space then select it. You can also check the documentation",
	    "by pressing ? at (nearly) any time.",
	    "The first place you can explore is Barrow-downs. Go to the west of town",
	    "and you should see a #v>#y there.",
	    "If you miss any of this you can press ctrl+p to see your message log.",
	    "Now I must reveal your task here. You are on a quest to investigate",
	    "the dreadful tower of Dol Guldur in the Mirkwood forest to see what evil",
	    "lurks there, but beware, you are not yet ready.",
	    "If you do not want me to bother you any more with tips, press = then go",
	    "into the ToME options and deactivate the ingame_help option.",
	    "You can see your quest log by pressing ctrl+q. Now go to your destiny!",
	    NULL
	  }
	},
	{ HELP_GAME_THEME,
	  HOOK_END_TURN,
	  trigger_game_theme,
	  { "Welcome to Theme! I am the spirit of knowledge and my task is to help you",
	    "to get used to how to play. I have prepared a #vparchment#y for you to #vread#y.",
	    "Press r, then space then select it. You can also check the documentation",
	    "by pressing ? at (nearly) any time.",
	    "The first place you can explore is Barrow-downs. Go to the west of town",
	    "and you should see a #v>#y there.",
	    "If you miss any of this you can press ctrl+p to see your message log.",
	    "Now I must reveal your task here. You are on a quest to investigate",
	    "the dreadful tower of Dol Guldur in the Mirkwood forest to see what evil",
	    "lurks there, but beware, you are not yet ready.",
	    "If you do not want me to bother you any more with tips, press = then go",
	    "into the ToME options and deactivate the ingame_help option.",
	    "You can see your quest log by pressing ctrl+q. Now go to your destiny!",
	    NULL
	  }
	},
	{ HELP_1ST_LEVEL,
	  HOOK_PLAYER_LEVEL,
	  trigger_1st_level,
	  { "Ok, so you now gained a level, and you have skill points to spend.",
	    "To do so simply press G to learn skills. Reading the documentation",
	    "about skills and abilities is also strongly recommended.",
	    NULL
	  }
	},
	{ HELP_20TH_LEVEL,
	  HOOK_PLAYER_LEVEL,
	  trigger_20th_level,
	  { "I see you are now at least level 20. Nice! If you want to gloat about your",
	    "character you could press 'C' then 'f' to make a character dump and post it to",
	    "http://angband.oook.cz/ where it will end up in the ladder.",
	    NULL
	  }
	},
	{ HELP_ID_SPELL_ITM,
	  HOOK_IDENTIFY,
	  trigger_identify_spell_item,
	  { "Ah, an item that can contain a spell. To use it you must have some levels of",
	    "Magic skill and then you get the option to copy a spell when pressing m.",
	    "Then just select which spell to copy and to which object. Note that doing so",
	    "is permanent; the spell cannot be removed or changed later.",
	    NULL
	  }
	},
	{ HELP_MELEE_SKILLS,
	  HOOK_RECALC_SKILLS,
	  trigger_melee_skills,
	  { "Ah, you now possess more than one melee type. To switch between them press m",
	    "and select the switch melee type option.",
	    NULL
	  }
	},
	{ HELP_MON_ASK_HELP,
	  HOOK_MON_ASK_HELP,
	  trigger_always,
	  { "Somebody is speaking to you it seems. You can talk back with the Y key.",
	    "This can lead to quests. You can also give items to 'monsters' with the y key.",
	    NULL
	  }
	}
};

static bool_ triggered_help_hook(void *data, void *in, void *out)
{
	triggered_help_type *triggered_help = (triggered_help_type *) data;
	/* Not triggered before and trigger now? */
	if ((option_ingame_help) &&
	    (!p_ptr->help.activated[triggered_help->help_index]) &&
	    triggered_help->trigger_func(in,out))
	{
		int i;

		/* Triggered */
		p_ptr->help.activated[triggered_help->help_index] = TRUE;

		/* Show the description */
		for (i = 0; (i < DESC_MAX) && (triggered_help->desc[i] != NULL); i++)
		{
			cmsg_print(TERM_YELLOW, triggered_help->desc[i]);
		}
	}
	/* Don't stop processing */
	return FALSE;
}

static bool_ hook_game_start(void *data, void *in, void *out)
{
	game_started = TRUE;
	return FALSE;
}

static void setup_triggered_help_hook(int i)
{
	static int counter = 0;
	char name[40];
	triggered_help_type *h = &triggered_help[i];

	/* Build name */
	sprintf(name, "help_trigger_%d", counter);
	counter++;

	/* Add the hook */
	add_hook_new(h->hook_type,
		     triggered_help_hook,
		     name,
		     h);
}

static void setup_triggered_help_hooks()
{
	int i;

	for (i = 0; i < TRIGGERED_HELP_MAX; i++)
	{
		setup_triggered_help_hook(i);
	}

	add_hook_new(HOOK_GAME_START,
		     hook_game_start,
		     "help_game_start",
		     NULL);
}

/*
 * Driver for the context-sensitive help system
 */
void init_hooks_help()
{
	setup_triggered_help_hooks();
}

/*
 * Show help file
 */
static void show_context_help(context_help_type *context_help)
{
	assert(context_help != NULL);

	screen_save();

	show_file(context_help->file_name, 0, -context_help->anchor, 0);

	screen_load();
}

/*
 * Find context help
 */
static context_help_type *find_context_help(context_help_type table[], cptr key)
{
	int i;

	for (i = 0; ; i++)
	{
		context_help_type *context_help = &table[i];

		if (context_help->key == NULL)
		{
			return NULL; /* End of list */
		}

		if (streq(key, context_help->key))
		{
			return context_help;
		}
	}
}

/*
 * Racial help
 */
void help_race(cptr race)
{
	show_context_help(find_context_help(race_table, race));
}

void help_subrace(cptr subrace)
{
	show_context_help(find_context_help(subrace_table, subrace));
}

void help_class(cptr klass)
{
	show_context_help(find_context_help(class_table, klass));
}

void help_god(cptr god)
{
	context_help_type *context_help =
		find_context_help(god_table, god);

	if (context_help != NULL)
	{
		show_context_help(context_help);
	}
}

void help_skill(cptr skill)
{
	show_context_help(find_context_help(skill_table, skill));
}

void help_ability(cptr ability)
{
	show_context_help(find_context_help(ability_table, ability));
}
