/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "object1.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cmd2.hpp"
#include "cmd6.hpp"
#include "dungeon.hpp"
#include "dungeon_info_type.hpp"
#include "ego_item_type.hpp"
#include "feature_type.hpp"
#include "files.hpp"
#include "hook_get_in.hpp"
#include "hooks.hpp"
#include "lua_bind.hpp"
#include "mimic.hpp"
#include "monster1.hpp"
#include "monster2.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "quark.hpp"
#include "set_type.hpp"
#include "skills.hpp"
#include "spell_type.hpp"
#include "spells5.hpp"
#include "squeltch.hpp"
#include "stats.hpp"
#include "store_info_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "wilderness_type_info.hpp"
#include "xtra1.hpp"
#include "z-rand.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <format.h>

using boost::starts_with;

static bool_ apply_flags_set(s16b a_idx, s16b set_idx,
	u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp);

/*
 * Hack -- note that "TERM_MULTI" is now just "TERM_VIOLET".
 * We will have to find a cleaner method for "MULTI_HUED" later.
 * There were only two multi-hued "flavors" (one potion, one food).
 * Plus five multi-hued "base-objects" (3 dragon scales, one blade
 * of chaos, and one something else).  See the SHIMMER_OBJECTS code
 * in "dungeon.c" and the object color extractor in "cave.c".
 */
#define TERM_MULTI      TERM_VIOLET


/**
 * Show all equipment/inventory slots, even when empty?
 */
static bool item_tester_full;


/*
 * Max sizes of the following arrays
 */
#define MAX_ROCKS      62       /* Used with rings (min 58) */
#define MAX_AMULETS    34       /* Used with amulets (min 30) */
#define MAX_WOODS      35       /* Used with staffs (min 32) */
#define MAX_METALS     39       /* Used with wands/rods (min 32/30) */
#define MAX_COLORS     66       /* Used with potions (min 62) */
#define MAX_SHROOM     20       /* Used with mushrooms (min 20) */
#define MAX_TITLES     55       /* Used with scrolls (min 55) */
#define MAX_SYLLABLES 164       /* Used with scrolls (see below) */


/*
 * Rings (adjectives and colors)
 */

static cptr ring_adj[MAX_ROCKS] =
{
	"Alexandrite", "Amethyst", "Aquamarine", "Azurite", "Beryl",
	"Bloodstone", "Calcite", "Carnelian", "Corundum", "Diamond",
	"Emerald", "Fluorite", "Garnet", "Granite", "Jade",
	"Jasper", "Lapis Lazuli", "Malachite", "Marble", "Moonstone",
	"Onyx", "Opal", "Pearl", "Quartz", "Quartzite",
	"Rhodonite", "Ruby", "Sapphire", "Tiger Eye", "Topaz",
	"Turquoise", "Zircon", "Platinum", "Bronze", "Gold",
	"Obsidian", "Silver", "Tortoise Shell", "Mithril", "Jet",
	"Engagement", "Adamantite",
	"Wire", "Dilithium", "Bone", "Wooden",
	"Spikard", "Serpent", "Wedding", "Double",
	"Plain", "Brass", "Scarab", "Shining",
	"Rusty", "Transparent", "Copper", "Black Opal", "Nickel",
	"Glass", "Fluorspar", "Agate",
};

static byte ring_col[MAX_ROCKS] =
{
	TERM_GREEN, TERM_VIOLET, TERM_L_BLUE, TERM_L_BLUE, TERM_L_GREEN,
	TERM_RED, TERM_WHITE, TERM_RED, TERM_SLATE, TERM_WHITE,
	TERM_GREEN, TERM_L_GREEN, TERM_RED, TERM_L_DARK, TERM_L_GREEN,
	TERM_UMBER, TERM_BLUE, TERM_GREEN, TERM_WHITE, TERM_L_WHITE,
	TERM_L_RED, TERM_L_WHITE, TERM_WHITE, TERM_L_WHITE, TERM_L_WHITE,
	TERM_L_RED, TERM_RED, TERM_BLUE, TERM_YELLOW, TERM_YELLOW,
	TERM_L_BLUE, TERM_L_UMBER, TERM_WHITE, TERM_L_UMBER, TERM_YELLOW,
	TERM_L_DARK, TERM_L_WHITE, TERM_GREEN, TERM_L_BLUE, TERM_L_DARK,
	TERM_YELLOW, TERM_VIOLET,
	TERM_UMBER, TERM_L_WHITE, TERM_WHITE, TERM_UMBER,
	TERM_BLUE, TERM_GREEN, TERM_YELLOW, TERM_ORANGE,
	TERM_YELLOW, TERM_ORANGE, TERM_L_GREEN, TERM_YELLOW,
	TERM_RED, TERM_WHITE, TERM_UMBER, TERM_L_DARK, TERM_L_WHITE,
	TERM_WHITE, TERM_BLUE, TERM_L_WHITE
};


/*
 * Amulets (adjectives and colors)
 */

static cptr amulet_adj[MAX_AMULETS] =
{
	"Amber", "Driftwood", "Coral", "Agate", "Ivory",
	"Obsidian", "Bone", "Brass", "Bronze", "Pewter",
	"Tortoise Shell", "Golden", "Azure", "Crystal", "Silver",
	"Copper", "Amethyst", "Mithril", "Sapphire", "Dragon Tooth",
	"Carved Oak", "Sea Shell", "Flint Stone", "Ruby", "Scarab",
	"Origami Paper", "Meteoric Iron", "Platinum", "Glass", "Beryl",
	"Malachite", "Adamantite", "Mother-of-pearl", "Runed"
};

static byte amulet_col[MAX_AMULETS] =
{
	TERM_YELLOW, TERM_L_UMBER, TERM_WHITE, TERM_L_WHITE, TERM_WHITE,
	TERM_L_DARK, TERM_WHITE, TERM_ORANGE, TERM_L_UMBER, TERM_SLATE,
	TERM_GREEN, TERM_YELLOW, TERM_L_BLUE, TERM_L_BLUE, TERM_L_WHITE,
	TERM_L_UMBER, TERM_VIOLET, TERM_L_BLUE, TERM_BLUE, TERM_L_WHITE,
	TERM_UMBER, TERM_L_BLUE, TERM_SLATE, TERM_RED, TERM_L_GREEN,
	TERM_WHITE, TERM_L_DARK, TERM_L_WHITE, TERM_WHITE, TERM_L_GREEN,
	TERM_GREEN, TERM_VIOLET, TERM_L_WHITE, TERM_UMBER
};


/*
 * Staffs (adjectives and colors)
 */

static cptr staff_adj[MAX_WOODS] =
{
	"Aspen", "Balsa", "Banyan", "Birch", "Cedar",
	"Cottonwood", "Cypress", "Dogwood", "Elm", "Eucalyptus",
	"Hemlock", "Hickory", "Ironwood", "Locust", "Mahogany",
	"Maple", "Mulberry", "Oak", "Pine", "Redwood",
	"Rosewood", "Spruce", "Sycamore", "Teak", "Walnut",
	"Mistletoe", "Hawthorn", "Bamboo", "Silver", "Runed",
	"Golden", "Ashen", "Gnarled", "Ivory", "Willow"
};

static byte staff_col[MAX_WOODS] =
{
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER, TERM_L_UMBER, TERM_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_RED,
	TERM_RED, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER,
	TERM_GREEN, TERM_L_UMBER, TERM_L_UMBER, TERM_L_WHITE, TERM_UMBER,
	TERM_YELLOW, TERM_SLATE, TERM_UMBER, TERM_L_WHITE, TERM_L_UMBER
};


/*
 * Wands (adjectives and colors)
 */

static cptr wand_adj[MAX_METALS] =
{
	"Aluminium", "Cast Iron", "Chromium", "Copper", "Gold",
	"Iron", "Magnesium", "Molybdenum", "Nickel", "Rusty",
	"Silver", "Steel", "Tin", "Titanium", "Tungsten",
	"Zirconium", "Zinc", "Aluminium-Plated", "Copper-Plated", "Gold-Plated",
	"Nickel-Plated", "Silver-Plated", "Steel-Plated", "Tin-Plated", "Zinc-Plated",
	"Mithril-Plated", "Mithril", "Runed", "Bronze", "Brass",
	"Platinum", "Lead", "Lead-Plated", "Ivory" , "Adamantite",
	"Uridium", "Long", "Short", "Hexagonal"
};

static byte wand_col[MAX_METALS] =
{
	TERM_L_BLUE, TERM_L_DARK, TERM_WHITE, TERM_UMBER, TERM_YELLOW,
	TERM_SLATE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_RED,
	TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_WHITE, TERM_WHITE,
	TERM_L_WHITE, TERM_L_WHITE, TERM_L_BLUE, TERM_L_UMBER, TERM_YELLOW,
	TERM_L_UMBER, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE,
	TERM_L_BLUE, TERM_L_BLUE, TERM_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_WHITE, TERM_SLATE, TERM_SLATE, TERM_WHITE, TERM_VIOLET,
	TERM_L_RED, TERM_L_BLUE, TERM_BLUE, TERM_RED
};


/*
 * Rods (adjectives and colors).
 * Efficiency -- copied from wand arrays
 */

static cptr rod_adj[MAX_METALS];

static byte rod_col[MAX_METALS];


/*
 * Mushrooms (adjectives and colors)
 */

static cptr food_adj[MAX_SHROOM] =
{
	"Blue", "Black", "Black Spotted", "Brown", "Dark Blue",
	"Dark Green", "Dark Red", "Yellow", "Furry", "Green",
	"Grey", "Light Blue", "Light Green", "Violet", "Red",
	"Slimy", "Tan", "White", "White Spotted", "Wrinkled",
};

static byte food_col[MAX_SHROOM] =
{
	TERM_BLUE, TERM_L_DARK, TERM_L_DARK, TERM_UMBER, TERM_BLUE,
	TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_L_WHITE, TERM_GREEN,
	TERM_SLATE, TERM_L_BLUE, TERM_L_GREEN, TERM_VIOLET, TERM_RED,
	TERM_SLATE, TERM_L_UMBER, TERM_WHITE, TERM_WHITE, TERM_UMBER
};


/*
 * Color adjectives and colors, for potions.
 * Hack -- The first four entries are hard-coded.
 * (water, apple juice, slime mold juice, something)
 */

static cptr potion_adj[MAX_COLORS] =
{
	"Clear", "Light Brown", "Icky Green", "Strangely Phosphorescent",
	"Azure", "Blue", "Blue Speckled", "Black", "Brown", "Brown Speckled",
	"Bubbling", "Chartreuse", "Cloudy", "Copper Speckled", "Crimson", "Cyan",
	"Dark Blue", "Dark Green", "Dark Red", "Gold Speckled", "Green",
	"Green Speckled", "Grey", "Grey Speckled", "Hazy", "Indigo",
	"Light Blue", "Light Green", "Magenta", "Metallic Blue", "Metallic Red",
	"Metallic Green", "Metallic Purple", "Misty", "Orange", "Orange Speckled",
	"Pink", "Pink Speckled", "Puce", "Purple", "Purple Speckled",
	"Red", "Red Speckled", "Silver Speckled", "Smoky", "Tangerine",
	"Violet", "Vermilion", "White", "Yellow", "Violet Speckled",
	"Pungent", "Clotted Red", "Viscous Pink", "Oily Yellow", "Gloopy Green",
	"Shimmering", "Coagulated Crimson", "Yellow Speckled", "Gold",
	"Manly", "Stinking", "Oily Black", "Ichor", "Ivory White", "Sky Blue",
};

static byte potion_col[MAX_COLORS] =
{
	TERM_WHITE, TERM_L_UMBER, TERM_GREEN, TERM_MULTI,
	TERM_L_BLUE, TERM_BLUE, TERM_BLUE, TERM_L_DARK, TERM_UMBER, TERM_UMBER,
	TERM_L_WHITE, TERM_L_GREEN, TERM_WHITE, TERM_L_UMBER, TERM_RED, TERM_L_BLUE,
	TERM_BLUE, TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_GREEN,
	TERM_GREEN, TERM_SLATE, TERM_SLATE, TERM_L_WHITE, TERM_VIOLET,
	TERM_L_BLUE, TERM_L_GREEN, TERM_RED, TERM_BLUE, TERM_RED,
	TERM_GREEN, TERM_VIOLET, TERM_L_WHITE, TERM_ORANGE, TERM_ORANGE,
	TERM_L_RED, TERM_L_RED, TERM_VIOLET, TERM_VIOLET, TERM_VIOLET,
	TERM_RED, TERM_RED, TERM_L_WHITE, TERM_L_DARK, TERM_ORANGE,
	TERM_VIOLET, TERM_RED, TERM_WHITE, TERM_YELLOW, TERM_VIOLET,
	TERM_L_RED, TERM_RED, TERM_L_RED, TERM_YELLOW, TERM_GREEN,
	TERM_MULTI, TERM_RED, TERM_YELLOW, TERM_YELLOW,
	TERM_L_UMBER, TERM_UMBER, TERM_L_DARK, TERM_RED, TERM_WHITE, TERM_L_BLUE
};


/*
 * Syllables for scrolls (must be 1-4 letters each)
 */

static cptr syllables[MAX_SYLLABLES] =
{
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
	"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
	"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
	"byt", "comp", "con", "cos", "cre", "dalf", "dan",
	"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
	"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
	"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
	"kho", "kli", "klis", "la", "lech", "man", "mar",
	"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
	"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
	"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
	"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
	"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
	"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
	"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
	"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
	"vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
	"yerg", "yp", "zun", "tri", "blaa", "jah", "bul", "on",
	"foo", "ju", "xuxu"
};

/*
 * Hold the titles of scrolls, 6 to 14 characters each
 * Also keep an array of scroll colors (always WHITE for now)
 */

static char scroll_adj[MAX_TITLES][16];

static byte scroll_col[MAX_TITLES];


/*
 * Certain items have a flavor
 * This function is used only by "flavor_init()"
 */
static byte object_flavor(int k_idx)
{
	object_kind *k_ptr = &k_info[k_idx];

	/* Analyze the item */
	switch (k_ptr->tval)
	{
	case TV_AMULET:
		{
			return (0x80 + amulet_col[k_ptr->sval]);
		}

	case TV_RING:
		{
			return (0x90 + ring_col[k_ptr->sval]);
		}

	case TV_STAFF:
		{
			return (0xA0 + staff_col[k_ptr->sval]);
		}

	case TV_WAND:
		{
			return (0xB0 + wand_col[k_ptr->sval]);
		}

	case TV_ROD:
		{
			return (0xC0 + rod_col[k_ptr->sval]);
		}

	case TV_SCROLL:
		{
			return (0xD0 + scroll_col[k_ptr->sval]);
		}

	case TV_POTION:
	case TV_POTION2:
		{
			return (0xE0 + potion_col[k_ptr->sval]);
		}

	case TV_FOOD:
		{
			if (k_ptr->sval < SV_FOOD_MIN_FOOD)
			{
				return (0xF0 + food_col[k_ptr->sval]);
			}

			break;
		}
	}

	/* No flavor */
	return (0);
}


/*
 * Certain items, if aware, are known instantly
 * This function is used only by "flavor_init()"
 *
 * XXX XXX XXX Add "EASY_KNOW" flag to "k_info.txt" file
 */
static bool_ object_easy_know(int i)
{
	object_kind *k_ptr = &k_info[i];

	/* Analyze the "tval" */
	switch (k_ptr->tval)
	{
		/* Spellbooks */
	case TV_DRUID_BOOK:
	case TV_MUSIC_BOOK:
	case TV_SYMBIOTIC_BOOK:
		{
			return (TRUE);
		}

		/* Simple items */
	case TV_FLASK:
	case TV_EGG:
	case TV_BOTTLE:
	case TV_SKELETON:
	case TV_CORPSE:
	case TV_HYPNOS:
	case TV_SPIKE:
	case TV_JUNK:
		{
			return (TRUE);
		}

		/* All Food, Potions, Scrolls, Rods */
	case TV_FOOD:
	case TV_POTION:
	case TV_POTION2:
	case TV_SCROLL:
	case TV_ROD:
	case TV_ROD_MAIN:
		{
			if (k_ptr->flags3 & TR3_NORM_ART)
				return ( FALSE );
			return (TRUE);
		}

		/* Some Rings, Amulets, Lites */
	case TV_RING:
	case TV_AMULET:
	case TV_LITE:
		{
			if (k_ptr->flags3 & (TR3_EASY_KNOW)) return (TRUE);
			return (FALSE);
		}
	}

	/* Nope */
	return (FALSE);
}

/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first four letters (not the most efficient method, and not
 * the least efficient method, but it will always work).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 *
 * Note that the "hacked seed" may provide an RNG with alternating parity!
 */
void flavor_init(void)
{
	int i, j;

	byte temp_col;

	cptr temp_adj;


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;


	/* Efficiency -- Rods/Wands share initial array */
	for (i = 0; i < MAX_METALS; i++)
	{
		rod_adj[i] = wand_adj[i];
		rod_col[i] = wand_col[i];
	}


	/* Rings have "ring colors" */
	for (i = 0; i < MAX_ROCKS; i++)
	{
		j = rand_int(MAX_ROCKS);
		temp_adj = ring_adj[i];
		ring_adj[i] = ring_adj[j];
		ring_adj[j] = temp_adj;
		temp_col = ring_col[i];
		ring_col[i] = ring_col[j];
		ring_col[j] = temp_col;
	}

	/* Amulets have "amulet colors" */
	for (i = 0; i < MAX_AMULETS; i++)
	{
		j = rand_int(MAX_AMULETS);
		temp_adj = amulet_adj[i];
		amulet_adj[i] = amulet_adj[j];
		amulet_adj[j] = temp_adj;
		temp_col = amulet_col[i];
		amulet_col[i] = amulet_col[j];
		amulet_col[j] = temp_col;
	}

	/* Staffs */
	for (i = 0; i < MAX_WOODS; i++)
	{
		j = rand_int(MAX_WOODS);
		temp_adj = staff_adj[i];
		staff_adj[i] = staff_adj[j];
		staff_adj[j] = temp_adj;
		temp_col = staff_col[i];
		staff_col[i] = staff_col[j];
		staff_col[j] = temp_col;
	}

	/* Wands */
	for (i = 0; i < MAX_METALS; i++)
	{
		j = rand_int(MAX_METALS);
		temp_adj = wand_adj[i];
		wand_adj[i] = wand_adj[j];
		wand_adj[j] = temp_adj;
		temp_col = wand_col[i];
		wand_col[i] = wand_col[j];
		wand_col[j] = temp_col;
	}

	/* Rods */
	for (i = 0; i < MAX_METALS; i++)
	{
		j = rand_int(MAX_METALS);
		temp_adj = rod_adj[i];
		rod_adj[i] = rod_adj[j];
		rod_adj[j] = temp_adj;
		temp_col = rod_col[i];
		rod_col[i] = rod_col[j];
		rod_col[j] = temp_col;
	}

	/* Foods (Mushrooms) */
	for (i = 0; i < MAX_SHROOM; i++)
	{
		j = rand_int(MAX_SHROOM);
		temp_adj = food_adj[i];
		food_adj[i] = food_adj[j];
		food_adj[j] = temp_adj;
		temp_col = food_col[i];
		food_col[i] = food_col[j];
		food_col[j] = temp_col;
	}

	/* Potions */
	for (i = 4; i < MAX_COLORS; i++)
	{
		j = rand_int(MAX_COLORS - 4) + 4;
		temp_adj = potion_adj[i];
		potion_adj[i] = potion_adj[j];
		potion_adj[j] = temp_adj;
		temp_col = potion_col[i];
		potion_col[i] = potion_col[j];
		potion_col[j] = temp_col;
	}

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		/* Get a new title */
		while (TRUE)
		{
			std::string buf;

			/* Collect words until done */
			while (1)
			{
				/* Choose one or two syllables */
				int s = ((rand_int(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				std::string tmp;
				for (int q = 0; q < s; q++)
				{
					tmp += syllables[rand_int(MAX_SYLLABLES)];
				}

				/* Stop before getting too long */
				if (buf.size() + tmp.size() + 1 > 15)
				{
					break;
				}

				/* Add the word with separator */
				buf += " ";
				buf += tmp;
			}

			/* Save the title */
			strcpy(scroll_adj[i], buf.c_str());

			/* Assume okay */
			bool_ okay = TRUE;

			/* Check for "duplicate" scroll titles */
			for (j = 0; j < i; j++)
			{
				cptr hack1 = scroll_adj[j];
				cptr hack2 = scroll_adj[i];

				/* Compare first four characters */
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;

				/* Not okay */
				okay = FALSE;

				/* Stop looking */
				break;
			}

			/* Break when done */
			if (okay) break;
		}

		/* All scrolls are white */
		scroll_col[i] = TERM_WHITE;
	}


	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < max_k_idx; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* Extract "flavor" (if any) */
		k_ptr->flavor = object_flavor(i);

		/* No flavor yields aware */
		if ((!k_ptr->flavor) && (k_ptr->tval != TV_ROD_MAIN)) k_ptr->aware = TRUE;

		/* Check for "easily known" */
		k_ptr->easy_know = object_easy_know(i);
	}
}

/*
 * Reset the "visual" lists
 *
 * This involves resetting various things to their "default" state.
 *
 * If the "prefs" flag is TRUE, then we will also load the appropriate
 * "user pref file" based on the current setting of the "use_graphics"
 * flag.  This is useful for switching "graphics" on/off.
 *
 * The features, objects, and monsters, should all be encoded in the
 * relevant "font.pref".  XXX XXX XXX
 *
 * The "prefs" parameter is no longer meaningful.  XXX XXX XXX
 */
void reset_visuals(void)
{
	int i;

	/* Extract some info about terrain features */
	for (i = 0; i < max_f_idx; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		f_ptr->x_attr = f_ptr->d_attr;
		f_ptr->x_char = f_ptr->d_char;
	}

	/* Extract default attr/char code for stores */
	for (i = 0; i < max_st_idx; i++)
	{
		store_info_type *st_ptr = &st_info[i];

		/* Default attr/char */
		st_ptr->x_attr = st_ptr->d_attr;
		st_ptr->x_char = st_ptr->d_char;
	}

	/* Extract default attr/char code for objects */
	for (i = 0; i < max_k_idx; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Default attr/char */
		k_ptr->x_attr = k_ptr->d_attr;
		k_ptr->x_char = k_ptr->d_char;
	}

	/* Extract default attr/char code for monsters */
	for (i = 0; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Default attr/char */
		r_ptr->x_attr = r_ptr->d_attr;
		r_ptr->x_char = r_ptr->d_char;
	}

	/* Reset attr/char code for ego monster overlay graphics */
	for (i = 0; i < max_re_idx; i++)
	{
		monster_ego *re_ptr = &re_info[i];

		/* Default attr/char */
		re_ptr->g_attr = 0;
		re_ptr->g_char = 0;
	}

	/* Reset attr/char code for race modifier overlay graphics */
	for (i = 0; i < max_rmp_idx; i++)
	{
		player_race_mod *rmp_ptr = &race_mod_info[i];

		/* Default attr/char */
		rmp_ptr->g_attr = 0;
		rmp_ptr->g_char = 0;
	}

	/* Normal symbols */
	process_pref_file("font.prf");
}


/*
 * Extract "xtra" flags from object.
 */
static void object_flags_xtra(object_type const *o_ptr, u32b *f2, u32b *f3, u32b *esp)
{
	switch (o_ptr->xtra1)
	{
	case EGO_XTRA_SUSTAIN:
	{
		/* Choose a sustain */
		switch (o_ptr->xtra2 % 6)
		{
		case 0:
			(*f2) |= (TR2_SUST_STR);
			break;
		case 1:
			(*f2) |= (TR2_SUST_INT);
			break;
		case 2:
			(*f2) |= (TR2_SUST_WIS);
			break;
		case 3:
			(*f2) |= (TR2_SUST_DEX);
			break;
		case 4:
			(*f2) |= (TR2_SUST_CON);
			break;
		case 5:
			(*f2) |= (TR2_SUST_CHR);
			break;
		}
		
		break;
	}
	
	case EGO_XTRA_POWER:
	{
		/* Choose a power */
		switch (o_ptr->xtra2 % 11)
		{
		case 0:
			(*f2) |= (TR2_RES_BLIND);
			break;
		case 1:
			(*f2) |= (TR2_RES_CONF);
			break;
		case 2:
			(*f2) |= (TR2_RES_SOUND);
			break;
		case 3:
			(*f2) |= (TR2_RES_SHARDS);
			break;
		case 4:
			(*f2) |= (TR2_RES_NETHER);
			break;
		case 5:
			(*f2) |= (TR2_RES_NEXUS);
			break;
		case 6:
			(*f2) |= (TR2_RES_CHAOS);
			break;
		case 7:
			(*f2) |= (TR2_RES_DISEN);
			break;
		case 8:
			(*f2) |= (TR2_RES_POIS);
			break;
		case 9:
			(*f2) |= (TR2_RES_DARK);
			break;
		case 10:
			(*f2) |= (TR2_RES_LITE);
			break;
		}
		
		break;
	}
	
	case EGO_XTRA_ABILITY:
	{
		/* Choose an ability */
		switch (o_ptr->xtra2 % 8)
		{
		case 0:
			(*f3) |= (TR3_FEATHER);
			break;
		case 1:
			(*f3) |= (TR3_LITE1);
			break;
		case 2:
			(*f3) |= (TR3_SEE_INVIS);
			break;
		case 3:
			(*esp) |= (ESP_ALL);
			break;
		case 4:
			(*f3) |= (TR3_SLOW_DIGEST);
			break;
		case 5:
			(*f3) |= (TR3_REGEN);
			break;
		case 6:
			(*f2) |= (TR2_FREE_ACT);
			break;
		case 7:
			(*f2) |= (TR2_HOLD_LIFE);
			break;
		}
		
		break;
	}

	}
}


/*
 * Obtain the "flags" for an item
 */
bool_ object_flags_no_set = FALSE;
void object_flags(object_type const *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Base object */
	(*f1) = k_ptr->flags1;
	(*f2) = k_ptr->flags2;
	(*f3) = k_ptr->flags3;
	(*f4) = k_ptr->flags4;
	(*f5) = k_ptr->flags5;
	(*esp) = k_ptr->esp;

	/* Artifact */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];

		(*f1) = a_ptr->flags1;
		(*f2) = a_ptr->flags2;
		(*f3) = a_ptr->flags3;
		(*f4) = a_ptr->flags4;
		(*f5) = a_ptr->flags5;
		(*esp) = a_ptr->esp;

		if ((!object_flags_no_set) && (a_ptr->set != -1))
			apply_flags_set(o_ptr->name1, a_ptr->set, f1, f2, f3, f4, f5, esp);
	}

	/* Random artifact ! */
	if (o_ptr->art_flags1 || o_ptr->art_flags2 || o_ptr->art_flags3 || o_ptr->art_flags4 || o_ptr->art_flags5 || o_ptr->art_esp)
	{
		(*f1) |= o_ptr->art_flags1;
		(*f2) |= o_ptr->art_flags2;
		(*f3) |= o_ptr->art_flags3;
		(*f4) |= o_ptr->art_flags4;
		(*f5) |= o_ptr->art_flags5;
		(*esp) |= o_ptr->art_esp;
	}

	/* Extra powers */
	if (!(o_ptr->art_name))
	{
		object_flags_xtra(o_ptr, f2, f3, esp);
	}
}

/* Return object granted power */
int object_power(object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	int power = -1;

	/* Base object */
	power = k_ptr->power;

	/* Ego-item */
	if (o_ptr->name2)
	{
		ego_item_type *e_ptr = &e_info[o_ptr->name2];

		if (power == -1) power = e_ptr->power;

		if (o_ptr->name2b)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2b];

			if (power == -1) power = e_ptr->power;
		}
	}

	/* Artifact */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];

		if (power == -1) power = a_ptr->power;
	}

	return (power);
}



/*
 * Obtain the "flags" for an item which are known to the player
 */
void object_flags_known(object_type const *o_ptr, u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Clear */
	(*f1) = (*f2) = (*f3) = (*f4) = (*esp) = (*f5) = 0L;

	/* Must be identified */
	if (!object_known_p(o_ptr)) return;

	/* Base object */
	(*f1) = k_ptr->flags1;
	(*f2) = k_ptr->flags2;
	(*f3) = k_ptr->flags3;
	(*f4) = k_ptr->flags4;
	(*f5) = k_ptr->flags5;
	(*esp) = k_ptr->esp;

	(*f1) |= k_ptr->oflags1;
	(*f2) |= k_ptr->oflags2;
	(*f3) |= k_ptr->oflags3;
	(*f4) |= k_ptr->oflags4;
	(*f5) |= k_ptr->oflags5;
	(*esp) |= k_ptr->oesp;

	/* Artifact */
	if (o_ptr->name1)
	{
		artifact_type *a_ptr = &a_info[o_ptr->name1];

		/* Need full knowledge or spoilers */
		if ((o_ptr->ident & IDENT_MENTAL))
		{
			(*f1) = a_ptr->flags1;
			(*f2) = a_ptr->flags2;
			(*f3) = a_ptr->flags3;
			(*f4) = a_ptr->flags4;
			(*f5) = a_ptr->flags5;
			(*esp) = a_ptr->esp;

			if ((!object_flags_no_set) && (a_ptr->set != -1))
				apply_flags_set(o_ptr->name1, a_ptr->set, f1, f2, f3, f4, f5, esp);
		}
		else
		{
			(*f1) = (*f2) = (*f3) = (*f4) = (*esp) = (*f5) = 0L;
		}

		(*f1) |= a_ptr->oflags1;
		(*f2) |= a_ptr->oflags2;
		(*f3) |= a_ptr->oflags3;
		(*f4) |= a_ptr->oflags4;
		(*f5) |= a_ptr->oflags5;
		(*esp) |= a_ptr->oesp;
	}

	/* Random artifact or ego item! */
	if (o_ptr->art_flags1 || o_ptr->art_flags2 || o_ptr->art_flags3 || o_ptr->art_flags4 || o_ptr->art_flags5 || o_ptr->art_esp)
	{
		/* Need full knowledge or spoilers */
		if ((o_ptr->ident & IDENT_MENTAL))
		{
			(*f1) |= o_ptr->art_flags1;
			(*f2) |= o_ptr->art_flags2;
			(*f3) |= o_ptr->art_flags3;
			(*f4) |= o_ptr->art_flags4;
			(*f5) |= o_ptr->art_flags5;
			(*esp) |= o_ptr->art_esp;
		}

		(*f1) |= o_ptr->art_oflags1;
		(*f2) |= o_ptr->art_oflags2;
		(*f3) |= o_ptr->art_oflags3;
		(*f4) |= o_ptr->art_oflags4;
		(*f5) |= o_ptr->art_oflags5;
		(*esp) |= o_ptr->art_oesp;
	}

	/* Full knowledge for *identified* objects */
	if (!(o_ptr->ident & IDENT_MENTAL)) return;

	if (!(o_ptr->art_name))
	{
		object_flags_xtra(o_ptr, f2, f3, esp);
	}

	/* Hack - Res Chaos -> Res Confusion */
	if (*f2 & TR2_RES_CHAOS) (*f2) |= (TR2_RES_CONF);
}


/**
 * Calculate amount of EXP needed for the given object to
 * level, assuming it's a sentient object.
 */
s32b calc_object_need_exp(object_type const *o_ptr)
{
	return (player_exp[o_ptr->elevel - 1] * 5 / 2);
}



/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under 79 chars (plus a terminator).
 *
 * Note the use of "object_desc_num()" and "object_desc_int()" as hyper-efficient,
 * portable, versions of some common "sprintf()" commands.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * If "pref" then a "numeric" prefix will be pre-pended.
 *
 * Mode:
 *   0 -- The Cloak of Death
 *   1 -- The Cloak of Death [1,+3]
 *   2 -- The Cloak of Death [1,+3] (+2 to Stealth)
 *   3 -- The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
 */
std::string object_desc_aux(object_type *o_ptr, int pref, int mode)
{
	bool_ hack_name = FALSE;

	bool_ aware = FALSE;
	bool_ known = FALSE;

	bool_ append_name = FALSE;

	bool_ show_weapon = FALSE;
	bool_ show_armour = FALSE;

	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	/* Extract some flags */
	u32b f1, f2, f3, f4, f5, esp;
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);


	/* See if the object is "aware" */
	if (object_aware_p(o_ptr)) aware = TRUE;

	/* See if the object is "known" */
	if (object_known_p(o_ptr)) known = TRUE;

	/* Hack -- Extract the sub-type "indexx" */
	auto const indexx = o_ptr->sval;

	/* Extract default "base" string */
	std::string basenm(k_ptr->name);

	/* Assume no "modifier" string */
	std::string modstr;

	/* Analyze the object */
	switch (o_ptr->tval)
	{
		/* Some objects are easy to describe */
	case TV_SKELETON:
	case TV_BOTTLE:
	case TV_JUNK:
	case TV_SPIKE:
	case TV_FLASK:
	case TV_CHEST:
	case TV_INSTRUMENT:
	case TV_TOOL:
	case TV_DIGGING:
		{
			break;
		}


		/* Missiles/ Bows/ Weapons */
	case TV_SHOT:
	case TV_BOLT:
	case TV_ARROW:
	case TV_BOOMERANG:
	case TV_BOW:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
		{
			show_weapon = TRUE;
			break;
		}

		/* Armour */
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CROWN:
	case TV_HELM:
	case TV_SHIELD:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
		{
			show_armour = TRUE;
			break;
		}


		/* Lites (including a few "Specials") */
	case TV_LITE:
		{
			break;
		}

		/* Amulets (including a few "Specials") */
	case TV_AMULET:
		{
			/* Color the object */
			modstr = amulet_adj[indexx];
			if (aware) append_name = TRUE;

			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Amulet~";
			else
				basenm = aware ? "& # Amulet~" : "& # Amulet~";

			if (known && !o_ptr->art_name && artifact_p(o_ptr))
			{
				basenm = k_ptr->name;
			}

			break;
		}

		/* Rings (including a few "Specials") */
	case TV_RING:
		{
			/* Color the object */
			modstr = ring_adj[indexx];
			if (aware) append_name = TRUE;

			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Ring~";
			else
				basenm = aware ? "& # Ring~" : "& # Ring~";

			/* Hack -- The One Ring */
			if (!aware && (o_ptr->sval == SV_RING_POWER)) modstr = "Plain Gold";

			if (known && !o_ptr->art_name && artifact_p(o_ptr))
			{
				basenm = k_ptr->name;
			}

			break;
		}

	case TV_STAFF:
		{
			/* Color the object */
			modstr = staff_adj[o_ptr->pval2 % MAX_WOODS];
			if (aware) append_name = TRUE;
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Staff~";
			else
				basenm = "& # Staff~";
			break;
		}

	case TV_WAND:
		{
			/* Color the object */
			modstr = wand_adj[o_ptr->pval2 % MAX_METALS];
			if (aware) append_name = TRUE;
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Wand~";
			else
				basenm = "& # Wand~";
			break;
		}

	case TV_ROD:
		{
			/* Color the object */
			modstr = rod_adj[indexx];
			if (aware) append_name = TRUE;
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Rod Tip~";
			else
				basenm = aware ? "& # Rod Tip~" : "& # Rod Tip~";
			if (o_ptr->sval == SV_ROD_HOME)
			{
				basenm = "& Great Rod Tip~ of Home Summoning";
				hack_name = TRUE;
			}
			break;
		}

	case TV_ROD_MAIN:
		{
			modstr = k_info[lookup_kind(TV_ROD, o_ptr->pval)].name;
			break;
		}

	case TV_SCROLL:
		{
			/* Color the object */
			modstr = scroll_adj[indexx];
			if (aware) append_name = TRUE;
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Scroll~";
			else
				basenm = aware ? "& Scroll~ titled \"#\"" : "& Scroll~ titled \"#\"";
			break;
		}

	case TV_POTION:
	case TV_POTION2:
		{
			/* Color the object */
			if ((o_ptr->tval != TV_POTION2) || (o_ptr->sval != SV_POTION2_MIMIC) || (!aware))
			{
				modstr = potion_adj[indexx];
				if (aware) append_name = TRUE;
			}
			else
			{
				modstr = get_mimic_name(o_ptr->pval2);
			}
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Potion~";
			else
				basenm = aware ? "& # Potion~" : "& # Potion~";
			break;
		}

	case TV_FOOD:
		{
			/* Ordinary food is "boring" */
			if (o_ptr->sval >= SV_FOOD_MIN_FOOD) break;

			/* Color the object */
			modstr = food_adj[indexx];
			if (aware) append_name = TRUE;
			if (aware || o_ptr->ident & IDENT_STOREB)
				basenm = "& Mushroom~";
			else
				basenm = aware ? "& # Mushroom~" : "& # Mushroom~";
			break;
		}


		/* Cloak of Mimicry */
	case TV_CLOAK:
		{
			show_armour = TRUE;
			if (o_ptr->sval == SV_MIMIC_CLOAK)
			{
				modstr = get_mimic_object_name(o_ptr->pval2);
			}
			break;
		}


	case TV_SYMBIOTIC_BOOK:
		{
			modstr = basenm;
			basenm = "& Symbiotic Spellbook~ #";
			break;
		}

	case TV_MUSIC_BOOK:
		{
			modstr = basenm;
			basenm = "& Songbook~ #";
			break;
		}

		/* Druid Books */
	case TV_DRUID_BOOK:
		{
			modstr = basenm;
			basenm = "& Elemental Stone~ #";
			break;
		}

	case TV_PARCHMENT:
		{
			modstr = basenm;
			basenm = "& Parchment~ - #";
			break;
		}


		/* Hack -- Gold/Gems */
	case TV_GOLD:
		{
			return basenm;
		}

	case TV_CORPSE:
		{
			monster_race* r_ptr = &r_info[o_ptr->pval2];
			modstr = basenm;
			if (r_ptr->flags1 & RF1_UNIQUE)
			{
				basenm = fmt::format("& {}'s #~", r_ptr->name);
			}
			else
			{
				basenm = fmt::format("& {} #~", r_ptr->name);
			}
			break;
		}

	case TV_EGG:
		{
			monster_race* r_ptr = &r_info[o_ptr->pval2];
			modstr = basenm;
			basenm = fmt::format("& {} #~", r_ptr->name);
			break;
		}

	case TV_HYPNOS:
		{
			/* We print hit points further down. --dsb */
			monster_race* r_ptr = &r_info[o_ptr->pval];
			modstr = basenm;
			basenm = fmt::format("& {}~", r_ptr->name);
			break;
		}

	case TV_TOTEM:
		{
			monster_type monster;
			monster.r_idx = o_ptr->pval;
			monster.ego = o_ptr->pval2;
			monster.ml = TRUE;
			monster.status = MSTATUS_ENEMY;

			char name[80];
			monster_desc(name, &monster, 0x188);

			modstr = basenm;
			basenm = fmt::format("& #~ of {}", name);
			break;
		}

	case TV_RANDART:
		{
			modstr = basenm;

			if (known)
			{
				basenm = random_artifacts[indexx].name_full;
			}
			else
			{
				basenm = random_artifacts[indexx].name_short;
			}
			break;
		}

	case TV_RUNE2:
		{
			if (o_ptr->sval != RUNE_STONE)
			{
				modstr = basenm;
				basenm = "& Rune~ [#]";
			}
			break;
		}

	case TV_RUNE1:
		{
			modstr = basenm;
			basenm = "& Rune~ [#]";
			break;
		}

	case TV_DAEMON_BOOK:
	case TV_BOOK:
		{
			basenm = k_ptr->name;
			if (o_ptr->sval == 255)
			{
				modstr = spell_type_name(spell_at(o_ptr->pval));
			}
			break;
		}

		/* Used in the "inventory" routine */
	default:
		return "(nothing)";
	}

	/* Mega Hack */
	if ((!hack_name) && known && (k_ptr->flags5 & TR5_FULL_NAME))
	{
		basenm = k_ptr->name;
	}

	/* Copy of the base string _without_ a prefix */
	std::string s;

	/* Start dumping the result */
	std::string t;

	/* The object "expects" a "number" */
	if (starts_with(basenm, "&"))
	{
		cptr ego = NULL;

		monster_race* r_ptr;
		if (o_ptr->tval == TV_CORPSE)
		{
			r_ptr = &r_info[o_ptr->pval2];
		}
		else
		{
			r_ptr = &r_info[o_ptr->pval];
		}

		/* Grab any ego-item name */
		if (known && (o_ptr->name2 || o_ptr->name2b) && (o_ptr->tval != TV_ROD_MAIN))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];
			ego_item_type *e2_ptr = &e_info[o_ptr->name2b];

			if (e_ptr->before)
			{
				ego = e_ptr->name;
			}
			else if (e2_ptr->before)
			{
				ego = e2_ptr->name;
			}
		}

		/* Skip the ampersand (and space) */
		s = basenm.substr(2);

		/* No prefix */
		if (pref <= 0)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			t += "no more ";
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			t += std::to_string(o_ptr->number);
			t += ' ';
		}

		else if ((o_ptr->tval == TV_CORPSE) && (r_ptr->flags1 & RF1_UNIQUE))
		{}


		else if ((o_ptr->tval == TV_HYPNOS) && (r_ptr->flags1 & RF1_UNIQUE))
		{}

		/* Hack -- The only one of its kind */
		else if (known && (artifact_p(o_ptr) || o_ptr->art_name))
		{
			t += "The ";
		}

		else if (ego != NULL)
		{
			if (is_a_vowel(ego[0]))
			{
				t += "an ";
			}
			else
			{
				t += "a ";
			}
		}

		/* A single one, with a vowel in the modifier */
		else if ((s[0] == '#') && (is_a_vowel(modstr[0])))
		{
			t += "an ";
		}

		/* A single one, with a vowel */
		else if (is_a_vowel(s[0]))
		{
			t += "an ";
		}

		/* A single one, without a vowel */
		else
		{
			t += "a ";
		}

		/* Grab any ego-item name */
		if (known && (o_ptr->name2 || o_ptr->name2b) && (o_ptr->tval != TV_ROD_MAIN))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];
			ego_item_type *e2_ptr = &e_info[o_ptr->name2b];

			if (e_ptr->before)
			{
				t += e_ptr->name;
				t += ' ';
			}
			if (e2_ptr->before)
			{
				t += e2_ptr->name;
				t += ' ';
			}
		}

		/* -TM- Hack -- Add false-artifact names */
		/* Dagger inscribed {@w0%Smelly} will be named
		 * Smelly Dagger {@w0} */

		if (o_ptr->note)
		{
			cptr str = strchr(quark_str(o_ptr->note), '%');

			/* Add the false name */
			if (str)
			{
				t += &str[1];
				t += ' ';
			}
		}

	}

	/* Hack -- objects that "never" take an article */
	else
	{
		/* No ampersand */
		s = basenm;

		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			t += "no more ";
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			t += std::to_string(o_ptr->number);
			t += ' ';
		}

		else if (o_ptr->tval == TV_RANDART)
		{
			/* Do nothing, since randarts have their prefix already included */
		}

		/* Hack -- The only one of its kind */
		else if (known && (artifact_p(o_ptr) || o_ptr->art_name))
		{
			t += "The ";
		}

		/* Hack -- single items get no prefix */
		else
		{
			/* Nothing */
		}

		/* Grab any ego-item name */
		if (known && (o_ptr->name2 || o_ptr->name2b) && (o_ptr->tval != TV_ROD_MAIN))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];
			ego_item_type *e2_ptr = &e_info[o_ptr->name2b];

			if (e_ptr->before)
			{
				t += e_ptr->name;
				t += ' ';
			}
			if (e2_ptr->before)
			{
				t += e2_ptr->name;
				t += ' ';
			}
		}
	}

	/* Copy the string */
	for (auto const c: s)
	{
		/* Pluralizer */
		if (c == '~')
		{
			/* Add a plural if needed */
			if ((o_ptr->number != 1) && (pref >= 0))
			{
				assert(t.size() > 0);
				char k = t[t.size() - 1];

				/* XXX XXX XXX Mega-Hack */

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h')) {
					t += "e";
				}

				/* Add an 's' */
				t += "s";
			}
		}

		/* Modifier */
		else if (c == '#')
		{
			/* Grab any ego-item name */
			if (o_ptr->tval == TV_ROD_MAIN)
			{
				t += ' ';

				if (known && o_ptr->name2)
				{
					ego_item_type *e_ptr = &e_info[o_ptr->name2];
					t += e_ptr->name;
				}
			}

			/* Insert the modifier */
			t += modstr;
		}

		/* Normal */
		else
		{
			/* Copy */
			t += c;
		}
	}

	/* Append the "kind name" to the "base name" */
	if ((append_name) && (!artifact_p(o_ptr)))
	{
		t += " of ";

		if (((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF)))
		{
			t += spell_type_name(spell_at(o_ptr->pval2));
			if (mode >= 1)
			{
				s32b bonus = o_ptr->pval3 & 0xFFFF;
				s32b max = o_ptr->pval3 >> 16;
				t += fmt::format("[{:d}|{:d}]", bonus, max);
			}
		}
		else
		{
			t += k_ptr->name;
		}
	}

	/* Hack -- Append "Artifact" or "Special" names */
	if (known)
	{

		/* -TM- Hack -- Add false-artifact names */
		/* Dagger inscribed {@w0#of Smell} will be named
		 * Dagger of Smell {@w0} */
		if (o_ptr->note)
		{
			cptr str = strchr(quark_str(o_ptr->note), '#');

			/* Add the false name */
			if (str)
			{
				t += ' ';
				t += &str[1];
			}
		}

		/* Is it a new random artifact ? */
		if (o_ptr->art_name)
		{
			t += ' ';
			t += quark_str(o_ptr->art_name);
		}


		/* Grab any artifact name */
		else if (o_ptr->name1)
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			/* Unique corpses don't require another name */
			if (o_ptr->tval != TV_CORPSE)
			{
				t += ' ';
				t += a_ptr->name;
			}
		}

		/* Grab any ego-item name */
		else if ((o_ptr->name2 || o_ptr->name2b) && (o_ptr->tval != TV_ROD_MAIN))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];
			ego_item_type *e2_ptr = &e_info[o_ptr->name2b];

			if (o_ptr->name2 && !e_ptr->before)
			{
				t += ' ';
				t += e_ptr->name;
			}
			if (o_ptr->name2b && !e2_ptr->before)
			{
				t += ' ';
				t += e2_ptr->name;
			}
		}
	}

	/* It contains a spell */
	if ((known) && (f5 & TR5_SPELL_CONTAIN) && (o_ptr->pval2 != -1))
	{
		t += fmt::format(" [{}]", spell_type_name(spell_at(o_ptr->pval2)));
	}

	/* Add symbiote hp here, after the "fake-artifact" name. --dsb */
	if (o_ptr->tval == TV_HYPNOS)
	{
		t += fmt::format(" ({:d} hp)", o_ptr->pval2);
	}

	/* No more details wanted */
	if (mode < 1)
	{
		return t;
	}

	/* Hack -- Some objects can have an exp level */
	if ((f4 & TR4_LEVELS) && known)
	{
		auto need_exp = (o_ptr->elevel < PY_MAX_LEVEL)
			? std::to_string(calc_object_need_exp(o_ptr) - o_ptr->exp)
			: "*****";
		t += fmt::format(" (E:{}, L:{})", need_exp, o_ptr->elevel);
	}

	/* Hack -- Chests must be described in detail */
	if (o_ptr->tval == TV_CHEST)
	{
		/* Not searched yet */
		if (!known)
		{
			/* Nothing */
		}

		/* May be "empty" */
		else if (!o_ptr->pval)
		{
			t += " (empty)";
		}
	}


	/* Display the item like a weapon */
	if (f3 & (TR3_SHOW_MODS)) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if (o_ptr->ac) show_armour = TRUE;

	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Missiles and Weapons */
	case TV_SHOT:
	case TV_BOLT:
	case TV_ARROW:
		/* Exploding arrow? */
		if (o_ptr->pval2 != 0)
			t += " (exploding)";
		/* No break, we want to continue the description */

	case TV_BOOMERANG:
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_AXE:
	case TV_SWORD:
	case TV_DAEMON_BOOK:
		if ((o_ptr->tval == TV_DAEMON_BOOK) && (o_ptr->sval != SV_DEMONBLADE))
			break;

		/* Append a "damage" string */
		t += fmt::format(" ({:d}d{:d})", o_ptr->dd, o_ptr->ds);

		/* All done */
		break;


		/* Bows get a special "damage string" */
	case TV_BOW:
		/* Mega-Hack -- Extract the "base power" */
		s32b power = (o_ptr->sval % 10);

		/* Apply the "Extra Might" flag */
		if (f3 & (TR3_XTRA_MIGHT)) power += o_ptr->pval;

		/* Append a special "damage" string */
		t += fmt::format(" (x{:d})", power);

		/* All done */
		break;
	}


	/* Add the weapon bonuses */
	if (known)
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			t += fmt::format(" ({:+d},{:+d})", o_ptr->to_h, o_ptr->to_d);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			t += fmt::format(" ({:+d}", o_ptr->to_h);
			if (!(f3 & (TR3_HIDE_TYPE)) || o_ptr->art_name)
			{
				t += " to accuracy";
			}
			t += ')';
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			t += fmt::format(" ({:+d}", o_ptr->to_d);
			if (!(f3 & (TR3_HIDE_TYPE)) || o_ptr->art_name)
			{
				t += " to damage";
			}
			t += ')';
		}
	}


	/* Add the armor bonuses */
	if (known)
	{
		/* Show the armor class info */
		if (show_armour)
		{
			t += fmt::format(" [{:d},{:+d}]", o_ptr->ac, o_ptr->to_a);
		}

		/* No base armor, but does increase armor */
		else if (o_ptr->to_a)
		{
			t += fmt::format(" [{:+d}]", o_ptr->to_a);
		}
	}

	/* Hack -- always show base armor */
	else if (show_armour)
	{
		t += fmt::format(" [{:d}]", o_ptr->ac);
	}

	if ((f1 & TR1_MANA) && (known) && (o_ptr->pval > 0))
	{
		t += fmt::format("({:d}%)", 100 * o_ptr->pval / 5);
	}

	if ((known) && (f2 & TR2_LIFE) ) /* Can disp neg now -- Improv */
	{
		t += fmt::format("({:d}%)", 100 * o_ptr->pval / 5);
	}

	/* No more details wanted */
	if (mode < 2)
	{
		return t;
	}


	/* Hack -- Wands and Staffs have charges */
	if (known &&
	                ((o_ptr->tval == TV_STAFF) ||
	                 (o_ptr->tval == TV_WAND)))
	{
		auto plural = (o_ptr->pval != 1) ? "s" : "";
		t += fmt::format(" ({:d} charge{})", o_ptr->pval, plural);
	}

	/*
	 * Hack -- Rods have a "charging" indicator.
	 */
	else if (known && (o_ptr->tval == TV_ROD_MAIN))
	{
		t += fmt::format(" ({:d}/{:d})", o_ptr->timeout, o_ptr->pval2);
	}

	/*
	 * Hack -- Rods have a "charging" indicator.
	 */
	else if (known && (o_ptr->tval == TV_ROD))
	{
		t += fmt::format(" ({:d} Mana to cast)", o_ptr->pval);
	}

	/* Hack -- Process Lanterns/Torches */
	else if ((o_ptr->tval == TV_LITE) && (f4 & TR4_FUEL_LITE))
	{
		t += fmt::format(" (with {:d}  turns of light)", o_ptr->timeout);
	}


	/* Dump "pval" flags for wearable items */
	if (known && ((f1 & (TR1_PVAL_MASK)) || (f5 & (TR5_PVAL_MASK))))
	{
		/* Start the display */
		t += fmt::format(" ({:+d}", o_ptr->pval);

		/* Do not display the "pval" flags */
		if (f3 & (TR3_HIDE_TYPE))
		{
			/* Nothing */
		}

		/* Speed */
		else if (f1 & (TR1_SPEED))
		{
			t += " to speed";
		}

		/* Attack speed */
		else if (f1 & (TR1_BLOWS))
		{
			t += " attack";
			if (ABS(o_ptr->pval) != 1)
			{
				t += 's';
			}
		}

		/* Critical chance */
		else if (f5 & (TR5_CRIT))
		{
			t += "% of critical hits";
		}

		/* Stealth */
		else if (f1 & (TR1_STEALTH))
		{
			t += " to stealth";
		}

		/* Search */
		else if (f1 & (TR1_SEARCH))
		{
			t += " to searching";
		}

		/* Infravision */
		else if (f1 & (TR1_INFRA))
		{
			t += " to infravision";
		}

		/* Tunneling */
		else if (f1 & (TR1_TUNNEL))
		{
			/* Nothing */
		}

		/* Finish the display */
		t += ')';
	}


	/* Indicate "charging" artifacts XXX XXX XXX */
	if (known && (f3 & TR3_ACTIVATE) && o_ptr->timeout)
	{
		if (o_ptr->tval == TV_EGG)
		{
			t += " (stopped)";
		}
		else
		{
			t += " (charging)";
		}
	}

	/* Indicate "charging" Mage Staffs XXX XXX XXX */
	if (known && o_ptr->timeout && (is_ego_p(o_ptr, EGO_MSTAFF_SPELL)))
	{
		t += " (charging spell1)";
	}
	if (known && o_ptr->xtra2 && (is_ego_p(o_ptr, EGO_MSTAFF_SPELL)))
	{
		t += " (charging spell2)";
	}


	/* No more details wanted */
	if (mode < 3)
	{
		return t;
	}


	/* Inscribe */
	{
		std::vector<std::string> inscrip;

		/* Sensed stuff */
		if ((o_ptr->ident & (IDENT_SENSE)) && sense_desc[o_ptr->sense] && sense_desc[o_ptr->sense][0] != '\0')
		{
			inscrip.push_back(sense_desc[o_ptr->sense]);
		}

		/* Hack - Note "cursed" if the item is 'known' and cursed */
		if (cursed_p(o_ptr) && known && inscrip.empty())
		{
			inscrip.push_back("cursed");
		}

		/* Use the standard inscription if available */
		if (o_ptr->note)
		{
			// Chop at '#' or '%' if present. The suffix of the
			// '%' or '#' is handled elsewhere in this function.
			std::string note = quark_str(o_ptr->note);
			auto const pos = note.find_first_of("%#");
			if (pos > 0)
			{
				inscrip.push_back(note.substr(0, pos));
			}
		}

		/* Mega-Hack -- note empty wands/staffs */
		if (!known && (o_ptr->ident & (IDENT_EMPTY)))
		{
			inscrip.push_back("empty");
		}

		/* Note "tried" if the object has been tested unsuccessfully */
		if (!aware && object_tried_p(o_ptr))
		{
			inscrip.push_back("tried");
		}

		/* Note the discount, if any */
		if ((o_ptr->discount) && inscrip.empty())
		{
			inscrip.push_back(fmt::format("{:d}% off", o_ptr->discount));
		}

		/* Append the inscription, if any */
		if (!inscrip.empty())
		{
			auto inscrip_str = boost::algorithm::join(inscrip, ", ");

			/* Make sure we don't exceed 75 characters */
			t.resize(std::min<std::size_t>(t.size(), 75));

			/* Append the inscription */
			t += fmt::format(" {{{}}}", inscrip_str);
		}
	}

	return t;
}

void object_desc(char *buf, object_type *o_ptr, int pref, int mode)
{
	auto s = object_desc_aux(o_ptr, pref, mode);
	auto n = std::min<std::size_t>(s.size(), 79);
	s.copy(buf, n);
	buf[n] = '\0';
}

/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void object_desc_store(char *buf, object_type *o_ptr, int pref, int mode)
{
	/* Save the "aware" flag */
	bool_ hack_aware = k_info[o_ptr->k_idx].aware;

	/* Save the "known" flag */
	bool_ hack_known = (o_ptr->ident & (IDENT_KNOWN)) ? TRUE : FALSE;


	/* Set the "known" flag */
	o_ptr->ident |= (IDENT_KNOWN);

	/* Force "aware" for description */
	k_info[o_ptr->k_idx].aware = TRUE;


	/* Describe the object */
	object_desc(buf, o_ptr, pref, mode);


	/* Restore "aware" flag */
	k_info[o_ptr->k_idx].aware = hack_aware;

	/* Clear the known flag */
	if (!hack_known) o_ptr->ident &= ~(IDENT_KNOWN);
}




/*
 * Determine the "Activation" (if any) for an artifact
 * Return a string, or NULL for "no activation"
 */
cptr item_activation(object_type *o_ptr, byte num)
{
	u32b f1, f2, f3, f4, f5, esp;

	/* Needed hacks */
	static char rspell[2][80];

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Require activation ability */
	if (!(f3 & (TR3_ACTIVATE))) return (NULL);


	/*
	 * We need to deduce somehow that it is a random artifact -- one
	 * problem: It could be a random artifact which has NOT YET received
	 * a name. Thus we eliminate other possibilities instead of checking
	 * for art_name
	 */

	if (is_ego_p(o_ptr, EGO_MSTAFF_SPELL))
	{
		int gf, mod, mana;

		if (!num)
		{
			gf = o_ptr->pval & 0xFFFF;
			mod = o_ptr->pval3 & 0xFFFF;
			mana = o_ptr->pval2 & 0xFF;
		}
		else
		{
			gf = o_ptr->pval >> 16;
			mod = o_ptr->pval3 >> 16;
			mana = o_ptr->pval2 >> 8;
		}
		sprintf(rspell[num], "runespell(%s, %s, %d) every %d turns",
			k_info[lookup_kind(TV_RUNE1, gf)].name,
			k_info[lookup_kind(TV_RUNE2, mod)].name,
		        mana, mana * 5);
		return rspell[num];
	}

	if (o_ptr->tval == TV_EGG)
	{
		return "stop or resume the egg development";
	}

	if (o_ptr->tval == TV_INSTRUMENT)
	{
		if (!((o_ptr->name1 && a_info[o_ptr->name1].activate) ||
		                (o_ptr->name2 && e_info[o_ptr->name2].activate) ||
		                (o_ptr->name2b && e_info[o_ptr->name2b].activate)))
		{
			if (o_ptr->sval == SV_HORN)
			{
				return "aggravate monster every 100 turns";
			}
		}
	}

	return activation_aux(o_ptr, FALSE, 0);
}

/* Grab the tval desc */
static bool_ grab_tval_desc(int tval)
{
	int tv = 0;

	while (tval_descs[tv].tval && (tval_descs[tv].tval != tval))
	{
		tv++;
	}

	if (!tval_descs[tv].tval) return FALSE;

	text_out_c(TERM_L_BLUE, tval_descs[tv].desc);
	text_out("\n");

	return TRUE;
}

static void check_first(bool_ *first)
{
	if (*first) {
		*first = FALSE;
	}
	else
	{
		text_out(", ");
	}
}

/*
 * Display the damage done with a multiplier
 */
void output_dam(object_type *o_ptr, int mult, int mult2, cptr against, cptr against2, bool_ *first)
{
	int dam;

	dam = (o_ptr->dd + (o_ptr->dd * o_ptr->ds)) * 5 * mult;
	dam += (o_ptr->to_d + p_ptr->to_d + p_ptr->to_d_melee) * 10;
	dam *= p_ptr->num_blow;
	check_first(first);
	if (dam > 0)
	{
		if (dam % 10)
			text_out_c(TERM_L_GREEN, format("%d.%d", dam / 10, dam % 10));
		else
			text_out_c(TERM_L_GREEN, format("%d", dam / 10));
	}
	else
		text_out_c(TERM_L_RED, "0");
	text_out(format(" against %s", against));

	if (mult2)
	{
		dam = (o_ptr->dd + (o_ptr->dd * o_ptr->ds)) * 5 * mult2;
		dam += (o_ptr->to_d + p_ptr->to_d + p_ptr->to_d_melee) * 10;
		dam *= p_ptr->num_blow;
		check_first(first);
		if (dam > 0)
		{
			if (dam % 10)
				text_out_c(TERM_L_GREEN, format("%d.%d", dam / 10, dam % 10));
			else
				text_out_c(TERM_L_GREEN, format("%d", dam / 10));
		}
		else
			text_out_c(TERM_L_RED, "0");
		text_out(format(" against %s", against2));
	}
}

/*
 * Outputs the damage we do/would do with the weapon
 */
void display_weapon_damage(object_type *o_ptr)
{
	object_type forge, *old_ptr = &forge;
	u32b f1, f2, f3, f4, f5, esp;
	bool_ first = TRUE;
	bool_ full = o_ptr->ident & (IDENT_MENTAL);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Ok now the hackish stuff, we replace the current weapon with this one */
	object_copy(old_ptr, &p_ptr->inventory[INVEN_WIELD]);
	object_copy(&p_ptr->inventory[INVEN_WIELD], o_ptr);
	calc_bonuses(TRUE);

	text_out("\nUsing it you would have ");
	text_out_c(TERM_L_GREEN, format("%d ", p_ptr->num_blow));
	text_out(format("blow%s and do an average damage per turn of ", (p_ptr->num_blow) ? "s" : ""));

	if (full && (f1 & TR1_SLAY_ANIMAL)) output_dam(o_ptr, 2, 0, "animals", NULL, &first);
	if (full && (f1 & TR1_SLAY_EVIL)) output_dam(o_ptr, 2, 0, "evil creatures", NULL, &first);
	if (full && (f1 & TR1_SLAY_ORC)) output_dam(o_ptr, 3, 0, "orcs", NULL, &first);
	if (full && (f1 & TR1_SLAY_TROLL)) output_dam(o_ptr, 3, 0, "trolls", NULL, &first);
	if (full && (f1 & TR1_SLAY_GIANT)) output_dam(o_ptr, 3, 0, "giants", NULL, &first);
	if (full && (f1 & TR1_KILL_DRAGON)) output_dam(o_ptr, 5, 0, "dragons", NULL, &first);
	else if (full && (f1 & TR1_SLAY_DRAGON)) output_dam(o_ptr, 3, 0, "dragons", NULL, &first);
	if (full && (f5 & TR5_KILL_UNDEAD)) output_dam(o_ptr, 5, 0, "undead", NULL, &first);
	else if (full && (f1 & TR1_SLAY_UNDEAD)) output_dam(o_ptr, 3, 0, "undead", NULL, &first);
	if (full && (f5 & TR5_KILL_DEMON)) output_dam(o_ptr, 5, 0, "demons", NULL, &first);
	else if (full && (f1 & TR1_SLAY_DEMON)) output_dam(o_ptr, 3, 0, "demons", NULL, &first);

	if (full && (f1 & TR1_BRAND_FIRE)) output_dam(o_ptr, 3, 6, "non fire resistant creatures", "fire susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_COLD)) output_dam(o_ptr, 3, 6, "non cold resistant creatures", "cold susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_ELEC)) output_dam(o_ptr, 3, 6, "non lightning resistant creatures", "lightning susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_ACID)) output_dam(o_ptr, 3, 6, "non acid resistant creatures", "acid susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_POIS)) output_dam(o_ptr, 3, 6, "non poison resistant creatures", "poison susceptible creatures", &first);

	output_dam(o_ptr, 1, 0, (first) ? "all monsters" : "other monsters", NULL, &first);

	text_out(".");

	/* get our weapon back */
	object_copy(&p_ptr->inventory[INVEN_WIELD], old_ptr);
	calc_bonuses(TRUE);
}

/*
 * Display the ammo damage done with a multiplier
 */
void output_ammo_dam(object_type *o_ptr, int mult, int mult2, cptr against, cptr against2, bool_ *first)
{
	int dam;
	object_type *b_ptr = &p_ptr->inventory[INVEN_BOW];
	int is_boomerang = (o_ptr->tval == TV_BOOMERANG);
	int tmul = get_shooter_mult(b_ptr) + p_ptr->xtra_might;
	if (is_boomerang) tmul = p_ptr->throw_mult;

	dam = (o_ptr->dd + (o_ptr->dd * o_ptr->ds)) * 5;
	dam += o_ptr->to_d * 10;
	if (!is_boomerang) dam += b_ptr->to_d * 10;
	dam *= tmul;
	if (!is_boomerang) dam += (p_ptr->to_d_ranged) * 10;
	dam *= mult;
	check_first(first);
	if (dam > 0)
	{
		if (dam % 10)
			text_out_c(TERM_L_GREEN, format("%d.%d", dam / 10, dam % 10));
		else
			text_out_c(TERM_L_GREEN, format("%d", dam / 10));
	}
	else
		text_out_c(TERM_L_RED, "0");
	text_out(format(" against %s", against));

	if (mult2)
	{
		dam = (o_ptr->dd + (o_ptr->dd * o_ptr->ds)) * 5;
		dam += o_ptr->to_d * 10;
		if (!is_boomerang) dam += b_ptr->to_d * 10;
		dam *= tmul;
		if (!is_boomerang) dam += (p_ptr->to_d_ranged) * 10;
		dam *= mult2;
		check_first(first);
		if (dam > 0)
		{
			if (dam % 10)
				text_out_c(TERM_L_GREEN, format("%d.%d", dam / 10, dam % 10));
			else
				text_out_c(TERM_L_GREEN, format("%d", dam / 10));
		}
		else
			text_out_c(TERM_L_RED, "0");
		text_out(format(" against %s", against2));
	}
}

/*
 * Outputs the damage we do/would do with the current bow and this ammo
 */
void display_ammo_damage(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;
	bool_ first = TRUE;
	int i;
	bool_ full = o_ptr->ident & (IDENT_MENTAL);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (o_ptr->tval == TV_BOOMERANG)
		text_out("\nUsing it you would do an average damage per throw of ");
	else
		text_out("\nUsing it with your current shooter you would do an average damage per shot of ");
	if (full && (f1 & TR1_SLAY_ANIMAL)) output_ammo_dam(o_ptr, 2, 0, "animals", NULL, &first);
	if (full && (f1 & TR1_SLAY_EVIL)) output_ammo_dam(o_ptr, 2, 0, "evil creatures", NULL, &first);
	if (full && (f1 & TR1_SLAY_ORC)) output_ammo_dam(o_ptr, 3, 0, "orcs", NULL, &first);
	if (full && (f1 & TR1_SLAY_TROLL)) output_ammo_dam(o_ptr, 3, 0, "trolls", NULL, &first);
	if (full && (f1 & TR1_SLAY_GIANT)) output_ammo_dam(o_ptr, 3, 0, "giants", NULL, &first);
	if (full && (f1 & TR1_KILL_DRAGON)) output_ammo_dam(o_ptr, 5, 0, "dragons", NULL, &first);
	else if (full && (f1 & TR1_SLAY_DRAGON)) output_ammo_dam(o_ptr, 3, 0, "dragons", NULL, &first);
	if (full && (f5 & TR5_KILL_UNDEAD)) output_ammo_dam(o_ptr, 5, 0, "undeads", NULL, &first);
	else if (full && (f1 & TR1_SLAY_UNDEAD)) output_ammo_dam(o_ptr, 3, 0, "undeads", NULL, &first);
	if (full && (f5 & TR5_KILL_DEMON)) output_ammo_dam(o_ptr, 5, 0, "demons", NULL, &first);
	else if (full && (f1 & TR1_SLAY_DEMON)) output_ammo_dam(o_ptr, 3, 0, "demons", NULL, &first);

	if (full && (f1 & TR1_BRAND_FIRE)) output_ammo_dam(o_ptr, 3, 6, "non fire resistant creatures", "fire susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_COLD)) output_ammo_dam(o_ptr, 3, 6, "non cold resistant creatures", "cold susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_ELEC)) output_ammo_dam(o_ptr, 3, 6, "non lightning resistant creatures", "lightning susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_ACID)) output_ammo_dam(o_ptr, 3, 6, "non acid resistant creatures", "acid susceptible creatures", &first);
	if (full && (f1 & TR1_BRAND_POIS)) output_ammo_dam(o_ptr, 3, 6, "non poison resistant creatures", "poison susceptible creatures", &first);

	output_ammo_dam(o_ptr, 1, 0, (first) ? "all monsters" : "other monsters", NULL, &first);
	text_out(". ");

	if (o_ptr->pval2)
	{
		text_out("The explosion will be ");
		i = 0;
		while (gf_names[i].gf != -1)
		{
			if (gf_names[i].gf == o_ptr->pval2)
				break;
			i++;
		}
		text_out_c(TERM_L_GREEN, (gf_names[i].gf != -1) ? gf_names[i].name : "something weird");
		text_out(".");
	}
}

/*
 * Describe a magic stick powers
 */
static void describe_device(object_type *o_ptr)
{
	char buf[128];

	/* Wands/... of shcool spell */
	if (((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF)) && object_known_p(o_ptr))
	{
		/* Enter device mode  */
		set_stick_mode(o_ptr);

		// Spell reference
		auto spell = spell_at(o_ptr->pval2);

		text_out("\nSpell description:\n");
		spell_type_description_foreach(spell,
					       [] (std::string const &text) -> void {
						       text_out("\n");
						       text_out(text.c_str());
					       });

		text_out("\nSpell level: ");
		sprintf(buf, FMTs32b, get_level(o_ptr->pval2, 50));
		text_out_c(TERM_L_BLUE, buf);

		text_out("\nMinimum Magic Device level to increase spell level: ");
		text_out_c(TERM_L_BLUE, format("%d", spell_type_skill_level(spell)));

		text_out("\nSpell fail: ");
		sprintf(buf, FMTs32b, spell_chance_device(spell));
		text_out_c(TERM_GREEN, buf);

		text_out("\nSpell info: ");
		text_out_c(TERM_YELLOW, spell_type_info(spell));

		/* Leave device mode  */
		unset_stick_mode();

		text_out("\n");
	}
}


/*
 * Helper for object_out_desc
 *
 * Print the level something was found on
 *
 */
static cptr object_out_desc_where_found(s16b level, s16b dungeon)
{
	static char str[80];

	if (dungeon == DUNGEON_WILDERNESS)
	{
		/* Taking care of older objects */
		if (level == 0)
		{
			sprintf(str, "in the wilderness or in a town");
		}
		else if (wf_info[level].terrain_idx == TERRAIN_TOWN)
		{
			sprintf(str, "in the town of %s", wf_info[level].name);
		}
		else
		{
			sprintf(str, "in %s", wf_info[level].text);
		}
	}
	else
	{
		sprintf(str, "on level %d of %s", level, d_info[dungeon].name);
	}

	return str;
}

/*
 * Describe an item
 */
bool_ object_out_desc(object_type *o_ptr, FILE *fff, bool_ trim_down, bool_ wait_for_it)
{
	u32b f1, f2, f3, f4, f5, esp;

	cptr vp[64];
	byte vc[64];
	int vn;

	/* Extract the flags */
	if ((!(o_ptr->ident & (IDENT_MENTAL))) && (!fff))
	{
		f1 = o_ptr->art_oflags1;
		f2 = o_ptr->art_oflags2;
		f3 = o_ptr->art_oflags3;
		f4 = o_ptr->art_oflags4;
		f5 = o_ptr->art_oflags5;
		esp = o_ptr->art_oesp;
	}
	else
	{
		object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);
	}

	if (fff)
	{
		/* Set up stuff for text_out */
		text_out_file = fff;
		text_out_hook = text_out_to_file;
	}
	else
	{
		/* Save the screen */
		character_icky = TRUE;
		Term_save();

		/* Set up stuff for text_out */
		text_out_hook = text_out_to_screen;
		text_out("\n");
	}

	/* No need to dump that */
	if (!fff)
	{
		if (!trim_down) grab_tval_desc(o_ptr->tval);
	}

	if (object_known_p(o_ptr))
	{
		if (o_ptr->k_idx && (!trim_down))
		{
			object_kind *k_ptr = &k_info[o_ptr->k_idx];

			text_out_c(TERM_ORANGE, k_ptr->text);
			text_out("\n");
		}

		if (o_ptr->name1 && (!trim_down))
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			text_out_c(TERM_YELLOW, a_ptr->text);
			text_out("\n");

			if (a_ptr->set != -1)
			{
				text_out_c(TERM_GREEN, set_info[a_ptr->set].desc);
				text_out("\n");
			}
		}

		if ((f4 & TR4_LEVELS) && (!trim_down))
		{
			int j = 0;

			if (count_bits(o_ptr->pval3) == 0) text_out("It is sentient");
			else if (count_bits(o_ptr->pval3) > 1) text_out("It is sentient and can have access to the realms of ");
			else text_out("It is sentient and can have access to the realm of ");

			bool_ first = TRUE;
			for (j = 0; j < MAX_FLAG_GROUP; j++)
			{
				if (BIT(j) & o_ptr->pval3)
				{
					check_first(&first);
					text_out_c(flags_groups[j].color, flags_groups[j].name);
				}
			}

			text_out(".  ");
		}

		if (f4 & TR4_ULTIMATE)
		{
			if ((wield_slot(o_ptr) == INVEN_WIELD) ||
			                (wield_slot(o_ptr) == INVEN_BOW))
				text_out_c(TERM_VIOLET, "It is part of the trinity of the ultimate weapons.  ");
			else
				text_out_c(TERM_VIOLET, "It is the ultimate armor.  ");
		}

		if (f4 & TR4_COULD2H) text_out("It can be wielded two-handed.  ");
		if (f4 & TR4_MUST2H) text_out("It must be wielded two-handed.  ");

		/* Mega-Hack -- describe activation */
		if (f3 & (TR3_ACTIVATE))
		{
			text_out("It can be activated for ");
			if (is_ego_p(o_ptr, EGO_MSTAFF_SPELL))
			{
				text_out(item_activation(o_ptr, 0));
				text_out(" and ");
				text_out(item_activation(o_ptr, 1));
			}
			else
				text_out(item_activation(o_ptr, 0));

			/* Mega-hack -- get rid of useless line for e.g. randarts */
			if (f5 & (TR5_ACTIVATE_NO_WIELD))
				text_out(".  ");
			else
				text_out(" if it is being worn. ");
		}
		/* Granted power */
		if (object_power(o_ptr) != -1)
		{
			text_out("It grants you the power of ");
			text_out(powers_type[object_power(o_ptr)].name);
			text_out(" if it is being worn.  ");
		}

		/* Hack -- describe lites */
		if ((o_ptr->tval == TV_LITE) || (f3 & TR3_LITE1) || (f4 & TR4_LITE2) || (f4 & TR4_LITE3))
		{
			int radius = 0;

			if (f3 & TR3_LITE1) radius++;
			if (f4 & TR4_LITE2) radius += 2;
			if (f4 & TR4_LITE3) radius += 3;
			if (radius > 5) radius = 5;

			if (f4 & TR4_FUEL_LITE)
			{
				text_out(format("It provides light (radius %d) when fueled.  ", radius));
			}
			else
			{
				text_out(format("It provides light (radius %d) forever.  ", radius));
			}
		}

		/* Mega Hack^3 -- describe the Anchor of Space-time */
		if (o_ptr->name1 == ART_ANCHOR)
		{
			text_out("It prevents the space-time continuum from being disrupted.  ");
		}

		if (f4 & TR4_ANTIMAGIC_50)
		{
			text_out("It generates an antimagic field.  ");
		}

		if (f5 & TR5_SPELL_CONTAIN)
		{
			if (o_ptr->pval2 == -1)
				text_out("It can be used to store a spell.  ");
			else
				text_out("It has a spell stored inside.  ");
		}

		/* Pick up stat bonuses */
		vn = 0;
		if (f1 & (TR1_STR)) vp[vn++] = "strength";
		if (f1 & (TR1_INT)) vp[vn++] = "intelligence";
		if (f1 & (TR1_WIS)) vp[vn++] = "wisdom";
		if (f1 & (TR1_DEX)) vp[vn++] = "dexterity";
		if (f1 & (TR1_CON)) vp[vn++] = "constitution";
		if (f1 & (TR1_CHR)) vp[vn++] = "charisma";
		if (f1 & (TR1_STEALTH)) vp[vn++] = "stealth";
		if (f1 & (TR1_SEARCH)) vp[vn++] = "searching";
		if (f1 & (TR1_INFRA)) vp[vn++] = "infravision";
		if (f1 & (TR1_TUNNEL)) vp[vn++] = "ability to tunnel";
		if (f1 & (TR1_SPEED)) vp[vn++] = "speed";
		if (f1 & (TR1_BLOWS)) vp[vn++] = "attack speed";
		if (f5 & (TR5_CRIT)) vp[vn++] = "ability to score critical hits";
		if (f5 & (TR5_LUCK)) vp[vn++] = "luck";
		if (f1 & (TR1_SPELL)) vp[vn++] = "spell power";

		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It ");

			/* What it does */
			if (o_ptr->pval > 0) text_out("increases ");
			else text_out("decreases ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("your ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out(vp[i]);
			}

			text_out(" by ");
			if (o_ptr->pval > 0)
				text_out_c(TERM_L_GREEN, format("%i", o_ptr->pval));
			else
				text_out_c(TERM_L_RED, format("%i", -o_ptr->pval));
			text_out(".  ");
		}


		vn = 0;
		if (f1 & (TR1_MANA)) vp[vn++] = "mana capacity";
		if (f2 & (TR2_LIFE)) vp[vn++] = "hit points";

		/* Describe with percentuals */
		if (vn)
		{
			int percent;

			/* What it does */
			if (o_ptr->pval > 0)
				text_out("It increases");
			else
				text_out("It decreases");

			text_out(" your ");
			text_out(vp[0]);
			if (vn == 2)
			{
				text_out(" and ");
				text_out(vp[1]);
			}

			text_out(" by ");
			percent = 100 * o_ptr->pval / 5;


			if (o_ptr->pval > 0)
				text_out_c(TERM_L_GREEN, format("%i%%", percent));
			else
				text_out_c(TERM_L_RED, format("%i%%", -percent));
			text_out(".  ");
		}

		vn = 0;
		if (f1 & (TR1_BRAND_ACID))
		{
			vc[vn] = TERM_GREEN;
			vp[vn++] = "acid";
		}
		if (f1 & (TR1_BRAND_ELEC))
		{
			vc[vn] = TERM_L_BLUE;
			vp[vn++] = "electricity";
		}
		if (f1 & (TR1_BRAND_FIRE))
		{
			vc[vn] = TERM_RED;
			vp[vn++] = "fire";
		}
		if (f1 & (TR1_BRAND_COLD))
		{
			vc[vn] = TERM_L_WHITE;
			vp[vn++] = "frost";
		}
		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It does extra damage ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("from ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out_c(vc[i], vp[i]);
			}
			text_out(".  ");
		}


		if (f1 & (TR1_BRAND_POIS))
		{
			text_out("It ");
			text_out_c(TERM_L_GREEN, "poisons your foes");
			text_out(".  ");
		}

		if (f1 & (TR1_CHAOTIC))
		{
			text_out("It produces chaotic effects.  ");
		}

		if (f1 & (TR1_VAMPIRIC))
		{
			text_out("It drains life from your foes.  ");
		}

		if (f1 & (TR1_IMPACT))
		{
			text_out("It can cause earthquakes.  ");
		}

		if (f1 & (TR1_VORPAL))
		{
			text_out("It is very sharp and can cut your foes.  ");
		}

		if (f5 & (TR5_WOUNDING))
		{
			text_out("It is very sharp and can make your foes bleed.  ");
		}

		if (f1 & (TR1_KILL_DRAGON))
		{
			text_out("It is a great bane of dragons.  ");
		}
		else if (f1 & (TR1_SLAY_DRAGON))
		{
			text_out("It is especially deadly against dragons.  ");
		}
		if (f1 & (TR1_SLAY_ORC))
		{
			text_out("It is especially deadly against orcs.  ");
		}
		if (f1 & (TR1_SLAY_TROLL))
		{
			text_out("It is especially deadly against trolls.  ");
		}
		if (f1 & (TR1_SLAY_GIANT))
		{
			text_out("It is especially deadly against giants.  ");
		}
		if (f5 & (TR5_KILL_DEMON))
		{
			text_out("It is a great bane of demons.  ");
		}
		else if (f1 & (TR1_SLAY_DEMON))
		{
			text_out("It strikes at demons with holy wrath.  ");
		}
		if (f5 & (TR5_KILL_UNDEAD))
		{
			text_out("It is a great bane of undead.  ");
		}
		else if (f1 & (TR1_SLAY_UNDEAD))
		{
			text_out("It strikes at undead with holy wrath.  ");
		}
		if (f1 & (TR1_SLAY_EVIL))
		{
			text_out("It fights against evil with holy fury.  ");
		}
		if (f1 & (TR1_SLAY_ANIMAL))
		{
			text_out("It is especially deadly against natural creatures.  ");
		}

		if (f2 & (TR2_INVIS))
		{
			text_out("It makes you invisible.  ");
		}

		vn = 0;
		if (f2 & (TR2_SUST_STR))
		{
			vp[vn++] = "strength";
		}
		if (f2 & (TR2_SUST_INT))
		{
			vp[vn++] = "intelligence";
		}
		if (f2 & (TR2_SUST_WIS))
		{
			vp[vn++] = "wisdom";
		}
		if (f2 & (TR2_SUST_DEX))
		{
			vp[vn++] = "dexterity";
		}
		if (f2 & (TR2_SUST_CON))
		{
			vp[vn++] = "constitution";
		}
		if (f2 & (TR2_SUST_CHR))
		{
			vp[vn++] = "charisma";
		}
		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It sustains ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("your ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out(vp[i]);
			}
			text_out(".  ");
		}

		vn = 0;
		if (f2 & (TR2_IM_ACID))
		{
			vc[vn] = TERM_GREEN;
			vp[vn++] = "acid";
		}
		if (f2 & (TR2_IM_ELEC))
		{
			vc[vn] = TERM_L_BLUE;
			vp[vn++] = "electricity";
		}
		if (f2 & (TR2_IM_FIRE))
		{
			vc[vn] = TERM_RED;
			vp[vn++] = "fire";
		}
		if (f2 & (TR2_IM_COLD))
		{
			vc[vn] = TERM_L_WHITE;
			vp[vn++] = "cold";
		}
		if (f4 & (TR4_IM_NETHER))
		{
			vc[vn] = TERM_L_GREEN;
			vp[vn++] = "nether";
		}
		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It provides immunity ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("to ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out_c(vc[i], vp[i]);
			}
			text_out(".  ");
		}

		if (f2 & (TR2_FREE_ACT))
		{
			text_out("It provides immunity to paralysis.  ");
		}
		if (f2 & (TR2_RES_FEAR))
		{
			text_out("It makes you completely fearless.  ");
		}

		vn = 0;
		if (f2 & (TR2_HOLD_LIFE))
		{
			vp[vn++] = "life draining";
		}
		if ((f2 & (TR2_RES_ACID)) && !(f2 & (TR2_IM_ACID)))
		{
			vp[vn++] = "acid";
		}
		if ((f2 & (TR2_RES_ELEC)) && !(f2 & (TR2_IM_ELEC)))
		{
			vp[vn++] = "electricity";
		}
		if ((f2 & (TR2_RES_FIRE)) && !(f2 & (TR2_IM_FIRE)))
		{
			vp[vn++] = "fire";
		}
		if ((f2 & (TR2_RES_COLD)) && !(f2 & (TR2_IM_COLD)))
		{
			vp[vn++] = "cold";
		}
		if (f2 & (TR2_RES_POIS))
		{
			vp[vn++] = "poison";
		}
		if (f2 & (TR2_RES_LITE))
		{
			vp[vn++] = "light";
		}
		if (f2 & (TR2_RES_DARK))
		{
			vp[vn++] = "dark";
		}
		if (f2 & (TR2_RES_BLIND))
		{
			vp[vn++] = "blindness";
		}
		if (f2 & (TR2_RES_CONF))
		{
			vp[vn++] = "confusion";
		}
		if (f2 & (TR2_RES_SOUND))
		{
			vp[vn++] = "sound";
		}
		if (f2 & (TR2_RES_SHARDS))
		{
			vp[vn++] = "shards";
		}
		if ((f2 & (TR2_RES_NETHER)) && !(f4 & (TR4_IM_NETHER)))
		{
			vp[vn++] = "nether";
		}
		if (f2 & (TR2_RES_NEXUS))
		{
			vp[vn++] = "nexus";
		}
		if (f2 & (TR2_RES_CHAOS))
		{
			vp[vn++] = "chaos";
		}
		if (f2 & (TR2_RES_DISEN))
		{
			vp[vn++] = "disenchantment";
		}
		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It provides resistance ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("to ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out(vp[i]);
			}
			text_out(".  ");
		}

		if (f2 & (TR2_SENS_FIRE))
		{
			text_out("It renders you especially vulnerable to fire.  ");
		}
		if (f3 & (TR3_WRAITH))
		{
			text_out("It renders you incorporeal.  ");
		}
		if (f5 & (TR5_WATER_BREATH))
		{
			text_out("It allows you to breathe underwater.  ");
		}
		if (f5 & (TR5_MAGIC_BREATH))
		{
			text_out("It allows you to breathe without air.  ");
		}
		if (f3 & (TR3_FEATHER))
		{
			text_out("It allows you to levitate.  ");
		}
		if (f4 & (TR4_FLY))
		{
			text_out("It allows you to fly.  ");
		}
		if (f4 & (TR4_CLIMB))
		{
			text_out("It allows you to climb mountains.  ");
		}
		if (f5 & (TR5_IMMOVABLE))
		{
			text_out("It renders you immovable.  ");
		}
		if (f3 & (TR3_SEE_INVIS))
		{
			text_out("It allows you to see invisible monsters.  ");
		}
		if (esp)
		{
			if (esp & ESP_ALL) text_out("It gives telepathic powers.  ");
			else
			{
				vn = 0;
				if (esp & ESP_ORC) vp[vn++] = "orcs";
				if (esp & ESP_TROLL) vp[vn++] = "trolls";
				if (esp & ESP_DRAGON) vp[vn++] = "dragons";
				if (esp & ESP_SPIDER) vp[vn++] = "spiders";
				if (esp & ESP_GIANT) vp[vn++] = "giants";
				if (esp & ESP_DEMON) vp[vn++] = "demons";
				if (esp & ESP_UNDEAD) vp[vn++] = "undead";
				if (esp & ESP_EVIL) vp[vn++] = "evil beings";
				if (esp & ESP_ANIMAL) vp[vn++] = "animals";
				if (esp & ESP_THUNDERLORD) vp[vn++] = "thunderlords";
				if (esp & ESP_GOOD) vp[vn++] = "good beings";
				if (esp & ESP_NONLIVING) vp[vn++] = "non-living things";
				if (esp & ESP_UNIQUE) vp[vn++] = "unique beings";
				/* Describe */
				if (vn)
				{
					int i;

					/* Intro */
					text_out("It allows you to sense the presence ");

					/* List */
					for (i = 0; i < vn; i++)
					{
						/* Connectives */
						if (i == 0) text_out("of ");
						else if (i < (vn - 1)) text_out(", ");
						else text_out(" and ");

						/* Dump the stat */
						text_out(vp[i]);
					}
					text_out(".  ");
				}
			}
		}

		if (f3 & (TR3_SLOW_DIGEST))
		{
			text_out("It slows your metabolism.  ");
		}
		if (f3 & (TR3_REGEN))
		{
			text_out("It speeds your regenerative powers.  ");
		}
		if (f2 & (TR2_REFLECT))
		{
			text_out("It reflects bolts and arrows.  ");
		}
		if (f3 & (TR3_SH_FIRE))
		{
			text_out("It produces a fiery sheath.  ");
		}
		if (f3 & (TR3_SH_ELEC))
		{
			text_out("It produces an electric sheath.  ");
		}
		if (f3 & (TR3_NO_MAGIC))
		{
			text_out("It produces an anti-magic shell.  ");
		}
		if (f3 & (TR3_NO_TELE))
		{
			text_out("It prevents teleportation.  ");
		}
		if (f3 & (TR3_XTRA_MIGHT))
		{
			text_out("It fires missiles with extra might.  ");
		}
		if (f3 & (TR3_XTRA_SHOTS))
		{
			text_out("It fires missiles excessively fast.  ");
		}

		vn = 0;
		if (f5 & (TR5_DRAIN_MANA))
		{
			vc[vn] = TERM_BLUE;
			vp[vn++] = "mana";
		}
		if (f5 & (TR5_DRAIN_HP))
		{
			vc[vn] = TERM_RED;
			vp[vn++] = "life";
		}
		if (f3 & (TR3_DRAIN_EXP))
		{
			vc[vn] = TERM_L_DARK;
			vp[vn++] = "experience";
		}
		/* Describe */
		if (vn)
		{
			int i;

			/* Intro */
			text_out("It ");

			/* List */
			for (i = 0; i < vn; i++)
			{
				/* Connectives */
				if (i == 0) text_out("drains ");
				else if (i < (vn - 1)) text_out(", ");
				else text_out(" and ");

				/* Dump the stat */
				text_out_c(vc[i], vp[i]);
			}
			text_out(".  ");
		}

		if (f3 & (TR3_BLESSED))
		{
			text_out("It has been blessed by the gods.  ");
		}
		if (f4 & (TR4_AUTO_ID))
		{
			text_out("It identifies all items for you.  ");
		}

		if (f3 & (TR3_TELEPORT))
		{
			text_out("It induces random teleportation.  ");
		}
		if (f3 & (TR3_AGGRAVATE))
		{
			text_out("It aggravates nearby creatures.  ");
		}
		if (f4 & (TR4_NEVER_BLOW))
		{
			text_out("It can't attack.  ");
		}
		if (f4 & (TR4_BLACK_BREATH))
		{
			text_out("It fills you with the Black Breath.  ");
		}
		if (cursed_p(o_ptr))
		{
			if (f3 & (TR3_PERMA_CURSE))
			{
				text_out("It is permanently cursed.  ");
			}
			else if (f3 & (TR3_HEAVY_CURSE))
			{
				text_out("It is heavily cursed.  ");
			}
			else
			{
				text_out("It is cursed.  ");
			}
		}
		if (f3 & (TR3_TY_CURSE))
		{
			text_out("It carries an ancient foul curse.  ");
		}

		if (f4 & (TR4_DG_CURSE))
		{
			text_out("It carries an ancient Morgothian curse.  ");
		}
		if (f4 & (TR4_CLONE))
		{
			text_out("It can clone monsters.  ");
		}
		if (f4 & (TR4_CURSE_NO_DROP))
		{
			text_out("It cannot be dropped while cursed.  ");
		}
		if (f3 & (TR3_AUTO_CURSE))
		{
			text_out("It can re-curse itself.  ");
		}

		if (f4 & (TR4_CAPACITY))
		{
			text_out("It can hold more mana.  ");
		}
		if (f4 & (TR4_CHEAPNESS))
		{
			text_out("It can cast spells for a lesser mana cost.  ");
		}
		if (f4 & (TR4_FAST_CAST))
		{
			text_out("It can cast spells faster.  ");
		}
		if (f4 & (TR4_CHARGING))
		{
			text_out("It regenerates its mana faster.  ");
		}

		if (f5 & (TR5_RES_MORGUL))
		{
			text_out("It can resist being shattered by morgul beings.  ");
		}
		if ((f3 & (TR3_IGNORE_ACID)) && (f3 & (TR3_IGNORE_FIRE)) && (f3 & (TR3_IGNORE_COLD)) && (f3 & (TR3_IGNORE_ELEC)))
		{
			text_out("It cannot be harmed by acid, cold, lightning or fire.  ");
		}
		else
		{
			if (f3 & (TR3_IGNORE_ACID))
			{
				text_out("It cannot be harmed by acid.  ");
			}
			if (f3 & (TR3_IGNORE_ELEC))
			{
				text_out("It cannot be harmed by electricity.  ");
			}
			if (f3 & (TR3_IGNORE_FIRE))
			{
				text_out("It cannot be harmed by fire.  ");
			}
			if (f3 & (TR3_IGNORE_COLD))
			{
				text_out("It cannot be harmed by cold.  ");
			}
		}
	}


	if (!trim_down && !fff)
	{
		describe_device(o_ptr);

		if (object_known_p(o_ptr))
		{
			/* Damage display for weapons */
			if (wield_slot(o_ptr) == INVEN_WIELD)
				display_weapon_damage(o_ptr);

			/* Breakage/Damage display for boomerangs */
			if (o_ptr->tval == TV_BOOMERANG)
			{
				if (artifact_p(o_ptr))
					text_out("\nIt can never be broken.");
				else
					text_out("\nIt has 1% chance to break upon hit.");
				display_ammo_damage(o_ptr);
			}

			/* Breakage/Damage display for ammo */
			if (wield_slot(o_ptr) == INVEN_AMMO)
			{
				if (artifact_p(o_ptr))
				{
					text_out("\nIt can never be broken.");
				}
				/* Exclude exploding arrows */
				else if (o_ptr->pval2 == 0)
				{
					text_out("\nIt has ");
					text_out_c(TERM_L_RED, format("%d", breakage_chance(o_ptr)));
					text_out("% chance to break upon hit.");
				}
				display_ammo_damage(o_ptr);
			}

			/* Monster recall for totems and corpses */
			if (o_ptr->tval == TV_TOTEM)
			{
				monster_description_out(o_ptr->pval, 0);
			}
			if (o_ptr->tval == TV_CORPSE)
			{
				monster_description_out(o_ptr->pval2, 0);
			}
		}

		if (!object_known_p(o_ptr))
			text_out("\nYou might need to identify the item to know some more about it...");
		else if (!(o_ptr->ident & (IDENT_MENTAL)))
			text_out("\nYou might need to *identify* the item to know more about it...");
	}

	/* Copying how others seem to do it. -- neil */
	if (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET ||
	                !trim_down || (ego_item_p(o_ptr)) || (artifact_p(o_ptr)))
	{
		/* Where did we found it ? */
		if (o_ptr->found == OBJ_FOUND_MONSTER)
		{
			char m_name[80];

			monster_race_desc(m_name, o_ptr->found_aux1, o_ptr->found_aux2);
			text_out(format("\nYou found it in the remains of %s %s.",
			                m_name, object_out_desc_where_found(o_ptr->found_aux4, o_ptr->found_aux3)));
		}
		else if (o_ptr->found == OBJ_FOUND_FLOOR)
		{
			text_out(format("\nYou found it lying on the ground %s.",
			                object_out_desc_where_found(o_ptr->found_aux2, o_ptr->found_aux1)));
		}
		else if (o_ptr->found == OBJ_FOUND_VAULT)
		{
			text_out(format("\nYou found it lying in a vault %s.",
			                object_out_desc_where_found(o_ptr->found_aux2, o_ptr->found_aux1)));
		}
		else if (o_ptr->found == OBJ_FOUND_SPECIAL)
		{
			text_out("\nYou found it lying on the floor of a special level.");
		}
		else if (o_ptr->found == OBJ_FOUND_RUBBLE)
		{
			text_out("\nYou found it while digging a rubble.");
		}
		else if (o_ptr->found == OBJ_FOUND_REWARD)
		{
			text_out("\nIt was given to you as a reward.");
		}
		else if (o_ptr->found == OBJ_FOUND_STORE)
		{
			text_out(format("\nYou bought it from the %s.",
					st_info[o_ptr->found_aux1].name));
		}
		else if (o_ptr->found == OBJ_FOUND_STOLEN)
		{
			text_out(format("\nYou stole it from the %s.",
					st_info[o_ptr->found_aux1].name));
		}
		else if (o_ptr->found == OBJ_FOUND_SELFMADE)
		{
			text_out("\nYou made it yourself.");
		}
		/* useful for debugging
		else
	{
			text_out("\nYou ordered it from a catalog in the Town.");
	}*/
	}

	if (fff)
	{
		/* Flush the line position. */
		text_out("\n");
		text_out_file = NULL;
	}
	else
	{
		if (wait_for_it)
		{
			/* Wait for it */
			inkey();

			/* Restore the screen */
			Term_load();
		}
		character_icky = FALSE;

	}

	/* Reset stuff for text_out */
	text_out_hook = text_out_to_screen;

	/* Gave knowledge */
	return (TRUE);
}



/*
 * Convert an inventory index into a one character label
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven"
 * Return "-1" if the label does not indicate a real item
 */
static s16b label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return ( -1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].k_idx) return ( -1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
static s16b label_to_equip(int c)
{
	int i;

	/* Convert */
	i = ((islower(c) || (c > 'z')) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return ( -1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].k_idx) return ( -1);

	/* Return the index */
	return (i);
}

/*
 * Returns the next free slot of the given "type", return the first
 * if all are used
 */
static int get_slot(int slot)
{
	int i = 0;

	/* If there are at least one body part corretsonding, the find the free one */
	if (p_ptr->body_parts[slot - INVEN_WIELD] == slot)
	{
		/* Find a free body part */
		while ((i < 6) && (slot + i < INVEN_TOTAL) && (p_ptr->body_parts[slot - INVEN_WIELD + i] == slot))
		{
			if (p_ptr->body_parts[slot + i - INVEN_WIELD])
			{
				/* Free ? return the slot */
				if (!p_ptr->inventory[slot + i].k_idx) return (slot + i);
			}
			else break;

			i++;
		}
		/* Found nothing ? return the first one */
		return slot;
	}
	/* No body parts ? return -1 */
	else return ( -1);
}

/*
 * Determine which equipment slot (if any) an item likes, ignoring the player's
 * current body and stuff if ideal == TRUE
 */
s16b wield_slot_ideal(object_type const *o_ptr, bool_ ideal)
{
	/* Theme has restrictions for winged races. */
	if (game_module_idx == MODULE_THEME)
	{
		cptr race_name = rp_ptr->title;

		if (streq(race_name, "Dragon") ||
		    streq(race_name, "Eagle"))
		{
			switch (o_ptr->tval)
			{
			case TV_CLOAK:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
				return -1;
			}
		}
	}

	/* Slot for equipment */
	switch (o_ptr->tval)
	{
	case TV_DIGGING:
	case TV_TOOL:
		{
			return ideal ? INVEN_TOOL : get_slot(INVEN_TOOL);
		}

	case TV_HAFTED:
	case TV_POLEARM:
	case TV_MSTAFF:
	case TV_SWORD:
	case TV_AXE:
		{
			return ideal ? INVEN_WIELD : get_slot(INVEN_WIELD);
		}

	case TV_BOOMERANG:
	case TV_BOW:
	case TV_INSTRUMENT:
		{
			return ideal ? INVEN_BOW : get_slot(INVEN_BOW);
		}

	case TV_RING:
		{
			return ideal ? INVEN_RING : get_slot(INVEN_RING);
		}

	case TV_AMULET:
		{
			return ideal ? INVEN_NECK : get_slot(INVEN_NECK);
		}

	case TV_LITE:
		{
			return ideal ? INVEN_LITE : get_slot(INVEN_LITE);
		}

	case TV_DRAG_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
		{
			return ideal ? INVEN_BODY : get_slot(INVEN_BODY);
		}

	case TV_CLOAK:
		{
			return ideal ? INVEN_OUTER : get_slot(INVEN_OUTER);
		}

	case TV_SHIELD:
		{
			return ideal ? INVEN_ARM : get_slot(INVEN_ARM);
		}

	case TV_CROWN:
	case TV_HELM:
		{
			return ideal ? INVEN_HEAD : get_slot(INVEN_HEAD);
		}

	case TV_GLOVES:
		{
			return ideal ? INVEN_HANDS : get_slot(INVEN_HANDS);
		}

	case TV_BOOTS:
		{
			return ideal ? INVEN_FEET : get_slot(INVEN_FEET);
		}

	case TV_HYPNOS:
		{
			return ideal ? INVEN_CARRY : get_slot(INVEN_CARRY);
		}

	case TV_SHOT:
		{
			if (ideal)
			{
				return INVEN_AMMO;
			}
			else if (p_ptr->inventory[INVEN_AMMO].k_idx &&
			         object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]) &&
			         p_ptr->inventory[INVEN_AMMO].number + o_ptr->number < MAX_STACK_SIZE)
			{
				return get_slot(INVEN_AMMO);
			}
			else if ((p_ptr->inventory[INVEN_BOW].k_idx) && (p_ptr->inventory[INVEN_BOW].tval == TV_BOW))
			{
				if (p_ptr->inventory[INVEN_BOW].sval < 10)
					return get_slot(INVEN_AMMO);
			}
			return -1;
		}

	case TV_ARROW:
		{
			if (ideal)
			{
				return INVEN_AMMO;
			}
			else if (p_ptr->inventory[INVEN_AMMO].k_idx &&
			         object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]) &&
			         p_ptr->inventory[INVEN_AMMO].number + o_ptr->number < MAX_STACK_SIZE)
			{
				return get_slot(INVEN_AMMO);
			}
			else if ((p_ptr->inventory[INVEN_BOW].k_idx) && (p_ptr->inventory[INVEN_BOW].tval == TV_BOW))
			{
				if ((p_ptr->inventory[INVEN_BOW].sval >= 10) && (p_ptr->inventory[INVEN_BOW].sval < 20))
					return get_slot(INVEN_AMMO);
			}
			return -1;
		}

	case TV_BOLT:
		{
			if (ideal)
			{
				return INVEN_AMMO;
			}
			else if (p_ptr->inventory[INVEN_AMMO].k_idx &&
			         object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]) &&
			         p_ptr->inventory[INVEN_AMMO].number + o_ptr->number < MAX_STACK_SIZE)
			{
				return get_slot(INVEN_AMMO);
			}
			else if ((p_ptr->inventory[INVEN_BOW].k_idx) && (p_ptr->inventory[INVEN_BOW].tval == TV_BOW))
			{
				if (p_ptr->inventory[INVEN_BOW].sval >= 20)
					return get_slot(INVEN_AMMO);
			}
			return -1;
		}

	case TV_DAEMON_BOOK:
		{
			int slot = -1;

			switch (o_ptr->sval)
			{
			case SV_DEMONBLADE : slot = INVEN_WIELD; break;
			case SV_DEMONSHIELD: slot = INVEN_ARM; break;
			case SV_DEMONHORN  : slot = INVEN_HEAD; break;
			}

			if ((slot >= 0) && (!ideal))
			{
				slot = get_slot(slot);
			}

			return slot;
		}
	}

	/* No slot available */
	return ( -1);
}

/*
 * Determine which equipment slot (if any) an item likes for the player's
 * current body and stuff
 */
s16b wield_slot(object_type const *o_ptr)
{
	return wield_slot_ideal(o_ptr, FALSE);
}

/*
 * Return a string mentioning how a given item is carried
 */
static cptr mention_use(int i)
{
	cptr p;

	/* Examine the location */
	switch (i)
	{
	case INVEN_WIELD:
	case INVEN_WIELD + 1:
	case INVEN_WIELD + 2:
		p = "Wielding";
		break;
	case INVEN_BOW:
		p = "Shooting";
		break;
	case INVEN_RING:
	case INVEN_RING + 1:
	case INVEN_RING + 2:
	case INVEN_RING + 3:
	case INVEN_RING + 4:
	case INVEN_RING + 5:
		p = "On finger";
		break;
	case INVEN_NECK:
	case INVEN_NECK + 1:
		p = "Around neck";
		break;
	case INVEN_LITE:
		p = "Light source";
		break;
	case INVEN_BODY:
		p = "On body";
		break;
	case INVEN_OUTER:
		p = "About body";
		break;
	case INVEN_ARM:
	case INVEN_ARM + 1:
	case INVEN_ARM + 2:
		p = "On arm";
		break;
	case INVEN_HEAD:
	case INVEN_HEAD + 1:
		p = "On head";
		break;
	case INVEN_HANDS:
	case INVEN_HANDS + 1:
	case INVEN_HANDS + 2:
		p = "On hands";
		break;
	case INVEN_FEET:
	case INVEN_FEET + 1:
		p = "On feet";
		break;
	case INVEN_CARRY:
		p = "Symbiote";
		break;
	case INVEN_AMMO:
		p = "Quiver";
		break;
	case INVEN_TOOL:
		p = "Using";
		break;
	default:
		p = "In pack";
		break;
	}

	/* Hack -- Heavy weapons */
	if ((INVEN_WIELD <= i) && (i <= INVEN_WIELD + 2))
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "Just lifting";
		}
	}

	/* Hack -- music instruments and heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (o_ptr->tval == TV_INSTRUMENT)
		{
			p = "Playing";
		}
		else if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10) 
		{
			p = "Just holding";
		}
	}

	/* Return the result */
	return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(int i)
{
	cptr p = nullptr;

	switch (i)
	{
	case INVEN_WIELD:
	case INVEN_WIELD + 1:
	case INVEN_WIELD + 2:
		p = "attacking monsters with";
		break;
	case INVEN_BOW:
		p = "shooting missiles with";
		break;
	case INVEN_RING:
	case INVEN_RING + 1:
	case INVEN_RING + 2:
	case INVEN_RING + 3:
	case INVEN_RING + 4:
	case INVEN_RING + 5:
		p = "wearing on your finger";
		break;
	case INVEN_NECK:
	case INVEN_NECK + 1:
		p = "wearing around your neck";
		break;
	case INVEN_LITE:
		p = "using to light the way";
		break;
	case INVEN_BODY:
		p = "wearing on your body";
		break;
	case INVEN_OUTER:
		p = "wearing on your back";
		break;
	case INVEN_ARM:
	case INVEN_ARM + 1:
	case INVEN_ARM + 2:
		p = "wearing on your arm";
		break;
	case INVEN_HEAD:
	case INVEN_HEAD + 1:
		p = "wearing on your head";
		break;
	case INVEN_HANDS:
	case INVEN_HANDS + 1:
	case INVEN_HANDS + 2:
		p = "wearing on your hands";
		break;
	case INVEN_FEET:
	case INVEN_FEET + 1:
		p = "wearing on your feet";
		break;
	case INVEN_CARRY:
		p = "in symbiosis with";
		break;
	case INVEN_AMMO:
		p = "carrying in your quiver";
		break;
	case INVEN_TOOL:
		p = "using as a tool";
		break; 
	default:
		p = "carrying in your pack";
		break;
	}

	/* Hack -- Heavy weapons */
	if ((INVEN_WIELD <= i) && (i <= INVEN_WIELD + 2))
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Music instruments and heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (o_ptr->tval == TV_INSTRUMENT)
		{
			p = "playing music with";
		}
		else if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}

/*
 * Check an item against the item tester info
 */
static bool item_tester_okay(object_type const *o_ptr, object_filter_t const &filter)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full)
	{
		return true;
	}

	/* Require an item */
	if (!o_ptr->k_idx)
	{
		return false;
	}

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD)
	{
		return false;
	}

	/* Check against the filter */
	return filter(o_ptr);
}




static void show_equip_aux(bool_ mirror, bool_ everything, object_filter_t const &filter);
static void show_inven_aux(bool_ mirror, bool_ everything, object_filter_t const &filter);

/*
 * Choice window "shadow" of the "show_inven()" function
 */
void display_inven(void)
{
	show_inven_aux(TRUE, inventory_no_move, object_filter::True());
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(void)
{
	show_equip_aux(TRUE, inventory_no_move, object_filter::True());
}



/* Get the color of the letter idx */
byte get_item_letter_color(object_type *o_ptr)
{
	byte color = TERM_WHITE;

	/* Must have knowlegde */
	if (!object_known_p(o_ptr)) return (TERM_SLATE);

	if (ego_item_p(o_ptr)) color = TERM_L_BLUE;
	if (artifact_p(o_ptr)) color = TERM_YELLOW;
	if (o_ptr->name1 && ( -1 != a_info[o_ptr->name1].set)) color = TERM_GREEN;
	if (o_ptr->name1 && (a_info[o_ptr->name1].flags4 & TR4_ULTIMATE) && (o_ptr->ident & (IDENT_MENTAL))) color = TERM_VIOLET;

	return (color);
}


/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven_aux(bool_ mirror, bool_ everything, const object_filter_t &filter)
{
	int i, j, k, l, z = 0;
	int row, col, len, lim;
	int wid, hgt;
	object_type *o_ptr;
	char o_name[80];
	char tmp_val[80];
	int out_index[23];
	byte out_color[23];
	char out_desc[23][80];


	/* Retrive current screen size */
	Term_get_size(&wid, &hgt);

	/* Starting row */
	row = mirror ? 0 : 1;

	/* Starting column */
	col = mirror ? 0 : 50;

	/* Default "max-length" */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Space for weight */
	lim -= 9;

	/* Space for icon */
	lim -= 2;

	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr, filter))
		{
			if ( !everything )
				continue;
			out_index[k] = -i - 1;
		}
		else
		{
			/* Save the object index */
			out_index[k] = i + 1;
		}

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the object color, and description */
		out_color[k] = tval_to_attr[o_ptr->tval % 128];
		(void)strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		l += 9;

		/* Account for icon */
		l += 2;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	if (mirror) col = 0;
	else col = (len > wid - 4) ? 0 : (wid - len - 1);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &p_ptr->inventory[ABS(i) - 1];

		/* Clear the line */
		prt("", row + j, col ? col - 2 : col);

		/* Prepare an index --(-- */
		/* Prepare a blank index if not selectable */
		if ( i > 0 )
			sprintf(tmp_val, "%c)", index_to_label(i - 1));
		else
			sprintf(tmp_val, "  ");

		/* Clear the line with the (possibly indented) index */
		c_put_str(get_item_letter_color(o_ptr), tmp_val, row + j, col);

		/* Display graphics for object */
		{
			byte a = object_attr(o_ptr);
			char c = object_char(o_ptr);

			if (!o_ptr->k_idx) c = ' ';

			Term_draw(col + 3, row + j, a, c);
		}

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], row + j,
		          col + 5);

		/* Display the weight */
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, row + j, wid - 9);
		}
	}

	/* Shadow windows */
	if (mirror)
	{
		int hgt;
		Term_get_size(nullptr, &hgt);
		/* Erase the rest of the window */
		for (j = row + k; j < hgt; j++)
		{
			/* Erase the line */
			Term_erase(0, j, 255);
		}
	}

	/* Main window */
	else
	{
		/* Make a "shadow" below the list (only if needed) */
		if (j && (j < 23)) prt("", row + j, col ? col - 2 : col);
	}
}


static void show_inven(object_filter_t const &filter)
{
	show_inven_aux(FALSE, FALSE, filter);
}

void show_inven_full()
{
	item_tester_full = true;
	show_inven(object_filter::True());
	item_tester_full = false;
}

static void show_equip(object_filter_t const &filter)
{
	show_equip_aux(FALSE, FALSE, filter);
}

void show_equip_full()
{
	item_tester_full = true;
	show_equip(object_filter::True());
	item_tester_full = false;
}

/*
 * Display the equipment.
 */
void show_equip_aux(bool_ mirror, bool_ everything, object_filter_t const &filter)
{
	int i, j, k, l;
	int row, col, len, lim, idx;
	int wid, hgt;
	object_type *o_ptr;
	char tmp_val[80];
	char o_name[80];
	int out_index[INVEN_TOOL - INVEN_WIELD];
	int out_rindex[INVEN_TOOL - INVEN_WIELD];
	byte out_color[INVEN_TOOL - INVEN_WIELD];
	char out_desc[INVEN_TOOL - INVEN_WIELD][80];


	/* Retrive current screen size */
	Term_get_size(&wid, &hgt);

	/* Starting row */
	row = mirror ? 0 : 1;

	/* Starting column */
	col = mirror ? 0 : 50;

	/* Maximal length */
	len = 79 - col;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for labels */
	lim -= (14 + 2);

	/* Require space for weight */
	lim -= 9;

	/* Require space for icon */
	lim -= 2;

	/* Scan the equipment list */
	idx = 0;
	for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Is there actualy a body part here ? */
		if (!p_ptr->body_parts[i - INVEN_WIELD]) continue;

		/* Mega Hack -- don't show symbiote slot if we can't use it */
		if ((i == INVEN_CARRY) && (!get_skill(SKILL_SYMBIOTIC))) continue;

		o_ptr = &p_ptr->inventory[i];

		/* Inform the player that he/she can't use a shield */
		if ((p_ptr->body_parts[i - INVEN_WIELD] == INVEN_ARM) &&
		                !o_ptr->k_idx &&
		                p_ptr->inventory[i - INVEN_ARM + INVEN_WIELD].k_idx)
		{
			u32b f1, f2, f3, f4, f5, esp;
			object_type *q_ptr = &p_ptr->inventory[i - INVEN_ARM + INVEN_WIELD];
			char q_name[80];

			/* Description */
			object_desc(q_name, q_ptr, TRUE, 3);

			/* Get weapon flags */
			object_flags(q_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

			if (f4 & TR4_MUST2H)
			{
				sprintf(o_name, "(two handed) %s", q_name);

				/* Truncate the description */
				o_name[lim] = 0;

				/* Save the index */
				out_index[k] = idx;
				out_rindex[k] = i;
				idx++;

				/* Save the color */
				out_color[k] = TERM_L_RED;
				(void)strcpy(out_desc[k], o_name);
				continue;
			}
		}

		if ((p_ptr->body_parts[i - INVEN_WIELD] == INVEN_WIELD) &&
		                !o_ptr->k_idx)
		{
			sprintf(o_name, "(%s)", get_melee_name());

			/* Truncate the description */
			o_name[lim] = 0;

			/* Save the index */
			out_index[k] = idx;
			out_rindex[k] = i;
			idx++;

			/* Save the color */
			out_color[k] = TERM_L_BLUE;
			(void)strcpy(out_desc[k], o_name);
		}
		else
		{
			idx++;

			/* Description */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Truncate the description */
			o_name[lim] = 0;
			/* Is this item acceptable? */
			if (!item_tester_okay(o_ptr, filter))
			{
				if (!everything) continue;
				out_index[k] = -1;
			}
			else
			{
				/* Save the index */
				out_index[k] = idx;
			}
			out_rindex[k] = i;

			/* Save the color */
			out_color[k] = tval_to_attr[o_ptr->tval % 128];
			(void)strcpy(out_desc[k], o_name);
		}

		/* Extract the maximal length (see below) */
		l = strlen(out_desc[k]) + (2 + 3);

		/* Increase length for labels (if needed) */
		l += (14 + 2);

		/* Increase length for weight */
		l += 9;

		/* Icon */
		l += 2;

		/* Maintain the max-length */
		if (l > len) len = l;

		/* Advance the entry */
		k++;
	}

	/* Hack -- Find a column to start in */
	if (mirror) col = 0;
	else col = (len > wid - 4) ? 0 : (wid - len - 1);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		if (j > 20) break;

		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &p_ptr->inventory[out_rindex[j]];

		/* Clear the line */
		prt("", row + j, col ? col - 2 : col);

		/* Prepare an index --(-- */
		if (out_index[j] >= 0 )
			sprintf(tmp_val, "%c)", index_to_label(out_rindex[j]));
		else
			sprintf(tmp_val, "  ");

		/* Clear the line with the (possibly indented) index */
		c_put_str(get_item_letter_color(o_ptr), tmp_val, row + j, col);

		/* Show icon */
		{
			byte a = object_attr(o_ptr);
			char c = object_char(o_ptr);

			if (!o_ptr->k_idx) c = ' ';

			Term_draw(col + 3, row + j, a, c);
		}

		/* Use labels */
		{
			/* Mention the use */
			(void)sprintf(tmp_val, "%-14s: ", mention_use(out_rindex[j]));
			put_str(tmp_val, row + j, col + 5);

			/* Display the entry itself */
			c_put_str(out_color[j], out_desc[j], row + j, col + 21);
		}

		/* Display the weight */
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, row + j, wid - 9);
		}
	}


	/* Shadow windows */
	if (mirror)
	{
		int hgt;
		Term_get_size(nullptr, &hgt);
		/* Erase the rest of the window */
		for (j = row + k; j < hgt; j++)
		{
			/* Erase the line */
			Term_erase(0, j, 255);
		}
	}

	/* Main window */
	else
	{
		/* Make a "shadow" below the list (only if needed) */
		if (j && (j < 23)) prt("", row + j, col ? col - 2 : col);
	}
}




/*
 * Flip "inven" and "equip" in any sub-windows
 */
void toggle_inven_equip(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < 8; j++)
	{
		/* Unused */
		if (!angband_term[j]) continue;

		/* Flip inven to equip */
		if (window_flag[j] & (PW_INVEN))
		{
			/* Flip flags */
			window_flag[j] &= ~(PW_INVEN);
			window_flag[j] |= (PW_EQUIP);

			/* Window stuff */
			p_ptr->window |= (PW_EQUIP);
		}

		/* Flip inven to equip */
		else if (window_flag[j] & (PW_EQUIP))
		{
			/* Flip flags */
			window_flag[j] &= ~(PW_EQUIP);
			window_flag[j] |= (PW_INVEN);

			/* Window stuff */
			p_ptr->window |= (PW_INVEN);
		}
	}
}



/*
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
bool_ verify(cptr prompt, int item)
{
	char o_name[80];

	char out_val[160];

	object_type *o_ptr;

	/* Get object */
	o_ptr = get_object(item);

	/* Describe */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Prompt */
	(void)sprintf(out_val, "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices
 *
 * The item can be negative to mean "item on floor".
 */
static bool_ get_item_allow(int item)
{
	cptr s;

	object_type *o_ptr;

	/* Get object */
	o_ptr = get_object(item);

	/* No inscription */
	if (!o_ptr->note) return (TRUE);

	/* Find a '!' */
	s = strchr(quark_str(o_ptr->note), '!');

	/* Process preventions */
	while (s)
	{
		/* Check the "restriction" */
		if ((s[1] == command_cmd) || (s[1] == '*'))
		{
			/* Verify the choice */
			if (!verify("Really try", item)) return (FALSE);
		}

		/* Find another '!' */
		s = strchr(s + 1, '!');
	}

	/* Allow it */
	return (TRUE);
}



/*
 * Auxiliary function for "get_item()" -- test an index
 */
static bool get_item_okay(int i, object_filter_t const &filter)
{
	/* Illegal items */
	if ((i < 0) || (i >= INVEN_TOTAL))
	{
		return (FALSE);
	}

	/* Verify the item */
	return item_tester_okay(&p_ptr->inventory[i], filter);
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" command_cmd code.
 */
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;


	/* Check every object */
	for (i = 0; i < INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip empty inscriptions */
		if (!o_ptr->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(o_ptr->note), '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return (FALSE);
}

/*
 * scan_floor --
 *
 * Return a list of o_list[] indexes of items at the given cave
 * location.
 */
static std::vector<int> scan_floor(int y, int x, object_filter_t const &filter)
{
	std::vector<int> items;

	/* Sanity */
	if (!in_bounds(y, x))
	{
		return items;
	}

	/* Scan all objects in the grid */
	for (auto const this_o_idx: cave[y][x].o_idxs)
	{
		/* Acquire object */
		object_type * o_ptr = &o_list[this_o_idx];

		/* Item tester */
		if (!item_tester_okay(o_ptr, filter))
		{
			continue;
		}

		/* Accept this item */
		items.push_back(this_o_idx);

		/* XXX Hack -- Enforce limit */
		if (items.size() == 23) break;
	}

	/* Result */
	return items;
}

/*
 * Display a list of the items on the floor at the given location.
 */
static void show_floor(int y, int x, object_filter_t const &filter)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[23];
	byte out_color[23];
	char out_desc[23][80];

	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight */
	lim -= 9;

	/* Scan for objects in the grid, using item_tester_okay() */
	auto const floor_list = scan_floor(y, x, filter);
	assert(floor_list.size() <= 23);
	int const floor_num = floor_list.size(); // "int" for warning avoidance

	/* Display the inventory */
	for (k = 0, i = 0; i < floor_num; i++)
	{
		o_ptr = &o_list[floor_list[i]];

		/* Describe the object */
		object_desc(o_name, o_ptr, TRUE, 3);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Acquire inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval & 0x7F];

		/* Save the object description */
		strcpy(out_desc[k], o_name);

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		/* Get the index */
		i = floor_list[out_index[j]];

		/* Get the item */
		o_ptr = &o_list[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		sprintf(tmp_val, "%c)", index_to_label(j));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		{
			int wgt = o_ptr->weight * o_ptr->number;
			sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, j + 1, 71);
		}
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}

/*
 * This version of get_item() is called by get_item() when
 * the easy_floor is on.
 */
static bool_ get_item_floor(int *cp, cptr pmt, cptr str, int mode, object_filter_t const &filter, select_by_name_t const &select_by_name)
{
	char n1 = 0, n2 = 0, which = ' ';

	int j, k, i1, i2, e1, e2;

	bool_ done, item;

	bool_ oops = FALSE;

	bool_ equip = FALSE;
	bool_ inven = FALSE;
	bool_ floor = FALSE;
	bool_ automat = FALSE;

	bool_ allow_equip = FALSE;
	bool_ allow_inven = FALSE;
	bool_ allow_floor = FALSE;

	bool_ toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_top = 0;

	k = 0;

	/* Get the item index */
	if (repeat_pull(cp))
	{
		/* Floor item? */
		if (*cp < 0)
		{
			object_type *o_ptr;

			/* Special index */
			k = 0 - (*cp);

			/* Acquire object */
			o_ptr = &o_list[k];

			/* Validate the item */
			if (item_tester_okay(o_ptr, filter))
			{
				/* Success */
				return (TRUE);
			}
		}

		/* Verify the item */
		else if (get_item_okay(*cp, filter))
		{
			/* Success */
			return (TRUE);
		}
	}


	/* Extract args */
	if (mode & (USE_EQUIP)) equip = TRUE;
	if (mode & (USE_INVEN)) inven = TRUE;
	if (mode & (USE_FLOOR)) floor = TRUE;
	if (mode & (USE_AUTO)) automat = TRUE;


	/* Paranoia XXX XXX XXX */
	msg_print(NULL);


	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;


	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;

	/* Forbid inventory */
	if (!inven) i2 = -1;

	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!get_item_okay(i1, filter))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2, filter))) i2--;


	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = INVEN_TOTAL - 1;

	/* Forbid equipment */
	if (!equip) e2 = -1;

	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!get_item_okay(e1, filter))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2, filter))) e2--;


	/* Floor items? */
	auto const floor_list =
		floor ? scan_floor(p_ptr->py, p_ptr->px, filter)
		      : std::vector<int>();
	assert(floor_list.size() <= 23);
	int const floor_num = floor_list.size(); // "int" for warning avoidance

	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;

	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;

	/* Accept floor */
	if (!floor_list.empty()) allow_floor = TRUE;

	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_floor)
	{
		/* Oops */
		oops = TRUE;

		/* Done */
		done = TRUE;
	}

	/* Analyze choices */
	else
	{
		/* Hack -- Start on equipment if requested */
		if ((command_wrk == (USE_EQUIP)) && allow_equip)
		{
			command_wrk = (USE_EQUIP);
		}

		/* Use inventory if allowed */
		else if (allow_inven)
		{
			command_wrk = (USE_INVEN);
		}

		/* Use equipment if allowed */
		else if (allow_equip)
		{
			command_wrk = (USE_EQUIP);
		}

		/* Use floor if allowed */
		else if (allow_floor)
		{
			command_wrk = (USE_FLOOR);
		}
	}

	/* Save screen */
	screen_save();

	/* Repeat until done */
	while (!done)
	{
		/* Show choices */
		{
			int ni = 0;
			int ne = 0;

			/* Scan windows */
			for (j = 0; j < 8; j++)
			{
				/* Unused */
				if (!angband_term[j]) continue;

				/* Count windows displaying inven */
				if (window_flag[j] & (PW_INVEN)) ni++;

				/* Count windows displaying equip */
				if (window_flag[j] & (PW_EQUIP)) ne++;
			}

			/* Toggle if needed */
			if ((command_wrk == (USE_EQUIP) && ni && !ne) ||
			                (command_wrk == (USE_INVEN) && !ni && ne))
			{
				/* Toggle */
				toggle_inven_equip();

				/* Track toggles */
				toggle = !toggle;
			}

			/* Update */
			p_ptr->window |= (PW_INVEN | PW_EQUIP);

			/* Redraw windows */
			window_stuff();
		}

		/* Inventory screen */
		if (command_wrk == (USE_INVEN))
		{
			/* Extract the legal requests */
			n1 = I2A(i1);
			n2 = I2A(i2);

			/* Redraw */
			show_inven(filter);
		}

		/* Equipment screen */
		else if (command_wrk == (USE_EQUIP))
		{
			/* Extract the legal requests */
			n1 = I2A(e1 - INVEN_WIELD);
			n2 = I2A(e2 - INVEN_WIELD);

			/* Redraw */
			show_equip(filter);
		}

		/* Floor screen */
		else if (command_wrk == (USE_FLOOR))
		{
			j = floor_top;
			k = MIN(floor_top + 23, floor_num) - 1;

			/* Extract the legal requests */
			n1 = I2A(j - floor_top);
			n2 = I2A(k - floor_top);

			/* Redraw */
			show_floor(p_ptr->py, p_ptr->px, filter);
		}

		/* Viewing inventory */
		if (command_wrk == (USE_INVEN))
		{
			/* Begin the prompt */
			sprintf(out_val, "Inven:");

			/* Build the prompt */
			sprintf(tmp_val, " %c-%c,",
			        index_to_label(i1), index_to_label(i2));

			/* Append */
			strcat(out_val, tmp_val);

			/* Append */
			if (allow_equip) strcat(out_val, " / for Equip,");

			/* Append */
			if (allow_floor) strcat(out_val, " - for floor,");
		}

		/* Viewing equipment */
		else if (command_wrk == (USE_EQUIP))
		{
			/* Begin the prompt */
			sprintf(out_val, "Equip:");

			/* Build the prompt */
			sprintf(tmp_val, " %c-%c,",
			        index_to_label(e1), index_to_label(e2));

			/* Append */
			strcat(out_val, tmp_val);

			/* Append */
			if (allow_inven) strcat(out_val, " / for Inven,");

			/* Append */
			if (allow_floor) strcat(out_val, " - for floor,");
		}

		/* Viewing floor */
		else if (command_wrk == (USE_FLOOR))
		{
			/* Begin the prompt */
			sprintf(out_val, "Floor:");

			/* Build the prompt */
			sprintf(tmp_val, " %c-%c,", n1, n2);

			/* Append */
			strcat(out_val, tmp_val);

			/* Append */
			if (allow_inven)
			{
				strcat(out_val, " / for Inven,");
			}
			else if (allow_equip)
			{
				strcat(out_val, " / for Equip,");
			}
		}

		/* Do we allow selection by name? */
		if (select_by_name)
		{
			strcat(out_val, " @ for extra selection,");
		}

		/* Create automatizer rule?? */
		if (automat)
		{
			if (automatizer_create)
				strcat(out_val, " $ new automatizer rule(ON),");
			else
				strcat(out_val, " $ new automatizer rule(OFF),");
		}

		/* Finish the prompt */
		strcat(out_val, " ESC");

		/* Build the prompt */
		sprintf(tmp_val, "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);

		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which)
		{
		case ESCAPE:
			{
				done = TRUE;
				break;
			}

		case '/':
			{
				if (command_wrk == (USE_INVEN))
				{
					if (!allow_equip)
					{
						bell();
						break;
					}
					command_wrk = (USE_EQUIP);
				}
				else if (command_wrk == (USE_EQUIP))
				{
					if (!allow_inven)
					{
						bell();
						break;
					}
					command_wrk = (USE_INVEN);
				}
				else if (command_wrk == (USE_FLOOR))
				{
					if (allow_inven)
					{
						command_wrk = (USE_INVEN);
					}
					else if (allow_equip)
					{
						command_wrk = (USE_EQUIP);
					}
					else
					{
						bell();
						break;
					}
				}

				/* Hack -- Fix screen */
				screen_load();
				screen_save();

				/* Need to redraw */
				break;
			}

		case '-':
			{
				if (!allow_floor)
				{
					bell();
					break;
				}

				/*
				 * If we are already examining the floor, and there
				 * is only one item, we will always select it.
				 * If we aren't examining the floor and there is only
				 * one item, we will select it if floor_query_flag
				 * is FALSE.
				 */
				if (floor_num == 1)
				{
					if (command_wrk == (USE_FLOOR))
					{
						/* Special index */
						k = 0 - floor_list[0];

						/* Allow player to "refuse" certain actions */
						if (!get_item_allow(k))
						{
							done = TRUE;
							break;
						}

						/* Accept that choice */
						(*cp) = k;
						item = TRUE;
						done = TRUE;

						break;
					}
				}

				/* Hack -- Fix screen */
				screen_load();
				screen_save();

				command_wrk = (USE_FLOOR);

				break;
			}

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				/* Look up the tag */
				if (!get_tag(&k, which))
				{
					bell();
					break;
				}

				/* Hack -- Validate the item */
				if ((k < INVEN_WIELD) ? !inven : !equip)
				{
					bell();
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k, filter))
				{
					bell();
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

		case '\n':
		case '\r':
			{
				/* Choose "default" inventory item */
				if (command_wrk == (USE_INVEN))
				{
					k = ((i1 == i2) ? i1 : -1);
				}

				/* Choose "default" equipment item */
				else if (command_wrk == (USE_EQUIP))
				{
					k = ((e1 == e2) ? e1 : -1);
				}

				/* Choose "default" floor item */
				else if (command_wrk == (USE_FLOOR))
				{
					if (floor_num == 1)
					{
						/* Special index */
						k = 0 - floor_list[0];

						/* Allow player to "refuse" certain actions */
						if (!get_item_allow(k))
						{
							done = TRUE;
							break;
						}

						/* Accept that choice */
						(*cp) = k;
						item = TRUE;
						done = TRUE;
					}
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k, filter))
				{
					bell();
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

		case '@':
			{
				// Ignore if not "enabled"
				if (!select_by_name)
				{
					break;
				}
				// Find by name
				if (auto i = select_by_name(filter))
				{
					(*cp) = *i;
					item = TRUE;
					done = TRUE;
				}
				break;
			}

		case '$':
			{
				automatizer_create = !automatizer_create;
				break;
			}

		default:
			{
				int ver;

				ver = isupper(which);
				which = tolower(which);

				/* Convert letter to inventory index */
				if (command_wrk == (USE_INVEN))
				{
					k = label_to_inven(which);
					if (k == -1)
					{
						bell();
						break;
					}
				}

				/* Convert letter to equipment index */
				else if (command_wrk == (USE_EQUIP))
				{
					k = label_to_equip(which);
					if (k == -1)
					{
						bell();
						break;
					}
				}

				/* Convert letter to floor index */
				else if (command_wrk == (USE_FLOOR))
				{
					k = islower(which) ? A2I(which) : -1;
					if (k < 0 || k >= floor_num)
					{
						bell();
						break;
					}

					/* Special index */
					k = 0 - floor_list[k];
				}

				/* Validate the item */
				if ((k >= 0) && !get_item_okay(k, filter))
				{
					bell();
					break;
				}

				/* Verify the item */
				if (ver && !verify("Try", k))
				{
					done = TRUE;
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}

	/* Fix the screen */
	screen_load();

	/* Track */
	if (item && done)
	{
		if (*cp >= 0)
		{
			object_track(&p_ptr->inventory[*cp]);
		}
		else
		{
			object_track(&o_list[0 - *cp]);
		}
	}

	/* Clean up */
	{
		/* Toggle again if needed */
		if (toggle) toggle_inven_equip();

		/* Update */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Window stuff */
		window_stuff();
	}


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg_print(str);


	if (item) repeat_push(*cp);

	/* Result */
	return (item);
}


/*
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven listings.
 * If it is TRUE then we are viewing inventory, else equipment.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 */
bool_ get_item(int *cp, cptr pmt, cptr str, int mode, object_filter_t const &filter, select_by_name_t const &select_by_name)
{
	automatizer_create = FALSE;
	return get_item_floor(cp, pmt, str, mode, filter, select_by_name);
}

/*
 * Hook to determine if an object is getable
 */
static bool item_tester_hook_getable(object_type const *o_ptr)
{
	if (!inven_carry_okay(o_ptr)) return (FALSE);

	if ((o_ptr->tval == TV_HYPNOS) && (!get_skill(SKILL_SYMBIOTIC))) return FALSE;

	/* Assume yes */
	return (TRUE);
}

/*
 * Wear a single item from o_ptr
 */
int wear_ammo(object_type *o_ptr)
{
	int slot, num = 1;

	object_type forge;
	object_type *q_ptr;

	/* Check the slot */
	slot = wield_slot(o_ptr);

	if(slot == -1)
		return -1;

	/* Get local object */
	q_ptr = &forge;

	/* Obtain local object */
	object_copy(q_ptr, o_ptr);

	num = o_ptr->number;

	/* Modify quantity */
	q_ptr->number = num;

	/* Access the wield slot */
	o_ptr = &p_ptr->inventory[slot];

	q_ptr->number += o_ptr->number;

	/* Wear the new stuff */
	object_copy(o_ptr, q_ptr);

	/* Increment the equip counter by hand */
	equip_cnt++;

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		msg_print("Oops! It feels deathly cold!");

		/* Note the curse */
		o_ptr->ident |= (IDENT_SENSE);
		o_ptr->sense = SENSE_CURSED;
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate hitpoint */
	p_ptr->update |= (PU_HP);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA);

	/* Redraw monster hitpoint */
	p_ptr->redraw |= (PR_FRAME);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	return slot;
}


/*
 * Try to pickup arrows
 */
void pickup_ammo()
{
	/* Copy list of objects since we're manipulating the list */
	auto const object_idxs(cave[p_ptr->py][p_ptr->px].o_idxs);

	/* Scan the pile of objects */
	for (auto const this_o_idx: object_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		if (object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]))
		{
			msg_print("You add the ammo to your quiver.");
			s16b slot = wear_ammo(o_ptr);

			if (slot != -1)
			{
				/* Get the item again */
				o_ptr = &p_ptr->inventory[slot];

				/* Describe the object */
				char o_name[80];
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Message */
				msg_format("You have %s (%c).", o_name, index_to_label(slot));

				/* Delete the object */
				delete_object_idx(this_o_idx);
			}
		}
	}
}


/**
 * Check for encumberance if player were to pick up
 * given item.
 */
static bool can_carry_heavy(object_type const *o_ptr)
{
	/* Extract the "weight limit" (in tenth pounds) */
	int i = weight_limit();

	/* Calculate current encumbarance */
	int j = calc_total_weight();

	/* Apply encumbarance from weight */
	int old_enc = 0;
	if (j > i / 2) old_enc = ((j - (i / 2)) / (i / 10));

	/* Increase the weight, recalculate encumbarance */
	j += (o_ptr->number * o_ptr->weight);

	/* Apply encumbarance from weight */
	int new_enc = 0;
	if (j > i / 2) new_enc = ((j - (i / 2)) / (i / 10));

	/* If the encumberance is the same, then we pick up without prompt */
	return (new_enc <= old_enc);
}

/* Do the actuall picking up */
void object_pickup(int this_o_idx)
{
	int slot = 0;
	char o_name[80] = "";
	object_type *q_ptr, *o_ptr;

	/* Access the item */
	o_ptr = &o_list[this_o_idx];

	if (p_ptr->auto_id)
	{
		object_aware(o_ptr);
		object_known(o_ptr);
	}

	/* Describe the object */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Note that the pack is too full */
	if (!inven_carry_okay(o_ptr) && !object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]))
	{
		msg_format("You have no room for %s.", o_name);
	}

	/* Pick up object */
	else
	{
		/* Hooks */
		{
			hook_get_in in = { o_ptr, this_o_idx };
			if (process_hooks_new(HOOK_GET, &in, NULL))
			{
				return;
			}
		}

		q_ptr = &p_ptr->inventory[INVEN_AMMO];

		/* Carry the item */
		if (object_similar(o_ptr, q_ptr))
		{
			msg_print("You add the ammo to your quiver.");
			slot = wear_ammo(o_ptr);
		}
		else
		{
			slot = inven_carry(o_ptr, FALSE);
		}

		/* Sanity check */
		if (slot != -1)
		{
			/* Get the item again */
			o_ptr = &p_ptr->inventory[slot];

			object_track(o_ptr);

			/* Describe the object */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Message */
			msg_format("You have %s (%c).", o_name, index_to_label(slot));

			/* Delete the object */
			delete_object_idx(this_o_idx);

                        /* Sense object. */
                        sense_inventory();
		}
	}
}


static void absorb_gold(cave_type const *c_ptr)
{
	/* Copy list of objects since we're going to manipulate the list itself */
	auto const object_idxs(c_ptr->o_idxs);

	/* Go through everything */
	for (auto const this_o_idx: object_idxs)
	{
		/* Acquire object */
		object_type *o_ptr = &o_list[this_o_idx];

		/* Hack -- disturb */
		disturb(0);

		/* Pick up gold */
		if (o_ptr->tval == TV_GOLD)
		{
			char goldname[80];
			object_desc(goldname, o_ptr, TRUE, 3);
			/* Message */
			msg_format("You have found %ld gold pieces worth of %s.",
				   (long)o_ptr->pval, goldname);

			/* Collect the gold */
			p_ptr->au += o_ptr->pval;

			/* Redraw gold */
			p_ptr->redraw |= (PR_FRAME);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);

			/* Delete the gold */
			delete_object_idx(this_o_idx);
		}
	}
}

static void sense_floor(cave_type const *c_ptr)
{
	/* Build a list of the floor objects. */
	std::vector<int> floor_object_idxs;
	{
		/* Reserve the correct number of slots */
		floor_object_idxs.reserve(c_ptr->o_idxs.size());
		/* Fill in the indexes */
		for (auto const this_o_idx: c_ptr->o_idxs)
		{
			// Note the "-"! We need it for get_object()
			// lookups to function correctly.
			floor_object_idxs.push_back(0 - this_o_idx);
		}
	}

	/* Mega Hack -- If we have auto-Id, do an ID sweep *before* squleching,
	 * so that we don't have to walk over things twice to get them
	 * squelched.  --dsb */
	if (p_ptr->auto_id)
	{
		for (auto const o_idx: floor_object_idxs)
		{
			object_type *o_ptr = get_object(o_idx);
			object_aware(o_ptr);
			object_known(o_ptr);
		}
	}

	/* Sense floor tile */
	sense_objects(floor_object_idxs);
}

void py_pickup_floor(int pickup)
{
	/* Get the tile */
	auto c_ptr = &cave[p_ptr->py][p_ptr->px];

	/* Try to grab ammo */
	pickup_ammo();

	/* Auto-ID and pseudo-ID */
	sense_floor(c_ptr);

	/* Squeltch the floor */
	squeltch_grid();

	/* Absorb gold on the tile */
	absorb_gold(&cave[p_ptr->py][p_ptr->px]);

	/* We handle 0, 1, or "many" items cases separately */
	if (c_ptr->o_idxs.empty())
	{
		/* Nothing to do */
	}
	else if (c_ptr->o_idxs.size() == 1)
	{
		/* Acquire object */
		auto floor_o_idx = c_ptr->o_idxs.front();
		auto o_ptr = &o_list[floor_o_idx];

		/* Describe or pick up? */
		if (!pickup)
		{
			/* Describe */
			char o_name[80] = "";
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Message */
			msg_format("You see %s.", o_name);
		}
		else
		{
			/* Are we actually going to pick up? */
			bool_ do_pickup = TRUE;

			/* Hack -- query every item */
			if (carry_query_flag || (!can_carry_heavy(&o_list[floor_o_idx])))
			{
				char o_name[80] = "";
				object_desc(o_name, o_ptr, TRUE, 3);

				if (!inven_carry_okay(o_ptr) && !object_similar(o_ptr, &p_ptr->inventory[INVEN_AMMO]))
				{
					msg_format("You have no room for %s.", o_name);
					return; /* Done */
				}
				else
				{
					char out_val[160];
					sprintf(out_val, "Pick up %s? ", o_name);
					do_pickup = get_check(out_val);
				}
			}

			/* Just pick it up; unless it's a symbiote and we don't have Symbiosis */
			if (do_pickup && ((o_list[floor_o_idx].tval != TV_HYPNOS) || (get_skill(SKILL_SYMBIOTIC))))
			{
				object_pickup(floor_o_idx);
			}
		}
	}
	else
	{
		/* Describe or pick up? */
		if (!pickup)
		{
			/* Message */
			msg_format("You see a pile of %d items.", c_ptr->o_idxs.size());
		}
		else
		{
			/* Prompt for the item to pick up */
			cptr q = "Get which item? ";
			cptr s = "You have no room in your pack for any of the items here.";
			int item;
			if (get_item(&item, q, s, (USE_FLOOR), item_tester_hook_getable))
			{
				s16b this_o_idx = 0 - item;

				bool_ do_pickup = TRUE;
				if (!can_carry_heavy(&o_list[this_o_idx]))
				{
					/* Describe the object */
					char o_name[80] = "";
					object_desc(o_name, &o_list[this_o_idx], TRUE, 3);

					/* Prompt */
					char out_val[160];
					sprintf(out_val, "Pick up %s? ", o_name);
					do_pickup = get_check(out_val);
				}

				/* Pick up the item */
				if (do_pickup)
				{
					object_pickup(this_o_idx);
				}
			}
		}
	}
}

/* Add a flags group */
static void gain_flag_group(object_type *o_ptr)
{
	int grp = 0;
	int tries = 1000;

	while (tries--)
	{
		grp = rand_int(MAX_FLAG_GROUP);

		/* If we already got this group continue */
		if (o_ptr->pval3 & BIT(grp)) continue;

		/* Not enough points ? */
		if (flags_groups[grp].price > o_ptr->pval2) continue;

		/* Ok, enough points and not already got it */
		break;
	}

	/* Ack, nothing found */
	if (tries <= 1) return;

	o_ptr->pval2 -= flags_groups[grp].price;
	o_ptr->pval3 |= BIT(grp);

	/* Message */
	{
		char o_name[80];

		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("%s gains access to the %s realm.", o_name, flags_groups[grp].name);
	}
}

static u32b get_flag(object_type *o_ptr, int grp, int k)
{
	u32b f = 0, flag_set = 0;
	int tries = 1000;
	u32b f1, f2, f3, f4, f5, esp, flag_test;

	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* get the corresponding flag set of the group */
	switch (k)
	{
	case 0:
		flag_set = flags_groups[grp].flags1;
		flag_test = f1;
		break;
	case 1:
		flag_set = flags_groups[grp].flags2;
		flag_test = f2;
		break;
	case 2:
		flag_set = flags_groups[grp].flags3;
		flag_test = f3;
		break;
	case 3:
		flag_set = flags_groups[grp].flags4;
		flag_test = f4;
		break;
	case 4:
		flag_set = flags_groups[grp].esp;
		flag_test = esp;
		break;
	default:
		flag_set = flags_groups[grp].flags1;
		flag_test = f1;
		break;
	}

	/* If no flags, no need to look */
	if (!count_bits(flag_set)) return 0;

	while (tries--)
	{
		/* get a random flag */
		f = BIT(rand_int(32));

		/* is it part of the group */
		if (!(f & flag_set)) continue;

		/* Already got it */
		if (f & flag_test) continue;

		/* Ok one */
		break;
	}

	if (tries <= 1) return (0);
	else return (f);
}

/* Add a flags from a flag group */
static void gain_flag_group_flag(object_type *o_ptr)
{
	int grp = 0, k = 0;
	u32b f = 0;
	int tries = 20000;

	if (!count_bits(o_ptr->pval3)) return;

	while (tries--)
	{
		/* Get a flag set */
		k = rand_int(5);

		/* get a flag group */
		grp = rand_int(MAX_FLAG_GROUP);

		if (!(BIT(grp) & o_ptr->pval3)) continue;

		/* Return a flag from the group/set */
		f = get_flag(o_ptr, grp, k);

		if (!f) continue;

		break;
	}

	if (tries <= 1) return;

	switch (k)
	{
	case 0:
		o_ptr->art_flags1 |= f;
		break;
	case 1:
		o_ptr->art_flags2 |= f;
		break;
	case 2:
		o_ptr->art_flags3 |= f;
		break;
	case 3:
		o_ptr->art_flags4 |= f;
		break;
	case 4:
		o_ptr->art_esp |= f;
		break;
	}

	/* Message */
	{
		char o_name[80];

		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("%s gains a new power from the %s realm.", o_name, flags_groups[grp].name);
	}
}

/*
 * When an object gain a level, he can gain some attributes
 */
void object_gain_level(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;

	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* First it can gain some tohit and todam */
	if ((o_ptr->tval == TV_AXE) || (o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM) ||
	                (o_ptr->tval == TV_HAFTED) || (o_ptr->tval == TV_MSTAFF))
	{
		int k = rand_int(100);

		/* gain +2,+1 */
		if (k < 33)
		{
			o_ptr->to_h += randint(2);
			o_ptr->to_d += 1;
		}
		/* +1 and 1 point */
		else if (k < 66)
		{
			o_ptr->to_h += 1;
			o_ptr->pval2++;

			if (magik(NEW_GROUP_CHANCE)) gain_flag_group(o_ptr);
		}
		else
		{
			if (!o_ptr->pval3) gain_flag_group(o_ptr);

			gain_flag_group_flag(o_ptr);

			if (!o_ptr->pval) o_ptr->pval = 1;
			else
			{
				while (magik(20 - (o_ptr->pval * 2))) o_ptr->pval++;

				if (o_ptr->pval > 5) o_ptr->pval = 5;
			}
		}
	}
}


/*
 * Item sets fcts
 */
bool_ wield_set(s16b a_idx, s16b set_idx, bool_ silent)
{
	set_type *s_ptr = &set_info[set_idx];
	int i;

	if ( -1 == a_info[a_idx].set) return (FALSE);
	for (i = 0; i < s_ptr->num; i++)
		if (a_idx == s_ptr->arts[i].a_idx) break;
	if (!s_ptr->arts[i].present)
	{
		s_ptr->num_use++;
		s_ptr->arts[i].present = TRUE;
		if (s_ptr->num_use > s_ptr->num)
		{
			msg_print("ERROR!! s_ptr->num_use > s_ptr->use");
		}
		else if ((s_ptr->num_use == s_ptr->num) && (!silent))
		{
			cmsg_format(TERM_GREEN, "%s item set completed.", s_ptr->name);
		}
		return (TRUE);
	}
	return (FALSE);
}

bool_ takeoff_set(s16b a_idx, s16b set_idx)
{
	set_type *s_ptr = &set_info[set_idx];
	int i;

	if ( -1 == a_info[a_idx].set) return (FALSE);
	for (i = 0; i < s_ptr->num; i++)
		if (a_idx == s_ptr->arts[i].a_idx) break;

	if (s_ptr->arts[i].present)
	{
		s_ptr->arts[i].present = FALSE;

		assert(s_ptr->num_use > 0);
		s_ptr->num_use--;

		if (s_ptr->num_use == s_ptr->num - 1)
		{
			cmsg_format(TERM_GREEN, "%s item set not complete anymore.", s_ptr->name);
		}

		return (TRUE);
	}
	return (FALSE);
}

bool_ apply_set(s16b a_idx, s16b set_idx)
{
	set_type *s_ptr = &set_info[set_idx];
	int i, j;

	if ( -1 == a_info[a_idx].set) return (FALSE);
	for (i = 0; i < s_ptr->num; i++)
		if (a_idx == s_ptr->arts[i].a_idx) break;
	if (s_ptr->arts[i].present)
	{
		for (j = 0; j < s_ptr->num_use; j++)
		{
			apply_flags(s_ptr->arts[i].flags1[j],
			            s_ptr->arts[i].flags2[j],
			            s_ptr->arts[i].flags3[j],
			            s_ptr->arts[i].flags4[j],
			            s_ptr->arts[i].flags5[j],
			            s_ptr->arts[i].esp[j],
			            s_ptr->arts[i].pval[j],
			            0, 0, 0, 0);
		}
		return (TRUE);
	}
	return (FALSE);
}

static bool_ apply_flags_set(s16b a_idx, s16b set_idx,
                     u32b *f1, u32b *f2, u32b *f3, u32b *f4, u32b *f5, u32b *esp)
{
	set_type *s_ptr = &set_info[set_idx];
	int i, j;

	if ( -1 == a_info[a_idx].set) return (FALSE);

	for (i = 0; i < s_ptr->num; i++)
	{
		if (a_idx == s_ptr->arts[i].a_idx) break;
	}

	if (s_ptr->arts[i].present)
	{
		for (j = 0; j < s_ptr->num_use; j++)
		{
			(*f1) |= s_ptr->arts[i].flags1[j];
			(*f2) |= s_ptr->arts[i].flags2[j];
			(*f3) |= s_ptr->arts[i].flags3[j];
			(*f4) |= s_ptr->arts[i].flags4[j];
			(*f5) |= s_ptr->arts[i].flags5[j];
			(*esp) |= s_ptr->arts[i].esp[j];
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Return the "attr" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */

byte object_attr(object_type const *o_ptr)
{
	if (o_ptr->tval == TV_RANDART)
	{
		return random_artifacts[o_ptr->sval].attr;
	}
	else if (k_info[o_ptr->k_idx].flavor)
	{
		return misc_to_attr[k_info[o_ptr->k_idx].flavor];
	}
	else
	{
		return k_info[o_ptr->k_idx].x_attr;
	}
}

byte object_attr_default(object_type *o_ptr)
{
	if (o_ptr->tval == TV_RANDART)
	{
		return random_artifacts[o_ptr->sval].attr;
	}
	else if (k_info[o_ptr->k_idx].flavor)
	{
		return misc_to_attr[k_info[o_ptr->k_idx].flavor];
	}
	else
	{
		return k_info[o_ptr->k_idx].d_attr;
	}
}

/*
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */

char object_char(object_type const *o_ptr)
{
	if (k_info[o_ptr->k_idx].flavor)
	{
		return misc_to_char[k_info[o_ptr->k_idx].flavor];
	}
	else
	{
		return k_info[o_ptr->k_idx].x_char;
	}
}

char object_char_default(object_type const *o_ptr)
{
	if (k_info[o_ptr->k_idx].flavor)
	{
		return misc_to_char[k_info[o_ptr->k_idx].flavor];
	}
	else
	{
		return k_info[o_ptr->k_idx].d_char;
	}
}

/**
 * Is the given object an artifact?
 */
bool artifact_p(object_type const *o_ptr)
{
	return
		(o_ptr->tval == TV_RANDART) ||
		(o_ptr->name1 ? true : false) ||
		(o_ptr->art_name ? true : false) ||
		((k_info[o_ptr->k_idx].flags3 & TR3_NORM_ART) ? true : false);
}

/**
 * Is the given object an ego item?
 */
bool ego_item_p(object_type const *o_ptr)
{
	return o_ptr->name2 || (o_ptr->name2b ? TRUE : FALSE);
}

/*
 * Is the given object an ego item of the given type?
 */
bool is_ego_p(object_type const *o_ptr, s16b ego)
{
	return (o_ptr->name2 == ego) || (o_ptr->name2b == ego);
}

/**
 * Is the given object identified as cursed?
 */
bool cursed_p(object_type const *o_ptr)
{
	return o_ptr->ident & (IDENT_CURSED);
}
