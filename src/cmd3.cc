/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */
#include "cmd3.hpp"

#include "artifact_type.hpp"
#include "cave.hpp"
#include "cave_type.hpp"
#include "cli_comm.hpp"
#include "files.hpp"
#include "game.hpp"
#include "gods.hpp"
#include "hook_drop_in.hpp"
#include "hook_wield_in.hpp"
#include "hooks.hpp"
#include "monster1.hpp"
#include "monster_race.hpp"
#include "monster_race_flag.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_flag.hpp"
#include "object_kind.hpp"
#include "object_type.hpp"
#include "options.hpp"
#include "player_type.hpp"
#include "squeltch.hpp"
#include "store.hpp"
#include "store_type.hpp"
#include "tables.hpp"
#include "town_type.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "xtra1.hpp"
#include "xtra2.hpp"
#include "z-form.hpp"
#include "z-rand.hpp"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <utility>

using boost::algorithm::equals;

/*
 * Display p_ptr->inventory
 */
void do_cmd_inven()
{
	char out_val[160];


	/* Note that we are in "p_ptr->inventory" mode */
	command_wrk = false;

	/* Save the screen */
	screen_save_no_flush();

	/* Show the inventory */
	show_inven_full();

	/* Show prompt */
	{
		s32b total_weight = calc_total_weight();

		strnfmt(out_val, 160,
		        "Inventory: carrying %ld.%ld pounds (%ld%% of capacity). Command: ",
		        total_weight / 10, total_weight % 10,
		        (total_weight * 100) / ((weight_limit()) / 2));
	}

	/* Get a command */
	prt(out_val, 0, 0);

	/* Get a new command */
	command_new = inkey();

	/* Restore the screen */
	screen_load_no_flush();

	/* Process "Escape" */
	if (command_new == ESCAPE)
	{
		/* Reset stuff */
		command_new = 0;
	}

	/* Process normal keys */
	else
	{
		/* Mega-Hack -- Don't disable keymaps for this key */
		request_command_inven_mode = true;
	}
}


/*
 * Display equipment
 */
void do_cmd_equip()
{
	char out_val[160];


	/* Note that we are in "equipment" mode */
	command_wrk = true;

	/* Save the screen */
	screen_save_no_flush();

	/* Display the equipment */
	show_equip_full();

	/* Show prompt */
	{
		s32b total_weight = calc_total_weight();

		/* Build a prompt */
		strnfmt(out_val, 160,
		        "Equipment: carrying %ld.%ld pounds (%ld%% of capacity). Command: ",
		        total_weight / 10, total_weight % 10,
		        (total_weight * 100) / ((weight_limit()) / 2));
	}

	/* Get a command */
	prt(out_val, 0, 0);

	/* Get a new command */
	command_new = inkey();

	/* Restore the screen */
	screen_load_no_flush();

	/* Process "Escape" */
	if (command_new == ESCAPE)
	{
		/* Reset stuff */
		command_new = 0;
	}

	/* Process normal keys */
	else
	{
		/* Mega-Hack -- Don't disable keymaps for this key */
		request_command_inven_mode = true;
	}
}


/*
 * The "wearable" tester
 */
static bool item_tester_hook_wear(object_type const *o_ptr)
{
	int slot = wield_slot(o_ptr);

	/* Only one ultimate at a time */
	if (object_flags(o_ptr) & TR_ULTIMATE)
	{
		for (int i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			object_type *q_ptr = &p_ptr->inventory[i];

			if (!q_ptr->k_ptr)
			{
				continue;
			}

			if (object_flags(q_ptr) & TR_ULTIMATE)
			{
				return false;
			}
		}
	}

	if ((slot < INVEN_WIELD) || ((p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_WIELD) && (p_ptr->melee_style != SKILL_MASTERY)))
		return false;

	/* Check for a usable slot */
	if (slot >= INVEN_WIELD) return true;

	/* Assume not wearable */
	return false;
}


static bool is_slot_ok(int slot)
{
	return (slot >= INVEN_WIELD) && (slot < INVEN_TOTAL);
}


/*
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_wield()
{
	auto const &a_info = game->edit_data.a_info;

	int item, slot, num = 1;

	object_type forge;

	object_type *q_ptr;

	object_type *i_ptr;

	const char *act;

	char o_name[80];

	/* Get an item */
	if (!get_item(&item,
		      "Wear/Wield which item? ",
		      "You have nothing you can wear or wield.",
		      (USE_INVEN | USE_FLOOR),
		      item_tester_hook_wear))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Check the slot */
	slot = wield_slot(o_ptr);

	/* Prevent wielding into a cursed slot */
	if (cursed_p(&p_ptr->inventory[slot]))
	{
		/* Describe it */
		object_desc(o_name, &p_ptr->inventory[slot], false, 0);

		/* Message */
		msg_format("The %s you are %s appears to be cursed.",
		           o_name, describe_use(slot));

		/* Cancel the command */
		return;
	}

	if ((cursed_p(o_ptr)) && (options->wear_confirm)
			&& (object_known_p(o_ptr)))
	{
		char dummy[512];

		/* Describe it */
		object_desc(o_name, o_ptr, false, 0);

		strnfmt(dummy, 512, "Really use the %s {cursed}? ", o_name);
		if (!(get_check(dummy)))
			return;
	}

	/* Can we wield */
	{
		struct hook_wield_in in = { o_ptr };
		if (process_hooks_new(HOOK_WIELD, &in, NULL))
		{
			return;
		}
	}

	/* Extract the flags */
	auto const flags = object_flags(o_ptr);

	/* Two handed weapons can't be wielded with a shield */
	if ((is_slot_ok(slot - INVEN_WIELD + INVEN_ARM)) &&
		(flags & TR_MUST2H) &&
		(p_ptr->inventory[slot - INVEN_WIELD + INVEN_ARM].k_ptr))
	{
		object_desc(o_name, o_ptr, false, 0);
		msg_format("You cannot wield your %s with a shield.", o_name);
		return;
	}

	if (is_slot_ok(slot - INVEN_ARM + INVEN_WIELD))
	{
		i_ptr = &p_ptr->inventory[slot - INVEN_ARM + INVEN_WIELD];

		/* Extract the flags */
		auto const i_flags = object_flags(i_ptr);

		/* Prevent shield from being put on if wielding 2H */
		if ((i_flags & TR_MUST2H) &&
			i_ptr->k_ptr &&
			(p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_ARM))
		{
			object_desc(o_name, o_ptr, false, 0);
			msg_format("You cannot wield your %s with a two-handed weapon.", o_name);
			return;
		}

		if ((p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_ARM) &&
			(i_flags & TR_COULD2H))
		{
			if (!get_check("Are you sure you want to restrict your fighting? "))
			{
				return;
			}
		}
	}

	if ((is_slot_ok(slot - INVEN_WIELD + INVEN_ARM)) &&
		p_ptr->inventory[slot - INVEN_WIELD + INVEN_ARM].k_ptr &&
		(flags & TR_COULD2H))
	{
		if (!get_check("Are you sure you want to use this weapon with a shield?"))
		{
			return;
		}
	}

	/* Take a turn */
	energy_use = 100;

	/* Get local object */
	q_ptr = &forge;

	/* Obtain local object */
	object_copy(q_ptr, o_ptr);

	if (slot == INVEN_AMMO) num = o_ptr->number;

	/* Modify quantity */
	q_ptr->number = num;

	/* Decrease the item */
	inc_stack_size_ex(item, -num, OPTIMIZE, NO_DESCRIBE);

	/* Access the wield slot */
	o_ptr = &p_ptr->inventory[slot];

	/* Take off existing item */
	if (slot != INVEN_AMMO)
	{
		if (o_ptr->k_ptr)
		{
			/* Take off existing item */
			inven_takeoff(slot, 255, false);
		}
	}
	else
	{
		if (o_ptr->k_ptr)
		{
			if (!object_similar(o_ptr, q_ptr))
			{
				/* Take off existing item */
				inven_takeoff(slot, 255, false);
			}
			else
			{
				q_ptr->number += o_ptr->number;
			}
		}
	}


	/* Wear the new stuff */
	object_copy(o_ptr, q_ptr);

	/* Increment the equip counter by hand */
	equip_cnt++;

	/* Where is the item now */
	if (slot == INVEN_WIELD)
	{
		act = "You are wielding";
	}
	else if (( slot == INVEN_BOW ) && (o_ptr->tval == TV_INSTRUMENT))
	{
		act = "You are holding";
	}
	else if (slot == INVEN_BOW)
	{
		act = "You are shooting with";
	}
	else if (slot == INVEN_LITE)
	{
		act = "Your light source is";
	}
	else if (slot == INVEN_AMMO)
	{
		act = "In your quiver you have";
	}
	else if (slot == INVEN_TOOL)
	{
		act = "You are using";
	}
	else
	{
		act = "You are wearing";
	}

	/* Describe the result */
	object_desc(o_name, o_ptr, true, 3);

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(slot));

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		msg_print("Oops! It feels deathly cold!");
	}

	/* Take care of item sets */
	if (o_ptr->name1)
	{
		wield_set(o_ptr->name1, a_info[o_ptr->name1].set, false);
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);

	/* Recalculate hitpoint */
	p_ptr->update |= (PU_HP);

	/* Recalculate mana */
	p_ptr->update |= (PU_MANA | PU_SPELLS);

	/* Redraw monster hitpoint */
	p_ptr->redraw |= (PR_FRAME);

	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
}



/*
 * Take off an item
 */
void do_cmd_takeoff()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Take off which item? ",
		      "You are not wearing anything to take off.",
		      (USE_EQUIP)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Item is cursed */
	if (cursed_p(o_ptr) && (!wizard))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");

		/* Nope */
		return;
	}


	/* Take a partial turn */
	energy_use = 50;

	/* Take off the item */
	inven_takeoff(item, 255, false);

	/* Recalculate hitpoint */
	p_ptr->update |= (PU_HP);

	p_ptr->redraw |= (PR_FRAME);
}


/*
 * Drop an item
 */
void do_cmd_drop()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Drop which item? ",
		      "You have nothing to drop.",
		      (USE_EQUIP | USE_INVEN)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);
	auto const flags = object_flags(o_ptr);

	/* Can we drop */
	struct hook_drop_in in = { item };
	if (process_hooks_new(HOOK_DROP, &in, NULL)) return;

	/* Hack -- Cannot remove cursed items */
	if (cursed_p(o_ptr))
	{
		if (item >= INVEN_WIELD)
		{
			/* Oops */
			msg_print("Hmmm, it seems to be cursed.");

			/* Nope */
			return;
		}
		else
		{
			if (flags & TR_CURSE_NO_DROP)
			{
				/* Oops */
				msg_print("Hmmm, you seem to be unable to drop it.");

				/* Nope */
				return;
			}
		}
	}

	/* See how many items */
	int amt = 1;
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}

	/* Take a partial turn */
	energy_use = 50;

	/* Drop (some of) the item */
	inven_drop(item, amt, p_ptr->py, p_ptr->px, false);
}


/*
 * Destroy an item
 */
void do_cmd_destroy()
{
	int old_number;

	bool force = false;

	char o_name[80];

	char out_val[160];

	/* Hack -- force destruction */
	if (command_arg > 0) force = true;


	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Destroy which item? ",
		      "You have nothing to destroy.",
		      (USE_INVEN | USE_FLOOR | USE_AUTO)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* See how many items */
	int amt = 1;
	if (o_ptr->number > 1)
	{
		/* Get a quantity */
		amt = get_quantity(NULL, o_ptr->number);

		/* Allow user abort */
		if (amt <= 0) return;
	}


	/* Describe the object */
	old_number = o_ptr->number;
	o_ptr->number = amt;
	object_desc(o_name, o_ptr, true, 3);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		/* Make a verification */
		strnfmt(out_val, 160, "Really destroy %s? ", o_name);
		if (!get_check(out_val)) return;
	}

	/* Take no time, just like the automatizer */
	energy_use = 0;

	auto const flags = object_flags(o_ptr);
	if ((flags & TR_CURSE_NO_DROP) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, you seem to be unable to destroy it.");

		/* Nope */
		return;
	}


	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr))
	{
		/* Don't use any energy */
		energy_use = 0;

		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		/* Done */
		return;
	}

	/* Message */
	msg_format("You destroy %s.", o_name);

	/* Create an automatizer rule */
	if (automatizer_create)
	{
		automatizer_add_rule(o_ptr);
	}

	/*
	 * Hack -- If rods or wand are destroyed, the total maximum timeout or
	 * charges of the stack needs to be reduced, unless all the items are
	 * being destroyed. -LM-
	 */
	if ((o_ptr->tval == TV_WAND) && (amt < o_ptr->number))
	{
		o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;
	}

	/* Eru wont be happy */
	if (flags & TR_BLESSED)
	{
		inc_piety(GOD_ERU, -10 * o_ptr->k_ptr->level);
	}

	/* Eliminate the item */
	inc_stack_size(item, -amt);
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Examine which item? ",
		      "You have nothing to examine.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Description */
	char o_name[80];
	object_desc(o_name, o_ptr, true, 3);

	/* Describe */
	cmsg_format(TERM_L_BLUE, "%s", o_name);

	/* Describe it fully */
	if (!object_out_desc(o_ptr, NULL, false, true)) msg_print("You see nothing special.");
}



/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Un-inscribe which item? ",
		      "You have nothing to un-inscribe.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Nothing to remove */
	if (o_ptr->inscription.empty())
	{
		msg_print("That item had no inscription to remove.");
		return;
	}

	/* Message */
	msg_print("Inscription removed.");

	/* Remove the incription */
	o_ptr->inscription.clear();

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Inscribe which item? ",
		      "You have nothing to inscribe.",
		      (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Describe the activity */
	char o_name[80];
	object_desc(o_name, o_ptr, true, 3);

	/* Message */
	msg_format("Inscribing %s.", o_name);
	msg_print(NULL);

	/* Start with old inscription */
	char out_val[80];
	strcpy(out_val, o_ptr->inscription.c_str());

	/* Get a new inscription (possibly empty) */
	if (get_string("Inscription: ", out_val, sizeof(out_val)))
	{
		/* Save the inscription */
		o_ptr->inscription = out_val;

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * An "item_tester_hook" for refilling lanterns
 */
static object_filter_t const &item_tester_refill_lantern()
{
	using namespace object_filter;
	static auto instance = Or(
		TVal(TV_FLASK),
		And(
			TVal(TV_LITE),
			SVal(SV_LITE_LANTERN)));
	return instance;
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Refill with which flask? ",
		      "You have no flasks of oil.",
		      (USE_INVEN | USE_FLOOR),
		      item_tester_refill_lantern()))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Take a partial turn */
	energy_use = 50;

	/* Access the lantern */
	object_type *j_ptr = &p_ptr->inventory[INVEN_LITE];

	/* Refuel */
	if (o_ptr->tval == TV_FLASK)
		j_ptr->timeout += o_ptr->pval;
	else
		j_ptr->timeout += o_ptr->timeout;

	/* Message */
	msg_print("You fuel your lamp.");

	/* Comment */
	if (j_ptr->timeout >= FUEL_LAMP)
	{
		j_ptr->timeout = FUEL_LAMP;
		msg_print("Your lamp is full.");
	}

	/* Decrease the item stack */
	inc_stack_size(item, -1);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
}


/*
 * An "item_tester_hook" for refilling torches
 */
static object_filter_t const &item_tester_refill_torch()
{
	using namespace object_filter;
	static auto instance =
		And(
			TVal(TV_LITE),
			SVal(SV_LITE_TORCH));
	return instance;
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch()
{
	/* Get an item */
	int item;
	if (!get_item(&item,
		      "Refuel with which torch? ",
		      "You have no extra torches.",
		      (USE_INVEN | USE_FLOOR),
		      item_tester_refill_torch()))
	{
		return;
	}

	/* Get the item */
	object_type *o_ptr = get_object(item);

	/* Take a partial turn */
	energy_use = 50;

	/* Access the primary torch */
	object_type *j_ptr = &p_ptr->inventory[INVEN_LITE];

	/* Refuel */
	j_ptr->timeout += o_ptr->timeout + 5;

	/* Message */
	msg_print("You combine the torches.");

	/* Over-fuel message */
	if (j_ptr->timeout >= FUEL_TORCH)
	{
		j_ptr->timeout = FUEL_TORCH;
		msg_print("Your torch is fully fueled.");
	}

	/* Refuel message */
	else
	{
		msg_print("Your torch glows more brightly.");
	}

	/* Decrease the item stack */
	inc_stack_size(item, -1);

	/* Recalculate torch */
	p_ptr->update |= (PU_TORCH);
}


/*
 * Refill the players lamp, or restock his torches
 */
void do_cmd_refill()
{
	/* Get the light */
	auto o_ptr = &p_ptr->inventory[INVEN_LITE];

	/* It is nothing */
	if (o_ptr->tval != TV_LITE)
	{
		msg_print("You are not wielding a light.");
		return;
	}

	auto const flags = object_flags(o_ptr);

	if (flags & TR_FUEL_LITE)
	{
		/* It's a torch */
		if (o_ptr->sval == SV_LITE_TORCH ||
		                o_ptr->sval == SV_LITE_TORCH_EVER)
		{
			do_cmd_refill_torch();
		}

		/* It's a lamp */
		else if (o_ptr->sval == SV_LITE_LANTERN ||
		                o_ptr->sval == SV_LITE_DWARVEN ||
		                o_ptr->sval == SV_LITE_FEANORIAN)
		{
			do_cmd_refill_lamp();
		}
	}

	/* No torch to refill */
	else
	{
		msg_print("Your light cannot be refilled.");
	}
}


/*
 * Target command
 */
void do_cmd_target()
{
	/* Target set */
	if (target_set(TARGET_KILL))
	{
		msg_print("Target Selected.");
	}

	/* Target aborted */
	else
	{
		msg_print("Target Aborted.");
	}
}



/*
 * Look command
 */
void do_cmd_look()
{
	/* Look around */
	if (target_set(TARGET_LOOK))
	{
		msg_print("Target Selected.");
	}
}



/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate()
{
	int dir, y1, x1, y2, x2;
	int panel_hgt, panel_wid;
	char tmp_val[80];
	char out_val[160];


	/* Retrieve size of the Angband window */
	Term_get_size(&panel_wid, &panel_hgt);

	/* Calcurate size of the dungeon map area */
	panel_hgt = (panel_hgt - (ROW_MAP + 1)) / 2;
	panel_wid = (panel_wid - (COL_MAP + 1)) / 2;

	/* Start at current panel */
	y2 = y1 = panel_row_min;
	x2 = x1 = panel_col_min;

	/* Show panels until done */
	while (true)
	{
		/* Describe the location */
		if ((y2 == y1) && (x2 == x1))
		{
			tmp_val[0] = '\0';
		}
		else
		{
			strnfmt(tmp_val, 80, "%s%s of",
			        ((y2 < y1) ? " North" : (y2 > y1) ? " South" : ""),
			        ((x2 < x1) ? " West" : (x2 > x1) ? " East" : ""));
		}

		/* Prepare to ask which way to look */
		if ((panel_hgt == PANEL_HGT) && (panel_wid == PANEL_WID))
		{
			/* Avoid surprising the standard screen users */
			strnfmt(out_val, 160,
			        "Map sector [%d,%d], which is%s your sector. Direction?",
			        y2 / panel_hgt, x2 / panel_wid, tmp_val);
		}

		/* Big screen */
		else
		{
			/* Panels are measured by current map area size */
			strnfmt(out_val, 160,
			        "Map sector [%d(%02d),%d(%02d)], which is%s your sector. Direction?",
			        y2 / panel_hgt, y2 % panel_hgt,
			        x2 / panel_wid, x2 % panel_wid, tmp_val);
		}

		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			char ch;

			/* Get a command (or cancel) */
			if (!get_com(out_val, &ch)) break;

			/* Extract the action (if any) */
			dir = get_keymap_dir(ch);

			/* Error */
			if (!dir) bell();
		}

		/* No direction */
		if (!dir) break;

		/* Apply the motion */
		if (change_panel(ddy[dir], ddx[dir]))
		{
			y2 = panel_row_min;
			x2 = panel_col_min;
		}
	}

	/* Recenter the map around the player */
	verify_panel();

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff();
}






/*
 * The table of "symbol info" -- each entry is a string of the form
 * "X:desc" where "X" is the trigger, and "desc" is the "info".
 */
static const char *ident_info[] =
{
	" :A dark grid",
	"!:A potion (or oil)",
	"\":An amulet (or necklace)",
	"#:A wall (or secret door)",
	"$:Treasure (gold or gems)",
	"%:A vein (magma or quartz)",
	/* "&:unused", */
	"':An open door",
	"(:Soft armor",
	"):A shield",
	"*:A vein with treasure",
	"+:A closed door",
	",:Food (or mushroom patch)",
	"-:A wand (or rod)",
	".:Floor",
	"/:A polearm (Axe/Pike/etc)",
	"0:An altar",
	"1:Entrance to General Store",
	"2:Entrance to Armory",
	"3:Entrance to Weaponsmith",
	"4:Entrance to Temple",
	"5:Entrance to Alchemy shop",
	"6:Entrance to Magic store",
	"7:Entrance to Black Market",
	"8:Entrance to your home",
	"9:Entrance to Bookstore",
	"::Rubble",
	";:A glyph of warding / explosive rune",
	"<:An up staircase",
	"=:A ring",
	">:A down staircase",
	"?:A scroll",
	"@:You",
	"A:Angel",
	"B:Bird",
	"C:Canine",
	"D:Ancient Dragon/Wyrm",
	"E:Elemental",
	"F:Dragon Fly",
	"G:Ghost",
	"H:Hybrid",
	"I:Insect",
	"J:Snake",
	"K:Killer Beetle",
	"L:Lich",
	"M:Multi-Headed Reptile",
	/* "N:unused", */
	"O:Ogre",
	"P:Giant Humanoid",
	"Q:Quylthulg (Pulsing Flesh Mound)",
	"R:Reptile/Amphibian",
	"S:Spider/Scorpion/Tick",
	"T:Troll",
	"U:Major Demon",
	"V:Vampire",
	"W:Wight/Wraith/etc",
	"X:Xorn/Xaren/etc",
	"Y:Yeti",
	"Z:Zephyr Hound",
	"[:Hard armor",
	"\\:A hafted weapon (mace/whip/etc)",
	"]:Misc. armor",
	"^:A trap",
	"_:A staff",
	/* "`:unused", */
	"a:Ant",
	"b:Bat",
	"c:Centipede",
	"d:Dragon",
	"e:Floating Eye",
	"f:Feline",
	"g:Golem",
	"h:Hobbit/Elf/Dwarf",
	"i:Icky Thing",
	"j:Jelly",
	"k:Kobold",
	"l:Louse",
	"m:Mold",
	"n:Naga",
	"o:Orc",
	"p:Person/Human",
	"q:Quadruped",
	"r:Rodent",
	"s:Skeleton",
	"t:Townsperson",
	"u:Minor Demon",
	"v:Vortex",
	"w:Worm/Worm-Mass",
	/* "x:unused", */
	"y:Yeek",
	"z:Zombie/Mummy",
	"{:A missile (arrow/bolt/shot)",
	"|:An edged weapon (sword/dagger/etc)",
	"}:A launcher (bow/crossbow/sling)",
	"~:A tool (or miscellaneous item)",
	NULL
};


/**
 * Sort by monster experience.
 */
static bool compare_monster_experience(int w1, int w2)
{
	auto const &r_info = game->edit_data.r_info;

	/* Extract experience */
	s32b z1 = r_info[w1].mexp;
	s32b z2 = r_info[w2].mexp;

	/* Compare experience */
	if (z1 < z2) return true;
	if (z1 > z2) return false;

	/* Punt to index */
	return w1 < w2;
}

/**
 * Sort by monster level.
 */
static bool compare_monster_level(int w1, int w2)
{
	auto const &r_info = game->edit_data.r_info;

	/* Extract levels */
	byte z1 = r_info[w1].level;
	byte z2 = r_info[w2].level;

	/* Compare levels */
	if (z1 < z2) return true;
	if (z1 > z2) return false;
	
	/* Punt to monster experience. */
	return compare_monster_experience(w1, w2);
}

/*
 * Sort by player kills
 */
static bool compare_player_kills(int w1, int w2)
{
	auto const &r_info = game->edit_data.r_info;

	/* Extract player kills */
	s16b z1 = r_info[w1].r_pkills;
	s16b z2 = r_info[w2].r_pkills;

	/* Compare player kills */
	if (z1 < z2) return true;
	if (z1 > z2) return false;

	/* Punt to monster level. */
	return compare_monster_level(w1, w2);
}


/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_top(int r_idx)
{
	auto const &r_info = game->edit_data.r_info;

	auto r_ptr = &r_info[r_idx];

	byte a1, a2;

	char c1, c2;


	/* Access the chars */
	c1 = r_ptr->d_char;
	c2 = r_ptr->x_char;

	/* Access the attrs */
	a1 = r_ptr->d_attr;
	a2 = r_ptr->x_attr;


	/* Clear the top line */
	Term_erase(0, 0, 255);

	/* Reset the cursor */
	Term_gotoxy(0, 0);

	/* A title (use "The" for non-uniques) */
	if (!(r_ptr->flags & RF_UNIQUE))
	{
		Term_addstr( -1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr( -1, TERM_WHITE, r_ptr->name);

	/* Append the "standard" attr/char info */
	Term_addstr( -1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	Term_addstr( -1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr( -1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	Term_addstr( -1, TERM_WHITE, "'):");
}


/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "multiple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 *   ^M (case insensitive name search)
 *
 * The responses may be sorted in several ways, see below.
 *
 * Note that the player ghosts are ignored. XXX XXX XXX
 */
void do_cmd_query_symbol()
{
	auto const &r_info = game->edit_data.r_info;

	char sym, query;

	char buf[128];


	bool all = false;

	bool uniq = false;

	bool norm = false;


	bool name = false;

	char temp[80] = "";


	bool recall = false;

	bool (*sort_by)(int,int) = nullptr;

	/* Get a character, or abort */
	if (!get_com("Enter character to be identified, "
	                "or (Ctrl-A, Ctrl-U, Ctrl-N, Ctrl-M):", &sym)) return;

	/* Find that character info, and describe it */
	std::size_t i;
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (sym == KTRL('A'))
	{
		all = true;
		strcpy(buf, "Full monster list.");
	}
	else if (sym == KTRL('U'))
	{
		all = uniq = true;
		strcpy(buf, "Unique monster list.");
	}
	else if (sym == KTRL('N'))
	{
		all = norm = true;
		strcpy(buf, "Non-unique monster list.");
	}
	else if (sym == KTRL('M'))
	{
		all = name = true;
		if (!get_string("Name:", temp, 70)) return;
		strnfmt(buf, 128, "Monsters with a name \"%s\"", temp);
		strlower(temp);
	}
	else if (ident_info[i])
	{
		strnfmt(buf, 128, "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		strnfmt(buf, 128, "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	prt(buf, 0, 0);

	/* Collect matching monsters */
	std::vector<std::size_t> who;
	for (std::size_t i = 1; i < r_info.size(); i++)
	{
		auto r_ptr = &r_info[i];

		/* Require non-unique monsters if needed */
		if (norm && (r_ptr->flags & RF_UNIQUE)) continue;

		/* Require unique monsters if needed */
		if (uniq && !(r_ptr->flags & RF_UNIQUE)) continue;

		/* Require monsters with the name requested if needed */
		if (name)
		{
			char mon_name[80];

			strcpy(mon_name, r_ptr->name);
			strlower(mon_name);

			if (!strstr(mon_name, temp)) continue;
		}

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) {
			who.push_back(i);
		}
	}

	/* Nothing to recall */
	if (who.empty())
	{
		return;
	}


	/* Prompt XXX XXX XXX */
	put_str("Recall details? (k/p/y/n): ", 0, 40);

	/* Query */
	query = inkey();

	/* Restore */
	prt(buf, 0, 0);


	/* Sort by kills (and level) */
	if (query == 'k')
	{
		sort_by = compare_player_kills;
		query = 'y';
	}

	/* Sort by level */
	if (query == 'p')
	{
		sort_by = compare_monster_level;
		query = 'y';
	}

	/* Catch "escape" */
	if (query != 'y')
	{
		return;
	}


	/* Sort if needed */
	if (sort_by)
	{
		/* Sort the array */
		std::sort(std::begin(who), std::end(who), sort_by);
	}


	/* Start at the end */
	i = who.size() - 1;

	/* Scan the monster memory */
	while (true)
	{
		/* Extract a race */
		auto r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx, 0);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr( -1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (true)
		{
			/* Recall */
			if (recall)
			{
				/* Save the screen */
				screen_save_no_flush();

				/* Recall on screen */
				screen_roff(who[i], 0);

				/* Hack -- Complete the prompt (again) */
				Term_addstr( -1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Restore */
				screen_load_no_flush();
			}

			/* Normal commands */
			if (query != 'r') break;

			/* Toggle recall */
			recall = !recall;
		}

		/* Stop scanning */
		if (query == ESCAPE) break;

		/* Move to "prev" monster */
		if (query == '-')
		{
			i++;
			assert(i >= 0);
			if (static_cast<size_t>(i) == who.size())
			{
				i = 0;
			}
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
			{
				i = who.size() - 1;
			}
		}
	}

	/* Re-display the identity */
	prt(buf, 0, 0);
}


/*
 * Try to "sense" the grid's mana
 */
void do_cmd_sense_grid_mana()
{
	int chance, i;


	/* Take (a lot of) time */
	energy_use = 200;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* Hight mana grids are harder */
	chance = chance - (cave[p_ptr->py][p_ptr->px].mana / 10);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (rand_int(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint(chance) < USE_DEVICE))
	{
		flush_on_failure();
		msg_print("You failed to sense the grid's mana.");
		return;
	}

	/* Try to give an "average" value */
	i = (101 - p_ptr->skill_dev) / 2;
	i = (i < 1) ? 1 : (i > 50) ? 50 : i;

	if (wizard)
	{
		msg_format("Grid's mana: %d.", cave[p_ptr->py][p_ptr->px].mana);
		msg_format("Average grid's mana: %d.", (cave[p_ptr->py][p_ptr->px].mana / i) * i);
	}
	else
	{
		msg_format("Average Area's mana: %d", (cave[p_ptr->py][p_ptr->px].mana / i) * i);
	}
}


/*
 * Try to add a CLI action.
 */
void cli_add(const char *active, const char *trigger, const char *descr)
{
	s16b num;
	cli_comm *cli_ptr, *old_ptr;

	/* Too many macros. */
	if (cli_total >= CLI_MAX) return;

	/* First try to read active as a number. */
	if (strtol(active, 0, 0))
	{
		num = strtol(active, 0, 0);
	}
	/* Then try to read it as a character. */
	else if (strlen(active) == 1)
	{
		num = active[0];
	}
	/* Give up if it doesn't work. */
	else
	{
		return;
	}

	/* Dump the macro. */
	cli_ptr = cli_info + cli_total;
	old_ptr = cli_info + cli_total - 1;

	/*
	 * Trim 's from the ends of a token. This turns '@' into @ and
	 * ''' into '. This may be the intent of the code in tokenize(),
	 * but I've left it for lack of comments to back me up.
	 */
	if (strchr(trigger, '\''))
	{
		char temp[80], *t;
		const char *s;
		for (s = trigger, t = temp; ; s++, t++)
		{
			/* tokenize() causes each ' to be followed by another character,
			 * and then another '. Trim the 's here. */
			if (*s == '\'')
			{
				*t = *(++s);
				s++;
			}
			else
			{
				*t = *s;
			}
			if (*t == '\0') break;
		}
		cli_ptr->comm = strdup(temp);
	}
	else
	{
		cli_ptr->comm = strdup(trigger);
	}

	/* First try copying everything across. */
	cli_ptr->key = num;
	cli_ptr->descrip = nullptr;
	if (descr) {
		cli_ptr->descrip = strdup(descr);
	}

	/* Take description for the previous record if appropriate. */
	if ((cli_total > 0) && (old_ptr->key == cli_ptr->key) && (cli_ptr->descrip == 0))
	{
		cli_ptr->descrip = old_ptr->descrip;
	}

	/* Accept the macro. */
	if (cli_ptr->key && cli_ptr->comm && cli_ptr->descrip) cli_total++;
}



/*
 * Get a string using CLI completion.
 */
static bool get_string_cli(const char *prompt, char *buf, int len)
{
	bool res;


	/* Paranoia XXX XXX XXX */
	msg_print(NULL);

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	res = askfor_aux_with_completion(buf, len);

	/* Clear prompt */
	prt("", 0, 0);

	/* Result */
	return (res);
}


/*
 * Do a command line command
 *
 * This is a wrapper around process command to provide a "reverse keymap"
 * whereby a set of keypresses is mapped to one.
 *
 * This is useful because command_cmd is a s16b, and so allows each command a
 * unique representation.
 *
 * See defines.h for a list of the codes used.
 */
void do_cmd_cli()
{
	char buff[80];

	cli_comm *cli_ptr;

	/* Clear the input buffer */
	strcpy(buff, "");

	/* Accept command */
	if (!get_string_cli("Command: ", buff, 30)) return;


	/* Analyse the input */
	for (cli_ptr = cli_info; cli_ptr->comm; cli_ptr++)
	{
		if (equals(buff, cli_ptr->comm))
		{
			/* Process the command without keymaps or macros. */
			command_new = cli_ptr->key;
			return;
		}
	}

	msg_format("No such command: %s", buff);
}


/*
 * Display on-line help for the CLI commands
 */
void do_cmd_cli_help()
{
	fmt::MemoryWriter w;
	for (int i = 0, j = -1; i < cli_total; i++)
	{
		if (j < i - 1)
		{
			w << "/";
		}

		w.write("[[[[[G{}]", cli_info[i].comm);

		if (cli_info[i].descrip != cli_info[i + 1].descrip)
		{
			w.write("   {}\n", cli_info[i].descrip);
			j = i;
		}
	}

	/* Save the screen */
	screen_save_no_flush();

	/* Display the file contents */
	show_string(w.c_str(), "Command line help");

	/* Restore the screen */
	screen_load_no_flush();
}


/*
 * Dump screen shot in HTML
 */
void do_cmd_html_dump()
{
	char tmp_val[81];
	bool html = true;
	term_win *save;

	/* Save the screen */
	save = Term_save_to();

	if (wizard && get_check("WIZARD MODE: Do an help file dump?"))
		html = false;

	/* Ask for a file */
	if (html)
	{
		strcpy(tmp_val, "dummy.htm");
		if (!get_string("File(you can post it to http://angband.oook.cz/): ", tmp_val, 80))
		{
			/* Now restore the screen to initial state */
			Term_load_from(save);
			Term_fresh();
			return;
		}
	}
	else
	{
		strcpy(tmp_val, "dummy.txt");
		if (!get_string("File: ", tmp_val, 80))
		{
			/* Now restore the screen to initial state */
			Term_load_from(save);
			Term_fresh();
			return;
		}
	}

	/* Now restore the screen to dump it */
	Term_load_from(save);

	if (html)
		html_screenshot(tmp_val);
	else
		help_file_screenshot(tmp_val);

	Term_erase(0, 0, 255);
	msg_print("Dump saved.");
	Term_fresh();
	fix_message();
}
