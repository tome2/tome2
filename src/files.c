/* File: files.c */

/* Purpose: code dealing with files (and death) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


static bool setuid_grabbed = TRUE;


/*
 * You may or may not want to use the following "#undef".
 */
/* #undef _POSIX_SAVED_IDS */


/*
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{
	if (setuid_grabbed)
	{
		setuid_grabbed = FALSE;
#ifdef SET_UID

# ifdef SAFE_SETUID

# ifdef SAFE_SETUID_POSIX

		if (setuid(getuid()) != 0)
		{
			quit("setuid(): cannot set permissions correctly!");
		}
		if (setgid(getgid()) != 0)
		{
			quit("setgid(): cannot set permissions correctly!");
		}

# else

		if (setreuid(geteuid(), getuid()) != 0)
		{
			quit("setreuid(): cannot set permissions correctly!");
		}
		if (setregid(getegid(), getgid()) != 0)
		{
			quit("setregid(): cannot set permissions correctly!");
		}

# endif

# endif

#endif
	}

}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{
	if (!setuid_grabbed)
	{
		setuid_grabbed = TRUE;
#ifdef SET_UID

# ifdef SAFE_SETUID

# ifdef SAFE_SETUID_POSIX

		if (setuid(player_euid) != 0)
		{
			quit("setuid(): cannot set permissions correctly!");
		}
		if (setgid(player_egid) != 0)
		{
			quit("setgid(): cannot set permissions correctly!");
		}

# else

		if (setreuid(geteuid(), getuid()) != 0)
		{
			quit("setreuid(): cannot set permissions correctly!");
		}
		if (setregid(getegid(), getgid()) != 0)
		{
			quit("setregid(): cannot set permissions correctly!");
		}

# endif  /* SAFE_SETUID_POSIX */

# endif  /* SAFE_SETUID */

#endif /* SET_UID */
	}

}


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
			monster_race *r_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= max_r_idx) return (1);
			r_ptr = &r_info[i];
			if (n1) r_ptr->x_attr = n1;
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
				monster_ego *re_ptr;
				i = (huge)strtol(zz[0], NULL, 0);
				n1 = strtol(zz[1], NULL, 0);
				n2 = strtol(zz[2], NULL, 0);
				if (i >= max_re_idx) return (1);
				re_ptr = &re_info[i];
				if (n1) re_ptr->g_attr = n1;
				if (n2)
				{
					re_ptr->g_char = n2;
				}
				return (0);
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
				if (i >= max_rmp_idx) return (1);
				rmp_ptr = &race_mod_info[i];
				if (n1) rmp_ptr->g_attr = n1;
				if (n2)
				{
					rmp_ptr->g_char = n2;
				}
				return (0);
			}
		}

		/* Process "G:T:<num>:<a>/<c>" -- attr/char for traps */
		if (buf[2] == 'T')
		{
			if (tokenize(buf + 4, 3, zz, ':', '/') == 3)
			{
				trap_type *t_ptr;
				i = (huge)strtol(zz[0], NULL, 0);
				n1 = strtol(zz[1], NULL, 0);
				n2 = strtol(zz[2], NULL, 0);
				if (i >= max_t_idx) return (1);
				t_ptr = &t_info[i];
				if (n1) t_ptr->g_attr = n1;
				if (n2)
				{
					t_ptr->g_char = n2;
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
			object_kind *k_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= max_k_idx) return (1);
			k_ptr = &k_info[i];
			if (n1) k_ptr->x_attr = n1;
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
			feature_type *f_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= max_f_idx) return (1);
			f_ptr = &f_info[i];
			if (n1) f_ptr->x_attr = n1;
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
			store_info_type *st_ptr;
			i = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			if (i >= max_st_idx) return (1);
			st_ptr = &st_info[i];
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
			j = (huge)strtol(zz[0], NULL, 0);
			n1 = strtol(zz[1], NULL, 0);
			n2 = strtol(zz[2], NULL, 0);
			for (i = 1; i < max_k_idx; i++)
			{
				object_kind *k_ptr = &k_info[i];
				if (k_ptr->tval == j)
				{
					if (n1) k_ptr->d_attr = n1;
					if (n2) k_ptr->d_char = n2;
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

		string_free(keymap_act[mode][i]);

		keymap_act[mode][i] = string_make(macro__buf);

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
				free(macro_template);
				macro_template = NULL;
				for (i = 0; i < max_macrotrigger; i++)
					free(macro_trigger_name[i]);
				max_macrotrigger = 0;
			}

			if (*zz[0] == '\0') return 0;  /* clear template */
			num = strlen(zz[1]);
			if (2 + num != tok) return 1;  /* error */

			len = strlen(zz[0]) + 1 + num + 1;
			for (i = 0; i < num; i++)
				len += strlen(zz[2 + i]) + 1;
			macro_template = malloc(len);

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
			macro_trigger_name[m] = malloc(len);

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
		for (i = 0; option_info[i].o_desc; i++)
		{
			if (option_info[i].o_var &&
			                option_info[i].o_text &&
			                streq(option_info[i].o_text, buf + 2))
			{
				(*option_info[i].o_var) = FALSE;
				return (0);
			}
		}
	}

	/* Process "Y:<str>" -- turn option on */
	else if (buf[0] == 'Y')
	{
		for (i = 0; option_info[i].o_desc; i++)
		{
			if (option_info[i].o_var &&
			                option_info[i].o_text &&
			                streq(option_info[i].o_text, buf + 2))
			{
				(*option_info[i].o_var) = TRUE;
				return (0);
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

			else if (streq(b + 1, "KEYBOARD"))
			{
				v = ANGBAND_KEYBOARD;
			}

			/* Graphics */
			if (streq(b + 1, "GRAF"))
			{
				v = ANGBAND_GRAF;
			}

			/* Race */
			else if (streq(b + 1, "RACE"))
			{
				v = rp_ptr->title + rp_name;
			}

			/* Race */
			else if (streq(b + 1, "RACEMOD"))
			{
				v = rmp_ptr->title + rmp_name;
			}

			/* Class */
			else if (streq(b + 1, "CLASS"))
			{
				v = spp_ptr->title + c_name;
			}

			/* Player */
			else if (streq(b + 1, "PLAYER"))
			{
				v = player_base;
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

	bool bypass = FALSE;

	/* Build the filename -- Allow users to override system pref files */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* No such file -- Try system pref file */
	if (!fp)
	{
		/* Build the pathname, this time using the system pref directory */
		path_build(buf, 1024, ANGBAND_DIR_PREF, name);

		/* Grab permission */
		safe_setuid_grab();

		/* Open the file */
		fp = my_fopen(buf, "r");

		/* Drop permission */
		safe_setuid_drop();

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
			(void)process_pref_file(buf + 2);

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







#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] =
	{
		"SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
		"MON:XXXXXXXX.........XXXXXXX",
		"TUE:XXXXXXXX.........XXXXXXX",
		"WED:XXXXXXXX.........XXXXXXX",
		"THU:XXXXXXXX.........XXXXXXX",
		"FRI:XXXXXXXX.........XXXXXXX",
		"SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
	};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

	time_t c;

	struct tm *tp;


	/* No restrictions */
	if (!check_time_flag) return (0);

	/* Check for time violation */
	c = time((time_t *)0);
	tp = localtime(&c);

	/* Violation */
	if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);

#endif

	/* Success */
	return (0);
}



/*
 * !Ran under the game's permission!
 *
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

	FILE *fp;

	char buf[1024];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "time.txt");

	/*
	 * XXX No need to grab permission here because this function is called
	 * only once before the game drops "game" permission
	 */

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Assume restrictions */
	check_time_flag = TRUE;

	/* Parse the file */
	while (0 == my_fgets(fp, buf, 80))
	{
		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Chop the buffer */
		buf[29] = '\0';

		/* Extract the info */
		if (prefix(buf, "SUN:")) strcpy(days[0], buf);
		if (prefix(buf, "MON:")) strcpy(days[1], buf);
		if (prefix(buf, "TUE:")) strcpy(days[2], buf);
		if (prefix(buf, "WED:")) strcpy(days[3], buf);
		if (prefix(buf, "THU:")) strcpy(days[4], buf);
		if (prefix(buf, "FRI:")) strcpy(days[5], buf);
		if (prefix(buf, "SAT:")) strcpy(days[6], buf);
	}

	/* Close it */
	my_fclose(fp);

#endif

	/* Success */
	return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

typedef struct statstime statstime;

struct statstime
{
	int cp_time[4];
	int dk_xfer[4];
	unsigned int v_pgpgin;
	unsigned int v_pgpgout;
	unsigned int v_pswpin;
	unsigned int v_pswpout;
	unsigned int v_intr;
	int if_ipackets;
	int if_ierrors;
	int if_opackets;
	int if_oerrors;
	int if_collisions;
	unsigned int v_swtch;
	long avenrun[3];
	struct timeval boottime;
	struct timeval curtime;
};

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

	struct statstime st;


	/* Success if not checking */
	if (!check_load_value) return (0);

	/* Check the load */
	if (0 == rstat("localhost", &st))
	{
		long val1 = (long)(st.avenrun[2]);
		long val2 = (long)(check_load_value) * FSCALE;

		/* Check for violation */
		if (val1 >= val2) return (1);
	}

#endif

	/* Success */
	return (0);
}


/*
 * !Ran under the game's permission!
 *
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

	FILE *fp;

	char buf[1024];

	char temphost[MAXHOSTNAMELEN + 1];
	char thishost[MAXHOSTNAMELEN + 1];


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, "load.txt");

	/*
	 * XXX No need to grab permission here because this function is called
	 * only once before the game drops "game" permission
	 */

	/* Open the "load" file */
	fp = my_fopen(buf, "r");

	/* No file, no restrictions */
	if (!fp) return (0);

	/* Default load */
	check_load_value = 100;

	/* Get the host name */
	(void)gethostname(thishost, (sizeof thishost) - 1);

	/* Parse it */
	while (0 == my_fgets(fp, buf, 1024))
	{
		int value;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Parse, or ignore */
		if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;

		/* Skip other hosts */
		if (!streq(temphost, thishost) &&
		                !streq(temphost, "localhost")) continue;

		/* Use that value */
		check_load_value = value;

		/* Done */
		break;
	}

	/* Close the file */
	my_fclose(fp);

#endif

	/* Success */
	return (0);
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
	(void)sprintf(out_val, "%9ld", (long)num);
	c_put_str(color, out_val, row, col + len);
}


/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color,
                    char *space)
{
	int len = strlen(header);
	char out_val[32];

	put_str(header, row, col);
	put_str(space, row, col + len);
	(void)sprintf(out_val, "%6ld", (long)num);
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
	(void)sprintf(out_val, "%6s", str);
	c_put_str(color, out_val, row, col + len + 3);
}


/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
static void display_player_middle(void)
{
	int show_tohit = p_ptr->dis_to_h;
	int show_todam = p_ptr->dis_to_d;

	object_type *o_ptr = &p_ptr->inventory[INVEN_WIELD];
	char num[7];
	byte color;
	int speed;


	/* Hack -- add in weapon info if known */
	if (object_known_p(o_ptr)) show_tohit = p_ptr->dis_to_h + p_ptr->to_h_melee + o_ptr->to_h;
	else show_tohit = p_ptr->dis_to_h + p_ptr->to_h_melee;
	if (object_known_p(o_ptr)) show_todam = p_ptr->dis_to_d + p_ptr->to_d_melee + o_ptr->to_d;
	else show_todam = p_ptr->dis_to_d + p_ptr->to_d_melee;

	/* Dump the bonuses to hit/dam */
	prt_num("+ To Melee Hit   ", show_tohit, 9, 1, TERM_L_BLUE, "   ");
	prt_num("+ To Melee Damage", show_todam, 10, 1, TERM_L_BLUE, "   ");

	o_ptr = &p_ptr->inventory[INVEN_BOW];

	/* Hack -- add in weapon info if known */
	if (object_known_p(o_ptr)) show_tohit = p_ptr->dis_to_h + p_ptr->to_h_ranged + o_ptr->to_h;
	else show_tohit = p_ptr->dis_to_h + p_ptr->to_h_ranged;
	if (object_known_p(o_ptr)) show_todam = p_ptr->to_d_ranged + o_ptr->to_d;
	else show_todam = p_ptr->to_d_ranged;

	prt_num("+ To Ranged Hit   ", show_tohit, 11, 1, TERM_L_BLUE, "  ");
	prt_num("+ To Ranged Damage", show_todam, 12, 1, TERM_L_BLUE, "  ");

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

	if ((p_ptr->lev >= PY_MAX_LEVEL) || (p_ptr->lev >= max_plev))
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
		else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10)
		{
			color = TERM_VIOLET;
		}
		else
		{
			color = TERM_L_RED;
		}
		(void)sprintf(num, "%6ld", (long)p_ptr->chp);
		c_put_str(color, num, 9, 65);
		put_str("/", 9, 71);
		(void)sprintf(num, "%6ld", (long)p_ptr->mhp);
		c_put_str(TERM_L_BLUE, num, 9, 72);
	}
	else
	{
		put_str("Hit Points   ", 9, 52);
		if (p_ptr->chp >= p_ptr->mhp)
		{
			color = TERM_L_GREEN;
		}
		else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10)
		{
			color = TERM_YELLOW;
		}
		else
		{
			color = TERM_RED;
		}
		(void)sprintf(num, "%6ld", (long)p_ptr->chp);
		c_put_str(color, num, 9, 65);
		put_str("/", 9, 71);
		(void)sprintf(num, "%6ld", (long)p_ptr->mhp);
		c_put_str(TERM_L_GREEN, num, 9, 72);
	}

	put_str("Spell Points ", 10, 52);
	if (p_ptr->csp >= p_ptr->msp)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csp > (p_ptr->msp * hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	(void)sprintf(num, "%6ld", (long)p_ptr->csp);
	c_put_str(color, num, 10, 65);
	put_str("/", 10, 71);
	(void)sprintf(num, "%6ld", (long)p_ptr->msp);
	c_put_str(TERM_L_GREEN, num, 10, 72);

	put_str("Sanity       ", 11, 52);
	if (p_ptr->csane >= p_ptr->msane)
	{
		color = TERM_L_GREEN;
	}
	else if (p_ptr->csane > (p_ptr->msane * hitpoint_warn) / 10)
	{
		color = TERM_YELLOW;
	}
	else
	{
		color = TERM_RED;
	}
	(void)sprintf(num, "%6ld", (long)p_ptr->csane);
	c_put_str(color, num, 11, 65);
	put_str("/", 11, 71);
	(void)sprintf(num, "%6ld", (long)p_ptr->msane);
	c_put_str(TERM_L_GREEN, num, 11, 72);

	if (p_ptr->pgod != GOD_NONE)
	{
		prt_num("Piety          ", p_ptr->grace, 12, 52, TERM_L_GREEN, "     ");
	}

	put_str("Speed           ", 13, 52);
	speed = p_ptr->pspeed;
	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->searching) speed += 10;
	if (speed > 110)
	{
		char s[11];
		(void)sprintf(s, "Fast (+%d)", speed - 110);
		c_put_str(TERM_L_GREEN, s, 13, (speed >= 120) ? 68 : 69);
	}
	else if (speed < 110)
	{
		char s[11];
		(void)sprintf(s, "Slow (-%d)", 110 - speed);
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
static void display_player_various(void)
{
	int tmp, tmp2, damdice, damsides, dambonus, blows;
	int xthn, xthb, xfos, xsrh;
	int xdis, xdev, xsav, xstl;
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
	xdis = p_ptr->skill_dis;
	xdev = p_ptr->skill_dev;
	xsav = p_ptr->skill_sav;
	xstl = p_ptr->skill_stl;
	xsrh = p_ptr->skill_srh;
	xfos = p_ptr->skill_fos;


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


	put_str("Perception  :", 16, 28);
	desc = likert(xfos, 6);
	c_put_str(likert_color, desc, 16, 42);

	put_str("Searching   :", 17, 28);
	desc = likert(xsrh, 6);
	c_put_str(likert_color, desc, 17, 42);

	put_str("Disarming   :", 18, 28);
	desc = likert(xdis, 8);
	c_put_str(likert_color, desc, 18, 42);

	put_str("Magic Device:", 19, 28);
	desc = likert(xdev, 6);
	c_put_str(likert_color, desc, 19, 42);


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
		if (r_info[p_ptr->body_monster].flags1 & RF1_NEVER_BLOW)
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

		if (object_known_p(o_ptr)) dambonus += o_ptr->to_d;

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

void wield_monster_flags(u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp)
{
	object_type *o_ptr;
	monster_race *r_ptr;

	/* Clear */
	(*f1) = (*f2) = (*f3) = (*f4) = (*f5) = (*esp) = 0L;

	/* Get the carried monster */
	o_ptr = &p_ptr->inventory[INVEN_CARRY];

	if (o_ptr->k_idx)
	{
		r_ptr = &r_info[o_ptr->pval];

		if (r_ptr->flags2 & RF2_INVISIBLE)
			(*f2) |= TR2_INVIS;
		if (r_ptr->flags2 & RF2_REFLECTING)
			(*f2) |= TR2_REFLECT;
		if (r_ptr->flags7 & RF7_CAN_FLY)
			(*f3) |= TR3_FEATHER;
		if (r_ptr->flags7 & RF7_AQUATIC)
			(*f5) |= TR5_WATER_BREATH;
	}
}


/*
 * Obtain the "flags" for the player as if he was an item
 */
void player_flags(u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp)
{
	int i;

	/* Clear */
	(*f1) = (*f2) = (*f3) = (*f4) = (*f5) = (*esp) = 0L;

	/* Astral chars */
	if (p_ptr->astral)
	{
		(*f3) |= TR3_WRAITH;
	}

/* Skills */
	if (get_skill(SKILL_DAEMON) > 20) (*f2) |= TR2_RES_CONF;
	if (get_skill(SKILL_DAEMON) > 30) (*f2) |= TR2_RES_FEAR;
	if (get_skill(SKILL_MINDCRAFT) >= 40) (*esp) |= ESP_ALL;
	if (p_ptr->melee_style == SKILL_HAND && get_skill(SKILL_HAND) > 24 && !monk_heavy_armor())
		(*f2) |= TR2_FREE_ACT;
/* Hack - from Lua */
	if (get_skill(SKILL_MANA) >= 35) (*f1) |= TR1_MANA;
	if (get_skill(SKILL_AIR) >= 50) (*f5) |= (TR5_MAGIC_BREATH | TR5_WATER_BREATH);
	if (get_skill(SKILL_WATER) >= 30) (*f5) |= TR5_WATER_BREATH;

/* Gods */
	GOD(GOD_ERU)
	{
		if ((p_ptr->grace >= 100) || (p_ptr->grace <= -100))  (*f1) |= TR1_MANA;
		if (p_ptr->grace > 10000) (*f1) |= TR1_WIS;
	}

	GOD(GOD_MELKOR)
	{
		(*f2) |= TR2_RES_FIRE;
		if (p_ptr->melkor_sacrifice > 0) (*f2) |= TR2_LIFE;
		if (p_ptr->grace > 10000) (*f1) |= (TR1_STR | TR1_CON | TR1_INT | TR1_WIS | TR1_CHR);
		PRAY_GOD(GOD_MELKOR)
		{
			if (p_ptr->grace > 5000)  (*f2) |= TR2_INVIS;
			if (p_ptr->grace > 15000) (*f2) |= TR2_IM_FIRE;
		}
	}

	GOD(GOD_MANWE)
	{
		if (p_ptr->grace >= 2000) (*f3) |= TR3_FEATHER;
		PRAY_GOD(GOD_MANWE)
		{
			if (p_ptr->grace >= 7000)  (*f2) |= TR2_FREE_ACT;
			if (p_ptr->grace >= 15000) (*f4) |= TR4_FLY;
			if ((p_ptr->grace >= 5000) || (p_ptr->grace <= -5000)) (*f1) |= TR1_SPEED;
		}
	}

	GOD(GOD_TULKAS)
	{
		if (p_ptr->grace > 5000)  (*f1) |= TR1_CON;
		if (p_ptr->grace > 10000) (*f1) |= TR1_STR;
	}

	/* Classes */
	for (i = 1; i <= p_ptr->lev; i++)
	{
		(*f1) |= cp_ptr->oflags1[i];
		(*f2) |= cp_ptr->oflags2[i];
		(*f3) |= cp_ptr->oflags3[i];
		(*f4) |= cp_ptr->oflags4[i];
		(*f5) |= cp_ptr->oflags5[i];
		(*esp) |= cp_ptr->oesp[i];
	}

	/* Races */
	if ((!p_ptr->mimic_form) && (!p_ptr->body_monster))
	{
		for (i = 1; i <= p_ptr->lev; i++)
		{
			(*f1) |= rp_ptr->oflags1[i];
			(*f2) |= rp_ptr->oflags2[i];
			(*f3) |= rp_ptr->oflags3[i];
			(*f4) |= rp_ptr->oflags4[i];
			(*f5) |= rp_ptr->oflags5[i];
			(*esp) |= rp_ptr->oesp[i];

			(*f1) |= rmp_ptr->oflags1[i];
			(*f2) |= rmp_ptr->oflags2[i];
			(*f3) |= rmp_ptr->oflags3[i];
			(*f4) |= rmp_ptr->oflags4[i];
			(*f5) |= rmp_ptr->oflags5[i];
			(*esp) |= rmp_ptr->oesp[i];
		}
	}
#if 0 /* DGDGDG? */
	else if (p_ptr->mimic_form)
	{
		switch (p_ptr->mimic_form)
		{
		case MIMIC_GOAT:
			{
				(*f3) |= (TR3_SLOW_DIGEST);
				break;
			}
		case MIMIC_INSECT:
			{
				(*esp) |= (ESP_ANIMAL);
				break;
			}
		case MIMIC_SPARROW:
			{
				(*f3) |= (TR3_FEATHER);
				break;
			}
		case MIMIC_VAMPIRE:
			{
				(*f3) |= (TR2_HOLD_LIFE);
				(*f3) |= (TR2_RES_DARK);
				(*f3) |= (TR2_RES_NEXUS);
				(*f3) |= (TR2_RES_BLIND);
				(*f3) |= (TR3_LITE1);
				break;
			}
		case MIMIC_SPIDER:
			{
				(*f3) |= (TR2_RES_FEAR);
				(*f3) |= (TR2_RES_POIS);
				break;
			}
		case MIMIC_MANA_BALL:
			{
				(*f3) |= (TR3_FEATHER);
				(*f2) |= (TR2_INVIS);
				(*f3) |= (TR3_TELEPORT);
				(*f3) |= (TR2_RES_DISEN);
				break;
			}
		case MIMIC_FIRE_CLOUD:
			{
				(*f3) |= (TR2_RES_LITE);
				(*f3) |= (TR2_IM_FIRE);
				(*f3) |= (TR3_SH_FIRE);
				break;
			}
		case MIMIC_COLD_CLOUD:
			{
				(*f3) |= (TR2_RES_LITE);
				(*f3) |= (TR2_IM_COLD);
				(*f3) |= (TR3_SH_ELEC);
				(*esp) |= (ESP_EVIL);
				(*f3) |= (TR3_REGEN);
				break;
			}
		case MIMIC_CHAOS_CLOUD:
			{
				(*f3) |= (TR2_RES_DISEN);
				(*f3) |= (TR2_RES_CHAOS);
				(*f3) |= (TR2_RES_LITE);
				(*f3) |= (TR2_IM_FIRE);
				(*f3) |= (TR2_IM_COLD);
				(*f3) |= (TR3_SH_FIRE);
				(*f3) |= (TR3_SH_ELEC);
				break;
			}
		case MIMIC_GOST:
			{
				(*f3) |= (TR3_WRAITH);
				(*f2) |= TR2_HOLD_LIFE;
				break;
			}
		case MIMIC_ENT:
			{
				(*f2) |= TR2_SENS_FIRE;
				break;
			}
		case MIMIC_KOBOLD:
			{
				(*f2) |= TR2_RES_POIS;
				break;
			}
		case MIMIC_DRAGON:
			{
				(*f3) |= TR3_FEATHER;
				(*f2) |= TR2_RES_FIRE;
				(*f2) |= TR2_RES_COLD;
				(*f2) |= TR2_RES_ELEC;
				(*f2) |= TR2_RES_DARK;
				break;
			}
		case MIMIC_DEMON:
			{
				(*f2) |= TR2_RES_CHAOS;
				(*f2) |= TR2_RES_NETHER;
				(*f2) |= TR2_HOLD_LIFE;
				break;
			}
		case MIMIC_HOUND:
			{
				(*f1) |= TR1_SPEED;
				(*f2) |= TR2_RES_LITE;
				(*f2) |= TR2_RES_DARK;
				break;
			}
		case MIMIC_QUYLTHULG:
			{
				(*f3) |= TR3_SEE_INVIS;
				break;
			}
		case MIMIC_MAIAR:
			{
				(*f2) |= TR2_IM_ACID;
				(*f2) |= TR2_IM_ELEC;
				(*f2) |= TR2_IM_FIRE;
				(*f2) |= TR2_IM_COLD;
				(*f2) |= TR2_RES_POIS;
				(*f2) |= TR2_RES_LITE;
				(*f2) |= TR2_RES_DARK;
				(*f2) |= TR2_RES_CHAOS;
				(*f2) |= TR2_HOLD_LIFE;
				(*f3) |= TR3_FEATHER;
				(*f3) |= TR3_REGEN;
				break;
			}
		case MIMIC_SERPENT:
			{
				(*f1) |= TR1_SPEED;
				break;
			}
		case MIMIC_GIANT:
			{
				(*f2) |= TR2_RES_ACID;
				(*f2) |= TR2_RES_ELEC;
				(*f2) |= TR2_RES_FIRE;
				(*f2) |= TR2_RES_COLD;
				(*f2) |= TR2_RES_POIS;
				(*f2) |= TR2_RES_CONF;
				(*f2) |= TR2_RES_SOUND;
				(*f2) |= TR2_RES_LITE;
				(*f2) |= TR2_RES_DARK;
				(*f2) |= TR2_RES_NEXUS;
				(*f2) |= TR2_RES_FEAR;
				(*f2) |= TR2_REFLECT;
				break;
			}
		case MIMIC_VALAR:
			{
				(*f3) |= TR3_SEE_INVIS;
				(*f2) |= TR2_FREE_ACT;
				(*f3) |= TR3_SLOW_DIGEST;
				(*f3) |= TR3_REGEN;
				(*f3) |= TR3_FEATHER;
				(*f2) |= TR2_HOLD_LIFE;
				(*esp) |= ESP_ALL;
				(*f3) |= TR3_LITE1;
				(*f2) |= TR2_SUST_STR;
				(*f1) |= TR1_INT;
				(*f1) |= TR1_WIS;
				(*f1) |= TR1_DEX;
				(*f1) |= TR1_CON;
				(*f1) |= TR1_CHR;
				(*f2) |= TR2_RES_ACID;
				(*f2) |= TR2_RES_ELEC;
				(*f2) |= TR2_RES_FIRE;
				(*f2) |= TR2_RES_COLD;
				(*f2) |= TR2_RES_POIS;
				(*f2) |= TR2_RES_CONF;
				(*f2) |= TR2_RES_SOUND;
				(*f2) |= TR2_RES_LITE;
				(*f2) |= TR2_RES_DARK;
				(*f2) |= TR2_RES_CHAOS;
				(*f2) |= TR2_RES_DISEN;
				(*f2) |= TR2_RES_SHARDS;
				(*f2) |= TR2_RES_NEXUS;
				(*f2) |= TR2_RES_BLIND;
				(*f2) |= TR2_RES_NETHER;
				(*f2) |= TR2_RES_FEAR;
				(*f2) |= TR2_REFLECT;
				(*f3) |= TR3_SH_FIRE;
				(*f3) |= TR3_SH_ELEC;
				(*f2) |= TR2_IM_FIRE;
				(*f2) |= TR2_IM_COLD;
				(*f2) |= TR2_IM_ELEC;
				(*f2) |= TR2_IM_ACID;
				break;
			}

		case MIMIC_WEREWOLF:
			{
				(*f3) |= TR3_REGEN;
				(*f3) |= TR3_AGGRAVATE;
				break;
			}
		case MIMIC_BALROG:
			{
				(*f2) |= TR2_IM_ACID;
				(*f2) |= TR2_IM_ELEC;
				(*f2) |= TR2_IM_FIRE;
				(*f2) |= TR2_RES_POIS;
				(*f2) |= TR2_RES_DARK;
				(*f2) |= TR2_RES_CHAOS;
				(*f2) |= TR2_HOLD_LIFE;
				(*f3) |= TR3_FEATHER;
				(*f3) |= TR3_REGEN;
				break;
			}
		case MIMIC_DEMON_LORD:
			{
				(*f3) |= TR3_SEE_INVIS;
				(*f2) |= TR2_FREE_ACT;
				(*f3) |= TR3_REGEN;
				(*f3) |= TR3_FEATHER;
				(*f2) |= TR2_HOLD_LIFE;
				(*esp) |= ESP_EVIL | ESP_GOOD | ESP_DEMON;
				(*f2) |= TR2_RES_ACID;
				(*f2) |= TR2_RES_ELEC;
				(*f2) |= TR2_RES_FIRE;
				(*f2) |= TR2_RES_POIS;
				(*f2) |= TR2_RES_CONF;
				(*f2) |= TR2_RES_SOUND;
				(*f2) |= TR2_RES_LITE;
				(*f2) |= TR2_RES_DARK;
				(*f2) |= TR2_RES_CHAOS;
				(*f2) |= TR2_RES_DISEN;
				(*f2) |= TR2_RES_SHARDS;
				(*f2) |= TR2_RES_NEXUS;
				(*f2) |= TR2_RES_BLIND;
				(*f2) |= TR2_RES_NETHER;
				(*f2) |= TR2_RES_FEAR;
				(*f2) |= TR2_REFLECT;
				(*f3) |= TR3_SH_FIRE;
				(*f2) |= TR2_IM_FIRE;
				(*f2) |= TR2_IM_ELEC;
				(*f2) |= TR2_IM_ACID;
				break;
			}
		}
	}
#endif
	else
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

		if (r_ptr->flags2 & RF2_REFLECTING) (*f2) |= TR2_REFLECT;
		if (r_ptr->flags2 & RF2_REGENERATE) (*f3) |= TR3_REGEN;
		if (r_ptr->flags2 & RF2_AURA_FIRE) (*f3) |= TR3_SH_FIRE;
		if (r_ptr->flags2 & RF2_AURA_ELEC) (*f3) |= TR3_SH_ELEC;
		if (r_ptr->flags2 & RF2_PASS_WALL) (*f3) |= TR3_WRAITH;
		if (r_ptr->flags3 & RF3_SUSCEP_FIRE) (*f2) |= TR2_SENS_FIRE;
		if (r_ptr->flags3 & RF3_IM_ACID) (*f2) |= TR2_RES_ACID;
		if (r_ptr->flags3 & RF3_IM_ELEC) (*f2) |= TR2_RES_ELEC;
		if (r_ptr->flags3 & RF3_IM_FIRE) (*f2) |= TR2_RES_FIRE;
		if (r_ptr->flags3 & RF3_IM_POIS) (*f2) |= TR2_RES_POIS;
		if (r_ptr->flags3 & RF3_IM_COLD) (*f2) |= TR2_RES_COLD;
		if (r_ptr->flags3 & RF3_RES_NETH) (*f2) |= TR2_RES_NETHER;
		if (r_ptr->flags3 & RF3_RES_NEXU) (*f2) |= TR2_RES_NEXUS;
		if (r_ptr->flags3 & RF3_RES_DISE) (*f2) |= TR2_RES_DISEN;
		if (r_ptr->flags3 & RF3_NO_FEAR) (*f2) |= TR2_RES_FEAR;
		if (r_ptr->flags3 & RF3_NO_SLEEP) (*f2) |= TR2_FREE_ACT;
		if (r_ptr->flags3 & RF3_NO_CONF) (*f2) |= TR2_RES_CONF;
		if (r_ptr->flags7 & RF7_CAN_FLY) (*f3) |= TR3_FEATHER;
	}

	(*f1) |= p_ptr->xtra_f1;
	(*f2) |= p_ptr->xtra_f2;
	(*f3) |= p_ptr->xtra_f3;
	(*f4) |= p_ptr->xtra_f4;
	(*f5) |= p_ptr->xtra_f5;
	(*esp) |= p_ptr->xtra_esp;

	if (p_ptr->black_breath)
	{
		(*f4) |= TR4_BLACK_BREATH;
	}

	if (p_ptr->hp_mod != 0)
	{
		(*f2) |= TR2_LIFE;
	}
}

/*
 * Object flag names
 */
static cptr object_flag_names[192] =
{
	"Add Str",
	"Add Int",
	"Add Wis",
	"Add Dex",
	"Add Con",
	"Add Chr",
	"Mul Mana",
	"Mul SPower",
	"Add Stea.",
	"Add Sear.",
	"Add Infra",
	"Add Tun..",
	"Add Speed",
	"Add Blows",
	"Chaotic",
	"Vampiric",
	"Slay Anim.",
	"Slay Evil",
	"Slay Und.",
	"Slay Demon",
	"Slay Orc",
	"Slay Troll",
	"Slay Giant",
	"Slay Drag.",
	"Kill Drag.",
	"Sharpness",
	"Impact",
	"Poison Brd",
	"Acid Brand",
	"Elec Brand",
	"Fire Brand",
	"Cold Brand",

	"Sust Str",
	"Sust Int",
	"Sust Wis",
	"Sust Dex",
	"Sust Con",
	"Sust Chr",
	"Invisible",
	"Mul life",
	"Imm Acid",
	"Imm Elec",
	"Imm Fire",
	"Imm Cold",
	"Sens Fire",
	"Reflect",
	"Free Act",
	"Hold Life",
	"Res Acid",
	"Res Elec",
	"Res Fire",
	"Res Cold",
	"Res Pois",
	"Res Fear",
	"Res Light",
	"Res Dark",
	"Res Blind",
	"Res Conf",
	"Res Sound",
	"Res Shard",
	"Res Neth",
	"Res Nexus",
	"Res Chaos",
	"Res Disen",



	"Aura Fire",
	"Aura Elec",
	"Auto Curse",
	NULL,
	"NoTeleport",
	"AntiMagic",
	"WraithForm",
	"EvilCurse",
	NULL,
	NULL,
	NULL,
	NULL,
	"Levitate",
	"Lite",
	"See Invis",
	NULL,
	"Digestion",
	"Regen",
	"Xtra Might",
	"Xtra Shots",
	NULL,
	NULL,
	NULL,
	NULL,
	"Activate",
	"Drain Exp",
	"Teleport",
	"Aggravate",
	"Blessed",
	"Cursed",
	"Hvy Curse",
	"Prm Curse",

	"No blows",
	"Precogn.",
	"B.Breath",
	"Recharge",
	"Fly",
	"Mrg.Curse",
	NULL,
	NULL,
	"Sentient",
	"Clone",
	NULL,
	"Climb",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Imm Neth",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	"Orc.ESP",
	"Troll.ESP",
	"Dragon.ESP",
	"Giant.ESP",
	"Demon.ESP",
	"Undead.ESP",
	"Evil.ESP",
	"Animal.ESP",
	"TLord.ESP",
	"Good.ESP",
	"Nlive.ESP",
	"Unique.ESP",
	"Spider ESP",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Full ESP",
};

/*
 * Summarize resistances
 */
static void display_player_ben_one(int mode)
{
	int i, n, x, y, z, dispx, modetemp, xtemp;

	object_type *o_ptr;

	char dummy[80], c;

	u32b f1, f2, f3, f4, f5, esp;

	u16b b[INVEN_TOTAL - INVEN_WIELD + 1][10];

	int d[INVEN_TOTAL - INVEN_WIELD + 1];

	bool got;

	byte a;

	cptr name;

	/* Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Index */
		n = (i - INVEN_WIELD);

		/* Object */
		o_ptr = &p_ptr->inventory[i];

		/* Known object flags */
		object_flags_known(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Incorporate */
		b[n][0] = (u16b)(f1 & 0xFFFF);
		b[n][1] = (u16b)(f1 >> 16);
		b[n][2] = (u16b)(f2 & 0xFFFF);
		b[n][3] = (u16b)(f2 >> 16);
		b[n][4] = (u16b)(f3 & 0xFFFF);
		b[n][5] = (u16b)(f3 >> 16);
		b[n][6] = (u16b)(f4 & 0xFFFF);
		b[n][7] = (u16b)(f4 >> 16);
		b[n][8] = (u16b)(esp & 0xFFFF);
		b[n][9] = (u16b)(esp >> 16);
		d[n] = o_ptr->pval;
	}

	/* Carried symbiote */
	n = INVEN_CARRY - INVEN_WIELD;

	/* Player flags */
	wield_monster_flags(&f1, &f2, &f3, &f4, &f5, &esp);

	/* Incorporate */
	b[n][0] = (u16b)(f1 & 0xFFFF);
	b[n][1] = (u16b)(f1 >> 16);
	b[n][2] = (u16b)(f2 & 0xFFFF);
	b[n][3] = (u16b)(f2 >> 16);
	b[n][4] = (u16b)(f3 & 0xFFFF);
	b[n][5] = (u16b)(f3 >> 16);
	b[n][6] = (u16b)(f4 & 0xFFFF);
	b[n][7] = (u16b)(f4 >> 16);
	b[n][8] = (u16b)(esp & 0xFFFF);
	b[n][9] = (u16b)(esp >> 16);

	/* Index */
	n = INVEN_TOTAL - INVEN_WIELD;

	/* Player flags */
	player_flags(&f1, &f2, &f3, &f4, &f5, &esp);

	/* Incorporate */
	b[n][0] = (u16b)(f1 & 0xFFFF);
	b[n][1] = (u16b)(f1 >> 16);
	b[n][2] = (u16b)(f2 & 0xFFFF);
	b[n][3] = (u16b)(f2 >> 16);
	b[n][4] = (u16b)(f3 & 0xFFFF);
	b[n][5] = (u16b)(f3 >> 16);
	b[n][6] = (u16b)(f4 & 0xFFFF);
	b[n][7] = (u16b)(f4 >> 16);
	b[n][8] = (u16b)(esp & 0xFFFF);
	b[n][9] = (u16b)(esp >> 16);

	/* Generate the equip chars */
	sprintf(dummy, " ");
	for (i = 0; i < INVEN_TOTAL - INVEN_WIELD; i++)
	{
		/* If you have that body part then show it */
		if (p_ptr->body_parts[i])
		{
			strcat(dummy, format("%c", i + 'a'));
		}
	}
	strcat(dummy, "@");

	/* Scan cols */
	for (x = 1; x > -1; x--)
	{
		/* Label */
		Term_putstr(x * 40 + 11, 3, -1, TERM_WHITE, dummy);

		/* Scan rows */
		for (y = 0; y < 16; y++)
		{
			if (mode == 3 && x == 1)
			{
				modetemp = 4;
				xtemp = 0;
			}
			else
			{
				modetemp = mode;
				xtemp = x;
			}

			for (z = mode; z <= modetemp; z++)
			{
				if (mode == 3 && x == 1 && z == modetemp) xtemp = 1;
				name = object_flag_names[32 * modetemp + 16 * xtemp + y];
				got = FALSE;

				/* No name */
				if (!name) continue;

				/* Dump colon */
				if (!(modetemp == 1 && x == 0 && y > 7 && y < 12))
				{
					Term_putch(x * 40 + 10, y + 4, TERM_WHITE, ':');
				}

				/* Check flags */
				dispx = 0;
				for (n = 0; n < INVEN_TOTAL - INVEN_WIELD + 1; n++)
				{
					/* Change colour every two columns */
					bool is_green = (dispx & 0x02);
					a = (is_green ? TERM_GREEN : TERM_SLATE);
					c = '.';

					/* If the body part doesn't exists then skip it :) */
					if ((n < INVEN_TOTAL - INVEN_WIELD) && (!p_ptr->body_parts[n])) continue;

					/* Increment the drawing coordinates */
					dispx++;

					/* Check flag */
					if (b[n][2 * modetemp + xtemp] & (1 << y))
					{
						a = (is_green ? TERM_L_GREEN : TERM_WHITE);
						if (modetemp == 1 && x == 0 && y > 7 && y < 12)
						{
							c = '*';
						}
						else if (modetemp == 0 && x == 0 && y < 14 && (y < 6 || y > 7))
						{
							if (n == INVEN_TOTAL - INVEN_WIELD)
							{
								c = '+';
							}
							else
							{
								c = d[n];
								if (c < 0)
								{
									c = -c;
									a = TERM_RED;
								}
								c = (c > 9 ? '*' : I2D(c));
							}
						}
						else
						{
							c = '+';
						}
						got = TRUE;
					}

					/* HACK - Check for nether immunity and
					   apply to Res Neth line */
					if (modetemp == 1 && x == 1 && y == 12)
					{
						if (b[n][7] & (1 << 6))
						{
							a = (is_green ? TERM_L_GREEN : TERM_WHITE);
							c = '*';
							got = TRUE;
						}
					}

					/* Monochrome */
					if (!use_color) a = TERM_WHITE;

					/* Dump flag */
					if (modetemp == 1 && x == 0 && y > 7 && y < 12)
					{
						if (c == '*') Term_putch(40 + 11 + dispx, y - 4, a, c);
					}
					else
					{
						Term_putch(x * 40 + 11 + dispx, y + 4, a, c);
					}
				}

				a = TERM_WHITE;
				if (use_color && got)
				{
					if (modetemp == 1 && x == 0 && y > 7 && y < 12)
					{
						a = TERM_L_GREEN;
					}
					else if (modetemp != 0)
					{
						a = TERM_GREEN;
					}
				}

				/* HACK - Check for nether immunity and change "Res Neth" */
				if (modetemp == 1 && x == 1 && y == 12 && p_ptr->immune_neth)
				{
					name = "Imm Neth";
					a = TERM_L_GREEN;
				}

				/* Dump name */
				if (modetemp == 1 && x == 0 && y > 7 && y < 12)
				{
					if (got) Term_putstr(40, y - 4, -1, a, name);
				}
				else
				{
					Term_putstr(x * 40, y + 4, -1, a, name);
				}
			}
		}
	}
}


/*
 * Display the character on the screen (various modes)
 *
 * The top two and bottom two lines are left blank.
 *
 * Mode 0 = standard display with skills
 * Mode 1 = standard display with history
 * Mode 2 = current flags (part 1)
 * Mode 3 = current flags (part 2)
 * Mode 4 = current flags (part 3)
 * Mode 5 = current flags (part 4)
 * Mode 6 = current flags (part 5 -- esp)
 */
void display_player(int mode)
{
	int i;

	char buf[80];


	/* Erase screen */
	clear_from(0);

	/* Standard */
	if ((mode == 0) || (mode == 1))
	{
		monster_race *r_ptr = &r_info[p_ptr->body_monster];

		/* Name, Sex, Race, Class */
		put_str("Name  :", 2, 1);
		put_str("Sex   :", 3, 1);
		put_str("Race  :", 4, 1);
		put_str("Class :", 5, 1);
		put_str("Body  :", 6, 1);
		put_str("God   :", 7, 1);
		c_put_str(TERM_L_BLUE, player_name, 2, 9);
		if (p_ptr->body_monster != 0)
		{
			monster_race *r_ptr = &r_info[p_ptr->body_monster];
			char tmp[12];

			if ((r_ptr->flags1 & RF1_MALE) != 0)
				strcpy(tmp, "Male");
			else if ((r_ptr->flags1 & RF1_FEMALE) != 0)
				strcpy(tmp, "Female");
			else
				strcpy(tmp, "Neuter");
			c_put_str(TERM_L_BLUE, tmp, 3, 9);
		}
		else
			c_put_str(TERM_L_BLUE, sp_ptr->title, 3, 9);
		sprintf(buf, "%s", get_player_race_name(p_ptr->prace, p_ptr->pracem));
		c_put_str(TERM_L_BLUE, buf, 4, 9);
		c_put_str(TERM_L_BLUE, spp_ptr->title + c_name, 5, 9);
		c_put_str(TERM_L_BLUE, r_name + r_ptr->name, 6, 9);
		c_put_str(TERM_L_BLUE, deity_info[p_ptr->pgod].name, 7, 9);

		/* Age, Height, Weight, Social */
		prt_num("Age          ", (int)p_ptr->age + bst(YEAR, turn - (START_DAY * 10)), 2, 32, TERM_L_BLUE, "   ");
		prt_num("Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE, "   ");
		prt_num("Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE, "   ");
		prt_num("Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE, "   ");

		/* Display the stats */
		for (i = 0; i < 6; i++)
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

		/* Display "history" info */
		if (mode == 1)
		{
			put_str("(Character Background)", 15, 25);

			for (i = 0; i < 4; i++)
			{
				put_str(history[i], i + 16, 10);
			}
		}

		/* Display "various" info */
		else
		{
			put_str("(Miscellaneous Abilities)", 15, 25);

			display_player_various();
		}
	}

	/* Special */
	else
	{
		display_player_ben_one(mode - 2);
	}
}

/*
 * Utility function; should probably be in some other file...
 *
 * Describe the player's location -- either by dungeon level, town, or in
 * wilderness with landmark reference.
 */
cptr describe_player_location()
{
	int i;
	static char desc[80];
	int pwx = (p_ptr->wild_mode ? p_ptr->px : p_ptr->wilderness_x);
	int pwy = (p_ptr->wild_mode ? p_ptr->py : p_ptr->wilderness_y);
	int feat = wild_map[pwy][pwx].feat;

	if (dungeon_type != DUNGEON_WILDERNESS && dun_level > 0)
		sprintf(desc, "on level %d of %s", dun_level, d_info[dungeon_type].name + d_name);
	else if (wf_info[feat].terrain_idx == TERRAIN_TOWN)
		sprintf(desc, "in the town of %s", wf_info[feat].name + wf_name);
	else if (wf_info[feat].entrance)
		sprintf(desc, "near %s", wf_info[feat].name + wf_name);
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
		int i;

		for (i = 0; i < max_wf_idx; i++)
		{
			int wx = wf_info[i].wild_x;
			int wy = wf_info[i].wild_y;
			int dist;

			/* Skip if not a landmark */
			if (!wf_info[i].entrance) continue;

			/* Skip if we haven't seen it */
			if (!wild_map[wy][wx].known) continue;

			dist = distance(wy, wx, pwy, pwx);
			if (dist < l_dist || l_dist < 0)
			{
				landmark = i;
				l_dist = dist;
				lwx = wx;
				lwy = wy;
			}
		}

		if (!landmark)
			sprintf(desc, "in %s", wf_info[feat].text + wf_text);
		else if (pwx == lwx && pwy == lwy)
			/* Paranoia; this should have been caught above */
			sprintf(desc, "near %s", wf_info[feat].name + wf_name);
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

			sprintf(desc, "in %s %s%s of %s",
			        wf_info[feat].text + wf_text, ns, ew,
			        wf_info[landmark].name + wf_name);
		}
	}

	/* strip trailing whitespace */
	for (i = 0; desc[i]; ++i);
	while (desc[--i] == ' ')
		desc[i] = 0;

	return desc;
}

/*
 * Helper function or file_character_print_grid
 *
 * Figure out if a row on the grid is empty
 */
static bool file_character_print_grid_check_row(const char *buf)
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
static void file_character_print_grid(FILE *fff, bool show_gaps, bool show_legend)
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
			(void)(Term_what(x, y, &a, &c));
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
			(void)(Term_what(x, y, &a, &c));
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
void file_character_print_item(FILE *fff, char label, object_type *obj, bool full)
{
	static char o_name[80];
	static cptr paren = ")";
	object_desc(o_name, obj, TRUE, 3);
	fprintf(fff, "%c%s %s\n", label, paren, o_name);

	if ((artifact_p(obj) || ego_item_p(obj) || obj->tval == TV_RING || obj->tval == TV_AMULET || full) &&
	                (obj->ident & IDENT_MENTAL))
	{
		object_out_desc(obj, fff, TRUE, TRUE);
	}
}

/*
 * Helper function for file_character
 *
 * Prints out one "store" (for Home and Mathom-house)
 */
void file_character_print_store(FILE *fff, wilderness_type_info *place, int store, bool full)
{
	int i;
	town_type *town = &town_info[place->entrance];
	store_type *st_ptr = &town->store[store];

	if (st_ptr->stock_num)
	{
		/* Header with name of the town */
		fprintf(fff, "  [%s Inventory - %s]\n\n", st_name + st_info[store].name, wf_name + place->name);

		/* Dump all available items */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			file_character_print_item(fff, I2A(i%24), &st_ptr->stock[i], full);
		}

		/* Add an empty line */
		fprintf(fff, "\n\n");
	}
}

/*
 * Helper function for file_character
 *
 * Checks if the store hasn't been added to the list yet, and then adds it if it
 * was not already there.  XXX This is an ugly workaround for the double Gondolin
 * problem.
 *
 * Beware of the ugly pointer gymnastics.
 */
bool file_character_check_stores(store_type ***store_list, int *store_list_count, wilderness_type_info *place, int store)
{
	town_type *town = &town_info[place->entrance];
	store_type *st_ptr = &town->store[store];
	store_type **head = *store_list;
	int i;

	/* check the list for this store */
	for (i = 0; i < *store_list_count; ++i)
	{
		if (*head == st_ptr) return FALSE;
		++head;
	}

	/* make room for the new one in the array */
	if (*store_list)
	{
		head = *store_list;
		*store_list = C_RNEW(*store_list_count + 1, store_type *);
		C_COPY(*store_list, head, *store_list_count, store_type *);
		C_FREE(head, *store_list_count, store_type *);
	}
	else
	{
		*store_list = RNEW(store_type *);
	}

	/* update data */
	(*store_list)[*store_list_count] = st_ptr;
	++*store_list_count;

	return TRUE;
}

/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(cptr name, bool full)
{
	int i, j, x, y;
	byte a;
	char c;
	int fd = -1;
	FILE *fff = NULL;
	char buf[1024];
	store_type **store_list = NULL;
	int store_list_count = 0;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Check for existing file */
	fd = fd_open(buf, O_RDONLY);

	/* Existing file */
	if (fd >= 0)
	{
		char out_val[160];

		/* Close the file */
		(void)fd_close(fd);

		/* Build query */
		(void)sprintf(out_val, "Replace existing file %s? ", buf);

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
	fprintf(fff, "  [%s %ld.%ld.%ld%s Character Sheet]\n\n",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);


	/* Display player */
	display_player(0);

	/* Dump part of the screen */
	for (y = 2; y < 22; y++)
	{
		/* Dump each row */
		for (x = 0; x < 79; x++)
		{
			/* Get the attr/char */
			(void)(Term_what(x, y, &a, &c));

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
			(void)(Term_what(x, y, &a, &c));

			/* Dump it */
			buf[x] = c;
		}

		/* Terminate */
		buf[x] = '\0';

		/* End the row */
		fprintf(fff, "%s\n", buf);
	}

	/* List the patches */
	hook_file = fff;
	exec_lua("patchs_list()");

	fprintf(fff, "\n\n  [Miscellaneous information]\n");
	if (joke_monsters)
		fprintf(fff, "\n Joke monsters:        ON");
	else
		fprintf(fff, "\n Joke monsters:        OFF");

	if (p_ptr->maximize)
		fprintf(fff, "\n Maximize mode:        ON");
	else
		fprintf(fff, "\n Maximize mode:        OFF");

	if (p_ptr->preserve)
		fprintf(fff, "\n Preserve Mode:        ON");
	else
		fprintf(fff, "\n Preserve Mode:        OFF");

	if (auto_scum)
		fprintf(fff, "\n Autoscum:             ON");
	else
		fprintf(fff, "\n Autoscum:             OFF");

	if (always_small_level)
		fprintf(fff, "\n Small Levels:         ALWAYS");
	else if (small_levels)
		fprintf(fff, "\n Small Levels:         ON");
	else
		fprintf(fff, "\n Small Levels:         OFF");

	if (empty_levels)
		fprintf(fff, "\n Arena Levels:         ON");
	else
		fprintf(fff, "\n Arena Levels:         OFF");

	if (ironman_rooms)
		fprintf(fff, "\n Always unusual rooms: ON");
	else
		fprintf(fff, "\n Always unusual rooms: OFF");

	if (seed_dungeon)
		fprintf(fff, "\n Persistent Dungeons:  ON");
	else
		fprintf(fff, "\n Persistent Dungeons:  OFF");

	fprintf(fff, "\n\n Recall Depth:");
	for (y = 1; y < max_d_idx; y++)
	{
		if (max_dlv[y])
			fprintf(fff, "\n        %s: Level %d (%d')",
			        d_name + d_info[y].name,
			        max_dlv[y], 50 * (max_dlv[y]));
	}
	fprintf(fff, "\n");

	if (noscore)
		fprintf(fff, "\n You have done something illegal.");

	if (PRACE_FLAGS(PR1_EXPERIMENTAL) || seed_dungeon)
		fprintf(fff, "\n You have done something experimental.");

	if (stupid_monsters)
		fprintf(fff, "\n Your opponents are behaving stupidly.");

	{
		char desc[80];
		cptr mimic;

		monster_race_desc(desc, p_ptr->body_monster, 0);
		fprintf(fff, "\n Your body %s %s.", (death ? "was" : "is"), desc);

		if (p_ptr->tim_mimic)
		{
			call_lua("get_mimic_info", "(d,s)", "s", p_ptr->mimic_form, "name", &mimic);
			fprintf(fff, "\n You %s disguised as a %s.", (death ? "were" : "are"), mimic);
		}
	}

	/* Where we are, if we're alive */
	if (!death) fprintf(fff, "\n You are currently %s.", describe_player_location());

	/* Monsters slain */
	{
		int k;
		s32b Total = 0;

		for (k = 1; k < max_r_idx; k++)
		{
			monster_race *r_ptr = &r_info[k];

			if (r_ptr->flags1 & (RF1_UNIQUE))
			{
				bool dead = (r_ptr->max_num == 0);
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
			fprintf(fff, "\n You have defeated %lu enemies.", Total);
	}

	hook_file = fff;
	process_hooks(HOOK_CHAR_DUMP, "()");

	/* Date */
	{
		char buf2[20];
		u32b days = bst(DAY, turn - (START_DAY * 10));

		strnfmt(buf2, 20, get_day(bst(YEAR, START_DAY * 10) + START_YEAR));
		fprintf(fff, "\n\n You started your adventure the %s of the %s year of the third age.",
		        get_month_name(bst(DAY, START_DAY * 10), wizard, FALSE), buf2);

		strnfmt(buf2, 20, get_day(bst(YEAR, turn) + START_YEAR));
		fprintf(fff, "\n %s the %s of the %s year of the third age.",
		        (death ? "You ended your adventure" : "It is currently"),
		        get_month_name(bst(DAY, turn), wizard, FALSE), buf2);
		fprintf(fff,
		        (death ? "\n Your adventure lasted %ld day%s." : "\n You have been adventuring for %ld day%s."),
		        days, (days == 1) ? "" : "s");
	}

	fprintf (fff, "\n\n");

	/* Emit the self-knowledge lines, even though they duplicate the
	   information in the grids (below), because they contain information
	   that's not in the grids (racial abilities, luck, etc.). */
	if (full)
	{
		self_knowledge(fff);
		fprintf(fff, "\n\n");
	}

	/* adds and slays */
	display_player (2);
	file_character_print_grid(fff, FALSE, TRUE);

	/* sustains and resistances */
	display_player (3);
	file_character_print_grid(fff, TRUE, FALSE);

	/* stuff */
	display_player (4);
	file_character_print_grid(fff, FALSE, FALSE);

	/* a little bit of stuff */
	display_player (5);
	file_character_print_grid(fff, FALSE, FALSE);

	/* Dump corruptions */
	if (got_corruptions())
	{
		fprintf(fff, "\n Corruption list:\n");
		dump_corruptions(fff, FALSE);
	}

	/* Dump skills */
	dump_skills(fff);
	dump_abilities(fff);

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
			dump_fates(fff);
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

			file_character_print_item(fff, index_to_label(i), &p_ptr->inventory[i], full);
		}
		fprintf(fff, "\n\n");
	}

	/* Dump the inventory */
	fprintf(fff, "  [Character Inventory]\n\n");
	for (i = 0; i < INVEN_PACK; i++)
	{
		file_character_print_item(fff, index_to_label(i), &p_ptr->inventory[i], full);
	}
	fprintf(fff, "\n\n");

	/* Print all homes in the different towns */
	for (j = 0; j < max_wf_idx; j++)
	{
		if (wf_info[j].feat == FEAT_TOWN &&
		                file_character_check_stores(&store_list, &store_list_count, &wf_info[j], 7))
			file_character_print_store(fff, &wf_info[j], 7, full);
	}
	store_list = C_FREE(store_list, store_list_count, store_type *);
	store_list_count = 0;

	/* Print all Mathom-houses in the different towns */
	for (j = 0; j < max_wf_idx; j++)
	{
		if (wf_info[j].feat == FEAT_TOWN &&
		                file_character_check_stores(&store_list, &store_list_count, &wf_info[j], 57))
			file_character_print_store(fff, &wf_info[j], 57, full);
	}
	store_list = C_FREE(store_list, store_list_count, store_type *);
	store_list_count = 0;

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

bool show_file(cptr name, cptr what, int line, int mode)
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
	bool menu = FALSE;

	/* Current help file */
	FILE *fff = NULL;

	/* Find this string (if any) */
	cptr find = NULL;

	/* Char array type of hyperlink info */
	hyperlink_type *h_ptr;

	/* Pointer to general buffer in the above */
	char *buf;

	int cur_link = 0, max_link = 0;

	/* Read size of screen for big-screen stuff -pav- */
	int wid, hgt;

	/* Allocate hyperlink data */
	MAKE(h_ptr, hyperlink_type);

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

		/* Grab permission */
		safe_setuid_grab();

		/* Open */
		fff = my_fopen(h_ptr->path, "r");

		/* Drop permission */
		safe_setuid_drop();
	}

	/* Look in "help" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "Help file '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, name);

		/* Grab permission */
		safe_setuid_grab();

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");

		/* Drop permission */
		safe_setuid_drop();
	}

	/* Look in "info" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "Info file '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_INFO, name);

		/* Grab permission */
		safe_setuid_grab();

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");

		/* Drop permission */
		safe_setuid_drop();
	}

	/* Look in "file" */
	if (!fff)
	{
		/* h_ptr->caption */
		sprintf(h_ptr->caption, "File '%s'", name);

		/* Build the filename */
		path_build(h_ptr->path, 1024, ANGBAND_DIR_FILE, name);

		/* Grab permission */
		safe_setuid_grab();

		/* Open the file */
		fff = my_fopen(h_ptr->path, "r");

		/* Drop permission */
		safe_setuid_drop();
	}

	/* Oops */
	if (!fff)
	{
		/* Message */
		msg_format("Cannot open '%s'.", name);
		msg_print(NULL);

		/* Free hyperlink info */
		KILL(h_ptr, hyperlink_type);

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

			/* Grab permission */
			safe_setuid_grab();

			/* Hack -- Re-Open the file */
			fff = my_fopen(h_ptr->path, "r");

			/* Drop permission */
			safe_setuid_drop();


			/* Oops */
			if (!fff)
			{
				/* Free hyperlink info */
				KILL(h_ptr, hyperlink_type);

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
		prt(format("[%s %ld.%ld.%ld, %s, Line %d/%d]", game_module,
		           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
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
			(void)askfor_aux(h_ptr->shower, 80);
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
				if (!show_file(tmp, NULL, 0, mode)) k = ESCAPE;
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
				if (!show_file(h_ptr->link[cur_link], NULL, h_ptr->link_line[cur_link], mode)) k = ESCAPE;
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
				if (!show_file(h_ptr->link[i], NULL, h_ptr->link_line[i], mode)) k = ESCAPE;
				break;
			}
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Free hyperlink buffers */
	KILL(h_ptr, hyperlink_type);

	/* Escape */
	if (k == ESCAPE) return (FALSE);

	/* Normal return */
	return (TRUE);
}

bool txt_to_html(cptr head, cptr foot, cptr base, cptr ext, bool force, bool recur)
{
	int i, x;

	/* Number of "real" lines passed by */
	int next = 0;

	/* Number of "real" lines in the file */
	int size = 0;

	char buf_name[80];

	/* Color of the next line */
	byte color = TERM_WHITE;

	/* Current help file */
	FILE *fff = NULL;

	/* Current aux file */
	FILE *aux = NULL;

	/* Current html file */
	FILE *htm = NULL;

	/* Char array type of hyperlink info */
	hyperlink_type *h_ptr;

	cptr file_ext;
	cptr link_prefix;
	cptr link_suffix;

	/* Pointer to general buffer in the above */
	char *buf;

	/* Allocate hyperlink data */
	MAKE(h_ptr, hyperlink_type);

	/* Setup buffer pointer */
	buf = h_ptr->rbuf;

	/* Wipe the links */
	for (i = 0; i < MAX_LINKS; i++)
	{
		h_ptr->link_x[i] = -1;
	}

	/* Parse it(yeah lua is neat :) */
	tome_dofile_anywhere(ANGBAND_DIR_HELP, "def.aux", TRUE);

	/* Ok now get the parameters */
	file_ext = string_exec_lua("return file_ext");
	link_prefix = string_exec_lua("return link_prefix");
	link_suffix = string_exec_lua("return link_suffix");

	sprintf(buf_name, "%s.%s", base, file_ext);

	if ((!force) && file_exist(buf_name)) return FALSE;

	/* Build the filename */
	path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, buf_name);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	htm = my_fopen(h_ptr->path, "w");

	/* Drop permission */
	safe_setuid_drop();

	sprintf(buf_name, "%s.%s", base, ext);

	/* h_ptr->caption */
	sprintf(h_ptr->caption, "Help file '%s'", buf_name);

	/* Build the filename */
	path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, buf_name);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fff = my_fopen(h_ptr->path, "r");

	/* Drop permission */
	safe_setuid_drop();

	/* Oops */
	if (!fff || !htm)
	{
		/* Free hyperlink info */
		KILL(h_ptr, hyperlink_type);

		my_fclose(fff);
		my_fclose(htm);

		/* Oops */
		return (TRUE);
	}

	/* Save the number of "real" lines */
	size = next;

	/* Build the filename */
	path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, head);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	aux = my_fopen(h_ptr->path, "r");

	/* Drop permission */
	safe_setuid_drop();

	/* Copy the header */
	if (aux)
	{
		while (TRUE)
		{
			char *find;

			if (my_fgets(aux, h_ptr->rbuf, 1024)) break;
			find = strstr(h_ptr->rbuf, "%t");
			if (find != NULL)
			{
				*find = '\0';
				find += 2;
				fprintf(htm, "%s", h_ptr->rbuf);
				fprintf(htm, "%s", base);
				fprintf(htm, "%s\n", find);
			}
			else
				fprintf(htm, "%s\n", h_ptr->rbuf);
		}
		my_fclose(aux);
	}

	/* Display the file */
	while (TRUE)
	{
		bool do_color = FALSE;

		/* Skip a line */
		if (my_fgets(fff, h_ptr->rbuf, 1024)) break;

		color = TERM_WHITE;

		{
			int print_x;

			/* Get a color */
			if (prefix(h_ptr->rbuf, "#####"))
			{
				color = color_char_to_attr(h_ptr->rbuf[5]);
				do_color = TRUE;
				fprintf(htm, "<FONT COLOR=\"#%02X%02X%02X\">",
				        angband_color_table[color][1],
				        angband_color_table[color][2],
				        angband_color_table[color][3]);
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
				int i;

				for (i = 5; (buf[i] >= '0') && (buf[i] <= '9'); i++)
					;
				buf[i] = '\0';
				fprintf(htm, "<A NAME=\"%s\"></A>", buf + 5);
				continue;
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
						int xx = x + 5, z = 0;
						char buff[80];
						char link_line[80], *s;

						if (buf[xx] == '/') xx += 2;

						/* Zap the link info */
						while (buf[xx] != '*')
						{
							buff[z++] = buf[xx];
							xx++;
						}
						xx++;
						buff[z] = '\0';

						/* Zap the link info */
						z = 0;
						while (buf[xx] != '[')
						{
							link_line[z++] = buf[xx];
							xx++;
						}
						xx++;
						link_line[z] = '\0';

						/* parse it */
						s = buff;
						while (*s != '.') s++;
						*s = '\0';
						s++;
						if (recur) txt_to_html(head, foot, buff, s, FALSE, recur);

						if (atoi(link_line)) fprintf(htm, "<A HREF=\"%s%s.%s%s#%d\">", link_prefix, buff, file_ext, link_suffix, atoi(link_line));
						else fprintf(htm, "<A HREF=\"%s%s.%s%s\">", link_prefix, buff, file_ext, link_suffix);

						/* Ok print the link name */
						while (buf[xx] != ']')
						{
							/* Now we treat the next char as printable */
							if (buf[xx] == '\\')
								xx++;
							fprintf(htm, "%c", buf[xx]);
							xx++;
							print_x++;
						}
						x = xx;

						fprintf(htm, "</A>");
					}
					/* Color ? */
					else if (prefix(buf + x, "[[[[["))
					{
						int xx = x + 6;

						color = color_char_to_attr(buf[x + 5]);
						fprintf(htm, "<FONT COLOR=\"#%02X%02X%02X\">",
						        angband_color_table[color][1],
						        angband_color_table[color][2],
						        angband_color_table[color][3]);

						/* Ok print the link name */
						while (buf[xx] != ']')
						{
							/* Now we treat the next char as printable */
							if (buf[xx] == '\\')
								xx++;
							fprintf(htm, "%c", buf[xx]);
							xx++;
							print_x++;
						}
						x++;
						x = xx;

						fprintf(htm, "</FONT>");
					}
					/* Hidden HTML tag? */
					else if (prefix(buf + x, "{{{{{"))
					{
						int xx = x + 5;

						/* Ok output the tag inside */
						while (buf[xx] != '}')
						{
							fprintf(htm, "%c", buf[xx]);
							xx++;
						}
						x++;
						x = xx;
					}
					else
					{
						fprintf(htm, "%c", buf[x]);
						print_x++;
					}

					x++;
				}
			}
			/* Verbatim mode: i.e: acacacac */
			else
			{
				byte old_color;

				x = 5;
				old_color = color_char_to_attr(buf[x]);
				fprintf(htm, "<FONT COLOR=\"#%02X%02X%02X\">",
				        angband_color_table[color][1],
				        angband_color_table[color][2],
				        angband_color_table[color][3]);
				while (buf[x])
				{
					color = color_char_to_attr(buf[x]);
					if (color != old_color)
						fprintf(htm, "</FONT><FONT COLOR=\"#%02X%02X%02X\">",
						        angband_color_table[color][1],
						        angband_color_table[color][2],
						        angband_color_table[color][3]);

					fprintf(htm, "%c", buf[x + 1]);
					print_x++;
					x += 2;
				}
				fprintf(htm, "</FONT>");
			}
		}
		if (do_color)
		{
			fprintf(htm, "</FONT>");
		}
		fprintf(htm, "\n");
	}

	/* Build the filename */
	path_build(h_ptr->path, 1024, ANGBAND_DIR_HELP, foot);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	aux = my_fopen(h_ptr->path, "r");

	/* Drop permission */
	safe_setuid_drop();

	/* Copy the footer */
	if (aux)
	{
		while (TRUE)
		{
			if (my_fgets(aux, h_ptr->rbuf, 1024)) break;
			fprintf(htm, "%s\n", h_ptr->rbuf);
		}
		my_fclose(aux);
	}

	/* Close the file */
	my_fclose(htm);
	my_fclose(fff);

	/* Free hyperlink buffers */
	KILL(h_ptr, hyperlink_type);

	/* Normal return */
	return (TRUE);
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

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

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

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

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
	fprintf(htm, "<meta name=\"GENERATOR\" content=\"%s %ld.%ld.%ld\"/>\n",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
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
 * Because this is dead code and hardly anyone but DG needs it.
 * IMHO this should never been included in the game code -- pelpel
 */
#if !defined(WINDOWS) && !defined(MACINTOSH) && !defined(ACORN)

#define KEY_NUM         9
static int keys_tab[KEY_NUM] =
{
	'I', 'G', 'M', 'O', 'P', 'm', 'D', 'B', 'V',
};

static cptr keys_desc[KEY_NUM] =
{
	"Interface changes:",
	"Gameplay changes:",
	"Monster changes:",
	"Object changes:",
	"Player changes:",
	"Misc changes:",
	"Dungeon changes:",
	"Bug fixes:",
	"Version:",
};

static int get_key(char c)
{
	int i;

	i = 0;
	while (keys_tab[i] != c)
		i++;
	return ((i > KEY_NUM) ? KEY_NUM : i);
}

/*
 * Some ports don't like huge stacks
 */
typedef char chg_type[500][100];

bool chg_to_txt(cptr base, cptr newname)
{
	int i, j, key = 0;

	char buf[1024];

	int lens[KEY_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
	chg_type *strs;

	/* Current chg file */
	FILE *fff = NULL;

	/* Current txt file */
	FILE *txt = NULL;

	/* Open the file */
	fff = my_fopen(base, "r");

	/* Oops */
	if (!fff)
	{
		my_fclose(fff);
		my_fclose(txt);

		/* Oops */
		return (TRUE);
	}

	/* Count the file */
	while (TRUE)
	{
		/* Skip a line */
		if (my_fgets(fff, buf, 1024)) break;

		if ((!(*buf)) ||
		                ((buf[0] >= '0') && (buf[0] <= '9')) || (buf[0] == '#')) continue;

		if (buf[1] != ' ')
			lens[get_key(buf[1])]++;
	}

	/* Open the file */
	txt = my_fopen(newname, "w");

	/* Open the file */
	fff = my_fopen(base, "r");

	/* Oops */
	if (!fff || !txt)
	{
		my_fclose(fff);
		my_fclose(txt);

		/* Oops */
		return (TRUE);
	}

	for (i = 0; i < KEY_NUM; i++) lens[i] = 0;

	/* Allocate big amount of temporary storage */
	C_MAKE(strs, KEY_NUM, chg_type);

	/* Display the file */
	while (TRUE)
	{
		/* Skip a line */
		if (my_fgets(fff, buf, 1024)) break;

		if ((!(*buf)) ||
		                ((buf[0] >= '0') && (buf[0] <= '9')) || (buf[0] == '#')) continue;

		if (buf[1] != ' ') key = get_key(buf[1]);

		if (key == KEY_NUM - 1)
			strcpy(strs[key][lens[key]++], buf + 5);
		else
			strcpy(strs[key][lens[key]++], buf + 3);
	}

	fprintf(txt, "%s changes\n", strs[KEY_NUM - 1][0]);

	for (i = 0; i < KEY_NUM - 1; i++)
	{
		if (lens[i]) fprintf(txt, "\n%s\n", keys_desc[i]);

		for (j = 0; j < lens[i]; j++)
		{
			fprintf(txt, "%s\n", strs[i][j]);
		}
	}

	/* Close the file */
	my_fclose(txt);
	my_fclose(fff);

	/* Free temporary memory */
	C_FREE(strs, KEY_NUM, chg_type);

	/* Normal return */
	return (TRUE);
}

#endif /* !WINDOWS && !MACINTOSH && !ACORN */

/*
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
	/* Save screen */
	screen_save();

	/* Peruse the main help file */
	(void)show_file("help.hlp", NULL, 0, 0);

	/* Load screen */
	screen_load();
}




/*
 * Process the player name.
 * Extract a clean "base name".
 * Build the savefile name if needed.
 */
void process_player_base()
{
	char temp[128];

#if defined(SAVEFILE_USE_UID) && !defined(PRIVATE_USER_PATH)
	/* Rename the savefile, using the player_uid and player_base */
	(void)sprintf(temp, "%d.%s", player_uid, player_base);
#else
	/* Rename the savefile, using the player_base */
	(void)sprintf(temp, "%s", player_base);
#endif

#ifdef VM
	/* Hack -- support "flat directory" usage on VM/ESA */
	(void)sprintf(temp, "%s.sv", player_base);
#endif /* VM */

	/* Build the filename */
	path_build(savefile, 1024, ANGBAND_DIR_SAVE, temp);
}

void process_player_name(bool sf)
{
	int i, k = 0;
	char tmp[50];

	/* Cannot be too long */
	if (strlen(player_base) > 15)
	{
		/* Name too long */
		quit_fmt("The name '%s' is too long!", player_base);
	}

	/* Cannot contain "icky" characters */
	for (i = 0; player_base[i]; i++)
	{
		/* No control characters */
		if (iscntrl(player_base[i]))
		{
			/* Illegal characters */
			quit_fmt("The name '%s' contains control chars!", player_base);
		}
	}


#ifdef MACINTOSH

	/* Extract "useful" letters */
	for (i = 0; player_base[i]; i++)
	{
		char c = player_base[i];

		/* Convert "dot" to "underscore" */
		if (c == '@.') c = '_';

		/* Accept all the letters */
		tmp[k++] = c;
	}

#else

	/* Extract "useful" letters */
	for (i = 0; player_base[i]; i++)
	{
		char c = player_base[i];

		/* Accept some letters */
		if (isalpha(c) || isdigit(c)) tmp[k++] = c;

		/* Convert space, dot, and underscore to underscore */
		else if (strchr("@. _", c)) tmp[k++] = '_';
	}

#endif


#if defined(WINDOWS) || defined(MSDOS)

	/* Hack -- max length */
	if (k > 8) k = 8;

#endif

	/* Terminate */
	tmp[k] = '\0';
	sprintf(player_base, tmp);

	/* Require a "base" name */
	if (!player_base[0]) strcpy(player_base, "PLAYER");


#ifdef SAVEFILE_MUTABLE

	/* Accept */
	sf = TRUE;

#endif

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
void get_name(void)
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
		strcpy(tmp, player_name);

		/* Get an input, ignore "Escape" */
		if (askfor_aux(tmp, 31)) strcpy(player_name, tmp);

		/* Process the player name */
		process_player_name(FALSE);

		/* All done */
		break;
	}

	/* Pad the name (to clear junk) */
	sprintf(tmp, "%-31.31s", player_name);

	/* Re-Draw the name (in light blue) */
	c_put_str(TERM_L_BLUE, tmp, 2, 9);

	/* Erase the prompt, etc */
	clear_from(22);
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
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
	(void)strcpy(died_from, "Quitting");
}


	/* HACK - Remove / set the CAVE_VIEW flag, since view_x / view_y
	 * is not saved, and the visible locations are not lighted correctly
	 * when the game is loaded again
	 * Alternatively forget_view() and update_view() can be used
	 */
void remove_cave_view(bool remove)
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
void do_cmd_save_game(void)
{
	panic_save = 0;   /* Fixes an apparently long-lived bug */

	remove_cave_view(TRUE);

	/* Save the current level if in a persistent level */
	save_dungeon();

	/* Autosaves do not disturb */
	if (!is_autosave)
	{
		/* Disturb the player */
		disturb(1, 0);
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
	(void)strcpy(died_from, "(saved)");

	/* Forbid suspend */
	signals_ignore_tstp();

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

	/* Allow suspend again */
	signals_handle_tstp();

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	(void)strcpy(died_from, "(alive and well)");
}



/*
 * Hack -- Calculates the total number of points earned                -JWT-
 */
long total_points(void)
{
#if 0   /* Old calculation */
	s16b max_dl = 0, i;

	for (i = 0; i < max_d_idx; i++)
		if (max_dlv[i] > max_dl)
			max_dl = max_dlv[i];

	return (p_ptr->max_exp + (100 * max_dl));
#else   /* New calculation */
	s16b max_dl = 0, i, k;
	long temp, Total = 0;
	long mult = 20; /* was 100. Divided values by 5 because of an overflow error */
	long comp_death = (p_ptr->companion_killed * 2 / 5);

	if (!comp_death) comp_death = 1;

	if (p_ptr->preserve) mult -= 1;  /* Penalize preserve, maximize modes */
	if (p_ptr->maximize) mult -= 1;
	if (auto_scum) mult -= 4;
	if (stupid_monsters) mult -= 10;
	if (small_levels) mult += ((always_small_level) ? 4 : 10);
	if (empty_levels) mult += 2;
	if (smart_learn) mult += 4;
	if (smart_cheat) mult += 4;

	if (mult < 2) mult = 2;  /* At least 10% of the original score */
	/* mult is now between 2 and 40, i.e. 10% and 200% */

	for (i = 0; i < max_d_idx; i++)
		if (max_dlv[i] > max_dl)
			max_dl = max_dlv[i];

	temp = p_ptr->lev * p_ptr->lev * p_ptr->lev * p_ptr->lev + (100 * max_dl);

	temp += p_ptr->max_exp / 5;

	temp = (temp * mult / 20);

	/* Gold increases score */
	temp += p_ptr->au / 5;

	/* Completing quest increase score */
	for (i = 0; i < max_q_idx; i++)
	{
		if (quest[i].status >= QUEST_STATUS_COMPLETED)
		{
			temp += 2000;
			temp += quest[i].level * 100;
		}
	}

	/* Death of a companion is BAD */
	temp /= comp_death;

	/* The know objects increase the score */
	/* Scan the object kinds */
	for (k = 1; k < max_k_idx; k++)
	{
		object_kind *k_ptr = &k_info[k];

		/* Hack -- skip artifacts */
		if (k_ptr->flags3 & (TR3_INSTA_ART)) continue;

		/* List known flavored objects */
		if (k_ptr->flavor && k_ptr->aware)
		{
			object_type *i_ptr;
			object_type object_type_body;

			/* Get local object */
			i_ptr = &object_type_body;

			/* Create fake object */
			object_prep(i_ptr, k);

			temp += object_value_real(i_ptr);
		}
	}

	for (k = 1; k < max_r_idx; k++)
	{
		monster_race *r_ptr = &r_info[k];

		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			bool dead = (r_ptr->max_num == 0);

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

	temp += total_bounties * 100;

	if (total_winner) temp += 1000000;



	return (temp);
#endif
}



/*
 * Centers a string within a 31 character string                -JWT-
 */
static void center_string(char *buf, cptr str)
{
	int i, j;

	/* Total length */
	i = strlen(str);

	/* Necessary border */
	j = 15 - i / 2;

	/* Mega-Hack */
	(void)sprintf(buf, "%*s%s%*s", j, "", str, 31 - i - j, "");
}


/*
 * Redefinable "print_tombstone" action
 */
bool (*tombstone_aux)(void) = NULL;


/*
 * Display a "tomb-stone"
 */
static void print_tomb(void)
{
	bool done = FALSE;

	/* Do we use a special tombstone ? */
	if (tombstone_aux)
	{
		/* Use tombstone hook */
		done = (*tombstone_aux)();
	}

	/* Print the text-tombstone */
	if (!done)
	{
		cptr p;

		char tmp[160];

		char buf[1024];
		char dummy[80];

		FILE *fp;

		time_t ct = time((time_t)0);


		/* Clear screen */
		Term_clear();

		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_FILE, "dead.txt");

		/* Grab permission */
		safe_setuid_grab();

		/* Open the News file */
		fp = my_fopen(buf, "r");

		/* Drop permission */
		safe_setuid_drop();

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


		/* King or Queen */
		if (total_winner || (p_ptr->lev > PY_MAX_LEVEL))
		{
			p = "Magnificent";
		}

		/* Normal */
		else
		{
			p = cp_ptr->titles[(p_ptr->lev - 1) / 5] + c_text;
		}

		center_string(buf, player_name);
		put_str(buf, 6, 11);

		center_string(buf, "the");
		put_str(buf, 7, 11);

		center_string(buf, p);
		put_str(buf, 8, 11);


		center_string(buf, spp_ptr->title + c_name);
		put_str(buf, 10, 11);

		(void)sprintf(tmp, "Level: %d", (int)p_ptr->lev);
		center_string(buf, tmp);
		put_str(buf, 11, 11);

		(void)sprintf(tmp, "Exp: %ld", (long)p_ptr->exp);
		center_string(buf, tmp);
		put_str(buf, 12, 11);

		(void)sprintf(tmp, "AU: %ld", (long)p_ptr->au);
		center_string(buf, tmp);
		put_str(buf, 13, 11);

		(void)sprintf(tmp, "Killed on Level %d", dun_level);
		center_string(buf, tmp);
		put_str(buf, 14, 11);


		if (strlen(died_from) > 24)
		{
			strncpy(dummy, died_from, 24);
			dummy[24] = '\0';
			(void)sprintf(tmp, "by %s.", dummy);
		}
		else
			(void)sprintf(tmp, "by %s.", died_from);

		center_string(buf, tmp);
		put_str(buf, 15, 11);


		(void)sprintf(tmp, "%-.24s", ctime(&ct));
		center_string(buf, tmp);
		put_str(buf, 17, 11);
	}
}


/*
 * Display some character info
 */
static void show_info(void)
{
	int i, j, k;
	object_type *o_ptr;
	store_type *st_ptr;

	/* Hack -- Know everything in the inven/equip */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Aware and Known */
		object_aware(o_ptr);
		object_known(o_ptr);
	}

	for (i = 1; i < max_towns; i++)
	{
		st_ptr = &town_info[i].store[7];

		/* Hack -- Know everything in the home */
		for (j = 0; j < st_ptr->stock_num; j++)
		{
			o_ptr = &st_ptr->stock[j];

			/* Skip non-objects */
			if (!o_ptr->k_idx) continue;

			/* Aware and Known */
			object_aware(o_ptr);
			object_known(o_ptr);
		}
	}

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
		(void)file_character(out_val, TRUE);

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
		item_tester_full = TRUE;
		show_equip(FALSE);
		prt("You are using: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Inventory -- if any */
	if (inven_cnt)
	{
		Term_clear();
		item_tester_full = TRUE;
		show_inven(FALSE);
		prt("You are carrying: -more-", 0, 0);
		if (inkey() == ESCAPE) return;
	}

	/* Homes in the different towns */
	for (k = 1; k < max_towns; k++)
	{
		st_ptr = &town_info[k].store[7];

#if 0 /* TODO -- actualy somewhere set the real variable */
		/* Step to the real towns */
		if (!(town_info[k].flags & (TOWN_REAL))) continue;
#endif

		/* Home -- if anything there */
		if (st_ptr->stock_num)
		{
			/* Display contents of the home */
			for (k = 0, i = 0; i < st_ptr->stock_num; k++)
			{
				/* Clear screen */
				Term_clear();

				/* Show 12 items */
				for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
				{
					char o_name[80];
					char tmp_val[80];

					/* Acquire item */
					o_ptr = &st_ptr->stock[i];

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
 * Semi-Portable High Score List Entry (128 bytes) -- BEN
 *
 * All fields listed below are null terminated ascii strings.
 *
 * In addition, the "number" fields are right justified, and
 * space padded, to the full available length (minus the "null").
 *
 * Note that "string comparisons" are thus valid on "pts".
 */

typedef struct high_score high_score;

struct high_score
{
	char what[8];                 /* Version info (string) */

	char pts[10];                 /* Total Score (number) */

	char gold[10];                 /* Total Gold (number) */

	char turns[10];                 /* Turns Taken (number) */

	char day[10];                 /* Time stamp (string) */

	char who[16];                 /* Player Name (string) */

	char uid[8];                 /* Player UID (number) */

	char sex[2];                 /* Player Sex (string) */
	char p_r[3];                 /* Player Race (number) */
	char p_s[3];             /* Player Subrace (number) */
	char p_c[3];                 /* Player Class (number) */
	char p_cs[3];            /* Player Class spec (number) */

	char cur_lev[4];                 /* Current Player Level (number) */
	char cur_dun[4];                 /* Current Dungeon Level (number) */
	char max_lev[4];                 /* Max Player Level (number) */
	char max_dun[4];                 /* Max Dungeon Level (number) */

	char arena_number[4];         /* Arena level attained -KMW- */
	char inside_arena[4];    /* Did the player die in the arena? */
	char inside_quest[4];    /* Did the player die in a quest? */
	char exit_bldg[4];         /* Can the player exit arena? Goal obtained? -KMW- */

	char how[32];                 /* Method of death (string) */
};



/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{
	/* Seek for the requested record */
	return (fd_seek(highscore_fd, (huge)(i) * sizeof(high_score)));
}


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
	/* Read the record, note failure */
	return (fd_read(highscore_fd, (char*)(score), sizeof(high_score)));
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
	/* Write the record, note failure */
	return (fd_write(highscore_fd, (char*)(score), sizeof(high_score)));
}




/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(high_score *score)
{
	int i;

	high_score the_score;

	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return ( -1);

	/* Go to the start of the highscore file */
	if (highscore_seek(0)) return ( -1);

	/* Read until we get to a higher score */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) return (i);
		if (strcmp(the_score.pts, score->pts) < 0) return (i);
	}

	/* The "last" entry is always usable */
	return (MAX_HISCORES - 1);
}


/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
	int i, slot;
	bool done = FALSE;

	high_score the_score, tmpscore;


	/* Paranoia -- it may not have opened */
	if (highscore_fd < 0) return ( -1);

	/* Determine where the score should go */
	slot = highscore_where(score);

	/* Hack -- Not on the list */
	if (slot < 0) return ( -1);

	/* Hack -- prepare to dump the new score */
	the_score = (*score);

	/* Slide all the scores down one */
	for (i = slot; !done && (i < MAX_HISCORES); i++)
	{
		/* Read the old guy, note errors */
		if (highscore_seek(i)) return ( -1);
		if (highscore_read(&tmpscore)) done = TRUE;

		/* Back up and dump the score we were holding */
		if (highscore_seek(i)) return ( -1);
		if (highscore_write(&the_score)) return ( -1);

		/* Hack -- Save the old score, for the next pass */
		the_score = tmpscore;
	}

	/* Return location used */
	return (slot);
}



/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
static void display_scores_aux(int from, int to, int note, high_score *score)
{
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
	if (highscore_seek(0)) return;

	/* Hack -- Count the high scores */
	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) break;
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

			cptr user, gold, when, aged;

			int in_arena, in_quest;

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
				if (highscore_seek(j)) break;
				if (highscore_read(&the_score)) break;
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

			in_arena = atoi(the_score.inside_arena);
			in_quest = atoi(the_score.inside_quest);

			/* Hack -- extract the gold and such */
			for (user = the_score.uid; isspace(*user); user++) /* loop */;
			for (when = the_score.day; isspace(*when); when++) /* loop */;
			for (gold = the_score.gold; isspace(*gold); gold++) /* loop */;
			for (aged = the_score.turns; isspace(*aged); aged++) /* loop */;

			/* Dump some info */
			sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
			        place, the_score.pts, the_score.who,
			        get_player_race_name(pr, ps), class_info[pc].spec[pcs].title + c_name,
			        clev);

			/* Append a "maximum level" */
			if (mlev > clev) strcat(out_val, format(" (Max %d)", mlev));

			/* Dump the first line */
			c_put_str(attr, out_val, n*4 + 2, 0);

			/* Another line of info */
			if (in_arena)
			{
				sprintf(out_val, "               Killed by %s in the Arena",
				        the_score.how);
			}
			else if (in_quest)
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
			        "               (User %s, Date %s, Gold %s, Turn %s).",
			        user, when, gold, aged);
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
 * Hack -- Display the scores in a given range and quit.
 *
 * This function is only called from "main.c" when the user asks
 * to see the "high scores".
 */
void display_scores(int from, int to)
{
	char buf[1024];

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Open the binary high score file, for reading */
	highscore_fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Paranoia -- No score file */
	if (highscore_fd < 0) quit("Score file unavailable.");

	/* Clear screen */
	Term_clear();

	/* Display the scores */
	display_scores_aux(from, to, -1, NULL);

	/* Shut the high score file */
	(void)fd_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = -1;

	/* Quit */
	quit(NULL);
}


/*
 * show_highclass - selectively list highscores based on class
 * -KMW-
 */
void show_highclass(int building)
{

	register int i = 0, j, m = 0;
	int pcs, pr, ps, pc, clev, al;
	high_score the_score;
	char buf[1024], out_val[256];

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
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permission */
	safe_setuid_grab();

	highscore_fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return;
	}

	if (highscore_seek(0)) return;

	for (i = 0; i < MAX_HISCORES; i++)
		if (highscore_read(&the_score)) break;

	m = 0;
	j = 0;
	clev = 0;

	while ((m < 9) || (j < MAX_HISCORES))
	{
		if (highscore_seek(j)) break;
		if (highscore_read(&the_score)) break;
		pr = atoi(the_score.p_r);
		ps = atoi(the_score.p_s);
		pc = atoi(the_score.p_c);
		pcs = atoi(the_score.p_cs);
		clev = atoi(the_score.cur_lev);
		al = atoi(the_score.arena_number);
		if (((pc == (building - 10)) && (building != 1) && (building != 2)) ||
		                ((building == 1) && (clev >= PY_MAX_LEVEL)) ||
		                ((building == 2) && (al > MAX_ARENA_MONS)))
		{
			sprintf(out_val, "%3d) %s the %s (Level %2d)",
			        (m + 1), the_score.who, rp_name + race_info[pr].title, clev);
			prt(out_val, (m + 7), 0);
			m++;
		}
		j++;
	}

	/* Now, list the active player if they qualify */
	if ((building == 1) && (p_ptr->lev >= PY_MAX_LEVEL))
	{
		sprintf(out_val, "You) %s the %s (Level %2d)",
		        player_name, rp_name + race_info[p_ptr->prace].title, p_ptr->lev);
		prt(out_val, (m + 8), 0);
	}
	else if ((building == 2) && (p_ptr->arena_number > MAX_ARENA_MONS))
	{
		sprintf(out_val, "You) %s the %s (Level %2d)",
		        player_name, rp_name + race_info[p_ptr->prace].title, p_ptr->lev);
		prt(out_val, (m + 8), 0);
	}
	else if ((building != 1) && (building != 2))
	{
		if ((p_ptr->lev > clev) && (p_ptr->pclass == (building - 10)))
		{
			sprintf(out_val, "You) %s the %s (Level %2d)",
			        player_name, rp_name + race_info[p_ptr->prace].title, p_ptr->lev);
			prt(out_val, (m + 8), 0);
		}
	}

	(void)fd_close(highscore_fd);
	highscore_fd = -1;
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
	register int i = 0, j, m = 0;
	int pr, ps, pc, clev, pcs, al, lastlev;
	high_score the_score;
	char buf[1024], out_val[256], tmp_str[80];

	lastlev = 0;

	/* rr9: TODO - pluralize the race */
	sprintf(tmp_str, "The Greatest of all the %s", rp_name + race_info[race_num].title);
	prt(tmp_str, 5, 3);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Open the highscore file */
	highscore_fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return;
	}

	if (highscore_seek(0)) return;

	for (i = 0; i < MAX_HISCORES; i++)
	{
		if (highscore_read(&the_score)) break;
	}

	m = 0;
	j = 0;

	while ((m < 10) && (j < i))
	{
		if (highscore_seek(j)) break;
		if (highscore_read(&the_score)) break;
		pr = atoi(the_score.p_r);
		ps = atoi(the_score.p_s);
		pc = atoi(the_score.p_c);
		pcs = atoi(the_score.p_cs);
		clev = atoi(the_score.cur_lev);
		al = atoi(the_score.arena_number);
		if (pr == race_num)
		{
			sprintf(out_val, "%3d) %s the %s (Level %3d)",
			        (m + 1), the_score.who,
			        rp_name + race_info[pr].title, clev);
			prt(out_val, (m + 7), 0);
			m++;
			lastlev = clev;
		}
		j++;
	}

	/* add player if qualified */
	if ((p_ptr->prace == race_num) && (p_ptr->lev >= lastlev))
	{
		sprintf(out_val, "You) %s the %s (Level %3d)",
		        player_name, rp_name + race_info[p_ptr->prace].title, p_ptr->lev);
		prt(out_val, (m + 8), 0);
	}

	(void)fd_close(highscore_fd);
	highscore_fd = -1;
}


/*
 * Race Legends
 * -KMW-
 */
void race_legends(void)
{
	int i, j;

	for (i = 0; i < max_rp_idx; i++)
	{
		race_score(i);
		msg_print("Hit any key to continue");
		msg_print(NULL);
		for (j = 5; j < 19; j++)
			prt("", j, 0);
	}
}




/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static errr top_twenty(void)
{
	int j;

	high_score the_score, *tmp;

	time_t ct = time((time_t*)0);


	/* Clear screen */
	Term_clear();

	/* No score file */
	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return (0);
	}

#ifndef SCORE_WIZARDS
	/* Wizard-mode pre-empts scoring */
	if (noscore & 0x000F)
	{
		msg_print("Score not registered for wizards.");
		msg_print(NULL);
		display_scores_aux(0, 10, -1, NULL);
		return (0);
	}
#endif

#ifndef SCORE_BORGS
	/* Borg-mode pre-empts scoring */
	if (noscore & 0x00F0)
	{
		msg_print("Score not registered for borgs.");
		msg_print(NULL);
		display_scores_aux(0, 10, -1, NULL);
		return (0);
	}
#endif

#ifndef SCORE_CHEATERS
	/* Cheaters are not scored */
	if (noscore & 0xFF00)
	{
		msg_print("Score not registered for cheaters.");
		msg_print(NULL);
		display_scores_aux(0, 10, -1, NULL);
		return (0);
	}
#endif

	/* Interupted */
	if (!total_winner && streq(died_from, "Interrupting"))
	{
		msg_print("Score not registered due to interruption.");
		msg_print(NULL);
		display_scores_aux(0, 10, -1, NULL);
		return (0);
	}

	/* Quitter */
	if (!total_winner && streq(died_from, "Quitting"))
	{
		msg_print("Score not registered due to quitting.");
		msg_print(NULL);
		display_scores_aux(0, 10, -1, NULL);
		return (0);
	}


	/* Clear the record */
	tmp = WIPE(&the_score, high_score);

	/* Save the version */
	sprintf(the_score.what, "%lu.%lu.%lu",
	        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn - (START_DAY * 10L));
	the_score.turns[9] = '\0';

#ifdef HIGHSCORE_DATE_HACK
	/* Save the date in a hacked up form (9 chars) */
	sprintf(the_score.day, "%-.6s %-.2s", ctime(&ct) + 4, ctime(&ct) + 22);
#else
	/* Save the date in standard form (8 chars) */
	strftime(the_score.day, 9, "%m/%d/%y", localtime(&ct));
#endif

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", player_name);

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.uid, "%7u", player_uid);
	sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_s, "%2d", p_ptr->pracem);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_cs, "%2d", p_ptr->pspec);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", dun_level);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", max_dlv[dungeon_type]);

	sprintf(the_score.arena_number, "%3d", p_ptr->arena_number);  /* -KMW- */
	sprintf(the_score.inside_arena, "%3d", p_ptr->inside_arena);
	sprintf(the_score.inside_quest, "%3d", p_ptr->inside_quest);
	sprintf(the_score.exit_bldg, "%3d", p_ptr->exit_bldg);  /* -KMW- */

	/* Save the cause of death (31 chars) */
	sprintf(the_score.how, "%-.31s", died_from);


	/* Lock (for writing) the highscore file, or fail */
	if (fd_lock(highscore_fd, F_WRLCK)) return (1);

	/* Add a new entry to the score list, see where it went */
	j = highscore_add(&the_score);

	/* Unlock the highscore file, or fail */
	if (fd_lock(highscore_fd, F_UNLCK)) return (1);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(0, 15, j, NULL);
	}

	/* Display the scores surrounding the player */
	else
	{
		display_scores_aux(0, 5, j, NULL);
		display_scores_aux(j - 2, j + 7, j, NULL);
	}


	/* Success */
	return (0);
}


/*
 * Predict the players location, and display it.
 */
errr predict_score(void)
{
	int j;

	high_score the_score;


	/* No score file */
	if (highscore_fd < 0)
	{
		msg_print("Score file unavailable.");
		msg_print(NULL);
		return (0);
	}


	/* Save the version */
	sprintf(the_score.what, "%lu.%lu.%lu",
	        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	/* Calculate and save the points */
	sprintf(the_score.pts, "%9lu", (long)total_points());
	the_score.pts[9] = '\0';

	/* Save the current gold */
	sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
	the_score.gold[9] = '\0';

	/* Save the current turn */
	sprintf(the_score.turns, "%9lu", (long)turn - (START_DAY * 10L));
	the_score.turns[9] = '\0';

	/* Hack -- no time needed */
	strcpy(the_score.day, "TODAY");

	/* Save the player name (15 chars) */
	sprintf(the_score.who, "%-.15s", player_name);

	/* Save the player info XXX XXX XXX */
	sprintf(the_score.uid, "%7u", player_uid);
	sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
	sprintf(the_score.p_r, "%2d", p_ptr->prace);
	sprintf(the_score.p_s, "%2d", p_ptr->pracem);
	sprintf(the_score.p_c, "%2d", p_ptr->pclass);
	sprintf(the_score.p_cs, "%2d", p_ptr->pspec);

	/* Save the level and such */
	sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
	sprintf(the_score.cur_dun, "%3d", dun_level);
	sprintf(the_score.max_lev, "%3d", p_ptr->max_plv);
	sprintf(the_score.max_dun, "%3d", max_dlv[dungeon_type]);

	sprintf(the_score.arena_number, "%3d", p_ptr->arena_number);  /* -KMW- */
	sprintf(the_score.inside_arena, "%3d", p_ptr->inside_arena);
	sprintf(the_score.inside_quest, "%3d", p_ptr->inside_quest);
	sprintf(the_score.exit_bldg, "%3d", p_ptr->exit_bldg);  /* -KMW- */

	/* Hack -- no cause of death */
	strcpy(the_score.how, "nobody (yet!)");


	/* See where the entry would be placed */
	j = highscore_where(&the_score);


	/* Hack -- Display the top fifteen scores */
	if (j < 10)
	{
		display_scores_aux(0, 15, j, &the_score);
	}

	/* Display some "useful" scores */
	else
	{
		display_scores_aux(0, 5, -1, NULL);
		display_scores_aux(j - 2, j + 7, j, &the_score);
	}


	/* Success */
	return (0);
}



/*
 * Change the player into a King!                        -RAK-
 */
static void kingly(void)
{
	/* Hack -- retire in town */
	dun_level = 0;

	/* Fake death */
	(void)strcpy(died_from, "Ripe Old Age");

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
	put_str(format("All Hail the Mighty %s!", sp_ptr->winner), 17, 22);

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
	int d, l, od = dungeon_type, ol = dun_level;

	for (d = 0; d < max_d_idx; d++)
	{
		dungeon_info_type *d_ptr = &d_info[d];

		for (l = d_ptr->mindepth; l <= d_ptr->maxdepth; l++)
		{
			char buf[10];

			dun_level = l;
			dungeon_type = d;
			if (get_dungeon_save(buf))
			{
				char tmp[80], name[1024];

				sprintf(tmp, "%s.%s", player_base, buf);
				path_build(name, 1024, ANGBAND_DIR_SAVE, tmp);

				/* Grab permission */
				safe_setuid_grab();

				/* Remove the dungeon save file */
				fd_kill(name);

				/* Drop permission */
				safe_setuid_drop();
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
void close_game(void)
{
	char buf[1024];


	/* Handle stuff */
	handle_stuff();

	/* Flush the messages */
	msg_print(NULL);

	/* Flush the input */
	flush();


	/* No suspending now */
	signals_ignore_tstp();

	/* Hack -- Character is now "icky" */
	character_icky = TRUE;


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Open the high score file, for reading/writing */
	highscore_fd = fd_open(buf, O_RDWR);

	/* Drop permission */
	safe_setuid_drop();

	/* Handle death */
	if (death)
	{
		/* Handle retirement */
		if (total_winner)
		{
			/* Write a note, if that option is on */
			if (take_notes)
			{
				add_note_type(NOTE_WINNER);
			}

			irc_disconnect_aux(format("Retired; %s %ld.%ld.%ld rules",
			                          game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH), FALSE);
			kingly();
		}
		else
		{
			irc_disconnect_aux(format("Killed by %s; %s %ld.%ld.%ld rules",
			                          died_from, game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH),
			                   FALSE);
		}

		/* Wipe the saved levels */
		wipe_saved();

		/* Save memories */
		if (!save_player()) msg_print("Death save failed!");

		/* You are dead */
		print_tomb();

		/* Show more info */
		show_info();

		/* Write a note */
		if (take_notes)
		{
			char long_day[30];
			char buf[80];
			time_t ct = time((time_t*)NULL);

			/* Get the date */
			strftime(long_day, 30,
			         "%Y-%m-%d at %H:%M:%S", localtime(&ct));

			/* Create string */
			sprintf(buf, "\n%s was killed by %s on %s\n", player_name,
			        died_from, long_day);

			/* Output to the notes file */
			output_note(buf);
		}

		/* Dump bones file */
		make_bones();

		/* Handle score, show Top scores */
		top_twenty();
	}

	/* Still alive */
	else
	{
		is_autosave = FALSE;

		/* Save the game */
		do_cmd_save_game();

		/* If note-taking enabled, write session end to notes file */
		if (take_notes)
		{
			add_note_type(NOTE_SAVE_GAME);
		}

		irc_disconnect_aux(format("Alive... for the time being; %s %ld.%ld.%ld rules",
		                          game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH),
		                   FALSE);

		/* Prompt for scores XXX XXX XXX */
		prt("Press Return (or Escape).", 0, 40);

		/* Predict score (or ESCAPE) */
		if (inkey() != ESCAPE) predict_score();
	}


	/* Shut the high score file */
	(void)fd_close(highscore_fd);

	/* Forget the high score fd */
	highscore_fd = -1;

	/* Allow suspending now */
	signals_handle_tstp();
}


/*
 * Handle abrupt death of the visual system
 *
 * This routine is called only in very rare situations, and only
 * by certain visual systems, when they experience fatal errors.
 *
 * XXX XXX Hack -- clear the death flag when creating a HANGUP
 * save file so that player can see tombstone when restart.
 */
void exit_game_panic(void)
{
	/* If nothing important has happened, just quit */
	if (!character_generated || character_saved) quit("panic");

	/* Mega-Hack -- see "msg_print()" */
	msg_flag = FALSE;

	/* Clear the top line */
	prt("", 0, 0);

	/* Hack -- turn off some things */
	disturb(1, 0);

	/* Mega-Hack -- Delay death */
	if (p_ptr->chp < 0) death = FALSE;

	/* Hardcode panic save */
	panic_save = 1;

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Indicate panic save */
	(void)strcpy(died_from, "(panic save)");

	/* Panic save, or get worried */
	if (!save_player()) quit("panic save failed!");

	/* Successful panic save */
	quit("panic save succeeded!");
}


/*
 * Grab a randomly selected line in lib/file/file_name
 */
errr get_rnd_line(char *file_name, char *output)
{
	FILE *fp;

	char buf[1024];

	int lines = 0;

	int line;

	int i;


	/* Clear the output buffer */
	strcpy(output, "");

	/* test hack */
	if (wizard && cheat_xtra) msg_print(file_name);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, file_name);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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
char *get_line(char* fname, cptr fdir, char *linbuf, int line)
{
	FILE* fp;
	int i;
	char buf[1024];


	/* Don't count the first line in the file, which is a comment line */
	line++;

	/* Build the filename */
	path_build(buf, 1024, fdir, fname);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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
errr get_xtra_line(char *file_name, monster_type *m_ptr, char *output)
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
	if (wizard && cheat_xtra)
	{
		msg_print(file_name);
	}

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_FILE, file_name);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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
	if (wizard && cheat_xtra)
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


#ifdef HANDLE_SIGNALS


#include <signal.h>


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
	/* Disable handler */
	(void)signal(sig, SIG_IGN);

#ifdef SIGSTOP

	/* Flush output */
	Term_fresh();

	/* Suspend the "Term" */
	Term_xtra(TERM_XTRA_ALIVE, 0);

	/* Suspend ourself */
	(void)kill(0, SIGSTOP);

	/* Resume the "Term" */
	Term_xtra(TERM_XTRA_ALIVE, 1);

	/* Redraw the term */
	Term_redraw();

	/* Flush the term */
	Term_fresh();

#endif

	/* Restore handler */
	(void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
	/* Disable handler */
	(void)signal(sig, SIG_IGN);


	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(NULL);


	/* Count the signals */
	signal_count++;


	/* Terminate dead characters */
	if (death)
	{
		/* Mark the savefile */
		(void)strcpy(died_from, "Abortion");

		/* Close stuff */
		close_game();

		/* Quit */
		quit("interrupt");
	}

	/* Allow suicide (after 5) */
	else if (signal_count >= 5)
	{
		/* Cause of "death" */
		(void)strcpy(died_from, "Interrupting");

		/* Stop playing */
		alive = FALSE;

		/* Suicide */
		death = TRUE;

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Close stuff */
		close_game();

		/* Quit */
		quit("interrupt");
	}

	/* Give warning (after 4) */
	else if (signal_count >= 4)
	{
		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);

		/* Clear the top line */
		Term_erase(0, 0, 255);

		/* Display the cause */
		Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");

		/* Flush */
		Term_fresh();
	}

	/* Give warning (after 2) */
	else if (signal_count >= 2)
	{
		/* Make a noise */
		Term_xtra(TERM_XTRA_NOISE, 0);
	}

	/* Restore handler */
	(void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
	/* Disable handler */
	(void)signal(sig, SIG_IGN);


	/* Nothing to save, just quit */
	if (!character_generated || character_saved) quit(NULL);


	/* Clear the bottom line */
	Term_erase(0, 23, 255);

	/* Give a warning */
	Term_putstr(0, 23, -1, TERM_RED,
	            "A gruesome software bug LEAPS out at you!");

	/* Message */
	Term_putstr(45, 23, -1, TERM_RED, "Panic save...");

	/* Flush output */
	Term_fresh();

	/* Panic Save */
	panic_save = 1;

	/* Panic save */
	(void)strcpy(died_from, "(panic save)");

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Attempt to save */
	if (save_player())
	{
		Term_putstr(45, 23, -1, TERM_RED, "Panic save succeeded!");
	}

	/* Save failed */
	else
	{
		Term_putstr(45, 23, -1, TERM_RED, "Panic save failed!");
	}

	/* Flush output */
	Term_fresh();

	/* Quit */
	quit(format("software bug %d %d", p_ptr->px, p_ptr->py));
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
	(void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
	(void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init(void)
{

#ifdef SIGHUP
	(void)signal(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
	(void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
	(void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
	(void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
	(void)signal(SIGFPE, handle_signal_abort);
#endif

#ifdef SIGILL
	(void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
	(void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
	(void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
	(void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
	(void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
	(void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
	(void)signal(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
	(void)signal(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
	(void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
	(void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
	(void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
	(void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
	(void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else        /* HANDLE_SIGNALS */


/*
* Do nothing
*/
void signals_ignore_tstp(void)
{}

/*
* Do nothing
*/
void signals_handle_tstp(void)
{}

/*
* Do nothing
*/
void signals_init(void)
{}


#endif        /* HANDLE_SIGNALS */
