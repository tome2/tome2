#include "q_library.hpp"

#include "cave_type.hpp"
#include "dungeon_flag.hpp"
#include "game.hpp"
#include "hooks.hpp"
#include "hook_quest_gen_in.hpp"
#include "lua_bind.hpp"
#include "monster2.hpp"
#include "monster_type.hpp"
#include "object2.hpp"
#include "player_type.hpp"
#include "spells3.hpp"
#include "spells4.hpp"
#include "tables.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-rand.hpp"

#include <cassert>
#include <fmt/format.h>

#define cquest (quest[QUEST_LIBRARY])

#define MONSTER_LICH 518
#define MONSTER_MONASTIC_LICH 611
#define MONSTER_FLESH_GOLEM 256
#define MONSTER_CLAY_GOLEM 261
#define MONSTER_IRON_GOLEM 367
#define MONSTER_MITHRIL_GOLEM 464

#define MAX_BOOKABLE_SPELLS 128

static s16b bookable_spells[MAX_BOOKABLE_SPELLS];
static s16b bookable_spells_size = 0;

static void push_spell(s16b spell_idx)
{
	assert(bookable_spells_size < MAX_BOOKABLE_SPELLS);
	bookable_spells[bookable_spells_size++] = spell_idx;
}

void initialize_bookable_spells()
{
	push_spell(MANATHRUST);
	push_spell(DELCURSES);
	push_spell(GLOBELIGHT);
	push_spell(FIREGOLEM);
	push_spell(FIREFLASH);
	push_spell(FIREWALL);
	push_spell(GEYSER);
	push_spell(VAPOR);
	push_spell(ENTPOTION);
	push_spell(NOXIOUSCLOUD);
	push_spell(POISONBLOOD);
	push_spell(STONESKIN);
	push_spell(DIG);
	push_spell(RECHARGE);
	push_spell(DISPERSEMAGIC);
	push_spell(BLINK);
	push_spell(TELEPORT);
	push_spell(SENSEMONSTERS);
	push_spell(SENSEHIDDEN);
	push_spell(REVEALWAYS);
	push_spell(VISION);
	push_spell(MAGELOCK);
	push_spell(SLOWMONSTER);
	push_spell(ESSENCESPEED);
	push_spell(CHARM);
	push_spell(CONFUSE);
	push_spell(ARMOROFFEAR);
	push_spell(STUN);
	push_spell(GROWTREE);
	push_spell(HEALING);
	push_spell(RECOVERY);
	push_spell(ERU_SEE);
	push_spell(ERU_LISTEN);
	push_spell(MANWE_BLESS);
	push_spell(MANWE_SHIELD);
	push_spell(YAVANNA_CHARM_ANIMAL);
	push_spell(YAVANNA_GROW_GRASS);
	push_spell(YAVANNA_TREE_ROOTS);
	push_spell(TULKAS_AIM);
	push_spell(TULKAS_SPIN);
	push_spell(MELKOR_CURSE);
	push_spell(MELKOR_CORPSE_EXPLOSION);
	push_spell(DRAIN);

	if (game_module_idx == MODULE_THEME)
	{
		push_spell(AULE_FIREBRAND);
		push_spell(AULE_CHILD);
		push_spell(VARDA_LIGHT_VALINOR);
		push_spell(VARDA_EVENSTAR);
		push_spell(ULMO_BELEGAER);
		push_spell(ULMO_WRATH);
		push_spell(MANDOS_TEARS_LUTHIEN);
		push_spell(MANDOS_TALE_DOOM);
	}
}

static s16b library_quest_place_random(int minY, int minX, int maxY, int maxX, int r_idx)
{
	int y = randint(maxY - minY + 1) + minY;
	int x = randint(maxX - minX + 1) + minX;
	return place_monster_one(y, x, r_idx, 0, true, MSTATUS_ENEMY);
}

static void library_quest_place_nrandom(int minY, int minX, int maxY, int maxX, int r_idx, int n)
{
	while(n > 0)
	{
		if (0 < library_quest_place_random(minY, minX, maxY, maxX, r_idx))
		{
			n--;
		}
	}
}

static s32b library_quest_book_get_slot(int slot)
{
	return cquest.data[slot-1];
}

static void library_quest_book_set_slot(int slot, s32b spell)
{
	cquest.data[slot-1] = spell;
}

static int library_quest_book_slots_left()
{
	if (library_quest_book_get_slot(1) == -1) {
		return 3;
	} else if (library_quest_book_get_slot(2) == -1) {
		return 2;
	} else if (library_quest_book_get_slot(3) == -1) {
		return 1;
	} else {
		return 0;
	}
}

static bool library_quest_book_contains_spell(int spell)
{
	int i;
	for (i = 1; i <= 3; i++)
	{
		if (library_quest_book_get_slot(i) == spell)
		{
			return true;
		}
	}
	return false;
}

static void quest_library_finalize_book()
{
	int i = 0;
	for (i = 1; i <= 3; i++)
	{
		school_book *school_book = school_books_at(BOOK_PLAYER);
		school_book_add_spell(school_book, library_quest_book_get_slot(i));
	}
}

static void library_quest_add_spell(int spell) {
	if (library_quest_book_get_slot(1) == -1) {
		library_quest_book_set_slot(1, spell);
	} else if (library_quest_book_get_slot(2) == -1) {
		library_quest_book_set_slot(2, spell);
	} else if (library_quest_book_get_slot(3) == -1) {
		library_quest_book_set_slot(3, spell);
	}
}

static void library_quest_remove_spell(int spell) {
	if (library_quest_book_get_slot(1) == spell) {
		library_quest_book_set_slot(1, library_quest_book_get_slot(2));
		library_quest_book_set_slot(2, library_quest_book_get_slot(3));
		library_quest_book_set_slot(3, -1);
	} else if (library_quest_book_get_slot(2) == spell) {
		library_quest_book_set_slot(2, library_quest_book_get_slot(3));
		library_quest_book_set_slot(3, -1);
	} else if (library_quest_book_get_slot(3) == spell) {
		library_quest_book_set_slot(3, -1);
	}
}

/* spell selection routines inspired by skills.c */
static void library_quest_print_spells(int first, int current)
{
	int width, height;
	int slots, row;
	int index;

	Term_clear();
	Term_get_size(&width, &height);

	slots = library_quest_book_slots_left();

	c_prt(TERM_WHITE, "Book Creation Screen", 0, 0);
	c_prt(TERM_WHITE, "Up/Down to move, Right/Left to modify, I to describe, Esc to Save/Cancel", 1, 0);

	if (slots == 0) {
		c_prt(TERM_L_RED, "The book can hold no more spells.", 2, 0);
	} else if (slots == 1) {
		c_prt(TERM_L_BLUE, "The book can hold 1 more spell.", 2, 0);
	} else {
		c_prt(TERM_L_BLUE, fmt::format("The book can hold {} more spells.", slots), 2, 0);
	}

	row = 3;

	for (index = 0; index < bookable_spells_size; index++) {
		int spell = bookable_spells[index];
		if (index >= first) {
			int color;
			if (index == current) {
				color = TERM_GREEN;
			} else if (library_quest_book_contains_spell(spell)) {
				color = TERM_WHITE;
			} else {
				color = TERM_ORANGE;
			}

			print_spell(NULL, color, row, spell);

			if (row == height - 1) {
				return;
			}
			row = row + 1;
		}
	}
}

static void library_quest_fill_book()
{
	int width, height, margin, first, current;
	bool done;

	/* Always start with a cleared book */
	library_quest_book_set_slot(1, -1);
	library_quest_book_set_slot(2, -1);
	library_quest_book_set_slot(3, -1);

	screen_save();
	Term_get_size(&width, &height);

	/* room for legend */
	margin = 3;

	first = 0;
	current = 0;
	done = false;

	while (done == false)
	{
		char ch;
		int dir, spell_idx;

		library_quest_print_spells(first, current);

		ch = inkey();
		dir = get_keymap_dir(ch);

		spell_idx = bookable_spells[current];

		if (ch == ESCAPE) {
			if (library_quest_book_slots_left() == 0) {
				flush();
				done = get_check("Really create the book?");
			} else {
				done = true;
			}
		} else if (ch == '\r') {
			/* TODO: make tree of schools */
		} else if (ch == 'n') {
			current = current + height;
		} else if (ch == 'p') {
			current = current - height;
		} else if (ch == 'I') {
			print_spell_desc(spell_idx, 0);
			inkey();
		} else if (dir == 2) {
			current = current + 1;
		} else if (dir == 8) {
			current = current - 1;
		} else if (dir == 6) {
			if (library_quest_book_contains_spell(spell_idx) == false)
			{
				library_quest_add_spell(spell_idx);
			}
		} else if (dir == 4) {
			library_quest_remove_spell(spell_idx);
		}

		if (current >= bookable_spells_size) {
			current = bookable_spells_size - 1;
		} else if (current < 0) {
			current = 0;
		}
		
		if (current > (first + height - margin - 1)) {
			first = current - height + margin + 1;
		} else if (first > current) {
			first = current;
		}
	}

	screen_load();
}

static bool quest_library_gen_hook(void *, void *in_, void *)
{
	auto in = static_cast<hook_quest_gen_in *>(in_);

	/* Only if player doing this quest */
	if (p_ptr->inside_quest != QUEST_LIBRARY)
	{
		return false;
	}

	{
		int y = 2;
		int x = 2;
		load_map("library.map", &y, &x);
		in->dungeon_flags_ref |= DF_NO_GENO;
	}

	/* Generate monsters */
	library_quest_place_nrandom(
		4, 4, 14, 37, MONSTER_LICH, damroll(4,2));

	library_quest_place_nrandom(
		14, 34, 37, 67, MONSTER_MONASTIC_LICH, damroll(1, 2));

	library_quest_place_nrandom(
		4, 34, 14, 67, MONSTER_MONASTIC_LICH, damroll(1, 2) - 1);

	library_quest_place_nrandom(
		14, 4, 37, 34, MONSTER_MONASTIC_LICH, damroll(1, 2) - 1);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_FLESH_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_CLAY_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_IRON_GOLEM, 2);

	library_quest_place_nrandom(
		10, 10, 37, 67, MONSTER_MITHRIL_GOLEM, 1);

	return true;
}

static bool quest_library_stair_hook(void *, void *, void *)
{
	/* only ask this if player about to go up stairs of quest and hasn't won yet */
	if ((p_ptr->inside_quest != QUEST_LIBRARY) ||
	    (cquest.status == QUEST_STATUS_COMPLETED))
	{
		return false;
	}

	if (cave[p_ptr->py][p_ptr->px].feat != FEAT_LESS)
	{
		return false;
	}

	/* flush all pending input */
	flush();

	/* confirm */
	if (get_check("Really abandon the quest?"))
	{
		/* fail the quest */
		cquest.status = QUEST_STATUS_FAILED;
		return false;
	}
	else
	{
		/* if no, they stay in the quest */
		return true;
	}
}

static bool quest_library_monster_death_hook(void *, void *, void *)
{
	int i, count = -1;

	/* if they're in the quest and haven't won, continue */
	if ((p_ptr->inside_quest != QUEST_LIBRARY) ||
	    (cquest.status == QUEST_STATUS_COMPLETED))
	{
		return false;
	}

	/* Count all the enemies left alive */
	for (i = 0; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		if ((m_ptr->r_idx > 0) &&
		    (m_ptr->status <= MSTATUS_ENEMY))
		{
			count = count + 1;
		}
	}

	/* We've just killed the last monster */
	if (count == 0)
	{
		cquest.status = QUEST_STATUS_COMPLETED;
		cmsg_print(TERM_YELLOW, "The library is safe now.");
	}

	/* Normal processing */
	return false;
}

void quest_library_building(bool *paid, bool *recreate)
{
	int status = cquest.status;

	/* the quest hasn't been requested already, right? */
	if (status == QUEST_STATUS_UNTAKEN)
	{
		/* quest has been taken now */
		cquest.status = QUEST_STATUS_TAKEN;

		/* issue instructions */
		msg_print("I need get some stock from my main library, but it is infested with monsters!");
		msg_print("Please use the side entrance and vanquish the intruders for me.");

		*paid = false;
		*recreate = true;
	}

	/* if quest completed */
	else if (status == QUEST_STATUS_COMPLETED)
	{
		msg_print("Thank you!  Let me make a special book for you.");
		msg_print("Tell me three spells and I will write them in the book.");
		library_quest_fill_book();
		if (library_quest_book_slots_left() == 0)
		{
			cquest.status = QUEST_STATUS_REWARDED;

			{
				object_type forge;
				object_type *q_ptr = &forge;
				object_prep(q_ptr, lookup_kind(TV_BOOK, 61));
				q_ptr->artifact_name = game->player_name;
				q_ptr->found = OBJ_FOUND_REWARD;
				inven_carry(q_ptr, false);
			}

			quest_library_finalize_book();
		}
	}

	/* if the player asks for a quest when they already have it,
	 * but haven't failed it, give them some extra instructions */
	else if (status == QUEST_STATUS_TAKEN)
	{
		msg_print("Please use the side entrance and vanquish the intruders for me.");
	}

	/* quest failed or completed, then give no more quests */
	else if ((status == QUEST_STATUS_FAILED) || (status == QUEST_STATUS_REWARDED))
	{
		msg_print("I have no more quests for you.");
	}
}

std::string quest_library_describe()
{
	fmt::MemoryWriter w;

	if (cquest.status == QUEST_STATUS_TAKEN)
	{
		w.write("#####yAn Old Mages Quest! (Danger Level: 35)\n");
		w.write("Make the library safe for the old mage in Minas Anor.");
	}
	else if (cquest.status == QUEST_STATUS_COMPLETED)
	{
		w.write("#####yAn Old Mages Quest!\n");
		w.write("You have made the library safe for the old mage in Minas Anor.\n");
		w.write("Perhaps you should see about a reward.");
	}

	return w.str();
}

void quest_library_init_hook()
{
	/* Only need hooks if the quest is unfinished. */
	if ((cquest.status >= QUEST_STATUS_UNTAKEN) &&
	    (cquest.status < QUEST_STATUS_FINISHED))
	{
		add_hook_new(HOOK_GEN_QUEST    , quest_library_gen_hook          , "library_gen_hook",           NULL);
		add_hook_new(HOOK_STAIR        , quest_library_stair_hook        , "library_stair_hook",         NULL);
		add_hook_new(HOOK_MONSTER_DEATH, quest_library_monster_death_hook, "library_monster_death_hook", NULL);
	}

	/* If quest was rewarded we need to initialize the real player's spellbook. */
	if (cquest.status == QUEST_STATUS_REWARDED)
	{
		quest_library_finalize_book();
	}
}
