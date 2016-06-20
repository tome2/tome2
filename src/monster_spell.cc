#include "monster_spell.hpp"

#include "monster_spell_flag.hpp"

#include <boost/preprocessor/cat.hpp>

std::vector<monster_spell const *> const &monster_spells()
{
	// Static instance for one-time initialization.
	static std::vector<monster_spell const *> instance;

	if (instance.empty())
	{
#define SF(tier, index, name, is_summon, is_annoy, is_damage, is_bolt, is_smart, is_innate, is_escape, is_tactic, is_haste, is_heal) \
		instance.emplace_back(new monster_spell { \
			BOOST_PP_CAT(SF_, BOOST_PP_CAT(name, _IDX)), \
			BOOST_PP_CAT(SF_, name), \
			#name, \
			is_summon, \
			is_annoy, \
			is_damage, \
			is_bolt, \
			is_smart, \
			is_innate, \
			is_escape, \
			is_tactic, \
			is_haste, \
			is_heal, \
			!is_innate, \
		});
#include "monster_spell_flag_list.hpp"
#undef SF
	};

	return instance;
}
