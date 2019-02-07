/* File: h-system.h */

#ifndef INCLUDED_H_SYSTEM_H
#define INCLUDED_H_SYSTEM_H

/*
 * Include the basic "system" files.
 *
 * Make sure all "system" constants/macros are defined.
 * Make sure all "system" functions have "extern" declarations.
 *
 * This file is a big hack to make other files less of a hack.
 * This file has been rebuilt -- it may need a little more work.
 */


#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <stdlib.h>


#ifdef SET_UID

# include <sys/types.h>

# if defined(linux)
#  include <sys/time.h>
# endif

#endif


#include <time.h>



#if defined(WINDOWS)
# include <io.h>
#endif

#include <memory.h>


# include <fcntl.h>


#ifdef SET_UID

#  include <sys/param.h>
#  include <sys/file.h>

# ifdef linux
#  include <sys/file.h>
# endif

# include <pwd.h>

# include <unistd.h>

# include <sys/stat.h>


#endif

#ifdef SET_UID

# include <strings.h>

#else

# include <string.h>

#endif




#include <stdarg.h>


#endif

/* There was a bug introduced in 10.4.11; working around it */
#ifdef __APPLE__
#define GETLOGIN_BROKEN
#endif
