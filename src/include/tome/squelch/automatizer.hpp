#pragma once

#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>
#include <jansson.h>

#include "tome/squelch/rule_fwd.hpp"
#include "tome/squelch/cursor_fwd.hpp"
#include "tome/squelch/tree_printer_fwd.hpp"
#include "tome/squelch/condition_fwd.hpp"
#include "../object_type_fwd.hpp"

namespace squelch {

/**
 * Automatizer
 */
class Automatizer : public boost::noncopyable
{
public:
	Automatizer(std::shared_ptr<TreePrinter> tree_printer,
		    std::shared_ptr<Cursor> cursor)
		: m_selected_rule(0)
		, m_begin(0)
		, m_tree_printer(tree_printer)
		, m_cursor(cursor)
		, m_rules() {
	}

	/**
	 * Append a rule
	 */
	int append_rule(std::shared_ptr<Rule> rule);

	/**
	 * Swap two rules
	 */
	void swap_rules(int i, int j);

	/**
	 * Apply all rules to the given object
	 */
	bool apply_rules(object_type *o_ptr, int item_idx) const;

	/**
	 * Build a JSON data structure to represent
	 * all the rules.
	 */
        std::shared_ptr<json_t> to_json() const;

	/**
	 * Load rules from a JSON data structure.
	 */
	void load_json(json_t *json);

	/**
	 * Remove currently selected condition or rule.
	 */
	int remove_current_selection();

	/**
	 * Reset view.
	 */
	void reset_view();

	/**
	 * Show current rule
	 */
	void show_current() const;

	/**
	 * Scroll view up
	 */
	void scroll_up();

	/**
	 * Scroll view down
	 */
	void scroll_down();

	/**
	 * Scroll view left
	 */
	void scroll_left();

	/**
	 * Scroll view right
	 */
	void scroll_right();

	/**
	 * Move selection up
	 */
	void move_up();

	/**
	 * Move selection down
	 */
	void move_down();

	/**
	 * Move selection left
	 */
	void move_left();

	/**
	 * Move selection right
	 */
	void move_right();

	/**
	 * Add new condition to selected rule
	 */
	void add_new_condition(std::function<std::shared_ptr<Condition> ()> factory);

	/**
	 * Get rule names. The names are not stable across multiple
	 * calls to methods on this class.
	 */
	void get_rule_names(std::vector<const char *> *names) const;

	/**
	 * Get current number of rules.
	 */
	int rules_count() const;

	/**
	 * Get the "beginning" rule.
	 */
	int rules_begin() const;

	/**
	 * Select a new rule.
	 */
	void select_rule(int selected_rule);

	/**
	 * Return selected rule index
	 */
	int selected_rule() const;

	/**
	 * Return selected rule
	 */
	std::shared_ptr<Rule> current_rule();

private:
	int m_selected_rule;
	int m_begin;
	std::shared_ptr<TreePrinter> m_tree_printer;
	std::shared_ptr<Cursor> m_cursor;
	std::vector < std::shared_ptr < Rule > > m_rules;
};

} // namespace
