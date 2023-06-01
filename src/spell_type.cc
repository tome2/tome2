#include "spell_type.hpp"

#include "range.hpp"
#include "device_allocation.hpp"
#include "dice.hpp"
#include "skills_defs.hpp"
#include "spells4.hpp"
#include "stats.hpp"

#include <cassert>
#include <vector>
#include <string>
#include <type_traits>

#define SCHOOL_IDXS_MAX 3

/**
 * Spell type definition.
 */
struct spell_type
{
	const char *name;                      /* Name */
	byte skill_level;               /* Required level (to learn) */
	std::vector<std::string> m_description;       /* List of strings */

	casting_result (*effect_func)();  /* Spell effect function */
	std::string (*info_func)();           /* Information function */
	int (*lasting_func)();         /* Lasting effect function */
	bool (*depend_func)();         /* Check dependencies */

	s16b minimum_pval;              /* Minimum required pval for item-based spells */

	enum casting_type casting_type; /* Type of casting required */
	s16b         casting_stat;      /* Stat used for casting */

	bool        castable_while_blind;
	bool        castable_while_confused;

	dice_type          device_charges;      /* Number of charges for devices */
	std::vector<device_allocation *> m_device_allocation;	/* Allocation table for devices */

	s16b         random_type;       /* Type of random items in which skill may appear */

	s32b         failure_rate;      /* Failure rate */

	s32b         inertia_difficulty; /* Mana cost when used in Inertia Control */
	s32b         inertia_delay;      /* Delay between castings */

	range_type   mana_range;

	size_t school_idxs_count;
	s32b school_idxs[3];

public:

	spell_type(const char *_name)
		: name(_name)
		, skill_level(0)
		, m_description()
		, effect_func(nullptr)
		, info_func(nullptr)
		, lasting_func(nullptr)
		, depend_func(nullptr)
		, minimum_pval(0)
		, casting_type(USE_SPELL_POINTS)
		, casting_stat(0)
		, castable_while_blind(false)
		, castable_while_confused(false)
		, device_charges({ 0, 0, 0 })
		, m_device_allocation()
		, random_type(-1)
		, failure_rate(0)
		, inertia_difficulty(-1)
		, inertia_delay(-1)
		, mana_range({ -1, -1 })
		, school_idxs_count(0)
		, school_idxs{ -1, -1, -1 }
	{
	}

};

static void school_idx_add_new(spell_type *spell, s32b i)
{
	assert(spell != NULL);
	assert(spell->school_idxs_count < SCHOOL_IDXS_MAX);

	spell->school_idxs[spell->school_idxs_count] = i;
	spell->school_idxs_count++;
}

void spell_type_set_inertia(spell_type *spell, s32b difficulty, s32b delay)
{
	assert(spell != NULL);
	spell->inertia_difficulty = difficulty;
	spell->inertia_delay = delay;
}

void spell_type_init_music(spell_type *spell,
			   s16b minimum_pval,
			   std::string (*info_func)(),
			   casting_result (*effect_func)())
{
	assert(spell != NULL);

	/* Set up callbacks */
	spell->info_func = info_func;
	spell->effect_func = effect_func;

	/* Use spell points, but CHR for success/failure calculations */
	spell->casting_type = USE_SPELL_POINTS;
	spell->casting_stat = A_CHR;
	spell->random_type = SKILL_MUSIC;
	spell->minimum_pval = minimum_pval;
	/* Add school */
	school_idx_add_new(spell, SCHOOL_MUSIC);
}

void spell_type_init_music_lasting(spell_type *spell,
				   s16b minimum_pval,
				   std::string (*info_func)(),
				   casting_result (*effect_func)(),
				   int (*lasting_func)())
{
	spell_type_init_music(
		spell,
		minimum_pval,
		info_func,
		effect_func);

	spell->lasting_func = lasting_func;
}

void spell_type_init_mage(spell_type *spell,
			  random_type random_type,
			  s32b school_idx,
			  std::string (*info_func)(),
			  casting_result (*effect_func)())
{
	assert(spell != NULL);

	spell->info_func = info_func;
	spell->effect_func = effect_func;

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
		assert(false);
	}

	/* Add first school */
	spell_type_add_school(spell, school_idx);
}

void spell_type_init_priest(spell_type *spell,
			    s32b school_idx,
			    std::string (*info_func)(),
			    casting_result (*effect_func)())
{
	assert(spell != NULL);

	spell->info_func = info_func;
	spell->effect_func = effect_func;

	spell->random_type = SKILL_SPIRITUALITY;
	spell->casting_type = USE_PIETY;
	spell->casting_stat = A_WIS;

	school_idx_add_new(spell, school_idx);
}

void spell_type_init_device(spell_type *spell,
			    std::string (*info_func)(),
			    casting_result (*effect_func)())
{
	assert(spell != NULL);

	spell_type_init_mage(spell,
			     NO_RANDOM,
			     SCHOOL_DEVICE,
			     info_func,
			     effect_func);
}

void spell_type_init_demonology(spell_type *spell,
				std::string (*info_func)(),
				casting_result (*effect_func)())
{
	spell_type_init_mage(spell,
			     NO_RANDOM,
			     SCHOOL_DEMON,
			     info_func,
			     effect_func);
}

void spell_type_init_geomancy(spell_type *spell,
			      std::string (*info_func)(),
			      casting_result (*effect_func)(),
			      bool (*depend_func)())
{
	spell_type_init_mage(spell,
			     NO_RANDOM,
			     SCHOOL_GEOMANCY,
			     info_func,
			     effect_func);

	spell->depend_func = depend_func;
}

void spell_type_set_difficulty(spell_type *spell, byte skill_level, s32b failure_rate)
{
	assert(spell != NULL);

	spell->skill_level = skill_level;
	spell->failure_rate = failure_rate;
}

void spell_type_set_mana(spell_type *spell, s32b min, s32b max)
{
	assert(spell != NULL);

	range_init(&spell->mana_range, min, max);
}

void spell_type_set_castable_while_blind(spell_type *spell, bool value)
{
	assert(spell != NULL);

	spell->castable_while_blind = value;
}

void spell_type_set_castable_while_confused(spell_type *spell, bool value)
{
	assert(spell != NULL);

	spell->castable_while_confused = value;
}

void spell_type_describe(spell_type *spell, const char *line)
{
	assert(spell != NULL);

	spell->m_description.push_back(std::string(line));
}

void spell_type_add_school(spell_type *spell, s32b school_idx)
{
	school_idx_add_new(spell, school_idx);
}

void spell_type_set_device_charges(spell_type *spell, const char *charges_s)
{
	assert(spell != NULL);

	dice_parse_checked(&spell->device_charges, charges_s);
}

void spell_type_add_device_allocation(spell_type *spell, struct device_allocation *a)
{
	assert(spell != NULL);
	assert(a != NULL);
	spell->m_device_allocation.push_back(a);
}

spell_type *spell_type_new(const char *name)
{
	spell_type *spell = new spell_type(name);
	assert(spell != NULL);
	return spell;
}

int spell_type_produce_effect_lasting(spell_type *spell)
{
	assert(spell->lasting_func != NULL);
	return spell->lasting_func();
}

casting_result spell_type_produce_effect(spell_type *spell)
{
	assert(spell->effect_func != NULL);
	return spell->effect_func();
}

const char *spell_type_name(spell_type *spell)
{
	assert(spell != NULL);

	return spell->name;
}

int spell_type_skill_level(spell_type *spell)
{
	assert(spell != NULL);

	return spell->skill_level;
}

void spell_type_description_foreach(spell_type *spell, std::function<void (std::string const &text)> callback)
{
	for (auto line: spell->m_description)
	{
		callback(line);
	}
}

long spell_type_roll_charges(spell_type *spell)
{
	return dice_roll(&spell->device_charges);
}

device_allocation *spell_type_device_allocation(spell_type *spell, byte tval)
{
	for (auto device_allocation_ptr: spell->m_device_allocation)
	{
		if (device_allocation_ptr->tval == tval)
		{
			return device_allocation_ptr;
		}
	}

	return NULL;
}

bool spell_type_uses_piety_to_cast(spell_type *spell)
{
	assert(spell != NULL);
	return spell->casting_type == USE_PIETY;
}

bool spell_type_castable_while_blind(spell_type *spell)
{
	assert(spell != NULL);
	return spell->castable_while_blind;
}

bool spell_type_castable_while_confused(spell_type *spell)
{
	assert(spell != NULL);
	return spell->castable_while_confused;
}

s16b spell_type_minimum_pval(spell_type *spell)
{
	return spell->minimum_pval;
}

s16b spell_type_random_type(spell_type *spell)
{
	return spell->random_type;
}

std::vector<s32b> const spell_type_get_schools(spell_type *spell)
{
	std::vector<s32b> school_idxs;
	for (size_t i = 0; i < spell->school_idxs_count; i++)
	{
		school_idxs.push_back(spell->school_idxs[i]);
	}
	return school_idxs;
}

bool spell_type_inertia(spell_type *spell, s32b *difficulty, s32b *delay)
{
	if ((spell->inertia_difficulty < 0) ||
	    (spell->inertia_delay < 0))
	{
		return false;
	}

	if (difficulty != NULL)
	{
		*difficulty = spell->inertia_difficulty;
	}

	if (delay != NULL)
	{
		*delay = spell->inertia_delay;
	}

	return true;
}

std::string spell_type_info(spell_type *spell)
{
	assert(spell != NULL);

	return spell->info_func();
}

s32b spell_type_failure_rate(spell_type *spell)
{
	assert(spell != NULL);

	return spell->failure_rate;
}

s16b spell_type_casting_stat(spell_type *spell)
{
	assert(spell != NULL);

	return spell->casting_stat;
}

void spell_type_mana_range(spell_type *spell, range_type *range)
{
	assert(spell != NULL);

	if (range != NULL)
	{
		*range = spell->mana_range;
	}
}

bool spell_type_dependencies_satisfied(spell_type *spell)
{
	assert(spell != NULL);

	if (spell->depend_func != NULL) {
		return spell->depend_func();
	} else {
		return true;
	}
}
