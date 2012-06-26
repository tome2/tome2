/* File: cmd3.c */

/* Purpose: Inventory commands */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#include "quark.h"

/*
 * Display p_ptr->inventory
 */
void do_cmd_inven(void)
{
	char out_val[160];


	/* Note that we are in "p_ptr->inventory" mode */
	command_wrk = FALSE;

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the p_ptr->inventory */
	show_inven();

	/* Hack -- hide empty slots */
	item_tester_full = FALSE;


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
	Term_load();
	character_icky = FALSE;


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
		request_command_inven_mode = TRUE;
	}
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
	char out_val[160];


	/* Note that we are in "equipment" mode */
	command_wrk = TRUE;

	/* Save the screen */
	character_icky = TRUE;
	Term_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the equipment */
	show_equip();

	/* Hack -- undo the hack above */
	item_tester_full = FALSE;

	/* Build a prompt */
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
	Term_load();
	character_icky = FALSE;


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
		request_command_inven_mode = TRUE;
	}
}


/*
 * The "wearable" tester
 */
static bool_ item_tester_hook_wear(object_type *o_ptr)
{
	u32b f1, f2, f3, f4, f5, esp;
	int slot = wield_slot(o_ptr);


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Only one ultimate at a time */
	if (f4 & TR4_ULTIMATE)
	{
		int i;

		for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
		{
			object_type *q_ptr = &p_ptr->inventory[i];

			/* Extract the flags */
			object_flags(q_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

			if (!q_ptr->k_idx) continue;

			if (f4 & TR4_ULTIMATE) return (FALSE);
		}
	}

	if ((slot < INVEN_WIELD) || ((p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_WIELD) && (p_ptr->melee_style != SKILL_MASTERY)))
		return (FALSE);

	/* Check for a usable slot */
	if (slot >= INVEN_WIELD) return (TRUE);

	/* Assume not wearable */
	return (FALSE);
}


bool_ is_slot_ok(int slot)
{
	if ((slot >= INVEN_WIELD) && (slot < INVEN_TOTAL))
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}


/*
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_wield(void)
{
	int item, slot, num = 1;

	object_type forge;

	object_type *q_ptr;

	object_type *o_ptr, *i_ptr;

	cptr act;

	char o_name[80];

	cptr q, s;

	u32b f1, f2, f3, f4, f5, esp;


	/* Restrict the choices */
	item_tester_hook = item_tester_hook_wear;

	/* Get an item */
	q = "Wear/Wield which item? ";
	s = "You have nothing you can wear or wield.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Check the slot */
	slot = wield_slot(o_ptr);

	/* Prevent wielding into a cursed slot */
	if (cursed_p(&p_ptr->inventory[slot]))
	{
		/* Describe it */
		object_desc(o_name, &p_ptr->inventory[slot], FALSE, 0);

		/* Message */
		msg_format("The %s you are %s appears to be cursed.",
		           o_name, describe_use(slot));

		/* Cancel the command */
		return;
	}

	if ((cursed_p(o_ptr)) && (wear_confirm)
	                && (object_known_p(o_ptr) || (o_ptr->ident & (IDENT_SENSE))))
	{
		char dummy[512];

		/* Describe it */
		object_desc(o_name, o_ptr, FALSE, 0);

		strnfmt(dummy, 512, "Really use the %s {cursed}? ", o_name);
		if (!(get_check(dummy)))
			return;
	}

	/* Can we wield */
	if (process_hooks(HOOK_WIELD, "(d)", item)) return;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Two handed weapons can't be wielded with a shield */
	if ((is_slot_ok(slot - INVEN_WIELD + INVEN_ARM)) &&
	                (f4 & TR4_MUST2H) &&
	                (p_ptr->inventory[slot - INVEN_WIELD + INVEN_ARM].k_idx != 0))
	{
		object_desc(o_name, o_ptr, FALSE, 0);
		msg_format("You cannot wield your %s with a shield.", o_name);
		return;
	}

	if (is_slot_ok(slot - INVEN_ARM + INVEN_WIELD))
	{
		i_ptr = &p_ptr->inventory[slot - INVEN_ARM + INVEN_WIELD];

		/* Extract the flags */
		object_flags(i_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

		/* Prevent shield from being put on if wielding 2H */
		if ((f4 & TR4_MUST2H) && (i_ptr->k_idx) &&
		                (p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_ARM))
		{
			object_desc(o_name, o_ptr, FALSE, 0);
			msg_format("You cannot wield your %s with a two-handed weapon.", o_name);
			return;
		}

		if ((p_ptr->body_parts[slot - INVEN_WIELD] == INVEN_ARM) &&
		                (f4 & TR4_COULD2H))
		{
			if (!get_check("Are you sure you want to restrict your fighting? "))
			{
				return;
			}
		}
	}


	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if ((is_slot_ok(slot - INVEN_WIELD + INVEN_ARM)) &&
	                (p_ptr->inventory[slot - INVEN_WIELD + INVEN_ARM].k_idx != 0) &&
	                (f4 & TR4_COULD2H))
	{
		if (!get_check("Are you sure you want to use this weapon with a shield?"))
		{
			return;
		}
	}

	/* Can we take off existing item */
	if (slot != INVEN_AMMO)
	{
		if (p_ptr->inventory[slot].k_idx)
			if (process_hooks(HOOK_TAKEOFF, "(d)", slot)) return;
	}
	else
	{
		if (p_ptr->inventory[slot].k_idx)
			if (!object_similar(&p_ptr->inventory[slot], o_ptr))
				if (process_hooks(HOOK_TAKEOFF, "(d)", slot)) return;
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
		if (o_ptr->k_idx)
		{
			/* Take off existing item */
			(void)inven_takeoff(slot, 255, FALSE);
		}
	}
	else
	{
		if (o_ptr->k_idx)
		{
			if (!object_similar(o_ptr, q_ptr))
			{
				/* Take off existing item */
				(void)inven_takeoff(slot, 255, FALSE);
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
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("%s %s (%c).", act, o_name, index_to_label(slot));

	/* Cursed! */
	if (cursed_p(o_ptr))
	{
		/* Warn the player */
		msg_print("Oops! It feels deathly cold!");

		/* Note the curse */
		o_ptr->ident |= (IDENT_SENSE);
		o_ptr->sense = SENSE_CURSED;
	}

	/* Take care of item sets */
	if (o_ptr->name1)
	{
		wield_set(o_ptr->name1, a_info[o_ptr->name1].set, FALSE);
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
	p_ptr->redraw |= (PR_MH);

	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
}



/*
 * Take off an item
 */
void do_cmd_takeoff(void)
{
	int item;

	object_type *o_ptr;

	cptr q, s;


	/* Get an item */
	q = "Take off which item? ";
	s = "You are not wearing anything to take off.";
	if (!get_item(&item, q, s, (USE_EQUIP))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Can we take it off */
	if (process_hooks(HOOK_TAKEOFF, "(d)", item)) return;

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
	(void)inven_takeoff(item, 255, FALSE);

	/* Recalculate hitpoint */
	p_ptr->update |= (PU_HP);

	p_ptr->redraw |= (PR_MH);
}


/*
 * Drop an item
 */
void do_cmd_drop(void)
{
	int item, amt = 1;

	object_type *o_ptr;

	u32b f1, f2, f3, f4, f5, esp;

	cptr q, s;


	/* Get an item */
	q = "Drop which item? ";
	s = "You have nothing to drop.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN))) return;

	/* Get the item */
	o_ptr = get_object(item);

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	/* Can we drop */
	if (process_hooks(HOOK_DROP, "(d)", item)) return;

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
			if (f4 & TR4_CURSE_NO_DROP)
			{
				/* Oops */
				msg_print("Hmmm, you seem to be unable to drop it.");

				/* Nope */
				return;
			}
		}
	}


	/* See how many items */
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
	inven_drop(item, amt, p_ptr->py, p_ptr->px, FALSE);
}


/*
 * Destroy an item
 */
void do_cmd_destroy(void)
{
	int item, amt = 1;

	int old_number;

	bool_ force = FALSE;

	object_type *o_ptr;

	char o_name[80];

	char out_val[160];

	cptr q, s;

	u32b f1, f2, f3, f4, f5, esp;


	/* Hack -- force destruction */
	if (command_arg > 0) force = TRUE;


	/* Get an item */
	q = "Destroy which item? ";
	s = "You have nothing to destroy.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_AUTO))) return;

	/* Get the item */
	o_ptr = get_object(item);


	/* See how many items */
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
	object_desc(o_name, o_ptr, TRUE, 3);
	o_ptr->number = old_number;

	/* Verify unless quantity given */
	if (!force)
	{
		if (!((auto_destroy) && (object_value(o_ptr) < 1)))
		{
			/* Make a verification */
			strnfmt(out_val, 160, "Really destroy %s? ", o_name);
			if (!get_check(out_val)) return;
		}
	}

	/* Take no time, just like the automatizer */
	energy_use = 0;

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if ((f4 & TR4_CURSE_NO_DROP) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, you seem to be unable to destroy it.");

		/* Nope */
		return;
	}


	/* Artifacts cannot be destroyed */
	if (artifact_p(o_ptr) || o_ptr->art_name)
	{
		byte feel = SENSE_SPECIAL;

		energy_use = 0;

		/* Message */
		msg_format("You cannot destroy %s.", o_name);

		/* Hack -- Handle icky artifacts */
		if (cursed_p(o_ptr)) feel = SENSE_TERRIBLE;

		/* Hack -- inscribe the artifact */
		o_ptr->sense = feel;

		/* We have "felt" it (again) */
		o_ptr->ident |= (IDENT_SENSE);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Done */
		return;
	}

	/* Message */
	msg_format("You destroy %s.", o_name);
	sound(SOUND_DESTITEM);

	/* Create an automatizer rule */
	if (automatizer_create)
	{
		automatizer_add_rule(o_ptr, TRUE);
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
	if (f3 & TR3_BLESSED)
		inc_piety(GOD_ERU, -10 * k_info[o_ptr->k_idx].level);

	/* Eliminate the item */
	inc_stack_size(item, -amt);
}


/*
 * Observe an item which has been *identify*-ed
 */
void do_cmd_observe(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	cptr q, s;


	/* Get an item */
	q = "Examine which item? ";
	s = "You have nothing to examine.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Description */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Describe */
	cmsg_format(TERM_L_BLUE, "%s", o_name);

	/* Describe it fully */
	if (!object_out_desc(o_ptr, NULL, FALSE, TRUE)) msg_print("You see nothing special.");
}



/*
 * Remove the inscription from an object
 * XXX Mention item (when done)?
 */
void do_cmd_uninscribe(void)
{
	int item;

	object_type *o_ptr;

	cptr q, s;


	/* Get an item */
	q = "Un-inscribe which item? ";
	s = "You have nothing to un-inscribe.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Nothing to remove */
	if (!o_ptr->note)
	{
		msg_print("That item had no inscription to remove.");
		return;
	}

	/* Message */
	msg_print("Inscription removed.");

	/* Remove the incription */
	o_ptr->note = 0;

	/* Combine the pack */
	p_ptr->notice |= (PN_COMBINE);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
}


/*
 * Inscribe an object with a comment
 */
void do_cmd_inscribe(void)
{
	int item;

	object_type *o_ptr;

	char o_name[80];

	char out_val[80];

	cptr q, s;


	/* Get an item */
	q = "Inscribe which item? ";
	s = "You have nothing to inscribe.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Describe the activity */
	object_desc(o_name, o_ptr, TRUE, 3);

	/* Message */
	msg_format("Inscribing %s.", o_name);
	msg_print(NULL);

	/* Start with nothing */
	strcpy(out_val, "");

	/* Use old inscription */
	if (o_ptr->note)
	{
		/* Start with the old inscription */
		strcpy(out_val, quark_str(o_ptr->note));
	}

	/* Get a new inscription (possibly empty) */
	if (get_string("Inscription: ", out_val, 80))
	{
		/* Save the inscription */
		o_ptr->note = quark_add(out_val);

		/* Combine the pack */
		p_ptr->notice |= (PN_COMBINE);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);
	}
}



/*
 * An "item_tester_hook" for refilling lanterns
 */
static bool_ item_tester_refill_lantern(object_type *o_ptr)
{
	/* Flasks of oil are okay */
	if (o_ptr->tval == TV_FLASK) return (TRUE);

	/* Lanterns are okay */
	if ((o_ptr->tval == TV_LITE) &&
	                (o_ptr->sval == SV_LITE_LANTERN)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refill the players lamp (from the pack or floor)
 */
static void do_cmd_refill_lamp(void)
{
	int item;

	object_type *o_ptr;
	object_type *j_ptr;

	cptr q, s;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_lantern;

	/* Get an item */
	q = "Refill with which flask? ";
	s = "You have no flasks of oil.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Take a partial turn */
	energy_use = 50;

	/* Access the lantern */
	j_ptr = &p_ptr->inventory[INVEN_LITE];

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
static bool_ item_tester_refill_torch(object_type *o_ptr)
{
	/* Torches are okay */
	if ((o_ptr->tval == TV_LITE) &&
	                (o_ptr->sval == SV_LITE_TORCH)) return (TRUE);

	/* Assume not okay */
	return (FALSE);
}


/*
 * Refuel the players torch (from the pack or floor)
 */
static void do_cmd_refill_torch(void)
{
	int item;

	object_type *o_ptr;

	object_type *j_ptr;

	cptr q, s;


	/* Restrict the choices */
	item_tester_hook = item_tester_refill_torch;

	/* Get an item */
	q = "Refuel with which torch? ";
	s = "You have no extra torches.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item */
	o_ptr = get_object(item);

	/* Take a partial turn */
	energy_use = 50;

	/* Access the primary torch */
	j_ptr = &p_ptr->inventory[INVEN_LITE];

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
void do_cmd_refill(void)
{
	object_type *o_ptr;

	u32b f1, f2, f3, f4, f5, esp;


	/* Get the light */
	o_ptr = &p_ptr->inventory[INVEN_LITE];

	/* It is nothing */
	if (o_ptr->tval != TV_LITE)
	{
		msg_print("You are not wielding a light.");
		return;
	}

	object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

	if (f4 & TR4_FUEL_LITE)
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
void do_cmd_target(void)
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
void do_cmd_look(void)
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
void do_cmd_locate(void)
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
	while (1)
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
static cptr ident_info[] =
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



/*
 * Sorting hook -- Comp function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform on "u".
 */
static bool_ ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b *why = (u16b*)(v);

	int w1 = who[a];

	int w2 = who[b];

	int z1, z2;


	/* Sort by player kills */
	if (*why >= 4)
	{
		/* Extract player kills */
		z1 = r_info[w1].r_pkills;
		z2 = r_info[w2].r_pkills;

		/* Compare player kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by total kills */
	if (*why >= 3)
	{
		/* Extract total kills */
		z1 = r_info[w1].r_tkills;
		z2 = r_info[w2].r_tkills;

		/* Compare total kills */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster level */
	if (*why >= 2)
	{
		/* Extract levels */
		z1 = r_info[w1].level;
		z2 = r_info[w2].level;

		/* Compare levels */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Sort by monster experience */
	if (*why >= 1)
	{
		/* Extract experience */
		z1 = r_info[w1].mexp;
		z2 = r_info[w2].mexp;

		/* Compare experience */
		if (z1 < z2) return (TRUE);
		if (z1 > z2) return (FALSE);
	}


	/* Compare indexes */
	return (w1 <= w2);
}


/*
 * Sorting hook -- Swap function -- see below
 *
 * We use "u" to point to array of monster indexes,
 * and "v" to select the type of sorting to perform.
 */
static void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
	u16b *who = (u16b*)(u);

	u16b holder;


	/* XXX XXX */
	v = v ? v : 0;

	/* Swap */
	holder = who[a];
	who[a] = who[b];
	who[b] = holder;
}



/*
 * Hack -- Display the "name" and "attr/chars" of a monster race
 */
static void roff_top(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

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
	if (!(r_ptr->flags1 & (RF1_UNIQUE)))
	{
		Term_addstr( -1, TERM_WHITE, "The ");
	}

	/* Dump the name */
	Term_addstr( -1, TERM_WHITE, (r_name + r_ptr->name));

	/* Append the "standard" attr/char info */
	Term_addstr( -1, TERM_WHITE, " ('");
	Term_addch(a1, c1);
	if (use_bigtile && (a1 & 0x80)) Term_addch(255, 255);
	Term_addstr( -1, TERM_WHITE, "')");

	/* Append the "optional" attr/char info */
	Term_addstr( -1, TERM_WHITE, "/('");
	Term_addch(a2, c2);
	if (use_bigtile && (a2 & 0x80)) Term_addch(255, 255);
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
void do_cmd_query_symbol(void)
{
	int i, n, r_idx;

	char sym, query;

	char buf[128];


	bool_ all = FALSE;

	bool_ uniq = FALSE;

	bool_ norm = FALSE;


	bool_ name = FALSE;

	char temp[80] = "";


	bool_ recall = FALSE;


	u16b why = 0;

	u16b *who;


	/* Get a character, or abort */
	if (!get_com("Enter character to be identified, "
	                "or (Ctrl-A, Ctrl-U, Ctrl-N, Ctrl-M):", &sym)) return;

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	/* Describe */
	if (sym == KTRL('A'))
	{
		all = TRUE;
		strcpy(buf, "Full monster list.");
	}
	else if (sym == KTRL('U'))
	{
		all = uniq = TRUE;
		strcpy(buf, "Unique monster list.");
	}
	else if (sym == KTRL('N'))
	{
		all = norm = TRUE;
		strcpy(buf, "Non-unique monster list.");
	}
	else if (sym == KTRL('M'))
	{
		all = name = TRUE;
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

	/* Allocate the "who" array */
	C_MAKE(who, max_r_idx, u16b);

	/* Collect matching monsters */
	for (n = 0, i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Nothing to recall */
		if (!cheat_know && !r_ptr->r_sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Require unique monsters if needed */
		if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Require monsters with the name requested if needed */
		if (name)
		{
			char mon_name[80];

			strcpy(mon_name, r_name + r_ptr->name);
			strlower(mon_name);

			if (!strstr(mon_name, temp)) continue;
		}

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n)
	{
		/* Free the "who" array */
		C_KILL(who, max_r_idx, u16b);

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
		why = 4;
		query = 'y';
	}

	/* Sort by level */
	if (query == 'p')
	{
		why = 2;
		query = 'y';
	}

	/* Catch "escape" */
	if (query != 'y')
	{
		/* Free the "who" array */
		C_KILL(who, max_r_idx, u16b);

		return;
	}


	/* Sort if needed */
	if (why)
	{
		/* Select the sort method */
		ang_sort_comp = ang_sort_comp_hook;
		ang_sort_swap = ang_sort_swap_hook;

		/* Sort the array */
		ang_sort(who, &why, n);
	}


	/* Start at the end */
	i = n - 1;

	/* Scan the monster memory */
	while (1)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx, 0);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr( -1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save the screen */
				character_icky = TRUE;
				Term_save();

				/* Recall on screen */
				screen_roff(who[i], 0, 0);

				/* Hack -- Complete the prompt (again) */
				Term_addstr( -1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Restore */
				Term_load();
				character_icky = FALSE;
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
			if (++i == n)
			{
				i = 0;
				if (!expand_list) break;
			}
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
			{
				i = n - 1;
				if (!expand_list) break;
			}
		}
	}

	/* Re-display the identity */
	prt(buf, 0, 0);

	/* Free the "who" array */
	C_KILL(who, max_r_idx, u16b);
}


/*
 *  research_mon
 *  -KMW-
 */
bool_ research_mon()
{
	int i, n, r_idx;

	char sym, query;

	char buf[128];


	s16b oldkills;

	byte oldwake;

	bool_ oldcheat;


	bool_ all = FALSE;

	bool_ uniq = FALSE;

	bool_ norm = FALSE;

	bool_ notpicked;


	bool_ recall = FALSE;

	u16b why = 0;

	monster_race *r2_ptr;

	u16b *who;


	/* Hack -- Remember "cheat_know" flag */
	oldcheat = cheat_know;


	/* Get a character, or abort */
	if (!get_com("Enter character of monster: ", &sym)) return (TRUE);

	/* Allocate the "who" array */
	C_MAKE(who, max_r_idx, u16b);

	/* Find that character info, and describe it */
	for (i = 0; ident_info[i]; ++i)
	{
		if (sym == ident_info[i][0]) break;
	}

	if (ident_info[i])
	{
		strnfmt(buf, 128, "%c - %s.", sym, ident_info[i] + 2);
	}
	else
	{
		strnfmt(buf, 128, "%c - %s.", sym, "Unknown Symbol");
	}

	/* Display the result */
	prt(buf, 16, 10);


	/* Collect matching monsters */
	for (n = 0, i = 1; i < max_r_idx; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Hack -- Force "cheat_know" */
		cheat_know = TRUE;

		/* Nothing to recall */
		if (!cheat_know && !r_ptr->r_sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && (r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Require unique monsters if needed */
		if (uniq && !(r_ptr->flags1 & (RF1_UNIQUE))) continue;

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == sym)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n)
	{
		/* Free the "who" array */
		C_KILL(who, max_r_idx, u16b);

		/* Restore the "cheat_know" flag */
		cheat_know = oldcheat;

		return (TRUE);
	}


	/* Sort by level */
	why = 2;
	query = 'y';

	/* Sort if needed */
	if (why)
	{
		/* Select the sort method */
		ang_sort_comp = ang_sort_comp_hook;
		ang_sort_swap = ang_sort_swap_hook;

		/* Sort the array */
		ang_sort(who, &why, n);
	}


	/* Start at the end */
	i = n - 1;

	notpicked = TRUE;

	/* Scan the monster memory */
	while (notpicked)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx, 0);

		/* Hack -- Handle stuff */
		handle_stuff();

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr( -1, TERM_WHITE, " [(r)ecall, ESC, space to continue]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save the screen */
				character_icky = TRUE;
				Term_save();

				/* Recall on screen */
				r2_ptr = &r_info[r_idx];

				oldkills = r2_ptr->r_tkills;
				oldwake = r2_ptr->r_wake;
				screen_roff(who[i], 0, 1);
				r2_ptr->r_tkills = oldkills;
				r2_ptr->r_wake = oldwake;
				r2_ptr->r_sights = 1;
				cheat_know = oldcheat;
				notpicked = FALSE;
				break;

			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Restore */
				Term_load();
				character_icky = FALSE;
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
			if (++i == n)
			{
				i = 0;
				if (!expand_list) break;
			}
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
			{
				i = n - 1;
				if (!expand_list) break;
			}
		}
	}


	/* Re-display the identity */
	/* prt(buf, 5, 5);*/

	/* Free the "who" array */
	C_KILL(who, max_r_idx, u16b);

	/* Restore the "cheat_know" flag */
	cheat_know = oldcheat;

	return (notpicked);
}


/*
 * Try to "sense" the grid's mana
 */
bool_ do_cmd_sense_grid_mana()
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
		if (flush_failure) flush();
		msg_print("You failed to sense the grid's mana.");
		sound(SOUND_FAIL);
		return FALSE;
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
	return TRUE;
}


/*
 * Calculate the weight of the portable holes
 */
s32b portable_hole_weight(void)
{
	s32b weight, i;

	store_type *st_ptr = &town_info[TOWN_RANDOM].store[STORE_HOME];


	/* Sum the objects in the appropriate home */
	for (i = 0, weight = 0; i < st_ptr->stock_num; i++)
	{
		object_type *o_ptr = &st_ptr->stock[i];

		weight += (o_ptr->weight * o_ptr->number);
	}

	/* Multiply the sum with 1.5 */
	weight = (weight * 3) / 2 + 2;

	return (weight);
}


/*
 * Calculate and set the weight of the portable holes
 */
void set_portable_hole_weight(void)
{
	s32b weight, i, j;

	/* Calculate the weight of items in home */
	weight = portable_hole_weight();

	/* Set the weight of portable holes in the shops, ... */
	for (i = 1; i < max_towns; i++)
	{
		for (j = 0; j < max_st_idx; j++)
		{
			store_type *st_ptr = &town_info[i].store[j];
			int k;

			for (k = 0; k < st_ptr->stock_num; k++)
			{
				object_type *o_ptr = &st_ptr->stock[k];

				if ((o_ptr->tval == TV_TOOL) &&
				                (o_ptr->sval == SV_PORTABLE_HOLE))
					o_ptr->weight = weight;
			}
		}
	}

	/* ... in the object list, ... */
	for (i = 1; i < o_max; i++)
	{
		object_type *o_ptr = &o_list[i];

		if ((o_ptr->tval == TV_TOOL) &&
		                (o_ptr->sval == SV_PORTABLE_HOLE)) o_ptr->weight = weight;
	}

	/* ... and in the p_ptr->inventory to the appropriate value */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if ((o_ptr->tval == TV_TOOL) &&
		                (o_ptr->sval == SV_PORTABLE_HOLE)) o_ptr->weight = weight;
	}
}


/*
 * Use a portable hole
 */
void do_cmd_portable_hole(void)
{
	cave_type *c_ptr = &cave[p_ptr->py][p_ptr->px];

	int feat, special, town_num;

	/* Is it currently wielded? */
	if (!p_ptr->inventory[INVEN_TOOL].k_idx ||
	                (p_ptr->inventory[INVEN_TOOL].tval != TV_TOOL) ||
	                (p_ptr->inventory[INVEN_TOOL].sval != SV_PORTABLE_HOLE))
	{
		/* No, it isn't */
		msg_print("You have to wield a portable hole to use your abilities");
		return;
	}

	/* Mega-hack: Saving the old values, and then... */
	feat = c_ptr->feat;
	special = c_ptr->special;
	town_num = p_ptr->town_num;

	/* ... change the current grid to the home in town #1 */
	/* DG -- use the first random town, since random towns cannot have houses */
	/*
	 * pelpel -- This doesn't affect LoS, so we can manipulate
	 * terrain feature without calling cave_set_feat()
	 */
	c_ptr->feat = FEAT_SHOP;
	c_ptr->special = STORE_HOME;
	p_ptr->town_num = TOWN_RANDOM;

	/* Now use the portable hole */
	do_cmd_store();

	/* Mega-hack part II: change the current grid to the original value */
	c_ptr->feat = feat;
	c_ptr->special = special;
	p_ptr->town_num = town_num;

	set_portable_hole_weight();

	/* Recalculate bonuses */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);
	p_ptr->update |= (PU_BONUS);
}


/*
 * Try to add a CLI action.
 */
void cli_add(cptr active, cptr trigger, cptr descr)
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
		cptr s;
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
		cli_ptr->comm = string_make(temp);
	}
	else
	{
		cli_ptr->comm = string_make(trigger);
	}

	/* First try copying everything across. */
	cli_ptr->key = num;
	cli_ptr->descrip = string_make(descr);

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
bool_ get_string_cli(cptr prompt, char *buf, int len)
{
	bool_ res;


	/* Paranoia XXX XXX XXX */
	msg_print(NULL);

	/* Display prompt */
	prt(prompt, 0, 0);

	/* Ask the user for a string */
	askfor_aux_complete = TRUE;
	res = askfor_aux(buf, len);
	askfor_aux_complete = FALSE;

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
void do_cmd_cli(void)
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
		if (!strcmp(buff, cli_ptr->comm))
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
	int i, j;

	FILE *fff;

	char file_name[1024];


	/* Temporary file */
	if (path_temp(file_name, 1024)) return;

	/* Open a new file */
	fff = my_fopen(file_name, "w");

	for (i = 0, j = -1; i < cli_total; i++)
	{
		if (j < i - 1) fprintf(fff, "/");
		fprintf(fff, "[[[[[G%s]", cli_info[i].comm);
		if (cli_info[i].descrip != cli_info[i + 1].descrip)
		{
			fprintf(fff, "   %s\n", cli_info[i].descrip);
			j = i;
		}
	}

	/* Close the file */
	my_fclose(fff);

	/* Enter "icky" mode */
	character_icky = TRUE;

	/* Save the screen */
	Term_save();

	/* Display the file contents */
	show_file(file_name, "Command line help", 0, 0);

	/* Restore the screen */
	Term_load();

	/* Leave "icky" mode */
	character_icky = FALSE;

	/* Remove the file */
	fd_kill(file_name);
}


/*
 * Dump screen shot in HTML
 */
void do_cmd_html_dump()
{
	char tmp_val[81];
	bool_ html = TRUE;
	term_win *save;

	/* Save the screen */
	save = Term_save_to();

	if (wizard && get_check("WIZARD MODE: Do an help file dump?"))
		html = FALSE;

	/* Ask for a file */
	if (html)
	{
		strcpy(tmp_val, "dummy.htm");
		if (!get_string("File(you can post it to http://angband.oook.cz/): ", tmp_val, 80))
		{
			/* Now restore the screen to initial state */
			Term_load_from(save, TRUE);
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
			Term_load_from(save, TRUE);
			Term_fresh();
			return;
		}
	}

	/* Now restore the screen to dump it */
	Term_load_from(save, TRUE);

	if (html)
		html_screenshot(tmp_val);
	else
		help_file_screenshot(tmp_val);

	Term_erase(0, 0, 255);
	msg_print("Dump saved.");
	Term_fresh();
	fix_message();
}
