#pragma once

#include <memory>
#include <jsoncons/json.hpp>

#include "tome/squelch/condition_fwd.hpp"
#include "tome/squelch/cursor_fwd.hpp"
#include "tome/squelch/tree_printer_fwd.hpp"
#include "tome/enum_string_map.hpp"
#include "../object_type_fwd.hpp"

namespace squelch {

/**
 * Types of automatizer actions: destroy, pick up, and inscribe.
 */
enum class action_type { AUTO_DESTROY, AUTO_PICKUP, AUTO_INSCRIBE };

/**
 * Bidirectional map between rule actions and strings.
 */
EnumStringMap<action_type> &action_mapping();

/**
 * Rules are the representation of "when condition X is true, do Y".
 */
class Rule : public boost::noncopyable
{
public:
	Rule(std::string name,
	     action_type action,
	     int module_idx,
	     std::shared_ptr<Condition> condition)
		: m_name(name)
		, m_action(action)
		, m_module_idx(module_idx)
		, m_condition(condition) {
	}

	/**
	 * Set the name of the rule
	 */
	void set_name(const std::string &new_name);

	/**
	 * Get the name of the rule
	 */
	std::string get_name() const;

	/**
	 * Get condition
	 */
	std::shared_ptr<Condition> get_condition() const;

	/**
	 * Add new condition using a factory to instantiate the
	 * condition only if the rule permits adding a condition.
	 */
	void add_new_condition(Cursor *cursor,
			       ConditionFactory const &factory);

	/**
	 * Remove currently selected condition
	 */
	void delete_selected_condition(Cursor *cursor);

	/**
	 * Write out tree representing rule
	 */
	void write_tree(TreePrinter *p, Cursor *cursor) const;

	/**
	 * Apply rule to object
	 */
	bool apply_rule(object_type *o_ptr, int item_idx) const;

	/**
	 * Convert rule to JSON.
	 */
	virtual jsoncons::json to_json() const;

	/**
	 * Parse rule from JSON
	 */
	static std::shared_ptr<Rule> parse_rule(jsoncons::json const &);

protected:
	virtual bool do_apply_rule(object_type *, int) const = 0;
	virtual void do_write_tree(TreePrinter *p) const = 0;

protected:
	// Rule name
	std::string m_name;
	// What does the rule do?
	action_type m_action;
	// Which module does this rule apply to?
	int m_module_idx;
	// Condition to check
	std::shared_ptr<Condition> m_condition;
};

/**
 * Rule for destroying matching objects
 */
class DestroyRule : public Rule
{
public:
	DestroyRule(std::string name,
		    int module_idx,
		    std::shared_ptr<Condition> condition)
		: Rule(name, action_type::AUTO_DESTROY, module_idx, condition) {
	}

protected:
	virtual bool do_apply_rule(object_type *o_ptr, int item_idx) const override;
	virtual void do_write_tree(TreePrinter *p) const override;
};

/**
 * Rule for picking up matching objects
 */
class PickUpRule : public Rule
{
public:

	PickUpRule(std::string name,
		   int module_idx,
		   std::shared_ptr<Condition> condition)
		: Rule(name, action_type::AUTO_PICKUP, module_idx, condition) {
	}

protected:
	virtual void do_write_tree(TreePrinter *p) const override;
	virtual bool do_apply_rule(object_type *o_ptr, int item_idx) const override;
};

/**
 * Rule for inscribing matching objects
 */
class InscribeRule : public Rule
{
public:
	InscribeRule(std::string name,
		     int module_idx,
		     std::shared_ptr<Condition> condition,
		     std::string inscription)
		: Rule(name, action_type::AUTO_INSCRIBE, module_idx, condition)
		, m_inscription(inscription) {
	}

	jsoncons::json to_json() const override;

protected:
        virtual void do_write_tree(TreePrinter *p) const override;
        virtual bool do_apply_rule(object_type *o_ptr, int) const override;

private:
	// Inscription to use for inscription rules.
	std::string m_inscription;
};

} // namespace
