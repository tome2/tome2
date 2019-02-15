#pragma once

#include "tome/squelch/condition_fwd.hpp"

#include <boost/noncopyable.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <jsoncons/json.hpp>

#include "tome/squelch/cursor_fwd.hpp"
#include "tome/squelch/tree_printer_fwd.hpp"
#include "tome/squelch/object_status_fwd.hpp"
#include "tome/enum_string_map.hpp"
#include "../object_type_fwd.hpp"

namespace squelch {

/**
 * Types of matches used for conditions.
 */
enum class match_type {
	AND      , OR      , NOT      , NAME      , CONTAIN,
	INSCRIBED, DISCOUNT, SYMBOL   , STATUS    , TVAL   ,
	SVAL     , RACE    , SUBRACE  , CLASS     , LEVEL  ,
	SKILL    , ABILITY , INVENTORY, EQUIPMENT };

/**
 * Bidirectional map between enumeration values and strings.
 */
EnumStringMap<match_type> &match_mapping();

/**
 * Condition represents a tree of checks which
 * can be applied to objects, the player, etc.
 */
class Condition : public boost::noncopyable
{
public:
	Condition(match_type match_) : match(match_) {
	}

	void display(TreePrinter *, Cursor *) const;

	virtual bool is_match(object_type *) const = 0;

        virtual ~Condition() {
	}

public:
	jsoncons::json to_json() const;

	virtual void add_child(ConditionFactory const &factory) {
		// Default is to not support children.
	};

	virtual void remove_child(Condition *c) {
		// Nothing to do by default.
	}

	virtual std::shared_ptr<Condition> first_child() {
		// No children.
		return nullptr;
	}

	virtual std::shared_ptr<Condition> previous_child(Condition *) {
		// Default no children, so no predecessor.
		return nullptr;
	}

	virtual std::shared_ptr<Condition> next_child(Condition *) {
		// Default no children, so no successor.
		return nullptr;
	}

	/**
	 * Parse condition from JSON
	 */
	static std::shared_ptr<Condition> parse_condition(jsoncons::json const &);

	/**
	 * Convert an (optional) condition to JSON.
	 */
	static jsoncons::json optional_to_json(std::shared_ptr<Condition> condition);

protected:
	virtual void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const = 0;
	virtual void to_json(jsoncons::json &) const = 0;

	// What do we want to match?
	match_type match;
};

/**
 * Check for a specific TVAL
 */
class TvalCondition : public Condition
{
public:
	TvalCondition(uint8_t tval)
		: Condition(match_type::TVAL)
		, m_tval(tval) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	uint8_t m_tval;
};

/**
 * Check for object name
 */
class NameCondition : public Condition
{
public:
	NameCondition(std::string name) :
		Condition(match_type::NAME),
		m_name(name) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_name;
};

/**
 * Check for infix of object name
 */
class ContainCondition : public Condition
{
public:
	ContainCondition(std::string contain) :
		Condition(match_type::CONTAIN),
		m_contain(contain) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_contain;
};

/**
 * Check for specific SVAL
 */
class SvalCondition : public Condition
{
public:
	SvalCondition(uint8_t min, uint8_t max)
		: Condition(match_type::SVAL)
		, m_min(min)
		, m_max(max) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	uint8_t m_min;
	uint8_t m_max;
};

/**
 * Groupings of subconditions
 */
class GroupingCondition : public Condition
{
public:
	GroupingCondition(match_type match,
			  std::vector< std::shared_ptr<Condition> > conditions = std::vector< std::shared_ptr<Condition> >())
		: Condition(match)
		, m_conditions(conditions) {
	}

	virtual void add_condition(std::shared_ptr<Condition> condition) {
		if (condition)
		{
			m_conditions.push_back(condition);
		}
	}

	// Child manipulation
	virtual void add_child(ConditionFactory const &factory) override;
	virtual void remove_child(Condition *condition) override;
	virtual std::shared_ptr<Condition> first_child() override;
	virtual std::shared_ptr<Condition> previous_child(Condition *) override;
	virtual std::shared_ptr<Condition> next_child(Condition *current) override;

	// Parse a list of conditions from JSON property
	static std::vector< std::shared_ptr<Condition> > parse_conditions(jsoncons::json const &);

protected:
	void to_json(jsoncons::json &) const override;

protected:
	std::vector< std::shared_ptr<Condition> > m_conditions;
};

/**
 * Conditions that are AND'ed together
 */
class AndCondition : public GroupingCondition
{
public:
	AndCondition() : GroupingCondition(match_type::AND) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;
};

/**
 * Conditions that are OR'ed together
 */
class OrCondition : public GroupingCondition
{
public:
	OrCondition() : GroupingCondition(match_type::OR) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;
};

/**
 * Check for object status
 */
class StatusCondition : public Condition
{
public:
	StatusCondition(status_type status)
		: Condition(match_type::STATUS)
		, m_status(status) {
	}

public:
	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	status_type m_status;
};

/**
 * Check for player race
 */
class RaceCondition : public Condition
{
public:
	RaceCondition(std::string race)
		: Condition(match_type::RACE)
		, m_race(race) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_race;
};

/**
 * Check player subrace
 */
class SubraceCondition : public Condition
{
public:
	SubraceCondition(std::string subrace)
		: Condition(match_type::SUBRACE)
		, m_subrace(subrace) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_subrace;
};

/**
 * Check player class
 */
class ClassCondition : public Condition
{
public:
	ClassCondition(std::string klass)
		: Condition(match_type::CLASS)
		, m_class(klass) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_class;
};

/**
 * Check object inscription
 */
class InscriptionCondition : public Condition
{
public:
	InscriptionCondition(std::string inscription)
		: Condition(match_type::INSCRIBED)
		, m_inscription(inscription) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	std::string m_inscription;
};

/**
 * Check object discount
 */
class DiscountCondition : public Condition
{
public:
	DiscountCondition(int min, int max)
		: Condition(match_type::DISCOUNT)
		, m_min(min)
		, m_max(max) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	int m_min;
	int m_max;
};

/**
 * Check player level
 */
class LevelCondition : public Condition
{
public:
	LevelCondition(int min, int max)
		: Condition(match_type::LEVEL)
		, m_min(min)
		, m_max(max) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	int m_min;
	int m_max;
};

/**
 * Check player's skill level
 */
class SkillCondition : public Condition
{
public:
	SkillCondition(uint16_t skill_idx, uint16_t min, uint16_t max)
		: Condition(match_type::SKILL)
		, m_skill_idx(skill_idx)
		, m_min(min)
		, m_max(max) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	uint16_t m_skill_idx;
	uint16_t m_min;
	uint16_t m_max;
};

/**
 * Check object symbol
 */
class SymbolCondition : public Condition
{
public:
	SymbolCondition(char symbol)
		: Condition(match_type::SYMBOL)
		, m_symbol(symbol) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	char m_symbol;
};

/**
 * Check if player has a particular ability
 */
class AbilityCondition : public Condition
{
public:
	AbilityCondition(uint16_t ability_idx)
		: Condition(match_type::ABILITY)
		, m_ability_idx(ability_idx) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;

	void to_json(jsoncons::json &) const override;

private:
	uint16_t m_ability_idx;
};

/**
 * Condition with a single subcondition
 */
class SingleSubconditionCondition : public Condition
{
public:
	SingleSubconditionCondition(match_type match,
				    std::shared_ptr<Condition> subcondition)
		: Condition(match)
		, m_subcondition(subcondition) {
	}

	virtual void add_child(std::function< std::shared_ptr<Condition> () > const &factory) override;

	virtual void remove_child(Condition *c) override;

	virtual std::shared_ptr<Condition> first_child() override;

protected:
	void to_json(jsoncons::json &) const override;

	static std::shared_ptr<Condition> parse_single_subcondition(
		jsoncons::json const &condition_json);

protected:
	std::shared_ptr<Condition> m_subcondition;
};

/**
 * Condition which negates another condition
 */
class NotCondition : public SingleSubconditionCondition
{
public:
	NotCondition(std::shared_ptr<Condition> subcondition = nullptr)
		: SingleSubconditionCondition(match_type::NOT, subcondition) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;
};

/**
 * Condition which checks if player inventory contains object(s)
 * satisfying another condition.
 */
class InventoryCondition : public SingleSubconditionCondition
{
public:
	InventoryCondition(std::shared_ptr<Condition> subcondition = nullptr)
		: SingleSubconditionCondition(match_type::INVENTORY, subcondition) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:

	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;
};

/**
 * Condition which checks if player equipment contains object(s)
 * satisfying another condition.
 */
class EquipmentCondition : public SingleSubconditionCondition
{
public:
	EquipmentCondition(std::shared_ptr<Condition> subcondition = nullptr)
		: SingleSubconditionCondition(match_type::EQUIPMENT, subcondition) {
	}

	bool is_match(object_type *) const override;

	static std::shared_ptr<Condition> from_json(jsoncons::json const &);

protected:
	void write_tree(TreePrinter *, Cursor *, uint8_t, uint8_t) const override;
};

} // namespace
