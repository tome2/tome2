/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#include "z-virt.h"

/*
 * Calloc wrapper which aborts if NULL is returned by calloc
 */
extern void *safe_calloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);
	if ((nmemb > 0) && (p == NULL))
	{
		abort();
	}
	return p;
}
