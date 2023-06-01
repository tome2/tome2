#include "tome/squelch/condition_fwd.hpp"
#include "tome/squelch/condition.hpp"

#include <map>
#include <boost/algorithm/string/predicate.hpp>

#include "jsoncons_helpers.hpp"
#include "tome/squelch/cursor.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "../ability_type.hpp"
#include "../game.hpp"
#include "../object1.hpp"
#include "../object2.hpp"
#include "../object_kind.hpp"
#include "../object_type.hpp"
#include "../player_race.hpp"
#include "../player_race_mod.hpp"
#include "../player_spec.hpp"
#include "../player_type.hpp"
#include "../skills.hpp"
#include "../skill_type.hpp"
#include "../util.hpp"
#include "../variable.hpp"
#include "../z-term.hpp"

#include <fmt/format.h>

namespace squelch {

EnumStringMap<match_type> &match_mapping()
{
	// TODO: This is quite ugly and leads to valgrind complaints
	static auto m = new EnumStringMap<match_type> {
		{ match_type::AND, "and" },
		{ match_type::OR, "or" },
		{ match_type::NOT, "not" },
		{ match_type::NAME, "name" },
		{ match_type::CONTAIN, "contain" },
		{ match_type::INSCRIBED, "inscribed" },
		{ match_type::DISCOUNT, "discount" },
		{ match_type::SYMBOL, "symbol" },
		{ match_type::STATUS, "status" },
		{ match_type::TVAL, "tval" },
		{ match_type::SVAL, "sval" },
		{ match_type::RACE, "race" },
		{ match_type::SUBRACE, "subrace" },
		{ match_type::CLASS, "class" },
		{ match_type::LEVEL, "level" },
		{ match_type::SKILL, "skill" },
		{ match_type::ABILITY, "ability" },
		{ match_type::INVENTORY, "inventory" },
		{ match_type::EQUIPMENT, "equipment" } };
	return *m;
};

jsoncons::json Condition::to_json() const
{
	// Start with an object with only 'type' property
	jsoncons::json j;
	j["type"] = match_mapping().stringify(match);

	// Add sub-class properties
	to_json(j);
	// Return the completed JSON
	return j;
}

void Condition::display(TreePrinter *tree_printer, Cursor *cursor) const
{
	assert(tree_printer);

	// Use normal or "selected" colours?
	uint8_t bcol = TERM_L_GREEN;
	uint8_t ecol = TERM_GREEN;
	if (cursor->is_selected(this))
	{
		bcol = TERM_VIOLET;
		ecol = TERM_VIOLET;
	}

	// Indent a level and display tree.
	tree_printer->indent();
	write_tree(tree_printer, cursor, ecol, bcol);
	tree_printer->dedent();
}

std::shared_ptr<Condition> Condition::parse_condition(jsoncons::json const &condition_json)
{
	// Parsers for concrete types of conditions.
	static std::map< match_type,
		  std::function<std::shared_ptr<Condition>(jsoncons::json const &)>> parsers {
		{ match_type::AND, &AndCondition::from_json },
		{ match_type::OR, &OrCondition::from_json },
		{ match_type::NOT, &NotCondition::from_json },
		{ match_type::INVENTORY, &InventoryCondition::from_json },
		{ match_type::EQUIPMENT, &EquipmentCondition::from_json },
		{ match_type::NAME, &NameCondition::from_json },
		{ match_type::CONTAIN, &ContainCondition::from_json },
		{ match_type::SYMBOL, &SymbolCondition::from_json },
		{ match_type::INSCRIBED, &InscriptionCondition::from_json },
		{ match_type::DISCOUNT, &DiscountCondition::from_json },
		{ match_type::TVAL, &TvalCondition::from_json },
		{ match_type::SVAL, &SvalCondition::from_json },
		{ match_type::STATUS, &StatusCondition::from_json },
		{ match_type::RACE, &RaceCondition::from_json },
		{ match_type::SUBRACE, &SubraceCondition::from_json },
		{ match_type::CLASS, &ClassCondition::from_json },
		{ match_type::LEVEL, &LevelCondition::from_json },
		{ match_type::SKILL, &SkillCondition::from_json },
		{ match_type::ABILITY, &AbilityCondition::from_json } };

	if (condition_json.is_null())
	{
		return nullptr;
	}

	auto maybe_type_s = get_optional<std::string>(condition_json, "type");
	if (!maybe_type_s)
	{
		msg_print("Missing/invalid 'type' in condition");
		return nullptr;
	}
	auto type_s = *maybe_type_s;

	match_type match;
	if (!match_mapping().parse(type_s, &match))
	{
		msg_format("Invalid 'type' in condition: %s", type_s.c_str());
		return nullptr;
	}

	// Look up parser and... parse
	auto parser_i = parsers.find(match);
	if (parser_i != parsers.end())
	{
		return parser_i->second(condition_json);
	}

	assert(false && "Missing parser");
	return nullptr;
}

jsoncons::json Condition::optional_to_json(std::shared_ptr<Condition> condition)
{
	return condition
		? condition->to_json()
		: jsoncons::null_type();
}

bool TvalCondition::is_match(object_type *o_ptr) const
{
	return (o_ptr->tval == m_tval);
}

std::shared_ptr<Condition> TvalCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_tval = get_optional<uint8_t>(j, "tval"))
	{
		return std::make_shared<TvalCondition>(*maybe_tval);
	}
	else
	{
		msg_print("Missing/invalid 'tval' property");
		return nullptr;
	}
}

void TvalCondition::to_json(jsoncons::json &j) const
{
	j["tval"]  = m_tval;
}

void TvalCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "tval");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, fmt::format("{}", m_tval));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

bool NameCondition::is_match(object_type *o_ptr) const
{
	char buf1[128];
	object_desc(buf1, o_ptr, -1, 0);

	return boost::algorithm::iequals(m_name, buf1);
}

std::shared_ptr<Condition> NameCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "name"))
	{
		return std::make_shared<NameCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'name' property");
		return nullptr;
	}
}

void NameCondition::write_tree(TreePrinter *p, Cursor *cursor, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "name");
	p->write(ecol, " is \"");
	p->write(TERM_WHITE, m_name.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void NameCondition::to_json(jsoncons::json &j) const
{
	j["name"] = m_name;
}

bool ContainCondition::is_match(object_type *o_ptr) const
{
	char buf1[128];
	object_desc(buf1, o_ptr, -1, 0);
	return boost::algorithm::icontains(buf1, m_contain);
}

std::shared_ptr<Condition> ContainCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "contain"))
	{
		return std::make_shared<ContainCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'contain' property");
		return nullptr;
	}
}

void ContainCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "name");
	p->write(ecol, " contains \"");
	p->write(TERM_WHITE, m_contain.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void ContainCondition::to_json(jsoncons::json &j) const
{
	j["contain"] = m_contain;
}

bool SvalCondition::is_match(object_type *o_ptr) const
{
	return (object_aware_p(o_ptr) &&
		(o_ptr->sval >= m_min) &&
		(o_ptr->sval <= m_max));
}

std::shared_ptr<Condition> SvalCondition::from_json(jsoncons::json const &j)
{
	auto min_j = get_optional<uint8_t>(j, "min");
	if (!min_j)
	{
		msg_print("Missing/invalid 'min' property");
		return nullptr;
	}

	auto max_j = get_optional<uint8_t>(j, "max");
	if (!max_j)
	{
		msg_print("Missing/invalid 'max' property");
		return nullptr;
	}

	return std::make_shared<SvalCondition>(*min_j, *max_j);
}

void SvalCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "sval");
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, fmt::format("{}", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, fmt::format("{}", m_max));
	p->write(TERM_WHITE, "\n");
}

void SvalCondition::to_json(jsoncons::json &j) const
{
	j["min"] = m_min;
	j["max"] = m_max;
}

void GroupingCondition::add_child(ConditionFactory const &factory)
{
	auto c_ptr = factory();
	if (c_ptr)
	{
		m_conditions.push_back(c_ptr);
	}
}

void GroupingCondition::remove_child(Condition *condition)
{
	m_conditions.erase(
		std::remove_if(
			std::begin(m_conditions),
			std::end(m_conditions),
			[&] (std::shared_ptr<Condition> p) {
				return p.get() == condition;
			}),
		std::end(m_conditions));
}

std::shared_ptr<Condition> GroupingCondition::first_child()
{
	if (!m_conditions.empty())
	{
		return m_conditions.front();
	}
	return nullptr;
}

std::shared_ptr<Condition> GroupingCondition::previous_child(Condition *current)
{
	std::shared_ptr<Condition> prev_condition;

	for (auto condition_p : m_conditions)
	{
		if (condition_p.get() == current)
		{
			// Do we have a previous child?
			if (prev_condition)
			{
				return prev_condition;
			}
			else
			{
				// No predecessor
				return nullptr;
			}
		}
		// Keep track of predecessor
		prev_condition = condition_p;
	}

	return nullptr;
}

std::shared_ptr<Condition> GroupingCondition::next_child(Condition *current)
{
	for (auto it = m_conditions.begin();
	     it != m_conditions.end();
	     it++)
	{
		if (it->get() == current)
		{
			it++;
			// Move to next child (if any)
			if (it == m_conditions.end())
			{
				// No successor
				return nullptr;
			}

			return *it;
		}
	}

	return nullptr;
}

std::vector< std::shared_ptr<Condition> > GroupingCondition::parse_conditions(jsoncons::json const &j)
{
	auto conditions_j = j.get_with_default<jsoncons::json>("conditions", jsoncons::null_type());

	if (conditions_j.is_null())
	{
		return std::vector< std::shared_ptr<Condition> >();
	}
	else if (!conditions_j.is_array())
	{
		msg_print("'conditions' property has invalid type");
		return std::vector< std::shared_ptr<Condition> >();
	}
	else
	{
		std::vector< std::shared_ptr<Condition> > subconditions;
		for (auto const &subcondition_j: conditions_j.array_value())
		{
			std::shared_ptr<Condition> subcondition =
				parse_condition(subcondition_j);

			if (subcondition != nullptr)
			{
				subconditions.push_back(subcondition);
			}
		}
		return subconditions;
	}
}

void GroupingCondition::to_json(jsoncons::json &j) const
{
	// Put all the sub-conditions into an array
	jsoncons::json::array ja;
	for (auto condition_p : m_conditions)
	{
		ja.push_back(optional_to_json(condition_p));
	}
	// Add to JSON object
	j["conditions"] = ja;
}

bool AndCondition::is_match(object_type *o_ptr) const
{
	for (auto condition_p : m_conditions)
	{
		if (!condition_p->is_match(o_ptr))
		{
			return false;
		}
	}
	return true;
}

std::shared_ptr<Condition> AndCondition::from_json(jsoncons::json const &j)
{
	auto condition = std::make_shared<AndCondition>();
	for (auto subcondition : parse_conditions(j))
	{
		condition->add_condition(subcondition);
	}
	return condition;
}

void AndCondition::write_tree(TreePrinter *p, Cursor *c, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "All of the following are true:");
	p->write(TERM_WHITE, "\n");

	for (auto condition_p : m_conditions)
	{
		condition_p->display(p, c);
	}
}

bool OrCondition::is_match(object_type *o_ptr) const
{
	for (auto condition_p : m_conditions)
	{
		if (condition_p->is_match(o_ptr))
		{
			return true;
		}
	}
	return false;
}

std::shared_ptr<Condition> OrCondition::from_json(jsoncons::json const &j)
{
	std::shared_ptr<OrCondition> condition =
		std::make_shared<OrCondition>();

	for (auto subcondition : parse_conditions(j))
	{
		condition->add_condition(subcondition);
	}

	return condition;
}

void OrCondition::write_tree(TreePrinter *p, Cursor *c, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "At least one of the following are true:");
	p->write(TERM_WHITE, "\n");

	for (auto condition_p : m_conditions)
	{
		condition_p->display(p, c);
	}
}

bool StatusCondition::is_match(object_type *o_ptr) const
{
	return m_status == object_status(o_ptr);
}

std::shared_ptr<Condition> StatusCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "status"))
	{
		std::string s = *maybe_s;
		status_type status;
		if (!status_mapping().parse(s, &status))
		{
			msg_format("Invalid 'status' property: %s", s.c_str());
			return nullptr;
		}

		return std::make_shared<StatusCondition>(status);
	}
	else
	{
		msg_print("Missing/invalid 'status' property");
		return nullptr;
	}
}

void StatusCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "status");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, status_mapping().stringify(m_status));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void StatusCondition::to_json(jsoncons::json &j) const
{
	j["status"] = status_mapping().stringify(m_status);
}

bool RaceCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_race, rp_ptr->title);
}

std::shared_ptr<Condition> RaceCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "race"))
	{
		return std::make_shared<RaceCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'race' property");
		return nullptr;
	}
}

void RaceCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Player ");
	p->write(bcol, "race");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, m_race.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void RaceCondition::to_json(jsoncons::json &j) const
{
	j["race"] = m_race;
}

bool SubraceCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_subrace, rmp_ptr->title);
}

std::shared_ptr<Condition> SubraceCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "subrace"))
	{
		return std::make_shared<SubraceCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'subrace' property");
		return nullptr;
	}
}

void SubraceCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Player ");
	p->write(bcol, "subrace");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, m_subrace.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void SubraceCondition::to_json(jsoncons::json &j) const
{
	j["subrace"] = m_subrace;
}

bool ClassCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_class, spp_ptr->title);
}

std::shared_ptr<Condition> ClassCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "class"))
	{
		return std::make_shared<ClassCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'class' property");
		return nullptr;
	}
}

void ClassCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Player ");
	p->write(bcol, "class");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, m_class.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void ClassCondition::to_json(jsoncons::json &j) const
{
	j["class"] = m_class;
}

bool InscriptionCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::icontains(
	        o_ptr->inscription,
		m_inscription);
}

std::shared_ptr<Condition> InscriptionCondition::from_json(jsoncons::json const &j)
{
	if (auto maybe_s = get_optional<std::string>(j, "inscription"))
	{
		return std::make_shared<InscriptionCondition>(*maybe_s);
	}
	else
	{
		msg_print("Missing/invalid 'inscription' property");
		return nullptr;
	}
}

void InscriptionCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "It is ");
	p->write(bcol, "inscribed");
	p->write(ecol, " with ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, m_inscription.c_str());
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void InscriptionCondition::to_json(jsoncons::json &j) const
{
	j["inscription"] = m_inscription;
}

bool DiscountCondition::is_match(object_type *o_ptr) const
{
	return (object_aware_p(o_ptr) &&
		(o_ptr->discount >= m_min) &&
		(o_ptr->discount <= m_max));
}

std::shared_ptr<Condition> DiscountCondition::from_json(jsoncons::json const &j)
{
	auto maybe_min = get_optional<int>(j, "min");
	if (!maybe_min)
	{
		msg_print("Missing/invalid 'min' property");
		return nullptr;
	}
	auto min = *maybe_min;

	auto maybe_max = get_optional<int>(j, "max");
	if (!maybe_max)
	{
		msg_print("Missing/invalid 'max' property");
		return nullptr;
	}
	auto max = *maybe_max;

	return std::make_shared<DiscountCondition>(min, max);
}

void DiscountCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "discount");
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, fmt::format("{}", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, fmt::format("{}", m_max));
	p->write(TERM_WHITE, "\n");
}

void DiscountCondition::to_json(jsoncons::json &j) const
{
	j["min"] = m_min;
	j["max"] = m_max;
}

bool LevelCondition::is_match(object_type *) const
{
	return ((p_ptr->lev >= m_min) &&
		(p_ptr->lev <= m_max));
}

std::shared_ptr<Condition> LevelCondition::from_json(jsoncons::json const &j)
{
	auto maybe_min = get_optional<int>(j, "min");
	if (!maybe_min)
	{
		msg_print("Missing/invalid 'min' property");
		return nullptr;
	}
	auto min = *maybe_min;

	auto maybe_max = get_optional<int>(j, "max");
	if (!maybe_max)
	{
		msg_print("Missing/invalid 'max' property");
		return nullptr;
	}
	int max = *maybe_max;

	return std::make_shared<LevelCondition>(min, max);
}

void LevelCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Your ");
	p->write(bcol, "level");
	p->write(ecol, " is from ");

	p->write(TERM_WHITE, fmt::format("{}", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, fmt::format("{}", m_max));
	p->write(TERM_WHITE, "\n");
}

void LevelCondition::to_json(jsoncons::json &j) const
{
	j["min"] = m_min;
	j["max"] = m_max;
}

bool SkillCondition::is_match(object_type *) const
{
	uint16_t sk = get_skill(m_skill_idx);
	return ((sk >= m_min) &&
		(sk <= m_max));
}

std::shared_ptr<Condition> SkillCondition::from_json(jsoncons::json const &j)
{
	auto maybe_min = get_optional<int>(j, "min");
	if (!maybe_min)
	{
		msg_print("Missing/invalid 'min' property");
		return nullptr;
	}
	auto min = *maybe_min;

	auto maybe_max = get_optional<int>(j, "max");
	if (!maybe_max)
	{
		msg_print("Missing/invalid 'max' property");
		return nullptr;
	}
	auto max = *maybe_max;

	auto maybe_s = get_optional<std::string>(j, "name");
	if (!maybe_s)
	{
		msg_print("Missing/invalid 'name' property");
		return nullptr;
	}
	auto s = *maybe_s;

	auto si = find_skill_i(s);
	if (si < 0)
	{
		msg_format("Invalid 'name' property: %s", s.c_str());
		return nullptr;
	}

	return std::make_shared<SkillCondition>(si, min, max);
}

void SkillCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	p->write(ecol, "Your skill in ");
	p->write(bcol, s_descriptors[m_skill_idx].name);
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, fmt::format("{}", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, fmt::format("{}", m_max));
	p->write(TERM_WHITE, "\n");
}

void SkillCondition::to_json(jsoncons::json &j) const
{
	auto const &s_descriptors = game->edit_data.s_descriptors;

	j["name"] = s_descriptors[m_skill_idx].name;
	j["min"] = m_min;
	j["max"] = m_max;
}

bool SymbolCondition::is_match(object_type *o_ptr) const
{
	return o_ptr->k_ptr->d_char == m_symbol;
}

std::shared_ptr<Condition> SymbolCondition::from_json(jsoncons::json const &j)
{
	auto s = get_optional<std::string>(j, "symbol").value_or("");

	if (s.empty())
	{
		msg_print("Missing/invalid 'symbol' property");
		return nullptr;
	}

	if (s.size() > 1)
	{
		msg_print("Invalid 'symbol' property: Too long");
		return nullptr;
	}

	return std::make_shared<SymbolCondition>(s[0]);
}

void SymbolCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "symbol");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, fmt::format("{}", m_symbol));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void SymbolCondition::to_json(jsoncons::json &j) const
{
	j["symbol"] = fmt::format("{}", m_symbol);
}

bool AbilityCondition::is_match(object_type *) const
{
	return p_ptr->has_ability(m_ability_idx);
}

std::shared_ptr<Condition> AbilityCondition::from_json(jsoncons::json const &j)
{
	auto maybe_a = get_optional<std::string>(j, "ability");
	if (!maybe_a)
	{
		msg_print("Missing/invalid 'ability' property");
		return nullptr;
	}
	auto a = *maybe_a;

	auto ai = find_ability(a);
	if (ai < 0)
	{
		msg_format("Invalid 'ability' property: %s", a.c_str());
		return nullptr;
	}

	return std::make_shared<AbilityCondition>(ai);
}

void AbilityCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	auto const &ab_info = game->edit_data.ab_info;

	p->write(ecol, "You have the ");
	p->write(bcol, ab_info[m_ability_idx].name);
	p->write(ecol, " ability");
	p->write(TERM_WHITE, "\n");
}

void AbilityCondition::to_json(jsoncons::json &j) const
{
	auto const &ab_info = game->edit_data.ab_info;

	j["ability"] = ab_info[m_ability_idx].name;
}

void SingleSubconditionCondition::add_child(std::function< std::shared_ptr<Condition> () > const &factory)
{
	// If we already have a subcondition then we cannot
	// add one.
	if (!m_subcondition)
	{
		m_subcondition = factory();
	}
}

void SingleSubconditionCondition::remove_child(Condition *c)
{
	if (m_subcondition.get() == c) {
		m_subcondition.reset();
	}
}

std::shared_ptr<Condition> SingleSubconditionCondition::first_child()
{
	return m_subcondition;
}

void SingleSubconditionCondition::to_json(jsoncons::json &j) const
{
	j["condition"] = optional_to_json(m_subcondition);
}

std::shared_ptr<Condition> SingleSubconditionCondition::parse_single_subcondition(jsoncons::json const &in_json)
{
	auto condition_j = in_json.get_with_default<jsoncons::json>("condition", jsoncons::null_type());

	if (condition_j.is_null())
	{
		return nullptr;
	}
	else if (!condition_j.is_object())
	{
		msg_format("Invalid 'condition' property");
		return nullptr;
	}
	else
	{
		return parse_condition(condition_j);
	}
}

bool NotCondition::is_match(object_type *o_ptr) const
{
	if (!m_subcondition)
	{
		return true;
	}

	return !m_subcondition->is_match(o_ptr);
}

std::shared_ptr<Condition> NotCondition::from_json(jsoncons::json const &j)
{
	return std::make_shared<NotCondition>(parse_single_subcondition(j));
}

void NotCondition::write_tree(TreePrinter *p, Cursor *c, byte ecol, byte bcol) const
{
	p->write(ecol, "Negate the following:");
	p->write(TERM_WHITE, "\n");
	if (m_subcondition)
	{
		m_subcondition->display(p, c);
	}
}

bool InventoryCondition::is_match(object_type *) const
{
	if (!m_subcondition)
	{
		return false;
	}

	for (int i = 0; i < INVEN_WIELD; i++)
	{
		if (m_subcondition->is_match(&p_ptr->inventory[i]))
		{
			return true;
		}
	}
	
	return false;
}

std::shared_ptr<Condition> InventoryCondition::from_json(jsoncons::json const &j)
{
	return std::make_shared<InventoryCondition>(
		parse_single_subcondition(j));
}

void InventoryCondition::write_tree(TreePrinter *p, Cursor *c, byte ecol, byte bcol) const
{
	p->write(ecol, "Something in your ");
	p->write(bcol, "inventory");
	p->write(ecol, " matches the following:");
	p->write(TERM_WHITE, "\n");
	if (m_subcondition)
	{
		m_subcondition->display(p, c);
	}
}

bool EquipmentCondition::is_match(object_type *) const
{
	if (!m_subcondition)
	{
		return false;
	}

	for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		if (m_subcondition->is_match(&p_ptr->inventory[i]))
		{
			return true;
		}
	}
		
	return false;
}

std::shared_ptr<Condition> EquipmentCondition::from_json(jsoncons::json const &j)
{
	return std::make_shared<EquipmentCondition>(
		parse_single_subcondition(j));
}

void EquipmentCondition::write_tree(TreePrinter *p, Cursor *c, byte ecol, byte bcol) const
{
	p->write(ecol, "Something in your ");
	p->write(bcol, "equipment");
	p->write(ecol, " matches the following:");
	p->write(TERM_WHITE, "\n");
	if (m_subcondition)
	{
		m_subcondition->display(p, c);
	}
}

} // namespace
