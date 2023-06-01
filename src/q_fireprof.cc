#include "q_fireprof.hpp"

#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "feature_flag.hpp"
#include "feature_type.hpp"
#include "hook_get_in.hpp"
#include "hook_quest_gen_in.hpp"
#include "hooks.hpp"
#include "lua_bind.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"
#include "z-term.hpp"

#include <cassert>
#include <fmt/format.h>

#define cquest (quest[QUEST_FIREPROOF])

/*
 * Per-module "settings"
 */
typedef struct fireproof_settings fireproof_settings;
struct fireproof_settings
{
	byte tval; /* tval of object to use. */
	byte sval; /* sval of object to use. */
	const char *tval_name; /* descriptive name of tval */
	const char *tval_name_plural; /* descriptive name of tval (plural) */
	s32b total_points; /* total number of points awarded */
};

static fireproof_settings const *fireproof_get_settings()
{
	static fireproof_settings fireproof_settings =
		{ TV_SCROLL, SV_SCROLL_FIRE, "scroll", "scrolls", 24 };
	return &fireproof_settings;
}

/* These constants are how many 'points' each type of item will take
 * up. So currently, you can fireproof 3 books, 4 staves or 12
 * scrolls. */
#define FIREPROOF_BOOK_POINTS 4
#define FIREPROOF_STAFF_POINTS 3
#define FIREPROOF_SCROLL_POINTS 1

static s32b get_item_points_remaining()
{
	fireproof_settings const *settings = fireproof_get_settings();
	return settings->total_points - cquest.data[0];
}

static void set_item_points_remaining(s32b v)
{
	fireproof_settings const *settings = fireproof_get_settings();
	cquest.data[0] = settings->total_points - v;
}

static bool item_tester_hook_eligible(object_type const *o_ptr)
{
	fireproof_settings const *settings = fireproof_get_settings();
	/* check it's the 'marked' item */
	return ((o_ptr->tval == settings->tval) &&
	    (o_ptr->sval == settings->sval) &&
	    (o_ptr->pval2 == settings->sval));
}

static object_filter_t const &item_tester_hook_proofable()
{
	using namespace object_filter;
	static auto instance = And(
		// Must be the correct item base type
		Or(
			TVal(TV_BOOK),
			TVal(TV_SCROLL),
			TVal(TV_STAFF)),
		// Must NOT already be fireproof
		Not(
			HasFlags(TR_IGNORE_FIRE)));
	return instance;
}

/*
 * This function makes sure the player has enough 'points' left to fireproof stuff.
 */
static bool fireproof_enough_points(object_type *o_ptr, int *stack)
{
	int item_value;

	/* are the items in a stack? */
	if (o_ptr->number > 1)
	{
		/* how many to fireproof? */
		*stack = get_quantity("How many would you like fireproofed?", o_ptr->number);
	}
	else
	{
		*stack = 1;
	}
	
	/* check for item type and multiply number in the stack by the
	 * amount of points per item of that type */
	switch (o_ptr->tval)
	{
	case TV_BOOK:
		item_value = FIREPROOF_BOOK_POINTS * (*stack);
		break;
	case TV_STAFF:
		item_value = FIREPROOF_STAFF_POINTS * (*stack);
		break;
	case TV_SCROLL:
		item_value = FIREPROOF_SCROLL_POINTS * (*stack);
		break;
	default:
		assert(false);
	}

	/* do we have enough points? */
	if (item_value > get_item_points_remaining())
	{
		msg_print("I do not have enough fireproofing material for that.");
		return false;
	}
	else
	{
		/* if so then subtract those points before we do the fireproofing */
		set_item_points_remaining(get_item_points_remaining() - item_value);
	}

	/* Used all the points? the quest is completely rewarded. */
	if (get_item_points_remaining() == 0)
	{
		cquest.status = QUEST_STATUS_REWARDED;
	}

	return true;
}

static bool fireproof()
{
	int item;
	if (!get_item(&item,
		      "Which item shall I fireproof?",
		      "You have no more items I can fireproof, come back when you have some.",
		      USE_INVEN,
		      item_tester_hook_proofable()))
	{
		return false;
	}

	/* get the object type from the number */
	object_type *obj2 = get_object(item);

	/* check we have enough points (if we 'got' an item) */
	int stack = 0;
	if (!fireproof_enough_points(obj2, &stack))
	{
		return false;
	}

	/* Do the actual fireproofing */
	{
		bool carry_it;
		object_type *obj3;
		object_type obj_forge;
		s32b oldpval, oldpval2, oldpval3;

		/* are we part of the items from a stack? */
		if (obj2->number != stack) {

			/* make a new object to handle */
			object_copy(&obj_forge, obj2);

			/* give it the right number of items */
			obj_forge.number = stack;

			/* adjust for number of items in pack not to be fireproofed */
			obj2->number = obj2->number - stack;
			obj3 = &obj_forge;

			/* we'll need to add this to the inventory after fireproofing */
			carry_it = true;
		}
		else
		{
			/* use the whole stack */
			obj3 = obj2;

			/* we'll be dealing this while it's still in the inventory */
			carry_it = false;
		}

		/* make it fireproof */
		obj3->name2 = 149;

		/* apply it, making sure the pvals don't change with
		 * apply_magic (it would change the type of book!) */
		oldpval = obj3->pval;
		oldpval2 = obj3->pval2;
		oldpval3 = obj3->pval3;
		apply_magic(obj3, -1, false, false, false);
		obj3->pval = oldpval;
		obj3->pval2 = oldpval2;
		obj3->pval3 = oldpval3;

		/* put it in the inventory if it's only part of a stack */
		if (carry_it == true)
		{
			inven_carry(obj3, true);
		}

		/* id and notice it */
		object_aware(obj3);
		object_known(obj3);

		return true;
	}
}


void quest_fireproof_building(bool *paid, bool *recreate)
{
	fireproof_settings const *settings = fireproof_get_settings();
	int num_books, num_staff, num_scroll;

	num_books = get_item_points_remaining() / FIREPROOF_BOOK_POINTS;
	num_staff = get_item_points_remaining() / FIREPROOF_STAFF_POINTS;
	num_scroll = get_item_points_remaining() / FIREPROOF_SCROLL_POINTS;

	/* the quest hasn't been requested already, right? */
	if (cquest.status == QUEST_STATUS_UNTAKEN)
	{
		/* quest has been taken now */
		cquest.status = QUEST_STATUS_TAKEN;

		/* issue instructions */
		msg_format("I need a very special %s for a spell I am"
			   " working on. I am too old to ", settings->tval_name);
		msg_print("fetch it myself. Please bring it back to me. You can find it north of here.");
		msg_print("Be careful with it, it's fragile and might be destroyed easily.");

		*paid = false;
		*recreate = true;
	}

	/* if quest completed (item was retrieved) */
	else if (cquest.status == QUEST_STATUS_COMPLETED)
	{
		char p[512];
		char pni[512];
		int item_idx;
		int ret;

		/* generate prompt strings */
		snprintf(p  , sizeof(p)  , "Which %s?", settings->tval_name);
		snprintf(pni, sizeof(pni), "You have no %s to return", settings->tval_name_plural);

		/* ask for item */
		ret = get_item(&item_idx, p, pni, USE_INVEN, item_tester_hook_eligible);

		/* didn't get the item? */
		if (!ret)
		{
			return;
		}

		/* got the item! */
		else
		{
			int items;

			/* take item */
			inc_stack_size_ex(item_idx, -1, OPTIMIZE, NO_DESCRIBE);

			msg_print(fmt::format("Great! Let me fireproof some of your items in thanks. I can do {} books, ", num_books));
			msg_print(fmt::format("{} staves, or {} scrolls.", num_staff, num_scroll));

			/* how many items to proof? */
			items = get_item_points_remaining();
			
			/* repeat till up to 3 (value defined as constant) books fireproofed */
			while (items > 0)
			{
				ret = fireproof();

				/* don't loop the fireproof if there's nothing to fireproof */
				if (ret == false)
				{
					break;
				}

				/* subtract item points */
				items = get_item_points_remaining();
			}

			/* have they all been done? */
			if (get_item_points_remaining() == 0)
			{
				/* mark quest to make sure no more quests are given */
				cquest.status = QUEST_STATUS_REWARDED;
			}
			else
			{
				/* mark in preparation of anymore books to fireproof */
				cquest.status = QUEST_STATUS_FINISHED;
			}
		}
	}

	/* if the player asks for a quest when they already have it, but haven't failed it, give them some extra instructions */
	else if (cquest.status == QUEST_STATUS_TAKEN)
	{
		msg_format("The %s is in a cave just behind the shop.",
			   settings->tval_name);
	}

	/* ok not all books have been fireproofed... lets do the rest */
	else if (cquest.status == QUEST_STATUS_FINISHED)
	{

		/* how many books still to proof? */
		int items = get_item_points_remaining();

		/* repeat as necessary */
		while (items > 0)
		{
			int ret = fireproof();

			/* don't loop the fireproof if there's nothing to fireproof */
			if (ret == false)
			{
				break;
			}
			else 
			{
				/* have they all been done? */
				if (get_item_points_remaining() == 0)
				{
					cquest.status = QUEST_STATUS_REWARDED;
				}
			}

			/* subtract item points */
			items = get_item_points_remaining();
		}

	}

	/* quest failed or completed, then give no more quests */
	else if ((cquest.status == QUEST_STATUS_FAILED) ||
		 (cquest.status == QUEST_STATUS_REWARDED))
	{
		msg_print("I have no more quests for you");
	}
}

static bool fireproof_get_hook(void *, void *in_, void *)
{
	struct hook_get_in *in = static_cast<struct hook_get_in *>(in_);
	object_type *o_ptr = in->o_ptr;

	/* check that player is in the quest, haven't picked up the
	 * item already, and check that it's the real item and not another one
	 * generated via random object placement */
	if ((p_ptr->inside_quest == QUEST_FIREPROOF) &&
	    (cquest.status != QUEST_STATUS_COMPLETED) &&
	    (o_ptr->pval2 == fireproof_get_settings()->sval))
	{
		/* ok mark the quest 'completed' */
		cquest.status = QUEST_STATUS_COMPLETED;
		cmsg_print(TERM_YELLOW, "Fine! Looks like you've found it.");
	}

	return false;
}

static bool fireproof_stair_hook(void *, void *, void *)
{
	/* only ask this if player about to go up stairs of quest and
	 * hasn't retrieved item */
	if ((p_ptr->inside_quest != QUEST_FIREPROOF) ||
	    (cquest.status == QUEST_STATUS_COMPLETED))
	{
		return false;
	}
	else
	{
		bool ret;

		if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS)
		{
			return false;
		}

		/* flush all pending input */
		flush();

		/* confirm */
		ret = get_check("Really abandon the quest?");

		/* if yes, then */
		if  (ret == true)
		{
			/* fail the quest */
			cquest.status = QUEST_STATUS_FAILED;
			return false;
		}
		else
		{
			/* if no, they stay in the quest */
			return true;
		}
	}
}

std::string quest_fireproof_describe()
{
	fireproof_settings const *settings = fireproof_get_settings();
	int num_books, num_staff, num_scroll;
	int status = cquest.status;

	num_books = get_item_points_remaining() / FIREPROOF_BOOK_POINTS;
	num_staff = get_item_points_remaining() / FIREPROOF_STAFF_POINTS;
	num_scroll = get_item_points_remaining() / FIREPROOF_SCROLL_POINTS;

	fmt::MemoryWriter w;

	if (status == QUEST_STATUS_TAKEN)
	{
		/* Quest taken */
		w.write("#####yAn Old Mages Quest!\n");
		w.write("Retrieve the strange {} for the old mage in Lothlorien.", settings->tval_name);
	}
	else if (status == QUEST_STATUS_COMPLETED)
	{
		/* essence retrieved, not taken to mage */
		w.write("#####yAn Old Mages Quest!\n");
		w.write("You have retrieved the {} for the old mage in Lothlorien.\n", settings->tval_name);
		w.write("Perhaps you should see about a reward.");
	}
	else if ((status == QUEST_STATUS_FINISHED) &&
		 (get_item_points_remaining() > 0))
	{
		/* essence returned, not all books fireproofed */
		w.write("#####yAn Old Mages Quest!\n");
		w.write("You have retrieved the {} for the old "
			"mage in Lothlorien. He will still\n", settings->tval_name);
		w.write("fireproof {} book(s) or {} staff/staves "
			"or {} scroll(s) for you.",
			num_books, num_staff, num_scroll);
	}

	return w.str();
}

static bool fireproof_gen_hook(void *, void *in_, void *)
{
	fireproof_settings const *settings = fireproof_get_settings();
	auto in = static_cast<hook_quest_gen_in *>(in_);

	/* Only if player doing this quest */
	if (p_ptr->inside_quest != QUEST_FIREPROOF)
	{
		return false;
	}

	/* Go ahead */
	{
		/* load the map */
		{
			int x0 = 2;
			int y0 = 2;
			load_map("fireprof.map", &y0, &x0);
		}

		/* no teleport */
		in->dungeon_flags_ref |= DF_NO_TELEPORT;

		/* create quest item */
		{
			object_type forge;
			object_prep(&forge, lookup_kind(settings->tval, settings->sval));

			/* mark item */
			forge.pval2 = settings->sval;
			forge.inscription = "quest";

			/* roll for co-ordinates in top half of map */
			int const y = randint(3) + 2;
			int const x = randint(45) + 2;

			/* drop it */
			drop_near(&forge, -1, y, x);
		}

		return true;
	}
}

void quest_fireproof_init_hook()
{
	/* Only need hooks if the quest is unfinished. */
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) &&
	    (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_GEN_QUEST, fireproof_gen_hook  , "fireproof_gen_hook",   NULL);
		add_hook_new(HOOK_GET      , fireproof_get_hook  , "fireproof_get_hook",   NULL);
		add_hook_new(HOOK_STAIR    , fireproof_stair_hook, "fireproof_stair_hook", NULL);
	}
}
