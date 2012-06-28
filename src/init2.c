/* File: init2.c */

/* Purpose: Initialisation (part 2) -BEN- */

#include "angband.h"

#include <assert.h>

#include "messages.h"
#include "quark.h"

/*
 * This file is used to initialise various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * The "init1.c" file is used only to parse the ascii template files.
 */



/*
 * Find the default paths to all of our important sub-directories.
 *
 * The purpose of each sub-directory is described in "variable.c".
 *
 * All of the sub-directories should, by default, be located inside
 * the main "lib" directory, whose location is very system dependant.
 *
 * This function takes a writable buffer, initially containing the
 * "path" to the "lib" directory, for example, "/pkg/lib/angband/",
 * or a system dependant string, for example, ":lib:".  The buffer
 * must be large enough to contain at least 32 more characters.
 *
 * Various command line options may allow some of the important
 * directories to be changed to user-specified directories, most
 * importantly, the "info" and "user" and "save" directories,
 * but this is done after this function, see "main.c".
 *
 * In general, the initial path should end in the appropriate "PATH_SEP"
 * string.  All of the "sub-directory" paths (created below or supplied
 * by the user) will NOT end in the "PATH_SEP" string, see the special
 * "path_build()" function in "util.c" for more information.
 *
 * Mega-Hack -- support fat raw files under NEXTSTEP, using special
 * "suffixed" directories for the "ANGBAND_DIR_DATA" directory, but
 * requiring the directories to be created by hand by the user.
 *
 * Hack -- first we free all the strings, since this is known
 * to succeed even if the strings have not been allocated yet,
 * as long as the variables start out as "NULL".  This allows
 * this function to be called multiple times, for example, to
 * try several base "path" values until a good one is found.
 */
void init_file_paths(char *path)
{
	char *tail;
	int   pathlen;

	/*** Free everything ***/

	/* Free the main path */
	string_free(ANGBAND_DIR);

	/* Free the sub-paths */
	string_free(ANGBAND_DIR_CORE);
	string_free(ANGBAND_DIR_DNGN);
	string_free(ANGBAND_DIR_DATA);
	string_free(ANGBAND_DIR_EDIT);
	string_free(ANGBAND_DIR_FILE);
	string_free(ANGBAND_DIR_HELP);
	string_free(ANGBAND_DIR_INFO);
	string_free(ANGBAND_DIR_MODULES);
	string_free(ANGBAND_DIR_NOTE);
	string_free(ANGBAND_DIR_SAVE);
	string_free(ANGBAND_DIR_SCPT);
	string_free(ANGBAND_DIR_PREF);
	string_free(ANGBAND_DIR_PATCH);
	string_free(ANGBAND_DIR_USER);
	string_free(ANGBAND_DIR_XTRA);
	string_free(ANGBAND_DIR_CMOV);


	/*** Prepare the "path" ***/

	pathlen = strlen(path);

	/* Hack -- save the main directory without trailing PATH_SEP if present */
	if (strlen(PATH_SEP) > 0 && pathlen > 0)
	{
		int seplen = strlen(PATH_SEP);

		if (strcmp(path + pathlen - seplen, PATH_SEP) == 0)
		{
			path[pathlen - seplen] = '\0';
			ANGBAND_DIR = string_make(path);
			path[pathlen - seplen] = *PATH_SEP;
		}
		else
		{
			ANGBAND_DIR = string_make(path);
		}
	}
	else
	{
		ANGBAND_DIR = string_make(path);
	}

	/* Prepare to append to the Base Path */
	tail = path + pathlen;



	/*** Build the sub-directory names ***/

	/* Build a path name */
	strcpy(tail, "core");
	ANGBAND_DIR_CORE = string_make(path);

	/* Build a path name */
	strcpy(tail, "dngn");
	ANGBAND_DIR_DNGN = string_make(path);

	/* Build a path name */
	strcpy(tail, "data");
	ANGBAND_DIR_DATA = string_make(path);

	/* Build a path name */
	strcpy(tail, "edit");
	ANGBAND_DIR_EDIT = string_make(path);

	/* Build a path name */
	strcpy(tail, "file");
	ANGBAND_DIR_FILE = string_make(path);

	/* Build a path name */
	strcpy(tail, "help");
	ANGBAND_DIR_HELP = string_make(path);

	/* Build a path name */
	strcpy(tail, "info");
	ANGBAND_DIR_INFO = string_make(path);

	/* Build a path name */
	strcpy(tail, "mods");
	ANGBAND_DIR_MODULES = string_make(path);

	/* Build a path name */
	strcpy(tail, "patch");
	ANGBAND_DIR_PATCH = string_make(path);

	/* Build a path name */
	strcpy(tail, "scpt");
	ANGBAND_DIR_SCPT = string_make(path);

	/* Build a path name */
	strcpy(tail, "pref");
	ANGBAND_DIR_PREF = string_make(path);

	/* synchronize with module_reset_dir */
	{
		char user_path[1024];

		/* Get an absolute path from the file name */
		path_parse(user_path, 1024, PRIVATE_USER_PATH);
		strcat(user_path, USER_PATH_VERSION);
		ANGBAND_DIR_USER = string_make(user_path);
		ANGBAND_DIR_NOTE = string_make(user_path);
		ANGBAND_DIR_CMOV = string_make(user_path);
#ifdef PRIVATE_USER_PATH_MODULES
		ANGBAND_DIR_MODULES = string_make(user_path);
#endif
#ifdef PRIVATE_USER_PATH_DATA
		{
		char user_path_data[1024];
		strcpy(user_path_data, user_path);
		strcat(user_path_data, "/data");
		ANGBAND_DIR_DATA = string_make(user_path_data);
		}
#endif

		/* Savefiles are in user directory */
		strcat(user_path, "/save");
		ANGBAND_DIR_SAVE = string_make(user_path);
	}

	/* Build a path name */
	strcpy(tail, "xtra");
	ANGBAND_DIR_XTRA = string_make(path);
}


/**
 * Realloc the given character array.
 */
static void z_realloc(char **p, size_t n) {
	/* realloc doesn't really support size 0, but we want to shrink the allocated area regardless. */
	if (n == 0) {
		n = 1;
	}
	/* do the reallocation */
	*p = realloc(*p, n);
	if (*p == NULL) {
		quit("Error during realloc.");
        }
}

/*
 * Hack -- help give useful error messages
 */
s16b error_idx;
s16b error_line;


/*
 * Hack -- help initialise the fake "name" and "text" arrays when
 * parsing an "ascii" template file.
 */
u32b fake_name_size;
u32b fake_text_size;


/*
 * Standard error message text
 */
static cptr err_str[9] =
{
	NULL,
	"parse error",
	"obsolete file",
	"missing record header",
	"non-sequential records",
	"invalid flag specification",
	"undefined directive",
	"out of memory",
	"invalid skill chart"
};


/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, str);
	Term_fresh();
}



/*
 * Initialise the "f_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_f_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(f_head, header);

	/* Save the "record" information */
	f_head->info_num = max_f_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "f_name" and "f_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "f_info" array */
	C_MAKE(f_info, f_head->info_num, feature_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(f_name, fake_name_size, char);
	C_MAKE(f_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "f_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'f_info.txt' file.");

	/* Parse the file */
	err = init_f_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'f_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'f_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&f_name, f_head->name_size);
        z_realloc(&f_text, f_head->text_size);

	/* Success */
	return (0);
}



/*
 * Initialise the "k_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_k_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(k_head, header);

	/* Save the "record" information */
	k_head->info_num = max_k_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "k_name" and "k_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "k_info" array */
	C_MAKE(k_info, k_head->info_num, object_kind);

	/* Hack -- make "fake" arrays */
	C_MAKE(k_name, fake_name_size, char);
	C_MAKE(k_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "k_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'k_info.txt' file.");

	/* Parse the file */
	err = init_k_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'k_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'k_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&k_name, k_head->name_size);
        z_realloc(&k_text, k_head->text_size);

	/* Success */
	return (0);
}



/*
 * Initialise the "set_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_set_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(set_head, header);

	/* Save the "record" information */
	set_head->info_num = max_set_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "set_name" and "set_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "set_info" array */
	C_MAKE(set_info, set_head->info_num, set_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(set_name, fake_name_size, char);
	C_MAKE(set_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "set_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'set_info.txt' file.");

	/* Parse the file */
	err = init_set_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'set_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'set_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&set_name, set_head->name_size);
        z_realloc(&set_text, set_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "a_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_a_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(a_head, header);

	/* Save the "record" information */
	a_head->info_num = max_a_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "a_name" and "a_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "a_info" array */
	C_MAKE(a_info, a_head->info_num, artifact_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(a_name, fake_name_size, char);
	C_MAKE(a_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "a_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'a_info.txt' file.");

	/* Parse the file */
	err = init_a_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'a_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'a_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&a_name, a_head->name_size);
        z_realloc(&a_text, a_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "s_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_s_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(s_head, header);

	/* Save the "record" information */
	s_head->info_num = max_s_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "a_name" and "a_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "s_info" array */
	C_MAKE(s_info, s_head->info_num, skill_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(s_name, fake_name_size, char);
	C_MAKE(s_text, fake_text_size, char);

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "s_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 's_info.txt' file.");

	/* Parse the file */
	err = init_s_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 's_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 's_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&s_name, s_head->name_size);
        z_realloc(&s_text, s_head->text_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "ab_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_ab_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];



	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(ab_head, header);

	/* Save the "record" information */
	ab_head->info_num = max_ab_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "a_name" and "a_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "ab_info" array */
	C_MAKE(ab_info, ab_head->info_num, ability_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(ab_name, fake_name_size, char);
	C_MAKE(ab_text, fake_text_size, char);

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ab_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'ab_info.txt' file.");

	/* Parse the file */
	err = init_ab_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'ab_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'ab_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&ab_name, ab_head->name_size);
        z_realloc(&ab_text, ab_head->text_size);

	/* Success */
	return (0);
}



/*
 * Initialise the "e_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_e_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(e_head, header);

	/* Save the "record" information */
	e_head->info_num = max_e_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "e_name" and "e_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "e_info" array */
	C_MAKE(e_info, e_head->info_num, ego_item_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(e_name, fake_name_size, char);
	C_MAKE(e_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "e_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'e_info.txt' file.");

	/* Parse the file */
	err = init_e_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'e_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'e_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&e_name, e_head->name_size);
        z_realloc(&e_text, e_head->text_size);

	/* Success */
	return (0);
}



/*
 * Initialise the "ra_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_ra_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];

	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(ra_head, header);

	/* Save the "record" information */
	ra_head->info_num = max_ra_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "ra_name" and "ra_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "ra_info" array */
	C_MAKE(ra_info, ra_head->info_num, randart_part_type);

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ra_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'ra_info.txt' file.");

	/* Parse the file */
	err = init_ra_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'ra_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'ra_info.txt' file.");
	}

	/* Success */
	return (0);
}



/*
 * Initialise the "r_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_r_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(r_head, header);

	/* Save the "record" information */
	r_head->info_num = max_r_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "r_name" and "r_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "r_info" array */
	C_MAKE(r_info, r_head->info_num, monster_race);

	/* Hack -- make "fake" arrays */
	C_MAKE(r_name, fake_name_size, char);
	C_MAKE(r_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "r_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'r_info.txt' file.");

	/* Parse the file */
	err = init_r_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'r_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'r_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&r_name, r_head->name_size);
        z_realloc(&r_text, r_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "re_info" array
 *
 * Note that we let each entry have a unique "name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_re_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(re_head, header);

	/* Save the "record" information */
	re_head->info_num = max_re_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "re_name" */
	fake_name_size = FAKE_NAME_SIZE;

	/* Allocate the "re_info" array */
	C_MAKE(re_info, re_head->info_num, monster_ego);

	/* Hack -- make "fake" arrays */
	C_MAKE(re_name, fake_name_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "re_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 're_info.txt' file.");

	/* Parse the file */
	err = init_re_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 're_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 're_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&re_name, re_head->name_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "d_info" array
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_d_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(d_head, header);

	/* Save the "record" information */
	d_head->info_num = max_d_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "d_name" and "d_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "d_info" array */
	C_MAKE(d_info, d_head->info_num, dungeon_info_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(d_name, fake_name_size, char);
	C_MAKE(d_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "d_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'd_info.txt' file.");

	/* Parse the file */
	err = init_d_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'd_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'd_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&d_name, d_head->name_size);
        z_realloc(&d_text, d_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "player" arrays
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_player_info(void)
{
	int i;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(rp_head, header);

	/* Save the "record" information */
	rp_head->info_num = max_rp_idx;


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(rmp_head, header);

	/* Save the "record" information */
	rmp_head->info_num = max_rmp_idx;


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(c_head, header);

	/* Save the "record" information */
	c_head->info_num = max_c_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "rp_name" and "rp_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "rp_info" array */
	C_MAKE(race_info, rp_head->info_num, player_race);

	/* Hack -- make "fake" arrays */
	C_MAKE(rp_name, fake_name_size, char);
	C_MAKE(rp_text, fake_text_size, char);

	/* Allocate the "rmp_info" array */
	C_MAKE(race_mod_info, rmp_head->info_num, player_race_mod);

	/* Hack -- make "fake" arrays */
	C_MAKE(rmp_name, fake_name_size, char);
	C_MAKE(rmp_text, fake_text_size, char);

	/* Allocate the "c_info" array */
	C_MAKE(class_info, c_head->info_num, player_class);

	/* Hack -- make "fake" arrays */
	C_MAKE(c_name, fake_name_size, char);
	C_MAKE(c_text, fake_text_size, char);

	/* Allocate the "bg" array */
	C_MAKE(bg, max_bg_idx, hist_type);

	/* Allocate the "meta_class" array */
	C_MAKE(meta_class_info, max_mc_idx, meta_class_type);
	for (i = 0; i < max_mc_idx; i++)
	{
		C_MAKE(meta_class_info[i].classes, max_c_idx, s16b);
	}

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "p_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'p_info.txt' file.");

	/* Parse the file */
	err = init_player_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'p_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'p_info.txt' file.");
	}

        /* Reallocate arrays. */
        z_realloc(&rp_name, rp_head->name_size);
	z_realloc(&rp_text, rp_head->text_size);
        z_realloc(&rmp_name, rmp_head->name_size);
	z_realloc(&rmp_text, rmp_head->text_size);
        z_realloc(&c_name, c_head->name_size);
	z_realloc(&c_text, c_head->text_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "st_info" array
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_st_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(st_head, header);

	/* Save the "record" information */
	st_head->info_num = max_st_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "st_name" and "st_text" */
	fake_name_size = FAKE_NAME_SIZE;

	/* Allocate the "st_info" array */
	C_MAKE(st_info, st_head->info_num, store_info_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(st_name, fake_name_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "st_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'st_info.txt' file.");

	/* Parse the file */
	err = init_st_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'st_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'st_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&st_name, st_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "ow_info" array
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_ow_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(ow_head, header);

	/* Save the "record" information */
	ow_head->info_num = max_ow_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "ow_name" and "ow_text" */
	fake_name_size = FAKE_NAME_SIZE;

	/* Allocate the "ow_info" array */
	C_MAKE(ow_info, ow_head->info_num, owner_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(ow_name, fake_name_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ow_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'ow_info.txt' file.");

	/* Parse the file */
	err = init_ow_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'ow_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'ow_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&ow_name, ow_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "ba_info" array
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_ba_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(ba_head, header);

	/* Save the "record" information */
	ba_head->info_num = max_ba_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "ba_name" and "ba_text" */
	fake_name_size = FAKE_NAME_SIZE;

	/* Allocate the "ba_info" array */
	C_MAKE(ba_info, ba_head->info_num, store_action_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(ba_name, fake_name_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ba_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'ba_info.txt' file.");

	/* Parse the file */
	err = init_ba_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'ba_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'ba_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&ba_name, ba_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "wf_info" array
 *
 * Note that we let each entry have a unique "name" and "short name" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_wf_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(wf_head, header);

	/* Save the "record" information */
	wf_head->info_num = max_wf_idx;


	/*** Make the fake arrays ***/

	/* Assume the size of "wf_name" and "wf_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "r_info" array */
	C_MAKE(wf_info, wf_head->info_num, wilderness_type_info);

	/* Hack -- make "fake" arrays */
	C_MAKE(wf_name, fake_name_size, char);
	C_MAKE(wf_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "wf_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'wf_info.txt' file.");

	/* Parse the file */
	err = init_wf_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d df 'wf_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'wf_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&wf_name, wf_head->name_size);
        z_realloc(&wf_text, wf_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "t_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_t_info(void)
{
	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(t_head, header);

	/* Save the "record" information */
	t_head->info_num = max_t_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "t_name" and "t_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "t_info" array */
	C_MAKE(t_info, t_head->info_num, trap_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(t_name, fake_name_size, char);
	C_MAKE(t_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "tr_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'tr_info.txt' file.");

	/* Parse the file */
	err = init_t_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'tr_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'tr_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&t_name, t_head->name_size);
        z_realloc(&t_text, t_head->text_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "al_info" array
 *
 * Not a flat array, but an array none the less
 */
errr init_al_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];

	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(al_head, header);

	/* Save the "record" information */
	al_head->info_num = max_al_idx;




	fake_text_size = FAKE_TEXT_SIZE;
	fake_name_size = FAKE_NAME_SIZE;

	/* Allocate the "al_info" array */
	C_MAKE(alchemist_recipes, al_head->info_num, alchemist_recipe);

	/* Allocate the fake arrays */
	/* ok, so we fudge a bit, but
	   fake text size will ALWAYS be larger
	   than 32*5*sizeof(artifact_select_flag) = 10 int and 5 bytes
	   which is the maximum size of the a_select_flags array
	   */
	C_MAKE(al_name, fake_name_size, char);

	{
		char *hack;
		C_MAKE(hack, fake_text_size, char);
		a_select_flags = (artifact_select_flag *) hack;
	}

	/*** Load the ascii template file ***/


	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "al_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'al_info.txt' file.");

	/* Parse the file */
	err = init_al_info_txt(fp, buf);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'al_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'al_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&al_name, al_head->name_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "v_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
errr init_v_info(void)
{
	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(v_head, header);

	/* Save the "record" information */
	v_head->info_num = max_v_idx;


	/*** Make the fake arrays ***/

	/* Fake the size of "v_name" and "v_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "k_info" array */
	C_MAKE(v_info, v_head->info_num, vault_type);

	/* Hack -- make "fake" arrays */
	C_MAKE(v_name, fake_name_size, char);
	C_MAKE(v_text, fake_text_size, char);


	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "v_info.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) quit("Cannot open 'v_info.txt' file.");

	/* Parse the file */
	err = init_v_info_txt(fp, buf, TRUE);

	/* Close it */
	my_fclose(fp);

	/* Errors */
	if (err)
	{
		cptr oops;

		/* Error string */
		oops = (((err > 0) && (err < 8)) ? err_str[err] : "unknown");

		/* Oops */
		msg_format("Error %d at line %d of 'v_info.txt'.", err, error_line);
		msg_format("Record %d contains a '%s' error.", error_idx, oops);
		msg_format("Parsing '%s'.", buf);
		msg_print(NULL);

		/* Quit */
		quit("Error in 'v_info.txt' file.");
	}

        /* Reduce sizes of the arrays */
        z_realloc(&v_name, v_head->name_size);
        z_realloc(&v_text, v_head->text_size);

	/* Success */
	return (0);
}

/*
 * Initialize the very basic arrays
 */
static void init_basic()
{
	int i;

	/* Macro variables */
	C_MAKE(macro__pat, MACRO_MAX, cptr);
	C_MAKE(macro__act, MACRO_MAX, cptr);
	C_MAKE(macro__cmd, MACRO_MAX, bool_);

	/* Macro action buffer */
	C_MAKE(macro__buf, 1024, char);

	/* Extended trigger macros */
	C_MAKE(cli_info, CLI_MAX, cli_comm);

	/* Wipe the directory list */
	for (i = 0; i < 255; i++)
	{
		scansubdir_result[i] = NULL;
	}
}


/*
 * Initialise misc. values
 */
static errr init_misc(void)
{
	int xstart = 0;
	int ystart = 0;
	int i;

	/*** Prepare the various "bizarre" arrays ***/

	/* Initialize quark subsystem */
	quark_init();

	/* Initialize messages subsystem */
	message_init();

	/* Initialize game */
	process_hooks(HOOK_INIT_GAME, "(s)", "begin");

	/* Initialise the values */
	process_dungeon_file("misc.txt", &ystart, &xstart, 0, 0, TRUE, FALSE);

	/* Init the spell effects */
	for (i = 0; i < MAX_EFFECTS; i++)
		effects[i].time = 0;

	/* Initialize timers */
	TIMER_INERTIA_CONTROL =
		new_timer(meta_inertia_control_timer_callback,
			  10);
	TIMER_AGGRAVATE_EVIL =
		new_timer(timer_aggravate_evil_callback,
			  10);

	return 0;
}


/*
 * Initialise town array
 */
static errr init_towns(void)
{
	int i = 0, j = 0;

	/*** Prepare the Towns ***/

	/* Allocate the towns */
	C_MAKE(town_info, max_towns, town_type);

	for (i = 1; i < max_towns; i++)
	{
		if (i <= max_real_towns) town_info[i].flags |= (TOWN_REAL);

		/* Allocate the stores */
		C_MAKE(town_info[i].store, max_st_idx, store_type);

		/* Fill in each store */
		for (j = 0; j < max_st_idx; j++)
		{
			/* Access the store */
			store_type *st_ptr = &town_info[i].store[j];

			/* Know who we are */
			st_ptr->st_idx = j;

			/* Assume full stock */
			st_ptr->stock_size = 0;
		}
	}
	return 0;
}

void create_stores_stock(int t)
{
	int j;
	town_type *t_ptr = &town_info[t];

	if (t_ptr->stocked) return;

	for (j = 0; j < max_st_idx; j++)
	{
		store_type *st_ptr = &t_ptr->store[j];

		/* Assume full stock */
		st_ptr->stock_size = st_info[j].max_obj;

		/* Allocate the stock */
		C_MAKE(st_ptr->stock, st_ptr->stock_size, object_type);
	}
	t_ptr->stocked = TRUE;
}

/*
 * Pointer to wilderness_map
 */
typedef wilderness_map *wilderness_map_ptr;

/*
 * Initialise wilderness map array
 */
static errr init_wilderness(void)
{
	int i;

	/* Allocate the wilderness (two-dimension array) */
	C_MAKE(wild_map, max_wild_y, wilderness_map_ptr);
	C_MAKE(wild_map[0], max_wild_x * max_wild_y, wilderness_map);

	/* Init the other pointers */
	for (i = 1; i < max_wild_y; i++)
		wild_map[i] = wild_map[0] + i * max_wild_x;

	/* No encounter right now */
	generate_encounter = FALSE;

	return 0;
}

/*
 * Initialise some other arrays
 */
static errr init_other(void)
{
	int i, n;

	/*** Prepare the "dungeon" information ***/

	/* Allocate and Wipe the special gene flags */
	C_MAKE(m_allow_special, max_r_idx, bool_);
	C_MAKE(k_allow_special, max_k_idx, bool_);
	C_MAKE(a_allow_special, max_a_idx, bool_);


	/*** Prepare "vinfo" array ***/

	/* Used by "update_view()" */
	(void)vinfo_init();


	/* Allocate and Wipe the object list */
	C_MAKE(o_list, max_o_idx, object_type);

	/* Allocate and Wipe the monster list */
	C_MAKE(m_list, max_m_idx, monster_type);

	/* Allocate and Wipe the to keep monster list */
	C_MAKE(km_list, max_m_idx, monster_type);

	/* Allocate and Wipe the max dungeon level */
	C_MAKE(max_dlv, max_d_idx, s16b);

	/* Allocate and Wipe the special levels */
	for (i = 0; i < MAX_DUNGEON_DEPTH; i++)
	{
		C_MAKE(special_lvl[i], max_d_idx, bool_);
	}

	/* Allocate and wipe each line of the cave */
	for (i = 0; i < MAX_HGT; i++)
	{
		/* Allocate one row of the cave */
		C_MAKE(cave[i], MAX_WID, cave_type);
	}

	/*** Pre-allocate the basic "auto-inscriptions" ***/

	/* The "basic" feelings */
	(void)quark_add("cursed");
	(void)quark_add("broken");
	(void)quark_add("average");
	(void)quark_add("good");

	/* The "extra" feelings */
	(void)quark_add("excellent");
	(void)quark_add("worthless");
	(void)quark_add("special");
	(void)quark_add("terrible");

	/* Some extra strings */
	(void)quark_add("uncursed");
	(void)quark_add("on sale");


	/*** Prepare the options ***/

	/* Scan the options */
	for (i = 0; option_info[i].o_desc; i++)
	{
		int os = option_info[i].o_page;
		int ob = option_info[i].o_bit;

		/* Set the "default" options */
		if (option_info[i].o_var)
		{
			/* Accept */
			option_mask[os] |= (1L << ob);

			/* Set */
			if (option_info[i].o_norm)
			{
				/* Set */
				option_flag[os] |= (1L << ob);
			}

			/* Clear */
			else
			{
				/* Clear */
				option_flag[os] &= ~(1L << ob);
			}
		}
	}

	/* Analyze the windows */
	for (n = 0; n < 8; n++)
	{
		/* Analyze the options */
		for (i = 0; i < 32; i++)
		{
			/* Accept */
			if (window_flag_desc[i])
			{
				/* Accept */
				window_mask[n] |= (1L << i);
			}
		}
	}


	/*
	 * Install the various level generators
	 */
	add_level_generator("dungeon", level_generate_dungeon, TRUE, TRUE, TRUE, TRUE);
	add_level_generator("maze", level_generate_maze, TRUE, TRUE, TRUE, TRUE);
	add_level_generator("life", level_generate_life, TRUE, TRUE, TRUE, TRUE);

	/*** Pre-allocate space for the "format()" buffer ***/

	/* Hack -- Just call the "format()" function */
	(void)format("%s (%s).", "Dark God <darkgod@t-o-m-e.net>", MAINTAINER);

	/* Success */
	return (0);
}



/*
 * Initialise some other arrays
 */
static errr init_alloc(void)
{
	int i, j;

	object_kind *k_ptr;

	monster_race *r_ptr;

	alloc_entry *table;

	s16b num[MAX_DEPTH_MONSTER];

	s16b aux[MAX_DEPTH_MONSTER];

	/*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	C_WIPE(&aux, MAX_DEPTH_MONSTER, s16b);

	/* Clear the "num" array */
	C_WIPE(&num, MAX_DEPTH_MONSTER, s16b);

	/* Size of "alloc_kind_table" */
	alloc_kind_size = 0;

	/* Scan the objects */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				/* Count the entries */
				alloc_kind_size++;

				/* Group by level */
				num[k_ptr->locale[j]]++;
			}
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH_MONSTER; i++)
	{
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town objects!");


	/*** Initialise object allocation info ***/

	/* Allocate the alloc_kind_table */
	C_MAKE(alloc_kind_table, alloc_kind_size, alloc_entry);

	/* Access the table entry */
	table = alloc_kind_table;

	/* Scan the objects */
	for (i = 1; i < max_k_idx; i++)
	{
		k_ptr = &k_info[i];

		/* Scan allocation pairs */
		for (j = 0; j < 4; j++)
		{
			/* Count the "legal" entries */
			if (k_ptr->chance[j])
			{
				int p, x, y, z;

				/* Extract the base level */
				x = k_ptr->locale[j];

				/* Extract the base probability */
				p = (100 / k_ptr->chance[j]);

				/* Skip entries preceding our locale */
				y = (x > 0) ? num[x - 1] : 0;

				/* Skip previous entries at this locale */
				z = y + aux[x];

				/* Load the entry */
				table[z].index = i;
				table[z].level = x;
				table[z].prob1 = p;
				table[z].prob2 = p;
				table[z].prob3 = p;

				/* Another entry complete for this locale */
				aux[x]++;
			}
		}
	}


	/*** Analyze monster allocation info ***/

	/* Clear the "aux" array */
	C_WIPE(&aux, MAX_DEPTH_MONSTER, s16b);

	/* Clear the "num" array */
	C_WIPE(&num, MAX_DEPTH_MONSTER, s16b);

	/* Size of "alloc_race_table" */
	alloc_race_size = 0;

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Legal monsters */
		if (r_ptr->rarity)
		{
			/* Count the entries */
			alloc_race_size++;

			/* Group by level */
			num[r_ptr->level]++;
		}
	}

	/* Collect the level indexes */
	for (i = 1; i < MAX_DEPTH_MONSTER; i++)
	{
		/* Group by level */
		num[i] += num[i - 1];
	}

	/* Paranoia */
	if (!num[0]) quit("No town monsters!");


	/*** Initialise monster allocation info ***/

	/* Allocate the alloc_race_table */
	C_MAKE(alloc_race_table, alloc_race_size, alloc_entry);

	/* Access the table entry */
	table = alloc_race_table;

	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		/* Get the i'th race */
		r_ptr = &r_info[i];

		/* Count valid pairs */
		if (r_ptr->rarity)
		{
			int p, x, y, z;

			/* Extract the base level */
			x = r_ptr->level;

			/* Extract the base probability */
			p = (100 / r_ptr->rarity);

			/* Skip entries preceding our locale */
			y = (x > 0) ? num[x - 1] : 0;

			/* Skip previous entries at this locale */
			z = y + aux[x];

			/* Load the entry */
			table[z].index = i;
			table[z].level = x;
			table[z].prob1 = p;
			table[z].prob2 = p;
			table[z].prob3 = p;

			/* Another entry complete for this locale */
			aux[x]++;
		}
	}


	/* Success */
	return (0);
}

/* Init the sets in a_info */
void init_sets_aux()
{
	int i, j;

	for (i = 0; i < max_a_idx; i++)
		a_info[i].set = -1;
	for (i = 0; i < max_set_idx; i++)
	{
		for (j = 0; j < set_info[i].num; j++)
		{
			a_info[set_info[i].arts[j].a_idx].set = i;
		}
	}
}

/*
 * Mark guardians and their artifacts with SPECIAL_GENE flag
 */
static void init_guardians(void)
{
	int i;

	/* Scan dungeons */
	for (i = 0; i < max_d_idx; i++)
	{
		dungeon_info_type *d_ptr = &d_info[i];

		/* Mark the guadian monster */
		if (d_ptr->final_guardian)
		{
			monster_race *r_ptr = &r_info[d_ptr->final_guardian];

			r_ptr->flags9 |= RF9_SPECIAL_GENE;

			/* Mark the final artifact */
			if (d_ptr->final_artifact)
			{
				artifact_type *a_ptr = &a_info[d_ptr->final_artifact];

				a_ptr->flags4 |= TR4_SPECIAL_GENE;
			}

			/* Mark the final object */
			if (d_ptr->final_object)
			{
				object_kind *k_ptr = &k_info[d_ptr->final_object];

				k_ptr->flags4 |= TR4_SPECIAL_GENE;
			}

			/* Give randart if there are no final artifacts */
			if (!(d_ptr->final_artifact) && !(d_ptr->final_object))
			{
				r_ptr->flags7 |= RF7_DROP_RANDART;
			}
		}
	}
}

/*
 * Hack -- Explain a broken "lib" folder and quit (see below).
 *
 * XXX XXX XXX This function is "messy" because various things
 * may or may not be initialised, but the "plog()" and "quit()"
 * functions are "supposed" to work under any conditions.
 */
static void init_angband_aux(cptr why)
{
	/* Why */
	plog(why);

	/* Explain */
	plog("The 'lib' directory is probably missing or broken.");

	/* More details */
	plog("Perhaps the archive was not extracted correctly.");

	/* Explain */
	plog("See the 'README' file for more information.");

	/* Quit with error */
	quit("Fatal Error.");
}

/*
 * Hack -- main Angband initialisation entry point
 *
 * Verify some files, display the "news.txt" file, create
 * the high score file, initialise all internal arrays, and
 * load the basic "user pref files".
 *
 * Note that we blindly assume that "news2.txt" exists. XXX
 *
 * Be very careful to keep track of the order in which things
 * are initialised, in particular, the only thing *known* to
 * be available when this function is called is the "z-term.c"
 * package, and that may not be fully initialised until the
 * end of this function, when the default "user pref files"
 * are loaded and "Term_xtra(TERM_XTRA_REACT,0)" is called.
 *
 * Note that this function attempts to verify the "news" file,
 * and the game aborts (cleanly) on failure, since without the
 * "news" file, it is likely that the "lib" folder has not been
 * correctly located.  Otherwise, the news file is displayed for
 * the user.
 *
 * Note that this function attempts to verify (or create) the
 * "high score" file, and the game aborts (cleanly) on failure,
 * since one of the most common "extraction" failures involves
 * failing to extract all sub-directories (even empty ones), such
 * as by failing to use the "-d" option of "pkunzip", or failing
 * to use the "save empty directories" option with "Compact Pro".
 * This error will often be caught by the "high score" creation
 * code below, since the "lib/apex" directory, being empty in the
 * standard distributions, is most likely to be "lost", making it
 * impossible to create the high score file.
 *
 * Note that various things are initialised by this function,
 * including everything that was once done by "init_some_arrays".
 *
 * This initialisation involves the parsing of special files
 * in the "lib/data" and sometimes the "lib/edit" directories.
 *
 * Note that the "template" files are initialised first, since they
 * often contain errors.  This means that macros and message recall
 * and things like that are not available until after they are done.
 *
 * We load the default "user pref files" here in case any "color"
 * changes are needed before character creation.
 *
 * Note that the "graf-xxx.prf" file must be loaded separately,
 * if needed, in the first (?) pass through "TERM_XTRA_REACT".
 */
void init_angband(void)
{
	int fd = -1;

	int mode = FILE_MODE;

	FILE *fp;

	char *news_file;

	char buf[1024];

	/* Init some VERY basic stuff, like macro arrays */
	init_basic();

	/* Select & init a module if needed */
	select_module();

	/*** Choose which news.txt file to use ***/

	/* Choose the news file */
	switch (time(NULL) % 2)
	{
	default:
		{
			news_file = "news.txt";
			break;
		}

	case 0:
		{
			news_file = "news2.txt";
			break;
		}
	}

	/*** Verify the "news" file ***/

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, news_file);

	/* Attempt to open the file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		char why[1024];

		/* Message */
		sprintf(why, "Cannot access the '%s' file!", buf);

		/* Crash and burn */
		init_angband_aux(why);
	}

	/* Close it */
	(void)fd_close(fd);


	/*** Display the "news" file ***/

	/* Clear screen */
	Term_clear();

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_FILE, news_file);

	/* Open the News file */
	fp = my_fopen(buf, "r");

	/* Dump */
	if (fp)
	{
		int i = 0;

		/* Dump the file to the screen */
		while (0 == my_fgets(fp, buf, 1024))
		{
			/* Display and advance - we use display_message to parse colour codes XXX */
			display_message(0, i++, strlen(buf), TERM_WHITE, buf);
		}

		/* Close */
		my_fclose(fp);
	}

	/* Flush it */
	Term_fresh();


	/*** Verify (or create) the "high score" file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, "scores.raw");

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	/* Failure */
	if (fd < 0)
	{
		/* File type is "DATA" */
		FILE_TYPE(FILE_TYPE_DATA);

		/* Create a new high score file */
		fd = fd_make(buf, mode);

		/* Failure */
		if (fd < 0)
		{
			char why[1024];

			/* Message */
			sprintf(why, "Cannot create the '%s' file!", buf);

			/* Crash and burn */
			init_angband_aux(why);
		}
	}

	/* Close it */
	(void)fd_close(fd);


	/*** Initialise some arrays ***/

	/* Initialise misc. values */
	note("[Initialising values... (misc)]");
	if (init_misc()) quit("Cannot initialise misc. values");

	wipe_hooks();

	/* Initialise some other arrays */
	note("[Initialising scripting... (script)]");
	init_lua_init();

	/* Initialise skills info */
	note("[Initialising arrays... (skills)]");
	if (init_s_info()) quit("Cannot initialise skills");

	/* Initialise abilities info */
	note("[Initialising arrays... (abilities)]");
	if (init_ab_info()) quit("Cannot initialise abilities");

	/* Initialise alchemy info */
	note("[Initialising arrays... (alchemy)]");
	if (init_al_info()) quit("Cannot initialise alchemy");

	/* Initialise player info */
	note("[Initialising arrays... (players)]");
	if (init_player_info()) quit("Cannot initialise players");

	/* Initialise feature info */
	note("[Initialising arrays... (features)]");
	if (init_f_info()) quit("Cannot initialise features");

	/* Initialise object info */
	note("[Initialising arrays... (objects)]");
	if (init_k_info()) quit("Cannot initialise objects");

	/* Initialise artifact info */
	note("[Initialising arrays... (artifacts)]");
	if (init_a_info()) quit("Cannot initialise artifacts");

	/* Initialise set info */
	note("[Initialising item sets... (sets)]");
	if (init_set_info()) quit("Cannot initialise item sets");
	init_sets_aux();

	/* Initialise ego-item info */
	note("[Initialising arrays... (ego-items)]");
	if (init_e_info()) quit("Cannot initialise ego-items");

	/* Initialise randart parts info */
	note("[Initialising arrays... (randarts)]");
	if (init_ra_info()) quit("Cannot initialise randarts");

	/* Initialise monster info */
	note("[Initialising arrays... (monsters)]");
	if (init_r_info()) quit("Cannot initialise monsters");

	/* Initialise ego monster info */
	note("[Initialising arrays... (ego monsters)]");
	if (init_re_info()) quit("Cannot initialise ego monsters");

	/* Initialise dungeon type info */
	note("[Initialising arrays... (dungeon types)]");
	if (init_d_info()) quit("Cannot initialise dungeon types");
	init_guardians();

	/* Initialise actions type info */
	note("[Initialising arrays... (action types)]");
	if (init_ba_info()) quit("Cannot initialise action types");

	/* Initialise owners type info */
	note("[Initialising arrays... (owners types)]");
	if (init_ow_info()) quit("Cannot initialise owners types");

	/* Initialise stores type info */
	note("[Initialising arrays... (stores types)]");
	if (init_st_info()) quit("Cannot initialise stores types");

	/* Initialise wilderness features array */
	note("[Initialising arrays... (wilderness features)]");
	if (init_wf_info()) quit("Cannot initialise wilderness features");

	/* Initialise wilderness map array */
	note("[Initialising arrays... (wilderness map)]");
	if (init_wilderness()) quit("Cannot initialise wilderness map");

	/* Initialise town array */
	note("[Initialising arrays... (towns)]");
	if (init_towns()) quit("Cannot initialise towns");

	/* Initialise trap info */
	note("[Initialising arrays... (traps)]");
	if (init_t_info()) quit("Cannot initialise traps");

	/* Initialise some other arrays */
	note("[Initialising arrays... (other)]");
	if (init_other()) quit("Cannot initialise other stuff");

	/* Initialise some other arrays */
	note("[Initialising arrays... (alloc)]");
	if (init_alloc()) quit("Cannot initialise alloc stuff");

	/* Init random artifact names */
	build_prob(artifact_names_list);

	/*** Load default user pref files ***/

	/* Initialise feature info */
	note("[Initialising user pref files...]");

	/* Access the "basic" pref file */
	strcpy(buf, "pref.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "basic" system pref file */
	sprintf(buf, "pref-%s.prf", ANGBAND_SYS);

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" pref file */
	sprintf(buf, "user.prf");

	/* Process that file */
	process_pref_file(buf);

	/* Access the "user" system pref file */
	sprintf(buf, "user-%s.prf", ANGBAND_SYS);

	/* Process that file */
	process_pref_file(buf);

	/* Initialise the automatizer */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "automat.atm");
	automatizer_init(buf);

	/* Done */
	note("[Initialisation complete]");

	process_hooks(HOOK_INIT_GAME, "(s)", "end");
}
