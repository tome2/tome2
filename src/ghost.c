/* File: ghost.c */

/*
 * Purpose: ghost functions
 *
 * Created by DarkGod for PernAngband 4.1.0
 * Lot of code from Drangband
 */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Save a "bones" file for a dead character
 *
 * Should probably attempt some form of locking...
 */
void make_bones(void)
{
#if 0 /* DGDGDGDG */
	FILE *fp;

	int i;

	char str[1024];


	/* Ignore wizards and borgs */
	if (!(noscore & 0x00FF))
	{
		/* Ignore people who die in town */
		if (dun_level)
		{
			int level;
			char tmp[128];

			/* Slightly more tenacious saving routine. */
			for (i = 0; i < 5; i++)
			{
				/* Ghost hovers near level of death. */
				if (i == 0) level = dun_level;
				else level = dun_level + 5 - damroll(2, 4);
				if (level < 1) level = randint(4);

				/* XXX XXX XXX "Bones" name */
				sprintf(tmp, "bone%03d.%03d", dungeon_type, level);

				/* Build the filename */
				path_build(str, 1024, ANGBAND_DIR_BONE, tmp);

				/* Grab permission */
				safe_setuid_grab();

				/* Attempt to open the bones file */
				fp = my_fopen(str, "r");

				/* Drop permission */
				safe_setuid_drop();

				/* Close it right away */
				if (fp) my_fclose(fp);

				/* Do not over-write a previous ghost */
				if (fp) continue;

				/* If no file by that name exists, we can make a new one. */
				if (!(fp)) break;
			}


			/* File type is "TEXT" */
			FILE_TYPE(FILE_TYPE_TEXT);

			/* Grab permission */
			safe_setuid_grab();

			/* Try to write a new "Bones File" */
			fp = my_fopen(str, "w");

			/* Drop permission */
			safe_setuid_drop();

			/* Not allowed to write it?  Weird. */
			if (!fp) return;

			/* Save the info */
			fprintf(fp, "%s\n", player_name);
			fprintf(fp, "%d\n", p_ptr->mhp);
			fprintf(fp, "%d\n", p_ptr->prace);
			fprintf(fp, "%d\n", p_ptr->pclass);

			/* Close and save the Bones file */
			my_fclose(fp);
		}
	}
#endif
}

#if 0 /* DGDGDGDG */
/*
 * Ghost generation info
 */

static int ghost_race;
static int ghost_class;

static char gb_name[32];


/*
 * Set a "blow" record for the ghost
 */
static void ghost_blow(int i, int m, int e, int d, int s)
{
	monster_race *g = &r_info[max_r_idx - 1];

	/* Save the data */
	g->blow[i].method = m;
	g->blow[i].effect = e;
	g->blow[i].d_dice = d;
	g->blow[i].d_side = s;
}


/*
 * Prepare the "ghost" race (method 1)
 */
static void set_ghost_aux_1(void)
{
	monster_race *r_ptr = &r_info[max_r_idx - 1];

	int i, d1, d2;

	int attack1, attack2;

	int lev = r_ptr->level;

	int grace = ghost_race;
	int gclass = ghost_class;

	cptr gr_name = rp_name + race_info[grace].title;
	cptr gc_name = class_info[grace].title;


	/* A wanderer from the town */
	sprintf(r_name + r_ptr->name, "%s, the skeletal %s %s",
	        gb_name, gr_name, gc_name);


	/* Use a "player" symbol */
	r_ptr->d_char = 's';

	/* Open doors, bash doors */
	r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);


	/* Treasure drops */
	r_ptr->flags1 |= (RF1_DROP_60 | RF1_DROP_90);

	/* Treasure drops */
	if (lev >= 10) r_ptr->flags1 |= (RF1_DROP_1D2);
	if (lev >= 20) r_ptr->flags1 |= (RF1_DROP_2D2);
	if (lev >= 30) r_ptr->flags1 |= (RF1_DROP_4D2);

	/* Treasure drops */
	if (lev >= 40) r_ptr->flags1 &= ~(RF1_DROP_4D2);
	if (lev >= 40) r_ptr->flags1 |= (RF1_DROP_GREAT);


	/* Extract an "immunity power" */
	i = (lev / 5) + randint(5);

	/* Immunity (by level) */
	switch ((i > 12) ? 12 : i)
	{
	case 12 :
		{
			r_ptr->flags3 |= (RF3_IM_POIS);
		}

	case 11:
	case 10:
		{
			r_ptr->flags3 |= (RF3_IM_ACID);
		}

	case 9:
	case 8:
	case 7:
		{
			r_ptr->flags3 |= (RF3_IM_FIRE);
		}

	case 6:
	case 5:
	case 4:
		{
			r_ptr->flags3 |= (RF3_IM_COLD);
		}

	case 3:
	case 2:
	case 1:
		{
			r_ptr->flags3 |= (RF3_IM_ELEC);
		}
	}


	/* Extract some spells */
	switch (gclass)
	{
		/* Warrior */
	case CLASS_WARRIOR:
	case CLASS_UNBELIEVER:
	case CLASS_ARCHER:
	case CLASS_MONK:
	case CLASS_SYMBIANT:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
			r_ptr->flags4 |= (RF4_ARROW_1);
			if (lev > 15) r_ptr->flags4 |= (RF4_ARROW_2);
			if (lev > 30) r_ptr->flags4 |= (RF4_ARROW_3);
			if (lev > 45) r_ptr->flags4 |= (RF4_ARROW_4);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_UMBER;
			break;
		}

		/* Mage */
	case CLASS_MAGE:
	case CLASS_HIGH_MAGE:
	case CLASS_POWERMAGE:
	case CLASS_RUNECRAFTER:
	case CLASS_HARPER:
	case CLASS_SORCERER:
	case CLASS_ILLUSIONIST:
	case CLASS_DRUID:
	case CLASS_NECRO:
	case CLASS_ALCHEMIST:
	case CLASS_CHAOS_WARRIOR:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 2;
			r_ptr->flags4 |= (RF4_ARROW_1);
			r_ptr->flags5 |= (RF5_SLOW | RF5_CONF);
			r_ptr->flags6 |= (RF6_BLINK);
			if (lev > 5) r_ptr->flags5 |= (RF5_BA_POIS);
			if (lev > 7) r_ptr->flags5 |= (RF5_BO_ELEC);
			if (lev > 10) r_ptr->flags5 |= (RF5_BO_COLD);
			if (lev > 12) r_ptr->flags6 |= (RF6_TPORT);
			if (lev > 15) r_ptr->flags5 |= (RF5_BO_ACID);
			if (lev > 20) r_ptr->flags5 |= (RF5_BO_FIRE);
			if (lev > 25) r_ptr->flags5 |= (RF5_BA_COLD);
			if (lev > 25) r_ptr->flags6 |= (RF6_HASTE);
			if (lev > 30) r_ptr->flags5 |= (RF5_BA_FIRE);
			if (lev > 40) r_ptr->flags5 |= (RF5_BO_MANA);
			if (lev > 50) r_ptr->flags6 |= (RF6_S_DRAGON);
			if (lev > 60) r_ptr->flags5 |= (RF5_BA_MANA);
			if (lev > 70) r_ptr->flags6 |= (RF6_S_HI_UNDEAD);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_RED;
			break;
		}

		/* Priest */
	case CLASS_PRIEST:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
			r_ptr->flags5 |= (RF5_CAUSE_1 | RF5_SCARE);
			if (lev > 5) r_ptr->flags6 |= (RF6_HEAL);
			if (lev > 10) r_ptr->flags5 |= (RF5_BLIND);
			if (lev > 12) r_ptr->flags5 |= (RF5_CAUSE_2);
			if (lev > 18) r_ptr->flags5 |= (RF5_HOLD);
			if (lev > 25) r_ptr->flags5 |= (RF5_CONF);
			if (lev > 30) r_ptr->flags5 |= (RF5_CAUSE_3);
			if (lev > 35) r_ptr->flags5 |= (RF5_DRAIN_MANA);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_L_BLUE;
			break;
		}

		/* Rogue */
	case CLASS_ROGUE:
	case CLASS_MERCHANT:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 6;
			r_ptr->flags6 |= (RF6_BLINK);
			if (lev > 10) r_ptr->flags5 |= (RF5_CONF);
			if (lev > 18) r_ptr->flags5 |= (RF5_SLOW);
			if (lev > 25) r_ptr->flags6 |= (RF6_TPORT);
			if (lev > 30) r_ptr->flags5 |= (RF5_HOLD);
			if (lev > 35) r_ptr->flags6 |= (RF6_TELE_TO);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_BLUE;
			break;
		}

		/* Ranger */
	case CLASS_RANGER:
	case CLASS_WARLOCK:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 6;
			r_ptr->flags4 |= (RF4_ARROW_1);
			if (lev > 5) r_ptr->flags5 |= (RF5_BA_POIS);
			if (lev > 7) r_ptr->flags5 |= (RF5_BO_ELEC);
			if (lev > 10) r_ptr->flags5 |= (RF5_BO_COLD);
			if (lev > 18) r_ptr->flags5 |= (RF5_BO_ACID);
			if (lev > 25) r_ptr->flags5 |= (RF5_BO_FIRE);
			if (lev > 30) r_ptr->flags5 |= (RF5_BA_COLD);
			if (lev > 35) r_ptr->flags5 |= (RF5_BA_FIRE);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_GREEN;
			break;
		}

		/* Paladin */
	case CLASS_PALADIN:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
			r_ptr->flags5 |= (RF5_CAUSE_1 | RF5_SCARE);
			if (lev > 5) r_ptr->flags6 |= (RF6_HEAL);
			if (lev > 10) r_ptr->flags5 |= (RF5_BLIND);
			if (lev > 12) r_ptr->flags5 |= (RF5_CAUSE_2);
			if (lev > 18) r_ptr->flags5 |= (RF5_HOLD);
			if (lev > 25) r_ptr->flags5 |= (RF5_CONF);
			if (lev > 30) r_ptr->flags5 |= (RF5_CAUSE_3);
			if (lev > 35) r_ptr->flags5 |= (RF5_DRAIN_MANA);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_WHITE;
			break;
		}

		/* Beastmaster */
	case CLASS_BEASTMASTER:
	case CLASS_DAEMONOLOGIST:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 3;
			r_ptr->flags6 |= (RF6_S_KIN);
			if (lev > 10) r_ptr->flags6 |= (RF6_S_SPIDER);
			if (lev > 15) r_ptr->flags6 |= (RF6_S_MONSTERS);
			if (lev > 20) r_ptr->flags6 |= (RF6_S_HOUND);
			if (lev > 35) r_ptr->flags6 |= (RF6_S_DEMON);
			if (lev > 30) r_ptr->flags6 |= (RF6_S_UNDEAD);
			if (lev > 35) r_ptr->flags6 |= (RF6_S_DRAGON);
			if (lev > 40) r_ptr->flags6 |= (RF6_S_HI_UNDEAD);
			if (lev > 45) r_ptr->flags6 |= (RF6_S_HI_DRAGON);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_UMBER;
			break;
		}

		/* Mindcrafter */
	case CLASS_MINDCRAFTER:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
			r_ptr->flags5 |= (RF5_CAUSE_1 | RF5_SCARE);
			if (lev > 5) r_ptr->flags6 |= (RF6_HEAL);
			if (lev > 10) r_ptr->flags5 |= (RF5_MIND_BLAST);
			if (lev > 12) r_ptr->flags6 |= (RF6_FORGET);
			if (lev > 18) r_ptr->flags5 |= (RF5_BRAIN_SMASH);
			if (lev > 25) r_ptr->flags5 |= (RF5_CONF);
			if (lev > 30) r_ptr->flags5 |= (RF5_CAUSE_3);
			if (lev > 35) r_ptr->flags5 |= (RF5_DRAIN_MANA);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_L_BLUE;
			break;
		}

		/* Possessor */
	case CLASS_POSSESSOR:
	case CLASS_MIMIC:
		{
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 4;
			if (lev > 7) r_ptr->flags4 |= (RF4_BR_ELEC);
			if (lev > 10) r_ptr->flags4 |= (RF4_BR_COLD);
			if (lev > 18) r_ptr->flags4 |= (RF4_BR_ACID);
			if (lev > 25) r_ptr->flags4 |= (RF4_BR_FIRE);
			if (lev > 35) r_ptr->flags4 |= (RF4_BR_CHAO);
			if (lev > 45) r_ptr->flags4 |= (RF4_BR_TIME);
			r_ptr->flags1 |= (RF1_CHAR_MULTI);
			r_ptr->flags1 |= (RF1_ATTR_MULTI);

			/* Use a "player" color */
			r_ptr->d_attr = TERM_UMBER;
			break;
		}

	default:
		{
			/* Use a "player" color */
			r_ptr->d_attr = TERM_WHITE;
			break;
		}
	}


	/* Racial properties */
	if (grace == RACE_HALF_ORC) r_ptr->flags3 |= (RF3_ORC);
	if (grace == RACE_KOBOLD) r_ptr->flags3 |= (RF3_ORC);
	if (grace == RACE_HALF_TROLL) r_ptr->flags3 |= (RF3_TROLL);
	if (grace == RACE_THUNDERLORD) r_ptr->flags3 |= (RF3_THUNDERLORD);


	/* Armor class */
	r_ptr->ac = 15 + randint(15);

	/* Non mage/priest gets extra armor */
	if ((gclass != CLASS_MAGE) && (gclass != CLASS_PRIEST))
	{
		r_ptr->ac += randint(60);
	}


	/* Default speed (normal) */
	r_ptr->speed = 110;

	/* Higher level they are faster */
	if (lev >= 50) r_ptr->speed += 10;

	/* High level mages are fast... */
	if ((gclass == CLASS_MAGE) && (lev >= 20)) r_ptr->speed += 10;

	/* High level rogues are fast... */
	if ((gclass == CLASS_ROGUE) && (lev >= 30)) r_ptr->speed += 10;


	/* Base damage */
	d1 = 1;
	d2 = 2 * (lev + 5);

	/* Break up the damage */
	while ((d1 * 8) < d2)
	{
		d1 = d1 * 2;
		d2 = d2 / 2;
	}

	attack1 = attack2 = RBM_HIT;

	/* Extract attacks */
	switch (gclass)
	{

		/* Warrior */
	case CLASS_WARRIOR:
	case CLASS_UNBELIEVER:
	case CLASS_ARCHER:
	case CLASS_MONK:
	case CLASS_SYMBIANT:
	case CLASS_BEASTMASTER:
	case CLASS_DAEMONOLOGIST:
	case CLASS_POSSESSOR:
	case CLASS_MIMIC:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 2;

			/* Normal attacks (four) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(1, attack2, RBE_HURT, d1, d2);
			ghost_blow(2, attack1, RBE_HURT, d1, d2);
			ghost_blow(3, attack2, RBE_HURT, d1, d2);

			break;
		}

		/* Mage */
	case CLASS_MAGE:
	case CLASS_HIGH_MAGE:
	case CLASS_POWERMAGE:
	case CLASS_RUNECRAFTER:
	case CLASS_HARPER:
	case CLASS_SORCERER:
	case CLASS_ILLUSIONIST:
	case CLASS_DRUID:
	case CLASS_NECRO:
	case CLASS_ALCHEMIST:
	case CLASS_CHAOS_WARRIOR:
	case CLASS_MERCHANT:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 3 / 2;

			/* Normal attacks (one) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);

			break;
		}

		/* Priest */
	case CLASS_PRIEST:
	case CLASS_MINDCRAFTER:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 3 / 2;

			/* Normal attacks (two) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(0, attack2, RBE_HURT, d1, d2);

			break;
		}

		/* Rogue */
	case CLASS_ROGUE:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 2;

			/* Normal attacks */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(1, attack2, RBE_HURT, d1, d2);

			/* Special attacks -- Touch to steal */
			ghost_blow(2, RBM_TOUCH, RBE_EAT_ITEM, 0, 0);
			ghost_blow(3, RBM_TOUCH, RBE_EAT_ITEM, 0, 0);

			break;
		}

		/* Ranger */
	case CLASS_RANGER:
	case CLASS_WARLOCK:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 2;

			/* Normal attacks (three) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(1, attack2, RBE_HURT, d1, d2);
			ghost_blow(2, attack1, RBE_HURT, d1, d2);

			break;
		}

		/* Paladin */
	case CLASS_PALADIN:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 2;

			/* Normal attacks (three) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(1, attack2, RBE_HURT, d1, d2);
			ghost_blow(2, attack1, RBE_HURT, d1, d2);
			break;
		}

	default:
		{
			/* Sometimes increase damage */
			if (lev >= 30) d2 = d2 * 2;

			/* Normal attacks (four) */
			ghost_blow(0, attack1, RBE_HURT, d1, d2);
			ghost_blow(1, attack2, RBE_HURT, d1, d2);
			ghost_blow(2, attack1, RBE_HURT, d1, d2);
			ghost_blow(3, attack2, RBE_HURT, d1, d2);

			break;
		}
	}
}



/*
 * Prepare the ghost -- method 2
 */
static void set_ghost_aux_2(void)
{
	monster_race *r_ptr = &r_info[max_r_idx - 1];

	int lev = r_ptr->level;

	int grace = ghost_race;

	cptr gr_name = rp_name + race_info[grace].title;


	/* The ghost is cold blooded */
	r_ptr->flags2 |= (RF2_COLD_BLOOD);

	/* The ghost is undead */
	r_ptr->flags3 |= (RF3_UNDEAD);

	/* The ghost is immune to poison */
	r_ptr->flags3 |= (RF3_IM_POIS);


	switch ((lev / 4) + randint(3))
	{
	case 1:
	case 2:
	case 3:
		{
			sprintf(r_name + r_ptr->name, "%s, the Skeleton %s", gb_name, gr_name);
			r_ptr->d_char = 's';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			r_ptr->flags3 |= (RF3_IM_COLD);
			if (grace == RACE_HALF_ORC) r_ptr->flags3 |= (RF3_ORC);
			if (grace == RACE_HALF_TROLL) r_ptr->flags3 |= (RF3_TROLL);
			if (grace == RACE_THUNDERLORD) r_ptr->flags3 |= (RF3_THUNDERLORD);
			r_ptr->ac = 26;
			r_ptr->speed = 110;

			ghost_blow(0, RBM_HIT, RBE_HURT, 2, 6);
			ghost_blow(1, RBM_HIT, RBE_HURT, 2, 6);

			break;
		}

	case 4:
	case 5:
		{
			sprintf(r_name + r_ptr->name, "%s, the Zombified %s", gb_name, gr_name);
			r_ptr->d_char = 'z';
			r_ptr->d_attr = TERM_L_DARK;
			r_ptr->flags1 |= (RF1_DROP_60 | RF1_DROP_90);
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			if (grace == RACE_HALF_ORC) r_ptr->flags3 |= (RF3_ORC);
			if (grace == RACE_HALF_TROLL) r_ptr->flags3 |= (RF3_TROLL);
			if (grace == RACE_THUNDERLORD) r_ptr->flags3 |= (RF3_THUNDERLORD);
			r_ptr->ac = 30;
			r_ptr->speed = 110;
			r_ptr->hside *= 2;

			ghost_blow(0, RBM_HIT, RBE_HURT, 2, 9);

			break;
		}

	case 6:
	case 7:
		{
			sprintf(r_name + r_ptr->name, "%s, the Mummified %s", gb_name, gr_name);
			r_ptr->d_char = 'z';
			r_ptr->d_attr = TERM_L_DARK;
			r_ptr->flags1 |= (RF1_DROP_1D2);
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			if (grace == RACE_HALF_ORC) r_ptr->flags3 |= (RF3_ORC);
			if (grace == RACE_HALF_TROLL) r_ptr->flags3 |= (RF3_TROLL);
			if (grace == RACE_THUNDERLORD) r_ptr->flags3 |= (RF3_THUNDERLORD);
			r_ptr->ac = 35;
			r_ptr->speed = 110;
			r_ptr->hside *= 2;
			r_ptr->mexp = (r_ptr->mexp * 3) / 2;

			ghost_blow(0, RBM_HIT, RBE_HURT, 3, 8);
			ghost_blow(1, RBM_HIT, RBE_HURT, 3, 8);
			ghost_blow(2, RBM_HIT, RBE_HURT, 3, 8);

			break;
		}

	case 8:
		{
			sprintf(r_name + r_ptr->name, "%s, the Poltergeist", gb_name);
			r_ptr->d_char = 'G';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_RAND_50 | RF1_RAND_25 | RF1_DROP_1D2);
			r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->ac = 20;
			r_ptr->speed = 130;
			r_ptr->mexp = (r_ptr->mexp * 3) / 2;

			ghost_blow(0, RBM_HIT, RBE_HURT, 2, 6);
			ghost_blow(1, RBM_HIT, RBE_HURT, 2, 6);
			ghost_blow(2, RBM_TOUCH, RBE_TERRIFY, 0, 0);
			ghost_blow(3, RBM_TOUCH, RBE_TERRIFY, 0, 0);

			break;
		}

	case 9:
	case 10:
		{
			sprintf(r_name + r_ptr->name, "%s, the Spirit", gb_name);
			r_ptr->d_char = 'G';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_DROP_1D2);
			r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->ac = 20;
			r_ptr->speed = 110;
			r_ptr->hside *= 2;
			r_ptr->mexp = r_ptr->mexp * 3;

			ghost_blow(0, RBM_TOUCH, RBE_LOSE_WIS, 2, 6);
			ghost_blow(1, RBM_TOUCH, RBE_LOSE_DEX, 2, 6);
			ghost_blow(2, RBM_HIT, RBE_HURT, 4, 6);
			ghost_blow(3, RBM_WAIL, RBE_TERRIFY, 0, 0);

			break;
		}

	case 11:
		{
			sprintf(r_name + r_ptr->name, "%s, the Ghost", gb_name);
			r_ptr->d_char = 'G';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_DROP_1D2);
			r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->flags5 |= (RF5_BLIND | RF5_HOLD | RF5_DRAIN_MANA);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 15;
			r_ptr->ac = 40;
			r_ptr->speed = 120;
			r_ptr->hside *= 2;
			r_ptr->mexp = (r_ptr->mexp * 7) / 2;

			ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
			ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
			ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 2, 6);
			ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 2, 6);

			break;
		}

	case 12:
		{
			sprintf(r_name + r_ptr->name, "%s, the Vampire", gb_name);
			r_ptr->d_char = 'V';
			r_ptr->d_attr = TERM_VIOLET;
			r_ptr->flags1 |= (RF1_DROP_2D2);
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			r_ptr->flags3 |= (RF3_HURT_LITE);
			r_ptr->flags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_2);
			r_ptr->flags6 |= (RF6_TELE_TO);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
			r_ptr->ac = 40;
			r_ptr->speed = 110;
			r_ptr->hside *= 3;
			r_ptr->mexp = r_ptr->mexp * 3;

			ghost_blow(0, RBM_HIT, RBE_HURT, 5, 8);
			ghost_blow(1, RBM_HIT, RBE_HURT, 5, 8);
			ghost_blow(2, RBM_BITE, RBE_EXP_40, 0, 0);

			break;
		}

	case 13:
		{
			sprintf(r_name + r_ptr->name, "%s, the Wraith", gb_name);
			r_ptr->d_char = 'W';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_4D2);
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			r_ptr->flags3 |= (RF3_IM_COLD | RF3_HURT_LITE);
			r_ptr->flags5 |= (RF5_BLIND | RF5_SCARE | RF5_HOLD);
			r_ptr->flags5 |= (RF5_CAUSE_3 | RF5_BO_NETH);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 7;
			r_ptr->ac = 60;
			r_ptr->speed = 120;
			r_ptr->hside *= 3;
			r_ptr->mexp = r_ptr->mexp * 5;

			ghost_blow(0, RBM_HIT, RBE_HURT, 6, 8);
			ghost_blow(1, RBM_HIT, RBE_HURT, 6, 8);
			ghost_blow(2, RBM_TOUCH, RBE_EXP_20, 0, 0);

			break;
		}

	case 14:
		{
			sprintf(r_name + r_ptr->name, "%s, the Vampire Lord", gb_name);
			r_ptr->d_char = 'V';
			r_ptr->d_attr = TERM_BLUE;
			r_ptr->flags1 |= (RF1_DROP_1D2 | RF1_DROP_GREAT);
			r_ptr->flags2 |= (RF2_OPEN_DOOR | RF2_BASH_DOOR);
			r_ptr->flags3 |= (RF3_HURT_LITE);
			r_ptr->flags5 |= (RF5_SCARE | RF5_HOLD | RF5_CAUSE_3 | RF5_BO_NETH);
			r_ptr->flags6 |= (RF6_TELE_TO);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 8;
			r_ptr->ac = 80;
			r_ptr->speed = 110;
			r_ptr->hside *= 2;
			r_ptr->hdice *= 2;
			r_ptr->mexp = r_ptr->mexp * 20;

			ghost_blow(0, RBM_HIT, RBE_HURT, 6, 8);
			ghost_blow(1, RBM_HIT, RBE_HURT, 6, 8);
			ghost_blow(2, RBM_HIT, RBE_HURT, 6, 8);
			ghost_blow(3, RBM_BITE, RBE_EXP_80, 0, 0);

			break;
		}

	case 15:
		{
			sprintf(r_name + r_ptr->name, "%s, the Ghost", gb_name);
			r_ptr->d_char = 'G';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_GREAT);
			r_ptr->flags2 |= (RF2_INVISIBLE | RF2_PASS_WALL);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->flags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_DRAIN_MANA);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 5;
			r_ptr->ac = 90;
			r_ptr->speed = 130;
			r_ptr->hside *= 3;
			r_ptr->mexp = r_ptr->mexp * 20;

			ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
			ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
			ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 2, 6);
			ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 2, 6);

			break;
		}

	case 17:
		{
			sprintf(r_name + r_ptr->name, "%s, the Lich", gb_name);
			r_ptr->d_char = 'L';
			r_ptr->d_attr = TERM_ORANGE;
			r_ptr->flags1 |= (RF1_DROP_2D2 | RF1_DROP_1D2 | RF1_DROP_GREAT);
			r_ptr->flags2 |= (RF2_SMART | RF2_OPEN_DOOR | RF2_BASH_DOOR);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->flags5 |= (RF5_BLIND | RF5_SCARE | RF5_CONF | RF5_HOLD);
			r_ptr->flags5 |= (RF5_DRAIN_MANA | RF5_BA_FIRE | RF5_BA_COLD);
			r_ptr->flags5 |= (RF5_CAUSE_3 | RF5_CAUSE_4 | RF5_BRAIN_SMASH);
			r_ptr->flags6 |= (RF6_BLINK | RF6_TPORT | RF6_TELE_TO | RF6_S_UNDEAD);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 3;
			r_ptr->ac = 120;
			r_ptr->speed = 120;
			r_ptr->hside *= 3;
			r_ptr->hdice *= 2;
			r_ptr->mexp = r_ptr->mexp * 50;

			ghost_blow(0, RBM_TOUCH, RBE_LOSE_DEX, 4, 12);
			ghost_blow(1, RBM_TOUCH, RBE_LOSE_DEX, 4, 12);
			ghost_blow(2, RBM_TOUCH, RBE_UN_POWER, 0, 0);
			ghost_blow(3, RBM_TOUCH, RBE_EXP_40, 0, 0);

			break;
		}

	default:
		{
			sprintf(r_name + r_ptr->name, "%s, the Ghost", gb_name);
			r_ptr->d_char = 'G';
			r_ptr->d_attr = TERM_WHITE;
			r_ptr->flags1 |= (RF1_DROP_1D2 | RF1_DROP_2D2 | RF1_DROP_GREAT);
			r_ptr->flags2 |= (RF2_SMART | RF2_INVISIBLE | RF2_PASS_WALL);
			r_ptr->flags3 |= (RF3_IM_COLD);
			r_ptr->flags5 |= (RF5_BLIND | RF5_CONF | RF5_HOLD | RF5_BRAIN_SMASH);
			r_ptr->flags5 |= (RF5_DRAIN_MANA | RF5_BA_NETH | RF5_BO_NETH);
			r_ptr->flags6 |= (RF6_TELE_TO | RF6_TELE_LEVEL);
			r_ptr->freq_inate = r_ptr->freq_spell = 100 / 2;
			r_ptr->ac = 130;
			r_ptr->speed = 130;
			r_ptr->hside *= 2;
			r_ptr->hdice *= 2;
			r_ptr->mexp = r_ptr->mexp * 30;

			ghost_blow(0, RBM_WAIL, RBE_TERRIFY, 0, 0);
			ghost_blow(1, RBM_TOUCH, RBE_EXP_20, 0, 0);
			ghost_blow(2, RBM_CLAW, RBE_LOSE_INT, 2, 6);
			ghost_blow(3, RBM_CLAW, RBE_LOSE_WIS, 2, 6);

			break;
		}
	}
}


/*
 * Hack -- Prepare the "ghost" race
 *
 * We are given a "name" of the form "Bob" (or "Bob, the xxx"), and
 * a race/class (by index), and a level (usually the dungeon level),
 * and a special "town" flag (which chooses the major ghost "type").
 *
 * Note that "town" ghosts are always level 1 to 50, and other ghosts
 * are always level 1 to 100 (or deeper?)
 *
 * Currently we save the current "ghost race info" in the savefile.
 * Note that ghosts from pre-2.7.7 savefiles are always ignored.
 *
 * Eventually we should probably save the ghost in such a way as
 * to allow it to be "re-extracted" from a small amount of info,
 * such as the "base name", the "race", the "class", the base "hp",
 * the "level", the "town" flag, and the "random seed".  This would
 * make the savefile impervious to changes in the race format.
 *
 * Thus we would need to save "pn", "hp", "gr", "gc", and "lev",
 * plus the "town" flag, plus a random seed of some form.  Note that
 * we already save the "pn" value, followed by a "comma" and "title",
 * and we have the "lev" field as the actual ghost level.  But it is
 * probably best to ignore this storage method for a few versions.
 *
 * We "could" extract the "hp" from the ghost name and current hp's.
 * We "could" extract the "town" flag from the ghost race symbol.
 *
 * Note that each new ghost needs a new "random seed".  And actually,
 * we do not really need a "full" random seed, we could just use a
 * random value from which random numbers can be extracted.  (?)
 */
static void set_ghost(cptr pname, int hp, int grace, int gclass, int lev, bool town)
{
	int i;

	monster_race *r_ptr = &r_info[max_r_idx - 1];

	/* Ghosts are too weak otherwise */
	hp *= 2;

	/* Extract the basic ghost name */
	strcpy(gb_name, pname);

	/* Find the first comma, or end of string */
	for (i = 0; (i < 16) && (gb_name[i]) && (gb_name[i] != ','); i++);

	/* Terminate the name */
	gb_name[i] = '\0';

	/* Force a name */
	if (!gb_name[1]) strcpy(gb_name, "Nobody");

	/* Capitalize the name */
	if (islower(gb_name[0])) gb_name[0] = toupper(gb_name[0]);


	/* Clear the normal flags */
	r_ptr->flags1 = r_ptr->flags2 = r_ptr->flags3 = r_ptr->flags7 = r_ptr->flags8 = r_ptr->flags9 = 0L;

	/* Clear the spell flags */
	r_ptr->flags4 = r_ptr->flags5 = r_ptr->flags6 = 0L;


	/* Clear the attacks */
	ghost_blow(0, 0, 0, 0, 0);
	ghost_blow(1, 0, 0, 0, 0);
	ghost_blow(2, 0, 0, 0, 0);
	ghost_blow(3, 0, 0, 0, 0);


	/* The ghost never sleeps */
	r_ptr->sleep = 0;

	/* The ghost is very attentive */
	r_ptr->aaf = 100;


	/* Save the level */
	r_ptr->level = lev;

	/* Extract the default experience */
	r_ptr->mexp = lev * 5 + 5;


	/* Hack -- Break up the hitpoints */
	for (i = 1; i * i < hp; i++);

	/* Extract the basic hit dice and sides */
	r_ptr->hdice = r_ptr->hside = i;


	/* Unique monster */
	r_ptr->flags1 |= (RF1_UNIQUE);

	/* Only carry good items */
	r_ptr->flags1 |= (RF1_ONLY_ITEM | RF1_DROP_GOOD);

	/* The ghost is always evil */
	r_ptr->flags3 |= (RF3_EVIL);

	/* Cannot be slept or confused */
	r_ptr->flags3 |= (RF3_NO_SLEEP | RF3_NO_CONF);

	/* All ghosts are undeads */
	r_ptr->flags3 |= RF3_UNDEAD;


	/* Save the race and class */
	ghost_race = grace;
	ghost_class = gclass;


	/* Prepare the ghost (method 1) */
	if (town)
	{
		/* Method 1 */
		set_ghost_aux_1();
	}

	/* Prepare the ghost (method 2) */
	else
	{
		/* Method 2 */
		set_ghost_aux_2();
	}
}
#endif


/*
 * Places a ghost somewhere.
 */
s16b place_ghost(void)
{
#if 0 /* DGDGDGDG */
	int y, x, hp, level, grace, gclass;

	monster_race *r_ptr = &r_info[max_r_idx - 1];

	FILE *fp;

	bool err = FALSE;
	bool town = FALSE;

	char name[100];
	char tmp[1024];

	/* Hack -- no ghosts in the town */
	if (!dun_level) return (FALSE);

	/* Already have a ghost */
	if (r_ptr->cur_num >= r_ptr->max_num)
	{
		return (FALSE);
	}

	/* Dungeon -- Use Dungeon Level */
	else
	{
		/* And even then, it only happens sometimes */
		if (14 > randint((dun_level / 2) + 11)) return (FALSE);

		/* Only a 45% chance */
		if (magik(45)) return (FALSE);

		/* Level is dungeon level */
		level = dun_level;
	}


	/* Choose a bones file */
	sprintf(tmp, "%s%sbone%03d.%03d", ANGBAND_DIR_BONE, PATH_SEP, dungeon_type, level);

	/* Grab permission */
	safe_setuid_grab();

	/* Open the bones file */
	fp = my_fopen(tmp, "r");

	/* Drop permission */
	safe_setuid_drop();

	/* No bones file to use */
	if (!fp) return (FALSE);

	/* Scan the file */
	err = (fscanf(fp, "%[^\n]\n%d\n%d\n%d", name, &hp, &grace, &gclass) != 4);


	/* Close the file */
	fclose(fp);

	/* Previously, the bone file would now be deleted. The new
	 * method is to only remove the bone file when the ghost is
	 * destroyed. This means that failing to kill a ghost will
	 * not lose it permenently -TM-
	 * fd_kill(tmp); */

	/* Catch errors */
	if (err)
	{
		msg_print("Warning -- deleted corrupt 'ghost' file!");
		return (FALSE);
	}

	/* Create "town" flag */
	/* TM- What is this? Previously, if the player and dungeon levels
	 * were equal then a 'town' ghost was created. I can't see why
	 * 'town'. They are simply ghosts with abilities determined by
	 * previous class. Currently we just pick between the two.
	 * WAS: if (level == p_ptr->lev) town = TRUE;
	 */
	if (!rand_int(2)) town = TRUE;

	/* Set up the ghost */
	set_ghost(name, hp, grace, gclass, level, town);


	/* Hack -- pick a nice (far away) location */
	while (1)
	{

		/* Pick a location */
		y = randint(cur_hgt - 2);
		x = randint(cur_wid - 2);

		/* Require "naked" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Accept far away grids */
		if (distance(p_ptr->py, p_ptr->px, y, x) > MAX_SIGHT + 5) break;
	}


	/*** Place the Ghost by Hand (so no-one else does it accidentally) ***/

	r_ptr->cur_num = 0;
	r_ptr->max_num = 1;

	if (!place_monster_one(y, x, max_r_idx - 1, 0, FALSE, MSTATUS_ENEMY))
	{
		return FALSE;
	}

	/* Make sure it looks right */
	r_ptr->x_attr = r_ptr->d_attr;
	r_ptr->x_char = r_ptr->d_char;
	return TRUE;
#else 
	return (FALSE);
#endif
}
