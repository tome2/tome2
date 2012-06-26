#include "quark.h"

#include "angband.h"

/*
 * The number of quarks
 */
static s16b quark__num = 0;


/*
 * The pointers to the quarks [QUARK_MAX]
 */
static cptr *quark__str = NULL;


/*
 * Initialize the quark subsystem
 */
void quark_init()
{
	quark__num = 0;
	C_MAKE(quark__str, QUARK_MAX, cptr);
}


/*
* We use a global array for all inscriptions to reduce the memory
* spent maintaining inscriptions.  Of course, it is still possible
* to run out of inscription memory, especially if too many different
* inscriptions are used, but hopefully this will be rare.
*
* We use dynamic string allocation because otherwise it is necessary
* to pre-guess the amount of quark activity.  We limit the total
* number of quarks, but this is much easier to "expand" as needed.
*
* Any two items with the same inscription will have the same "quark"
* index, which should greatly reduce the need for inscription space.
*
* Note that "quark zero" is NULL and should not be "dereferenced".
*/

/*
* Add a new "quark" to the set of quarks.
*/
s16b quark_add(cptr str)
{
	int i;

	/* Look for an existing quark */
	for (i = 1; i < quark__num; i++)
	{
		/* Check for equality */
		if (streq(quark__str[i], str)) return (i);
	}

	/* Paranoia -- Require room */
	if (quark__num == QUARK_MAX) return (0);

	/* New maximal quark */
	quark__num = i + 1;

	/* Add a new quark */
	quark__str[i] = string_make(str);

	/* Return the index */
	return (i);
}


/*
* This function looks up a quark
*/
cptr quark_str(s16b i)
{
	cptr q;

	/* Verify */
	if ((i < 0) || (i >= quark__num)) i = 0;

	/* Access the quark */
	q = quark__str[i];

	/* Return the quark */
	return (q);
}


