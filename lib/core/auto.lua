-- This file is the core of the Automatizer
-- Please do not touch unless you know what you are doing

__rules = {}
__rules_max = 0

rule_aux = {}

-- Rule apply function, does .. nothing
function auto_nothing(obj, item)
	return
end

function auto_inscribe(obj, item, note)
	if obj.note ~= 0 then return end
	msg_print("<Auto-Inscribe {"..note.."}>")
	obj.note = quark_add(note)
	return TRUE
end

-- Rule apply function, pickup object
function auto_pickup(obj, item)
	if item >= 0 then return end
	if inven_carry_okay(obj) == FALSE then return end
	msg_print("<Auto-pickup>")
	object_pickup(-item)
	return TRUE
end

-- Rule apply function, destroy item
function auto_destroy(obj, item)
	-- be carefull to what we can destroy
	-- Unaware things won't be destroyed.
	if is_aware(obj) == FALSE then return end

	-- Inscribed things won't be destroyed!
	if obj.note ~= 0 then return end

	-- Keep Artifacts -- they cannot be destroyed anyway
	if is_artifact(obj) == TRUE then return end

	-- Cannot destroy CURSE_NO_DROP objects
	local f1, f2, f3, f4, f5, esp = object_flags(obj);
	if band(f4, TR4_CURSE_NO_DROP) ~= 0 and band(obj.ident, IDENT_CURSED) then return end

	msg_print("<Auto-destroy>")

	-- Eliminate the item (from the pack)
	if item >= 0 then
		inven_item_increase(item, -obj.number)
		inven_item_describe(item)
		inven_item_optimize(item)
	-- Eliminate the item (from the floor)
	else
		floor_item_increase(0 - item, -obj.number)
		floor_item_describe(0 - item)
		floor_item_optimize(0 - item)
	end
	return TRUE
end

-- Report the status of an object
function object_status(obj)
	local sense =
	{
		[SENSE_CURSED] = "bad",
		[SENSE_WORTHLESS] = "very bad",
		[SENSE_AVERAGE] = "average",
		[SENSE_GOOD_LIGHT] = "good",
		[SENSE_GOOD_HEAVY] = "good",
		[SENSE_EXCELLENT] = "very good",
		[SENSE_SPECIAL] = "special",
		[SENSE_TERRIBLE] = "terrible",
	}

	if is_known(obj) == FALSE then
		if sense[obj.sense] then
			return sense[obj.sense]
		else
			return ""
		end
	else
if nil then -- test
		local osense = -1
		local type = select_sense(obj, TRUE, TRUE)
		if type == 1 then
			osense = value_check_aux1(obj)
		elseif type == 2 then
			osense = value_check_aux1_magic(obj)
		end
print("type : "..type)
		if sense[osense] then
		print("sense: "..sense[osense])
			return sense[osense]
		else
		print("sense: ")
			return ""
		end

else -- the real one

		local slot = wield_slot_ideal(obj, TRUE)

		-- Arts items
		if is_artifact(obj) == TRUE then
			if band(obj.ident, IDENT_CURSED) == 0 then return "special"
			else return "terrible" end
		-- Ego items
		elseif (obj.name2 > 0 or obj.name2b > 0) then
			if band(obj.ident, IDENT_CURSED) == 0 then return "very good"
			else return "very bad" end
		-- weapon
		elseif (slot == INVEN_WIELD) or (slot == INVEN_BOW) or (slot == INVEN_AMMO) or (slot == INVEN_TOOL) then
		    if obj.to_h + obj.to_d < 0 then
			 return "bad"
		    elseif obj.to_h + obj.to_d > 0 then
			 return "good"
		    else
			 return "average"
		    end
		-- armor
		elseif (slot >= INVEN_BODY) and (slot <= INVEN_FEET) then
		    if obj.to_a < 0 then
			 return "bad"
		    elseif obj.to_a > 0 then
			 return "good"
		    else
			 return "average"
		    end
		-- ring
		elseif slot == INVEN_RING then
		    if (obj.to_d + obj.to_h < 0) or (obj.to_a < 0) or (obj.pval < 0) then
			 return "bad"
		    else
			 return "average"
		    end
		-- amulet
		elseif slot == INVEN_NECK then
		    if (obj.pval < 0) then
			 return "bad"
		    else
			 return "average"
		    end
		-- chests
			 elseif obj.tval == TV_CHEST then
			 	if obj.pval == 0 then
					return "empty"
				elseif obj.pval < 0 then
					return "disarmed"
				else
					return "average"
				end
		else
		    return "average"
		end
end
	end
end

-- Recursive function to generate a rule function tree
function gen_rule_fct(r)
	-- It is a test rule (or, and, ...)
	if r.label == "and" or r.label == "or" then
		local i
		local fct_tbl = {}
		for i = 1, getn(r) do
			if r[i].label ~= "comment" then
				tinsert(fct_tbl, gen_rule_fct(r[i]))
			end
		end
		if r.label == "and" then
			return function(object)
				local fcts = %fct_tbl
				local i
				for i = 1, getn(fcts) do
					if not fcts[i](object) then return end
				end
				return TRUE
			end
		elseif r.label == "or" then
			return function(object)
				local fcts = %fct_tbl
				local i
				for i = 1, getn(fcts) do
					if fcts[i](object) then return TRUE end
				end
			end
		end
	-- It is a condition rule (name, type, level, ...)
	else
		if r.label == "not" then
			local f
			if not r[1] then
				f = function (object) return TRUE end
			else
				f = gen_rule_fct(r[1])
			end
			return function(object) return not %f(object) end
		elseif r.label == "name" then
			return function(object) if strlower(object_desc(object, -1, 0)) == strlower(%r[1]) then return TRUE end end
		elseif r.label == "contain" then
			return function(object) if strfind(strlower(object_desc(object, -1, 0)), strlower(%r[1])) then return TRUE end end
		elseif r.label == "symbol" then
			return function(object) if strchar(get_kind(object).d_char) == %r[1] then return TRUE end end
		elseif r.label == "inscribed" then
			return function(object) if object.note ~= 0 and strfind(strlower(quark_str(object.note)), strlower(%r[1])) then return TRUE end end
		elseif r.label == "discount" then
			local d1 = r.args.min
			local d2 = r.args.max
			if tonumber(d1) == nil then d1 = getglobal(d1) else d1 = tonumber(d1) end
			if tonumber(d2) == nil then d2 = getglobal(d2) else d2 = tonumber(d2) end
			return function(object) if is_aware(object) == TRUE and object.discount >= %d1 and object.discount <= %d2 then return TRUE end end
		elseif r.label == "tval" then
			local tv = r[1]
			if tonumber(tv) == nil then tv = getglobal(tv) else tv = tonumber(tv) end
			return function(object) if object.tval == %tv then return TRUE end end
		elseif r.label == "sval" then
			assert(r.args.min and r.args.max, "sval rule lacks min or max")
			local sv1 = r.args.min
			local sv2 = r.args.max
			if tonumber(sv1) == nil then sv1 = getglobal(sv1) else sv1 = tonumber(sv1) end
			if tonumber(sv2) == nil then sv2 = getglobal(sv2) else sv2 = tonumber(sv2) end
			return function(object) if is_aware(object) == TRUE and object.sval >= %sv1 and object.sval <= %sv2 then return TRUE end end
		elseif r.label == "status" then
			return function(object) if object_status(object) == strlower(%r[1]) then return TRUE end end
		elseif r.label == "state" then
			if r[1] == "identified" then
				return function(object) if is_known(object) == TRUE then return TRUE end end
			else
				return function(object) if is_known(object) == FALSE then return TRUE end end
			end
		elseif r.label == "race" then
			return function(object) if strlower(get_race_name()) == strlower(%r[1]) then return TRUE end end
		elseif r.label == "subrace" then
			return function(object) if strlower(get_subrace_name()) == strlower(%r[1]) then return TRUE end end
		elseif r.label == "class" then
			return function(object) if strlower(get_class_name()) == strlower(%r[1]) then return TRUE end end
		elseif r.label == "level" then
			assert(r.args.min and r.args.max, "level rule lacks min or max")
			return function(object) if player.lev >= tonumber(%r.args.min) and player.lev <= tonumber(%r.args.max) then return TRUE end end
		elseif r.label == "skill" then
			assert(r.args.min and r.args.max, "skill rule lacks min or max")
			local s = find_skill_i(r[1])
			assert(s ~= -1, "no skill "..r[1])
			return function(object) if get_skill(%s) >= tonumber(%r.args.min) and get_skill(%s) <= tonumber(%r.args.max) then return TRUE end end
		elseif r.label == "ability" then
			local s = find_ability(r[1])
			assert(s ~= -1, "no ability "..r[1])
			return function(object) if has_ability(%s) == TRUE then return TRUE end end
		end
	end
end

function auto_inscribe_maker(inscription)
    return function(...)
	arg.n = arg.n + 1
	arg[getn(arg)] = %inscription
	return call(auto_inscribe, arg)
    end
end

-- Generate a rule from a table
function gen_full_rule(t)
	-- only honor rules for this module
	if not t.args.module then
		t.args.module = "ToME"
	end

	if not ((t.args.module == "all") or (t.args.module == game_module)) then
		return function() end
	end

	-- Check for which action to do
	local apply_fct = auto_nothing
	if t.args.type == "destroy" then apply_fct = auto_destroy
	elseif t.args.type == "pickup" then apply_fct = auto_pickup
	elseif t.args.type == "inscribe" then apply_fct = auto_inscribe_maker(t.args.inscription)
	end

	-- create the function tree
	local rf
	if t[1] then
		rf = gen_rule_fct(t[1])
	else
		rf = function (object) end
	end

	-- create the final function
	return function(...)
		local rf = %rf
		if rf(arg[1]) then
			if call(%apply_fct, arg) == TRUE then return TRUE end
		end
	end
end

-- Create a function that checks for the rules(passed in xml form)
function add_ruleset(s)
	local tbl = xml:collect(s)
	local i

	-- Add all rules
	for i = 1, getn(tbl) do
		local t = tbl[i]

		if t.label == "rule" then
			-- Create the function tree
			local fct = gen_full_rule(t)

			-- Create the test function
			__rules[__rules_max] =
			{
				["table"] =	t,
				["fct"] =	fct
			}
			__rules_max = __rules_max + 1
		end
	end
end

-- Apply the current rules to an object
-- call with at least (object, idx)
function apply_rules(...)
	local i
	for i = 0, __rules_max - 1 do
		if call(__rules[i].fct, arg) then return TRUE end
	end
	return FALSE
end

-- Clear the current rules
function clean_ruleset()
	__rules_max = 0
	__rules = {}
end

------ helper fonctions for the GUI

auto_aux = {}
auto_aux.stack = { n = 0 }
auto_aux.idx = 1
auto_aux.rule = 1
function auto_aux:go_right()
	if auto_aux.rule[1] and type(auto_aux.rule[1]) == "table" then
		tinsert(auto_aux.stack, auto_aux.idx)
		tinsert(auto_aux.stack, auto_aux.rule)
		auto_aux.rule = auto_aux.rule[1]
		auto_aux.idx = 1
	end
end

function auto_aux:go_left(sel)
	local n = getn(auto_aux.stack)

	if n > 0 then
		auto_aux.idx = auto_aux.stack[n - 1]
		auto_aux.rule = auto_aux.stack[n]
		tremove(auto_aux.stack)
		tremove(auto_aux.stack)
	end
end

function auto_aux:go_down()
	if getn(auto_aux.stack) > 1 then
		if auto_aux.stack[getn(auto_aux.stack)][auto_aux.idx + 1] then
			auto_aux.idx = auto_aux.idx + 1
			auto_aux.rule = auto_aux.stack[getn(auto_aux.stack)][auto_aux.idx]
		end
	end
end

function auto_aux:go_up()
	if getn(auto_aux.stack) > 1 then
		if auto_aux.stack[getn(auto_aux.stack)][auto_aux.idx - 1] then
			auto_aux.idx = auto_aux.idx - 1
			auto_aux.rule = auto_aux.stack[getn(auto_aux.stack)][auto_aux.idx]
		end
	end
end

function auto_aux:scroll_up()
	xml.write_off_y = xml.write_off_y - 1
end

function auto_aux:scroll_down()
	xml.write_off_y = xml.write_off_y + 1
end

function auto_aux:scroll_left()
	xml.write_off_x = xml.write_off_x + 1
end

function auto_aux:scroll_right()
	xml.write_off_x = xml.write_off_x - 1
end

function auto_aux:adjust_current(sel)
	if __rules_max == 0 then return end

	xml.write_off_y = 0
	xml.write_off_x = 0
	auto_aux.idx = 1
	auto_aux.stack = { n = 0 }
	auto_aux.rule = __rules[sel].table
end

function auto_aux:move_up(sel)
	if sel > 0 then
		local u = __rules[sel - 1]
		local d = __rules[sel]
		__rules[sel - 1] = d
		__rules[sel] = u
		return sel - 1
	end
	return sel
end

function auto_aux:move_down(sel)
	if sel < __rules_max - 1 then
		local u = __rules[sel]
		local d = __rules[sel + 1]
		__rules[sel + 1] = u
		__rules[sel] = d
		return sel + 1
	end
	return sel
end

function auto_aux:new_rule(sel, nam, typ, arg)
	local r


	-- nam can also directly be the table itself
	if type(nam) == "table" then
		r =
		{
			["table"] = nam,
			["fct"] = function (object) end
		}
	elseif typ == "inscribe" then
		if arg == "" then
		    arg = input_box("Inscription?", 79)
		end
		r =
		{
			["table"] =
			{
				label = "rule",
				args = { name = nam, type = typ, inscription = arg, module = game_module },
			},
			["fct"] = function (object) end
		}
	else
		r =
		{
			["table"] =
			{
				label = "rule",
				args = { name = nam, type = typ, module = game_module },
			},
			["fct"] = function (object) end
		}
	end
	tinsert(__rules, sel, r)
	__rules_max = __rules_max + 1
end

function auto_aux:rename_rule(sel, nam)
	if sel >= 0 and sel < __rules_max then
		__rules[sel].table.args.name = nam
	end
end

function auto_aux:save_ruleset()
	xml.write = xml.write_file

	print_hook("clean_ruleset()\nadd_ruleset\n[[\n")
	local i
	for i = 0, __rules_max - 1 do
		xml:print_xml(__rules[i].table, '')
	end
	print_hook("]]\n")

	xml.write = xml.write_screen
end

function auto_aux:del_self(sel)
	if auto_aux.rule.label == "rule" then
		tremove(__rules, sel)
		__rules_max = __rules_max - 1
		return sel - 1
	else
		local idx = auto_aux.idx
		auto_aux:go_left(sel)
		tremove(auto_aux.rule, idx)
		return sel
	end
end

auto_aux.types_desc =
{
	["and"] =
	{
		"Check is true if all rules within it are true",
		xml:collect([[<and><foo1>...</foo1><foo2>...</foo2><foo3>...</foo3></and>]]),
		function ()
			return xml:collect("<and></and>")
		end,
	},
	["or"] =
	{
		"Check is true if at least one rule within it is true",
		xml:collect([[<or><foo1>...</foo1><foo2>...</foo2><foo3>...</foo3></or>]]),
		function ()
			return xml:collect("<or></or>")
		end,
	},
	["not"] =
	{
		"Invert the result of its child rule",
		xml:collect([[<not><foo1>...</foo1></not>]]),
		function ()
			return xml:collect("<not></not>")
		end,
	},
	["comment"] =
	{
		"Comments are meaningless",
		xml:collect([[<comment>Comment explaining something</comment>]]),
		function ()
			local n = input_box("Comment?", 79)
			if n == "" then return end
			return xml:collect("<comment>"..n.."</comment>")
	       	end,
	},
	["name"] =
	{
		"Check is true if object name matches name",
		xml:collect([[<name>potion of healing</name>]]),
		function ()
			local n = input_box("Object name to match?", 79)
			if n == "" then return end
			return xml:collect("<name>"..n.."</name>")
	       	end,
	},
	["contain"] =
	{
		"Check is true if object name contains word",
		xml:collect([[<contain>healing</contain>]]),
		function ()
			local n = input_box("Word to find in object name?", 79)
			if n == "" then return end
			return xml:collect("<contain>"..n.."</contain>")
	       	end,
	},
	["inscribed"] =
	{
		"Check is true if object inscription contains word",
		xml:collect([[<inscribed>=g</inscribed>]]),
		function ()
			local n = input_box("Word to find in object inscription?", 79)
			if n == "" then return end
			return xml:collect("<inscribed>"..n.."</inscribed>")
	       	end,
	},
	["discount"] =
	{
		"Check is true if object discount is between 2 values",
		xml:collect([[<sval min='50' max='100'></sval>]]),
		function ()
			local s = "<discount "

			local n = input_box("Min discount?", 79)
			if n == "" then return end
			s = s.."min='"..n.."' "

			n = input_box("Max discount?", 79)
			if n == "" then return end
			s = s.."max='"..n.."'></discount>"
			return xml:collect(s)
	       	end,
	},
	["symbol"] =
	{
		"Check is true if object symbol is ok",
		xml:collect([[<symbol>!</symbol>]]),
		function ()
			local n = input_box("Symbol to match?", 1)
			if n == "" then return end
			return xml:collect("<symbol>"..n.."</symbol>")
	       	end,
	},
	["status"] =
	{
		"Check is true if object status is ok",
		xml:collect([[<status>good</status>]]),
		function ()
			local n = msg_box("[t]errible, [v]ery bad, [b]ad, [a]verage, [G]ood, [V]ery good, [S]pecial?")
			local t =
			{
				["t"] = "terrible",
				["v"] = "very bad",
				["b"] = "bad",
				["a"] = "average",
				["G"] = "good",
				["V"] = "very good",
				["S"] = "special",
			}
			if not t[strchar(n)] then return end
			return xml:collect("<status>"..t[strchar(n)].."</status>")
	       	end,
	},
	["state"] =
	{
		"Check is true if object is identified/unidentified",
		xml:collect([[<state>identified</state>]]),
		function ()
			local n = msg_box("[i]dentified, [n]on identified?")
			local t =
			{
				["i"] = "identified",
				["n"] = "not identified",
			}
			if not t[strchar(n)] then return end
			return xml:collect("<state>"..t[strchar(n)].."</state>")
	       	end,
	},
	["tval"] =
	{
		"Check is true if object tval(from k_info.txt) is ok",
		xml:collect([[<tval>55</tval>]]),
		function ()
			local n = input_box("Tval to match?", 79)
			if n == "" then return end
			return xml:collect("<tval>"..n.."</tval>")
	       	end,
	},
	["sval"] =
	{
		{
			"Check is true if object sval(from k_info.txt) is between",
			"2 values",
		},
		xml:collect([[<sval min='0' max='100'></sval>]]),
		function ()
			local s = "<sval "

			local n = input_box("Min sval?", 79)
			if n == "" then return end
			s = s.."min='"..n.."' "

			n = input_box("Max sval?", 79)
			if n == "" then return end
			s = s.."max='"..n.."'></sval>"
			return xml:collect(s)
	       	end,
	},
	["race"] =
	{
		"Check is true if player race is ok",
		xml:collect([[<race>dunadan</race>]]),
		function ()
			local n = input_box("Player race to match?", 79)
			if n == "" then return end
			return xml:collect("<race>"..n.."</race>")
	       	end,
	},
	["subrace"] =
	{
		"Check is true if player subrace is ok",
		xml:collect([[<subrace>vampire</subrace>]]),
		function ()
			local n = input_box("Player subrace to match?", 79)
			if n == "" then return end
			return xml:collect("<subrace>"..n.."</subrace>")
	       	end,
	},
	["class"] =
	{
		"Check is true if player class is ok",
		xml:collect([[<class>sorceror</class>]]),
		function ()
			local n = input_box("Player class to match?", 79)
			if n == "" then return end
			return xml:collect("<class>"..n.."</class>")
	       	end,
	},
	["level"] =
	{
		"Check is true if player level is between 2 values",
		xml:collect([[<level min='20' max='50'></level>]]),
		function ()
			local s = "<level "

			local n = input_box("Min player level?", 79)
			if n == "" then return end
			s = s.."min='"..n.."' "

			n = input_box("Max player level?", 79)
			if n == "" then return end
			s = s.."max='"..n.."'></level>"

			return xml:collect(s)
	       	end,
	},
	["skill"] =
	{
		"Check is true if player skill level is between 2 values",
		xml:collect([[<skill min='10' max='20'>Divination</skill>]]),
		function ()
			local s = "<skill "

			local n = input_box("Min skill level?", 79)
			if n == "" then return end
			s = s.."min='"..n.."' "

			n = input_box("Max skill level?", 79)
			if n == "" then return end
			s = s.."max='"..n.."'>"

			n = input_box("Skill name?", 79)
			if n == "" then return end
			if find_skill_i(n) == -1 then return end
			s = s..n.."</skill>"

			return xml:collect(s)
	       	end,
	},
	["ability"] =
	{
		"Check is true if player has the ability",
		xml:collect([[<ability>Ammo creation</ability>]]),
		function()
			local n = input_box("Ability name?", 79)
			if n == "" then return end
			if find_ability(n) == -1 then return end
			return xml:collect("<ability>"..n.."</ability>")
		end,
	},
}

function auto_aux:display_desc(sel)
	local d = auto_aux.types_desc[sel][1]
	if type(d) == "string" then
		c_prt(TERM_WHITE, d, 1, 17)
	else
		local k, e, i
		i = 0
		for k, e in d do
			c_prt(TERM_WHITE, e, 1 + i, 17)
			i = i + 1
		end
	end
end

function auto_aux:add_child(sel)
	-- <rule> and <not> contain only one match
	if (auto_aux.rule.label == "rule" or auto_aux.rule.label == "not") and auto_aux.rule[1] then return end

	-- Only <and> and <or> can contain
	if auto_aux.rule.label ~= "rule" and auto_aux.rule.label ~= "and" and auto_aux.rule.label ~= "or"  and auto_aux.rule.label ~= "not" then return end

	-- get it
	local r = auto_aux.types_desc[sel][3]()
	if not r then return end

	-- Ok add it
	tinsert(auto_aux.rule, r[1])
end

function auto_aux.regen_ruleset()
	local i
	for i = 0, __rules_max - 1 do
		__rules[i].fct = gen_full_rule(__rules[i].table)
	end
end


-- Easily add new rules
function easy_add_rule(typ, mode, do_status, obj)
	local detect_rule

	if mode == "tval" then
		detect_rule = "<tval>"..obj.tval.."</tval>"
	elseif mode == "tsval" then
		detect_rule = "<and><tval>"..obj.tval.."</tval><sval min='"..obj.sval.."' max='"..obj.sval.."'></sval></and>"
	elseif mode == "name" then
		detect_rule = "<name>"..strlower(object_desc(obj, -1, 0)).."</name>"
	end

	if do_status == TRUE then
		local status = object_status(obj)
		if status and not (status == "") then
			detect_rule = "<and>"..detect_rule.."<status>"..status.."</status></and>"
		end
	end

	local rule = "<rule module='"..game_module.."' name='"..typ.."' type='"..typ.."'>"..detect_rule.."</rule>"
	auto_aux:new_rule(0, xml:collect(rule)[1], '')
	auto_aux.regen_ruleset()
	msg_print("Rule added. Please go to the Automatizer screen (press = then T)")
	msg_print("to save the modified ruleset.")
end
