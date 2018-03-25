/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "files.hpp"
#include "files.h"

#include "cave.hpp"
#include "cave_type.hpp"
#include "corrupt.hpp"
#include "cmd3.hpp"
#include "dungeon_info_type.hpp"
#include "feature_type.hpp"
#include "game.hpp"
#include "hiscore.hpp"
#include "hook_chardump_in.hpp"
#include "hooks.hpp"
#include "init1.hpp"
#include "levels.hpp"
#include "loadsave.h"
#include "loadsave.hpp"
#include "mimic.hpp"
#include "monoid.hpp"
#include "monster2.hpp"
#include "monster3.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_type.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_flag_meta.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_spec.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "spells2.hpp"
#include "store_info_type.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"
#include "xtra1.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fmt/format.h>
#include <fstream>
#include <limits>
#include <memory>
#include <unordered_set>

/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" and delim arg as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens, char delim1, char delim2)
{
	int i = 0;

	char *s = buf;


	/* Process */
	while (i < num - 1)
	{
		char *t;

		/* Scan the string */
		for (t = s; *t; t++)
		{
			/* Found a delimiter */
			if ((*t == delim1) || (*t == delim2)) break;

			/* Handle single quotes */
			if (*t == '\'')
			{
				/* Advance */
				t++;

				/* Handle backslash */
				if (*t == '\\') t++;

				/* Require a character */
				if (!*t) break;

				/* Advance */
				t++;

				/* Hack -- Require a close quote */
				if (*t != '\'') *t = '\'';
			}

			/* Handle back-slash */
			if (*t == '\\') t++;
		}

		/* Nothing left */
		if (!*t) break;

		/* Nuke and advance */
		*t++ = '\0';

		/* Save the token */
		tokens[i++] = s;

		/* Advance */
		s = t;
	}

	/* Save the token */
	tokens[i++] = s;

	/* Number found */
	return (i);
}



/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Parse another file recursively, see below for details
 *   %:<filename>
 *
 * Specify the attr/char values for "monsters" by race index
 *   R:<num>:<a>:<c>
 *
 * Specify the attr/char values for "objects" by kind index
 *   K:<num>:<a>:<c>
 *
 * Specify the attr/char values for "features" by feature index
 *   F:<num>:<a>:<c>
 *
 * Specify the attr/char values for "stores" by store index
 *   B:<num>:<a>:<c>
 *
 * Specify the attr/char values for unaware "objects" by kind tval
 *   U:<tv>:<a>:<c>
 *
 * Specify the attr/char values for inventory "objects" by kind tval
 *   E:<tv>:<a>:<c>
 *
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a normal macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a command macro, given an encoded macro trigger
 *   C:<str>
 *
 * Create a keyset mapping
 *   S:<key>:<key>:<dir>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify squelch settings
 *   Q:<num>:<squelch>
 */
errr process_pref_file_aux(char *buf)
{
	auto &race_mod_info = game->edit_data.race_mod_info;
	auto &st_info = game->edit_data.st_info;
	auto &re_info = game->edit_data.re_info;
	auto &r_info = game->edit_data.r_info;
	auto &f_info = game->edit_data.f_info;
	auto &k_info = game->edit_data.k_info;

	int i, j, n1, n2;

	char *zz[16];


	/* Skip "empty" lines */
	if (!buf[0]) return (0);

	/* Skip "blank" lines */
	if (isspace(buf[0])) return (0);

	/* Skip comments */
	if (buf[0] == '#') return (0);

	/* Require "?:*" format */
	if (buf[1] != ':') return (1);


	/* Process "%:<fname>" */
	if (buf[0] == '%')
	{
		/* Attempt to Process the given file */
		return (process_pref_file(buf + 2));
	}


	/* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
	if (buf[0] == 'R')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			std::size_t i = strtoul(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			if (i >= r_info.size())
			{
				return (1);
			}

			auto r_ptr = &r_info[i];

			if (n1)
			{
				r_ptr->x_attr = n1;
			}
			if (n2)
			{
				r_ptr->x_char = n2;
			}

			return (0);
		}
	}


	/* Process "G:<type>:<num>:<a>/<c>" -- attr/char for overlay graphics */
	if (buf[0] == 'G')
	{
		/* Process "G:M:<num>:<a>/<c>" -- attr/char for ego monsters */
		if (buf[2] == 'M')
		{
			if (tokenize(buf + 4, 3, zz, ':', '/') == 3)
			{
				std::size_t i = strtoul(zz[0], NULL, 0);
				n1 = strtol(zz[1], NULL, 0);
				n2 = strtol(zz[2], NULL, 0);

				if (i >= re_info.size())
				{
					return 1;
				}

				auto re_ptr = &re_info[i];

				if (n1)
				{
					re_ptr->g_attr = n1;
				}
				if (n2)
				{
					re_ptr->g_char = n2;
				}

				return 0;
			}
		}

		/* Process "G:P:<num>:<a>/<c>" -- attr/char for race modifiers */
		if (buf[2] == 'P')
		{
			if (tokenize(buf + 4, 3, zz, ':', '/') == 3)
			{
				player_race_mod *rmp_ptr;
				i = (huge)strtol(zz[0], NULL, 0);
				n1 = strtol(zz[1], NULL, 0);
				n2 = strtol(zz[2], NULL, 0);
				if (i >= static_cast<int>(race_mod_info.size())) return (1);
				rmp_ptr = &race_mod_info[i];
				if (n1) rmp_ptr->g_attr = n1;
				if (n2)
				{
					rmp_ptr->g_char = n2;
				}
				return (0);
			}
		}
	}


	/* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
	else if (buf[0] == 'K')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			std::size_t i = strtoul(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			if (!k_info.count(i))
			{
				return (1);
			}

			auto k_ptr = k_info.at(i);

			if (n1)
			{
				k_ptr->x_attr = n1;
			}
			if (n2)
			{
				k_ptr->x_char = n2;
			}
			return (0);
		}
	}


	/* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
	else if (buf[0] == 'F')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			std::size_t f_idx = strtoul(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			if (f_idx >= f_info.size()) return (1);

			auto f_ptr = &f_info[f_idx];

			if (n1)
			{
				f_ptr->x_attr = n1;
			}
			if (n2)
			{
				f_ptr->x_char = n2;
			}
			return (0);
		}
	}

	/* Process "B:<num>:<a>/<c>" -- attr/char for stores */
	else if (buf[0] == 'B')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			std::size_t i = std::stoul(zz[0], 0, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			if (i >= st_info.size()) return (1);

			auto st_ptr = &st_info[i];
			if (n1) st_ptr->x_attr = n1;
			if (n2) st_ptr->x_char = n2;
			return (0);
		}
	}

	/* Process "S:<num>:<a>/<c>" -- attr/char for special things */
	else if (buf[0] == 'S')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			j = (byte)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			misc_to_attr[j] = n1;
			misc_to_char[j] = n2;
			return (0);
		}
	}

	/* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
	else if (buf[0] == 'U')
	{
		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			j = strtoul(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);

			for (auto &k_entry: k_info)
			{
				auto k_ptr = k_entry.second;
				if (k_ptr->tval == j)
				{
					if (n1)
					{
						k_ptr->d_attr = n1;
					}
					if (n2)
					{
						k_ptr->d_char = n2;
					}
				}
			}
			return (0);
		}
	}


	/* Process "E:<tv>:<a>" -- attribute for inventory objects */
	else if (buf[0] == 'E')
	{
		if (tokenize(buf + 2, 2, zz, ':', '/') == 2)
		{
			j = (byte)strtol(zz[0], NULL, 0) % 128;
			n1 = strtol(zz[1], NULL, 0);
			if (n1) tval_to_attr[j] = n1;
			return (0);
		}
	}


	/* Process "A:<str>" -- save an "action" for later */
	else if (buf[0] == 'A')
	{
		text_to_ascii(macro__buf, buf + 2);
		return (0);
	}

	/* Process "P:<str>" -- normal macro */
	else if (buf[0] == 'P')
	{
		char tmp[1024];
		text_to_ascii(tmp, buf + 2);
		macro_add(tmp, macro__buf);
		return (0);
	}

	/* Process "L:<num>:<trigger>:<descr> -- extended command macro */
	else if (buf[0] == 'L')
	{
		switch (tokenize(buf + 2, 3, zz, ':', 0))
		{
		case 3:
			cli_add(zz[0], zz[1], zz[2]);
			return 0;
		case 2:
			cli_add(zz[0], zz[1], 0);
			return 0;
		default:
			return 1;
		}
	}

	/* Process "C:<str>" -- create keymap */
	else if (buf[0] == 'C')
	{
		int mode;

		char tmp[1024];

		if (tokenize(buf + 2, 2, zz, ':', '/') != 2) return (1);

		mode = strtol(zz[0], NULL, 0);
		if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);

		text_to_ascii(tmp, zz[1]);
		if (!tmp[0] || tmp[1]) return (1);
		i = (byte)(tmp[0]);

		free(keymap_act[mode][i]);

		keymap_act[mode][i] = strdup(macro__buf);

		return (0);
	}


	/* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
	else if (buf[0] == 'V')
	{
		if (tokenize(buf + 2, 5, zz, ':', '/') == 5)
		{
			i = (byte)strtol(zz[0], NULL, 0);
			angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
			angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
			angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
			angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
			return (0);
		}
	}
	/* set macro trigger names and a template */
	/* Process "T:<trigger>:<keycode>:<shift-keycode>" */
	/* Process "T:<template>:<modifier chr>:<modifier name>:..." */
	else if (buf[0] == 'T')
	{
		int len, tok;
		tok = tokenize(buf + 2, 2 + MAX_MACRO_MOD, zz, ':', '/');
		if (tok >= 4)
		{
			int i;
			int num;

			if (macro_template != NULL)
			{
				delete[] macro_template;
				macro_template = NULL;

				for (i = 0; i < max_macrotrigger; i++)
				{
					delete[] macro_trigger_name[i];
					macro_trigger_name[i] = nullptr;
				}
				max_macrotrigger = 0;
			}

			if (*zz[0] == '\0') return 0;  /* clear template */
			num = strlen(zz[1]);
			if (2 + num != tok) return 1;  /* error */

			len = strlen(zz[0]) + 1 + num + 1;
			for (i = 0; i < num; i++)
				len += strlen(zz[2 + i]) + 1;
			macro_template = new char[len];

			strcpy(macro_template, zz[0]);
			macro_modifier_chr =
			        macro_template + strlen(macro_template) + 1;
			strcpy(macro_modifier_chr, zz[1]);
			macro_modifier_name[0] =
			        macro_modifier_chr + strlen(macro_modifier_chr) + 1;
			for (i = 0; i < num; i++)
			{
				strcpy(macro_modifier_name[i], zz[2 + i]);
				macro_modifier_name[i + 1] = macro_modifier_name[i] +
				                             strlen(macro_modifier_name[i]) + 1;
			}
		}
		else if (tok >= 2)
		{
			int m;
			char *t, *s;
			if (max_macrotrigger >= MAX_MACRO_TRIG)
			{
				msg_print("Too many macro triggers!");
				return 1;
			}
			m = max_macrotrigger;
			max_macrotrigger++;

			len = strlen(zz[0]) + 1 + strlen(zz[1]) + 1;
			if (tok == 3)
				len += strlen(zz[2]) + 1;
			macro_trigger_name[m] = new char[len];

			t = macro_trigger_name[m];
			s = zz[0];
			while (*s)
			{
				if ('\\' == *s) s++;
				*t++ = *s++;
			}
			*t = '\0';

			macro_trigger_keycode[0][m] = macro_trigger_name[m] +
			                              strlen(macro_trigger_name[m]) + 1;
			strcpy(macro_trigger_keycode[0][m], zz[1]);
			if (tok == 3)
			{
				macro_trigger_keycode[1][m] = macro_trigger_keycode[0][m] +
				                              strlen(macro_trigger_keycode[0][m]) + 1;
				strcpy(macro_trigger_keycode[1][m], zz[2]);
			}
			else
			{
				macro_trigger_keycode[1][m] = macro_trigger_keycode[0][m];
			}
		}
		return 0;
	}

	/* Process "X:<str>" -- turn option off */
	else if (buf[0] == 'X')
	{
		for (auto const &option: options->standard_options)
		{
			if (option.o_var && streq(option.o_text, buf + 2))
			{
				*option.o_var = FALSE;
				return 0;
			}
		}
	}

	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'Y')
	{
		for (auto const &option: options->standard_options)
		{
			if (option.o_var && streq(option.o_text, buf + 2))
			{
				*option.o_var = TRUE;
				return 0;
			}
		}
	}

	/* Process "W:<win>:<flag>:<value>" -- window flags */
	else if (buf[0] == 'W')
	{
		int win, flag, value;

		if (tokenize(buf + 2, 3, zz, ':', '/') == 3)
		{
			win = strtol(zz[0], NULL, 0);
			flag = strtol(zz[1], NULL, 0);
			value = strtol(zz[2], NULL, 0);

			/* Ignore illegal windows */
			/* Hack -- Ignore the main window */
			if ((win <= 0) || (win >= ANGBAND_TERM_MAX)) return (1);

			/* Ignore illegal flags */
			if ((flag < 0) || (flag >= 32)) return (1);

			/* Require a real flag */
			if (window_flag_desc[flag])
			{
				if (value)
				{
					/* Turn flag on */
					window_flag[win] |= (1L << flag);
				}
				else
				{
					/* Turn flag off */
					window_flag[win] &= ~(1L << flag);
				}
			}

			/* Success */
			return (0);
		}
	}

	/*  Process "Q:<num>:<squelch>" -- item squelch flags */
	else if (buf[0] == 'Q')
	{
		/* This option isn't used anymore */
		return (0);
	}
	/* Failure */
	return (1);
}


/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static cptr process_pref_file_expr(char **sp, char *fp)
{
	cptr v;

	char *b;
	char *s;

	char b1 = '[';
	char b2 = ']';

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace(*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == b1)
	{
		const char *p;
		const char *t;

		/* Skip b1 */
		s++;

		/* First */
		t = process_pref_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) > 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) < 0)) v = "0";
			}
		}

		/* Function: LEQN */
		else if (streq(t, "LEQN"))
		{
			int n = 0;
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				n = atoi(t);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (atoi(t) < n)) v = "0";
			}
		}

		/* Function: GEQN */
		else if (streq(t, "GEQN"))
		{
			int n = 0;
			v = "1";
			if (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				n = atoi(t);
			}
			while (*s && (f != b2))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (atoi(t) > n)) v = "0";
			}
		}

		/* Function SKILL */
		else if (streq(t, "SKILL"))
		{
			static char skill_val[4*sizeof(int) + 1];
			s16b skill = -1;
			v = "0";
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t) skill = find_skill_i(t);
			}
			if (skill > 0)
			{
				sprintf(skill_val, "%d", (int)get_skill(skill));
				v = skill_val;
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != b2))
			{
				t = process_pref_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != b2) v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') * s++ = '\0';
	}

	/* Other */
	else
	{
		/* Accept all printables except spaces and brackets */
		while (isprint(*s) && !strchr(" []", *s)) ++s;

		/* Extract final and Terminate */
		if ((f = *s) != '\0') * s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			/* System */
			if (streq(b + 1, "SYS"))
			{
				v = ANGBAND_SYS;
			}

			/* Race */
			else if (streq(b + 1, "RACE"))
			{
				v = rp_ptr->title.c_str(); // The string SHOULD be stable enough for this
			}

			/* Race */
			else if (streq(b + 1, "RACEMOD"))
			{
				v = rmp_ptr->title.c_str(); // The string SHOULD be stable enough for this
			}

			/* Class */
			else if (streq(b + 1, "CLASS"))
			{
				v = spp_ptr->title;
			}

			/* Player */
			else if (streq(b + 1, "PLAYER"))
			{
				v = game->player_base.c_str(); // The string SHOULD be stable enough for this
			}
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;

	/* Save */
	(*sp) = s;

	/* Result */
	return (v);
}




/*
 * Process the "user pref file" with the given name
 *
 * See the function above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
	FILE *fp;

	char buf[1024];

	int num = -1;

	errr err = 0;

	bool_ bypass = FALSE;

	/* Build the filename -- Allow users to override system pref files */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* No such file -- Try system pref file */
	if (!fp)
	{
		/* Build the pathname, this time using the system pref directory */
		path_build(buf, 1024, ANGBAND_DIR_PREF, name);

		/* Open the file */
		fp = my_fopen(buf, "r");

		/* Failed again */
		if (!fp) return ( -1);
	}


	/* Process the file */
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Count lines */
		num++;


		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace(buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;


		/* Process "?:<expr>" */
		if ((buf[0] == '?') && (buf[1] == ':'))
		{
			char f;
			cptr v;
			char *s;

			/* Start */
			s = buf + 2;

			/* Parse the expr */
			v = process_pref_file_expr(&s, &f);

			/* Set flag */
			bypass = (streq(v, "0") ? TRUE : FALSE);

			/* Continue */
			continue;
		}

		/* Apply conditionals */
		if (bypass) continue;


		/* Process "%:<file>" */
		if (buf[0] == '%')
		{
			/* Process that file if allowed */
			process_pref_file(buf + 2);

			/* Continue */
			continue;
		}


		/* Process the line */
		err = process_pref_file_aux(buf);

		/* Oops */
		if (err) break;
	}


	/* Error */
	if (err)
	{
		/* Useful error message */
		msg_format("Error %d in line %d of file '%s'.", err, num, name);
		msg_format("Parsing '%s'", buf);
	}

	/* Close the file */
	my_fclose(fp);

	/* Result */
	return (err);
}




/*
 * Print long number with header at given row, column
 * Use the color for the number, not the header
 */
static void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];

	put_str(header, row, col);
	sprintf(out_val, "%9ld", (long)num);
	c_put_str(color, out_val, row, col + len);
}


/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color,
                    cptr space)
{
	int len = strlen(header);
	char out_val[32];

	put_str(header, row, col);
	put_str(space, row, col + len);
	sprintf(out_val, "%6ld", (long)num);
	c_put_str(color, out_val, row, col + len + strlen(space));
}


/*
 * Print str with header at given row, column
 */
static void prt_str(cptr header, cptr str, int row, int col, byte color)
{
	int len = strlen(header);
	char out_val[32];

	put_str(header, row, col);
	put_str("   ", row, col + len);
	sprintf(out_val, "%6s", str);
	c_put_str(color, out_val, row, col + len + 3);
}


/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
static void display_player_middle()
{
	char num[7];
	byte color;
	int speed;


	/* Dump the melee bonuses to hit/dam */
	{
		auto const o_ptr = &p_ptr->inventory[INVEN_WIELD];
		int show_tohit = p_ptr->dis_to_h + p_ptr->to_h_melee + o_ptr->to_h;
		int show_todam = p_ptr->dis_to_d + p_ptr->to_d_melee + o_ptr->to_d;

		prt_num("+ To Melee Hit   ", show_tohit, 9, 1, TERM_L_BLUE, "   ");
		prt_num("+ To Melee Damage", show_todam, 10, 1, TERM_L_BLUE, "   ");
	}

	/* Dump the ranged bonuses to hit/dam */
	{
		auto const o_ptr = &p_ptr->inventory[INVEN_BOW];
		int show_tohit = p_ptr->dis_to_h + p_ptr->to_h_ranged + o_ptr->to_h;
		int show_todam = p_ptr->to_d_ranged + o_ptr->to_d;

		prt_num("+ To Ranged Hit   ", show_tohit, 11, 1, TERM_L_BLUE, "  ");
		prt_num("+ To Ranged Damage", show_todam, 12, 1, TERM_L_BLUE, "  ");
	}

	/* Dump the total armor class */
	prt_str("  AC             ", format("%d+%d", p_ptr->ac, p_ptr->dis_to_a), 13, 1, TERM_L_BLUE);

	prt_num("Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN, "   ");

	if (p_ptr->exp >= p_ptr->max_exp)
	{
		prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_L_GREEN);
	}
	else
	{
		prt_lnum("Experience ", p_ptr->exp, 10, 28, TERM_YELLOW);
	}

	prt_lnum("Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);

	if (p_ptr->lev >= PY_MAX_LEVEL)
	{
		put_str("Exp to Adv.", 12, 28);
		c_put_str(TERM_L_GREEN, "    *****", 12, 28 + 11);
	}
	else
	{
		prt_lnum("Exp to Adv.",
		         (s32b)(player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L),
		         12, 28, TERM_L_GREEN);
	}

	prt_lnum("Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);

	if (p_ptr->necro_extra & CLASS_UNDEAD)
	{
		put_str("Death Points ", 9, 52);
		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_BLUE;
		}
		else if (p_ptr->chp > (p_ptr->mhp * options->hitpoint_warn) / 10)
		{
			color = TERM_VIOLET;
		}
		else
		{
			color = TERM_L_RED;
		}
		sprintf(num, "%6ld", (long)p_ptr->chp);
		c_put_str(color, num, 9, 65);
		put_str("/", 9, 71);
		sprintf(num, "%6ld", (long)p_ptr->mhp);
		c_put_str(TERM_L_BLUE, num, 9, 72);
	}
	else
	{
		put_str("Hit Points   ", 9, 52);
		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_GREEN;
		}
		else if (p_ptr->chp > (p_ptr->mhp * options->hitpoint_warn) / 10)
		{
			color = TERM_YELLOW;
		}
		else
		{
			color = TERM_RED;
		}
		sprintf(num, "%6ld", (long)p_ptr->chp);
		c_put_str(color, num, 9, 65);
		put_str("/", 9, 71);
		sprintf(num, "%6ld", (long)p_ptr->mhp);
		c_put_str(TERM_L_GREEN, num, 9, 72);
	}

	put_str("Spell Points ", 10, 52);
	if (p_ptr->csp >= p_ptr->msp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csp > (p_ptr->msp * options->hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	sprintf(num, "%6ld", (long)p_ptr->csp);
	c_put_str(color, num, 10, 65);
	put_str("/", 10, 71);
	sprintf(num, "%6ld", (long)p_ptr->msp);
	c_put_str(TERM_L_GREEN, num, 10, 72);

	put_str("Sanity       ", 11, 52);
	if (p_ptr->csane >= p_ptr->msane)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csane > (p_ptr->msane * options->hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	sprintf(num, "%6ld", (long)p_ptr->csane);
	c_put_str(color, num, 11, 65);
	put_str("/", 11, 71);
	sprintf(num, "%6ld", (long)p_ptr->msane);
	c_put_str(TERM_L_GREEN, num, 11, 72);

	if (p_ptr->pgod != GOD_NONE)
	{
		prt_num("Piety          ", p_ptr->grace, 12, 52, TERM_L_GREEN, "     ");
	}

	put_str("Speed           ", 13, 52);
	speed = p_ptr->pspeed;
	if (speed > 110)
	{
		char s[11];
		sprintf(s, "Fast (+%d)", speed - 110);
		c_put_str(TERM_L_GREEN, s, 13, (speed >= 120) ? 68 : 69);
	}
	else if (speed < 110)
	{
		char s[11];
		sprintf(s, "Slow (-%d)", 110 - speed);
		c_put_str(TERM_L_UMBER, s, 13, (speed <= 100) ? 68 : 69);
	}
	else
	{
		put_str("Normal", 13, 72);
	}
}




/*
 * Hack -- pass color info around this file
 */
static byte likert_color = TERM_WHITE;


/*
 * Returns a "rating" of x depending on y
 */
static cptr likert(int x, int y)
{
	static char dummy[20] = "";

	/* Paranoia */
	if (y <= 0) y = 1;

	/* Negative value */
	if (x < 0)
	{
		likert_color = TERM_L_DARK;
		return ("Very Bad");
	}

	/* Analyze the value */
	switch ((x / y))
	{
	case 0:
	case 1:
		{
			likert_color = TERM_RED;
			return ("Bad");
		}
	case 2:
		{
			likert_color = TERM_L_RED;
			return ("Poor");
		}
	case 3:
	case 4:
		{
			likert_color = TERM_ORANGE;
			return ("Fair");
		}
	case 5:
		{
			likert_color = TERM_YELLOW;
			return ("Good");
		}
	case 6:
		{
			likert_color = TERM_YELLOW;
			return ("Very Good");
		}
	case 7:
	case 8:
		{
			likert_color = TERM_L_GREEN;
			return ("Excellent");
		}
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		{
			likert_color = TERM_GREEN;
			return ("Superb");
		}
	case 14:
	case 15:
	case 16:
	case 17:
		{
			likert_color = TERM_L_GREEN;
			return ("Heroic");
		}
	default:
		{
			likert_color = TERM_L_GREEN;
			sprintf(dummy, "Legendary[%d]", (int)((((x / y) - 17) * 5) / 2));
			return dummy;
		}
	}
}


/*
 * Prints ratings on certain abilities
 *
 * This code is "imitated" elsewhere to "dump" a character sheet.
 */
static void display_player_various()
{
	auto const &r_info = game->edit_data.r_info;

	int tmp, tmp2, damdice, damsides, dambonus, blows;
	int xthn, xthb;
	int xdev, xsav, xstl;
	cptr desc;
	int i;

	object_type *o_ptr;


	/* Fighting Skill (with current weapon) */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];
	tmp = p_ptr->to_h + o_ptr->to_h + p_ptr->to_h_melee;
	xthn = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);

	/* Shooting Skill (with current bow and normal missile) */
	o_ptr = &p_ptr->inventory[INVEN_BOW];
	tmp = p_ptr->to_h + o_ptr->to_h + p_ptr->to_h_ranged;
	xthb = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);

	/* variables for all types of melee damage */
	dambonus = p_ptr->dis_to_d;
	blows = p_ptr->num_blow;

	/* Basic abilities */
	xdev = p_ptr->skill_dev;
	xsav = p_ptr->skill_sav;
	xstl = p_ptr->skill_stl;


	put_str("Fighting    :", 16, 1);
	desc = likert(xthn, 12);
	c_put_str(likert_color, desc, 16, 15);

	put_str("Bows/Throw  :", 17, 1);
	desc = likert(xthb, 12);
	c_put_str(likert_color, desc, 17, 15);

	put_str("Saving Throw:", 18, 1);
	desc = likert(xsav, 6);
	c_put_str(likert_color, desc, 18, 15);

	put_str("Stealth     :", 19, 1);
	desc = likert(xstl, 1);
	c_put_str(likert_color, desc, 19, 15);


	put_str("Magic Device:", 20, 1);
	desc = likert(xdev, 6);
	c_put_str(likert_color, desc, 20, 15);


	put_str("Blows/Round:", 16, 55);
	put_str(format("%d", p_ptr->num_blow), 16, 69);

	put_str("Shots/Round:", 17, 55);
	put_str(format("%d", p_ptr->num_fire), 17, 69);

	put_str("Mel.dmg/Rnd:", 18, 55);     /* From PsiAngband */

	if (p_ptr->melee_style == SKILL_HAND || p_ptr->melee_style == SKILL_BEAR)
	{
		/* This is all based on py_attack_hand */
		martial_arts *blow_table, *min_attack, *max_attack;
		int max_blow, plev, i;

		if (p_ptr->melee_style == SKILL_HAND)
		{
			blow_table = ma_blows;
			max_blow = MAX_MA;
			plev = get_skill(SKILL_HAND);
		}
		else /* SKILL_BEAR */
		{
			blow_table = bear_blows;
			max_blow = MAX_BEAR;
			plev = get_skill(SKILL_BEAR);
		}
		min_attack = blow_table;
		i = max_blow - 1;
		while (blow_table[i].min_level > plev && i != 0)
			--i;
		max_attack = &blow_table[i];

		dambonus += p_ptr->to_d_melee;
		tmp = min_attack->dd + dambonus;
		if (tmp < 0) tmp = 0;
		tmp2 = maxroll(max_attack->dd, max_attack->ds) + dambonus;
		if (tmp2 < 0) tmp2 = 0;
		if (!tmp && !tmp2)
			desc = "0";
		else
			desc = format("%d-%d", blows * tmp, blows * tmp2);
	}
	else if (!r_info[p_ptr->body_monster].body_parts[BODY_WEAPON])
	{
		if (r_info[p_ptr->body_monster].flags & RF_NEVER_BLOW)
			desc = "nil!";
		else
		{
			tmp = tmp2 = 0;
			for (i = 0; i < blows; i++)
			{
				tmp += r_info[p_ptr->body_monster].blow[i].d_dice;
				tmp2 += maxroll(r_info[p_ptr->body_monster].blow[i].d_dice,
				                r_info[p_ptr->body_monster].blow[i].d_side);
			}
			if (dambonus > 0)
			{
				tmp += dambonus;
				tmp2 += dambonus;
			}
			desc = format("%d-%d", tmp, tmp2);
		}
	}
	else
	{
		/* Increase the bonus to damage for weapon combat */
		dambonus += p_ptr->to_d_melee;

		/* Access the first weapon */
		o_ptr = &p_ptr->inventory[INVEN_WIELD];

		dambonus += o_ptr->to_d;

		damdice = o_ptr->dd;
		damsides = o_ptr->ds;

		if ((damdice == 0) || (damsides == 0))
		{
			if (dambonus <= 0)
				desc = "nil!";
			else
				desc = format("%d", blows * dambonus);
		}
		else
		{
			if (dambonus == 0)
				desc = format("%dd%d", blows * damdice, damsides);
			else
				desc = format("%dd%d%c%d", blows * damdice, damsides,
				              ( dambonus > 0 ? '+' : '\0' ), blows * dambonus );
		}
	}
	put_str(desc, 18, 69);


	put_str("Infra-Vision:", 19, 55);
	put_str(format("%d feet", p_ptr->see_infra * 10), 19, 69);

	/* jk - add tactic */
	put_str("Tactic:", 20, 55);
	c_put_str(TERM_L_BLUE, tactic_info[(byte)p_ptr->tactic].name, 20, 69);

	/* jk - add movement */
	put_str("Explor:", 21, 55);
	c_put_str(TERM_L_BLUE, move_info[(byte)p_ptr->movement].name, 21, 69);
}



/*
 * Obtain the "flags" of the wielded symbiote
 */

static object_flag_set wield_monster_flags()
{
	auto const &r_info = game->edit_data.r_info;

	object_flag_set flags;

	/* Get the carried monster */
	auto o_ptr = &p_ptr->inventory[INVEN_CARRY];
	if (o_ptr->k_ptr)
	{
		auto r_ptr = &r_info[o_ptr->pval];

		if (r_ptr->flags & RF_INVISIBLE)
		{
			flags |= TR_INVIS;
		}

		if (r_ptr->flags & RF_REFLECTING)
		{
			flags |= TR_REFLECT;
		}

		if (r_ptr->flags & RF_CAN_FLY)
		{
			flags |= TR_FEATHER;
		}

		if (r_ptr->flags & RF_AQUATIC)
		{
			flags |= TR_WATER_BREATH;
		}
	}

	return flags;
}


template<class LF>
static void apply_lflags(LF const &lflags, object_flag_set *f)
{
	for (int i = 1; i <= p_ptr->lev; i++)
	{
		(*f) |= lflags[i].oflags;
	}
}


/*
 * Obtain the "flags" for the player as if he was an item
 */
object_flag_set player_flags()
{
	auto const &r_info = game->edit_data.r_info;

	/* Clear */
	object_flag_set f;

	/* Astral chars */
	if (p_ptr->astral)
	{
		f |= TR_WRAITH;
	}

/* Skills */
	if (get_skill(SKILL_DAEMON) > 20) f |= TR_RES_CONF;
	if (get_skill(SKILL_DAEMON) > 30) f |= TR_RES_FEAR;
	if (get_skill(SKILL_MINDCRAFT) >= 40) f |= ESP_ALL;
	if (p_ptr->melee_style == SKILL_HAND && get_skill(SKILL_HAND) > 24 && !monk_heavy_armor())
	{
		f |= TR_FREE_ACT;
	}
	if (get_skill(SKILL_MANA) >= 35) f |= TR_MANA;
	if (get_skill(SKILL_AIR) >= 50) f |= (TR_MAGIC_BREATH | TR_WATER_BREATH);
	if (get_skill(SKILL_WATER) >= 30) f |= TR_WATER_BREATH;

/* Gods */
	if (p_ptr->pgod == GOD_ERU)
	{
		if ((p_ptr->grace >= 100) || (p_ptr->grace <= -100))  f |= TR_MANA;
		if (p_ptr->grace > 10000) f |= TR_WIS;
	}

	if (p_ptr->pgod == GOD_MELKOR)
	{
		f |= TR_RES_FIRE;
		if (p_ptr->melkor_sacrifice > 0) f |= TR_LIFE;
		if (p_ptr->grace > 10000) f |= (TR_STR | TR_CON | TR_INT | TR_WIS | TR_CHR);
		if (p_ptr->praying)
		{
			if (p_ptr->grace > 5000)  f |= TR_INVIS;
			if (p_ptr->grace > 15000) f |= TR_IM_FIRE;
		}
	}

	if (p_ptr->pgod == GOD_MANWE)
	{
		if (p_ptr->grace >= 2000) f |= TR_FEATHER;
		if (p_ptr->praying)
		{
			if (p_ptr->grace >= 7000)  f |= TR_FREE_ACT;
			if (p_ptr->grace >= 15000) f |= TR_FLY;
			if ((p_ptr->grace >= 5000) || (p_ptr->grace <= -5000)) f |= TR_SPEED;
		}
	}

	if (p_ptr->pgod == GOD_TULKAS)
	{
		if (p_ptr->grace > 5000)  f |= TR_CON;
		if (p_ptr->grace > 10000) f |= TR_STR;
	}

	if (p_ptr->pgod == GOD_AULE)
	{
		if (p_ptr->grace > 5000)
		{
			f |= TR_RES_FIRE;
		}
	}

	if (p_ptr->pgod == GOD_MANDOS)
	{
		f |= TR_RES_NETHER;

		if ((p_ptr->grace > 10000) &&
		    (p_ptr->praying == TRUE))
		{
			f |= TR_NO_TELE;
		}

		if ((p_ptr->grace > 20000) &&
		    (p_ptr->praying == TRUE))
		{
			f |= TR_IM_NETHER;
		}
	}

	if (p_ptr->pgod == GOD_ULMO)
	{
		f |= TR_WATER_BREATH;

		if ((p_ptr->grace > 1000) &&
		    (p_ptr->praying == TRUE))
		{
			f |= TR_RES_POIS;
		}

		if ((p_ptr->grace > 15000) &&
		    (p_ptr->praying == TRUE))
		{
			f |= TR_MAGIC_BREATH;
		}
	}

	/* Classes */
	apply_lflags(cp_ptr->lflags, &f);

	/* Races */
	if ((!p_ptr->mimic_form) && (!p_ptr->body_monster))
	{
		apply_lflags(rp_ptr->lflags, &f);
		apply_lflags(rmp_ptr->lflags, &f);
	}
	else
	{
		auto &r_ref = r_info[p_ptr->body_monster];

		if (r_ref.flags & RF_REFLECTING) f |= TR_REFLECT;
		if (r_ref.flags & RF_REGENERATE) f |= TR_REGEN;
		if (r_ref.flags & RF_AURA_FIRE) f |= TR_SH_FIRE;
		if (r_ref.flags & RF_AURA_ELEC) f |= TR_SH_ELEC;
		if (r_ref.flags & RF_PASS_WALL) f |= TR_WRAITH;
		if (r_ref.flags & RF_SUSCEP_FIRE) f |= TR_SENS_FIRE;
		if (r_ref.flags & RF_IM_ACID) f |= TR_RES_ACID;
		if (r_ref.flags & RF_IM_ELEC) f |= TR_RES_ELEC;
		if (r_ref.flags & RF_IM_FIRE) f |= TR_RES_FIRE;
		if (r_ref.flags & RF_IM_POIS) f |= TR_RES_POIS;
		if (r_ref.flags & RF_IM_COLD) f |= TR_RES_COLD;
		if (r_ref.flags & RF_RES_NETH) f |= TR_RES_NETHER;
		if (r_ref.flags & RF_RES_NEXU) f |= TR_RES_NEXUS;
		if (r_ref.flags & RF_RES_DISE) f |= TR_RES_DISEN;
		if (r_ref.flags & RF_NO_FEAR) f |= TR_RES_FEAR;
		if (r_ref.flags & RF_NO_SLEEP) f |= TR_FREE_ACT;
		if (r_ref.flags & RF_NO_CONF) f |= TR_RES_CONF;
		if (r_ref.flags & RF_CAN_FLY) f |= TR_FEATHER;
	}

	f |= p_ptr->xtra_flags;

	if (p_ptr->black_breath)
	{
		f |= TR_BLACK_BREATH;
	}

	if (p_ptr->hp_mod != 0)
	{
		f |= TR_LIFE;
	}

	return f;
}

namespace { // <anonymous>

/*
 * Build an return a (static) index of all the object_flag_meta
 * information indexed by page->column->row.
 */
static std::vector<object_flag_meta const *> const &object_flag_metas_by_pcr(int page, int column, int row)
{
	static std::vector<std::vector<std::vector<std::vector<object_flag_meta const *>>>> instance;

	if (instance.empty())
	{
		// Find number of pages, columns and rows.
		std::size_t n_pages = 0;
		std::size_t n_columns = 0;
		std::size_t n_rows = 0;

		for (auto const &object_flag_meta: object_flags_meta())
		{
			n_pages = std::max<std::size_t>(n_pages, object_flag_meta->c_page + 1);
			n_columns = std::max<std::size_t>(n_columns, object_flag_meta->c_column + 1);
			n_rows = std::max<std::size_t>(n_rows, object_flag_meta->c_row + 1);
		}

		// Sanity check; we should always have enough data.
		assert(n_pages > 0);
		assert(n_columns > 0);
		assert(n_rows > 0);

		// Build the scaffolding structure without the actual data.
		instance.reserve(n_pages);
		for (std::size_t i = 0; i < n_pages; i++)
		{
			std::vector<std::vector<std::vector<object_flag_meta const *>>> page;
			page.reserve(n_columns);

			for (std::size_t j = 0; j < n_columns; j++)
			{
				std::vector<std::vector<object_flag_meta const *>> column;
				column.reserve(n_rows);

				for (std::size_t k = 0; k < n_rows; k++)
				{
					std::vector<object_flag_meta const *> row;
					column.push_back(row);
				}

				page.push_back(column);
			}

			instance.push_back(page);
		}

		// Insert all the data.
		for (auto const object_flag_meta: object_flags_meta())
		{
			// Ignore if not mapped to any page.
			if (!object_flag_meta->c_name)
			{
				continue;
			}

			// Find the row
			auto &row = instance
			        .at(object_flag_meta->c_page)
			        .at(object_flag_meta->c_column)
			        .at(object_flag_meta->c_row);

			// Insert
			row.push_back(object_flag_meta);
		}
	}

	return instance.at(page).at(column).at(row);
}


/**
 * Convert a number to a digit, capping at '9'. Ignores the sign of the number.
 */
static char number_to_digit(int n) {
	// Throw away sign.
	n = std::abs(n);
	// Convert to digit or '*'
	return (n > 9 ? '*' : I2D(n));
};


/**
 * Check that two object_flag types are compatible.
 */
static bool object_flag_types_are_compatible(char type_a, char type_b)
{
	switch (type_a)
	{
	case 'n':
		return (type_b == 'n');
	case 'b':
		return (type_b == 'b');
	case '+':
	case '*':
		return (type_b == '+') || (type_b == '*');
	case 'f':
		return (type_b == 'f');
	case '\0':
		return true;
	default:
		abort();
	}
}

/**
 * Object flag data for calculating cells on the character sheet.
 */
struct object_flag_cell {
	/**
	 * Type designator for the cell, if any.
	 */
	char type;

	/**
	 * Associated PVAL, if any.
	 */
	int pval;

	/**
	 * Label for the cell, given its current contents.
	 */
	const char *label;

	/**
	 * Create object_flag_cell from object_flag_meta.
	 */
	static object_flag_cell from_object_flag_meta(object_flag_meta const &object_flag_meta, int pval)
	{
		// The FIXED type flags require special handling.
		if ((object_flag_meta.c_type == '1') || (object_flag_meta.c_type == '2') || (object_flag_meta.c_type == '3'))
		{
			return object_flag_cell {
				'f',
				D2I(object_flag_meta.c_type),
				object_flag_meta.c_name
			};
		}
		else
		{
			return object_flag_cell {
				object_flag_meta.c_type,
				pval,
				object_flag_meta.c_name
			};
		}
	}

};

namespace detail { // "hide" from surrounding scope; should only be accessed through the monoid

static object_flag_cell object_flag_cell_append(object_flag_cell const &a, object_flag_cell const &b)
{
	// The "empty" value automatically gets swallowed, whatever
	// "side" of the append it's on.
	if (a.type == '\0')
	{
		return b;
	}

	if (b.type == '\0')
	{
		return a;
	}

	// The rest of the code assumes compatible types, so we
	// assert this to a) avoid over-complicated logic, and
	// b) breaking under 'unexpected' changes to the object
	// flag list.
	if (!object_flag_types_are_compatible(a.type, b.type))
	{
		abort();
	}

	// Any boolean flag which is "set" overrides the previous
	// flag. (If the flag was already set we won't lose any
	// information.)
	if (b.type == 'b')
	{
		return b;
	}

	// Flags with a numerical value get added together.
	if (b.type == 'n')  // (a.type == 'n') by symmetry and object_flag_types_are_compatible()
	{
		// Arbitrary choice of label -- the labels *should* be the same, by definition
		return object_flag_cell { 'n', a.pval + b.pval, a.label };
	}

	// Fixed-value flags.
	if (a.type == 'f')   // (b.type == 'f') by symmetry and object_flag_types_are_compatible()
	{
		// Arbitrary choice of label -- the labels *should* be the same, by definition
		return object_flag_cell { 'f', a.pval + b.pval, a.label };
	}

	// Flags of the TERNARY variety have a "supercedes" rule
	// where immunity supercedes resistance.
	if (a.type == '*')
	{
		return object_flag_cell { '*', 0, a.label };
	}
	else if (b.type == '*')
	{
		return object_flag_cell { '*', 0, b.label };
	}
	else  // Both must be '+'
	{
		// Arbitrary choice of label -- the labels *should* be the same, by definition
		return object_flag_cell { '+', 0, a.label };
	}
}

constexpr object_flag_cell object_flag_cell_empty { '\0', 0, nullptr };

} // namespace "detail"

using object_flag_cell_monoid = monoid<object_flag_cell, detail::object_flag_cell_append, detail::object_flag_cell_empty>;

} // namespace <anonymous>

namespace { // <anonymous>

static object_flag_meta const *get_lowest_priority_object_flag_meta(std::vector<object_flag_meta const *> const &object_flag_metas)
{
	object_flag_meta const *found = nullptr;

	for (auto object_flag_meta: object_flag_metas)
	{
		if ((!found) || (found->c_priority > object_flag_meta->c_priority))
		{
			found = object_flag_meta;
		}
	}

	return found;
}


static std::tuple<char, int> object_flag_cell_to_char(object_flag_cell const &object_flag_cell)
{
	switch (object_flag_cell.type)
	{
	case 'n':
	case 'f':
		// If we have no pval, we use a simple '+'. This applies
		// to the 'player' slot.
		if (object_flag_cell.pval == 0)
		{
			return std::make_tuple('+', 1);
		}
		else
		{
			return std::make_tuple(
			        number_to_digit(object_flag_cell.pval),
			        (object_flag_cell.pval >= 0) ? 1 : -1);
		}
		break;

	case 'b':
	case '+':
		return std::make_tuple('+', 1);
		break;

	case '*':
		return std::make_tuple('*', 1);
		break;

	default:
		return std::make_tuple('.', 0);
		break;
	}

	abort();
}


/*
 * Output a slot
 */
static void display_flag_row(
        int y,
        int x0,
        std::vector<std::tuple<char, int, object_flag_set>> const &slots,
        std::vector<object_flag_meta const *> const &object_flag_metas_at_pcr)
{
	assert(!object_flag_metas_at_pcr.empty());

	// Accumulated value of all of the slots
	auto acc = object_flag_cell_monoid::empty;

	// Go through each slot
	for (std::size_t i = 0; i < slots.size(); i++)
	{
		object_flag_cell combined = object_flag_cell_monoid::empty;

		// Go through all flags that are actually set for this 'cell'.
		{
			auto const &slot = slots[i];
			auto const pval = std::get<1>(slot);
			auto const flags = std::get<2>(slot);

			for (auto const object_flag_meta: object_flag_metas_at_pcr)
			{
				if (object_flag_meta->flag_set & flags)
				{
					combined = object_flag_cell_monoid::append(
					        combined,
					        object_flag_cell::from_object_flag_meta(*object_flag_meta, pval));
				}
			}
		}

		// Accumulate into the global accumulator
		acc = object_flag_cell_monoid::append(acc, combined);

		// Write the cell's value.
		auto const char_and_rating = object_flag_cell_to_char(combined);
		auto const ch = std::get<0>(char_and_rating);
		auto const rating = std::get<1>(char_and_rating);

		// Convert good flag into a color.
		byte a;
		if (rating == 0)
		{
			a = (i & 0x02) ? TERM_GREEN : TERM_SLATE;
		}
		else if (rating < 0)
		{
			a = TERM_RED;
		}
		else
		{
			a = TERM_L_GREEN;
		}

		// Output the flag
		Term_putch(x0 + 11 + i, y, a, ch);
	}

	// Extract the label. If the flag isn't set at all then we don't have
	// an actual label, so we'll use the one from the meta-level object
	// flag definition. Note that the prioritization is crucial for the
	// labeling to work properly; e.g. IMM_FIRE comes *before* the
	// RES_FIRE flag in the object flag list, but we don't want the *label*
	// from IMM_FIRE to be used when the flag is not set at all.
	auto const label = (acc.label != nullptr)
	        ? acc.label
	        : get_lowest_priority_object_flag_meta(object_flag_metas_at_pcr)->c_name;

	// Get the "rating" for the label.
	auto const rating = std::get<1>(object_flag_cell_to_char(acc));

	byte label_attr;
	if (rating == 0) {
		label_attr = TERM_WHITE;
	} else if (rating < 0) {
		label_attr = TERM_RED;
	} else {
		label_attr = TERM_L_GREEN;
	}

	Term_putch(x0 + 10, y, TERM_WHITE, ':');
	Term_putstr(x0, y, -1, label_attr, label);
}

/*
 * Summarize resistances
 */
static void display_player_ben_one(int page)
{
	// Slots of flags to show.
	std::vector<std::tuple<char, int, object_flag_set>> slots;
	slots.reserve(INVEN_TOTAL - INVEN_WIELD + 1);

	// Scan equipment
	for (std::size_t i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		// Skip inventory slots that don't exist on the body.
		auto n = i - INVEN_WIELD;
		if ((n < INVEN_TOTAL - INVEN_WIELD) && (!p_ptr->body_parts[n])) continue;

		// Extract object flags
		auto const o_ptr = &p_ptr->inventory[i];
		auto const flags = object_flags_known(o_ptr);
		// Add slot
		slots.emplace_back(
		        std::make_tuple('a' + i - INVEN_WIELD, o_ptr->pval, flags));
	}

	// Carried symbiote
	{
		// Extract flags
		auto const flags = wield_monster_flags();
		// Add slot
		slots.emplace_back(
		        std::make_tuple('z', 0, flags));
	}

	// Player
	slots.emplace_back(
	        std::make_tuple('@', 0, player_flags()));

	// Go through each column
	for (int col = 0; col < 2; col++)
	{
		// Base coordinate for output
		const auto x0 = col * 40;
		const auto y0 = 3;

		// Add slot headings.
		{
			std::string buf;
			buf.reserve(slots.size());

			for (auto const &slot: slots)
			{
				buf += std::get<0>(slot);
			}

			Term_putstr(x0 + 11, y0, -1, TERM_WHITE, buf.c_str());
		}

		// Scan rows
		for (int row = 0; row < 16; row++)
		{
			// Extract the flag metadata for the current page/col/row
			auto const object_flag_metas_at_pcr =
			        object_flag_metas_by_pcr(page, col, row);

			// Ignore flags which we don't actually map to anything.
			if (object_flag_metas_at_pcr.empty())
			{
				continue;
			}

			// Y coordinate for the row
			auto const y = y0 + 1 + row;

			// Show the row
			display_flag_row(y, x0, slots, object_flag_metas_at_pcr);
		}
	}
}

} // namespace <anonymous>

/*
 * Display the character on the screen
 */
void display_player(int mode)
{
	auto const &r_info = game->edit_data.r_info;

	assert(mode >= 0);
	assert(mode < 5);

	char buf[80];


	/* Erase screen */
	clear_from(0);

	/* Standard */
	if (mode == 0)
	{
		auto r_ptr = &r_info[p_ptr->body_monster];

		/* Name, Sex, Race, Class */
		put_str("Name  :", 2, 1);
		c_put_str(TERM_L_BLUE, game->player_name.c_str(), 2, 9);

		put_str("Race  :", 3, 1);
		auto const player_race_name = get_player_race_name(p_ptr->prace, p_ptr->pracem);
		c_put_str(TERM_L_BLUE, player_race_name.c_str(), 3, 9);

		put_str("Class :", 4, 1);
		c_put_str(TERM_L_BLUE, spp_ptr->title, 4, 9);

		put_str("Body  :", 5, 1);
		c_put_str(TERM_L_BLUE, r_ptr->name, 5, 9);

		put_str("God   :", 6, 1);
		c_put_str(TERM_L_BLUE, deity_info[p_ptr->pgod].name, 6, 9);

		/* Display the stats */
		for (int i = 0; i < 6; i++)
		{
			char punctuation = p_ptr->stat_max[i] == 18 + 100 ? '!' : ':';
			/* Special treatment of "injured" stats */
			if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
			{
				int value;
				int colour;

				if (p_ptr->stat_cnt[i])
					colour = TERM_ORANGE;
				else
					colour = TERM_YELLOW;

				/* Use lowercase stat name */
				put_str(format("%s%c ", stat_names_reduced[i], punctuation), 2 + i, 61);

				/* Get the current stat */
				value = p_ptr->stat_use[i];

				/* Obtain the current stat (modified) */
				cnv_stat(value, buf);

				/* Display the current stat (modified) */
				c_put_str(colour, buf, 2 + i, 66);

				/* Acquire the max stat */
				value = p_ptr->stat_top[i];

				/* Obtain the maximum stat (modified) */
				cnv_stat(value, buf);

				/* Display the maximum stat (modified) */
				c_put_str(TERM_L_GREEN, buf, 2 + i, 73);
			}

			/* Normal treatment of "normal" stats */
			else
			{
				/* Assume uppercase stat name */
				put_str(format("%s%c ", stat_names[i], punctuation), 2 + i, 61);

				/* Obtain the current stat (modified) */
				cnv_stat(p_ptr->stat_use[i], buf);

				/* Display the current stat (modified) */
				c_put_str(TERM_L_GREEN, buf, 2 + i, 66);
			}
		}

		/* Extra info */
		display_player_middle();

		/* Display "various" info */
		put_str("(Miscellaneous Abilities)", 15, 25);
		display_player_various();
	}

	/* Special */
	else
	{
		display_player_ben_one(mode - 1);
	}
}

/*
 * Utility function; should probably be in some other file...
 *
 * Describe the player's location -- either by dungeon level, town, or in
 * wilderness with landmark reference.
 */
std::string describe_player_location()
{
	auto const &wilderness = game->wilderness;
	auto const &d_info = game->edit_data.d_info;
	auto const &wf_info = game->edit_data.wf_info;

	std::string desc;

	int pwx = (p_ptr->wild_mode ? p_ptr->px : p_ptr->wilderness_x);
	int pwy = (p_ptr->wild_mode ? p_ptr->py : p_ptr->wilderness_y);
	int feat = wilderness(pwx, pwy).feat;

	if (dungeon_type != DUNGEON_WILDERNESS && dun_level > 0)
	{
		desc += fmt::format("on level {:d} of {}", dun_level, d_info[dungeon_type].name);
	}
	else if (wf_info[feat].terrain_idx == TERRAIN_TOWN)
	{
		desc += fmt::format("in the town of {}", wf_info[feat].name);
	}
	else if (wf_info[feat].entrance)
	{
		desc += fmt::format("near {}", wf_info[feat].name);
	}
	else
	{
		/*
		 * The complicated case.  Find the nearest known landmark,
		 * and describe our position relative to that.  Note that
		 * we may not even have any known landmarks (for instance,
		 * a Lost Soul character just after escaping the Halls of
		 * Mandos).
		 */
		int landmark = 0, lwx = 0, lwy = 0;
		int l_dist = -1;

		for (std::size_t i = 0; i < wf_info.size(); i++)
		{
			int wx = wf_info[i].wild_x;
			int wy = wf_info[i].wild_y;

			/* Skip if not a landmark */
			if (!wf_info[i].entrance) continue;

			/* Skip if we haven't seen it */
			if (!wilderness(wx, wy).known) continue;

			int dist = distance(wy, wx, pwy, pwx);
			if (dist < l_dist || l_dist < 0)
			{
				landmark = i;
				l_dist = dist;
				lwx = wx;
				lwy = wy;
			}
		}

		if (!landmark)
		{
			desc += fmt::format("in {}", wf_info[feat].text);
		}
		else if (pwx == lwx && pwy == lwy)
		{
			/* Paranoia; this should have been caught above */
			desc += fmt::format("near {}", wf_info[feat].name);
		}
		else
		{
			/*
			 * We split the circle into eight equal octants of
			 * size pi/4 radians; the "east" octant, for
			 * instance, is defined as due east plus or minus
			 * pi/8 radians.  Now sin(pi/8) ~= 0.3826 ~= 31/81,
			 * so we check |dx|/|dy| and |dy|/|dx| against that
			 * ratio to determine which octant we're in.
			 */
			int dx = pwx - lwx;
			int dy = pwy - lwy;
			cptr ns = (dy > 0 ? "south" : "north");
			cptr ew = (dx > 0 ? "east" : "west");

			dx = (dx < 0 ? -dx : dx);
			dy = (dy < 0 ? -dy : dy);
			if (dy * 81 < dx * 31) ns = "";
			if (dx * 81 < dy * 31) ew = "";

			desc += fmt::format(
				"in {} {}{} of {}",
				wf_info[feat].text,
				ns,
				ew,
				wf_info[landmark].name);
		}
	}

	/* Strip trailing whitespace */
	boost::trim_right(desc);

	return desc;
}

/*
 * Helper function or file_character_print_grid
 *
 * Figure out if a row on the grid is empty
 */
static bool_ file_character_print_grid_check_row(const char *buf)
{
	if (strstr(buf + 12, "+")) return TRUE;
	if (strstr(buf + 12, "*")) return TRUE;
	if (strstr(buf + 12, "1")) return TRUE;
	if (strstr(buf + 12, "2")) return TRUE;
	if (strstr(buf + 12, "3")) return TRUE;
	if (strstr(buf + 12, "4")) return TRUE;
	if (strstr(buf + 12, "5")) return TRUE;
	if (strstr(buf + 12, "6")) return TRUE;
	if (strstr(buf + 12, "7")) return TRUE;
	if (strstr(buf + 12, "8")) return TRUE;
	if (strstr(buf + 12, "9")) return TRUE;
	return FALSE;
}

/*
 * Helper function for file_character
 *
 * Prints the big ugly grid
 */
static void file_character_print_grid(FILE *fff, bool_ show_gaps, bool_ show_legend)
{
	static cptr blank_line = "                                        ";
	static char buf[1024];
	byte a;
	char c;
	int x, y;

	y = show_legend ? 3 : 4;
	for (; y < 23; y++)
	{
		for (x = 0; x < 40; x++)
		{
			(Term_what(x, y, &a, &c));
			buf[x] = c;
		}

		buf[x] = '\0';
		if (strcmp(buf, blank_line) &&
		                (y == 3 || show_gaps || file_character_print_grid_check_row(buf)))
			fprintf (fff, "        %s\n", buf);
	}
	for (y = 4; y < 23; y++)
	{
		for (x = 40; x < 80; x++)
		{
			(Term_what(x, y, &a, &c));
			buf[x - 40] = c;
		}

		buf[x] = '\0';
		if (strcmp(buf, blank_line) &&
		                (show_gaps || file_character_print_grid_check_row(buf)))
			fprintf (fff, "        %s\n", buf);
	}
}

/*
 * Helper function for file_character
 *
 * Outputs one item (for Inventory, Equipment, Home, and Mathom-house)
 */
static void file_character_print_item(FILE *fff, char label, object_type *obj)
{
	static char o_name[80];
	static cptr paren = ")";

	object_desc(o_name, obj, TRUE, 3);
	fprintf(fff, "%c%s %s\n", label, paren, o_name);
	object_out_desc(obj, fff, TRUE, TRUE);
}

/*
 * Helper function for file_character
 *
 * Prints out one "store" (for Home and Mathom-house)
 */
static void file_character_print_store(FILE *fff, wilderness_type_info const *place, std::size_t store)
{
	auto const &st_info = game->edit_data.st_info;

	town_type *town = &town_info[place->entrance];
	store_type *st_ptr = &town->store[store];

	if (st_ptr->stock.size())
	{
		/* Header with name of the town */
		fprintf(fff, "  [%s Inventory - %s]\n\n", st_info[store].name.c_str(), place->name);

		/* Dump all available items */
		for (std::size_t i = 0; i < st_ptr->stock.size(); i++)
		{
			file_character_print_item(fff, I2A(i%24), &st_ptr->stock[i]);
		}

		/* Add an empty line */
		fprintf(fff, "\n\n");
	}
}

/**
 * Helper function for file_character
 *
 * Checks if the store hasn't been added to the list yet, and then adds it if it
 * was not already there.  XXX This is an ugly workaround for the double Gondolin
 * problem.
 */
static bool_ file_character_check_stores(std::unordered_set<store_type *> *seen_stores, wilderness_type_info const *place, int store)
{
	town_type *town = &town_info[place->entrance];
	store_type *st_ptr = &town->store[store];

	// Already seen store?
	if (seen_stores->find(st_ptr) != seen_stores->end())
	{
		return FALSE;
	}

	// Add
	seen_stores->insert(st_ptr);
	return TRUE;
}

/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(cptr name)
{
	auto const &d_info = game->edit_data.d_info;
	auto const &wf_info = game->edit_data.wf_info;
	auto const &r_info = game->edit_data.r_info;

	int i, x, y;
	byte a;
	char c;
	int fd = -1;
	FILE *fff = NULL;
	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* Check for existing file */
	fd = fd_open(buf, O_RDONLY);

	/* Existing file */
	if (fd >= 0)
	{
		char out_val[160];

		/* Close the file */
		fd_close(fd);

		/* Build query */
		sprintf(out_val, "Replace existing file %s? ", buf);

		/* Ask */
		if (get_check(out_val)) fd = -1;
	}

	/* Open the non-existing file */
	if (fd < 0) fff = my_fopen(buf, "w");

	/* Invalid file */
	if (!fff)
	{
		/* Message */
		msg_format("Character sheet creation failed!");
		msg_print(NULL);

		/* Error */
		return ( -1);
	}


	/* Begin dump */
	fprintf(fff, "  [%s Character Sheet]\n\n", get_version_string());


	/* Display player */
	display_player(0);

	/* Dump part of the screen */
	for (y = 2; y < 22; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		fprintf(fff, "%s\n", buf);
	}

	/* Display history */
	display_player(1);

	/* Dump part of the screen */
	for (y = 15; y < 20; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		fprintf(fff, "%s\n", buf);
	}

	/* List the patches */
	fprintf(fff, "\n\n  [Miscellaneous information]\n");
	if (options->joke_monsters)
		fprintf(fff, "\n Joke monsters:        ON");
	else
		fprintf(fff, "\n Joke monsters:        OFF");

	if (options->preserve)
		fprintf(fff, "\n Preserve Mode:        ON");
	else
		fprintf(fff, "\n Preserve Mode:        OFF");

	if (options->auto_scum)
		fprintf(fff, "\n Autoscum:             ON");
	else
		fprintf(fff, "\n Autoscum:             OFF");

	if (options->always_small_level)
		fprintf(fff, "\n Small Levels:         ALWAYS");
	else if (options->small_levels)
		fprintf(fff, "\n Small Levels:         ON");
	else
		fprintf(fff, "\n Small Levels:         OFF");

	if (options->empty_levels)
		fprintf(fff, "\n Arena Levels:         ON");
	else
		fprintf(fff, "\n Arena Levels:         OFF");

	if (options->ironman_rooms)
		fprintf(fff, "\n Always unusual rooms: ON");
	else
		fprintf(fff, "\n Always unusual rooms: OFF");

	fprintf(fff, "\n\n Recall Depth:");
	for (y = 1; y < static_cast<int>(d_info.size()); y++)
	{
		if (max_dlv[y])
			fprintf(fff, "\n        %s: Level %d",
				d_info[y].name.c_str(),
				max_dlv[y]);
	}
	fprintf(fff, "\n");

	if (noscore)
		fprintf(fff, "\n You have done something illegal.");

	if (race_flags_p(PR_EXPERIMENTAL))
		fprintf(fff, "\n You have done something experimental.");

	{
		char desc[80];
		cptr mimic;

		monster_race_desc(desc, p_ptr->body_monster, 0);
		fprintf(fff, "\n Your body %s %s.", (death ? "was" : "is"), desc);

		if (p_ptr->tim_mimic)
		{
			mimic = get_mimic_name(p_ptr->mimic_form);
			fprintf(fff, "\n You %s disguised as a %s.", (death ? "were" : "are"), mimic);
		}
	}

	/* Where we are, if we're alive */
	if (!death)
	{
		fprintf(fff, "\n You are currently %s.", describe_player_location().c_str());
	}

	/* Monsters slain */
	{
		s32b Total = 0;

		for (auto const &r_ref: r_info)
		{
			auto r_ptr = &r_ref;

			if (r_ptr->flags & RF_UNIQUE)
			{
				bool_ dead = (r_ptr->max_num == 0);
				if (dead)
				{
					Total++;
				}
			}
			else
			{
				s16b This = r_ptr->r_pkills;
				if (This > 0)
				{
					Total += This;
				}
			}
		}

		if (Total < 1)
			fprintf(fff, "\n You have defeated no enemies yet.");
		else if (Total == 1)
			fprintf(fff, "\n You have defeated one enemy.");
		else
                  fprintf(fff, "\n You have defeated %ld enemies.", (long int) Total);
	}

	struct hook_chardump_in in = { fff };
	process_hooks_new(HOOK_CHAR_DUMP, &in, NULL);

	/* Date */
	{
		u32b days = bst(DAY, turn);
		fprintf(fff,
		        (death ? "\n Your adventure lasted %ld day%s." : "\n You have been adventuring for %ld day%s."),
		        (long int) days, (days == 1) ? "" : "s");
	}

	fprintf (fff, "\n\n");

	/* adds and slays */
	display_player(1);
	file_character_print_grid(fff, FALSE, TRUE);

	/* sustains and resistances */
	display_player(2);
	file_character_print_grid(fff, TRUE, FALSE);

	/* stuff */
	display_player(3);
	file_character_print_grid(fff, FALSE, FALSE);

	/* a little bit of stuff */
	display_player(4);
	file_character_print_grid(fff, FALSE, FALSE);

	/* Dump corruptions */
	fprintf(fff, "\n%s\n", dump_corruptions(false, true).c_str());

	/* Dump skills */
	dump_skills(fff);
	dump_abilities(fff);

	/* Dump companions. */
	dump_companions(fff);

	if (p_ptr->companion_killed)
	{
		if (p_ptr->companion_killed == 1)
			fprintf(fff, "\n One of your companion(s) has been killed.");
		else
			fprintf(fff, "\n %d of your companions have been killed.", p_ptr->companion_killed);
	}

	for (i = 0; i < MAX_FATES; i++)
	{
		if ((fates[i].fate) && (fates[i].know))
		{
			fprintf(fff, "\n\n  [Fates]\n\n");
			fprintf(fff, "%s", dump_fates().c_str());
			break;
		}
	}

	/* Skip some lines */
	fprintf(fff, "\n\n");


	/* Dump the equipment */
	text_out_indent = 4;
	if (equip_cnt)
	{
		fprintf(fff, "  [Character Equipment]\n\n");
		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			if (!p_ptr->body_parts[i - INVEN_WIELD]) continue;

			file_character_print_item(fff, index_to_label(i), &p_ptr->inventory[i]);
		}
		fprintf(fff, "\n\n");
	}

	/* Dump the inventory */
	fprintf(fff, "  [Character Inventory]\n\n");
	for (i = 0; i < INVEN_PACK; i++)
	{
		file_character_print_item(fff, index_to_label(i), &p_ptr->inventory[i]);
	}
	fprintf(fff, "\n\n");

	/* Print all homes in the different towns */
	{
		std::unordered_set<store_type *> seen_stores;
		for (auto const &wf_ref: wf_info)
		{
			if (wf_ref.feat == FEAT_TOWN &&
			    file_character_check_stores(&seen_stores, &wf_ref, 7))
			{
				file_character_print_store(fff, &wf_ref, 7);
			}
		}
	}

	/* Print all Mathom-houses in the different towns */
	{
		std::unordered_set<store_type *> seen_stores;
		for (auto const &wf_ref: wf_info)
		{
			if (wf_ref.feat == FEAT_TOWN &&
			    file_character_check_stores(&seen_stores, &wf_ref, 57))
			{
				file_character_print_store(fff, &wf_ref, 57);
			}
		}
	}

	text_out_indent = 0;

	/* Close it */
	my_fclose(fff);


	/* Message */
	msg_print("Character sheet creation successful.");
	msg_print(NULL);

	/* Success */
	return (0);
}


/*
 * Recursive file perusal.
 *
 * Return FALSE on "ESCAPE", otherwise TRUE.
 *
 * Process various special text in the input file, including
 * the "menu" structures used by the "help file" system.
 *
 * XXX XXX XXX Consider using a temporary file.
 *
 * XXX XXX XXX Allow the user to "save" the current file.
 */

/*
 * A structure to hold (some of == XXX) the hyperlink information.
 * This prevents excessive use of stack.
 */
#define MAX_LINKS 1024
struct hyperlink
{
	/* Path buffer */
	char path[1024];

	/* General buffer */
	char rbuf[1024];

	/* Hold a string to find */
	char finder[81];

	/* Hold a string to show */
	char shower[81];

	/* Describe this thing */
	char caption[128];

	/* Hypertext info */
	char link[MAX_LINKS][32], link_key[MAX_LINKS];
	int link_x[MAX_LINKS], link_y[MAX_LINKS], link_line[MAX_LINKS];
};

typedef struct hyperlink hyperlink_type;

static bool_ show_file_aux(cptr name, cptr what, int line)
{
	int i, k, x;

	byte link_color = TERM_ORANGE, link_color_sel = TERM_YELLOW;

	/* Number of "real" lines passed by */
	int next = 0;

	/* Number of "real" lines in the file */
	int size = 0;

	/* Backup value for "line" */
	int back = 0;

	/* Color of the next line */
	byte color = TERM_WHITE;

	/* This screen has sub-screens */
	bool_ menu = FALSE;

	/* Current help file */
	FILE *fff = NULL;

	/* Find this string (if any) */
	cptr find = NULL;

	/* Pointer to general buffer in the above */
	char *buf;

	int cur_link = 0, max_link = 0;

	/* Read size of screen for big-screen stuff -pav- */
	int wid, hgt;

	/* Allocate hyperlink data */
	std::unique_ptr<hyperlink_type> h_ptr(new hyperlink_type);
	memset(h_ptr.get(), 0, sizeof(hyperlink_type));

	/* Setup buffer pointer */
	buf = h_ptr->rbuf;

	/* Wipe the links */
	for (i = 0; i < MAX_LINKS; i++)
	{
		h_ptr->link_x[i] = -1;
	}

	/* Hack XXX XXX XXX */
	if (what)
	{
		/* h_ptr->caption */
		strcpy(h_ptr->caption, what);

		/* Access the "file" */
		strcpy(h_ptr->path, name);

		/* Open */
		fff = my_fopen(h_ptr->path, "r");
	}

	/* Look in "help" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "Help file '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, name);

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");
	}

	/* Look in "info" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "Info file '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_INFO, name);

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");
	}

	/* Look in "file" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "File '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_FILE, name);

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");
	}

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg_format("Cannot open '%s'.", name);
		msg_print(NULL);

		/* Oops */
		return (TRUE);
	}


	/* Pre-Parse the file */
	while (TRUE)
	{
		/* Read a line or stop */
		if (my_fgets(fff, h_ptr->rbuf, 1024)) break;

		/* Get a color */
		if (prefix(h_ptr->rbuf, "#####"))
		{
			buf = &h_ptr->rbuf[6];
		}
		else buf = h_ptr->rbuf;

		/* Get the link colors */
		if (prefix(buf, "|||||"))
		{
			link_color = color_char_to_attr(buf[5]);
			link_color_sel = color_char_to_attr(buf[6]);
		}

		/* Tag ? */
		if (prefix(buf, "~~~~~"))
		{
			if (line < 0)
			{
				int i;
				char old_c;

				for (i = 5; (buf[i] >= '0') && (buf[i] <= '9'); i++)
					;
				old_c = buf[i];
				buf[i] = '\0';

				if (atoi(buf + 5) == -line)
				{
					line = next + 1;
				}
				buf[i] = old_c;
			}
		}

		x = 0;
		while (buf[x])
		{
			/* Hyperlink ? */
			if (prefix(buf + x, "*****"))
			{
				int xx = x + 5, stmp, xdeb = x + 5, z;
				char tmp[20];

				for (z = 0; z < 20; z++) tmp[z] = '\0';

				h_ptr->link_x[max_link] = x;
				h_ptr->link_y[max_link] = next;

				if (buf[xx] == '/')
				{
					xx++;
					h_ptr->link_key[max_link] = buf[xx];
					xx++;
					xdeb += 2;
				}
				else
				{
					h_ptr->link_key[max_link] = 0;
				}

				/* Zap the link info */
				while (buf[xx] != '*')
				{
					h_ptr->link[max_link][xx - xdeb] = buf[xx];
					xx++;
				}
				h_ptr->link[max_link][xx - xdeb] = '\0';
				xx++;
				stmp = xx;
				while (buf[xx] != '[')
				{
					tmp[xx - stmp] = buf[xx];
					xx++;
				}
				xx++;
				tmp[xx - stmp] = '\0';
				h_ptr->link_line[max_link] = -atoi(tmp);
				max_link++;
			}
			x++;
		}

		/* Count the "real" lines */
		next++;
	}

	/* Save the number of "real" lines */
	size = next;



	/* Display the file */
	while (TRUE)
	{
		/* Clear screen */
		Term_clear();

		Term_get_size(&wid, &hgt);

		/* Restart when necessary */
		if (line >= size) line = 0;


		/* Re-open the file if needed */
		if (next > line)
		{
			/* Close it */
			my_fclose(fff);

			/* Hack -- Re-Open the file */
			fff = my_fopen(h_ptr->path, "r");

			/* Oops */
			if (!fff)
			{
				return (FALSE);
			}

			/* File has been restarted */
			next = 0;
		}

		/* Skip lines if needed */
		for (; next < line; next++)
		{
			/* Skip a line */
			if (my_fgets(fff, buf, 1024)) break;
		}


		/* Dump the next 20 (or more in bigscreen) lines of the file */
		for (i = 0; i < (hgt - 4); )
		{
			int print_x;

			/* Hack -- track the "first" line */
			if (!i) line = next;

			/* Get a line of the file or stop */
			if (my_fgets(fff, h_ptr->rbuf, 1024)) break;

			/* Get a color */
			if (prefix(h_ptr->rbuf, "#####"))
			{
				color = color_char_to_attr(h_ptr->rbuf[5]);
				buf = &h_ptr->rbuf[6];
			}
			else buf = h_ptr->rbuf;

			/* Count the "real" lines */
			next++;

			/* Skip link colors */
			if (prefix(buf, "|||||")) continue;

			/* Skip tags */
			if (prefix(buf, "~~~~~"))
			{
				i++;
				continue;
			}

			/* Hack -- keep searching */
			if (find && !i && !strstr(buf, find)) continue;

			/* Hack -- stop searching */
			find = NULL;

			/* Be sure to get a correct cur_link */
			if (h_ptr->link_y[cur_link] >= line + (hgt - 4))
			{
				while ((cur_link > 0) && (h_ptr->link_y[cur_link] >= line + (hgt - 4)))
				{
					cur_link--;
				}
			}
			if (h_ptr->link_y[cur_link] < line)
			{
				while ((cur_link < max_link) && (h_ptr->link_y[cur_link] < line))
				{
					cur_link++;
				}
			}

			/* Dump the line */
			print_x = 0;
			if (!prefix(buf, "&&&&&"))
			{
				x = 0;
				while (buf[x])
				{
					/* Hyperlink ? */
					if (prefix(buf + x, "*****"))
					{
						int xx = x + 5;

						/* Zap the link info */
						while (buf[xx] != '[')
						{
							xx++;
						}
						xx++;
						/* Ok print the link name */
						while (buf[xx] != ']')
						{
							byte color = link_color;

							if ((h_ptr->link_x[cur_link] == x) && (h_ptr->link_y[cur_link] == line + i))
								color = link_color_sel;

							/* Now we treat the next char as printable */
							if (buf[xx] == '\\')
								xx++;

							Term_putch(print_x, i + 2, color, buf[xx]);
							xx++;
							print_x++;
						}
						x = xx;
					}
					/* Color ? */
					else if (prefix(buf + x, "[[[[["))
					{
						int xx = x + 6;

						/* Ok print the link name */
						while (buf[xx] != ']')
						{
							/* Now we treat the next char as printable */
							if (buf[xx] == '\\')
								xx++;
							Term_putch(print_x, i + 2, color_char_to_attr(buf[x + 5]), buf[xx]);
							xx++;
							print_x++;
						}
						x = xx;
					}
					/* Remove HTML ? */
					else if (prefix(buf + x, "{{{{{"))
					{
						int xx = x + 6;

						/* Ok remove this section */
						while (buf[xx] != '}')
						{
							xx++;
						}
						x = xx;
					}
					else
					{
						Term_putch(print_x, i + 2, color, buf[x]);
						print_x++;
					}

					x++;
				}
			}
			/* Verbatim mode: i.e: acacacac */
			else
			{
				x = 5;
				while (buf[x])
				{
					Term_putch(print_x, i + 2, color_char_to_attr(buf[x]), buf[x + 1]);
					print_x++;
					x += 2;
				}
			}
			color = TERM_WHITE;

			/* Hilite "h_ptr->shower" */
			if (h_ptr->shower[0])
			{
				cptr str = buf;

				/* Display matches */
				while ((str = strstr(str, h_ptr->shower)) != NULL)
				{
					int len = strlen(h_ptr->shower);

					/* Display the match */
					Term_putstr(str - buf, i + 2, len, TERM_YELLOW, h_ptr->shower);

					/* Advance */
					str += len;
				}
			}

			/* Count the printed lines */
			i++;
		}

		/* Hack -- failed search */
		if (find)
		{
			bell();
			line = back;
			find = NULL;
			continue;
		}


		/* Show a general "title" */
		prt(format("[%s, %s, Line %d/%d]", get_version_string(),
		           h_ptr->caption, line, size), 0, 0);

		/* Prompt -- menu screen */
		if (menu)
		{
			/* Wait for it */
			prt("[Press a Number, or ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- small files */
		else if (size <= (hgt - 4))
		{
			/* Wait for it */
			prt("[Press ESC to exit.]", hgt - 1, 0);
		}

		/* Prompt -- large files */
		else
		{
			/* Wait for it */
			prt("[Press 2, 8, 4, 6, /, =, #, %, backspace, or ESC to exit.]", hgt - 1, 0);
		}

		/* Get a keypress */
		k = inkey();

		/* Hack -- return to last screen */
		if ((k == '?') || (k == 0x7F) || (k == '\010')) break;

		/* Hack -- try showing */
		if (k == '=')
		{
			/* Get "h_ptr->shower" */
			prt("Show: ", hgt - 1, 0);
			askfor_aux(h_ptr->shower, 80);
		}

		/* Hack -- try finding */
		if (k == '/')
		{
			/* Get "h_ptr->finder" */
			prt("Find: ", hgt - 1, 0);
			if (askfor_aux(h_ptr->finder, 80))
			{
				/* Find it */
				find = h_ptr->finder;
				back = line;
				line = line + 1;

				/* Show it */
				strcpy(h_ptr->shower, h_ptr->finder);
			}
		}

		/* Hack -- go to a specific line */
		if (k == '#')
		{
			char tmp[81];
			prt("Goto Line: ", hgt - 1, 0);
			strcpy(tmp, "0");
			if (askfor_aux(tmp, 80))
			{
				line = atoi(tmp);
			}
		}

		/* Hack -- go to a specific file */
		if (k == '%')
		{
			char tmp[81];
			prt("Goto File: ", hgt - 1, 0);
			strcpy(tmp, "help.hlp");
			if (askfor_aux(tmp, 80))
			{
				if (!show_file_aux(tmp, NULL, 0)) k = ESCAPE;
			}
		}

		/* Hack -- Allow backing up */
		if (k == '-')
		{
			line = line - (hgt - 4);
			if (line < 0) line = 0;
		}

		if (k == '8')
		{
			line--;
			if (line < 0) line = 0;
		}

		/* Hack -- Advance a single line */
		if (k == '2')
		{
			line = line + 1;
		}

		/* Advance one page */
		if (k == ' ')
		{
			line = line + (hgt - 4);
		}

		/* Advance one link */
		if ((k == '6') || (k == '\t'))
		{
			cur_link++;
			if (cur_link >= max_link) cur_link = max_link - 1;

			if (h_ptr->link_y[cur_link] < line) line = h_ptr->link_y[cur_link];
			if (h_ptr->link_y[cur_link] >= line + (hgt - 4)) line = h_ptr->link_y[cur_link] - (hgt - 4);
		}
		/* Return one link */
		if (k == '4')
		{
			cur_link--;
			if (cur_link < 0) cur_link = 0;

			if (h_ptr->link_y[cur_link] < line) line = h_ptr->link_y[cur_link];
			if (h_ptr->link_y[cur_link] >= line + (hgt - 4)) line = h_ptr->link_y[cur_link] - (hgt - 4);
		}

		/* Recurse on numbers */
		if (k == '\r')
		{
			if (h_ptr->link_x[cur_link] != -1)
			{
				/* Recurse on that file */
				if (!show_file_aux(h_ptr->link[cur_link], NULL, h_ptr->link_line[cur_link])) k = ESCAPE;
			}
		}

		/* Exit on escape */
		if (k == ESCAPE) break;

		/* No other key ? lets look for a shortcut */
		for (i = 0; i < max_link; i++)
		{
			if (h_ptr->link_key[i] == k)
			{
				/* Recurse on that file */
				if (!show_file_aux(h_ptr->link[i], NULL, h_ptr->link_line[i])) k = ESCAPE;
				break;
			}
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Escape */
	if (k == ESCAPE) return (FALSE);

	/* Normal return */
	return (TRUE);
}

void show_string(const char *lines, const char *title, int line)
{
	// Temporary file
	auto const file_name = boost::filesystem::unique_path().string();

	// Open a new file
	std::ofstream ofs(file_name);
	ofs.exceptions(std::ofstream::failbit);
	ofs << lines;
	ofs.close();

	// Display the file contents
	show_file_aux(file_name.c_str(), title, line);

	// Remove the file
	fd_kill(file_name.c_str());
}

void show_file(cptr name, cptr what, int line)
{
	show_file_aux(name, what, line);
}

static void cmovie_clean_line(int y, char *abuf, char *cbuf)
{
	const byte *ap = Term->scr->a[y];
	const char *cp = Term->scr->c[y];

	byte a;
	char c;

	int x;
	int wid, hgt;
	int screen_wid, screen_hgt;


	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	/* Calculate the size of dungeon map area */
	screen_wid = wid - (COL_MAP + 1);
	screen_hgt = hgt - (ROW_MAP + 1);

	/* For the time being, assume 80 column display XXX XXX XXX */
	for (x = 0; x < wid; x++)
	{
		/* Convert dungeon map into default attr/chars */
		if (!character_icky &&
				((x - COL_MAP) >= 0) &&
				((x - COL_MAP) < screen_wid) &&
				((y - ROW_MAP) >= 0) &&
				((y - ROW_MAP) < screen_hgt))
		{
			/* Retrieve default attr/char */
			map_info_default(y + panel_row_prt, x + panel_col_prt, &a, &c);

			abuf[x] = conv_color[a & 0xf];

			if (c == '\0') cbuf[x] = ' ';
			else cbuf[x] = c;
		}

		else
		{
			abuf[x] = conv_color[ap[x] & 0xf];
			cbuf[x] = cp[x];
		}
	}

	/* Null-terminate the prepared strings */
	abuf[x] = '\0';
	cbuf[x] = '\0';
}

/* Take an help file screenshot(yes yes I know..) */
void help_file_screenshot(cptr name)
{
	int y, x;
	int wid, hgt;

	byte a = 0;
	char c = ' ';

	FILE *htm;

	char buf[1024];

	/* The terms package supports up to 255x255 screen size */
	char abuf[256];
	char cbuf[256];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* Append to the file */
	htm = my_fopen(buf, "w");

	/* Oops */
	if (!htm) return;

	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	/* Dump the screen */
	for (y = 0; y < hgt; y++)
	{
		cmovie_clean_line(y, abuf, cbuf);

		/* Dump each row */
		fprintf(htm, "&&&&&");
		for (x = 0; x < wid; x++)
		{
			a = abuf[x];
			c = cbuf[x];

			fprintf(htm, "%c%c", a, c);
		}

		/* End the row */
		fprintf(htm, "\n");
	}

	/* Close it */
	my_fclose(htm);
}

/* Take an html screenshot */
void html_screenshot(cptr name)
{
	int y, x;
	int wid, hgt;

	byte a = 0, oa = TERM_WHITE;
	char c = ' ';

	FILE *htm;

	char buf[1024];

	/* The terms package supports up to 255x255 screen size */
	char abuf[256];
	char cbuf[256];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* Append to the file */
	htm = my_fopen(buf, "w");

	/* Oops */
	if (!htm) return;

	/* Retrieve current screen size */
	Term_get_size(&wid, &hgt);

	fprintf(htm, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
	             "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">\n"
	             "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	             "<head>\n");
	fprintf(htm, "<meta name=\"GENERATOR\" content=\"%s\"/>\n",
	        get_version_string());
	fprintf(htm, "<title>%s</title>\n", name);
	fprintf(htm, "</head>\n"
	             "<body>\n"
	             "<pre style=\"color: #ffffff; background-color: #000000; font-family: monospace\">\n");
	fprintf(htm, "<span style=\"color: #%02X%02X%02X\">\n",
	        angband_color_table[TERM_WHITE][1],
	        angband_color_table[TERM_WHITE][2],
	        angband_color_table[TERM_WHITE][3]);

	/* Dump the screen */
	for (y = 0; y < hgt; y++)
	{
		cmovie_clean_line(y, abuf, cbuf);

		/* Dump each row */
		for (x = 0; x < wid; x++)
		{
			a = color_char_to_attr(abuf[x]);
			c = cbuf[x];

			if (oa != a)
			{
				fprintf(htm, "</span><span style=\"color: #%02X%02X%02X\">", angband_color_table[a][1], angband_color_table[a][2], angband_color_table[a][3]);
				oa = a;
			}
			if (c == '<')
				fprintf(htm, "&lt;");
			else if (c == '>')
				fprintf(htm, "&gt;");
			else if (c == '&')
				fprintf(htm, "&amp;");
			else
				fprintf(htm, "%c", c);
		}

		/* End the row */
		fprintf(htm, "\n");
	}
	fprintf(htm, "</span>\n"
	             "</pre>\n"
	             "</body>\n"
	             "</html>\n");

	/* Close it */
	my_fclose(htm);
}


/*
 * Peruse the On-Line-Help
 */
void do_cmd_help()
{
	/* Save screen */
	screen_save();

	/* Peruse the main help file */
	show_file("help.hlp", NULL);

	/* Load screen */
	screen_load();
}




void process_player_base()
{
	path_build(savefile, 1024, ANGBAND_DIR_SAVE, game->player_base.c_str());
}

void process_player_name(bool_ sf)
{
	/* Cannot be too long */
	if (game->player_base.size() > 15)
	{
		quit_fmt("The name '%s' is too long!", game->player_base.c_str());
	}

	/* Cannot contain control characters */
	for (auto c : game->player_base)
	{
		if (iscntrl(c))
		{
			quit_fmt("The name '%s' contains control chars!", game->player_base.c_str());
		}
	}

	/* Extract "useful" letters */
	std::string buf;
	for (auto c : game->player_base)
	{
		/* Accept some letters */
		if (isalpha(c) || isdigit(c))
		{
			buf += c;
		}

		/* Convert space, dot, and underscore to underscore */
		else if (strchr("@. _", c))
		{
			buf += '_';
		}
	}

	/* Terminate */
	game->player_base = buf;

	/* Require a "base" name */
	if (game->player_base.empty())
	{
		game->player_base = "PLAYER";
	}

	/* Change the savefile name */
	if (sf)
	{
		process_player_base();
	}
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Assumes that "display_player(0)" has just been called
 *
 * Perhaps we should NOT ask for a name (at "birth()") on
 * Unix machines?  XXX XXX
 *
 * What a horrible name for a global function.  XXX XXX XXX
 */
void get_name()
{
	char tmp[32];

	/* Clear last line */
	clear_from(22);

	/* Prompt and ask */
	prt("[Enter your player's name above, or hit ESCAPE]", 23, 2);

	/* Ask until happy */
	while (1)
	{
		/* Go to the "name" field */
		move_cursor(2, 9);

		/* Save the player name */
		strcpy(tmp, game->player_name.c_str());

		/* Get an input, ignore "Escape" */
		if (askfor_aux(tmp, 31))
		{
			game->player_name = tmp;
		}

		/* Process the player name */
		process_player_name(FALSE);

		/* All done */
		break;
	}

	/* Pad the name (to clear junk) */
	sprintf(tmp, "%-31.31s", game->player_name.c_str());

	/* Re-Draw the name (in light blue) */
	c_put_str(TERM_L_BLUE, tmp, 2, 9);

	/* Erase the prompt, etc */
	clear_from(22);
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide()
{
	int i;

	/* Flush input */
	flush();

	/* Verify Retirement */
	if (total_winner)
	{
		/* Verify */
		if (!get_check("Do you want to retire? ")) return;
	}

	/* Verify Suicide */
	else
	{
		/* Verify */
		if (!get_check("Do you really want to quit? ")) return;

		if (!noscore)
		{
			/* Special Verification for suicide */
			prt("Please verify QUITTING by typing the '@' sign: ", 0, 0);
			flush();
			i = inkey();
			prt("", 0, 0);
			if (i != '@') return;
		}
	}

	/* Stop playing */
	alive = FALSE;

	/* Kill the player */
	death = TRUE;

	/* Leaving */
	p_ptr->leaving = TRUE;

	/* Cause of death */
	game->died_from = "Quitting";
}


	/* HACK - Remove / set the CAVE_VIEW flag, since view_x / view_y
	 * is not saved, and the visible locations are not lighted correctly
	 * when the game is loaded again
	 * Alternatively forget_view() and update_view() can be used
	 */
void remove_cave_view(bool_ remove)
{
	int i;
	cave_type *c_ptr;

	if (view_n)
	{
		/* Clear them all */
		for (i = 0; i < view_n; i++)
		{
			int y = view_y[i];
			int x = view_x[i];

			/* Access the grid */
			c_ptr = &cave[y][x];

			if (remove)
				c_ptr->info &= ~(CAVE_VIEW);
			else
				c_ptr->info |= (CAVE_VIEW);
		}
	}
}

/*
 * Save the game
 */
void do_cmd_save_game()
{
	remove_cave_view(TRUE);

	/* Save the current level if in a persistent level */
	save_dungeon();

	/* Autosaves do not disturb */
	if (!is_autosave)
	{
		/* Disturb the player */
		disturb();
	}

	/* Clear messages */
	msg_print(NULL);

	/* Handle stuff */
	handle_stuff();

	/* Message */
	prt("Saving game...", 0, 0);

	/* Refresh */
	Term_fresh();

	/* The player is not dead */
	game->died_from = "(saved)";

	/* Save the player */
	if (save_player())
	{
		prt("Saving game... done.", 0, 0);
	}

	/* Save failed (oops) */
	else
	{
		prt("Saving game... failed!", 0, 0);
	}

	remove_cave_view(FALSE);

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	game->died_from = "(alive and well)";
}

/*
 * Auto-save depending on whether the auto save flag is set.
 */
void autosave_checkpoint()
{
	if (options->autosave_l)
	{
		is_autosave = TRUE;
		msg_print("Autosaving the game...");
		do_cmd_save_game();
		is_autosave = FALSE;
	}
}

/*
 * Hack -- Calculates the total number of points earned                -JWT-
 */
static long total_points()
{
	auto const &d_info = game->edit_data.d_info;
	auto const &r_info = game->edit_data.r_info;

	s16b max_dl = 0;
	long temp, Total = 0;
	long mult = 20; /* was 100. Divided values by 5 because of an overflow error */
	long comp_death = (p_ptr->companion_killed * 2 / 5);

	if (!comp_death) comp_death = 1;

	if (options->preserve) mult -= 1;  /* Penalize preserve, maximize modes */
	mult -= 1; /* maximize pentalty, always on */
	if (options->auto_scum) mult -= 4;
	if (options->small_levels) mult += ((options->always_small_level) ? 4 : 10);
	if (options->empty_levels) mult += 2;
	if (options->smart_learn) mult += 4;

	if (mult < 2) mult = 2;  /* At least 10% of the original score */
	/* mult is now between 2 and 40, i.e. 10% and 200% */

	for (std::size_t i = 0; i < d_info.size(); i++)
	{
		if (max_dlv[i] > max_dl)
		{
			max_dl = max_dlv[i];
		}
	}

	temp = p_ptr->lev * p_ptr->lev * p_ptr->lev * p_ptr->lev + (100 * max_dl);

	temp += p_ptr->max_exp / 5;

	temp = (temp * mult / 20);

	/* Gold increases score */
	temp += p_ptr->au / 5;

	/* Completing quest increase score */
	for (std::size_t i = 0; i < MAX_Q_IDX; i++)
	{
		if (quest[i].status >= QUEST_STATUS_COMPLETED)
		{
			temp += 2000;
			temp += quest[i].level * 100;
		}
	}

	/* Death of a companion is BAD */
	temp /= comp_death;

	for (auto const &r_ref: r_info)
	{
		auto r_ptr = &r_ref;

		if (r_ptr->flags & RF_UNIQUE)
		{
			bool_ dead = (r_ptr->max_num == 0);

			if (dead)
			{
				/* Uniques are supposed to be harder */
				Total += 50;
			}
		}
		else
		{
			s16b This = r_ptr->r_pkills;

			if (This > 0)
			{
				Total += This;
			}
		}
	}
	temp += Total * 50;

	if (total_winner) temp += 1000000;



	return (temp);
}


/*
 * Display a "tomb-stone"
 */
static void print_tomb()
{
	time_t ct = time(nullptr);

	auto center = [](std::string const &s) -> std::string {
		return fmt::format("{:^31s}", s);
	};

	/* Clear screen */
	Term_clear();

	/* Build the filename */
	char buf[1024];
	path_build(buf, 1024, ANGBAND_DIR_FILE, "dead.txt");

	/* Open the News file */
	FILE *fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, 1024))
		{
			/* Display and advance */
			display_message(0, i++, strlen(buf), TERM_WHITE, buf);
		}

		/* Close */
		my_fclose(fp);
	}

	std::string p_title;
	if (total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p_title = "Magnificent";
	}
	else
	{
		p_title = cp_ptr->titles[(p_ptr->lev - 1) / 5];
	}

	put_str(center(game->player_name), 6, 11);
	put_str(center("the"), 7, 11);
	put_str(center(p_title), 8, 11);
	put_str(center(spp_ptr->title), 10, 11);
	put_str(center(fmt::format("Level: {}", p_ptr->lev)), 11, 11);
	put_str(center(fmt::format("Exp: {}", p_ptr->exp)), 12, 11);
	put_str(center(fmt::format("AU: {}", p_ptr->au)), 13, 11);
	put_str(center(fmt::format("Killed on Level {}", dun_level)), 14, 11);
	put_str(center(fmt::format("by {}.", game->died_from.substr(0, 24))), 15, 11);
	put_str(center(std::string(ctime(&ct)).substr(0, 24)), 17, 11);
}


/*
 * Display some character info
 */
static void show_info()
{
	/* Hack -- Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Flush all input keys */
	flush();

	/* Flush messages */
	msg_print(NULL);


	/* Describe options */
	prt("You may now dump a character record to one or more files.", 21, 0);
	prt("Then, hit RETURN to see the character, or ESC to abort.", 22, 0);

	/* Dump character records as requested */
	while (TRUE)
	{
		char out_val[160];

		/* Prompt */
		put_str("Filename(you can post it to http://angband.oook.cz/): ", 23, 0);

		/* Default */
		strcpy(out_val, "");

		/* Ask for filename (or abort) */
		if (!askfor_aux(out_val, 60)) return;

		/* Return means "show on screen" */
		if (!out_val[0]) break;

		/* Save screen */
		character_icky = TRUE;
		Term_save();

		/* Dump a character file */
		file_character(out_val);

		/* Load screen */
		Term_load();
		character_icky = FALSE;
	}


	/* Display player */
	display_player(0);

	/* Prompt for p_ptr->inventory */
	prt("Hit any key to see more information (ESC to abort): ", 23, 0);

	/* Allow abort at this point */
	if (inkey() == ESCAPE) return;


	/* Show equipment and inventory */

	/* Equipment -- if any */
	if (equip_cnt)
	{
		Term_clear();
		show_equip_full();
		prt("You are using: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (inven_cnt)
	{
		Term_clear();
		show_inven_full();
		prt("You are carrying: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Homes in the different towns */
	for (int k = 1; k < max_towns; k++)
	{
		store_type *st_ptr = &town_info[k].store[7];

		/* Home -- if anything there */
		if (!st_ptr->stock.empty())
		{
			std::size_t i;
			/* Display contents of the home */
			for (k = 0, i = 0; i < st_ptr->stock.size(); k++)
			{
				/* Clear screen */
				Term_clear();

				/* Show 12 items */
				for (int j = 0; (j < 12) && (i < st_ptr->stock.size()); j++, i++)
				{
					char o_name[80];
					char tmp_val[80];

					/* Acquire item */
					auto o_ptr = &st_ptr->stock[i];

					/* Print header, clear line */
					sprintf(tmp_val, "%c) ", I2A(j));
					prt(tmp_val, j + 2, 4);

					/* Display object description */
					object_desc(o_name, o_ptr, TRUE, 3);
					c_put_str(tval_to_attr[o_ptr->tval], o_name, j + 2, 7);
				}

				/* h_ptr->caption */
				prt(format("Your home contains (page %d): -more-", k + 1), 0, 0);

				/* Wait for it */
				if (inkey() == ESCAPE) return;
			}
		}
	}
}





/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
static void display_scores_aux(int highscore_fd, int from, int to, int note, high_score *score)
{
	auto const &class_info = game->edit_data.class_info;

	int i, j, k, n, place;
	byte attr;
	char out_val[256];
	char tmp_val[160];
	high_score the_score;


	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return;


	/* Assume we will show the first 10 */
	if (from < 0) from = 0;
	if (to < 0) to = 10;
	if (to > MAX_HISCORES) to = MAX_HISCORES;


	/* Seek to the beginning */
	if (highscore_seek(highscore_fd, 0)) return;

	/* Hack -- Count the high scores */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(highscore_fd, &the_score)) break;
	}

	/* Hack -- allow "fake" entry to be last */
	if ((note == i) && score) i++;

	/* Forget about the last entries */
	if (i > to) i = to;


	/* Show 5 per page, until "done" */
	for (k = from, place = k + 1; k < i; k += 5)
	{
		/* Clear screen */
		Term_clear();

		/* Title */
		put_str(format("              %s Hall of Fame", game_module), 0, 0);

		/* Indicate non-top scores */
		if (k > 0)
		{
			sprintf(tmp_val, "(from position %d)", k + 1);
			put_str(tmp_val, 0, 40);
		}

		/* Dump 5 entries */
		for (j = k, n = 0; j < i && n < 5; place++, j++, n++)
		{
			int pcs, pr, ps, pc, clev, mlev, cdun, mdun;

			cptr gold, when, aged;

			int in_quest;

			/* Hack -- indicate death in yellow */
			attr = (j == note) ? TERM_YELLOW : TERM_WHITE;


			/* Mega-Hack -- insert a "fake" record */
			if ((note == j) && score)
			{
				the_score = (*score);
				attr = TERM_L_GREEN;
				score = NULL;
				note = -1;
				j--;
			}

			/* Read a normal record */
			else
			{
				/* Read the proper record */
				if (highscore_seek(highscore_fd, j)) break;
				if (highscore_read(highscore_fd, &the_score)) break;
			}

			/* Extract the race/class */
			pr = atoi(the_score.p_r);
			ps = atoi(the_score.p_s);
			pc = atoi(the_score.p_c);
			pcs = atoi(the_score.p_cs);

			/* Extract the level info */
			clev = atoi(the_score.cur_lev);
			mlev = atoi(the_score.max_lev);
			cdun = atoi(the_score.cur_dun);
			mdun = atoi(the_score.max_dun);

			in_quest = atoi(the_score.inside_quest);

			/* Hack -- extract the gold and such */
			for (when = the_score.day; isspace(*when); when++) /* loop */;
			for (gold = the_score.gold; isspace(*gold); gold++) /* loop */;
			for (aged = the_score.turns; isspace(*aged); aged++) /* loop */;

			/* Dump some info */
			auto const player_race_name = get_player_race_name(pr, ps);
			sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
				place,
				the_score.pts,
				the_score.who,
				player_race_name.c_str(),
				class_info[pc].spec[pcs].title,
			        clev);

			/* Append a "maximum level" */
			if (mlev > clev) strcat(out_val, format(" (Max %d)", mlev));

			/* Dump the first line */
			c_put_str(attr, out_val, n*4 + 2, 0);

			/* Another line of info */
			if (in_quest)
			{
				sprintf(out_val, "               Killed by %s while questing",
				        the_score.how);
			}
			/* Hack -- some people die in the town */
			else if (!cdun)
			{
				sprintf(out_val, "               Killed by %s in the Town",
				        the_score.how);
			}
			else
			{
				sprintf(out_val, "               Killed by %s on %s %d",
				        the_score.how, "Dungeon Level", cdun);
			}

			/* Append a "maximum level" */
			if (mdun > cdun) strcat(out_val, format(" (Max %d)", mdun));

			/* Dump the info */
			c_put_str(attr, out_val, n*4 + 3, 0);

			/* And still another line of info */
			sprintf(out_val,
			        "               (Date %s, Gold %s, Turn %s).",
			        when, gold, aged);
			c_put_str(attr, out_val, n*4 + 4, 0);
		}


		/* Wait for response */
		prt("[Press ESC to quit, any other key to continue.]", 23, 17);
		j = inkey();
		prt("", 23, 0);

		/* Hack -- notice Escape */
		if (j == ESCAPE) break;
	}
}


/*
 * show_highclass - selectively list highscores based on class
 * -KMW-
 */
void show_highclass(int building)
{
	auto const &race_info = game->edit_data.race_info;

	int i = 0, j, m = 0;
	int pr, pc, clev;
	high_score the_score;
	char buf[1024];
	int highscore_fd;

	switch (building)
	{
	case 1:
		prt("               Busts of Greatest Kings", 5, 0);
		break;
	case 2:
		prt("               Plaque - Greatest Arena Champions", 5, 0);
		break;
	case 10:
		prt("               Plaque - Greatest Fighters", 5, 0);
		break;
	case 11:
		prt("               Spires of the Greatest Magic-Users", 5, 0);
		break;
	case 12:
		prt("               Busts of Greatest Priests", 5, 0);
		break;
	case 13:
		prt("               Wall Inscriptions - Greatest Thieves", 5, 0);
		break;
	case 14:
		prt("               Plaque - Greatest Rangers", 5, 0);
		break;
	case 15:
		prt("               Plaque - Greatest Paladins", 5, 0);
		break;
	case 16:
		prt("               Spires of the Greatest Illusionists", 5, 0);
		break;
	default:
		bell();
		break;
	}

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "scores.raw");

	/* Open file */
	highscore_fd = fd_open(buf, O_RDONLY);

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return;
	}

	if (highscore_seek(highscore_fd, 0)) return;

	for (i = 0; i < MAX_HISCORES; i++)
		if (highscore_read(highscore_fd, &the_score)) break;

	m = 0;
	j = 0;
	clev = 0;

	auto const format_num = "{:>3d}) {} the {} (Level {:>2d})"; // See also race_score()
	auto const format_you = "You) {} the {} (Level {:>2d})";

	while ((m < 9) || (j < MAX_HISCORES))
	{
		if (highscore_seek(highscore_fd, j)) break;
		if (highscore_read(highscore_fd, &the_score)) break;
		pr = atoi(the_score.p_r);
		pc = atoi(the_score.p_c);
		clev = atoi(the_score.cur_lev);
		if (((pc == (building - 10)) && (building != 1)) ||
				((building == 1) && (clev >= PY_MAX_LEVEL)))
		{
			auto out_val = fmt::format(format_num,
				(m + 1), the_score.who, race_info[pr].title, clev);
			prt(out_val.c_str(), (m + 7), 0);
			m++;
		}
		j++;
	}

	/* Now, list the active player if they qualify */
	if ((building == 1) && (p_ptr->lev >= PY_MAX_LEVEL))
	{
		auto out_val = fmt::format(format_you,
			game->player_name,
			race_info[p_ptr->prace].title,
			p_ptr->lev);
		prt(out_val.c_str(), (m + 8), 0);
	}
	else if ((building != 1))
	{
		if ((p_ptr->lev > clev) && (p_ptr->pclass == (building - 10)))
		{
			auto out_val = fmt::format(format_you,
				game->player_name,
				race_info[p_ptr->prace].title,
				p_ptr->lev);
			prt(out_val.c_str(), (m + 8), 0);
		}
	}

	fd_close(highscore_fd);

	msg_print("Hit any key to continue");
	msg_print(NULL);
	for (j = 5; j < 18; j++)
		prt("", j, 0);
}


/*
 * Race Legends
 * -KMW-
 */
void race_score(int race_num)
{
	auto const &race_info = game->edit_data.race_info;

	int i = 0, j, m = 0;
	int pr, clev, lastlev;
	high_score the_score;
	char buf[1024], tmp_str[80];
	int highscore_fd;

	lastlev = 0;

	/* rr9: TODO - pluralize the race */
	sprintf(tmp_str, "The Greatest of all the %s", race_info[race_num].title.c_str());
	prt(tmp_str, 5, 3);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "scores.raw");

	/* Open the highscore file */
	highscore_fd = fd_open(buf, O_RDONLY);

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return;
	}

	if (highscore_seek(highscore_fd, 0)) return;

	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(highscore_fd, &the_score)) break;
	}

	m = 0;
	j = 0;

	auto const format_num = "{:>3d}) {} the {} (Level {:>2d})"; // See also show_highclass()
	auto const format_you = "You) {} the {} (Level {:>2d})";

	while ((m < 10) && (j < i))
	{
		if (highscore_seek(highscore_fd, j)) break;
		if (highscore_read(highscore_fd, &the_score)) break;
		pr = atoi(the_score.p_r);
		clev = atoi(the_score.cur_lev);
		if (pr == race_num)
		{
			auto out_val = fmt::format(format_num,
				(m + 1),
				the_score.who,
				race_info[pr].title,
				clev);
			prt(out_val.c_str(), (m + 7), 0);
			m++;
			lastlev = clev;
		}
		j++;
	}

	/* add player if qualified */
	if ((p_ptr->prace == race_num) && (p_ptr->lev >= lastlev))
	{
		auto out_val = fmt::format(format_you,
			game->player_name,
			race_info[p_ptr->prace].title,
			p_ptr->lev);
		prt(out_val.c_str(), (m + 8), 0);
	}

	fd_close(highscore_fd);
}


/*
 * Race Legends
 * -KMW-
 */
void race_legends()
{
	auto const &race_info = game->edit_data.race_info;

	for (size_t i = 0; i < race_info.size(); i++)
	{
		race_score(i);
		msg_print("Hit any key to continue");
		msg_print(NULL);
		for (int j = 5; j < 19; j++)
		{
			prt("", j, 0);
		}
	}
}




/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 */
static errr top_twenty()
{
	int j;

	high_score the_score;

	time_t ct = time((time_t*)0);

	char buf[1024];

	int highscore_fd = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores.raw");

	/* Open the highscore file, for reading/writing */
	highscore_fd = fd_open(buf, O_RDWR);

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		goto out;
	}

	/* Clear screen */
	Term_clear();

	/* Wizard-mode pre-empts scoring */
	if (noscore & 0x000F)
	{
		msg_print("Score not registered for wizards.");
		msg_print(NULL);
		display_scores_aux(highscore_fd, 0, 10, -1, NULL);
		goto out;
	}

	/* Cheaters are not scored */
	if (noscore & 0xFF00)
	{
		msg_print("Score not registered for cheaters.");
		msg_print(NULL);
		display_scores_aux(highscore_fd, 0, 10, -1, NULL);
		goto out;
	}

	/* Quitter */
	if (!total_winner && (game->died_from == "Quitting"))
	{
		msg_print("Score not registered due to quitting.");
		msg_print(NULL);
		display_scores_aux(highscore_fd, 0, 10, -1, NULL);
		goto out;
	}


	/* Clear the record */
	static_assert(std::is_pod<high_score>::value,
		      "Cannot memset a non-POD type");
	memset(&the_score, 0, sizeof(high_score));

	/* Save the version */
	sprintf(the_score.what, "%ld.%ld.%ld",
	        (long int) VERSION_MAJOR, (long int) VERSION_MINOR, (long int) VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn);
	the_score.turns[9] = '\0';

	/* Save the date in standard form (8 chars) */
	strftime(the_score.day, 9, "%m/%d/%y", localtime(&ct));

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", game->player_name.c_str());

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_s, "%2d", p_ptr->pracem);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_cs, "%2d", p_ptr->pspec);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", dun_level);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", max_dlv[dungeon_type]);

	sprintf(the_score.inside_quest, "%3d", p_ptr->inside_quest);

	/* Save the cause of death (31 chars) */
	sprintf(the_score.how, "%-.31s", game->died_from.c_str());


	/* Add a new entry to the score list, see where it went */
	j = highscore_add(highscore_fd, &the_score);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(highscore_fd, 0, 15, j, NULL);
	}

	/* Display the scores surrounding the player */
	else
	{
		display_scores_aux(highscore_fd, 0, 5, j, NULL);
		display_scores_aux(highscore_fd, j - 2, j + 7, j, NULL);
	}


out:
	if (highscore_fd >= 0)
	{
		fd_close(highscore_fd);
	}

	/* Success */
	return (0);
}


/*
 * Predict the players location, and display it.
 */
static errr predict_score()
{
	int j;

	high_score the_score;

	char buf[1024];

	int highscore_fd = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores.raw");

	/* Open the highscore file */
	highscore_fd = fd_open(buf, O_RDONLY);

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		goto out;
	}

	/* Clear the record */
	static_assert(std::is_pod<high_score>::value,
		      "Cannot memset a non-POD type");
	memset(&the_score, 0, sizeof(high_score));

	/* Save the version */
	sprintf(the_score.what, "%ld.%ld.%ld",
	        (long int) VERSION_MAJOR, (long int) VERSION_MINOR, (long int) VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn);
	the_score.turns[9] = '\0';

	/* Hack -- no time needed */
	strcpy(the_score.day, "TODAY");

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", game->player_name.c_str());

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_s, "%2d", p_ptr->pracem);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_cs, "%2d", p_ptr->pspec);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", dun_level);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", max_dlv[dungeon_type]);

	sprintf(the_score.inside_quest, "%3d", p_ptr->inside_quest);

	/* Hack -- no cause of death */
	strcpy(the_score.how, "nobody (yet!)");


	/* See where the entry would be placed */
	j = highscore_where(highscore_fd, &the_score);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(highscore_fd, 0, 15, j, &the_score);
	}

	/* Display some "useful" scores */
	else
	{
		display_scores_aux(highscore_fd, 0, 5, -1, NULL);
		display_scores_aux(highscore_fd, j - 2, j + 7, j, &the_score);
	}

out:
	if (highscore_fd >= 0)
	{
		fd_close(highscore_fd);
	}

	/* Success */
	return (0);
}


void predict_score_gui(bool_ *initialized_p, bool_ *game_in_progress_p)
{
	char buf[1024];
	int highscore_fd;

	/* Paranoia */
	if (!(*initialized_p) || character_icky ||
	    !(*game_in_progress_p) || !character_generated)
	{
		/* Can't happen but just in case */
		plog("You may not do that right now.");
		return;
	}

	/* Build the pathname of the score file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "scores.raw");

	/* Hack - open the score file for reading */
	highscore_fd = fd_open(buf, O_RDONLY);

	/* Paranoia - No score file */
	if (highscore_fd < 0)
	{
		msg_print("Score file is not available.");
		return;
	}

	/* Mega-Hack - prevent various functions XXX XXX XXX */
	*initialized_p = FALSE;

	/* Save screen */
	screen_save();

	/* Clear screen */
	Term_clear();

	/* Prepare scores */
	if ((*game_in_progress_p) && character_generated)
	{
		predict_score();
	}

	/* Close the high score file */
	fd_close(highscore_fd);

	/* Forget the fd */
	highscore_fd = -1;

	/* Restore screen */
	screen_load();

	/* Hack - Flush it */
	Term_fresh();

	/* Mega-Hack - We are ready again */
	*initialized_p = TRUE;
}


/*
 * Change the player into a King!                        -RAK-
 */
static void kingly()
{
	/* Hack -- retire in town */
	dun_level = 0;

	/* Fake death */
	game->died_from = "Ripe Old Age";

	/* Restore the experience */
	p_ptr->exp = p_ptr->max_exp;

	/* Restore the level */
	p_ptr->lev = p_ptr->max_plv;

	/* Hack -- Instant Gold */
	p_ptr->au += 10000000L;

	/* Clear screen */
	Term_clear();

	/* Would like to see something more Tolkienian here... */

	/* Display a crown */
	put_str("#", 1, 34);
	put_str("#####", 2, 32);
	put_str("#", 3, 34);
	put_str(",,,  $$$  ,,,", 4, 28);
	put_str(",,=$   \"$$$$$\"   $=,,", 5, 24);
	put_str(",$$        $$$        $$,", 6, 22);
	put_str("*>         <*>         <*", 7, 22);
	put_str("$$         $$$         $$", 8, 22);
	put_str("\"$$        $$$        $$\"", 9, 22);
	put_str("\"$$       $$$       $$\"", 10, 23);
	put_str("*#########*#########*", 11, 24);
	put_str("*#########*#########*", 12, 24);

	/* Display a message */
	put_str("Veni, Vidi, Vici!", 15, 26);
	put_str("I came, I saw, I conquered!", 16, 21);
	put_str(format("All Hail the Mighty %s!", game->player_name.c_str()), 17, 22);

	/* Flush input */
	flush();

	/* Wait for response */
	pause_line(23);
}


/*
 * Wipe the saved levels
 */
void wipe_saved()
{
	auto const &d_info = game->edit_data.d_info;

	int od = dungeon_type;
	int ol = dun_level;

	for (std::size_t d = 0; d < d_info.size(); d++)
	{
		auto d_ptr = &d_info[d];

		for (auto l = d_ptr->mindepth; l <= d_ptr->maxdepth; l++)
		{
			char buf[10];

			dun_level = l;
			dungeon_type = d;
			if (get_dungeon_save(buf))
			{
				auto tmp = fmt::format("{}.{}", game->player_base, buf);

				char name[1024];
				path_build(name, 1024, ANGBAND_DIR_SAVE, tmp.c_str());

				/* Remove the dungeon save file */
				fd_kill(name);
			}
		}
	}

	dungeon_type = od;
	dun_level = ol;
}


/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 */
void close_game()
{
	/* Handle stuff */
	handle_stuff();

	/* Flush the messages */
	msg_print(NULL);

	/* Flush the input */
	flush();


	/* Hack -- Character is now "icky" */
	character_icky = TRUE;


	/* Handle death */
	if (death)
	{
		/* Handle retirement */
		if (total_winner)
		{
			/* Make a note */
			add_note_type(NOTE_WINNER);

			kingly();
		}

		/* Wipe the saved levels */
		wipe_saved();

		/* Save memories */
		if (!save_player()) msg_print("Death save failed!");

		/* You are dead */
		print_tomb();

		/* Show more info */
		show_info();

		/* Make a note */
		{
			char long_day[30];
			time_t ct = time((time_t*)NULL);

			/* Get the date */
			strftime(long_day, 30,
			         "%Y-%m-%d at %H:%M:%S", localtime(&ct));

			/* Create string */
			auto buf = fmt::format("\n{} was killed by {} on {}\n",
				game->player_name,
				game->died_from,
				long_day);

			/* Output to the notes file */
			output_note(buf.c_str());
		}

		/* Handle score, show Top scores */
		top_twenty();
	}

	/* Still alive */
	else
	{
		is_autosave = FALSE;

		/* Save the game */
		do_cmd_save_game();

		/* Make a note pf session end */
		add_note_type(NOTE_SAVE_GAME);

		/* Prompt for scores XXX XXX XXX */
		prt("Press Return (or Escape).", 0, 40);

		/* Predict score (or ESCAPE) */
		if (inkey() != ESCAPE) predict_score();
	}
}


/*
 * Grab a randomly selected line in lib/file/file_name
 */
errr get_rnd_line(const char *file_name, char *output)
{
	FILE *fp;

	char buf[1024];

	int lines = 0;

	int line;

	int i;


	/* Clear the output buffer */
	strcpy(output, "");

	/* test hack */
	if (wizard && options->cheat_xtra) msg_print(file_name);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, file_name);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Failed */
	if (!fp) return ( -1);

	/* Read the first line */
	if (0 != my_fgets(fp, buf, 80))
	{
		my_fclose(fp);
		return ( -1);
	}

	/* Retrieve number of valid lines in the file */
	lines = atoi(buf);

	/* Pick a line in the file */
	line = randint(lines);

	/*
	 * Scan through the file XXX XXX XXX
	 * Seemingly wrong use of the counter is justified by the
	 * stupid 'buffer' lines in the random text files.
	 */
	for (i = 0; i <= line; i++)
	{
		if (0 != my_fgets(fp, buf, 80))
		{
			my_fclose(fp);
			return ( -1);
		}

		/* Found the line */
		if (i == line) break;
	}

	/* Copy the line to the output buffer */
	strcpy(output, buf);

	/* Close the file */
	my_fclose(fp);

	/* Success */
	return (0);
}


/*
 * Read line'th line file the file
 * and return pointer to it, or NULL if it fails.
 *
 * Nuked the static buffer. Caller should provide one. -- pelpel
 *
 * Caution: 'linbuf' should be at least 80 byte long.
 */
char *get_line(const char* fname, cptr fdir, char *linbuf, int line)
{
	FILE* fp;
	int i;
	char buf[1024];


	/* Don't count the first line in the file, which is a comment line */
	line++;

	/* Build the filename */
	path_build(buf, 1024, fdir, fname);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Failed */
	if (!fp) return (NULL);

	/* Read past specified number of lines */
	for (i = 0; i <= line; i++)
	{
		/* Oops */
		if (my_fgets(fp, linbuf, 80) != 0)
		{
			my_fclose(fp);
			return (NULL);
		}
	}

	my_fclose(fp);

	return (linbuf);
}


/*
 * Return a line for a speaking unique, by Matt G.
 *
 * XXX XXX XXX Opening a file and scanning it through whenever a unique
 * tries to say something? Something like DELAY_LOAD_?_TEXT would be
 * much better -- pelpel
 *
 * XXX XXX XXX I must say the original is an extremely poor and unreliable
 * implementation...  I removed noxious flag -- I'm too stupid to
 * understand such complexities -- and added extra error checkings
 * and made sure fd is always closed -- pelpel
 */
errr get_xtra_line(const char *file_name, monster_type *m_ptr, char *output)
{
	FILE *fp;
	char buf[1024];
	int line;
	int num_entries;
	int i;
	int mnum;


	/* Clear the message buffer */
	strcpy(output, "");

	/* test and DEBUG hack */
	if (wizard && options->cheat_xtra)
	{
		msg_print(file_name);
	}

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, file_name);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Failed */
	if (!fp) return ( -1);

	/* Monster number we are looking for */
	mnum = m_ptr->r_idx;

	/* Find matching N: line */
	while (1)
	{
		int n;

		/* Read a line */
		if (my_fgets(fp, buf, 90) != 0)
		{
			my_fclose(fp);
			return ( -1);
		}

		/* Not a N: line */
		if (buf[0] != 'N') continue;

		/* Skip "N:" and parse off a number */
		sscanf(buf + 2, "%d", &n);

		/* Match found */
		if (n == mnum) break;
	}

	/* Retrieve number of normal messages */
	while (1)
	{
		/* Read next line */
		if (my_fgets(fp, buf, 90) != 0)
		{
			my_fclose(fp);
			return ( -1);
		}

		/* The first line not beginning with 'N:' holds number of lines */
		if (buf[0] != 'N')
		{
			num_entries = atoi(buf);
			break;
		}
	}

	/* The monster is afraid */
	if (m_ptr->monfear)
	{
		/* Read past normal lines */
		for (line = 0; line < num_entries + 1; line++)
		{
			if (my_fgets(fp, buf, 90))
			{
				my_fclose(fp);
				return ( -1);
			}
		}

		/* Retrieve number of 'afraid' lines */
		num_entries = atoi(buf);
	}


	/* Pick a random line */
	line = rand_int(num_entries);

	/* test and DEBUG hack */
	if (wizard && options->cheat_xtra)
	{
		sprintf(buf, "Line number %d", line);
		msg_print(buf);
	}

	/* Find the selected line */
	for (i = 0; i <= line; i++)
	{
		/* Oops */
		if (0 != my_fgets(fp, buf, 90))
		{
			my_fclose(fp);
			return ( -1);
		}

		/* Found it */
		if (i == line) break;
	}

	/* Copy it to the output buffer */
	strcpy(output, buf);

	/* Close the file */
	my_fclose(fp);

	/* Success */
	return (0);
}
