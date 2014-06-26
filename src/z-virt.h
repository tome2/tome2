/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Calloc wrapper which aborts if NULL is returned by calloc
 */
extern void *safe_calloc(size_t nmemb, size_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif
