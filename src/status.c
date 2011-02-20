/* File status.c */

/* Purpose: Status information */

/* Written by Pat Gunn <qc@apk.net> for ToME
 * This file is released into the public domain
 */

/* Throughout this file, I make use of 2-d arrays called flag_arr.
 * I fill them out with esp as the 0th element because then the second
 * index is equal to the f-number that they are in the attributes, and
 * that makes it more intuitive to use. I do need to fill them out in an
 * odd order, but that's all abstracted nicely away. The 6th element is used
 * to mark if the rest of the array is filled, that is, if there's an object
 * there.
 */

#include "angband.h"

static void row_trival(char*, s16b, u32b, s16b, u32b, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static void row_bival(char*, s16b, u32b, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static void row_npval(char*, s16b, u32b, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static void statline(char*, int, u32b, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static void row_hd_bon(int, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static void row_count(char*, s16b, u32b, int, s16b, u32b, int, s16b, u32b, int, s16b, u32b, int, int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
static int row_x_start = 0;

static void status_count(s32b val1, int v1, s32b val2, int v2, s32b val3, int v3, s32b val4, int v4, byte ypos, byte xpos);
static void status_trival(s32b, s32b, byte, byte);
static void status_bival(s32b, byte, byte);
static void status_numeric(s32b, byte, byte);
static void status_curses(void);
static void status_companion(void);
void status_sight(void);
void status_attr(void);
void status_combat(void);
void status_main(void);
void status_move(void);
void status_item(void);
void az_line(int, u32b[INVEN_TOTAL - INVEN_WIELD + 2][7]);
#define STATNM_LENGTH 		11
#define SL_LENGTH 		11

#define INVEN_PLAYER            (INVEN_TOTAL - INVEN_WIELD + 1)

void status_attr(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 0;
	char c;

	clear_from(0);
	c_put_str(TERM_L_BLUE, "Statistics", yo++, 1);
	yo += 2;
	az_line(SL_LENGTH, flag_arr);
	statline("Str", A_STR, TR1_STR, yo++, flag_arr);
	statline("Int", A_INT, TR1_INT, yo++, flag_arr);
	statline("Wis", A_INT, TR1_WIS, yo++, flag_arr);
	statline("Con", A_CON, TR1_CON, yo++, flag_arr);
	statline("Dex", A_DEX, TR1_DEX, yo++, flag_arr);
	statline("Chr", A_CHR, TR1_CHR, yo++, flag_arr);
	row_npval("Luck", 5, TR5_LUCK, yo++, flag_arr);
	yo++;
	row_npval("Life", 2, TR2_LIFE, yo++, flag_arr);
	row_npval("Mana", 1, TR1_MANA, yo++, flag_arr);


	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		c = inkey();
		if (c == ESCAPE) break;
	}
}

void status_move(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;
	clear_from(0);
	c_put_str(TERM_L_BLUE, "Movement", 0, 1);
	az_line(STATNM_LENGTH, flag_arr);

	row_trival("Fly/Lev", 4, TR4_FLY, 3, TR3_FEATHER, yo++, flag_arr);
	row_bival("Climb", 4, TR4_CLIMB, yo++, flag_arr);
	row_npval("Dig", 1, TR1_TUNNEL, yo++, flag_arr);
	row_npval("Speed", 1, TR1_SPEED, yo++, flag_arr);
	row_bival("Wraith", 3, TR3_WRAITH, yo++, flag_arr);
	yo++;
	row_npval("Stealth", 1, TR1_STEALTH, yo++, flag_arr);
	row_bival("Telep", 3, TR3_TELEPORT, yo++, flag_arr);

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;
		c = inkey();
		switch (c)
		{
		case ESCAPE:
			{
				loop_exit = TRUE;
			}
		}
		if (loop_exit)
		{
			break;
		}
	}
}

void status_sight(void)
/* Tell player about ESP, infravision, auto-id, see invis, and similar */
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;
	clear_from(0);
	c_put_str(TERM_L_BLUE, "Sight", 0, 1);
	az_line(STATNM_LENGTH, flag_arr);

	row_bival("SeeInvis", 3, TR3_SEE_INVIS, yo++, flag_arr);
	row_npval("Invis", 2, TR2_INVIS, yo++, flag_arr);
	row_npval("Infra", 1, TR1_INFRA, yo++, flag_arr);
	row_npval("Search", 1, TR1_SEARCH, yo++, flag_arr);
	row_bival("AutoID", 4, TR4_AUTO_ID, yo++, flag_arr);
	row_count("Light", 3, TR3_LITE1, 1, 4, TR4_LITE2, 2, 4, TR4_LITE3, 3, 0, 0, 0, yo++, flag_arr);
	row_bival("Full ESP", 0, ESP_ALL, yo++, flag_arr);
	row_bival("Orc  ESP", 0, ESP_ORC, yo++, flag_arr);
	row_bival("Trol ESP", 0, ESP_TROLL, yo++, flag_arr);
	row_bival("Drag ESP", 0, ESP_DRAGON, yo++, flag_arr);
	row_bival("GiantESP", 0, ESP_GIANT, yo++, flag_arr);
	row_bival("DemonESP", 0, ESP_DEMON, yo++, flag_arr);
	row_bival("Undd ESP", 0, ESP_UNDEAD, yo++, flag_arr);
	row_bival("Evil ESP", 0, ESP_EVIL, yo++, flag_arr);
	row_bival("Anim ESP", 0, ESP_ANIMAL, yo++, flag_arr);
	row_bival("Drid ESP", 0, ESP_THUNDERLORD, yo++, flag_arr);
	row_bival("Good ESP", 0, ESP_GOOD, yo++, flag_arr);
	row_bival("SpidrESP", 0, ESP_SPIDER, yo++, flag_arr);
	row_bival("NonlvESP", 0, ESP_NONLIVING, yo++, flag_arr);
	row_bival("Uniq ESP", 0, ESP_UNIQUE, yo++, flag_arr);

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;
		c = inkey();
		switch (c)
		{
		case ESCAPE:
			{
				loop_exit = TRUE;
			}
		}
		if (loop_exit)
		{
			break;
		}
	}
}

void status_item(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;
	clear_from(0);
	c_put_str(TERM_L_BLUE, "Misc", 0, 1);

	row_x_start = 40;
	az_line(STATNM_LENGTH + row_x_start, flag_arr);
	row_bival("Blessed", 3, TR3_BLESSED, yo++, flag_arr);
	row_bival("Activate", 3, TR3_ACTIVATE, yo++, flag_arr);
	row_bival("EasyKnow", 3, TR3_EASY_KNOW, yo++, flag_arr);
	row_bival("HideType", 3, TR3_HIDE_TYPE, yo++, flag_arr);
	yo++;
	row_bival("SafeAcid", 3, TR3_IGNORE_ACID, yo++, flag_arr);
	row_bival("SafeElec", 3, TR3_IGNORE_ELEC, yo++, flag_arr);
	row_bival("SafeFire", 3, TR3_IGNORE_FIRE, yo++, flag_arr);
	row_bival("SafeCold", 3, TR3_IGNORE_COLD, yo++, flag_arr);
	row_bival("ResMorgul", 5, TR5_RES_MORGUL, yo++, flag_arr);

	yo = 3;
	row_x_start = 0;
	az_line(STATNM_LENGTH, flag_arr);
	row_bival("Sh.fire", 3, TR3_SH_FIRE, yo++, flag_arr);
	row_bival("Sh.elec", 3, TR3_SH_ELEC, yo++, flag_arr);
	row_bival("Regen", 3, TR3_REGEN, yo++, flag_arr);
	row_bival("SlowDigest", 3, TR3_SLOW_DIGEST, yo++, flag_arr);
	row_bival("Precog", 4, TR4_PRECOGNITION, yo++, flag_arr);
	row_bival("Auto.Id", 4, TR4_AUTO_ID, yo++, flag_arr);
	row_bival("Spell.In", 5, TR5_SPELL_CONTAIN, yo++, flag_arr);

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;
		c = inkey();
		switch (c)
		{
		case ESCAPE:
			loop_exit = TRUE;
		}
		if (loop_exit)
		{
			break;
		}
	}
}

void status_combat(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;
	clear_from(0);
	c_put_str(TERM_L_BLUE, "Combat", 0, 1);
	az_line(STATNM_LENGTH, flag_arr);

	row_npval("Spell", 1, TR1_SPELL, yo++, flag_arr);
	row_npval("Blows", 1, TR1_BLOWS, yo++, flag_arr);
	row_npval("Crits", 5, TR5_CRIT, yo++, flag_arr);
	row_npval("Ammo_Mgt", 3, TR3_XTRA_MIGHT, yo++, flag_arr);
	row_npval("Ammo_Sht", 3, TR3_XTRA_SHOTS, yo++, flag_arr);
	row_bival("Vorpal", 1, TR1_VORPAL, yo++, flag_arr);
	row_bival("Quake", 1, TR1_IMPACT, yo++, flag_arr);
	row_bival("Chaotic", 1, TR1_CHAOTIC, yo++, flag_arr);
	row_bival("Vampiric", 1, TR1_VAMPIRIC, yo++, flag_arr);
	row_bival("Poison", 1, TR1_BRAND_POIS, yo++, flag_arr);
	row_bival("Acidic", 1, TR1_BRAND_ACID, yo++, flag_arr);
	row_bival("Shocks", 1, TR1_BRAND_ELEC, yo++, flag_arr);
	row_bival("Burns", 1, TR1_BRAND_FIRE, yo++, flag_arr);
	row_bival("Chills", 1, TR1_BRAND_COLD, yo++, flag_arr);
	row_bival("Wound", 5, TR5_WOUNDING, yo++, flag_arr);

	row_x_start = 40;
	yo = 3;
	az_line(row_x_start + STATNM_LENGTH, flag_arr);
	row_bival("No.Blow", 4, TR4_NEVER_BLOW, yo++, flag_arr);
	row_trival("S/K Undd", 1, TR1_SLAY_UNDEAD, 5, TR5_KILL_UNDEAD, yo++, flag_arr);
	row_trival("S/K Dmn", 1, TR1_SLAY_DEMON, 5, TR5_KILL_DEMON, yo++, flag_arr);
	row_trival("S/K Drag", 1, TR1_SLAY_DRAGON, 1, TR1_KILL_DRAGON, yo++, flag_arr);
	row_bival("Sl.Orc", 1, TR1_SLAY_ORC, yo++, flag_arr);
	row_bival("Sl.Troll", 1, TR1_SLAY_TROLL, yo++, flag_arr);
	row_bival("Sl.Giant", 1, TR1_SLAY_GIANT, yo++, flag_arr);
	row_bival("Sl.Evil", 1, TR1_SLAY_EVIL, yo++, flag_arr);
	row_bival("Sl.Animal", 1, TR1_SLAY_ANIMAL, yo++, flag_arr);
	row_hd_bon(0, yo++, flag_arr);
	row_hd_bon(1, yo++, flag_arr);
	row_x_start = 0;

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;
		c = inkey();
		switch (c)
		{
		case ESCAPE:
			loop_exit = TRUE;
		}
		if (loop_exit)
		{
			break;
		}
	}
}

void status_curses(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;

	clear_from(0);
	c_put_str(TERM_L_BLUE, "Curses", 0, 1);
	az_line(STATNM_LENGTH, flag_arr);

	row_trival("Hvy/Nrm", 3, TR3_HEAVY_CURSE, 3, TR3_CURSED, yo++, flag_arr);
	row_bival("Perma", 3, TR3_PERMA_CURSE, yo++, flag_arr);
	row_trival("DG/Ty", 4, TR4_DG_CURSE, 3, TR3_TY_CURSE, yo++, flag_arr);
	row_trival("Prm/Auto", 3, TR3_PERMA_CURSE, 3, TR3_AUTO_CURSE, yo++, flag_arr);
	row_bival("NoDrop", 4, TR4_CURSE_NO_DROP, yo++, flag_arr);
	yo++;
	row_bival("B.Breath", 4, TR4_BLACK_BREATH, yo++, flag_arr);
	row_bival("Dr.Exp", 3, TR3_DRAIN_EXP, yo++, flag_arr);
	row_bival("Dr.Mana", 5, TR5_DRAIN_MANA, yo++, flag_arr);
	row_bival("Dr.HP", 5, TR5_DRAIN_HP, yo++, flag_arr);
	row_bival("No Hit", 4, TR4_NEVER_BLOW, yo++, flag_arr);
	row_bival("NoTelep", 3, TR3_NO_TELE, yo++, flag_arr);
	row_bival("NoMagic", 3, TR3_NO_MAGIC, yo++, flag_arr);
	row_bival("Aggrav", 3, TR3_AGGRAVATE, yo++, flag_arr);
	row_bival("Clone", 4, TR4_CLONE, yo++, flag_arr);
	row_bival("Temp", 5, TR5_TEMPORARY, yo++, flag_arr);
	yo++;
	row_count("Antimagic", 4, TR4_ANTIMAGIC_50, 5, 4, TR4_ANTIMAGIC_30, 3, 4, TR4_ANTIMAGIC_20, 2, 4, TR4_ANTIMAGIC_10, 1, yo++, flag_arr);

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;

		c = inkey();
		switch (c)
		{
		case ESCAPE:
			{
				loop_exit = TRUE;
			}
		}
		if (loop_exit == TRUE)
		{
			break;
		}
	}
}

void status_res(void)
{
	u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7];
	int yo = 3;

	clear_from(0);
	c_put_str(TERM_L_BLUE, "Resistances", 0, 1);
	az_line(STATNM_LENGTH, flag_arr);

	row_trival("Fire", 2, TR2_IM_FIRE, 2, TR2_RES_FIRE, yo++, flag_arr);
	row_trival("Cold", 2, TR2_IM_COLD, 2, TR2_RES_COLD, yo++, flag_arr);
	row_trival("Acid", 2, TR2_IM_ACID, 2, TR2_RES_ACID, yo++, flag_arr);
	row_trival("Lightning", 2, TR2_IM_ELEC, 2, TR2_RES_ELEC, yo++, flag_arr);
	row_bival("Poison", 2, TR2_RES_POIS, yo++, flag_arr);
	row_bival("Lite", 2, TR2_RES_LITE, yo++, flag_arr);
	row_bival("Dark", 2, TR2_RES_DARK, yo++, flag_arr);
	row_bival("Sound", 2, TR2_RES_SOUND, yo++, flag_arr);
	row_bival("Shards", 2, TR2_RES_SHARDS, yo++, flag_arr);
	row_trival("Nether", 4, TR4_IM_NETHER, 2, TR2_RES_NETHER, yo++, flag_arr);
	row_bival("Nexus", 2, TR2_RES_NEXUS, yo++, flag_arr);
	row_bival("Chaos", 2, TR2_RES_CHAOS, yo++, flag_arr);
	row_bival("Disen.", 2, TR2_RES_DISEN, yo++, flag_arr);
	row_bival("Confusion", 2, TR2_RES_CONF, yo++, flag_arr);
	row_bival("Blindness", 2, TR2_RES_BLIND, yo++, flag_arr);
	row_bival("Fear", 2, TR2_RES_FEAR, yo++, flag_arr);
	row_bival("Free Act", 2, TR2_FREE_ACT, yo++, flag_arr);
	row_bival("Reflect", 2, TR2_REFLECT, yo++, flag_arr);
	row_bival("Hold Life", 2, TR2_HOLD_LIFE, yo++, flag_arr);

	c_put_str(TERM_WHITE, "Press ESC to continue", 23, 0);
	Term_fresh();
	while (1)
	{
		bool_ loop_exit = FALSE;
		char c;

		c = inkey();
		switch (c)
		{
		case ESCAPE:
			{
				loop_exit = TRUE;
			}
		}
		if (loop_exit == TRUE)
		{
			break;
		}
	}
}

void status_main(void)
{
	int do_quit = 0;
	char c;

	character_icky = TRUE;
	Term_save();
	while (1)
	{
		clear_from(0);
		c_put_str(TERM_WHITE, format("%s Character Status screen", game_module), 0, 10);
		c_put_str(TERM_WHITE, "1) Statistics", 2, 5);
		c_put_str(TERM_WHITE, "2) Movement", 3, 5);
		c_put_str(TERM_WHITE, "3) Combat", 4, 5);
		c_put_str(TERM_WHITE, "4) Resistances", 5, 5);
		c_put_str(TERM_WHITE, "5) Misc", 6, 5);
		c_put_str(TERM_WHITE, "6) Curses", 7, 5);
		c_put_str(TERM_WHITE, "7) Sight", 8, 5);
		c_put_str(TERM_WHITE, "8) Companions", 9, 5);
		c_put_str(TERM_RED, "Press 'q' to Quit", 23, 5);
		c = inkey();
		switch (c)
		{
		case '1':
			status_attr();
			break;
		case '2':
			status_move();
			break;
		case '3':
			status_combat();
			break;
		case '4':
			status_res();
			break;
		case '5':
			status_item();
			break;
		case '6':
			status_curses();
			break;
		case '7':
			status_sight();
			break;
		case '8':
			status_companion();
			break;
		case 'q':
		case ESCAPE:
			do_quit = 1;  /* Schedule leaving the outer loop */
			break;
		}
		Term_fresh();
		if (do_quit) break;
	}
	Term_load();
	character_icky = FALSE;
	p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP);
	handle_stuff();
}

void az_line(int xo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
{
	int index = xo;  /* Leave room for description */
	int i;
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		if (p_ptr->inventory[i].k_idx) /* Wearing anything here? */
		{
			char cstrng[2];
			cstrng[0] = (i - INVEN_WIELD) + 'a';  /* Assumes ASCII */
			cstrng[1] = '\0';  /* terminate it */
			c_put_str(TERM_WHITE, cstrng, 2, index++);  /* Assumes ASCII */

			/* DGDGDGDG */
			/*                        object_flags_known(&inventory[i],*/
			object_flags(&p_ptr->inventory[i],  	/* Help me debug */
			             &flag_arr[i - INVEN_WIELD][1],  /* f1 */
			             &flag_arr[i - INVEN_WIELD][2],  /* f2 */
			             &flag_arr[i - INVEN_WIELD][3],  /* f3 */
			             &flag_arr[i - INVEN_WIELD][4],  /* f4 */
			             &flag_arr[i - INVEN_WIELD][5],  /* f5 */
			             &flag_arr[i - INVEN_WIELD][0]);   /* esp */
			flag_arr[i - INVEN_WIELD][6] = 1;  /* And mark it to display */
		}
		else flag_arr[i - INVEN_WIELD][6] = 0;  /* Otherwise don't display it */
	}
	c_put_str(TERM_WHITE, "@", 2, index++);
	player_flags(
	        &flag_arr[INVEN_PLAYER][1],  /* f1 */
	        &flag_arr[INVEN_PLAYER][2],  /* f2 */
	        &flag_arr[INVEN_PLAYER][3],  /* f3 */
	        &flag_arr[INVEN_PLAYER][4],  /* f4 */
	        &flag_arr[INVEN_PLAYER][5],  /* f5 */
	        &flag_arr[INVEN_PLAYER][0] /* esp */
	);
	flag_arr[INVEN_PLAYER][6] = 1;
}

static void status_trival(s32b val1, s32b val2, byte ypos, byte xpos)
{
	if (val1 != 0)
		c_put_str(TERM_L_BLUE, "*", ypos, xpos);
	else if (val2 != 0)
		c_put_str(TERM_L_BLUE, "+", ypos, xpos);
	else
		c_put_str(TERM_WHITE, ".", ypos, xpos);
}

static void status_bival(s32b val, byte ypos, byte xpos)
{
	if (val != 0)
		c_put_str(TERM_L_BLUE, "+", ypos, xpos);
	else
		c_put_str(TERM_WHITE, ".", ypos, xpos);
}

static void status_numeric(s32b val, byte ypos, byte xpos)
{
	u32b magnitude = ABS(val);
	int color = TERM_WHITE; /* default */
	char strnum[2];

	if (val<0) {
		color = TERM_RED;
	};
	if (val>0) {
		color = TERM_GREEN;
	};

	if (magnitude == 0) {
		sprintf(strnum, ".");
	} if (magnitude > 9) {
		sprintf(strnum, "*");
	} else {
		sprintf(strnum, "%lu", (unsigned long int) magnitude);
	}

	c_put_str(color, strnum, ypos, xpos);
}

static void status_count(s32b val1, int v1, s32b val2, int v2, s32b val3, int v3, s32b val4, int v4, byte ypos, byte xpos)
{
	int v = 0;

	if (val1 != 0) v += v1;
	if (val2 != 0) v += v2;
	if (val3 != 0) v += v3;
	if (val4 != 0) v += v4;

	status_numeric(v, ypos, xpos);
}

static void row_count(char* statname, s16b row1, u32b flag1, int v1, s16b row2, u32b flag2, int v2, s16b row3, u32b flag3, int v3, s16b row4, u32b flag4, int v4, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
{
	int i;
	int x = row_x_start;

	c_put_str(TERM_L_GREEN, statname, yo, row_x_start);

	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		if (flag_arr[i][6] == 1)
		{
			status_count((flag_arr[i][row1] & flag1), v1, (flag_arr[i][row2] & flag2), v2, (flag_arr[i][row3] & flag3), v3, (flag_arr[i][row4] & flag4), v4, yo, x + STATNM_LENGTH);
			x++;
		}
	}
}

static void row_trival(char* statname, s16b row, u32b flag, s16b row2, u32b flag2, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
{
	int i;
	int x = row_x_start;
	c_put_str(TERM_L_GREEN, statname, yo, row_x_start);
	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		if (flag_arr[i][6] == 1)
		{
			status_trival(
			        (flag_arr[i][row] & flag),
			        (flag_arr[i][row2] & flag2),
			        yo, x + STATNM_LENGTH);
			x++;
		}
	}
}

static void row_bival(char* statname, s16b row, u32b flag, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
{
	int i;
	int x = row_x_start;
	c_put_str(TERM_L_GREEN, statname, yo, row_x_start);
	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		if (flag_arr[i][6] == 1)
		{
			status_bival((flag_arr[i][row] & flag), yo, x + STATNM_LENGTH);
			x++;
		}
	}
}

static void row_npval(char* statname, s16b row, u32b flag, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
/* Displays nicely a pval-based status row */
{
	int i;
	int x = row_x_start;
	c_put_str(TERM_L_GREEN, statname, yo, row_x_start);
	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		if (flag_arr[i][6] == 1)
		{
			if (i == INVEN_PLAYER)
				/* Special case, player_flags */
				/* Players lack a pval, no way to calc value */
			{
				if (flag_arr[i][row] & flag)
					c_put_str(TERM_YELLOW, "*", yo, x + STATNM_LENGTH);
				else c_put_str(TERM_WHITE, ".", yo, x + STATNM_LENGTH);
				x++;
				continue;
			}
			if (flag_arr[i][row] & flag)
				status_numeric(p_ptr->inventory[i + INVEN_WIELD].pval, yo, x + STATNM_LENGTH);
			else
				c_put_str(TERM_WHITE, ".", yo, x + STATNM_LENGTH);
			x++;
		}
	}
}

static void statline(char* statname, int statidx, u32b flag, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
/* Displays a status row for a primary stat */
{
	int i;
	int x = row_x_start;
	char statstr[8];
	byte stat_color = TERM_L_RED;

	cnv_stat(p_ptr->stat_use[statidx], statstr);

	c_put_str(TERM_L_GREEN, statstr, yo, 4 + row_x_start);
	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		byte color = TERM_L_RED;

		if (flag_arr[i][6] == 1)
		{
			switch (statidx)
			{
			case A_STR:
				if (flag_arr[i][2] & TR2_SUST_STR)
					color = TERM_L_BLUE;
				break;
			case A_INT:
				if (flag_arr[i][2] & TR2_SUST_INT)
					color = TERM_L_BLUE;
				break;
			case A_WIS:
				if (flag_arr[i][2] & TR2_SUST_WIS)
					color = TERM_L_BLUE;
				break;
			case A_DEX:
				if (flag_arr[i][2] & TR2_SUST_DEX)
					color = TERM_L_BLUE;
				break;
			case A_CON:
				if (flag_arr[i][2] & TR2_SUST_CON)
					color = TERM_L_BLUE;
				break;
			case A_CHR:
				if (flag_arr[i][2] & TR2_SUST_CHR)
					color = TERM_L_BLUE;
				break;
			}

			if (i == INVEN_PLAYER ) /* Player flags */
			{
				if (flag_arr[i][1] & flag)
					c_put_str((color == TERM_L_RED) ? TERM_YELLOW : color, "*", yo, x + SL_LENGTH);
				else c_put_str((color == TERM_L_RED) ? TERM_WHITE : color, ".", yo, x + SL_LENGTH);
				x++;
				continue;
			}
			if (flag_arr[i][1] & flag)
				status_numeric(p_ptr->inventory[i + INVEN_WIELD].pval, yo, x + SL_LENGTH);
			else
				c_put_str((color == TERM_L_RED) ? TERM_WHITE : color, ".", yo, x + SL_LENGTH);

			if (color != TERM_L_RED)
				stat_color = color;

			x++;
		}
	}

	c_put_str(stat_color, statname, yo, row_x_start);
}

static void row_hd_bon(int which, int yo, u32b flag_arr[INVEN_TOTAL - INVEN_WIELD + 2][7])
/* To-hit/dmg modifiers, selected by 1st argument */
{
	int i;
	int x = row_x_start;
	if ((which != 0) && (which != 1)) return;
	c_put_str(TERM_L_GREEN, ((which == 0) ? "To-Hit" : "To-Dmg"), yo, row_x_start);
	for (i = 0; i < (INVEN_TOTAL - INVEN_WIELD + 2); i++)
	{
		if (flag_arr[i][6] == 1)
		{
			if (i == INVEN_PLAYER) /* Player? */
			{
				c_put_str(TERM_WHITE, ".", yo, x + STATNM_LENGTH);
				x++;
				continue;
			}
			if ( (which == 0) && (p_ptr->inventory[INVEN_WIELD + i].to_h != 0))
			{
				status_numeric(p_ptr->inventory[INVEN_WIELD + i].to_h, yo, x + STATNM_LENGTH);
				x++;
				continue;
			}
			if ( (which == 1) && (p_ptr->inventory[INVEN_WIELD + i].to_d != 0))
			{
				status_numeric(p_ptr->inventory[INVEN_WIELD + i].to_d, yo, x + STATNM_LENGTH);
				x++;
				continue;
			}
			c_put_str(TERM_WHITE, ".", yo, x + STATNM_LENGTH);
			x++;
		}
	}
}

static void status_companion(void)
{
	int i;

	FILE *fff;

	char file_name[1024];

	Term_clear();

	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	/* Calculate companions */
	/* Process the monsters (backwards) */
	for (i = m_max - 1; i >= 1; i--)
	{
		/* Access the monster */
		monster_type *m_ptr = &m_list[i];

		if (m_ptr->status == MSTATUS_COMPANION)
		{
			char m_name[80];
			int b, y = 0;

			/* Extract monster name */
			monster_desc(m_name, m_ptr, 0x80);

			fprintf(fff, "#####BCompanion: %s\n", m_name);

			fprintf(fff, "  Lev/Exp : [[[[[G%d / %ld]\n", m_ptr->level, (long int) m_ptr->exp);
			if (m_ptr->level < MONSTER_LEVEL_MAX) fprintf(fff, "  Next lvl: [[[[[G%ld]\n", (long int) MONSTER_EXP((s32b)m_ptr->level + 1));
			else fprintf(fff, "  Next lvl: [[[[[G****]\n");

			fprintf(fff, "  HP      : [[[[[G%ld / %ld]\n", (long int) m_ptr->hp, (long int) m_ptr->maxhp);
			fprintf(fff, "  AC      : [[[[[G%d]\n", m_ptr->ac);
			fprintf(fff, "  Speed   : [[[[[G%d]\n", m_ptr->mspeed - 110);

			for (b = 0; b < 4; b++)
			{
				if (!m_ptr->blow[b].d_dice) continue;
				if (!m_ptr->blow[b].d_side) continue;

				fprintf(fff, "  Blow %1d  : [[[[[G%dd%d]\n", y + 1, m_ptr->blow[b].d_dice, m_ptr->blow[b].d_side);
				y++;
			}

			fprintf(fff, "\n");
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Display the file contents */
	show_file(file_name, "Companion List", 0, 0);

	/* Remove the file */
	fd_kill(file_name);
}
