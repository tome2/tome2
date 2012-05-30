#include <angband.h>
#include <assert.h>

typedef enum { RANDOM, NO_RANDOM } random_type;

static void spell_inertia_init(spell_type *spell, s32b diff, s32b delay)
{
	spell->inertia_difficulty = diff;
	spell->inertia_delay = delay;
}

static void spell_init(spell_type *spell)
{
	memset(spell, 0, sizeof(spell_type));

	spell->name = NULL;
	spell->description = NULL;
	spell->effect_func = NULL;
	spell->info_func = NULL;
	spell->lasting_func = NULL;
	spell->depend_func = NULL;

	spell->device_allocation = NULL;
	spell->schools = NULL;

	spell->random_type = -1;

	spell_inertia_init(spell, -1, -1);

	spell->castable_while_blind = FALSE;
	spell->castable_while_confused = FALSE;
}

static spell_type *spell_new(s32b *index, cptr id, cptr name)
{
	assert(school_spells_count < SCHOOL_SPELLS_MAX);

	spell_type *spell = &school_spells[school_spells_count];

	spell_init(spell);
	spell->name = name;

	*index = school_spells_count;
	school_spells_count++;

	return spell;
}

spell_type *spell_at(s32b index)
{
	assert(index >= 0);
	assert(index < school_spells_count);

	return &school_spells[index];
}

int find_spell(cptr name)
{
	int i;

	for (i = 0; i < school_spells_count; i++)
	{
		spell_type *spell = spell_at(i);
		if (streq(spell->name, name))
		{
			return i;
		}
	}

	/* Not found */
	return -1;
}

s16b get_random_spell(s16b random_type, int level)
{
	int tries;

	for (tries = 0; tries < 1000; tries++)
	{
		s16b spl = rand_int(school_spells_count);
		spell_type *spell = spell_at(spl);

		if ((can_spell_random(spl) == random_type) &&
		    (rand_int(spell->skill_level * 3) < level))
		{
			return spl;
		}
	}

	return -1;
}

bool_ check_spell_depends(spell_type *spell)
{
	assert(spell != NULL);

	if (spell->depend_func != NULL) {
		return spell->depend_func();
	} else {
		return TRUE;
	}
}

static void spell_init_music(spell_type *spell, s16b minimum_pval)
{
	assert(spell != NULL);
	/* Use spell points, but CHR for success/failure calculations */
	spell->casting_type = USE_SPELL_POINTS;
	spell->casting_stat = A_CHR;
	spell->random_type = SKILL_MUSIC;
	spell->minimum_pval = minimum_pval;
	/* Add school */
	school_idx_add_new(&spell->schools, SCHOOL_MUSIC);
}

static void spell_init_mage(spell_type *spell, random_type random_type)
{
	assert(spell != NULL);

	spell->casting_type = USE_SPELL_POINTS;
	spell->casting_stat = A_INT;

	switch (random_type)
	{
	case RANDOM:
		spell->random_type = SKILL_MAGIC;
		break;
	case NO_RANDOM:
		spell->random_type = -1;
		break;
	default:
		/* Cannot happen */
		assert(FALSE);
	}
}

static void spell_init_priest(spell_type *spell)
{
	assert(spell != NULL);

	spell->random_type = SKILL_SPIRITUALITY;
	spell->casting_type = USE_PIETY;
	spell->casting_stat = A_WIS;
}

static void spells_init_tome()
{
	{
		spell_type *spell = spell_new(&DEVICE_LEBOHAUM, "DEVICE_LEBOHAUM", "Artifact Lebauhaum");
		dice_parse_checked(&spell->activation_duration, "3");
		string_list_append(&spell->description, "sing a cheerful song");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_lebohaum_info;
		spell->effect_func = device_lebohaum;
		spell->failure_rate = 0;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEVICE_DURANDIL, "DEVICE_DURANDIL", "Artifact Durandil");
		dice_parse_checked(&spell->activation_duration, "3");
		string_list_append(&spell->description, "sing a cheerful song");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_durandil_info;
		spell->effect_func = device_durandil;
		spell->failure_rate = 0;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEVICE_THUNDERLORDS, "DEVICE_THUNDERLORDS", "Artifact Thunderlords");
		string_list_append(&spell->description, "A thunderlord will appear to transport you quickly to the surface.");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 1, 1);
		spell->info_func = device_thunderlords_info;
		spell->effect_func = device_thunderlords;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}
}

static void spells_init_theme()
{
	{
		spell_type *spell = spell_new(&GROW_ATHELAS, "GROW_ATHELAS", "Grow Athelas");
		string_list_append(&spell->description, "Cures the Black Breath");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 60, 100);
		spell->info_func = nature_grow_athelas_info;
		spell->effect_func = nature_grow_athelas;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&AULE_FIREBRAND, "AULE_FIREBRAND", "Firebrand");
		string_list_append(&spell->description, "Imbues your melee weapon with fire to deal more damage");
		string_list_append(&spell->description, "At level 15 it spreads over a 1 radius zone around your target");
		string_list_append(&spell->description, "At level 30 it deals holy fire damage");
		school_idx_add_new(&spell->schools, SCHOOL_AULE);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = aule_firebrand_info;
		spell->effect_func = aule_firebrand_spell;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&AULE_ENCHANT_WEAPON, "AULE_ENCHANT_WEAPON", "Enchant Weapon");
		string_list_append(&spell->description, "Tries to enchant a weapon to-hit");
		string_list_append(&spell->description, "At level 5 it also enchants to-dam");
		string_list_append(&spell->description, "At level 45 it enhances the special powers of magical weapons");
		string_list_append(&spell->description, "The might of the enchantment increases with the level");
		school_idx_add_new(&spell->schools, SCHOOL_AULE);
		range_init(&spell->mana_range, 100, 200);
		spell->info_func = aule_enchant_weapon_info;
		spell->effect_func = aule_enchant_weapon_spell;
		spell->failure_rate = 20;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&AULE_ENCHANT_ARMOUR, "AULE_ENCHANT_ARMOUR", "Enchant Armour");
		string_list_append(&spell->description, "Tries to enchant a piece of armour");
		string_list_append(&spell->description, "At level 20 it also enchants to-hit and to-dam");
		string_list_append(&spell->description, "At level 40 it enhances the special powers of magical armour");
		string_list_append(&spell->description, "The might of the enchantment increases with the level");
		school_idx_add_new(&spell->schools, SCHOOL_AULE);
		range_init(&spell->mana_range, 100, 200);
		spell->info_func = aule_enchant_armour_info;
		spell->effect_func = aule_enchant_armour_spell;
		spell->failure_rate = 20;
		spell->skill_level = 15;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&AULE_CHILD, "AULE_CHILD", "Child of Aule");
		string_list_append(&spell->description, "Summons a levelled Dwarven warrior to help you battle the forces");
		string_list_append(&spell->description, "of Morgoth");
		school_idx_add_new(&spell->schools, SCHOOL_AULE);
		range_init(&spell->mana_range, 200, 500);
		spell->info_func = aule_child_info;
		spell->effect_func = aule_child_spell;
		spell->failure_rate = 40;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_LIGHT_VALINOR, "VARDA_LIGHT_VALINOR", "Light of Valinor");
		string_list_append(&spell->description, "Lights a room");
		string_list_append(&spell->description, "At level 3 it starts damaging monsters");
		string_list_append(&spell->description, "At level 15 it starts creating a more powerful kind of light");
		school_idx_add_new(&spell->schools, SCHOOL_VARDA);
		range_init(&spell->mana_range, 1, 100);
		spell->info_func = varda_light_of_valinor_info;
		spell->effect_func = varda_light_of_valinor_spell;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_CALL_ALMAREN, "VARDA_CALL_ALMAREN", "Call of Almaren");
		string_list_append(&spell->description, "Banishes evil beings");
		string_list_append(&spell->description, "At level 20 it dispels evil beings");
		school_idx_add_new(&spell->schools, SCHOOL_VARDA);
		range_init(&spell->mana_range, 5, 150);
		spell->info_func = varda_call_of_almaren_info;
		spell->effect_func = varda_call_of_almaren_spell;
		spell->failure_rate = 20;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_EVENSTAR, "VARDA_EVENSTAR", "Evenstar");
		string_list_append(&spell->description, "Maps and lights the whole level.");
		string_list_append(&spell->description, "At level 40 it maps and lights the whole level,");
		string_list_append(&spell->description, "in addition to letting you know yourself better");
		string_list_append(&spell->description, "and identifying your whole pack.");
		school_idx_add_new(&spell->schools, SCHOOL_VARDA);
		range_init(&spell->mana_range, 20, 200);
		spell->info_func = varda_evenstar_info;
		spell->effect_func = varda_evenstar_spell;
		spell->failure_rate = 20;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_STARKINDLER, "VARDA_STARKINDLER", "Star Kindler");
		string_list_append(&spell->description, "Does multiple bursts of light damage.");
		string_list_append(&spell->description, "The damage increases with level.");
		school_idx_add_new(&spell->schools, SCHOOL_VARDA);
		range_init(&spell->mana_range, 50, 250);
		spell->info_func = varda_star_kindler_info;
		spell->effect_func = varda_star_kindler_spell;
		spell->failure_rate = 20;
		spell->skill_level = 30;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_BELEGAER, "ULMO_BELEGAER", "Song of Belegaer");
		string_list_append(&spell->description, "Channels the power of the Great Sea into your fingertips.");
		string_list_append(&spell->description, "Sometimes it can blast through its first target.");
		school_idx_add_new(&spell->schools, SCHOOL_ULMO);
		range_init(&spell->mana_range, 1, 100);
		spell->info_func = ulmo_song_of_belegaer_info;
		spell->effect_func = ulmo_song_of_belegaer_spell;
		spell->failure_rate = 25;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_DRAUGHT_ULMONAN, "ULMO_DRAUGHT_ULMONAN", "Draught of Ulmonan");
		string_list_append(&spell->description, "Fills you with a draught with powerful curing effects,");
		string_list_append(&spell->description, "prepared by Ulmo himself.");
		string_list_append(&spell->description, "Level 1: blindness, poison, cuts and stunning");
		string_list_append(&spell->description, "Level 10: drained STR, DEX and CON");
		string_list_append(&spell->description, "Level 20: parasites and mimicry");
		school_idx_add_new(&spell->schools, SCHOOL_ULMO);
		range_init(&spell->mana_range, 25, 200);
		spell->info_func = ulmo_draught_of_ulmonan_info;
		spell->effect_func = ulmo_draught_of_ulmonan_spell;
		spell->failure_rate = 50;
		spell->skill_level = 15;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_CALL_ULUMURI, "ULMO_CALL_ULUMURI", "Call of the Ulumuri");
		string_list_append(&spell->description, "Summons a leveled water spirit or elemental");
		string_list_append(&spell->description, "to fight for you");
		school_idx_add_new(&spell->schools, SCHOOL_ULMO);
		range_init(&spell->mana_range, 50, 300);
		spell->info_func = ulmo_call_of_the_ulumuri_info;
		spell->effect_func = ulmo_call_of_the_ulumuri_spell;
		spell->failure_rate = 75;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_WRATH, "ULMO_WRATH", "Wrath of Ulmo");
		string_list_append(&spell->description, "Conjures up a sea storm.");
		string_list_append(&spell->description, "At level 30 it turns into a more forceful storm.");
		school_idx_add_new(&spell->schools, SCHOOL_ULMO);
		range_init(&spell->mana_range, 100, 400);
		spell->info_func = ulmo_wrath_of_ulmo_info;
		spell->effect_func = ulmo_wrath_of_ulmo_spell;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_TEARS_LUTHIEN, "MANDOS_TEARS_LUTHIEN", "Tears of Luthien");
		string_list_append(&spell->description, "Calls upon the spirit of Luthien to ask Mandos for healing and succour.");
		school_idx_add_new(&spell->schools, SCHOOL_MANDOS);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = mandos_tears_of_luthien_info;
		spell->effect_func = mandos_tears_of_luthien_spell;
		spell->failure_rate = 25;
		spell->skill_level = 5;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_SPIRIT_FEANTURI, "MANDOS_SPIRIT_FEANTURI", "Feanturi");
		string_list_append(&spell->description, "Channels the power of Mandos to cure fear and confusion.");
		string_list_append(&spell->description, "At level 20 it restores lost INT and WIS");
		string_list_append(&spell->description, "At level 30 it cures hallucinations and restores a percentage of lost sanity");
		school_idx_add_new(&spell->schools, SCHOOL_MANDOS);
		range_init(&spell->mana_range, 40, 200);
		spell->info_func = mandos_spirit_of_the_feanturi_info;
		spell->effect_func = mandos_spirit_of_the_feanturi_spell;
		spell->failure_rate = 50;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_TALE_DOOM, "MANDOS_TALE_DOOM", "Tale of Doom");
		string_list_append(&spell->description, "Allows you to predict the future for a short time.");
		school_idx_add_new(&spell->schools, SCHOOL_MANDOS);
		range_init(&spell->mana_range, 60, 300);
		spell->info_func = mandos_tale_of_doom_info;
		spell->effect_func = mandos_tale_of_doom_spell;
		spell->failure_rate = 75;
		spell->skill_level = 25;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_CALL_HALLS, "MANDOS_CALL_HALLS", "Call to the Halls");
		string_list_append(&spell->description, "Summons a leveled spirit from the Halls of Mandos");
		string_list_append(&spell->description, "to fight for you.");
		school_idx_add_new(&spell->schools, SCHOOL_MANDOS);
		range_init(&spell->mana_range, 80, 400);
		spell->info_func = mandos_call_to_the_halls_info;
		spell->effect_func = mandos_call_to_the_halls_spell;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&DEVICE_THUNDERLORDS, "DEVICE_THUNDERLORDS", "Artifact Thunderlords");
		string_list_append(&spell->description, "An Eagle of Manwe will appear to transport you quickly to the town.");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 1, 1);
		spell->info_func = device_thunderlords_info;
		spell->effect_func = device_thunderlords;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_RADAGAST, "DEVICE_RADAGAST", "Artifact Radagast");
		dice_parse_checked(&spell->activation_duration, "15000");
		string_list_append(&spell->description, "purity and health");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_radagast_info;
		spell->effect_func = device_radagast;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEVICE_VALAROMA, "DEVICE_VALAROMA", "Artifact Valaroma");
		dice_parse_checked(&spell->activation_duration, "250");
		string_list_append(&spell->description, "banish evil (level x5)");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_valaroma_info;
		spell->effect_func = device_valaroma;
		spell->failure_rate = 25;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}
}

void school_spells_init()
{
	{
		spell_type *spell = spell_new(&GLOBELIGHT, "GLOBELIGHT", "Globe of Light");
		string_list_append(&spell->description, "Creates a globe of pure light");
		string_list_append(&spell->description, "At level 3 it starts damaging monsters");
		string_list_append(&spell->description, "At level 15 it starts creating a more powerful kind of light");
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		range_init(&spell->mana_range, 2, 15);
		spell_inertia_init(spell, 1, 40);
		spell->info_func = fire_globe_of_light_info;
		spell->effect_func = fire_globe_of_light;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "10+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 7;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 45);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREFLASH, "FIREFLASH", "Fireflash");
		string_list_append(&spell->description, "Conjures a ball of fire to burn your foes to ashes");
		string_list_append(&spell->description, "At level 20 it turns into a ball of holy fire");
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		range_init(&spell->mana_range, 5, 70);
		spell->info_func = fire_fireflash_info;
		spell->effect_func = fire_fireflash;
		spell->failure_rate = 35;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 15, 35);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIERYAURA, "FIERYAURA", "Fiery Shield");
		string_list_append(&spell->description, "Creates a shield of fierce flames around you");
		string_list_append(&spell->description, "At level 8 it turns into a greater kind of flame that can not be resisted");
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		range_init(&spell->mana_range, 20, 60);
		spell_inertia_init(spell, 2, 15);
		spell->info_func = fire_fiery_shield_info;
		spell->effect_func = fire_fiery_shield;
		spell->failure_rate = 50;
		spell->skill_level = 20;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 5, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREWALL, "FIREWALL", "Firewall");
		string_list_append(&spell->description, "Creates a fiery wall to incinerate monsters stupid enough to attack you");
		string_list_append(&spell->description, "At level 6 it turns into a wall of hell fire");
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		range_init(&spell->mana_range, 25, 100);
		spell->info_func = fire_firewall_info;
		spell->effect_func = fire_firewall;
		spell->failure_rate = 40;
		spell->skill_level = 15;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "4+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 55;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 5, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREGOLEM, "FIREGOLEM", "Fire Golem");
		string_list_append(&spell->description, "Creates a fiery golem and controls it");
		string_list_append(&spell->description, "During the control the available keylist is:");
		string_list_append(&spell->description, "Movement keys: movement of the golem(depending on its speed");
		string_list_append(&spell->description, "               it can move more than one square)");
		string_list_append(&spell->description, ", : pickup all items on the floor");
		string_list_append(&spell->description, "d : drop all carried items");
		string_list_append(&spell->description, "i : list all carried items");
		string_list_append(&spell->description, "m : end the possession/use golem powers");
		string_list_append(&spell->description, "Most of the other keys are disabled, you cannot interact with your");
		string_list_append(&spell->description, "real body while controlling the golem");
		string_list_append(&spell->description, "But to cast the spell you will need a lantern or a wooden torch to");
		string_list_append(&spell->description, "Create the golem from");
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		school_idx_add_new(&spell->schools, SCHOOL_MIND);
		range_init(&spell->mana_range, 16, 70);
		spell->info_func = fire_golem_info;
		spell->effect_func = fire_golem;
		spell->failure_rate = 40;
		spell->skill_level = 7;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&MANATHRUST, "MANATHRUST", "Manathrust");
		string_list_append(&spell->description, "Conjures up mana into a powerful bolt");
		string_list_append(&spell->description, "The damage is irresistible and will increase with level");
		school_idx_add_new(&spell->schools, SCHOOL_MANA);
		range_init(&spell->mana_range, 1, 25);
		spell->info_func = mana_manathrust_info;
		spell->effect_func = mana_manathrust;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 5;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 15, 33);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DELCURSES, "DELCURSES", "Remove Curses");
		string_list_append(&spell->description, "Remove curses of worn objects");
		string_list_append(&spell->description, "At level 20 switches to *remove curses*");
		school_idx_add_new(&spell->schools, SCHOOL_MANA);
		range_init(&spell->mana_range, 20, 40);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = mana_remove_curses_info;
		spell->effect_func = mana_remove_curses;
		spell->failure_rate = 30;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d8");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 70;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RESISTS, "RESISTS", "Elemental Shield");
		string_list_append(&spell->description, "Provide resistances to the four basic elements");
		school_idx_add_new(&spell->schools, SCHOOL_MANA);
		range_init(&spell->mana_range, 17, 20);
		spell_inertia_init(spell, 2, 25);
		spell->info_func = mana_elemental_shield_info;
		spell->effect_func = mana_elemental_shield;
		spell->failure_rate = 40;
		spell->skill_level = 20;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&MANASHIELD, "MANASHIELD", "Disruption Shield");
		string_list_append(&spell->description, "Uses mana instead of hp to take damage");
		string_list_append(&spell->description, "At level 5 switches to Globe of Invulnerability.");
		string_list_append(&spell->description, "The spell breaks as soon as a melee, shooting, throwing or magical");
		string_list_append(&spell->description, "skill action is attempted, and lasts only a short time.");
		school_idx_add_new(&spell->schools, SCHOOL_MANA);
		range_init(&spell->mana_range, 50, 50);
		spell_inertia_init(spell, 9, 10);
		spell->info_func = mana_disruption_shield_info;
		spell->effect_func = mana_disruption_shield;
		spell->failure_rate = 90;
		spell->skill_level = 45;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&TIDALWAVE, "TIDALWAVE", "Tidal Wave");
		string_list_append(&spell->description, "Summons a monstrous tidal wave that will expand and crush the");
		string_list_append(&spell->description, "monsters under its mighty waves.");
		school_idx_add_new(&spell->schools, SCHOOL_WATER);
		range_init(&spell->mana_range, 16, 40);
		spell_inertia_init(spell, 4, 100);
		spell->info_func = water_tidal_wave_info;
		spell->effect_func = water_tidal_wave;
		spell->failure_rate = 65;
		spell->skill_level = 16;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "6+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 54;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ICESTORM, "ICESTORM", "Ice Storm");
		string_list_append(&spell->description, "Engulfs you in a storm of roaring cold that strikes your foes.");
		string_list_append(&spell->description, "At level 10 it turns into shards of ice.");
		school_idx_add_new(&spell->schools, SCHOOL_WATER);
		range_init(&spell->mana_range, 30, 60);
		spell_inertia_init(spell, 3, 40);
		spell->info_func = water_ice_storm_info;
		spell->effect_func = water_ice_storm;
		spell->failure_rate = 80;
		spell->skill_level = 22;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 65;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 25, 45);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ENTPOTION, "ENTPOTION", "Ent's Potion");
		string_list_append(&spell->description, "Fills up your stomach.");
		string_list_append(&spell->description, "At level 5 it boldens your heart.");
		string_list_append(&spell->description, "At level 12 it makes you heroic.");
		school_idx_add_new(&spell->schools, SCHOOL_WATER);
		range_init(&spell->mana_range, 7, 15);
		spell_inertia_init(spell, 1, 30);
		spell->info_func = water_ent_potion_info;
		spell->effect_func = water_ent_potion;
		spell->failure_rate = 35;
		spell->skill_level = 6;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&VAPOR, "VAPOR", "Vapor");
		string_list_append(&spell->description, "Fills the air with toxic moisture to eradicate annoying critters.");
		school_idx_add_new(&spell->schools, SCHOOL_WATER);
		range_init(&spell->mana_range, 2, 12);
		spell_inertia_init(spell, 1, 30);
		spell->info_func = water_vapor_info;
		spell->effect_func = water_vapor;
		spell->failure_rate = 20;
		spell->skill_level = 2;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&GEYSER, "GEYSER", "Geyser");
		string_list_append(&spell->description, "Shoots a geyser of water from your fingertips.");
		string_list_append(&spell->description, "Sometimes it can blast through its first target.");
		school_idx_add_new(&spell->schools, SCHOOL_WATER);
		range_init(&spell->mana_range, 1, 35);
		spell->info_func = water_geyser_info;
		spell->effect_func = water_geyser;
		spell->failure_rate = 5;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&NOXIOUSCLOUD, "NOXIOUSCLOUD", "Noxious Cloud");
		string_list_append(&spell->description, "Creates a cloud of poison");
		string_list_append(&spell->description, "The cloud will persist for some turns, damaging all monsters passing by");
		string_list_append(&spell->description, "At spell level 30 it turns into a thick gas attacking all living beings");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		range_init(&spell->mana_range, 3, 30);
		spell->info_func = air_noxious_cloud_info;
		spell->effect_func = air_noxious_cloud;
		spell->failure_rate = 20;
		spell->skill_level = 3;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 15;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 25, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&AIRWINGS, "AIRWINGS", "Wings of Winds");
		string_list_append(&spell->description, "Grants the power of levitation");
		string_list_append(&spell->description, "At level 16 it grants the power of controlled flight");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 30, 40);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = air_wings_of_winds_info;
		spell->effect_func = air_wings_of_winds;
		spell->failure_rate = 60;
		spell->skill_level = 22;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 27;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&INVISIBILITY, "INVISIBILITY", "Invisibility");
		string_list_append(&spell->description, "Grants invisibility");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		range_init(&spell->mana_range, 10, 20);
		spell_inertia_init(spell, 1, 30);
		spell->info_func = air_invisibility_info;
		spell->effect_func = air_invisibility;
		spell->failure_rate = 50;
		spell->skill_level = 16;
		spell_init_mage(spell, RANDOM);

	}

	{
		spell_type *spell = spell_new(&POISONBLOOD, "POISONBLOOD", "Poison Blood");
		string_list_append(&spell->description, "Grants resist poison");
		string_list_append(&spell->description, "At level 15 it provides poison branding to wielded weapon");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		range_init(&spell->mana_range, 10, 20);
		spell_inertia_init(spell, 1, 35);
		spell->info_func = air_poison_blood_info;
		spell->effect_func = air_poison_blood;
		spell->failure_rate = 30;
		spell->skill_level = 12;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "10+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 25);
			range_init(&device_allocation->max_level, 35, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&THUNDERSTORM, "THUNDERSTORM", "Thunderstorm");
		string_list_append(&spell->description, "Charges up the air around you with electricity");
		string_list_append(&spell->description, "Each turn it will throw a thunder bolt at a random monster in sight");
		string_list_append(&spell->description, "The thunder does 3 types of damage, one third of lightning");
		string_list_append(&spell->description, "one third of sound and one third of light");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 40, 60);
		spell_inertia_init(spell, 2, 15);
		spell->info_func = air_thunderstorm_info;
		spell->effect_func = air_thunderstorm;
		spell->failure_rate = 60;
		spell->skill_level = 25;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 25, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STERILIZE, "STERILIZE", "Sterilize");
		string_list_append(&spell->description, "Prevents explosive breeding for a while.");
		school_idx_add_new(&spell->schools, SCHOOL_AIR);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = air_sterilize_info;
		spell->effect_func = air_sterilize;
		spell->failure_rate = 50;
		spell->skill_level = 20;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 20;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STONESKIN, "STONESKIN", "Stone Skin");
		string_list_append(&spell->description, "Creates a shield of earth around you to protect you");
		string_list_append(&spell->description, "At level 25 it starts dealing damage to attackers");
		school_idx_add_new(&spell->schools, SCHOOL_EARTH);
		range_init(&spell->mana_range, 1, 50);
		spell_inertia_init(spell, 2, 50);
		spell->info_func = earth_stone_skin_info;
		spell->effect_func = earth_stone_skin;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&DIG, "DIG", "Dig");
		string_list_append(&spell->description, "Digs a hole in a wall much faster than any shovels");
		school_idx_add_new(&spell->schools, SCHOOL_EARTH);
		range_init(&spell->mana_range, 14, 14);
		spell->info_func = earth_dig_info;
		spell->effect_func = earth_dig;
		spell->failure_rate = 20;
		spell->skill_level = 12;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "15+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 25;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STONEPRISON, "STONEPRISON", "Stone Prison");
		string_list_append(&spell->description, "Creates a prison of walls around you");
		string_list_append(&spell->description, "At level 10 it allows you to target a monster");
		school_idx_add_new(&spell->schools, SCHOOL_EARTH);
		range_init(&spell->mana_range, 30, 50);
		spell->info_func = earth_stone_prison_info;
		spell->effect_func = earth_stone_prison;
		spell->failure_rate = 65;
		spell->skill_level = 25;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 57;
			range_init(&device_allocation->base_level, 1, 3);
			range_init(&device_allocation->max_level, 5, 20);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STRIKE, "STRIKE", "Strike");
		string_list_append(&spell->description, "Creates a micro-ball of force that will push monsters backwards");
		string_list_append(&spell->description, "If the monster is caught near a wall, it'll be crushed against it");
		string_list_append(&spell->description, "At level 12 it turns into a ball of radius 1");
		school_idx_add_new(&spell->schools, SCHOOL_EARTH);
		range_init(&spell->mana_range, 30, 50);
		spell->info_func = earth_strike_info;
		spell->effect_func = earth_strike;
		spell->failure_rate = 60;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "2+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 635;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SHAKE, "SHAKE", "Shake");
		string_list_append(&spell->description, "Creates a localised earthquake");
		string_list_append(&spell->description, "At level 10 it can be targeted at any location");
		school_idx_add_new(&spell->schools, SCHOOL_EARTH);
		range_init(&spell->mana_range, 25, 30);
		spell_inertia_init(spell, 2, 50);
		spell->info_func = earth_shake_info;
		spell->effect_func = earth_shake;
		spell->failure_rate = 60;
		spell->skill_level = 27;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 3);
			range_init(&device_allocation->max_level, 9, 20);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&BLINK, "BLINK", "Phase Door");
		string_list_append(&spell->description, "Teleports you on a small scale range");
		string_list_append(&spell->description, "At level 30 it creates void jumpgates");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 1, 3);
		spell_inertia_init(spell, 1, 5);
		spell->info_func = convey_blink_info;
		spell->effect_func = convey_blink;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&DISARM, "DISARM", "Disarm");
		string_list_append(&spell->description, "Destroys doors and traps");
		string_list_append(&spell->description, "At level 10 it destroys doors and traps, then reveals and unlocks any secret");
		string_list_append(&spell->description, "doors");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 2, 4);
		spell->info_func = convey_disarm_info;
		spell->effect_func = convey_disarm;
		spell->failure_rate = 15;
		spell->skill_level = 3;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "10+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 4;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 10, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TELEPORT, "TELEPORT", "Teleportation");
		string_list_append(&spell->description, "Teleports you around the level. The casting time decreases with level");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 8, 14);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = convey_teleport_info;
		spell->effect_func = convey_teleport;
		spell->failure_rate = 30;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TELEAWAY, "TELEAWAY", "Teleport Away");
		string_list_append(&spell->description, "Teleports a line of monsters away");
		string_list_append(&spell->description, "At level 10 it turns into a ball");
		string_list_append(&spell->description, "At level 20 it teleports all monsters in sight");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 15, 40);
		spell->info_func = convey_teleport_away_info;
		spell->effect_func = convey_teleport_away;
		spell->failure_rate = 60;
		spell->skill_level = 23;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECALL, "RECALL", "Recall");
		string_list_append(&spell->description, "Cast on yourself it will recall you to the surface/dungeon.");
		string_list_append(&spell->description, "Cast at a monster you will swap positions with the monster.");
		string_list_append(&spell->description, "Cast at an object it will fetch the object to you.");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 25, 25);
		spell->info_func = convey_recall_info;
		spell->effect_func = convey_recall;
		spell->failure_rate = 60;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&PROBABILITY_TRAVEL, "PROBABILITY_TRAVEL", "Probability Travel");
		string_list_append(&spell->description, "Renders you immaterial, when you hit a wall you travel through it and");
		string_list_append(&spell->description, "instantly appear on the other side of it. You can also float up and down");
		string_list_append(&spell->description, "at will");
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 30, 50);
		spell_inertia_init(spell, 6, 40);
		spell->info_func = convey_probability_travel_info;
		spell->effect_func = convey_probability_travel;
		spell->failure_rate = 90;
		spell->skill_level = 35;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 97;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 8, 25);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&GROWTREE, "GROWTREE", "Grow Trees");
		string_list_append(&spell->description, "Makes trees grow extremely quickly around you");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		school_idx_add_new(&spell->schools, SCHOOL_TEMPORAL);
		range_init(&spell->mana_range, 6, 30);
		spell_inertia_init(spell, 5, 50);
		spell->info_func = nature_grow_trees_info;
		spell->effect_func = nature_grow_trees;
		spell->failure_rate = 35;
		spell->skill_level = 6;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&HEALING, "HEALING", "Healing");
		string_list_append(&spell->description, "Heals a percent of hitpoints");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 15, 50);
		spell->info_func = nature_healing_info;
		spell->effect_func = nature_healing;
		spell->failure_rate = 45;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "2+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 90;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECOVERY, "RECOVERY", "Recovery");
		string_list_append(&spell->description, "Reduces the length of time that you are poisoned");
		string_list_append(&spell->description, "At level 5 it cures poison and cuts");
		string_list_append(&spell->description, "At level 10 it restores drained stats");
		string_list_append(&spell->description, "At level 15 it restores lost experience");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 10, 25);
		spell_inertia_init(spell, 2, 100);
		spell->info_func = nature_recovery_info;
		spell->effect_func = nature_recovery;
		spell->failure_rate = 60;
		spell->skill_level = 15;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 30);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&REGENERATION, "REGENERATION", "Regeneration");
		string_list_append(&spell->description, "Increases your body's regeneration rate");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 30, 55);
		spell_inertia_init(spell, 4, 40);
		spell->info_func = nature_regeneration_info;
		spell->effect_func = nature_regeneration;
		spell->failure_rate = 70;
		spell->skill_level = 20;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&SUMMONANNIMAL, "SUMMONANNIMAL", "Summon Animal");
		string_list_append(&spell->description, "Summons a leveled animal to your aid");
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 25, 50);
		spell->info_func = nature_summon_animal_info;
		spell->effect_func = nature_summon_animal;
		spell->failure_rate = 90;
		spell->skill_level = 25;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STARIDENTIFY, "STARIDENTIFY", "Greater Identify");
		string_list_append(&spell->description, "Asks for an object and fully identify it, providing the full list of powers");
		string_list_append(&spell->description, "Cast at yourself it will reveal your powers");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 30, 30);
		spell->info_func = divination_greater_identify_info;
		spell->effect_func = divination_greater_identify;
		spell->failure_rate = 80;
		spell->skill_level = 35;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&IDENTIFY, "IDENTIFY", "Identify");
		string_list_append(&spell->description, "Asks for an object and identifies it");
		string_list_append(&spell->description, "At level 17 it identifies all objects in the inventory");
		string_list_append(&spell->description, "At level 27 it identifies all objects in the inventory and in a");
		string_list_append(&spell->description, "radius on the floor, as well as probing monsters in that radius");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 10, 50);
		spell->info_func = divination_identify_info;
		spell->effect_func = divination_identify;
		spell->failure_rate = 40;
		spell->skill_level = 8;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 15, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&VISION, "VISION", "Vision");
		string_list_append(&spell->description, "Detects the layout of the surrounding area");
		string_list_append(&spell->description, "At level 25 it maps and lights the whole level");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 7, 55);
		spell_inertia_init(spell, 2, 200);
		spell->info_func = divination_vision_info;
		spell->effect_func = divination_vision;
		spell->failure_rate = 45;
		spell->skill_level = 15;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "4+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 60;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 30);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SENSEHIDDEN, "SENSEHIDDEN", "Sense Hidden");
		string_list_append(&spell->description, "Detects the traps in a certain radius around you");
		string_list_append(&spell->description, "At level 15 it allows you to sense invisible for a while");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 2, 10);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = divination_sense_hidden_info;
		spell->effect_func = divination_sense_hidden;
		spell->failure_rate = 25;
		spell->skill_level = 5;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 20;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&REVEALWAYS, "REVEALWAYS", "Reveal Ways");
		string_list_append(&spell->description, "Detects the doors/stairs/ways in a certain radius around you");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 3, 15);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = divination_reveal_ways_info;
		spell->effect_func = divination_reveal_ways;
		spell->failure_rate = 20;
		spell->skill_level = 9;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "6+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 25, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SENSEMONSTERS, "SENSEMONSTERS", "Sense Monsters");
		string_list_append(&spell->description, "Detects all monsters near you");
		string_list_append(&spell->description, "At level 30 it allows you to sense monster minds for a while");
		school_idx_add_new(&spell->schools, SCHOOL_DIVINATION);
		range_init(&spell->mana_range, 1, 20);
		spell_inertia_init(spell, 1, 10);
		spell->info_func = divination_sense_monsters_info;
		spell->effect_func = divination_sense_monsters;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 37;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 15, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&MAGELOCK, "MAGELOCK", "Magelock");
		string_list_append(&spell->description, "Magically locks a door");
		string_list_append(&spell->description, "At level 30 it creates a glyph of warding");
		string_list_append(&spell->description, "At level 40 the glyph can be placed anywhere in the field of vision");
		school_idx_add_new(&spell->schools, SCHOOL_TEMPORAL);
		range_init(&spell->mana_range, 1, 35);
		spell->info_func = tempo_magelock_info;
		spell->effect_func = tempo_magelock;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 30;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SLOWMONSTER, "SLOWMONSTER", "Slow Monster");
		string_list_append(&spell->description, "Magically slows down the passing of time around a monster");
		string_list_append(&spell->description, "At level 20 it affects a zone");
		school_idx_add_new(&spell->schools, SCHOOL_TEMPORAL);
		range_init(&spell->mana_range, 10, 15);
		spell->info_func = tempo_slow_monster_info;
		spell->effect_func = tempo_slow_monster;
		spell->failure_rate = 35;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 23;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ESSENCESPEED, "ESSENCESPEED", "Essence of Speed");
		string_list_append(&spell->description, "Magically decreases the passing of time around you, making you move faster with");
		string_list_append(&spell->description, "respect to the rest of the universe.");
		school_idx_add_new(&spell->schools, SCHOOL_TEMPORAL);
		range_init(&spell->mana_range, 20, 40);
		spell_inertia_init(spell, 5, 20);
		spell->info_func = tempo_essence_of_speed_info;
		spell->effect_func = tempo_essence_of_speed;
		spell->failure_rate = 50;
		spell->skill_level = 15;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 80;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 10, 39);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&BANISHMENT, "BANISHMENT", "Banishment");
		string_list_append(&spell->description, "Disrupts the space/time continuum in your area and teleports all monsters away.");
		string_list_append(&spell->description, "At level 15 it may also lock them in a time bubble for a while.");
		school_idx_add_new(&spell->schools, SCHOOL_TEMPORAL);
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 30, 40);
		spell_inertia_init(spell, 5, 50);
		spell->info_func = tempo_banishment_info;
		spell->effect_func = tempo_banishment;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 98;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 36);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECHARGE, "RECHARGE", "Recharge");
		string_list_append(&spell->description, "Taps on the ambient mana to recharge an object's power (charges or mana)");
		school_idx_add_new(&spell->schools, SCHOOL_META);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = meta_recharge_info;
		spell->effect_func = meta_recharge;
		spell->failure_rate = 20;
		spell->skill_level = 5;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&SPELLBINDER, "SPELLBINDER", "Spellbinder");
		string_list_append(&spell->description, "Stores spells in a trigger.");
		string_list_append(&spell->description, "When the condition is met all spells fire off at the same time");
		string_list_append(&spell->description, "This spell takes a long time to cast so you are advised to prepare it");
		string_list_append(&spell->description, "in a safe area.");
		string_list_append(&spell->description, "Also it will use the mana for the Spellbinder and the mana for the");
		string_list_append(&spell->description, "selected spells");
		school_idx_add_new(&spell->schools, SCHOOL_META);
		range_init(&spell->mana_range, 100, 300);
		spell->info_func = meta_spellbinder_info;
		spell->effect_func = meta_spellbinder;
		spell->failure_rate = 85;
		spell->skill_level = 20;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&DISPERSEMAGIC, "DISPERSEMAGIC", "Disperse Magic");
		string_list_append(&spell->description, "Dispels a lot of magic that can affect you, be it good or bad");
		string_list_append(&spell->description, "Level 1: blindness and light");
		string_list_append(&spell->description, "Level 5: confusion and hallucination");
		string_list_append(&spell->description, "Level 10: speed (both bad or good) and light speed");
		string_list_append(&spell->description, "Level 15: stunning, meditation, cuts");
		string_list_append(&spell->description, "Level 20: hero, super hero, bless, shields, afraid, parasites, mimicry");
		school_idx_add_new(&spell->schools, SCHOOL_META);
		range_init(&spell->mana_range, 30, 60);
		spell_inertia_init(spell, 1, 5);
		spell->info_func = meta_disperse_magic_info;
		spell->effect_func = meta_disperse_magic;
		spell->failure_rate = 40;
		spell->skill_level = 15;
		spell->castable_while_blind = TRUE;
		spell->castable_while_confused = TRUE;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 25;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 5, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TRACKER, "TRACKER", "Tracker");
		string_list_append(&spell->description, "Tracks down the last teleportation that happened on the level and teleports");
		string_list_append(&spell->description, "you to it");
		school_idx_add_new(&spell->schools, SCHOOL_META);
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 50, 50);
		spell->info_func = meta_tracker_info;
		spell->effect_func = meta_tracker;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&INERTIA_CONTROL, "INERTIA_CONTROL", "Inertia Control");
		string_list_append(&spell->description, "Changes the energy flow of a spell to be continuously recasted");
		string_list_append(&spell->description, "at a given interval. The inertia controlled spell reduces your");
		string_list_append(&spell->description, "maximum mana by four times its cost.");
		school_idx_add_new(&spell->schools, SCHOOL_META);
		range_init(&spell->mana_range, 300, 700);
		spell->info_func = meta_inertia_control_info;
		spell->effect_func = meta_inertia_control;
		spell->failure_rate = 95;
		spell->skill_level = 37;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&CHARM, "CHARM", "Charm");
		string_list_append(&spell->description, "Tries to manipulate the mind of a monster to make it friendly");
		string_list_append(&spell->description, "At level 15 it turns into a ball");
		string_list_append(&spell->description, "At level 35 it affects all monsters in sight");
		school_idx_add_new(&spell->schools, SCHOOL_MIND);
		range_init(&spell->mana_range, 1, 20);
		spell->info_func = mind_charm_info;
		spell->effect_func = mind_charm;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&CONFUSE, "CONFUSE", "Confuse");
		string_list_append(&spell->description, "Tries to manipulate the mind of a monster to confuse it");
		string_list_append(&spell->description, "At level 15 it turns into a ball");
		string_list_append(&spell->description, "At level 35 it affects all monsters in sight");
		school_idx_add_new(&spell->schools, SCHOOL_MIND);
		range_init(&spell->mana_range, 5, 30);
		spell->info_func = mind_confuse_info;
		spell->effect_func = mind_confuse;
		spell->failure_rate = 20;
		spell->skill_level = 5;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d4");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ARMOROFFEAR, "ARMOROFFEAR", "Armor of Fear");
		string_list_append(&spell->description, "Creates a shield of pure fear around you. Any monster attempting to hit you");
		string_list_append(&spell->description, "must save or flee");
		school_idx_add_new(&spell->schools, SCHOOL_MIND);
		range_init(&spell->mana_range, 10, 50);
		spell_inertia_init(spell, 2, 20);
		spell->info_func = mind_armor_of_fear_info;
		spell->effect_func = mind_armor_of_fear;
		spell->failure_rate = 35;
		spell->skill_level = 10;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&STUN, "STUN", "Stun");
		string_list_append(&spell->description, "Tries to manipulate the mind of a monster to stun it");
		string_list_append(&spell->description, "At level 20 it turns into a ball");
		school_idx_add_new(&spell->schools, SCHOOL_MIND);
		range_init(&spell->mana_range, 10, 90);
		spell->info_func = mind_stun_info;
		spell->effect_func = mind_stun;
		spell->failure_rate = 45;
		spell->skill_level = 15;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&DRAIN, "DRAIN", "Drain");
		string_list_append(&spell->description, "Drains the mana contained in wands, staves and rods to increase yours");
		school_idx_add_new(&spell->schools, SCHOOL_UDUN);
		school_idx_add_new(&spell->schools, SCHOOL_MANA);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = udun_drain_info;
		spell->effect_func = udun_drain;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&GENOCIDE, "GENOCIDE", "Genocide");
		string_list_append(&spell->description, "Genocides all monsters of a race on the level");
		string_list_append(&spell->description, "At level 10 it can genocide all monsters near you");
		school_idx_add_new(&spell->schools, SCHOOL_UDUN);
		school_idx_add_new(&spell->schools, SCHOOL_NATURE);
		range_init(&spell->mana_range, 50, 50);
		spell->info_func = udun_genocide_info;
		spell->effect_func = udun_genocide;
		spell->failure_rate = 90;
		spell->skill_level = 25;
		spell_init_mage(spell, RANDOM);

		dice_parse_checked(&spell->device_charges, "2+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 5, 15);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&WRAITHFORM, "WRAITHFORM", "Wraithform");
		string_list_append(&spell->description, "Turns you into an immaterial being");
		school_idx_add_new(&spell->schools, SCHOOL_UDUN);
		school_idx_add_new(&spell->schools, SCHOOL_CONVEYANCE);
		range_init(&spell->mana_range, 20, 40);
		spell_inertia_init(spell, 4, 30);
		spell->info_func = udun_wraithform_info;
		spell->effect_func = udun_wraithform;
		spell->failure_rate = 95;
		spell->skill_level = 30;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&FLAMEOFUDUN, "FLAMEOFUDUN", "Flame of Udun");
		string_list_append(&spell->description, "Turns you into a powerful Balrog");
		school_idx_add_new(&spell->schools, SCHOOL_UDUN);
		school_idx_add_new(&spell->schools, SCHOOL_FIRE);
		range_init(&spell->mana_range, 70, 100);
		spell_inertia_init(spell, 7, 15);
		spell->info_func = udun_flame_of_udun_info;
		spell->effect_func = udun_flame_of_udun;
		spell->failure_rate = 95;
		spell->skill_level = 35;
		spell_init_mage(spell, RANDOM);
	}

	{
		spell_type *spell = spell_new(&CALL_THE_ELEMENTS, "CALL_THE_ELEMENTS", "Call the Elements");
		string_list_append(&spell->description, "Randomly creates various elements around you");
		string_list_append(&spell->description, "Each type of element chance is controlled by your level");
		string_list_append(&spell->description, "in the corresponding skill");
		string_list_append(&spell->description, "At level 17 it can be targeted");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 2, 20);
		spell->info_func = geomancy_call_the_elements_info;
		spell->effect_func = geomancy_call_the_elements;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&CHANNEL_ELEMENTS, "CHANNEL_ELEMENTS", "Channel Elements");
		string_list_append(&spell->description, "Draws on the caster's immediate environs to form an attack or other effect.");
		string_list_append(&spell->description, "Grass/Flower heals.");
		string_list_append(&spell->description, "Water creates water bolt attacks.");
		string_list_append(&spell->description, "Ice creates ice bolt attacks.");
		string_list_append(&spell->description, "Sand creates a wall of thick, blinding, burning sand around you.");
		string_list_append(&spell->description, "Lava creates fire bolt attacks.");
		string_list_append(&spell->description, "Deep lava creates fire ball attacks.");
		string_list_append(&spell->description, "Chasm creates darkness bolt attacks.");
		string_list_append(&spell->description, "At Earth level 18, darkness becomes nether.");
		string_list_append(&spell->description, "At Water level 8, water attacks become beams with a striking effect.");
		string_list_append(&spell->description, "At Water level 12, ice attacks become balls of ice shards.");
		string_list_append(&spell->description, "At Water level 18, water attacks push monsters back.");
		string_list_append(&spell->description, "At Fire level 15, fire become hellfire.");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 3, 30);
		spell->info_func = geomancy_channel_elements_info;
		spell->effect_func = geomancy_channel_elements;
		spell->failure_rate = 20;
		spell->skill_level = 3;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&ELEMENTAL_WAVE, "ELEMENTAL_WAVE", "Elemental Wave");
		string_list_append(&spell->description, "Draws on an adjacent special square to project a slow-moving");
		string_list_append(&spell->description, "wave of that element in that direction");
		string_list_append(&spell->description, "Abyss squares cannot be channeled into a wave.");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 15, 50);
		spell->info_func = geomancy_elemental_wave_info;
		spell->effect_func = geomancy_elemental_wave;
		spell->failure_rate = 20;
		spell->skill_level = 15;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&VAPORIZE, "VAPORIZE", "Vaporize");
		string_list_append(&spell->description, "Draws upon your immediate environs to form a cloud of damaging vapors");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 3, 30);
		spell->info_func = geomancy_vaporize_info;
		spell->effect_func = geomancy_vaporize;
		spell->depend_func = geomancy_vaporize_depends;
		spell->failure_rate = 15;
		spell->skill_level = 4;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&GEOLYSIS, "GEOLYSIS", "Geolysis");
		string_list_append(&spell->description, "Burrows deeply and slightly at random into a wall,");
		string_list_append(&spell->description, "leaving behind tailings of various different sorts of walls in the passage.");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 15, 40);
		spell->info_func = geomancy_geolysis_info;
		spell->effect_func = geomancy_geolysis;
		spell->depend_func = geomancy_geolysis_depends;
		spell->failure_rate = 15;
		spell->skill_level = 7;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DRIPPING_TREAD, "DRIPPING_TREAD", "Dripping Tread");
		string_list_append(&spell->description, "Causes you to leave random elemental forms behind as you walk");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 15, 25);
		spell->info_func = geomancy_dripping_tread_info;
		spell->effect_func = geomancy_dripping_tread;
		spell->depend_func = geomancy_dripping_tread_depends;
		spell->failure_rate = 15;
		spell->skill_level = 10;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&GROW_BARRIER, "GROW_BARRIER", "Grow Barrier");
		string_list_append(&spell->description, "Creates impassable terrain (walls, trees, etc.) around you.");
		string_list_append(&spell->description, "At level 20 it can be projected around another area.");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 30, 40);
		spell->info_func = geomancy_grow_barrier_info;
		spell->effect_func = geomancy_grow_barrier;
		spell->depend_func = geomancy_grow_barrier_depends;
		spell->failure_rate = 15;
		spell->skill_level = 12;
		spell->castable_while_blind = TRUE;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&ELEMENTAL_MINION, "ELEMENTAL_MINION", "Elemental Minion");
		string_list_append(&spell->description, "Summons a minion from a nearby element.");
		string_list_append(&spell->description, "Walls can summon Earth elmentals, Xorns and Xarens");
		string_list_append(&spell->description, "Dark Pits can summon Air elementals, Ancient blue dragons, Great Storm Wyrms");
		string_list_append(&spell->description, "and Sky Drakes");
		string_list_append(&spell->description, "Sandwalls and lava can summon Fire elementals and Ancient red dragons");
		string_list_append(&spell->description, "Icewall, and water can summon Water elementals, Water trolls and Water demons");
		school_idx_add_new(&spell->schools, SCHOOL_GEOMANCY);
		range_init(&spell->mana_range, 40, 80);
		spell->info_func = geomancy_elemental_minion_info;
		spell->effect_func = geomancy_elemental_minion;
		spell->failure_rate = 25;
		spell->skill_level = 20;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&ERU_SEE, "ERU_SEE", "See the Music");
		string_list_append(&spell->description, "Allows you to 'see' the Great Music from which the world");
		string_list_append(&spell->description, "originates, allowing you to see unseen things");
		string_list_append(&spell->description, "At level 10 it allows you to see your surroundings");
		string_list_append(&spell->description, "At level 20 it allows you to cure blindness");
		string_list_append(&spell->description, "At level 30 it allows you to fully see all the level");
		school_idx_add_new(&spell->schools, SCHOOL_ERU);
		range_init(&spell->mana_range, 1, 50);
		spell->info_func = eru_see_the_music_info;
		spell->effect_func = eru_see_the_music;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
		spell->castable_while_blind = TRUE;
	}

	{
		spell_type *spell = spell_new(&ERU_LISTEN, "ERU_LISTEN", "Listen to the Music");
		string_list_append(&spell->description, "Allows you to listen to the Great Music from which the world");
		string_list_append(&spell->description, "originates, allowing you to understand the meaning of things");
		string_list_append(&spell->description, "At level 14 it allows you to identify all your pack");
		string_list_append(&spell->description, "At level 30 it allows you to identify all items on the level");
		school_idx_add_new(&spell->schools, SCHOOL_ERU);
		range_init(&spell->mana_range, 15, 200);
		spell->info_func = eru_listen_to_the_music_info;
		spell->effect_func = eru_listen_to_the_music;
		spell->failure_rate = 25;
		spell->skill_level = 7;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ERU_UNDERSTAND, "ERU_UNDERSTAND", "Know the Music");
		string_list_append(&spell->description, "Allows you to understand the Great Music from which the world");
		string_list_append(&spell->description, "originates, allowing you to know the full abilities of things");
		string_list_append(&spell->description, "At level 10 it allows you to *identify* all your pack");
		school_idx_add_new(&spell->schools, SCHOOL_ERU);
		range_init(&spell->mana_range, 200, 600);
		spell->info_func = eru_know_the_music_info;
		spell->effect_func = eru_know_the_music;
		spell->failure_rate = 50;
		spell->skill_level = 30;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&ERU_PROT, "ERU_PROT", "Lay of Protection");
		string_list_append(&spell->description, "Creates a circle of safety around you");
		school_idx_add_new(&spell->schools, SCHOOL_ERU);
		range_init(&spell->mana_range, 400, 400);
		spell->info_func = eru_lay_of_protection_info;
		spell->effect_func = eru_lay_of_protection;
		spell->failure_rate = 80;
		spell->skill_level = 35;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANWE_SHIELD, "MANWE_SHIELD", "Wind Shield");
		string_list_append(&spell->description, "It surrounds you with a shield of wind that deflects blows from evil monsters");
		string_list_append(&spell->description, "At level 10 it increases your armour rating");
		string_list_append(&spell->description, "At level 20 it retaliates against monsters that melee you");
		school_idx_add_new(&spell->schools, SCHOOL_MANWE);
		range_init(&spell->mana_range, 100, 500);
		spell->info_func = manwe_wind_shield_info;
		spell->effect_func = manwe_wind_shield;
		spell->failure_rate = 30;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANWE_AVATAR, "MANWE_AVATAR", "Avatar");
		string_list_append(&spell->description, "It turns you into a full grown Maia");
		school_idx_add_new(&spell->schools, SCHOOL_MANWE);
		range_init(&spell->mana_range, 1000, 1000);
		spell->info_func = manwe_avatar_info;
		spell->effect_func = manwe_avatar;
		spell->failure_rate = 80;
		spell->skill_level = 35;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANWE_BLESS, "MANWE_BLESS", "Manwe's Blessing");
		string_list_append(&spell->description, "Manwe's Blessing removes your fears, blesses you and surrounds you with");
		string_list_append(&spell->description, "holy light");
		string_list_append(&spell->description, "At level 10 it also grants heroism");
		string_list_append(&spell->description, "At level 20 it also grants super heroism");
		string_list_append(&spell->description, "At level 30 it also grants holy luck and life protection");
		school_idx_add_new(&spell->schools, SCHOOL_MANWE);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = manwe_blessing_info;
		spell->effect_func = manwe_blessing;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MANWE_CALL, "MANWE_CALL", "Manwe's Call");
		string_list_append(&spell->description, "Manwe's Call summons a Great Eagle to help you battle the forces");
		string_list_append(&spell->description, "of Morgoth");
		school_idx_add_new(&spell->schools, SCHOOL_MANWE);
		range_init(&spell->mana_range, 200, 500);
		spell->info_func = manwe_call_info;
		spell->effect_func = manwe_call;
		spell->failure_rate = 40;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&TULKAS_AIM, "TULKAS_AIM", "Divine Aim");
		string_list_append(&spell->description, "It makes you more accurate in combat");
		string_list_append(&spell->description, "At level 20 all your blows are critical hits");
		school_idx_add_new(&spell->schools, SCHOOL_TULKAS);
		range_init(&spell->mana_range, 30, 500);
		spell->info_func = tulkas_divine_aim_info;
		spell->effect_func = tulkas_divine_aim;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&TULKAS_WAVE, "TULKAS_WAVE", "Wave of Power");
		string_list_append(&spell->description, "It allows you to project a number of melee blows across a distance");
		school_idx_add_new(&spell->schools, SCHOOL_TULKAS);
		range_init(&spell->mana_range, 200, 200);
		spell->info_func = tulkas_wave_of_power_info;
		spell->effect_func = tulkas_wave_of_power;
		spell->failure_rate = 75;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&TULKAS_SPIN, "TULKAS_SPIN", "Whirlwind");
		string_list_append(&spell->description, "It allows you to spin around and hit all monsters nearby");
		school_idx_add_new(&spell->schools, SCHOOL_TULKAS);
		range_init(&spell->mana_range, 100, 100);
		spell->info_func = tulkas_whirlwind_info;
		spell->effect_func = tulkas_whirlwind;
		spell->failure_rate = 45;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MELKOR_CURSE, "MELKOR_CURSE", "Curse");
		string_list_append(&spell->description, "It curses a monster, reducing its melee power");
		string_list_append(&spell->description, "At level 5 it can be auto-casted (with no piety cost) while fighting");
		string_list_append(&spell->description, "At level 15 it also reduces armor");
		string_list_append(&spell->description, "At level 25 it also reduces speed");
		string_list_append(&spell->description, "At level 35 it also reduces max life (but it is never fatal)");
		school_idx_add_new(&spell->schools, SCHOOL_MELKOR);
		range_init(&spell->mana_range, 50, 300);
		spell->info_func = melkor_curse_info;
		spell->effect_func = melkor_curse;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MELKOR_CORPSE_EXPLOSION, "MELKOR_CORPSE_EXPLOSION", "Corpse Explosion");
		string_list_append(&spell->description, "It makes corpses in an area around you explode for a percent of their");
		string_list_append(&spell->description, "hit points as damage");
		school_idx_add_new(&spell->schools, SCHOOL_MELKOR);
		range_init(&spell->mana_range, 100, 500);
		spell->info_func = melkor_corpse_explosion_info;
		spell->effect_func = melkor_corpse_explosion;
		spell->failure_rate = 45;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&MELKOR_MIND_STEAL, "MELKOR_MIND_STEAL", "Mind Steal");
		string_list_append(&spell->description, "It allows your spirit to temporarily leave your own body, which will");
		string_list_append(&spell->description, "be vulnerable, to control one of your enemies body.");
		school_idx_add_new(&spell->schools, SCHOOL_MELKOR);
		range_init(&spell->mana_range, 1000, 3000);
		spell->info_func = melkor_mind_steal_info;
		spell->effect_func = melkor_mind_steal;
		spell->failure_rate = 90;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_CHARM_ANIMAL, "YAVANNA_CHARM_ANIMAL", "Charm Animal");
		string_list_append(&spell->description, "It tries to tame an animal");
		school_idx_add_new(&spell->schools, SCHOOL_YAVANNA);
		range_init(&spell->mana_range, 10, 100);
		spell->info_func = yavanna_charm_animal_info;
		spell->effect_func = yavanna_charm_animal;
		spell->failure_rate = 30;
		spell->skill_level = 1;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_GROW_GRASS, "YAVANNA_GROW_GRASS", "Grow Grass");
		string_list_append(&spell->description, "Create a floor of grass around you. While on grass and praying");
		string_list_append(&spell->description, "a worshipper of Yavanna will know a greater regeneration rate");
		school_idx_add_new(&spell->schools, SCHOOL_YAVANNA);
		range_init(&spell->mana_range, 70, 150);
		spell->info_func = yavanna_grow_grass_info;
		spell->effect_func = yavanna_grow_grass;
		spell->failure_rate = 65;
		spell->skill_level = 10;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_TREE_ROOTS, "YAVANNA_TREE_ROOTS", "Tree Roots");
		string_list_append(&spell->description, "Creates roots deep in the floor from your feet, making you more stable and able");
		string_list_append(&spell->description, "to make stronger attacks, but prevents any movement (even teleportation).");
		string_list_append(&spell->description, "It also makes you recover from stunning almost immediately.");
		school_idx_add_new(&spell->schools, SCHOOL_YAVANNA);
		range_init(&spell->mana_range, 50, 1000);
		spell->info_func = yavanna_tree_roots_info;
		spell->effect_func = yavanna_tree_roots;
		spell->failure_rate = 70;
		spell->skill_level = 15;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_WATER_BITE, "YAVANNA_WATER_BITE", "Water Bite");
		string_list_append(&spell->description, "Imbues your melee weapon with a natural stream of water");
		string_list_append(&spell->description, "At level 25, it spreads over a 1 radius zone around your target");
		school_idx_add_new(&spell->schools, SCHOOL_YAVANNA);
		range_init(&spell->mana_range, 150, 300);
		spell->info_func = yavanna_water_bite_info;
		spell->effect_func = yavanna_water_bite;
		spell->failure_rate = 90;
		spell->skill_level = 20;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_UPROOT, "YAVANNA_UPROOT", "Uproot");
		string_list_append(&spell->description, "Awakes a tree to help you battle the forces of Morgoth");
		school_idx_add_new(&spell->schools, SCHOOL_YAVANNA);
		range_init(&spell->mana_range, 250, 350);
		spell->info_func = yavanna_uproot_info;
		spell->effect_func = yavanna_uproot;
		spell->failure_rate = 95;
		spell->skill_level = 35;
		spell_init_priest(spell);
	}

	{
		spell_type *spell = spell_new(&DEMON_BLADE, "DEMON_BLADE", "Demon Blade");
		string_list_append(&spell->description, "Imbues your blade with fire to deal more damage");
		string_list_append(&spell->description, "At level 30 it deals hellfire damage");
		string_list_append(&spell->description, "At level 45 it spreads over a 1 radius zone around your target");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 4, 44);
		spell->info_func = demonology_demon_blade_info;
		spell->effect_func = demonology_demon_blade;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "3+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 17);
			range_init(&device_allocation->max_level, 20, 40);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEMON_MADNESS, "DEMON_MADNESS", "Demon Madness");
		string_list_append(&spell->description, "Fire 2 balls in opposite directions of randomly chaos, confusion or charm");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 5, 20);
		spell->info_func = demonology_demon_madness_info;
		spell->effect_func = demonology_demon_madness;
		spell->failure_rate = 25;
		spell->skill_level = 10;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEMON_FIELD, "DEMON_FIELD", "Demon Field");
		string_list_append(&spell->description, "Fires a cloud of deadly nexus over a radius of 7");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 20, 60);
		spell->info_func = demonology_demon_field_info;
		spell->effect_func = demonology_demon_field;
		spell->failure_rate = 60;
		spell->skill_level = 20;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DOOM_SHIELD, "DOOM_SHIELD", "Doom Shield");
		string_list_append(&spell->description, "Raises a mirror of pain around you, doing very high damage to your foes");
		string_list_append(&spell->description, "that dare hit you, but greatly reduces your armour class");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 2, 30);
		spell->info_func = demonology_doom_shield_info;
		spell->effect_func = demonology_doom_shield;
		spell->failure_rate = 10;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&UNHOLY_WORD, "UNHOLY_WORD", "Unholy Word");
		string_list_append(&spell->description, "Kills a pet to heal you");
		string_list_append(&spell->description, "There is a chance that the pet won't die but will turn against you");
		string_list_append(&spell->description, "it will decrease with higher level");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 15, 45);
		spell->info_func = demonology_unholy_word_info;
		spell->effect_func = demonology_unholy_word;
		spell->failure_rate = 55;
		spell->skill_level = 25;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEMON_CLOAK, "DEMON_CLOAK", "Demon Cloak");
		string_list_append(&spell->description, "Raises a mirror that can reflect bolts and arrows for a time");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 10, 40);
		spell->info_func = demonology_demon_cloak_info;
		spell->effect_func = demonology_demon_cloak;
		spell->failure_rate = 70;
		spell->skill_level = 20;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEMON_SUMMON, "DEMON_SUMMON", "Summon Demon");
		string_list_append(&spell->description, "Summons a leveled demon to your side");
		string_list_append(&spell->description, "At level 35 it summons a high demon");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 10, 50);
		spell->info_func = demonology_summon_demon_info;
		spell->effect_func = demonology_summon_demon;
		spell->failure_rate = 30;
		spell->skill_level = 5;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DISCHARGE_MINION, "DISCHARGE_MINION", "Discharge Minion");
		string_list_append(&spell->description, "The targeted pet will explode in a burst of gravity");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 20, 50);
		spell->info_func = demonology_discharge_minion_info;
		spell->effect_func = demonology_discharge_minion;
		spell->failure_rate = 30;
		spell->skill_level = 10;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&CONTROL_DEMON, "CONTROL_DEMON", "Control Demon");
		string_list_append(&spell->description, "Attempts to control a demon");
		school_idx_add_new(&spell->schools, SCHOOL_DEMON);
		range_init(&spell->mana_range, 30, 70);
		spell->info_func = demonology_control_demon_info;
		spell->effect_func = demonology_control_demon;
		spell->failure_rate = 55;
		spell->skill_level = 25;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEVICE_HEAL_MONSTER, "DEVICE_HEAL_MONSTER", "Heal Monster");
		string_list_append(&spell->description, "Heals a monster");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 5, 20);
		spell->info_func = device_heal_monster_info;
		spell->effect_func = device_heal_monster;
		spell->failure_rate = 15;
		spell->skill_level = 3;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "10+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 17;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_SPEED_MONSTER, "DEVICE_SPEED_MONSTER", "Haste Monster");
		string_list_append(&spell->description, "Haste a monster");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 10, 10);
		spell->info_func = device_haste_monster_info;
		spell->effect_func = device_haste_monster;
		spell->failure_rate = 30;
		spell->skill_level = 10;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "10+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 7;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 20, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_WISH, "DEVICE_WISH", "Wish");
		string_list_append(&spell->description, "This grants you a wish, beware of what you ask for!");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 400, 400);
		spell->info_func = device_wish_info;
		spell->effect_func = device_wish;
		spell->failure_rate = 99;
		spell->skill_level = 50;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 98;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_SUMMON, "DEVICE_SUMMON", "Summon");
		string_list_append(&spell->description, "Summons hostile monsters near you");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 5, 25);
		spell->info_func = device_summon_monster_info;
		spell->effect_func = device_summon_monster;
		spell->failure_rate = 20;
		spell->skill_level = 5;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "1+d20");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 13;
			range_init(&device_allocation->base_level, 1, 40);
			range_init(&device_allocation->max_level, 25, 50);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_MANA, "DEVICE_MANA", "Mana");
		string_list_append(&spell->description, "Restores a part(or all) of your mana");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 1, 1);
		spell->info_func = device_mana_info;
		spell->effect_func = device_mana;
		spell->failure_rate = 80;
		spell->skill_level = 30;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "2+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 78;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 35);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_NOTHING, "DEVICE_NOTHING", "Nothing");
		string_list_append(&spell->description, "It does nothing.");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_nothing_info;
		spell->effect_func = device_nothing;
		spell->failure_rate = 0;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "0+d0");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 3;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 3;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_MAGGOT, "DEVICE_MAGGOT", "Artifact Maggot");
		dice_parse_checked(&spell->activation_duration, "10+d50");
		string_list_append(&spell->description, "terrify");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 7, 7);
		spell->info_func = device_maggot_info;
		spell->effect_func = device_maggot;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&DEVICE_HOLY_FIRE, "DEVICE_HOLY_FIRE", "Holy Fire of Mithrandir");
		string_list_append(&spell->description, "The Holy Fire created by this staff will deeply(double damage) burn");
		string_list_append(&spell->description, "all that is evil.");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 50, 150);
		spell->info_func = device_holy_fire_info;
		spell->effect_func = device_holy_fire;
		spell->failure_rate = 75;
		spell->skill_level = 30;
		spell_init_mage(spell, NO_RANDOM);

		dice_parse_checked(&spell->device_charges, "2+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 35, 35);
			sglib_device_allocation_add(&spell->device_allocation, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_ETERNAL_FLAME, "DEVICE_ETERNAL_FLAME", "Artifact Eternal Flame");
		dice_parse_checked(&spell->activation_duration, "0");
		string_list_append(&spell->description, "Imbuing an object with the eternal fire");
		school_idx_add_new(&spell->schools, SCHOOL_DEVICE);
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = device_eternal_flame_info;
		spell->effect_func = device_eternal_flame;
		spell->failure_rate = 0;
		spell->skill_level = 1;
		spell_init_mage(spell, NO_RANDOM);
	}

	{
		spell_type *spell = spell_new(&MUSIC_STOP, "MUSIC_STOP", "Stop singing(I)");
		string_list_append(&spell->description, "Stops the current song, if any.");
		range_init(&spell->mana_range, 0, 0);
		spell->info_func = music_stop_singing_info;
		spell->effect_func = music_stop_singing_spell;
		spell->failure_rate = -400;
		spell->skill_level = 1;
		spell->castable_while_blind = TRUE;
		spell_init_music(spell, 1);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HOLD, "MUSIC_HOLD", "Holding Pattern(I)");
		string_list_append(&spell->description, "Slows down all monsters listening the song.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 1, 10);
		spell->info_func = music_holding_pattern_info;
		spell->effect_func = music_holding_pattern_spell;
		spell->lasting_func = music_holding_pattern_lasting;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell->castable_while_blind = TRUE;
		spell_init_music(spell, 1);
	}

	{
		spell_type *spell = spell_new(&MUSIC_CONF, "MUSIC_CONF", "Illusion Pattern(II)");
		string_list_append(&spell->description, "Tries to confuse all monsters listening the song.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 2, 15);
		spell->info_func = music_illusion_pattern_info;
		spell->effect_func = music_illusion_pattern_spell;
		spell->lasting_func = music_illusion_pattern_lasting;
		spell->failure_rate = 30;
		spell->skill_level = 5;
		spell->castable_while_blind = TRUE;
		spell_init_music(spell, 2);
	}

	{
		spell_type *spell = spell_new(&MUSIC_STUN, "MUSIC_STUN", "Stun Pattern(IV)");
		string_list_append(&spell->description, "Stuns all monsters listening the song.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 3, 25);
		spell->info_func = music_stun_pattern_info;
		spell->effect_func = music_stun_pattern_spell;
		spell->lasting_func = music_stun_pattern_lasting;
		spell->failure_rate = 45;
		spell->skill_level = 10;
		spell->castable_while_blind = TRUE;
		spell_init_music(spell, 4);
	}

	{
		spell_type *spell = spell_new(&MUSIC_LITE, "MUSIC_LITE", "Song of the Sun(I)");
		string_list_append(&spell->description, "Provides light as long as you sing.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 1, 1);
		spell->info_func = music_song_of_the_sun_info;
		spell->effect_func = music_song_of_the_sun_spell;
		spell->lasting_func = music_song_of_the_sun_lasting;
		spell->failure_rate = 20;
		spell->skill_level = 1;
		spell->castable_while_blind = TRUE;
		spell_init_music(spell, 1);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HEAL, "MUSIC_HEAL", "Flow of Life(II)");
		string_list_append(&spell->description, "Heals you as long as you sing.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 5, 30);
		spell->info_func = music_flow_of_life_info;
		spell->effect_func = music_flow_of_life_spell;
		spell->lasting_func = music_flow_of_life_lasting;
		spell->failure_rate = 35;
		spell->skill_level = 7;
		spell_init_music(spell, 2);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HERO, "MUSIC_HERO", "Heroic Ballad(II)");
		string_list_append(&spell->description, "Increases melee accuracy");
		string_list_append(&spell->description, "At level 10 it increases it even more and reduces armour a bit");
		string_list_append(&spell->description, "At level 20 it increases it again");
		string_list_append(&spell->description, "At level 25 it grants protection against chaos and confusion");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 4, 14);
		spell->info_func = music_heroic_ballad_info;
		spell->effect_func = music_heroic_ballad_spell;
		spell->lasting_func = music_heroic_ballad_lasting;
		spell->failure_rate = 45;
		spell->skill_level = 10;
		spell_init_music(spell, 2);
	}

	{
		spell_type *spell = spell_new(&MUSIC_TIME, "MUSIC_TIME", "Hobbit Melodies(III)");
		string_list_append(&spell->description, "Greatly increases your reflexes allowing you to block more melee blows.");
		string_list_append(&spell->description, "At level 15 it also makes you faster.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 10, 30);
		spell->info_func = music_hobbit_melodies_info;
		spell->effect_func = music_hobbit_melodies_spell;
		spell->lasting_func = music_hobbit_melodies_lasting;
		spell->failure_rate = 70;
		spell->skill_level = 20;
		spell_init_music(spell, 3);
	}

	{
		spell_type *spell = spell_new(&MUSIC_MIND, "MUSIC_MIND", "Clairaudience(IV)");
		string_list_append(&spell->description, "Allows you to sense monster minds as long as you sing.");
		string_list_append(&spell->description, "At level 10 it identifies all objects in a radius on the floor,");
		string_list_append(&spell->description, "as well as probing monsters in that radius.");
		string_list_append(&spell->description, "Consumes the amount of mana each turn.");
		range_init(&spell->mana_range, 15, 30);
		spell->info_func = music_clairaudience_info;
		spell->effect_func = music_clairaudience_spell;
		spell->lasting_func = music_clairaudience_lasting;
		spell->failure_rate = 75;
		spell->skill_level = 25;
		spell_init_music(spell, 4);
	}

	{
		spell_type *spell = spell_new(&MUSIC_BLOW, "MUSIC_BLOW", "Blow(I)");
		string_list_append(&spell->description, "Produces a powerful, blowing, sound all around you.");
		range_init(&spell->mana_range, 3, 30);
		spell->info_func = music_blow_info;
		spell->effect_func = music_blow_spell;
		spell->failure_rate = 20;
		spell->skill_level = 4;
		spell_init_music(spell, 1);
	}

	{
		spell_type *spell = spell_new(&MUSIC_WIND, "MUSIC_WIND", "Gush of Wind(II)");
		string_list_append(&spell->description, "Produces a outgoing gush of wind that sends monsters away.");
		range_init(&spell->mana_range, 15, 45);
		spell->info_func = music_gush_of_wind_info;
		spell->effect_func = music_gush_of_wind_spell;
		spell->failure_rate = 30;
		spell->skill_level = 14;
		spell_init_music(spell, 2);
	}

	{
		spell_type *spell = spell_new(&MUSIC_YLMIR, "MUSIC_YLMIR", "Horns of Ylmir(III)");
		string_list_append(&spell->description, "Produces an earth shaking sound.");
		range_init(&spell->mana_range, 25, 30);
		spell->info_func = music_horns_of_ylmir_info;
		spell->effect_func = music_horns_of_ylmir_spell;
		spell->failure_rate = 20;
		spell->skill_level = 20;
		spell_init_music(spell, 3);
	}

	{
		spell_type *spell = spell_new(&MUSIC_AMBARKANTA, "MUSIC_AMBARKANTA", "Ambarkanta(IV)");
		string_list_append(&spell->description, "Produces a reality shaking sound that transports you to a nearly");
		string_list_append(&spell->description, "identical reality.");
		range_init(&spell->mana_range, 70, 70);
		spell->info_func = music_ambarkanta_info;
		spell->effect_func = music_ambarkanta_spell;
		spell->failure_rate = 60;
		spell->skill_level = 25;
		spell_init_music(spell, 4);
	}

	/* Module-specific spells */
	switch (game_module_idx)
	{
	case MODULE_TOME:
		spells_init_tome();
		break;
	case MODULE_THEME:
		spells_init_theme();
		break;
	default:
		assert(FALSE);
	}

}
