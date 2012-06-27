/* File: plots.c */

/* Purpose: plots & quests */

/*
 * Copyright (c) 2001 James E. Wilson, Robert A. Koeneke, DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include <assert.h>

#include "messages.h"
#include "quark.h"

/******** Hooks stuff *********/
FILE *hook_file;

#define MAX_ARGS        50

static hooks_chain *hooks_heads[MAX_HOOKS];

/* Wipe hooks and init them with quest hooks */
void wipe_hooks()
{
	int i;

	for (i = 0; i < MAX_HOOKS; i++)
	{
		hooks_heads[i] = NULL;
	}
}
void init_hooks()
{
	int i;

	for (i = 0; i < MAX_Q_IDX; i++)
	{
		if (quest[i].init != NULL)
		{
			quest[i].init(i);
		}
	}
}

void dump_hooks(int h_idx)
{
	int min = 0, max = MAX_HOOKS, i;

	if (h_idx != -1)
	{
		min = h_idx;
		max = h_idx + 1;
	}

	for (i = min; i < max; i++)
	{
		hooks_chain *c = hooks_heads[i];

		/* Find it */
		while (c != NULL)
		{
			msg_format("%s(%s)", c->name, (c->type == HOOK_TYPE_C) ? "C" : "Lua");

			c = c->next;
		}
	}
}

/* Check a hook */
bool_ check_hook(int h_idx)
{
	hooks_chain *c = hooks_heads[h_idx];

	return (c != NULL);
}

/* Add a hook */
hooks_chain* add_hook(int h_idx, hook_type hook, cptr name)
{
	hooks_chain *new_, *c = hooks_heads[h_idx];

	/* Find it */
	while ((c != NULL) && (strcmp(c->name, name)))
	{
		c = c->next;
	}

	/* If not already in the list, add it */
	if (c == NULL)
	{
		MAKE(new_, hooks_chain);
		new_->hook = hook;
		sprintf(new_->name, "%s", name);
		new_->next = hooks_heads[h_idx];
		hooks_heads[h_idx] = new_;
		return (new_);
	}
	else return (c);
}

void add_hook_new(int h_idx, bool_ (*hook_f)(void *, void *, void *), cptr name, void *data)
{
	hooks_chain *c = add_hook(h_idx, NULL, name);
	c->hook_f = hook_f;
	c->hook_data = data;
	c->type = HOOK_TYPE_NEW;
}

/* Remove a hook */
void del_hook(int h_idx, hook_type hook)
{
	hooks_chain *c = hooks_heads[h_idx], *p = NULL;

	/* Find it */
	while ((c != NULL) && (c->hook != hook))
	{
		p = c;
		c = c->next;
	}

	/* Remove it */
	if (c != NULL)
	{
		if (p == NULL)
		{
			hooks_heads[h_idx] = c->next;
			FREE(c, hooks_chain);
		}
		else
		{
			p->next = c->next;
			FREE(c, hooks_chain);
		}
	}
}

void del_hook_name(int h_idx, cptr name)
{
	hooks_chain *c = hooks_heads[h_idx], *p = NULL;

	/* Find it */
	while ((c != NULL) && (strcmp(c->name, name)))
	{
		p = c;
		c = c->next;
	}

	/* Remove it */
	if (c != NULL)
	{
		if (p == NULL)
		{
			hooks_heads[h_idx] = c->next;
			FREE(c, hooks_chain);
		}
		else
		{
			p->next = c->next;
			FREE(c, hooks_chain);
		}
	}
}

/* get the next argument */
static hook_return param_pile[MAX_ARGS];
static int get_next_arg_pos = 0;
static int get_next_arg_pile_pos = 0;
s32b get_next_arg(char *fmt)
{
	while (TRUE)
	{
		switch (fmt[get_next_arg_pos++])
		{
		case 'd':
		case 'l':
			return (param_pile[get_next_arg_pile_pos++].num);
		case ')':
			get_next_arg_pos--;
			return 0;
		case '(':
		case ',':
			break;
		}
	}
}
char* get_next_arg_str(char *fmt)
{
	while (TRUE)
	{
		switch (fmt[get_next_arg_pos++])
		{
		case 's':
			return (char*)(param_pile[get_next_arg_pile_pos++].str);
		case ')':
			get_next_arg_pos--;
			return 0;
		case '(':
		case ',':
			break;
		}
	}
}

/* Actually process the hooks */
int process_hooks_restart = FALSE;
hook_return process_hooks_return[20];
static bool_ vprocess_hooks_return (int h_idx, char *ret, char *fmt, va_list *ap)
{
	hooks_chain *c = hooks_heads[h_idx];
	va_list real_ap;

	while (c != NULL)
	{
		if (c->type == HOOK_TYPE_C)
		{
			int i = 0, nb = 0;

			/* Push all args in the pile */
			i = 0;
			COPY(&real_ap, ap, va_list);
			while (fmt[i])
			{
				switch (fmt[i])
				{
				case 'O':
					param_pile[nb++].o_ptr = va_arg(real_ap, object_type *);
					break;
				case 's':
					param_pile[nb++].str = va_arg(real_ap, char *);
					break;
				case 'd':
				case 'l':
					param_pile[nb++].num = va_arg(real_ap, s32b);
					break;
				case '(':
				case ')':
				case ',':
					break;
				}
				i++;
			}

			get_next_arg_pos = 0;
			get_next_arg_pile_pos = 0;
			if (c->hook(fmt))
			{
				return TRUE;
			}

			/* Should we restart ? */
			if (process_hooks_restart)
			{
				c = hooks_heads[h_idx];
				process_hooks_restart = FALSE;
			}
			else
			{
				c = c->next;
			}
		}
		else if (c->type == HOOK_TYPE_NEW)
		{
			/* Skip; handled in process_hooks_new */
			c = c->next;
		}
		else
		{
			msg_format("Unkown hook type %d, name %s", c->type, c->name);
			c = c->next;
		}
	}

	return FALSE;
}

bool_ process_hooks_ret(int h_idx, char *ret, char *fmt, ...)
{
	va_list ap;
	bool_ r;

	va_start(ap, fmt);
	r = vprocess_hooks_return (h_idx, ret, fmt, &ap);
	va_end(ap);
	return (r);
}

bool_ process_hooks(int h_idx, char *fmt, ...)
{
	va_list ap;
	bool_ ret;

	va_start(ap, fmt);
	ret = vprocess_hooks_return (h_idx, "", fmt, &ap);
	va_end(ap);
	return (ret);
}

bool_ process_hooks_new(int h_idx, void *in, void *out)
{
	hooks_chain *c = hooks_heads[h_idx];

	while (c != NULL)
	{
		/* Only new-style hooks; skip the rest. */
		if (c->type != HOOK_TYPE_NEW)
		{
			c = c->next;
			continue;
		}

		/* Invoke hook function; stop processing if
		   the hook returns TRUE */
		if (c->hook_f(c->hook_data, in, out))
		{
			return TRUE;
		}

		/* Should we restart processing at the beginning? */
		if (process_hooks_restart)
		{
			c = hooks_heads[h_idx];
			process_hooks_restart = FALSE;
		}
		else
		{
			c = c->next;
		}
	}

	return FALSE;
}

/******** Plots & Quest stuff ********/

static void quest_describe(int q_idx)
{
	int i = 0;

	while ((i < 10) && (quest[q_idx].desc[i][0] != '\0'))
	{
		cmsg_print(TERM_YELLOW, quest[q_idx].desc[i++]);
	}
}

/* Catch-all quest hook */
bool_ quest_null_hook(int q)
{
	/* Do nothing */
	return (FALSE);
}

/************************** Random Quests *************************/
#include "q_rand.c"

/**************************** Main plot ***************************/
#include "q_main.c"
#include "q_one.c"
#include "q_ultrag.c"
#include "q_ultrae.c"

/**************************** Bree plot ***************************/
#include "q_thief.c"
#include "q_hobbit.c"
#include "q_troll.c"
#include "q_wight.c"
#include "q_nazgul.c"
#include "q_shroom.c"

/*************************** Lorien plot **************************/
#include "q_wolves.c"
#include "q_spider.c"
#include "q_poison.c"

/************************** Gondolin plot *************************/
#include "q_dragons.c"
#include "q_eol.c"
#include "q_nirna.c"
#include "q_invas.c"

/************************* Minas Anor plot ************************/
#include "q_haunted.c"
#include "q_betwen.c"

/************************* Khazad-dum plot ************************/
#include "q_evil.c"

/*************************** Other plot ***************************/
#include "q_narsil.c"
#include "q_thrain.c"

/*************************** Bounty Quest *************************/
#include "q_bounty.c"

/************************** Library Quest *************************/
#include "q_library.c"

/************************* Fireproofing Quest *********************/
#include "q_fireprof.c"
