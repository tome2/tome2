#include "tome/squelch/rule_fwd.hpp"
#include "tome/squelch/rule.hpp"

#include "tome/squelch/cursor.hpp"
#include "tome/squelch/condition.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "../angband.h"
#include "../modules.hpp"
#include "../object1.hpp"
#include "../object2.hpp"
#include "../object_flag.hpp"
#include "../object_type.hpp"
#include "../quark.hpp"
#include "../tables.hpp"
#include "../util.hpp"
#include "../variable.hpp"

namespace squelch {

EnumStringMap<action_type> &action_mapping()
{
	static auto m = new EnumStringMap<action_type> {
		{ action_type::AUTO_DESTROY, "destroy" },
		{ action_type::AUTO_PICKUP, "pickup" },
		{ action_type::AUTO_INSCRIBE, "inscribe" } };
	return *m;
}

void Rule::set_name(const char *new_name)
{
	assert(new_name != nullptr);
	m_name = new_name;
}

const char *Rule::get_name() const
{
	return m_name.c_str();
}

std::shared_ptr<Condition> Rule::get_condition() const
{
	return m_condition;
}

json_t *Rule::to_json() const
{
	json_t *rule_json = json_object();
	json_object_set_new(rule_json,
			    "name",
			    json_string(m_name.c_str()));
	json_object_set_new(rule_json,
			    "action",
			    json_string(action_mapping().stringify(m_action)));
	json_object_set_new(rule_json,
			    "module",
			    json_string(modules[m_module_idx].meta.name));
	json_object_set_new(rule_json,
			    "condition",
			    Condition::optional_to_json(m_condition));
	return rule_json;
}

void Rule::add_new_condition(Cursor *cursor,
			     ConditionFactory const &factory)
{
	// Top-level condition?
	if (!m_condition)
	{
		// Sanity check for navigation stack
		assert(cursor->empty());

		// Create new top-level condition
		m_condition = factory();

		// Select the condition
		if (m_condition)
		{
			cursor->push(m_condition.get());
		}
	}
	else
	{
		cursor->current()->add_child(factory);
	}
}

void Rule::delete_selected_condition(Cursor *cursor)
{
	assert(cursor->size() >= 1);

	if (cursor->size() == 1)
	{
		cursor->pop();
		m_condition.reset();
	}
	else
	{
		Condition *prev_top = cursor->pop();
		Condition *top = cursor->current();

		// Jump up a level; this is a simple way to ensure a
		// valid cursor. We could be a little cleverer here by
		// trying to move inside the current level, but it
		// gets a little complicated.
		cursor->move_left();

		// Now we can remove the condition from its parent
		top->remove_child(prev_top);
	}
}

void Rule::write_tree(TreePrinter *tree_printer, Cursor *cursor) const
{
	// Write out the main rule
	do_write_tree(tree_printer);

	// Write out the condition
	if (m_condition)
	{
		m_condition->display(tree_printer, cursor);
	}
}

bool Rule::apply_rule(object_type *o_ptr, int item_idx) const
{
	// Check module
	if (m_module_idx != game_module_idx)
	{
		return false;
	}

	// Check condition
	if (m_condition && m_condition->is_match(o_ptr))
	{
		return do_apply_rule(o_ptr, item_idx);
	}

	// Doesn't apply
	return false;
}

std::shared_ptr<Rule> Rule::parse_rule(json_t *rule_json)
{
	if (!json_is_object(rule_json))
	{
		msg_print("Rule is not an object");
		return nullptr;
	}

	// Retrieve the attributes
	char *rule_name_s = nullptr;
	char *rule_action_s = nullptr;
	char *rule_module_s = nullptr;
	if (json_unpack(rule_json,
			"{s:s,s:s,s:s}",
			"name", &rule_name_s,
			"action", &rule_action_s,
			"module", &rule_module_s) < 0)
	{
		msg_print("Rule missing required field(s)");
		return nullptr;
	}

	// Convert attributes
	action_type action;
	if (!action_mapping().parse((cptr) rule_action_s, &action))
	{
		msg_format("Invalid rule action '%s'", rule_action_s);
		return nullptr;
	}

	int module_idx = find_module((cptr) rule_module_s);
	if (module_idx < 0)
	{
		msg_format("Skipping rule for unrecognized module '%s'",
			   (cptr) rule_module_s);
		return nullptr;
	}

	// Parse condition
	std::shared_ptr<Condition> condition =
		Condition::parse_condition(json_object_get(rule_json, "condition"));

	// Parse rule
	switch (action)
	{
	case action_type::AUTO_INSCRIBE:
	{
		json_t *rule_inscription_j = json_object_get(rule_json, "inscription");

		if (rule_inscription_j == nullptr)
		{
			msg_print("Inscription rule missing 'inscription' attribute");
			return nullptr;
		}
		if (!json_is_string(rule_inscription_j))
		{
			msg_print("Inscription rule 'inscription' attribute wrong type");
			return nullptr;
		}

		std::string inscription =
			json_string_value(rule_inscription_j);
		return std::make_shared<InscribeRule>(
			rule_name_s, module_idx, condition, inscription);
	}

	case action_type::AUTO_PICKUP:
		return std::make_shared<PickUpRule>(
			rule_name_s, module_idx, condition);

	case action_type::AUTO_DESTROY:
		return std::make_shared<DestroyRule>(
			rule_name_s, module_idx, condition);
	}

	assert(false);
	return nullptr;
}


void DestroyRule::do_write_tree(TreePrinter *p) const
{
	p->write(TERM_GREEN, "A rule named \"");
	p->write(TERM_WHITE, m_name.c_str());
	p->write(TERM_GREEN, "\" to ");
	p->write(TERM_L_GREEN, "destroy");
	p->write(TERM_GREEN, " when");
	p->write(TERM_WHITE, "\n");
}

bool DestroyRule::do_apply_rule(object_type *o_ptr, int item_idx) const
{
	// Must be identified
	if (object_aware_p(o_ptr) == FALSE)
	{
		return false;
	}

	// Never destroy inscribed items
	if (!o_ptr->inscription.empty())
	{
		return false;
	}

	// Ignore artifacts; cannot be destroyed anyway
	if (artifact_p(o_ptr))
	{
		return false;
	}
	
	// Cannot destroy CURSE_NO_DROP objects.
	{
		auto const f = object_flags(o_ptr);
		if (f & TR_CURSE_NO_DROP)
		{
			return false;
		}
	}
	
	// Destroy
	msg_print("<Auto-destroy>");
	inc_stack_size(item_idx, -o_ptr->number);
	return true;
}

void PickUpRule::do_write_tree(TreePrinter *p) const
{
	p->write(TERM_GREEN, "A rule named \"");
	p->write(TERM_WHITE, m_name.c_str());
	p->write(TERM_GREEN, "\" to ");
	p->write(TERM_L_GREEN, "pick up");
	p->write(TERM_GREEN, " when");
	p->write(TERM_WHITE, "\n");
}

bool PickUpRule::do_apply_rule(object_type *o_ptr, int item_idx) const
{
	if (item_idx >= 0)
	{
		return false;
	}
	
	if (!inven_carry_okay(o_ptr))
	{
		return false;
	}
	
	msg_print("<Auto-pickup>");
	object_pickup(-item_idx);
	return true;
}

json_t *InscribeRule::to_json() const
{
	json_t *j = Rule::to_json();

	json_object_set_new(j,
			    "inscription",
			    json_string(m_inscription.c_str()));
	
	return j;
}

void InscribeRule::do_write_tree(TreePrinter *p) const
{
	p->write(TERM_GREEN, "A rule named \"");
	p->write(TERM_WHITE, m_name.c_str());
	p->write(TERM_GREEN, "\" to ");
	p->write(TERM_L_GREEN, "inscribe");
	p->write(TERM_GREEN, " an item with \"");
	p->write(TERM_WHITE, m_inscription.c_str());
	p->write(TERM_GREEN, "\" when");
	p->write(TERM_WHITE, "\n");
}

bool InscribeRule::do_apply_rule(object_type *o_ptr, int) const
{
	// Already inscribed?
	if (!o_ptr->inscription.empty())
	{
		return false;
	}
	
	// Inscribe
	msg_format("<Auto-Inscribe {%s}>", m_inscription.c_str());
	o_ptr->inscription = m_inscription;
	return true;
}

} // namespace
