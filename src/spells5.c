#include <angband.h>

#include <assert.h>

#include "spell_type.h"

static spell_type *spell_new(s32b *index, cptr id, cptr name)
{
	assert(school_spells_count < SCHOOL_SPELLS_MAX);

	spell_type *spell = spell_type_new(name);
	school_spells[school_spells_count] = spell;
	*index = school_spells_count;
	school_spells_count++;

	return spell;
}

spell_type *spell_at(s32b index)
{
	assert(index >= 0);
	assert(index < school_spells_count);

	return school_spells[index];
}

int find_spell(cptr name)
{
	int i;

	for (i = 0; i < school_spells_count; i++)
	{
		if (streq(spell_type_name(spell_at(i)), name))
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

		if ((spell_type_random_type(spell) == random_type) &&
		    (rand_int(spell_type_skill_level(spell) * 3) < level))
		{
			return spl;
		}
	}

	return -1;
}

static void spells_init_tome()
{
	{
		spell_type *spell = spell_new(&DEVICE_LEBOHAUM, "DEVICE_LEBOHAUM", "Artifact Lebauhaum");
		spell_type_set_activation_timeout(spell, "3");
		spell_type_describe(spell, "sing a cheerful song");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 0);
		spell_type_init_device(spell,
				       device_lebohaum_info,
				       device_lebohaum);
	}

	{
		spell_type *spell = spell_new(&DEVICE_DURANDIL, "DEVICE_DURANDIL", "Artifact Durandil");
		spell_type_set_activation_timeout(spell, "3");
		spell_type_describe(spell, "sing a cheerful song");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 0);
		spell_type_init_device(spell,
				       device_durandil_info,
				       device_durandil);
	}

	{
		spell_type *spell = spell_new(&DEVICE_THUNDERLORDS, "DEVICE_THUNDERLORDS", "Artifact Thunderlords");
		spell_type_describe(spell, "A thunderlord will appear to transport you quickly to the surface.");
		spell_type_set_mana(spell, 1, 1);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_device(spell,
				       device_thunderlords_info,
				       device_thunderlords);

		spell_type_set_device_charges(spell, "3+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}
}

static void spells_init_theme()
{
	{
		spell_type *spell = spell_new(&GROW_ATHELAS, "GROW_ATHELAS", "Grow Athelas");
		spell_type_describe(spell, "Cures the Black Breath");
		spell_type_set_mana(spell, 60, 100);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_grow_athelas_info,
				     nature_grow_athelas);

		spell_type_set_device_charges(spell, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&AULE_FIREBRAND, "AULE_FIREBRAND", "Firebrand");
		spell_type_describe(spell, "Imbues your melee weapon with fire to deal more damage");
		spell_type_describe(spell, "At level 15 it spreads over a 1 radius zone around your target");
		spell_type_describe(spell, "At level 30 it deals holy fire damage");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_AULE,
				       aule_firebrand_info,
				       aule_firebrand_spell);
	}

	{
		spell_type *spell = spell_new(&AULE_ENCHANT_WEAPON, "AULE_ENCHANT_WEAPON", "Enchant Weapon");
		spell_type_describe(spell, "Tries to enchant a weapon to-hit");
		spell_type_describe(spell, "At level 5 it also enchants to-dam");
		spell_type_describe(spell, "At level 45 it enhances the special powers of magical weapons");
		spell_type_describe(spell, "The might of the enchantment increases with the level");
		spell_type_set_mana(spell, 100, 200);
		spell_type_set_difficulty(spell, 10, 20);
		spell_type_init_priest(spell,
				       SCHOOL_AULE,
				       aule_enchant_weapon_info,
				       aule_enchant_weapon_spell);
	}

	{
		spell_type *spell = spell_new(&AULE_ENCHANT_ARMOUR, "AULE_ENCHANT_ARMOUR", "Enchant Armour");
		spell_type_describe(spell, "Tries to enchant a piece of armour");
		spell_type_describe(spell, "At level 20 it also enchants to-hit and to-dam");
		spell_type_describe(spell, "At level 40 it enhances the special powers of magical armour");
		spell_type_describe(spell, "The might of the enchantment increases with the level");
		spell_type_set_mana(spell, 100, 200);
		spell_type_set_difficulty(spell, 15, 20);
		spell_type_init_priest(spell,
				       SCHOOL_AULE,
				       aule_enchant_armour_info,
				       aule_enchant_armour_spell);
	}

	{
		spell_type *spell = spell_new(&AULE_CHILD, "AULE_CHILD", "Child of Aule");
		spell_type_describe(spell, "Summons a levelled Dwarven warrior to help you battle the forces");
		spell_type_describe(spell, "of Morgoth");
		spell_type_set_mana(spell, 200, 500);
		spell_type_set_difficulty(spell, 20, 40);
		spell_type_init_priest(spell,
				       SCHOOL_AULE,
				       aule_child_info,
				       aule_child_spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_LIGHT_VALINOR, "VARDA_LIGHT_VALINOR", "Light of Valinor");
		spell_type_describe(spell, "Lights a room");
		spell_type_describe(spell, "At level 3 it starts damaging monsters");
		spell_type_describe(spell, "At level 15 it starts creating a more powerful kind of light");
		spell_type_set_mana(spell, 1, 100);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_VARDA,
				       varda_light_of_valinor_info,
				       varda_light_of_valinor_spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_CALL_ALMAREN, "VARDA_CALL_ALMAREN", "Call of Almaren");
		spell_type_describe(spell, "Banishes evil beings");
		spell_type_describe(spell, "At level 20 it dispels evil beings");
		spell_type_set_mana(spell, 5, 150);
		spell_type_set_difficulty(spell, 10, 20);
		spell_type_init_priest(spell,
				       SCHOOL_VARDA,
				       varda_call_of_almaren_info,
				       varda_call_of_almaren_spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_EVENSTAR, "VARDA_EVENSTAR", "Evenstar");
		spell_type_describe(spell, "Maps and lights the whole level.");
		spell_type_describe(spell, "At level 40 it maps and lights the whole level,");
		spell_type_describe(spell, "in addition to letting you know yourself better");
		spell_type_describe(spell, "and identifying your whole pack.");
		spell_type_set_mana(spell, 20, 200);
		spell_type_set_difficulty(spell, 20, 20);
		spell_type_init_priest(spell,
				       SCHOOL_VARDA,
				       varda_evenstar_info,
				       varda_evenstar_spell);
	}

	{
		spell_type *spell = spell_new(&VARDA_STARKINDLER, "VARDA_STARKINDLER", "Star Kindler");
		spell_type_describe(spell, "Does multiple bursts of light damage.");
		spell_type_describe(spell, "The damage increases with level.");
		spell_type_set_mana(spell, 50, 250);
		spell_type_set_difficulty(spell, 30, 20);
		spell_type_init_priest(spell,
				       SCHOOL_VARDA,
				       varda_star_kindler_info,
				       varda_star_kindler_spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_BELEGAER, "ULMO_BELEGAER", "Song of Belegaer");
		spell_type_describe(spell, "Channels the power of the Great Sea into your fingertips.");
		spell_type_describe(spell, "Sometimes it can blast through its first target.");
		spell_type_set_mana(spell, 1, 100);
		spell_type_set_difficulty(spell, 1, 25);
		spell_type_init_priest(spell,
				       SCHOOL_ULMO,
				       ulmo_song_of_belegaer_info,
				       ulmo_song_of_belegaer_spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_DRAUGHT_ULMONAN, "ULMO_DRAUGHT_ULMONAN", "Draught of Ulmonan");
		spell_type_describe(spell, "Fills you with a draught with powerful curing effects,");
		spell_type_describe(spell, "prepared by Ulmo himself.");
		spell_type_describe(spell, "Level 1: blindness, poison, cuts and stunning");
		spell_type_describe(spell, "Level 10: drained STR, DEX and CON");
		spell_type_describe(spell, "Level 20: parasites and mimicry");
		spell_type_set_mana(spell, 25, 200);
		spell_type_set_difficulty(spell, 15, 50);
		spell_type_init_priest(spell,
				       SCHOOL_ULMO,
				       ulmo_draught_of_ulmonan_info,
				       ulmo_draught_of_ulmonan_spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_CALL_ULUMURI, "ULMO_CALL_ULUMURI", "Call of the Ulumuri");
		spell_type_describe(spell, "Summons a leveled water spirit or elemental");
		spell_type_describe(spell, "to fight for you");
		spell_type_set_mana(spell, 50, 300);
		spell_type_set_difficulty(spell, 20, 75);
		spell_type_init_priest(spell,
				       SCHOOL_ULMO,
				       ulmo_call_of_the_ulumuri_info,
				       ulmo_call_of_the_ulumuri_spell);
	}

	{
		spell_type *spell = spell_new(&ULMO_WRATH, "ULMO_WRATH", "Wrath of Ulmo");
		spell_type_describe(spell, "Conjures up a sea storm.");
		spell_type_describe(spell, "At level 30 it turns into a more forceful storm.");
		spell_type_set_mana(spell, 100, 400);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_priest(spell,
				       SCHOOL_ULMO,
				       ulmo_wrath_of_ulmo_info,
				       ulmo_wrath_of_ulmo_spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_TEARS_LUTHIEN, "MANDOS_TEARS_LUTHIEN", "Tears of Luthien");
		spell_type_describe(spell, "Calls upon the spirit of Luthien to ask Mandos for healing and succour.");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 5, 25);
		spell_type_init_priest(spell,
				       SCHOOL_MANDOS,
				       mandos_tears_of_luthien_info,
				       mandos_tears_of_luthien_spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_SPIRIT_FEANTURI, "MANDOS_SPIRIT_FEANTURI", "Feanturi");
		spell_type_describe(spell, "Channels the power of Mandos to cure fear and confusion.");
		spell_type_describe(spell, "At level 20 it restores lost INT and WIS");
		spell_type_describe(spell, "At level 30 it cures hallucinations and restores a percentage of lost sanity");
		spell_type_set_mana(spell, 40, 200);
		spell_type_set_difficulty(spell, 10, 50);
		spell_type_init_priest(spell,
				       SCHOOL_MANDOS,
				       mandos_spirit_of_the_feanturi_info,
				       mandos_spirit_of_the_feanturi_spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_TALE_DOOM, "MANDOS_TALE_DOOM", "Tale of Doom");
		spell_type_describe(spell, "Allows you to predict the future for a short time.");
		spell_type_set_mana(spell, 60, 300);
		spell_type_set_difficulty(spell, 25, 75);
		spell_type_init_priest(spell,
				       SCHOOL_MANDOS,
				       mandos_tale_of_doom_info,
				       mandos_tale_of_doom_spell);
	}

	{
		spell_type *spell = spell_new(&MANDOS_CALL_HALLS, "MANDOS_CALL_HALLS", "Call to the Halls");
		spell_type_describe(spell, "Summons a leveled spirit from the Halls of Mandos");
		spell_type_describe(spell, "to fight for you.");
		spell_type_set_mana(spell, 80, 400);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_priest(spell,
				       SCHOOL_MANDOS,
				       mandos_call_to_the_halls_info,
				       mandos_call_to_the_halls_spell);
	}

	{
		spell_type *spell = spell_new(&DEVICE_THUNDERLORDS, "DEVICE_THUNDERLORDS", "Artifact Thunderlords");
		spell_type_describe(spell, "An Eagle of Manwe will appear to transport you quickly to the town.");
		spell_type_set_mana(spell, 1, 1);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_device(spell,
				       device_thunderlords_info,
				       device_thunderlords);

		spell_type_set_device_charges(spell, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_RADAGAST, "DEVICE_RADAGAST", "Artifact Radagast");
		spell_type_set_activation_timeout(spell, "15000");
		spell_type_describe(spell, "purity and health");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_device(spell,
				       device_radagast_info,
				       device_radagast);
	}

	{
		spell_type *spell = spell_new(&DEVICE_VALAROMA, "DEVICE_VALAROMA", "Artifact Valaroma");
		spell_type_set_activation_timeout(spell, "250");
		spell_type_describe(spell, "banish evil (level x5)");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 25);
		spell_type_init_device(spell,
				       device_valaroma_info,
				       device_valaroma);
	}
}

void school_spells_init()
{
	/* Zero out spell array */
	{
		int i = 0;
		for (i = 0; i < SCHOOL_SPELLS_MAX; i++)
		{
			school_spells[i] = NULL;
		}
	}

	/* Spells */
	{
		spell_type *spell = spell_new(&GLOBELIGHT, "GLOBELIGHT", "Globe of Light");
		spell_type_describe(spell, "Creates a globe of pure light");
		spell_type_describe(spell, "At level 3 it starts damaging monsters");
		spell_type_describe(spell, "At level 15 it starts creating a more powerful kind of light");
		spell_type_set_mana(spell, 2, 15);
		spell_type_set_inertia(spell, 1, 40);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_FIRE,
				     fire_globe_of_light_info,
				     fire_globe_of_light);

		spell_type_set_device_charges(spell, "10+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 7;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 45);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREFLASH, "FIREFLASH", "Fireflash");
		spell_type_describe(spell, "Conjures a ball of fire to burn your foes to ashes");
		spell_type_describe(spell, "At level 20 it turns into a ball of holy fire");
		spell_type_set_mana(spell, 5, 70);
		spell_type_set_difficulty(spell, 10, 35);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_FIRE,
				     fire_fireflash_info,
				     fire_fireflash);

		spell_type_set_device_charges(spell, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 15, 35);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIERYAURA, "FIERYAURA", "Fiery Shield");
		spell_type_describe(spell, "Creates a shield of fierce flames around you");
		spell_type_describe(spell, "At level 8 it turns into a greater kind of flame that can not be resisted");
		spell_type_set_mana(spell, 20, 60);
		spell_type_set_inertia(spell, 2, 15);
		spell_type_set_difficulty(spell, 20, 50);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_FIRE,
				     fire_fiery_shield_info,
				     fire_fiery_shield);

		spell_type_set_device_charges(spell, "3+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 5, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREWALL, "FIREWALL", "Firewall");
		spell_type_describe(spell, "Creates a fiery wall to incinerate monsters stupid enough to attack you");
		spell_type_describe(spell, "At level 6 it turns into a wall of hell fire");
		spell_type_set_mana(spell, 25, 100);
		spell_type_set_difficulty(spell, 15, 40);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_FIRE,
				     fire_firewall_info,
				     fire_firewall);

		spell_type_set_device_charges(spell, "4+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 55;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 5, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&FIREGOLEM, "FIREGOLEM", "Fire Golem");
		spell_type_describe(spell, "Creates a fiery golem and controls it");
		spell_type_describe(spell, "During the control the available keylist is:");
		spell_type_describe(spell, "Movement keys: movement of the golem(depending on its speed");
		spell_type_describe(spell, "               it can move more than one square)");
		spell_type_describe(spell, ", : pickup all items on the floor");
		spell_type_describe(spell, "d : drop all carried items");
		spell_type_describe(spell, "i : list all carried items");
		spell_type_describe(spell, "m : end the possession/use golem powers");
		spell_type_describe(spell, "Most of the other keys are disabled, you cannot interact with your");
		spell_type_describe(spell, "real body while controlling the golem");
		spell_type_describe(spell, "But to cast the spell you will need a lantern or a wooden torch to");
		spell_type_describe(spell, "Create the golem from");
		spell_type_add_school(spell, SCHOOL_MIND);
		spell_type_set_mana(spell, 16, 70);
		spell_type_set_difficulty(spell, 7, 40);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_FIRE,
				     fire_golem_info,
				     fire_golem);
	}

	{
		spell_type *spell = spell_new(&MANATHRUST, "MANATHRUST", "Manathrust");
		spell_type_describe(spell, "Conjures up mana into a powerful bolt");
		spell_type_describe(spell, "The damage is irresistible and will increase with level");
		spell_type_set_mana(spell, 1, 25);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MANA,
				     mana_manathrust_info,
				     mana_manathrust);

		spell_type_set_device_charges(spell, "7+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 5;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 15, 33);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DELCURSES, "DELCURSES", "Remove Curses");
		spell_type_describe(spell, "Remove curses of worn objects");
		spell_type_describe(spell, "At level 20 switches to *remove curses*");
		spell_type_set_mana(spell, 20, 40);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 10, 30);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MANA,
				     mana_remove_curses_info,
				     mana_remove_curses);

		spell_type_set_device_charges(spell, "3+d8");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 70;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RESISTS, "RESISTS", "Elemental Shield");
		spell_type_describe(spell, "Provide resistances to the four basic elements");
		spell_type_set_mana(spell, 17, 20);
		spell_type_set_inertia(spell, 2, 25);
		spell_type_set_difficulty(spell, 20, 40);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MANA,
				     mana_elemental_shield_info,
				     mana_elemental_shield);
	}

	{
		spell_type *spell = spell_new(&MANASHIELD, "MANASHIELD", "Disruption Shield");
		spell_type_describe(spell, "Uses mana instead of hp to take damage");
		spell_type_describe(spell, "At level 5 switches to Globe of Invulnerability.");
		spell_type_describe(spell, "The spell breaks as soon as a melee, shooting, throwing or magical");
		spell_type_describe(spell, "skill action is attempted, and lasts only a short time.");
		spell_type_set_mana(spell, 50, 50);
		spell_type_set_inertia(spell, 9, 10);
		spell_type_set_difficulty(spell, 45, 90);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MANA,
				     mana_disruption_shield_info,
				     mana_disruption_shield);
	}

	{
		spell_type *spell = spell_new(&TIDALWAVE, "TIDALWAVE", "Tidal Wave");
		spell_type_describe(spell, "Summons a monstrous tidal wave that will expand and crush the");
		spell_type_describe(spell, "monsters under its mighty waves.");
		spell_type_set_mana(spell, 16, 40);
		spell_type_set_inertia(spell, 4, 100);
		spell_type_set_difficulty(spell, 16, 65);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_WATER,
				     water_tidal_wave_info,
				     water_tidal_wave);

		spell_type_set_device_charges(spell, "6+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 54;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ICESTORM, "ICESTORM", "Ice Storm");
		spell_type_describe(spell, "Engulfs you in a storm of roaring cold that strikes your foes.");
		spell_type_describe(spell, "At level 10 it turns into shards of ice.");
		spell_type_set_mana(spell, 30, 60);
		spell_type_set_inertia(spell, 3, 40);
		spell_type_set_difficulty(spell, 22, 80);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_WATER,
				     water_ice_storm_info,
				     water_ice_storm);

		spell_type_set_device_charges(spell, "3+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 65;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 25, 45);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ENTPOTION, "ENTPOTION", "Ent's Potion");
		spell_type_describe(spell, "Fills up your stomach.");
		spell_type_describe(spell, "At level 5 it boldens your heart.");
		spell_type_describe(spell, "At level 12 it makes you heroic.");
		spell_type_set_mana(spell, 7, 15);
		spell_type_set_inertia(spell, 1, 30);
		spell_type_set_difficulty(spell, 6, 35);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_WATER,
				     water_ent_potion_info,
				     water_ent_potion);
	}

	{
		spell_type *spell = spell_new(&VAPOR, "VAPOR", "Vapor");
		spell_type_describe(spell, "Fills the air with toxic moisture to eradicate annoying critters.");
		spell_type_set_mana(spell, 2, 12);
		spell_type_set_inertia(spell, 1, 30);
		spell_type_set_difficulty(spell, 2, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_WATER,
				     water_vapor_info,
				     water_vapor);
	}

	{
		spell_type *spell = spell_new(&GEYSER, "GEYSER", "Geyser");
		spell_type_describe(spell, "Shoots a geyser of water from your fingertips.");
		spell_type_describe(spell, "Sometimes it can blast through its first target.");
		spell_type_set_mana(spell, 1, 35);
		spell_type_set_difficulty(spell, 1, 5);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_WATER,
				     water_geyser_info,
				     water_geyser);
	}

	{
		spell_type *spell = spell_new(&NOXIOUSCLOUD, "NOXIOUSCLOUD", "Noxious Cloud");
		spell_type_describe(spell, "Creates a cloud of poison");
		spell_type_describe(spell, "The cloud will persist for some turns, damaging all monsters passing by");
		spell_type_describe(spell, "At spell level 30 it turns into a thick gas attacking all living beings");
		spell_type_set_mana(spell, 3, 30);
		spell_type_set_difficulty(spell, 3, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_noxious_cloud_info,
				     air_noxious_cloud);

		spell_type_set_device_charges(spell, "5+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 15;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 25, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&AIRWINGS, "AIRWINGS", "Wings of Winds");
		spell_type_describe(spell, "Grants the power of levitation");
		spell_type_describe(spell, "At level 16 it grants the power of controlled flight");
		spell_type_add_school(spell, SCHOOL_CONVEYANCE);
		spell_type_set_mana(spell, 30, 40);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 22, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_wings_of_winds_info,
				     air_wings_of_winds);

		spell_type_set_device_charges(spell, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 27;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&INVISIBILITY, "INVISIBILITY", "Invisibility");
		spell_type_describe(spell, "Grants invisibility");
		spell_type_set_mana(spell, 10, 20);
		spell_type_set_inertia(spell, 1, 30);
		spell_type_set_difficulty(spell, 16, 50);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_invisibility_info,
				     air_invisibility);
	}

	{
		spell_type *spell = spell_new(&POISONBLOOD, "POISONBLOOD", "Poison Blood");
		spell_type_describe(spell, "Grants resist poison");
		spell_type_describe(spell, "At level 15 it provides poison branding to wielded weapon");
		spell_type_set_mana(spell, 10, 20);
		spell_type_set_inertia(spell, 1, 35);
		spell_type_set_difficulty(spell, 12, 30);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_poison_blood_info,
				     air_poison_blood);

		spell_type_set_device_charges(spell, "10+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 25);
			range_init(&device_allocation->max_level, 35, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&THUNDERSTORM, "THUNDERSTORM", "Thunderstorm");
		spell_type_describe(spell, "Charges up the air around you with electricity");
		spell_type_describe(spell, "Each turn it will throw a thunder bolt at a random monster in sight");
		spell_type_describe(spell, "The thunder does 3 types of damage, one third of lightning");
		spell_type_describe(spell, "one third of sound and one third of light");
		spell_type_add_school(spell, SCHOOL_NATURE);
		spell_type_set_mana(spell, 40, 60);
		spell_type_set_inertia(spell, 2, 15);
		spell_type_set_difficulty(spell, 25, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_thunderstorm_info,
				     air_thunderstorm);

		spell_type_set_device_charges(spell, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 25, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STERILIZE, "STERILIZE", "Sterilize");
		spell_type_describe(spell, "Prevents explosive breeding for a while.");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 20, 50);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_AIR,
				     air_sterilize_info,
				     air_sterilize);

		spell_type_set_device_charges(spell, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 20;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STONESKIN, "STONESKIN", "Stone Skin");
		spell_type_describe(spell, "Creates a shield of earth around you to protect you");
		spell_type_describe(spell, "At level 25 it starts dealing damage to attackers");
		spell_type_set_mana(spell, 1, 50);
		spell_type_set_inertia(spell, 2, 50);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_EARTH,
				     earth_stone_skin_info,
				     earth_stone_skin);
	}

	{
		spell_type *spell = spell_new(&DIG, "DIG", "Dig");
		spell_type_describe(spell, "Digs a hole in a wall much faster than any shovels");
		spell_type_set_mana(spell, 14, 14);
		spell_type_set_difficulty(spell, 12, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_EARTH,
				     earth_dig_info,
				     earth_dig);

		spell_type_set_device_charges(spell, "15+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 25;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STONEPRISON, "STONEPRISON", "Stone Prison");
		spell_type_describe(spell, "Creates a prison of walls around you");
		spell_type_describe(spell, "At level 10 it allows you to target a monster");
		spell_type_set_mana(spell, 30, 50);
		spell_type_set_difficulty(spell, 25, 65);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_EARTH,
				     earth_stone_prison_info,
				     earth_stone_prison);

		spell_type_set_device_charges(spell, "5+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 57;
			range_init(&device_allocation->base_level, 1, 3);
			range_init(&device_allocation->max_level, 5, 20);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STRIKE, "STRIKE", "Strike");
		spell_type_describe(spell, "Creates a micro-ball of force that will push monsters backwards");
		spell_type_describe(spell, "If the monster is caught near a wall, it'll be crushed against it");
		spell_type_describe(spell, "At level 12 it turns into a ball of radius 1");
		spell_type_set_mana(spell, 30, 50);
		spell_type_set_difficulty(spell, 30, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_EARTH,
				     earth_strike_info,
				     earth_strike);

		spell_type_set_device_charges(spell, "2+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 635;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SHAKE, "SHAKE", "Shake");
		spell_type_describe(spell, "Creates a localised earthquake");
		spell_type_describe(spell, "At level 10 it can be targeted at any location");
		spell_type_set_mana(spell, 25, 30);
		spell_type_set_inertia(spell, 2, 50);
		spell_type_set_difficulty(spell, 27, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_EARTH,
				     earth_shake_info,
				     earth_shake);

		spell_type_set_device_charges(spell, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 3);
			range_init(&device_allocation->max_level, 9, 20);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&BLINK, "BLINK", "Phase Door");
		spell_type_describe(spell, "Teleports you on a small scale range");
		spell_type_describe(spell, "At level 30 it creates void jumpgates");
		spell_type_set_mana(spell, 1, 3);
		spell_type_set_inertia(spell, 1, 5);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_blink_info,
				     convey_blink);
	}

	{
		spell_type *spell = spell_new(&DISARM, "DISARM", "Disarm");
		spell_type_describe(spell, "Destroys doors and traps");
		spell_type_describe(spell, "At level 10 it destroys doors and traps, then reveals and unlocks any secret");
		spell_type_describe(spell, "doors");
		spell_type_set_mana(spell, 2, 4);
		spell_type_set_difficulty(spell, 3, 15);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_disarm_info,
				     convey_disarm);

		spell_type_set_device_charges(spell, "10+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 4;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 10, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TELEPORT, "TELEPORT", "Teleportation");
		spell_type_describe(spell, "Teleports you around the level. The casting time decreases with level");
		spell_type_set_mana(spell, 8, 14);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 10, 30);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_teleport_info,
				     convey_teleport);

		spell_type_set_device_charges(spell, "7+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TELEAWAY, "TELEAWAY", "Teleport Away");
		spell_type_describe(spell, "Teleports a line of monsters away");
		spell_type_describe(spell, "At level 10 it turns into a ball");
		spell_type_describe(spell, "At level 20 it teleports all monsters in sight");
		spell_type_set_mana(spell, 15, 40);
		spell_type_set_difficulty(spell, 23, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_teleport_away_info,
				     convey_teleport_away);

		spell_type_set_device_charges(spell, "3+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 20);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECALL, "RECALL", "Recall");
		spell_type_describe(spell, "Cast on yourself it will recall you to the surface/dungeon.");
		spell_type_describe(spell, "Cast at a monster you will swap positions with the monster.");
		spell_type_describe(spell, "Cast at an object it will fetch the object to you.");
		spell_type_set_mana(spell, 25, 25);
		spell_type_set_difficulty(spell, 30, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_recall_info,
				     convey_recall);
	}

	{
		spell_type *spell = spell_new(&PROBABILITY_TRAVEL, "PROBABILITY_TRAVEL", "Probability Travel");
		spell_type_describe(spell, "Renders you immaterial, when you hit a wall you travel through it and");
		spell_type_describe(spell, "instantly appear on the other side of it. You can also float up and down");
		spell_type_describe(spell, "at will");
		spell_type_set_mana(spell, 30, 50);
		spell_type_set_inertia(spell, 6, 40);
		spell_type_set_difficulty(spell, 35, 90);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_CONVEYANCE,
				     convey_probability_travel_info,
				     convey_probability_travel);

		spell_type_set_device_charges(spell, "1+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 97;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 8, 25);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&GROWTREE, "GROWTREE", "Grow Trees");
		spell_type_describe(spell, "Makes trees grow extremely quickly around you");
		spell_type_add_school(spell, SCHOOL_TEMPORAL);
		spell_type_set_mana(spell, 6, 30);
		spell_type_set_inertia(spell, 5, 50);
		spell_type_set_difficulty(spell, 6, 35);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_grow_trees_info,
				     nature_grow_trees);
	}

	{
		spell_type *spell = spell_new(&HEALING, "HEALING", "Healing");
		spell_type_describe(spell, "Heals a percent of hitpoints");
		spell_type_set_mana(spell, 15, 50);
		spell_type_set_difficulty(spell, 10, 45);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_healing_info,
				     nature_healing);

		spell_type_set_device_charges(spell, "2+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 90;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECOVERY, "RECOVERY", "Recovery");
		spell_type_describe(spell, "Reduces the length of time that you are poisoned");
		spell_type_describe(spell, "At level 5 it cures poison and cuts");
		spell_type_describe(spell, "At level 10 it restores drained stats");
		spell_type_describe(spell, "At level 15 it restores lost experience");
		spell_type_set_mana(spell, 10, 25);
		spell_type_set_inertia(spell, 2, 100);
		spell_type_set_difficulty(spell, 15, 60);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_recovery_info,
				     nature_recovery);

		spell_type_set_device_charges(spell, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 50;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 30);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&REGENERATION, "REGENERATION", "Regeneration");
		spell_type_describe(spell, "Increases your body's regeneration rate");
		spell_type_set_mana(spell, 30, 55);
		spell_type_set_inertia(spell, 4, 40);
		spell_type_set_difficulty(spell, 20, 70);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_regeneration_info,
				     nature_regeneration);
	}

	{
		spell_type *spell = spell_new(&SUMMONANNIMAL, "SUMMONANNIMAL", "Summon Animal");
		spell_type_describe(spell, "Summons a leveled animal to your aid");
		spell_type_set_mana(spell, 25, 50);
		spell_type_set_difficulty(spell, 25, 90);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_NATURE,
				     nature_summon_animal_info,
				     nature_summon_animal);

		spell_type_set_device_charges(spell, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&STARIDENTIFY, "STARIDENTIFY", "Greater Identify");
		spell_type_describe(spell, "Asks for an object and fully identify it, providing the full list of powers");
		spell_type_describe(spell, "Cast at yourself it will reveal your powers");
		spell_type_set_mana(spell, 30, 30);
		spell_type_set_difficulty(spell, 35, 80);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_greater_identify_info,
				     divination_greater_identify);
	}

	{
		spell_type *spell = spell_new(&IDENTIFY, "IDENTIFY", "Identify");
		spell_type_describe(spell, "Asks for an object and identifies it");
		spell_type_describe(spell, "At level 17 it identifies all objects in the inventory");
		spell_type_describe(spell, "At level 27 it identifies all objects in the inventory and in a");
		spell_type_describe(spell, "radius on the floor, as well as probing monsters in that radius");
		spell_type_set_mana(spell, 10, 50);
		spell_type_set_difficulty(spell, 8, 40);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_identify_info,
				     divination_identify);

		spell_type_set_device_charges(spell, "7+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 15, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&VISION, "VISION", "Vision");
		spell_type_describe(spell, "Detects the layout of the surrounding area");
		spell_type_describe(spell, "At level 25 it maps and lights the whole level");
		spell_type_set_mana(spell, 7, 55);
		spell_type_set_inertia(spell, 2, 200);
		spell_type_set_difficulty(spell, 15, 45);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_vision_info,
				     divination_vision);

		spell_type_set_device_charges(spell, "4+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 60;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 10, 30);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SENSEHIDDEN, "SENSEHIDDEN", "Sense Hidden");
		spell_type_describe(spell, "Detects the traps in a certain radius around you");
		spell_type_describe(spell, "At level 15 it allows you to sense invisible for a while");
		spell_type_set_mana(spell, 2, 10);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 5, 25);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_sense_hidden_info,
				     divination_sense_hidden);

		spell_type_set_device_charges(spell, "1+d15");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 20;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&REVEALWAYS, "REVEALWAYS", "Reveal Ways");
		spell_type_describe(spell, "Detects the doors/stairs/ways in a certain radius around you");
		spell_type_set_mana(spell, 3, 15);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 9, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_reveal_ways_info,
				     divination_reveal_ways);

		spell_type_set_device_charges(spell, "6+d6");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 25, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SENSEMONSTERS, "SENSEMONSTERS", "Sense Monsters");
		spell_type_describe(spell, "Detects all monsters near you");
		spell_type_describe(spell, "At level 30 it allows you to sense monster minds for a while");
		spell_type_set_mana(spell, 1, 20);
		spell_type_set_inertia(spell, 1, 10);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_DIVINATION,
				     divination_sense_monsters_info,
				     divination_sense_monsters);

		spell_type_set_device_charges(spell, "5+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 37;
			range_init(&device_allocation->base_level, 1, 10);
			range_init(&device_allocation->max_level, 15, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&MAGELOCK, "MAGELOCK", "Magelock");
		spell_type_describe(spell, "Magically locks a door");
		spell_type_describe(spell, "At level 30 it creates a glyph of warding");
		spell_type_describe(spell, "At level 40 the glyph can be placed anywhere in the field of vision");
		spell_type_set_mana(spell, 1, 35);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_TEMPORAL,
				     tempo_magelock_info,
				     tempo_magelock);

		spell_type_set_device_charges(spell, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 30;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 15, 45);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&SLOWMONSTER, "SLOWMONSTER", "Slow Monster");
		spell_type_describe(spell, "Magically slows down the passing of time around a monster");
		spell_type_describe(spell, "At level 20 it affects a zone");
		spell_type_set_mana(spell, 10, 15);
		spell_type_set_difficulty(spell, 10, 35);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_TEMPORAL,
				     tempo_slow_monster_info,
				     tempo_slow_monster);

		spell_type_set_device_charges(spell, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 23;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ESSENCESPEED, "ESSENCESPEED", "Essence of Speed");
		spell_type_describe(spell, "Magically decreases the passing of time around you, making you move faster with");
		spell_type_describe(spell, "respect to the rest of the universe.");
		spell_type_set_mana(spell, 20, 40);
		spell_type_set_inertia(spell, 5, 20);
		spell_type_set_difficulty(spell, 15, 50);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_TEMPORAL,
				     tempo_essence_of_speed_info,
				     tempo_essence_of_speed);

		spell_type_set_device_charges(spell, "3+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 80;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 10, 39);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&BANISHMENT, "BANISHMENT", "Banishment");
		spell_type_describe(spell, "Disrupts the space/time continuum in your area and teleports all monsters away.");
		spell_type_describe(spell, "At level 15 it may also lock them in a time bubble for a while.");
		spell_type_add_school(spell, SCHOOL_CONVEYANCE);
		spell_type_set_mana(spell, 30, 40);
		spell_type_set_inertia(spell, 5, 50);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_TEMPORAL,
				     tempo_banishment_info,
				     tempo_banishment);

		spell_type_set_device_charges(spell, "1+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 98;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 10, 36);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&RECHARGE, "RECHARGE", "Recharge");
		spell_type_describe(spell, "Taps on the ambient mana to recharge an object's power (charges or mana)");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 5, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_META,
				     meta_recharge_info,
				     meta_recharge);
	}

	{
		spell_type *spell = spell_new(&SPELLBINDER, "SPELLBINDER", "Spellbinder");
		spell_type_describe(spell, "Stores spells in a trigger.");
		spell_type_describe(spell, "When the condition is met all spells fire off at the same time");
		spell_type_describe(spell, "This spell takes a long time to cast so you are advised to prepare it");
		spell_type_describe(spell, "in a safe area.");
		spell_type_describe(spell, "Also it will use the mana for the Spellbinder and the mana for the");
		spell_type_describe(spell, "selected spells");
		spell_type_set_mana(spell, 100, 300);
		spell_type_set_difficulty(spell, 20, 85);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_META,
				     meta_spellbinder_info,
				     meta_spellbinder);
	}

	{
		spell_type *spell = spell_new(&DISPERSEMAGIC, "DISPERSEMAGIC", "Disperse Magic");
		spell_type_describe(spell, "Dispels a lot of magic that can affect you, be it good or bad");
		spell_type_describe(spell, "Level 1: blindness and light");
		spell_type_describe(spell, "Level 5: confusion and hallucination");
		spell_type_describe(spell, "Level 10: speed (both bad or good) and light speed");
		spell_type_describe(spell, "Level 15: stunning, meditation, cuts");
		spell_type_describe(spell, "Level 20: hero, super hero, bless, shields, afraid, parasites, mimicry");
		spell_type_set_mana(spell, 30, 60);
		spell_type_set_inertia(spell, 1, 5);
		spell_type_set_difficulty(spell, 15, 40);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_set_castable_while_confused(spell, TRUE);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_META,
				     meta_disperse_magic_info,
				     meta_disperse_magic);

		spell_type_set_device_charges(spell, "5+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 25;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 5, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&TRACKER, "TRACKER", "Tracker");
		spell_type_describe(spell, "Tracks down the last teleportation that happened on the level and teleports");
		spell_type_describe(spell, "you to it");
		spell_type_add_school(spell, SCHOOL_CONVEYANCE);
		spell_type_set_mana(spell, 50, 50);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_META,
				     meta_tracker_info,
				     meta_tracker);
	}

	{
		spell_type *spell = spell_new(&INERTIA_CONTROL, "INERTIA_CONTROL", "Inertia Control");
		spell_type_describe(spell, "Changes the energy flow of a spell to be continuously recasted");
		spell_type_describe(spell, "at a given interval. The inertia controlled spell reduces your");
		spell_type_describe(spell, "maximum mana by four times its cost.");
		spell_type_set_mana(spell, 300, 700);
		spell_type_set_difficulty(spell, 37, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_META,
				     meta_inertia_control_info,
				     meta_inertia_control);
	}

	{
		spell_type *spell = spell_new(&CHARM, "CHARM", "Charm");
		spell_type_describe(spell, "Tries to manipulate the mind of a monster to make it friendly");
		spell_type_describe(spell, "At level 15 it turns into a ball");
		spell_type_describe(spell, "At level 35 it affects all monsters in sight");
		spell_type_set_mana(spell, 1, 20);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MIND,
				     mind_charm_info,
				     mind_charm);

		spell_type_set_device_charges(spell, "7+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 35;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&CONFUSE, "CONFUSE", "Confuse");
		spell_type_describe(spell, "Tries to manipulate the mind of a monster to confuse it");
		spell_type_describe(spell, "At level 15 it turns into a ball");
		spell_type_describe(spell, "At level 35 it affects all monsters in sight");
		spell_type_set_mana(spell, 5, 30);
		spell_type_set_difficulty(spell, 5, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MIND,
				     mind_confuse_info,
				     mind_confuse);

		spell_type_set_device_charges(spell, "3+d4");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 45;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&ARMOROFFEAR, "ARMOROFFEAR", "Armor of Fear");
		spell_type_describe(spell, "Creates a shield of pure fear around you. Any monster attempting to hit you");
		spell_type_describe(spell, "must save or flee");
		spell_type_set_mana(spell, 10, 50);
		spell_type_set_inertia(spell, 2, 20);
		spell_type_set_difficulty(spell, 10, 35);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MIND,
				     mind_armor_of_fear_info,
				     mind_armor_of_fear);
	}

	{
		spell_type *spell = spell_new(&STUN, "STUN", "Stun");
		spell_type_describe(spell, "Tries to manipulate the mind of a monster to stun it");
		spell_type_describe(spell, "At level 20 it turns into a ball");
		spell_type_set_mana(spell, 10, 90);
		spell_type_set_difficulty(spell, 15, 45);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_MIND,
				     mind_stun_info,
				     mind_stun);
	}

	{
		spell_type *spell = spell_new(&DRAIN, "DRAIN", "Drain");
		spell_type_describe(spell, "Drains the mana contained in wands, staves and rods to increase yours");
		spell_type_add_school(spell, SCHOOL_MANA);
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_UDUN,
				     udun_drain_info,
				     udun_drain);
	}

	{
		spell_type *spell = spell_new(&GENOCIDE, "GENOCIDE", "Genocide");
		spell_type_describe(spell, "Genocides all monsters of a race on the level");
		spell_type_describe(spell, "At level 10 it can genocide all monsters near you");
		spell_type_add_school(spell, SCHOOL_NATURE);
		spell_type_set_mana(spell, 50, 50);
		spell_type_set_difficulty(spell, 25, 90);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_UDUN,
				     udun_genocide_info,
				     udun_genocide);

		spell_type_set_device_charges(spell, "2+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 85;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 5, 15);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&WRAITHFORM, "WRAITHFORM", "Wraithform");
		spell_type_describe(spell, "Turns you into an immaterial being");
		spell_type_add_school(spell, SCHOOL_CONVEYANCE);
		spell_type_set_mana(spell, 20, 40);
		spell_type_set_inertia(spell, 4, 30);
		spell_type_set_difficulty(spell, 30, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_UDUN,
				     udun_wraithform_info,
				     udun_wraithform);
	}

	{
		spell_type *spell = spell_new(&FLAMEOFUDUN, "FLAMEOFUDUN", "Flame of Udun");
		spell_type_describe(spell, "Turns you into a powerful Balrog");
		spell_type_add_school(spell, SCHOOL_FIRE);
		spell_type_set_mana(spell, 70, 100);
		spell_type_set_inertia(spell, 7, 15);
		spell_type_set_difficulty(spell, 35, 95);
		spell_type_init_mage(spell,
				     RANDOM,
				     SCHOOL_UDUN,
				     udun_flame_of_udun_info,
				     udun_flame_of_udun);
	}

	{
		spell_type *spell = spell_new(&CALL_THE_ELEMENTS, "CALL_THE_ELEMENTS", "Call the Elements");
		spell_type_describe(spell, "Randomly creates various elements around you");
		spell_type_describe(spell, "Each type of element chance is controlled by your level");
		spell_type_describe(spell, "in the corresponding skill");
		spell_type_describe(spell, "At level 17 it can be targeted");
		spell_type_set_mana(spell, 2, 20);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_mage(spell,
				     NO_RANDOM,
				     SCHOOL_GEOMANCY,
				     geomancy_call_the_elements_info,
				     geomancy_call_the_elements);
	}

	{
		spell_type *spell = spell_new(&CHANNEL_ELEMENTS, "CHANNEL_ELEMENTS", "Channel Elements");
		spell_type_describe(spell, "Draws on the caster's immediate environs to form an attack or other effect.");
		spell_type_describe(spell, "Grass/Flower heals.");
		spell_type_describe(spell, "Water creates water bolt attacks.");
		spell_type_describe(spell, "Ice creates ice bolt attacks.");
		spell_type_describe(spell, "Sand creates a wall of thick, blinding, burning sand around you.");
		spell_type_describe(spell, "Lava creates fire bolt attacks.");
		spell_type_describe(spell, "Deep lava creates fire ball attacks.");
		spell_type_describe(spell, "Chasm creates darkness bolt attacks.");
		spell_type_describe(spell, "At Earth level 18, darkness becomes nether.");
		spell_type_describe(spell, "At Water level 8, water attacks become beams with a striking effect.");
		spell_type_describe(spell, "At Water level 12, ice attacks become balls of ice shards.");
		spell_type_describe(spell, "At Water level 18, water attacks push monsters back.");
		spell_type_describe(spell, "At Fire level 15, fire become hellfire.");
		spell_type_set_mana(spell, 3, 30);
		spell_type_set_difficulty(spell, 3, 20);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_mage(spell,
				     NO_RANDOM,
				     SCHOOL_GEOMANCY,
				     geomancy_channel_elements_info,
				     geomancy_channel_elements);
	}

	{
		spell_type *spell = spell_new(&ELEMENTAL_WAVE, "ELEMENTAL_WAVE", "Elemental Wave");
		spell_type_describe(spell, "Draws on an adjacent special square to project a slow-moving");
		spell_type_describe(spell, "wave of that element in that direction");
		spell_type_describe(spell, "Abyss squares cannot be channeled into a wave.");
		spell_type_set_mana(spell, 15, 50);
		spell_type_set_difficulty(spell, 15, 20);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_mage(spell,
				     NO_RANDOM,
				     SCHOOL_GEOMANCY,
				     geomancy_elemental_wave_info,
				     geomancy_elemental_wave);
	}

	{
		spell_type *spell = spell_new(&VAPORIZE, "VAPORIZE", "Vaporize");
		spell_type_describe(spell, "Draws upon your immediate environs to form a cloud of damaging vapors");
		spell_type_set_mana(spell, 3, 30);
		spell_type_set_difficulty(spell, 4, 15);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_geomancy(
			spell,
			geomancy_vaporize_info,
			geomancy_vaporize,
			geomancy_vaporize_depends);
	}

	{
		spell_type *spell = spell_new(&GEOLYSIS, "GEOLYSIS", "Geolysis");
		spell_type_describe(spell, "Burrows deeply and slightly at random into a wall,");
		spell_type_describe(spell, "leaving behind tailings of various different sorts of walls in the passage.");
		spell_type_set_mana(spell, 15, 40);
		spell_type_set_difficulty(spell, 7, 15);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_geomancy(
			spell,
			geomancy_geolysis_info,
			geomancy_geolysis,
			geomancy_geolysis_depends);
	}

	{
		spell_type *spell = spell_new(&DRIPPING_TREAD, "DRIPPING_TREAD", "Dripping Tread");
		spell_type_describe(spell, "Causes you to leave random elemental forms behind as you walk");
		spell_type_set_mana(spell, 15, 25);
		spell_type_set_difficulty(spell, 10, 15);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_geomancy(
			spell,
			geomancy_dripping_tread_info,
			geomancy_dripping_tread,
			geomancy_dripping_tread_depends);
	}

	{
		spell_type *spell = spell_new(&GROW_BARRIER, "GROW_BARRIER", "Grow Barrier");
		spell_type_describe(spell, "Creates impassable terrain (walls, trees, etc.) around you.");
		spell_type_describe(spell, "At level 20 it can be projected around another area.");
		spell_type_set_mana(spell, 30, 40);
		spell_type_set_difficulty(spell, 12, 15);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_geomancy(
			spell,
			geomancy_grow_barrier_info,
			geomancy_grow_barrier,
			geomancy_grow_barrier_depends);
	}

	{
		spell_type *spell = spell_new(&ELEMENTAL_MINION, "ELEMENTAL_MINION", "Elemental Minion");
		spell_type_describe(spell, "Summons a minion from a nearby element.");
		spell_type_describe(spell, "Walls can summon Earth elmentals, Xorns and Xarens");
		spell_type_describe(spell, "Dark Pits can summon Air elementals, Ancient blue dragons, Great Storm Wyrms");
		spell_type_describe(spell, "and Sky Drakes");
		spell_type_describe(spell, "Sandwalls and lava can summon Fire elementals and Ancient red dragons");
		spell_type_describe(spell, "Icewall, and water can summon Water elementals, Water trolls and Water demons");
		spell_type_set_mana(spell, 40, 80);
		spell_type_set_difficulty(spell, 20, 25);
		spell_type_init_geomancy(
			spell,
			geomancy_elemental_minion_info,
			geomancy_elemental_minion,
			NULL);
	}

	{
		spell_type *spell = spell_new(&ERU_SEE, "ERU_SEE", "See the Music");
		spell_type_describe(spell, "Allows you to 'see' the Great Music from which the world");
		spell_type_describe(spell, "originates, allowing you to see unseen things");
		spell_type_describe(spell, "At level 10 it allows you to see your surroundings");
		spell_type_describe(spell, "At level 20 it allows you to cure blindness");
		spell_type_describe(spell, "At level 30 it allows you to fully see all the level");
		spell_type_set_mana(spell, 1, 50);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_ERU,
				       eru_see_the_music_info,
				       eru_see_the_music);
		spell_type_set_castable_while_blind(spell, TRUE);
	}

	{
		spell_type *spell = spell_new(&ERU_LISTEN, "ERU_LISTEN", "Listen to the Music");
		spell_type_describe(spell, "Allows you to listen to the Great Music from which the world");
		spell_type_describe(spell, "originates, allowing you to understand the meaning of things");
		spell_type_describe(spell, "At level 14 it allows you to identify all your pack");
		spell_type_describe(spell, "At level 30 it allows you to identify all items on the level");
		spell_type_set_mana(spell, 15, 200);
		spell_type_set_difficulty(spell, 7, 25);
		spell_type_init_priest(spell,
				       SCHOOL_ERU,
				       eru_listen_to_the_music_info,
				       eru_listen_to_the_music);
	}

	{
		spell_type *spell = spell_new(&ERU_UNDERSTAND, "ERU_UNDERSTAND", "Know the Music");
		spell_type_describe(spell, "Allows you to understand the Great Music from which the world");
		spell_type_describe(spell, "originates, allowing you to know the full abilities of things");
		spell_type_describe(spell, "At level 10 it allows you to *identify* all your pack");
		spell_type_set_mana(spell, 200, 600);
		spell_type_set_difficulty(spell, 30, 50);
		spell_type_init_priest(spell,
				       SCHOOL_ERU,
				       eru_know_the_music_info,
				       eru_know_the_music);
	}

	{
		spell_type *spell = spell_new(&ERU_PROT, "ERU_PROT", "Lay of Protection");
		spell_type_describe(spell, "Creates a circle of safety around you");
		spell_type_set_mana(spell, 400, 400);
		spell_type_set_difficulty(spell, 35, 80);
		spell_type_init_priest(spell,
				       SCHOOL_ERU,
				       eru_lay_of_protection_info,
				       eru_lay_of_protection);
	}

	{
		spell_type *spell = spell_new(&MANWE_SHIELD, "MANWE_SHIELD", "Wind Shield");
		spell_type_describe(spell, "It surrounds you with a shield of wind that deflects blows from evil monsters");
		spell_type_describe(spell, "At level 10 it increases your armour rating");
		spell_type_describe(spell, "At level 20 it retaliates against monsters that melee you");
		spell_type_set_mana(spell, 100, 500);
		spell_type_set_difficulty(spell, 10, 30);
		spell_type_init_priest(spell,
				       SCHOOL_MANWE,
				       manwe_wind_shield_info,
				       manwe_wind_shield);
	}

	{
		spell_type *spell = spell_new(&MANWE_AVATAR, "MANWE_AVATAR", "Avatar");
		spell_type_describe(spell, "It turns you into a full grown Maia");
		spell_type_set_mana(spell, 1000, 1000);
		spell_type_set_difficulty(spell, 35, 80);
		spell_type_init_priest(spell,
				       SCHOOL_MANWE,
				       manwe_avatar_info,
				       manwe_avatar);
	}

	{
		spell_type *spell = spell_new(&MANWE_BLESS, "MANWE_BLESS", "Manwe's Blessing");
		spell_type_describe(spell, "Manwe's Blessing removes your fears, blesses you and surrounds you with");
		spell_type_describe(spell, "holy light");
		spell_type_describe(spell, "At level 10 it also grants heroism");
		spell_type_describe(spell, "At level 20 it also grants super heroism");
		spell_type_describe(spell, "At level 30 it also grants holy luck and life protection");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_MANWE,
				       manwe_blessing_info,
				       manwe_blessing);
	}

	{
		spell_type *spell = spell_new(&MANWE_CALL, "MANWE_CALL", "Manwe's Call");
		spell_type_describe(spell, "Manwe's Call summons a Great Eagle to help you battle the forces");
		spell_type_describe(spell, "of Morgoth");
		spell_type_set_mana(spell, 200, 500);
		spell_type_set_difficulty(spell, 20, 40);
		spell_type_init_priest(spell,
				       SCHOOL_MANWE,
				       manwe_call_info,
				       manwe_call);
	}

	{
		spell_type *spell = spell_new(&TULKAS_AIM, "TULKAS_AIM", "Divine Aim");
		spell_type_describe(spell, "It makes you more accurate in combat");
		spell_type_describe(spell, "At level 20 all your blows are critical hits");
		spell_type_set_mana(spell, 30, 500);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_TULKAS,
				       tulkas_divine_aim_info,
				       tulkas_divine_aim);
	}

	{
		spell_type *spell = spell_new(&TULKAS_WAVE, "TULKAS_WAVE", "Wave of Power");
		spell_type_describe(spell, "It allows you to project a number of melee blows across a distance");
		spell_type_set_mana(spell, 200, 200);
		spell_type_set_difficulty(spell, 20, 75);
		spell_type_init_priest(spell,
				       SCHOOL_TULKAS,
				       tulkas_wave_of_power_info,
				       tulkas_wave_of_power);
	}

	{
		spell_type *spell = spell_new(&TULKAS_SPIN, "TULKAS_SPIN", "Whirlwind");
		spell_type_describe(spell, "It allows you to spin around and hit all monsters nearby");
		spell_type_set_mana(spell, 100, 100);
		spell_type_set_difficulty(spell, 10, 45);
		spell_type_init_priest(spell,
				       SCHOOL_TULKAS,
				       tulkas_whirlwind_info,
				       tulkas_whirlwind);
	}

	{
		spell_type *spell = spell_new(&MELKOR_CURSE, "MELKOR_CURSE", "Curse");
		spell_type_describe(spell, "It curses a monster, reducing its melee power");
		spell_type_describe(spell, "At level 5 it can be auto-casted (with no piety cost) while fighting");
		spell_type_describe(spell, "At level 15 it also reduces armor");
		spell_type_describe(spell, "At level 25 it also reduces speed");
		spell_type_describe(spell, "At level 35 it also reduces max life (but it is never fatal)");
		spell_type_set_mana(spell, 50, 300);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_priest(spell,
				       SCHOOL_MELKOR,
				       melkor_curse_info,
				       melkor_curse);
	}

	{
		spell_type *spell = spell_new(&MELKOR_CORPSE_EXPLOSION, "MELKOR_CORPSE_EXPLOSION", "Corpse Explosion");
		spell_type_describe(spell, "It makes corpses in an area around you explode for a percent of their");
		spell_type_describe(spell, "hit points as damage");
		spell_type_set_mana(spell, 100, 500);
		spell_type_set_difficulty(spell, 10, 45);
		spell_type_init_priest(spell,
				       SCHOOL_MELKOR,
				       melkor_corpse_explosion_info,
				       melkor_corpse_explosion);
	}

	{
		spell_type *spell = spell_new(&MELKOR_MIND_STEAL, "MELKOR_MIND_STEAL", "Mind Steal");
		spell_type_describe(spell, "It allows your spirit to temporarily leave your own body, which will");
		spell_type_describe(spell, "be vulnerable, to control one of your enemies body.");
		spell_type_set_mana(spell, 1000, 3000);
		spell_type_set_difficulty(spell, 20, 90);
		spell_type_init_priest(spell,
				       SCHOOL_MELKOR,
				       melkor_mind_steal_info,
				       melkor_mind_steal);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_CHARM_ANIMAL, "YAVANNA_CHARM_ANIMAL", "Charm Animal");
		spell_type_describe(spell, "It tries to tame an animal");
		spell_type_set_mana(spell, 10, 100);
		spell_type_set_difficulty(spell, 1, 30);
		spell_type_init_priest(spell,
				       SCHOOL_YAVANNA,
				       yavanna_charm_animal_info,
				       yavanna_charm_animal);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_GROW_GRASS, "YAVANNA_GROW_GRASS", "Grow Grass");
		spell_type_describe(spell, "Create a floor of grass around you. While on grass and praying");
		spell_type_describe(spell, "a worshipper of Yavanna will know a greater regeneration rate");
		spell_type_set_mana(spell, 70, 150);
		spell_type_set_difficulty(spell, 10, 65);
		spell_type_init_priest(spell,
				       SCHOOL_YAVANNA,
				       yavanna_grow_grass_info,
				       yavanna_grow_grass);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_TREE_ROOTS, "YAVANNA_TREE_ROOTS", "Tree Roots");
		spell_type_describe(spell, "Creates roots deep in the floor from your feet, making you more stable and able");
		spell_type_describe(spell, "to make stronger attacks, but prevents any movement (even teleportation).");
		spell_type_describe(spell, "It also makes you recover from stunning almost immediately.");
		spell_type_set_mana(spell, 50, 1000);
		spell_type_set_difficulty(spell, 15, 70);
		spell_type_init_priest(spell,
				       SCHOOL_YAVANNA,
				       yavanna_tree_roots_info,
				       yavanna_tree_roots);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_WATER_BITE, "YAVANNA_WATER_BITE", "Water Bite");
		spell_type_describe(spell, "Imbues your melee weapon with a natural stream of water");
		spell_type_describe(spell, "At level 25, it spreads over a 1 radius zone around your target");
		spell_type_set_mana(spell, 150, 300);
		spell_type_set_difficulty(spell, 20, 90);
		spell_type_init_priest(spell,
				       SCHOOL_YAVANNA,
				       yavanna_water_bite_info,
				       yavanna_water_bite);
	}

	{
		spell_type *spell = spell_new(&YAVANNA_UPROOT, "YAVANNA_UPROOT", "Uproot");
		spell_type_describe(spell, "Awakes a tree to help you battle the forces of Morgoth");
		spell_type_set_mana(spell, 250, 350);
		spell_type_set_difficulty(spell, 35, 95);
		spell_type_init_priest(spell,
				       SCHOOL_YAVANNA,
				       yavanna_uproot_info,
				       yavanna_uproot);
	}

	{
		spell_type *spell = spell_new(&DEMON_BLADE, "DEMON_BLADE", "Demon Blade");
		spell_type_describe(spell, "Imbues your blade with fire to deal more damage");
		spell_type_describe(spell, "At level 30 it deals hellfire damage");
		spell_type_describe(spell, "At level 45 it spreads over a 1 radius zone around your target");
		spell_type_set_mana(spell, 4, 44);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_demonology(spell,
					   demonology_demon_blade_info,
					   demonology_demon_blade);

		spell_type_set_device_charges(spell, "3+d7");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 75;
			range_init(&device_allocation->base_level, 1, 17);
			range_init(&device_allocation->max_level, 20, 40);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEMON_MADNESS, "DEMON_MADNESS", "Demon Madness");
		spell_type_describe(spell, "Fire 2 balls in opposite directions of randomly chaos, confusion or charm");
		spell_type_set_mana(spell, 5, 20);
		spell_type_set_difficulty(spell, 10, 25);
		spell_type_init_demonology(spell,
					   demonology_demon_madness_info,
					   demonology_demon_madness);
	}

	{
		spell_type *spell = spell_new(&DEMON_FIELD, "DEMON_FIELD", "Demon Field");
		spell_type_describe(spell, "Fires a cloud of deadly nexus over a radius of 7");
		spell_type_set_mana(spell, 20, 60);
		spell_type_set_difficulty(spell, 20, 60);
		spell_type_init_demonology(spell,
					   demonology_demon_field_info,
					   demonology_demon_field);
	}

	{
		spell_type *spell = spell_new(&DOOM_SHIELD, "DOOM_SHIELD", "Doom Shield");
		spell_type_describe(spell, "Raises a mirror of pain around you, doing very high damage to your foes");
		spell_type_describe(spell, "that dare hit you, but greatly reduces your armour class");
		spell_type_set_mana(spell, 2, 30);
		spell_type_set_difficulty(spell, 1, 10);
		spell_type_init_demonology(spell,
					   demonology_doom_shield_info,
					   demonology_doom_shield);
	}

	{
		spell_type *spell = spell_new(&UNHOLY_WORD, "UNHOLY_WORD", "Unholy Word");
		spell_type_describe(spell, "Kills a pet to heal you");
		spell_type_describe(spell, "There is a chance that the pet won't die but will turn against you");
		spell_type_describe(spell, "it will decrease with higher level");
		spell_type_set_mana(spell, 15, 45);
		spell_type_set_difficulty(spell, 25, 55);
		spell_type_init_demonology(spell,
					   demonology_unholy_word_info,
					   demonology_unholy_word);
	}

	{
		spell_type *spell = spell_new(&DEMON_CLOAK, "DEMON_CLOAK", "Demon Cloak");
		spell_type_describe(spell, "Raises a mirror that can reflect bolts and arrows for a time");
		spell_type_set_mana(spell, 10, 40);
		spell_type_set_difficulty(spell, 20, 70);
		spell_type_init_demonology(spell,
					   demonology_demon_cloak_info,
					   demonology_demon_cloak);
	}

	{
		spell_type *spell = spell_new(&DEMON_SUMMON, "DEMON_SUMMON", "Summon Demon");
		spell_type_describe(spell, "Summons a leveled demon to your side");
		spell_type_describe(spell, "At level 35 it summons a high demon");
		spell_type_set_mana(spell, 10, 50);
		spell_type_set_difficulty(spell, 5, 30);
		spell_type_init_demonology(spell,
					   demonology_summon_demon_info,
					   demonology_summon_demon);
	}

	{
		spell_type *spell = spell_new(&DISCHARGE_MINION, "DISCHARGE_MINION", "Discharge Minion");
		spell_type_describe(spell, "The targeted pet will explode in a burst of gravity");
		spell_type_set_mana(spell, 20, 50);
		spell_type_set_difficulty(spell, 10, 30);
		spell_type_init_demonology(spell,
					   demonology_discharge_minion_info,
					   demonology_discharge_minion);
	}

	{
		spell_type *spell = spell_new(&CONTROL_DEMON, "CONTROL_DEMON", "Control Demon");
		spell_type_describe(spell, "Attempts to control a demon");
		spell_type_set_mana(spell, 30, 70);
		spell_type_set_difficulty(spell, 25, 55);
		spell_type_init_demonology(spell,
					   demonology_control_demon_info,
					   demonology_control_demon);
	}

	{
		spell_type *spell = spell_new(&DEVICE_HEAL_MONSTER, "DEVICE_HEAL_MONSTER", "Heal Monster");
		spell_type_describe(spell, "Heals a monster");
		spell_type_set_mana(spell, 5, 20);
		spell_type_set_difficulty(spell, 3, 15);
		spell_type_init_device(spell,
				       device_heal_monster_info,
				       device_heal_monster);

		spell_type_set_device_charges(spell, "10+d10");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 17;
			range_init(&device_allocation->base_level, 1, 15);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_SPEED_MONSTER, "DEVICE_SPEED_MONSTER", "Haste Monster");
		spell_type_describe(spell, "Haste a monster");
		spell_type_set_mana(spell, 10, 10);
		spell_type_set_difficulty(spell, 10, 30);
		spell_type_init_device(spell,
				       device_haste_monster_info,
				       device_haste_monster);

		spell_type_set_device_charges(spell, "10+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 7;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 20, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_WISH, "DEVICE_WISH", "Wish");
		spell_type_describe(spell, "This grants you a wish, beware of what you ask for!");
		spell_type_set_mana(spell, 400, 400);
		spell_type_set_difficulty(spell, 50, 99);
		spell_type_init_device(spell,
				       device_wish_info,
				       device_wish);

		spell_type_set_device_charges(spell, "1+d2");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 98;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_SUMMON, "DEVICE_SUMMON", "Summon");
		spell_type_describe(spell, "Summons hostile monsters near you");
		spell_type_set_mana(spell, 5, 25);
		spell_type_set_difficulty(spell, 5, 20);
		spell_type_init_device(spell,
				       device_summon_monster_info,
				       device_summon_monster);

		spell_type_set_device_charges(spell, "1+d20");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 13;
			range_init(&device_allocation->base_level, 1, 40);
			range_init(&device_allocation->max_level, 25, 50);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_MANA, "DEVICE_MANA", "Mana");
		spell_type_describe(spell, "Restores a part(or all) of your mana");
		spell_type_set_mana(spell, 1, 1);
		spell_type_set_difficulty(spell, 30, 80);
		spell_type_init_device(spell,
				       device_mana_info,
				       device_mana);

		spell_type_set_device_charges(spell, "2+d3");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 78;
			range_init(&device_allocation->base_level, 1, 5);
			range_init(&device_allocation->max_level, 20, 35);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_NOTHING, "DEVICE_NOTHING", "Nothing");
		spell_type_describe(spell, "It does nothing.");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 0);
		spell_type_init_device(spell,
				       device_nothing_info,
				       device_nothing);

		spell_type_set_device_charges(spell, "0+d0");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 3;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}

		{
			device_allocation *device_allocation = device_allocation_new(TV_WAND);
			device_allocation->rarity = 3;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 1, 1);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_MAGGOT, "DEVICE_MAGGOT", "Artifact Maggot");
		spell_type_set_activation_timeout(spell, "10+d50");
		spell_type_describe(spell, "terrify");
		spell_type_set_mana(spell, 7, 7);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_init_device(spell,
				       device_maggot_info,
				       device_maggot);
	}

	{
		spell_type *spell = spell_new(&DEVICE_HOLY_FIRE, "DEVICE_HOLY_FIRE", "Holy Fire of Mithrandir");
		spell_type_describe(spell, "The Holy Fire created by this staff will deeply(double damage) burn");
		spell_type_describe(spell, "all that is evil.");
		spell_type_set_mana(spell, 50, 150);
		spell_type_set_difficulty(spell, 30, 75);
		spell_type_init_device(spell,
				       device_holy_fire_info,
				       device_holy_fire);

		spell_type_set_device_charges(spell, "2+d5");

		{
			device_allocation *device_allocation = device_allocation_new(TV_STAFF);
			device_allocation->rarity = 999;
			range_init(&device_allocation->base_level, 1, 1);
			range_init(&device_allocation->max_level, 35, 35);
			spell_type_add_device_allocation(spell, device_allocation);
		}
	}

	{
		spell_type *spell = spell_new(&DEVICE_ETERNAL_FLAME, "DEVICE_ETERNAL_FLAME", "Artifact Eternal Flame");
		spell_type_set_activation_timeout(spell, "0");
		spell_type_describe(spell, "Imbuing an object with the eternal fire");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, 0);
		spell_type_init_device(spell,
				       device_eternal_flame_info,
				       device_eternal_flame);
	}

	{
		spell_type *spell = spell_new(&MUSIC_STOP, "MUSIC_STOP", "Stop singing(I)");
		spell_type_describe(spell, "Stops the current song, if any.");
		spell_type_set_mana(spell, 0, 0);
		spell_type_set_difficulty(spell, 1, -400);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_music(spell,
				      1,
				      music_stop_singing_info,
				      music_stop_singing_spell);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HOLD, "MUSIC_HOLD", "Holding Pattern(I)");
		spell_type_describe(spell, "Slows down all monsters listening the song.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 1, 10);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_music_lasting(
			spell,
			1,
			music_holding_pattern_info,
			music_holding_pattern_spell,
			music_holding_pattern_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_CONF, "MUSIC_CONF", "Illusion Pattern(II)");
		spell_type_describe(spell, "Tries to confuse all monsters listening the song.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 2, 15);
		spell_type_set_difficulty(spell, 5, 30);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_music_lasting(
			spell,
			2,
			music_illusion_pattern_info,
			music_illusion_pattern_spell,
			music_illusion_pattern_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_STUN, "MUSIC_STUN", "Stun Pattern(IV)");
		spell_type_describe(spell, "Stuns all monsters listening the song.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 3, 25);
		spell_type_set_difficulty(spell, 10, 45);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_music_lasting(
			spell,
			4,
			music_stun_pattern_info,
			music_stun_pattern_spell,
			music_stun_pattern_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_LITE, "MUSIC_LITE", "Song of the Sun(I)");
		spell_type_describe(spell, "Provides light as long as you sing.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 1, 1);
		spell_type_set_difficulty(spell, 1, 20);
		spell_type_set_castable_while_blind(spell, TRUE);
		spell_type_init_music_lasting(
			spell,
			1,
			music_song_of_the_sun_info,
			music_song_of_the_sun_spell,
			music_song_of_the_sun_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HEAL, "MUSIC_HEAL", "Flow of Life(II)");
		spell_type_describe(spell, "Heals you as long as you sing.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 5, 30);
		spell_type_set_difficulty(spell, 7, 35);
		spell_type_init_music_lasting(
			spell,
			2,
			music_flow_of_life_info,
			music_flow_of_life_spell,
			music_flow_of_life_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_HERO, "MUSIC_HERO", "Heroic Ballad(II)");
		spell_type_describe(spell, "Increases melee accuracy");
		spell_type_describe(spell, "At level 10 it increases it even more and reduces armour a bit");
		spell_type_describe(spell, "At level 20 it increases it again");
		spell_type_describe(spell, "At level 25 it grants protection against chaos and confusion");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 4, 14);
		spell_type_set_difficulty(spell, 10, 45);
		spell_type_init_music_lasting(
			spell,
			2,
			music_heroic_ballad_info,
			music_heroic_ballad_spell,
			music_heroic_ballad_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_TIME, "MUSIC_TIME", "Hobbit Melodies(III)");
		spell_type_describe(spell, "Greatly increases your reflexes allowing you to block more melee blows.");
		spell_type_describe(spell, "At level 15 it also makes you faster.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 10, 30);
		spell_type_set_difficulty(spell, 20, 70);
		spell_type_init_music_lasting(
			spell,
			3,
			music_hobbit_melodies_info,
			music_hobbit_melodies_spell,
			music_hobbit_melodies_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_MIND, "MUSIC_MIND", "Clairaudience(IV)");
		spell_type_describe(spell, "Allows you to sense monster minds as long as you sing.");
		spell_type_describe(spell, "At level 10 it identifies all objects in a radius on the floor,");
		spell_type_describe(spell, "as well as probing monsters in that radius.");
		spell_type_describe(spell, "Consumes the amount of mana each turn.");
		spell_type_set_mana(spell, 15, 30);
		spell_type_set_difficulty(spell, 25, 75);
		spell_type_init_music_lasting(
			spell,
			4,
			music_clairaudience_info,
			music_clairaudience_spell,
			music_clairaudience_lasting);
	}

	{
		spell_type *spell = spell_new(&MUSIC_BLOW, "MUSIC_BLOW", "Blow(I)");
		spell_type_describe(spell, "Produces a powerful, blowing, sound all around you.");
		spell_type_set_mana(spell, 3, 30);
		spell_type_set_difficulty(spell, 4, 20);
		spell_type_init_music(spell,
				      1,
				      music_blow_info,
				      music_blow_spell);
	}

	{
		spell_type *spell = spell_new(&MUSIC_WIND, "MUSIC_WIND", "Gush of Wind(II)");
		spell_type_describe(spell, "Produces a outgoing gush of wind that sends monsters away.");
		spell_type_set_mana(spell, 15, 45);
		spell_type_set_difficulty(spell, 14, 30);
		spell_type_init_music(spell,
				      2,
				      music_gush_of_wind_info,
				      music_gush_of_wind_spell);
	}

	{
		spell_type *spell = spell_new(&MUSIC_YLMIR, "MUSIC_YLMIR", "Horns of Ylmir(III)");
		spell_type_describe(spell, "Produces an earth shaking sound.");
		spell_type_set_mana(spell, 25, 30);
		spell_type_set_difficulty(spell, 20, 20);
		spell_type_init_music(spell,
				      3,
				      music_horns_of_ylmir_info,
				      music_horns_of_ylmir_spell);
	}

	{
		spell_type *spell = spell_new(&MUSIC_AMBARKANTA, "MUSIC_AMBARKANTA", "Ambarkanta(IV)");
		spell_type_describe(spell, "Produces a reality shaking sound that transports you to a nearly");
		spell_type_describe(spell, "identical reality.");
		spell_type_set_mana(spell, 70, 70);
		spell_type_set_difficulty(spell, 25, 60);
		spell_type_init_music(spell,
				      4,
				      music_ambarkanta_info,
				      music_ambarkanta_spell);
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
