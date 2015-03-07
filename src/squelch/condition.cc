#include "tome/squelch/condition_fwd.hpp"
#include "tome/squelch/condition.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include "tome/squelch/cursor.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "angband.h"
#include "object1.hpp"
#include "skills.hpp"
#include "quark.h"
#include "util.hpp"
#include "variable.hpp"

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
		{ match_type::STATE, "state" },
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

EnumStringMap<identification_state> &identification_state_mapping()
{
	// TODO: This is quite ugly and leads to valgrind complaints
	static auto m = new EnumStringMap<identification_state> {
		{ identification_state::IDENTIFIED, "identified" },
		{ identification_state::NOT_IDENTIFIED, "not identified" } };
	return *m;
}

json_t *Condition::to_json() const
{
	json_t *j = json_object();
	json_object_set_new(j, "type",
			    json_string(match_mapping().stringify(match)));
	to_json(j);
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

std::shared_ptr<Condition> Condition::parse_condition(json_t *condition_json)
{
	// Parsers for concrete types of conditions.
	static std::map< match_type,
		  std::function< std::shared_ptr< Condition > ( json_t * ) > > parsers {
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
		{ match_type::STATE, &StateCondition::from_json },
		{ match_type::RACE, &RaceCondition::from_json },
		{ match_type::SUBRACE, &SubraceCondition::from_json },
		{ match_type::CLASS, &ClassCondition::from_json },
		{ match_type::LEVEL, &LevelCondition::from_json },
		{ match_type::SKILL, &SkillCondition::from_json },
		{ match_type::ABILITY, &AbilityCondition::from_json } };

	if ((condition_json == nullptr) || json_is_null(condition_json))
	{
		return nullptr;
	}

	cptr type_s = nullptr;
	if (json_unpack(condition_json,
			"{s:s}",
			"type", &type_s) < 0)
	{
		msg_print("Missing/invalid 'type' in condition");
		return nullptr;
	}

	match_type match;
	if (!match_mapping().parse(type_s, &match))
	{
		msg_format("Invalid 'type' in condition: %s", type_s);
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

json_t *Condition::optional_to_json(std::shared_ptr<Condition> condition)
{
	return condition
		? condition->to_json()
		: json_null();
}

bool TvalCondition::is_match(object_type *o_ptr) const
{
	return (o_ptr->tval == m_tval);
}

std::shared_ptr<Condition> TvalCondition::from_json(json_t *j)
{
	int tval;

	if (json_unpack(j, "{s:i}", "tval", &tval) < 0)
	{
		msg_print("Missing/invalid 'tval' property");
		return nullptr;
	}

	return std::make_shared<TvalCondition>(tval);
}

void TvalCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "tval", json_integer(m_tval));
}

void TvalCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "tval");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, format("%d", (int) m_tval));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

bool NameCondition::is_match(object_type *o_ptr) const
{
	char buf1[128];
	object_desc(buf1, o_ptr, -1, 0);

	return boost::algorithm::iequals(m_name, buf1);
}

std::shared_ptr<Condition> NameCondition::from_json(json_t *j)
{
	cptr s = nullptr;
	if (json_unpack(j, "{s:s}", "name", &s) < 0)
	{
		msg_print("Missing/invalid 'name' property");
		return nullptr;
	}
	return std::make_shared<NameCondition>(s);
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

void NameCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "name", json_string(m_name.c_str()));
}

bool ContainCondition::is_match(object_type *o_ptr) const
{
	char buf1[128];
	object_desc(buf1, o_ptr, -1, 0);
	return boost::algorithm::icontains(buf1, m_contain);
}

std::shared_ptr<Condition> ContainCondition::from_json(json_t *j)
{
	cptr s = nullptr;
	if (json_unpack(j, "{s:s}", "contain", &s) < 0)
	{
		msg_print("Missing/invalid 'contain' property");
		return nullptr;
	}
	return std::make_shared<ContainCondition>(s);
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

void ContainCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "contain", json_string(m_contain.c_str()));
}

bool SvalCondition::is_match(object_type *o_ptr) const
{
	return (object_aware_p(o_ptr) &&
		(o_ptr->sval >= m_min) &&
		(o_ptr->sval <= m_max));
}

std::shared_ptr<Condition> SvalCondition::from_json(json_t *j)
{
	int min, max;

	if (json_unpack(j, "{s:i,s:i}",
			"min", &min,
			"max", &max) < 0)
	{
		msg_print("Missing/invalid 'min'/'max' properties");
		return nullptr;
	}

	return std::make_shared<SvalCondition>(min, max);
}

void SvalCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "sval");
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, format("%d", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, format("%d", m_max));
	p->write(TERM_WHITE, "\n");
}

void SvalCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "min", json_integer(m_min));
	json_object_set_new(j, "max", json_integer(m_max));
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

std::vector< std::shared_ptr<Condition> > GroupingCondition::parse_conditions(json_t *j)
{
	json_t *conditions_j = json_object_get(j, "conditions");

	if ((conditions_j == nullptr) ||
	    (json_is_null(conditions_j)))
	{
		return std::vector< std::shared_ptr<Condition> >();
	}
	else if (!json_is_array(conditions_j))
	{
		msg_print("'conditions' property has invalid type");
		return std::vector< std::shared_ptr<Condition> >();
	}
	else
	{
		std::vector< std::shared_ptr<Condition> > subconditions;
		for (size_t i = 0; i < json_array_size(conditions_j); i++)
		{
			json_t *subcondition_j =
				json_array_get(conditions_j, i);

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

void GroupingCondition::to_json(json_t *j) const
{
	json_t *ja = json_array();
	for (auto condition_p : m_conditions)
	{
		json_array_append_new(ja, optional_to_json(condition_p));
	}
	json_object_set_new(j, "conditions", ja);
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

std::shared_ptr<Condition> AndCondition::from_json(json_t *j)
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

std::shared_ptr<Condition> OrCondition::from_json(json_t *j)
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

std::shared_ptr<Condition> StatusCondition::from_json(json_t *j)
{
	cptr s;
	if (json_unpack(j, "{s:s}", "status", &s) < 0)
	{
		msg_print("Missing/invalid 'status' property");
		return nullptr;
	}

	status_type status;
	if (!status_mapping().parse(s, &status))
	{
		msg_format("Invalid 'status' property: %s", s);
		return nullptr;
	}

	return std::make_shared<StatusCondition>(status);
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

void StatusCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "status", json_string(status_mapping().stringify(m_status)));
}

bool RaceCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_race, rp_ptr->title);
}

std::shared_ptr<Condition> RaceCondition::from_json(json_t *j)
{
	cptr s;

	if (json_unpack(j, "{s:s}", "race", &s) < 0)
	{
		msg_print("Missing/invalid 'race' property");
		return nullptr;
	}

	return std::make_shared<RaceCondition>(s);
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

void RaceCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "race", json_string(m_race.c_str()));
}

bool SubraceCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_subrace, rmp_ptr->title);
}

std::shared_ptr<Condition> SubraceCondition::from_json(json_t *j)
{
	cptr s;

	if (json_unpack(j, "{s:s}", "subrace", &s) < 0)
	{
		msg_print("Missing/invalid 'subrace' property");
		return nullptr;
	}

	return std::make_shared<SubraceCondition>(s);
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

void SubraceCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "subrace", json_string(m_subrace.c_str()));
}

bool ClassCondition::is_match(object_type *o_ptr) const
{
	return boost::algorithm::iequals(m_class, spp_ptr->title);
}

std::shared_ptr<Condition> ClassCondition::from_json(json_t *j)
{
	cptr s;

	if (json_unpack(j, "{s:s}", "class", &s) < 0)
	{
		msg_print("Missing/invalid 'class' property");
		return nullptr;
	}

	return std::make_shared<ClassCondition>(s);
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

void ClassCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "class", json_string(m_class.c_str()));
}

bool InscriptionCondition::is_match(object_type *o_ptr) const
{
	if (o_ptr->note == 0)
	{
		return false;
	}
	return boost::algorithm::icontains(
		quark_str(o_ptr->note),
		m_inscription);
}

std::shared_ptr<Condition> InscriptionCondition::from_json(json_t *j)
{
	cptr s = nullptr;
	if (json_unpack(j, "{s:s}", "inscription", &s) < 0)
	{
		msg_print("Missing/invalid 'inscription' property");
		return nullptr;
	}
	return std::make_shared<InscriptionCondition>(s);
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

void InscriptionCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "inscription", json_string(m_inscription.c_str()));
}

bool DiscountCondition::is_match(object_type *o_ptr) const
{
	return (object_aware_p(o_ptr) &&
		(o_ptr->discount >= m_min) &&
		(o_ptr->discount <= m_max));
}

std::shared_ptr<Condition> DiscountCondition::from_json(json_t *j)
{
	int min, max;

	if (json_unpack(j, "{s:i,s:i}",
			"min", &min,
			"max", &max) < 0)
	{
		msg_print("Missing/invalid 'min'/'max' properties");
		return nullptr;
	}

	return std::make_shared<DiscountCondition>(min, max);
}

void DiscountCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "discount");
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, format("%d", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, format("%d", m_max));
	p->write(TERM_WHITE, "\n");
}

void DiscountCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "min", json_integer(m_min));
	json_object_set_new(j, "max", json_integer(m_max));
}

bool LevelCondition::is_match(object_type *) const
{
	return ((p_ptr->lev >= m_min) &&
		(p_ptr->lev <= m_max));
}

std::shared_ptr<Condition> LevelCondition::from_json(json_t *j)
{
	int min, max;
	if (json_unpack(j, "{s:i,s:i}",
			"min", &min,
			"max", &max) < 0)
	{
		msg_print("Missing/invalid 'min'/'max' properties");
		return nullptr;
	}

	return std::make_shared<LevelCondition>(min, max);
}

void LevelCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Your ");
	p->write(bcol, "level");
	p->write(ecol, " is from ");

	p->write(TERM_WHITE, format("%d", m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, format("%d", m_max));
	p->write(TERM_WHITE, "\n");
}

void LevelCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "min", json_integer(m_min));
	json_object_set_new(j, "max", json_integer(m_max));
}

bool SkillCondition::is_match(object_type *) const
{
	uint16_t sk = get_skill(m_skill_idx);
	return ((sk >= m_min) &&
		(sk <= m_max));
}

std::shared_ptr<Condition> SkillCondition::from_json(json_t *j)
{
	cptr s;
	int min, max;
	if (json_unpack(j, "{s:i,s:i,s:s}",
			"min", &min,
			"max", &max,
			"name", &s) < 0)
	{
		msg_print("Missing/invalid 'min'/'max'/'name' properties");
		return nullptr;
	}

	uint16_t si = find_skill_i(s);
	if (si < 0)
	{
		msg_print("Invalid 'name' property");
		return nullptr;
	}

	return std::make_shared<SkillCondition>(si, min, max);
}

void SkillCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Your skill in ");
	p->write(bcol, s_info[m_skill_idx].name);
	p->write(ecol, " is from ");
	p->write(TERM_WHITE, format("%d", (int) m_min));
	p->write(ecol, " to ");
	p->write(TERM_WHITE, format("%d", (int) m_max));
	p->write(TERM_WHITE, "\n");
}

void SkillCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "name",
			    json_string(s_info[m_skill_idx].name));
	json_object_set_new(j, "min",
			    json_integer(m_min));
	json_object_set_new(j, "max",
			    json_integer(m_max));
}

bool StateCondition::is_match(object_type *o_ptr) const
{
	switch (m_state)
	{
	case identification_state::IDENTIFIED:
		return object_known_p(o_ptr);
	case identification_state::NOT_IDENTIFIED:
		return !object_known_p(o_ptr);
	}

	assert(false);
	return false;
}

std::shared_ptr<Condition> StateCondition::from_json(json_t *j)
{
	cptr s;
	if (json_unpack(j, "{s:s}", "state", &s) < 0)
	{
		msg_print("Missing/invalid 'state' property");
		return nullptr;
	}

	identification_state state;
	if (!identification_state_mapping().parse(s, &state))
	{
		msg_format("Invalid 'state' property: %s", s);
		return nullptr;
	}

	return std::make_shared<StateCondition>(state);
}

void StateCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	p->write(ecol, "Its ");
	p->write(bcol, "state");
	p->write(ecol, " is ");
	p->write(ecol, "\"");
	p->write(TERM_WHITE, identification_state_mapping().stringify(m_state));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void StateCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "state",
			    json_string(identification_state_mapping().
					stringify(m_state)));
}

bool SymbolCondition::is_match(object_type *o_ptr) const
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	return k_ptr->d_char == m_symbol;
}

std::shared_ptr<Condition> SymbolCondition::from_json(json_t *j)
{
	cptr s_ = nullptr;
	if (json_unpack(j, "{s:s}", "symbol", &s_) < 0)
	{
		msg_print("Missing/invalid 'symbol' property");
		return nullptr;
	}

	std::string s(s_);
	if (s.empty())
	{
		msg_print("Invalid 'symbol' property: Too short");
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
	p->write(TERM_WHITE, format("%c", m_symbol));
	p->write(ecol, "\"");
	p->write(TERM_WHITE, "\n");
}

void SymbolCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "symbol",
			    json_string(format("%c", m_symbol)));
}

bool AbilityCondition::is_match(object_type *) const
{
	return has_ability(m_ability_idx);
}

std::shared_ptr<Condition> AbilityCondition::from_json(json_t *j)
{
	cptr a;
	if (json_unpack(j, "{s:s}", "ability", &a) < 0)
	{
		msg_print("Missing/invalid 'ability' property");
		return nullptr;
	}

	uint16_t ai = find_ability(a);
	if (ai < 0)
	{
		msg_print("Invalid 'ability' property");
		return nullptr;
	}

	return std::make_shared<AbilityCondition>(ai);
}

void AbilityCondition::write_tree(TreePrinter *p, Cursor *, uint8_t ecol, uint8_t bcol) const
{
	cptr ability_s = ab_info[m_ability_idx].name;

	p->write(ecol, "You have the ");
	p->write(bcol, ability_s);
	p->write(ecol, " ability");
	p->write(TERM_WHITE, "\n");
}

void AbilityCondition::to_json(json_t *j) const
{
	cptr ability_s = ab_info[m_ability_idx].name;
	json_object_set_new(j, "ability", json_string(ability_s));
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

void SingleSubconditionCondition::to_json(json_t *j) const
{
	json_object_set_new(j, "condition",
			    optional_to_json(m_subcondition));
}

std::shared_ptr<Condition> SingleSubconditionCondition::parse_single_subcondition(json_t *in_json)
{
	json_t *condition_j =
		json_object_get(in_json, "condition");

	if ((condition_j == nullptr) ||
	    (json_is_null(condition_j)))
	{
		return nullptr;
	}
	else if (!json_is_object(condition_j))
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

std::shared_ptr<Condition> NotCondition::from_json(json_t *j)
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

std::shared_ptr<Condition> InventoryCondition::from_json(json_t *j)
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

std::shared_ptr<Condition> EquipmentCondition::from_json(json_t *j)
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
