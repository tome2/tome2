#include "spells6.hpp"

#include "game.hpp"
#include "gods.hpp"
#include "lua_bind.hpp"
#include "object2.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "skills.hpp"
#include "skill_type.hpp"
#include "spell_type.hpp"
#include "spells4.hpp"
#include "variable.hpp"

#include <cassert>
#include <vector>
#include <type_traits>

struct school_provider
{
	byte deity_idx; /* Deity which provides school levels */

	s16b skill_idx; /* Skill used for determining the boost */

	long mul; /* Multiplier */

	long div; /* Divisor */
};

struct school_provider_list {
public:
	std::vector<school_provider> v;
};

static school_provider school_provider_new(byte deity_idx, long mul, long div)
{
	school_provider p;
	p.deity_idx = deity_idx;
	p.skill_idx = SKILL_PRAY;
	p.mul = mul;
	p.div = div;
	return p;
}

school_type *school_at(int index)
{
	assert(index >= 0);
	assert(index < schools_count);

	return &schools[index];
}

static void school_init(school_type *school, const char *name, s16b skill)
{
	assert(school != NULL);

	static_assert(std::is_pod<school_type>::value, "Cannot initialize non-POD using memset!");
	memset(school, 0, sizeof(school_type));

	school->providers = new school_provider_list();
	school->name = name;
	school->skill = skill;

	school->deity_idx = -1;
}

static school_type *school_new(s32b *school_idx, const char *name, s16b skill)
{
	assert(schools_count < SCHOOLS_MAX);

	*school_idx = schools_count;
	schools_count++;

	school_type *school = &schools[*school_idx];
	school_init(school, name, skill);

	return school;
}

static school_type *sorcery_school_new(s32b *school_idx, const char *name, s16b skill)
{
	school_type *school = school_new(school_idx, name, skill);
	school->spell_power = true;
	school->sorcery = true;
	return school;
}

static school_type *god_school_new(s32b *school_idx, byte god)
{
	school_type *school = NULL;
	deity_type *deity = NULL;

	/* Get the god */
	deity = god_at(god);
	assert(deity != NULL);

	/* Ignore gods which aren't enabled for this module. */
	if (god_enabled(deity))
	{
		school = school_new(school_idx, deity->name, SKILL_PRAY);
		school->spell_power = true;
		school->deity_idx = god;
		school->deity = deity;
		return school;
	}
	else
	{
		return NULL;
	}
}

static void school_god(school_type *school, byte god, int mul, int div)
{
	assert(school->providers != nullptr);

	deity_type *deity = god_at(god);
	assert(deity != NULL);

	/* Ignore gods which aren't enabled for this module. */
	if (god_enabled(deity))
	{
		school->providers->v.push_back(school_provider_new(god, mul, div));
	}
}

static int udun_bonus_levels()
{
	return (p_ptr->lev * 2) / 3;
}

static bool geomancy_depends_satisfied()
{
	/* Require at least one point in each school */
	if ((get_skill(SKILL_FIRE) <= 0) ||
	    (get_skill(SKILL_AIR) <= 0) ||
	    (get_skill(SKILL_EARTH) <= 0) ||
	    (get_skill(SKILL_WATER) <= 0))
	{
		return false;
	}

 	/* Require to wield a Mage Staff, as the spells requries the
	 * caster to stomp the floor with it. */
	auto o_ptr = get_object(INVEN_WIELD);

	return ((o_ptr != NULL) &&
		(o_ptr->k_ptr) &&
		(o_ptr->tval == TV_MSTAFF));
}

long get_provided_levels(school_type *school)
{
	auto const &s_info = game->s_info;

	for (auto school_provider: school->providers->v)
	{
		if (school_provider.deity_idx == p_ptr->pgod)
		{
			return (s_info[school_provider.skill_idx].value * school_provider.mul) / school_provider.div;
		}
	}

	return 0;
}

struct get_level_school_callback_data {
	bool allow_spell_power;
	long bonus;
	long lvl;
	long num;
};

static bool get_level_school_callback(struct get_level_school_callback_data *data, int school_idx)
{
	auto const &s_info = game->s_info;

	school_type *school = school_at(school_idx);
	long r = 0, s = 0, p = 0, ok = 0;

	/* Does it require we worship a specific god? */
	if ((school->deity_idx > 0) &&
	    (school->deity_idx != p_ptr->pgod))
	{
		return false;
	}

	/* Take the basic skill value */
	r = s_info[school->skill].value;

	/* Do we pass tests? */
	if ((school->depends_satisfied != NULL) &&
	    (!school->depends_satisfied()))
	{
		return false;
	}

	/* Include effects of Sorcery (if applicable) */
	if (school->sorcery)
	{
		s = s_info[SKILL_SORCERY].value;
	}

	/* Include effects of Spell Power? Every school must
	 * allow use of Spell Power for it to apply. */
	if (!school->spell_power)
	{
		data->allow_spell_power = false;
	}

	/* Calculate effects of provided levels */
	p = get_provided_levels(school);

	/* Find the highest of Skill, Sorcery and Provided levels. */
	ok = r;
	if (ok < s)
	{
		ok = s;
	}
	if (ok < p)
	{
		ok = p;
	}

	/* Do we need to add a special bonus? */
	if (school->bonus_levels != NULL)
	{
		data->bonus += (school->bonus_levels() * (SKILL_STEP / 10));
	}

	/* All schools must be non-zero to be able to use it. */
	if (ok <= 0)
	{
		return false;
	}

	/* Apply it */
	data->lvl += ok;
	data->num += 1;

	/* Keep going */
	return true;
}

void get_level_school(spell_type *spell, s32b max, s32b min, s32b *level, bool *na)
{
	assert(level != NULL);
	assert(na != NULL);

	/* Do we pass tests? */
	if (!spell_type_dependencies_satisfied(spell))
	{
		*level = min;
		*na = true;
		return;
	}

	/* Set up initial state */
	get_level_school_callback_data data;
	data.allow_spell_power = true;
	data.bonus = 0;
	data.lvl = 0;
	data.num = 0;
	
	// Go through all the spell's schools and count up all the
	// levels and make sure we can actually cast the spell.
	for (auto school_idx : spell_type_get_schools(spell))
	{
		if (!get_level_school_callback(&data, school_idx))
		{
			*level = min;
			*na = true;
			return;
		}
	}

	/* Add the Spellpower skill as a bonus on top */
	if (data.allow_spell_power)
	{
		data.bonus += (get_skill_scale(SKILL_SPELL, 20) * (SKILL_STEP / 10));
	}

	/* Add bonus from objects */
	data.bonus += (p_ptr->to_s * (SKILL_STEP / 10));

	/* We divide by 10 because otherwise we can overflow a s32b
	 * and we can use a u32b because the value can be negative.
	 * The loss of information should be negligible since 1 skill
	 * point is 1000 internally. */
	data.lvl = (data.lvl / data.num) / 10;
	data.lvl = lua_get_level(spell, data.lvl, max, min, data.bonus);

	/* Result */
	*level = data.lvl;
	*na = false;
}

void schools_init()
{
	{
		school_type *school = sorcery_school_new(&SCHOOL_MANA, "Mana", SKILL_MANA);
		school_god(school, GOD_ERU, 1, 2);
		school_god(school, GOD_VARDA, 1, 4);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_FIRE, "Fire", SKILL_FIRE);
		school_god(school, GOD_AULE, 3, 5);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_AIR, "Air", SKILL_AIR);
		school_god(school, GOD_MANWE, 2, 3);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_WATER, "Water", SKILL_WATER);
		school_god(school, GOD_YAVANNA, 1, 2);
		school_god(school, GOD_ULMO, 3, 5);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_EARTH, "Earth", SKILL_EARTH);
		school_god(school, GOD_AULE, 1, 3);
		school_god(school, GOD_TULKAS, 4, 5);
		school_god(school, GOD_YAVANNA, 1, 2);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_CONVEYANCE, "Conveyance", SKILL_CONVEYANCE);
		school_god(school, GOD_MANWE, 1, 2);
	}

	{
		school_type *school = school_new(&SCHOOL_GEOMANCY, "Geomancy", SKILL_GEOMANCY);
		school->spell_power = true;
		school->depends_satisfied = geomancy_depends_satisfied;
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_DIVINATION, "Divination", SKILL_DIVINATION);
		school_god(school, GOD_ERU, 2, 3);
		school_god(school, GOD_MANDOS, 1, 3);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_TEMPORAL, "Temporal", SKILL_TEMPORAL);
		school_god(school, GOD_YAVANNA, 1, 6);
		school_god(school, GOD_MANDOS, 1, 4);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_NATURE, "Nature", SKILL_NATURE);
		school_god(school, GOD_YAVANNA, 1, 2);
		school_god(school, GOD_ULMO, 1, 2);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_META, "Meta", SKILL_META);
		school_god(school, GOD_MANWE, 1, 3);
		school_god(school, GOD_VARDA, 1, 2);
	}

	{
		school_type *school = sorcery_school_new(&SCHOOL_MIND, "Mind", SKILL_MIND);
		school_god(school, GOD_ERU, 1, 3);
 		school_god(school, GOD_MELKOR, 1, 3);
	}

	{
		school_type *school = school_new(&SCHOOL_UDUN, "Udun", SKILL_UDUN);
		school->bonus_levels = udun_bonus_levels;
	}

	{
		school_new(&SCHOOL_DEMON, "Demon", SKILL_DAEMON);
	}

	/* God-specific schools; all with a standard setup */
	{
		god_school_new(&SCHOOL_ERU, GOD_ERU);
		god_school_new(&SCHOOL_MANWE, GOD_MANWE);
		god_school_new(&SCHOOL_TULKAS, GOD_TULKAS);
		god_school_new(&SCHOOL_MELKOR, GOD_MELKOR);
		god_school_new(&SCHOOL_YAVANNA, GOD_YAVANNA);

		god_school_new(&SCHOOL_AULE, GOD_AULE);
		god_school_new(&SCHOOL_VARDA, GOD_VARDA);
		god_school_new(&SCHOOL_ULMO, GOD_ULMO);
		god_school_new(&SCHOOL_MANDOS, GOD_MANDOS);
	}

	/* Placeholder schools */
	{
		school_new(&SCHOOL_DEVICE, "Device", SKILL_DEVICE);
		school_new(&SCHOOL_MUSIC, "Music", SKILL_MUSIC);
	}

}

void mana_school_calc_mana(int *msp)
{
	if (get_skill(SKILL_MANA) >= 35)
	{
		*msp = *msp + (*msp * ((get_skill(SKILL_MANA) - 34)) / 100);
	}
}
