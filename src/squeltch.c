/* File: squeltch.c */

/* Purpose: Automatizer */

/*
 * Copyright (c) 2002 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include <jansson.h>

#include "quark.h"

#define RULES_MAX 4096
#define STACK_MAX 1024

typedef enum { BAD, VERY_BAD, AVERAGE,
	       GOOD, VERY_GOOD, SPECIAL,
	       TERRIBLE, NONE, CHEST_EMPTY,
	       CHEST_DISARMED } status_type;

struct status_map_type {
	status_type status;
	cptr status_s;
};

status_type object_status(object_type *o_ptr)
{
	if (!object_known_p(o_ptr))
	{
		switch (o_ptr->sense)
		{
		case SENSE_CURSED: return BAD;
		case SENSE_WORTHLESS: return VERY_BAD;
		case SENSE_AVERAGE: return AVERAGE;
		case SENSE_GOOD_LIGHT: return GOOD;
		case SENSE_GOOD_HEAVY: return GOOD;
		case SENSE_EXCELLENT: return VERY_GOOD;
		case SENSE_SPECIAL: return SPECIAL;
		case SENSE_TERRIBLE: return TERRIBLE;
		default: return NONE;
		}
	}
	else
	{
		s16b slot = wield_slot_ideal(o_ptr, TRUE);

		if (artifact_p(o_ptr))
		{
			if (!(o_ptr->ident & IDENT_CURSED))
			{
				return SPECIAL;
			}
			else
			{
				return TERRIBLE;
			}
		}
		else if ((o_ptr->name2 > 0) ||
			 (o_ptr->name2b > 0))
		{
			if (!(o_ptr->ident & IDENT_CURSED))
			{
				return VERY_GOOD;
			}
			else
			{
				return VERY_BAD;
			}
		}
		else if ((slot == INVEN_WIELD) ||
			 (slot == INVEN_BOW) ||
			 (slot == INVEN_AMMO) ||
			 (slot == INVEN_TOOL))
		{
			if (o_ptr->to_h + o_ptr->to_d < 0)
			{
				return BAD;
			}
			else if (o_ptr->to_h + o_ptr->to_d > 0)
			{
				return GOOD;
			}
			else
			{
				return AVERAGE;
			}
		}
		else if ((slot >= INVEN_BODY) &&
			 (slot <= INVEN_FEET))
		{
			if (o_ptr->to_a < 0)
			{
				return BAD;
			}
			else if (o_ptr->to_a > 0)
			{
				return GOOD;
			}
			else
			{
				return AVERAGE;
			}
		}
		else if (slot == INVEN_RING)
		{
			if ((o_ptr->to_d + o_ptr->to_h < 0) ||
			    (o_ptr->to_a < 0) ||
			    (o_ptr->pval < 0))
			{
				return BAD;
			}
			else
			{
				return AVERAGE;
			}
		}
		else if (slot == INVEN_NECK)
		{
			if (o_ptr->pval < 0)
			{
				return BAD;
			}
			else
			{
				return AVERAGE;
			}
		}
		else if (o_ptr->tval == TV_CHEST)
		{
			if (o_ptr->pval == 0)
			{
				return CHEST_EMPTY;
			}
			else if (o_ptr->pval < 0)
			{
				return CHEST_DISARMED;
			}
			else
			{
				return AVERAGE;
			}
		}
		else
		{
			return AVERAGE;
		}
	}
}

#define STATUS_MAP_SIZE 10
struct status_map_type status_map[STATUS_MAP_SIZE] = {
	{ BAD, "bad" },
	{ VERY_BAD, "very bad" },
	{ AVERAGE, "average" },
	{ GOOD, "good" },
	{ VERY_GOOD, "very good" },
	{ SPECIAL, "special" },
	{ TERRIBLE, "terrible" },
	{ NONE, "none" },
	{ CHEST_EMPTY, "(empty chest)" },
	{ CHEST_DISARMED, "(disarmed chest)" },
};

static cptr status_to_string(status_type status)
{
	int i;

	for (i = 0; i < STATUS_MAP_SIZE; i++)
	{
		if (status_map[i].status == status)
		{
			return status_map[i].status_s;
		}
	}

	assert(FALSE);
	return NULL;
}

static bool_ status_from_string(cptr s, status_type *status)
{
	int i;

	for (i = 0; i < STATUS_MAP_SIZE; i++)
	{
		if (streq(status_map[i].status_s, s))
		{
			*status = status_map[i].status;
			return TRUE;
		}
	}

	return FALSE;
}

/* Type of automatizer actions */
typedef enum { AUTO_DESTROY,
	       AUTO_PICKUP,
	       AUTO_INSCRIBE } action_type;

/* Convert action to/from string */
struct action_map_type {
	action_type action;
	cptr action_s;
};

#define ACTION_MAP_SIZE 3
struct action_map_type action_map[ACTION_MAP_SIZE] = {
	{ AUTO_DESTROY, "destroy" },
	{ AUTO_PICKUP, "pickup" },
	{ AUTO_INSCRIBE, "inscribe" }
};

static cptr action_to_string(action_type action)
{
	int i = 0;

	for (i = 0; i < ACTION_MAP_SIZE; i++)
	{
		if (action == action_map[i].action)
		{
			return action_map[i].action_s;
		}
	}

	assert(FALSE);
	return NULL;
}

static bool_ action_from_string(cptr s, action_type *action)
{
	int i = 0;

	for (i = 0; i < ACTION_MAP_SIZE; i++)
	{
		if (streq(action_map[i].action_s, s))
		{
			*action = action_map[i].action;
			return TRUE;
		}
	}

	return FALSE;
}

/* Identification state */
typedef enum { IDENTIFIED, NOT_IDENTIFIED } identification_state;

#define S_IDENTIFIED "identified"
#define S_NOT_IDENTIFIED "not identified"

cptr identification_state_to_string(identification_state i)
{
	switch (i)
	{
	case IDENTIFIED: return S_IDENTIFIED;
	case NOT_IDENTIFIED: return S_NOT_IDENTIFIED;
	}

	assert(FALSE);
	return NULL;
}

bool_ identification_state_from_string(cptr s, identification_state *state)
{
	if (streq(s, S_IDENTIFIED))
	{
		*state = IDENTIFIED;
		return TRUE;
	}
	else if (streq(s, S_NOT_IDENTIFIED))
	{
		*state = NOT_IDENTIFIED;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* Match type */
typedef enum { M_AND      , M_OR      , M_NOT    , M_NAME     , M_CONTAIN   ,
	       M_INSCRIBED, M_DISCOUNT, M_SYMBOL , M_STATE    , M_STATUS    ,
	       M_TVAL     , M_SVAL    , M_RACE   , M_SUBRACE  , M_CLASS     ,
	       M_LEVEL    , M_SKILL   , M_ABILITY, M_INVENTORY, M_EQUIPMENT }
	match_type;

struct match_type_map {
	match_type i;
	cptr s;
};

#define MATCH_TYPE_MAP_SIZE 20
struct match_type_map match_type_map[MATCH_TYPE_MAP_SIZE] = {
	{ M_AND, "and" },
	{ M_OR, "or" },
	{ M_NOT, "not" },
	{ M_NAME, "name" },
	{ M_CONTAIN, "contain" },
	{ M_INSCRIBED, "inscribed" },
	{ M_DISCOUNT, "discount" },
	{ M_SYMBOL, "symbol" },
	{ M_STATE, "state" },
	{ M_STATUS, "status" },
	{ M_TVAL, "tval" },
	{ M_SVAL, "sval" },
	{ M_RACE, "race" },
	{ M_SUBRACE, "subrace" },
	{ M_CLASS, "class" },
	{ M_LEVEL, "level" },
	{ M_SKILL, "skill" },
	{ M_ABILITY, "ability" },
	{ M_INVENTORY, "inventory" },
	{ M_EQUIPMENT, "equipment" },
};

cptr match_type_to_string(match_type m)
{
	int i;

	for (i = 0; i < MATCH_TYPE_MAP_SIZE; i++)
	{
		if (match_type_map[i].i == m)
		{
			return match_type_map[i].s;
		}
	}

	assert(FALSE);
	return NULL;
}

bool_ match_type_from_string(cptr s, match_type *match)
{
	int i;

	for (i = 0; i < MATCH_TYPE_MAP_SIZE; i++)
	{
		if (streq(match_type_map[i].s, s))
		{
			*match = match_type_map[i].i;
			return TRUE;
		}
	}

	return FALSE;
}

/* Forward declarations */
typedef struct condition_type condition_type;
struct condition_type;

/* List of conditions */
typedef struct condition_list condition_list;
struct condition_list {
	condition_type *condition;
	condition_list *next;
};

int compare_condition_list(condition_list *a, condition_list *b)
{
	assert(FALSE);
}

SGLIB_DEFINE_LIST_PROTOTYPES(condition_list, compare_condition_list, next);
SGLIB_DEFINE_LIST_FUNCTIONS(condition_list, compare_condition_list, next);

/* Condition instance */
struct condition_type
{
	/* What do we want to match? */
	match_type match;
	/* Sub-conditions for logical connectives; if applicable */
	struct {
		condition_list *c;
	} conditions;
	/* Sub-condition for cases where there is only a single subcondition */
	condition_type *subcondition;
	/* Tval to match if applicable. */
	byte tval;
	/* Sval range if applicable. */
	struct {
		byte min;
		byte max;
	} sval_range;
	/* Discount range. */
	struct {
		int min;
		int max;
	} discount;
	/* Level range */
	struct {
		int min;
		int max;
	} level_range;
	/* Skill range */
	struct {
		s16b min;
		s16b max;
		s16b skill_idx;
	} skill_range;
	/* Identification state to match if applicable */
	identification_state identification_state;
	/* Status to match if applicable */
	status_type status;
	/* Name to match */
	char *name;
	/* Symbol to match if applicable */
	char symbol;
	/* Inscription to find */
	char *inscription;
	/* Subrace to match if applicable */
	char *subrace;
	/* Race to match if applicable */
	char *race;
	/* Class to match if applicable */
	char *klass;
	/* Ability to match if applicable */
	s16b ability;
	/* Comment */
	char *comment;
};

static condition_type *condition_new(match_type match)
{
	condition_type *cp = malloc(sizeof(condition_type));
	memset(cp, 0, sizeof(condition_type));
	cp->match = match;
	return cp;
}

static condition_type *condition_new_tval(byte tval)
{
	condition_type *cp = condition_new(M_TVAL);
	cp->tval = tval;
	return cp;
}

static condition_type *condition_new_sval(byte min, byte max)
{
	condition_type *cp = condition_new(M_SVAL);
	cp->sval_range.min = min;
	cp->sval_range.max = max;
	return cp;
}

static condition_type *condition_new_and()
{
	condition_type *cp = condition_new(M_AND);
	return cp;
}

static condition_type *condition_new_or()
{
	condition_type *cp = condition_new(M_OR);
	return cp;
}

static condition_type *condition_new_not()
{
	condition_type *cp = condition_new(M_NOT);
	return cp;
}

static condition_type *condition_new_name(cptr name)
{
	condition_type *cp = condition_new(M_NAME);
	cp->name = strdup(name);
	return cp;
}

static condition_type *condition_new_contain(cptr name)
{
	condition_type *cp = condition_new(M_CONTAIN);
	cp->name = strdup(name);
	return cp;
}

static condition_type *condition_new_inscribed(cptr name)
{
	condition_type *cp = condition_new(M_INSCRIBED);
	cp->inscription = strdup(name);
	return cp;
}

static condition_type *condition_new_status(status_type status)
{
	condition_type *cp = condition_new(M_STATUS);
	cp->status = status;
	return cp;
}

static condition_type *condition_new_state(identification_state state)
{
	condition_type *cp = condition_new(M_STATE);
	cp->identification_state = state;
	return cp;
}

static condition_type *condition_new_discount(int min, int max)
{
	condition_type *cp = condition_new(M_DISCOUNT);
	cp->discount.min = min;
	cp->discount.max = max;
	return cp;
}

static condition_type *condition_new_symbol(char c)
{
	condition_type *cp = condition_new(M_SYMBOL);
	cp->symbol = c;
	return cp;
}

static condition_type *condition_new_race(cptr race)
{
	condition_type *cp = condition_new(M_RACE);
	cp->race = strdup(race);
	return cp;
}

static condition_type *condition_new_subrace(cptr subrace)
{
	condition_type *cp = condition_new(M_SUBRACE);
	cp->subrace = strdup(subrace);
	return cp;
}

static condition_type *condition_new_class(cptr klass)
{
	condition_type *cp = condition_new(M_CLASS);
	cp->klass = strdup(klass);
	return cp;
}

static condition_type *condition_new_level(int min, int max)
{
	condition_type *cp = condition_new(M_LEVEL);
	cp->level_range.min = min;
	cp->level_range.max = max;
	return cp;
}

static condition_type *condition_new_skill(s16b min, s16b max, s16b skill_idx)
{
	condition_type *cp = condition_new(M_SKILL);
	cp->skill_range.min = min;
	cp->skill_range.max = max;
	cp->skill_range.skill_idx = skill_idx;
	return cp;
}

static condition_type *condition_new_ability(s16b ability_idx)
{
	condition_type *cp = condition_new(M_ABILITY);
	cp->ability = ability_idx;
	return cp;
}

static condition_type *condition_new_inventory()
{
	condition_type *cp = condition_new(M_INVENTORY);
	return cp;
}

static condition_type *condition_new_equipment()
{
	condition_type *cp = condition_new(M_EQUIPMENT);
	return cp;
}

static void condition_and_add(condition_type *and_c, condition_type *c)
{
	assert(and_c != NULL);
	assert(c != NULL);
	assert((and_c->match == M_AND) || (and_c->match == M_OR));

	condition_list *cl = malloc(sizeof(condition_list));
	cl->condition = c;
	cl->next = NULL;

	sglib_condition_list_add(&and_c->conditions.c, cl);
}

static void condition_or_add(condition_type *or_c, condition_type *c)
{
	condition_and_add(or_c, c);
}

static void condition_destroy(condition_type **cp)
{
	condition_type *c = NULL;
	assert(cp != NULL);
	assert(*cp != NULL);

	c = *cp;

	/* Free sub-conditions if any */
	{
		condition_list *current = NULL;
		condition_list *next = NULL;

		for (current = c->conditions.c;
		     current != NULL;
		     current = next)
		{
			condition_destroy(&current->condition);
			next = current->next;
			free(current);
		}
	}

	/* Free sub-condition if any */
	if (c->subcondition)
	{
		condition_destroy(&c->subcondition);
	}

	/* Free name if any */
	if (c->name)
	{
		free(c->name);
		c->name = NULL;
	}

	/* Free inscription if any */
	if (c->inscription)
	{
		free(c->inscription);
		c->inscription = NULL;
	}

	/* Free subrace if any */
	if (c->subrace)
	{
		free(c->subrace);
		c->subrace = NULL;
	}

	/* Free race if any */
	if (c->race)
	{
		free(c->race);
		c->race = NULL;
	}

	/* Free class if any */
	if (c->klass)
	{
		free(c->klass);
		c->klass = NULL;
	}

	/* Free comment if any */
	if (c->comment)
	{
		free(c->comment);
		c->comment = NULL;
	}

	/* Free the condition itself */
	free(*cp);
	*cp = NULL;
}

static bool_ condition_eval(condition_type *c, object_type *o_ptr)
{
	bool_ is_and = (c->match == M_AND);
	bool_ is_or  = (c->match == M_OR);

	switch (c->match)
	{
	case M_AND:
	case M_OR:
	{
		struct sglib_condition_list_iterator it;
		struct condition_list *child = NULL;

		for (child = sglib_condition_list_it_init(&it, c->conditions.c);
		     child != NULL;
		     child = sglib_condition_list_it_next(&it))
		{
			if (is_and && (!condition_eval(child->condition, o_ptr)))
			{
				return FALSE;
			}

			if (is_or && condition_eval(child->condition, o_ptr))
			{
				return TRUE;
			}
		}

		if (is_and)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	case M_NOT:
	{
		if (c->subcondition == NULL)
		{
			return TRUE;
		}

		return !condition_eval(c->subcondition, o_ptr);
	}

	case M_INVENTORY:
	{
		int i;

		if (c->subcondition == NULL)
		{
			return FALSE;
		}

		for (i = 0; i < INVEN_WIELD; i++)
		{
			if (condition_eval(c->subcondition, &p_ptr->inventory[i]))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	case M_EQUIPMENT:
	{
		int i;

		if (c->subcondition == NULL)
		{
			return FALSE;
		}

		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			if (condition_eval(c->subcondition, &p_ptr->inventory[i]))
			{
				return TRUE;
			}
		}
		
		return FALSE;
	}

	case M_NAME:
	{
		char buf1[128];
		char buf2[128];

		object_desc(buf1, o_ptr, -1, 0);
		strlower(buf1);

		sprintf(buf2, "%s", c->name);
		strlower(buf2);

		return streq(buf1, buf2);
	}

        case M_CONTAIN:
	{
		char buf1[128];
		char buf2[128];

		object_desc(buf1, o_ptr, -1, 0);
		strlower(buf1);

		sprintf(buf2, "%s", c->name);
		strlower(buf2);

		return (strstr(buf1, buf2) != NULL);
	}

	case M_SYMBOL:
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		return k_ptr->d_char == c->symbol;
	}

	case M_INSCRIBED:
	{
		char buf1[128];
		char buf2[128];

		if (o_ptr->note == 0)
		{
			return FALSE;
		}

		sprintf(buf1, "%s", quark_str(o_ptr->note));
		strlower(buf1);

		sprintf(buf2, "%s", c->inscription);
		strlower(buf2);

		return (strstr(buf1, buf2) != NULL);
	}

	case M_DISCOUNT:
	{
		return (object_aware_p(o_ptr) &&
			(o_ptr->discount >= c->discount.min) &&
			(o_ptr->discount <= c->discount.max));
	}

	case M_TVAL:
	{
		return (o_ptr->tval == c->tval);
	}

	case M_SVAL:
	{
		return (object_aware_p(o_ptr) &&
			(o_ptr->sval >= c->sval_range.min) &&
			(o_ptr->sval <= c->sval_range.max));
	}

	case M_STATUS:
	{
		return c->status == object_status(o_ptr);
	}

	case M_STATE:
	{
		switch (c->identification_state)
		{
		case IDENTIFIED:
			return object_known_p(o_ptr);
		case NOT_IDENTIFIED:
			return !object_known_p(o_ptr);
		default:
			assert(FALSE);
		}
	}

	case M_RACE:
	{
		char buf1[128];
		char buf2[128];

		sprintf(buf1, "%s", rp_ptr->title + rp_name);
		strlower(buf1);

		sprintf(buf2, "%s", c->race);
		strlower(buf2);

		return streq(buf1, buf2);
	}

	case M_SUBRACE:
	{
		char buf1[128];
		char buf2[128];
		
		sprintf(buf1, "%s", rmp_ptr->title + rmp_name);
		strlower(buf1);

		sprintf(buf2, "%s", c->subrace);
		strlower(buf2);

		return streq(buf1, buf2);
	}

	case M_CLASS:
	{
		char buf1[128];
		char buf2[128];

		sprintf(buf1, "%s", spp_ptr->title + c_name);
		strlower(buf1);

		sprintf(buf2, "%s", c->race);
		strlower(buf2);

		return streq(buf1, buf2);
	}

	case M_LEVEL:
	{
		return ((p_ptr->lev >= c->level_range.min) &&
			(p_ptr->lev <= c->level_range.max));
	}

	case M_SKILL:
	{
		s16b sk = get_skill(c->skill_range.skill_idx); 
		return ((sk >= c->skill_range.min) &&
			(sk <= c->skill_range.max));
	}

	case M_ABILITY:
	{
		return has_ability(c->ability);
	}

	}

	/* Don't match by default */
	return FALSE;
}

static json_t *condition_to_json(condition_type *c)
{
	json_t *json = NULL;

	if (c == NULL)
	{
		return json_null();
	}

	json = json_object();
	json_object_set_new(json, "type",
			    json_string(match_type_to_string(c->match)));

	switch (c->match)
	{
	case M_AND:
	case M_OR:
	{
		struct sglib_condition_list_iterator it;
		struct condition_list *child = NULL;

		json_t *conditions_json = json_array();

		for (child = sglib_condition_list_it_init(&it, c->conditions.c);
		     child != NULL;
		     child = sglib_condition_list_it_next(&it))
		{
			json_array_append_new(conditions_json,
					      condition_to_json(child->condition));
		}

		json_object_set_new(json, "conditions", conditions_json);
		break;
	}

	case M_NOT:
	case M_INVENTORY:
	case M_EQUIPMENT:
	{
		json_object_set_new(json, "condition",
				    condition_to_json(c->subcondition));
		break;
	}

	case M_NAME:
	{
		json_object_set_new(json, "name",
				    json_string(c->name));
		break;
	}

	case M_CONTAIN:
	{
		json_object_set_new(json, "contain",
				    json_string(c->name));
		break;
	}

	case M_SYMBOL:
	{
		json_object_set_new(json, "symbol",
				    json_string(format("%c", c->symbol)));
		break;
	}

	case M_INSCRIBED:
	{
		json_object_set_new(json, "inscription",
				    json_string(c->inscription));
		break;
	}

	case M_DISCOUNT:
	{
		json_object_set_new(json, "min",
				    json_integer(c->discount.min));
		json_object_set_new(json, "max",
				    json_integer(c->discount.max));
		break;
	}

	case M_TVAL:
	{
		json_object_set_new(json, "tval",
				    json_integer(c->tval));
		break;
	}

	case M_SVAL:
	{
		json_object_set_new(json, "min",
				    json_integer(c->sval_range.min));
		json_object_set_new(json, "max",
				    json_integer(c->sval_range.max));
		break;
	}

	case M_STATUS:
	{
		json_object_set_new(json, "status",
				    json_string(status_to_string(c->status)));
		break;
	}

	case M_STATE:
	{
		json_object_set_new(json, "state",
				    json_string(identification_state_to_string(c->identification_state)));
		break;
	}

	case M_RACE:
	{
		json_object_set_new(json, "race",
				    json_string(c->race));
		break;
	}

	case M_SUBRACE:
	{
		json_object_set_new(json, "subrace",
				    json_string(c->subrace));
		break;
	}

	case M_CLASS:
	{
		json_object_set_new(json, "class",
				    json_string(c->klass));
		break;
	}

	case M_LEVEL:
	{
		json_object_set_new(json, "min",
				    json_integer(c->level_range.min));
		json_object_set_new(json, "max",
				    json_integer(c->level_range.max));
		break;
	}

	case M_SKILL:
	{
		json_object_set_new(json, "name",
				    json_string(s_info[c->skill_range.skill_idx].name + s_name));
		json_object_set_new(json, "min",
				    json_integer(c->skill_range.min));
		json_object_set_new(json, "max",
				    json_integer(c->skill_range.max));
		break;
	}

	case M_ABILITY:
	{
		json_object_set_new(json, "ability",
				    json_string(ab_info[c->ability].name + ab_name));
		break;
	}

	}

	return json;
}

/*
 * Cursor to maintain position in condition tree
 */
static condition_type *cursor_stack[STACK_MAX];
static int cursor_count = 0;

static void cursor_push(condition_type *condition)
{
	assert(cursor_count < STACK_MAX);

	cursor_stack[cursor_count] = condition;
	cursor_count++;
}

static condition_type *cursor_pop()
{
	condition_type *c = NULL;

	assert(cursor_count > 0);

	c = cursor_stack[cursor_count-1];
	cursor_stack[cursor_count] = NULL;
	cursor_count--;
	return c;
}

static condition_type *cursor_top()
{
	assert(cursor_count > 0);

	return cursor_stack[cursor_count - 1];
}

static void cursor_clear()
{
	while (cursor_count > 0)
	{
		cursor_pop();
	}
}

/* Rule */
typedef struct arule_type arule_type;
struct arule_type {
	/* Rule name */
	char *name;
	/* Which action do we take? */
	action_type action; /* Which action to take */
	/* Which module does this rule apply to? */
	int module_idx;
	/* Inscription to use for inscription rules. */
	char *inscription;
	/* Condition. */
	condition_type *condition; /* Condition for rule match */
};

/* Initialize a rule */
static arule_type *rule_new(cptr name, action_type action, int module_idx, condition_type *condition, cptr inscription)
{
	arule_type *rp = malloc(sizeof(arule_type));
	rp->name = strdup(name);
	rp->action = action;
	rp->module_idx = module_idx;
	rp->condition = condition;
	rp->inscription = (inscription == NULL) ? NULL : strdup(inscription);
	return rp;
}

static void rule_set_name(arule_type *rule, cptr new_name)
{
	if (rule->name)
	{
		free(rule->name);
	}

	rule->name = strdup(new_name);
}

static void rule_destroy(arule_type **rp)
{
	if ((*rp)->name)
	{
		free((*rp)->name);
	}

	if ((*rp)->inscription)
	{
		free((*rp)->inscription);
	}

	if ((*rp)->condition)
	{
		condition_destroy(&(*rp)->condition);
	}

	free(*rp);
	*rp = NULL;
}

/* Current list of rules. */
static arule_type *rules[RULES_MAX];
static int rules_count = 0; /* Number of rules currently in list */

static int rules_append(arule_type *rule)
{
	assert(rules_count < RULES_MAX);

	rules[rules_count] = rule;
	rules_count++;
	return rules_count-1;
}

static void rules_remove(arule_type *rule)
{
	int i, j;

	for (i = 0; i < rules_count; i++)
	{
		if (rules[i] == rule)
		{
			/* Free the rule */
			rule_destroy(&rule);
			/* Move rest of rest "up" */
			for (j = i+1; j < rules_count; j++)
			{
				rules[j-1] = rules[j];
			}
			/* We're done */
			rules_count--;
			return;
		}
	}
}

static void rules_swap(int i, int j)
{
	arule_type *tmp_rptr = NULL;

	assert(i >= 0);
	assert(i < rules_count);

	assert(j >= 0);
	assert(j < rules_count);

	tmp_rptr = rules[i];
	rules[i] = rules[j];
	rules[j] = tmp_rptr;
}

static bool_* automatizer_auto_destroy(object_type *o_ptr, int item_idx)
{
	static bool_ TRUE_VAL = TRUE;

	/* Must be identified */
	if (object_aware_p(o_ptr) == FALSE)
	{
		return NULL;
	}

	/* Inscribed things won't be destroyed! */
	if (o_ptr->note)
	{
		return NULL;
	}

	/* Ignore artifacts; cannot be destroyed anyway. */
	if (artifact_p(o_ptr))
	{
		return NULL;
	}

	/* Cannot destroy CURSE_NO_DROP objects. */
	{
		u32b f1, f2, f3, f4, f5, esp;
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		if ((f4 & TR4_CURSE_NO_DROP) != 0)
		{
			return NULL;
		}
	}

	/* Destroy! */
	msg_print("<Auto-destroy>");

	inc_stack_size(item_idx, -o_ptr->number);
	return &TRUE_VAL;
}

static bool_* automatizer_auto_pickup(object_type *o_ptr, int item_idx)
{
	static bool_ TRUE_VAL = TRUE;

	if (item_idx >= 0)
	{
		return NULL;
	}

	if (!inven_carry_okay(o_ptr))
	{
		return NULL;
	}

	msg_print("<Auto-pickup>");

	object_pickup(-item_idx);
	return &TRUE_VAL;
}

/* Apply rules */
static bool_ apply_rule(arule_type *rule, object_type *o_ptr, int item_idx)
{
	/* Check module */
	if (rule->module_idx != game_module_idx)
	{
		return FALSE;
	}

	/* Check condition */
	assert (rule->condition != NULL);
	if (condition_eval(rule->condition, o_ptr))
	{
		switch (rule->action)
		{

		case AUTO_DESTROY:
		{
			automatizer_auto_destroy(o_ptr, item_idx);
			break;
		}

		case AUTO_PICKUP:
		{
			automatizer_auto_pickup(o_ptr, item_idx);
			break;
		}

		case AUTO_INSCRIBE:
		{
			/* Already inscribed? */
			if (o_ptr->note != 0)
			{
				return FALSE;
			}

			/* Inscribe */
			msg_format("<Auto-Inscribe {%s}>", rule->inscription);
			o_ptr->note = quark_add(rule->inscription);
			break;
		}

		}

		return TRUE;
	}

	return FALSE;
}

static bool_ apply_rules(object_type *o_ptr, int item_idx)
{
	int i;

	for (i = 0; i < rules_count; i++)
	{
		if (apply_rule(rules[i], o_ptr, item_idx))
		{
			return TRUE;
		}
	}

	/* Don't keep trying */
	return FALSE;
}

static json_t *rule_to_json(arule_type *rule)
{
	json_t *rule_json = json_object();

	json_object_set_new(rule_json,
			    "name",
			    json_string(rule->name));
	json_object_set_new(rule_json,
			    "action",
			    json_string(action_to_string(rule->action)));
	json_object_set_new(rule_json,
			    "module",
			    json_string(modules[rule->module_idx].meta.name));

	if (rule->inscription)
	{
		json_object_set_new(rule_json,
				    "inscription",
				    json_string(rule->inscription));
	}

	json_object_set_new(rule_json,
			    "condition",
			    condition_to_json(rule->condition));

	return rule_json;
}

static json_t *rules_to_json()
{
	int i;
	json_t *rules_json = json_array();

	for (i = 0; i < rules_count; i++)
	{
		json_array_append_new(rules_json, rule_to_json(rules[i]));
	}

	return rules_json;
}

/* Check the floor for "crap" */
void squeltch_grid(void)
{
	s16b this_o_idx, next_o_idx = 0;

	if (!automatizer_enabled) return;

	/* Scan the pile of objects */
	for (this_o_idx = cave[p_ptr->py][p_ptr->px].o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Acquire object */
		object_type * o_ptr = &o_list[this_o_idx];

		/* We've now seen one of these */
		if (!k_info[o_ptr->k_idx].flavor)
		{
			object_aware(o_ptr);
		}

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Apply rules */
		apply_rules(o_ptr, -this_o_idx);
	}
}


/* Check the inventory for "crap" */
void squeltch_inventory(void)
{
	int i;
	int num_iter = 0;
	bool_ found = TRUE;

	if (!automatizer_enabled) return;

	while (found && num_iter ++ < 100)
	{
		/* Sometimes an index in the inventory is skipped */
		found = FALSE;

		for (i = 0; i < INVEN_PACK; i++)
		{
			object_type *o_ptr = &p_ptr->inventory[i];

			if (apply_rules(o_ptr, i))
			{
				found = TRUE;
				break;
			}
		}
	}
	if (num_iter >= 100)
	{
		cmsg_format(TERM_VIOLET, "'apply_rules' ran too often.");
	}
}

/********************** The interface **********************/
static void get_rule_names(cptr *list)
{
	int i;

	for (i = 0; i < rules_count; i++)
	{
		list[i] = rules[i]->name;
	}
}

typedef struct condition_metadata condition_metadata;
struct condition_metadata {
	match_type match;
	cptr description[3];
	condition_type *(*create_condition)();
};

#define TYPES_LIST_SIZE 21

static condition_type *create_condition_name()
{
	cptr s = lua_input_box("Object name to match?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_name(s);
}

static condition_type *create_condition_contain()
{
	cptr s = lua_input_box("Word to find in object name?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_contain(s);
}

static condition_type *create_condition_inscribed()
{
	cptr s = lua_input_box("Word to find in object inscription?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_inscribed(s);
}

static condition_type *create_condition_discount()
{
	int min, max;

	{
		cptr s = lua_input_box("Min discount?", 79);
		if (sscanf(s, "%d", &min) < 1)
		{
			return NULL;
		}
	}

	{
		cptr s = lua_input_box("Max discount?", 79);
		if (sscanf(s, "%d", &max) < 1)
		{
			return NULL;
		}
	}

	return condition_new_discount(min, max);
}

static condition_type *create_condition_symbol()
{
	char c;
	cptr s = lua_input_box("Symbol to match?", 1);
	if (sscanf(s, "%c", &c) < 1)
	{
		return NULL;
	}

	return condition_new_symbol(c);
}

static condition_type *create_condition_status()
{
	status_type status;
	char c;

	c = lua_msg_box("[t]errible, [v]ery bad, [b]ad, "
			"[a]verage, [G]ood, [V]ery good, [S]pecial?");

	switch (c)
	{
	case 't': status = TERRIBLE; break;
	case 'v': status = VERY_BAD; break;
	case 'b': status = BAD; break;
	case 'a': status = AVERAGE; break;
	case 'G': status = GOOD; break;
	case 'V': status = VERY_GOOD; break;
	case 'S': status = SPECIAL; break;
	default: return NULL;
	}

	return condition_new_status(status);
}

static condition_type *create_condition_state()
{
	identification_state s;
	char c;

	c = lua_msg_box("[i]dentified, [n]on identified?");

	switch (c)
	{
	case 'i': s = IDENTIFIED; break;
	case 'n': s = NOT_IDENTIFIED; break;
	default: return NULL;
	}

	return condition_new_state(s);
}

static condition_type *create_condition_tval()
{
	int tval;
	cptr s = lua_input_box("Tval to match?", 79);
	if (sscanf(s, "%d", &tval) < 1)
	{
		return NULL;
	}

	return condition_new_tval(tval);
}

static condition_type *create_condition_sval()
{
	int sval_min, sval_max;

	{
		cptr s = lua_input_box("Min sval?", 79);
		if (sscanf(s, "%d", &sval_min) < 1)
		{
			return NULL;
		}
	}

	{
		cptr s = lua_input_box("Max sval?", 79);
		if (sscanf(s, "%d", &sval_max) < 1)
		{
			return NULL;
		}
	}

	return condition_new_sval(sval_min, sval_max);
}

static condition_type *create_condition_race()
{
	cptr s = lua_input_box("Player race to match?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_race(s);
}

static condition_type *create_condition_subrace()
{
	cptr s = lua_input_box("Player subrace to match?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_subrace(s); 
}

static condition_type *create_condition_class()
{
	cptr s = lua_input_box("Player class to match?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	return condition_new_class(s);
}

static condition_type *create_condition_level()
{
	int min, max;

	{
		cptr s = lua_input_box("Min player level?", 79);
		if (sscanf(s, "%d", &min) < 1)
		{
			return NULL;
		}
	}

	{
		cptr s = lua_input_box("Max player level?", 79);
		if (sscanf(s, "%d", &max) < 1)
		{
			return NULL;
		}
	}

	return condition_new_level(min, max);
}

static condition_type *create_condition_skill()
{
	int min, max;
	s16b skill_idx;

	{
		cptr s = lua_input_box("Min skill level?", 79);
		if (sscanf(s, "%d", &min) < 1)
		{
			return NULL;
		}
	}

	{
		cptr s = lua_input_box("Max skill level?", 79);
		if (sscanf(s, "%d", &max) < 1)
		{
			return NULL;
		}
	}

	{
		cptr s = lua_input_box("Skill name?", 79);
		if (strlen(s) == 0)
		{
			return NULL;
		}

		skill_idx = find_skill_i(s);
		if (skill_idx < 0)
		{
			return NULL;
		}
	}

	return condition_new_skill(min, max, skill_idx);
}

static condition_type *create_condition_ability()
{
	s16b ai;
	cptr s = lua_input_box("Ability name?", 79);
	if (strlen(s) == 0)
	{
		return NULL;
	}

	ai = find_ability(s);
	if (ai < 0)
	{
		return NULL;
	}

	return condition_new_ability(ai);
}

static condition_metadata types_list[TYPES_LIST_SIZE] =
{
	{ M_AND,
	  { "Check is true if all rules within it are true",
	    NULL },
	  condition_new_and,
	},
	{ M_OR,
	  { "Check is true if at least one rule within it is true",
	    NULL },
	  condition_new_or,
	},
	{ M_NOT,
	  { "Invert the result of its child rule",
	    NULL },
	  condition_new_not,
	},
	{ M_NAME,
	  { "Check is true if object name matches name",
	    NULL },
	  create_condition_name,
	},
	{ M_CONTAIN,
	  { "Check is true if object name contains word",
	    NULL },
	  create_condition_contain,
	},
	{ M_INSCRIBED,
	  { "Check is true if object inscription contains word",
	    NULL },
	  create_condition_inscribed,
	},
	{ M_DISCOUNT,
	  { "Check is true if object discount is between two values",
	    NULL },
	  create_condition_discount,
	},
	{ M_SYMBOL,
	  { "Check is true if object symbol is ok",
	    NULL },
	  create_condition_symbol,
	},
	{ M_STATE,
	  { "Check is true if object is identified/unidentified",
	    NULL },
	  create_condition_state,
	},
	{ M_STATUS,
	  { "Check is true if object status is ok",
	    NULL },
	  create_condition_status,
	},
	{ M_TVAL,
	  { "Check is true if object tval(from k_info.txt) is ok",
	    NULL },
	  create_condition_tval,
	},
	{ M_SVAL,
	  { "Check is true if object sval(from k_info.txt) is between",
	    "two values",
	    NULL },
	  create_condition_sval,
	},
	{ M_RACE,
	  { "Check is true if player race is ok",
	    NULL },
	  create_condition_race,
	},
	{ M_SUBRACE,
	  { "Check is true if player subrace is ok",
	    NULL },
	  create_condition_subrace,
	},
	{ M_CLASS,
	  { "Check is true if player class is ok",
	    NULL },
	  create_condition_class,
	},
	{ M_LEVEL,
	  { "Check is true if player level is between 2 values",
	    NULL },
	  create_condition_level,
	},
	{ M_SKILL,
	  { "Check is true if player skill level is between 2 values",
	    NULL },
	  create_condition_skill,
	},
	{ M_ABILITY,
	  { "Check is true if player has the ability",
	    NULL },
	  create_condition_ability,
	},
	{ M_INVENTORY,
	  { "Check is true if something in player's inventory matches",
	    "the contained rule",
	    NULL },
	  condition_new_inventory,
	},
	{ M_EQUIPMENT,
	  { "Check is true if something in player's equipment matches",
	    "the contained rule",
	    NULL },
	  condition_new_equipment,
	},
};

static void display_desc(condition_metadata *condition_metadata)
{
	int i;

	assert(condition_metadata != NULL);

	for (i = 0; condition_metadata->description[i] != NULL; i++)
	{
		c_prt(TERM_WHITE, condition_metadata->description[i], i + 1, 17);
	}
}

/* Create a new rule */
static condition_metadata *automatizer_select_condition_type()
{
	int wid, hgt, max = TYPES_LIST_SIZE, begin = 0, sel = 0, i;
	char c;
	cptr types_names[TYPES_LIST_SIZE];

	/* Create list of names for display */
	for (i = 0; i < TYPES_LIST_SIZE; i++)
	{
		types_names[i] = match_type_to_string(types_list[i].match);
	}

	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		display_list(0, 0, hgt - 1, 15, "Rule types", types_names, max, begin, sel, TERM_L_GREEN);

		display_desc(&types_list[sel]);

		c = inkey();

		if (c == ESCAPE) break;
		else if (c == '8')
		{
			sel--;
			if (sel < 0)
			{
				sel = max - 1;
				begin = max - hgt;
				if (begin < 0) begin = 0;
			}
			if (sel < begin) begin = sel;
		}
		else if (c == '2')
		{
			sel++;
			if (sel >= max)
			{
				sel = 0;
				begin = 0;
			}
			if (sel >= begin + hgt - 1) begin++;
		}
		else if (c == '\r')
		{
			return &types_list[sel];
		}
	}
	return NULL;
}

static void adjust_begin(int *begin, int *sel, int max, int hgt)
{
	if (*sel < 0)
	{
		*sel = max - 1;
		*begin = *sel - hgt + 3;
		if (*begin < 0) *begin = 0;
	}
	if (*sel < *begin) *begin = *sel;

	if (*sel >= max)
	{
		*sel = 0;
		*begin = 0;
	}
	if (*sel >= *begin + hgt - 2) (*begin)++;
}

static int create_new_rule()
{
	action_type action;
	char name[20] = { '\0' };
	char *inscription = NULL;
	int wid = 0, hgt = 0;
	char typ;
	arule_type *rule = NULL;

	Term_get_size(&wid, &hgt);

	sprintf(name, "%s", "No name");
	if (!input_box("Name?", hgt / 2, wid / 2, name, sizeof(name)+1))
	{
		return -1;
	}

	typ = lua_msg_box("[D]estroy, [P]ickup, [I]nscribe?");

	switch (typ)
	{
	case 'd':
	case 'D':
		action = AUTO_DESTROY;
		break;

	case 'p':
	case 'P':
		action = AUTO_PICKUP;
		break;

	case 'i':
	case 'I':
	{
		char *i = NULL;

		action = AUTO_INSCRIBE;

		i = lua_input_box("Inscription?", 79);
		if ((i == NULL) || (strlen(i) == 0))
		{
			return -1;
		}

		inscription = i;

		break;
	}

	default:
		return -1;
	}

	/* Make rule */
	rule = rule_new(name, action, game_module_idx, NULL, inscription);

	/* Append to list of rules */
	return rules_append(rule);
}

static void add_child(condition_type *current)
{
	condition_metadata *metadata = NULL;

	switch (current->match)
	{
	case M_NOT:
	case M_EQUIPMENT:
	case M_INVENTORY:
	{
		if (current->subcondition != NULL)
		{
			return;
		}

		metadata = automatizer_select_condition_type();
		if (metadata == NULL)
		{
			return;
		}

		current->subcondition = metadata->create_condition();
		break;
	}

	case M_AND:
		metadata = automatizer_select_condition_type();
		if (metadata == NULL)
		{
			return;
		}
		condition_and_add(current, metadata->create_condition());
		break;

	case M_OR:
		metadata = automatizer_select_condition_type();
		if (metadata == NULL)
		{
			return;
		}
		condition_or_add(current, metadata->create_condition());
		break;

	default:
		/* No other types of conditions have children */
		break;
	}
}

static int tree_indent = 0;
static int tree_write_out_y = 0;
static int tree_write_out_x = 0;
static int tree_write_out_h = 0;
static int tree_write_out_w = 0;
static int tree_write_y = 0;
static int tree_write_x = 0;
static int tree_write_off_x = 0;
static int tree_write_off_y = 0;

static void tree_write(byte color, cptr line)
{
	cptr p = line;

	for (p = line; *p != '\0'; p++)
	{
		char c = *p;
		int x = tree_write_x - tree_write_off_x + 3*tree_indent;
		int y = tree_write_y - tree_write_off_y;

		if (c != '\n')
		{
			if ((y >= 0) &&
			    (y < tree_write_out_h) &&
			    (x >= 0) &&
			    (x < tree_write_out_w))
			{
				Term_putch(x + tree_write_out_x,
					   y + tree_write_out_y,
					   color,
					   c);
			}

			tree_write_x += 1;
		}
		else
		{
			tree_write_x = 0;
			tree_write_y += 1;
		}
	}
}

static void display_condition(condition_type *condition)
{
	byte bcol = TERM_L_GREEN;
	byte ecol = TERM_GREEN;
	int i;

	assert(condition != NULL);

	/* If this condition is present in the cursor stack,
	   then we use the "active" colors. */
	for (i = 0; i < cursor_count; i++)
	{
		if (cursor_stack[i] == condition)
		{
			bcol = TERM_VIOLET;
			ecol = TERM_VIOLET;
			break;
		}
	}

	tree_indent++;

	switch (condition->match)
	{
	case M_INVENTORY:
	case M_EQUIPMENT:
	{
		cptr where_s = (condition->match == M_INVENTORY)
			? "inventory"
			: "equipment";

		tree_write(ecol, "Something in your ");
		tree_write(bcol, where_s);
		tree_write(ecol, " matches the following:");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_SKILL:
	{
		cptr skill_name =
			s_info[condition->skill_range.skill_idx].name + s_name;

		tree_write(ecol, "Your skill in ");
		tree_write(bcol, skill_name);
		tree_write(ecol, " is from ");
		tree_write(TERM_WHITE, format("%d", (int) condition->skill_range.min));
		tree_write(ecol, " to ");
		tree_write(TERM_WHITE, format("%d", (int) condition->skill_range.max));
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_ABILITY:
	{
		cptr ability_name =
			ab_info[condition->ability].name + ab_name;

		tree_write(ecol, "You have the ");
		tree_write(bcol, ability_name);
		tree_write(ecol, " ability");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_LEVEL:
	{
		tree_write(ecol, "Your ");
		tree_write(bcol, "level");
 		tree_write(ecol, " is from ");

		tree_write(TERM_WHITE, format("%d", condition->level_range.min));
		tree_write(ecol, " to ");
		tree_write(TERM_WHITE, format("%d", condition->level_range.max));
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_SVAL:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "sval");
		tree_write(ecol, " is from ");
		tree_write(TERM_WHITE, format("%d", condition->sval_range.min));
		tree_write(ecol, " to ");
		tree_write(TERM_WHITE, format("%d", condition->sval_range.max));
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_DISCOUNT:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "discount");
		tree_write(ecol, " is from ");
		tree_write(TERM_WHITE, format("%d", condition->discount.min));
		tree_write(ecol, " to ");
		tree_write(TERM_WHITE, format("%d", condition->discount.max));
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_AND:
	{
		struct sglib_condition_list_iterator it;
		struct condition_list *child = NULL;

		tree_write(ecol, "All of the following are true:");
		tree_write(TERM_WHITE, "\n");

		for (child = sglib_condition_list_it_init(&it, condition->conditions.c);
		     child != NULL;
		     child = sglib_condition_list_it_next(&it))
		{
			display_condition(child->condition);
		}

		break;
	}

	case M_OR:
	{
		struct sglib_condition_list_iterator it;
		struct condition_list *child = NULL;

		tree_write(ecol, "At least one of the following are true:");
		tree_write(TERM_WHITE, "\n");

		for (child = sglib_condition_list_it_init(&it, condition->conditions.c);
		     child != NULL;
		     child = sglib_condition_list_it_next(&it))
		{
			display_condition(child->condition);
		}

		break;
	}

	case M_NOT:
	{
		tree_write(ecol, "Negate the following:");
		tree_write(TERM_WHITE, "\n");
		display_condition(condition->subcondition);
		break;
	}

	case M_NAME:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "name");
		tree_write(ecol, " is \"");
		tree_write(TERM_WHITE, condition->name);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_CONTAIN:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "name");
		tree_write(ecol, " contains \"");
		tree_write(TERM_WHITE, condition->name);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_INSCRIBED:
	{
		tree_write(ecol, "It is ");
		tree_write(bcol, "inscribed");
		tree_write(ecol, " with ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, condition->inscription);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_SYMBOL:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "symbol");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, format("%c", condition->symbol));
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_STATE:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "state");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, identification_state_to_string(condition->identification_state));
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_STATUS:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "status");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, status_to_string(condition->status));
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_TVAL:
	{
		tree_write(ecol, "Its ");
		tree_write(bcol, "tval");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, format("%d", condition->tval));
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_RACE:
	{
		tree_write(ecol, "Player ");
		tree_write(bcol, "race");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, condition->race);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_SUBRACE:
	{
		tree_write(ecol, "Player ");
		tree_write(bcol, "subrace");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, condition->subrace);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case M_CLASS:
	{
		tree_write(ecol, "Player ");
		tree_write(bcol, "class");
		tree_write(ecol, " is ");
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, condition->klass);
		tree_write(ecol, "\"");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	}

	tree_indent--;
}

static void display_rule(arule_type *rule)
{
	cptr action_s;
	int hgt, wid;

	action_s = action_to_string(rule->action);

	Term_get_size(&wid, &hgt);

	tree_write_out_y = 1;
	tree_write_out_x = 16;
	tree_write_out_h = hgt - 4 - 1;
	tree_write_out_w = wid - 1 - 15 - 1;
	tree_write_y = 0;
	tree_write_x = 0;

	switch (rule->action)
	{
	case AUTO_DESTROY:
	case AUTO_PICKUP:
	{
		tree_write(TERM_GREEN, "A rule named \"");
		tree_write(TERM_WHITE, rule->name);
		tree_write(TERM_GREEN, "\" to ");
		tree_write(TERM_L_GREEN, action_s);
		tree_write(TERM_GREEN, " when");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	case AUTO_INSCRIBE:
	{
		tree_write(TERM_GREEN, "A rule named \"");
		tree_write(TERM_WHITE, rule->name);
		tree_write(TERM_GREEN, "\" to ");
		tree_write(TERM_L_GREEN, "inscribe");
		tree_write(TERM_GREEN, " an item with \"");
		tree_write(TERM_WHITE, rule->inscription);
		tree_write(TERM_GREEN, "\" when");
		tree_write(TERM_WHITE, "\n");
		break;
	}

	}

	/* Write out the condition */
	if (rule->condition != NULL)
	{
		display_condition(rule->condition);
	}
}

static void adjust_current(int sel)
{
	if (rules_count == 0)
	{
		cursor_clear();
		return;
	}

	tree_write_off_y = 0;
	tree_write_off_x = 0;

	/* Put the top-level condition into cursor */
	cursor_clear();
	if (rules[sel]->condition != NULL)
	{
		cursor_push(rules[sel]->condition);
	}
}

static void tree_scroll_up()
{
	tree_write_off_y = tree_write_off_y - 1;
}

static void tree_scroll_down()
{
	tree_write_off_y = tree_write_off_y + 1;
}

static void tree_scroll_left()
{
	tree_write_off_x = tree_write_off_x + 1;
}

static void tree_scroll_right()
{
	tree_write_off_x = tree_write_off_x - 1;
}

static void automatizer_save_rules()
{
	char name[30] = { '\0' };
	char buf[1025];
	char ch;
	int hgt, wid;

	Term_get_size(&wid, &hgt);

	sprintf(name, "automat.atm");
	if (!input_box("Save name?", hgt / 2, wid / 2, name, sizeof(name)+1))
	{
		return;
	}

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_USER, name);
		
	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	if (file_exist(buf))
	{
		c_put_str(TERM_WHITE, "File exists, continue?[y/n]",
			  hgt / 2,
			  wid / 2 - 14);
		ch = inkey();
		if ((ch != 'Y') && (ch != 'y'))
		{
			return;
		}
	}
		
	/* Write to file */
	{
		json_t *rules_json = rules_to_json();
		int status = json_dump_file(rules_json, buf, JSON_INDENT(2) | JSON_SORT_KEYS);
		if (status == 0)
		{
			c_put_str(TERM_WHITE, "Saved rules in file        ",
				  hgt / 2,
				  wid / 2 - 14);
		}
		else
		{
			c_put_str(TERM_WHITE, "Saving rules failed!       ",
				  hgt / 2,
				  wid / 2 - 14);
		}

		/* Deallocate JSON */
		json_decref(rules_json);

		/* Wait for keypress */
		inkey();
	}
}

static void rename_rule(arule_type *rule)
{
	char name[16];
	int wid, hgt;

	assert(rule != NULL);

	Term_get_size(&wid, &hgt);

	sprintf(name, "%s", rule->name);
	if (input_box("New name?", hgt / 2, wid / 2, name, sizeof(name)-1))
	{
		rule_set_name(rule, name);
	}
}

static void add_new_condition(arule_type *current_rule)
{
	/* Top-level condition? */
	if (current_rule->condition == NULL)
	{
		condition_metadata *metadata = NULL;

		/* Sanity check for navigation stack */
		assert(cursor_count == 0);

		/* Select type of clause */
		metadata = automatizer_select_condition_type();
		if (metadata == NULL)
		{
			return;
		}

		/* Create the condition directly; we can
		   always add a top-level condition so there's
		   no need for the sanity checking in
		   add_child(). */
		current_rule->condition = metadata->create_condition();
		if (current_rule->condition != NULL)
		{
			cursor_push(current_rule->condition);
		}
	}
	else
	{
		condition_type *current_condition = cursor_top();
		add_child(current_condition);
	}
}

static void tree_go_right()
{
	condition_type *top = cursor_top();

	/* Can only go right if the currently selected condition
	   has children. */
	switch (top->match)
	{
	case M_AND:
	case M_OR:
	{
		/* Pick first child */
		struct sglib_condition_list_iterator it;
		condition_list *i = sglib_condition_list_it_init(&it, top->conditions.c);
		/* Move right if possible */
		if (i != NULL)
		{
			cursor_push(i->condition);
		}
		break;
	}

	case M_NOT:
	case M_INVENTORY:
	case M_EQUIPMENT:
	{
		if (top->subcondition != NULL)
		{
			cursor_push(top->subcondition);
		}
		break;
	}

	default:
		/* Not possible to move */
		break;
	}
}

static void tree_go_left()
{
	if (cursor_count > 1)
	{
		cursor_pop();
	}
}

static void tree_go_up()
{
	if (cursor_count > 1)
	{
		condition_type *prev_top = cursor_pop();
		condition_type *top = cursor_top();

		switch (top->match)
		{
		case M_AND:
		case M_OR:
		{
			struct sglib_condition_list_iterator it;
			condition_list *child = NULL;
			condition_list *prev_child = NULL;

			/* We have a list of children */
			for (child = sglib_condition_list_it_init(&it, top->conditions.c);
			     child != NULL;
			     child = sglib_condition_list_it_next(&it))
			{
				if (child->condition == prev_top)
				{
					/* Do we have a previous child? */
					if (prev_child == NULL)
					{
						/* No predecessor; don't move */
						cursor_push(prev_top);
						break;
					}
					else
					{
						cursor_push(prev_child->condition);
						break; /* Done */
					}
				}
				/* Keep track of previous child */
				prev_child = child;
			}

			break;
		}

		default:
		{
			/* No other match types have children; restore
			 original top. */
			cursor_push(prev_top);
			break;
		}

		}
	}
}

static void tree_go_down()
{
	if (cursor_count > 1)
	{
		condition_type *prev_top = cursor_pop();
		condition_type *top = cursor_top();

		switch (top->match)
		{
		case M_AND:
		case M_OR:
		{
			struct sglib_condition_list_iterator it;
			condition_list *child = NULL;

			/* We have a list of children */
			for (child = sglib_condition_list_it_init(&it, top->conditions.c);
			     child != NULL;
			     child = sglib_condition_list_it_next(&it))
			{
				if (child->condition == prev_top)
				{
					/* Move to next child (if any) */
					child = sglib_condition_list_it_next(&it);
					if (child == NULL)
					{
						/* No successor; don't move */
						cursor_push(prev_top);
						break;
					}
					else
					{
						cursor_push(child->condition);
						break; /* Done */
					}
				}
			}

			break;
		}

		default:
		{
			/* No other match types have multiple children; restore
			 original top. */
			cursor_push(prev_top);
			break;
		}

		}
	}
}

static int automatizer_del_self(int sel)
{
	/* If the cursor is at the top level then
	   we want to delete the rule itself */
	if (cursor_count < 1)
	{
		rules_remove(rules[sel]);
		return sel - 1; /* Move selection up */
	}
	else if (cursor_count == 1)
	{
		cursor_pop();
		condition_destroy(&rules[sel]->condition);
		return sel;
	}
	else
	{
		condition_type *prev_top = cursor_pop();
		condition_type *top = cursor_top();

		/* Jump up a level; this is a simple way to ensure a
		   valid cursor. We could be a little cleverer here by
		   trying to move inside the current level, but it's a
		   little complicated. */
		tree_go_left();

		/* Now we can remove the condition from its parent */
		switch (top->match)
		{

		case M_AND:
		case M_OR:
		{
			struct sglib_condition_list_iterator it;
			condition_list *item = NULL;

			/* We have a list of children */
			for (item = sglib_condition_list_it_init(&it, top->conditions.c);
			     item != NULL;
			     item = sglib_condition_list_it_next(&it))
			{
				if (item->condition == prev_top)
				{
					/* Found */
					break;
				}
			}

			/* Must have found item; otherwise internal structure
			   is damaged. */
			assert (item != NULL);
			sglib_condition_list_delete(&top->conditions.c, item);

			/* Destroy the condition */
			condition_destroy(&prev_top);
			break;
		}
		
		case M_NOT:
		case M_EQUIPMENT:
		case M_INVENTORY:
		{
			assert(top->subcondition != NULL);
			condition_destroy(&top->subcondition);
			break;
		}
		
		default:
			/* If we get here, something's wrong with the
			   navigation structures. */
			assert(FALSE);
			break;
		}

		/* Keep selection */
		return sel;
	}
}

#define ACTIVE_LIST     0
#define ACTIVE_RULE     1
void do_cmd_automatizer()
{
	int wid = 0, hgt = 0;
	char c;
	int max, begin = 0, sel = 0;
	int active = ACTIVE_LIST;
	cptr keys;
	cptr keys2;
	cptr keys3;
	cptr rule_names[RULES_MAX];

	Term_get_size(&wid, &hgt);

	if (!automatizer_enabled)
	{
		if (msg_box("Automatizer is currently disabled, enable it? (y/n)", hgt / 2, wid / 2) == 'y')
		{
			automatizer_enabled = TRUE;
		}
		else
			return;
	}

	screen_save();

	adjust_current(sel);

	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		max = rules_count;
		get_rule_names(rule_names);
		display_list(0, 0, hgt - 1, 15, "Rules", rule_names, max, begin, sel, (active == ACTIVE_LIST) ? TERM_L_GREEN : TERM_GREEN);

		draw_box(0, 15, hgt - 4, wid - 1 - 15);
		if (active == ACTIVE_RULE)
		{
			keys = "#Bup#W/#Bdown#W/#Bleft#W/#Bright#W to navitage rule, #B9#W/#B3#W/#B7#W/#B1#W to scroll";
			keys2 = "#Btab#W for switch, #Ba#Wdd clause, #Bd#Welete clause/rule";
			keys3 = "#G?#W for Automatizer help";
		}
		else
		{
			keys = "#Bup#W/#Bdown#W to scroll, #Btab#W to switch to the rule window";
			keys2 = "#Bu#W/#Bd#W to move rules, #Bn#Wew rule, #Br#Wename rule, #Bs#Wave rules";
			keys3 = "#Rk#W to #rdisable#W the automatizer, #G?#W for Automatizer help";
		}
		display_message(17, hgt - 3, strlen(keys), TERM_WHITE, keys);
		display_message(17, hgt - 2, strlen(keys2), TERM_WHITE, keys2);
		display_message(17, hgt - 1, strlen(keys3), TERM_WHITE, keys3);

		if (max)
		{
			display_rule(rules[sel]);
		}

		c = inkey();

		if (c == ESCAPE) break;
		if (active == ACTIVE_LIST)
		{
			if (c == '?')
			{
				screen_save();
				show_file("automat.txt", "Automatizer help", 0, 0);
				screen_load();
			}
			else if (c == '8')
			{
				if (!max) continue;
				sel--;
				adjust_begin(&begin, &sel, max, hgt);
				adjust_current(sel);
			}
			else if (c == '2')
			{
				if (!max) continue;
				sel++;
				adjust_begin(&begin, &sel, max, hgt);
				adjust_current(sel);
			}
			else if (c == 'u')
			{
				if (sel > 0)
				{
					rules_swap(sel-1, sel);
					sel -= 1;

					adjust_begin(&begin, &sel, max, hgt);
					adjust_current(sel);
				}
			}
			else if (c == 'd')
			{
				if (!max) continue;

				if (sel < rules_count - 1)
				{
					rules_swap(sel, sel+1);
					sel += 1;

					adjust_begin(&begin, &sel, max, hgt);
					adjust_current(sel);
				}
			}
			else if (c == 'n')
			{
				int i = create_new_rule();
				if (i >= 0)
				{
					sel = i;
					adjust_current(sel);
					active = ACTIVE_RULE;
				}
			}
			else if (c == 's')
			{
				automatizer_save_rules();
			}
			else if (c == 'r')
			{
				if (!max) continue;

				rename_rule(rules[sel]);
				continue;
			}
			else if (c == 'k')
			{
				automatizer_enabled = FALSE;
				break;
			}
			else if (c == '\t')
			{
				if (!max) continue;
				active = ACTIVE_RULE;
			}
		}
		else if (active == ACTIVE_RULE)
		{
			if (c == '?')
			{
				screen_save();
				show_file("automat.txt", "Automatizer help", 0, 0);
				screen_load();
			}
			else if (c == '8')
			{
				tree_go_up();
			}
			else if (c == '2')
			{
				tree_go_down();
			}
			else if (c == '6')
			{
				tree_go_right();
			}
			else if (c == '4')
			{
				tree_go_left();
			}
			else if (c == '9')
			{
				tree_scroll_up();
			}
			else if (c == '3')
			{
				tree_scroll_down();
			}
			else if (c == '7')
			{
				tree_scroll_left();
			}
			else if (c == '1')
			{
				tree_scroll_right();
			}
			else if (c == 'a')
			{
				add_new_condition(rules[sel]);
			}
			else if (c == 'd')
			{
				if (max)
				{
					int new_sel;

					new_sel = automatizer_del_self(sel);
					if ((sel != new_sel) && (new_sel >= 0))
					{
						sel = new_sel;
						adjust_begin(&begin, &sel, max, hgt);
						adjust_current(sel);
					}
					else if (new_sel == -1)
					{
						active = ACTIVE_LIST;
					}
				}
			}
			else if (c == '\t')
			{
				active = ACTIVE_LIST;
			}
		}
	}

	screen_load();
}

static void easy_add_rule(action_type action, cptr mode, bool_ do_status, object_type *o_ptr)
{
	condition_type *condition = NULL;

	if (streq(mode, "tval"))
	{
		condition = condition_new_tval(o_ptr->tval);
	}
	else if (streq(mode, "tsval"))
	{
		condition_type *sval_condition =
			condition_new_sval(o_ptr->sval, o_ptr->sval);
		condition_type *tval_condition = 
			condition_new_tval(o_ptr->tval);

		condition = condition_new_and();
		condition_and_add(condition, tval_condition);
		condition_and_add(condition, sval_condition);
	}
	else if (streq(mode, "name"))
	{
		char buf[128];
		object_desc(buf, o_ptr, -1, 0);
		strlower(buf);

		condition = condition_new_name(buf);
	}

	/* Use object status? */
	if (do_status == TRUE)
	{
		status_type status = object_status(o_ptr);
		condition_type *status_condition =
			condition_new_status(status);
		condition_type *and_condition =
			condition_new_and();

		condition_and_add(and_condition, condition);
		condition_and_add(and_condition, status_condition);
		/* Replace condition */
		condition = and_condition;
	}

	/* Build rule */
	{
		static arule_type *rule = NULL;
		/* Make rule */
		rule = rule_new(action_to_string(action),
				action,
				game_module_idx,
				condition,
				NULL);

		/* Append to list of rules */
		rules_append(rule);
	}

	msg_print("Rule added. Please go to the Automatizer screen (press = then T)");
	msg_print("to save the modified ruleset.");
}

/* Add a new rule in an easy way */
bool_ automatizer_create = FALSE;
void automatizer_add_rule(object_type *o_ptr, bool_ destroy)
{
	char ch;
	bool_ do_status = FALSE;
	action_type action = AUTO_DESTROY;

	if (!destroy)
	{
		action = AUTO_PICKUP;
	}

	while (TRUE)
	{
		if (!get_com(format("%s all of the same [T]ype, [F]amily or [N]ame, also use [S]tatus (%s)? ", (destroy) ? "Destroy" : "Pickup", (do_status) ? "Yes" : "No"), &ch))
		{
			break;
		}

		if (ch == 'S' || ch == 's')
		{
			do_status = !do_status;
			continue;
		}

		if (ch == 'T' || ch == 't')
		{
			easy_add_rule(action, "tsval", do_status, o_ptr);
			break;
		}

		if (ch == 'F' || ch == 'f')
		{
			easy_add_rule(action, "tval", do_status, o_ptr);
			break;
		}

		if (ch == 'N' || ch == 'n')
		{
			easy_add_rule(action, "name", do_status, o_ptr);
			break;
		}
	}
}

static condition_type *parse_condition(json_t *condition_json)
{
	cptr type_s = NULL;
	match_type match;

	if ((condition_json == NULL) || json_is_null(condition_json))
	{
		return NULL;
	}

	if (json_unpack(condition_json,
			"{s:s}",
			"type", &type_s) < 0)
	{
		msg_print("Missing/invalid 'type' in condition");
		return NULL;
	}

	if (!match_type_from_string(type_s, &match))
	{
		msg_format("Invalid 'type' in condition: %s", type_s);
		return NULL;
	}

	switch (match)
	{
	case M_AND:
	case M_OR:
	{
		json_t *conditions_j = json_object_get(condition_json,
						     "conditions");

		if ((conditions_j == NULL) ||
		    (json_is_null(conditions_j)))
		{
			return NULL;
		}
		else if (json_is_array(conditions_j))
		{
			int i;
			json_t *subcondition_j = NULL;
			condition_type *condition = condition_new(match);
			condition_type *subcondition = NULL;

			for (i = 0; i < json_array_size(conditions_j); i++)
			{
				subcondition_j =
					json_array_get(conditions_j, i);
				subcondition =
					parse_condition(subcondition_j);

				if (subcondition != NULL)
				{
					condition_and_add(condition, subcondition);
				}
			}

			return condition;
		}
		else
		{
			msg_print("'conditions' property has invalid type");
			return NULL;
		}

		break;
	}

	case M_NOT:
	case M_INVENTORY:
	case M_EQUIPMENT:
	{
		json_t *condition_j = json_object_get(condition_json,
						      "condition");

		if ((condition_j == NULL) ||
		    (json_is_null(condition_j)))
		{
			return NULL;
		}
		else if (json_is_object(condition_j))
		{
			condition_type *condition =
				condition_new(match);
			condition->subcondition =
				parse_condition(condition_j);
			return condition;
		}
		else
		{
			msg_print("Invlalid 'condition' property");
			return NULL;
		}
	}

	case M_NAME:
	{
		cptr s = NULL;
		if (json_unpack(condition_json,
				"{s:s}",
				"name", &s) < 0)
		{
			msg_print("Missing/invalid 'name' property");
			return NULL;
		}

		return condition_new_name(s);
	}

	case M_CONTAIN:
	{
		cptr s = NULL;
		if (json_unpack(condition_json,
				"{s:s}",
				"contain", &s) < 0)
		{
			msg_print("Missing/invalid 'contain' property");
			return NULL;
		}

		return condition_new_contain(s);
	}

	case M_SYMBOL:
	{
		cptr s = NULL;
		int sl;
		if (json_unpack(condition_json, "{s:s}", "symbol", &s) < 0)
		{
			msg_print("Missing/invalid 'symbol' property");
			return NULL;
		}

		sl = strlen(s);
		if (sl == 0)
		{
			msg_print("Invalid 'symbol' property: Too short");
			return NULL;
		}
		if (sl > 1)
		{
			msg_print("Invalid 'symbol' property: Too long");
			return NULL;
		}

		return condition_new_symbol(s[0]);
	}

        case M_INSCRIBED:
	{
		cptr s = NULL;
		if (json_unpack(condition_json, "{s:s}", "inscription", &s) < 0)
		{
			msg_print("Missing/invalid 'inscription' property");
			return NULL;
		}

		return condition_new_inscribed(s);
	}

	case M_DISCOUNT:
	{
		int min, max;

		if (json_unpack(condition_json, "{s:i,s:i}",
				"min", &min,
				"max", &max) < 0)
		{
			msg_print("Missing/invalid 'min'/'max' properties");
			return NULL;
		}

		return condition_new_discount(min, max);
	}

	case M_TVAL:
	{
		int tval;

		if (json_unpack(condition_json, "{s:i}", "tval", &tval) < 0)
		{
			msg_print("Missing/invalid 'tval' property");
			return NULL;
		}

		return condition_new_tval(tval);
	}

	case M_SVAL:
	{
		int min, max;

		if (json_unpack(condition_json, "{s:i,s:i}",
				"min", &min,
				"max", &max) < 0)
		{
			msg_print("Missing/invalid 'min'/'max' properties");
			return NULL;
		}

		return condition_new_sval(min, max);
	}

	case M_STATUS:
	{
		cptr s;
		status_type status;

		if (json_unpack(condition_json, "{s:s}", "status", &s) < 0)
		{
			msg_print("Missing/invalid 'status' property");
			return NULL;
		}

		if (!status_from_string(s, &status))
		{
			msg_format("Invalid 'status' property: %s", s);
			return NULL;
		}

		return condition_new_status(status);
	}

	case M_STATE:
	{
		cptr s;
		identification_state state;

		if (json_unpack(condition_json, "{s:s}", "state", &s) < 0)
		{
			msg_print("Missing/invalid 'state' property");
			return NULL;
		}

		if (!identification_state_from_string(s, &state))
		{
			msg_format("Invalid 'state' property: %s", s);
			return NULL;
		}

		return condition_new_state(state);
	}

	case M_RACE:
	{
		cptr s;

		if (json_unpack(condition_json, "{s:s}", "race", &s) < 0)
		{
			msg_print("Missing/invalid 'race' property");
			return NULL;
		}

		return condition_new_race(s);
	}

	case M_SUBRACE:
	{
		cptr s;

		if (json_unpack(condition_json, "{s:s}", "subrace", &s) < 0)
		{
			msg_print("Missing/invalid 'subrace' property");
			return NULL;
		}

		return condition_new_subrace(s);
	}

	case M_CLASS:
	{
		cptr s;

		if (json_unpack(condition_json, "{s:s}", "class", &s) < 0)
		{
			msg_print("Missing/invalid 'class' property");
			return NULL;
		}

		return condition_new_class(s);
	}

	case M_LEVEL:
	{
		int min, max;

		if (json_unpack(condition_json, "{s:i,s:i}",
				"min", &min,
				"max", &max) < 0)
		{
			msg_print("Missing/invalid 'min'/'max' properties");
			return NULL;
		}

		return condition_new_level(min, max);
	}

	case M_SKILL:
	{
		cptr s;
		s16b si;
		int min, max;

		if (json_unpack(condition_json, "{s:i,s:i,s:s}",
				"min", &min,
				"max", &max,
				"name", &s) < 0)
		{
			msg_print("Missing/invalid 'min'/'max'/'name' properties");
			return NULL;
		}

		si = find_skill_i(s);
		if (si < 0)
		{
			msg_print("Invalid 'name' property");
			return NULL;
		}

		return condition_new_skill(min, max, si);
	}

	case M_ABILITY:
	{
		cptr a;
		s16b ai;

		if (json_unpack(condition_json, "{s:s}",
				"ability", &a) < 0)
		{
			msg_print("Missing/invalid 'ability' property");
			return NULL;
		}

		ai = find_ability(a);
		if (ai < 0)
		{
			msg_print("Invalid 'ability' property");
			return NULL;
		}

		return condition_new_ability(ai);
	}

	}

	/* Could not parse */
	return NULL;
}

static void parse_rule(json_t *rule_json)
{
	char *rule_name_s = NULL;
	char *rule_action_s = NULL;
	char *rule_module_s = NULL;
	json_t *rule_inscription_j = NULL;
	arule_type *rule = NULL;
	action_type action;
	int module_idx;

	if (!json_is_object(rule_json))
	{
		msg_print("Rule is not an object");
		return;
	}

	/* Retrieve the attributes */
	if (json_unpack(rule_json,
			"{s:s,s:s,s:s}",
			"name", &rule_name_s,
			"action", &rule_action_s,
			"module", &rule_module_s) < 0)
	{
		msg_print("Rule missing required field(s)");
		return;
	}

	/* Get the optional inscription */
	rule_inscription_j = json_object_get(rule_json, "inscription");

	/* Convert attributes */
	if (!action_from_string((cptr) rule_action_s, &action))
	{
		msg_format("Invalid rule action '%s'", rule_action_s);
		return;
	}

	module_idx = find_module((cptr) rule_module_s);
	if (module_idx < 0)
	{
		msg_format("Skipping rule for unrecognized module '%s'",
			   (cptr) rule_module_s);
		return;
	}

	/* Sanity check: Inscription */
	if (action == AUTO_INSCRIBE)
	{
		if (rule_inscription_j == NULL)
		{
			msg_print("Inscription rule missing 'inscription' attribute");
			return;
		}
		if (!json_is_string(rule_inscription_j))
		{
			msg_print("Inscription rule 'inscription' attribute wrong type");
			return;
		}
	}

	/* Create rule */
	rule = rule_new(rule_name_s,
			action,
			module_idx,
			NULL,
			json_string_value(rule_inscription_j));
	rules_append(rule);

	/* Parse the conditions */
	rule->condition = parse_condition(json_object_get(rule_json, "condition"));
}

static void parse_rules(json_t *rules)
{
	int i;

	if (!json_is_array(rules))
	{
		msg_format("Error 'rules' is not an array");
		return;
	}

	for (i = 0; i < json_array_size(rules); i++)
	{
		json_t *rule = json_array_get(rules, i);
		parse_rule(rule);
	}
}

/**
 * Initialize the automatizer. This function may be called multiple
 * times with different file names -- it should NOT clear any
 * automatizer state (including loaded rules).
 */
void automatizer_init(cptr file_path)
{
	json_t *rules_json = NULL;
	json_error_t error;

	/* Does the file exist? */
	if (!file_exist(file_path))
	{
		/* No big deal, we'll just skip */
		goto out;
	}

	/* Parse file */
	rules_json = json_load_file(file_path, 0, &error);
	if (rules_json == NULL)
	{
		msg_format("Error parsing automatizer rules from '%s'.", file_path);
		msg_format("Line %d, Column %d", error.line, error.column);
		msg_print(NULL);
		goto out;
	}

	/* Go through all the found rules */
	parse_rules(rules_json);

out:
	if (rules_json == NULL)
	{
		json_decref(rules_json);
	}
}
