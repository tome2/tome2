/* File: init2.c */

/* Purpose: Initialisation (part 2) -BEN- */

#include "angband.h"


#if !defined(MACINTOSH) && !defined(RISCOS) && defined(CHECK_MODIFICATION_TIME)
#include <sys/types.h>
#include <sys/stat.h>
#endif /* !MACINTOSH && !RISCOS && CHECK_MODIFICATION_TIME */


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
 * The "init1.c" file is used only to parse the ascii template files,
 * to create the binary image files.  If you include the binary image
 * files instead of the ascii template files, then you can undefine
 * "ALLOW_TEMPLATES", saving about 20K by removing "init1.c".  Note
 * that the binary image files are extremely system dependant.
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
	string_free(ANGBAND_DIR_APEX);
	string_free(ANGBAND_DIR_BONE);
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



#ifdef VM

	/*** Use "flat" paths with VM/ESA ***/

	/* Use "blank" path names */
	ANGBAND_DIR_APEX = string_make("");
	ANGBAND_DIR_BONE = string_make("");
	ANGBAND_DIR_CORE = string_make("");
	ANGBAND_DIR_DNGN = string_make("");
	ANGBAND_DIR_DATA = string_make("");
	ANGBAND_DIR_EDIT = string_make("");
	ANGBAND_DIR_FILE = string_make("");
	ANGBAND_DIR_HELP = string_make("");
	ANGBAND_DIR_INFO = string_make("");
	ANGBAND_DIR_MODULES = string_make("");
	ANGBAND_DIR_NOTE = string_make("");
	ANGBAND_DIR_PATCH = string_make("");
	ANGBAND_DIR_SAVE = string_make("");
	ANGBAND_DIR_SCPT = string_make("");
	ANGBAND_DIR_PREF = string_make("");
	ANGBAND_DIR_USER = string_make("");
	ANGBAND_DIR_XTRA = string_make("");
	ANGBAND_DIR_CMOV = string_make("");

#else /* VM */


	/*** Build the sub-directory names ***/

	/* Build a path name */
	strcpy(tail, "apex");
	ANGBAND_DIR_APEX = string_make(path);

	/* Build a path name */
	strcpy(tail, "bone");
	ANGBAND_DIR_BONE = string_make(path);

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

#ifdef PRIVATE_USER_PATH

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
#ifdef PRIVATE_USER_PATH_APEX
		ANGBAND_DIR_APEX = string_make(user_path);
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
		savefile_setuid = 0;
	}

#else /* PRIVATE_USER_PATH */

	/* Build a path name */
	strcpy(tail, "save");
	ANGBAND_DIR_SAVE = string_make(path);

	/* Build a path name */
	strcpy(tail, "user");
	ANGBAND_DIR_USER = string_make(path);

	/* Build a path name */
	strcpy(tail, "note");
	ANGBAND_DIR_NOTE = string_make(path);

	/* Build a .. blah blah -- Improv */
	strcpy(tail, "cmov");
	ANGBAND_DIR_CMOV = string_make(path);

#endif /* PRIVATE_USER_PATH */

	/* Build a path name */
	strcpy(tail, "xtra");
	ANGBAND_DIR_XTRA = string_make(path);

#endif /* VM */


#ifdef NeXT

	/* Allow "fat binary" usage with NeXT */
	if (TRUE)
	{
		cptr next = NULL;

# if defined(m68k)
		next = "m68k";
# endif

# if defined(i386)
		next = "i386";
# endif

# if defined(sparc)
		next = "sparc";
# endif

# if defined(hppa)
		next = "hppa";
# endif

		/* Use special directory */
		if (next)
		{
			/* Forget the old path name */
			string_free(ANGBAND_DIR_DATA);

			/* Build a new path name */
			sprintf(tail, "data-%s", next);
			ANGBAND_DIR_DATA = string_make(path);
		}
	}

#endif /* NeXT */

}



#ifdef ALLOW_TEMPLATES


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


#endif /* ALLOW_TEMPLATES */


#if !defined(RISCOS) && defined(CHECK_MODIFICATION_TIME)

static errr check_modification_date(int fd, cptr template_file)
{
	char buf[1024];

	struct stat txt_stat, raw_stat;

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, template_file);

	/* Access stats on text file */
	if (stat(buf, &txt_stat))
	{
		/* Error */
		return ( -1);
	}

	/* Access stats on raw file */
	if (fstat(fd, &raw_stat))
	{
		/* Error */
		return ( -1);
	}

	/* Ensure text file is not newer than raw file */
	if (txt_stat.st_mtime > raw_stat.st_mtime)
	{
		/* Reprocess text file */
		return ( -1);
	}

	return (0);
}

#endif /* CHECK_MODIFICATION_TIME */

/*
 * Hack -- take notes on line 23
 */
static void note(cptr str)
{
	Term_erase(0, 23, 255);
	Term_putstr(20, 23, -1, TERM_WHITE, str);
	Term_fresh();
}



/*** Initialise from binary image files ***/


/*
 * Initialise the "f_info" array, by parsing a binary "image" file
 */
static errr init_f_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != f_head->v_major) ||
	                (test.v_minor != f_head->v_minor) ||
	                (test.v_patch != f_head->v_patch) ||
	                (test.v_extra != f_head->v_extra) ||
	                (test.info_num != f_head->info_num) ||
	                (test.info_len != f_head->info_len) ||
	                (test.head_size != f_head->head_size) ||
	                (test.info_size != f_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*f_head) = test;


	/* Allocate the "f_info" array */
	C_MAKE(f_info, f_head->info_num, feature_type);

	/* Read the "f_info" array */
	fd_read(fd, (char*)(f_info), f_head->info_size);


	/* Allocate the "f_name" array */
	C_MAKE(f_name, f_head->name_size, char);

	/* Read the "f_name" array */
	fd_read(fd, (char*)(f_name), f_head->name_size);


#ifndef DELAY_LOAD_F_TEXT

	/* Allocate the "f_text" array */
	C_MAKE(f_text, f_head->text_size, char);

	/* Read the "f_text" array */
	fd_read(fd, (char*)(f_text), f_head->text_size);

#endif /* DELAY_LOAD_F_TEXT */


	/* Success */
	return (0);
}



/*
 * Initialise the "f_info" array
 *
 * Note that we let each entry have a unique "name" and "text" string,
 * even if the string happens to be empty (everyone has a unique '\0').
 */
static errr init_f_info(void)
{
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(f_head, header);

	/* Save the "version" */
	f_head->v_major = VERSION_MAJOR;
	f_head->v_minor = VERSION_MINOR;
	f_head->v_patch = VERSION_PATCH;
	f_head->v_extra = 0;

	/* Save the "record" information */
	f_head->info_num = max_f_idx;
	f_head->info_len = sizeof(feature_type);

	/* Save the size of "f_head" and "f_info" */
	f_head->head_size = sizeof(header);
	f_head->info_size = f_head->info_num * f_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "f_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_f_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'f_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(f_head), f_head->head_size);

		/* Dump the "f_info" array */
		fd_write(fd, (char*)(f_info), f_head->info_size);

		/* Dump the "f_name" array */
		fd_write(fd, (char*)(f_name), f_head->name_size);

		/* Dump the "f_text" array */
		fd_write(fd, (char*)(f_text), f_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "f_info" array */
	C_KILL(f_info, f_head->info_num, feature_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(f_name, fake_name_size, char);
	C_KILL(f_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "f_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'f_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_f_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'f_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialise the "k_info" array, by parsing a binary "image" file
 */
static errr init_k_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != k_head->v_major) ||
	                (test.v_minor != k_head->v_minor) ||
	                (test.v_patch != k_head->v_patch) ||
	                (test.v_extra != k_head->v_extra) ||
	                (test.info_num != k_head->info_num) ||
	                (test.info_len != k_head->info_len) ||
	                (test.head_size != k_head->head_size) ||
	                (test.info_size != k_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*k_head) = test;


	/* Allocate the "k_info" array */
	C_MAKE(k_info, k_head->info_num, object_kind);

	/* Read the "k_info" array */
	fd_read(fd, (char*)(k_info), k_head->info_size);


	/* Allocate the "k_name" array */
	C_MAKE(k_name, k_head->name_size, char);

	/* Read the "k_name" array */
	fd_read(fd, (char*)(k_name), k_head->name_size);


#ifndef DELAY_LOAD_K_TEXT

	/* Allocate the "k_text" array */
	C_MAKE(k_text, k_head->text_size, char);

	/* Read the "k_text" array */
	fd_read(fd, (char*)(k_text), k_head->text_size);

#endif /* DELAY_LOAD_K_TEXT */


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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(k_head, header);

	/* Save the "version" */
	k_head->v_major = VERSION_MAJOR;
	k_head->v_minor = VERSION_MINOR;
	k_head->v_patch = VERSION_PATCH;
	k_head->v_extra = 0;

	/* Save the "record" information */
	k_head->info_num = max_k_idx;
	k_head->info_len = sizeof(object_kind);

	/* Save the size of "k_head" and "k_info" */
	k_head->head_size = sizeof(header);
	k_head->info_size = k_head->info_num * k_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "k_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_k_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'k_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(k_head), k_head->head_size);

		/* Dump the "k_info" array */
		fd_write(fd, (char*)(k_info), k_head->info_size);

		/* Dump the "k_name" array */
		fd_write(fd, (char*)(k_name), k_head->name_size);

		/* Dump the "k_text" array */
		fd_write(fd, (char*)(k_text), k_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "k_info" array */
	C_KILL(k_info, k_head->info_num, object_kind);

	/* Hack -- Free the "fake" arrays */
	C_KILL(k_name, fake_name_size, char);
	C_KILL(k_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "k_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'k_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_k_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'k_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialise the "a_info" array, by parsing a binary "image" file
 */
static errr init_a_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != a_head->v_major) ||
	                (test.v_minor != a_head->v_minor) ||
	                (test.v_patch != a_head->v_patch) ||
	                (test.v_extra != a_head->v_extra) ||
	                (test.info_num != a_head->info_num) ||
	                (test.info_len != a_head->info_len) ||
	                (test.head_size != a_head->head_size) ||
	                (test.info_size != a_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*a_head) = test;


	/* Allocate the "a_info" array */
	C_MAKE(a_info, a_head->info_num, artifact_type);

	/* Read the "a_info" array */
	fd_read(fd, (char*)(a_info), a_head->info_size);


	/* Allocate the "a_name" array */
	C_MAKE(a_name, a_head->name_size, char);

	/* Read the "a_name" array */
	fd_read(fd, (char*)(a_name), a_head->name_size);


#ifndef DELAY_LOAD_A_TEXT

	/* Allocate the "a_text" array */
	C_MAKE(a_text, a_head->text_size, char);

	/* Read the "a_text" array */
	fd_read(fd, (char*)(a_text), a_head->text_size);

#endif /* DELAY_LOAD_A_TEXT */


	/* Success */
	return (0);
}

/*
 * Initialise the "s_info" array, by parsing a binary "image" file
 */
static errr init_s_info_raw(int fd)
{
	header test;
	/*int i;*/

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != s_head->v_major) ||
	                (test.v_minor != s_head->v_minor) ||
	                (test.v_patch != s_head->v_patch) ||
	                (test.v_extra != s_head->v_extra) ||
	                (test.info_num != s_head->info_num) ||
	                (test.info_len != s_head->info_len) ||
	                (test.head_size != s_head->head_size) ||
	                (test.info_size != s_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*s_head) = test;


	/* Allocate the "s_info" array */
	C_MAKE(s_info, s_head->info_num, skill_type);

	/* Read the "s_info" array */
	fd_read(fd, (char*)(s_info), s_head->info_size);


	/* Allocate the "s_name" array */
	C_MAKE(s_name, s_head->name_size, char);

	/* Read the "s_name" array */
	fd_read(fd, (char*)(s_name), s_head->name_size);


	/* Allocate the "s_text" array */
	C_MAKE(s_text, s_head->text_size, char);

	/* Read the "s_text" array */
	fd_read(fd, (char*)(s_text), s_head->text_size);


	/* Success */
	return (0);
}

/*
 * Initialise the "ab_info" array, by parsing a binary "image" file
 */
static errr init_ab_info_raw(int fd)
{
	header test;
	/*int i;*/

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != ab_head->v_major) ||
	                (test.v_minor != ab_head->v_minor) ||
	                (test.v_patch != ab_head->v_patch) ||
	                (test.v_extra != ab_head->v_extra) ||
	                (test.info_num != ab_head->info_num) ||
	                (test.info_len != ab_head->info_len) ||
	                (test.head_size != ab_head->head_size) ||
	                (test.info_size != ab_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*ab_head) = test;


	/* Allocate the "ab_info" array */
	C_MAKE(ab_info, ab_head->info_num, ability_type);

	/* Read the "ab_info" array */
	fd_read(fd, (char*)(ab_info), ab_head->info_size);


	/* Allocate the "ab_name" array */
	C_MAKE(ab_name, ab_head->name_size, char);

	/* Read the "ab_name" array */
	fd_read(fd, (char*)(ab_name), ab_head->name_size);


	/* Allocate the "ab_text" array */
	C_MAKE(ab_text, ab_head->text_size, char);

	/* Read the "ab_text" array */
	fd_read(fd, (char*)(ab_text), ab_head->text_size);


	/* Success */
	return (0);
}

/*
 * Initialise the "set_info" array, by parsing a binary "image" file
 */
static errr init_set_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != set_head->v_major) ||
	                (test.v_minor != set_head->v_minor) ||
	                (test.v_patch != set_head->v_patch) ||
	                (test.v_extra != set_head->v_extra) ||
	                (test.info_num != set_head->info_num) ||
	                (test.info_len != set_head->info_len) ||
	                (test.head_size != set_head->head_size) ||
	                (test.info_size != set_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*set_head) = test;


	/* Allocate the "a_info" array */
	C_MAKE(set_info, set_head->info_num, set_type);

	/* Read the "a_info" array */
	fd_read(fd, (char*)(set_info), set_head->info_size);


	/* Allocate the "a_name" array */
	C_MAKE(set_name, set_head->name_size, char);

	/* Read the "a_name" array */
	fd_read(fd, (char*)(set_name), set_head->name_size);


	/* Allocate the "a_text" array */
	C_MAKE(set_text, set_head->text_size, char);

	/* Read the "a_text" array */
	fd_read(fd, (char*)(set_text), set_head->text_size);

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(set_head, header);

	/* Save the "version" */
	set_head->v_major = VERSION_MAJOR;
	set_head->v_minor = VERSION_MINOR;
	set_head->v_patch = VERSION_PATCH;
	set_head->v_extra = 0;

	/* Save the "record" information */
	set_head->info_num = max_set_idx;
	set_head->info_len = sizeof(set_type);

	/* Save the size of "set_head" and "set_info" */
	set_head->head_size = sizeof(header);
	set_head->info_size = set_head->info_num * set_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "set_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "set_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_set_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'set_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "set_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(set_head), set_head->head_size);

		/* Dump the "set_info" array */
		fd_write(fd, (char*)(set_info), set_head->info_size);

		/* Dump the "set_name" array */
		fd_write(fd, (char*)(set_name), set_head->name_size);

		/* Dump the "set_text" array */
		fd_write(fd, (char*)(set_text), set_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "set_info" array */
	C_KILL(set_info, set_head->info_num, set_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(set_name, fake_name_size, char);
	C_KILL(set_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "set_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 'set_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_set_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'set_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(a_head, header);

	/* Save the "version" */
	a_head->v_major = VERSION_MAJOR;
	a_head->v_minor = VERSION_MINOR;
	a_head->v_patch = VERSION_PATCH;
	a_head->v_extra = 0;

	/* Save the "record" information */
	a_head->info_num = max_a_idx;
	a_head->info_len = sizeof(artifact_type);

	/* Save the size of "a_head" and "a_info" */
	a_head->head_size = sizeof(header);
	a_head->info_size = a_head->info_num * a_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "a_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_a_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'a_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(a_head), a_head->head_size);

		/* Dump the "a_info" array */
		fd_write(fd, (char*)(a_info), a_head->info_size);

		/* Dump the "a_name" array */
		fd_write(fd, (char*)(a_name), a_head->name_size);

		/* Dump the "a_text" array */
		fd_write(fd, (char*)(a_text), a_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "a_info" array */
	C_KILL(a_info, a_head->info_num, artifact_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(a_name, fake_name_size, char);
	C_KILL(a_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "a_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 'a_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_a_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'a_info.raw' file.");

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
	int fd;

	/* int i; */

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(s_head, header);

	/* Save the "version" */
	s_head->v_major = VERSION_MAJOR;
	s_head->v_minor = VERSION_MINOR;
	s_head->v_patch = VERSION_PATCH;
	s_head->v_extra = 0;

	/* Save the "record" information */
	s_head->info_num = max_s_idx;
	s_head->info_len = sizeof(skill_type);

	/* Save the size of "s_head" and "s_info" */
	s_head->head_size = sizeof(header);
	s_head->info_size = s_head->info_num * s_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "s_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "s_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_s_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 's_info.raw' file.");
		msg_print(NULL);
	}

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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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

	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "s_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(s_head), s_head->head_size);

		/* Dump the "s_info" array */
		fd_write(fd, (char*)(s_info), s_head->info_size);

		/* Dump the "s_name" array */
		fd_write(fd, (char*)(s_name), s_head->name_size);

		/* Dump the "s_text" array */
		fd_write(fd, (char*)(s_text), s_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "s_info" array */
	C_KILL(s_info, s_head->info_num, skill_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(s_name, fake_name_size, char);
	C_KILL(s_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "s_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 's_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_s_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 's_info.raw' file.");

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
	int fd;

	/* int i; */

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(ab_head, header);

	/* Save the "version" */
	ab_head->v_major = VERSION_MAJOR;
	ab_head->v_minor = VERSION_MINOR;
	ab_head->v_patch = VERSION_PATCH;
	ab_head->v_extra = 0;

	/* Save the "record" information */
	ab_head->info_num = max_ab_idx;
	ab_head->info_len = sizeof(ability_type);

	/* Save the size of "ab_head" and "ab_info" */
	ab_head->head_size = sizeof(header);
	ab_head->info_size = ab_head->info_num * ab_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ab_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "ab_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_ab_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'ab_info.raw' file.");
		msg_print(NULL);
	}

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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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

	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ab_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(ab_head), ab_head->head_size);

		/* Dump the "ab_info" array */
		fd_write(fd, (char*)(ab_info), ab_head->info_size);

		/* Dump the "ab_name" array */
		fd_write(fd, (char*)(ab_name), ab_head->name_size);

		/* Dump the "ab_text" array */
		fd_write(fd, (char*)(ab_text), ab_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "ab_info" array */
	C_KILL(ab_info, ab_head->info_num, ability_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(ab_name, fake_name_size, char);
	C_KILL(ab_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ab_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot open 'ab_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_ab_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'ab_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialise the "e_info" array, by parsing a binary "image" file
 */
static errr init_e_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != e_head->v_major) ||
	                (test.v_minor != e_head->v_minor) ||
	                (test.v_patch != e_head->v_patch) ||
	                (test.v_extra != e_head->v_extra) ||
	                (test.info_num != e_head->info_num) ||
	                (test.info_len != e_head->info_len) ||
	                (test.head_size != e_head->head_size) ||
	                (test.info_size != e_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*e_head) = test;


	/* Allocate the "e_info" array */
	C_MAKE(e_info, e_head->info_num, ego_item_type);

	/* Read the "e_info" array */
	fd_read(fd, (char*)(e_info), e_head->info_size);


	/* Allocate the "e_name" array */
	C_MAKE(e_name, e_head->name_size, char);

	/* Read the "e_name" array */
	fd_read(fd, (char*)(e_name), e_head->name_size);


#ifndef DELAY_LOAD_E_TEXT

	/* Allocate the "e_text" array */
	C_MAKE(e_text, e_head->text_size, char);

	/* Read the "e_text" array */
	fd_read(fd, (char*)(e_text), e_head->text_size);

#endif /* DELAY_LOAD_E_TEXT */


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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(e_head, header);

	/* Save the "version" */
	e_head->v_major = VERSION_MAJOR;
	e_head->v_minor = VERSION_MINOR;
	e_head->v_patch = VERSION_PATCH;
	e_head->v_extra = 0;

	/* Save the "record" information */
	e_head->info_num = max_e_idx;
	e_head->info_len = sizeof(ego_item_type);

	/* Save the size of "e_head" and "e_info" */
	e_head->head_size = sizeof(header);
	e_head->info_size = e_head->info_num * e_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{

#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "e_info.txt");

#endif /* CHECK_MODIFICATION_TIME */


		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_e_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'e_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(e_head), e_head->head_size);

		/* Dump the "e_info" array */
		fd_write(fd, (char*)(e_info), e_head->info_size);

		/* Dump the "e_name" array */
		fd_write(fd, (char*)(e_name), e_head->name_size);

		/* Dump the "e_text" array */
		fd_write(fd, (char*)(e_text), e_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "e_info" array */
	C_KILL(e_info, e_head->info_num, ego_item_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(e_name, fake_name_size, char);
	C_KILL(e_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "e_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'e_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_e_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'e_info.raw' file.");

	/* Success */
	return (0);
}


/*
 * Initialise the "ra_info" array, by parsing a binary "image" file
 */
static errr init_ra_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != ra_head->v_major) ||
	                (test.v_minor != ra_head->v_minor) ||
	                (test.v_patch != ra_head->v_patch) ||
	                (test.v_extra != ra_head->v_extra) ||
	                (test.info_num != ra_head->info_num) ||
	                (test.info_len != ra_head->info_len) ||
	                (test.head_size != ra_head->head_size) ||
	                (test.info_size != ra_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*ra_head) = test;


	/* Allocate the "ra_info" array */
	C_MAKE(ra_info, ra_head->info_num, randart_part_type);

	/* Read the "ra_info" array */
	fd_read(fd, (char*)(ra_info), ra_head->info_size);
	fd_read(fd, (char*)(ra_gen), 30 * sizeof (randart_gen_type));

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the "header" ***/

	/* Allocate the "header" */
	MAKE(ra_head, header);

	/* Save the "version" */
	ra_head->v_major = VERSION_MAJOR;
	ra_head->v_minor = VERSION_MINOR;
	ra_head->v_patch = VERSION_PATCH;
	ra_head->v_extra = 0;

	/* Save the "record" information */
	ra_head->info_num = max_ra_idx;
	ra_head->info_len = sizeof(randart_part_type);

	/* Save the size of "ra_head" and "ra_info" */
	ra_head->head_size = sizeof(header);
	ra_head->info_size = ra_head->info_num * ra_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ra_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{

#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "ra_info.txt");

#endif /* CHECK_MODIFICATION_TIME */


		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_ra_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'ra_info.raw' file.");
		msg_print(NULL);
	}


	/*** Make the fake arrays ***/

	/* Fake the size of "ra_name" and "ra_text" */
	fake_name_size = FAKE_NAME_SIZE;
	fake_text_size = FAKE_TEXT_SIZE;

	/* Allocate the "ra_info" array */
	C_MAKE(ra_info, ra_head->info_num, randart_part_type);

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "ra_info.txt");

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ra_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(ra_head), ra_head->head_size);

		/* Dump the "ra_info" array */
		fd_write(fd, (char*)(ra_info), ra_head->info_size);
		fd_write(fd, (char*)(ra_gen), 30 * sizeof (randart_gen_type));

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "ra_info" array */
	C_KILL(ra_info, ra_head->info_num, randart_part_type);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ra_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'ra_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_ra_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'ra_info.raw' file.");

	/* Success */
	return (0);
}



/*
 * Initialise the "r_info" array, by parsing a binary "image" file
 */
static errr init_r_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != r_head->v_major) ||
	                (test.v_minor != r_head->v_minor) ||
	                (test.v_patch != r_head->v_patch) ||
	                (test.v_extra != r_head->v_extra) ||
	                (test.info_num != r_head->info_num) ||
	                (test.info_len != r_head->info_len) ||
	                (test.head_size != r_head->head_size) ||
	                (test.info_size != r_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*r_head) = test;


	/* Allocate the "r_info" array */
	C_MAKE(r_info, r_head->info_num, monster_race);

	/* Read the "r_info" array */
	fd_read(fd, (char*)(r_info), r_head->info_size);


	/* Allocate the "r_name" array */
	C_MAKE(r_name, r_head->name_size, char);

	/* Read the "r_name" array */
	fd_read(fd, (char*)(r_name), r_head->name_size);


#ifndef DELAY_LOAD_R_TEXT

	/* Allocate the "r_text" array */
	C_MAKE(r_text, r_head->text_size, char);

	/* Read the "r_text" array */
	fd_read(fd, (char*)(r_text), r_head->text_size);

#endif /* DELAY_LOAD_R_TEXT */


	/* Success */
	return (0);
}

/*
 * Initialise the "re_info" array, by parsing a binary "image" file
 */
static errr init_re_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != re_head->v_major) ||
	                (test.v_minor != re_head->v_minor) ||
	                (test.v_patch != re_head->v_patch) ||
	                (test.v_extra != re_head->v_extra) ||
	                (test.info_num != re_head->info_num) ||
	                (test.info_len != re_head->info_len) ||
	                (test.head_size != re_head->head_size) ||
	                (test.info_size != re_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*re_head) = test;


	/* Allocate the "re_info" array */
	C_MAKE(re_info, re_head->info_num, monster_ego);

	/* Read the "re_info" array */
	fd_read(fd, (char*)(re_info), re_head->info_size);


	/* Allocate the "re_name" array */
	C_MAKE(re_name, re_head->name_size, char);

	/* Read the "re_name" array */
	fd_read(fd, (char*)(re_name), re_head->name_size);

	/* Success */
	return (0);
}


/*
 * Initialise the "d_info" array, by parsing a binary "image" file
 */
static errr init_d_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != d_head->v_major) ||
	                (test.v_minor != d_head->v_minor) ||
	                (test.v_patch != d_head->v_patch) ||
	                (test.v_extra != d_head->v_extra) ||
	                (test.info_num != d_head->info_num) ||
	                (test.info_len != d_head->info_len) ||
	                (test.head_size != d_head->head_size) ||
	                (test.info_size != d_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*d_head) = test;


	/* Allocate the "d_info" array */
	C_MAKE(d_info, d_head->info_num, dungeon_info_type);

	/* Read the "d_info" array */
	fd_read(fd, (char*)(d_info), d_head->info_size);


	/* Allocate the "r_name" array */
	C_MAKE(d_name, d_head->name_size, char);

	/* Read the "d_name" array */
	fd_read(fd, (char*)(d_name), d_head->name_size);

	/* Allocate the "d_text" array */
	C_MAKE(d_text, d_head->text_size, char);

	/* Read the "d_text" array */
	fd_read(fd, (char*)(d_text), d_head->text_size);


	/* Success */
	return (0);
}

/*
 * Initialise the "st_info" array, by parsing a binary "image" file
 */
static errr init_st_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != st_head->v_major) ||
	                (test.v_minor != st_head->v_minor) ||
	                (test.v_patch != st_head->v_patch) ||
	                (test.v_extra != st_head->v_extra) ||
	                (test.info_num != st_head->info_num) ||
	                (test.info_len != st_head->info_len) ||
	                (test.head_size != st_head->head_size) ||
	                (test.info_size != st_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*st_head) = test;


	/* Allocate the "st_info" array */
	C_MAKE(st_info, st_head->info_num, store_info_type);

	/* Read the "st_info" array */
	fd_read(fd, (char*)(st_info), st_head->info_size);


	/* Allocate the "st_name" array */
	C_MAKE(st_name, st_head->name_size, char);

	/* Read the "st_name" array */
	fd_read(fd, (char*)(st_name), st_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "ba_info" array, by parsing a binary "image" file
 */
static errr init_ba_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != ba_head->v_major) ||
	                (test.v_minor != ba_head->v_minor) ||
	                (test.v_patch != ba_head->v_patch) ||
	                (test.v_extra != ba_head->v_extra) ||
	                (test.info_num != ba_head->info_num) ||
	                (test.info_len != ba_head->info_len) ||
	                (test.head_size != ba_head->head_size) ||
	                (test.info_size != ba_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*ba_head) = test;


	/* Allocate the "ba_info" array */
	C_MAKE(ba_info, ba_head->info_num, store_action_type);

	/* Read the "ba_info" array */
	fd_read(fd, (char*)(ba_info), ba_head->info_size);


	/* Allocate the "ba_name" array */
	C_MAKE(ba_name, ba_head->name_size, char);

	/* Read the "ba_name" array */
	fd_read(fd, (char*)(ba_name), ba_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "ow_info" array, by parsing a binary "image" file
 */
static errr init_ow_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != ow_head->v_major) ||
	                (test.v_minor != ow_head->v_minor) ||
	                (test.v_patch != ow_head->v_patch) ||
	                (test.v_extra != ow_head->v_extra) ||
	                (test.info_num != ow_head->info_num) ||
	                (test.info_len != ow_head->info_len) ||
	                (test.head_size != ow_head->head_size) ||
	                (test.info_size != ow_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*ow_head) = test;


	/* Allocate the "ow_info" array */
	C_MAKE(ow_info, ow_head->info_num, owner_type);

	/* Read the "ow_info" array */
	fd_read(fd, (char*)(ow_info), ow_head->info_size);


	/* Allocate the "ow_name" array */
	C_MAKE(ow_name, ow_head->name_size, char);

	/* Read the "ow_name" array */
	fd_read(fd, (char*)(ow_name), ow_head->name_size);

	/* Success */
	return (0);
}

/*
 * Initialise the "wf_info" array, by parsing a binary "image" file
 */
static errr init_wf_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != wf_head->v_major) ||
	                (test.v_minor != wf_head->v_minor) ||
	                (test.v_patch != wf_head->v_patch) ||
	                (test.v_extra != wf_head->v_extra) ||
	                (test.info_num != wf_head->info_num) ||
	                (test.info_len != wf_head->info_len) ||
	                (test.head_size != wf_head->head_size) ||
	                (test.info_size != wf_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*wf_head) = test;


	/* Allocate the "wf_info" array */
	C_MAKE(wf_info, wf_head->info_num, wilderness_type_info);

	/* Read the "wf_info" array */
	fd_read(fd, (char*)(wf_info), wf_head->info_size);


	/* Allocate the "wf_name" array */
	C_MAKE(wf_name, wf_head->name_size, char);

	/* Read the "wf_name" array */
	fd_read(fd, (char*)(wf_name), wf_head->name_size);

	/* Allocate the "wf_text" array */
	C_MAKE(wf_text, wf_head->text_size, char);

	/* Read the "wf_text" array */
	fd_read(fd, (char*)(wf_text), wf_head->text_size);

	/* Read the "wildc2i" array */
	fd_read(fd, (char*)wildc2i, 256 * sizeof(int));

	/* Success */
	return (0);
}

/*
 * Initialise the "player" arrays, by parsing a binary "image" file
 */
static errr init_player_info_raw(int fd)
{
	header test;
	int i;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != rp_head->v_major) ||
	                (test.v_minor != rp_head->v_minor) ||
	                (test.v_patch != rp_head->v_patch) ||
	                (test.v_extra != rp_head->v_extra) ||
	                (test.info_num != rp_head->info_num) ||
	                (test.info_len != rp_head->info_len) ||
	                (test.head_size != rp_head->head_size) ||
	                (test.info_size != rp_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*rp_head) = test;


	/* Allocate the "rp_info" array */
	C_MAKE(race_info, rp_head->info_num, player_race);

	/* Read the "rp_info" array */
	fd_read(fd, (char*)(race_info), rp_head->info_size);


	/* Allocate the "rp_name" array */
	C_MAKE(rp_name, rp_head->name_size, char);

	/* Read the "rp_name" array */
	fd_read(fd, (char*)(rp_name), rp_head->name_size);

	/* Allocate the "rp_text" array */
	C_MAKE(rp_text, rp_head->text_size, char);

	/* Read the "rp_text" array */
	fd_read(fd, (char*)(rp_text), rp_head->text_size);


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != rmp_head->v_major) ||
	                (test.v_minor != rmp_head->v_minor) ||
	                (test.v_patch != rmp_head->v_patch) ||
	                (test.v_extra != rmp_head->v_extra) ||
	                (test.info_num != rmp_head->info_num) ||
	                (test.info_len != rmp_head->info_len) ||
	                (test.head_size != rmp_head->head_size) ||
	                (test.info_size != rmp_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*rmp_head) = test;


	/* Allocate the "rmp_info" array */
	C_MAKE(race_mod_info, rmp_head->info_num, player_race_mod);

	/* Read the "rmp_info" array */
	fd_read(fd, (char*)(race_mod_info), rmp_head->info_size);


	/* Allocate the "rmp_name" array */
	C_MAKE(rmp_name, rmp_head->name_size, char);

	/* Read the "rmp_name" array */
	fd_read(fd, (char*)(rmp_name), rmp_head->name_size);

	/* Allocate the "rmp_text" array */
	C_MAKE(rmp_text, rmp_head->text_size, char);

	/* Read the "rmp_text" array */
	fd_read(fd, (char*)(rmp_text), rmp_head->text_size);


	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != c_head->v_major) ||
	                (test.v_minor != c_head->v_minor) ||
	                (test.v_patch != c_head->v_patch) ||
	                (test.v_extra != c_head->v_extra) ||
	                (test.info_num != c_head->info_num) ||
	                (test.info_len != c_head->info_len) ||
	                (test.head_size != c_head->head_size) ||
	                (test.info_size != c_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*c_head) = test;


	/* Allocate the "c_info" array */
	C_MAKE(class_info, c_head->info_num, player_class);

	/* Read the "c_info" array */
	fd_read(fd, (char*)(class_info), c_head->info_size);


	/* Allocate the "c_name" array */
	C_MAKE(c_name, c_head->name_size, char);

	/* Read the "c_name" array */
	fd_read(fd, (char*)(c_name), c_head->name_size);

	/* Allocate the "c_text" array */
	C_MAKE(c_text, c_head->text_size, char);

	/* Read the "c_text" array */
	fd_read(fd, (char*)(c_text), c_head->text_size);

	/* Allocate the "bg" array */
	C_MAKE(bg, max_bg_idx, hist_type);

	/* Read the "bg" array */
	fd_read(fd, (char*)bg, max_bg_idx * sizeof(hist_type));

	/* Allocate the "meta_class" array */
	C_MAKE(meta_class_info, max_mc_idx, meta_class_type);

	/* Read the "meta_class" array */
	fd_read(fd, (char*)meta_class_info, max_mc_idx * sizeof(meta_class_type));

	for (i = 0; i < max_mc_idx; i++)
	{
		C_MAKE(meta_class_info[i].classes, max_c_idx, s16b);
		fd_read(fd, (char*)meta_class_info[i].classes, max_c_idx * sizeof(s16b));
	}

	/* Read the "gen skills" array */
	fd_read(fd, (char*)(gen_skill_base), MAX_SKILLS * sizeof(u32b));
	fd_read(fd, (char*)(gen_skill_mod), MAX_SKILLS * sizeof(s16b));
	fd_read(fd, (char*)(gen_skill_basem), MAX_SKILLS * sizeof(char));
	fd_read(fd, (char*)(gen_skill_modm), MAX_SKILLS * sizeof(char));

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(r_head, header);

	/* Save the "version" */
	r_head->v_major = VERSION_MAJOR;
	r_head->v_minor = VERSION_MINOR;
	r_head->v_patch = VERSION_PATCH;
	r_head->v_extra = 0;

	/* Save the "record" information */
	r_head->info_num = max_r_idx;
	r_head->info_len = sizeof(monster_race);

	/* Save the size of "r_head" and "r_info" */
	r_head->head_size = sizeof(header);
	r_head->info_size = r_head->info_num * r_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "r_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_r_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'r_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(r_head), r_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(r_info), r_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(r_name), r_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(r_text), r_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "r_info" array */
	C_KILL(r_info, r_head->info_num, monster_race);

	/* Hack -- Free the "fake" arrays */
	C_KILL(r_name, fake_name_size, char);
	C_KILL(r_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'r_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_r_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'r_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(re_head, header);

	/* Save the "version" */
	re_head->v_major = VERSION_MAJOR;
	re_head->v_minor = VERSION_MINOR;
	re_head->v_patch = VERSION_PATCH;
	re_head->v_extra = 0;

	/* Save the "record" information */
	re_head->info_num = max_re_idx;
	re_head->info_len = sizeof(monster_ego);

	/* Save the size of "re_head" and "re_info" */
	re_head->head_size = sizeof(header);
	re_head->info_size = re_head->info_num * re_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "re_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "re_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_re_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 're_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "re_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(re_head), re_head->head_size);

		/* Dump the "re_info" array */
		fd_write(fd, (char*)(re_info), re_head->info_size);

		/* Dump the "re_name" array */
		fd_write(fd, (char*)(re_name), re_head->name_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "re_info" array */
	C_KILL(re_info, re_head->info_num, monster_ego);

	/* Hack -- Free the "fake" arrays */
	C_KILL(re_name, fake_name_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "re_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 're_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_re_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 're_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(d_head, header);

	/* Save the "version" */
	d_head->v_major = VERSION_MAJOR;
	d_head->v_minor = VERSION_MINOR;
	d_head->v_patch = VERSION_PATCH;
	d_head->v_extra = 0;

	/* Save the "record" information */
	d_head->info_num = max_d_idx;
	d_head->info_len = sizeof(dungeon_info_type);

	/* Save the size of "d_head" and "d_info" */
	d_head->head_size = sizeof(header);
	d_head->info_size = d_head->info_num * d_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "d_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "d_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_d_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'd_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "d_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(d_head), d_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(d_info), d_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(d_name), d_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(d_text), d_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "d_info" array */
	C_KILL(d_info, d_head->info_num, dungeon_info_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(d_name, fake_name_size, char);
	C_KILL(d_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "d_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'd_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_d_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'd_info.raw' file.");

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
	int fd;

	int i;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(rp_head, header);

	/* Save the "version" */
	rp_head->v_major = VERSION_MAJOR;
	rp_head->v_minor = VERSION_MINOR;
	rp_head->v_patch = VERSION_PATCH;
	rp_head->v_extra = 0;

	/* Save the "record" information */
	rp_head->info_num = max_rp_idx;
	rp_head->info_len = sizeof(player_race);

	/* Save the size of "rp_head" and "rp_info" */
	rp_head->head_size = sizeof(header);
	rp_head->info_size = rp_head->info_num * rp_head->info_len;

	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(rmp_head, header);

	/* Save the "version" */
	rmp_head->v_major = VERSION_MAJOR;
	rmp_head->v_minor = VERSION_MINOR;
	rmp_head->v_patch = VERSION_PATCH;
	rmp_head->v_extra = 0;

	/* Save the "record" information */
	rmp_head->info_num = max_rmp_idx;
	rmp_head->info_len = sizeof(player_race_mod);

	/* Save the size of "rmp_head" and "rmp_info" */
	rmp_head->head_size = sizeof(header);
	rmp_head->info_size = rmp_head->info_num * rmp_head->info_len;

	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(c_head, header);

	/* Save the "version" */
	c_head->v_major = VERSION_MAJOR;
	c_head->v_minor = VERSION_MINOR;
	c_head->v_patch = VERSION_PATCH;
	c_head->v_extra = 0;

	/* Save the "record" information */
	c_head->info_num = max_c_idx;
	c_head->info_len = sizeof(player_class);

	/* Save the size of "c_head" and "c_info" */
	c_head->head_size = sizeof(header);
	c_head->info_size = c_head->info_num * c_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "p_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_player_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'p_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Read the "meta_class" array */
	fd_read(fd, (char*)meta_class_info, max_mc_idx * sizeof(meta_class_type));

	for (i = 0; i < max_mc_idx; i++)
	{
		C_MAKE(meta_class_info[i].classes, max_c_idx, s16b);
		fd_read(fd, (char*)meta_class_info[i].classes, max_c_idx * sizeof(s16b));
	}

	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "p_info.txt");

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(rp_head), rp_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(race_info), rp_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(rp_name), rp_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(rp_text), rp_head->text_size);

		/* Dump it */
		fd_write(fd, (char*)(rmp_head), rmp_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(race_mod_info), rmp_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(rmp_name), rmp_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(rmp_text), rmp_head->text_size);

		/* Dump it */
		fd_write(fd, (char*)(c_head), c_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(class_info), c_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(c_name), c_head->name_size);

		/* Dump the "r_text" array */
		fd_write(fd, (char*)(c_text), c_head->text_size);

		/* Dump the "bg" array */
		fd_write(fd, (char*)bg, max_bg_idx * sizeof(hist_type));

		/* Dump the "meta_class" array */
		fd_write(fd, (char*)meta_class_info, max_mc_idx * sizeof(meta_class_type));

		for (i = 0; i < max_mc_idx; i++)
		{
			fd_write(fd, (char*)meta_class_info[i].classes, max_c_idx * sizeof(s16b));
		}

		/* Read the "gen skills" array */
		fd_write(fd, (char*)(gen_skill_base), MAX_SKILLS * sizeof(u32b));
		fd_write(fd, (char*)(gen_skill_mod), MAX_SKILLS * sizeof(s16b));
		fd_write(fd, (char*)(gen_skill_basem), MAX_SKILLS * sizeof(char));
		fd_write(fd, (char*)(gen_skill_modm), MAX_SKILLS * sizeof(char));

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "rp_info" array */
	C_KILL(race_info, rp_head->info_num, player_race);

	/* Hack -- Free the "fake" arrays */
	C_KILL(rp_name, fake_name_size, char);
	C_KILL(rp_text, fake_text_size, char);

	/* Free the "c_info" array */
	C_KILL(class_info, c_head->info_num, player_class);

	/* Hack -- Free the "fake" arrays */
	C_KILL(c_name, fake_name_size, char);
	C_KILL(c_text, fake_text_size, char);

	/* Free the "rp_info" array */
	C_KILL(race_mod_info, rmp_head->info_num, player_race_mod);

	/* Hack -- Free the "fake" arrays */
	C_KILL(rmp_name, fake_name_size, char);
	C_KILL(rmp_text, fake_text_size, char);

	/* Allocate the "rp_text" array */
	C_KILL(bg, max_bg_idx, hist_type);

	/* Free the "meta_class" array */
	for (i = 0; i < max_mc_idx; i++)
	{
		C_FREE(meta_class_info[i].classes, max_c_idx, s16b);
	}
	C_FREE(meta_class_info, max_mc_idx, meta_class_type);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "p_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'p_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_player_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'p_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(st_head, header);

	/* Save the "version" */
	st_head->v_major = VERSION_MAJOR;
	st_head->v_minor = VERSION_MINOR;
	st_head->v_patch = VERSION_PATCH;
	st_head->v_extra = 0;

	/* Save the "record" information */
	st_head->info_num = max_st_idx;
	st_head->info_len = sizeof(store_info_type);

	/* Save the size of "st_head" and "st_info" */
	st_head->head_size = sizeof(header);
	st_head->info_size = st_head->info_num * st_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "st_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "st_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_st_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'st_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "st_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(st_head), st_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(st_info), st_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(st_name), st_head->name_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "st_info" array */
	C_KILL(st_info, st_head->info_num, store_info_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(st_name, fake_name_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "st_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'st_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_st_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'st_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(ow_head, header);

	/* Save the "version" */
	ow_head->v_major = VERSION_MAJOR;
	ow_head->v_minor = VERSION_MINOR;
	ow_head->v_patch = VERSION_PATCH;
	ow_head->v_extra = 0;

	/* Save the "record" information */
	ow_head->info_num = max_ow_idx;
	ow_head->info_len = sizeof(owner_type);

	/* Save the size of "head" and "ow_info" */
	ow_head->head_size = sizeof(header);
	ow_head->info_size = ow_head->info_num * ow_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ow_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "ow_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_ow_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'ow_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ow_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(ow_head), ow_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(ow_info), ow_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(ow_name), ow_head->name_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "ow_info" array */
	C_KILL(ow_info, ow_head->info_num, owner_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(ow_name, fake_name_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ow_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'ow_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_ow_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'ow_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(ba_head, header);

	/* Save the "version" */
	ba_head->v_major = VERSION_MAJOR;
	ba_head->v_minor = VERSION_MINOR;
	ba_head->v_patch = VERSION_PATCH;
	ba_head->v_extra = 0;

	/* Save the "record" information */
	ba_head->info_num = max_ba_idx;
	ba_head->info_len = sizeof(store_action_type);

	/* Save the size of "head" and "ba_info" */
	ba_head->head_size = sizeof(header);
	ba_head->info_size = ba_head->info_num * ba_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ba_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "ba_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_ba_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'ba_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ba_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(ba_head), ba_head->head_size);

		/* Dump the "r_info" array */
		fd_write(fd, (char*)(ba_info), ba_head->info_size);

		/* Dump the "r_name" array */
		fd_write(fd, (char*)(ba_name), ba_head->name_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "ba_info" array */
	C_KILL(ba_info, ba_head->info_num, store_action_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(ba_name, fake_name_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "ba_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'ba_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_ba_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'ba_info.raw' file.");

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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(wf_head, header);

	/* Save the "version" */
	wf_head->v_major = VERSION_MAJOR;
	wf_head->v_minor = VERSION_MINOR;
	wf_head->v_patch = VERSION_PATCH;
	wf_head->v_extra = 0;

	/* Save the "record" information */
	wf_head->info_num = max_wf_idx;
	wf_head->info_len = sizeof(wilderness_type_info);

	/* Save the size of "wf_head" and "wf_info" */
	wf_head->head_size = sizeof(header);
	wf_head->info_size = wf_head->info_num * wf_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "wf_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "wf_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_wf_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'wf_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "wf_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(wf_head), wf_head->head_size);

		/* Dump the "wf_info" array */
		fd_write(fd, (char*)(wf_info), wf_head->info_size);

		/* Dump the "wf_name" array */
		fd_write(fd, (char*)(wf_name), wf_head->name_size);

		/* Dump the "wf_text" array */
		fd_write(fd, (char*)(wf_text), wf_head->text_size);

		/* Dump the "wildc2i" array */
		fd_write(fd, (char*)(wildc2i), 256 * sizeof(int));

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "wf_info" array */
	C_KILL(wf_info, wf_head->info_num, wilderness_type_info);

	/* Hack -- Free the "fake" arrays */
	C_KILL(wf_name, fake_name_size, char);
	C_KILL(wf_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif	/* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "wf_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'wf_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_wf_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'wf_info.raw' file.");

	/* Success */
	return (0);
}

/*
 * Initialise the "t_info" array, by parsing a binary "image" file
 */
static errr init_t_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != t_head->v_major) ||
	                (test.v_minor != t_head->v_minor) ||
	                (test.v_patch != t_head->v_patch) ||
	                (test.v_extra != t_head->v_extra) ||
	                (test.info_num != t_head->info_num) ||
	                (test.info_len != t_head->info_len) ||
	                (test.head_size != t_head->head_size) ||
	                (test.info_size != t_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*t_head) = test;


	/* Allocate the "f_info" array */
	C_MAKE(t_info, t_head->info_num, trap_type);

	/* Read the "t_info" array */
	fd_read(fd, (char*)(t_info), t_head->info_size);


	/* Allocate the "t_name" array */
	C_MAKE(t_name, t_head->name_size, char);

	/* Read the "t_name" array */
	fd_read(fd, (char*)(t_name), t_head->name_size);


#ifndef DELAY_LOAD_T_TEXT

	/* Allocate the "t_text" array */
	C_MAKE(t_text, t_head->text_size, char);

	/* Read the "t_text" array */
	fd_read(fd, (char*)(t_text), t_head->text_size);

#endif /* DELAY_LOAD_T_TEXT */


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
	int fd;

	int mode = FILE_MODE;

	errr err = 0;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(t_head, header);

	/* Save the "version" */
	t_head->v_major = VERSION_MAJOR;
	t_head->v_minor = VERSION_MINOR;
	t_head->v_patch = VERSION_PATCH;
	t_head->v_extra = 0;

	/* Save the "record" information */
	t_head->info_num = max_t_idx;
	t_head->info_len = sizeof(trap_type);

	/* Save the size of "t_head" and "t_info" */
	t_head->head_size = sizeof(header);
	t_head->info_size = t_head->info_num * t_head->info_len;


#ifdef ALLOW_TEMPLATES
	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "tr_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "tr_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_t_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'tr_info.raw' file.");
		msg_print(NULL);
	}

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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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

	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "tr_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(t_head), t_head->head_size);

		/* Dump the "f_info" array */
		fd_write(fd, (char*)(t_info), t_head->info_size);

		/* Dump the "f_name" array */
		fd_write(fd, (char*)(t_name), t_head->name_size);

		/* Dump the "f_text" array */
		fd_write(fd, (char*)(t_text), t_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "h_info" array */
	C_KILL(t_info, t_head->info_num, trap_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(t_name, fake_name_size, char);
	C_KILL(t_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;
#endif	/* ALLOW_TEMPLATES */

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "tr_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'tr_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_t_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'tr_info.raw' file.");

	/* Success */
	return (0);
}

/*
 * Initialise the "al_info" array, by parsing a binary "image" file
 */
static errr init_al_info_raw(int fd)
{
	header test;
	char *hack;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != al_head->v_major) ||
	                (test.v_minor != al_head->v_minor) ||
	                (test.v_patch != al_head->v_patch) ||
	                (test.v_extra != al_head->v_extra) ||
	                (test.info_num != al_head->info_num) ||
	                (test.info_len != al_head->info_len) ||
	                (test.head_size != al_head->head_size) ||
	                (test.info_size != al_head->info_size))
	{
		/* Error */
		return ( -1);
	}

	/* Accept the header */
	(*al_head) = test;

	/* Allocate the "al_info" array */
	C_MAKE(alchemist_recipes, al_head->info_num, alchemist_recipe);

	/* Read the "al_info" array */
	fd_read(fd, (char*)(alchemist_recipes), al_head->info_size);

	/* Allocate the "al_name" array */
	C_MAKE(al_name, al_head->name_size, char );

	/* Read the "al_info" array */
	fd_read(fd, (char*)(al_name), al_head->name_size);

	/* Allocate the "al_text" array */
	C_MAKE(hack, al_head->text_size, char );
	a_select_flags = (artifact_select_flag *) hack;

	/* Read the "al_info" array */
	fd_read(fd, (char*)(a_select_flags), al_head->text_size);

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
	int fd;

	int mode = FILE_MODE;

	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];

	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(al_head, header);

	/* Save the "version" */
	al_head->v_major = VERSION_MAJOR;
	al_head->v_minor = VERSION_MINOR;
	al_head->v_patch = VERSION_PATCH;
	al_head->v_extra = 0;

	/* Save the "record" information */
	al_head->info_num = max_al_idx;
	al_head->info_len = sizeof(alchemist_recipe);

	/* Save the size of "al_head" and "al_info" */
	al_head->head_size = sizeof(header);

	al_head->info_size = al_head->info_num * al_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "al_info.raw");

#if 0

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

#else

fd = -1;

#endif

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "al_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_al_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'v_info.raw' file.");
		msg_print(NULL);
	}

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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "al_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(al_head), al_head->head_size);

		/* Dump the "al_info" array */
		fd_write(fd, (char*)(alchemist_recipes), al_head->info_size);

		/* Dump the "al_name" array */
		fd_write(fd, (char*)(al_name), al_head->name_size);

		/* Dump the "al_info" array */
		fd_write(fd, (char*)(a_select_flags), al_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "al_info" array */
	C_KILL(alchemist_recipes, al_head->info_num, alchemist_recipe);

	/* Free the 'Fake' arrays */
	C_KILL(al_name, al_head->name_size, char);
	{
		char *hack = (char *) a_select_flags;
		C_KILL(hack, al_head->text_size, char);
	}

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif /* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "al_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'al_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_al_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'al_info.raw' file.");

	/* Success */
	return (0);
}

/*
 * Initialise the "v_info" array, by parsing a binary "image" file
 */
static errr init_v_info_raw(int fd)
{
	header test;

	/* Read and Verify the header */
	if (fd_read(fd, (char*)(&test), sizeof(header)) ||
	                (test.v_major != v_head->v_major) ||
	                (test.v_minor != v_head->v_minor) ||
	                (test.v_patch != v_head->v_patch) ||
	                (test.v_extra != v_head->v_extra) ||
	                (test.info_num != v_head->info_num) ||
	                (test.info_len != v_head->info_len) ||
	                (test.head_size != v_head->head_size) ||
	                (test.info_size != v_head->info_size))
	{
		/* Error */
		return ( -1);
	}


	/* Accept the header */
	(*v_head) = test;


	/* Allocate the "v_info" array */
	C_MAKE(v_info, v_head->info_num, vault_type);

	/* Read the "v_info" array */
	fd_read(fd, (char*)(v_info), v_head->info_size);


	/* Allocate the "v_name" array */
	C_MAKE(v_name, v_head->name_size, char);

	/* Read the "v_name" array */
	fd_read(fd, (char*)(v_name), v_head->name_size);


#ifndef DELAY_LOAD_V_TEXT

	/* Allocate the "v_text" array */
	C_MAKE(v_text, v_head->text_size, char);

	/* Read the "v_text" array */
	fd_read(fd, (char*)(v_text), v_head->text_size);

#endif /* DELAY_LOAD_V_TEXT */

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
	int fd;

	int mode = FILE_MODE;

	errr err;

	FILE *fp;

	/* General buffer */
	char buf[1024];


	/*** Make the header ***/

	/* Allocate the "header" */
	MAKE(v_head, header);

	/* Save the "version" */
	v_head->v_major = VERSION_MAJOR;
	v_head->v_minor = VERSION_MINOR;
	v_head->v_patch = VERSION_PATCH;
	v_head->v_extra = 0;

	/* Save the "record" information */
	v_head->info_num = max_v_idx;
	v_head->info_len = sizeof(vault_type);

	/* Save the size of "v_head" and "v_info" */
	v_head->head_size = sizeof(header);
	v_head->info_size = v_head->info_num * v_head->info_len;


#ifdef ALLOW_TEMPLATES

	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

#if 0

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

#else

	fd = -1;

#endif

	/* Process existing "raw" file */
	if (fd >= 0)
	{
#ifdef CHECK_MODIFICATION_TIME

		err = check_modification_date(fd, "v_info.txt");

#endif /* CHECK_MODIFICATION_TIME */

		/* Attempt to parse the "raw" file */
		if (!err)
			err = init_v_info_raw(fd);

		/* Close it */
		(void)fd_close(fd);

		/* Success */
		if (!err) return (0);

		/* Information */
		msg_print("Ignoring obsolete/defective 'v_info.raw' file.");
		msg_print(NULL);
	}


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

	/* Grab permission */
	safe_setuid_grab();

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Drop permission */
	safe_setuid_drop();

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


	/*** Dump the binary image file ***/

	/* File type is "DATA" */
	FILE_TYPE(FILE_TYPE_DATA);

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Kill the old file */
	(void)fd_kill(buf);

	/* Attempt to create the raw file */
	fd = fd_make(buf, mode);

	/* Drop permission */
	safe_setuid_drop();

	/* Dump to the file */
	if (fd >= 0)
	{
		/* Dump it */
		fd_write(fd, (char*)(v_head), v_head->head_size);

		/* Dump the "v_info" array */
		fd_write(fd, (char*)(v_info), v_head->info_size);

		/* Dump the "v_name" array */
		fd_write(fd, (char*)(v_name), v_head->name_size);

		/* Dump the "v_text" array */
		fd_write(fd, (char*)(v_text), v_head->text_size);

		/* Close */
		(void)fd_close(fd);
	}


	/*** Kill the fake arrays ***/

	/* Free the "v_info" array */
	C_KILL(v_info, v_head->info_num, vault_type);

	/* Hack -- Free the "fake" arrays */
	C_KILL(v_name, fake_name_size, char);
	C_KILL(v_text, fake_text_size, char);

	/* Forget the array sizes */
	fake_name_size = 0;
	fake_text_size = 0;

#endif /* ALLOW_TEMPLATES */


	/*** Load the binary image file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_DATA, "v_info.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the "raw" file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Process existing "raw" file */
	if (fd < 0) quit("Cannot load 'v_info.raw' file.");

	/* Attempt to parse the "raw" file */
	err = init_v_info_raw(fd);

	/* Close it */
	(void)fd_close(fd);

	/* Error */
	if (err) quit("Cannot parse 'v_info.raw' file.");

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
	C_MAKE(macro__cmd, MACRO_MAX, bool);

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
 * Pseudo, dummy quest initializer, to actualy disable them
 */
static bool quest_disable_init_hook(int q_idx)
{
	q_idx = q_idx;
	return FALSE;
}


/*
 * Initialise misc. values
 */
static errr init_misc(void)
{
	int xstart = 0;
	int ystart = 0;
	int i;
	s32b allow_quest;
	s32b allow_rquest;

	/*** Prepare the various "bizarre" arrays ***/

	/* Quark variables */
	C_MAKE(quark__str, QUARK_MAX, cptr);

	/* Message variables */
	C_MAKE(message__ptr, MESSAGE_MAX, u16b);
	C_MAKE(message__color, MESSAGE_MAX, byte);
	C_MAKE(message__type, MESSAGE_MAX, byte);
	C_MAKE(message__count, MESSAGE_MAX, u16b);
	C_MAKE(message__buf, MESSAGE_BUF, char);

	/* Hack -- No messages yet */
	message__tail = MESSAGE_BUF;

	/* Prepare powers */
	p_ptr->powers = NULL;
	powers_type = NULL;
	power_max = POWER_MAX_INIT;
	reinit_powers_type(power_max);
	C_COPY(powers_type, powers_type_init, POWER_MAX_INIT, power_type);

	/* Prepare quests */
	call_lua("get_module_info", "(s)", "d", "C_quest", &allow_quest);
	call_lua("get_module_info", "(s)", "d", "rand_quest", &allow_rquest);

	quest = NULL;
	max_q_idx = MAX_Q_IDX_INIT;
	reinit_quests(max_q_idx);

	C_COPY(quest, quest_init_tome, MAX_Q_IDX_INIT, quest_type);

	/* If we dont allow C quests, we dont let them init */
	if (!allow_quest)
	{
		for (i = 0; i < MAX_Q_IDX_INIT; i++)
		{
			if (allow_rquest && (i == QUEST_RANDOM))
				continue;
			quest[i].init = quest_disable_init_hook;
		}
	}

	/* Prepare gods */
	deity_info = NULL;
	max_gods = MAX_GODS_INIT;
	reinit_gods(max_gods);

	C_COPY(deity_info, deity_info_init, MAX_GODS_INIT, deity_type);

	/* Prepare schools */
	max_spells = 0;
	max_schools = 0;
	schools = NULL;
	school_spells = NULL;

	process_hooks(HOOK_INIT_GAME, "(s)", "begin");

	/* Initialise the values */
	process_dungeon_file(NULL, "misc.txt", &ystart, &xstart, 0, 0, TRUE);

	/* Init the spell effects */
	for (i = 0; i < MAX_EFFECTS; i++)
		effects[i].time = 0;

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
 * XXX XXX XXX XXX XXX Realloc is not guaranteed to work (see main-gtk.c
 * and main-mac.c.
 */
void reinit_powers_type(s16b new_size)
{
	power_type *new_powers_type;
	bool *new_powers;

	C_MAKE(new_powers_type, new_size, power_type);
	C_MAKE(new_powers, new_size, bool);

	/* Reallocate the extra memory */
	if (powers_type && p_ptr->powers)
	{
		C_COPY(new_powers_type, powers_type, power_max, power_type);
		C_COPY(new_powers, p_ptr->powers, power_max, bool);

		C_FREE(powers_type, power_max, power_type);
		C_FREE(p_ptr->powers, power_max, bool);
	}

	powers_type = new_powers_type;
	p_ptr->powers = new_powers;

	power_max = new_size;
}

void reinit_quests(s16b new_size)
{
	quest_type *new_quest;

	C_MAKE(new_quest, new_size, quest_type);

	/* Reallocate the extra memory */
	if (quest)
	{
		C_COPY(new_quest, quest, max_q_idx, quest_type);

		C_FREE(quest, max_q_idx, quest_type);
	}

	quest = new_quest;

	max_q_idx = new_size;
}

void reinit_gods(s16b new_size)
{
	deity_type *new_deity;

	C_MAKE(new_deity, new_size, deity_type);

	/* Reallocate the extra memory */
	if (deity_info)
	{
		C_COPY(new_deity, deity_info, max_gods, deity_type);

		C_FREE(deity_info, max_gods, deity_type);
	}

	deity_info = new_deity;

	max_gods = new_size;
}

void init_spells(s16b new_size)
{
	/* allocate the extra memory */
	C_MAKE(school_spells, new_size, spell_type);
	max_spells = new_size;
}

void init_schools(s16b new_size)
{
	/* allocate the extra memory */
	C_MAKE(schools, new_size, school_type);
	max_schools = new_size;
}

void init_corruptions(s16b new_size)
{
	/* allocate the extra memory */
	C_MAKE(p_ptr->corruptions, new_size, bool);
	max_corruptions = new_size;
}

/*
 * Initialise some other arrays
 */
static errr init_other(void)
{
	int i, n;

	/*** Prepare the "dungeon" information ***/

	/* Allocate and Wipe the special gene flags */
	C_MAKE(m_allow_special, max_r_idx, bool);
	C_MAKE(k_allow_special, max_k_idx, bool);
	C_MAKE(a_allow_special, max_a_idx, bool);


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
		C_MAKE(special_lvl[i], max_d_idx, bool);
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

	s16b *tmp;

	/*** Analyze object allocation info ***/

	/* Clear the "aux" array */
	tmp = C_WIPE(&aux, MAX_DEPTH_MONSTER, s16b);

	/* Clear the "num" array */
	tmp = C_WIPE(&num, MAX_DEPTH_MONSTER, s16b);

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
	tmp = C_WIPE(&aux, MAX_DEPTH_MONSTER, s16b);

	/* Clear the "num" array */
	tmp = C_WIPE(&num, MAX_DEPTH_MONSTER, s16b);

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

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

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
	path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");

	/* Grab permission */
	safe_setuid_grab();

	/* Attempt to open the high score file */
	fd = fd_open(buf, O_RDONLY);

	/* Drop permission */
	safe_setuid_drop();

	/* Failure */
	if (fd < 0)
	{
		/* File type is "DATA" */
		FILE_TYPE(FILE_TYPE_DATA);

		/* Grab permission */
		safe_setuid_grab();

		/* Create a new high score file */
		fd = fd_make(buf, mode);

		/* Drop permission */
		safe_setuid_drop();

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

	/* Initilize the socket */
	zsock_init();

	/* Initialise misc. values */
	note("[Initialising values... (misc)]");
	if (init_misc()) quit("Cannot initialise misc. values");

	wipe_hooks();

	/* Initialise some other arrays */
	note("[Initialising lua scripting... (lua)]");
	init_lua();
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
	tome_dofile_anywhere(ANGBAND_DIR_CORE, "auto.lua", TRUE);
	tome_dofile_anywhere(ANGBAND_DIR_USER, "automat.atm", FALSE);

	/* Done */
	note("[Initialisation complete]");

	process_hooks(HOOK_INIT_GAME, "(s)", "end");
}
