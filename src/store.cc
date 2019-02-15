/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "store.hpp"

#include "bldg.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd3.hpp"
#include "cmd4.hpp"
#include "cmd5.hpp"
#include "files.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "obj_theme.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "owner_type.hpp"
#include "player_type.hpp"
#include "spell_type.hpp"
#include "skills.hpp"
#include "spells5.hpp"
#include "stats.hpp"
#include "store_action_type.hpp"
#include "store_flag.hpp"
#include "store_type.hpp"
#include "store_info_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"

#include <cassert>
#include <fmt/format.h>

#define STORE_GENERAL_STORE "General Store"
#define STORE_ARMOURY "Armoury"
#define STORE_WEAPONSMITH "Weaponsmith"
#define STORE_TEMPLE "Temple"
#define STORE_ALCHEMY "Alchemy shop"
#define STORE_MAGIC "Magic shop"
#define STORE_BLACK_MARKET "Black Market"
#define STORE_BOOKS "Book Store"
#define STORE_PETS "Pet Shop"
#define STORE_HUNTING_SUPPLIES "Hunting Supply Store"
#define STORE_CONSTRUCTION_SUPPLIES "Construction Supply Store"
#define STORE_MUSIC "Music Store"

#define RUMOR_CHANCE 8

#define MAX_COMMENT_1	6

static const char *comment_1[MAX_COMMENT_1] =
{
	"Okay.",
	"Fine.",
	"Accepted!",
	"Agreed!",
	"Done!",
	"Taken!"
};

#define MAX_COMMENT_4A	4

static const char *comment_4a[MAX_COMMENT_4A] =
{
	"Enough!  You have abused me once too often!",
	"Arghhh!  I have had enough abuse for one day!",
	"That does it!  You shall waste my time no more!",
	"This is getting nowhere!  I'm going to Londis!"
};

#define MAX_COMMENT_4B	4

static const char *comment_4b[MAX_COMMENT_4B] =
{
	"Leave my store!",
	"Get out of my sight!",
	"Begone, you scoundrel!",
	"Out, out, out!"
};


/*
 * Successful haggle.
 */
static void say_comment_1()
{
	char rumour[80];

	msg_print(comment_1[rand_int(MAX_COMMENT_1)]);

	if (randint(RUMOR_CHANCE) == 1)
	{
		msg_print("The shopkeeper whispers something into your ear:");
		get_rnd_line("rumors.txt", rumour);
		msg_print(rumour);
	}
}


/*
 * Kick 'da bum out.					-RAK-
 */
static void say_comment_4()
{
	msg_print(comment_4a[rand_int(MAX_COMMENT_4A)]);
	msg_print(comment_4b[rand_int(MAX_COMMENT_4B)]);
}



/*
 * Messages for reacting to purchase prices.
 */

#define MAX_COMMENT_7A	4

static const char *comment_7a[MAX_COMMENT_7A] =
{
	"Arrgghh!",
	"You moron!",
	"You hear someone sobbing...",
	"The shopkeeper howls in agony!"
};

#define MAX_COMMENT_7B	4

static const char *comment_7b[MAX_COMMENT_7B] =
{
	"Darn!",
	"You fiend!",
	"The shopkeeper yells at you.",
	"The shopkeeper glares at you."
};

#define MAX_COMMENT_7C	4

static const char *comment_7c[MAX_COMMENT_7C] =
{
	"Cool!",
	"You've made my day!",
	"The shopkeeper giggles.",
	"The shopkeeper laughs loudly."
};

#define MAX_COMMENT_7D	4

static const char *comment_7d[MAX_COMMENT_7D] =
{
	"Yippee!",
	"I think I'll retire!",
	"The shopkeeper jumps for joy.",
	"The shopkeeper smiles gleefully."
};

/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
	/* Item was worthless, but we bought it */
	if ((value <= 0) && (price > value))
	{
		/* Comment */
		msg_print(comment_7a[rand_int(MAX_COMMENT_7A)]);
	}

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
	{
		/* Comment */
		msg_print(comment_7b[rand_int(MAX_COMMENT_7B)]);
	}

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
	{
		/* Comment */
		msg_print(comment_7c[rand_int(MAX_COMMENT_7C)]);
	}

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
	{
		/* Comment */
		msg_print(comment_7d[rand_int(MAX_COMMENT_7D)]);
	}
}





/*
 * We store the current "store number" here so everyone can access it
 */
static int cur_store_num = 7;

/*
 * We store the current "store page" here so everyone can access it
 */
static int store_top = 0;

/*
 * We store the current "store pointer" here so everyone can access it
 */
static store_type *st_ptr = NULL;

/*
 * We store the current "owner type" here so everyone can access it
 */
static owner_type const *ot_ptr = NULL;



/*
 * Determine the price of an item (qty one) in a store.
 *
 * This function takes into account the player's charisma, and the
 * shop-keepers friendliness, and the shop-keeper's base greed, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * item, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 *
 * Charisma adjustment runs from 80 to 130
 * Racial adjustment runs from 95 to 130
 *
 * Since greed/charisma/racial adjustments are centered at 100, we need
 * to adjust (by 200) to extract a usable multiplier.  Note that the
 * "greed" value is always something (?).
 */
static s32b price_item(object_type *o_ptr, int greed, bool flip)
{
	auto const &st_info = game->edit_data.st_info;

	int factor;
	int adjust;
	s32b price;


	/* Get the value of one of the items */
	price = object_value(o_ptr);

	/* Worthless items */
	if (price <= 0) return (0L);

	/* Compute the racial factor */
	if (is_state(st_ptr, STORE_LIKED))
	{
		factor = ot_ptr->costs[STORE_LIKED];
	}
	else if (is_state(st_ptr, STORE_HATED))
	{
		factor = ot_ptr->costs[STORE_HATED];
	}
	else
	{
		factor = ot_ptr->costs[STORE_NORMAL];
	}

	/* Add in the charisma factor */
	factor += adj_chr_gold[p_ptr->stat_ind[A_CHR]];

	/* Shop is buying */
	if (flip)
	{
		/* Mega Hack^3 */
		switch (o_ptr->tval)
		{
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
			price /= 5;
			break;
		}

		/* Adjust for greed */
		adjust = 100 + (300 - (greed + factor));

		/* Never get "silly" */
		if (adjust > 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (st_info[st_ptr->st_idx].flags & STF_ALL_ITEM) price = price / 2;
		
		/* No selling means you get no money */
		if (options->no_selling) price = 0;
	}

	/* Shop is selling */
	else
	{
		/* Adjust for greed */
		adjust = 100 + ((greed + factor) - 300);

		/* Never get "silly" */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (st_info[st_ptr->st_idx].flags & STF_ALL_ITEM) price = price * 2;
		
		/* Never give items away for free */
		if (price <= 0L) price = 1L;
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Return the price */
	return (price);
}


/*
 * Special "mass production" computation
 */
static int mass_roll(int num, int max)
{
	int i, t = 0;
	for (i = 0; i < num; i++) t += rand_int(max);
	return (t);
}


/*
 * Certain "cheap" objects should be created in "piles"
 * Some objects can be sold at a "discount" (in small piles)
 */
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	int discount = 0;

	s32b cost = object_value(o_ptr);


	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lites */
	case TV_FOOD:
	case TV_FLASK:
	case TV_LITE:
		{
			if (cost <= 5L) size += mass_roll(3, 5);
			if (cost <= 20L) size += mass_roll(3, 5);
			break;
		}

	case TV_POTION:
	case TV_POTION2:
	case TV_SCROLL:
		{
			if (cost <= 60L) size += mass_roll(3, 5);
			if (cost <= 240L) size += mass_roll(1, 5);
			break;
		}

	case TV_SYMBIOTIC_BOOK:
	case TV_MUSIC_BOOK:
	case TV_DRUID_BOOK:
	case TV_DAEMON_BOOK:
	case TV_BOOK:
		{
			if (cost <= 50L) size += mass_roll(2, 3);
			if (cost <= 500L) size += mass_roll(1, 3);
			break;
		}

	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SHIELD:
	case TV_GLOVES:
	case TV_BOOTS:
	case TV_CLOAK:
	case TV_HELM:
	case TV_CROWN:
	case TV_SWORD:
	case TV_AXE:
	case TV_POLEARM:
	case TV_HAFTED:
	case TV_DIGGING:
	case TV_BOW:
		{
			if (o_ptr->name2) break;
			if (cost <= 10L) size += mass_roll(3, 5);
			if (cost <= 100L) size += mass_roll(3, 5);
			break;
		}

	case TV_SPIKE:
	case TV_SHOT:
	case TV_ARROW:
	case TV_BOLT:
		{
			if (cost <= 5L) size += mass_roll(5, 5);
			if (cost <= 50L) size += mass_roll(5, 5);
			if (cost <= 500L) size += mass_roll(5, 5);
			break;
		}

		/* Because many rods (and a few wands and staffs) are useful mainly
		 * in quantity, the Black Market will occasionally have a bunch of
		 * one kind. -LM- */
	case TV_ROD:
	case TV_WAND:
	case TV_STAFF:
		{
			if (cost < 1601L) size += mass_roll(1, 5);
			else if (cost < 3201L) size += mass_roll(1, 3);
			break;
		}
	}


	/* Pick a discount */
	if (cost < 5)
	{
		discount = 0;
	}
	else if (rand_int(25) == 0)
	{
		discount = 25;
	}
	else if (rand_int(150) == 0)
	{
		discount = 50;
	}
	else if (rand_int(300) == 0)
	{
		discount = 75;
	}
	else if (rand_int(500) == 0)
	{
		discount = 90;
	}


	if (!o_ptr->artifact_name.empty())
	{
		if (options->cheat_peek && discount)
		{
			msg_print("No discount on random artifacts.");
		}
		discount = 0;
	}

	/* Save the discount */
	o_ptr->discount = discount;

	/* Save the total pile size */
	o_ptr->number = size - (size * discount / 100);
}








/*
 * Determine if a store item can "absorb" another item
 *
 * See "object_similar()" for the same function for the "player"
 */
static bool store_object_similar(object_type const *o_ptr, object_type *j_ptr)
{
	/* Hack -- Identical items cannot be stacked */
	if (o_ptr == j_ptr) return (false);

	/* Different objects cannot be stacked */
	if (o_ptr->k_ptr != j_ptr->k_ptr) return (false);

	/* Different charges (etc) cannot be stacked, unless wands or rods. */
	if ((o_ptr->pval != j_ptr->pval) && (o_ptr->tval != TV_WAND)) return (false);

	/* Require many identical values */
	if (o_ptr->pval2 != j_ptr->pval2) return (false);
	if (o_ptr->pval3 != j_ptr->pval3) return (false);

	/* Require many identical values */
	if (o_ptr->to_h != j_ptr->to_h) return (false);
	if (o_ptr->to_d != j_ptr->to_d) return (false);
	if (o_ptr->to_a != j_ptr->to_a) return (false);

	/* Require identical "artifact" names */
	if (o_ptr->name1 != j_ptr->name1) return (false);

	/* Require identical "ego-item" names */
	if (o_ptr->name2 != j_ptr->name2) return (false);

	/* Require identical "ego-item" names */
	if (o_ptr->name2b != j_ptr->name2b) return (false);

	/* Random artifacts don't stack !*/
	if (!o_ptr->artifact_name.empty()) return false;
	if (!j_ptr->artifact_name.empty()) return false;

	/* Hack -- Identical art_flags! */
	if (o_ptr->art_flags != j_ptr->art_flags)
		return (false);

	/* Hack -- Never stack "powerful" items */
	if (o_ptr->xtra1 || j_ptr->xtra1) return (false);

	if (o_ptr->tval == TV_LITE)
	{
		/* Require identical "turns of light" */
		if (o_ptr->timeout != j_ptr->timeout) return (false);
	}
	else
	{
		/* Hack -- Never stack recharging items */
		if (o_ptr->timeout || j_ptr->timeout) return (false);
	}

	/* Require many identical values */
	if (o_ptr->ac	!= j_ptr->ac) return (false);
	if (o_ptr->dd	!= j_ptr->dd) return (false);
	if (o_ptr->ds	!= j_ptr->ds) return (false);

	/* Require matching discounts */
	if (o_ptr->discount != j_ptr->discount) return (false);

	/* They match, so they must be similar */
	return true;
}


/*
 * Allow a store item to absorb another item
 */
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > 99) ? 99 : total;

	/* Hack -- if wands are stacking, combine the charges. -LM- */
	if (o_ptr->tval == TV_WAND)
	{
		o_ptr->pval += j_ptr->pval;
	}
}


/*
 * Check to see if the shop will be carrying too many objects	-RAK-
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.	Before, one could "nuke" potions this way.
 */
static bool store_check_num(object_type *o_ptr)
{
	auto const &st_info = game->edit_data.st_info;

	/* Free space is always usable */
	if (st_ptr->stock.size() < st_ptr->stock_size)
	{
		return true;
	}

	/* The "home" acts like the player */
	if ((cur_store_num == 7) || (st_info[st_ptr->st_idx].flags & STF_MUSEUM))
	{
		/* Check all the items */
		for (auto const &o_ref: st_ptr->stock)
		{
			/* Can the new object be combined with the old one? */
			if (object_similar(&o_ref, o_ptr))
			{
				return true;
			}
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the items */
		for (auto const &o_ref: st_ptr->stock)
		{
			/* Can the new object be combined with the old one? */
			if (store_object_similar(&o_ref, o_ptr))
			{
				return true;
			}
		}
	}

	/* But there was no room at the inn... */
	return false;
}


static bool is_blessed(object_type const *o_ptr)
{
	auto flags = object_flags_known(o_ptr);
	return bool(flags & TR_BLESSED);
}



/*
 * Determine if the current store will purchase the given item
 *
 * Note that a shop-keeper must refuse to buy "worthless" items
 */
static bool store_will_buy(object_type const *o_ptr)
{
	auto const &st_info = game->edit_data.st_info;

	auto const &store_name = st_info[st_ptr->st_idx].name;

	/* Hack -- The Home is simple */
	if (cur_store_num == 7)
	{
		return true;
	}

	if (st_info[st_ptr->st_idx].flags & STF_MUSEUM)
	{
		return true;
	}

	/* XXX XXX XXX Ignore "worthless" items */
	if (object_value(o_ptr) <= 0)
	{
		return false;
	}

	/* What do stores buy? */
	if ((store_name == STORE_GENERAL_STORE))
	{
		switch (o_ptr->tval)
		{
		case TV_CORPSE:
		case TV_FOOD:
		case TV_LITE:
		case TV_FLASK:
		case TV_SPIKE:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_DIGGING:
		case TV_CLOAK:
		case TV_BOTTLE:
			return true;
		}
	}
	else if ((store_name == STORE_ARMOURY))
	{
		switch (o_ptr->tval)
		{
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			return true;
		}
	}
	else if ((store_name == STORE_WEAPONSMITH))
	{
		switch (o_ptr->tval)
		{
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOOMERANG:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_AXE:
		case TV_MSTAFF:
			return true;
		}
	}
	else if ((store_name == STORE_TEMPLE))
	{
		switch (o_ptr->tval)
		{
		case TV_DRUID_BOOK:
		case TV_SCROLL:
		case TV_POTION2:
		case TV_POTION:
		case TV_HAFTED:
			return true;
		}

		if ((o_ptr->tval == TV_BOOK) &&
		    (o_ptr->sval == BOOK_RANDOM) &&
		    (spell_type_random_type(spell_at(o_ptr->pval)) == SKILL_SPIRITUALITY))
		{
			return true;
		}
		else if ((o_ptr->tval == TV_POLEARM) &&
			 is_blessed(o_ptr))
		{
			return true;
		}
		else if ((o_ptr->tval == TV_SWORD) &&
			 is_blessed(o_ptr))
		{
			return true;
		}
		else if ((o_ptr->tval == TV_AXE) &&
			 is_blessed(o_ptr))
		{
			return true;
		}
		else if ((o_ptr->tval == TV_BOOMERANG) &&
			 is_blessed(o_ptr))
		{
			return true;
		}
	}
	else if ((store_name == STORE_ALCHEMY))
	{
		switch (o_ptr->tval)
		{
		case TV_SCROLL:
		case TV_POTION2:
		case TV_POTION:
		case TV_BOTTLE:
			return true;
		}
	}
	else if ((store_name == STORE_MAGIC))
	{
		switch (o_ptr->tval)
		{
		case TV_SYMBIOTIC_BOOK:
		case TV_AMULET:
		case TV_RING:
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
		case TV_ROD_MAIN:
		case TV_SCROLL:
		case TV_POTION2:
		case TV_POTION:
		case TV_MSTAFF:
		case TV_RANDART:
			return true;
		}

		if ((o_ptr->tval == TV_BOOK) &&
		    (o_ptr->sval == BOOK_RANDOM) &&
		    (spell_type_random_type(spell_at(o_ptr->pval)) == SKILL_MAGIC))
		{
			return true;
		}
		else if ((o_ptr->tval == TV_BOOK) &&
			 (o_ptr->sval != BOOK_RANDOM))
		{
			return true;
		}
	}
	else if ((store_name == STORE_BLACK_MARKET))
	{
		return true;
	}
	else if ((store_name == STORE_BOOKS))
	{
		switch (o_ptr->tval)
		{
		case TV_BOOK:
		case TV_SYMBIOTIC_BOOK:
		case TV_MUSIC_BOOK:
		case TV_DAEMON_BOOK:
		case TV_DRUID_BOOK:
			return true;
		}
	}
	else if ((store_name == STORE_PETS))
	{
		return (o_ptr->tval == TV_EGG);
	}
	else if ((store_name == STORE_HUNTING_SUPPLIES))
	{
		switch (o_ptr->tval)
		{
		case TV_BOOMERANG:
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_POTION2:
			return true;
		}
	}
	else if ((store_name == STORE_CONSTRUCTION_SUPPLIES))
	{
		switch (o_ptr->tval)
		{
		case TV_LITE:
		case TV_DIGGING:
			return true;
		}
	}
	else if ((store_name == STORE_MUSIC))
	{
		return (o_ptr->tval == TV_INSTRUMENT);
	}

	/* Assume not okay */
	return false;
}



/*
 * Add the item "o_ptr" to the inventory of the "Home"
 *
 * In all cases, return the slot (or -1) where the object was placed
 *
 * Note that this is a hacked up version of "inven_carry()".
 *
 * Also note that it may not correctly "adapt" to "knowledge" bacoming
 * known, the player may have to pick stuff up and drop it again.
 */
static int home_carry(object_type *o_ptr)
{
	std::size_t slot;

	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock.size(); slot++)
	{
		/* Get the existing item */
		auto j_ptr = &st_ptr->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return slot;
		}
	}

	/* No space? */
	if (st_ptr->stock.size() >= st_ptr->stock_size)
	{
		return -1;
	}

	/* Determine the "value" of the item */
	auto const value = object_value(o_ptr);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock.size(); slot++)
	{
		/* Get that item */
		auto j_ptr = &st_ptr->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Can happen in the home */
		if (!object_aware_p(o_ptr)) continue;
		if (!object_aware_p(j_ptr)) break;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Objects in the home can be unknown */
		if (!object_known_p(o_ptr)) continue;
		if (!object_known_p(j_ptr)) break;


		/*
		 * Hack:  otherwise identical rods sort by
		 * increasing recharge time --dsb
		 */
		if (o_ptr->tval == TV_ROD_MAIN)
		{
			if (o_ptr->timeout < j_ptr->timeout) break;
			if (o_ptr->timeout > j_ptr->timeout) continue;
		}

		/* Objects sort by decreasing value */
		auto const j_value = object_value(j_ptr);
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Insert */
	st_ptr->stock.insert(st_ptr->stock.begin() + slot, *o_ptr);

	/* Return the location */
	return slot;
}


/*
 * Add the item "o_ptr" to a real stores inventory.
 *
 * If the item is "worthless", it is thrown away (except in the home).
 *
 * If the item cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.	Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an item to a "fixed" price stack will not change the fixed price.
 *
 * In all cases, return the slot (or -1) where the object was placed
 */
static int store_carry(object_type *o_ptr)
{
	std::size_t slot;

	/* Evaluate the object */
	auto const value = object_value(o_ptr);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0)
	{
		return -1;
	}

	/* Erase the inscription */
	o_ptr->inscription.clear();

	/* Check each existing item (try to combine) */
	for (slot = 0; slot < st_ptr->stock.size(); slot++)
	{
		/* Get the existing item */
		auto j_ptr = &st_ptr->stock[slot];

		/* Can the existing items be incremented? */
		if (store_object_similar(j_ptr, o_ptr))
		{
			/* Hack -- extra items disappear */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return slot;
		}
	}

	/* No space? */
	if (st_ptr->stock.size() >= st_ptr->stock_size)
	{
		return -1;
	}


	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock.size(); slot++)
	{
		/* Get that item */
		auto j_ptr = &st_ptr->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;


		/*
		 * Hack:  otherwise identical rods sort by
		 * increasing recharge time --dsb
		 */
		if (o_ptr->tval == TV_ROD_MAIN)
		{
			if (o_ptr->timeout < j_ptr->timeout) break;
			if (o_ptr->timeout > j_ptr->timeout) continue;
		}

		/* Evaluate that slot */
		auto const j_value = object_value(j_ptr);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Insert the new item */
	st_ptr->stock.insert(st_ptr->stock.begin() + slot, *o_ptr);

	/* Return the location */
	return slot;
}


/*
 * Increase, by a given amount, the number of a certain item
 * in a certain store.	This can result in zero items.
 */
static void store_item_increase(int item, int num)
{
	/* Get the item */
	auto o_ptr = &st_ptr->stock[item];

	/* Verify the number */
	int cnt = o_ptr->number + num;
	if (cnt > 255) cnt = 255;
	else if (cnt < 0) cnt = 0;
	num = cnt - o_ptr->number;

	/* Save the new number */
	o_ptr->number += num;
}


/*
 * Remove a slot if it is empty
 */
static void store_item_optimize(int item)
{
	/* Get the item */
	auto const o_ptr = &st_ptr->stock[item];

	/* Must exist */
	if (!o_ptr->k_ptr) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* Wipe the item */
	object_wipe(&st_ptr->stock[item]);

	/* Erase the item */
	st_ptr->stock.erase(st_ptr->stock.begin() + item);
}


/*
 * This function will keep 'crap' out of the black market.
 * Crap is defined as any item that is "available" elsewhere
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
 */
static bool black_market_crap(object_type *o_ptr)
{
	auto const &st_info = game->edit_data.st_info;

	/* Ego items are never crap */
	if (o_ptr->name2) return false;

	/* Good items are never crap */
	if (o_ptr->to_a > 0) return false;
	if (o_ptr->to_h > 0) return false;
	if (o_ptr->to_d > 0) return false;

	/* Check all stores */
	for (std::size_t i = 0; i < st_info.size(); i++)
	{
		if (i == STORE_HOME) continue;
		if (st_info[i].flags & STF_MUSEUM) continue;

		/* Check every item in the store */
		for (auto const &stock_obj: town_info[p_ptr->town_num].store[i].stock)
		{
			/* Duplicate item "type", assume crappy */
			if (o_ptr->k_ptr == stock_obj.k_ptr)
			{
				return true;
			}
		}
	}

	/* Assume okay */
	return false;
}


/*
 * Attempt to delete (some of) a random item from the store
 * Hack -- we attempt to "maintain" piles of items when possible.
 */
static void store_delete()
{
	/* Pick a random slot */
	int const what = rand_int(st_ptr->stock.size());

	/* Determine how many items are here */
	int num = st_ptr->stock[what].number;

	/* Hack -- sometimes, only destroy half the items */
	if (rand_int(100) < 50) num = (num + 1) / 2;

	/* Hack -- sometimes, only destroy a single item */
	if (rand_int(100) < 50) num = 1;

	/* Hack -- decrement the maximum timeouts and total charges of rods and wands. -LM- */
	if (st_ptr->stock[what].tval == TV_WAND)
	{
		st_ptr->stock[what].pval -= num * st_ptr->stock[what].pval / st_ptr->stock[what].number;
	}

	/* Actually destroy (part of) the item */
	store_item_increase(what, -num);
	store_item_optimize(what);
}

/* Analyze store flags and return a level */
int return_level()
{
	auto const &st_info = game->edit_data.st_info;

	auto sti_ptr = &st_info[st_ptr->st_idx];

	int level;

	if (sti_ptr->flags & STF_RANDOM) level = 0;
	else level = rand_range(1, STORE_OBJ_LEVEL);

	if (sti_ptr->flags & STF_DEPEND_LEVEL) level += dun_level;

	if (sti_ptr->flags & STF_SHALLOW_LEVEL) level += 5 + rand_int(5);
	if (sti_ptr->flags & STF_MEDIUM_LEVEL) level += 25 + rand_int(25);
	if (sti_ptr->flags & STF_DEEP_LEVEL) level += 45 + rand_int(45);

	if (sti_ptr->flags & STF_ALL_ITEM) level += p_ptr->lev;

	return (level);
}

/* Is it an ok object ? */
static int store_tval = 0, store_level = 0;

/*
 * Hack -- determine if a template is "good"
 */
static bool kind_is_storeok(object_kind const *k_ptr)
{
	if (k_ptr->flags & TR_NORM_ART)
	{
		return false;
	}

	if (k_ptr->flags & TR_INSTA_ART)
	{
		return false;
	}

	if (!kind_is_legal(k_ptr))
	{
		return false;
	}

	if (k_ptr->tval != store_tval)
	{
		return false;
	}

	if (k_ptr->level < (store_level / 2))
	{
		return false;
	}

	return true;
}

namespace { // anonymous

struct is_artifact_p : public boost::static_visitor<bool> {

	bool operator ()(store_item_filter_by_k_idx f) const
	{
		auto const &k_info = game->edit_data.k_info;
		return bool(k_info.at(f.k_idx)->flags & TR_NORM_ART);
	}

	bool operator ()(store_item_filter_by_tval) const
	{
		return false;
	}
};

class choose_k_idx : public boost::static_visitor<int> {

	int m_level;

public:

	explicit choose_k_idx(int level)
		: m_level(level)
	{
	}

	int operator ()(store_item_filter_by_k_idx f) const
	{
		return f.k_idx;
	}

	int operator ()(store_item_filter_by_tval f) const
	{
		auto const &st_info = game->edit_data.st_info;
		auto &alloc = game->alloc;

		/* No themes */
		init_match_theme(obj_theme::no_theme());

		/* Activate restriction */
		get_object_hook = kind_is_storeok;
		store_tval = f.tval;

		/* Do we forbid too shallow items ? */
		if (st_info[st_ptr->st_idx].flags & STF_FORCE_LEVEL)
		{
			store_level = m_level;
		}
		else
		{
			store_level = 0;
		}

		/* Prepare allocation table */
		get_obj_num_prep();

		/* Get it! */
		auto k_idx = get_obj_num(m_level);

		/* Invalidate the cached allocation table */
		alloc.kind_table_valid = false;

		return k_idx;
	}
};

} // namespace (anonymous)

/*
 * Creates a random item and gives it to a store
 * This algorithm needs to be rethought.  A lot.
 *
 * Note -- the "level" given to "obj_get_num()" is a "favored"
 * level, that is, there is a much higher chance of getting
 * items with a level approaching that of the given level...
 *
 * Should we check for "permission" to have the given item?
 */
static void store_create()
{
	auto const &st_info = game->edit_data.st_info;
	auto const &k_info = game->edit_data.k_info;
	auto &alloc = game->alloc;

	int k_idx = -1;
	int level = 0;

	object_type forge;
	object_type *q_ptr = NULL;
	bool obj_all_done = false;

	/* Paranoia -- no room left */
	if (st_ptr->stock.size() >= st_ptr->stock_size)
	{
		return;
	}

	/* Hack -- consider up to four items */
	for (int tries = 0; tries < 4; tries++)
	{
		obj_all_done = false;

		/* Magic Shop */
		if ((st_info[st_ptr->st_idx].name == STORE_MAGIC) &&
		    magik(20))
		{
			s16b spell;

			object_prep(&forge, lookup_kind(TV_BOOK, BOOK_RANDOM));
			spell = get_random_spell(SKILL_MAGIC, 20);
			assert (spell > -1);
			forge.pval = spell;

			/* Use the forged object */
			q_ptr = &forge;
			obj_all_done = true;
		}

		/* Temple */
		else if ((st_info[st_ptr->st_idx].name == STORE_TEMPLE) &&
			 magik(20))
		{
			s16b spell;

			object_prep(&forge, lookup_kind(TV_BOOK, BOOK_RANDOM));
			spell = get_random_spell(SKILL_SPIRITUALITY, 20);
			assert(spell > -1);
			forge.pval = spell;

			/* Use the forged object */
			q_ptr = &forge;
			obj_all_done = true;
		}

		/* Black Market */
		else if (st_info[st_ptr->st_idx].flags & STF_ALL_ITEM)
		{
			/* No themes */
			init_match_theme(obj_theme::no_theme());

			/*
			 * Even in Black Markets, illegal objects can be
			 * problematic -- Oxymoron?
			 */
			get_object_hook = kind_is_legal;

			/* Rebuild the allocation table */
			get_obj_num_prep();

			/* Pick a level for object/magic */
			level = return_level();

			/* Choose a k_info index */
			k_idx = get_obj_num(level);
			if (k_idx <= 0)
			{
				continue;
			}

			/* Invalidate the cached allocation table */
			alloc.kind_table_valid = false;

		}

		/* Normal Store */
		else
		{
			/* Hack -- Pick an item to sell */
			auto const &item = *uniform_element(st_info[st_ptr->st_idx].items);
			auto filter = item.filter;
			auto chance = item.chance;

			/* Don't allow k_info artifacts */
			if (boost::apply_visitor(is_artifact_p(), filter))
			{
				continue;
			}

			/* Does it passes the rarity check ? */
			if (!magik(chance)) continue;

			/* Hack -- fake level for apply_magic() */
			level = return_level();

			/* Choose the k_info index */
			k_idx = boost::apply_visitor(choose_k_idx(level), filter);
			if (k_idx <= 0)
			{
				continue;
			}
		}

		/* Only if not already done */
		if (!obj_all_done)
		{
			auto k_ptr = k_info.at(k_idx);
			/* Don't allow k_info artifacts */
			if (k_ptr->flags & TR_NORM_ART)
			{
				continue;
			}

			/* Don't allow artifacts */
			if (k_ptr->flags & TR_INSTA_ART)
			{
				continue;
			}

			/* Get local object */
			q_ptr = &forge;

			/* Create a new object of the chosen kind */
			object_prep(q_ptr, k_idx);

			/* Apply some "low-level" magic (no artifacts) */
			apply_magic(q_ptr, level, false, false, false);

			/* Hack -- Charge lite's */
			if (q_ptr->tval == TV_LITE)
			{
				auto const flags = object_flags(q_ptr);
				if (flags & TR_FUEL_LITE)
				{
					q_ptr->timeout = q_ptr->k_ptr->pval2;
				}
			}

		}

		/* The item is "known" */
		object_known(q_ptr);

		/* Prune the black market */
		if (st_info[st_ptr->st_idx].flags & STF_ALL_ITEM)
		{
			/* Hack -- No "crappy" items */
			if (black_market_crap(q_ptr)) continue;

			/* Hack -- No "cheap" items */
			if (object_value(q_ptr) < 10) continue;
		}

		/* Prune normal stores */
		else
		{
			/* No "worthless" items */
			if (object_value(q_ptr) <= 0) continue;
		}


		/* Mass produce and/or Apply discount */
		mass_produce(q_ptr);

		/* The charges an wands are per each, so multiply to get correct number */
		if (!obj_all_done && q_ptr->tval == TV_WAND)
		{
			q_ptr->pval *= q_ptr->number;
		}

		/* Attempt to carry the (known) item */
		store_carry(q_ptr);

		/* Definitely done */
		break;
	}
}



/*
 * Re-displays a single store entry
 */
static void display_entry(int pos)
{
	auto const &st_info = game->edit_data.st_info;

	/* Get the item */
	auto o_ptr = &st_ptr->stock[pos];

	/* Get the "offset" */
	auto const i = (pos % 12);

	/* Label it, clear the line --(-- */
	char out_val[160];
	strnfmt(out_val, 160, "%c) ", I2A(i));
	c_prt(get_item_letter_color(o_ptr), out_val, i + 6, 0);


	int cur_col = 3;
	{
		byte a = object_attr(o_ptr);
		char c = object_char(o_ptr);

		if (!o_ptr->k_ptr)
		{
			c = ' ';
		}

		Term_draw(cur_col, i + 6, a, c);
		cur_col += 2;
	}

	/* Describe an item in the home */
	if ((cur_store_num == 7) ||
	        (st_info[st_ptr->st_idx].flags & STF_MUSEUM))
	{
		int maxwid = 75;

		/* Leave room for weights */
		maxwid -= 10;

		/* Describe the object */
		char o_name[80];
		object_desc(o_name, o_ptr, true, 3);
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i + 6, cur_col);

		/* Show weights */
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			strnfmt(out_val, 160, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(out_val, i + 6, 68);
		}
	}

	/* Describe an item (fully) in a store */
	else
	{
		byte color = TERM_WHITE;

		/* Must leave room for the "price" */
		int maxwid = 65;

		/* Leave room for weights */
		maxwid -= 7;

		/* Describe the object (fully) */
		char o_name[80];
		object_desc_store(o_name, o_ptr, true, 3);
		o_name[maxwid] = '\0';
		c_put_str(tval_to_attr[o_ptr->tval], o_name, i + 6, cur_col);

		/* Show weights */
		{
			/* Only show the weight of an individual item */
			int wgt = o_ptr->weight;
			strnfmt(out_val, 160, "%3d.%d", wgt / 10, wgt % 10);
			put_str(out_val, i + 6, 61);
		}

		/* Extract the "minimum" price */
		auto const x = price_item(o_ptr, ot_ptr->inflation, false);

		/* Can we buy one ? */
		if (x > p_ptr->au) color = TERM_L_DARK;

		/* Actually draw the price */
		strnfmt(out_val, 160, "%9ld  ", static_cast<long>(x));
		c_put_str(color, out_val, i + 6, 68);
	}
}


/*
 * Displays a store's inventory 		-RAK-
 * All prices are listed as "per individual object".  -BEN-
 */
static void display_inventory()
{
	int k;

	/* Display the next 12 items */
	for (k = 0; k < 12; k++)
	{
		/* Do not display "dead" items */
		if (store_top + k >= static_cast<int>(st_ptr->stock.size()))
		{
			break;
		}

		/* Display that line */
		display_entry(store_top + k);
	}

	/* Erase the extra lines and the "more" prompt */
	for (int i = k; i < 13; i++) prt("", i + 6, 0);

	/* Assume "no current page" */
	put_str("         ", 5, 20);

	/* Visual reminder of "more items" */
	if (st_ptr->stock.size() > 12)
	{
		/* Show "more" reminder (after the last item) */
		prt("-more-", k + 6, 3);

		/* Indicate the "current page" */
		put_str(format("(Page %d) ", store_top / 12 + 1), 5, 20);
	}
}


/*
 * Displays players gold					-RAK-
 */
void store_prt_gold()
{
	char out_val[64];

	prt("Gold Remaining: ", 19, 53);

	strnfmt(out_val, 64, "%9ld", static_cast<long>(p_ptr->au));
	prt(out_val, 19, 68);
}


/*
 * Displays store (after clearing screen)		-RAK-
 */
void display_store()
{
	auto const &st_info = game->edit_data.st_info;

	char buf[80];


	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (cur_store_num == 7)
	{
		put_str("Your Home", 3, 30);

		/* Label the item descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		put_str("Weight", 5, 70);
	}

	else if (st_info[st_ptr->st_idx].flags & STF_MUSEUM)
	{
		/* Show the name of the store */
		prt(st_info[cur_store_num].name, 3, 30);

		/* Label the item descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		put_str("Weight", 5, 70);
	}

	/* Normal stores */
	else
	{
		/* Put the owner name and race */
		strnfmt(buf, 80, "%s", ot_ptr->name.c_str());
		put_str(buf, 3, 10);

		/* Show the max price in the store (above prices) */
		prt(fmt::format("{:s} ({:d})", st_info[cur_store_num].name, ot_ptr->max_cost), 3, 50);

		/* Label the item descriptions */
		put_str("Item Description", 5, 3);

		/* If showing weights, show label */
		put_str("Weight", 5, 60);

		/* Label the asking price (in stores) */
		put_str("Price", 5, 72);
	}

	/* Display the current gold */
	store_prt_gold();

	/* Draw in the inventory */
	display_inventory();
}



/*
 * Get the ID of a store item and return its value	-RAK-
 */
static int get_stock(int *com_val, const char *pmt, int i, int j)
{
	char	command;

	char	out_val[160];

	/* Get the item index */
	if (repeat_pull(com_val))
	{

		/* Verify the item */
		if ((*com_val >= i) && (*com_val <= j))
		{
			/* Success */
			return true;
		}
	}

	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Assume failure */
	*com_val = ( -1);

	/* Build the prompt */
	strnfmt(out_val, 160, "(Items %c-%c, ESC to exit) %s",
	        I2A(i), I2A(j), pmt);

	/* Ask until done */
	while (true)
	{
		int k;

		/* Escape */
		if (!get_com(out_val, &command)) break;

		/* Convert */
		k = (islower(command) ? A2I(command) : -1);

		/* Legal responses */
		if ((k >= i) && (k <= j))
		{
			*com_val = k;
			break;
		}

		/* Oops */
		bell();
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Cancel */
	if (command == ESCAPE) return false;

	repeat_push(*com_val);

	/* Success */
	return true;
}



/**
 * Prompt for a yes/no during selling/buying
 *
 * @return true if 'yes' was selected, otherwise returns false.
 */
static bool prompt_yesno(const char *prompt)
{
	const char *allowed = "yn\r\n";
	const char *yes = "y\r\n";
	char buf[128];
	bool ret;

	/* Build prompt */
	snprintf(buf, sizeof(buf), "%s [y/n/RET/ESC] ", prompt);

	/* Prompt for it */
	msg_print(NULL);
	prt(buf, 0, 0);

	/* Get answer */
	while (true)
	{
		int key = inkey();

		/* ESC means no. */
		if (key == ESCAPE) {
			ret = false;
			break;
		}

		/* Any other key must be in the allowed set to break the loop. */
		if ((strchr(allowed, key) != NULL) || options->quick_messages) {
			/* Check for presence in the 'yes' set */
			ret = (strchr(yes, key) != NULL);
			break;
		}

		/* Retry */
		bell();
	}

	/* Erase the prompt */
	prt("", 0, 0);

	/* Success */
	return ret;
}



/*
 * Haggling routine 				-RAK-
 *
 * Return true if purchase is NOT successful
 */
static bool purchase_haggle(object_type *o_ptr, s32b *price)
{
	s32b	cur_ask;
	bool	cancel = false;
	char	out_val[160];
	char    prompt[128];
	char    o_name[80];


	*price = 0;

	/* Extract the price */
	cur_ask = price_item(o_ptr, ot_ptr->inflation, false);

	/* Buy for the whole pile */
	cur_ask *= o_ptr->number;

	/* Describe the object (fully) */
	object_desc_store(o_name, o_ptr, true, 3);

	/* Prompt */
	strnfmt(out_val, sizeof(out_val), "%s: " FMTs32b, "Price", cur_ask);
	put_str(out_val, 1, 0);
	strnfmt(prompt, sizeof(prompt), "Buy %s?", o_name);
	cancel = !prompt_yesno(prompt);

	/* Handle result */
	if (cancel)
	{
		/* Cancel */
		return true;
	}
	else
	{
		*price = cur_ask;
		/* Do not cancel */
		return false;
	}
}


/*
 * Haggling routine 				-RAK-
 *
 * Return true if purchase is NOT successful
 */
static bool sell_haggle(object_type *o_ptr, s32b *price)
{
	s32b	cur_ask;
	bool	cancel = false;
	char	out_val[160];
	char    prompt[128];
	char    o_name[80];


	*price = 0;

	/* Extract price */
	cur_ask = price_item(o_ptr, ot_ptr->inflation, true);

	/* Limit to shopkeeper's purse */
	if (cur_ask > ot_ptr->max_cost) {
		cur_ask = ot_ptr->max_cost;
	}

	/* Sell the whole pile */
	cur_ask *= o_ptr->number;

	/* Describe the object */
	object_desc(o_name, o_ptr, true, 3);

	/* Prompt */
	strnfmt(out_val, sizeof(out_val), "%s: " FMTs32b, "Price", cur_ask);
	put_str(out_val, 1, 0);
	strnfmt(prompt, sizeof(prompt), "Sell %s?", o_name);
	cancel = !prompt_yesno(prompt);

	/* Handle result */
	if (cancel)
	{
		/* Cancel */
		return true;
	}
	else
	{
		*price = cur_ask;
		/* Do not cancel */
		return false;
	}
}

/*
 * Will the owner retire?
 */
static bool retire_owner_p()
{
	auto const &st_info = game->edit_data.st_info;

	auto sti_ptr = &st_info[town_info[p_ptr->town_num].store[cur_store_num].st_idx];

	if (sti_ptr->owners.size() > 1)
	{
		return false; // No other possible owner
	}

	if (rand_int(STORE_SHUFFLE) != 0)
	{
		return false;
	}

	return true;
}

/*
 * Adjust store_top to account for a removed item
 */
static void adjust_store_top_item_removed()
{
	/* Nothing left? */
	if (st_ptr->stock.empty() == 0)
	{
		store_top = 0;
	}

	/* Already at the top beginning? */
	else if (store_top == 0)
	{
		/* Nothing to do */
	}

	/* Nothing left on current screen? */
	else if (store_top >= static_cast<int>(st_ptr->stock.size()))
	{
		store_top -= 12;
	}
}


/*
 * Stole an item from a store                   -DG-
 */
void store_stole()
{
	if (cur_store_num == 7)
	{
		msg_print("You can't steal from your home!");
		return;
	}

	/* Empty? */
	if (st_ptr->stock.empty())
	{
		msg_print("There is no item to steal.");
		return;
	}


	/* Find the number of objects on this and following pages */
	int i = (st_ptr->stock.size() - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	char out_val[160];
	strnfmt(out_val, 160, "Which item do you want to steal? ");

	/* Get the item number to be bought */
	int item;
	if (!get_stock(&item, out_val, 0, i - 1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	object_type *o_ptr = &st_ptr->stock[item];

	/* Assume the player wants just one of them */
	int amt = 1;

	/* Get a copy of the object */
	object_type forge;
	object_type *j_ptr = &forge;
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many different items.");
		return;
	}

	/* Find out how many the player wants */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	j_ptr = &forge;

	/* Get desired object */
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Player tries to stole it */
	if (rand_int((40 - p_ptr->stat_ind[A_DEX]) +
	                ((j_ptr->weight * amt) / (5 + get_skill_scale(SKILL_STEALING, 15))) -
	                (get_skill_scale(SKILL_STEALING, 15))) <= 10)
	{
		/* Hack -- buying an item makes you aware of it */
		object_aware(j_ptr);

		/* Be aware of how you found it */
		j_ptr->found = OBJ_FOUND_STOLEN;
		j_ptr->found_aux1 = st_ptr->st_idx;

		/* "Hot" merchandise can't be sold back.  It doesn't make sense
		   to be able to sell back to a guy what you just stole from him.
		   Also, without the discount one could fairly easily macro himself
		   an infinite money supply */
		j_ptr->discount = 100;

		if (o_ptr->tval == TV_WAND)
		{
			j_ptr->pval = o_ptr->pval * amt / o_ptr->number;
			o_ptr->pval -= j_ptr->pval;
		}

		/* Describe the transaction */
		char o_name[80];
		object_desc(o_name, j_ptr, true, 3);

		/* Message */
		msg_format("You steal %s.", o_name);

		/* Erase the inscription */
		j_ptr->inscription.clear();

		/* Give it to the player */
		int const item_new = inven_carry(j_ptr, false);

		/* Describe the final result */
		object_desc(o_name, &p_ptr->inventory[item_new], true, 3);

		/* Message */
		msg_format("You have %s (%c).",
		           o_name, index_to_label(item_new));

		/* Handle stuff */
		handle_stuff();

		/* Note how many slots the store used to have */
		auto prev_stock_size = st_ptr->stock.size();

		/* Remove the bought items from the store */
		store_item_increase(item, -amt);
		store_item_optimize(item);

		/* Store is empty */
		if (st_ptr->stock.empty())
		{
			/* Shuffle */
			if (retire_owner_p())
			{
				/* Message */
				msg_print("The shopkeeper retires.");

				/* Shuffle the store */
				store_shuffle(cur_store_num);
			}

			/* Maintain */
			else
			{
				/* Message */
				msg_print("The shopkeeper brings out some new stock.");
			}

			/* New inventory */
			for (int k = 0; k < 10; k++)
			{
				/* Maintain the store */
				store_maint(p_ptr->town_num, cur_store_num);
			}

			/* Start over */
			store_top = 0;

			/* Redraw everything */
			display_inventory();
		}

		/* The item is gone */
		else if (st_ptr->stock.size() != prev_stock_size)
		{
			adjust_store_top_item_removed();

			/* Redraw everything */
			display_inventory();
		}

		/* Item is still here */
		else
		{
			/* Redraw the item */
			display_entry(item);
		}
	}
	else
	{
		/* Complain */
		say_comment_4();

		/* Kicked out for a LONG time */
		st_ptr->store_open = turn + 500000 + randint(500000);
	}
}

/*
 * Buy an item from a store 			-RAK-
 */
void store_purchase()
{
	auto const &st_info = game->edit_data.st_info;

	/* Museum? */
	if (st_info[st_ptr->st_idx].flags & STF_MUSEUM)
	{
		msg_print("You cannot take items from the museum!");
		return;
	}

	/* Empty? */
	if (st_ptr->stock.empty())
	{
		if (cur_store_num == 7) msg_print("Your home is empty.");
		else msg_print("I am currently out of stock.");
		return;
	}


	/* Find the number of objects on this and following pages */
	int i = (st_ptr->stock.size() - store_top);

	/* And then restrict it to the current page */
	if (i > 12) i = 12;

	/* Prompt */
	char out_val[160];
	if (cur_store_num == 7)
	{
		strnfmt(out_val, 160, "Which item do you want to take? ");
	}
	else
	{
		strnfmt(out_val, 160, "Which item are you interested in? ");
	}

	/* Get the item number to be bought */
	int item;
	if (!get_stock(&item, out_val, 0, i - 1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	auto o_ptr = &st_ptr->stock[item];

	/* Get a copy of one object to determine the price */
	object_type forge;
	auto j_ptr = &forge;
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = 1;

	/* Hack -- If a wand, allocate the number of charges of one wand */
	if (j_ptr->tval == TV_WAND)
	{
		j_ptr->pval = o_ptr->pval / o_ptr->number;
	}

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many different items.");
		return;
	}

	/* Determine the "best" price (per item) */
	auto const best = price_item(j_ptr, ot_ptr->inflation, false);

	/* Find out how many the player wants */
	int amt = 1;
	if (o_ptr->number > 1)
	{
		s32b q;


		/* How many can we buy ? 99 if price is 0*/
		if (cur_store_num == STORE_HOME)
		{
			q = 99;
		}
		else if (best == 0)
		{
			q = 99;
		}
		else
		{
			q = p_ptr->au / best;
		}
		if (o_ptr->number < q)
			q = o_ptr->number;

		/* None ? ahh too bad */
		if (!q)
		{
			msg_print("You do not have enough gold to buy one.");
			return;
		}

		/* Get a quantity */
		amt = get_quantity(NULL, q);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get desired object */
	j_ptr = &forge;
	object_copy(j_ptr, o_ptr);

	/* Modify quantity */
	j_ptr->number = amt;

	/* Hack -- If a rod or wand, allocate total maximum timeouts or charges
	 * between those purchased and left on the shelf. -LM-
	 */
	if (o_ptr->tval == TV_WAND)
	{
		j_ptr->pval = o_ptr->pval * amt / o_ptr->number;
	}

	/* Hack -- require room in pack */
	if (!inven_carry_okay(j_ptr))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Attempt to buy it */
	if (cur_store_num != 7)
	{
		/* Haggle for a final price */
		s32b price;
		auto const choice = purchase_haggle(j_ptr, &price);

		/* Hack -- Got kicked out */
		if (st_ptr->store_open >= turn) return;


		/* Player wants it */
		if (!choice)
		{
			/* Player can afford it */
			if (p_ptr->au >= price)
			{
				/* Say "okay" */
				say_comment_1();

				/* Spend the money */
				p_ptr->au -= price;

				/* Update the display */
				store_prt_gold();

				/* Hack -- buying an item makes you aware of it */
				object_aware(j_ptr);

				/* Be aware of how you found it */
				j_ptr->found = OBJ_FOUND_STORE;
				j_ptr->found_aux1 = st_ptr->st_idx;

				/* Describe the transaction */
				char o_name[80];
				object_desc(o_name, j_ptr, true, 3);

				/* Message */
				msg_format("You bought %s for " FMTs32b " gold.", o_name, price);

				/* Erase the inscription */
				j_ptr->inscription.clear();

				/* Hack -- If a rod or wand, allocate total maximum
				 * timeouts or charges between those picked up and 
				 * those left behind. -LM-
				 */
				if (o_ptr->tval == TV_WAND)
				{
					j_ptr->pval = o_ptr->pval * amt / o_ptr->number;
					o_ptr->pval -= j_ptr->pval;
				}

				/* Give it to the player */
				int const item_new = inven_carry(j_ptr, false);

				/* Describe the final result */
				object_desc(o_name, &p_ptr->inventory[item_new], true, 3);

				/* Message */
				msg_format("You have %s (%c).",
				           o_name, index_to_label(item_new));

				/* Handle stuff */
				handle_stuff();

				/* Note how many slots the store used to have */
				auto prev_stock_size = st_ptr->stock.size();

				/* Remove the bought items from the store */
				store_item_increase(item, -amt);
				store_item_optimize(item);

				/* Store is empty */
				if (st_ptr->stock.empty())
				{
					/* Shuffle */
					if (retire_owner_p())
					{
						/* Message */
						msg_print("The shopkeeper retires.");

						/* Shuffle the store */
						store_shuffle(cur_store_num);
					}

					/* Maintain */
					else
					{
						/* Message */
						msg_print("The shopkeeper brings out some new stock.");
					}

					/* New inventory */
					for (int k = 0; k < 10; k++)
					{
						/* Maintain the store */
						store_maint(p_ptr->town_num, cur_store_num);
					}

					/* Start over */
					store_top = 0;
				}

				/* The item is gone */
				else if (st_ptr->stock.size() != prev_stock_size)
				{
					adjust_store_top_item_removed();
				}

				/* Redraw everything */
				display_inventory();
			}

			/* Player cannot afford it */
			else
			{
				/* Simple message (no insult) */
				msg_print("You do not have enough gold.");
			}
		}
	}

	/* Home is much easier */
	else
	{
		/* Hack -- If a rod or wand, allocate total maximum
		 * timeouts or charges between those picked up and 
		 * those left behind. -LM-
		 */
		if (o_ptr->tval == TV_WAND)
		{
			j_ptr->pval = o_ptr->pval * amt / o_ptr->number;
			o_ptr->pval -= j_ptr->pval;
		}

		/* Give it to the player */
		int const item_new = inven_carry(j_ptr, false);

		/* Describe just the result */
		char o_name[80];
		object_desc(o_name, &p_ptr->inventory[item_new], true, 3);

		/* Message */
		msg_format("You have %s (%c).", o_name, index_to_label(item_new));

		/* Handle stuff */
		handle_stuff();

		/* Take note if we take the last one */
		std::size_t prev_stock_size = st_ptr->stock.size();

		/* Remove the items from the home */
		store_item_increase(item, -amt);
		store_item_optimize(item);

		/* Hack -- Item is still here */
		if (prev_stock_size == st_ptr->stock.size())
		{
			/* Redraw the item */
			display_entry(item);
		}

		/* The item is gone */
		else
		{
			adjust_store_top_item_removed();

			/* Redraw everything */
			display_inventory();
		}
	}
}


/*
 * Sell an item to the store (or home)
 */
void store_sell()
{
	auto const &st_info = game->edit_data.st_info;

	int choice;
	int item, item_pos;
	int amt;

	s32b price, value, dummy;

	object_type forge;
	object_type *q_ptr;

	char o_name[80];

	bool museum = bool(st_info[st_ptr->st_idx].flags & STF_MUSEUM);

	/* Prepare prompt */
	const char *q;
	const char *s;
	if (cur_store_num == STORE_HOME)
	{
		q = "Drop which item? ";
		s = "You have nothing to drop.";
	}
	else if (museum)
	{
		q = "Donate which item?";
		s = "You have nothing to donate.";
	}
	else
	{
		q = "Sell which item? ";
		s = "You have nothing that I want.";
	}

	/* Get an item */
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN), store_will_buy))
	{
		return;
	}

	/* Get the item */
	auto o_ptr = get_object(item);

	auto const flags = object_flags(o_ptr);

	/* Hack -- Cannot remove cursed items */
	if (cursed_p(o_ptr))
	{
		if (item >= INVEN_WIELD)
		{
			/* Oops */
			msg_print("Hmmm, it seems to be cursed.");

			/* Nope */
			return;
		}
		else
		{
			if (flags & TR_CURSE_NO_DROP)
			{
				/* Oops */
				msg_print("Hmmm, you seem to be unable to drop it.");

				/* Nope */
				return;
			}
		}
	}


	/* Assume one item */
	amt = 1;

	/* Find out how many the player wants (letter means "all") */
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Get local object */
	q_ptr = &forge;

	/* Get a copy of the object */
	object_copy(q_ptr, o_ptr);

	/* Modify quantity */
	q_ptr->number = amt;

	/* Hack -- If a rod or wand, allocate total maximum
	 * timeouts or charges to those being sold. -LM-
	 */
	if (o_ptr->tval == TV_WAND)
	{
		q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
	}

	/* Get a full description */
	object_desc(o_name, q_ptr, true, 3);

	/* Remove any inscription for stores */
	if ((cur_store_num != 7) && !museum)
	{
		q_ptr->inscription.clear();
	}

	/* Is there room in the store (or the home?) */
	if (!store_check_num(q_ptr))
	{
		if (cur_store_num == 7) msg_print("Your home is full.");
		else if (museum) msg_print("The museum is full.");
		else msg_print("I have not the room in my store to keep it.");
		return;
	}


	/* Real store */
	if ((cur_store_num != 7) && !museum)
	{
		/* Haggle for it */
		choice = sell_haggle(q_ptr, &price);

		/* Kicked out */
		if (st_ptr->store_open >= turn) return;

		/* Sold... */
		if (choice == 0)
		{
			/* Say "okay" */
			say_comment_1();

			/* Get some money */
			p_ptr->au += price;

			/* Update the display */
			store_prt_gold();

			/* Get the "apparent" value */
			dummy = object_value(q_ptr) * q_ptr->number;

			/* Identify original item */
			object_aware(o_ptr);
			object_known(o_ptr);

			/* Combine / Reorder the pack (later) */
			p_ptr->notice |= (PN_COMBINE | PN_REORDER);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

			/* Get local object */
			q_ptr = &forge;

			/* Get a copy of the object */
			object_copy(q_ptr, o_ptr);

			/* Modify quantity */
			q_ptr->number = amt;

			/*
			 * Hack -- If a rod or wand, let the shopkeeper know just 
			 * how many charges he really paid for. -LM-
			 */
			if (o_ptr->tval == TV_WAND)
			{
				q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
			}

			/* Get the "actual" value */
			value = object_value(q_ptr) * q_ptr->number;

			/* Get the description all over again */
			object_desc(o_name, q_ptr, true, 3);

			/* Describe the result (in message buffer) */
			msg_format("You sold %s for " FMTs32b " gold.", o_name, price);

			/* Analyze the prices (and comment verbally) */
			purchase_analyze(price, value, dummy);

			/*
			 * Hack -- Allocate charges between those wands or rods sold 
			 * and retained, unless all are being sold. -LM-
			 */
			if (o_ptr->tval == TV_WAND)
			{
				q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

				if (o_ptr->number > amt) o_ptr->pval -= q_ptr->pval;
			}

			/* Take the item from the player, describe the result */
			inc_stack_size(item, -amt);

			/* Handle stuff */
			handle_stuff();

			/* The store gets that (known) item */
			item_pos = store_carry(q_ptr);

			/* Re-display if item is now in store */
			if (item_pos >= 0)
			{
				store_top = (item_pos / 12) * 12;
				display_inventory();
			}
		}
	}

	/* Player is at museum */
	else if (museum)
	{
		char o2_name[80];
		object_desc(o2_name, q_ptr, true, 0);

		msg_print("Once you donate something, you cannot take it back.");
		if (!get_check(format("Do you really want to donate %s?", o2_name))) return;

		/* Identify it */
		object_aware(q_ptr);
		object_known(q_ptr);

		/*
		 * Hack -- Allocate charges between those wands or rods sold 
		 * and retained, unless all are being sold. -LM-
		 */
		if (o_ptr->tval == TV_WAND)
		{
			q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

			if (o_ptr->number > amt) o_ptr->pval -= q_ptr->pval;
		}


		/* Describe */
		msg_format("You donate %s (%c).", o_name, index_to_label(item));

		choice = 0;

		/* Take it from the players inventory */
		inc_stack_size(item, -amt);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		item_pos = home_carry(q_ptr);

		/* Update store display */
		if (item_pos >= 0)
		{
			store_top = (item_pos / 12) * 12;
			display_inventory();
		}
	}

	/* Player is at home */
	else
	{
		/* Describe */
		msg_format("You drop %s (%c).", o_name, index_to_label(item));

		/*
		 * Hack -- Allocate charges between those wands or rods sold 
		 * and retained, unless all are being sold. -LM-
		 */
		if (o_ptr->tval == TV_WAND)
		{
			q_ptr->pval = o_ptr->pval * amt / o_ptr->number;

			if (o_ptr->number > amt) o_ptr->pval -= q_ptr->pval;
		}

		/* Take it from the players inventory */
		inc_stack_size(item, -amt);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		item_pos = home_carry(q_ptr);

		/* Update store display */
		if (item_pos >= 0)
		{
			store_top = (item_pos / 12) * 12;
			display_inventory();
		}
	}
}



/*
 * Examine an item in a store			   -JDL-
 */
void store_examine()
{
	auto const &st_info = game->edit_data.st_info;

	/* Empty? */
	if (st_ptr->stock.empty())
	{
		if (cur_store_num == 7) msg_print("Your home is empty.");
		else if (st_info[st_ptr->st_idx].flags & STF_MUSEUM) msg_print("The museum is empty.");
		else msg_print("I am currently out of stock.");
		return;
	}


	/* Find the number of objects on this and following pages */
	int i = (st_ptr->stock.size() - store_top);

	/* And then restrict it to the current page */
	if (i > 12)
	{
		i = 12;
	}

	/* Get the item number to be examined */
	int item;
	if (!get_stock(&item, "Which item do you want to examine? ", 0, i - 1)) return;

	/* Get the actual index */
	item = item + store_top;

	/* Get the actual item */
	auto o_ptr = &st_ptr->stock[item];

	/* Debug hack */
	if (wizard)
	{
		drop_near(o_ptr, -1, p_ptr->py, p_ptr->px);
	}

	/* Description */
	char o_name[80];
	object_desc(o_name, o_ptr, true, 3);

	/* Describe */
	msg_format("Examining %s...", o_name);

	/* Show the object's powers. */
	if (!object_out_desc(o_ptr, NULL, false, true))
	{
		msg_print("You see nothing special.");
	}

	/* Show spell listing for instruments, daemonwear and spellbooks. */
	if ((o_ptr->tval == TV_INSTRUMENT) || (o_ptr->tval == TV_DAEMON_BOOK)
	    || (o_ptr->tval == TV_BOOK))
	{
		do_cmd_browse_aux(o_ptr);
	}

}




/*
 * Hack -- set this to leave the store
 */
static bool leave_store = false;


/*
 * Find building action for command. Returns nullptr if no matching
 * action is found.
 */
static store_action_type const *find_store_action(s16b command_cmd)
{
	auto const &st_info = game->edit_data.st_info;
	auto const &ba_info = game->edit_data.ba_info;

	for (std::size_t i = 0; i < st_info[st_ptr->st_idx].actions.size(); i++)
	{
		auto ba_ptr = &ba_info[st_info[st_ptr->st_idx].actions[i]];

		if (ba_ptr->letter && (ba_ptr->letter == command_cmd))
		{
			return ba_ptr;
		}

		if (ba_ptr->letter_aux && (ba_ptr->letter_aux == command_cmd))
		{
			return ba_ptr;
		}
	}

	return nullptr;
}


/*
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands
 * in the stores which are not allowed in the dungeon, and we
 * must disable some commands which are allowed in the dungeon
 * but not in the stores, to prevent chaos.
 */
static bool store_process_command()
{
	bool recreate = false;

	/* Handle repeating the last command */
	repeat_check(&command_cmd);

	auto ba_ptr = find_store_action(command_cmd);

	if (ba_ptr)
	{
		recreate = bldg_process_command(st_ptr, ba_ptr);
	}
	else
	{
		/* Parse the command */
		switch (command_cmd)
		{
			/* Leave */
		case ESCAPE:
			{
				leave_store = true;
				break;
			}

			/* Browse */
		case ' ':
			{
				if (st_ptr->stock.size() <= 12)
				{
					msg_print("Entire inventory is shown.");
				}
				else
				{
					store_top += 12;
					if (store_top >= static_cast<int>(st_ptr->stock.size()))
					{
						store_top = 0;
					}
					display_inventory();
				}
				break;
			}

			/* Browse backwards */
		case '-':
			{
				if (st_ptr->stock.size() <= 12)
				{
					msg_print("Entire inventory is shown.");
				}
				else
				{
					store_top -= 12;
					if (store_top < 0)
					{
						store_top = ((st_ptr->stock.size() - 1) / 12) * 12;
					}
					display_inventory();
				}
				break;
			}

			/* Redraw */
		case KTRL('R'):
			{
				do_cmd_redraw();
				display_store();
				break;
			}

			/* Ignore return */
		case '\r':
			{
				break;
			}



			/*** Inventory Commands ***/

			/* Wear/wield equipment */
		case 'w':
			{
				do_cmd_wield();
				break;
			}

			/* Take off equipment */
		case 't':
			{
				do_cmd_takeoff();
				break;
			}

			/* Destroy an item */
		case 'k':
			{
				do_cmd_destroy();
				break;
			}

			/* Equipment list */
		case 'e':
			{
				do_cmd_equip();
				break;
			}

			/* Inventory list */
		case 'i':
			{
				do_cmd_inven();
				break;
			}


			/*** Various commands ***/

			/* Identify an object */
		case 'I':
			{
				do_cmd_observe();
				break;
			}

			/* Hack -- toggle windows */
		case KTRL('I'):
			{
				toggle_inven_equip();
				break;
			}



			/*** Use various objects ***/

			/* Browse a book */
		case 'b':
			{
				do_cmd_browse();
				break;
			}

			/* Inscribe an object */
		case '{':
			{
				do_cmd_inscribe();
				break;
			}

			/* Uninscribe an object */
		case '}':
			{
				do_cmd_uninscribe();
				break;
			}



			/*** Help and Such ***/

			/* Help */
		case '?':
			{
				do_cmd_help();
				break;
			}

			/* Identify symbol */
		case '/':
			{
				do_cmd_query_symbol();
				break;
			}

			/* Character description */
		case 'C':
			{
				do_cmd_change_name();
				display_store();
				break;
			}


			/*** System Commands ***/

			/* Single line from a pref file */
		case '"':
			{
				do_cmd_pref();
				break;
			}

			/* Interact with macros */
		case '@':
			{
				do_cmd_macros();
				break;
			}

			/* Interact with visuals */
		case '%':
			{
				do_cmd_visuals();
				break;
			}

			/* Interact with colors */
		case '&':
			{
				do_cmd_colors();
				break;
			}

			/* Interact with options */
		case '=':
			{
				do_cmd_options();
				break;
			}


			/*** Misc Commands ***/

			/* Take notes */
		case ':':
			{
				do_cmd_note();
				break;
			}

			/* Version info */
		case 'V':
			{
				do_cmd_version();
				break;
			}

			/* Repeat level feeling */
		case KTRL('F'):
			{
				do_cmd_feeling();
				break;
			}

			/* Show previous message */
		case KTRL('O'):
			{
				do_cmd_message_one();
				break;
			}

			/* Show previous messages */
		case KTRL('P'):
			{
				do_cmd_messages();
				break;
			}

			/* Check artifacts, uniques etc. */
		case '~':
		case '|':
			{
				do_cmd_knowledge();
				break;
			}

			/* Load "screen dump" */
		case '(':
			{
				do_cmd_load_screen();
				break;
			}

			/* Save "screen dump" */
		case ')':
			{
				do_cmd_save_screen();
				break;
			}


			/* Hack -- Unknown command */
		default:
			{
				if (st_ptr->st_idx == STORE_HOME)
					msg_print("That command does not work in this home.");
				else
					msg_print("That command does not work in this store.");
				break;
			}
		}
	}

	return recreate;
}


/*
 * Enter a store, and interact with it.
 *
 * Note that we use the standard "request_command()" function
 * to get a command, allowing us to use "command_arg" and all
 * command macros and other nifty stuff, but we use the special
 * "shopping" argument, to force certain commands to be converted
 * into other commands, normally, we convert "p" (pray) and "m"
 * (cast magic) into "g" (get), and "s" (search) into "d" (drop).
 */
void do_cmd_store()
{
	auto const &ow_info = game->edit_data.ow_info;
	auto const &ba_info = game->edit_data.ba_info;
	auto const &st_info = game->edit_data.st_info;

	int which;
	int maintain_num;
	int tmp_chr;
	int i;
	bool recreate = false;

	cave_type *c_ptr;


	/* Access the player grid */
	c_ptr = &cave[p_ptr->py][p_ptr->px];

	/* Verify a store */
	if (c_ptr->feat != FEAT_SHOP)
	{
		msg_print("You see no store here.");
		return;
	}

	/* Extract the store code */
	which = c_ptr->special;

	/* Hack -- Check the "locked doors" */
	if (town_info[p_ptr->town_num].store[which].store_open >= turn)
	{
		msg_print("The doors are locked.");
		return;
	}

	/* Calculate the number of store maintainances since the last visit */
	maintain_num = (turn - town_info[p_ptr->town_num].store[which].last_visit) / (10L * STORE_TURNS);

	/* Maintain the store max. 10 times */
	if (maintain_num > 10) maintain_num = 10;

	if (maintain_num)
	{
		/* Maintain the store */
		for (i = 0; i < maintain_num; i++)
			store_maint(p_ptr->town_num, which);

		/* Save the visit */
		town_info[p_ptr->town_num].store[which].last_visit = turn;
	}

	/* Forget the lite */
	/* forget_lite(); */

	/* Forget the view */
	forget_view();


	/* Hack -- Character is in "icky" mode */
	character_icky = true;


	/* No command argument */
	command_arg = 0;

	/* No repeated command */
	command_rep = 0;

	/* No automatic command */
	command_new = 0;


	/* Save the store number */
	cur_store_num = which;

	/* Save the store and owner pointers */
	st_ptr = &town_info[p_ptr->town_num].store[cur_store_num];
	ot_ptr = &ow_info[st_ptr->owner];


	/* Start at the beginning */
	store_top = 0;

	/* Display the store */
	display_store();

	/* Mega-Hack -- Ignore keymaps on store action letters */
	for (std::size_t i = 0; i < st_info[st_ptr->st_idx].actions.size(); i++)
	{
		auto ba_ptr = &ba_info[st_info[st_ptr->st_idx].actions[i]];
		request_command_ignore_keymaps[2*i] = ba_ptr->letter;
		request_command_ignore_keymaps[2*i+1] = ba_ptr->letter_aux;
	}

	/* Do not leave */
	leave_store = false;

	/* Interact with player */
	while (!leave_store)
	{
		/* Hack -- Clear line 1 */
		prt("", 1, 0);

		/* Hack -- Check the charisma */
		tmp_chr = p_ptr->stat_use[A_CHR];

		/* Clear */
		clear_from(21);


		/* Basic commands */
		c_prt(TERM_YELLOW, " ESC.", 22, 0);
		prt(") Exit.", 22, 4);

		/* Browse if necessary */
		if (st_ptr->stock.size() > 12)
		{
			c_prt(TERM_YELLOW, " SPACE", 23, 0);
			prt(") Next page", 23, 6);
		}

		/* Prompt */
		prt("You may: ", 21, 0);

		/* Show the commands */
		show_building(st_ptr);

		/* Get a command */
		request_command(true);

		/* Process the command */
		if (store_process_command()) recreate = true;

		/* Hack -- Character is still in "icky" mode */
		character_icky = true;

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();

		/* XXX XXX XXX Pack Overflow */
		if (p_ptr->inventory[INVEN_PACK].k_ptr)
		{
			int item = INVEN_PACK;

			object_type *o_ptr = &p_ptr->inventory[item];

			/* Hack -- Flee from the store */
			if (cur_store_num != 7)
			{
				/* Message */
				msg_print("Your pack is so full that you flee the store...");

				/* Leave */
				leave_store = true;
			}

			/* Hack -- Flee from the home */
			else if (!store_check_num(o_ptr))
			{
				/* Message */
				msg_print("Your pack is so full that you flee your home...");

				/* Leave */
				leave_store = true;
			}

			/* Hack -- Drop items into the home */
			else
			{
				/* Give a message */
				msg_print("Your pack overflows!");

				/* Get local object */
				object_type forge;
				auto q_ptr = &forge;

				/* Grab a copy of the item */
				object_copy(q_ptr, o_ptr);

				/* Describe it */
				char o_name[80];
				object_desc(o_name, q_ptr, true, 3);

				/* Message */
				msg_format("You drop %s (%c).", o_name, index_to_label(item));

				/* Remove it from the players inventory */
				inc_stack_size(item, -255);

				/* Handle stuff */
				handle_stuff();

				/* Let the home carry it */
				int item_pos = home_carry(q_ptr);

				/* Redraw the home */
				if (item_pos >= 0)
				{
					store_top = (item_pos / 12) * 12;
					display_inventory();
				}
			}
		}

		/* Hack -- Redisplay store prices if charisma changes */
		if (tmp_chr != p_ptr->stat_use[A_CHR]) display_inventory();

		/* Hack -- get kicked out of the store */
		if (st_ptr->store_open >= turn) leave_store = true;
	}

	/* Free turn XXX XXX XXX */
	energy_use = 0;

	/* Recreate the level only when needed */
	if (recreate)
	{
		/* Reinit wilderness to activate quests ... */
		p_ptr->oldpx = p_ptr->px;
		p_ptr->oldpy = p_ptr->py;

		p_ptr->leaving = true;
	}

	/* Hack -- Character is no longer in "icky" mode */
	character_icky = false;


	/* Hack -- Cancel automatic command */
	command_new = 0;

	/* Mega-Hack -- Clear the 'ignore-keymaps' list */
	memset(request_command_ignore_keymaps, 0, 12);

	/* Flush messages XXX XXX XXX */
	msg_print(NULL);


	/* Clear the screen */
	Term_clear();


	/* Update everything */
	p_ptr->update |= (PU_VIEW | PU_MON_LITE);
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_FRAME);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
 * Shuffle one of the stores.
 */
void store_shuffle(int which)
{
	auto const &ow_info = game->edit_data.ow_info;
	auto const &st_info = game->edit_data.st_info;

	/* Ignore home */
	if (which == STORE_HOME) return;

	/* Ignoer Museum */
	if (st_info[st_ptr->st_idx].flags & STF_MUSEUM) return;


	/* Save the store index */
	cur_store_num = which;

	/* Activate that store */
	st_ptr = &town_info[p_ptr->town_num].store[cur_store_num];

	/* Pick a new owner */
	for (auto j = st_ptr->owner; j == st_ptr->owner; )
	{
		st_ptr->owner = *uniform_element(st_info[st_ptr->st_idx].owners);
	}

	/* Activate the new owner */
	ot_ptr = &ow_info[st_ptr->owner];


	/* Reset the owner data */
	st_ptr->store_open = 0;


	/* Hack -- discount all the items */
	for (auto &o_ref: st_ptr->stock)
	{
		auto o_ptr = &o_ref;

		/* Hack -- Sell all old items for "half price" */
		if (o_ptr->artifact_name.empty())
			o_ptr->discount = 50;

		/* Mega-Hack -- Note that the item is "on sale" */
		o_ptr->inscription = "on sale";
	}
}


/*
 * Maintain the inventory at the stores.
 */
void store_maint(int town_num, int store_num)
{
	auto const &ow_info = game->edit_data.ow_info;
	auto const &st_info = game->edit_data.st_info;

	int const old_rating = rating;

	cur_store_num = store_num;

	/* Ignore home */
	if (store_num == STORE_HOME) return;

	/* Activate that store */
	st_ptr = &town_info[town_num].store[store_num];

	/* Ignoer Museum */
	if (st_info[st_ptr->st_idx].flags & STF_MUSEUM) return;

	/* Activate the owner */
	ot_ptr = &ow_info[st_ptr->owner];

	/* Mega-Hack -- prune the black market */
	if (st_info[st_ptr->st_idx].flags & STF_ALL_ITEM)
	{
		/* Destroy crappy black market items */
		for (int j = st_ptr->stock.size() - 1; j >= 0; j--)
		{
			object_type *o_ptr = &st_ptr->stock[j];

			/* Destroy crappy items */
			if (black_market_crap(o_ptr))
			{
				/* Destroy the item */
				store_item_increase(j, 0 - o_ptr->number);
				store_item_optimize(j);
			}
		}
	}


	/* Choose the number of slots to keep */
	int j = st_ptr->stock.size();

	/* Sell a few items */
	j = j - randint(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "underflow" */
	if (j < 0) j = 0;

	/* Destroy objects until only "j" slots are left */
	while (j < static_cast<int>(st_ptr->stock.size()))
	{
		store_delete();
	}

	/* Choose the number of slots to fill */
	j = st_ptr->stock.size();

	/* Buy some more items */
	j = j + randint(STORE_TURNOVER);

	/* Never keep more than "STORE_MAX_KEEP" slots */
	if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

	/* Always "keep" at least "STORE_MIN_KEEP" items */
	if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

	/* Hack -- prevent "overflow" */
	if (j >= st_ptr->stock_size)
	{
		j = st_ptr->stock_size - 1;
	}

	/* Acquire some new items */
	for (int tries = 0; (tries < 100) && (static_cast<int>(st_ptr->stock.size()) < j); tries++)
	{
		store_create();
	}

	/* Hack -- Restore the rating */
	rating = old_rating;
}


/*
 * Initialize the stores
 */
void store_init(int town_num, int store_num)
{
	auto const &ow_info = game->edit_data.ow_info;
	auto const &st_info = game->edit_data.st_info;

	cur_store_num = store_num;

	// Activate store
	st_ptr = &town_info[town_num].store[store_num];

	// Pick an owner. We use 0 for st_info[] which haven't been
	// initialized, i.e. where there's no entry in st_info.txt.
	st_ptr->owner = st_info[st_ptr->st_idx].owners.empty()
	        ? 0
	        : *uniform_element(st_info[st_ptr->st_idx].owners)
	        ;

	// Activate the new owner
	ot_ptr = &ow_info[st_ptr->owner];

	// Initialize the store
	st_ptr->store_open = 0;

	// Nothing in stock
	st_ptr->stock.reserve(st_ptr->stock_size);
	st_ptr->stock.clear();

	// MEGA-HACK - Last visit to store is BEFORE player
	// birth to enable store restocking.
	st_ptr->last_visit = -100L * STORE_TURNS;
}


/*
 * Enter the home, and interact with it from the dungeon (trump magic).
 *
 * Note that we use the standard "request_command()" function
 * to get a command, allowing us to use "command_arg" and all
 * command macros and other nifty stuff, but we use the special
 * "shopping" argument, to force certain commands to be converted
 * into other commands, normally, we convert "p" (pray) and "m"
 * (cast magic) into "g" (get), and "s" (search) into "d" (drop).
 */
void do_cmd_home_trump()
{
	auto const &ow_info = game->edit_data.ow_info;
	auto const &ba_info = game->edit_data.ba_info;
	auto const &st_info = game->edit_data.st_info;

	int which;
	int maintain_num;
	int tmp_chr;
	int town_num;

	/* Extract the store code */
	which = 7;

	if (p_ptr->town_num) town_num = p_ptr->town_num;
	else town_num = 1;

	/* Hack -- Check the "locked doors" */
	if (town_info[town_num].store[which].store_open >= turn)
	{
		msg_print("The doors are locked.");
		return;
	}

	/* Calculate the number of store maintainances since the last visit */
	maintain_num = (turn - town_info[town_num].store[which].last_visit) / (10L * STORE_TURNS);

	/* Maintain the store max. 10 times */
	if (maintain_num > 10) maintain_num = 10;

	if (maintain_num)
	{
		/* Maintain the store */
		for (int i = 0; i < maintain_num; i++)
		{
			store_maint(town_num, which);
		}

		/* Save the visit */
		town_info[town_num].store[which].last_visit = turn;
	}

	/* Forget the lite */
	/* forget_lite(); */

	/* Forget the view */
	forget_view();


	/* Hack -- Character is in "icky" mode */
	character_icky = true;


	/* No command argument */
	command_arg = 0;

	/* No repeated command */
	command_rep = 0;

	/* No automatic command */
	command_new = 0;


	/* Save the store number */
	cur_store_num = which;

	/* Save the store and owner pointers */
	st_ptr = &town_info[town_num].store[cur_store_num];
	ot_ptr = &ow_info[st_ptr->owner];


	/* Start at the beginning */
	store_top = 0;

	/* Display the store */
	display_store();

	/* Mega-Hack -- Ignore keymaps on store action letters */
	auto const &st_actions = st_info[st_ptr->st_idx].actions;
	for (std::size_t i = 0; (i < (MAX_IGNORE_KEYMAPS/2)) && (i < st_actions.size()); i++)
	{
		auto ba_ptr = &ba_info[st_actions[i]];
		request_command_ignore_keymaps[2*i] = ba_ptr->letter;
		request_command_ignore_keymaps[2*i+1] = ba_ptr->letter_aux;
	}

	/* Do not leave */
	leave_store = false;

	/* Interact with player */
	while (!leave_store)
	{
		/* Hack -- Clear line 1 */
		prt("", 1, 0);

		/* Hack -- Check the charisma */
		tmp_chr = p_ptr->stat_use[A_CHR];

		/* Clear */
		clear_from(21);


		/* Basic commands */
		prt(" ESC) Exit from Building.", 22, 0);

		/* Browse if necessary */
		if (st_ptr->stock.size() > 12)
		{
			prt(" SPACE) Next page of stock", 23, 0);
		}

		/* Home commands */
		if (cur_store_num == 7)
		{
			prt(" g) Get an item.", 22, 31);
			prt(" d) Drop an item.", 23, 31);
		}

		/* Shop commands XXX XXX XXX */
		else
		{
			prt(" p) Purchase an item.", 22, 31);
			prt(" s) Sell an item.", 23, 31);
		}

		/* Add in the eXamine option */
		prt(" x) eXamine an item.", 22, 56);

		/* Prompt */
		prt("You may: ", 21, 0);

		/* Get a command */
		request_command(true);

		/* Process the command */
		store_process_command();

		/* Hack -- Character is still in "icky" mode */
		character_icky = true;

		/* Notice stuff */
		notice_stuff();

		/* Handle stuff */
		handle_stuff();

		/* XXX XXX XXX Pack Overflow */
		if (p_ptr->inventory[INVEN_PACK].k_ptr)
		{
			int item = INVEN_PACK;

			object_type *o_ptr = &p_ptr->inventory[item];

			/* Hack -- Flee from the store */
			if (cur_store_num != 7)
			{
				/* Message */
				msg_print("Your pack is so full that you flee the store...");

				/* Leave */
				leave_store = true;
			}

			/* Hack -- Flee from the home */
			else if (!store_check_num(o_ptr))
			{
				/* Message */
				msg_print("Your pack is so full that you flee your home...");

				/* Leave */
				leave_store = true;
			}

			/* Hack -- Drop items into the home */
			else
			{
				/* Give a message */
				msg_print("Your pack overflows!");

				/* Get local object */
				object_type forge;
				auto q_ptr = &forge;

				/* Grab a copy of the item */
				object_copy(q_ptr, o_ptr);

				/* Describe it */
				char o_name[80];
				object_desc(o_name, q_ptr, true, 3);

				/* Message */
				msg_format("You drop %s (%c).", o_name, index_to_label(item));

				/* Remove it from the players inventory */
				inc_stack_size(item, -255);

				/* Handle stuff */
				handle_stuff();

				/* Let the home carry it */
				int const item_pos = home_carry(q_ptr);

				/* Redraw the home */
				if (item_pos >= 0)
				{
					store_top = (item_pos / 12) * 12;
					display_inventory();
				}
			}
		}

		/* Hack -- Redisplay store prices if charisma changes */
		if (tmp_chr != p_ptr->stat_use[A_CHR]) display_inventory();

		/* Hack -- get kicked out of the store */
		if (st_ptr->store_open >= turn) leave_store = true;
	}


	/* Hack -- Character is no longer in "icky" mode */
	character_icky = false;


	/* Hack -- Cancel automatic command */
	command_new = 0;

	/* Mega-Hack -- Clear the 'ignore-keymaps' list */
	memset(request_command_ignore_keymaps, 0, 12);

	/* Flush messages XXX XXX XXX */
	msg_print(NULL);


	/* Clear the screen */
	Term_clear();


	/* Update everything */
	p_ptr->update |= (PU_VIEW | PU_MON_LITE);
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_FRAME);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}
