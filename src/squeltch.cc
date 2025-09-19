/*
 * Copyright (c) 2002 DarkGod
 * Copyright (c) 2012 Bardur Arantsson
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "squeltch.hpp"

#include "cave_type.hpp"
#include "files.hpp"
#include "game.hpp"
#include "loadsave.hpp"
#include "lua_bind.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "player_type.hpp"
#include "tome/squelch/tree_printer.hpp"
#include "tome/squelch/condition.hpp"
#include "tome/squelch/condition_metadata.hpp"
#include "tome/squelch/rule.hpp"
#include "tome/squelch/cursor.hpp"
#include "tome/squelch/object_status.hpp"
#include "tome/squelch/automatizer.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-form.hpp"

#include <algorithm>
#include <deque>
#include <fmt/format.h>
#include <list>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using squelch::action_type;
using squelch::action_mapping;

using squelch::status_type;

using squelch::Rule;
using squelch::DestroyRule;
using squelch::PickUpRule;
using squelch::InscribeRule;

using squelch::Condition;
using squelch::AndCondition;
using squelch::TvalCondition;
using squelch::SvalCondition;
using squelch::NameCondition;
using squelch::StatusCondition;

static squelch::Automatizer *automatizer = nullptr;

bool automatizer_create = false;

void squeltch_grid()
{
	if (!automatizer_enabled)
	{
		return;
	}

	// Copy list of objects since we may modify it
	auto const object_idxs(cave[p_ptr->py][p_ptr->px].o_idxs);

	// Scan the pile of objects
	for (auto const this_o_idx: object_idxs)
	{
		// Acquire object
		object_type * o_ptr = &o_list[this_o_idx];

		// We've now seen one of these
		if (!o_ptr->k_ptr->flavor)
		{
			object_aware(o_ptr);
		}

		// Apply rules
		automatizer->apply_rules(o_ptr, -this_o_idx);
	}
}

void squeltch_inventory()
{
	if (!automatizer_enabled)
	{
		return;
	}

	bool changed = true;
	for (int num_iter = 0; changed && (num_iter < 100); num_iter++)
	{
		// No changes on this iteration.
		changed = false;
		// Traverse inventory
		for (int i = 0; i < INVEN_PACK; i++)
		{
			object_type *o_ptr = &p_ptr->inventory[i];
			if (o_ptr->k_ptr && automatizer->apply_rules(o_ptr, i))
			{
				// We have changes
				changed = true;
				// Re-traverse inventory
				break;
			}
		}
	}
	// If we reached the iteration limit, "changed" will be true
	if (changed)
	{
		cmsg_format(TERM_VIOLET, "'apply_rules' ran too often.");
	}
}

static int create_new_rule()
{
	std::string name = "No name";

	if (!input_box_auto("Name?", &name, 20))
	{
		return -1;
	}

	char typ = msg_box_auto("[D]estroy, [P]ickup, [I]nscribe?");

	std::shared_ptr<Rule> rule;
	switch (typ)
	{
	case 'd':
	case 'D':
		rule = std::make_shared<DestroyRule>(name, game_module_idx, nullptr);
		break;

	case 'p':
	case 'P':
		rule = std::make_shared<PickUpRule>(name, game_module_idx, nullptr);
		break;

	case 'i':
	case 'I':
	{
		auto s = input_box_auto("Inscription?", 79);
		if (s.empty())
		{
			return -1;
		}

		rule = std::make_shared<InscribeRule>(name, game_module_idx, nullptr, s);
		break;
	}

	default:
		return -1;
	}

	return automatizer->append_rule(rule);
}

static void automatizer_save_rules()
{
	char buf[1025];
	char ch;
	int hgt, wid;

	Term_get_size(&wid, &hgt);

	std::string name = fmt::format("{}.atm", game->player_name);

	if (!input_box_auto("Save name?", &name, 30))
	{
		return;
	}

	// Function for showing a message
	auto show_message = [hgt, wid](std::string text) {
		auto n = std::max<std::size_t>(text.size(), 28);
		while (text.size() < n)
		{
			text += ' ';
		}
		c_put_str(TERM_WHITE, text.c_str(), hgt/2, wid/2 - 14);
	};

	// Function for showing an error message
	auto error = [show_message]() {
		show_message("Saving rules FAILED!");
		inkey();
	};

	// Build the filename
	path_build(buf, 1024, ANGBAND_DIR_USER, name.c_str());
		
	if (boost::filesystem::exists(buf))
	{
		show_message("File exists, continue? [y/n]");
		ch = inkey();
		if ((ch != 'Y') && (ch != 'y'))
		{
			return;
		}
	}

	// Convert to a JSON document
	auto rules_document = automatizer->to_json();

	// Open output stream
	std::ofstream of(buf, std::ios_base::out | std::ios_base::binary);
	if (of.fail())
	{
		error();
		return;
	}

	// Write JSON to output
	jsoncons::json_options serialization_options;
	serialization_options.indent_size(2);
	of << jsoncons::pretty_print(rules_document, serialization_options);
	if (of.fail())
	{
		error();
		return;
	}

	// Success
	show_message("Saved rules in file");
	inkey();
}

static void rename_rule(Rule *rule)
{
	assert(rule != nullptr);

	std::string name = rule->get_name();
	if (input_box_auto("New name?", &name, 16))
	{
		rule->set_name(name.c_str());
	}
}

#define ACTIVE_LIST     0
#define ACTIVE_RULE     1
void do_cmd_automatizer()
{
	int active = ACTIVE_LIST;
	const char *keys;
	const char *keys2;
	const char *keys3;
	std::vector<const char *> rule_names;

	if (!automatizer_enabled)
	{
		if (msg_box_auto("Automatizer is currently disabled, enable it? (y/n)") == 'y')
		{
			automatizer_enabled = true;
		}
		else
			return;
	}

	screen_save();

	automatizer->reset_view();

	while (true)
	{
		Term_clear();

		int wid, hgt;
		Term_get_size(&wid, &hgt);

		auto rule_names = automatizer->get_rule_names();

		display_list(0, 0, hgt - 1, 15, "Rules", rule_names, automatizer->rules_begin(), automatizer->selected_rule(), (active == ACTIVE_LIST) ? TERM_L_GREEN : TERM_GREEN);

		draw_box(0, 15, hgt - 4, wid - 1 - 15);
		if (active == ACTIVE_RULE)
		{
			keys = "#Bup#W/#Bdown#W/#Bleft#W/#Bright#W to navitage rule, #B9#W/#B3#W/#B7#W/#B1#W to scroll";
			keys2 = "#Btab#W for switch, #Ba#Wdd clause, #Bd#Welete clause/rule";
			keys3 = "#G?#W for Automatizer help";
		}
		else
		{
			keys = "#Bup#W/#Bdown#W to scroll, #Btab#W to switch to the rule window";
			keys2 = "#Bu#W/#Bd#W to move rules, #Bn#Wew rule, #Br#Wename rule, #Bs#Wave rules";
			keys3 = "#Rk#W to #rdisable#W the automatizer, #G?#W for Automatizer help";
		}
		display_message(17, hgt - 3, strlen(keys), TERM_WHITE, keys);
		display_message(17, hgt - 2, strlen(keys2), TERM_WHITE, keys2);
		display_message(17, hgt - 1, strlen(keys3), TERM_WHITE, keys3);

		automatizer->show_current();

		char c = inkey();

		if (c == ESCAPE) break;
		if (active == ACTIVE_LIST)
		{
			if (c == '?')
			{
				screen_save();
				show_file("automat.txt", "Automatizer help");
				screen_load();
			}
			else if (c == '8')
			{
				if (!automatizer->rules_count()) continue;

				automatizer->select_rule(
					automatizer->selected_rule() - 1);
			}
			else if (c == '2')
			{
				if (!automatizer->rules_count()) continue;

				automatizer->select_rule(
					automatizer->selected_rule() + 1);
			}
			else if (c == 'u')
			{
				int sel = automatizer->selected_rule();
				if (sel > 0)
				{
					automatizer->swap_rules(sel-1, sel);
					automatizer->select_rule(sel-1);
				}
			}
			else if (c == 'd')
			{
				if (!automatizer->rules_count()) continue;

				int sel = automatizer->selected_rule();
				if (sel < automatizer->rules_count() - 1)
				{
					automatizer->swap_rules(sel, sel+1);
					automatizer->select_rule(sel+1);
				}
			}
			else if (c == 'n')
			{
				int i = create_new_rule();
				if (i >= 0)
				{
					automatizer->select_rule(i);
					active = ACTIVE_RULE;
				}
			}
			else if (c == 's')
			{
				automatizer_save_rules();
			}
			else if (c == 'r')
			{
				if (!automatizer->rules_count()) continue;

				rename_rule(automatizer->current_rule().get());
				continue;
			}
			else if (c == 'k')
			{
				automatizer_enabled = false;
				break;
			}
			else if (c == '\t')
			{
				if (!automatizer->rules_count()) continue;

				active = ACTIVE_RULE;
			}
		}
		else if (active == ACTIVE_RULE)
		{
			if (c == '?')
			{
				screen_save();
				show_file("automat.txt", "Automatizer help");
				screen_load();
			}
			else if (c == '8')
			{
				automatizer->move_up();
			}
			else if (c == '2')
			{
				automatizer->move_down();
			}
			else if (c == '6')
			{
				automatizer->move_right();
			}
			else if (c == '4')
			{
				automatizer->move_left();
			}
			else if (c == '9')
			{
				automatizer->scroll_up();
			}
			else if (c == '3')
			{
				automatizer->scroll_down();
			}
			else if (c == '7')
			{
				automatizer->scroll_left();
			}
			else if (c == '1')
			{
				automatizer->scroll_right();
			}
			else if (c == 'a')
			{
				if (!automatizer->selected_rule())
					continue;
				automatizer->add_new_condition(
					squelch::new_condition_interactive);
			}
			else if (c == 'd')
			{
				if (!automatizer->rules_count())
				{
					continue;
				}

				int new_sel =
					automatizer->remove_current_selection();

				if (!new_sel)
				{
					active = ACTIVE_LIST;
				}
			}
			else if (c == '\t')
			{
				active = ACTIVE_LIST;
			}
		}
	}

	screen_load();
}

enum class add_rule_mode { TVAL, TSVAL, NAME };

static void easy_add_rule(add_rule_mode mode, bool do_status, object_type *o_ptr)
{
	std::shared_ptr<Condition> condition;

	switch (mode)
	{

	case add_rule_mode::TVAL:
		condition = std::make_shared<TvalCondition>(o_ptr->tval);
		break;

	case add_rule_mode::TSVAL:
	{
		auto andCondition = std::make_shared<AndCondition>();

		andCondition->add_condition(
			std::make_shared<TvalCondition>(o_ptr->tval));
		andCondition->add_condition(
			std::make_shared<SvalCondition>(o_ptr->sval, o_ptr->sval));

		condition = andCondition;
		break;
	}

	case add_rule_mode::NAME:
	{
		char buf[128];
		object_desc(buf, o_ptr, -1, 0);

		condition = std::make_shared<NameCondition>(buf);
		break;
	}

	}

	// Use object status?
	if (do_status)
	{
		status_type status = squelch::object_status(o_ptr);

		auto andCondition = std::make_shared<AndCondition>();

		andCondition->add_condition(
			std::shared_ptr<Condition>(condition)); // cycle
		andCondition->add_condition(
			std::make_shared<StatusCondition>(status));

		// Replace condition; breaks cycle
		condition = andCondition;
	}

	// Rule name
	auto rule_name = action_mapping().stringify(action_type::AUTO_DESTROY);

	// Append to list of rules
	automatizer->append_rule(
		std::make_shared<DestroyRule>(
			rule_name, game_module_idx, condition));

	msg_print("Rule added. Please go to the Automatizer screen (press = then T)");
	msg_print("to save the modified ruleset.");
}

void automatizer_add_rule(object_type *o_ptr)
{
	bool do_status = false;

	while (true)
	{
		char ch;

		if (!get_com(format("Destroy all of the same [T]ype, [F]amily or [N]ame, also use [S]tatus (%s)? ", (do_status) ? "Yes" : "No"), &ch))
		{
			break;
		}

		if (ch == 'S' || ch == 's')
		{
			do_status = !do_status;
			continue;
		}

		if (ch == 'T' || ch == 't')
		{
			easy_add_rule(add_rule_mode::TSVAL, do_status, o_ptr);
			break;
		}

		if (ch == 'F' || ch == 'f')
		{
			easy_add_rule(add_rule_mode::TVAL, do_status, o_ptr);
			break;
		}

		if (ch == 'N' || ch == 'n')
		{
			easy_add_rule(add_rule_mode::NAME, do_status, o_ptr);
			break;
		}
	}
}

/**
 * Initialize the automatizer.
 */
void automatizer_init()
{
	// Only permit single initialization.
	assert(automatizer == nullptr);

	// Set up dependencies
	auto tree_printer(std::make_shared<squelch::TreePrinter>());
	auto cursor(std::make_shared<squelch::Cursor>());

	// Initialize
	automatizer = new squelch::Automatizer(tree_printer, cursor);
}

/**
 * Load automatizer file. Returns true iff automatizer
 * rules were loaded successfully.
 */
bool automatizer_load(boost::filesystem::path const &path)
{
	assert(automatizer != NULL);

	// Does the path exist?
	if (!boost::filesystem::exists(path))
	{
		return false; // Not fatal; just skip
	}

	// Parse into memory
	jsoncons::json rules_json;
	try
	{
		// Open
		std::ifstream ifs(
			path.string(),
			std::ifstream::in | std::ifstream::binary);
		// Parse
		ifs >> rules_json;
	}
	catch (jsoncons::json_exception const &exc)
	{
		msg_format("Error parsing automatizer rules from '%s'.", path.c_str());
		msg_print(exc.what());
		return false;
	}
	catch (const std::ifstream::failure &exc)
	{
		msg_format("I/O error reading automatizer rules from '%s'.", path.c_str());
		msg_print(exc.what());
		return false;
	}

	// We didn't return directly via an exception, so let's extract
	// the rules.
	automatizer->load_json(rules_json);
	return true;
}
