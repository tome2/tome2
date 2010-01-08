/* File: squeltch.c */

/* Purpose: Automatizer */

/*
 * Copyright (c) 2002 DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "lua/lua.h"
#include "tolua.h"

extern lua_State *L;

/*
 * The functions here use direct lua stack manipulation for calls instead of
 * exec_lua(format()) because string manipulations are too slow for such
 * functions
 */

/* Check the floor for "crap" */
void squeltch_grid(void)
{
	int oldtop;
	s16b this_o_idx, next_o_idx = 0;

	if (!automatizer_enabled) return;

	oldtop = lua_gettop(L);

	/* Scan the pile of objects */
	for (this_o_idx = cave[p_ptr->py][p_ptr->px].o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		/* Acquire object */
		object_type * o_ptr = &o_list[this_o_idx];

		/* We've now seen one of these */
		if (!k_info[o_ptr->k_idx].flavor)
		{
			object_aware(o_ptr);
		}

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Push the function */
		lua_settop(L, oldtop);
		lua_getglobal(L, "apply_rules");

		/* Push the args */
		tolua_pushusertype(L, o_ptr, tolua_tag(L, "object_type"));
		tolua_pushnumber(L, -this_o_idx);

		/* Call the function */
		if (lua_call(L, 2, 0))
		{
			cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling 'apply_rules'.");
			lua_settop(L, oldtop);
			return;
		}
	}
	lua_settop(L, oldtop);
}


/* Check the inventory for "crap" */
void squeltch_inventory(void)
{
	int oldtop;
	int i;
	int num_iter = 0;
	bool found = TRUE;

	if (!automatizer_enabled) return;

	oldtop = lua_gettop(L);
	while (found && num_iter ++ < 100)
	{
		/* Sometimes an index in the inventory is skipped */
		found = FALSE;

		for (i = 0; i < INVEN_PACK; i++)
		{
			object_type *o_ptr = &p_ptr->inventory[i];

			/* Push the function */
			lua_settop(L, oldtop);
			lua_getglobal(L, "apply_rules");

			/* Push the args */
			tolua_pushusertype(L, o_ptr, tolua_tag(L, "object_type"));
			tolua_pushnumber(L, i);

			/* Call the function */
			if (lua_call(L, 2, 1))
			{
				cmsg_format(TERM_VIOLET, "ERROR in lua_call while calling 'apply_rules'.");
				lua_settop(L, oldtop);
				return;
			}

			/* Did it return TRUE */
			if (tolua_getnumber(L, -(lua_gettop(L) - oldtop), 0))
			{
				found = TRUE;
				break;
			}
		}
	}
	if (num_iter >= 100)
	{
		cmsg_format(TERM_VIOLET, "'apply_rules' ran too often.");
	}
	lua_settop(L, oldtop);
}

/********************** The interface **********************/
static cptr *get_rule_list(int *max)
{
	cptr *list;
	int i;

	*max = exec_lua("return __rules_max");
	C_MAKE(list, *max, cptr);

	for (i = 0; i < *max; i++)
	{
		list[i] = string_exec_lua(format("return __rules[%d].table.args.name", i));
	}

	return list;
}

static cptr types_list[] =
{
	"and",
	"or",
	"not",
	"name",
	"contain",
	"inscribed",
	"discount",
	"symbol",
	"state",
	"status",
	"tval",
	"sval",
	"race",
	"subrace",
	"class",
	"level",
	"skill",
	"ability",
	"comment",
	NULL,
};

/* Create a new rule */
static int automatizer_new_rule()
{
	int wid, hgt, max, begin = 0, sel = 0;
	char c;

	/* Get the number of types */
	max = 0;
	while (types_list[max] != NULL)
		max++;

	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		display_list(0, 0, hgt - 1, 15, "Rule types", types_list, max, begin, sel, TERM_L_GREEN);

		exec_lua(format("auto_aux:display_desc('%s')", types_list[sel]));
		c_prt(TERM_YELLOW, "Example:", 5, 17);
		exec_lua(format("xml.write_out_y = 6; xml.write_out_x = 16; xml.write_out_h = %d; xml.write_out_w = %d; xml.write_y = 0; xml.write_x = 0; xml:display_xml(auto_aux.types_desc['%s'][2][1], '')", hgt - 3 - 5, wid - 1 - 15 - 1, types_list[sel]));

		c = inkey();

		if (c == ESCAPE) break;
		else if (c == '8')
		{
			sel--;
			if (sel < 0)
			{
				sel = max - 1;
				begin = max - hgt;
				if (begin < 0) begin = 0;
			}
			if (sel < begin) begin = sel;
		}
		else if (c == '2')
		{
			sel++;
			if (sel >= max)
			{
				sel = 0;
				begin = 0;
			}
			if (sel >= begin + hgt - 1) begin++;
		}
		else if (c == '\r')
		{
			return sel;
		}
	}
	return -1;
}

static void adjust_begin(int *begin, int *sel, int max, int hgt)
{
	if (*sel < 0)
	{
		*sel = max - 1;
		*begin = *sel - hgt + 3;
		if (*begin < 0) *begin = 0;
	}
	if (*sel < *begin) *begin = *sel;

	if (*sel >= max)
	{
		*sel = 0;
		*begin = 0;
	}
	if (*sel >= *begin + hgt - 2) (*begin)++;
}

#define ACTIVE_LIST     0
#define ACTIVE_RULE     1
void do_cmd_automatizer()
{
	int wid = 0, hgt = 0;
	char c;
	cptr *list = NULL;
	int max, begin = 0, sel = 0;
	int active = ACTIVE_LIST;
	cptr keys;
	cptr keys2;
	cptr keys3;

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

	exec_lua(format("auto_aux:adjust_current(%d)", sel));

	while (1)
	{
		Term_clear();
		Term_get_size(&wid, &hgt);

		list = get_rule_list(&max);
		display_list(0, 0, hgt - 1, 15, "Rules", list, max, begin, sel, (active == ACTIVE_LIST) ? TERM_L_GREEN : TERM_GREEN);
		C_FREE(list, max, cptr);

		draw_box(0, 15, hgt - 4, wid - 1 - 15);
		if (active == ACTIVE_RULE)
		{
			keys = "#Bup#W/#Bdown#W/#Bleft#W/#Bright#W to navitage rule, #B9#W/#B3#W/#B7#W/#B1#W to scroll";
			keys2 = "#Btab#W for switch, #Ba#Wdd clause, #Bd#Welete clause/rule";
			keys3 = "#Bx#W to toggle english/xml, #G?#W for Automatizer help";
			exec_lua("xml.write_active = not nil");
		}
		else
		{
			keys = "#Bup#W/#Bdown#W to scroll, #Btab#W to switch to the rule window";
			keys2 = "#Bu#W/#Bd#W to move rules, #Bn#Wew rule, #Br#Wename rule, #Bs#Wave rules";
			keys3 = "#Rk#W to #rdisable#W the automatizer, #G?#W for Automatizer help";
			exec_lua("xml.write_active = nil");
		}
		display_message(17, hgt - 3, strlen(keys), TERM_WHITE, keys);
		display_message(17, hgt - 2, strlen(keys2), TERM_WHITE, keys2);
		display_message(17, hgt - 1, strlen(keys3), TERM_WHITE, keys3);

		if (max) exec_lua(format("xml.write_out_y = 1; xml.write_out_x = 16; xml.write_out_h = %d; xml.write_out_w = %d; xml.write_y = 0; xml.write_x = 0; xml:display_xml(__rules[%d].table, '')", hgt - 4 - 1, wid - 1 - 15 - 1, sel));

		c = inkey();

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
				if (!max) continue;
				sel--;
				adjust_begin(&begin, &sel, max, hgt);
				exec_lua(format("auto_aux:adjust_current(%d)", sel));
			}
			else if (c == '2')
			{
				if (!max) continue;
				sel++;
				adjust_begin(&begin, &sel, max, hgt);
				exec_lua(format("auto_aux:adjust_current(%d)", sel));
			}
			else if (c == 'u')
			{
				if (!max) continue;
				sel = exec_lua(format("return auto_aux:move_up(%d)", sel));
				adjust_begin(&begin, &sel, max, hgt);
				exec_lua(format("auto_aux:adjust_current(%d)", sel));
			}
			else if (c == 'd')
			{
				if (!max) continue;
				sel = exec_lua(format("return auto_aux:move_down(%d)", sel));
				adjust_begin(&begin, &sel, max, hgt);
				exec_lua(format("auto_aux:adjust_current(%d)", sel));
			}
			else if (c == 'n')
			{
				char name[20];
				char typ;

				sprintf(name, "No name");
				if (input_box("Name?", hgt / 2, wid / 2, name, 15))
				{
					cptr styp = "nothing";
					typ = msg_box("[D]estroy, [P]ickup, [I]nscribe, [N]othing rule?", hgt / 2, wid / 2);
					if ((typ == 'd') || (typ == 'D')) styp = "destroy";
					else if ((typ == 'p') || (typ == 'P')) styp = "pickup";
					else if ((typ == 'i') || (typ == 'I')) styp = "inscribe";
					exec_lua(format("auto_aux:new_rule(%d, '%s', '%s', ''); auto_aux:adjust_current(%d)", sel, name, styp, sel));
					active = ACTIVE_RULE;
				}
			}
			else if (c == 's')
			{
				char name[20];

				sprintf(name, "automat.atm");
				if (input_box("Save name?", hgt / 2, wid / 2, name, 30))
				{
					char buf[1025];
					char ch;

					/* Build the filename */
					path_build(buf, 1024, ANGBAND_DIR_USER, name);

					/* File type is "TEXT" */
					FILE_TYPE(FILE_TYPE_TEXT);

					if (file_exist(buf))
					{
						c_put_str(TERM_WHITE, "File exists, continue?[y/n]", hgt / 2, wid / 2 - 14);
						ch = inkey();
						if ((ch != 'Y') && (ch != 'y'))
							continue;
					}

					/* Open the non-existing file */
					hook_file = my_fopen(buf, "w");

					/* Invalid file */
					if (!hook_file)
					{
						/* Message */
						c_put_str(TERM_WHITE, "Saving rules failed!       ", hgt / 2, wid / 2 - 14);
						(void) inkey();

						/* Error */
						continue;
					}



					exec_lua("auto_aux:save_ruleset()");
					my_fclose(hook_file);
					/* Overwrite query message */
					c_put_str(TERM_WHITE, "Saved rules in file        ", hgt / 2, wid / 2 - 14);
					(void) inkey();
				}
			}
			else if (c == 'r')
			{
				char name[20];

				if (!max) continue;

				sprintf(name, string_exec_lua(format("return __rules[%d].table.args.name", sel)));
				if (input_box("New name?", hgt / 2, wid / 2, name, 15))
				{
					exec_lua(format("auto_aux:rename_rule(%d, '%s')", sel, name));
				}

				continue;
			}
			else if (c == 'k')
			{
				automatizer_enabled = FALSE;
				break;
			}
			else if (c == '\t')
			{
				if (!max) continue;
				active = ACTIVE_RULE;
			}
			else if (c == 'x')
			{
				exec_lua("xml.display_english = not xml.display_english");
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
				exec_lua("auto_aux:go_up()");
			}
			else if (c == '2')
			{
				exec_lua("auto_aux:go_down()");
			}
			else if (c == '6')
			{
				exec_lua("auto_aux:go_right()");
			}
			else if (c == '4')
			{
				exec_lua(format("auto_aux:go_left(%d)", sel));
			}
			else if (c == '9')
			{
				exec_lua("auto_aux:scroll_up()");
			}
			else if (c == '3')
			{
				exec_lua("auto_aux:scroll_down()");
			}
			else if (c == '7')
			{
				exec_lua("auto_aux:scroll_left()");
			}
			else if (c == '1')
			{
				exec_lua("auto_aux:scroll_right()");
			}
			else if (c == 'a')
			{
				int s = automatizer_new_rule();
				if (s == -1) continue;
				exec_lua(format("auto_aux:add_child('%s')", types_list[s]));
			}
			else if (c == 'd')
			{
				if (max)
				{
					int new_sel;

					new_sel = exec_lua(format("return auto_aux:del_self(%d)", sel));
					if ((sel != new_sel) && (new_sel >= 0))
					{
						sel = new_sel;
						adjust_begin(&begin, &sel, max, hgt);
						exec_lua(format("auto_aux:adjust_current(%d)", sel));
					}
					else if (new_sel == -1)
					{
						active = ACTIVE_LIST;
					}
				}
			}
			else if (c == '\t')
			{
				active = ACTIVE_LIST;
			}
			else if (c == 'x')
			{
				exec_lua("xml.display_english = not xml.display_english");
			}
		}
	}

	/* Recalculate the rules */
	exec_lua("auto_aux.regen_ruleset()");

	screen_load();
}

/* Add a new rule in an easy way */
bool automatizer_create = FALSE;
void automatizer_add_rule(object_type *o_ptr, bool destroy)
{
	char ch;
	bool do_status = FALSE;
	cptr type = "destroy";

	if (!destroy)
	{
		type = "pickup";
	}

	while (TRUE)
	{
		if (!get_com(format("%s all of the same [T]ype, [F]amily or [N]ame, also use [S]tatus (%s)? ", (destroy) ? "Destroy" : "Pickup", (do_status) ? "Yes" : "No"), &ch))
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
			call_lua("easy_add_rule", "(s,s,d,O)", "", type, "tsval", do_status, o_ptr);
			break;
		}

		if (ch == 'F' || ch == 'f')
		{
			call_lua("easy_add_rule", "(s,s,d,O)", "", type, "tval", do_status, o_ptr);
			break;
		}

		if (ch == 'N' || ch == 'n')
		{
			call_lua("easy_add_rule", "(s,s,d,O)", "", type, "name", do_status, o_ptr);
			break;
		}
	}
}
