/* File: script.c */

/* Purpose: scripting in lua */

/*
 * Copyright (c) 2001 Dark God
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lauxlib.h"
#include "tolua.h"

#ifdef RISCOS
extern char *riscosify_name(const char *path);
#endif


int tolua_monster_open (lua_State *L);
int tolua_player_open (lua_State *L);
int tolua_player_c_open (lua_State *L);
int tolua_util_open (lua_State *L);
int tolua_z_pack_open (lua_State *L);
int tolua_object_open (lua_State *L);
int tolua_spells_open (lua_State *L);
int tolua_quest_open (lua_State *L);
int tolua_dungeon_open (lua_State *L);

/*
 * Lua state
 */
lua_State* L = NULL;

/* ToME Lua error message handler */
static int tome_errormessage(lua_State *L)
{
	char buf[200];
	cptr str = luaL_check_string(L, 1);
	int i = 0, j = 0;

	while (str[i])
	{
		if (str[i] == '#')
		{
			buf[j++] = '$';
		}
		else if (str[i] != '\n')
		{
			buf[j++] = str[i];
		}
		else
		{
			buf[j] = '\0';
			cmsg_format(TERM_VIOLET, "LUA: %s", buf);
			j = 0;
		}
		i++;
	}
	buf[j] = '\0';
	cmsg_format(TERM_VIOLET, "LUA: %s", buf);
	return (0);
}

static struct luaL_reg tome_iolib[] =
{
	{ "_ALERT", tome_errormessage },
};

#define luaL_check_bit(L, n)  ((long)luaL_check_number(L, n))
#define luaL_check_ubit(L, n) ((unsigned long)luaL_check_bit(L, n))

#if 0

/*
 * Nuked because they can confuse some compilers (lcc for example),
 * and because of their obscurity -- pelpel
 */

#define TDYADIC(name, op, t1, t2) \
static int int_ ## name(lua_State* L) { \
lua_pushnumber(L, \
luaL_check_ ## t1 ## bit(L, 1) op luaL_check_ ## t2 ## bit(L, 2)); \
return 1; \
}

#define DYADIC(name, op) \
static int int_ ## name(lua_State* L) { \
lua_pushnumber(L, \
luaL_check_bit(L, 1) op luaL_check_bit(L, 2)); \
return 1; \
}

#define MONADIC(name, op) \
static int int_ ## name(lua_State* L) { \
lua_pushnumber(L, op luaL_check_bit(L, 1)); \
return 1; \
}

#define VARIADIC(name, op) \
static int int_ ## name(lua_State *L) { \
int n = lua_gettop(L), i; \
long w = luaL_check_bit(L, 1); \
for (i = 2; i <= n; i++) \
w op ## = luaL_check_bit(L, i); \
lua_pushnumber(L, w); \
return 1; \
}

#endif


/*
 * Monadic bit negation operation
 * MONADIC(not,     ~)
 */
static int int_not(lua_State* L)
{
	lua_pushnumber(L, ~luaL_check_bit(L, 1));
	return 1;
}


/*
 * Dyadic integer modulus operation
 * DYADIC(mod,      %)
 */
static int int_mod(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) % luaL_check_bit(L, 2));
	return 1;
}


/*
 * Variable length bitwise AND operation
 * VARIADIC(and,    &)
 */
static int int_and(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);

	for (i = 2; i <= n; i++) w &= luaL_check_bit(L, i);
	lua_pushnumber(L, w);

	return 1;
}


/*
 * Variable length bitwise OR operation
 * VARIADIC(or,     |)
 */
static int int_or(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);

	for (i = 2; i <= n; i++) w |= luaL_check_bit(L, i);
	lua_pushnumber(L, w);

	return 1;
}


/*
 * Variable length bitwise XOR operation
 * VARIADIC(xor,    ^)
 */
static int int_xor(lua_State *L)
{
	int n = lua_gettop(L), i;
	long w = luaL_check_bit(L, 1);

	for (i = 2; i <= n; i++) w ^= luaL_check_bit(L, i);
	lua_pushnumber(L, w);

	return 1;
}


/*
 * Binary left shift operation
 * TDYADIC(lshift,  <<, , u)
 */
static int int_lshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) << luaL_check_ubit(L, 2));
	return 1;
}

/*
 * Binary logical right shift operation
 * TDYADIC(rshift,  >>, u, u)
 */
static int int_rshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_ubit(L, 1) >> luaL_check_ubit(L, 2));
	return 1;
}

/*
 * Binary arithmetic right shift operation
 * TDYADIC(arshift, >>, , u)
 */
static int int_arshift(lua_State* L)
{
	lua_pushnumber(L, luaL_check_bit(L, 1) >> luaL_check_ubit(L, 2));
	return 1;
}


static const struct luaL_reg bitlib[] =
{
	{"bnot", int_not},
	{"imod", int_mod},   /* "mod" already in Lua math library */
	{"band", int_and},
	{"bor", int_or},
	{"bxor", int_xor},
	{"lshift", int_lshift},
	{"rshift", int_rshift},
	{"arshift", int_arshift},
};

/*
 * Some special wrappers
 */
static int lua_zsock_read(lua_State* L)
{
	if (!tolua_istype(L, 1, tolua_tag(L, "zsock_hooks"), 0) ||
	                !tolua_istype(L, 2, tolua_tag(L, "ip_connection"), 0) ||
	                !tolua_istype(L, 3, LUA_TNUMBER, 0) ||
	                !tolua_istype(L, 4, LUA_TNUMBER, 0) ||
	                !tolua_isnoobj(L, 5)
	   )
	{
		tolua_error(L, "#ferror in function 'read'.");
		return 0;
	}
	else
	{
		zsock_hooks* self = (zsock_hooks*) tolua_getusertype(L, 1, 0);
		ip_connection* conn = ((ip_connection*) tolua_getusertype(L, 2, 0));
		int len = ((int) tolua_getnumber(L, 3, 0));
		int start_len = len;
		bool raw = ((bool) tolua_getnumber(L, 4, 0));
		char *str;

		if (!self) tolua_error(L, "invalid 'self' in function 'read'");
		{
			bool toluaI_ret;

			C_MAKE(str, start_len + 1, char);

			toluaI_ret = (bool)self->read(conn, str, &len, raw);
			tolua_pushnumber(L, (long)toluaI_ret);
			tolua_pushstring(L, str);
			tolua_pushnumber(L, (long)len);

			C_FREE(str, start_len + 1, char);

			return 3;
		}
		return 0;
	}
}

/*
 * Initialize lua scripting
 */
static bool init_lua_done = FALSE;
void init_lua()
{
	/* Hack -- Do not initialize more than once */
	if (init_lua_done) return;
	init_lua_done = TRUE;

	/* Start the interpreter with default stack size */
	L = lua_open(0);

	/* Register the Lua base libraries */
	lua_baselibopen(L);
	lua_strlibopen(L);
	lua_iolibopen(L);
	lua_dblibopen(L);

	/* Register tome lua debug library */
	luaL_openl(L, tome_iolib);

	/* Register the bitlib */
	luaL_openl(L, bitlib);

	/* Register the ToME main APIs */
	tolua_player_open(L);
	tolua_player_c_open(L);
	tolua_util_open(L);
	tolua_z_pack_open(L);
	tolua_object_open(L);
	tolua_monster_open(L);
	tolua_spells_open(L);
	tolua_quest_open(L);
	tolua_dungeon_open(L);

	/* Register some special wrappers */
	tolua_function(L, "zsock_hooks", "read", lua_zsock_read);
}

void init_lua_init()
{
	int i, max;

	/* Load the first lua file */
	tome_dofile_anywhere(ANGBAND_DIR_CORE, "init.lua", TRUE);

	/* Finish up schools */
	max = exec_lua("return __schools_num");
	init_schools(max);
	for (i = 0; i < max; i++)
	{
		exec_lua(format("finish_school(%d)", i));
	}

	/* Finish up the spells */
	max = exec_lua("return __tmp_spells_num");
	init_spells(max);
	for (i = 0; i < max; i++)
	{
		exec_lua(format("finish_spell(%d)", i));
	}

	/* Finish up the corruptions */
	max = exec_lua("return __corruptions_max");
	init_corruptions(max);
}

bool tome_dofile(char *file)
{
	char buf[1024];
	int oldtop = lua_gettop(L);

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_SCPT, file);

	if (!file_exist(buf))
	{
		/* No lua source(.lua), maybe a compiled one(.luo) ? */
		if (suffix(buf, ".lua"))
		{
			int len = strlen(buf);
			buf[len - 1] = 'o';
			if (!file_exist(buf))
			{
				cmsg_format(TERM_VIOLET,
				            "tome_dofile(): file %s(%s) doesn't exist.", file, buf);
				return (FALSE);
			}
		}
	}

#ifdef RISCOS
	{
		char *realname = riscosify_name(buf);
		lua_dofile(L, realname);
	}
#else /* RISCOS */
	lua_dofile(L, buf);
#endif /* RISCOS */

	lua_settop(L, oldtop);

	return (TRUE);
}

bool tome_dofile_anywhere(cptr dir, char *file, bool test_exist)
{
	char buf[1024];
	int oldtop = lua_gettop(L);

	/* Build the filename */
	path_build(buf, sizeof(buf), dir, file);

	if (!file_exist(buf))
	{
		/* No lua source(.lua), maybe a compiled one(.luo) ? */
		if (suffix(buf, ".lua"))
		{
			int len = strlen(buf);
			buf[len - 1] = 'o';
			if (!file_exist(buf))
			{
				if (test_exist)
					cmsg_format(TERM_VIOLET,
					            "tome_dofile_anywhere(): file %s(%s) doesn't exist in %s.", dir, file, buf);
				return (FALSE);
			}
		}
	}

#ifdef RISCOS
	{
		char *realname = riscosify_name(buf);
		lua_dofile(L, realname);
	}
#else /* RISCOS */
	lua_dofile(L, buf);
#endif /* RISCOS */

	lua_settop(L, oldtop);

	return (TRUE);
}

int exec_lua(char *file)
{
	int oldtop = lua_gettop(L);
	int res;

	if (!lua_dostring(L, file))
	{
		int size = lua_gettop(L) - oldtop;
		res = tolua_getnumber(L, -size, 0);
	}
	else
		res = 0;

	lua_settop(L, oldtop);
	return (res);
}

cptr string_exec_lua(char *file)
{
	int oldtop = lua_gettop(L);
	cptr res;

	if (!lua_dostring(L, file))
	{
		int size = lua_gettop(L) - oldtop;
		res = tolua_getstring(L, -size, "");
	}
	else
		res = "";
	lua_settop(L, oldtop);
	return (res);
}

void dump_lua_stack(int min, int max)
{
	int i;

	cmsg_print(TERM_YELLOW, "lua_stack:");
	for (i = min; i <= max; i++)
	{
		if (lua_isnumber(L, i)) cmsg_format(TERM_YELLOW, "%d [n] = %d", i, tolua_getnumber(L, i, 0));
		else if (lua_isstring(L, i)) cmsg_format(TERM_YELLOW, "%d [s] = '%s'", i, tolua_getstring(L, i, 0));
	}
	cmsg_print(TERM_YELLOW, "END lua_stack");
}

bool call_lua(cptr function, cptr args, cptr ret, ...)
{
	int i = 0, nb = 0, nbr = 0;
	int oldtop = lua_gettop(L), size;
	va_list ap;

	va_start(ap, ret);

	/* Push the function */
	lua_getglobal(L, function);

	/* Push and count the arguments */
	while (args[i])
	{
		switch (args[i++])
		{
		case 'd':
		case 'l':
			tolua_pushnumber(L, va_arg(ap, s32b));
			nb++;
			break;
		case 's':
			tolua_pushstring(L, va_arg(ap, char*));
			nb++;
			break;
		case 'O':
			tolua_pushusertype(L, (void*)va_arg(ap, object_type*), tolua_tag(L, "object_type"));
			nb++;
			break;
		case 'M':
			tolua_pushusertype(L, (void*)va_arg(ap, monster_type*), tolua_tag(L, "monster_type"));
			nb++;
			break;
		case 'n':
			lua_pushnil(L);
			nb++;
			break;
		case '(':
		case ')':
		case ',':
			break;
		}
	}

	/* Count returns */
	nbr = strlen(ret);

	/* Call the function */
	if (lua_call(L, nb, nbr))
	{
		cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling '%s' from call_lua. Things should start breaking up from now on!", function);
		return FALSE;
	}

	/* Number of returned values, SHOULD be the same as nbr, but I'm paranoid */
	size = lua_gettop(L) - oldtop;

	/* Get the returns */
	for (i = 0; ret[i]; i++)
	{
		switch (ret[i])
		{
		case 'd':
		case 'l':
			{
				s32b *tmp = va_arg(ap, s32b*);

				if (lua_isnumber(L, ( -size) + i)) *tmp = tolua_getnumber(L, ( -size) + i, 0);
				else *tmp = 0;
				break;
			}

		case 's':
			{
				cptr *tmp = va_arg(ap, cptr*);

				if (lua_isstring(L, ( -size) + i)) *tmp = tolua_getstring(L, ( -size) + i, "");
				else *tmp = NULL;
				break;
			}

		case 'O':
			{
				object_type **tmp = va_arg(ap, object_type**);

				if (tolua_istype(L, ( -size) + i, tolua_tag(L, "object_type"), 0))
					*tmp = (object_type*)tolua_getuserdata(L, ( -size) + i, NULL);
				else
					*tmp = NULL;
				break;
			}

		case 'M':
			{
				monster_type **tmp = va_arg(ap, monster_type**);

				if (tolua_istype(L, ( -size) + i, tolua_tag(L, "monster_type"), 0))
					*tmp = (monster_type*)tolua_getuserdata(L, ( -size) + i, NULL);
				else
					*tmp = NULL;
				break;
			}

		default:
			cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling '%s' from call_lua: Unknown return type '%c'", function, ret[i]);
			return FALSE;
		}
	}

	lua_settop(L, oldtop);

	va_end(ap);

	return TRUE;
}

bool get_lua_var(cptr name, char type, void *arg)
{
	int oldtop = lua_gettop(L), size;

	/* Push the function */
	lua_getglobal(L, name);

	size = lua_gettop(L) - oldtop;

	switch (type)
	{
	case 'd':
	case 'l':
		{
			s32b *tmp = (s32b*)arg;

			if (lua_isnumber(L, ( -size))) *tmp = tolua_getnumber(L, ( -size), 0);
			else *tmp = 0;
			break;
		}

	case 's':
		{
			cptr *tmp = (cptr*)arg;

			if (lua_isstring(L, ( -size))) *tmp = tolua_getstring(L, ( -size), "");
			else *tmp = NULL;
			break;
		}

	case 'O':
		{
			object_type **tmp = (object_type**)arg;

			if (tolua_istype(L, ( -size), tolua_tag(L, "object_type"), 0))
				*tmp = (object_type*)tolua_getuserdata(L, ( -size), NULL);
			else
				*tmp = NULL;
			break;
		}

	case 'M':
		{
			monster_type **tmp = (monster_type**)arg;

			if (tolua_istype(L, ( -size), tolua_tag(L, "monster_type"), 0))
				*tmp = (monster_type*)tolua_getuserdata(L, ( -size), NULL);
			else
				*tmp = NULL;
			break;
		}

	default:
		cmsg_format(TERM_VIOLET, "ERROR in get_lua_var while calling '%s': Unknown return type '%c'", name, type);
		return FALSE;
	}

	lua_settop(L, oldtop);

	return TRUE;
}
