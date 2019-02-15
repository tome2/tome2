#include "tome/squelch/automatizer_fwd.hpp"
#include "tome/squelch/automatizer.hpp"

#include "tome/squelch/rule.hpp"
#include "tome/squelch/cursor.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "util.hpp"
#include "z-term.hpp"

namespace squelch {

/**
 * Parse rules from JSON array
 */
static std::vector< std::shared_ptr < Rule > > parse_rules(jsoncons::json const &rules_json)
{
	std::vector< std::shared_ptr < Rule > > rules;

	if (!rules_json.is_array())
	{
		msg_format("Error 'rules' is not an array");
		return rules;
	}

	auto rules_array = rules_json.array_value();

	for (auto const &rule_value : rules_array)
	{
		auto rule = Rule::parse_rule(rule_value);
		if (rule)
		{
			rules.push_back(rule);
		}
	}

	return rules;
}

//----------------------------------------------------------
// Automatizer
//----------------------------------------------------------

int Automatizer::append_rule(std::shared_ptr< Rule > rule)
{
	m_rules.push_back(rule);
	return m_rules.size() - 1;
}

void Automatizer::swap_rules(int i, int j)
{
	swap(m_rules.at(i), m_rules.at(j));
}

bool Automatizer::apply_rules(object_type *o_ptr, int item_idx) const
{
	for (auto rule : m_rules)
	{
		if (rule->apply_rule(o_ptr, item_idx))
		{
			return true;
		}
	}

	return false;
}

jsoncons::json Automatizer::to_json() const
{
	auto document = jsoncons::json::array();

	for (auto rule : m_rules)
	{
		document.push_back(rule->to_json());
	}

	return document;
}

void Automatizer::load_json(jsoncons::json const &document)
{
	// Go through all the found rules
	auto rules = parse_rules(document);

	// Load rule
	for (auto rule: rules)
	{
		append_rule(rule);
	}
}

int Automatizer::remove_current_selection()
{
	assert(!m_rules.empty());

	// Previously selected rule
	int prev_selected_rule = m_selected_rule;
	int new_selected_rule;

	// If the cursor is at the top level then we want to delete
	// the rule itself
	if (m_cursor->size() < 1)
	{
		// Remove rule
		m_rules.erase(m_rules.begin() + m_selected_rule);
		// Select previous
		new_selected_rule = prev_selected_rule - 1;
	}
	else
	{
		// Delete the currently selected condition in rule.
		m_rules.at(m_selected_rule)->delete_selected_condition(m_cursor.get());
		// Keep selection
		new_selected_rule = m_selected_rule;
	}

	// Do we need to adjust to select a different rule?
	if ((prev_selected_rule != new_selected_rule) && (new_selected_rule >= 0))
	{
		select_rule(new_selected_rule);
	}

	// Return the selected rule.
	return m_selected_rule;
}

void Automatizer::reset_view()
{
	// Clear cursor
	m_cursor->clear();

	// Empty rules?
	if (m_rules.empty())
	{
		return;
	}

	// Reset scroll position
	m_tree_printer->reset_scroll();

	// Put the top-level condition into cursor
	auto condition = m_rules.at(m_selected_rule)->get_condition();
	if (condition)
	{
		m_cursor->push(condition.get());
	}
}

void Automatizer::show_current() const
{
	if (m_rules.empty())
	{
		return;
	}

	m_tree_printer->reset();
	m_rules.at(m_selected_rule)->write_tree(
		m_tree_printer.get(),
		m_cursor.get());
}

void Automatizer::scroll_up()
{
	m_tree_printer->scroll_up();
}

void Automatizer::scroll_down()
{
	m_tree_printer->scroll_down();
}

void Automatizer::scroll_left()
{
	m_tree_printer->scroll_left();
}

void Automatizer::scroll_right()
{
	m_tree_printer->scroll_right();
}

void Automatizer::move_up()
{
	m_cursor->move_up();
}

void Automatizer::move_down()
{
	m_cursor->move_down();
}

void Automatizer::move_left()
{
	m_cursor->move_left();
}

void Automatizer::move_right()
{
	m_cursor->move_right();
}

void Automatizer::add_new_condition(std::function<std::shared_ptr<Condition> ()> factory)
{
	m_rules.at(m_selected_rule)->add_new_condition(
		m_cursor.get(),
		factory);
}

std::vector<std::string> Automatizer::get_rule_names() const
{
	std::vector<std::string> names;
	names.reserve(m_rules.size());
	for (auto const &rule: m_rules)
	{
		names.push_back(rule->get_name());
	}
	return names;
}

int Automatizer::rules_count() const
{
	return m_rules.size();
}

int Automatizer::rules_begin() const
{
	return m_begin;
}

void Automatizer::select_rule(int selected_rule)
{
	m_selected_rule = selected_rule;

	int wid, hgt;
	Term_get_size(&wid, &hgt);

	// Adjust selection to conform to bounds.
	{
		int rules_size = m_rules.size(); // Convert to int to avoid warnings

		if (m_selected_rule < 0)
		{
			m_selected_rule = rules_size - 1;
			m_begin = m_selected_rule - hgt + 3;
			if (m_begin < 0)
			{
				m_begin = 0;
			}
		}

		if (m_selected_rule < m_begin)
		{
			m_begin = m_selected_rule;
		}

		if (m_selected_rule >= rules_size)
		{
			m_selected_rule = 0;
			m_begin = 0;
		}

		if (m_selected_rule >= m_begin + hgt - 2)
		{
			m_begin++;
		}
	}

	// Adjust tree printer and cursor.
	reset_view();
}

int Automatizer::selected_rule() const
{
	return m_selected_rule;
}

std::shared_ptr<Rule> Automatizer::current_rule()
{
	return m_rules.at(m_selected_rule);
}

} // namespace
