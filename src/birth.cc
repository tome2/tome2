/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */
#include "birth.hpp"

#include "ability_type.hpp"
#include "artifact_type.hpp"
#include "corrupt.hpp"
#include "cmd4.hpp"
#include "cmd5.hpp"
#include "dungeon_flag.hpp"
#include "dungeon_info_type.hpp"
#include "files.h"
#include "files.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "help.hpp"
#include "hooks.hpp"
#include "init2.hpp"
#include "mimic.hpp"
#include "messages.hpp"
#include "modules.hpp"
#include "monster2.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "notes.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "options.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_flag.hpp"
#include "player_race_mod.hpp"
#include "player_type.hpp"
#include "q_rand.hpp"
#include "skill_type.hpp"
#include "skills.hpp"
#include "spells2.hpp"
#include "spells3.hpp"
#include "spells5.hpp"
#include "stats.hpp"
#include "store.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "util.h"
#include "variable.h"
#include "variable.hpp"
#include "wilderness_map.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-rand.hpp"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <numeric>
#include <string>

/*
 * How often the autoroller will update the display and pause
 * to check for user interuptions.
 * Bigger values will make the autoroller faster, but slower
 * system may have problems because the user can't stop the
 * autoroller for this number of rolls.
 */
#define AUTOROLLER_STEP 25L

/*
 * Maximum number of tries for selection of a proper quest monster
 */
#define MAX_TRIES 100

/* Max quests */
static byte max_quests = 0;

/*
 * Current stats
 */
static s16b stat_use[6];

/*
 * Autoroll limit
 */
static s16b stat_limit[6];

/*
 * Autoroll matches
 */
static s32b stat_match[6];

/*
 * Autoroll round
 */
static s32b auto_round;

/*
 * Last round
 */
static s32b last_round;

/*
 * Random Name Generator
 * based on a Javascript by Michael Hensley
 * "http://geocities.com/timessquare/castle/6274/"
 */
static std::string create_random_name()
{
	static const std::vector<std::string> human_syllable1
	{
		"Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar",
		"B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et",
		"Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha",
		"Ib", "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir",
		"N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T",
		"Tr", "Th", "V", "Y", "Z", "W", "Wic",
	};

	static const std::vector<std::string> human_syllable2
	{
		"a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei",
		"ea", "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie",
		"ire", "ira", "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe",
		"ore", "u", "y",
	};

	static const std::vector<std::string> human_syllable3
	{
		"a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch",
		"can", "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g",
		"gord", "gan", "l", "li", "lgrin", "lin", "lith", "lath", "loth",
		"ld", "ldric", "ldan", "m", "mas", "mos", "mar", "mond", "n",
		"nydd", "nidd", "nnon", "nwan", "nyth", "nad", "nn", "nnor", "nd",
		"p", "r", "ron", "rd", "s", "sh", "seth", "sean", "t", "th", "tha",
		"tlan", "trem", "tram", "v", "vudd", "w", "wan", "win", "wyn", "wyr",
		"wyr", "wyth",
	};

	auto const &syl1 = *uniform_element(human_syllable1);
	auto const &syl2 = *uniform_element(human_syllable2);
	auto const &syl3 = *uniform_element(human_syllable3);

	return syl1 + syl2 + syl3;
}


void print_desc_aux(cptr txt, int y, int xx)
{
	int i = -1, x = xx;


	while (txt[++i] != 0)
	{
		if (txt[i] == '\n')
		{
			x = xx;
			y++;
		}
		else
		{
			Term_putch(x++, y, TERM_YELLOW, txt[i]);
		}
	}
}

void print_desc(cptr txt)
{
	print_desc_aux(txt, 12, 1);
}

/*
 * Save the current data for later
 */
static void save_prev_data()
{
	auto &previous_char = game->previous_char;

	/*** Save the current data ***/

	/* Save the data */
	previous_char.race = p_ptr->prace;
	previous_char.rmod = p_ptr->pracem;
	previous_char.pclass = p_ptr->pclass;
	previous_char.spec = p_ptr->pspec;

	previous_char.quests = max_quests;

	previous_char.god = p_ptr->pgod;
	previous_char.grace = p_ptr->grace;

	previous_char.au = p_ptr->au;

	/* Save the stats */
	for (int i = 0; i < 6; i++)
	{
		previous_char.stat[i] = p_ptr->stat_max[i];
	}
	previous_char.luck = p_ptr->luck_base;
}


/*
 * Load the previous data
 */
static void load_prev_data(bool_ save)
{
	auto &previous_char = game->previous_char;
	birther temp;


	/*** Save the current data ***/

	/* Save the data */
	temp.au = p_ptr->au;

	/* Save the stats */
	for (int i = 0; i < 6; i++)
	{
		temp.stat[i] = p_ptr->stat_max[i];
	}
	temp.luck = p_ptr->luck_base;

	/*** Load the previous data ***/

	/* Load the data */
	p_ptr->au = previous_char.au;

	/* Load the stats */
	for (int i = 0; i < 6; i++)
	{
		p_ptr->stat_max[i] = previous_char.stat[i];
		p_ptr->stat_cur[i] = previous_char.stat[i];
	}
	p_ptr->luck_base = previous_char.luck;
	p_ptr->luck_max = previous_char.luck;


	/*** Save the current data ***/
	if (!save) return;

	/* Save the data */
	previous_char.au = temp.au;

	/* Save the stats */
	for (int i = 0; i < 6; i++)
	{
		previous_char.stat[i] = temp.stat[i];
	}
	previous_char.luck = temp.luck;
}




/*
 * Returns adjusted stat -JK-  Algorithm by -JWT-
 *
 * auto_roll is boolean and states maximum changes should be used rather
 * than random ones to allow specification of higher values to wait for
 *
 * The "p_ptr->maximize" code is important        -BEN-
 */
static int adjust_stat(int value, int amount, int auto_roll)
{
	int i;


	/* Negative amounts */
	if (amount < 0)
	{
		/* Apply penalty */
		for (i = 0; i < (0 - amount); i++)
		{
			if (value >= 18 + 10)
			{
				value -= 10;
			}
			else if (value > 18)
			{
				value = 18;
			}
			else if (value > 3)
			{
				value--;
			}
		}
	}

	/* Positive amounts */
	else if (amount > 0)
	{
		/* Apply reward */
		for (i = 0; i < amount; i++)
		{
			if (value < 18)
			{
				value++;
			}
			else
			{
				value += 10;
			}
		}
	}

	/* Return the result */
	return (value);
}




/*
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats()
{
	int i, j;

	int bonus;

	int dice[18];


	/* Roll and verify some stats */
	while (TRUE)
	{
		/* Roll some dice */
		for (j = i = 0; i < 18; i++)
		{
			/* Roll the dice */
			dice[i] = randint(3 + i % 3);

			/* Collect the maximum */
			j += dice[i];
		}

		/*
		 * Verify totals
		 *
		 * 57 was 54... I hate 'magic numbers' :< TY
		 *
		 * (d3 + d4 + d5)         ~= 7.5 (+- 4.5)
		 * with 5 makes avg. stat value of 12.5 (min 8, max 17)
		 *
		 * (d3 + d4 + d5) x 6 ~= 45 (+- 18)
		 *
		 * So the original value (still used by Vanilla as of 2.9.3)
		 * allows (avg - 2)..(avg + 8), while this Z version
		 * (avg - 2)..(avg + 11). I don't understand what TY meant
		 * by "magic numbers", but I like big stats :) -- pelpel
		 *
		 */
		if ((j > 42) && (j < 57)) break;
	}

	/* Acquire the stats */
	for (i = 0; i < 6; i++)
	{
		/* Extract 5 + 1d3 + 1d4 + 1d5 */
		j = 5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2];

		/* Save that value */
		p_ptr->stat_max[i] = j;

		/* Obtain a "bonus" for "race" and "class" */
		bonus = rp_ptr->ps.adj[i] + rmp_ptr->ps.adj[i] + cp_ptr->ps.adj[i];

		/* Start fully healed */
		p_ptr->stat_cur[i] = p_ptr->stat_max[i];

		/* Efficiency -- Apply the racial/class bonuses */
		stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);

		/* No temporary drain (yet...) */
		p_ptr->stat_cnt[i] = 0;
		p_ptr->stat_los[i] = 0;
	}

	/* Get luck */
	p_ptr->luck_base = rp_ptr->luck + rmp_ptr->luck + rand_range( -5, 5);
	p_ptr->luck_max = p_ptr->luck_base;
}


/*
 * Roll for player HP
 */
void roll_player_hp()
{
	auto &player_hp = game->player_hp;

	// Minimum hitpoints at highest level
	int const min_value =
	        (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8 + PY_MAX_LEVEL;

	// Maximum hitpoints at highest level
	int const max_value =
	        (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8 + PY_MAX_LEVEL;

	// Pre-calculate level 1 hitdice; first roll is always maximal
	player_hp[0] = p_ptr->hitdie;

	// Roll out the hitpoints
	while (true)
	{
		// Roll the hitpoint values
		std::generate(
		        player_hp.begin() + 1,
		        player_hp.end(),
		        []() { return randint(p_ptr->hitdie); });

		// Sum along
		std::partial_sum(
		        player_hp.begin(),
		        player_hp.end(),
		        player_hp.begin());

		// Require "valid" hitpoints at highest level
		if (player_hp.back() < min_value) continue;
		if (player_hp.back() > max_value) continue;

		// Acceptable
		break;
	}
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra()
{
	/* Level one */
	p_ptr->max_plv = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->ps.exp + rmp_ptr->ps.exp + cp_ptr->ps.exp;

	/* Initialize quest */
	p_ptr->inside_quest = 0;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->ps.mhp + rmp_ptr->ps.mhp + cp_ptr->ps.mhp;

	/* Initial hitpoints */
	p_ptr->mhp = p_ptr->hitdie;

	/* Roll for HP */
	roll_player_hp();

	/* Set tactics and movement */
	p_ptr->tactic = 4;
	p_ptr->movement = 4;
}


/*
 * Fill the random_artifacts array with relevant info.
 */
static errr init_randart()
{
	char buf[80];

	for (int i = 0; i < MAX_RANDARTS; i++)
	{
		// Generate a 'cost'
		auto cost = randnor(0, 250);
		if (cost < 0)
		{
			cost = 0;
		}

		// Generate the random artifact
		random_artifact ra;
		ra.name_short = get_line("rart_s.txt", ANGBAND_DIR_FILE, buf, i);
		ra.name_full = get_line("rart_f.txt", ANGBAND_DIR_FILE, buf, i);
		ra.attr = randint(15);
		ra.activation = rand_int(MAX_T_ACT);
		ra.generated = FALSE;
		ra.cost = cost;

		// Push
		game->random_artifacts.push_back(ra);
	}

	return 0;
}



/*
 * Get the player's starting money
 */
static void get_money()
{
	/* Starting gold */
	int gold = randint(100) + 300;

	/* Process the stats */
	for (int i = 0; i < 6; i++)
	{
		/* Mega-Hack -- reduce gold for high stats */
		if (stat_use[i] >= 18 + 50) gold -= 300;
		else if (stat_use[i] >= 18 + 20) gold -= 200;
		else if (stat_use[i] > 18) gold -= 150;
		else gold -= (stat_use[i] - 8) * 10;
	}

	/* Minimum 100 gold */
	if (gold < 100) gold = 100;

	/* Save the gold */
	p_ptr->au = gold;
}



/*
 * Display stat values, subset of "put_stats()"
 *
 * See 'display_player()' for basic method.
 */
static void birth_put_stats()
{
	int i, p;

	byte attr;

	char buf[80];


	/* Put the stats (and percents) */
	for (i = 0; i < 6; i++)
	{
		/* Put the stat */
		cnv_stat(p_ptr->stat_use[i], buf);
		c_put_str(TERM_L_GREEN, buf, 2 + i, 66);

		/* Put the percent */
		if (stat_match[i])
		{
			p = 1000L * stat_match[i] / auto_round;
			attr = (p < 100) ? TERM_YELLOW : TERM_L_GREEN;
			strnfmt(buf, 80, "%3d.%d%%", p / 10, p % 10);
			c_put_str(attr, buf, 2 + i, 73);
		}

		/* Never happened */
		else
		{
			c_put_str(TERM_RED, "(NONE)", 2 + i, 73);
		}
	}
}


/*
 * Clear all the global "character" data
 */
static void player_wipe()
{
	auto const &d_info = game->edit_data.d_info;
	auto &r_info = game->edit_data.r_info;
	auto &k_info = game->edit_data.k_info;
	auto &a_info = game->edit_data.a_info;
	auto &level_markers = game->level_markers;

	/* Wipe special levels */
	wipe_saved();

	/* Hack -- zero the struct */
	*p_ptr = player_type();

	/* Level 1 is the first level */
	p_ptr->lev = 1;

	/* Not dead yet */
	p_ptr->lives = 0;

	/* Wipe the towns */
	for (auto &&i: level_markers)
	{
		for (auto &&j: i)
		{
			j = level_marker::NORMAL;
		}
	}

	/* Wipe the towns */
	for (std::size_t i = max_real_towns + 1; i < max_towns; i++)
	{
		town_info[i].flags = 0;
	}

	/* Wipe the quests */
	for (std::size_t i = 0; i < MAX_Q_IDX; i++)
	{
		quest[i].status = QUEST_STATUS_UNTAKEN;
		for (auto &quest_data : quest[i].data)
		{
			quest_data = 0;
		}
	}

	/* No items */
	inven_cnt = 0;
	equip_cnt = 0;

	/* Clear the inventory */
	for (std::size_t i = 0; i < INVEN_TOTAL; i++)
	{
		object_wipe(&p_ptr->inventory[i]);
	}

	/* Generate random artifacts */
	init_randart();

	/* Start with no artifacts made yet */
	for (auto &a_ref: a_info)
	{
		a_ref.cur_num = 0;
	}

	/* Reset the "objects" */
	for (auto &k_entry: k_info)
	{
		auto &k_ref = k_entry.second;
		k_ref.tried = FALSE;
		k_ref.aware = FALSE;
		k_ref.artifact = 0;
	}


	/* Reset the "monsters" */
	for (auto &r_ref: r_info)
	{
		auto r_ptr = &r_ref;

		/* Hack -- Reset the counter */
		r_ptr->cur_num = 0;

		/* Hack -- Reset the max counter */
		r_ptr->max_num = 100;

		/* Hack -- Reset the max counter */
		if (r_ptr->flags & RF_UNIQUE) r_ptr->max_num = 1;

		/* Clear player kills */
		r_ptr->r_pkills = 0;

		/* Clear saved flag */
		r_ptr->on_saved = FALSE;
	}


	/* Hack -- Well fed player */
	p_ptr->food = PY_FOOD_FULL - 1;

	/* Clear "cheat" options */
	options->reset_cheat_options();

	/* Assume no winning game */
	total_winner = 0;
	has_won = FALSE;

	/* Assume no cheating */
	noscore = 0;
	wizard = 0;

	/* Clear the fate */
	for (std::size_t i = 0; i < MAX_FATES; i++)
	{
		fates[i].fate = 0;
	}
	p_ptr->no_mortal = FALSE;

	/* Player don't have the black breath from the beginning !*/
	p_ptr->black_breath = FALSE;

	/* Default pet command settings */
	p_ptr->pet_follow_distance = 6;
	p_ptr->pet_open_doors = FALSE;
	p_ptr->pet_pickup_items = FALSE;

	/* Body changing initialisation */
	p_ptr->body_monster = 0;
	p_ptr->disembodied = FALSE;

	/* Wipe xtra hp */
	p_ptr->hp_mod = 0;

	/* Wipe the monsters */
	wipe_m_list();

	/* Wipe the doppleganger */
	doppleganger = 0;

	/* Wipe the recall depths */
	for (std::size_t i = 0; i < d_info.size(); i++)
	{
		max_dlv[i] = 0;
	}

	/* Wipe the known inscription list */
	for (auto &inscription_known: p_ptr->inscriptions)
	{
		inscription_known = false;
	}

	/* Reset wild_mode to FALSE */
	p_ptr->wild_mode = FALSE;
	p_ptr->old_wild_mode = FALSE;

	/* Initialize allow_one_death */
	p_ptr->allow_one_death = 0;

	/* Wipe the power list */
	for (std::size_t i = 0; i < POWER_MAX; i++)
	{
		p_ptr->powers_mod[i] = 0;
	}

	/* No companions killed */
	p_ptr->companion_killed = 0;

	/* Inertia control */
	p_ptr->inertia_controlled_spell = -1;

	/* Automatic stat-gain */
	p_ptr->last_rewarded_level = 1;
}


/* Create an object */
static void outfit_obj(object_proto const *proto)
{
	object_type forge;

	/* Get local object */
	auto q_ptr = &forge;
	q_ptr->pval = 0;
	q_ptr->pval2 = 0;

	/* Hack -- Give the player an object */
	object_prep(q_ptr, lookup_kind(proto->tval, proto->sval));

	if (proto->pval)
	{
		q_ptr->pval = proto->pval;
	}

	/* These objects are "storebought" */
	q_ptr->ident |= IDENT_MENTAL;
	q_ptr->number = damroll(proto->dd, proto->ds);

	object_aware(q_ptr);
	object_known(q_ptr);
	inven_carry(q_ptr, FALSE);
}


static void outfit_objs(std::vector<object_proto> const &protos)
{
	for (auto const &proto: protos)
	{
		outfit_obj(&proto);
	}
}


/*
 * Give the player an object.
 */
static void player_outfit_object(int qty, int tval, int sval)
{
	object_type forge;
	object_type *q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(tval, sval));
	q_ptr->number = qty;
	object_aware(q_ptr);
	object_known(q_ptr);
	inven_carry(q_ptr, FALSE);
}


/*
 * Give player a spell book.
 */
static void player_outfit_spellbook(cptr spell_name)
{
	object_type forge;
	object_type *q_ptr = &forge;
	object_prep(q_ptr, lookup_kind(TV_BOOK, 255));
	q_ptr->pval = find_spell(spell_name);
	q_ptr->ident |= IDENT_MENTAL | IDENT_KNOWN;
	inven_carry(q_ptr, FALSE);
}


/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit()
{
	// Shorthand names for convenience
	cptr class_name = spp_ptr->title;
	auto const &subrace_name = rmp_ptr->title;

	/*
	 * Get an adventurer guide describing a bit of the
	 * wilderness.
	 */
	{
		/* Hack -- Give the player an adventurer guide */
		player_outfit_object(1, TV_PARCHMENT, 20);
	}

	/*
	 * Provide spell books
	 */
	if (game_module_idx == MODULE_TOME)
	{
		if (streq(class_name, "Ranger"))
		{
			player_outfit_spellbook("Phase Door");
		}
	}
	if (streq(class_name, "Geomancer"))
	{
		player_outfit_spellbook("Geyser");
	}
	if (streq(class_name, "Priest(Eru)"))
	{
		player_outfit_spellbook("See the Music");
	}
	if (streq(class_name, "Priest(Manwe)"))
	{
		player_outfit_spellbook("Manwe's Blessing");
	}
	if (streq(class_name, "Druid"))
	{
		player_outfit_spellbook("Charm Animal");
	}
	if (streq(class_name, "Dark-Priest"))
	{
		player_outfit_spellbook("Curse");
	}
	if (streq(class_name, "Paladin"))
	{
		player_outfit_spellbook("Divine Aim");
	}
	if (game_module_idx == MODULE_THEME)
	{
		/* Priests */
		if (streq(class_name, "Stonewright"))
		{
			player_outfit_spellbook("Firebrand");
		}
		if (streq(class_name, "Priest(Varda)"))
		{
			player_outfit_spellbook("Light of Valinor");
		}
		if (streq(class_name, "Priest(Ulmo)"))
		{
			player_outfit_spellbook("Song of Belegaer");
		}
		if (streq(class_name, "Priest(Mandos)"))
		{
			player_outfit_spellbook("Tears of Luthien");
		}

		/* Dragons */
		if (subrace_name == "Red")
		{
			player_outfit_spellbook("Globe of Light");
		}
		if (subrace_name == "Black")
		{
			player_outfit_spellbook("Geyser");
		}
		if (subrace_name == "Green")
		{
			player_outfit_spellbook("Noxious Cloud");
		}
		if (subrace_name == "Blue")
		{
			player_outfit_spellbook("Stone Skin");
		}
		if (subrace_name == "White")
		{
			player_outfit_spellbook("Sense Monsters");
		}
		if (subrace_name == "Ethereal")
		{
			player_outfit_spellbook("Recharge");
		}

		/* Demons */
		if (subrace_name == "(Aewrog)")
		{
			player_outfit_spellbook("Charm");
		}
		if (subrace_name == "(Narrog)")
		{
			player_outfit_spellbook("Phase Door");
		}

		/* Peace-mages */
		if (streq(class_name, "Peace-mage"))
		{
			player_outfit_spellbook("Phase Door");
		}

		/* Wainriders */
		if (streq(class_name, "Wainrider"))
		{
			player_outfit_spellbook("Curse");
		}
	}

	if (streq(class_name, "Mimic"))
	{
		object_type forge;
		object_type *q_ptr = &forge;
		
		object_prep(q_ptr, lookup_kind(TV_CLOAK, SV_MIMIC_CLOAK));
		q_ptr->pval2 = resolve_mimic_name("Mouse");
		q_ptr->ident |= IDENT_MENTAL | IDENT_KNOWN;
		inven_carry(q_ptr, FALSE);
	}

	if (game_module_idx == MODULE_THEME)
	{
		/* Give everyone a scroll of WoR. */
		player_outfit_object(1, TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);

		/* Identify everything in pack. */
		identify_pack_fully();
	}

	if (rmp_ptr->title == "Vampire")
	{
		player_gain_corruption(CORRUPT_VAMPIRE_TEETH);
		player_gain_corruption(CORRUPT_VAMPIRE_STRENGTH);
		player_gain_corruption(CORRUPT_VAMPIRE_VAMPIRE);
	}

	process_hooks_new(HOOK_BIRTH_OBJECTS, NULL, NULL);
	meta_inertia_control_hook_birth_objects();

	{
		/* Hack -- Give the player some food */
		int qty = (byte)rand_range(3, 7);
		player_outfit_object(qty, TV_FOOD, SV_FOOD_RATION);
	}

	{
		object_type forge;
		object_type *q_ptr = &forge;
		/* Hack -- Give the player some torches */
		object_prep(q_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
		q_ptr->number = (byte)rand_range(3, 7);
		q_ptr->timeout = rand_range(3, 7) * 500;
		object_aware(q_ptr);
		object_known(q_ptr);
		inven_carry(q_ptr, FALSE);
	}

	/* Outfit the player with starting items */
	outfit_objs(rp_ptr->object_protos);
	outfit_objs(rmp_ptr->object_protos);
	outfit_objs(cp_ptr->object_protos);
	outfit_objs(cp_ptr->spec[p_ptr->pspec].object_protos);
}


static void dump_classes(std::vector<u16b> const &classes, int sel, u32b *restrictions)
{
	auto const &class_info = game->edit_data.class_info;

	char buf[80];

	/* Clean up */
	clear_from(12);

	int n_max = static_cast<int>(classes.size()); // Warning avoidance

	for (int n = 0; n < n_max; n++)
	{
		cptr mod = "";
		char p2 = ')', p1 = ' ';

		/* Analyze */
		p_ptr->pclass = classes[n];
		cp_ptr = &class_info[p_ptr->pclass];

		if (sel == n)
		{
			p1 = '[';
			p2 = ']';
		}

		/* Display */
		strnfmt(buf, 80, "%c%c%c %s%s", p1,
			(n <= 25) ? I2A(n) : I2D(n - 26), p2, cp_ptr->title.c_str(), mod);

		/* Print some more info */
		if (sel == n)
		{
			std::string desc;

			desc += cp_ptr->desc;
			if (cp_ptr->flags & PR_EXPERIMENTAL)
			{
				desc += "\nEXPERIMENTAL";
			}

			print_desc(desc.c_str());

			if (!(restrictions[classes[n] / 32] & BIT(classes[n])) ||
			                (cp_ptr->flags & PR_EXPERIMENTAL))
				c_put_str(TERM_BLUE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
			else
				c_put_str(TERM_L_BLUE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
		}
		else
		{
			if (!(restrictions[classes[n] / 32] & BIT(classes[n])) ||
			                (cp_ptr->flags & PR_EXPERIMENTAL))
				c_put_str(TERM_SLATE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
			else
				put_str(buf, 18 + (n / 4), 1 + 20 * (n % 4));
		}

	}
}

static void dump_specs(int sel_)
{
	auto const &class_info = game->edit_data.class_info;

	assert(sel_ >= 0);
	std::size_t sel = sel_;

	clear_from(12);

	auto specs = &class_info[p_ptr->pclass].spec;

	for (std::size_t n = 0; n < specs->size(); n++)
	{
		char p2 = ')';
		char p1 = ' ';

		/* Analyze */
		p_ptr->pspec = n;
		spp_ptr = &(*specs)[p_ptr->pspec];

		if (sel == n)
		{
			p1 = '[';
			p2 = ']';
		}

		/* Display */
		char buf[80];
		strnfmt(buf, 80, "%c%c%c %s", p1, I2A(n), p2, spp_ptr->title);

		/* Print some more info */
		if (sel == n)
		{
			std::string desc;

			desc += spp_ptr->desc;
			if (spp_ptr->flags & PR_EXPERIMENTAL)
			{
				desc += "\nEXPERIMENTAL";
			}

			print_desc(desc.c_str());

			if (spp_ptr->flags & PR_EXPERIMENTAL)
			{
				c_put_str(TERM_BLUE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
			}
			else
			{
				c_put_str(TERM_L_BLUE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
			}
		}
		else
		{
			if (spp_ptr->flags & PR_EXPERIMENTAL)
			{
				c_put_str(TERM_SLATE, buf, 18 + (n / 4), 1 + 20 * (n % 4));
			}
			else
			{
				put_str(buf, 18 + (n / 4), 1 + 20 * (n % 4));
			}
		}
	}
}

static int dump_races(int sel)
{
	auto const &race_info = game->edit_data.race_info;

	int n = 0;

	char buf[80];

	/* Clean up */
	clear_from(12);

	for (n = 0; n < static_cast<int>(race_info.size()); n++)
	{
		char p2 = ')', p1 = ' ';

		/* Analyze */
		p_ptr->prace = n;
		rp_ptr = &race_info[p_ptr->prace];

		if (sel == n)
		{
			p1 = '[';
			p2 = ']';
		}

		/* Display */
		strnfmt(buf, 80, "%c%c%c %s", p1, I2A(n), p2, rp_ptr->title.c_str());

		/* Print some more info */
		if (sel == n)
		{
			std::string desc;

			desc += rp_ptr->desc;
			if (rp_ptr->flags & PR_EXPERIMENTAL)
			{
				desc += "\nEXPERIMENTAL";
			}

			print_desc(desc.c_str());

			if (rp_ptr->flags & PR_EXPERIMENTAL)
				c_put_str(TERM_BLUE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
			else
				c_put_str(TERM_L_BLUE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
		}
		else
		{
			if (rp_ptr->flags & PR_EXPERIMENTAL)
				c_put_str(TERM_SLATE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
			else
				put_str(buf, 18 + (n / 5), 1 + 15 * (n % 5));
		}
	}

	return (n);
}


static int dump_rmods(int sel, int *racem, int max)
{
	auto const &race_mod_info = game->edit_data.race_mod_info;

	int n = 0;

	char buf[80];

	/* Clean up */
	clear_from(12);

	/* Dump races */
	for (n = 0; n < max; n++)
	{
		char p2 = ')', p1 = ' ';

		/* Analyze */
		p_ptr->pracem = racem[n];
		rmp_ptr = &race_mod_info[p_ptr->pracem];

		if (sel == n)
		{
			p1 = '[';
			p2 = ']';
		}

		/* Display */
		if (racem[n])
			strnfmt(buf, 80, "%c%c%c %s", p1, I2A(n), p2, rmp_ptr->title.c_str());
		else
			strnfmt(buf, 80, "%c%c%c Classical", p1, I2A(n), p2);

		/* Print some more info */
		if (sel == n)
		{
			std::string desc;

			desc += rmp_ptr->description;
			if (rmp_ptr->flags & PR_EXPERIMENTAL)
			{
				desc += "\nEXPERIMENTAL";
			}

			print_desc(desc.c_str());

			if (rmp_ptr->flags & PR_EXPERIMENTAL)
				c_put_str(TERM_BLUE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
			else
				c_put_str(TERM_L_BLUE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
		}
		else
		{
			if (rmp_ptr->flags & PR_EXPERIMENTAL)
				c_put_str(TERM_SLATE, buf, 18 + (n / 5), 1 + 15 * (n % 5));
			else
				put_str(buf, 18 + (n / 5), 1 + 15 * (n % 5));
		}
	}

	return (n);
}

static int dump_gods(int sel, int *choice, int max)
{
	int i, j;
	char buf[80];
	cptr str;

	/* Clean up */
	clear_from(12);

	Term_putstr(5, 17, -1, TERM_WHITE,
	            "You can choose to worship a god, some class must start with a god.");

	for (i = 0; i < max; i++)
	{
		char p2 = ')', p1 = ' ';
		int n = choice[i];
		deity_type *g_ptr = &deity_info[0];

		if (!n) str = "No God";
		else
		{
			g_ptr = &deity_info[n];
			str = g_ptr->name;
		}

		if (sel == i)
		{
			p1 = '[';
			p2 = ']';
		}

		/* Display */
		strnfmt(buf, 80, "%c%c%c %s", p1, I2A(i), p2, str);

		/* Print some more info */
		if (sel == i)
		{
			if (n)
			{
				/* Display the first four lines of the god description */
				for (j = 0; j < 4; j++)
					if (strcmp(g_ptr->desc[j], ""))
						print_desc_aux(g_ptr->desc[j], 12 + j, 1);
			}
			else print_desc("You can begin as an atheist and still convert to a god later.");

			c_put_str(TERM_L_BLUE, buf, 20 + (i / 4), 1 + 20 * (i % 4));
		}
		else
		{
			put_str(buf, 20 + (i / 4), 1 + 20 * (i % 4));
		}
	}

	return (max);
}


/* Ask questions */
static bool_ do_quick_start = FALSE;

static bool_ player_birth_aux_ask()
{
	auto &class_info = game->edit_data.class_info;
	auto const &race_info = game->edit_data.race_info;
	auto const &race_mod_info = game->edit_data.race_mod_info;
	auto const &d_info = game->edit_data.d_info;

	int k, n, v, sel;

	int racem[100], max_racem = 0;

	u32b restrictions[2];

	char c;

	char buf[200];
	char inp[200];

	int const NAME_ROW = 2;
	int const RACE_ROW = 3;
	int const CLASS_ROW = 4;

	/*** Intro ***/

	/* Clear screen */
	Term_clear();

	/* Title everything */
	put_str("Name  :", NAME_ROW, 1);
	c_put_str(TERM_L_BLUE, game->player_name.c_str(), NAME_ROW, 9);

	put_str("Race  :", RACE_ROW, 1);

	put_str("Class :", CLASS_ROW, 1);

	/*** Instructions ***/

	/* Display some helpful information */
	Term_putstr(5, 8, -1, TERM_WHITE,
	            "Please answer the following questions.  Most of the questions");
	Term_putstr(5, 9, -1, TERM_WHITE,
	            "display a set of standard answers, and many will also accept");
	Term_putstr(5, 10, -1, TERM_WHITE,
	            "some special responses, including 'Q' to quit, 'S' to restart,");
	Term_putstr(5, 11, -1, TERM_WHITE,
	            "and '?' for help.  Note that 'Q' and 'S' must be capitalized.");


	/*** Quick Start ***/

	if (game->previous_char.quick_ok)
	{
		/* Choose */
		while (1)
		{
			put_str("Use same character as last time (y/n)? ", 20, 2);
			c = inkey();
			if (c == 'Q') quit(NULL);
			else if (c == 'S') return (FALSE);
			else if ((c == 'y') || (c == 'Y'))
			{
				do_quick_start = TRUE;
				break;
			}
			else
			{
				do_quick_start = FALSE;
				break;
			}
		}
	}

	/* Clean up */
	clear_from(15);

	/* Clean up */
	clear_from(15);


	/*** Player race ***/
	if (do_quick_start)
	{
		k = game->previous_char.race;
	}
	else
	{
		/* Only one choice = instant choice */
		if (race_info.size() == 1)
		{
			k = 0;
		}
		else
		{
			/* Extra info */
			Term_putstr(5, 16, -1, TERM_WHITE,
			            "Your 'race' determines various intrinsic factors and bonuses.");

			/* Dump races */
			sel = 0;
			n = dump_races(sel);

			/* Choose */
			while (1)
			{
				strnfmt(buf, 200, "Choose a race (%c-%c), * for a random choice, = for options, 8/2/4/6 for movement: ",
					I2A(0), I2A(race_info.size() - 1));
				put_str(buf, 17, 2);

				c = inkey();
				if (c == 'Q') quit(NULL);
				if (c == 'S') return (FALSE);
				if (c == '*')
				{
					k = rand_int(race_info.size());
					break;
				}
				k = (islower(c) ? A2I(c) : -1);
				if ((k >= 0) && (k < n)) break;
				if (c == '?')
				{
					help_race(race_info[sel].title);
				}
				else if (c == '=')
				{
					screen_save();
					do_cmd_options_aux(6, "Startup Options", FALSE);
					screen_load();
				}
				else if (c == '2')
				{
					sel += 5;
					if (sel >= n) sel %= 5;
					dump_races(sel);
				}
				else if (c == '8')
				{
					sel -= 5;
					if (sel < 0) sel = n - 1 -( ( -sel) % 5);
					/* C's modulus operator does not have defined
					 results for negative first values. Damn. */
					dump_races(sel);
				}
				else if (c == '6')
				{
					sel++;
					if (sel >= n) sel = 0;
					dump_races(sel);
				}
				else if (c == '4')
				{
					sel--;
					if (sel < 0) sel = n - 1;
					dump_races(sel);
				}
				else if (c == '\r')
				{
					k = sel;
					break;
				}
				else bell();
			}
		}
	}
	/* Set race */
	p_ptr->prace = k;
	rp_ptr = &race_info[p_ptr->prace];

	/* Display */
	c_put_str(TERM_L_BLUE, rp_ptr->title.c_str(), RACE_ROW, 9);

	/* Get a random name */
	if (!do_quick_start)
	{
		game->player_name = create_random_name();
	}

	/* Display */
	c_put_str(TERM_L_BLUE, game->player_name.c_str(), NAME_ROW, 9);

	/* Clean up */
	clear_from(12);


	/*** Player race mod ***/
	if (do_quick_start)
	{
		k = game->previous_char.rmod;
		p_ptr->pracem = k;
		rmp_ptr = &race_mod_info[p_ptr->pracem];
	}
	else
	{
		/* Only one choice = instant choice */
		if (race_mod_info.size() == 1)
		{
			k = 0;
		}
		else
		{
			for (n = 0; n < 100; n++) racem[n] = 0;

			max_racem = 0;
			for (n = 0; n < static_cast<int>(race_mod_info.size()); n++)
			{
				/* Analyze */
				p_ptr->pracem = n;
				rmp_ptr = &race_mod_info[p_ptr->pracem];

				/* Must be an ok choice */
				if (!(BIT(p_ptr->prace) & rmp_ptr->choice[p_ptr->prace / 32])) continue;

				/* Ok thats a possibility */
				racem[max_racem++] = n;
			}

			/* Ah ! nothing found, lets use the default */
			if (!max_racem) p_ptr->pracem = 0;
			/* Only one ? use it */
			else if (max_racem == 1) p_ptr->pracem = racem[0];
			/* We got to ask the player */
			else
			{
				/* Extra info */
				Term_putstr(5, 15, -1, TERM_WHITE,
				            "Your 'race modifier' determines various intrinsic factors and bonuses.");

				/* Dump races */
				sel = 0;
				n = dump_rmods(sel, racem, max_racem);

				/* Choose */
				while (1)
				{
					strnfmt(buf, 200, "Choose a race modifier (%c-%c), * for a random choice, = for options: ",
					        I2A(0), I2A(max_racem - 1));
					put_str(buf, 17, 2);
					c = inkey();
					if (c == 'Q') quit(NULL);
					if (c == 'S') return (FALSE);
					if (c == '*')
					{
						// Which choices are legal?
						std::vector<int> valid_choices(max_racem);
						for (int i = 0; i < max_racem; i++) {
							if ((BIT(racem[i]) & rmp_ptr->choice[racem[i] / 32]))
							{
								valid_choices.push_back(i);
							}
						}

						// Choose
						assert(!valid_choices.empty());
						k = *uniform_element(valid_choices);

						break;
					}
					else if (c == '?')
					{
						help_subrace(race_mod_info[racem[sel]].title.c_str());
					}

					k = (islower(c) ? A2I(c) : -1);
					if ((k >= 0) && (k < max_racem) &&
					                (BIT(p_ptr->prace) & race_mod_info[racem[k]].choice[p_ptr->prace / 32])) break;

					else if (c == '=')
					{
						screen_save();
						do_cmd_options_aux(6, "Startup Options", FALSE);
						screen_load();
					}
					else if (c == '2')
					{
						sel += 5;
						if (sel >= n) sel = sel - n + 1;
						dump_rmods(sel, racem, max_racem);
					}
					else if (c == '8')
					{
						sel -= 5;
						if (sel < 0) sel = n - 1 + sel;
						dump_rmods(sel, racem, max_racem);
					}
					else if (c == '6')
					{
						sel++;
						if (sel >= n) sel = 0;
						dump_rmods(sel, racem, max_racem);
					}
					else if (c == '4')
					{
						sel--;
						if (sel < 0) sel = n - 1;
						dump_rmods(sel, racem, max_racem);
					}
					else if (c == '\r')
					{
						k = sel;
						break;
					}
					else bell();
				}

				/* Set race */
				p_ptr->pracem = racem[k];
			}
			rmp_ptr = &race_mod_info[p_ptr->pracem];

			/* Display */
			auto const race_name = get_player_race_name(p_ptr->prace, p_ptr->pracem);
			c_put_str(TERM_L_BLUE, race_name.c_str(), RACE_ROW, 9);
		}
	}

	/* Clean up */
	clear_from(12);


	/*** Player class ***/
	if (do_quick_start)
	{
		k = game->previous_char.pclass;
		p_ptr->pclass = k;
		cp_ptr = &class_info[p_ptr->pclass];
		k = game->previous_char.spec;
		p_ptr->pspec = k;
		spp_ptr = &class_info[p_ptr->pclass].spec[p_ptr->pspec];
	}
	else
	{
		for (int z = 0; z < 2; z++)
		{
			restrictions[z] = (rp_ptr->choice[z] | rmp_ptr->pclass[z]) & (~rmp_ptr->mclass[z]);
		}

		// Get list of all the classes.
		std::vector<u16b> class_types;
		for (std::size_t i = 0; i < class_info.size(); i++)
		{
			class_types.push_back(i);
		}

		// Sort into display order
		std::stable_sort(
			class_types.begin(),
			class_types.end(),
			[&class_info](auto i, auto j) {
				return class_info[i].display_order_idx < class_info[j].display_order_idx;
			}
		);

		/* Count classes */
		n = class_types.size();

		/* Only one choice = instant choice */
		if (n == 1)
		{
			k = 0;
		}
		else
		{
			/* Dump classes */
			sel = 0;
			dump_classes(class_types, sel, restrictions);

			/* Get a class */
			while (1)
			{
				strnfmt(buf, 200, "Choose a class (%c-%c), * for random, = for options, 8/2/4 for up/down/back: ", I2A(0), (n <= 25) ? I2A(n - 1) : I2D(n - 26-1));
				put_str(buf, 15, 2);
				c = inkey();
				if (c == 'Q') quit(NULL);
				if (c == 'S') return (FALSE);
				if (c == '*')
				{
					k = randint(n) - 1;
					break;
				}
				k = (islower(c) ? A2I(c) : (D2I(c) + 26));
				if ((k >= 0) && (k < n)) break;
				if (c == '?')
				{
					help_class(class_info[class_types[sel]].title);
				}
				else if (c == '=')
				{
					screen_save();
					do_cmd_options_aux(6, "Startup Options", FALSE);
					screen_load();
				}
				else if (c == '2')
				{
					sel += 4;
					if (sel >= n) sel %= 4;
					dump_classes(class_types, sel, restrictions);
				}
				else if (c == '8')
				{
					sel -= 4;
					if (sel < 0) sel = n - 1 -( ( -sel) % 4);
					/* C's modulus operator does not have defined
					 results for negative first values. Damn. */
					dump_classes(class_types, sel, restrictions);
				}
				else if (c == '6')
				{
					sel++;
					if (sel >= n) sel = 0;
					dump_classes(class_types, sel, restrictions);
				}
				else if (c == '4')
				{
					sel--;
					if (sel < 0) sel = n - 1;
					dump_classes(class_types, sel, restrictions);
				}
				else if (c == '\r')
				{
					k = sel;
					break;
				}
				else bell();
			}
		}

		/* Set class */
		p_ptr->pclass = class_types[k];

		/* Choose class spec */
		clear_from(15);

		/* Count choices */
		n = class_info[p_ptr->pclass].spec.size();

		/* Only one choice = auto choice */
		if (n == 1)
			k = 0;
		else
		{
			/* Dump classes spec */
			sel = 0;
			dump_specs(sel);

			/* Get a class */
			while (1)
			{
				strnfmt(buf, 200, "Choose a class specialisation (%c-%c), * for random, = for options, 8/2/4/6 for up/down/left/right: ", I2A(0), (n <= 25) ? I2A(n - 1) : I2D(n - 26-1));
				put_str(buf, 15, 2);
				c = inkey();
				if (c == 'Q') quit(NULL);
				if (c == 'S') return (FALSE);
				if (c == '*')
				{
					k = randint(n) - 1;
					break;
				}
				k = (islower(c) ? A2I(c) : (D2I(c) + 26));
				if ((k >= 0) && (k < n)) break;
				if (c == '?')
				{
					help_class(class_info[p_ptr->pclass].spec[sel].title);
				}
				else if (c == '=')
				{
					screen_save();
					do_cmd_options_aux(6, "Startup Options", FALSE);
					screen_load();
				}
				else if (c == '2')
				{
					sel += 4;
					if (sel >= n) sel = sel - n + 1;
					dump_specs(sel);
				}
				else if (c == '8')
				{
					sel -= 4;
					if (sel < 0) sel = n - 1 + sel;
					dump_specs(sel);
				}
				else if (c == '6')
				{
					sel++;
					if (sel >= n) sel = 0;
					dump_specs(sel);
				}
				else if (c == '4')
				{
					sel--;
					if (sel < 0) sel = n - 1;
					dump_specs(sel);
				}
				else if (c == '\r')
				{
					k = sel;
					break;
				}
				else bell();
			}
		}

		/* Set class spec */
		p_ptr->pspec = k;
	}
	cp_ptr = &class_info[p_ptr->pclass];
	spp_ptr = &class_info[p_ptr->pclass].spec[p_ptr->pspec];

	/* Display */
	c_put_str(TERM_L_BLUE, spp_ptr->title, CLASS_ROW, 9);

	/* Clean up */
	clear_from(15);

	/*** Player god ***/
	if (do_quick_start)
	{
		k = game->previous_char.god;
		p_ptr->pgod = k;
		set_grace(game->previous_char.grace);
	}
	else if (race_flags_p(PR_NO_GOD))
	{
		p_ptr->pgod = GOD_NONE;
	}
	else
	{
		int choice[MAX_GODS];
		int max = 0;

		/* Get the list of possible gods */
		for (n = 0; n < MAX_GODS; n++)
		{
			if (god_enabled(&deity_info[n]) &&
			    ((cp_ptr->gods | spp_ptr->gods) & BIT(n)))
			{
				choice[max++] = n;
			}
		}

		if (!max)
		{
			p_ptr->pgod = GOD_NONE;
		}
		else if (max == 1)
		{
			p_ptr->pgod = choice[0];
		}
		else if (max > 1)
		{
			sel = 0;
			n = dump_gods(sel, choice, max);

			/* Choose */
			while (1)
			{
				strnfmt(buf, 200, "Choose a god (%c-%c), * for a random choice, "
				        "= for options, 8/2/4/6 for movement: ",
				        I2A(0), I2A(max - 1));
				put_str(buf, 19, 2);

				c = inkey();
				if (c == 'Q') quit(NULL);
				if (c == 'S')
				{
					return (FALSE);
				}
				if (c == '*')
				{
					k = choice[randint(max) - 1];
					break;
				}
				k = (islower(c) ? A2I(c) : -1);
				if ((k >= 0) && (k < max))
				{
					k = choice[k];
					break;
				}
				if (c == '?')
				{
					help_god(deity_info[choice[sel]].name);
				}
				else if (c == '=')
				{
					screen_save();
					do_cmd_options_aux(6, "Startup Options", FALSE);
					screen_load();
				}
				else if (c == '2')
				{
					sel += 4;
					if (sel >= n) sel %= 4;
					dump_gods(sel, choice, max);
				}
				else if (c == '8')
				{
					sel -= 4;
					/* C's modulus operator does not have defined
					   results for negative first values. Damn. */
					if (sel < 0) sel = n - 1 -( ( -sel) % 4);
					dump_gods(sel, choice, max);
				}
				else if (c == '6')
				{
					sel++;
					if (sel >= n) sel = 0;
					dump_gods(sel, choice, max);
				}
				else if (c == '4')
				{
					sel--;
					if (sel < 0) sel = n - 1;
					dump_gods(sel, choice, max);
				}
				else if (c == '\r')
				{
					k = choice[sel];
					break;
				}
				else bell();
			}

			/* Set god */
			p_ptr->pgod = k;
			p_ptr->grace = 0;
		}

		/* A god that like us ? more grace ! */
		if (race_flags_p(PR_GOD_FRIEND))
		{
			set_grace(200);
		}
		else
		{
			set_grace(100);
		}
	}

	/* Clean up */
	clear_from(12);

	if (!do_quick_start)
	{
		/* Clear */
		clear_from(15);

		/*  */
		if (get_check("Do you want to modify the options"))
		{
			screen_save();
			do_cmd_options_aux(6, "Startup Options", FALSE);
			screen_load();
		}
	}

	/* Is the player an "astral" being? */
	p_ptr->astral = (race_flags_p(PR_ASTRAL)) ? TRUE : FALSE;

	/*
	 * A note by pelpel. (remove this please)
	 * Be it the new Vanilla way (adult vs. birth options) or
	 * the old one (player_type members), it would be less confusing
	 * to handle birth-only options in a uniform fashion,the above and
	 * the following:
	 * ironman_rooms,
	 * joke_monsters,
	 * always_small_level, and
	 * fate_option
	 */


	/* Set the recall dungeon accordingly */
	dungeon_type = DUNGEON_BASE;
	p_ptr->recall_dungeon = dungeon_type;
	max_dlv[dungeon_type] = d_info[dungeon_type].mindepth;

	if (p_ptr->astral)
	{
		/* Somewhere in the misty mountains */
		dungeon_type = DUNGEON_ASTRAL;
		p_ptr->wilderness_x = DUNGEON_ASTRAL_WILD_X;
		p_ptr->wilderness_y = DUNGEON_ASTRAL_WILD_Y;
	}

	/* Clean up */
	clear_from(10);

	/*** User enters number of quests ***/
	/* Heino Vander Sanden and Jimmy De Laet */

	if (!options->ironman_rooms)
	{
		if (do_quick_start)
		{
			v = game->previous_char.quests;
		}
		else
		{
			/* Extra info */
			Term_putstr(5, 15, -1, TERM_WHITE,
			            "Select the number of optional random quests you'd like to receive.");
			Term_putstr(5, 16, -1, TERM_WHITE,
			            "If you do not want any optional quests, enter 0.");

			/* Ask the number of additional quests */
			while (TRUE)
			{
				put_str(format("Number of quests? (0-%u) ",
				               MAX_RANDOM_QUEST - 1), 20, 2);

				/* Get a the number of additional quest */
				while (TRUE)
				{
					/* Move the cursor */
					put_str("", 20, 27);

					/* Default */
					strcpy(inp, "20");

					/* Get a response (or escape) */
					if (!askfor_aux(inp, 2)) inp[0] = '\0';
					if (inp[0] == '*') v = rand_int(MAX_RANDOM_QUEST);
					else v = atoi(inp);

					/* Break on valid input */
					if ((v < MAX_RANDOM_QUEST) && ( v >= 0 )) break;
				}
				break;
			}

			/* Clear */
			clear_from(15);
		}
	}
	else
	{
		/* NO quests for ironman rooms or persistent levels, since they
		   don't work */
		v = 0;
	}

	/* Set the quest monster hook */
	get_mon_num_hook = monster_quest;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Generate random quests */
	initialize_random_quests(v);
	max_quests = v;

	p_ptr->inside_quest = 0;

	/* Init the plots */
	{
		plots[PLOT_MAIN] = QUEST_NECRO;
		quest[plots[PLOT_MAIN]].status = QUEST_STATUS_TAKEN;

		plots[PLOT_BREE] = QUEST_THIEVES;
		quest[plots[PLOT_BREE]].status = QUEST_STATUS_UNTAKEN;

		plots[PLOT_LORIEN] = QUEST_WOLVES;
		quest[plots[PLOT_LORIEN]].status = QUEST_STATUS_UNTAKEN;

		plots[PLOT_GONDOLIN] = QUEST_DRAGONS;
		quest[plots[PLOT_GONDOLIN]].status = QUEST_STATUS_UNTAKEN;

		plots[PLOT_MINAS] = QUEST_HAUNTED;
		quest[plots[PLOT_MINAS]].status = QUEST_STATUS_UNTAKEN;

		plots[PLOT_KHAZAD] = QUEST_EVIL;
		quest[plots[PLOT_KHAZAD]].status = QUEST_STATUS_UNTAKEN;

		plots[PLOT_OTHER] = QUEST_NULL;
	}

	quest_random_init_hook();

	/* Ok */
	return (TRUE);
}




/*
 * Initial stat costs (initial stats always range from 10 to 18 inclusive).
 */
static const int birth_stat_costs[(18-10) + 1] =
{
	0, 1, 2, 4, 7, 11, 16, 22, 30
};


/*
 * Helper function for 'player_birth()'.
 *
 * This function handles "point-based" character creation.
 *
 * The player selects, for each stat, a value from 10 to 18 (inclusive),
 * each costing a certain amount of points (as above), from a pool of 48
 * available points, to which race/class modifiers are then applied.
 *
 * Each unused point is converted into 100 gold pieces, with a maximum of
 * 600 gp at birth.
 *
 * Taken from V 2.9.0
 */
static bool_ player_birth_aux_point()
{
	int i;

	int row = 3;

	int col = 42;

	int stat = 0;

	int stats[6];

	int cost;

	char ch;

	char buf[80];


	/* Initialize stats */
	for (i = 0; i < 6; i++)
	{
		/* Initial stats */
		stats[i] = 10;
	}


	/* Roll for base hitpoints */
	get_extra();

	/* Get luck */
	p_ptr->luck_base = rp_ptr->luck + rmp_ptr->luck + rand_range( -5, 5);
	p_ptr->luck_max = p_ptr->luck_base;

	/* Interact */
	while (1)
	{
		/* Reset cost */
		cost = 0;

		/* Process stats */
		for (i = 0; i < 6; i++)
		{
			/* Reset stats */
			p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stats[i];

			/* Total cost */
			cost += birth_stat_costs[stats[i] - 10];
		}

		/* Restrict cost */
		if (cost > 48)
		{
			/* Warning */
			bell();

			/* Reduce stat */
			stats[stat]--;

			/* Recompute costs */
			continue;
		}

		/* Gold is inversely proportional to cost */
		p_ptr->au = (100 * (48 - cost)) + 100;

		/* Maximum of 600 gold */
		if (p_ptr->au > 600) p_ptr->au = 600;

		/* Calculate the bonuses and hitpoints */
		p_ptr->update |= (PU_BONUS | PU_HP);

		/* Update stuff */
		update_stuff();

		/* Fully healed */
		p_ptr->chp = p_ptr->mhp;

		/* Fully rested */
		p_ptr->csp = p_ptr->msp;

		/* Display the player */
		display_player(0);

		/* Display the costs header */
		put_str("Cost", row - 2, col + 32);

		/* Display the costs */
		for (i = 0; i < 6; i++)
		{
			/* Display cost */
			strnfmt(buf, 80, "%4d", birth_stat_costs[stats[i] - 10]);
			put_str(buf, row + (i - 1), col + 32);
		}


		/* Prompt XXX XXX XXX */
		strnfmt(buf, 80, "Total Cost %2d/48.  Use 2/8 to move, 4/6 to modify, ESC to accept.", cost);
		prt(buf, 0, 0);

		/* Place cursor just after cost of current stat */
		Term_gotoxy(col + 36, row + stat - 1);

		/* Get key */
		ch = inkey();

		/* Quit */
		if (ch == 'Q') quit(NULL);

		/* Start over */
		if (ch == 'S') return (FALSE);

		/* Done */
		if (ch == ESCAPE) break;

		/* Prev stat */
		if (ch == '8')
		{
			stat = (stat + 6 - 1) % 6;
		}

		/* Next stat */
		if (ch == '2')
		{
			stat = (stat + 1) % 6;
		}

		/* Decrease stat */
		if ((ch == '4') && (stats[stat] > 10))
		{
			stats[stat]--;
		}

		/* Increase stat */
		if ((ch == '6') && (stats[stat] < 18))
		{
			stats[stat]++;
		}
	}


	/* Done */
	return (TRUE);
}

/*
 * Use the autoroller or not to generate a char
 */
static bool_ player_birth_aux_auto()
{
	int i, j, m, v;

	bool_ flag = FALSE;

	bool_ prev = FALSE;

	char c;

	char b1 = '[';

	char b2 = ']';

	char buf[80];

	char inp[80];


	/* Initialize */
	if (options->autoroll)
	{
		int mval[6];


		/* Clear fields */
		auto_round = 0L;
		last_round = 0L;

		/* Clean up */
		clear_from(10);

		/* Prompt for the minimum stats */
		put_str("Enter minimum attribute for: ", 15, 2);

		/* Output the maximum stats */
		for (i = 0; i < 6; i++)
		{
			char stat_buf[15];

			/* Reset the "success" counter */
			stat_match[i] = 0;

			/* Race/Class bonus */
			j = rp_ptr->ps.adj[i] + rmp_ptr->ps.adj[i] + cp_ptr->ps.adj[i];

			/* Obtain the "maximal" stat */
			m = adjust_stat(17, j, TRUE);


			/* Save the maximum */
			mval[i] = m;

			/* Extract a textual format */
			cnv_stat(m, stat_buf);

			strnfmt(inp, 80, "(Max of %s):", stat_buf);

			/* Prepare a prompt */
			strnfmt(buf, 80, "%-5s: %-20s", stat_names[i], inp);

			/* Dump the prompt */
			put_str(buf, 16 + i, 5);
		}

		/* Input the minimum stats */
		for (i = 0; i < 6; i++)
		{
			/* Get a minimum stat */
			while (TRUE)
			{
				char *s;

				/* Move the cursor */
				put_str("", 16 + i, 30);

				/* Default */
				strcpy(inp, "");

				/* Get a response (or escape) */
				if (!askfor_aux(inp, 8)) inp[0] = '\0';

				/* Weirdos stat display .. erm .. I mean, original stat display */
				if (!options->linear_stats)
				{
					/* Hack -- add a fake slash */
					strcat(inp, "/");

					/* Hack -- look for the "slash" */
					s = strchr(inp, '/');

					/* Hack -- Nuke the slash */
					*s++ = '\0';

					/* Hack -- Extract an input */
					v = atoi(inp) + atoi(s);
				}
				else
				{
					int z = atoi(inp);

					if (z <= 18)
						v = z;
					else
					{
						int extra = z - 18;
						v = 18 + (extra * 10);
					}
				}

				/* Break on valid input */
				if (v <= mval[i]) break;
			}

			/* Save the minimum stat */
			stat_limit[i] = (v > 0) ? v : 0;
		}
	}

	/* Roll */
	while (TRUE)
	{
		/* Feedback */
		if (options->autoroll)
		{
			Term_clear();

			put_str("Name :", 2, 1);
			c_put_str(TERM_L_BLUE, game->player_name.c_str(), 2, 9);

			put_str("Race :", 3, 1);
			auto const player_race_name = get_player_race_name(p_ptr->prace, p_ptr->pracem);
			c_put_str(TERM_L_BLUE, player_race_name.c_str(), 3, 9);

			put_str("Class:", 4, 1);
			c_put_str(TERM_L_BLUE, spp_ptr->title, 4, 9);

			/* Label stats */
			put_str("STR:", 2 + A_STR, 61);
			put_str("INT:", 2 + A_INT, 61);
			put_str("WIS:", 2 + A_WIS, 61);
			put_str("DEX:", 2 + A_DEX, 61);
			put_str("CON:", 2 + A_CON, 61);
			put_str("CHR:", 2 + A_CHR, 61);

			/* Note when we started */
			last_round = auto_round;

			/* Indicate the state */
			put_str("(Hit ESC to abort)", 11, 61);

			/* Label count */
			put_str("Round:", 9, 61);
		}

		/* Otherwise just get a character */
		else
		{
			/* Get a new character */
			get_stats();
		}

		/* Auto-roll */
		while (options->autoroll)
		{
			bool_ accept = TRUE;

			/* Get a new character */
			get_stats();

			/* Advance the round */
			auto_round++;

			/* Hack -- Prevent overflow */
			if (auto_round >= 1000000L) break;

			/* Check and count acceptable stats */
			for (i = 0; i < 6; i++)
			{
				/* This stat is okay */
				if (stat_use[i] >= stat_limit[i])
				{
					stat_match[i]++;
				}

				/* This stat is not okay */
				else
				{
					accept = FALSE;
				}
			}

			/* Break if "happy" */
			if (accept) break;

			/* Take note every 25 rolls */
			flag = (!(auto_round % AUTOROLLER_STEP));

			/* Update display occasionally */
			if (flag || (auto_round < last_round + 100))
			{
				/* Dump data */
				birth_put_stats();

				/* Dump round */
				put_str(format("%6ld", auto_round), 9, 73);

				/* Make sure they see everything */
				Term_fresh();

				/* Check for a keypress */
				if (inkey_scan()) break;
			}
		}

		/* Flush input */
		flush();


		/*** Display ***/

		/* Roll for base hitpoints */
		get_extra();

		/* Roll for gold */
		get_money();

		/* Input loop */
		while (TRUE)
		{
			/* Calculate the bonuses and hitpoints */
			p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_BODY);

			/* Update stuff */
			update_stuff();

			/* Fully healed */
			p_ptr->chp = p_ptr->mhp;

			/* Fully rested */
			p_ptr->csp = p_ptr->msp;

			/* Display the player */
			display_player(0);

			/* Prepare a prompt (must squeeze everything in) */
			Term_gotoxy(2, 23);
			Term_addch(TERM_WHITE, b1);
			Term_addstr( -1, TERM_WHITE, "'r' to reroll");
			if (prev) Term_addstr( -1, TERM_WHITE, ", 'p' for prev");
			Term_addstr( -1, TERM_WHITE, ", or ESC to accept");
			Term_addch(TERM_WHITE, b2);

			/* Prompt and get a command */
			c = inkey();

			/* Quit */
			if (c == 'Q') quit(NULL);

			/* Start over */
			if (c == 'S') return (FALSE);

			/* Escape accepts the roll */
			if (c == ESCAPE) break;

			/* Reroll this character */
			if ((c == ' ') || (c == 'r')) break;

			/* Previous character */
			if (prev && (c == 'p'))
			{
				load_prev_data(TRUE);
				continue;
			}

			/* Help */
			if (c == '?')
			{
				do_cmd_help();
				continue;
			}

			/* Warning */
			bell();
		}

		/* Are we done? */
		if (c == ESCAPE) break;

		/* Save this for the "previous" character */
		save_prev_data();

		/* Note that a previous roll exists */
		prev = TRUE;
	}

	/* Clear prompt */
	clear_from(23);

	return (TRUE);
}


/*
 * Helper function for 'player_birth()'
 *
 * The delay may be reduced, but is recommended to keep players
 * from continuously rolling up characters, which can be VERY
 * expensive CPU wise.  And it cuts down on player stupidity.
 */
static bool_ player_birth_aux()
{
	auto const &s_descriptors = game->edit_data.s_descriptors;
	auto &s_info = game->s_info;

	char c;

	/* Ask */
	if (!player_birth_aux_ask()) return (FALSE);

	for (std::size_t i = 1; i < s_descriptors.size(); i++)
	{
		s_info[i].dev = false;
	}

	for (std::size_t i = 1; i < s_descriptors.size(); i++)
	{
		s32b value = 0, mod = 0;

		compute_skills(&value, &mod, i);

		init_skill(value, mod, i);

		/* Develop only revelant branches */
		if (s_info[i].value || s_info[i].mod)
		{
			int z = s_descriptors[i].father;

			while (z != -1)
			{
				// Mark as developed
				s_info[z].dev = true;
				// Next node up the tree
				z = s_descriptors[z].father;
				if (z == 0)
				{
					break;
				}
			}
		}
	}

	if (do_quick_start)
	{
		load_prev_data(FALSE);

		/* Roll for base hitpoints */
		get_extra();

		/* Calculate the bonuses and hitpoints */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_BODY);

		/* Update stuff */
		update_stuff();

		/* Fully healed */
		p_ptr->chp = p_ptr->mhp;

		/* Fully rested */
		p_ptr->csp = p_ptr->msp;
	}
	else
	{
		/* Point based */
		if (options->point_based)
		{
			if (!player_birth_aux_point()) return FALSE;
		}
		/* Auto-roll */
		else
		{
			if (!player_birth_aux_auto()) return FALSE;
		}

		/*** Finish up ***/

		/* Get a name, recolor it, prepare savefile */
		get_name();


		/* Prompt for it */
		prt("['Q' to suicide, 'S' to start over, or ESC to continue]", 23, 10);

		/* Get a key */
		c = inkey();

		/* Quit */
		if (c == 'Q') quit(NULL);

		/* Start over */
		if (c == 'S') return (FALSE);
	}

	/* Save this for the next character */
	game->previous_char.quick_ok = TRUE;
	save_prev_data();

	/* Accept */
	return (TRUE);
}


/*
 * Initialize a random town
 */
static void init_town(int t_idx)
{
	town_type *t_ptr = &town_info[t_idx];

	/* Mark it as existent */
	t_ptr->flags |= (TOWN_REAL);

	/* Mark it as not found */
	t_ptr->flags &= ~(TOWN_KNOWN);

	/* Generation seed for the town */
	t_ptr->seed = seed_t::system();
}

/*
 * Create a new character.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth()
{
	auto const &st_info = game->edit_data.st_info;
	auto &d_info = game->edit_data.d_info;
	auto &messages = game->messages;

	/* Starting index for generated towns */
	std::size_t rtown = TOWN_RANDOM;

	/* Create a new character */
	while (1)
	{
		/* Wipe the player */
		player_wipe();

		/* Roll up a new character */
		if (player_birth_aux()) break;
	}

	/* Finish skills */
	p_ptr->skill_points = 0;
	p_ptr->skill_last_level = 1;

	recalc_skills(FALSE);

	/* grab level 1 abilities */
	p_ptr->abilities.clear();
	apply_level_abilities(1);

	/* Complete the god */
	{
		byte i = p_ptr->pgod;
		p_ptr->pgod = 0;
		follow_god(i, TRUE);
	}

	/* Select the default melee type */
	select_default_melee();

	/* Make a note file if that option is set */
	add_note_type(NOTE_BIRTH);

	/* Note player birth in the message recall */
	messages.add(" ", TERM_L_BLUE);
	messages.add("  ", TERM_L_BLUE);
	messages.add("====================", TERM_L_BLUE);
	messages.add("  ", TERM_L_BLUE);
	messages.add(" ", TERM_L_BLUE);

	/* Hack -- outfit the player */
	player_outfit();

	/* Initialize random towns in the dungeons */
	for (std::size_t i = 0; i < d_info.size(); i++)
	{
		auto d_ptr = &d_info[i];
		int num = 0, z;

		d_ptr->t_num = 0;
		for (z = 0; z < TOWN_DUNGEON; z++)
		{
			d_ptr->t_idx[z] = 0;
			d_ptr->t_level[z] = 0;
		}
		if (!(d_ptr->flags & DF_RANDOM_TOWNS)) continue;

		/* Can we add a town ? */
		while (magik(TOWN_CHANCE - (num * 10)))
		{
			int lev;

			d_ptr->t_idx[num] = rtown;
			rtown++;

			while (TRUE)
			{
				bool_ ok = TRUE;

				lev = rand_range(d_ptr->mindepth, d_ptr->maxdepth - 1);

				/* Be sure it wasnt already used */
				for (int j = 0; j < num; j++)
				{
					if (d_ptr->t_level[j] == lev)
					{
						ok = FALSE;
					}
				}

				/* Ok found one */
				if (ok) break;
			}
			d_ptr->t_level[num] = lev;

			if (wizard)
			{
				messages.add(format("Random dungeon town: d_idx:%d, lev:%d", i, lev), TERM_WHITE);
			}

			/* Create the town */
			init_town(d_ptr->t_idx[num]);

			num++;

			/* No free slots left */
			if (num >= TOWN_DUNGEON) break;
		}

		d_ptr->t_num = num;
	}

	/* Init the towns */
	for (std::size_t i = 1; i < max_towns; i++)
	{
		/* Not destroyed ! yet .. ;) */
		town_info[i].destroyed = FALSE;

		/* Ignore non-existent towns */
		if (!(town_info[i].flags & (TOWN_REAL))) continue;

		create_stores_stock(i);

		/* Init the stores */
		for (std::size_t j = 0; j < st_info.size(); j++)
		{
			/* Initialize */
			store_init(i, j);
		}
	}

	/* Init wilderness seeds */
	auto &wilderness = game->wilderness;
	for (std::size_t y = 0; y < wilderness.height(); y++)
	{
		for (std::size_t x = 0; x < wilderness.width(); x++)
		{
			auto &w = wilderness(x, y);
			w.seed = seed_t::system();
			w.entrance = 0;
			w.known = FALSE;
		}
	}
}




static char savefile_module[46][80];
static char savefile_names[46][30];
static char savefile_desc[46][80];
static bool_ savefile_alive[46];
static int savefile_idx[46];

/*
 * Grab all the names from an index
 */
int load_savefile_names()
{
	FILE *fff;
	char buf[1024];
	char tmp[50];
	std::string player_base_save;
	int max = 0, fd;


	/* Build the filename */
	strcpy(tmp, "global.svg");
	path_build(buf, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Read the file */
	fff = my_fopen(buf, "r");

	/* Failure */
	if (!fff) return (0);


	/* Save the current 'player_base' */
	player_base_save = game->player_base;


	/*
	 * Parse, use '@' intead of ':' as a separator because it cannot exists
	 * in savefiles
	 */
	while (0 == my_fgets(fff, buf, 1024))
	{
		int i = 0, start, count;

		/* Check for pre-ToME 2.1.2 file */
		count = 0;
		i = 0;
		while (buf[i] && buf[i] != '\n')
		{
			if (buf[i] == '@')
				++count;
			++i;
		}

		/* Check module if a current svg file */
		start = 0;
		i = 0;
		if (count > 1)
		{
			while (buf[i] != '@')
			{
				savefile_module[max][i - start] = buf[i];
				i++;
			}
			savefile_module[max][i] = '\0';
			i++;
		}
		/* Default to ToME for old files */
		else
		{
			savefile_module[max][0] = 'T';
			savefile_module[max][1] = 'o';
			savefile_module[max][2] = 'M';
			savefile_module[max][3] = 'E';
			savefile_module[max][4] = '\0';
		}

		if (buf[i] == '0') savefile_alive[max] = FALSE;
		else if (buf[i] == '1') savefile_alive[max] = TRUE;

		i++;
		start = i;
		while (buf[i] != '@')
		{
			savefile_names[max][i - start] = buf[i];
			i++;
		}
		savefile_names[max][i - start] = '\0';
		i++;
		strcpy(savefile_desc[max], buf + i);

		/* Build platform-dependent savefile name */
		game->player_base = savefile_names[max];
		process_player_name(TRUE);

		/* Try to open the savefile */
		fd = fd_open(savefile, O_RDONLY);

		/* Still existing ? */
		if (fd >= 0)
		{
			fd_close(fd);
			max++;
		}
	}

	my_fclose(fff);

	/* Restore the values of 'player_base' and 'savefile' */
	game->player_base  = player_base_save;
	process_player_name(TRUE);

	return (max);
}


/*
 * Save all the names from an index
 */
void save_savefile_names()
{
	FILE *fff;
	char buf[1024];
	char tmp[50];
	int max = load_savefile_names(), i;


	/* Build the filename */
	strcpy(tmp, "global.svg");
	path_build(buf, 1024, ANGBAND_DIR_SAVE, tmp);

	/* Read the file */
	fff = my_fopen(buf, "w");

	/* Failure */
	if (!fff) return;

	/*
	 * Save, use '@' intead of ':' as a separator because it cannot exists
	 * in savefiles
	 */
	auto const player_race_name = get_player_race_name(p_ptr->prace, p_ptr->pracem);
	fprintf(fff, "%s@%c%s@%s, the %s %s is %s\n", game_module,
		(death) ? '0' : '1', game->player_base.c_str(), game->player_name.c_str(),
		player_race_name.c_str(),
		spp_ptr->title,
	        (!death) ? "alive" : "dead");

	for (i = 0; i < max; i++)
	{
		if (!strcmp(savefile_names[i], game->player_base.c_str())) continue;
		fprintf(fff, "%s@%c%s@%s\n", savefile_module[i],
		        (savefile_alive[i]) ? '1' : '0', savefile_names[i], savefile_desc[i]);
	}

	my_fclose(fff);
}


static void dump_savefiles(int sel, int max)
{
	int i;

	char buf[40], pre = ' ', post = ')';

	char ind;


	for (i = 0; i < max; i++)
	{
		ind = I2A(i % 26);
		if (i >= 26) ind = toupper(ind);

		if (sel == i)
		{
			pre = '[';
			post = ']';
		}
		else
		{
			pre = ' ';
			post = ')';
		}

		if (i == 0) strnfmt(buf, 40, "%c%c%c New Character", pre, ind, post);
		else if (i == 1) strnfmt(buf, 40, "%c%c%c Load Savefile", pre, ind, post);
		else strnfmt(buf, 40, "%c%c%c %s", pre, ind, post, savefile_names[savefile_idx[i - 2]]);

		if (sel == i)
		{
			if (i >= 2)
			{
				if (savefile_alive[i - 2]) c_put_str(TERM_L_GREEN, savefile_desc[savefile_idx[i - 2]], 5, 0);
				else c_put_str(TERM_L_RED, savefile_desc[savefile_idx[i - 2]], 5, 0);
			}
			else if (i == 1) c_put_str(TERM_YELLOW, "Load an existing savefile that is not in the list", 5, 0);
			else c_put_str(TERM_YELLOW, "Create a new character", 5, 0);
			c_put_str(TERM_L_BLUE, buf, 6 + (i / 4), 20 * (i % 4));
		}
		else
			put_str(buf, 6 + (i / 4), 20 * (i % 4));
	}
}


/* Asks for new game or load game */
bool_ no_begin_screen = FALSE;

bool_ begin_screen()
{
	int m, k, sel, max;

savefile_try_again:
	sel = 0;

	/* Grab the savefiles */
	max = load_savefile_names();

	/* Get only the usable savefiles */
	for (k = 0, m = 0; k < max; k++)
	{
		s32b can_use;

		can_use = module_savefile_loadable(savefile_module[k]);
		if (can_use)
		{
			savefile_idx[m++] = k;
		}
	}
	max = m + 2;
	if (max > 2) sel = 2;

	while (TRUE)
	{
		/* Clear screen */
		Term_clear();

		/* Let the user choose */
		c_put_str(TERM_YELLOW, format("Welcome to %s!  To play you will need a character.", game_module), 1, 10);
		put_str("Press 8/2/4/6 to move, Return to select, Backspace to delete a savefile.", 3, 3);
		put_str("and Esc to quit.", 4, 32);

		dump_savefiles(sel, max);

		k = inkey();

		if (k == ESCAPE)
		{
			quit(NULL);
		}
		if (k == '6')
		{
			sel++;
			if (sel >= max) sel = 0;
			continue;
		}
		else if (k == '4')
		{
			sel--;
			if (sel < 0) sel = max - 1;
			continue;
		}
		else if (k == '2')
		{
			sel += 4;
			if (sel >= max) sel = sel % max;
			continue;
		}
		else if (k == '8')
		{
			sel -= 4;
			if (sel < 0) sel = (sel + max - 1) % max;
			continue;
		}
		else if (k == '\r')
		{
			if (sel < 26) k = I2A(sel);
			else k = toupper(I2A(sel));
		}
		else if (((k == 0x7F) || (k == '\010')) && (sel >= 2))
		{
			if (!get_check(format("Really delete '%s'?", savefile_names[savefile_idx[sel - 2]]))) continue;

			/* Save current 'player_base' */
			std::string player_base_save = game->player_base;

			/* Build platform-dependent save file name */
			game->player_base = savefile_names[savefile_idx[sel - 2]];
			process_player_name(TRUE);

			/* Remove the savefile */
			fd_kill(savefile);

			/* Restore 'player_base' and 'savefile' */
			game->player_base = player_base_save;
			process_player_name(TRUE);

			/* Reload, gods I hate using goto .. */
			goto savefile_try_again;

			continue;
		}

		//
		// React
		//

		if (k == 'a')
		{
			/* Display prompt */
			prt("Enter the name of the savefile that will hold this character: ", 23, 0);

			/* Ask the user for a string */
			if (!askfor_aux(&game->player_base, 15)) continue;

			/* Process the player name */
			process_player_name(TRUE);

			// If the savefile already exists, we do *NOT* want to
			// create a new game, so we'll need to return FALSE for
			// that.
			if (boost::filesystem::exists(savefile))
			{
				// Show a message so user doesn't get confused.
				msg_print(NULL);

				// Prompt for it
				prt(fmt::format(
					"Character '{}' already exists! Press any key to load.",
					game->player_base),
					0, 0);

				// Wait
				inkey();

				// Erase the prompt
				prt("", 0, 0);

				// Load character
				return FALSE;
			}
			else
			{
				// Start new game
				return TRUE;
			}
		}
		if (k == 'b')
		{
			/* Display prompt */
			prt("Enter the name of a savefile: ", 23, 0);

			/* Ask the user for a string */
			if (!askfor_aux(&game->player_base, 15)) continue;

			/* Process the player name */
			process_player_name(TRUE);

			return (FALSE);
		}
		else
		{
			int x;

			if (islower(k)) x = A2I(k);
			else x = A2I(tolower(k)) + 26;

			if ((x < 2) || (x >= max)) continue;

			game->player_base = savefile_names[savefile_idx[x - 2]];

			/* Process the player name */
			process_player_name(TRUE);

			return (FALSE);
		}
	}

	/* Shouldnt happen */
	return (FALSE);
}
