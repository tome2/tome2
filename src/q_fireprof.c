#undef cquest
#define cquest (quest[QUEST_FIREPROOF])

#define print_hook(fmt,...) do { fprintf(hook_file, fmt, ##__VA_ARGS__); } while (0)

/*
 * Per-module "settings"
 */
typedef struct fireproof_settings fireproof_settings;
struct fireproof_settings
{
	byte tval; /* tval of object to use. */
	cptr tval_name; /* descriptive name of tval */
	cptr tval_name_plural; /* descriptive name of tval (plural) */
	byte sval_max; /* max sval of object to use; sval will be 1<=X<=sval_max. */
	s32b total_points; /* total number of points awarded */
};

static fireproof_settings const *fireproof_get_settings()
{
	static fireproof_settings fireproof_settings_tome =
		{ TV_BATERIE, "essence", "essences", 18, 12 };
	static fireproof_settings fireproof_settings_theme =
		{ TV_RUNE2, "rune", "runes", 5, 24 };

	if (game_module_idx == MODULE_TOME)
	{
		return &fireproof_settings_tome;
	}
	if (game_module_idx == MODULE_THEME)
	{
		return &fireproof_settings_theme;
	}
	/* If we get here we're in trouble. */
	assert(FALSE);
	return NULL;
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

static void fireproof_set_sval(int sval_max)
{
	cquest.data[1] = sval_max;
}

static int fireproof_get_sval()
{
	return cquest.data[1];
}

static bool_ item_tester_hook_eligible(object_type *o_ptr)
{
	/* check it's the 'marked' item */
	if ((o_ptr->tval == fireproof_get_settings()->tval) &&
	    (o_ptr->sval == fireproof_get_sval()) &&
	    (o_ptr->pval2 == fireproof_get_sval()))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static bool_ item_tester_hook_proofable(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* is it a book/staff/scroll, is it already fireproof? */
	if (((o_ptr->tval == TV_BOOK) ||
	     (o_ptr->tval == TV_SCROLL) ||
	     (o_ptr->tval == TV_STAFF))
	    && ((f3 & TR3_IGNORE_FIRE) == 0))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
 * This function makes sure the player has enough 'points' left to fireproof stuff.
 */
static bool_ fireproof_enough_points(object_type *o_ptr, int *stack)
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
	if (o_ptr->tval == TV_BOOK)
	{
		item_value = FIREPROOF_BOOK_POINTS * (*stack);
	}
	else if (o_ptr->tval == TV_STAFF)
	{
		item_value = FIREPROOF_STAFF_POINTS * (*stack);
	}
	else if (o_ptr->tval == TV_SCROLL)
	{
		item_value = FIREPROOF_SCROLL_POINTS * (*stack);
	}

	/* do we have enough points? */
	if (item_value > get_item_points_remaining())
	{
		msg_print("I do not have enough fireproofing material for that.");
		return FALSE;
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

	return TRUE;
}

static bool_ fireproof()
{
	int ret, item, stack = 0;
	object_type *obj2 = NULL;
	bool_ ret2;

	item_tester_hook = item_tester_hook_proofable;
	ret = get_item(&item,
		       "Which item shall I fireproof?",
		       "You have no more items I can fireproof, come back when you have some.",
		       USE_INVEN);

	/* get the object type from the number */
	obj2 = get_object(item);

	/* check we have enough points (if we 'got' an item) */
	if (ret == TRUE) {
		ret2 = fireproof_enough_points(obj2, &stack);
	}

	/* did either routine fail? */
	if ((ret == FALSE) || (ret2 == FALSE))
	{
		return FALSE;
	}
	else
	{
		bool_ carry_it;
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
			carry_it = TRUE;
		}
		else
		{
			/* use the whole stack */
			obj3 = obj2;

			/* we'll be dealing this while it's still in the inventory */
			carry_it = FALSE;
		}

		/* make it fireproof */
		obj3->name2 = 149;

		/* apply it, making sure the pvals don't change with
		 * apply_magic (it would change the type of book!) */
		oldpval = obj3->pval;
		oldpval2 = obj3->pval2;
		oldpval3 = obj3->pval3;
		apply_magic(obj3, -1, FALSE, FALSE, FALSE);
		obj3->pval = oldpval;
		obj3->pval2 = oldpval2;
		obj3->pval3 = oldpval3;

		/* put it in the inventory if it's only part of a stack */
		if (carry_it == TRUE)
		{
			inven_carry(obj3, TRUE);
		}

		/* id and notice it */
		object_known(obj3);
		object_aware(obj3);

		return TRUE;
	}
}


void quest_fireproof_building(bool_ *paid, bool_ *recreate)
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

		*paid = FALSE;
		*recreate = TRUE;
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
		item_tester_hook = item_tester_hook_eligible;
		ret = get_item(&item_idx, p, pni, USE_INVEN);

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

			msg_print(format("Great! Let me fireproof some of your items in thanks. I can do %d books, ", num_books));
			msg_print(format("%d staves, or %d scrolls.", num_staff, num_scroll));

			/* how many items to proof? */
			items = get_item_points_remaining();
			
			/* repeat till up to 3 (value defined as constant) books fireproofed */
			while (items > 0)
			{
				ret = fireproof();

				/* don't loop the fireproof if there's nothing to fireproof */
				if (ret == FALSE)
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
			if (ret == FALSE)
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

static bool_ fireproof_get_hook(char *fmt)
{
	object_type *o_ptr = param_pile[0].o_ptr;
	assert(o_ptr != NULL);

	/* check that player is in the quest, haven't picked up the
	 * item already, and check that it's the real item and not another one
	 * generated via random object placement */
	if ((p_ptr->inside_quest == QUEST_FIREPROOF) &&
	    (cquest.status != QUEST_STATUS_COMPLETED) &&
	    (o_ptr->pval2 == fireproof_get_sval()))
	{
		/* ok mark the quest 'completed' */
		cquest.status = QUEST_STATUS_COMPLETED;
		cmsg_print(TERM_YELLOW, "Fine! Looks like you've found it.");
	}

	return FALSE;
}

static bool_ fireproof_stair_hook(char *fmt)
{
	/* only ask this if player about to go up stairs of quest and
	 * hasn't retrieved item */
	if ((p_ptr->inside_quest != QUEST_FIREPROOF) ||
	    (cquest.status == QUEST_STATUS_COMPLETED))
	{
		return FALSE;
	}
	else
	{
		bool_ ret;

		if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS)
		{
			return FALSE;
		}

		/* flush all pending input */
		flush();

		/* confirm */
		ret = get_check("Really abandon the quest?");

		/* if yes, then */
		if  (ret == TRUE)
		{
			/* fail the quest */
			cquest.status = QUEST_STATUS_FAILED;
			return FALSE;
		}
		else
		{
			/* if no, they stay in the quest */
			return TRUE;
		}
	}
}

bool_ quest_fireproof_describe(FILE *hook_file)
{
	fireproof_settings const *settings = fireproof_get_settings();
	int num_books, num_staff, num_scroll;
	int status = cquest.status;

	num_books = get_item_points_remaining() / FIREPROOF_BOOK_POINTS;
	num_staff = get_item_points_remaining() / FIREPROOF_STAFF_POINTS;
	num_scroll = get_item_points_remaining() / FIREPROOF_SCROLL_POINTS;

	if (status == QUEST_STATUS_TAKEN)
	{
		/* Quest taken */
		print_hook("#####yAn Old Mages Quest!\n");
		print_hook("Retrieve the strange %s for the old mage "
			   "in Lothlorien.\n", settings->tval_name);
		print_hook("\n");
	}
	else if (status == QUEST_STATUS_COMPLETED)
	{
		/* essence retrieved, not taken to mage */
		print_hook("#####yAn Old Mages Quest!\n");
		print_hook("You have retrieved the %s for the old "
			   "mage in Lothlorien. Perhaps you \n", settings->tval_name);
		print_hook("should see about a reward.\n");
		print_hook("\n");
	}
	else if ((status == QUEST_STATUS_FINISHED) &&
		 (get_item_points_remaining() > 0))
	{
		/* essence returned, not all books fireproofed */
		print_hook("#####yAn Old Mages Quest!\n");
		print_hook("You have retrieved the %s for the old "
			   "mage in Lothlorien. He will still \n", settings->tval_name);
		print_hook("fireproof %d book(s) or %d staff/staves "
			   "or %d scroll(s) for you.\n",
			   num_books, num_staff, num_scroll);
		print_hook("\n");
	}

	return TRUE;
}

static bool_ fireproof_gen_hook(char *fmt)
{
	fireproof_settings const *settings = fireproof_get_settings();

	/* Only if player doing this quest */
	if (p_ptr->inside_quest != QUEST_FIREPROOF)
	{
		return FALSE;
	}

	/* Go ahead */
	{
		int traps, trap_y, trap_x;

		/* load the map */
		{
			int x0 = 2;
			int y0 = 2;
			load_map("fireprof.map", &y0, &x0);
		}

		/* no teleport */
		dungeon_flags2 = DF2_NO_TELEPORT;

		/* determine type of item */
		fireproof_set_sval(randint(settings->sval_max));

		/* create essence */
		{
			int x, y;
			object_type forge;

			object_prep(&forge, lookup_kind(settings->tval, fireproof_get_sval()));

			/* mark item */
			forge.pval2 = fireproof_get_sval();
			forge.note = quark_add("quest");

			/* roll for co-ordinates in top half of map */
			y = randint(3) + 2;
			x = randint(45) + 2;

			/* drop it */
			drop_near(&forge, -1, y, x);
		}

		/* how many traps to generate */
		traps = rand_range(10, 30);
					
		/* generate the traps */
		while (traps > 0)
		{
			int tries = 0, trap_level = 0;

			/* make sure it's a safe place */
			while (tries == 0)
			{
				/* get grid coordinates */
				trap_y = randint(19) + 2;
				trap_x = randint(45) + 2;
				cave_type *c_ptr = &cave[trap_y][trap_x];

				/* are the coordinates on a stair, or a wall? */
				if (((f_info[c_ptr->feat].flags1 & FF1_PERMANENT) != 0) ||
				    ((f_info[c_ptr->feat].flags1 & FF1_FLOOR) == 0))
				{
					/* try again */
					tries = 0;
				}
				else
				{
					/* not a stair, then stop this 'while' */
					tries = 1;
				}
			}

			/* randomise level of trap */
			trap_level = rand_range(20, 40);

			/* put the trap there */
			place_trap_leveled(trap_y, trap_x, trap_level);

			/* that's one less trap to place */
			traps = traps - 1;
		}
		
		return TRUE;
	}
}

bool_ quest_fireproof_init_hook(int q)
{
	/* Only need hooks if the quest is unfinished. */
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) &&
	    (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook(HOOK_GEN_QUEST, fireproof_gen_hook  , "fireproof_gen_hook");
		add_hook(HOOK_GET      , fireproof_get_hook  , "fireproof_get_hook");
		add_hook(HOOK_STAIR    , fireproof_stair_hook, "fireproof_stair_hook");
	}

	return FALSE;
}

#undef print_hook
