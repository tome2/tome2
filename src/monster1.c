/* File: monster1.c */

/* Purpose: describe monsters (using monster memory) */

/*
 * Copyright (c) 1989 James E. Wilson, Christopher J. Stuart
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Pronoun arrays, by gender.
 */
static cptr wd_he[3] = { "it", "he", "she" };
static cptr wd_his[3] = { "its", "his", "her" };


/*
 * Pluralizer.  Args(count, singular, plural)
 */
#define plural(c,s,p) \
(((c) == 1) ? (s) : (p))






/*
 * Determine if the "armor" is known
 * The higher the level, the fewer kills needed.
 */
static bool know_armour(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b kills = r_ptr->r_tkills;

	/* Normal monsters */
	if (kills > 304 / (4 + level)) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags1 & (RF1_UNIQUE))) return (FALSE);

	/* Unique monsters */
	if (kills > 304 / (38 + (5*level) / 4)) return (TRUE);

	/* Assume false */
	return (FALSE);
}


/*
 * Determine if the "damage" of the given attack is known
 * the higher the level of the monster, the fewer the attacks you need,
 * the more damage an attack does, the more attacks you need
 */
static bool know_damage(int r_idx, int i)
{
	monster_race *r_ptr = &r_info[r_idx];

	s32b level = r_ptr->level;

	s32b a = r_ptr->r_blows[i];

	s32b d1 = r_ptr->blow[i].d_dice;
	s32b d2 = r_ptr->blow[i].d_side;

	s32b d = d1 * d2;

	/* Normal monsters */
	if ((4 + level) * a > 80 * d) return (TRUE);

	/* Skip non-uniques */
	if (!(r_ptr->flags1 & (RF1_UNIQUE))) return (FALSE);

	/* Unique monsters */
	if ((4 + level) * (2 * a) > 80 * d) return (TRUE);

	/* Assume false */
	return (FALSE);
}


/*
 * Hack -- display monster information using "text_out()"
 *
 * Note that there is now a compiler option to only read the monster
 * descriptions from the raw file when they are actually needed, which
 * saves about 60K of memory at the cost of disk access during monster
 * recall, which is optional to the user.
 *
 * This function should only be called with the cursor placed at the
 * left edge of the screen, on a cleared line, in which the recall is
 * to take place.  One extra blank line is left after the recall.
 */
static void roff_aux(int r_idx, int ego, int remem)
{
	monster_race *r_ptr;

	bool old = FALSE;
	bool sin = FALSE;

	int m, n, r;

	cptr p, q;

	int msex = 0;

	bool breath = FALSE;
	bool magic = FALSE;

	u32b	flags1;
	u32b	flags2;
	u32b	flags3;
	u32b	flags4;
	u32b	flags5;
	u32b	flags6;
	u32b flags7;
	u32b flags8;
	u32b flags9;

	int	vn = 0;
	byte color[64];
	cptr	vp[64];

	monster_race save_mem;



#if 0

	/* Nothing erased */
	roff_old = 0;

	/* Reset the row */
	roff_row = 1;

	/* Reset the pointer */
	roff_p = roff_buf;

	/* No spaces yet */
	roff_s = NULL;

#endif


	/* Access the race and lore */
	r_ptr = race_info_idx(r_idx, ego);


	/* Cheat -- Know everything */
	if (cheat_know)
	{
		/* XXX XXX XXX */

		/* Save the "old" memory */
		save_mem = *r_ptr;

		/* Hack -- Maximal kills */
		r_ptr->r_tkills = MAX_SHORT;

		/* Hack -- Maximal info */
		r_ptr->r_wake = r_ptr->r_ignore = MAX_UCHAR;

		/* Observe "maximal" attacks */
		for (m = 0; m < 4; m++)
		{
			/* Examine "actual" blows */
			if (r_ptr->blow[m].effect || r_ptr->blow[m].method)
			{
				/* Hack -- maximal observations */
				r_ptr->r_blows[m] = MAX_UCHAR;
			}
		}

		/* Hack -- maximal drops */
		r_ptr->r_drop_gold = r_ptr->r_drop_item =
		                             (((r_ptr->flags1 & (RF1_DROP_4D2)) ? 8 : 0) +
		                              ((r_ptr->flags1 & (RF1_DROP_3D2)) ? 6 : 0) +
		                              ((r_ptr->flags1 & (RF1_DROP_2D2)) ? 4 : 0) +
		                              ((r_ptr->flags1 & (RF1_DROP_1D2)) ? 2 : 0) +
		                              ((r_ptr->flags1 & (RF1_DROP_90)) ? 1 : 0) +
		                              ((r_ptr->flags1 & (RF1_DROP_60)) ? 1 : 0));

		/* Hack -- but only "valid" drops */
		if (r_ptr->flags1 & (RF1_ONLY_GOLD)) r_ptr->r_drop_item = 0;
		if (r_ptr->flags1 & (RF1_ONLY_ITEM)) r_ptr->r_drop_gold = 0;

		/* Hack -- observe many spells */
		r_ptr->r_cast_inate = MAX_UCHAR;
		r_ptr->r_cast_spell = MAX_UCHAR;

		/* Hack -- know all the flags */
		r_ptr->r_flags1 = r_ptr->flags1;
		r_ptr->r_flags2 = r_ptr->flags2;
		r_ptr->r_flags3 = r_ptr->flags3;
		r_ptr->r_flags4 = r_ptr->flags4;
		r_ptr->r_flags5 = r_ptr->flags5;
		r_ptr->r_flags6 = r_ptr->flags6;
		r_ptr->r_flags7 = r_ptr->flags7;
		r_ptr->r_flags8 = r_ptr->flags8;
		r_ptr->r_flags9 = r_ptr->flags9;
	}


	/* Extract a gender (if applicable) */
	if (r_ptr->flags1 & (RF1_FEMALE)) msex = 2;
	else if (r_ptr->flags1 & (RF1_MALE)) msex = 1;


	/* Obtain a copy of the "known" flags */
	flags1 = (r_ptr->flags1 & r_ptr->r_flags1);
	flags2 = (r_ptr->flags2 & r_ptr->r_flags2);
	flags3 = (r_ptr->flags3 & r_ptr->r_flags3);
	flags4 = (r_ptr->flags4 & r_ptr->r_flags4);
	flags5 = (r_ptr->flags5 & r_ptr->r_flags5);
	flags6 = (r_ptr->flags6 & r_ptr->r_flags6);
	flags7 = (r_ptr->flags7 & r_ptr->r_flags7);
	flags8 = (r_ptr->flags8 & r_ptr->r_flags8);
	flags9 = (r_ptr->flags9 & r_ptr->r_flags9);


	/* Assume some "obvious" flags */
	if (r_ptr->flags1 & (RF1_UNIQUE)) flags1 |= (RF1_UNIQUE);
	if (r_ptr->flags1 & (RF1_MALE)) flags1 |= (RF1_MALE);
	if (r_ptr->flags1 & (RF1_FEMALE)) flags1 |= (RF1_FEMALE);

	/* Assume some "creation" flags */
	if (r_ptr->flags1 & (RF1_FRIEND)) flags1 |= (RF1_FRIEND);
	if (r_ptr->flags1 & (RF1_FRIENDS)) flags1 |= (RF1_FRIENDS);
	if (r_ptr->flags1 & (RF1_ESCORT)) flags1 |= (RF1_ESCORT);
	if (r_ptr->flags1 & (RF1_ESCORTS)) flags1 |= (RF1_ESCORTS);

	/* Killing a monster reveals some properties */
	if (r_ptr->r_tkills)
	{
		/* Know "race" flags */
		if (r_ptr->flags3 & (RF3_ORC)) flags3 |= (RF3_ORC);
		if (r_ptr->flags3 & (RF3_TROLL)) flags3 |= (RF3_TROLL);
		if (r_ptr->flags3 & (RF3_GIANT)) flags3 |= (RF3_GIANT);
		if (r_ptr->flags3 & (RF3_DRAGON)) flags3 |= (RF3_DRAGON);
		if (r_ptr->flags3 & (RF3_DEMON)) flags3 |= (RF3_DEMON);
		if (r_ptr->flags3 & (RF3_UNDEAD)) flags3 |= (RF3_UNDEAD);
		if (r_ptr->flags3 & (RF3_EVIL)) flags3 |= (RF3_EVIL);
		if (r_ptr->flags3 & (RF3_GOOD)) flags3 |= (RF3_GOOD);
		if (r_ptr->flags3 & (RF3_ANIMAL)) flags3 |= (RF3_ANIMAL);
		if (r_ptr->flags3 & (RF3_THUNDERLORD)) flags3 |= (RF3_THUNDERLORD);
		if (r_ptr->flags7 & (RF7_SPIDER)) flags7 |= (RF7_SPIDER);

		/* Know "forced" flags */
		if (r_ptr->flags1 & (RF1_FORCE_DEPTH)) flags1 |= (RF1_FORCE_DEPTH);
		if (r_ptr->flags1 & (RF1_FORCE_MAXHP)) flags1 |= (RF1_FORCE_MAXHP);
	}


	/* Require a flag to show kills */
	if (!(show_details))
	{
		/* nothing */
	}

	/* Treat uniques differently */
	else if (flags1 & (RF1_UNIQUE))
	{
		/* Hack -- Determine if the unique is "dead" */
		bool dead = (r_ptr->max_num == 0) ? TRUE : FALSE;

		/* We've been killed... */
		if (r_ptr->r_deaths)
		{
			/* Killed ancestors */
			text_out(format("%^s has slain %d of your ancestors",
			                wd_he[msex], r_ptr->r_deaths));

			/* But we've also killed it */
			if (dead)
			{
				text_out(format(", but you have avenged them!  ") );
			}

			/* Unavenged (ever) */
			else
			{
				text_out(format(", who %s unavenged.  ",
				                plural(r_ptr->r_deaths, "remains", "remain")));
			}
		}

		/* Dead unique who never hurt us */
		else if (dead)
		{
			text_out("You have slain this foe.  ");
		}
	}

	/* Not unique, but killed us */
	else if (r_ptr->r_deaths)
	{
		/* Dead ancestors */
		text_out(format("%d of your ancestors %s been killed by this creature, ",
		                r_ptr->r_deaths, plural(r_ptr->r_deaths, "has", "have")));

		/* Some kills this life */
		if (r_ptr->r_pkills)
		{
			text_out("and you have exterminated at least ");
			text_out_c(TERM_L_GREEN, format("%d", r_ptr->r_pkills));
			text_out(" of the creatures.  ");
		}

		/* Some kills past lives */
		else if (r_ptr->r_tkills)
		{
			text_out(format("and %s have exterminated at least %d of the creatures.  ",
			                "your ancestors", r_ptr->r_tkills));
		}

		/* No kills */
		else
		{
			text_out(format("and %s is not ever known to have been defeated.  ",
			                wd_he[msex]));
		}
	}

	/* Normal monsters */
	else
	{
		/* Killed some this life */
		if (r_ptr->r_pkills)
		{
			text_out("You have killed at least ");
			text_out_c(TERM_L_GREEN, format("%d", r_ptr->r_pkills));
			text_out(" of these creatures.  ");
		}

		/* Killed some last life */
		else if (r_ptr->r_tkills)
		{
			text_out(format("Your ancestors have killed at least %d of these creatures.  ",
			                r_ptr->r_tkills));
		}

		/* Killed none */
		else
		{
			text_out("No battles to the death are recalled.  ");
		}
	}


	/* Descriptions */
	if (show_details)
	{
		char buf[2048];

#ifdef DELAY_LOAD_R_TEXT

		int fd;

		/* Build the filename */
		path_build(buf, 1024, ANGBAND_DIR_DATA, "r_info.raw");

		/* Grab permission */
		safe_setuid_grab();

		/* Open the "raw" file */
		fd = fd_open(buf, O_RDONLY);

		/* Drop permission */
		safe_setuid_drop();

		/* Use file */
		if (fd >= 0)
		{
			huge pos;

			/* Starting position */
			pos = r_ptr->text;

			/* Additional offsets */
			pos += r_head->head_size;
			pos += r_head->info_size;
			pos += r_head->name_size;

#if 0

			/* Maximal length */
			len = r_head->text_size - r_ptr->text;

			/* Actual length */
			for (i = r_idx + 1; i < max_r_idx; i++)
			{
				/* Actual length */
				if (r_info[i].text > r_ptr->text)
				{
					/* Extract length */
					len = r_info[i].text - r_ptr->text;

					/* Done */
					break;
				}
			}

			/* Maximal length */
			if (len > 2048) len = 2048;

#endif

			/* Seek */
			(void)fd_seek(fd, pos);

			/* Read a chunk of data */
			(void)fd_read(fd, buf, 2048);

			/* Close it */
			(void)fd_close(fd);
		}

#else

		/* Simple method */
		strcpy(buf, r_text + r_ptr->text);

#endif

		/* Dump it */
		text_out(buf);
		text_out("  ");
	}


	/* Nothing yet */
	old = FALSE;

	/* Describe location */
	if (r_ptr->flags7 & RF7_PET)
	{
		text_out(format("%^s is ", wd_he[msex]));
		text_out_c(TERM_L_BLUE, "friendly");
		text_out(" to you");
		old = TRUE;
	}

	/* Describe location */
	if (r_ptr->level == 0)
	{
		if (old)
			text_out(", ");
		else
			text_out(format("%^s ", wd_he[msex]));
		text_out_c(TERM_L_GREEN, "lives in the town or the wilderness");
		old = TRUE;
	}
	else if (r_ptr->r_tkills)
	{
		if (old)
			text_out(", ");
		else
			text_out(format("%^s ", wd_he[msex]));
		if (depth_in_feet)
		{
			text_out(format("is normally found at depths of ", wd_he[msex]));
			if (dun_level < r_ptr->level) /* out of depth monster */
			{
				text_out_c(TERM_L_RED, format("%d", r_ptr->level * 50));
			}
			else
			{
				text_out_c(TERM_L_GREEN, format("%d", r_ptr->level * 50));
			}
			text_out(" feet");
		}
		else
		{
			text_out(format("is normally found on level ", wd_he[msex]));
			if (dun_level < r_ptr->level) /* out of depth monster */
			{
				text_out_c(TERM_L_RED, format("%d", r_ptr->level));
			}
			else
			{
				text_out_c(TERM_L_GREEN, format("%d", r_ptr->level));
			}
		}
		old = TRUE;
	}


	/* Describe movement */
	if (TRUE)
	{
		/* Introduction */
		if (old)
		{
			text_out(", and ");
		}
		else
		{
			text_out(format("%^s ", wd_he[msex]));
			old = TRUE;
		}
		text_out("moves");

		/* Random-ness */
		if ((flags1 & (RF1_RAND_50)) || (flags1 & (RF1_RAND_25)))
		{
			/* Adverb */
			if ((flags1 & (RF1_RAND_50)) && (flags1 & (RF1_RAND_25)))
			{
				text_out(" extremely");
			}
			else if (flags1 & (RF1_RAND_50))
			{
				text_out(" somewhat");
			}
			else if (flags1 & (RF1_RAND_25))
			{
				text_out(" a bit");
			}

			/* Adjective */
			text_out(" erratically");

			/* Hack -- Occasional conjunction */
			if (r_ptr->speed != 110) text_out(", and");
		}

		/* Speed */
		if (r_ptr->speed > 110)
		{
			if (r_ptr->speed > 130) text_out_c(TERM_RED, " incredibly");
			else if (r_ptr->speed > 120) text_out_c(TERM_ORANGE, " very");
			text_out_c(TERM_L_RED, " quickly");
		}
		else if (r_ptr->speed < 110)
		{
			if (r_ptr->speed < 90) text_out_c(TERM_L_GREEN, " incredibly");
			else if (r_ptr->speed < 100) text_out_c(TERM_BLUE, " very");
			text_out_c(TERM_L_BLUE, " slowly");
		}
		else
		{
			text_out(" at normal speed");
		}
	}

	/* The code above includes "attack speed" */
	if (flags1 & (RF1_NEVER_MOVE))
	{
		/* Introduce */
		if (old)
		{
			text_out(", but ");
		}
		else
		{
			text_out(format("%^s ", wd_he[msex]));
			old = TRUE;
		}

		/* Describe */
		text_out("does not deign to chase intruders");
	}

	/* End this sentence */
	if (old)
	{
		text_out(".  ");
		old = FALSE;
	}


	/* Describe experience if known */
	if (r_ptr->r_tkills)
	{
		/* Introduction */
		if (flags1 & (RF1_UNIQUE))
		{
			text_out("Killing this");
		}
		else
		{
			text_out("A kill of this");
		}

		/* Describe the "quality" */
		if (flags2 & (RF2_ELDRITCH_HORROR)) text_out_c(TERM_VIOLET, " sanity-blasting");
		if (flags3 & (RF3_ANIMAL)) text_out_c(TERM_VIOLET, " natural");
		if (flags3 & (RF3_EVIL)) text_out_c(TERM_VIOLET, " evil");
		if (flags3 & (RF3_GOOD)) text_out_c(TERM_VIOLET, " good");
		if (flags3 & (RF3_UNDEAD)) text_out_c(TERM_VIOLET, " undead");

		/* Describe the "race" */
		if (flags3 & (RF3_DRAGON)) text_out_c(TERM_VIOLET, " dragon");
		else if (flags3 & (RF3_DEMON)) text_out_c(TERM_VIOLET, " demon");
		else if (flags3 & (RF3_GIANT)) text_out_c(TERM_VIOLET, " giant");
		else if (flags3 & (RF3_TROLL)) text_out_c(TERM_VIOLET, " troll");
		else if (flags3 & (RF3_ORC)) text_out_c(TERM_VIOLET, " orc");
		else if (flags3 & (RF3_THUNDERLORD))text_out_c(TERM_VIOLET, " Thunderlord");
		else if (flags7 & (RF7_SPIDER)) text_out_c(TERM_VIOLET, " spider");
		else if (flags7 & (RF7_NAZGUL)) text_out_c(TERM_VIOLET, " Nazgul");
		else text_out(" creature");

		/* Group some variables */
		if (TRUE)
		{
			long i, j;

			/* calculate the integer exp part */
			i = (long)r_ptr->mexp * r_ptr->level / p_ptr->lev;

			/* calculate the fractional exp part scaled by 100, */
			/* must use long arithmetic to avoid overflow  */
			j = ((((long)r_ptr->mexp * r_ptr->level % p_ptr->lev) *
			      (long)1000 / p_ptr->lev + 5) / 10);

			/* Mention the experience */
			text_out(" is worth ");
			text_out_c(TERM_ORANGE, format("%ld.%02ld", (long)i, (long)j));
			text_out(" point");
			text_out(((i == 1) && (j == 0)) ? "" : "s");

			/* Take account of annoying English */
			p = "th";
			i = p_ptr->lev % 10;
			if ((p_ptr->lev / 10) == 1) /* nothing */;
			else if (i == 1) p = "st";
			else if (i == 2) p = "nd";
			else if (i == 3) p = "rd";

			/* Take account of "leading vowels" in numbers */
			q = "";
			i = p_ptr->lev;
			if ((i == 8) || (i == 11) || (i == 18)) q = "n";

			/* Mention the dependance on the player's level */
			text_out(format(" for a%s %lu%s level character.  ",
			                q, (long)i, p));
		}
	}

	if ((flags2 & (RF2_AURA_FIRE)) && (flags2 & (RF2_AURA_ELEC)))
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_VIOLET, "flames and electricity");
		text_out(".  ");
	}
	else if (flags2 & (RF2_AURA_FIRE))
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_ORANGE, "flames");
		text_out(".  ");
	}
	else if (flags2 & (RF2_AURA_ELEC))
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_L_BLUE, "electricity");
		text_out(".  ");
	}

	if (flags2 & (RF2_REFLECTING))
	{
		text_out(format("%^s ", wd_he[msex]));
		text_out_c(TERM_L_UMBER, "reflects");
		text_out(" bolt spells.  ");
	}


	/* Describe escorts */
	if ((flags1 & (RF1_ESCORT)) || (flags1 & (RF1_ESCORTS)))
	{
		text_out(format("%^s usually appears with escorts.  ",
		                wd_he[msex]));
	}

	/* Describe friends */
	else if ((flags1 & (RF1_FRIEND)) || (flags1 & (RF1_FRIENDS)))
	{
		text_out(format("%^s usually appears in groups.  ",
		                wd_he[msex]));
	}


	/* Collect inate attacks */
	vn = 0;
	if (flags4 & (RF4_SHRIEK))	vp[vn++] = "shriek for help";
	if (flags4 & (RF4_ROCKET))	vp[vn++] = "shoot a rocket";
	if (flags4 & (RF4_ARROW_1))	vp[vn++] = "fire an arrow";
	if (flags4 & (RF4_ARROW_2))	vp[vn++] = "fire arrows";
	if (flags4 & (RF4_ARROW_3))	vp[vn++] = "fire a missile";
	if (flags4 & (RF4_ARROW_4))	vp[vn++] = "fire missiles";

	/* Describe inate attacks */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" may ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out_c(TERM_YELLOW, vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Collect breaths */
	vn = 0;
	if (flags4 & (RF4_BR_ACID))	vp[vn++] = "acid";
	if (flags4 & (RF4_BR_ELEC))	vp[vn++] = "lightning";
	if (flags4 & (RF4_BR_FIRE))	vp[vn++] = "fire";
	if (flags4 & (RF4_BR_COLD))	vp[vn++] = "frost";
	if (flags4 & (RF4_BR_POIS))	vp[vn++] = "poison";
	if (flags4 & (RF4_BR_NETH))	vp[vn++] = "nether";
	if (flags4 & (RF4_BR_LITE))	vp[vn++] = "light";
	if (flags4 & (RF4_BR_DARK))	vp[vn++] = "darkness";
	if (flags4 & (RF4_BR_CONF))	vp[vn++] = "confusion";
	if (flags4 & (RF4_BR_SOUN))	vp[vn++] = "sound";
	if (flags4 & (RF4_BR_CHAO))	vp[vn++] = "chaos";
	if (flags4 & (RF4_BR_DISE))	vp[vn++] = "disenchantment";
	if (flags4 & (RF4_BR_NEXU))	vp[vn++] = "nexus";
	if (flags4 & (RF4_BR_TIME))	vp[vn++] = "time";
	if (flags4 & (RF4_BR_INER))	vp[vn++] = "inertia";
	if (flags4 & (RF4_BR_GRAV))	vp[vn++] = "gravity";
	if (flags4 & (RF4_BR_SHAR))	vp[vn++] = "shards";
	if (flags4 & (RF4_BR_PLAS))	vp[vn++] = "plasma";
	if (flags4 & (RF4_BR_WALL))	vp[vn++] = "force";
	if (flags4 & (RF4_BR_MANA))	vp[vn++] = "mana";
	if (flags4 & (RF4_BR_NUKE))	vp[vn++] = "toxic waste";
	if (flags4 & (RF4_BR_DISI))	vp[vn++] = "disintegration";

	/* Describe breaths */
	if (vn)
	{
		/* Note breath */
		breath = TRUE;

		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" may breathe ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out_c(TERM_YELLOW, vp[n]);
		}
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
	if (flags5 & (RF5_BA_MANA)) vp[vn++] = "invoke mana storms";
	if (flags5 & (RF5_BA_DARK)) vp[vn++] = "invoke darkness storms";
	if (flags4 & (RF4_BA_CHAO)) vp[vn++] = "invoke raw chaos";
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
	if (flags5 & (RF5_HOLD)) vp[vn++] = "paralyze";
	if (flags6 & (RF6_HASTE)) vp[vn++] = "haste-self";
	if (flags6 & (RF6_HEAL)) vp[vn++] = "heal-self";
	if (flags6 & (RF6_BLINK)) vp[vn++] = "blink-self";
	if (flags6 & (RF6_TPORT)) vp[vn++] = "teleport-self";
	if (flags6 & (RF6_S_BUG)) vp[vn++] = "summon software bugs";
	if (flags6 & (RF6_S_RNG)) vp[vn++] = "summon RNG";
	if (flags6 & (RF6_TELE_TO)) vp[vn++] = "teleport to";
	if (flags6 & (RF6_TELE_AWAY)) vp[vn++] = "teleport away";
	if (flags6 & (RF6_TELE_LEVEL)) vp[vn++] = "teleport level";
	if (flags6 & (RF6_S_THUNDERLORD)) vp[vn++] = "summon a Thunderlord";
	if (flags6 & (RF6_DARKNESS)) vp[vn++] = "create darkness";
	if (flags6 & (RF6_TRAPS)) vp[vn++] = "create traps";
	if (flags6 & (RF6_FORGET)) vp[vn++] = "cause amnesia";
	if (flags6 & (RF6_RAISE_DEAD)) vp[vn++] = "raise dead";
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
	if (flags6 & (RF6_S_HI_UNDEAD)) vp[vn++] = "summon Greater Undead";
	if (flags6 & (RF6_S_HI_DRAGON)) vp[vn++] = "summon Ancient Dragons";
	if (flags6 & (RF6_S_HI_DEMON)) vp[vn++] = "summon Greater Demons";
	if (flags6 & (RF6_S_WRAITH)) vp[vn++] = "summon Ringwraith";
	if (flags6 & (RF6_S_UNIQUE)) vp[vn++] = "summon Unique Monsters";

	/* Describe spells */
	if (vn)
	{
		/* Note magic */
		magic = TRUE;

		/* Intro */
		if (breath)
		{
			text_out(", and is also");
		}
		else
		{
			text_out(format("%^s is", wd_he[msex]));
		}

		/* Verb Phrase */
		text_out(" magical, casting spells");

		/* Adverb */
		if (flags2 & (RF2_SMART)) text_out_c(TERM_YELLOW, " intelligently");

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" which ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out_c(TERM_YELLOW, vp[n]);
		}
	}


	/* End the sentence about inate/other spells */
	if (breath || magic)
	{
		/* Total casting */
		m = r_ptr->r_cast_inate + r_ptr->r_cast_spell;

		/* Average frequency */
		n = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

		/* Describe the spell frequency */
		if (m > 100)
		{
			text_out("; ");
			text_out_c(TERM_L_GREEN, "1");
			text_out(" time in ");
			text_out_c(TERM_L_GREEN, format("%d", 100 / n));
		}

		/* Guess at the frequency */
		else if (m)
		{
			n = ((n + 9) / 10) * 10;
			text_out("; about ");
			text_out_c(TERM_L_GREEN, "1");
			text_out(" time in ");
			text_out_c(TERM_L_GREEN, format("%d", 100 / n));
		}

		/* End this sentence */
		text_out(".  ");
	}


	/* Describe monster "toughness" */
	if (know_armour(r_idx))
	{
		/* Armor */
		text_out(format("%^s has an armor rating of ", wd_he[msex]));
		text_out_c(TERM_L_GREEN, format("%d", r_ptr->ac));

		/* Maximized hitpoints */
		if (flags1 & (RF1_FORCE_MAXHP))
		{
			text_out(" and a life rating of ");
			text_out_c(TERM_L_GREEN, format("%d", r_ptr->hdice * r_ptr->hside));
			text_out(".  ");
		}

		/* Variable hitpoints */
		else
		{
			text_out(" and a life rating of ");
			text_out_c(TERM_L_GREEN, format("%dd%d", r_ptr->hdice, r_ptr->hside));
			text_out(".  ");
		}
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

	/* Describe special abilities. */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" can ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out(vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Describe special abilities. */
	if (flags2 & (RF2_INVISIBLE))
	{
		text_out_c(TERM_GREEN, format("%^s is invisible.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_COLD_BLOOD))
	{
		text_out(format("%^s is cold blooded.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_EMPTY_MIND))
	{
		text_out(format("%^s is not detected by telepathy.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_WEIRD_MIND))
	{
		text_out(format("%^s is rarely detected by telepathy.  ", wd_he[msex]));
	}
	if (flags4 & (RF4_MULTIPLY))
	{
		text_out_c(TERM_L_UMBER, format("%^s breeds explosively.  ", wd_he[msex]));
	}
	if (flags2 & (RF2_REGENERATE))
	{
		text_out_c(TERM_L_WHITE, format("%^s regenerates quickly.  ", wd_he[msex]));
	}
	if (r_ptr->flags7 & (RF7_MORTAL))
	{
		text_out_c(TERM_RED, format("%^s is a mortal being.  ", wd_he[msex]));
	}
	else
	{
		text_out_c(TERM_L_BLUE, format("%^s is an immortal being.  ", wd_he[msex]));
	}


	/* Collect susceptibilities */
	vn = 0;
	if (flags3 & (RF3_HURT_ROCK))
	{
		vp[vn++] = "rock remover";
		color[vn - 1] = TERM_UMBER;
	}
	if (flags3 & (RF3_HURT_LITE))
	{
		vp[vn++] = "bright light";
		color[vn - 1] = TERM_YELLOW;
	}
	if (flags3 & (RF3_SUSCEP_FIRE))
	{
		vp[vn++] = "fire";
		color[vn - 1] = TERM_RED;
	}
	if (flags3 & (RF3_SUSCEP_COLD))
	{
		vp[vn++] = "cold";
		color[vn - 1] = TERM_L_WHITE;
	}
	if (flags9 & (RF9_SUSCEP_ACID))
	{
		vp[vn++] = "acid";
		color[vn - 1] = TERM_GREEN;
	}
	if (flags9 & (RF9_SUSCEP_ELEC))
	{
		vp[vn++] = "lightning";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags9 & (RF9_SUSCEP_POIS))
	{
		vp[vn++] = "poison";
		color[vn - 1] = TERM_L_GREEN;
	}

	/* Describe susceptibilities */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" is hurt by ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out_c(color[n], vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Collect immunities */
	vn = 0;
	if (flags3 & (RF3_IM_ACID))
	{
		vp[vn++] = "acid";
		color[vn - 1] = TERM_L_GREEN;
	}
	if (flags3 & (RF3_IM_ELEC))
	{
		vp[vn++] = "lightning";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags3 & (RF3_IM_FIRE))
	{
		vp[vn++] = "fire";
		color[vn - 1] = TERM_L_RED;
	}
	if (flags3 & (RF3_IM_COLD))
	{
		vp[vn++] = "cold";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags3 & (RF3_IM_POIS))
	{
		vp[vn++] = "poison";
		color[vn - 1] = TERM_L_GREEN;
	}

	/* Describe immunities */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" resists ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out_c(color[n], vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Collect resistances */
	vn = 0;
	if (flags3 & (RF3_RES_NETH)) vp[vn++] = "nether";
	if (flags3 & (RF3_RES_WATE)) vp[vn++] = "water";
	if (flags3 & (RF3_RES_PLAS)) vp[vn++] = "plasma";
	if (flags3 & (RF3_RES_NEXU)) vp[vn++] = "nexus";
	if (flags3 & (RF3_RES_DISE)) vp[vn++] = "disenchantment";
	if (flags3 & (RF3_RES_TELE)) vp[vn++] = "teleportation";

	/* Describe resistances */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" resists ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" and ");

			/* Dump */
			text_out_c(TERM_L_BLUE, vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Collect non-effects */
	vn = 0;
	if (flags3 & (RF3_NO_STUN)) vp[vn++] = "stunned";
	if (flags3 & (RF3_NO_FEAR)) vp[vn++] = "frightened";
	if (flags3 & (RF3_NO_CONF)) vp[vn++] = "confused";
	if (flags3 & (RF3_NO_SLEEP)) vp[vn++] = "slept";

	/* Describe non-effects */
	if (vn)
	{
		/* Intro */
		text_out(format("%^s", wd_he[msex]));

		/* Scan */
		for (n = 0; n < vn; n++)
		{
			/* Intro */
			if (n == 0) text_out(" cannot be ");
			else if (n < vn - 1) text_out(", ");
			else text_out(" or ");

			/* Dump */
			text_out(vp[n]);
		}

		/* End */
		text_out(".  ");
	}


	/* Do we know how aware it is? */
	if ((((int)r_ptr->r_wake * (int)r_ptr->r_wake) > r_ptr->sleep) ||
	                (r_ptr->r_ignore == MAX_UCHAR) ||
	                ((r_ptr->sleep == 0) && (r_ptr->r_tkills >= 10)))
	{
		cptr act;

		if (r_ptr->sleep > 200)
		{
			act = "prefers to ignore";
		}
		else if (r_ptr->sleep > 95)
		{
			act = "pays very little attention to";
		}
		else if (r_ptr->sleep > 75)
		{
			act = "pays little attention to";
		}
		else if (r_ptr->sleep > 45)
		{
			act = "tends to overlook";
		}
		else if (r_ptr->sleep > 25)
		{
			act = "takes quite a while to see";
		}
		else if (r_ptr->sleep > 10)
		{
			act = "takes a while to see";
		}
		else if (r_ptr->sleep > 5)
		{
			act = "is fairly observant of";
		}
		else if (r_ptr->sleep > 3)
		{
			act = "is observant of";
		}
		else if (r_ptr->sleep > 1)
		{
			act = "is very observant of";
		}
		else if (r_ptr->sleep > 0)
		{
			act = "is vigilant for";
		}
		else
		{
			act = "is ever vigilant for";
		}

		text_out(format("%^s %s intruders, which %s may notice from %d feet.  ",
		                wd_he[msex], act, wd_he[msex], 10 * r_ptr->aaf));
	}


	/* Drops gold and/or items */
	if (r_ptr->r_drop_gold || r_ptr->r_drop_item)
	{
		/* No "n" needed */
		sin = FALSE;

		/* Intro */
		text_out(format("%^s may carry", wd_he[msex]));

		/* Count maximum drop */
		n = MAX(r_ptr->r_drop_gold, r_ptr->r_drop_item);

		/* One drop (may need an "n") */
		if (n == 1)
		{
			text_out(" a");
			sin = TRUE;
		}

		/* Two drops */
		else if (n == 2)
		{
			text_out(" one or two");
		}

		/* Many drops */
		else
		{
			text_out(format(" up to %d", n));
		}


		/* Great */
		if (flags1 & (RF1_DROP_GREAT))
		{
			p = " exceptional";
		}

		/* Good (no "n" needed) */
		else if (flags1 & (RF1_DROP_GOOD))
		{
			p = " good";
			sin = FALSE;
		}

		/* Okay */
		else
		{
			p = NULL;
		}


		/* Objects */
		if (r_ptr->r_drop_item)
		{
			/* Handle singular "an" */
			if (sin) text_out("n");
			sin = FALSE;

			/* Dump "object(s)" */
			if (p) text_out_c(TERM_ORANGE, p);
			text_out(" object");
			if (n != 1) text_out("s");

			/* Conjunction replaces variety, if needed for "gold" below */
			p = " or";
		}

		/* Treasures */
		if (r_ptr->r_drop_gold)
		{
			/* Cancel prefix */
			if (!p) sin = FALSE;

			/* Handle singular "an" */
			if (sin) text_out("n");
			sin = FALSE;

			/* Dump "treasure(s)" */
			if (p) text_out(p);
			text_out(" treasure");
			if (n != 1) text_out("s");
		}

		/* End this sentence */
		text_out(".  ");
	}


	/* Count the number of "known" attacks */
	for (n = 0, m = 0; m < 4; m++)
	{
		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Count known attacks */
		if (r_ptr->r_blows[m]) n++;
	}

	/* Examine (and count) the actual attacks */
	for (r = 0, m = 0; m < 4; m++)
	{
		int method, effect, d1, d2;

		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Skip unknown attacks */
		if (!r_ptr->r_blows[m]) continue;


		/* Extract the attack info */
		method = r_ptr->blow[m].method;
		effect = r_ptr->blow[m].effect;
		d1 = r_ptr->blow[m].d_dice;
		d2 = r_ptr->blow[m].d_side;


		/* No method yet */
		p = NULL;

		/* Acquire the method */
		switch (method)
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
		q = NULL;

		/* Acquire the effect */
		switch (effect)
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
			q = "blast sanity";
			break;
		case RBE_HALLU:
			q = "cause hallucinations";
			break;
		case RBE_PARASITE:
			q = "parasite";
			break;
		}


		/* Introduce the attack description */
		if (!r)
		{
			text_out(format("%^s can ", wd_he[msex]));
		}
		else if (r < n - 1)
		{
			text_out(", ");
		}
		else
		{
			text_out(", and ");
		}


		/* Hack -- force a method */
		if (!p) p = "do something weird";

		/* Describe the method */
		text_out(p);


		/* Describe the effect (if any) */
		if (q)
		{
			/* Describe the attack type */
			text_out(" to ");
			text_out_c(TERM_YELLOW, q);

			/* Describe damage (if known) */
			if (d1 && d2 && know_damage(r_idx, m))
			{
				/* Display the damage */
				text_out(" with damage");
				text_out_c(TERM_L_GREEN, format(" %dd%d", d1, d2));
			}
		}


		/* Count the attacks as printed */
		r++;
	}

	/* Finish sentence above */
	if (r)
	{
		text_out(".  ");
	}

	/* Notice lack of attacks */
	else if (flags1 & (RF1_NEVER_BLOW))
	{
		text_out(format("%^s has no physical attacks.  ", wd_he[msex]));
	}

	/* Or describe the lack of knowledge */
	else
	{
		text_out(format("Nothing is known about %s attack.  ", wd_his[msex]));
	}


	/* All done */
	text_out("\n");

	/* Cheat -- know everything */
	if ((cheat_know) && (remem == 0))
	{
		/* Hack -- restore memory */
		COPY(r_ptr, &save_mem, monster_race);
	}
}

/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_name(int r_idx, int ego)
{
	monster_race *r_ptr = race_info_idx(r_idx, ego);

	byte	a1, a2;
	char	c1, c2;

	/* Access the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Access the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;

	/* Hack -- fake monochrome */
	if (!use_color) a1 = TERM_WHITE;
	if (!use_color) a2 = TERM_WHITE;

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		Term_addstr( -1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	if (ego)
	{
		if (re_info[ego].before) Term_addstr( -1, TERM_WHITE, format("%s %s", re_name + re_info[ego].name, r_name + r_ptr->name));
		else Term_addstr( -1, TERM_WHITE, format("%s %s", r_name + r_ptr->name, re_name + re_info[ego].name));
	}
	else
	{
		Term_addstr( -1, TERM_WHITE, r_name + r_ptr->name);
	}

	/* Append the "standard" attr/char info */
	Term_addstr( -1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	if (use_bigtile && (a1 & 0x80)) Term_addch(255, 255);
	Term_addstr( -1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr( -1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	if (use_bigtile && (a2 & 0x80)) Term_addch(255, 255);
	Term_addstr( -1, TERM_WHITE, "'):");
}

/*
 * Hack -- Display the "name" and "attr/chars" of a monster race on top
 */
static void roff_top(int r_idx, int ego)
{
	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	roff_name(r_idx, ego);
}

/*
 * Hack -- describe the given monster race at the top of the screen
 */
void screen_roff(int r_idx, int ego, int remember)
{
	/* Flush messages */
	msg_print(NULL);

	/* Begin recall */
	Term_erase(0, 1, 255);

	/* Recall monster */
	roff_aux(r_idx, ego, remember);

	/* Describe monster */
	roff_top(r_idx, ego);
}

/*
 * Ddescribe the given monster race at the current pos of the "term" window
 */
void monster_description_out(int r_idx, int ego)
{
	roff_name(r_idx, ego);
	roff_aux(r_idx, ego, 0);
}

/*
 * Hack -- describe the given monster race in the current "term" window
 */
void display_roff(int r_idx, int ego)
{
	int y;

	/* Erase the window */
	for (y = 0; y < Term->hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Recall monster */
	roff_aux(r_idx, ego, 0);

	/* Describe monster */
	roff_top(r_idx, ego);
}


bool monster_quest(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Random quests are in the dungeon */
	if (!(r_ptr->flags8 & RF8_DUNGEON)) return FALSE;

	/* No random quests for aquatic monsters */
	if (r_ptr->flags7 & RF7_AQUATIC) return FALSE;

	/* No random quests for multiplying monsters */
	if (r_ptr->flags4 & RF4_MULTIPLY) return FALSE;

	return TRUE;
}


bool monster_dungeon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_DUNGEON)
		return TRUE;
	else
		return FALSE;
}


bool monster_ocean(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_OCEAN)
		return TRUE;
	else
		return FALSE;
}


bool monster_shore(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_SHORE)
		return TRUE;
	else
		return FALSE;
}


bool monster_waste(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_WASTE)
		return TRUE;
	else
		return FALSE;
}


bool monster_town(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_TOWN)
		return TRUE;
	else
		return FALSE;
}


bool monster_wood(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_WOOD)
		return TRUE;
	else
		return FALSE;
}


bool monster_volcano(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_VOLCANO)
		return TRUE;
	else
		return FALSE;
}


bool monster_mountain(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_MOUNTAIN)
		return TRUE;
	else
		return FALSE;
}


bool monster_grass(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags8 & RF8_WILD_GRASS)
		return TRUE;
	else
		return FALSE;
}


bool monster_deep_water(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (r_ptr->flags7 & RF7_AQUATIC)
		return TRUE;
	else
		return FALSE;
}


bool monster_shallow_water(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (r_ptr->flags2 & RF2_AURA_FIRE)
		return FALSE;
	else
		return TRUE;
}


bool monster_lava(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (((r_ptr->flags3 & RF3_IM_FIRE) ||
	                (r_ptr->flags7 & RF7_CAN_FLY)) &&
	                !(r_ptr->flags3 & RF3_AURA_COLD))
		return TRUE;
	else
		return FALSE;
}


void set_mon_num_hook(void)
{
	if (!dun_level)
	{
		switch (wf_info[wild_map[p_ptr->wilderness_y][p_ptr->wilderness_x].feat].terrain_idx)
		{
		case TERRAIN_TOWN:
			get_mon_num_hook = monster_town;
			break;
		case TERRAIN_DEEP_WATER:
			get_mon_num_hook = monster_ocean;
			break;
		case TERRAIN_SHALLOW_WATER:
			get_mon_num_hook = monster_shore;
			break;
		case TERRAIN_DIRT:
			get_mon_num_hook = monster_waste;
			break;
		case TERRAIN_GRASS:
			get_mon_num_hook = monster_grass;
			break;
		case TERRAIN_TREES:
			get_mon_num_hook = monster_wood;
			break;
		case TERRAIN_SHALLOW_LAVA:
		case TERRAIN_DEEP_LAVA:
			get_mon_num_hook = monster_volcano;
			break;
		case TERRAIN_MOUNTAIN:
			get_mon_num_hook = monster_mountain;
			break;
		default:
			get_mon_num_hook = monster_dungeon;
			break;
		}
	}
	else
	{
		get_mon_num_hook = monster_dungeon;
	}
}


/*
 * Check if monster can cross terrain
 */
bool monster_can_cross_terrain(byte feat, monster_race *r_ptr)
{
	/* Deep water */
	if (feat == FEAT_DEEP_WATER)
	{
		if ((r_ptr->flags7 & RF7_AQUATIC) ||
		                (r_ptr->flags7 & RF7_CAN_FLY) ||
		                (r_ptr->flags7 & RF7_CAN_SWIM))
			return TRUE;
		else
			return FALSE;
	}
	/* Shallow water */
	else if (feat == FEAT_SHAL_WATER)
	{
		if (r_ptr->flags2 & RF2_AURA_FIRE)
			return FALSE;
		else
			return TRUE;
	}
	/* Aquatic monster */
	else if ((r_ptr->flags7 & RF7_AQUATIC) &&
	                !(r_ptr->flags7 & RF7_CAN_FLY))
	{
		return FALSE;
	}
	/* Lava */
	else if ((feat == FEAT_SHAL_LAVA) ||
	                (feat == FEAT_DEEP_LAVA))
	{
		if ((r_ptr->flags3 & RF3_IM_FIRE) ||
		                (r_ptr->flags7 & RF7_CAN_FLY))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}


void set_mon_num2_hook(int y, int x)
{
	/* Set the monster list */
	switch (cave[y][x].feat)
	{
	case FEAT_SHAL_WATER:
		get_mon_num2_hook = monster_shallow_water;
		break;
	case FEAT_DEEP_WATER:
		get_mon_num2_hook = monster_deep_water;
		break;
	case FEAT_DEEP_LAVA:
	case FEAT_SHAL_LAVA:
		get_mon_num2_hook = monster_lava;
		break;
	default:
		get_mon_num2_hook = NULL;
		break;
	}
}
