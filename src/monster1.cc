/*
 * Copyright (c) 1989 James E. Wilson, Christopher J. Stuart
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "monster1.hpp"

#include "cave_type.hpp"
#include "game.hpp"
#include "monster2.hpp"
#include "monster_ego.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "monster_spell_flag.hpp"
#include "player_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "wilderness_type_info.hpp"

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
static void roff_aux(std::shared_ptr<monster_race const> r_ptr)
{
	bool_ old = FALSE;
	bool_ sin = FALSE;

	int m, n, r;

	cptr p, q;

	bool_ breath = FALSE;
	bool_ magic = FALSE;

	int	vn = 0;
	byte color[64];
	cptr	vp[64];

	/* Shorthand */
	auto const flags = r_ptr->flags;
	monster_spell_flag_set spells = r_ptr->spells;

	/* Extract a gender (if applicable) */
	int msex = 0;
	if (flags & RF_FEMALE)
	{
		msex = 2;
	}
	else if (flags & RF_MALE)
	{
		msex = 1;
	}

	/* Treat uniques differently */
	if (flags & RF_UNIQUE)
	{
		if (r_ptr->max_num == 0)
		{
			text_out("You have slain this foe.  ");
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

		/* Killed none */
		else
		{
			text_out("No battles to the death are recalled.  ");
		}
	}


	/* Descriptions */
	{
		text_out(r_ptr->text);
		text_out("  ");
	}


	/* Nothing yet */
	old = FALSE;

	/* Describe location */
	if (r_ptr->flags & RF_PET)
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
	else
	{
		if (old)
			text_out(", ");
		else
			text_out(format("%^s ", wd_he[msex]));

		text_out(format("is normally found on level ", wd_he[msex]));
		if (dun_level < r_ptr->level) /* out of depth monster */
		{
			text_out_c(TERM_L_RED, format("%d", r_ptr->level));
		}
		else
		{
			text_out_c(TERM_L_GREEN, format("%d", r_ptr->level));
		}

		old = TRUE;
	}


	/* Describe movement */
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
		if ((flags & RF_RAND_50) || (flags & RF_RAND_25))
		{
			/* Adverb */
			if ((flags & RF_RAND_50) && (flags & RF_RAND_25))
			{
				text_out(" extremely");
			}
			else if (flags & RF_RAND_50)
			{
				text_out(" somewhat");
			}
			else if (flags & RF_RAND_25)
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
	if (flags & RF_NEVER_MOVE)
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
	{
		/* Introduction */
		if (flags & RF_UNIQUE)
		{
			text_out("Killing this");
		}
		else
		{
			text_out("A kill of this");
		}

		/* Describe the "quality" */
		if (flags & RF_ELDRITCH_HORROR) text_out_c(TERM_VIOLET, " sanity-blasting");
		if (flags & RF_ANIMAL) text_out_c(TERM_VIOLET, " natural");
		if (flags & RF_EVIL) text_out_c(TERM_VIOLET, " evil");
		if (flags & RF_GOOD) text_out_c(TERM_VIOLET, " good");
		if (flags & RF_UNDEAD) text_out_c(TERM_VIOLET, " undead");

		/* Describe the "race" */
		if (flags & RF_DRAGON) text_out_c(TERM_VIOLET, " dragon");
		else if (flags & RF_DEMON) text_out_c(TERM_VIOLET, " demon");
		else if (flags & RF_GIANT) text_out_c(TERM_VIOLET, " giant");
		else if (flags & RF_TROLL) text_out_c(TERM_VIOLET, " troll");
		else if (flags & RF_ORC) text_out_c(TERM_VIOLET, " orc");
		else if (flags & RF_THUNDERLORD)text_out_c(TERM_VIOLET, " Thunderlord");
		else if (flags & RF_SPIDER) text_out_c(TERM_VIOLET, " spider");
		else if (flags & RF_NAZGUL) text_out_c(TERM_VIOLET, " Nazgul");
		else text_out(" creature");

		/* Group some variables */
		if (TRUE)
		{
			long i, j;

			/* calculate the integer exp part */
			i = static_cast<long>(r_ptr->mexp) * r_ptr->level / p_ptr->lev;

			/* calculate the fractional exp part scaled by 100, */
			/* must use long arithmetic to avoid overflow  */
			j = (((static_cast<long>(r_ptr->mexp) * r_ptr->level % p_ptr->lev) *
			      1000L / p_ptr->lev + 5) / 10);

			/* Mention the experience */
			text_out(" is worth ");
			text_out_c(TERM_ORANGE, format("%ld.%02ld", i, j));
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
			                q, i, p));
		}
	}

	if ((flags & RF_AURA_FIRE) && (flags & RF_AURA_ELEC))
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_VIOLET, "flames and electricity");
		text_out(".  ");
	}
	else if (flags & RF_AURA_FIRE)
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_ORANGE, "flames");
		text_out(".  ");
	}
	else if (flags & RF_AURA_ELEC)
	{
		text_out(format("%^s is surrounded by ", wd_he[msex]));
		text_out_c(TERM_L_BLUE, "electricity");
		text_out(".  ");
	}

	if (flags & RF_REFLECTING)
	{
		text_out(format("%^s ", wd_he[msex]));
		text_out_c(TERM_L_UMBER, "reflects");
		text_out(" bolt spells.  ");
	}


	/* Describe escorts */
	if ((flags & RF_ESCORT) || (flags & RF_ESCORTS))
	{
		text_out(format("%^s usually appears with escorts.  ",
		                wd_he[msex]));
	}

	/* Describe friends */
	else if ((flags & RF_FRIEND) || (flags & RF_FRIENDS))
	{
		text_out(format("%^s usually appears in groups.  ",
		                wd_he[msex]));
	}


	/* Collect inate attacks */
	vn = 0;
	if (spells & SF_SHRIEK) vp[vn++] = "shriek for help";
	if (spells & SF_ROCKET) vp[vn++] = "shoot a rocket";
	if (spells & SF_ARROW_1) vp[vn++] = "fire an arrow";
	if (spells & SF_ARROW_2) vp[vn++] = "fire arrows";
	if (spells & SF_ARROW_3) vp[vn++] = "fire a missile";
	if (spells & SF_ARROW_4) vp[vn++] = "fire missiles";

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
	if (spells & SF_BR_ACID)	vp[vn++] = "acid";
	if (spells & SF_BR_ELEC)	vp[vn++] = "lightning";
	if (spells & SF_BR_FIRE)	vp[vn++] = "fire";
	if (spells & SF_BR_COLD)	vp[vn++] = "frost";
	if (spells & SF_BR_POIS)	vp[vn++] = "poison";
	if (spells & SF_BR_NETH)	vp[vn++] = "nether";
	if (spells & SF_BR_LITE)	vp[vn++] = "light";
	if (spells & SF_BR_DARK)	vp[vn++] = "darkness";
	if (spells & SF_BR_CONF)	vp[vn++] = "confusion";
	if (spells & SF_BR_SOUN)	vp[vn++] = "sound";
	if (spells & SF_BR_CHAO)	vp[vn++] = "chaos";
	if (spells & SF_BR_DISE)	vp[vn++] = "disenchantment";
	if (spells & SF_BR_NEXU)	vp[vn++] = "nexus";
	if (spells & SF_BR_TIME)	vp[vn++] = "time";
	if (spells & SF_BR_INER)	vp[vn++] = "inertia";
	if (spells & SF_BR_GRAV)	vp[vn++] = "gravity";
	if (spells & SF_BR_SHAR)	vp[vn++] = "shards";
	if (spells & SF_BR_PLAS)	vp[vn++] = "plasma";
	if (spells & SF_BR_WALL)	vp[vn++] = "force";
	if (spells & SF_BR_MANA)	vp[vn++] = "mana";
	if (spells & SF_BR_NUKE)	vp[vn++] = "toxic waste";
	if (spells & SF_BR_DISI)	vp[vn++] = "disintegration";

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
	if (spells & SF_BA_ACID) vp[vn++] = "produce acid balls";
	if (spells & SF_BA_ELEC) vp[vn++] = "produce lightning balls";
	if (spells & SF_BA_FIRE) vp[vn++] = "produce fire balls";
	if (spells & SF_BA_COLD) vp[vn++] = "produce frost balls";
	if (spells & SF_BA_POIS) vp[vn++] = "produce poison balls";
	if (spells & SF_BA_NETH) vp[vn++] = "produce nether balls";
	if (spells & SF_BA_WATE) vp[vn++] = "produce water balls";
	if (spells & SF_BA_NUKE) vp[vn++] = "produce balls of radiation";
	if (spells & SF_BA_MANA) vp[vn++] = "invoke mana storms";
	if (spells & SF_BA_DARK) vp[vn++] = "invoke darkness storms";
	if (spells & SF_BA_CHAO) vp[vn++] = "invoke raw chaos";
	if (spells & SF_HAND_DOOM) vp[vn++] = "invoke the Hand of Doom";
	if (spells & SF_DRAIN_MANA) vp[vn++] = "drain mana";
	if (spells & SF_MIND_BLAST) vp[vn++] = "cause mind blasting";
	if (spells & SF_BRAIN_SMASH) vp[vn++] = "cause brain smashing";
	if (spells & (SF_CAUSE_1)) vp[vn++] = "cause light wounds and cursing";
	if (spells & (SF_CAUSE_2)) vp[vn++] = "cause serious wounds and cursing";
	if (spells & (SF_CAUSE_3)) vp[vn++] = "cause critical wounds and cursing";
	if (spells & (SF_CAUSE_4)) vp[vn++] = "cause mortal wounds";
	if (spells & SF_BO_ACID) vp[vn++] = "produce acid bolts";
	if (spells & SF_BO_ELEC) vp[vn++] = "produce lightning bolts";
	if (spells & SF_BO_FIRE) vp[vn++] = "produce fire bolts";
	if (spells & SF_BO_COLD) vp[vn++] = "produce frost bolts";
	if (spells & SF_BO_POIS) vp[vn++] = "produce poison bolts";
	if (spells & SF_BO_NETH) vp[vn++] = "produce nether bolts";
	if (spells & SF_BO_WATE) vp[vn++] = "produce water bolts";
	if (spells & SF_BO_MANA) vp[vn++] = "produce mana bolts";
	if (spells & SF_BO_PLAS) vp[vn++] = "produce plasma bolts";
	if (spells & SF_BO_ICEE) vp[vn++] = "produce ice bolts";
	if (spells & SF_MISSILE) vp[vn++] = "produce magic missiles";
	if (spells & SF_SCARE) vp[vn++] = "terrify";
	if (spells & SF_BLIND) vp[vn++] = "blind";
	if (spells & SF_CONF) vp[vn++] = "confuse";
	if (spells & SF_SLOW) vp[vn++] = "slow";
	if (spells & SF_HOLD) vp[vn++] = "paralyze";
	if (spells & SF_HASTE) vp[vn++] = "haste-self";
	if (spells & SF_HEAL) vp[vn++] = "heal-self";
	if (spells & SF_BLINK) vp[vn++] = "blink-self";
	if (spells & SF_TPORT) vp[vn++] = "teleport-self";
	if (spells & SF_S_BUG) vp[vn++] = "summon software bugs";
	if (spells & SF_S_RNG) vp[vn++] = "summon RNG";
	if (spells & SF_TELE_TO) vp[vn++] = "teleport to";
	if (spells & SF_TELE_AWAY) vp[vn++] = "teleport away";
	if (spells & SF_TELE_LEVEL) vp[vn++] = "teleport level";
	if (spells & SF_S_THUNDERLORD) vp[vn++] = "summon a Thunderlord";
	if (spells & SF_DARKNESS) vp[vn++] = "create darkness";
	if (spells & SF_TRAPS) vp[vn++] = "create traps";
	if (spells & SF_FORGET) vp[vn++] = "cause amnesia";
	if (spells & SF_RAISE_DEAD) vp[vn++] = "raise dead";
	if (spells & SF_S_MONSTER) vp[vn++] = "summon a monster";
	if (spells & SF_S_MONSTERS) vp[vn++] = "summon monsters";
	if (spells & SF_S_KIN) vp[vn++] = "summon aid";
	if (spells & SF_S_ANT) vp[vn++] = "summon ants";
	if (spells & SF_S_SPIDER) vp[vn++] = "summon spiders";
	if (spells & SF_S_HOUND) vp[vn++] = "summon hounds";
	if (spells & SF_S_HYDRA) vp[vn++] = "summon hydras";
	if (spells & SF_S_ANGEL) vp[vn++] = "summon an angel";
	if (spells & SF_S_DEMON) vp[vn++] = "summon a demon";
	if (spells & SF_S_UNDEAD) vp[vn++] = "summon an undead";
	if (spells & SF_S_DRAGON) vp[vn++] = "summon a dragon";
	if (spells & SF_S_ANIMAL) vp[vn++] = "summon animal";
	if (spells & SF_S_ANIMALS) vp[vn++] = "summon animals";
	if (spells & SF_S_HI_UNDEAD) vp[vn++] = "summon Greater Undead";
	if (spells & SF_S_HI_DRAGON) vp[vn++] = "summon Ancient Dragons";
	if (spells & SF_S_HI_DEMON) vp[vn++] = "summon Greater Demons";
	if (spells & SF_S_WRAITH) vp[vn++] = "summon Ringwraith";
	if (spells & SF_S_UNIQUE) vp[vn++] = "summon Unique Monsters";

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
		if (flags & RF_SMART) text_out_c(TERM_YELLOW, " intelligently");

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
		/* Average frequency */
		n = (r_ptr->freq_inate + r_ptr->freq_spell) / 2;

		/* Describe the spell frequency */
		text_out("; ");
		text_out_c(TERM_L_GREEN, "1");
		text_out(" time in ");
		text_out_c(TERM_L_GREEN, format("%d", 100 / n));

		/* End this sentence */
		text_out(".  ");
	}


	/* Describe monster "toughness" */
	{
		/* Armor */
		text_out(format("%^s has an armor rating of ", wd_he[msex]));
		text_out_c(TERM_L_GREEN, format("%d", r_ptr->ac));

		/* Maximized hitpoints */
		if (flags & RF_FORCE_MAXHP)
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
	if (flags & RF_OPEN_DOOR) vp[vn++] = "open doors";
	if (flags & RF_BASH_DOOR) vp[vn++] = "bash down doors";
	if (flags & RF_PASS_WALL) vp[vn++] = "pass through walls";
	if (flags & RF_KILL_WALL) vp[vn++] = "bore through walls";
	if (flags & RF_MOVE_BODY) vp[vn++] = "push past weaker monsters";
	if (flags & RF_KILL_BODY) vp[vn++] = "destroy weaker monsters";
	if (flags & RF_TAKE_ITEM) vp[vn++] = "pick up objects";
	if (flags & RF_KILL_ITEM) vp[vn++] = "destroy objects";
	if (flags & RF_HAS_LITE) vp[vn++] = "illuminate the dungeon";

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
	if (flags & RF_INVISIBLE)
	{
		text_out_c(TERM_GREEN, format("%^s is invisible.  ", wd_he[msex]));
	}
	if (flags & RF_COLD_BLOOD)
	{
		text_out(format("%^s is cold blooded.  ", wd_he[msex]));
	}
	if (flags & RF_EMPTY_MIND)
	{
		text_out(format("%^s is not detected by telepathy.  ", wd_he[msex]));
	}
	if (flags & RF_WEIRD_MIND)
	{
		text_out(format("%^s is rarely detected by telepathy.  ", wd_he[msex]));
	}
	if (spells & SF_MULTIPLY)
	{
		text_out_c(TERM_L_UMBER, format("%^s breeds explosively.  ", wd_he[msex]));
	}
	if (flags & RF_REGENERATE)
	{
		text_out_c(TERM_L_WHITE, format("%^s regenerates quickly.  ", wd_he[msex]));
	}

	if (r_ptr->flags & RF_MORTAL)
	{
		text_out_c(TERM_RED, format("%^s is a mortal being.  ", wd_he[msex]));
	}
	else
	{
		text_out_c(TERM_L_BLUE, format("%^s is an immortal being.  ", wd_he[msex]));
	}


	/* Collect susceptibilities */
	vn = 0;
	if (flags & RF_HURT_ROCK)
	{
		vp[vn++] = "rock remover";
		color[vn - 1] = TERM_UMBER;
	}
	if (flags & RF_HURT_LITE)
	{
		vp[vn++] = "bright light";
		color[vn - 1] = TERM_YELLOW;
	}
	if (flags & RF_SUSCEP_FIRE)
	{
		vp[vn++] = "fire";
		color[vn - 1] = TERM_RED;
	}
	if (flags & RF_SUSCEP_COLD)
	{
		vp[vn++] = "cold";
		color[vn - 1] = TERM_L_WHITE;
	}
	if (flags & RF_SUSCEP_ACID)
	{
		vp[vn++] = "acid";
		color[vn - 1] = TERM_GREEN;
	}
	if (flags & RF_SUSCEP_ELEC)
	{
		vp[vn++] = "lightning";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags & RF_SUSCEP_POIS)
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
	if (flags & RF_IM_ACID)
	{
		vp[vn++] = "acid";
		color[vn - 1] = TERM_L_GREEN;
	}
	if (flags & RF_IM_ELEC)
	{
		vp[vn++] = "lightning";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags & RF_IM_FIRE)
	{
		vp[vn++] = "fire";
		color[vn - 1] = TERM_L_RED;
	}
	if (flags & RF_IM_COLD)
	{
		vp[vn++] = "cold";
		color[vn - 1] = TERM_L_BLUE;
	}
	if (flags & RF_IM_POIS)
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
	if (flags & RF_RES_NETH) vp[vn++] = "nether";
	if (flags & RF_RES_WATE) vp[vn++] = "water";
	if (flags & RF_RES_PLAS) vp[vn++] = "plasma";
	if (flags & RF_RES_NEXU) vp[vn++] = "nexus";
	if (flags & RF_RES_DISE) vp[vn++] = "disenchantment";
	if (flags & RF_RES_TELE) vp[vn++] = "teleportation";

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
	if (flags & RF_NO_STUN) vp[vn++] = "stunned";
	if (flags & RF_NO_FEAR) vp[vn++] = "frightened";
	if (flags & RF_NO_CONF) vp[vn++] = "confused";
	if (flags & RF_NO_SLEEP) vp[vn++] = "slept";

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


	/* How aware is it? */
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
	{
		/* Calculate drops */
		byte drop_gold;
		byte drop_item;

		drop_gold = drop_item =
			(((r_ptr->flags & RF_DROP_4D2) ? 8 : 0) +
			 ((r_ptr->flags & RF_DROP_3D2) ? 6 : 0) +
			 ((r_ptr->flags & RF_DROP_2D2) ? 4 : 0) +
			 ((r_ptr->flags & RF_DROP_1D2) ? 2 : 0) +
			 ((r_ptr->flags & RF_DROP_90) ? 1 : 0) +
			 ((r_ptr->flags & RF_DROP_60) ? 1 : 0));

		if (r_ptr->flags & RF_ONLY_GOLD) drop_item = 0;
		if (r_ptr->flags & RF_ONLY_ITEM) drop_gold = 0;

		/* No "n" needed */
		sin = FALSE;

		/* Count maximum drop */
		n = MAX(drop_gold, drop_item);

		/* Intro text */
		if (n == 0)
		{
			text_out(format("%^s carries no items", wd_he[msex]));

		}
		else if (n == 1)
		{
			text_out(format("%^s may carry a", wd_he[msex]));
			sin = TRUE;
		}
		else if (n == 2)
		{
			text_out(format("%^s may carry one or two", wd_he[msex]));
		}
		else
		{
			text_out(format("%^s may carry up to %d", wd_he[msex], n));
		}


		/* Great */
		if (flags & RF_DROP_GREAT)
		{
			p = " exceptional";
		}

		/* Good (no "n" needed) */
		else if (flags & RF_DROP_GOOD)
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
		if (drop_item)
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
		if (drop_gold)
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


	/* Count the number of attacks */
	for (n = 0, m = 0; m < 4; m++)
	{
		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Count known attacks */
		n++;
	}

	/* Examine the actual attacks */
	for (r = 0, m = 0; m < 4; m++)
	{
		/* Skip non-attacks */
		if (!r_ptr->blow[m].method) continue;

		/* Extract the attack info */
		int method = r_ptr->blow[m].method;
		int effect = r_ptr->blow[m].effect;
		int d1 = r_ptr->blow[m].d_dice;
		int d2 = r_ptr->blow[m].d_side;


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
			if (d1 && d2)
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
	else if (flags & RF_NEVER_BLOW)
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
}

/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_name(int r_idx, int ego)
{
	const auto &re_info = game->edit_data.re_info;

	const auto r_ptr = race_info_idx(r_idx, ego);

	/* Access the chars */
	const char c1 = r_ptr->d_char;
	const char c2 = r_ptr->x_char;

	/* Access the attrs */
	const byte a1 = r_ptr->d_attr;
	const byte a2 = r_ptr->x_attr;

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags & RF_UNIQUE))
	{
		Term_addstr( -1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	if (ego)
	{
		if (re_info[ego].before)
		{
			Term_addstr( -1, TERM_WHITE, format("%s %s", re_info[ego].name, r_ptr->name));
		}
		else
		{
			Term_addstr( -1, TERM_WHITE, format("%s %s", r_ptr->name, re_info[ego].name));
		}
	}
	else
	{
		Term_addstr( -1, TERM_WHITE, r_ptr->name);
	}

	/* Append the "standard" attr/char info */
	Term_addstr( -1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr( -1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr( -1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
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
void screen_roff(int r_idx, int ego)
{
	auto r_ptr = race_info_idx(r_idx, ego);

	/* Flush messages */
	msg_print(NULL);

	/* Begin recall */
	Term_erase(0, 1, 255);

	/* Recall monster */
	roff_aux(r_ptr);

	/* Describe monster */
	roff_top(r_idx, ego);
}

/*
 * Describe the given monster race at the current pos of the "term" window
 */
void monster_description_out(int r_idx, int ego)
{
	auto r_ptr = race_info_idx(r_idx, ego);
	roff_name(r_idx, ego);
	roff_aux(r_ptr);
}

/*
 * Hack -- describe the given monster race in the current "term" window
 */
void display_roff(int r_idx, int ego)
{
	int y;
	int hgt;
	Term_get_size(nullptr, &hgt);

	/* Erase the window */
	for (y = 0; y < hgt; y++)
	{
		/* Erase the line */
		Term_erase(0, y, 255);
	}

	/* Begin recall */
	Term_gotoxy(0, 1);

	/* Recall monster */
	auto r_ptr = race_info_idx(r_idx, ego);
	roff_aux(r_ptr);

	/* Describe monster */
	roff_top(r_idx, ego);
}


bool_ monster_quest(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* Random quests are in the dungeon */
	if (r_ptr->flags & RF_WILD_ONLY) return FALSE;

	/* No random quests for aquatic monsters */
	if (r_ptr->flags & RF_AQUATIC) return FALSE;

	/* No random quests for multiplying monsters */
	if (r_ptr->spells & SF_MULTIPLY) return FALSE;

	return TRUE;
}


bool_ monster_dungeon(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!(r_ptr->flags & RF_WILD_ONLY))
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_ocean(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_OCEAN)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_shore(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_SHORE)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_waste(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_WASTE)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_town(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_TOWN)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_wood(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_WOOD)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_volcano(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_VOLCANO)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_mountain(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_MOUNTAIN)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_grass(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (r_ptr->flags & RF_WILD_GRASS)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_deep_water(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (r_ptr->flags & RF_AQUATIC)
		return TRUE;
	else
		return FALSE;
}


static bool_ monster_shallow_water(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (r_ptr->flags & RF_AURA_FIRE)
		return FALSE;
	else
		return TRUE;
}


static bool_ monster_lava(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	if (!monster_dungeon(r_idx)) return FALSE;

	if (((r_ptr->flags & RF_IM_FIRE) ||
	                (r_ptr->flags & RF_CAN_FLY)) &&
	                !(r_ptr->flags & RF_AURA_COLD))
		return TRUE;
	else
		return FALSE;
}


void set_mon_num_hook(void)
{
	auto const &wf_info = game->edit_data.wf_info;

	if (!dun_level)
	{
		auto const &wilderness = game->wilderness;
		switch (wf_info[wilderness(p_ptr->wilderness_x, p_ptr->wilderness_y).feat].terrain_idx)
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
bool_ monster_can_cross_terrain(byte feat, std::shared_ptr<monster_race> r_ptr)
{
	/* Deep water */
	if (feat == FEAT_DEEP_WATER)
	{
		if ((r_ptr->flags & RF_AQUATIC) ||
		                (r_ptr->flags & RF_CAN_FLY) ||
		                (r_ptr->flags & RF_CAN_SWIM))
			return TRUE;
		else
			return FALSE;
	}
	/* Shallow water */
	else if (feat == FEAT_SHAL_WATER)
	{
		if (r_ptr->flags & RF_AURA_FIRE)
			return FALSE;
		else
			return TRUE;
	}
	/* Aquatic monster */
	else if ((r_ptr->flags & RF_AQUATIC) &&
	                !(r_ptr->flags & RF_CAN_FLY))
	{
		return FALSE;
	}
	/* Lava */
	else if ((feat == FEAT_SHAL_LAVA) ||
	                (feat == FEAT_DEEP_LAVA))
	{
		if ((r_ptr->flags & RF_IM_FIRE) ||
		                (r_ptr->flags & RF_CAN_FLY))
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
