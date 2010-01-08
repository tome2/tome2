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

#include "lua/lua.h"
#include "tolua.h"
extern lua_State* L;

/* #define DEBUG_HOOK */

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

	for (i = 0; i < MAX_Q_IDX_INIT; i++)
	{
		if ((quest[i].type == HOOK_TYPE_C) && (quest[i].init != NULL)) quest[i].init(i);
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
bool check_hook(int h_idx)
{
	hooks_chain *c = hooks_heads[h_idx];

	return (c != NULL);
}

/* Add a hook */
hooks_chain* add_hook(int h_idx, hook_type hook, cptr name)
{
	hooks_chain *new, *c = hooks_heads[h_idx];

	/* Find it */
	while ((c != NULL) && (strcmp(c->name, name)))
	{
		c = c->next;
	}

	/* If not already in the list, add it */
	if (c == NULL)
	{
		MAKE(new, hooks_chain);
		new->hook = hook;
		sprintf(new->name, name);
#ifdef DEBUG_HOOK
		if (wizard) cmsg_format(TERM_VIOLET, "HOOK ADD: %s", name);
		if (take_notes) add_note(format("HOOK ADD: %s", name), 'D');
#endif
		new->next = hooks_heads[h_idx];
		hooks_heads[h_idx] = new;
		return (new);
	}
	else return (c);
}

void add_hook_script(int h_idx, char *script, cptr name)
{
	hooks_chain *c = add_hook(h_idx, NULL, name);
#ifdef DEBUG_HOOK
	if (wizard) cmsg_format(TERM_VIOLET, "HOOK LUA ADD: %s : %s", name, script);
#endif
	sprintf(c->script, "%s", script);
	c->type = HOOK_TYPE_LUA;
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
#ifdef DEBUG_HOOK
			if (wizard) cmsg_format(TERM_VIOLET, "HOOK DEL: %s", c->name);
			if (take_notes) add_note(format("HOOK DEL: %s", c->name), 'D');
#endif
			hooks_heads[h_idx] = c->next;
			FREE(c, hooks_chain);
		}
		else
		{
#ifdef DEBUG_HOOK
			if (wizard) cmsg_format(TERM_VIOLET, "HOOK DEL: %s", c->name);
			if (take_notes) add_note(format("HOOK DEL: %s", c->name), 'D');
#endif
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
#ifdef DEBUG_HOOK
			if (wizard) cmsg_format(TERM_VIOLET, "HOOK DEL: %s", c->name);
			if (take_notes) add_note(format("HOOK DEL: %s", c->name), 'D');
#endif
			hooks_heads[h_idx] = c->next;
			FREE(c, hooks_chain);
		}
		else
		{
#ifdef DEBUG_HOOK
			if (wizard) cmsg_format(TERM_VIOLET, "HOOK DEL: %s", c->name);
			if (take_notes) add_note(format("HOOK DEL: %s", c->name), 'D');
#endif
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
static bool vprocess_hooks_return (int h_idx, char *ret, char *fmt, va_list *ap)
{
	hooks_chain *c = hooks_heads[h_idx];
	va_list real_ap;

	while (c != NULL)
	{
#ifdef DEBUG_HOOK
		if (wizard) cmsg_format(TERM_VIOLET, "HOOK: %s", c->name);
		if (take_notes) add_note(format("HOOK PROCESS: %s", c->name), 'D');
#endif
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
		else if (c->type == HOOK_TYPE_LUA)
		{
			int i = 0, nb = 0, nbr = 1;
			int oldtop = lua_gettop(L), size;

			/* Push the function */
			lua_getglobal(L, c->script);

			/* Push and count the arguments */
			COPY(&real_ap, ap, va_list);
			while (fmt[i])
			{
				switch (fmt[i++])
				{
				case 'd':
				case 'l':
					tolua_pushnumber(L, va_arg(real_ap, s32b));
					nb++;
					break;
				case 's':
					tolua_pushstring(L, va_arg(real_ap, char*));
					nb++;
					break;
				case 'O':
					tolua_pushusertype(L, (void*)va_arg(real_ap, object_type*), tolua_tag(L, "object_type"));
					nb++;
					break;
				case 'M':
					tolua_pushusertype(L, (void*)va_arg(real_ap, monster_type*), tolua_tag(L, "monster_type"));
					nb++;
					break;
				case '(':
				case ')':
				case ',':
					break;
				}
			}

			/* Count returns */
			nbr += strlen(ret);

			/* Call the function */
			if (lua_call(L, nb, nbr))
			{
				cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling '%s' lua hook script. Breaking the hook chain now.", c->script);
				return FALSE;
			}

			/* Number of returned values, SHOULD be the same as nbr, but I'm paranoid */
			size = lua_gettop(L) - oldtop;

			/* get the extra returns if needed */
			for (i = 0; i < nbr - 1; i++)
			{
				if ((ret[i] == 'd') || (ret[i] == 'l'))
				{
					if (lua_isnumber(L, ( -size) + 1 + i)) process_hooks_return[i].num = tolua_getnumber(L, ( -size) + 1 + i, 0);
					else process_hooks_return[i].num = 0;
				}
				else if (ret[i] == 's')
				{
					if (lua_isstring(L, ( -size) + 1 + i)) process_hooks_return[i].str = tolua_getstring(L, ( -size) + 1 + i, "");
					else process_hooks_return[i].str = NULL;
				}
				else if (ret[i] == 'O')
				{
					if (tolua_istype(L, ( -size) + 1 + i, tolua_tag(L, "object_type"), 0))
						process_hooks_return[i].o_ptr = (object_type*)tolua_getuserdata(L, ( -size) + 1 + i, NULL);
					else
						process_hooks_return[i].o_ptr = NULL;
				}
				else if (ret[i] == 'M')
				{
					if (tolua_istype(L, ( -size) + 1 + i, tolua_tag(L, "monster_type"), 0))
						process_hooks_return[i].m_ptr = (monster_type*)tolua_getuserdata(L, ( -size) + 1 + i, NULL);
					else
						process_hooks_return[i].m_ptr = NULL;
				}
				else process_hooks_return[i].num = 0;
			}

			/* Get the basic return(continue or stop the hook chain) */
			if (tolua_getnumber(L, -size, 0))
			{
				lua_settop(L, oldtop);
				return (TRUE);
			}
			if (process_hooks_restart)
			{
				c = hooks_heads[h_idx];
				process_hooks_restart = FALSE;
			}
			else
				c = c->next;
			lua_settop(L, oldtop);
		}
		else
		{
			msg_format("Unkown hook type %d, name %s", c->type, c->name);
		}
	}

	return FALSE;
}

bool process_hooks_ret(int h_idx, char *ret, char *fmt, ...)
{
	va_list ap;
	bool r;

	va_start(ap, fmt);
	r = vprocess_hooks_return (h_idx, ret, fmt, &ap);
	va_end(ap);
	return (r);
}

bool process_hooks(int h_idx, char *fmt, ...)
{
	va_list ap;
	bool ret;

	va_start(ap, fmt);
	ret = vprocess_hooks_return (h_idx, "", fmt, &ap);
	va_end(ap);
	return (ret);
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
bool quest_null_hook(int q)
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
