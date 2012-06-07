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
#define TRIGGERED_HELP_MAX 1

#define HELP_VOID_JUMPGATE 0

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
