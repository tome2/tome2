/* File: wizard1.c */

/* Purpose: Spoiler generation -BEN- */

#include "angband.h"


#ifdef ALLOW_SPOILERS


/*
 * The spoiler file being created
 */
static FILE *fff = NULL;


/*
 * Write out `n' of the character `c' to the spoiler file
 */
static void spoiler_out_n_chars(int n, char c)
{
	while (--n >= 0) fputc(c, fff);
}


/*
 * Write out `n' blank lines to the spoiler file
 */
static void spoiler_blanklines(int n)
{
	spoiler_out_n_chars(n, '\n');
}


/*
 * Write a line to the spoiler file and then "underline" it with hyphens
 */
static void spoiler_underline(cptr str)
{
	fprintf(fff, "%s\n", str);
	spoiler_out_n_chars(strlen(str), '-');
	fprintf(fff, "\n");
}


/*
 * Buffer text to the given file. (-SHAWN-)
 * This is basically c_roff() from mon-desc.c with a few changes.
 */
static void spoil_out(cptr str)
{
	cptr r;

	/* Line buffer */
	static char roff_buf[256];

	/* Current pointer into line roff_buf */
	static char *roff_p = roff_buf;

	/* Last space saved into roff_buf */
	static char *roff_s = NULL;

	/* Special handling for "new sequence" */
	if (!str)
	{
		if (roff_p != roff_buf) roff_p--;
		while (*roff_p == ' ' && roff_p != roff_buf) roff_p--;
		if (roff_p == roff_buf) fprintf(fff, "\n");
		else
		{
			*(roff_p + 1) = '\0';
			fprintf(fff, "%s\n\n", roff_buf);
		}
		roff_p = roff_buf;
		roff_s = NULL;
		roff_buf[0] = '\0';
		return;
	}

	/* Scan the given string, character at a time */
	for (; *str; str++)
	{
		char ch = *str;
		int wrap = (ch == '\n');

		if (!isprint(ch)) ch = ' ';
		if (roff_p >= roff_buf + 75) wrap = 1;
		if ((ch == ' ') && (roff_p + 2 >= roff_buf + 75)) wrap = 1;

		/* Handle line-wrap */
		if (wrap)
		{
			*roff_p = '\0';
			r = roff_p;
			if (roff_s && (ch != ' '))
			{
				*roff_s = '\0';
				r = roff_s + 1;
			}
			fprintf(fff, "%s\n", roff_buf);
			roff_s = NULL;
			roff_p = roff_buf;
			while (*r) *roff_p++ = *r++;
		}

		/* Save the char */
		if ((roff_p > roff_buf) || (ch != ' '))
		{
			if (ch == ' ') roff_s = roff_p;
			*roff_p++ = ch;
		}
	}
}


/*
 * Extract a textual representation of an attribute
 */
static cptr attr_to_text(byte a)
{
	switch (a)
	{
	case TERM_DARK:
		return ("xxx");
	case TERM_WHITE:
		return ("White");
	case TERM_SLATE:
		return ("Slate");
	case TERM_ORANGE:
		return ("Orange");
	case TERM_RED:
		return ("Red");
	case TERM_GREEN:
		return ("Green");
	case TERM_BLUE:
		return ("Blue");
	case TERM_UMBER:
		return ("Umber");
	case TERM_L_DARK:
		return ("L.Dark");
	case TERM_L_WHITE:
		return ("L.Slate");
	case TERM_VIOLET:
		return ("Violet");
	case TERM_YELLOW:
		return ("Yellow");
	case TERM_L_RED:
		return ("L.Red");
	case TERM_L_GREEN:
		return ("L.Green");
	case TERM_L_BLUE:
		return ("L.Blue");
	case TERM_L_UMBER:
		return ("L.Umber");
	}

	/* Oops */
	return ("Icky");
}



/*
 * A tval grouper
 */
typedef struct
{
	byte tval;
	cptr name;
}
grouper;



/*
 * Item Spoilers by: benh@phial.com (Ben Harrison)
 */


/*
 * The basic items categorized by type
 */
static grouper group_item[] =
{
	{ TV_SWORD, "Melee Weapons" },
	{ TV_POLEARM, NULL },
	{ TV_HAFTED, NULL },
	{ TV_AXE, NULL },
	{ TV_MSTAFF, NULL },

	{ TV_BOW, "Bows and Slings" },

	{ TV_SHOT, "Ammo" },
	{ TV_ARROW, NULL },
	{ TV_BOLT, NULL },

	{ TV_BOOMERANG, "Boomerangs" },

	{ TV_INSTRUMENT, "Instruments" },

	{ TV_SOFT_ARMOR, "Armour (Body)" },
	{ TV_HARD_ARMOR, NULL },
	{ TV_DRAG_ARMOR, NULL },

	{ TV_SHIELD, "Armour (Misc)" },
	{ TV_HELM, NULL },
	{ TV_CROWN, NULL },
	{ TV_GLOVES, NULL },
	{ TV_BOOTS, NULL },

	{ TV_CLOAK, "Cloaks" },
	{ TV_AMULET, "Amulets" },
	{ TV_RING, "Rings" },

	{ TV_SCROLL, "Scrolls" },
	{ TV_POTION, "Potions" },
	{ TV_POTION2, NULL },

	{ TV_FOOD, "Food" },

	{ TV_ROD_MAIN, "Rods" },
	{ TV_ROD, "Rod Tips" },
	{ TV_WAND, "Wands" },
	{ TV_STAFF, "Staves" },

	{ TV_BOOK, "Books (Magic, Gods, Music)" },
	{ TV_DAEMON_BOOK, "Demonic Equipment" },

	{ TV_RUNE1, "Runes" },
	{ TV_RUNE2, NULL },

	{ TV_BATERIE, "Essences" },

	{ TV_PARCHMENT, "Parchments" },

	{ TV_DIGGING, "Tools" },
	{ TV_TOOL, NULL },

	{ TV_TRAPKIT, "Trapping Kits" },

	{ TV_CHEST, "Chests" },

	{ TV_SPIKE, "Various" },
	{ TV_LITE, NULL },
	{ TV_FLASK, NULL },
	{ TV_BOTTLE, NULL },
	{ TV_JUNK, NULL },

	{ TV_SKELETON, "Corpses and Eggs" },
	{ TV_CORPSE, NULL },
	{ TV_EGG, NULL },

	{ 0, "" }
};


/*
 * Describe the kind
 */
static void kind_info(char *buf, char *dam, char *wgt, int *lev, s32b *val, int k)
{
	object_type forge;
	object_type *q_ptr;

	object_kind *k_ptr;


	/* Get local object */
	q_ptr = &forge;

	/* Prepare a fake item */
	object_prep(q_ptr, k);

	/* Obtain the "kind" info */
	k_ptr = &k_info[q_ptr->k_idx];

	/* It is known */
	q_ptr->ident |= (IDENT_KNOWN);

	/* Cancel bonuses */
	q_ptr->to_a = 0;
	q_ptr->to_h = 0;
	q_ptr->to_d = 0;

	if ((k_ptr->tval == TV_WAND) || (k_ptr->tval == TV_STAFF))
	{
		hack_apply_magic_power = -99;
		apply_magic(q_ptr, 0, FALSE, FALSE, FALSE);
	}

	/* Level */
	(*lev) = k_ptr->level;

	/* Value */
	(*val) = object_value(q_ptr);


	/* Hack */
	if (!buf || !dam || !wgt) return;


	/* Description (too brief) */
	object_desc_store(buf, q_ptr, FALSE, 0);


	/* Misc info */
	strcpy(dam, "");

	/* Damage */
	switch (q_ptr->tval)
	{
		/* Bows */
	case TV_BOW:
		{
			break;
		}

		/* Ammo */
	case TV_SHOT:
	case TV_BOLT:
	case TV_ARROW:

		/* Boomerangs */
	case TV_BOOMERANG:

		/* Weapons */
	case TV_HAFTED:
	case TV_POLEARM:
	case TV_SWORD:
	case TV_AXE:
	case TV_MSTAFF:

		/* Tools */
	case TV_DIGGING:
		{
			sprintf(dam, "%dd%d", q_ptr->dd, q_ptr->ds);
			break;
		}

		/* Armour */
	case TV_BOOTS:
	case TV_GLOVES:
	case TV_CLOAK:
	case TV_CROWN:
	case TV_HELM:
	case TV_SHIELD:
	case TV_SOFT_ARMOR:
	case TV_HARD_ARMOR:
	case TV_DRAG_ARMOR:
		{
			sprintf(dam, "%d", q_ptr->ac);
			break;
		}
	}


	/* Weight */
	sprintf(wgt, "%3ld.%ld", q_ptr->weight / 10, q_ptr->weight % 10);
}


/*
 * Create a spoiler file for items
 */
static void spoil_obj_desc(cptr fname)
{
	int i, k, s, t, n = 0;

	u16b who[200];

	char buf[1024];

	char wgt[80];
	char dam[80];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}


	/* Header */
	sprintf(buf, "Basic Items Spoilers for %s %ld.%ld.%ld%s",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);
	spoiler_underline(buf);
	spoiler_blanklines(2);

	/* More Header */
	fprintf(fff, "%-45s     %8s%7s%5s%9s\n",
	        "Description", "Dam/AC", "Wgt", "Lev", "Cost");
	fprintf(fff, "%-45s     %8s%7s%5s%9s\n",
	        "----------------------------------------",
	        "------", "---", "---", "----");

	/* List the groups */
	for (i = 0; TRUE; i++)
	{
		/* Write out the group title */
		if (group_item[i].name)
		{
			/* Hack -- bubble-sort by cost and then level */
			for (s = 0; s < n - 1; s++)
			{
				for (t = 0; t < n - 1; t++)
				{
					int i1 = t;
					int i2 = t + 1;

					int e1;
					int e2;

					s32b t1;
					s32b t2;

					kind_info(NULL, NULL, NULL, &e1, &t1, who[i1]);
					kind_info(NULL, NULL, NULL, &e2, &t2, who[i2]);

					if ((t1 > t2) || ((t1 == t2) && (e1 > e2)))
					{
						int tmp = who[i1];
						who[i1] = who[i2];
						who[i2] = tmp;
					}
				}
			}

			/* Spoil each item */
			for (s = 0; s < n; s++)
			{
				int e;
				s32b v;

				/* Describe the kind */
				kind_info(buf, dam, wgt, &e, &v, who[s]);

				/* Dump it */
				fprintf(fff, "     %-45s%8s%7s%5d%9ld\n",
				        buf, dam, wgt, e, (long)(v));
			}

			/* Start a new set */
			n = 0;

			/* Notice the end */
			if (!group_item[i].tval) break;

			/* Start a new set */
			fprintf(fff, "\n\n%s\n\n", group_item[i].name);
		}

		/* Acquire legal item types */
		for (k = 1; k < max_k_idx; k++)
		{
			object_kind *k_ptr = &k_info[k];

			/* Skip wrong tval's */
			if (k_ptr->tval != group_item[i].tval) continue;

			/* Hack -- Skip artifacts */
			if (k_ptr->flags3 & (TR3_INSTA_ART | TR3_NORM_ART)) continue;

			/* Hack -- Skip Ring of Powers */
			if (k == 785) continue;

			/* Save the index */
			who[n++] = k;
		}
	}


	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}



/*
 * Artifact Spoilers by: randy@PICARD.tamu.edu (Randy Hutson)
 */


/*
 * Returns a "+" string if a number is non-negative and an empty
 * string if negative
 */
#define POSITIZE(v) (((v) >= 0) ? "+" : "")

/*
 * These are used to format the artifact spoiler file. INDENT1 is used
 * to indent all but the first line of an artifact spoiler. INDENT2 is
 * used when a line "wraps". (Bladeturner's resistances cause this.)
 */
#define INDENT1 "    "
#define INDENT2 "      "

/*
 * MAX_LINE_LEN specifies when a line should wrap.
 */
#define MAX_LINE_LEN 75

/*
 * Given an array, determine how many elements are in the array
 */
#define N_ELEMENTS(a) (sizeof (a) / sizeof ((a)[0]))

/*
 * The artifacts categorized by type
 */
static grouper group_artifact[] =
{
	{ TV_SWORD, "Edged Weapons" },
	{ TV_POLEARM, "Polearms" },
	{ TV_HAFTED, "Hafted Weapons" },
	{ TV_AXE, "Axes" },

	{ TV_MSTAFF, "Mage Staffs" },

	{ TV_BOW, "Bows" },

	{ TV_SHOT, "Ammo" },
	{ TV_ARROW, NULL },
	{ TV_BOLT, NULL },

	{ TV_BOOMERANG, "Boomerangs" },

	{ TV_INSTRUMENT, "Instruments" },

	{ TV_SOFT_ARMOR, "Body Armor" },
	{ TV_HARD_ARMOR, NULL },
	{ TV_DRAG_ARMOR, NULL },

	{ TV_CLOAK, "Cloaks" },
	{ TV_SHIELD, "Shields" },
	{ TV_HELM, "Helms/Crowns" },
	{ TV_CROWN, NULL },
	{ TV_GLOVES, "Gloves" },
	{ TV_BOOTS, "Boots" },

	{ TV_DAEMON_BOOK, "Demonic Equipment" },

	{ TV_LITE, "Light Sources" },
	{ TV_AMULET, "Amulets" },
	{ TV_RING, "Rings" },

	{ TV_TOOL, "Tools" },
	{ TV_DIGGING, NULL },
	{ TV_TRAPKIT, "Trapping Kits" },

	{ 0, NULL }
};



/*
 * Pair together a constant flag with a textual description.
 *
 * Used by both "init.c" and "wiz-spo.c".
 *
 * Note that it sometimes more efficient to actually make an array
 * of textual names, where entry 'N' is assumed to be paired with
 * the flag whose value is "1L << N", but that requires hard-coding.
 */

typedef struct flag_desc flag_desc;

struct flag_desc
{
	const u32b flag;
	const char *const desc;
};



/*
 * These are used for "+3 to STR, DEX", etc. These are separate from
 * the other pval affected traits to simplify the case where an object
 * affects all stats.  In this case, "All stats" is used instead of
 * listing each stat individually.
 */

static flag_desc stat_flags_desc[] =
{
	{ TR1_STR, "STR" },
	{ TR1_INT, "INT" },
	{ TR1_WIS, "WIS" },
	{ TR1_DEX, "DEX" },
	{ TR1_CON, "CON" },
	{ TR1_CHR, "CHR" }
};

/*
 * Besides stats, these are the other player traits
 * which may be affected by an object's pval
 */

static flag_desc pval_flags1_desc[] =
{
	{ TR1_STEALTH, "Stealth" },
	{ TR1_SEARCH, "Searching" },
	{ TR1_INFRA, "Infravision" },
	{ TR1_TUNNEL, "Tunnelling" },
	{ TR1_BLOWS, "Attacks" },
	{ TR1_SPEED, "Speed" }
};

/*
 * Slaying preferences for weapons
 */

static flag_desc slay_flags_desc[] =
{
	{ TR1_SLAY_ANIMAL, "Animal" },
	{ TR1_SLAY_EVIL, "Evil" },
	{ TR1_SLAY_UNDEAD, "Undead" },
	{ TR1_SLAY_DEMON, "Demon" },
	{ TR1_SLAY_ORC, "Orc" },
	{ TR1_SLAY_TROLL, "Troll" },
	{ TR1_SLAY_GIANT, "Giant" },
	{ TR1_SLAY_DRAGON, "Dragon" },
	{ TR1_KILL_DRAGON, "Xdragon" }
};

/*
 * Elemental brands for weapons
 *
 * Clearly, TR1_IMPACT is a bit out of place here. To simplify
 * coding, it has been included here along with the elemental
 * brands. It does seem to fit in with the brands and slaying
 * more than the miscellaneous section.
 */
static flag_desc brand_flags_desc[] =
{
	{ TR1_BRAND_ACID, "Acid Brand" },
	{ TR1_BRAND_ELEC, "Lightning Brand" },
	{ TR1_BRAND_FIRE, "Flame Tongue" },
	{ TR1_BRAND_COLD, "Frost Brand" },
	{ TR1_BRAND_POIS, "Poisoned" },

	{ TR1_CHAOTIC, "Mark of Chaos" },
	{ TR1_VAMPIRIC, "Vampiric" },
	{ TR1_IMPACT, "Earthquake impact on hit" },
	{ TR1_VORPAL, "Very sharp" },
};


/*
 * The 15 resistables
 */
static const flag_desc resist_flags_desc[] =
{
	{ TR2_RES_ACID, "Acid" },
	{ TR2_RES_ELEC, "Lightning" },
	{ TR2_RES_FIRE, "Fire" },
	{ TR2_RES_COLD, "Cold" },
	{ TR2_RES_POIS, "Poison" },
	{ TR2_RES_FEAR, "Fear"},
	{ TR2_RES_LITE, "Light" },
	{ TR2_RES_DARK, "Dark" },
	{ TR2_RES_BLIND, "Blindness" },
	{ TR2_RES_CONF, "Confusion" },
	{ TR2_RES_SOUND, "Sound" },
	{ TR2_RES_SHARDS, "Shards" },
	{ TR2_RES_NETHER, "Nether" },
	{ TR2_RES_NEXUS, "Nexus" },
	{ TR2_RES_CHAOS, "Chaos" },
	{ TR2_RES_DISEN, "Disenchantment" },
};

/*
 * Elemental immunities (along with poison)
 */

static const flag_desc immune_flags_desc[] =
{
	{ TR2_IM_ACID, "Acid" },
	{ TR2_IM_ELEC, "Lightning" },
	{ TR2_IM_FIRE, "Fire" },
	{ TR2_IM_COLD, "Cold" },
};

/*
 * Sustain stats -  these are given their "own" line in the
 * spoiler file, mainly for simplicity
 */
static const flag_desc sustain_flags_desc[] =
{
	{ TR2_SUST_STR, "STR" },
	{ TR2_SUST_INT, "INT" },
	{ TR2_SUST_WIS, "WIS" },
	{ TR2_SUST_DEX, "DEX" },
	{ TR2_SUST_CON, "CON" },
	{ TR2_SUST_CHR, "CHR" },
};

/*
 * Miscellaneous magic given by an object's "flags2" field
 */

static const flag_desc misc_flags2_desc[] =
{
	{ TR2_REFLECT, "Reflection" },
	{ TR2_FREE_ACT, "Free Action" },
	{ TR2_HOLD_LIFE, "Hold Life" },
};

/*
 * Miscellaneous magic given by an object's "flags3" field
 *
 * Note that cursed artifacts and objects with permanent light
 * are handled "directly" -- see analyze_misc_magic()
 */

static const flag_desc misc_flags3_desc[] =
{
	{ TR3_SH_FIRE, "Fiery Aura" },
	{ TR3_SH_ELEC, "Electric Aura" },
	{ TR3_NO_TELE, "Prevent Teleportation" },
	{ TR3_NO_MAGIC, "Anti-Magic" },
	{ TR3_WRAITH, "Wraith Form" },
	{ TR3_FEATHER, "Levitation" },
	{ TR3_SEE_INVIS, "See Invisible" },
	{ TR3_SLOW_DIGEST, "Slow Digestion" },
	{ TR3_REGEN, "Regeneration" },
	{ TR3_XTRA_SHOTS, "+1 Extra Shot" },         /* always +1? */
	{ TR3_DRAIN_EXP, "Drains Experience" },
	{ TR3_AGGRAVATE, "Aggravates" },
	{ TR3_BLESSED, "Blessed Blade" },
};


/*
 * A special type used just for dealing with pvals
 */
typedef struct
{
	/*
	 * This will contain a string such as "+2", "-10", etc.
	 */
	char pval_desc[12];

	/*
	 * A list of various player traits affected by an object's pval such
	 * as stats, speed, stealth, etc.  "Extra attacks" is NOT included in
	 * this list since it will probably be desirable to format its
	 * description differently.
	 *
	 * Note that room need only be reserved for the number of stats - 1
	 * since the description "All stats" is used if an object affects all
	 * all stats. Also, room must be reserved for a sentinel NULL pointer.
	 *
	 * This will be a list such as ["STR", "DEX", "Stealth", NULL] etc.
	 *
	 * This list includes extra attacks, for simplicity.
	 */
	cptr pval_affects[N_ELEMENTS(stat_flags_desc) - 1 +
	                  N_ELEMENTS(pval_flags1_desc) + 1];

}
pval_info_type;


/*
 * An "object analysis structure"
 *
 * It will be filled with descriptive strings detailing an object's
 * various magical powers. The "ignore X" traits are not noted since
 * all artifacts ignore "normal" destruction.
 */

typedef struct
{
	/* "The Longsword Dragonsmiter (6d4) (+20, +25)" */
	char description[160];

	/* Description of what is affected by an object's pval */
	pval_info_type pval_info;

	/* A list of an object's slaying preferences */
	cptr slays[N_ELEMENTS(slay_flags_desc) + 1];

	/* A list if an object's elemental brands */
	cptr brands[N_ELEMENTS(brand_flags_desc) + 1];

	/* A list of immunities granted by an object */
	cptr immunities[N_ELEMENTS(immune_flags_desc) + 1];

	/* A list of resistances granted by an object */
	cptr resistances[N_ELEMENTS(resist_flags_desc) + 1];

	/* A list of stats sustained by an object */
	cptr sustains[N_ELEMENTS(sustain_flags_desc) - 1 + 1];

	/* A list of various magical qualities an object may have */
	cptr misc_magic[N_ELEMENTS(misc_flags2_desc) + N_ELEMENTS(misc_flags3_desc)
	                + 1       /* Permanent Light */
	                + 1       /* type of curse */
	                + 1];      /* sentinel NULL */

	/* A string describing an artifact's activation */
	cptr activation;

	/* "Level 20, Rarity 30, 3.0 lbs, 20000 Gold" */
	char misc_desc[80];
}
obj_desc_list;






/*
 * This function does most of the actual "analysis". Given a set of bit flags
 * (which will be from one of the flags fields from the object in question),
 * a "flag description structure", a "description list", and the number of
 * elements in the "flag description structure", this function sets the
 * "description list" members to the appropriate descriptions contained in
 * the "flag description structure".
 *
 * The possibly updated description pointer is returned.
 */
static cptr *spoiler_flag_aux(const u32b art_flags, const flag_desc *flag_ptr,
                              cptr *desc_ptr, const int n_elmnts)
{
	int i;

	for (i = 0; i < n_elmnts; ++i)
	{
		if (art_flags & flag_ptr[i].flag)
		{
			*desc_ptr++ = flag_ptr[i].desc;
		}
	}

	return desc_ptr;
}


/*
 * Acquire a "basic" description "The Cloak of Death [1,+10]"
 */
static void analyze_general (object_type *o_ptr, char *desc_ptr)
{
	/* Get a "useful" description of the object */
	object_desc_store(desc_ptr, o_ptr, TRUE, 1);
}


/*
 * List "player traits" altered by an artifact's pval. These include stats,
 * speed, infravision, tunnelling, stealth, searching, and extra attacks.
 */
static void analyze_pval (object_type *o_ptr, pval_info_type *p_ptr)
{
	const u32b all_stats = (TR1_STR | TR1_INT | TR1_WIS |
	                        TR1_DEX | TR1_CON | TR1_CHR);

	u32b f1, f2, f3, f4, f5, esp;

	cptr *affects_list;

	/* If pval == 0, there is nothing to do. */
	if (!o_ptr->pval)
	{
		/* An "empty" pval description indicates that pval == 0 */
		p_ptr->pval_desc[0] = '\0';
		return;
	}

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	affects_list = p_ptr->pval_affects;

	/* Create the "+N" string */
	sprintf(p_ptr->pval_desc, "%s%ld", POSITIZE(o_ptr->pval), o_ptr->pval);

	/* First, check to see if the pval affects all stats */
	if ((f1 & all_stats) == all_stats)
	{
		*affects_list++ = "All stats";
	}

	/* Are any stats affected? */
	else if (f1 & all_stats)
	{
		affects_list = spoiler_flag_aux(f1, stat_flags_desc,
		                                affects_list,
		                                N_ELEMENTS(stat_flags_desc));
	}

	/* And now the "rest" */
	affects_list = spoiler_flag_aux(f1, pval_flags1_desc,
	                                affects_list,
	                                N_ELEMENTS(pval_flags1_desc));

	/* Terminate the description list */
	*affects_list = NULL;
}


/* Note the slaying specialties of a weapon */
static void analyze_slay (object_type *o_ptr, cptr *slay_list)
{
	u32b f1, f2, f3, f4, f5, esp;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	slay_list = spoiler_flag_aux(f1, slay_flags_desc, slay_list,
	                             N_ELEMENTS(slay_flags_desc));

	/* Terminate the description list */
	*slay_list = NULL;
}

/* Note an object's elemental brands */
static void analyze_brand (object_type *o_ptr, cptr *brand_list)
{
	u32b f1, f2, f3, f4, f5, esp;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	brand_list = spoiler_flag_aux(f1, brand_flags_desc, brand_list,
	                              N_ELEMENTS(brand_flags_desc));

	/* Terminate the description list */
	*brand_list = NULL;
}


/* Note the resistances granted by an object */
static void analyze_resist (object_type *o_ptr, cptr *resist_list)
{
	u32b f1, f2, f3, f4, f5, esp;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	resist_list = spoiler_flag_aux(f2, resist_flags_desc,
	                               resist_list, N_ELEMENTS(resist_flags_desc));

	/* Terminate the description list */
	*resist_list = NULL;
}


/* Note the immunities granted by an object */
static void analyze_immune (object_type *o_ptr, cptr *immune_list)
{
	u32b f1, f2, f3, f4, f5, esp;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	immune_list = spoiler_flag_aux(f2, immune_flags_desc,
	                               immune_list, N_ELEMENTS(immune_flags_desc));

	/* Terminate the description list */
	*immune_list = NULL;
}

/* Note which stats an object sustains */

static void analyze_sustains (object_type *o_ptr, cptr *sustain_list)
{
	const u32b all_sustains = (TR2_SUST_STR | TR2_SUST_INT | TR2_SUST_WIS |
	                           TR2_SUST_DEX | TR2_SUST_CON | TR2_SUST_CHR);

	u32b f1, f2, f3, f4, f5, esp;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Simplify things if an item sustains all stats */
	if ((f2 & all_sustains) == all_sustains)
	{
		*sustain_list++ = "All stats";
	}

	/* Should we bother? */
	else if ((f2 & all_sustains))
	{
		sustain_list = spoiler_flag_aux(f2, sustain_flags_desc,
		                                sustain_list,
		                                N_ELEMENTS(sustain_flags_desc));
	}

	/* Terminate the description list */
	*sustain_list = NULL;
}


/*
 * Note miscellaneous powers bestowed by an artifact such as see invisible,
 * free action, permanent light, etc.
 */
static void analyze_misc_magic (object_type *o_ptr, cptr *misc_list)
{
	u32b f1, f2, f3, f4, f5, esp;
	int radius = 0;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	misc_list = spoiler_flag_aux(f2, misc_flags2_desc, misc_list,
	                             N_ELEMENTS(misc_flags2_desc));

	misc_list = spoiler_flag_aux(f3, misc_flags3_desc, misc_list,
	                             N_ELEMENTS(misc_flags3_desc));

	/*
	 * Glowing artifacts -- small radius light.
	 */

	if (f3 & TR3_LITE1) radius++;
	if (f4 & TR4_LITE2) radius += 2;
	if (f4 & TR4_LITE3) radius += 3;

	if (f4 & TR4_FUEL_LITE)
	{
		*misc_list++ = format("It provides light (radius %d) forever.", radius);
	}
	else
	{
		*misc_list++ = format("It provides light (radius %d) when fueled.", radius);
	}

	/*
	 * Handle cursed objects here to avoid redundancies such as noting
	 * that a permanently cursed object is heavily cursed as well as
	 * being "lightly cursed".
	 */

	if (cursed_p(o_ptr))
	{
		if (f3 & (TR3_TY_CURSE))
		{
			*misc_list++ = "Ancient Curse";
		}
		if (f3 & (TR3_PERMA_CURSE))
		{
			*misc_list++ = "Permanently Cursed";
		}
		else if (f3 & (TR3_HEAVY_CURSE))
		{
			*misc_list++ = "Heavily Cursed";
		}
		else
		{
			*misc_list++ = "Cursed";
		}
	}

	/* Terminate the description list */
	*misc_list = NULL;
}




/*
 * Determine the minimum depth an artifact can appear, its rarity, its weight,
 * and its value in gold pieces
 */
static void analyze_misc (object_type *o_ptr, char *misc_desc)
{
	artifact_type *a_ptr = &a_info[o_ptr->name1];

	sprintf(misc_desc, "Level %u, Rarity %u, %d.%d lbs, %ld Gold",
	        a_ptr->level, a_ptr->rarity,
	        a_ptr->weight / 10, a_ptr->weight % 10, a_ptr->cost);
}


/*
 * Fill in an object description structure for a given object
 */
static void object_analyze(object_type *o_ptr, obj_desc_list *desc_ptr)
{
	analyze_general(o_ptr, desc_ptr->description);

	analyze_pval(o_ptr, &desc_ptr->pval_info);

	analyze_brand(o_ptr, desc_ptr->brands);

	analyze_slay(o_ptr, desc_ptr->slays);

	analyze_immune(o_ptr, desc_ptr->immunities);

	analyze_resist(o_ptr, desc_ptr->resistances);

	analyze_sustains(o_ptr, desc_ptr->sustains);

	analyze_misc_magic(o_ptr, desc_ptr->misc_magic);

	analyze_misc(o_ptr, desc_ptr->misc_desc);

	desc_ptr->activation = item_activation(o_ptr, 0);
}


static void print_header(void)
{
	char buf[80];

	sprintf(buf, "Artifact Spoilers for %s %ld.%ld.%ld%s",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);
	spoiler_underline(buf);
}

/*
 * This is somewhat ugly.
 *
 * Given a header ("Resist", e.g.), a list ("Fire", "Cold", Acid", e.g.),
 * and a separator character (',', e.g.), write the list to the spoiler file
 * in a "nice" format, such as:
 *
 *      Resist Fire, Cold, Acid
 *
 * That was a simple example, but when the list is long, a line wrap
 * should occur, and this should induce a new level of indention if
 * a list is being spread across lines. So for example, Bladeturner's
 * list of resistances should look something like this
 *
 *     Resist Acid, Lightning, Fire, Cold, Poison, Light, Dark, Blindness,
 *       Confusion, Sound, Shards, Nether, Nexus, Chaos, Disenchantment
 *
 * However, the code distinguishes between a single list of many items vs.
 * many lists. (The separator is used to make this determination.) A single
 * list of many items will not cause line wrapping (since there is no
 * apparent reason to do so). So the lists of Ulmo's miscellaneous traits
 * might look like this:
 *
 *     Free Action; Hold Life; See Invisible; Slow Digestion; Regeneration
 *     Blessed Blade
 *
 * So comparing the two, "Regeneration" has no trailing separator and
 * "Blessed Blade" was not indented. (Also, Ulmo's lists have no headers,
 * but that's not relevant to line wrapping and indention.)
 */

/* ITEM_SEP separates items within a list */
#define ITEM_SEP ','


/* LIST_SEP separates lists */
#define LIST_SEP ';'


/* Create a spoiler file entry for an artifact */

static void spoiler_print_art(obj_desc_list *art_ptr, int name1, int set, object_type *o_ptr)
{
	/* Don't indent the first line */
#if 0 // DGDGDGDG
	fprintf(fff, "<P>%s<BR>", art_ptr->description);
#else
fprintf(fff, "%s\n    ", art_ptr->description);
#endif
	text_out_indent = 4;
	object_out_desc(o_ptr, fff, FALSE, TRUE);
	text_out_indent = 0;

	/* End with the miscellaneous facts */
#if 0 // DGDGDGDG
	fprintf(fff, "<BR>%s</P>", art_ptr->misc_desc);
#else
	fprintf(fff, "%s%s\n\n", INDENT1, art_ptr->misc_desc);
#endif
}


/*
 * Hack -- Create a "forged" artifact
 */
static bool make_fake_artifact(object_type *o_ptr, int name1)
{
	int i;
	int cur;

	artifact_type *a_ptr = &a_info[name1];


	/* Ignore "empty" artifacts */
	if (!a_ptr->name) return FALSE;

	/* Acquire the "kind" index */
	i = lookup_kind(a_ptr->tval, a_ptr->sval);

	/* Oops */
	if (!i) return (FALSE);

	/* Create the artifact */
	object_prep(o_ptr, i);

	/* Save the name */
	o_ptr->name1 = name1;

	/* Extract the fields */
#if 0
	o_ptr->pval = a_ptr->pval;
	o_ptr->ac = a_ptr->ac;
	o_ptr->dd = a_ptr->dd;
	o_ptr->ds = a_ptr->ds;
	o_ptr->to_a = a_ptr->to_a;
	o_ptr->to_h = a_ptr->to_h;
	o_ptr->to_d = a_ptr->to_d;
	o_ptr->weight = a_ptr->weight;
#else
	/* Keep the One Ring untouched by apply_magic */
	if (name1 != ART_POWER)
	{
		cur = a_ptr->cur_num;
		apply_magic(o_ptr, -1, TRUE, TRUE, TRUE);
		a_ptr->cur_num = cur;
	}
	else
	{
		o_ptr->pval = a_ptr->pval;
	}
#endif

	/* Success */
	return (TRUE);
}


/*
 * Create a spoiler file for artifacts
 */
static void spoil_artifact(cptr fname)
{
	int i, j;

	object_type forge;
	object_type *q_ptr;

	obj_desc_list artifact;

	char buf[1024];


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

#if 0 // DGDGDGDG
	fprintf(fff, "<HTML><BODY BGCOLOR=#000000 TEXT=#CCCCCC>");
#endif
	/* Dump the header */
	print_header();

	/* List the artifacts by tval */
	for (i = 0; group_artifact[i].tval; i++)
	{
		/* Write out the group title */
		if (group_artifact[i].name)
		{
			spoiler_blanklines(2);
			spoiler_underline(group_artifact[i].name);
			spoiler_blanklines(1);
		}

		/* Now search through all of the artifacts */
		for (j = 1; j < max_a_idx; ++j)
		{
			artifact_type *a_ptr = &a_info[j];

			/* We only want objects in the current group */
			if (a_ptr->tval != group_artifact[i].tval) continue;

			/* Get local object */
			q_ptr = &forge;

			/* Wipe the object */
			object_wipe(q_ptr);

			/* Attempt to "forge" the artifact */
			if (!make_fake_artifact(q_ptr, j)) continue;

			/* Aware and Known */
			object_known(q_ptr);

			/* Mark the item as fully known */
			q_ptr->ident |= (IDENT_MENTAL);

			/* Analyze the artifact */
			object_analyze(q_ptr, &artifact);

			/* Write out the artifact description to the spoiler file */
			spoiler_print_art(&artifact, j, a_ptr->set, q_ptr);
		}
	}

#if 0 // DGDGDGDG
	fprintf(fff, "</BODY></HTML>");
#endif
	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}





/*
 * Create a spoiler file for monsters   -BEN-
 */
static void spoil_mon_desc(cptr fname)
{
	int i, n = 0;

	s16b *who;

	char buf[1024];

	char nam[80];
	char lev[80];
	char rar[80];
	char spd[80];
	char ac[80];
	char hp[80];
	char exp[80];

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Allocate the "who" array */
	C_MAKE(who, max_r_idx, s16b);

	/* Dump the header */
	sprintf(buf, "Monster Spoilers for %s %ld.%ld.%ld%s",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);
	spoiler_underline(buf);
	spoiler_blanklines(2);

	/* Dump the header */
	fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
	        "Name", "Lev", "Rar", "Spd", "Hp", "Ac", "Visual Info");
	fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
	        "----", "---", "---", "---", "--", "--", "-----------");


	/* Scan the monsters */
	for (i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Use that monster */
		if (r_ptr->name) who[n++] = i;
	}


	/* Scan again */
	for (i = 0; i < n; i++)
	{
		monster_race *r_ptr = &r_info[who[i]];

		cptr name = (r_name + r_ptr->name);

		/* Get the "name" */
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			sprintf(nam, "[U] %s", name);
		}
		else
		{
			sprintf(nam, "The %s", name);
		}


		/* Level */
		sprintf(lev, "%d", r_ptr->level);

		/* Rarity */
		sprintf(rar, "%d", r_ptr->rarity);

		/* Speed */
		if (r_ptr->speed >= 110)
		{
			sprintf(spd, "+%d", (r_ptr->speed - 110));
		}
		else
		{
			sprintf(spd, "-%d", (110 - r_ptr->speed));
		}

		/* Armor Class */
		sprintf(ac, "%d", r_ptr->ac);

		/* Hitpoints */
		if ((r_ptr->flags1 & (RF1_FORCE_MAXHP)) || (r_ptr->hside == 1))
		{
			sprintf(hp, "%d", r_ptr->hdice * r_ptr->hside);
		}
		else
		{
			sprintf(hp, "%dd%d", r_ptr->hdice, r_ptr->hside);
		}


		/* Experience */
		sprintf(exp, "%ld", (long)(r_ptr->mexp));

		/* Hack -- use visual instead */
		sprintf(exp, "%s '%c'", attr_to_text(r_ptr->d_attr), r_ptr->d_char);

		/* Dump the info */
		fprintf(fff, "%-40.40s%4s%4s%6s%8s%4s  %11.11s\n",
		        nam, lev, rar, spd, hp, ac, exp);
	}

	/* End it */
	fprintf(fff, "\n");

	/* Free the "who" array */
	C_KILL(who, max_r_idx, s16b);

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Worked */
	msg_print("Successfully created a spoiler file.");
}




/*
 * Monster spoilers by: smchorse@ringer.cs.utsa.edu (Shawn McHorse)
 *
 * Primarily based on code already in mon-desc.c, mostly by -BEN-
 */

/*
 * Pronoun arrays
 */
static cptr wd_che[3] = { "It", "He", "She" };
static cptr wd_lhe[3] = { "it", "he", "she" };



/*
 * Create a spoiler file for monsters (-SHAWN-)
 */
static void spoil_mon_info(cptr fname)
{
	char buf[1024];
	int msex, vn, i, j, k, n;
	bool breath, magic, sin;
	cptr p, q;
	cptr vp[64];
	u32b flags1, flags2, flags3, flags4, flags5, flags6, flags9;


	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	/* Open the file */
	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}


	/* Dump the header */
	sprintf(buf, "Monster Spoilers for %s %ld.%ld.%ld%s",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);
	spoiler_underline(buf);
	spoiler_blanklines(2);

	/*
	 * List all monsters in order.
	 */
	for (n = 1; n < max_r_idx; n++)
	{
		monster_race *r_ptr = &r_info[n];

		/* Extract the flags */
		flags1 = r_ptr->flags1;
		flags2 = r_ptr->flags2;
		flags3 = r_ptr->flags3;
		flags4 = r_ptr->flags4;
		flags5 = r_ptr->flags5;
		flags6 = r_ptr->flags6;
		flags9 = r_ptr->flags9;
		breath = FALSE;
		magic = FALSE;

		/* Extract a gender (if applicable) */
		if (flags1 & (RF1_FEMALE)) msex = 2;
		else if (flags1 & (RF1_MALE)) msex = 1;
		else msex = 0;


		/* Prefix */
		if (flags1 & (RF1_UNIQUE))
		{
			spoil_out("[U] ");
		}
		else
		{
			spoil_out("The ");
		}

		/* Name */
		sprintf(buf, "%s  (", (r_name + r_ptr->name));   /* ---)--- */
		spoil_out(buf);

		/* Color */
		spoil_out(attr_to_text(r_ptr->d_attr));

		/* Symbol --(-- */
		sprintf(buf, " '%c')\n", r_ptr->d_char);
		spoil_out(buf);


		/* Indent */
		sprintf(buf, "=== ");
		spoil_out(buf);

		/* Number */
		sprintf(buf, "Num:%d  ", n);
		spoil_out(buf);

		/* Level */
		sprintf(buf, "Lev:%d  ", r_ptr->level);
		spoil_out(buf);

		/* Rarity */
		sprintf(buf, "Rar:%d  ", r_ptr->rarity);
		spoil_out(buf);

		/* Speed */
		if (r_ptr->speed >= 110)
		{
			sprintf(buf, "Spd:+%d  ", (r_ptr->speed - 110));
		}
		else
		{
			sprintf(buf, "Spd:-%d  ", (110 - r_ptr->speed));
		}
		spoil_out(buf);

		/* Hitpoints */
		if ((flags1 & (RF1_FORCE_MAXHP)) || (r_ptr->hside == 1))
		{
			sprintf(buf, "Hp:%d  ", r_ptr->hdice * r_ptr->hside);
		}
		else
		{
			sprintf(buf, "Hp:%dd%d  ", r_ptr->hdice, r_ptr->hside);
		}
		spoil_out(buf);

		/* Armor Class */
		sprintf(buf, "Ac:%d  ", r_ptr->ac);
		spoil_out(buf);

		/* Experience */
		sprintf(buf, "Exp:%ld\n", (long)(r_ptr->mexp));
		spoil_out(buf);


		/* Describe */
		spoil_out(r_text + r_ptr->text);
		spoil_out(" ");


		spoil_out("This");

		if (flags2 & (RF2_ELDRITCH_HORROR)) spoil_out (" sanity-blasting");
		if (flags3 & (RF3_ANIMAL)) spoil_out(" natural");
		if (flags3 & (RF3_EVIL)) spoil_out(" evil");
		if (flags3 & (RF3_GOOD)) spoil_out(" good");
		if (flags3 & (RF3_UNDEAD)) spoil_out(" undead");

		if (flags3 & (RF3_DRAGON)) spoil_out(" dragon");
		else if (flags3 & (RF3_DEMON)) spoil_out(" demon");
		else if (flags3 & (RF3_GIANT)) spoil_out(" giant");
		else if (flags3 & (RF3_TROLL)) spoil_out(" troll");
		else if (flags3 & (RF3_ORC)) spoil_out(" orc");
		else if (flags3 & (RF3_THUNDERLORD)) spoil_out (" Thunderlord");
		else spoil_out(" creature");

		spoil_out(" moves");

		if ((flags1 & (RF1_RAND_50)) && (flags1 & (RF1_RAND_25)))
		{
			spoil_out(" extremely erratically");
		}
		else if (flags1 & (RF1_RAND_50))
		{
			spoil_out(" somewhat erratically");
		}
		else if (flags1 & (RF1_RAND_25))
		{
			spoil_out(" a bit erratically");
		}
		else
		{
			spoil_out(" normally");
		}

		if (flags1 & (RF1_NEVER_MOVE))
		{
			spoil_out(", but does not deign to chase intruders");
		}

		spoil_out(".  ");

		if (!r_ptr->level || (flags1 & (RF1_FORCE_DEPTH)))
		{
			sprintf(buf, "%s is never found out of depth.  ", wd_che[msex]);
			spoil_out(buf);
		}

		if (flags1 & (RF1_FORCE_SLEEP))
		{
			sprintf(buf, "%s is always created sluggish.  ", wd_che[msex]);
			spoil_out(buf);
		}

		if (flags2 & (RF2_AURA_FIRE))
		{
			sprintf(buf, "%s is surrounded by flames.  ", wd_che[msex]);
			spoil_out(buf);
		}

		if (flags2 & (RF2_AURA_ELEC))
		{
			sprintf(buf, "%s is surrounded by electricity.  ", wd_che[msex]);
			spoil_out(buf);
		}

		if (flags2 & (RF2_REFLECTING))
		{
			sprintf(buf, "%s reflects bolt spells.  ", wd_che[msex]);
			spoil_out(buf);
		}

		if (flags1 & (RF1_ESCORT))
		{
			sprintf(buf, "%s usually appears with ", wd_che[msex]);
			spoil_out(buf);
			if (flags1 & (RF1_ESCORTS)) spoil_out("escorts.  ");
			else spoil_out("an escort.  ");
		}

		if ((flags1 & (RF1_FRIEND)) || (flags1 & (RF1_FRIENDS)))
		{
			sprintf(buf, "%s usually appears in groups.  ", wd_che[msex]);
			spoil_out(buf);
		}

		/* Collect innate attacks */
		vn = 0;
		if (flags4 & (RF4_SHRIEK)) vp[vn++] = "shriek for help";
		if (flags4 & (RF4_ROCKET)) vp[vn++] = "shoot a rocket";
		if (flags4 & (RF4_ARROW_1)) vp[vn++] = "fire arrows";
		if (flags4 & (RF4_ARROW_2)) vp[vn++] = "fire arrows";
		if (flags4 & (RF4_ARROW_3)) vp[vn++] = "fire missiles";
		if (flags4 & (RF4_ARROW_4)) vp[vn++] = "fire missiles";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" may ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" or ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		/* Collect breaths */
		vn = 0;
		if (flags4 & (RF4_BR_ACID)) vp[vn++] = "acid";
		if (flags4 & (RF4_BR_ELEC)) vp[vn++] = "lightning";
		if (flags4 & (RF4_BR_FIRE)) vp[vn++] = "fire";
		if (flags4 & (RF4_BR_COLD)) vp[vn++] = "frost";
		if (flags4 & (RF4_BR_POIS)) vp[vn++] = "poison";
		if (flags4 & (RF4_BR_NETH)) vp[vn++] = "nether";
		if (flags4 & (RF4_BR_LITE)) vp[vn++] = "light";
		if (flags4 & (RF4_BR_DARK)) vp[vn++] = "darkness";
		if (flags4 & (RF4_BR_CONF)) vp[vn++] = "confusion";
		if (flags4 & (RF4_BR_SOUN)) vp[vn++] = "sound";
		if (flags4 & (RF4_BR_CHAO)) vp[vn++] = "chaos";
		if (flags4 & (RF4_BR_DISE)) vp[vn++] = "disenchantment";
		if (flags4 & (RF4_BR_NEXU)) vp[vn++] = "nexus";
		if (flags4 & (RF4_BR_TIME)) vp[vn++] = "time";
		if (flags4 & (RF4_BR_INER)) vp[vn++] = "inertia";
		if (flags4 & (RF4_BR_GRAV)) vp[vn++] = "gravity";
		if (flags4 & (RF4_BR_SHAR)) vp[vn++] = "shards";
		if (flags4 & (RF4_BR_PLAS)) vp[vn++] = "plasma";
		if (flags4 & (RF4_BR_WALL)) vp[vn++] = "force";
		if (flags4 & (RF4_BR_MANA)) vp[vn++] = "mana";
		if (flags4 & (RF4_BR_NUKE)) vp[vn++] = "toxic waste";
		if (flags4 & (RF4_BR_DISI)) vp[vn++] = "disintegration";

		if (vn)
		{
			breath = TRUE;
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" may breathe ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" or ");
				spoil_out(vp[i]);
			}
			if (flags2 & (RF2_POWERFUL)) spoil_out(" powerfully");
		}

		/* Collect spells */
		vn = 0;
		if (flags5 & (RF5_BA_ACID)) vp[vn++] = "produce acid balls";
		if (flags5 & (RF5_BA_ELEC)) vp[vn++] = "produce lightning balls";
		if (flags5 & (RF5_BA_FIRE)) vp[vn++] = "produce fire balls";
		if (flags5 & (RF5_BA_COLD)) vp[vn++] = "produce frost balls";
		if (flags5 & (RF5_BA_POIS)) vp[vn++] = "produce poison balls";
		if (flags5 & (RF5_BA_NETH)) vp[vn++] = "produce nether balls";
		if (flags5 & (RF5_BA_WATE)) vp[vn++] = "produce water balls";
		if (flags4 & (RF4_BA_NUKE)) vp[vn++] = "produce balls of radiation";
		if (flags5 & (RF5_BA_MANA)) vp[vn++] = "produce mana storms";
		if (flags5 & (RF5_BA_DARK)) vp[vn++] = "produce darkness storms";
		if (flags4 & (RF4_BA_CHAO)) vp[vn++] = "invoke raw Chaos";
		if (flags6 & (RF6_HAND_DOOM)) vp[vn++] = "invoke the Hand of Doom";
		if (flags5 & (RF5_DRAIN_MANA)) vp[vn++] = "drain mana";
		if (flags5 & (RF5_MIND_BLAST)) vp[vn++] = "cause mind blasting";
		if (flags5 & (RF5_BRAIN_SMASH)) vp[vn++] = "cause brain smashing";
		if (flags5 & (RF5_CAUSE_1)) vp[vn++] = "cause light wounds and cursing";
		if (flags5 & (RF5_CAUSE_2)) vp[vn++] = "cause serious wounds and cursing";
		if (flags5 & (RF5_CAUSE_3)) vp[vn++] = "cause critical wounds and cursing";
		if (flags5 & (RF5_CAUSE_4)) vp[vn++] = "cause mortal wounds";
		if (flags5 & (RF5_BO_ACID)) vp[vn++] = "produce acid bolts";
		if (flags5 & (RF5_BO_ELEC)) vp[vn++] = "produce lightning bolts";
		if (flags5 & (RF5_BO_FIRE)) vp[vn++] = "produce fire bolts";
		if (flags5 & (RF5_BO_COLD)) vp[vn++] = "produce frost bolts";
		if (flags5 & (RF5_BO_POIS)) vp[vn++] = "produce poison bolts";
		if (flags5 & (RF5_BO_NETH)) vp[vn++] = "produce nether bolts";
		if (flags5 & (RF5_BO_WATE)) vp[vn++] = "produce water bolts";
		if (flags5 & (RF5_BO_MANA)) vp[vn++] = "produce mana bolts";
		if (flags5 & (RF5_BO_PLAS)) vp[vn++] = "produce plasma bolts";
		if (flags5 & (RF5_BO_ICEE)) vp[vn++] = "produce ice bolts";
		if (flags5 & (RF5_MISSILE)) vp[vn++] = "produce magic missiles";
		if (flags5 & (RF5_SCARE)) vp[vn++] = "terrify";
		if (flags5 & (RF5_BLIND)) vp[vn++] = "blind";
		if (flags5 & (RF5_CONF)) vp[vn++] = "confuse";
		if (flags5 & (RF5_SLOW)) vp[vn++] = "slow";
		if (flags5 & (RF5_HOLD)) vp[vn++] = "paralyse";
		if (flags6 & (RF6_HASTE)) vp[vn++] = "haste-self";
		if (flags6 & (RF6_HEAL)) vp[vn++] = "heal-self";
		if (flags6 & (RF6_BLINK)) vp[vn++] = "blink-self";
		if (flags6 & (RF6_TPORT)) vp[vn++] = "teleport-self";
		if (flags6 & (RF6_S_BUG)) vp[vn++] = "summon software bugs";
		if (flags6 & (RF6_S_RNG)) vp[vn++] = "summon RNGs";
		if (flags6 & (RF6_TELE_TO)) vp[vn++] = "teleport to";
		if (flags6 & (RF6_TELE_AWAY)) vp[vn++] = "teleport away";
		if (flags6 & (RF6_TELE_LEVEL)) vp[vn++] = "teleport level";
		if (flags6 & (RF6_DARKNESS)) vp[vn++] = "create darkness";
		if (flags6 & (RF6_TRAPS)) vp[vn++] = "create traps";
		if (flags6 & (RF6_FORGET)) vp[vn++] = "cause amnesia";
		if (flags6 & (RF6_RAISE_DEAD)) vp[vn++] = "raise dead";
		if (flags6 & (RF6_S_THUNDERLORD)) vp[vn++] = "summon a thunderlord";
		if (flags6 & (RF6_S_MONSTER)) vp[vn++] = "summon a monster";
		if (flags6 & (RF6_S_MONSTERS)) vp[vn++] = "summon monsters";
		if (flags6 & (RF6_S_KIN)) vp[vn++] = "summon aid";
		if (flags6 & (RF6_S_ANT)) vp[vn++] = "summon ants";
		if (flags6 & (RF6_S_SPIDER)) vp[vn++] = "summon spiders";
		if (flags6 & (RF6_S_HOUND)) vp[vn++] = "summon hounds";
		if (flags6 & (RF6_S_HYDRA)) vp[vn++] = "summon hydras";
		if (flags6 & (RF6_S_ANGEL)) vp[vn++] = "summon an angel";
		if (flags6 & (RF6_S_DEMON)) vp[vn++] = "summon a demon";
		if (flags6 & (RF6_S_UNDEAD)) vp[vn++] = "summon an undead";
		if (flags6 & (RF6_S_DRAGON)) vp[vn++] = "summon a dragon";
		if (flags4 & (RF4_S_ANIMAL)) vp[vn++] = "summon animal";
		if (flags6 & (RF6_S_ANIMALS)) vp[vn++] = "summon animals";
		if (flags6 & (RF6_S_HI_UNDEAD)) vp[vn++] = "summon greater undead";
		if (flags6 & (RF6_S_HI_DRAGON)) vp[vn++] = "summon ancient dragons";
		if (flags6 & (RF6_S_HI_DEMON)) vp[vn++] = "summon greater demons";
		if (flags6 & (RF6_S_WRAITH)) vp[vn++] = "summon Ringwraith";
		if (flags6 & (RF6_S_UNIQUE)) vp[vn++] = "summon unique monsters";

		if (vn)
		{
			magic = TRUE;
			if (breath)
			{
				spoil_out(", and is also");
			}
			else
			{
				spoil_out(wd_che[msex]);
				spoil_out(" is");
			}

			spoil_out(" magical, casting spells");
			if (flags2 & (RF2_SMART)) spoil_out(" intelligently");

			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" which ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" or ");
				spoil_out(vp[i]);
			}
		}

		if (breath || magic)
		{
			int times = r_ptr->freq_inate + r_ptr->freq_spell;
			sprintf(buf, "; 1 time in %d.  ",
			        200 / ((times) ? times : 1));
			spoil_out(buf);
		}

		/* Collect special abilities. */
		vn = 0;
		if (flags2 & (RF2_OPEN_DOOR)) vp[vn++] = "open doors";
		if (flags2 & (RF2_BASH_DOOR)) vp[vn++] = "bash down doors";
		if (flags2 & (RF2_PASS_WALL)) vp[vn++] = "pass through walls";
		if (flags2 & (RF2_KILL_WALL)) vp[vn++] = "bore through walls";
		if (flags2 & (RF2_MOVE_BODY)) vp[vn++] = "push past weaker monsters";
		if (flags2 & (RF2_KILL_BODY)) vp[vn++] = "destroy weaker monsters";
		if (flags2 & (RF2_TAKE_ITEM)) vp[vn++] = "pick up objects";
		if (flags2 & (RF2_KILL_ITEM)) vp[vn++] = "destroy objects";
		if (flags9 & (RF9_HAS_LITE)) vp[vn++] = "illuminate the dungeon";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" can ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" and ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		if (flags2 & (RF2_INVISIBLE))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" is invisible.  ");
		}
		if (flags2 & (RF2_COLD_BLOOD))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" is cold blooded.  ");
		}
		if (flags2 & (RF2_EMPTY_MIND))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" is not detected by telepathy.  ");
		}
		if (flags2 & (RF2_WEIRD_MIND))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" is rarely detected by telepathy.  ");
		}
		if (flags4 & (RF4_MULTIPLY))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" breeds explosively.  ");
		}
		if (flags2 & (RF2_REGENERATE))
		{
			spoil_out(wd_che[msex]);
			spoil_out(" regenerates quickly.  ");
		}

		/* Collect susceptibilities */
		vn = 0;
		if (flags3 & (RF3_HURT_ROCK)) vp[vn++] = "rock remover";
		if (flags3 & (RF3_HURT_LITE)) vp[vn++] = "bright light";
		if (flags3 & (RF3_SUSCEP_FIRE)) vp[vn++] = "fire";
		if (flags3 & (RF3_SUSCEP_COLD)) vp[vn++] = "cold";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" is hurt by ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" and ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		/* Collect immunities */
		vn = 0;
		if (flags3 & (RF3_IM_ACID)) vp[vn++] = "acid";
		if (flags3 & (RF3_IM_ELEC)) vp[vn++] = "lightning";
		if (flags3 & (RF3_IM_FIRE)) vp[vn++] = "fire";
		if (flags3 & (RF3_IM_COLD)) vp[vn++] = "cold";
		if (flags3 & (RF3_IM_POIS)) vp[vn++] = "poison";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" resists ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" and ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		/* Collect resistances */
		vn = 0;
		if (flags3 & (RF3_RES_NETH)) vp[vn++] = "nether";
		if (flags3 & (RF3_RES_WATE)) vp[vn++] = "water";
		if (flags3 & (RF3_RES_PLAS)) vp[vn++] = "plasma";
		if (flags3 & (RF3_RES_NEXU)) vp[vn++] = "nexus";
		if (flags3 & (RF3_RES_DISE)) vp[vn++] = "disenchantment";
		if (flags3 & (RF3_RES_TELE)) vp[vn++] = "teleportation";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" resists ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" and ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		/* Collect non-effects */
		vn = 0;
		if (flags3 & (RF3_NO_STUN)) vp[vn++] = "stunned";
		if (flags3 & (RF3_NO_FEAR)) vp[vn++] = "frightened";
		if (flags3 & (RF3_NO_CONF)) vp[vn++] = "confused";
		if (flags3 & (RF3_NO_SLEEP)) vp[vn++] = "slept";

		if (vn)
		{
			spoil_out(wd_che[msex]);
			for (i = 0; i < vn; i++)
			{
				if (!i) spoil_out(" cannot be ");
				else if (i < vn - 1) spoil_out(", ");
				else spoil_out(" or ");
				spoil_out(vp[i]);
			}
			spoil_out(".  ");
		}

		spoil_out(wd_che[msex]);
		if (r_ptr->sleep > 200) spoil_out(" prefers to ignore");
		else if (r_ptr->sleep > 95) spoil_out(" pays very little attention to");
		else if (r_ptr->sleep > 75) spoil_out(" pays little attention to");
		else if (r_ptr->sleep > 45) spoil_out(" tends to overlook");
		else if (r_ptr->sleep > 25) spoil_out(" takes quite a while to see");
		else if (r_ptr->sleep > 10) spoil_out(" takes a while to see");
		else if (r_ptr->sleep > 5) spoil_out(" is fairly observant of");
		else if (r_ptr->sleep > 3) spoil_out(" is observant of");
		else if (r_ptr->sleep > 1) spoil_out(" is very observant of");
		else if (r_ptr->sleep > 0) spoil_out(" is vigilant for");
		else spoil_out(" is ever vigilant for");

		sprintf(buf, " intruders, which %s may notice from %d feet.  ",
		        wd_lhe[msex], 10 * r_ptr->aaf);
		spoil_out(buf);

		i = 0;
		if (flags1 & (RF1_DROP_60)) i += 1;
		if (flags1 & (RF1_DROP_90)) i += 2;
		if (flags1 & (RF1_DROP_1D2)) i += 2;
		if (flags1 & (RF1_DROP_2D2)) i += 4;
		if (flags1 & (RF1_DROP_3D2)) i += 6;
		if (flags1 & (RF1_DROP_4D2)) i += 8;

		/* Drops gold and/or items */
		if (i)
		{
			sin = FALSE;
			spoil_out(wd_che[msex]);
			spoil_out(" will carry");

			if (i == 1)
			{
				spoil_out(" a");
				sin = TRUE;
			}
			else if (i == 2)
			{
				spoil_out(" one or two");
			}
			else
			{
				sprintf(buf, " up to %u", i);
				spoil_out(buf);
			}

			if (flags1 & (RF1_DROP_GREAT))
			{
				if (sin) spoil_out("n");
				spoil_out(" exceptional object");
			}
			else if (flags1 & (RF1_DROP_GOOD))
			{
				spoil_out(" good object");
			}
			else if (flags1 & (RF1_DROP_USEFUL))
			{
				spoil_out(" useful object");
			}
			else if (flags1 & (RF1_ONLY_ITEM))
			{
				spoil_out(" object");
			}
			else if (flags1 & (RF1_ONLY_GOLD))
			{
				spoil_out(" treasure");
			}
			else
			{
				if (sin) spoil_out("n");
				spoil_out(" object");
				if (i > 1) spoil_out("s");
				spoil_out(" or treasure");
			}
			if (i > 1) spoil_out("s");

			if (flags1 & (RF1_DROP_CHOSEN))
			{
				spoil_out(", in addition to chosen objects");
			}

			spoil_out(".  ");
		}

		/* Count the actual attacks */
		for (i = 0, j = 0; j < 4; j++)
		{
			if (r_ptr->blow[j].method) i++;
		}

		/* Examine the actual attacks */
		for (k = 0, j = 0; j < 4; j++)
		{
			if (!r_ptr->blow[j].method) continue;

			/* No method yet */
			p = "???";

			/* Acquire the method */
			switch (r_ptr->blow[j].method)
			{
			case RBM_HIT:
				p = "hit";
				break;
			case RBM_TOUCH:
				p = "touch";
				break;
			case RBM_PUNCH:
				p = "punch";
				break;
			case RBM_KICK:
				p = "kick";
				break;
			case RBM_CLAW:
				p = "claw";
				break;
			case RBM_BITE:
				p = "bite";
				break;
			case RBM_STING:
				p = "sting";
				break;
			case RBM_XXX1:
				break;
			case RBM_BUTT:
				p = "butt";
				break;
			case RBM_CRUSH:
				p = "crush";
				break;
			case RBM_ENGULF:
				p = "engulf";
				break;
			case RBM_CHARGE:
				p = "charge";
				break;
			case RBM_CRAWL:
				p = "crawl on you";
				break;
			case RBM_DROOL:
				p = "drool on you";
				break;
			case RBM_SPIT:
				p = "spit";
				break;
			case RBM_EXPLODE:
				p = "explode";
				break;
			case RBM_GAZE:
				p = "gaze";
				break;
			case RBM_WAIL:
				p = "wail";
				break;
			case RBM_SPORE:
				p = "release spores";
				break;
			case RBM_XXX4:
				break;
			case RBM_BEG:
				p = "beg";
				break;
			case RBM_INSULT:
				p = "insult";
				break;
			case RBM_MOAN:
				p = "moan";
				break;
			case RBM_SHOW:
				p = "sing";
				break;
			}


			/* Default effect */
			q = "???";

			/* Acquire the effect */
			switch (r_ptr->blow[j].effect)
			{
			case RBE_HURT:
				q = "attack";
				break;
			case RBE_POISON:
				q = "poison";
				break;
			case RBE_UN_BONUS:
				q = "disenchant";
				break;
			case RBE_UN_POWER:
				q = "drain charges";
				break;
			case RBE_EAT_GOLD:
				q = "steal gold";
				break;
			case RBE_EAT_ITEM:
				q = "steal items";
				break;
			case RBE_EAT_FOOD:
				q = "eat your food";
				break;
			case RBE_EAT_LITE:
				q = "absorb light";
				break;
			case RBE_ACID:
				q = "shoot acid";
				break;
			case RBE_ELEC:
				q = "electrocute";
				break;
			case RBE_FIRE:
				q = "burn";
				break;
			case RBE_COLD:
				q = "freeze";
				break;
			case RBE_BLIND:
				q = "blind";
				break;
			case RBE_CONFUSE:
				q = "confuse";
				break;
			case RBE_TERRIFY:
				q = "terrify";
				break;
			case RBE_PARALYZE:
				q = "paralyze";
				break;
			case RBE_LOSE_STR:
				q = "reduce strength";
				break;
			case RBE_LOSE_INT:
				q = "reduce intelligence";
				break;
			case RBE_LOSE_WIS:
				q = "reduce wisdom";
				break;
			case RBE_LOSE_DEX:
				q = "reduce dexterity";
				break;
			case RBE_LOSE_CON:
				q = "reduce constitution";
				break;
			case RBE_LOSE_CHR:
				q = "reduce charisma";
				break;
			case RBE_LOSE_ALL:
				q = "reduce all stats";
				break;
			case RBE_SHATTER:
				q = "shatter";
				break;
			case RBE_EXP_10:
				q = "lower experience (by 10d6+)";
				break;
			case RBE_EXP_20:
				q = "lower experience (by 20d6+)";
				break;
			case RBE_EXP_40:
				q = "lower experience (by 40d6+)";
				break;
			case RBE_EXP_80:
				q = "lower experience (by 80d6+)";
				break;
			case RBE_DISEASE:
				q = "disease";
				break;
			case RBE_TIME:
				q = "time";
				break;
			case RBE_SANITY:
				q = "make insane";
				break;
			case RBE_HALLU:
				q = "cause hallucinations";
				break;
			case RBE_PARASITE:
				q = "parasite";
				break;
			}


			if (!k)
			{
				spoil_out(wd_che[msex]);
				spoil_out(" can ");
			}
			else if (k < i - 1)
			{
				spoil_out(", ");
			}
			else
			{
				spoil_out(", and ");
			}

			/* Describe the method */
			spoil_out(p);

			/* Describe the effect, if any */
			if (r_ptr->blow[j].effect)
			{
				spoil_out(" to ");
				spoil_out(q);
				if (r_ptr->blow[j].d_dice && r_ptr->blow[j].d_side)
				{
					spoil_out(" with damage");
					if (r_ptr->blow[j].d_side == 1)
						sprintf(buf, " %d", r_ptr->blow[j].d_dice);
					else
						sprintf(buf, " %dd%d",
						        r_ptr->blow[j].d_dice, r_ptr->blow[j].d_side);
					spoil_out(buf);
				}
			}

			k++;
		}

		if (k)
		{
			spoil_out(".  ");
		}
		else if (flags1 & (RF1_NEVER_BLOW))
		{
			sprintf(buf, "%s has no physical attacks.  ", wd_che[msex]);
			spoil_out(buf);
		}

		spoil_out(NULL);
	}

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	msg_print("Successfully created a spoiler file.");
}

#if 0 /* not used anymore -- masmarangio */
static char* get_tval_name(int tval)
{
	switch (tval)
	{
	case TV_SWORD:
		return "Sword";
	case TV_POLEARM:
		return "Polearm";
	case TV_HAFTED:
		return "Hafted";
	case TV_AXE:
		return "Axe";
	case TV_CROWN:
		return "Crown";
	case TV_HELM:
		return "Helm";
	case TV_GLOVES:
		return "Gloves";
	case TV_CLOAK:
		return "Cloak";
	case TV_BOOTS:
		return "Boots";
	case TV_SOFT_ARMOR:
		return "Soft armor";
	case TV_HARD_ARMOR:
		return "Hard armor";
	}
	return "";
}
#endif

char *long_intro =
	"Essences are the tools of the trade for Alchemists, "
	"and unfortunately are useless for any other class. "
	"Alchemists use essences to create magical items for them to use.\n\n"
	"They can be either found on the floor while exploring the dungeon, "
	"or extracted from other magical items the alchemist finds "
	"during their adventures.\n\n"
	"To create an artifact, the alchemist will have to sacrifice 10 hit points, "
	"and an amount of magic essence similar to his skill in alchemy. "
	"The alchemist then allows the artifact to gain experience, "
	"and when it has enough, "
	"uses that experience to add abilities to the artifact. "
	"The alchemist can allow the artifact to continue to gain experience, "
	"thus keeping open the option to add more abilities later. "
	"This requires a similar amount of magic essence, "
	"but does not require the sacrifice of other hit points.\n\n"
	"Note that the experience you gain is divided among the artifacts "
	"that you have as well as going to yourself, "
	"so you will gain levels more slowly when empowering artifacts. "
	"Also, the artifact only gets 60% of the experience. "
	"So killing a creature worth 20xp would gain 10 for you, "
	"and 6 for the artifact.\n\n"
	"You can also modify existing artifacts when you attain skill level 50. "
	"Also at skill level 50 you will gain the ability to make temporary artifacts, "
	"which don't require the complex empowerments that regular items require, "
	"but also vanish after awhile.\n\n"
	"You cannot give an artifact an ability "
	"unless you have *Identified* an artifact which has that ability.\n\n"
	"For every four levels gained in the alchemy skill, "
	"the alchemist learns about objects of level (skill level)/4, "
	"starting by learning about level 1 objects at skill level 0. "
	"(actually 1, but who's counting?)\n\n"
	"At skill level 5 you gain the ability to make ego items - but watch it! "
	"Your base failure rate will be 90%, "
	"and won't be 0% until you reach skill level 50. "
	"Adding gold will increase the chances of success "
	"in direct proportion to the value of the item you are trying to create. "
	"Note that this results in automatic success "
	"when the item you are trying to create "
	"happens to pick up a curse in the process.\n\n"
	"At skill level 5 you also gain knowledge of some basic ego item recipes. "
	"These are: Acidic, Shocking, Fiery, Frozen, Venomous, and Chaotic weapons, "
	"Resist Fire armour, and light sources of Fearlessness.\n\n"
	"At skill level 10 you will gain knowledge of digging ego items, "
	"if you have selected the option "
	"'always  generate very unusual rooms' (ironman_rooms).\n\n"
	"At skill level 15 you can create ego wands, staves, rings, etc.\n\n"
	"At skill level 25 you gain the ability to empower artifacts "
	"and double ego items.\n\n"
	"At skill level 50 you gain the ability to create temporary artifacts, "
	"which don't require any exotic ingredients "
	"beyond a single corpse of any type.\n\n"
	"Between skill levels 25 and 50, "
	"you will steadily gain the ability to set more and more flags.\n\n"
	"To finalise an artifact, you 'P'ower it, and select the powers you want.\n"
	"Powers are divided into the following six categories:\n"
	"*****essences.txt*03[Stats, Sustains, Luck, Speed, Vision, etc.]\n"
	"*****essences.txt*04[Misc. (Auras, Light, See Invisibility, etc.)]\n"
	"*****essences.txt*05[Weapon Brands]\n"
	"*****essences.txt*06[Resistances and Immunities]\n"
	"*****essences.txt*07[ESP and Curses]\n"
	"*****essences.txt*08[Artifact Activations]\n";

/*
 * Create a spoiler file for essences
 */
static void spoil_bateries(cptr fname)
{
	int j, i, tval, sval, group;
	object_type forge, *o_ptr;

	char buf[1024];

	tval = sval = group = -1;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */

	fprintf(fff,
	        "|||||oy\n"
	        "~~~~~01|Spoilers|Essences\n"
	        "~~~~~02|Alchemist|Essence Spoiler\n"
	        "#####REssence Spoiler for %s %ld.%ld.%ld%s\n"
	        "#####R-----------------------------------\n\n",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);


	/*New code starts here -*/
	/*print header, including artifact header*/
	/*Cycle through artifact flags*/
	/*	print desc*/
	/*	cycle through all alchemist_recipies*/
	/*		print matching*/
	/*Print items header.*/
	/*Cycle through alchemist_recipies*/
	/*	sval or tval changed?*/
	/*		skip artifacts (tval=0)*/
	/*		print item desc (ego (tval=1) or item)*/
	/*	print essences required*/
	/*Done!*/

	/*Print basic header.*/
	spoil_out(long_intro);

	/*Cycle through artifact flags*/
	for ( i = 0 ; a_select_flags[i].group ; i++ )
	{
		if ( a_select_flags[i].group != group )
		{
			group = a_select_flags[i].group;
			spoil_out("\n~~~~~");
			switch (group)
			{
			case 1:
				spoil_out("03\n#####GStats, Sustains, Luck, Speed, Vision, etc.\n");
				break;
			case 2:
				spoil_out("04\n#####GMisc. (Auras, Light, See Invisibility, etc.)\n");
				break;
			case 3:
				spoil_out("05\n#####GWeapon Brands\n");
				break;
			case 4:
				spoil_out("06\n#####GResistances and Immunities\n");
				break;
			case 5:
				spoil_out("07\n#####GESP and Curses\n");
				break;
			case 88:
				spoil_out("08\n#####GArtifact Activations\n");
				break;
			default:
				spoil_out(format("09\n#####GExtra Group=%d\n", group));
			}
			spoil_out("lvl     xp   Power\n");

		}
		/*	print desc*/
		spoil_out(format("%-2d %8d  %-24s %s\n",
		                 a_select_flags[i].level,
		                 a_select_flags[i].xp,
		                 al_name + a_select_flags[i].desc,
		                 al_name + a_select_flags[i].item_desc));
		/*	cycle through all alchemist_recipies*/
		for ( j = 0 ; j < max_al_idx ; j++ )
		{
			/*		print matching*/
			if (alchemist_recipes[j].tval == 0
			                && alchemist_recipes[j].sval == a_select_flags[i].flag
			                && alchemist_recipes[j].qty
			   )
			{
				spoil_out(format("    %d essences of %s\n", alchemist_recipes[j].qty,
				                 k_name + k_info[lookup_kind(TV_BATERIE, alchemist_recipes[j].sval_essence)].name));
			}
		}
	}

	spoil_out("\n\nThe following basic item recipes also exist:\n");
	/*Cycle through alchemist_recipies*/
	for ( i = 0 ; i < max_al_idx ; i ++)
	{
		alchemist_recipe *ar_ptr = &alchemist_recipes[i];

		/*	sval or tval changed?*/
		if (tval != ar_ptr->tval || sval != ar_ptr->sval)
		{
			char o_name[80];
			/*		skip artifacts (tval=0)*/
			if (ar_ptr->tval == 0 )
				continue;

			tval = ar_ptr->tval;
			sval = ar_ptr->sval;

			/*		print item desc (ego (tval=1)or item)*/
			if (ar_ptr->tval == 1)
			{
				strcpy(o_name, e_name + e_info[ar_ptr->sval].name);
			}
			else
			{
				/* Find the name of that object */
				o_ptr = &forge;
				object_wipe(o_ptr);
				object_prep(o_ptr, lookup_kind(tval, sval));
				apply_magic(o_ptr, 1, FALSE, FALSE, FALSE);
				object_aware(o_ptr);
				object_known(o_ptr);
				o_ptr->name2 = o_ptr->name2b = 0;
				/* the 0 mode means only the text, leaving off any numbers */
				object_desc(o_name, o_ptr, FALSE, 0);
			}
			spoil_out("\n");
			spoil_out(o_name);
		}
		/*	print essence required*/
		spoil_out(format(" %d %s", ar_ptr->qty,
		                 k_name + k_info[lookup_kind(TV_BATERIE, ar_ptr->sval_essence)].name));
	}

	spoil_out(NULL);

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
	return;
}


/*
 * Print a bookless spell list
 */
void print_magic_powers( magic_power *powers, int max_powers, void(*power_info)(char *p, int power), int skill_num )
{
	int i, save_skill;

	char buf[80];

	magic_power spell;

	/* Use a maximal skill */
	save_skill = s_info[skill_num].value;
	s_info[skill_num].value = SKILL_MAX;

	/* Dump the header line */
	spoiler_blanklines(2);
	sprintf(buf, "%s", s_info[skill_num].name + s_name);
	spoiler_underline(buf);
	spoiler_blanklines(1);

	fprintf(fff, "   Name                         Lvl Mana Fail Info\n");

	/* Dump the spells */
	for (i = 0; i < max_powers; i++)
	{
		/* Access the spell */
		spell = powers[i];

		/* Get the additional info */
		power_info(buf, i);

		/* Dump the spell */
		spoil_out(format("%c) %-30s%2d %4d %3d%%%s\n",
		                 I2A(i), spell.name,
		                 spell.min_lev, spell.mana_cost, spell.fail, buf));
		spoil_out(format("%s\n", spell.desc));
	}

	/* Restore skill */
	s_info[skill_num].value = save_skill;
}


/*
 * Create a spoiler file for spells
 */

static void spoil_spells(cptr fname)
{
	char buf[1024];

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, fname);

	/* File type is "TEXT" */
	FILE_TYPE(FILE_TYPE_TEXT);

	fff = my_fopen(buf, "w");

	/* Oops */
	if (!fff)
	{
		msg_print("Cannot create spoiler file.");
		return;
	}

	/* Dump the header */
	sprintf(buf, "Spell Spoiler (Skill Level 50) for %s %ld.%ld.%ld%s",
	        game_module, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, IS_CVS);
	spoiler_underline(buf);

	/* Dump the bookless magic powers in alphabetical order */

	/* Mimicry */
	print_magic_powers(mimic_powers, MAX_MIMIC_POWERS, mimic_info, SKILL_MIMICRY);

	/* Mindcraft */
	print_magic_powers(mindcraft_powers, MAX_MINDCRAFT_POWERS, mindcraft_info, SKILL_MINDCRAFT);

	/* Necromancy */
	print_magic_powers(necro_powers, MAX_NECRO_POWERS, necro_info, SKILL_NECROMANCY);

	/* Symbiosis */
	print_magic_powers(symbiotic_powers, MAX_SYMBIOTIC_POWERS, symbiotic_info, SKILL_SYMBIOTIC);

	/* Check for errors */
	if (ferror(fff) || my_fclose(fff))
	{
		msg_print("Cannot close spoiler file.");
		return;
	}

	/* Message */
	msg_print("Successfully created a spoiler file.");
}



/*
 * Forward declare
 */
extern void do_cmd_spoilers(void);

/*
 * Create Spoiler files -BEN-
 */
void do_cmd_spoilers(void)
{
	int i;


	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Info */
		prt("Create a spoiler file.", 2, 0);

		/* Prompt for a file */
		prt("(1) Brief Object Info  (obj-desc.spo)", 5, 5);
		prt("(2) Full Artifact Info (artifact.spo)", 6, 5);
		prt("(3) Brief Monster Info (mon-desc.spo)", 7, 5);
		prt("(4) Full Monster Info  (mon-info.spo)", 8, 5);
		prt("(5) Full Essences Info (ess-info.spo)", 9, 5);
		prt("(6) Spell Info         (spell.spo)", 10, 5);

		/* Prompt */
		prt("Command: ", 12, 0);

		/* Get a choice */
		i = inkey();

		/* Escape */
		if (i == ESCAPE)
		{
			break;
		}

		/* Option (1) */
		else if (i == '1')
		{
			spoil_obj_desc("obj-desc.spo");
		}

		/* Option (2) */
		else if (i == '2')
		{
			spoil_artifact("artifact.spo");
		}

		/* Option (3) */
		else if (i == '3')
		{
			spoil_mon_desc("mon-desc.spo");
		}

		/* Option (4) */
		else if (i == '4')
		{
			spoil_mon_info("mon-info.spo");
		}

		/* Option (5) */
		else if (i == '5')
		{
			spoil_bateries("ess-info.spo");
		}

		/* Option (6) */
		else if (i == '6')
		{
			spoil_spells("spell.spo");
		}

		/* Oops */
		else
		{
			bell();
		}

		/* Flush messages */
		msg_print(NULL);
	}


	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif /* MACINTOSH */

#endif
