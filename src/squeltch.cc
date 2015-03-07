/*
 * Copyright (c) 2002 DarkGod
 * Copyright (c) 2012 Bardur Arantsson
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "squeltch.hpp"

#include <jansson.h>
#include <algorithm>
#include <memory>
#include <deque>
#include <list>
#include <string>
#include <vector>

#include "tome/squelch/tree_printer.hpp"
#include "tome/squelch/condition.hpp"
#include "tome/squelch/condition_metadata.hpp"
#include "tome/squelch/rule.hpp"
#include "tome/squelch/cursor.hpp"
#include "tome/squelch/object_status.hpp"
#include "tome/squelch/automatizer.hpp"
#include "util.hpp"
#include "util.h"

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

void squeltch_grid(void)
{
	if (!automatizer_enabled)
	{
		return;
	}

	// Scan the pile of objects
	s16b next_o_idx = 0;
	for (s16b this_o_idx = cave[p_ptr->py][p_ptr->px].o_idx;
	     this_o_idx;
	     this_o_idx = next_o_idx)
	{
		// Acquire object
		object_type * o_ptr = &o_list[this_o_idx];

		// We've now seen one of these
		if (!k_info[o_ptr->k_idx].flavor)
		{
			object_aware(o_ptr);
		}

		// Acquire next object
		next_o_idx = o_ptr->next_o_idx;

		// Apply rules
		automatizer->apply_rules(o_ptr, -this_o_idx);
	}
}

void squeltch_inventory(void)
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
			if ((o_ptr->k_idx > 0) && automatizer->apply_rules(o_ptr, i))
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
	char name[20] = { '\0' };
	int wid = 0, hgt = 0;

	Term_get_size(&wid, &hgt);

	sprintf(name, "%s", "No name");
	if (!input_box("Name?", hgt / 2, wid / 2, name, sizeof(name)))
	{
		return -1;
	}

	char typ = lua_msg_box("[D]estroy, [P]ickup, [I]nscribe?");

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
		cptr i = lua_input_box("Inscription?", 79);
		if ((i == nullptr) || (strlen(i) == 0))
		{
			return -1;
		}

		rule = std::make_shared<InscribeRule>(
			name, game_module_idx, nullptr, std::string(i));

		break;
	}

	default:
		return -1;
	}

	return automatizer->append_rule(rule);
}

static void automatizer_save_rules()
{
	char name[30] = { '\0' };
	char buf[1025];
	char ch;
	int hgt, wid;

	Term_get_size(&wid, &hgt);

	sprintf(name, "automat.atm");
	if (!input_box("Save name?", hgt / 2, wid / 2, name, sizeof(name)))
	{
		return;
	}

	// Build the filename
	path_build(buf, 1024, ANGBAND_DIR_USER, name);
		
	// File type is "TEXT"
	FILE_TYPE(FILE_TYPE_TEXT);

	if (file_exist(buf))
	{
		c_put_str(TERM_WHITE, "File exists, continue?[y/n]",
			  hgt / 2,
			  wid / 2 - 14);
		ch = inkey();
		if ((ch != 'Y') && (ch != 'y'))
		{
			return;
		}
	}

	// Write to file
	{
		auto rules_json = automatizer->to_json();

		int status = json_dump_file(rules_json.get(), buf,
					    JSON_INDENT(2) |
					    JSON_SORT_KEYS);
		if (status == 0)
		{
			c_put_str(TERM_WHITE, "Saved rules in file        ",
				  hgt / 2,
				  wid / 2 - 14);
		}
		else
		{
			c_put_str(TERM_WHITE, "Saving rules failed!       ",
				  hgt / 2,
				  wid / 2 - 14);
		}

		// Wait for keypress
		inkey();
	}
}

static void rename_rule(Rule *rule)
{
	char name[16];
	int wid, hgt;

	assert(rule != nullptr);

	Term_get_size(&wid, &hgt);

	sprintf(name, "%s", rule->get_name());
	if (input_box("New name?", hgt / 2, wid / 2, name, sizeof(name)))
	{
		rule->set_name(name);
	}
}

#define ACTIVE_LIST     0
#define ACTIVE_RULE     1
void do_cmd_automatizer()
{
	int wid = 0, hgt = 0;
	int active = ACTIVE_LIST;
	cptr keys;
	cptr keys2;
	cptr keys3;
	std::vector<cptr> rule_names;

	Term_get_size(&wid, &hgt);

	if (!automatizer_enabled)
	{
		if (msg_box("Automatizer is currently disabled, enable it? (y/n)", hgt / 2, wid / 2) == 'y')
		{
			automatizer_enabled = TRUE;
		}
		else
			return;
	}

	screen_save();

	automatizer->reset_view();

	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		automatizer->get_rule_names(&rule_names);

		display_list(0, 0, hgt - 1, 15, "Rules", rule_names.data(), automatizer->rules_count(), automatizer->rules_begin(), automatizer->selected_rule(), (active == ACTIVE_LIST) ? TERM_L_GREEN : TERM_GREEN);

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
				show_file("automat.txt", "Automatizer help", 0, 0);
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
				automatizer_enabled = FALSE;
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
				show_file("automat.txt", "Automatizer help", 0, 0);
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

				if (new_sel == -1)
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
 * Load automatizer file. This function may be called multiple times
 * with different file names -- it should NOT clear any automatizer
 * state (including loaded rules).
 */
void automatizer_load(cptr file_path)
{
	assert(file_path != NULL);
	assert(automatizer != NULL);

	// Does the file exist?
	if (!file_exist(file_path))
	{
		return; // Not fatal; just skip
	}

	// Parse file
	json_error_t error;
	std::shared_ptr<json_t> rules_json(
		json_load_file(file_path, 0, &error),
		&json_decref);
	if (rules_json == nullptr)
	{
		msg_format("Error parsing automatizer rules from '%s'.", file_path);
		msg_format("Line %d, Column %d", error.line, error.column);
		msg_print(nullptr);
		return;
	}

	// Load rules
	automatizer->load_json(rules_json.get());
}
