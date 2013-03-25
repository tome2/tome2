#pragma once

#include "sglib.h"
#include "angband.h"

/*
 * String list.
 */
typedef struct string_list string_list;
struct string_list {
	/* The string list owns the string */
	cptr s;
	/* Next */
	string_list *next;
};

int compare_string_list(string_list *a, string_list *b);
SGLIB_DEFINE_LIST_PROTOTYPES(string_list, compare_string, next);

void string_list_init(string_list *sl, cptr s); /* Initialize element; copies string */
void string_list_destroy(string_list *sl);      /* Destroy element */
void string_list_append(string_list **slist, cptr s); /* Append string */
