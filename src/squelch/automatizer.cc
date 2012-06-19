#include "tome/squelch/automatizer_fwd.hpp"
#include "tome/squelch/automatizer.hpp"

#include "tome/squelch/rule.hpp"
#include "tome/squelch/cursor.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "angband.h"

namespace squelch {

/**
 * Parse rules from JSON array
 */
static std::vector< std::shared_ptr < Rule > > parse_rules(json_t *rules_j)
{
	std::vector< std::shared_ptr < Rule > > rules;

	if (!json_is_array(rules_j))
	{
		msg_format("Error 'rules' is not an array");
		return rules;
	}

	for (size_t i = 0; i < json_array_size(rules_j); i++)
	{
		json_t *rule_j = json_array_get(rules_j, i);
		auto rule = Rule::parse_rule(rule_j);
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

	return true;
}

std::shared_ptr<json_t> Automatizer::to_json() const
{
	auto rules_json = std::shared_ptr<json_t>(json_array(), &json_decref);

	for (auto rule : m_rules)
	{
		json_array_append_new(rules_json.get(), rule->to_json());
	}

	return rules_json;
}

void Automatizer::load_json(json_t *json)
{
	// Go through all the found rules
	auto rules = parse_rules(json);

	// Load rule
	for (auto rule : rules)
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

void Automatizer::get_rule_names(std::vector<const char *> *names) const
{
	names->resize(m_rules.size());
	for (size_t i = 0; i < m_rules.size(); i++)
	{
		(*names)[i] = m_rules.at(i)->get_name();
	}
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
		if (m_selected_rule < 0)
		{
			m_selected_rule = m_rules.size() - 1;
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

		if (m_selected_rule >= m_rules.size())
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
