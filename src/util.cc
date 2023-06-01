/* File: util.c */

/* Purpose: Angband utilities -BEN- */

#include "util.hpp"

#include "cli_comm.hpp"
#include "cmd3.hpp"
#include "cmd4.hpp"
#include "game.hpp"
#include "init1.hpp"
#include "messages.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "timer_type.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "z-form.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <fmt/format.h>
#include <sstream>
#include <thread>

#ifdef SET_UID
#include <pwd.h>
#endif

using boost::algorithm::iequals;
using boost::algorithm::equals;
using boost::algorithm::starts_with;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;


enum class display_option_t {
	IMMEDIATE,
	DELAY,
};

/*
 * Read a number at a specific location on the screen
 *
 * Allow numbers of any size and save the last keypress.
 */
static std::tuple<u32b, char> get_number(u32b def, u32b max, int y, int x, display_option_t display_option)
{
	auto display_number = [x, y](u32b i)
	{
		prt(fmt::format("{}", i), y, x);
	};

	/* Player has not typed anything yet */
	bool no_keys = true;

	/* Begin the input with default */
	u32b res = def;

	/* Display? */
	switch (display_option)
	{
		case display_option_t::IMMEDIATE:
			// Show current value immediately
			display_number(res);
			break;
		case display_option_t::DELAY:
			// Don't show
			break;
	}

	/* Get a command count */
	while (true)
	{
		/* Get a new keypress */
		char key = inkey();

		/* Simple editing (delete or backspace) */
		if ((key == 0x7F) || (key == KTRL('H')))
		{
			/* Override the default */
			no_keys = false;

			/* Delete a digit */
			res = res / 10;

			display_number(res);
		}

		/* Actual numeric data */
		else if (key >= '0' && key <= '9')
		{
			/* Override the default */
			if (no_keys)
			{
				no_keys = false;
				res = 0;
			}

			/* Don't overflow */
			if (((u32b)(0 - 1) - D2I(key)) / 10 < res)
			{
				/* Warn */
				bell();

				/* Limit */
				res = (max + 1 == 0) ? (u32b)(0 - 1) : max;
			}

			/* Stop count at maximum */
			else if (res * 10 + D2I(key) > max)
			{
				/* Warn */
				bell();

				/* Limit */
				res = max;
			}

			/* Increase count */
			else
			{
				/* Incorporate that digit */
				res = res * 10 + D2I(key);
			}

			/* Show current count */
			display_number(res);
		}

		/* Escape cancels */
		else if (key == ESCAPE)
		{
			return { 0, key };
		}

		/* Exit on "unusable" input */
		else
		{
			return { res, key };
		}
	}
}


/*
* Find a default user name from the system.
*/
std::string user_name()
{
#ifdef SET_UID
	/* Get the user id (?) */
	int player_uid = getuid();

	struct passwd *pw;

	/* Look up the user name */
	if ((pw = getpwuid(player_uid)))
	{
		return pw->pw_name;
	}
#endif /* SET_UID */

	/* Default to "PLAYER" if we don't have platform support */
	return "PLAYER";
}



/*
* The concept of the "file" routines below (and elsewhere) is that all
* file handling should be done using as few routines as possible, since
* every machine is slightly different, but these routines always have the
* same semantics.
*
* In fact, perhaps we should use the "path_parse()" routine below to convert
* from "canonical" filenames (optional leading tilde's, internal wildcards,
* slash as the path seperator, etc) to "system" filenames (no special symbols,
* system-specific path seperator, etc).  This would allow the program itself
* to assume that all filenames are "Unix" filenames, and explicitly "extract"
* such filenames if needed (by "path_parse()", or perhaps "path_canon()").
*
* Note that "my_fopen()" and "my_open()" and "my_make()" and "my_kill()"
* and "my_move()" and "my_copy()" should all take "canonical" filenames.
*
* Note that "canonical" filenames use a leading "slash" to indicate an absolute
* path, and a leading "tilde" to indicate a special directory, and default to a
* relative path, but MSDOS uses a leading "drivename plus colon" to indicate the
* use of a "special drive", and then the rest of the path is parsed "normally",
* and MACINTOSH uses a leading colon to indicate a relative path, and an embedded
* colon to indicate a "drive plus absolute path", and finally defaults to a file
* in the current working directory, which may or may not be defined.
*
* We should probably parse a leading "~~/" as referring to "ANGBAND_DIR". (?)
*/


#ifdef SET_UID

/*
* Extract a "parsed" path from an initial filename
* Normally, we simply copy the filename into the buffer
* But leading tilde symbols must be handled in a special way
* Replace "~/" by the home directory of the current user
*/
errr path_parse(char *buf, int max, const char *file)
{
	const char *u;
	const char *s;
	struct passwd *pw;

	/* Assume no result */
	buf[0] = '\0';

	/* No file? */
	if (!file) return ( -1);

	/* File needs no parsing */
	if (file[0] != '~')
	{
		strcpy(buf, file);
		return (0);
	}

	/* Point at the user */
	u = file + 1;

	/* Look for non-user portion of the file */
	s = strstr(u, PATH_SEP);

#ifdef GETLOGIN_BROKEN
	/* Ask the environment for the home directory */
	u = getenv("HOME");

	if (!u) return (1);

	strcpy(buf, u);
#else
	/* Look up password data for user */
	pw = getpwuid(getuid());

	/* Nothing found? */
	if (!pw) return (1);

	/* Make use of the info */
	strcpy(buf, pw->pw_dir);
#endif

	/* Append the rest of the filename, if any */
	if (s) strcat(buf, s);

	/* Success */
	return (0);
}


#else /* SET_UID */


/*
* Extract a "parsed" path from an initial filename
*
* This requires no special processing on simple machines,
* except for verifying the size of the filename.
*/
errr path_parse(char *buf, int max, const char *file)
{
	/* Accept the filename */
	strnfmt(buf, max, "%s", file);

	/* Success */
	return (0);
}


#endif /* SET_UID */


/*
* Create a new path by appending a file (or directory) to a path
*
* This requires no special processing on simple machines, except
* for verifying the size of the filename, but note the ability to
* bypass the given "path" with certain special file-names.
*
* Note that the "file" may actually be a "sub-path", including
* a path and a file.
*
* Note that this function yields a path which must be "parsed"
* using the "parse" function above.
*/
errr path_build(char *buf, int max, const char *path, const char *file)
{
	/* Special file */
	if (file[0] == '~')
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* Absolute file, on "normal" systems */
	else if (starts_with(file, PATH_SEP) && !equals(PATH_SEP, ""))
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* No path given */
	else if (!path[0])
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* Path and File */
	else
	{
		/* Build the new path */
		strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
	}

	/* Success */
	return (0);
}


/*
* Hack -- replacement for "fopen()"
*/
FILE *my_fopen(const char *file, const char *mode)
{

	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return (NULL);

	/* Attempt to fopen the file anyway */
	return (fopen(buf, mode));

}


/*
* Hack -- replacement for "fclose()"
*/
errr my_fclose(FILE *fff)
{
	/* Require a file */
	if (!fff) return ( -1);

	/* Close, check for error */
	if (fclose(fff) == EOF) return (1);

	/* Success */
	return (0);
}


/*
* Hack -- replacement for "fgets()"
*
* Read a string, without a newline, to a file
*
* Process tabs, strip internal non-printables
*/
errr my_fgets(FILE *fff, char *buf, unsigned long n)
{
	unsigned long i = 0;

	while (true)
	{
		int c = fgetc(fff);

		if (c == EOF)
		{
			/* Terminate */
			buf[i] = '\0';

			/* Success (0) if some characters were read */
			return (i == 0);
		}

		/* Handle newline -- DOS (\015\012), Mac (\015), UNIX (\012) */
		else if (c == '\r')
		{
			c = fgetc(fff);
			if (c != '\n') ungetc(c, fff);

			/* Terminate */
			buf[i] = '\0';

			/* Success */
			return (0);
		}
		else if (c == '\n')
		{
			c = fgetc(fff);
			if (c != '\r') ungetc(c, fff);

			/* Terminate */
			buf[i] = '\0';

			/* Success */
			return (0);
		}

		/* Handle tabs */
		else if (c == '\t')
		{
			/* Hack -- require room */
			if (i + 8 >= n) break;

			/* Append 1-8 spaces */
			do { buf[i++] = ' '; } while (i % 8);
		}

		/* Handle printables */
		else if (isprint(c))
		{
			/* Copy */
			buf[i++] = c;

			/* Check length */
			if (i >= n) break;
		}
	}

	/* Nothing */
	buf[0] = '\0';

	/* Failure */
	return (1);
}


/*
* Several systems have no "O_BINARY" flag
*/
#ifndef O_BINARY
# define O_BINARY 0
#endif /* O_BINARY */


/*
* Hack -- attempt to delete a file
*/
errr fd_kill(const char *file)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return ( -1);

	/* Remove */
	remove(buf);

	/* XXX XXX XXX */
	return (0);
}


/*
* Hack -- attempt to move a file
*/
errr fd_move(const char *file, const char *what)
{
	char buf[1024];
	char aux[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return ( -1);

	/* Hack -- Try to parse the path */
	if (path_parse(aux, 1024, what)) return ( -1);

	/* Rename */
	rename(buf, aux);

	/* XXX XXX XXX */
	return (0);
}


/*
* Hack -- attempt to open a file descriptor (create file)
*
* This function should fail if the file already exists
*
* Note that we assume that the file should be "binary"
*
*/
int fd_make(const char *file, int mode)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return ( -1);


/* Create the file, fail if exists, write-only, binary */
return (open(buf, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, mode));


}


/*
* Hack -- attempt to open a file descriptor (existing file)
*
* Note that we assume that the file should be "binary"
*/
int fd_open(const char *file, int flags)
{
	char buf[1024];

	/* Hack -- Try to parse the path */
	if (path_parse(buf, 1024, file)) return ( -1);


/* Attempt to open the file */
return (open(buf, flags | O_BINARY, 0));

}


/*
* Hack -- attempt to seek on a file descriptor
*/
errr fd_seek(int fd, unsigned long n)
{
	s32b p;

	/* Verify fd */
	if (fd < 0) return ( -1);

	/* Seek to the given position */
	p = lseek(fd, n, SEEK_SET);

	/* Failure */
	if (p < 0) return (1);

	/* Failure */
	if ((unsigned long)p != n) return (1);

	/* Success */
	return (0);
}


/*
* Hack -- attempt to read data from a file descriptor
*/
errr fd_read(int fd, char *buf, unsigned long n)
{
	/* Verify the fd */
	if (fd < 0) return ( -1);

#ifndef SET_UID

	/* Read pieces */
	while (n >= 16384)
	{
		/* Read a piece */
		if (read(fd, buf, 16384) != 16384) return (1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Read the final piece */
	if ((unsigned long)read(fd, buf, n) != n) return (1);

	/* Success */
	return (0);
}


/*
* Hack -- Attempt to write data to a file descriptor
*/
errr fd_write(int fd, const char *buf, unsigned long n)
{
	/* Verify the fd */
	if (fd < 0) return ( -1);

#ifndef SET_UID

	/* Write pieces */
	while (n >= 16384)
	{
		/* Write a piece */
		if (write(fd, buf, 16384) != 16384) return (1);

		/* Shorten the task */
		buf += 16384;

		/* Shorten the task */
		n -= 16384;
	}

#endif

	/* Write the final piece */
	if ((unsigned long)write(fd, buf, n) != n) return (1);

	/* Success */
	return (0);
}


/*
* Hack -- attempt to close a file descriptor
*/
errr fd_close(int fd)
{
	/* Verify the fd */
	if (fd < 0) return ( -1);

	/* Close */
	close(fd);

	/* XXX XXX XXX */
	return (0);
}


/*
* XXX XXX XXX Important note about "colors" XXX XXX XXX
*
* The "TERM_*" color definitions list the "composition" of each
* "Angband color" in terms of "quarters" of each of the three color
* components (Red, Green, Blue), for example, TERM_UMBER is defined
* as 2/4 Red, 1/4 Green, 0/4 Blue.
*
* The following info is from "Torbjorn Lindgren" (see "main-xaw.c").
*
* These values are NOT gamma-corrected.  On most machines (with the
* Macintosh being an important exception), you must "gamma-correct"
* the given values, that is, "correct for the intrinsic non-linearity
* of the phosphor", by converting the given intensity levels based
* on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
*
* The actual formula for conversion is unknown to me at this time,
* but you can use the table below for the most common gamma values.
*
* So, on most machines, simply convert the values based on the "gamma"
* of the target screen, which is usually in the range 1.5 to 1.7, and
* usually is closest to 1.7.  The converted value for each of the five
* different "quarter" values is given below:
*
*  Given     Gamma 1.0       Gamma 1.5       Gamma 1.7     Hex 1.7
*  -----       ----            ----            ----          ---
*   0/4        0.00            0.00            0.00          #00
*   1/4        0.25            0.27            0.28          #47
*   2/4        0.50            0.55            0.56          #8f
*   3/4        0.75            0.82            0.84          #d7
*   4/4        1.00            1.00            1.00          #ff
*
* Note that some machines (i.e. most IBM machines) are limited to a
* hard-coded set of colors, and so the information above is useless.
*
* Also, some machines are limited to a pre-determined set of colors,
* for example, the IBM can only display 16 colors, and only 14 of
* those colors resemble colors used by Angband, and then only when
* you ignore the fact that "Slate" and "cyan" are not really matches,
* so on the IBM, we use "orange" for both "Umber", and "Light Umber"
* in addition to the obvious "Orange", since by combining all of the
* "indeterminate" colors into a single color, the rest of the colors
* are left with "meaningful" values.
*/



/*
* Convert a decimal to a single digit octal number
*/
static char octify(unsigned int i)
{
	return (hexsym[i % 8]);
}

/*
* Convert a decimal to a single digit hex number
*/
static char hexify(unsigned int i)
{
	return (hexsym[i % 16]);
}


/*
* Convert a octal-digit into a decimal
*/
static int deoct(char c)
{
	if (isdigit(c)) return (D2I(c));
	return (0);
}

/*
* Convert a hexidecimal-digit into a decimal
*/
static int dehex(char c)
{
	if (isdigit(c)) return (D2I(c));
	if (islower(c)) return (A2I(c) + 10);
	if (isupper(c)) return (A2I(tolower(c)) + 10);
	return (0);
}


static void trigger_text_to_ascii(char **bufptr, const char **strptr)
{
	char *s = *bufptr;
	const char *str = *strptr;
	bool mod_status[MAX_MACRO_MOD];

	int i, len = 0;
	int shiftstatus = 0;
	const char *key_code;

	if (macro_template == NULL)
		return;

	for (i = 0; macro_modifier_chr[i]; i++)
		mod_status[i] = false;
	str++;

	/* Examine modifier keys */
	while (true)
	{
		for (i = 0; macro_modifier_chr[i]; i++)
		{
			len = strlen(macro_modifier_name[i]);
			if (iequals(str, macro_modifier_name[i]))
				break;
		}
		if (!macro_modifier_chr[i]) break;
		str += len;
		mod_status[i] = true;
		if ('S' == macro_modifier_chr[i])
			shiftstatus = 1;
	}
	for (i = 0; i < max_macrotrigger; i++)
	{
		len = strlen(macro_trigger_name[i]);
		char tstr[34];
		memset(tstr,'\0',sizeof(tstr));
		strncpy(tstr,str,32); // assume the keycode isn't longer than 32
		if(strlen(tstr) > 0) { // assuming we actually copied something, trim of the ] at the end.
			tstr[strlen(tstr)-1]='\0';
		}
		if (iequals(tstr, macro_trigger_name[i]) && ']' == str[len])
		{
			/* a trigger name found */
			break;
		}
	}

	/* Invalid trigger name? */
	if (i == max_macrotrigger)
	{
		str = strchr(str, ']');
		if (str)
		{
			*s++ = (char)31;
			*s++ = (char)13;
			*bufptr = s;
			*strptr = str;  /* where **strptr == ']' */
		}
		return;
	}

	key_code = macro_trigger_keycode[shiftstatus][i];
	str += len;

	*s++ = (char)31;
	for (i = 0; macro_template[i]; i++)
	{
		char ch = macro_template[i];
		int j;

		switch (ch)
		{
		case '&':
			for (j = 0; macro_modifier_chr[j]; j++)
			{
				if (mod_status[j])
					*s++ = macro_modifier_chr[j];
			}
			break;
		case '#':
			strcpy(s, key_code);
			s += strlen(key_code);
			break;
		default:
			*s++ = ch;
			break;
		}
	}
	*s++ = (char)13;

	*bufptr = s;
	*strptr = str;
}


/*
* Hack -- convert a printable string into real ascii
*
* I have no clue if this function correctly handles, for example,
* parsing "\xFF" into a (signed) char.  Whoever thought of making
* the "sign" of a "char" undefined is a complete moron.  Oh well.
*/
void text_to_ascii(char *buf, const char *str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		/* Backslash codes */
		if (*str == '\\')
		{
			/* Skip the backslash */
			str++;

			/* Macro Trigger */
			if (*str == '[')
			{
				trigger_text_to_ascii(&s, &str);
			}

			/* Hex-mode XXX */
			else if (*str == 'x')
			{
				*s = 16 * dehex(*++str);
				*s++ += dehex(*++str);
			}

			/* Hack -- simple way to specify "backslash" */
			else if (*str == '\\')
			{
				*s++ = '\\';
			}

			/* Hack -- simple way to specify "caret" */
			else if (*str == '^')
			{
				*s++ = '^';
			}

			/* Hack -- simple way to specify "space" */
			else if (*str == 's')
			{
				*s++ = ' ';
			}

			/* Hack -- simple way to specify Escape */
			else if (*str == 'e')
			{
				*s++ = ESCAPE;
			}

			/* Backspace */
			else if (*str == 'b')
			{
				*s++ = '\b';
			}

			/* Newline */
			else if (*str == 'n')
			{
				*s++ = '\n';
			}

			/* Return */
			else if (*str == 'r')
			{
				*s++ = '\r';
			}

			/* Tab */
			else if (*str == 't')
			{
				*s++ = '\t';
			}

			/* Octal-mode */
			else if (*str == '0')
			{
				*s = 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '1')
			{
				*s = 64 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '2')
			{
				*s = 64 * 2 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Octal-mode */
			else if (*str == '3')
			{
				*s = 64 * 3 + 8 * deoct(*++str);
				*s++ += deoct(*++str);
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;
			*s++ = (*str++ & 037);
		}

		/* Normal chars */
		else
		{
			*s++ = *str++;
		}
	}

	/* Terminate */
	*s = '\0';
}


static bool trigger_ascii_to_text(char **bufptr, const char **strptr)
{
	char *s = *bufptr;
	const char *str = *strptr;
	char key_code[100];
	int i;
	const char *tmp;

	if (macro_template == NULL)
		return false;

	*s++ = '\\';
	*s++ = '[';

	for (i = 0; macro_template[i]; i++)
	{
		int j;
		char ch = macro_template[i];

		switch (ch)
		{
		case '&':
			while ((tmp = strchr(macro_modifier_chr, *str)))
			{
				j = (int)(tmp - macro_modifier_chr);
				tmp = macro_modifier_name[j];
				while (*tmp) *s++ = *tmp++;
				str++;
			}
			break;
		case '#':
			for (j = 0; *str && *str != (char)13; j++)
				key_code[j] = *str++;
			key_code[j] = '\0';
			break;
		default:
			if (ch != *str) return false;
			str++;
		}
	}
	if (*str++ != (char)13) return false;

	for (i = 0; i < max_macrotrigger; i++)
	{
		if (iequals(key_code, macro_trigger_keycode[0][i])
		                || iequals(key_code, macro_trigger_keycode[1][i]))
			break;
	}
	if (i == max_macrotrigger)
		return false;

	tmp = macro_trigger_name[i];
	while (*tmp) *s++ = *tmp++;

	*s++ = ']';

	*bufptr = s;
	*strptr = str;
	return true;
}


/*
* Hack -- convert a string into a printable form
*/
void ascii_to_text(char *buf, const char *str)
{
	char *s = buf;

	/* Analyze the "ascii" string */
	while (*str)
	{
		byte i = (byte)(*str++);

		/* Macro Trigger */
		if (i == 31)
		{
			if (!trigger_ascii_to_text(&s, &str))
			{
				*s++ = '^';
				*s++ = '_';
			}
		}
		else if (i == ESCAPE)
		{
			*s++ = '\\';
			*s++ = 'e';
		}
		else if (i == ' ')
		{
			*s++ = '\\';
			*s++ = 's';
		}
		else if (i == '\b')
		{
			*s++ = '\\';
			*s++ = 'b';
		}
		else if (i == '\t')
		{
			*s++ = '\\';
			*s++ = 't';
		}
		else if (i == '\n')
		{
			*s++ = '\\';
			*s++ = 'n';
		}
		else if (i == '\r')
		{
			*s++ = '\\';
			*s++ = 'r';
		}
		else if (i == '^')
		{
			*s++ = '\\';
			*s++ = '^';
		}
		else if (i == '\\')
		{
			*s++ = '\\';
			*s++ = '\\';
		}
		else if (i < 32)
		{
			*s++ = '^';
			*s++ = i + 64;
		}
		else if (i < 127)
		{
			*s++ = i;
		}
		else if (i < 64)
		{
			*s++ = '\\';
			*s++ = '0';
			*s++ = octify(i / 8);
			*s++ = octify(i % 8);
		}
		else
		{
			*s++ = '\\';
			*s++ = 'x';
			*s++ = hexify(i / 16);
			*s++ = hexify(i % 16);
		}
	}

	/* Terminate */
	*s = '\0';
}



/*
* The "macro" package
*
* Functions are provided to manipulate a collection of macros, each
* of which has a trigger pattern string and a resulting action string
* and a small set of flags.
*/



/*
* Determine if any macros have ever started with a given character.
*/
static bool macro__use[256];


/*
* Find the macro (if any) which exactly matches the given pattern
*/
int macro_find_exact(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return ( -1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not match the pattern */
		if (!equals(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* No matches */
	return ( -1);
}


/*
* Find the first macro (if any) which contains the given pattern
*/
static int macro_find_check(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return ( -1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not contain the pattern */
		if (!starts_with(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* Nothing */
	return ( -1);
}


/*
* Find the first macro (if any) which contains the given pattern and more
*/
static int macro_find_maybe(const char *pat)
{
	int i;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return ( -1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which do not contain the pattern */
		if (!starts_with(macro__pat[i], pat)) continue;

		/* Skip macros which exactly match the pattern XXX XXX */
		if (equals(macro__pat[i], pat)) continue;

		/* Found one */
		return (i);
	}

	/* Nothing */
	return ( -1);
}


/*
* Find the longest macro (if any) which starts with the given pattern
*/
static int macro_find_ready(const char *pat)
{
	int i, t, n = -1, s = -1;

	/* Nothing possible */
	if (!macro__use[(byte)(pat[0])])
	{
		return ( -1);
	}

	/* Scan the macros */
	for (i = 0; i < macro__num; ++i)
	{
		/* Skip macros which are not contained by the pattern */
		if (!starts_with(pat, macro__pat[i])) continue;

		/* Obtain the length of this macro */
		t = strlen(macro__pat[i]);

		/* Only track the "longest" pattern */
		if ((n >= 0) && (s > t)) continue;

		/* Track the entry */
		n = i;
		s = t;
	}

	/* Result */
	return (n);
}


/*
* Add a macro definition (or redefinition).
*
* We should use "act == NULL" to "remove" a macro, but this might make it
* impossible to save the "removal" of a macro definition.  XXX XXX XXX
*
* We should consider refusing to allow macros which contain existing macros,
* or which are contained in existing macros, because this would simplify the
* macro analysis code.  XXX XXX XXX
*
* We should consider removing the "command macro" crap, and replacing it
* with some kind of "powerful keymap" ability, but this might make it hard
* to change the "roguelike" option from inside the game.  XXX XXX XXX
*/
errr macro_add(const char *pat, const char *act)
{
	int n;


	/* Paranoia -- require data */
	if (!pat || !act) return ( -1);


	/* Look for any existing macro */
	n = macro_find_exact(pat);

	/* Replace existing macro */
	if (n >= 0)
	{
		/* Free the old macro action */
		free(macro__act[n]);
	}

	/* Create a new macro */
	else
	{
		/* Acquire a new index */
		n = macro__num++;

		/* Save the pattern */
		macro__pat[n] = strdup(pat);
	}

	/* Save the action */
	macro__act[n] = strdup(act);

	/* Efficiency */
	macro__use[(byte)(pat[0])] = true;

	/* Success */
	return (0);
}



/*
* Local "need flush" variable
*/
static bool flush_later = false;


/*
* Local variable -- we are inside a "macro action"
*
* Do not match any macros until "ascii 30" is found.
*/
static bool parse_macro = false;

/*
* Local variable -- we are inside a "macro trigger"
*
* Strip all keypresses until a low ascii value is found.
*/
static bool parse_under = false;


/*
* Flush all input chars.  Actually, remember the flush,
* and do a "special flush" before the next "inkey()".
*
* This is not only more efficient, but also necessary to make sure
* that various "inkey()" codes are not "lost" along the way.
*/
void flush()
{
	/* Do it later */
	flush_later = true;
}


/*
 * Flush input if the 'flush_failure' option is set.
 */
void flush_on_failure()
{
	if (options->flush_failure)
	{
		flush();
	}
}


/*
* Flush the screen, make a noise
*/
void bell()
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Make a bell noise (if allowed) */
	if (options->ring_bell)
	{
		Term_bell();
	}

	/* Flush the input (later!) */
	flush();
}



/*
* Helper function called only from "inkey()"
*
* This function does almost all of the "macro" processing.
*
* We use the "Term_key_push()" function to handle "failed" macros, as well
* as "extra" keys read in while choosing the proper macro, and also to hold
* the action for the macro, plus a special "ascii 30" character indicating
* that any macro action in progress is complete.  Embedded macros are thus
* illegal, unless a macro action includes an explicit "ascii 30" character,
* which would probably be a massive hack, and might break things.
*
* Only 500 (0+1+2+...+29+30) milliseconds may elapse between each key in
* the macro trigger sequence.  If a key sequence forms the "prefix" of a
* macro trigger, 500 milliseconds must pass before the key sequence is
* known not to be that macro trigger.  XXX XXX XXX
*/
static char inkey_aux()
{
	int k = 0, n, p = 0, w = 0;

	char ch;

	const char *pat;
	const char *act;

	char buf[1024];


	/* Wait for a keypress */
	Term_inkey(&ch, true, true);


	/* End "macro action" */
	if (ch == 30) parse_macro = false;

	/* Inside "macro action" */
	if (ch == 30) return (ch);

	/* Inside "macro action" */
	if (parse_macro) return (ch);

	/* Inside "macro trigger" */
	if (parse_under) return (ch);


	/* Save the first key, advance */
	buf[p++] = ch;
	buf[p] = '\0';


	/* Check for possible macro */
	k = macro_find_check(buf);

	/* No macro pending */
	if (k < 0) return (ch);


	/* Wait for a macro, or a timeout */
	while (true)
	{
		/* Check for pending macro */
		k = macro_find_maybe(buf);

		/* No macro pending */
		if (k < 0) break;

		/* Check for (and remove) a pending key */
		if (0 == Term_inkey(&ch, false, true))
		{
			/* Append the key */
			buf[p++] = ch;
			buf[p] = '\0';

			/* Restart wait */
			w = 0;
		}

		/* No key ready */
		else
		{
			/* Increase "wait" */
			w += 10;

			/* Excessive delay */
			if (w >= 100) break;

			/* Delay */
			sleep_for(milliseconds(w));
		}
	}


	/* Check for available macro */
	k = macro_find_ready(buf);

	/* No macro available */
	if (k < 0)
	{
		/* Push all the keys back on the queue */
		while (p > 0)
		{
			/* Push the key, notice over-flow */
			if (Term_key_push(buf[--p])) return (0);
		}

		/* Wait for (and remove) a pending key */
		Term_inkey(&ch, true, true);

		/* Return the key */
		return (ch);
	}


	/* Get the pattern */
	pat = macro__pat[k];

	/* Get the length of the pattern */
	n = strlen(pat);

	/* Push the "extra" keys back on the queue */
	while (p > n)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(buf[--p])) return (0);
	}


	/* Begin "macro action" */
	parse_macro = true;

	/* Push the "end of macro action" key */
	if (Term_key_push(30)) return (0);


	/* Access the macro action */
	act = macro__act[k];

	/* Get the length of the action */
	n = strlen(act);

	/* Push the macro "action" onto the key queue */
	while (n > 0)
	{
		/* Push the key, notice over-flow */
		if (Term_key_push(act[--n])) return (0);
	}


	/* Hack -- Force "inkey()" to call us again */
	return (0);
}


/*
* Mega-Hack -- special "inkey_next" pointer.  XXX XXX XXX
*
* This special pointer allows a sequence of keys to be "inserted" into
* the stream of keys returned by "inkey()".  This key sequence will not
* trigger any macros, and cannot be bypassed by the Borg.  It is used
* in Angband to handle "keymaps".
*/
static const char *inkey_next = NULL;

bool inkey_flag = false;


/*
* Get a keypress from the user.
*
* This function recognizes a few "global parameters".  These are variables
* which, if set to true before calling this function, will have an effect
* on this function, and which are always reset to false by this function
* before this function returns.  Thus they function just like normal
* parameters, except that most calls to this function can ignore them.
*
* If "inkey_scan" is true, then we will immediately return "zero" if no
* keypress is available, instead of waiting for a keypress.
*
* If "inkey_base" is true, then all macro processing will be bypassed.
* If "inkey_base" and "inkey_scan" are both true, then this function will
* not return immediately, but will wait for a keypress for as long as the
* normal macro matching code would, allowing the direct entry of macro
* triggers.  The "inkey_base" flag is extremely dangerous!
*
* If "inkey_flag" is true, then we will assume that we are waiting for a
* normal command, and we will only show the cursor if "hilite_player" is
* true (or if the player is in a store), instead of always showing the
* cursor.  The various "main-xxx.c" files should avoid saving the game
* in response to a "menu item" request unless "inkey_flag" is true, to
* prevent savefile corruption.
*
* If we are waiting for a keypress, and no keypress is ready, then we will
* refresh (once) the window which was active when this function was called.
*
* Note that "back-quote" is automatically converted into "escape" for
* convenience on machines with no "escape" key.  This is done after the
* macro matching, so the user can still make a macro for "backquote".
*
* Note the special handling of "ascii 30" (ctrl-caret, aka ctrl-shift-six)
* and "ascii 31" (ctrl-underscore, aka ctrl-shift-minus), which are used to
* provide support for simple keyboard "macros".  These keys are so strange
* that their loss as normal keys will probably be noticed by nobody.  The
* "ascii 30" key is used to indicate the "end" of a macro action, which
* allows recursive macros to be avoided.  The "ascii 31" key is used by
* some of the "main-xxx.c" files to introduce macro trigger sequences.
*
* Hack -- we use "ascii 29" (ctrl-right-bracket) as a special "magic" key,
* which can be used to give a variety of "sub-commands" which can be used
* any time.  These sub-commands could include commands to take a picture of
* the current screen, to start/stop recording a macro action, etc.
*
* If "angband_term[0]" is not active, we will make it active during this
* function, so that the various "main-xxx.c" files can assume that input
* is only requested (via "Term_inkey()") when "angband_term[0]" is active.
*
* Mega-Hack -- This function is used as the entry point for clearing the
* "signal_count" variable, and of the "character_saved" variable.
*
* Hack -- Note the use of "inkey_next" to allow "keymaps" to be processed.
*/
static char inkey_real(bool inkey_scan)
{
	/* Hack -- Use the "inkey_next" pointer */
	if (inkey_next && *inkey_next)
	{
		/* Get next character, and advance */
		char ch = *inkey_next++;

		/* Cancel the various "global parameters" */
		inkey_base = inkey_flag = inkey_scan = false;

		/* Accept result */
		macro_recorder_add(ch);
		return ch;
	}

	/* Forget pointer */
	inkey_next = NULL;


	/* Access cursor state */
	char ch = '\0';
	Term_with_saved_cursor_visbility([&ch, &inkey_scan]() {

		/* Show the cursor if waiting, except sometimes in "command" mode */
		if (!inkey_scan && (!inkey_flag || options->hilite_player || character_icky))
		{
			Term_show_cursor();
		}

		/* Hack -- Activate main screen */
		auto old = Term;
		Term_with_active(angband_term[0], [&ch, &old, &inkey_scan]() {
			/* Have with flushed the output? */
			bool flushed = false;
			/* Get a key */
			while (!ch)
			{
				char kk;

				/* Hack -- Handle "inkey_scan" */
				if (!inkey_base && inkey_scan &&
						(0 != Term_inkey(&kk, false, false)))
				{
					break;
				}


				/* Hack -- Flush output once when no key ready */
				if (!flushed && (0 != Term_inkey(&kk, false, false)))
				{
					/* Hack -- activate proper term */
					Term_with_active(old, []() {
						Term_fresh();
					});

					/* Only once */
					flushed = true;
				}


				/* Hack -- Handle "inkey_base" */
				if (inkey_base)
				{
					int w = 0;

					/* Wait forever */
					if (!inkey_scan)
					{
						/* Wait for (and remove) a pending key */
						if (0 == Term_inkey(&ch, true, true))
						{
							/* Done */
							break;
						}

						/* Oops */
						break;
					}

					/* Wait */
					while (true)
					{
						/* Check for (and remove) a pending key */
						if (0 == Term_inkey(&ch, false, true))
						{
							/* Done */
							break;
						}

						/* No key ready */
						else
						{
							/* Increase "wait" */
							w += 10;

							/* Excessive delay */
							if (w >= 100) break;

							/* Delay */
							sleep_for(milliseconds(w));
						}
					}

					/* Done */
					break;
				}


				/* Get a key (see above) */
				ch = inkey_aux();


				/* Handle "control-right-bracket" */
				if ((ch == 29) || ((!options->rogue_like_commands) && (ch == KTRL('D'))))
				{
					/* Strip this key */
					ch = 0;

					/* Do an html dump */
					do_cmd_html_dump();

					/* Continue */
					continue;
				}


				/* Treat back-quote as escape */
				if (ch == '`') ch = ESCAPE;


				/* End "macro trigger" */
				if (parse_under && (ch <= 32))
				{
					/* Strip this key */
					ch = 0;

					/* End "macro trigger" */
					parse_under = false;
				}


				/* Handle "control-caret" */
				if (ch == 30)
				{
					/* Strip this key */
					ch = 0;
				}

				/* Handle "control-underscore" */
				else if (ch == 31)
				{
					/* Strip this key */
					ch = 0;

					/* Begin "macro trigger" */
					parse_under = true;
				}

				/* Inside "macro trigger" */
				else if (parse_under)
				{
					/* Strip this key */
					ch = 0;
				}
			}

		});

	});

	/* Cancel the various "global parameters" */
	inkey_base = inkey_flag = false;

	/* Return the keypress */
	macro_recorder_add(ch);
	return (ch);
}

char inkey() {
	return inkey_real(false);
}

char inkey_scan() {
	return inkey_real(true);
}

/*
* Hack -- flush
*/
static void msg_flush(int x)
{
	byte a = TERM_L_BLUE;

	/* Pause for response */
	Term_putstr(x, 0, -1, a, "-more-");

	/* Get an acceptable keypress */
	while (true)
	{
		int cmd = inkey();
		if (options->quick_messages) break;
		if ((cmd == ESCAPE) || (cmd == ' ')) break;
		if ((cmd == '\n') || (cmd == '\r')) break;
		bell();
	}

	/* Clear the line */
	Term_erase(0, 0, 255);
}

/* Display a message */
void display_message(int x, int y, int split, byte color, const char *t)
{
	int i = 0, j = 0;

	while (i < split)
	{
		if (t[i] == '#')
		{
			if (t[i + 1] == '#')
			{
				Term_putstr(x + j, y, 1, color, "#");
				i += 2;
				j++;
			}
			else
			{
				color = color_char_to_attr(t[i + 1]);
				i += 2;
			}
		}
		else
		{
			Term_putstr(x + j, y, 1, color, t + i);
			i++;
			j++;
		}
	}
}

/*
* Output a message to the top line of the screen.
*
* Break long messages into multiple pieces (40-72 chars).
*
* Allow multiple short messages to "share" the top line.
*
* Prompt the user to make sure he has a chance to read them.
*
* These messages are memorized for later reference (see above).
*
* We could do "Term_fresh()" to provide "flicker" if needed.
*
* The global "msg_flag" variable can be cleared to tell us to
* "erase" any "pending" messages still on the screen.
*
* XXX XXX XXX Note that we must be very careful about using the
* "msg_print()" functions without explicitly calling the special
* "msg_print(NULL)" function, since this may result in the loss
* of information if the screen is cleared, or if anything is
* displayed on the top line.
*
* XXX XXX XXX Note that "msg_print(NULL)" will clear the top line
* even if no messages are pending.  This is probably a hack.
*/
void cmsg_print(byte color, const char *msg)
{
	auto &messages = game->messages;

	static int p = 0;

	char *t;

	char buf[1024];

	int wid;
	Term_get_size(&wid, nullptr);
	int lim = wid - 8;

	/* Hack -- Reset */
	if (!msg_flag) p = 0;

	/* Message Length */
	int n = (msg ? strlen(msg) : 0);

	/* Hack -- flush when requested or needed */
	if (p && (!msg || ((p + n) > lim)))
	{
		/* Flush */
		msg_flush(p);

		/* Forget it */
		msg_flag = false;

		/* Reset */
		p = 0;
	}


	/* No message */
	if (!msg) return;

	/* Paranoia */
	if (n > 1000) return;


	/* Memorize the message */
	if (character_generated)
	{
		messages.add(msg, color);
	}

	/* Handle "auto_more" */
	if (options->auto_more)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MESSAGE);

		/* Force window update */
		window_stuff();

		/* Done */
		return;
	}


	/* Copy it */
	strcpy(buf, msg);

	/* Analyze the buffer */
	t = buf;

	/* Split message */
	while (n > lim)
	{
		char oops;

		int check, split;

		/* Default split */
		split = lim;

		/* Find the "best" split point */
		for (check = 40; check < lim; check++)
		{
			/* Found a valid split point */
			if (t[check] == ' ') split = check;
		}

		/* Save the split character */
		oops = t[split];

		/* Split the message */
		t[split] = '\0';

		/* Display part of the message */
		display_message(0, 0, split, color, t);

		/* Flush it */
		msg_flush(split + 1);

		/* Memorize the piece */
		/* if (character_generated) message_add(t); */

		/* Restore the split character */
		t[split] = oops;

		/* Insert a space */
		t[--split] = ' ';

		/* Prepare to recurse on the rest of "buf" */
		t += split;
		n -= split;
	}


	/* Display the tail of the message */
	display_message(p, 0, n, color, t);

	/* Window stuff */
	if (p_ptr)
	{
		p_ptr->window |= (PW_MESSAGE);
	}

	/* Remember the message */
	msg_flag = true;

	/* Remember the position */
	p += n + 1;

	/* Optional refresh */
	if (options->fresh_message) Term_fresh();
}

void cmsg_print(byte color, std::string const &msg)
{
	cmsg_print(color, msg.c_str());
}

/* Hack -- for compatibility and easy sake */
void msg_print(const char *msg)
{
	cmsg_print(TERM_WHITE, msg);
}

void msg_print(std::string const &msg)
{
	cmsg_print(TERM_WHITE, msg.c_str());
}


/*
 * Hack -- prevent "accidents" in "screen_save()" or "screen_load()"
 */
static int screen_depth = 0;


/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save()
{
	/* Hack -- Flush messages */
	msg_print(NULL);

	/* Save the screen (if legal) */
	if (screen_depth++ == 0) Term_save();

	/* Enter "icky" mode */
	character_icky = true;
}

void screen_save_no_flush()
{
	/* Enter "icky" mode */
	character_icky = true;

	/* Save the screen */
	Term_save();
}

/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load()
{
	/* Hack -- Flush messages */
	msg_print(NULL);

	/* Load the screen (if legal) */
	if (--screen_depth == 0) Term_load();

	/* Leave "icky" mode */
	character_icky = false;
}

void screen_load_no_flush()
{
	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = false;
}


/*
* Display a formatted message, using "vstrnfmt()" and "msg_print()".
*/
void msg_format(const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	cmsg_print(TERM_WHITE, buf);
}

void cmsg_format(byte color, const char *fmt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, fmt);

	/* Format the args, save the length */
	vstrnfmt(buf, 1024, fmt, vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	cmsg_print(color, buf);
}

void c_put_str(byte attr, const char *str, int row, int col)
{
	Term_putstr(col, row, -1, attr, str);
}

void c_put_str(byte attr, std::string const &str, int row, int col)
{
	Term_putstr(col, row, -1, attr, str.c_str());
}

void put_str(const char *str, int row, int col)
{
	Term_putstr(col, row, -1, TERM_WHITE, str);
}

void put_str(std::string const &str, int row, int col)
{
	Term_putstr(col, row, -1, TERM_WHITE, str.c_str());
}


/*
* Display a string on the screen using an attribute, and clear
* to the end of the line.
*/
void c_prt(byte attr, const char *str, int row, int col)
{
	/* Clear line, position cursor */
	Term_erase(col, row, 255);

	/* Dump the attr/text */
	Term_addstr( -1, attr, str);
}

void c_prt(byte attr, std::string const &s, int row, int col)
{
	c_prt(attr, s.c_str(), row, col);
}

void prt(const char *str, int row, int col)
{
	c_prt(TERM_WHITE, str, row, col);
}

void prt(std::string const &s, int row, int col)
{
	prt(s.c_str(), row, col);
}

/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "text_out()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void text_out_to_screen(byte a, const char *str)
{
	int x, y;

	int wid, h;

	int wrap;

	const char *s;


	/* Obtain the size */
	Term_get_size(&wid, &h);

	/* Obtain the cursor */
	Term_locate(&x, &y);

	/* Wrapping boundary */
	wrap = wid;

	/* Process the string */
	for (s = str; *s; s++)
	{
		char ch;

		/* Force wrap */
		if (*s == '\n')
		{
			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			continue;
		}

		/* Clean up the char */
		ch = (isprint((unsigned char) * s) ? *s : ' ');

		/* Wrap words as needed */
		if ((x >= wrap - 1) && (ch != ' '))
		{
			int i, n = 0;

			byte av[256];
			char cv[256];

			/* Wrap word */
			if (x < wrap)
			{
				/* Scan existing text */
				for (i = wrap - 2; i >= 0; i--)
				{
					/* Grab existing attr/char */
					Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == ' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == 0) n = wrap;

			/* Clear line */
			Term_erase(n, y, 255);

			/* Wrap */
			x = text_out_indent;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, 255);

			/* Wrap the word (if any) */
			for (i = n; i < wrap - 1; i++)
			{
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > wrap) x = wrap;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance */
		if (++x > wrap) x = wrap;
	}
}


/*
 * Write text to the given file and apply line-wrapping.
 *
 * Hook function for text_out(). Make sure that text_out_file points
 * to an open text-file.
 *
 * Long lines will be wrapped at column 75 ; or at a newline character.
 *
 * You must be careful to end all file output with a newline character
 * to "flush" the stored line position.
 */
void text_out_to_file(byte a, const char *str)
{
	/* Current position on the line */
	static int pos = 0;

	/* Wrap width */
	int wrap = 75;

	/* Current location within "str" */
	const char *s = str;

	/* Unused parameter */
	a;

	/* Process the string */
	while (*s)
	{
		char ch;
		int n = 0;
		int len = wrap - pos;
		int l_space = 0;

		/* If we are at the start of the line... */
		if (pos == 0)
		{
			int i;

			/* Output the indent */
			for (i = 0; i < text_out_indent; i++)
			{
				fputc(' ', text_out_file);
				pos++;
			}
		}

		/* Find length of line up to next newline or end-of-string */
		while ((n < len) && !((s[n] == '\n') || (s[n] == '\0')))
		{
			/* Mark the most recent space in the string */
			if (s[n] == ' ') l_space = n;

			/* Increment */
			n++;
		}

		/* If we have encountered no spaces */
		if ((l_space == 0) && (n == len))
		{
			/* If we are at the start of a new line */
			if (pos == text_out_indent)
			{
				len = n;
			}
			else
			{
				/* Begin a new line */
				fputc('\n', text_out_file);

				/* Reset */
				pos = 0;

				continue;
			}
		}
		else
		{
			/* Wrap at the newline */
			if ((s[n] == '\n') || (s[n] == '\0')) len = n;

			/* Wrap at the last space */
			else len = l_space;
		}

		/* Write that line to file */
		for (n = 0; n < len; n++)
		{
			/* Ensure the character is printable */
			ch = (isprint(s[n]) ? s[n] : ' ');

			/* Write out the character */
			fputc(ch, text_out_file);

			/* Increment */
			pos++;
		}

		/* Move 's' past the stuff we've written */
		s += len;

		/* If we are at the end of the string, end */
		if (*s == '\0') return;

		/* Skip newlines */
		if (*s == '\n') s++;

		/* Begin a new line */
		fputc('\n', text_out_file);

		/* Reset */
		pos = 0;

		/* Skip whitespace */
		while (*s == ' ') s++;
	}
}


/*
 * Output text to the screen or to a file depending on the selected
 * text_out hook.
 */
void text_out(const char *str)
{
	text_out_c(TERM_WHITE, str);
}

void text_out(std::string const &str)
{
	text_out_c(TERM_WHITE, str);
}


/*
 * Output text to the screen (in color) or to a file depending on the
 * selected hook.
 */
void text_out_c(byte a, const char *str)
{
	text_out_hook(a, str);
}

void text_out_c(byte a, std::string const &str)
{
	text_out_c(a, str.c_str());
}


/*
* Clear part of the screen
*/
void clear_from(int row)
{
	int y;
	int hgt;
	Term_get_size(nullptr, &hgt);

	/* Erase requested rows */
	for (y = row; y < hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}

/*
 * Try to find a matching command completion.
 * Note that this is not so friendly since it doesn't give
 * a list of possible completions.
 *
 * First arg is the string to be completed, second is it's length,
 * third is it's maximum length.
 */
static int complete_where = 0;
static char complete_buf[100];
static int complete_command(char *buf, int clen, int mlen)
{
	int i, j = 1, max = clen;
	bool gotone = false;

	/* Forget the characters after the end of the string. */
	complete_buf[clen] = '\0';

	for (i = 0; i < cli_total; i++)
	{
		cli_comm *cli_ptr = cli_info + i;

		if (!strncmp(cli_ptr->comm, complete_buf, clen))
		{
			Term_erase(0, j, 80);
			Term_putstr(0, j++, -1, TERM_WHITE, cli_ptr->comm);

			/* For the first match, copy the whole string to buf. */
			if (!gotone)
			{
				sprintf(buf, "%.*s", mlen, cli_ptr->comm);
				gotone = true;
			}
			/* For later matches, simply notice how much of buf it
			 * matches. */
			else
			{
				for (max = clen; max < mlen; max++)
				{
					if (cli_ptr->comm[max] == '\0') break;
					if (cli_ptr->comm[max] != buf[max]) break;
				}
				if (max < mlen) buf[max] = '\0';
			}
		}
	}

	return strlen(buf) + 1;
}


bool askfor_aux(std::string *buf, std::size_t max_len)
{
	// Buffer
	char cstr[1024];
	// Sanity
	assert(max_len < sizeof(cstr));
	// Copy into temporary buffer
	cstr[buf->size()] = '\0';
	buf->copy(cstr, std::string::npos);
	// Delegate
	bool result  = askfor_aux(cstr, max_len);
	// Copy back
	(*buf) = cstr;
	// Done
	return result;
}

/*
* Get some input at the cursor location.
* Assume the buffer is initialized to a default string.
* Note that this string is often "empty" (see below).
* The default buffer is displayed in yellow until cleared.
* Pressing RETURN right away accepts the default entry.
* Normal chars clear the default and append the char.
* Backspace clears the default or deletes the final char.
* ESCAPE clears the buffer and the window and returns false.
* RETURN accepts the current buffer contents and returns true.
*/
static bool askfor_aux_complete = false;

bool askfor_aux(char *buf, int len)
{
	int y, x;

	int i = 0;

	int k = 0;

        int wid, hgt;

	bool done = false;


	/* Locate the cursor */
	Term_locate(&x, &y);

        /* Get terminal size */
        Term_get_size(&wid, &hgt);

	/* Paranoia -- check column */
	if ((x < 0) || (x >= wid)) x = 0;

	/* Restrict the length */
	if (x + len > wid) len = wid - x;


	/* Paranoia -- Clip the default entry */
	buf[len - 1] = '\0';


	/* Display the default answer */
	Term_erase(x, y, len);
	Term_putstr(x, y, -1, TERM_YELLOW, buf);

	if (askfor_aux_complete)
	{
		screen_save();
		complete_where = 0;
		strncpy(complete_buf, buf, 100);
	}

	/* Process input */
	while (!done)
	{
		/* Place cursor */
		Term_gotoxy(x + k, y);

		/* Get a key */
		i = inkey();

		/* Analyze the key */
		switch (i)
		{
		case ESCAPE:
			k = 0;
			done = true;
			break;

		case '\n':
		case '\r':
			k = strlen(buf);
			done = true;
			break;

		case '\t':
			if (askfor_aux_complete && k)
			{
				screen_load();
				screen_save();
				k = complete_command(buf, k, len);
			}
			else
			{
				bell();
			}

		case 0x7F:
		case '\010':
			if (k > 0) k--;
			strncpy(complete_buf, buf, k);
			break;

		default:
			if ((k < len) && (isprint(i)))
			{
				buf[k++] = i;
				strncpy(complete_buf, buf, k);
			}
			else
			{
				bell();
			}
			break;
		}

		/* Terminate */
		buf[k] = '\0';

		/* Update the entry */
		Term_erase(x, y, len);
		Term_putstr(x, y, -1, TERM_WHITE, buf);
	}

	if (askfor_aux_complete)
	{
		screen_load();
	}

	/* Aborted */
	if (i == ESCAPE) return false;

	/* Success */
	return true;
}

bool askfor_aux_with_completion(char *buf, int len)
{
	askfor_aux_complete = true;
	bool res = askfor_aux(buf, len);
	askfor_aux_complete = false;
	return res;
}

/*
* Get a string from the user
*
* The "prompt" should take the form "Prompt: "
*
* Note that the initial contents of the string is used as
* the default response, so be sure to "clear" it if needed.
*
* We clear the input, and return false, on "ESCAPE".
*/
bool get_string(const char *prompt, char *buf, int len)
{
	bool res;

	/* Paranoia XXX XXX XXX */
	msg_print(NULL);

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux(buf, len);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}


/*
* Verify something with the user
*
* The "prompt" should take the form "Query? "
*
* Note that "[y/n]" is appended to the prompt.
*/
bool get_check(const char *prompt)
{
	int i;

	char buf[80];

	/* Paranoia XXX XXX XXX */
	msg_print(NULL);

	/* Hack -- Build a "useful" prompt */
	strnfmt(buf, 78, "%.70s[y/n] ", prompt);

	/* Prompt for it */
	prt(buf, 0, 0);

	/* Get an acceptable answer */
	while (true)
	{
		i = inkey();
		if (options->quick_messages) break;
		if (i == ESCAPE) break;
		if (strchr("YyNn", i)) break;
		bell();
	}

	/* Erase the prompt */
	prt("", 0, 0);

	/* Normal negation */
	if ((i != 'Y') && (i != 'y')) return false;

	/* Success */
	return true;
}


bool get_check(std::string const &prompt)
{
	return get_check(prompt.c_str());
}


/*
* Prompts for a keypress
*
* The "prompt" should take the form "Command: "
*
* Returns true unless the character is "Escape"
*/
bool get_com(const char *prompt, char *command)
{
	/* Paranoia XXX XXX XXX */
	msg_print(NULL);

	/* Display a prompt */
	prt(prompt, 0, 0);

	/* Get a key */
	*command = inkey();

	/* Clear the prompt */
	prt("", 0, 0);

	/* Handle "cancel" */
	if (*command == ESCAPE) return false;

	/* Success */
	return true;
}


/*
* Request a "quantity" from the user
*
* Hack -- allow "command_arg" to specify a quantity
*/
s32b get_quantity(const char *prompt, s32b max)
{
	s32b amt;
	int aamt;

	char tmp[80];

	char buf[80];


	/* Use "command_arg" */
	if (command_arg)
	{
		/* Extract a number */
		amt = command_arg;

		/* Clear "command_arg" */
		command_arg = 0;

		/* Enforce the maximum */
		if (amt > max) amt = max;

		/* Use it */
		return (amt);
	}

	/* Get the item index */
	if ((max != 1) && repeat_pull(&aamt))
	{
		amt = aamt;

		/* Enforce the maximum */
		if (amt > max) amt = max;

		/* Enforce the minimum */
		if (amt < 0) amt = 0;

		/* Use it */
		return (amt);
	}

	/* Build a prompt if needed */
	if (!prompt)
	{
		/* Build a prompt */
		sprintf(tmp, "Quantity (1-%ld): ", (long int) max);

		/* Use that prompt */
		prompt = tmp;
	}


	/* Default to one */
	amt = 1;

	/* Build the default */
	sprintf(buf, "%ld", (long int) amt);

	/* Ask for a quantity */
	if (!get_string(prompt, buf, 9)) return (0);

	/* Extract a number */
	amt = atoi(buf);

	/* A letter means "all" */
	if (isalpha(buf[0])) amt = max;

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;


	if (amt) repeat_push(amt);

	/* Return the result */
	return (amt);
}


/*
* Pause for user response XXX XXX XXX
*/
void pause_line(int row)
{
	prt("", row, 0);
	put_str("[Press any key to continue]", row, 23);
	inkey();
	prt("", row, 0);
}


/*
* Hack -- special buffer to hold the action of the current keymap
*/
static char request_command_buffer[256];

/*
* Mega-Hack -- characters for which keymaps should be ignored in
* request_command().  This MUST have at least twice as many characters as
* there are building actions in the actions[] array in store_info_type.
*/
char request_command_ignore_keymaps[MAX_IGNORE_KEYMAPS];

/*
* Mega-Hack -- flag set by do_cmd_{inven,equip}() to allow keymaps in
* auto-command mode.
*/
bool request_command_inven_mode = false;


/*
* Request a command from the user.
*
* Sets p_ptr->command_cmd, p_ptr->command_dir, p_ptr->command_rep,
* p_ptr->command_arg.  May modify p_ptr->command_new.
*
* Note that "caret" ("^") is treated specially, and is used to
* allow manual input of control characters.  This can be used
* on many machines to request repeated tunneling (Ctrl-H) and
* on the Macintosh to request "Control-Caret".
*
* Note that "backslash" is treated specially, and is used to bypass any
* keymap entry for the following character.  This is useful for macros.
*
* Note that this command is used both in the dungeon and in
* stores, and must be careful to work in both situations.
*
* Note that "p_ptr->command_new" may not work any more.  XXX XXX XXX
*/
void request_command(int shopping)
{
	int i;

	s16b cmd;
	char cmd_char;

	int mode;

	const char *act;


	/* Keymap mode */
	mode = get_keymap_mode();

	/* No command yet */
	command_cmd = 0;

	/* No "argument" yet */
	command_arg = 0;

	/* No "direction" yet */
	command_dir = 0;


	/* Get command */
	while (true)
	{
		/* Hack -- auto-commands */
		if (command_new)
		{
			/* Flush messages */
			msg_print(NULL);

			/* Use auto-command */
			cmd = command_new;

			/* Forget it */
			command_new = 0;

			/* Hack - bypass keymaps, unless asked not to */
			if (!inkey_next && !request_command_inven_mode)
			{
				inkey_next = "";
			}

			/* Mega-Hack -- turn off this flag immediately */
			request_command_inven_mode = false;
		}

		/* Get a keypress in "command" mode */
		else
		{
			/* Hack -- no flush needed */
			msg_flag = false;

			/* Activate "command mode" */
			inkey_flag = true;

			/* Get a command */
			cmd = inkey();
		}

		/* Clear top line */
		prt("", 0, 0);


		/* Command Count */
		if (cmd == '0')
		{
			int old_arg = command_arg;

			prt("Count: ", 0, 0);
			std::tie(command_arg, cmd) =
				get_number(0, 9999, 0, 7, display_option_t::DELAY);

			/* Hack -- Handle "zero" */
			if (command_arg == 0)
			{
				/* Default to 99 */
				command_arg = 99;

				/* Show current count */
				prt(fmt::format("Count: {}", command_arg), 0, 0);
			}

			/* Hack -- Handle "old_arg" */
			if (old_arg != 0)
			{
				/* Restore old_arg */
				command_arg = old_arg;

				/* Show current count */
				prt(fmt::format("Count: {}", command_arg), 0, 0);
			}

			/* Hack -- white-space means "enter command now" */
			if ((cmd == ' ') || (cmd == '\n') || (cmd == '\r'))
			{
				/* Get a real command */
				bool temp = get_com("Command: ", &cmd_char);
				cmd = cmd_char;

				if (!temp)
				{
					/* Clear count */
					command_arg = 0;

					/* Continue */
					continue;
				}
			}
		}


		/* Allow "keymaps" to be bypassed */
		if (cmd == '\\')
		{
			/* Get a real command */
			get_com("Command: ", &cmd_char);

			cmd = cmd_char;

			/* Hack -- bypass keymaps */
			if (!inkey_next) inkey_next = "";
		}


		/* Allow "control chars" to be entered */
		if (cmd == '^')
		{
			/* Get a new command and controlify it */
			if (get_com("Control: ", &cmd_char)) cmd = KTRL(cmd_char);
			else cmd = 0;
		}


		/* Look up applicable keymap */
		act = keymap_act[mode][(byte)(cmd)];

		/* Mega-Hack -- Ignore certain keymaps */
		if (shopping && cmd > 0)
		{
			for (i = 0; i < MAX_IGNORE_KEYMAPS; i++)
				if (cmd == request_command_ignore_keymaps[i])
				{
					act = NULL;
					break;
				}
		}

		/* Apply keymap if not inside a keymap already */
		if (act && !inkey_next)
		{
			/* Install the keymap (limited buffer size) */
			strnfmt(request_command_buffer, 256, "%s", act);

			/* Start using the buffer */
			inkey_next = request_command_buffer;

			/* Continue */
			continue;
		}


		/* Paranoia */
		if (!cmd) continue;


		/* Use command */
		command_cmd = cmd;

		/* Done */
		break;
	}

	/* Hack -- Auto-repeat certain commands */
	if (options->always_repeat && (command_arg <= 0))
	{
		/* Hack -- auto repeat certain commands */
		if (strchr("TBDoc+", command_cmd))
		{
			/* Repeat 99 times */
			command_arg = 99;
		}
	}

	/* Hack -- Scan equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_ptr)
		{
			continue;
		}

		/* No inscription */
		if (o_ptr->inscription.empty())
		{
			continue;
		}

		/* Obtain the inscription */
		auto s = o_ptr->inscription.c_str();

		/* Find a '^' */
		s = strchr(s, '^');

		/* Process preventions */
		while (s)
		{
			/* Check the "restriction" character */
			if ((s[1] == command_cmd) || (s[1] == '*'))
			{
				/* Hack -- Verify command */
				if (!get_check("Are you sure? "))
				{
					/* Hack -- Use space */
					command_cmd = ' ';
				}
			}

			/* Find another '^' */
			s = strchr(s + 1, '^');
		}
	}


	/* Hack -- erase the message line. */
	prt("", 0, 0);
}




/*
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (ch)
	{
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
		return true;
	}

	return false;
}


/*
 * GH
 * Called from cmd4.c and a few other places. Just extracts
 * a direction from the keymap for ch (the last direction,
 * in fact) byte or char here? I'm thinking that keymaps should
 * generally only apply to single keys, which makes it no more
 * than 128, so a char should suffice... but keymap_act is 256...
 */
int get_keymap_dir(char ch)
{
	int d = 0;

	int mode;

	const char *act;

	const char *s;


	/* Already a direction? */
	if (isdigit(ch))
	{
		d = D2I(ch);
	}
	else
	{
		/* Keymap mode */
		mode = get_keymap_mode();

		/* Extract the action (if any) */
		act = keymap_act[mode][(byte)(ch)];

		/* Analyze */
		if (act)
		{
			/* Convert to a direction */
			for (s = act; *s; ++s)
			{
				/* Use any digits in keymap */
				if (isdigit(*s)) d = D2I(*s);
			}
		}
	}

	/* Paranoia */
	if (d == 5) d = 0;

	/* Return direction */
	return (d);
}


#define REPEAT_MAX		20

/* Number of chars saved */
static int repeat__cnt = 0;

/* Current index */
static int repeat__idx = 0;

/* Saved "stuff" */
static int repeat__key[REPEAT_MAX];


void repeat_push(int what)
{
	/* Too many keys */
	if (repeat__cnt == REPEAT_MAX) return;

	/* Push the "stuff" */
	repeat__key[repeat__cnt++] = what;

	/* Prevents us from pulling keys */
	++repeat__idx;
}


bool repeat_pull(int *what)
{
	/* All out of keys */
	if (repeat__idx == repeat__cnt) return false;

	/* Grab the next key, advance */
	*what = repeat__key[repeat__idx++];

	/* Success */
	return true;
}

void repeat_check(s16b *command_ptr)
{
	assert(command_ptr);
	auto &command_cmd = *command_ptr;

	/* Ignore some commands */
	if (command_cmd == ESCAPE) return;
	if (command_cmd == ' ') return;
	if (command_cmd == '\r') return;
	if (command_cmd == '\n') return;

	/* Repeat Last Command */
	if (command_cmd == 'n')
	{
		/* Reset */
		repeat__idx = 0;

		/* Get the command */
		int what;
		if (repeat_pull(&what))
		{
			/* Save the command */
			command_cmd = what;
		}
	}

	/* Start saving new command */
	else
	{
		/* Reset */
		repeat__cnt = 0;
		repeat__idx = 0;

		/* Save this command */
		repeat_push(command_cmd);
	}
}

/*
 * Allow the user to select multiple items without pressing '0'
 */
s16b get_count(int number, int max)
{
	/* Hack -- Optional flush */
	if (options->flush_command)
	{
		flush();
	}

	/* Clear top line */
	prt("", 0, 0);

	/* Begin the input */
	prt("How many?", 0, 0);

	/* Actually get a number */
	auto [res, cmd] = get_number(number, max, 0, 10, display_option_t::IMMEDIATE);

	prt("", 0, 0);

	return res;
}

byte count_bits(u32b array)
{
	byte k = 0, i;

	if (array)
		for (i = 0; i < 32; i++)
			if (array & (1 << i)) k++;

	return k;
}

/* Return the lowered string */
void strlower(char *buf)
{
	u16b i;

	for (i = 0; (buf[i] != 0) && (i < 256) ;i++)
	{
		if (isupper(buf[i])) buf[i] = tolower(buf[i]);
	}
}

/*
 * Given monster name as string, return the index in r_info array. Name
 * must exactly match (look out for commas and the like!), or else 0 is
 * returned. Case doesn't matter. -GSN-
 */
int test_monster_name(const char *name)
{
	auto const &r_info = game->edit_data.r_info;

	for (std::size_t i = 0; i < r_info.size(); i++)
	{
		auto r_ptr = &r_info[i];
		if (r_ptr->name && iequals(name, r_ptr->name))
		{
			return (i);
		}
	}
	return (0);
}

int test_mego_name(const char *needle)
{
	const auto &re_info = game->edit_data.re_info;

	/* Scan the monsters */
	for (std::size_t i = 0; i < re_info.size(); i++)
	{
		auto name = re_info[i].name;
		if (name && iequals(name, needle))
		{
			return i;
		}
	}
	return 0;
}

/*
 * Given item name as string, return the index in k_info array. Name
 * must exactly match (look out for commas and the like!), or else -1 is
 * returned. Case doesn't matter. -DG-
 */
int test_item_name(const char *needle)
{
	auto const &k_info = game->edit_data.k_info;

	for (auto const &k_entry: k_info)
	{
		if (iequals(needle, k_entry.second->name))
		{
			return k_entry.first;
		}
	}

	return -1;
}

/*
 * Break scalar time
 */
s32b bst(s32b what, s32b t)
{
	s32b turns = t + (10 * DAY_START);

	switch (what)
	{
	case MINUTE:
		return ((turns / 10 / MINUTE) % 60);
	case HOUR:
		return (turns / 10 / (HOUR) % 24);
	case DAY:
		return (turns / 10 / (DAY) % 365);
	case YEAR:
		return (turns / 10 / (YEAR));
	default:
		return (0);
	}
}

std::string get_day(s32b day_no)
{
	// Convert the number
	std::string day(std::to_string(day_no));
	// Suffix
	if ((day_no / 10) == 1)
	{
		return day + "th";
	}
	else if ((day_no % 10) == 1)
	{
		return day + "st";
	}
	else if ((day_no % 10) == 2)
	{
		return day + "nd";
	}
	else if ((day_no % 10) == 3)
	{
		return day + "rd";
	}
	else
	{
		return day + "th";
	}
}

std::string get_player_race_name(int pr, int ps)
{
	auto const &race_info = game->edit_data.race_info;
	auto const &race_mod_info = game->edit_data.race_mod_info;

	if (ps)
	{
		if (race_mod_info[ps].place)
		{
			return std::string(race_info[pr].title) + " " + race_mod_info[ps].title;
		}
		else
		{
			return std::string(race_mod_info[ps].title) + " " + race_info[pr].title;
		}
	}
	else
	{
		return std::string(race_info[pr].title);
	}
}

/*
 * Ask to select an item in a list
 */
int ask_menu(const char *ask, const std::vector<std::string> &items)
{
	int ret = -1, i, start = 0;
	char c;
	int size = static_cast<int>(items.size()); // Convert to int to avoid warnings

	/* Save the screen */
	screen_save_no_flush();

	while (true)
	{
		/* Display list */
		screen_load_no_flush();
		screen_save_no_flush();

		prt(ask, 0, 0);
		for (i = start; (i < size) && (i < start + 20); i++)
		{
			prt(format("%c) %s", I2A(i - start), items[i].c_str()), i - start + 1, 0);
		}

		/* Wait for user input */
		c = inkey();

		/* Leave the screen */
		if (c == ESCAPE) break;

		/* Scroll */
		else if (c == '+')
		{
			if (start + 20 < size)
				start += 20;
			continue;
		}

		/* Scroll */
		else if (c == '-')
		{
			start -= 20;
			if (start < 0) start = 0;
			continue;
		}

		/* Good selection */
		else
		{
			c = tolower(c);
			if (A2I(c) + start >= size)
			{
				bell();
				continue;
			}
			if (A2I(c) + start < 0)
			{
				bell();
				continue;
			}

			ret = A2I(c) + start;
			break;
		}
	}

	screen_load_no_flush();

	return ret;
}

/*
 * Displays a box
 */
void draw_box(int y, int x, int h, int w)
{
	int i, j;

	for (i = x + 1; i < x + w; i++)
		for (j = y + 1; j < y + h; j++)
			Term_putch(i, j, TERM_L_BLUE, ' ');

	for (i = x; i < x + w; i++)
	{
		c_put_str(TERM_L_BLUE, "-", y, i);
		c_put_str(TERM_L_BLUE, "-", y + h, i);
	}
	for (i = y; i < y + h; i++)
	{
		c_put_str(TERM_L_BLUE, "|", i, x);
		c_put_str(TERM_L_BLUE, "|", i, x + w);
	}
	Term_putch(x, y, TERM_L_BLUE, '/');
	Term_putch(x + w, y, TERM_L_BLUE, '\\');
	Term_putch(x, y + h, TERM_L_BLUE, '\\');
	Term_putch(x + w, y + h, TERM_L_BLUE, '/');
}


/*
 * Displays a scrollable boxed list with a selected item
 */
void display_list(int y, int x, int h, int w, const char *title, std::vector<std::string> const &list, std::size_t begin, std::size_t sel, byte sel_color)
{
	draw_box(y, x, h, w);
	c_put_str(TERM_L_BLUE, title, y, x + ((w - strlen(title)) / 2));

	for (int i = 0; i < h - 1; i++)
	{
		byte color = TERM_WHITE;

		if (i + begin >= list.size())
		{
			break;
		}

		if (i + begin == sel) color = sel_color;
		c_put_str(color, list[i + begin].c_str(), y + 1 + i, x + 1);
	}
}

bool input_box_auto(std::string const &prompt, std::string *buf, std::size_t max)
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);

	auto const y = hgt / 2;
	auto const x = wid / 2;

	auto const smax = std::max(prompt.size(), max) + 1;

	draw_box(y - 1, x - (smax / 2), 3, smax);
	c_put_str(TERM_WHITE, prompt.c_str(), y, x - (prompt.size() / 2));

	Term_gotoxy(x - (smax / 2) + 1, y + 1);
	return askfor_aux(buf, max);
}

std::string input_box_auto(std::string const &title, std::size_t max)
{
	std::string buf;
	input_box_auto(title, &buf, max);
	return buf;
}

char msg_box_auto(std::string const &text)
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);

	auto const y = hgt / 2;
	auto const x = wid / 2;

	draw_box(y - 1, x - ((text.size() + 1) / 2), 2, text.size() + 1);
	c_put_str(TERM_WHITE, text.c_str(), y, x - ((text.size() + 1) / 2) + 1);
	return inkey();
}

/*
 * Timers
 */
timer_type *new_timer(void (*callback)(), s32b delay)
{
	auto &timers = game->timers;

	timer_type *t_ptr = new timer_type(callback, delay);
	timers.push_back(t_ptr);
	return t_ptr;
}

int get_keymap_mode()
{
	if (options->rogue_like_commands)
	{
		return KEYMAP_MODE_ROGUE;
	}
	else
	{
		return KEYMAP_MODE_ORIG;
	}
}

/**
 * Determines if a map location is fully inside the outer walls
 */
bool in_bounds(int y, int x)
{
	return (y > 0) && (x > 0) && (y < cur_hgt-1) && (x < cur_wid-1);
}

/**
 * Determines if a map location is on or inside the outer walls
 */
bool in_bounds2(int y, int x)
{
	return (y >= 0) && (x >= 0) && (y < cur_hgt) && (x < cur_wid);
}

/**
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(Y,X)" always implies "in_bounds2(Y,X)".
 */
bool panel_contains(int y, int x)
{
	return (y >= panel_row_min) && (y <= panel_row_max) && (x >= panel_col_min) && (x <= panel_col_max);
}
