#undef cquest

#define MONSTER_LICH 518
#define MONSTER_MONASTIC_LICH 611
#define MONSTER_FLESH_GOLEM 256
#define MONSTER_CLAY_GOLEM 261
#define MONSTER_IRON_GOLEM 367
#define MONSTER_MITHRIL_GOLEM 464

static s16b library_quest_place_random(int minY, int minX, int maxY, int maxX, int r_idx)
{
	int y = randint(maxY - minY + 1) + minY;
	int x = randint(maxX - minX + 1) + minX;
	return place_monster_one(y, x, r_idx, 0, TRUE, MSTATUS_ENEMY);
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

static int library_quest_book_get_slot(int slot)
{
	return exec_lua(format("return school_book[61][%d]", slot));
}

static int library_quest_book_set_slot(int slot, int spell)
{
	return exec_lua(format("school_book[61][%d] = %d", slot, spell));
}

int library_quest_book_slots_left()
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

static bool_ library_quest_book_contains_spell(int spell)
{
	return exec_lua(format("return spell_in_book(61, %d)", spell));
}

static int library_quest_bookable_spells_at(int i) {
	return exec_lua(format("return library_quest.bookable_spells[%d]", i + 1));
}

static int library_quest_getn_bookable_spells() {
	return exec_lua("return getn(library_quest.bookable_spells)");
}

static int library_quest_print_spell(int color, int row, int spell) {
	return exec_lua(format("library_quest.print_spell(%d,%d,%d)", color, row, spell));
}

static int library_quest_print_spell_desc(int s, int y) {
	return exec_lua(format("print_spell_desc(%d, %d)", s, y));
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
	int nspells, index;

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
		c_prt(TERM_L_BLUE, format("The book can hold %d more spells.", slots), 2, 0);
	}

	row = 3;

	nspells = library_quest_getn_bookable_spells();
	for (index = 0; index < nspells; index++) {
		int spell = library_quest_bookable_spells_at(index);
		if (index >= first) {
			int color;
			if (index == current) {
				color = TERM_GREEN;
			} else if (library_quest_book_contains_spell(spell)) {
				color = TERM_WHITE;
			} else {
				color = TERM_ORANGE;
			}

			library_quest_print_spell(color, row, spell);

			if (row == height - 1) {
				return;
			}
			row = row + 1;
		}
	}
}

void library_quest_fill_book()
{
	int width, height, margin, first, current;
	bool_ done;

	/* Always start with a cleared book */
	exec_lua("school_book[61] = {-1, -1, -1}");

	screen_save();
	Term_get_size(&width, &height);

	/* room for legend */
	margin = 3;

	first = 0;
	current = 0;
	done = FALSE;

	while (done == FALSE)
	{
		char ch;
		int dir, total;

		library_quest_print_spells(first, current);

		inkey_scan = FALSE;
		ch = inkey();
		dir = get_keymap_dir(ch);

		if (ch == ESCAPE) {
			if (library_quest_book_slots_left() == 0) {
				flush();
				done = get_check("Really create the book?");
			} else {
				done = TRUE;
			}
		} else if (ch == '\r') {
			/* TODO: make tree of schools */
		} else if (ch == 'n') {
			current = current + height;
		} else if (ch == 'p') {
			current = current - height;
		} else if (ch == 'I') {
			library_quest_print_spell_desc(library_quest_bookable_spells_at(current), 0);
			inkey();
		} else if (dir == 2) {
			current = current + 1;
		} else if (dir == 8) {
			current = current - 1;
		} else if (dir == 6) {
			if (library_quest_book_contains_spell(library_quest_bookable_spells_at(current)) == FALSE) {
				library_quest_add_spell(library_quest_bookable_spells_at(current));
			}
		} else if (dir == 4) {
			library_quest_remove_spell(library_quest_bookable_spells_at(current));
		}

		total = library_quest_getn_bookable_spells();
		if (current >= total) {
			current = total - 1;
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

void quest_library_gen_hook()
{
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
}
