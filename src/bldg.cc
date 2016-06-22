/* File: bldg.c */

/*
 * Purpose: Building commands
 * Created by Ken Wigle for Kangband - a variant of Angband 2.8.3
 * -KMW-
 *
 * Rewritten for Kangband 2.8.3i using Kamband's version of
 * bldg.c as written by Ivan Tkatchev
 *
 * Changed for ZAngband by Robert Ruehlmann
 *
 * Heavily modified for ToME by DarkGod
 */

#include "cave_type.hpp"
#include "cmd3.hpp"
#include "files.hpp"
#include "hooks.hpp"
#include "hook_quest_finish_in.hpp"
#include "hook_quest_fail_in.hpp"
#include "hook_init_quest_in.hpp"
#include "mimic.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "owner_type.hpp"
#include "player_race_flag.hpp"
#include "player_type.hpp"
#include "q_library.hpp"
#include "q_fireprof.hpp"
#include "q_bounty.hpp"
#include "spells2.hpp"
#include "stats.hpp"
#include "store.hpp"
#include "store_action_type.hpp"
#include "store_info_type.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

/* remember building location */
static int building_loc = 0;


/*
 * A helper function for is_state
 */
static bool_ is_state_aux(store_type *s_ptr, int state)
{
	owner_type *ow_ptr = &ow_info[s_ptr->owner];


	/* Check race */
	if (ow_ptr->races[state][p_ptr->prace / 32] & (1 << p_ptr->prace))
	{
		return (TRUE);
	}

	/* Check class */
	if (ow_ptr->classes[state][p_ptr->prace / 32] & (1 << p_ptr->pclass))
	{
		return (TRUE);
	}

	/* All failed */
	return (FALSE);
}


/*
 * Test if the state accords with the player
 */
bool_ is_state(store_type *s_ptr, int state)
{
	if (state == STORE_NORMAL)
	{
		if (is_state_aux(s_ptr, STORE_LIKED)) return (FALSE);
		if (is_state_aux(s_ptr, STORE_HATED)) return (FALSE);
		return (TRUE);
	}

	else
	{
		return (is_state_aux(s_ptr, state));
	}
}


/*
 * Clear the building information
 */
static void clear_bldg(int min_row, int max_row)
{
	int i;


	for (i = min_row; i <= max_row; i++)
	{
		prt("", i, 0);
	}
}


/*
 * Display a building.
 */
void show_building(store_type *s_ptr)
{
	char buff[20];

	int i;

	byte action_color;

	char tmp_str[80];

	store_info_type *st_ptr = &st_info[s_ptr->st_idx];


	for (i = 0; i < 6; i++)
	{
		store_action_type *ba_ptr = &ba_info[st_ptr->actions[i]];

		if (ba_ptr->letter != '.')
		{
			if (ba_ptr->action_restr == 0)
			{
				if ((is_state(s_ptr, STORE_LIKED) && (ba_ptr->costs[STORE_LIKED] == 0)) ||
				                (is_state(s_ptr, STORE_HATED) && (ba_ptr->costs[STORE_HATED] == 0)) ||
				                (is_state(s_ptr, STORE_NORMAL) && (ba_ptr->costs[STORE_NORMAL] == 0)))
				{
					action_color = TERM_WHITE;
					buff[0] = '\0';
				}
				else if (is_state(s_ptr, STORE_LIKED))
				{
					action_color = TERM_L_GREEN;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_LIKED]);
				}
				else if (is_state(s_ptr, STORE_HATED))
				{
					action_color = TERM_RED;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_HATED]);
				}
				else
				{
					action_color = TERM_YELLOW;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_NORMAL]);
				}
			}
			else if (ba_ptr->action_restr == 1)
			{
				if ((is_state(s_ptr, STORE_LIKED) && (ba_ptr->costs[STORE_LIKED] == 0)) ||
				                (is_state(s_ptr, STORE_NORMAL) && (ba_ptr->costs[STORE_NORMAL] == 0)))
				{
					action_color = TERM_WHITE;
					buff[0] = '\0';
				}
				else if (is_state(s_ptr, STORE_LIKED))
				{
					action_color = TERM_L_GREEN;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_LIKED]);
				}
				else if (is_state(s_ptr, STORE_HATED))
				{
					action_color = TERM_L_DARK;
					strnfmt(buff, 20, "(closed)");
				}
				else
				{
					action_color = TERM_YELLOW;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_NORMAL]);
				}
			}
			else
			{
				if (is_state(s_ptr, STORE_LIKED) && (ba_ptr->costs[STORE_LIKED] == 0))
				{
					action_color = TERM_WHITE;
					buff[0] = '\0';
				}
				else if (is_state(s_ptr, STORE_LIKED))
				{
					action_color = TERM_L_GREEN;
					strnfmt(buff, 20, "(%dgp)", ba_ptr->costs[STORE_LIKED]);
				}
				else
				{
					action_color = TERM_L_DARK;
					strnfmt(buff, 20, "(closed)");
				}
			}

			strnfmt(tmp_str, 80, " %c", ba_ptr->letter);
			c_put_str(TERM_YELLOW, tmp_str, 21 + (i / 2), 17 + (30 * (i % 2)));

			strnfmt(tmp_str, 80, ") %s %s", ba_ptr->name, buff);
			c_put_str(action_color, tmp_str, 21 + (i / 2), 2 + 17 + (30 * (i % 2)));
		}
	}
}


/*
 * display fruit for dice slots
 */
static void display_fruit(int row, int col, int fruit)
{
	switch (fruit)
	{
	case 0:  /* lemon */
		{
			c_put_str(TERM_YELLOW, "   ####.", row, col);
			c_put_str(TERM_YELLOW, "  #    #", row + 1, col);
			c_put_str(TERM_YELLOW, " #     #", row + 2, col);
			c_put_str(TERM_YELLOW, "#      #", row + 3, col);
			c_put_str(TERM_YELLOW, "#      #", row + 4, col);
			c_put_str(TERM_YELLOW, "#     # ", row + 5, col);
			c_put_str(TERM_YELLOW, "#    #  ", row + 6, col);
			c_put_str(TERM_YELLOW, ".####   ", row + 7, col);
			prt(" Lemon  ", row + 8, col);

			break;
		}

	case 1:  /* orange */
		{
			c_put_str(TERM_ORANGE, "   ##   ", row, col);
			c_put_str(TERM_ORANGE, "  #..#  ", row + 1, col);
			c_put_str(TERM_ORANGE, " #....# ", row + 2, col);
			c_put_str(TERM_ORANGE, "#......#", row + 3, col);
			c_put_str(TERM_ORANGE, "#......#", row + 4, col);
			c_put_str(TERM_ORANGE, " #....# ", row + 5, col);
			c_put_str(TERM_ORANGE, "  #..#  ", row + 6, col);
			c_put_str(TERM_ORANGE, "   ##   ", row + 7, col);
			prt(" Orange ", row + 8, col);

			break;
		}

	case 2:  /* sword */
		{
			c_put_str(TERM_SLATE, "   /\\   ", row, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 1, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 2, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 3, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 4, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 5, col);
			c_put_str(TERM_UMBER, " ###### ", row + 6, col);
			c_put_str(TERM_UMBER, "   ##   ", row + 7, col);
			prt(" Sword  ", row + 8, col);

			break;
		}

	case 3:  /* shield */
		{
			c_put_str(TERM_SLATE, " ###### ", row, col);
			c_put_str(TERM_SLATE, "#      #", row + 1, col);
			c_put_str(TERM_SLATE, "# ++++ #", row + 2, col);
			c_put_str(TERM_SLATE, "# +==+ #", row + 3, col);
			c_put_str(TERM_SLATE, "#  ++  #", row + 4, col);
			c_put_str(TERM_SLATE, " #    # ", row + 5, col);
			c_put_str(TERM_SLATE, "  #  #  ", row + 6, col);
			c_put_str(TERM_SLATE, "   ##   ", row + 7, col);
			prt(" Shield ", row + 8, col);

			break;
		}

	case 4:  /* plum */
		{
			c_put_str(TERM_VIOLET, "   ##   ", row, col);
			c_put_str(TERM_VIOLET, " ###### ", row + 1, col);
			c_put_str(TERM_VIOLET, "########", row + 2, col);
			c_put_str(TERM_VIOLET, "########", row + 3, col);
			c_put_str(TERM_VIOLET, "########", row + 4, col);
			c_put_str(TERM_VIOLET, " ###### ", row + 5, col);
			c_put_str(TERM_VIOLET, "  ####  ", row + 6, col);
			c_put_str(TERM_VIOLET, "   ##   ", row + 7, col);
			prt("  Plum  ", row + 8, col);

			break;
		}

	case 5:  /* cherry */
		{
			c_put_str(TERM_RED, "      ##", row, col);
			c_put_str(TERM_RED, "   ###  ", row + 1, col);
			c_put_str(TERM_RED, "  #..#  ", row + 2, col);
			c_put_str(TERM_RED, "  #..#  ", row + 3, col);
			c_put_str(TERM_RED, " ###### ", row + 4, col);
			c_put_str(TERM_RED, "#..##..#", row + 5, col);
			c_put_str(TERM_RED, "#..##..#", row + 6, col);
			c_put_str(TERM_RED, " ##  ## ", row + 7, col);
			prt(" Cherry ", row + 8, col);

			break;
		}
	}
}


/*
 * gamble_comm
 */
static bool_ gamble_comm(int cmd)
{
	int roll1, roll2, roll3, choice, odds, win;

	s32b wager;

	s32b maxbet;

	s32b oldgold;

	static const char *fruit[6] =
	        {"Lemon", "Orange", "Sword", "Shield", "Plum", "Cherry"
	        };

	char out_val[160], tmp_str[80], again;

	cptr p;


	screen_save();

	if (cmd == BACT_GAMBLE_RULES)
	{
		/* Peruse the gambling help file */
		show_file("gambling.txt", NULL);
	}
	else
	{
		clear_bldg(5, 23);

		/* Set maximum bet */
		if (p_ptr->lev < 10)
			maxbet = (p_ptr->lev * 100);
		else
			maxbet = (p_ptr->lev * 1000);

		/* Get the wager */
		strcpy(out_val, "");
		strnfmt(tmp_str, 80, "Your wager (1-%ld) ? ", maxbet);
		get_string(tmp_str, out_val, 32);

		/* Strip spaces */
		for (p = out_val; *p == ' '; p++);

		wager = atol(p);

		if (wager > p_ptr->au)
		{
			msg_print("Hey! You don't have the gold - get out of here!");
			msg_print(NULL);
			screen_load();
			return (FALSE);
		}
		else if (wager > maxbet)
		{
			msg_format("I'll take $%ld of that. Keep the rest.", maxbet);
			wager = maxbet;
		}
		else if (wager < 1)
		{
			msg_print("Ok, we'll start with $1.");

			wager = 1;
		}
		msg_print(NULL);
		win = FALSE;
		odds = 0;
		oldgold = p_ptr->au;

		strnfmt(tmp_str, 80, "Gold before game: %9ld", oldgold);
		prt(tmp_str, 20, 2);

		strnfmt(tmp_str, 80, "Current Wager:    %9ld", wager);
		prt(tmp_str, 21, 2);

		do
		{
			switch (cmd)
			{
			case BACT_IN_BETWEEN:  /* Game of In-Between */
				{
					c_put_str(TERM_GREEN, "In Between", 5, 2);
					odds = 3;
					win = FALSE;
					roll1 = randint(10);
					roll2 = randint(10);
					choice = randint(10);
					strnfmt(tmp_str, 80, "Black die: %d       Black Die: %d",
					        roll1, roll2);
					prt(tmp_str, 8, 3);
					strnfmt(tmp_str, 80, "Red die: %d", choice);
					prt(tmp_str, 11, 14);
					if (((choice > roll1) && (choice < roll2)) ||
					                ((choice < roll1) && (choice > roll2)))
						win = TRUE;

					break;
				}
			case BACT_CRAPS:   /* Game of Craps */
				{
					c_put_str(TERM_GREEN, "Craps", 5, 2);
					win = 3;
					odds = 1;
					roll1 = randint(6);
					roll2 = randint(6);
					roll3 = roll1 + roll2;
					choice = roll3;
					strnfmt(tmp_str, 80, "First roll: %d %d    Total: %d", roll1,
					        roll2, roll3);
					prt(tmp_str, 7, 5);
					if ((roll3 == 7) || (roll3 == 11))
						win = TRUE;
					else if ((roll3 == 2) || (roll3 == 3) || (roll3 == 12))
						win = FALSE;
					else
					{
						do
						{
							msg_print("Hit any key to roll again");
							msg_print(NULL);
							roll1 = randint(6);
							roll2 = randint(6);
							roll3 = roll1 + roll2;

							strnfmt(tmp_str, 80, "Roll result: %d %d   Total:     %d",
							        roll1, roll2, roll3);
							prt(tmp_str, 8, 5);
							if (roll3 == choice)
								win = TRUE;
							else if (roll3 == 7)
								win = FALSE;
						}
						while ((win != TRUE) && (win != FALSE));
					}

					break;
				}

			case BACT_DICE_SLOTS:  /* The Dice Slots */
				{
					c_put_str(TERM_GREEN, "Dice Slots", 5, 2);
					win = FALSE;
					roll1 = randint(6);
					roll2 = randint(6);
					choice = randint(6);
					strnfmt(tmp_str, 80, "%s %s %s",
					        fruit[roll1 - 1], fruit[roll2 - 1],
					        fruit[choice - 1]);
					prt(tmp_str, 15, 37);
					prt("/--------------------------\\", 7, 2);
					prt("\\--------------------------/", 17, 2);
					display_fruit(8, 3, roll1 - 1);
					display_fruit(8, 12, roll2 - 1);
					display_fruit(8, 21, choice - 1);
					if ((roll1 == roll2) && (roll2 == choice))
					{
						win = TRUE;
						if (roll1 == 1)
							odds = 4;
						else if (roll1 == 2)
							odds = 6;
						else
							odds = roll1 * roll1;
					}
					else if ((roll1 == 6) && (roll2 == 6))
					{
						win = TRUE;
						odds = choice + 1;
					}

					break;
				}
			}

			if (win)
			{
				prt("YOU WON", 16, 37);
				p_ptr->au = p_ptr->au + (odds * wager);
				strnfmt(tmp_str, 80, "Payoff: %d", odds);
				prt(tmp_str, 17, 37);
			}
			else
			{
				prt("You Lost", 16, 37);
				p_ptr->au = p_ptr->au - wager;
				prt("", 17, 37);
			}
			strnfmt(tmp_str, 80, "Current Gold:     %9ld", p_ptr->au);
			prt(tmp_str, 22, 2);
			prt("Again(Y/N)?", 18, 37);
			move_cursor(18, 49);
			again = inkey();
			if (wager > p_ptr->au)
			{
				msg_print("Hey! You don't have the gold - get out of here!");
				msg_print(NULL);
				screen_load();
				return (FALSE);
				/*				strnfmt(tmp_str, 80, "Current Wager:    %9ld",wager);
								prt(tmp_str, 17, 2); */
			}
		}
		while ((again == 'y') || (again == 'Y'));

		prt("", 18, 37);
		if (p_ptr->au >= oldgold)
			msg_print("You came out a winner! We'll win next time, I'm sure.");
		else
			msg_print("You lost gold! Haha, better head home.");
		msg_print(NULL);
	}

	screen_load();

	return (TRUE);
}


/*
 * inn commands
 * Note that resting for the night was a perfect way to avoid player
 * ghosts in the town *if* you could only make it to the inn in time (-:
 * Now that the ghosts are temporarily disabled in 2.8.X, this function
 * will not be that useful.  I will keep it in the hopes the player
 * ghost code does become a reality again. Does help to avoid filthy urchins.
 * Resting at night is also a quick way to restock stores -KMW- 
 */
static bool_ inn_comm(int cmd)
{
	bool_ vampire;


	/* Extract race info */
	vampire = ((race_flags_p(PR_VAMPIRE)) || (p_ptr->mimic_form == resolve_mimic_name("Vampire")));

	switch (cmd)
	{
	case BACT_FOOD:  /* Buy food & drink */
		{
			if (!vampire)
			{
				msg_print("The barkeep gives you some gruel and a beer.");
				msg_print(NULL);
				(void) set_food(PY_FOOD_MAX - 1);
			}
			else
				msg_print("You're a vampire and I don't have any food for you!");

			break;
		}

		/*
		 * I revamped this... Don't know why normal races didn't get
		 * mana regenerated. It is the grand tradition of p&p games -- pelpel
		 */
	case BACT_REST:  /* Rest for the night */
		{
			bool_ nighttime;

			/* Extract the current time */
			nighttime = ((bst(HOUR, turn) < 6) || (bst(HOUR, turn) >= 18));

			/* Normal races rest at night */
			if (!vampire && !nighttime)
			{
				msg_print("The rooms are available only at night.");
				msg_print(NULL);
				return (FALSE);
			}

			/* Vampires rest during daytime */
			if (vampire && nighttime)
			{
				msg_print("The rooms are available only during daylight for your kind.");
				msg_print(NULL);
				return (FALSE);
			}

			/* Must cure HP draining status first */
			if ((p_ptr->poisoned > 0) || (p_ptr->cut > 0))
			{
				msg_print("You need a healer, not a room.");
				msg_print(NULL);
				msg_print("Sorry, but I don't want anyone dying in here.");
				return (FALSE);
			}

			/* Let the time pass XXX XXX XXX */
			if (vampire)
			{
				/* Wait for sunset */
				while ((bst(HOUR, turn) >= 6) && (bst(HOUR, turn) < 18))
				{
					turn += (10L * MINUTE);
				}
			}
			else
			{
				/* Wait for sunrise */
				while ((bst(HOUR, turn) < 6) || (bst(HOUR, turn) >= 18))
				{
					turn += (10L * MINUTE);
				}
			}

			/* Regen */
			p_ptr->chp = p_ptr->mhp;
			p_ptr->csp = p_ptr->msp;

			/* Restore status */
			set_blind(0);
			set_confused(0);
			p_ptr->stun = 0;

			/* Message */
			if (vampire) msg_print("You awake refreshed for the new night.");
			else msg_print("You awake refreshed for the new day.");

			/* Dungeon stuff */
			p_ptr->leaving = TRUE;
			p_ptr->oldpx = p_ptr->px;
			p_ptr->oldpy = p_ptr->py;

			break;
		}

	case BACT_RUMORS:  /* Listen for rumors */
		{
			char rumor[80];

			get_rnd_line("rumors.txt", rumor);
			msg_format("%s", rumor);
			msg_print(NULL);

			break;
		}
	}

	return (TRUE);
}


/*
 * Display quest information
 */
static void get_questinfo(int questnum)
{
	int i;


	/* Print the quest info */
	prt(format("Quest Information (Danger level: %d)", quest[questnum].level), 5, 0);

	prt(quest[questnum].name, 7, 0);

	i = 0;
	while ((i < 10) && (quest[questnum].desc[i][0] != '\0'))
	{
		c_put_str(TERM_YELLOW, quest[questnum].desc[i], i + 8, 0);
		i++;
	}
}


/*
 * Request a quest from the Lord.
 */
static bool_ castle_quest(int y, int x)
{
	int plot = 0;

	quest_type *q_ptr;


	clear_bldg(7, 18);

	/* Current plot of the building */
	plot = cave[y][x].special;

	/* Is there a quest available at the building? */
	if ((!plot) || (plots[plot] == QUEST_NULL))
	{
		put_str("I don't have a quest for you at the moment.", 8, 0);
		return FALSE;
	}

	q_ptr = &quest[plots[plot]];

	/* Quest is completed */
	if (q_ptr->status == QUEST_STATUS_COMPLETED)
	{
		/* Rewarded quest */
		q_ptr->status = QUEST_STATUS_FINISHED;

		struct hook_quest_finish_in in = { plots[plot] };
		process_hooks_new(HOOK_QUEST_FINISH, &in, NULL);

		return (TRUE);
	}

	/* Quest is still unfinished */
	else if (q_ptr->status == QUEST_STATUS_TAKEN)
	{
		put_str("You have not completed your current quest yet!", 8, 0);
		put_str("Use CTRL-Q to check the status of your quest.", 9, 0);
		put_str("Return when you have completed your quest.", 12, 0);

		return (FALSE);
	}
	/* Failed quest */
	else if (q_ptr->status == QUEST_STATUS_FAILED)
	{
		/* Mark quest as done (but failed) */
		q_ptr->status = QUEST_STATUS_FAILED_DONE;

		hook_quest_fail_in in = { plots[plot] };
		process_hooks_new(HOOK_QUEST_FAIL, &in, NULL);

		return (FALSE);
	}
	/* No quest yet */
	else if (q_ptr->status == QUEST_STATUS_UNTAKEN)
	{
		struct hook_init_quest_in in = { plots[plot] };
		if (process_hooks_new(HOOK_INIT_QUEST, &in, NULL))
		{
			return (FALSE);
		}

		q_ptr->status = QUEST_STATUS_TAKEN;

		/* Assign a new quest */
		get_questinfo(plots[plot]);

		/* Add the hooks */
		quest[plots[plot]].init(plots[plot]);

		return (TRUE);
	}

	return FALSE;
}

/*
 * Displaying town history -KMW-
 */
static void town_history(void)
{
	/* Save screen */
	screen_save();

	/* Peruse the building help file */
	show_file("bldg.txt", NULL);

	/* Load screen */
	screen_load();
}


/*
 * compare_weapon_aux2 -KMW-
 */
static void compare_weapon_aux2(object_type *o_ptr, int numblows, int r, int c, int mult, const char *attr, byte color)
{
	char tmp_str[80];

	c_put_str(color, attr, r, c);
	strnfmt(tmp_str, 80, "Attack: %d-%d damage",
	        numblows * ((o_ptr->dd * mult) + o_ptr->to_d),
	        numblows * ((o_ptr->ds * o_ptr->dd * mult) + o_ptr->to_d));
	put_str(tmp_str, r, c + 8);
	r++;
}


/*
 * compare_weapon_aux1 -KMW-
 */
static void compare_weapon_aux1(object_type *o_ptr, int col, int r)
{
	auto const f = object_flags(o_ptr);

	if (f & TR_SLAY_ANIMAL)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 2, "Animals:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_EVIL)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 2, "Evil:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_UNDEAD)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Undead:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_DEMON)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Demons:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_ORC)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Orcs:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_TROLL)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Trolls:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_GIANT)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Giants:",
				    TERM_YELLOW);
	}
	if (f & TR_SLAY_DRAGON)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Dragons:",
				    TERM_YELLOW);
	}
	if (f & TR_KILL_DRAGON)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 5, "Dragons:",
				    TERM_YELLOW);
	}
	if (f & TR_BRAND_ACID)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Acid:",
				    TERM_RED);
	}
	if (f & TR_BRAND_ELEC)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Elec:",
				    TERM_RED);
	}
	if (f & TR_BRAND_FIRE)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Fire:",
				    TERM_RED);
	}
	if (f & TR_BRAND_COLD)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Cold:",
				    TERM_RED);
	}
	if (f & TR_BRAND_POIS)
	{
		compare_weapon_aux2(o_ptr, p_ptr->num_blow, r++, col, 3, "Poison:",
				    TERM_RED);
	}
}


/*
 * list_weapon -KMW-
 */
static void list_weapon(object_type *o_ptr, int row, int col)
{
	char o_name[80];

	char tmp_str[80];


	object_desc(o_name, o_ptr, TRUE, 0);
	c_put_str(TERM_YELLOW, o_name, row, col);
	strnfmt(tmp_str, 80, "To Hit: %d   To Damage: %d", o_ptr->to_h, o_ptr->to_d);
	put_str(tmp_str, row + 1, col);
	strnfmt(tmp_str, 80, "Dice: %d   Sides: %d", o_ptr->dd, o_ptr->ds);
	put_str(tmp_str, row + 2, col);
	strnfmt(tmp_str, 80, "Number of Blows: %d", p_ptr->num_blow);
	put_str(tmp_str, row + 3, col);
	c_put_str(TERM_YELLOW, "Possible Damage:", row + 5, col);
	strnfmt(tmp_str, 80, "One Strike: %d-%d damage", o_ptr->dd + o_ptr->to_d,
	        (o_ptr->ds*o_ptr->dd) + o_ptr->to_d);
	put_str(tmp_str, row + 6, col + 1);
	strnfmt(tmp_str, 80, "One Attack: %d-%d damage", p_ptr->num_blow*(o_ptr->dd + o_ptr->to_d),
	        p_ptr->num_blow*(o_ptr->ds*o_ptr->dd + o_ptr->to_d));
	put_str(tmp_str, row + 7, col + 1);
}


/*
 * Select melee weapons
 */
static bool item_tester_hook_melee_weapon(object_type const *o_ptr)
{
	return (wield_slot(o_ptr) == INVEN_WIELD);
}

/*
 * compare_weapons -KMW-
 */
static bool_ compare_weapons(void)
{
	int item, i;

	object_type *o1_ptr, *o2_ptr, *orig_ptr;

	clear_bldg(6, 18);

	o1_ptr = NULL;
	o2_ptr = NULL;

	/* Store copy of original wielded weapon in pack slot */
	object_type *i_ptr = &p_ptr->inventory[INVEN_WIELD];
	orig_ptr = &p_ptr->inventory[INVEN_PACK];
	object_copy(orig_ptr, i_ptr);

	i = 6;

	/* Get first weapon */
	if (!get_item(&item,
		      "What is your first melee weapon? ",
		      "You have nothing to compare.",
		      (USE_EQUIP | USE_INVEN),
		      item_tester_hook_melee_weapon))
	{
		object_wipe(orig_ptr);
		return (FALSE);
	}

	/* Get the item (in the pack) */
	if (item >= 0)
		o1_ptr = &p_ptr->inventory[item];

	/* Get second weapon */
	int item2;
	if (!get_item(&item2,
		      "What is your second melee weapon? ",
		      "You have nothing to compare.",
		      (USE_EQUIP | USE_INVEN),
		      item_tester_hook_melee_weapon))
	{
		object_wipe(orig_ptr);
		return (FALSE);
	}

	/* Get the item (in the pack) */
	if (item2 >= 0) o2_ptr = &p_ptr->inventory[item2];

	put_str("Based on your current abilities, here is what your weapons will do", 4, 2);

	i_ptr = &p_ptr->inventory[INVEN_WIELD];
	object_copy(i_ptr, o1_ptr);
	calc_bonuses(TRUE);

	list_weapon(o1_ptr, i, 2);
	compare_weapon_aux1(o1_ptr, 2, i + 8);

	i_ptr = &p_ptr->inventory[INVEN_WIELD];
	if (item2 == INVEN_WIELD)
		object_copy(i_ptr, orig_ptr);
	else
		object_copy(i_ptr, o2_ptr);
	calc_bonuses(TRUE);

	list_weapon(o2_ptr, i, 40);
	compare_weapon_aux1(o2_ptr, 40, i + 8);

	i_ptr = &p_ptr->inventory[INVEN_WIELD];
	object_copy(i_ptr, orig_ptr);
	calc_bonuses(TRUE);

	object_wipe(orig_ptr);

	put_str("(Only highest damage applies per monster. Special damage not cumulative)", 20, 0);

	return (TRUE);
}


/*
 * general all-purpose fixing routine for items from building personnel
 * sharpen arrows, repair armor, repair weapon
 * -KMW-
 */
static bool_ fix_item(int istart, int iend, int ispecific, bool_ iac,
                     int ireward, bool_ set_reward)
{
	int i;

	int j = 9;

	int maxenchant = (p_ptr->lev / 5);

	object_type *o_ptr;

	char out_val[80], tmp_str[80];

	bool_ repaired = FALSE;

	clear_bldg(5, 18);
	strnfmt(tmp_str, 80, "  Based on your skill, we can improve up to +%d", maxenchant);
	prt(tmp_str, 5, 0);
	prt("Status", 7, 30);

	for (i = istart; i <= iend; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (ispecific > 0)
		{
			if (o_ptr->tval != ispecific)
				continue;
		}

		if (o_ptr->tval)
		{
			object_desc(tmp_str, o_ptr, FALSE, 1);

			if ((o_ptr->name1 && (o_ptr->ident & 0x08)))
				strnfmt(out_val, 80, "%-40s: beyond our skills!", tmp_str);
			else if (o_ptr->name1)
				strnfmt(out_val, 80, "%-40s: in fine condition", tmp_str);
			else
			{
				if ((iac) && (o_ptr->to_a <= -3))
				{
					strnfmt(out_val, 80, "%-40s: beyond repair, buy a new one", tmp_str);
				}
				else if ((iac) && (o_ptr->to_a < maxenchant))
				{
					o_ptr->to_a++;
					strnfmt(out_val, 80, "%-40s: polished -> (%d)", tmp_str, o_ptr->to_a);
					repaired = TRUE;
				}
				else if ((!iac) && ((o_ptr->to_h <= -3) || (o_ptr->to_d <= -3)))
				{
					strnfmt(out_val, 80, "%-40s: beyond repair, buy a new one", tmp_str);
				}
				/* Sharpen a weapon */
				else if ((!iac) && ((o_ptr->to_h < maxenchant) ||
				                    (o_ptr->to_d < maxenchant)))
				{
					if (o_ptr->to_h < maxenchant)
						o_ptr->to_h++;
					if (o_ptr->to_d < maxenchant)
						o_ptr->to_d++;
					strnfmt(out_val, 80, "%-40s: sharpened -> (%d,%d)", tmp_str,
					        o_ptr->to_h, o_ptr->to_d);
					repaired = TRUE;
				}
				else
					strnfmt(out_val, 80, "%-40s: in fine condition", tmp_str);
			}
			prt(out_val, j++, 0);
		}
	}

	if (!repaired)
	{
		msg_print("You don't have anything appropriate.");
		msg_print(NULL);
	}
	else
	{
		msg_print("Press the spacebar to continue");
		msg_print(NULL);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
		p_ptr->update |= (PU_BONUS);
	}
	clear_bldg(5, 18);

	return (repaired);
}


/*
 * Research Item
 */
static bool_ research_item(void)
{
	clear_bldg(5, 18);
	return (identify_fully());
}



/*
 * Execute a building command
 */
bool_ bldg_process_command(store_type *s_ptr, int i)
{
	store_action_type *ba_ptr = &ba_info[st_info[s_ptr->st_idx].actions[i]];

	int bact = ba_ptr->action;

	int bcost;

	bool_ paid = FALSE;

	bool_ set_reward = FALSE;

	bool_ recreate = FALSE;


	if (is_state(s_ptr, STORE_LIKED))
	{
		bcost = ba_ptr->costs[STORE_LIKED];
	}
	else if (is_state(s_ptr, STORE_HATED))
	{
		bcost = ba_ptr->costs[STORE_HATED];
	}
	else
	{
		bcost = ba_ptr->costs[STORE_NORMAL];
	}

	/* action restrictions */
	if (((ba_ptr->action_restr == 1) && (is_state(s_ptr, STORE_LIKED))) ||
	    ((ba_ptr->action_restr == 2) && (!is_state(s_ptr, STORE_LIKED))))
	{
		msg_print("You have no right to choose that!");
		msg_print(NULL);
		return FALSE;
	}

	/* check gold */
	if (bcost > p_ptr->au)
	{
		msg_print("You do not have the gold!");
		msg_print(NULL);
		return FALSE;
	}

	if (!bcost) set_reward = TRUE;

	switch (bact)
	{
	case BACT_RESEARCH_ITEM:
		{
			paid = research_item();
			break;
		}

	case BACT_TOWN_HISTORY:
		{
			town_history();
			break;
		}

	case BACT_RACE_LEGENDS:
		{
			race_legends();
			break;
		}

	case BACT_QUEST1:
		{
			int y = 1, x = 1;
			bool_ ok = FALSE;

			while ((x < cur_wid - 1) && !ok)
			{
				y = 1;
				while ((y < cur_hgt - 1) && !ok)
				{
					/* Found the location of the quest info ? */
					if (bact - BACT_QUEST1 + FEAT_QUEST1 == cave[y][x].feat)
					{
						/* Stop the loop */
						ok = TRUE;
					}
					y++;
				}
				x++;
			}

			if (ok)
			{
				recreate = castle_quest(y - 1, x - 1);
				;
			}
			else
			{
				msg_format("ERROR: no quest info feature found: %d", bact - BACT_QUEST1 + FEAT_QUEST1);
			}
			break;
		}

	case BACT_KING_LEGENDS:
		{
			show_highclass(building_loc);
			break;
		}

	case BACT_IN_BETWEEN:
	case BACT_CRAPS:
	case BACT_DICE_SLOTS:
	case BACT_GAMBLE_RULES:
		{
			gamble_comm(bact);
			break;
		}

	case BACT_REST:
	case BACT_RUMORS:
	case BACT_FOOD:
		{
			paid = inn_comm(bact);
			break;
		}

	case BACT_COMPARE_WEAPONS:
		{
			paid = compare_weapons();
			break;
		}

	case BACT_ENCHANT_WEAPON:
		{
			paid = fix_item(INVEN_WIELD, INVEN_WIELD, 0, FALSE,
			                BACT_ENCHANT_WEAPON, set_reward);
			break;
		}

	case BACT_ENCHANT_ARMOR:
		{
			paid = fix_item(INVEN_BODY, INVEN_FEET, 0, TRUE,
			                BACT_ENCHANT_ARMOR, set_reward);
			break;
		}

		/* needs work */
	case BACT_RECHARGE:
		{
			if (recharge(80)) paid = TRUE;
			break;
		}

		/* needs work */
	case BACT_IDENTS:
		{
			identify_pack();
			msg_print("Your possessions have been identified.");
			msg_print(NULL);
			paid = TRUE;
			break;
		}

		/* needs work */
	case BACT_STAR_HEAL:
		{
			hp_player(200);
			set_poisoned(0);
			set_blind(0);
			set_confused(0);
			set_cut(0);
			set_stun(0);
			if (p_ptr->black_breath)
			{
				msg_print("The hold of the Black Breath on you is broken!");
				p_ptr->black_breath = FALSE;
			}
			paid = TRUE;
			break;
		}

		/* needs work */
	case BACT_HEALING:
		{
			hp_player(200);
			set_poisoned(0);
			set_blind(0);
			set_confused(0);
			set_cut(0);
			set_stun(0);
			paid = TRUE;
			break;
		}

		/* needs work */
	case BACT_RESTORE:
		{
			if (do_res_stat(A_STR, TRUE)) paid = TRUE;
			if (do_res_stat(A_INT, TRUE)) paid = TRUE;
			if (do_res_stat(A_WIS, TRUE)) paid = TRUE;
			if (do_res_stat(A_DEX, TRUE)) paid = TRUE;
			if (do_res_stat(A_CON, TRUE)) paid = TRUE;
			if (do_res_stat(A_CHR, TRUE)) paid = TRUE;
			break;
		}

	case BACT_ENCHANT_ARROWS:
		{
			paid = fix_item(0, INVEN_WIELD, TV_ARROW, FALSE,
			                BACT_ENCHANT_ARROWS, set_reward);
			break;
		}

	case BACT_ENCHANT_BOW:
		{
			paid = fix_item(INVEN_BOW, INVEN_BOW, TV_BOW, FALSE,
			                BACT_ENCHANT_BOW, set_reward);
			break;
		}

	case BACT_RECALL:
		{
			p_ptr->word_recall = 1;
			msg_print("The air about you becomes charged...");
			paid = TRUE;
			break;
		}

	case BACT_TELEPORT_LEVEL:
		{
			if (reset_recall(FALSE))
			{
				p_ptr->word_recall = 1;
				msg_print("The air about you becomes charged...");
				paid = TRUE;
			}
			break;
		}

	case BACT_MIMIC_NORMAL:
		{
			set_mimic(0, 0, 0);
			paid = TRUE;
			break;
		}

	case BACT_DIVINATION:
		{
			int i, count = 0;
			bool_ something = FALSE;

			while (count < 1000)
			{
				count++;
				i = rand_int(MAX_FATES);
				if (!fates[i].fate) continue;
				if (fates[i].know) continue;
				msg_print("You know a little more of your fate.");

				fates[i].know = TRUE;
				something = TRUE;
				break;
			}

			if (!something) msg_print("Well, you have no fate, but I'll keep your money anyway!");

			paid = TRUE;
			break;

		}

	case BACT_BUY:
		{
			store_purchase();
			break;
		}

	case BACT_SELL:
		{
			store_sell();
			break;
		}

	case BACT_EXAMINE:
		{
			store_examine();
			break;
		}

	case BACT_STEAL:
		{
			store_stole();
			break;
		}

	case BACT_DROP_ITEM:
	{
		quest_bounty_drop_item();
		break;
	}

	case BACT_GET_ITEM:
	{
		quest_bounty_get_item();
		break;
	}

	case BACT_LIBRARY_QUEST:
	{
		quest_library_building(&paid, &recreate);
		break;
	}

	case BACT_FIREPROOF_QUEST:
	{
		quest_fireproof_building(&paid, &recreate);
		break;
	}

	case BACT_EREBOR_KEY:
	{
		msg_print("You will need Thorin's Key and Thrain's Map"
			  " to get anywhere in Erebor. One may be found"
			  " in the Barrow-Downs. The other, in Mirkwood.");
		break;
	}

	default:
		printf("Unknown building action %d\n", static_cast<int>(bact));
		break;
	}

	if (paid)
	{
		p_ptr->au -= bcost;

		/* Display the current gold */
		store_prt_gold();
	}

	return (recreate);
}


/*
 * Enter quest level
 */
void enter_quest(void)
{
	if (!(cave[p_ptr->py][p_ptr->px].feat == FEAT_QUEST_ENTER))
	{
		msg_print("You see no quest level here.");
		return;
	}
	else
	{
		/* Player enters a new quest */
		p_ptr->oldpy = p_ptr->py;
		p_ptr->oldpx = p_ptr->px;

		leaving_quest = p_ptr->inside_quest;

		p_ptr->inside_quest = cave[p_ptr->py][p_ptr->px].special;
		dun_level = 1;
		p_ptr->leaving = TRUE;
		p_ptr->oldpx = p_ptr->px;
		p_ptr->oldpy = p_ptr->py;
	}
}
