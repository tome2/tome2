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

#define DESC_MAX 10
#define TRIGGERED_HELP_MAX 11

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
}

/*
 * Driver for the context-sensitive help system
 */
void init_hooks_help()
{
	setup_triggered_help_hooks();
}
