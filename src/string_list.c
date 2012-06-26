#include "angband.h"

int compare_string_list(string_list *a, string_list *b)
{
	if (a == b)
	{
		return 0;
	}

	return strcmp(a->s, b->s);
}

SGLIB_DEFINE_LIST_FUNCTIONS(string_list, compare_string_list, next);

/*
 * Initialize a string_list value. Copies the input string.
 */
void string_list_init(string_list *sl, cptr s)
{
	assert(sl != NULL);

	sl->s = string_make(s);
	sl->next = NULL;
}

/*
 * Destroy string_value.
 */
void string_list_destroy(string_list *sl)
{
	assert(sl != NULL);

	/* Free contained string */
	if (sl->s) {
		string_free(sl->s);
		sl->s = NULL;
	}

	/* We do NOT free the rest of the list. */
	sl->next = NULL;
}

/**
 * Append a string to a string_list.
 */
void string_list_append(string_list **slist, cptr s)
{
	string_list *e = malloc(sizeof(string_list));
	string_list_init(e, s);

	sglib_string_list_add(slist, e);
}
