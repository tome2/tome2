#include "angband.h"

#include <assert.h>

#include "spell_type.h"

school_book_type school_books[SCHOOL_BOOKS_SIZE];

s32b SCHOOL_AIR;
s32b SCHOOL_AULE;
s32b SCHOOL_CONVEYANCE;
s32b SCHOOL_DEMON;
s32b SCHOOL_DEVICE;
s32b SCHOOL_DIVINATION;
s32b SCHOOL_EARTH;
s32b SCHOOL_ERU;
s32b SCHOOL_FIRE;
s32b SCHOOL_GEOMANCY;
s32b SCHOOL_MANA;
s32b SCHOOL_MANDOS;
s32b SCHOOL_MANWE;
s32b SCHOOL_MELKOR;
s32b SCHOOL_META;
s32b SCHOOL_MIND;
s32b SCHOOL_MUSIC;
s32b SCHOOL_NATURE;
s32b SCHOOL_TEMPORAL;
s32b SCHOOL_TULKAS;
s32b SCHOOL_UDUN;
s32b SCHOOL_ULMO;
s32b SCHOOL_VARDA;
s32b SCHOOL_WATER;
s32b SCHOOL_YAVANNA;

static int compare_spell_idx(spell_idx_list *a, spell_idx_list *b)
{
	return SGLIB_NUMERIC_COMPARATOR(a->i, b->i);
}

SGLIB_DEFINE_LIST_FUNCTIONS(spell_idx_list, compare_spell_idx, next);

static bool_ uses_piety_to_cast(int s)
{
	return spell_type_uses_piety_to_cast(spell_at(s));
}

/** Describe what type of energy the spell uses for casting */
cptr get_power_name(s32b s)
{
	return uses_piety_to_cast(s) ? "piety" : "mana";
}

/* Changes the amount of power(mana, piety, whatever) for the spell */
void adjust_power(s32b s, s32b amount)
{
	if (uses_piety_to_cast(s))
	{
		inc_piety(GOD_ALL, amount);
	}
	else
	{
		increase_mana(amount);
	}
}

/* Return the amount of power available for casting spell */
s32b get_power(s32b s)
{
	return uses_piety_to_cast(s) ? p_ptr->grace : p_ptr->csp;
}

static void print_spell_desc_callback(void *data, cptr text)
{
	int *y = (int *) data;

	c_prt(TERM_L_BLUE, text, *y, 0);
	(*y) += 1;
}

/* Output the describtion when it is used as a spell */
void print_spell_desc(int s, int y)
{
	spell_type *spell = spell_at(s);

	spell_type_description_foreach(spell,
				       print_spell_desc_callback,
				       &y);

	if (spell_type_uses_piety_to_cast(spell))
	{
		c_prt(TERM_L_WHITE, "It uses piety to cast.", y, 0);
		y++;
	}

	if (spell_type_castable_while_blind(spell))
	{
		c_prt(TERM_ORANGE, "It is castable even while blinded.", y, 0);
		y++;
	}

	if (spell_type_castable_while_confused(spell))
	{
		c_prt(TERM_ORANGE, "It is castable even while confused.", y, 0);
		y++;
	}
}

school_book_type *school_books_at(int i)
{
	assert(i >= 0);
	assert(i < SCHOOL_BOOKS_SIZE);
	return &school_books[i];
}

static void school_book_init(school_book_type *school_book)
{
	school_book->spell_idx_list = NULL;
}

static void spell_idx_init(spell_idx_list *p, s32b spell_idx)
{
	assert(p != NULL);

	p->i = spell_idx;
	p->next = NULL;
}

static spell_idx_list *new_spell_idx(void *ctx, s32b spell_idx)
{
	spell_idx_list *e = malloc(sizeof(spell_idx_list));
	spell_idx_init(e, spell_idx);
	return e;
}

void school_book_add_spell(school_book_type *school_book, s32b spell_idx)
{
	spell_idx_list *e;

	assert(school_book != NULL);

	e = new_spell_idx(school_book, spell_idx);
	sglib_spell_idx_list_add(&school_book->spell_idx_list, e);
}

int school_book_length(int sval)
{
	school_book_type *school_book = school_books_at(sval);

	if (sval == BOOK_RANDOM)
	{
		return 1;
	}

	/* Parse all spells */
	return sglib_spell_idx_list_len(school_book->spell_idx_list);
}

int spell_x(int sval, int pval, int i)
{
	assert(i >= 0);

	if (sval == BOOK_RANDOM)
	{
		return pval;
	}
	else
	{
		school_book_type *school_book;
		spell_idx_list *spell_idx = NULL;
		struct sglib_spell_idx_list_iterator it;

		school_book = school_books_at(sval);

		for (spell_idx = sglib_spell_idx_list_it_init(&it, school_book->spell_idx_list);
		     (spell_idx != NULL) && (i > 0);
		     spell_idx = sglib_spell_idx_list_it_next(&it))
		{
			i--;
		}

		assert(spell_idx != NULL); /* Went off the end of the list? */

		return spell_idx->i;
	}
}

bool_ school_book_contains_spell(int sval, s32b spell_idx)
{
	school_book_type *school_book = NULL;
	spell_idx_list e;

	random_book_setup(sval, spell_idx);

	school_book = school_books_at(sval);

	spell_idx_init(&e, spell_idx);
	return NULL != sglib_spell_idx_list_find_member(school_book->spell_idx_list, &e);
}

void push_spell(int book_idx, s32b spell_idx)
{
	school_book_type *school_book = school_books_at(book_idx);
	assert(school_book != NULL);
	school_book_add_spell(school_book, spell_idx);
}

void init_school_books()
{
	int i;

	/* Initialize the new school books */
	for (i = 0; i<SCHOOL_BOOKS_SIZE; i++)
	{
		school_book_init(&school_books[i]);
	}

	/* Note: We're adding the spells in the reverse order that
	   they appear in each book. This is because the list 
	   operations insert at the front. */

	/* Create the crystal of mana */
	push_spell(TOME_MANA, MANASHIELD);
	push_spell(TOME_MANA, RESISTS);
	push_spell(TOME_MANA, DELCURSES);
	push_spell(TOME_MANA, MANATHRUST);

	/* The book of the eternal flame */
	push_spell(TOME_FIRE, FIERYAURA);
	push_spell(TOME_FIRE, FIREWALL);
	push_spell(TOME_FIRE, FIREFLASH);
	push_spell(TOME_FIRE, FIREGOLEM);
	push_spell(TOME_FIRE, GLOBELIGHT);

	/* The book of the blowing winds */
	push_spell(TOME_WINDS, THUNDERSTORM);
	push_spell(TOME_WINDS, AIRWINGS);
	push_spell(TOME_WINDS, STERILIZE);
	push_spell(TOME_WINDS, INVISIBILITY);
	push_spell(TOME_WINDS, POISONBLOOD);
	push_spell(TOME_WINDS, NOXIOUSCLOUD);

	/* The book of the impenetrable earth */
	push_spell(TOME_EARTH, STRIKE);
	push_spell(TOME_EARTH, SHAKE);
	push_spell(TOME_EARTH, STONEPRISON);
	push_spell(TOME_EARTH, DIG);
	push_spell(TOME_EARTH, STONESKIN);

	/* The book of the unstopable wave */
	push_spell(TOME_WATER, ICESTORM);
	push_spell(TOME_WATER, TIDALWAVE);
	push_spell(TOME_WATER, ENTPOTION);
	push_spell(TOME_WATER, VAPOR);
	push_spell(TOME_WATER, GEYSER);

	/* Create the book of translocation */
	push_spell(TOME_TRANSLOCATION, PROBABILITY_TRAVEL);
	push_spell(TOME_TRANSLOCATION, RECALL);
	push_spell(TOME_TRANSLOCATION, TELEAWAY);
	push_spell(TOME_TRANSLOCATION, TELEPORT);
	push_spell(TOME_TRANSLOCATION, DISARM);
	push_spell(TOME_TRANSLOCATION, BLINK);

	/* Create the book of the tree */
	if (game_module_idx == MODULE_THEME)
	{
		push_spell(TOME_NATURE, GROW_ATHELAS);
	}
	push_spell(TOME_NATURE, SUMMONANNIMAL);
	push_spell(TOME_NATURE, REGENERATION);
	push_spell(TOME_NATURE, RECOVERY);
	push_spell(TOME_NATURE, HEALING);
	push_spell(TOME_NATURE, GROWTREE);

	/* Create the book of Knowledge */
	push_spell(TOME_KNOWLEDGE, STARIDENTIFY);
	push_spell(TOME_KNOWLEDGE, VISION);
	push_spell(TOME_KNOWLEDGE, IDENTIFY);
	push_spell(TOME_KNOWLEDGE, REVEALWAYS);
	push_spell(TOME_KNOWLEDGE, SENSEHIDDEN);
	push_spell(TOME_KNOWLEDGE, SENSEMONSTERS);

	/* Create the book of the Time */
	push_spell(TOME_TIME, BANISHMENT);
	push_spell(TOME_TIME, ESSENCESPEED);
	push_spell(TOME_TIME, SLOWMONSTER);
	push_spell(TOME_TIME, MAGELOCK);

	/* Create the book of meta spells */
	push_spell(TOME_META, INERTIA_CONTROL);
	push_spell(TOME_META, TRACKER);
	push_spell(TOME_META, SPELLBINDER);
	push_spell(TOME_META, DISPERSEMAGIC);
	push_spell(TOME_META, RECHARGE);

	/* Create the book of the mind */
	push_spell(TOME_MIND, STUN);
	push_spell(TOME_MIND, ARMOROFFEAR);
	push_spell(TOME_MIND, CONFUSE);
	push_spell(TOME_MIND, CHARM);

	/* Create the book of hellflame */
	push_spell(TOME_HELLFLAME, FLAMEOFUDUN);
	push_spell(TOME_HELLFLAME, WRAITHFORM);
	push_spell(TOME_HELLFLAME, GENOCIDE);
	push_spell(TOME_HELLFLAME, DRAIN);

	/* Create the book of eru */
	push_spell(TOME_ERU, ERU_PROT);
	push_spell(TOME_ERU, ERU_UNDERSTAND);
	push_spell(TOME_ERU, ERU_LISTEN);
	push_spell(TOME_ERU, ERU_SEE);

	/* Create the book of manwe */
	push_spell(TOME_MANWE, MANWE_AVATAR);
	push_spell(TOME_MANWE, MANWE_CALL);
	push_spell(TOME_MANWE, MANWE_SHIELD);
	push_spell(TOME_MANWE, MANWE_BLESS);

	/* Create the book of tulkas */
	push_spell(TOME_TULKAS, TULKAS_WAVE);
	push_spell(TOME_TULKAS, TULKAS_SPIN);
	push_spell(TOME_TULKAS, TULKAS_AIM);

	/* Create the book of melkor */
	push_spell(TOME_MELKOR, MELKOR_MIND_STEAL);
	push_spell(TOME_MELKOR, MELKOR_CORPSE_EXPLOSION);
	push_spell(TOME_MELKOR, MELKOR_CURSE);

	/* Create the book of yavanna */
	push_spell(TOME_YAVANNA, YAVANNA_UPROOT);
	push_spell(TOME_YAVANNA, YAVANNA_WATER_BITE);
	push_spell(TOME_YAVANNA, YAVANNA_TREE_ROOTS);
	push_spell(TOME_YAVANNA, YAVANNA_GROW_GRASS);
	push_spell(TOME_YAVANNA, YAVANNA_CHARM_ANIMAL);

	/* Create the book of beginner's cantrip */
	push_spell(BOOK_CANTRIPS, SENSEHIDDEN);
	push_spell(BOOK_CANTRIPS, SENSEMONSTERS);
	push_spell(BOOK_CANTRIPS, BLINK);
	push_spell(BOOK_CANTRIPS, ENTPOTION);
	push_spell(BOOK_CANTRIPS, GLOBELIGHT);
	push_spell(BOOK_CANTRIPS, MANATHRUST);

	/* Create the book of teleporatation */
	push_spell(BOOK_TELEPORTATION, TELEAWAY);
	push_spell(BOOK_TELEPORTATION, TELEPORT);
	push_spell(BOOK_TELEPORTATION, BLINK);

	/* Create the book of summoning */
	push_spell(BOOK_SUMMONING, SUMMONANNIMAL);
	push_spell(BOOK_SUMMONING, FIREGOLEM);

	/* Create the Armageddon Demonblade */
	push_spell(BOOK_DEMON_SWORD, DEMON_FIELD);
	push_spell(BOOK_DEMON_SWORD, DEMON_MADNESS);
	push_spell(BOOK_DEMON_SWORD, DEMON_BLADE);

	/* Create the Shield Demonblade */
	push_spell(BOOK_DEMON_SHIELD, UNHOLY_WORD);
	push_spell(BOOK_DEMON_SHIELD, DEMON_CLOAK);
	push_spell(BOOK_DEMON_SHIELD, DOOM_SHIELD);

	/* Create the Control Demonblade */
	push_spell(BOOK_DEMON_HELM, CONTROL_DEMON);
	push_spell(BOOK_DEMON_HELM, DISCHARGE_MINION);
	push_spell(BOOK_DEMON_HELM, DEMON_SUMMON);

	/* Create the Drums */
	push_spell(BOOK_DRUMS, MUSIC_STUN);
	push_spell(BOOK_DRUMS, MUSIC_CONF);
	push_spell(BOOK_DRUMS, MUSIC_HOLD);
	push_spell(BOOK_DRUMS, MUSIC_STOP);

	/* Create the Harps */
	push_spell(BOOK_HARPS, MUSIC_MIND);
	push_spell(BOOK_HARPS, MUSIC_TIME);
	push_spell(BOOK_HARPS, MUSIC_HEAL);
	push_spell(BOOK_HARPS, MUSIC_HERO);
	push_spell(BOOK_HARPS, MUSIC_LITE);
	push_spell(BOOK_HARPS, MUSIC_STOP);

	/* Create the Horns */
	push_spell(BOOK_HORNS, MUSIC_AMBARKANTA);
	push_spell(BOOK_HORNS, MUSIC_YLMIR);
	push_spell(BOOK_HORNS, MUSIC_WIND);
	push_spell(BOOK_HORNS, MUSIC_BLOW);
	push_spell(BOOK_HORNS, MUSIC_STOP);

	/* Book of the Player, filled in by the Library Quest */
	push_spell(BOOK_PLAYER, -1);

	/* Geomancy spells, not a real book */
	push_spell(BOOK_GEOMANCY, ELEMENTAL_MINION);
	push_spell(BOOK_GEOMANCY, GROW_BARRIER);
	push_spell(BOOK_GEOMANCY, DRIPPING_TREAD);
	push_spell(BOOK_GEOMANCY, GEOLYSIS);
	push_spell(BOOK_GEOMANCY, VAPORIZE);
	push_spell(BOOK_GEOMANCY, ELEMENTAL_WAVE);
	push_spell(BOOK_GEOMANCY, CHANNEL_ELEMENTS);
	push_spell(BOOK_GEOMANCY, CALL_THE_ELEMENTS);

	if (game_module_idx == MODULE_THEME)
	{
		/* Aule */
		push_spell(BOOK_AULE, AULE_CHILD);
		push_spell(BOOK_AULE, AULE_ENCHANT_ARMOUR);
		push_spell(BOOK_AULE, AULE_ENCHANT_WEAPON);
		push_spell(BOOK_AULE, AULE_FIREBRAND);

		/* Varda */
		push_spell(BOOK_VARDA, VARDA_STARKINDLER);
		push_spell(BOOK_VARDA, VARDA_EVENSTAR);
		push_spell(BOOK_VARDA, VARDA_CALL_ALMAREN);
		push_spell(BOOK_VARDA, VARDA_LIGHT_VALINOR);

		/* Ulmo */
		push_spell(BOOK_ULMO, ULMO_WRATH);
		push_spell(BOOK_ULMO, ULMO_CALL_ULUMURI);
		push_spell(BOOK_ULMO, ULMO_DRAUGHT_ULMONAN);
		push_spell(BOOK_ULMO, ULMO_BELEGAER);

		/* Mandos */
		push_spell(BOOK_MANDOS, MANDOS_CALL_HALLS);
		push_spell(BOOK_MANDOS, MANDOS_TALE_DOOM);
		push_spell(BOOK_MANDOS, MANDOS_SPIRIT_FEANTURI);
		push_spell(BOOK_MANDOS, MANDOS_TEARS_LUTHIEN);
    	}

	/* Random spell book; just initialize to anything */
	push_spell(BOOK_RANDOM, -1);
}

void random_book_setup(s16b sval, s32b spell_idx)
{
	if (sval == BOOK_RANDOM)
	{
		school_book_type *school_book = school_books_at(sval);

		assert(school_book->spell_idx_list != NULL);
		school_book->spell_idx_list->i = spell_idx;
	}
}

static bool_ spell_school_name_callback(void *data, s32b sch)
{
	school_type *school = school_at(sch);
	char *buf = (char *) data;

	/* Add separator? */
	if (buf[0] != '\0')
	{
		strcat(buf, "/");
	}

	/* Add school name */
	strcat(buf, school->name);

	/* Keep going */
	return TRUE;
}

static void spell_school_name(char *buf, spell_type *spell)
{
	buf[0] = '\0';
	spell_type_school_foreach(spell,
				  spell_school_name_callback,
				  buf);
}

int print_spell(cptr label_, byte color, int y, s32b s)
{
	s32b level;
	bool_ na;
	spell_type *spell = spell_at(s);
	char sch_str[128];
	cptr spell_info = spell_type_info(spell);
	cptr label = (label_ == NULL) ? "" : label_;
	char level_str[8] = "n/a";
	char buf[128];

	get_level_school(s, 50, -50, &level, &na);
	spell_school_name(sch_str, spell);

	if (!na)
	{
		sprintf(level_str, "%3d", (int) level);
	}

	sprintf(buf, "%s%-20s%-16s   %s %4d %3d%% %s",
		label,
		spell_type_name(spell_at(s)),
		sch_str,
		level_str,
		get_mana(s),
		(int) spell_chance(s),
		spell_info);
	c_prt(color, buf, y, 0);

	return y + 1;
}

int print_book(s16b sval, s32b pval, object_type *obj)
{
	int y = 2;
	int i;
	school_book_type *school_book;
	spell_idx_list *spell_idx;
	struct sglib_spell_idx_list_iterator it;

	random_book_setup(sval, pval);

	school_book = school_books_at(sval);

	/* Parse all spells */
	i = 0;
	for (spell_idx = sglib_spell_idx_list_it_init(&it, school_book->spell_idx_list);
	     spell_idx != NULL;
	     spell_idx = sglib_spell_idx_list_it_next(&it))
	{
		s32b s = spell_idx->i;
		byte color = TERM_L_DARK;
		bool_ is_ok;
		char label[8];

		is_ok = is_ok_spell(s, obj);
		if (is_ok)
		{
			color = (get_mana(s) > get_power(s)) ? TERM_ORANGE : TERM_L_GREEN;
		}

		sprintf(label, "%c) ", 'a' + i);

		y = print_spell(label, color, y, s);
		i++;
	}

	prt(format("   %-20s%-16s Level Cost Fail Info", "Name", "School"), 1, 0);
	return y;
}

void lua_cast_school_spell(s32b s, bool_ no_cost)
{
	bool_ use = FALSE;
	spell_type *spell = spell_at(s);

	/* No magic? */
	if (p_ptr->antimagic > 0)
	{
		msg_print("Your anti-magic field disrupts any magic attempts.");
		return;
	}

	/* No magic? */
	if (p_ptr->anti_magic)
	{
		msg_print("Your anti-magic shell disrupts any magic attempts.");
		return;
	}

	/* if it costs something then some condition must be met */
	if (!no_cost)
	{
	 	/* Require lite */
		if (!spell_type_castable_while_blind(spell) &&
		    ((p_ptr->blind > 0) || no_lite()))
		{
			msg_print("You cannot see!");
			return;
		}

		/* Not when confused */
		if (!spell_type_castable_while_confused(spell) &&
		    (p_ptr->confused > 0))
		{
			msg_print("You are too confused!");
			return;
		}

		/* Enough mana */
		if (get_mana(s) > get_power(s))
		{
			char buf[128];
			sprintf(buf,
				"You do not have enough %s, do you want to try anyway?",
				get_power_name(s));

			if (!get_check(buf))
			{
				return;
			}
		}
	
		/* Invoke the spell effect */
		if (!magik(spell_chance(s)))
		{
			use = (spell_type_produce_effect(spell, -1) != NO_CAST);
		}
		else
		{
			use  = TRUE;

			/* failures are dangerous; we'll flush the input buffer
			   so it isn't missed. */
			if (flush_failure)
			{
				flush();
			}

			msg_print("You failed to get the spell off!");
		}
	}
	else
	{
		spell_type_produce_effect(spell, -1);
	}

	/* Use the mana/piety */
	if (use == TRUE)
	{
		/* Reduce mana */
		adjust_power(s, -get_mana(s));

		/* Take a turn */
		energy_use = is_magestaff() ? 80 : 100;
	}

	/* Refresh player */
	p_ptr->redraw |= PR_MANA;
	p_ptr->window |= PW_PLAYER;
}

void device_allocation_init(device_allocation *device_allocation, byte tval)
{
	assert(device_allocation != NULL);

	device_allocation->tval = tval;
	device_allocation->rarity = 0;
	range_init(&device_allocation->base_level, 0, 0);
	range_init(&device_allocation->max_level, 0, 0);
	device_allocation->next = NULL;
}

device_allocation *device_allocation_new(byte tval)
{
	device_allocation *d = malloc(sizeof(device_allocation));
	device_allocation_init(d, tval);
	return d;
}

int compare_device_allocation(device_allocation *a, device_allocation *b)
{
	return SGLIB_NUMERIC_COMPARATOR(a->tval, b->tval);
}

SGLIB_DEFINE_LIST_FUNCTIONS(device_allocation, compare_device_allocation, next);

void dice_init(dice_type *dice, long base, long num, long sides)
{
	assert(dice != NULL);

	dice->base = base;
	dice->num = num;
	dice->sides = sides;
}

bool_ dice_parse(dice_type *dice, cptr s)
{
	long base, num, sides;

	if (sscanf(s, "%ld+%ldd%ld", &base, &num, &sides) == 3)
	{
		dice_init(dice, base, num, sides);
		return TRUE;
	}

	if (sscanf(s, "%ld+d%ld", &base, &sides) == 2)
	{
		dice_init(dice, base, 1, sides);
		return TRUE;
	}

	if (sscanf(s, "d%ld", &sides) == 1)
	{
		dice_init(dice, 0, 1, sides);
		return TRUE;
	}

	if (sscanf(s, "%ldd%ld", &num, &sides) == 2)
	{
		dice_init(dice, 0, num, sides);
		return TRUE;
	}

	if (sscanf(s, "%ld", &base) == 1)
	{
		dice_init(dice, base, 0, 0);
		return TRUE;
	}

	return FALSE;
}

void dice_parse_checked(dice_type *dice, cptr s)
{
	bool_ result = dice_parse(dice, s);
	if (!result)
	{
		abort();
	}
}

long dice_roll(dice_type *dice)
{
	assert(dice != NULL);
	return dice->base + damroll(dice->num, dice->sides);
}

void dice_print(dice_type *dice, char *output)
{
	char buf[16];

	output[0] = '\0';

	if (dice->base > 0)
	{
		sprintf(buf, "%ld", dice->base);
		strcat(output, buf);
	}

	if ((dice->num > 0) || (dice->sides > 0))
	{
		if (dice->base > 0)
		{
			strcat(output, "+");
		}

		if (dice->num > 1)
		{
			sprintf(buf, "%ld", dice->num);
			strcat(output, buf);
		}

		strcat(output, "d");

		sprintf(buf, "%ld", dice->sides);
		strcat(output, buf);
	}
}
