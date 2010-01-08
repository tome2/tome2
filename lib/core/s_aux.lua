-- Functions to help with spells, do not touch

__schools = {}
__schools_num = 0

__tmp_spells = {}
__tmp_spells_num = 0

function add_school(s)
	__schools[__schools_num] = s
	
	__schools_num = __schools_num + 1
	return (__schools_num - 1)
end

function finish_school(i)
	local s
	
	s = __schools[i]
	assert(s.name, "No school name!")
	assert(s.skill, "No school skill!")

	-- Need hooks?
	if s.hooks then
		add_hooks(s.hooks)
	end

	new_school(i, s.name, s.skill)
end

function add_spell(s)
	__tmp_spells[__tmp_spells_num] = s
	
	__tmp_spells_num = __tmp_spells_num + 1
	return (__tmp_spells_num - 1)
end

function finish_spell(must_i)
	local i, s
	
	s = __tmp_spells[must_i]
	assert(s.name, "No spell name!")
	assert(s.school, "No spell school!")
	assert(s.level, "No spell level!")
	assert(s.mana, "No spell mana!")
	if not s.mana_max then s.mana_max = s.mana end
	assert(s.fail, "No spell failure rate!")
	assert(s.spell, "No spell function!")
	if not s.info then s.info = function() return "" end end
	assert(s.desc, "No spell desc!")
	if not s.random then s.random = SKILL_MAGIC end
	if s.lasting then
		assert(type(s.lasting) == "function", "Spell lasting is not function")
	end
	if s.stick then
		local k, e
		for k, e in s.stick do
			if type(k) == "table" then
				assert(e.base_level, "Arg no stick base level")
				assert(e.max_level, "Arg no stick max level")
			end
		end
	end

	i = new_spell(must_i, s.name)
	assert(i == must_i, "ACK ! i != must_i ! please contact the maintainer")
	if type(s.school) == "number" then __spell_school[i] = {s.school}
	else __spell_school[i] = s.school end
	spell(i).mana = s.mana
	spell(i).mana_max = s.mana_max
	spell(i).fail = s.fail
	spell(i).skill_level = s.level
	__spell_spell[i] = s.spell
	__spell_info[i] = s.info
	__spell_desc[i] = s.desc
	return i
end

-- Creates the school books array
__spell_spell = {}
__spell_info = {}
__spell_desc = {}
__spell_school = {}
school_book = {}

-- Find a spell by name
function find_spell(name)
	local i

	i = 0
	while (i < __tmp_spells_num) do
		if __tmp_spells[i].name == name then return i end
		i = i + 1
	end
	return -1
end

-- Find if the school is under the influence of a god, returns nil or the level
function get_god_level(sch)
	if __schools[sch].gods[player.pgod] then
		return (s_info[__schools[sch].gods[player.pgod].skill + 1].value * __schools[sch].gods[player.pgod].mul) / __schools[sch].gods[player.pgod].div
	else
		return nil
	end
end

-- Change this fct if I want to switch to learnable spells
function get_level_school(s, max, min)
	local lvl, sch, index, num, bonus
	local allow_spell_power = TRUE

	lvl = 0
	num = 0
	bonus = 0

	-- No max specified ? assume 50
	if not max then
		max = 50
	end
	if not min then
		min = 1
	end

	-- Do we pass tests?
	if __tmp_spells[s].depend then
		if __tmp_spells[s].depend() ~= TRUE then
			return min, "n/a"
		end
	end

	for index, sch in __spell_school[s] do
		local r, s, p, ok = 0, 0, 0, 0

		-- Does it require we worship a specific god?
		if __schools[sch].god then
			if __schools[sch].god ~= player.pgod then
				if min then return min, "n/a"
				else return 1, "n/a" end
			end
		end

		-- Take the basic skill value
		r = s_info[(school(sch).skill) + 1].value

		-- Do we pass tests?
		if __schools[sch].depend then
			if __schools[sch].depend() ~= TRUE then
				return min, "n/a"
			end
		end

		-- Are we under sorcery effect ?
		if __schools[sch].sorcery then
			s = s_info[SKILL_SORCERY + 1].value
		end

		-- Are we affected by spell power ?
		-- All teh schools must allow it for it to work
		if not __schools[sch].spell_power then
			allow_spell_power = nil
		end

		-- Are we under a god effect ?
		if __schools[sch].gods then
			p = get_god_level(sch)
			if not p then p = 0 end
		end
		
		-- Find the higher
		ok = r
		if ok < s then ok = s end
		if ok < p then ok = p end

		-- Do we need to add a special bonus ?
		if __schools[sch].bonus_level then
			bonus = bonus + (__schools[sch].bonus_level() * (SKILL_STEP / 10))
		end

		-- All schools must be non zero to be able to use it
		if ok == 0 then return min, "n/a" end

		-- Apply it
		lvl = lvl + ok
		num = num + 1
	end

	-- Add the Spellpower skill as a bonus
	if allow_spell_power then
		bonus = bonus + (get_skill_scale(SKILL_SPELL, 20) * (SKILL_STEP / 10))
	end

	-- Add bonus from objects
	bonus = bonus + (player.to_s * (SKILL_STEP / 10))

	-- / 10 because otherwise we can overflow a s32b and we can use a u32b because the value can be negative
	-- The loss of information should be negligible since 1 skill = 1000 internally
	lvl = (lvl / num) / 10
	lvl = lua_get_level(s, lvl, max, min, bonus)

	return lvl, nil
end

-- This is the function to use when casting through a stick
function get_level_device(s, max, min)
	local lvl

	-- No max specified ? assume 50
	if not max then
		max = 50
	end

	lvl = s_info[SKILL_DEVICE + 1].value
	lvl = lvl + (get_level_use_stick * SKILL_STEP)

	-- Sticks are limited
	if lvl - ((spell(s).skill_level + 1) * SKILL_STEP) >= get_level_max_stick * SKILL_STEP then
		lvl = (get_level_max_stick + spell(s).skill_level - 1) * SKILL_STEP
	end

	-- / 10 because otherwise we can overflow a s32b and we can use a u32b because the value can be negative
	-- The loss of information should be negligible since 1 skill = 1000 internally
	lvl = lvl / 10
	if not min then
		lvl = lua_get_level(s, lvl, max, 1, 0)
	else
		lvl = lua_get_level(s, lvl, max, min, 0)
	end

	return lvl
end

-- The real get_level, works for schooled magic and for innate powers
get_level_use_stick = -1
get_level_max_stick = -1
function get_level(s, max, min)
	if type(s) == "number" then
		-- Ahah shall we use Magic device instead ?
		if get_level_use_stick > -1 then
			return get_level_device(s, max, min)
		else
			local lvl, na = get_level_school(s, max, min)
			return lvl
		end
	else
		return get_level_power(s, max, min)
	end
end

-- Can we cast the spell ?
function is_ok_spell(s, obj)
	if get_level(s, 50, 0) == 0 then return nil end
	if __tmp_spells[s].pval and obj.pval < __tmp_spells[s].pval then return nil end
	return 1
end

-- Get the amount of mana(or power) needed
function get_mana(s)
	return spell(s).mana + get_level(s, spell(s).mana_max - spell(s).mana, 0)
end

-- Return the amount of power(mana, piety, whatever) for the spell
function get_power(s)
	if check_affect(s, "piety", FALSE) then
		return player.grace
	else
		return player.csp
	end
end

-- Return the amount of power(mana, piety, whatever) for the spell
function get_power_name(s)
	if check_affect(s, "piety", FALSE) then
		return "piety"
	else
		return "mana"
	end
end

-- Changes the amount of power(mana, piety, whatever) for the spell
function adjust_power(s, x)
	if check_affect(s, "piety", FALSE) then
		inc_piety(GOD_ALL, x)
	else
		increase_mana(x)
	end
end

-- Print the book and the spells
function print_book(book, spl, obj)
	local x, y, index, sch, size, s

	x = 0
	y = 2
	size = 0

	-- Hack if the book is 255 it is a random book
	if book == 255 then
		school_book[book] = {spl}
	end

	-- Parse all spells
	for index, s in school_book[book] do
		local color = TERM_L_DARK
		local lvl, na = get_level_school(s, 50, -50)
		local xx, sch_str

		if is_ok_spell(s, obj) then
			if get_mana(s) > get_power(s) then color = TERM_ORANGE
			else color = TERM_L_GREEN end
		end
		
		xx = nil
		sch_str = ""
		for index, sch in __spell_school[s] do
			if xx then
				sch_str = sch_str.."/"..school(sch).name
			else
				xx = 1
				sch_str = sch_str..school(sch).name
			end
		end

		if na then
			c_prt(color, format("%c) %-20s%-16s   %3s %4s %3d%s %s", size + strbyte("a"), spell(s).name, sch_str, na, get_mana(s), spell_chance(s), "%", __spell_info[s]()), y, x)
		else
			c_prt(color, format("%c) %-20s%-16s   %3d %4s %3d%s %s", size + strbyte("a"), spell(s).name, sch_str, lvl, get_mana(s), spell_chance(s), "%", __spell_info[s]()), y, x)
		end
		y = y + 1
		size = size + 1
	end
	prt(format("   %-20s%-16s Level Cost Fail Info", "Name", "School"), 1, x)
	return y
end

-- Output the describtion when it is used as a spell
function print_spell_desc(s, y)
	local index, desc, x
	
	x = 0

	if type(__spell_desc[s]) == "string" then c_prt(TERM_L_BLUE, __spell_desc[s], y, x)
	else
		for index, desc in __spell_desc[s] do
			c_prt(TERM_L_BLUE, desc, y, x)
			y = y + 1
		end
	end
	if check_affect(s, "piety", FALSE) then
		c_prt(TERM_L_WHITE, "It uses piety to cast.", y, x)
		y = y + 1
	end
	if not check_affect(s, "blind") then
		c_prt(TERM_ORANGE, "It is castable even while blinded.", y, x)
		y = y + 1
	end
	if not check_affect(s, "confusion") then
		c_prt(TERM_ORANGE, "It is castable even while confused.", y, x)
		y = y + 1
	end
end

-- Output the desc when sued as a device
function print_device_desc(s)
	local index, desc

	if type(__spell_desc[s]) == "string" then text_out(__spell_desc[s])
	else
		for index, desc in __spell_desc[s] do
			text_out("\n" .. desc)
		end
	end
end

function book_spells_num(book)
	local size, index, sch

	size = 0

	-- Hack if the book is 255 it is a random book
	if book == 255 then
		return 1
	end

	-- Parse all spells
	for index, s in school_book[book] do
		size = size + 1
	end
	return size
end

function spell_x(book, spl, s)
	if book == 255 then
		return spl
	else
		local i, x, val

		i, val = next(school_book[book], nil)
		x = 0
		while x < s do
			i, val = next(school_book[book], i)
			x = x + 1
		end
		return val
	end
end

function spell_in_book(book, spell)
	local i, s

	for i, s in school_book[book] do
		if s == spell then return TRUE end
	end
	return FALSE
end

-- Returns spell chance of failure for spell
function spell_chance(s)
	local chance, s_ptr

	s_ptr = spell(s)

	-- Extract the base spell failure rate
	if get_level_use_stick > -1 then
		chance = lua_spell_device_chance(s_ptr.fail, get_level(s, 50), s_ptr.skill_level)
	else
		chance = lua_spell_chance(s_ptr.fail, get_level(s, 50), s_ptr.skill_level, get_mana(s), get_power(s), get_spell_stat(s))
	end

	-- Return the chance
	return chance
end

function check_affect(s, name, default)
	local s_ptr = __tmp_spells[s]
	local a

	if type(s_ptr[name]) == "number" then
		a = s_ptr[name]
	else
		a = default
	end
	if a == FALSE then
		return nil
	else
		return TRUE
	end
end

-- Returns the stat to use for the spell, INT by default
function get_spell_stat(s)
	if not __tmp_spells[s].stat then return A_INT
	else return __tmp_spells[s].stat end
end

function cast_school_spell(s, s_ptr, no_cost)
	local use = FALSE

	-- No magic
	if (player.antimagic > 0) then
		msg_print("Your anti-magic field disrupts any magic attempts.")
		return
	end

	-- No magic
	if (player.anti_magic == TRUE) then
		msg_print("Your anti-magic shell disrupts any magic attempts.")
		return
	end

	-- if it costs something then some condition must be met
	if not no_cost then
	 	-- Require lite
		if (check_affect(s, "blind")) and ((player.blind > 0) or (no_lite() == TRUE)) then
			msg_print("You cannot see!")
			return
		end

		-- Not when confused
		if (check_affect(s, "confusion")) and (player.confused > 0) then
			msg_print("You are too confused!")
			return
		end

		-- Enough mana
		if (get_mana(s) > get_power(s)) then
			if (get_check("You do not have enough "..get_power_name(s)..", do you want to try anyway?") == FALSE) then return end
		end
	
		-- Invoke the spell effect
		if (magik(spell_chance(s)) == FALSE) then
			if (__spell_spell[s]() ~= nil) then
				use  = TRUE
			end
		else
			local index, sch

			-- added because this is *extremely* important --pelpel
			if (flush_failure) then flush() end

			msg_print("You failed to get the spell off!")
			for index, sch in __spell_school[s] do
				if __schools[sch].fail then
					__schools[sch].fail(spell_chance(s))
				end
			end
			use  = TRUE
		end
	else
		__spell_spell[s]()
	end

	if use == TRUE then
		-- Reduce mana
		adjust_power(s, -get_mana(s))

		-- Take a turn
		if is_magestaff() == TRUE then energy_use = 80
		else energy_use = 100 end
	end

	player.redraw = bor(player.redraw, PR_MANA)
	player.window = bor(player.window, PW_PLAYER)
end


-- Helper functions
HAVE_ARTIFACT = 0
HAVE_OBJECT = 1
HAVE_EGO = 2
function have_object(mode, type, find, find2)
	local o, i, min, max
	
	max = 0
	min = 0
	if band(mode, USE_EQUIP) == USE_EQUIP then
		min = INVEN_WIELD
		max = INVEN_TOTAL
	end
	if band(mode, USE_INVEN) == USE_INVEN then
		min = 0
		if max == 0 then max = INVEN_WIELD end
	end

	i = min
	while i < max do
		o = get_object(i)
		if o.k_idx ~= 0 then
			if type == HAVE_ARTIFACT then
				if find == o.name1 then return i end
			end
			if type == HAVE_OBJECT then
				if find2 == nil then
					if find == o.k_idx then return i end
				else
					if (find == o.tval) and (find2 == o.sval) then return i end
				end
			end
			if type == HAVE_EGO then
				if (find == o.name2) or (find == o.name2b) then return i end
			end
		end
		i = i + 1
	end
	return -1
end

-- Can the spell be randomly found(in random books)
function can_spell_random(i)
	return __tmp_spells[i].random
end

-- Find a random spell
function get_random_spell(typ, level)
	local spl, tries

	tries = 1000
	while tries > 0 do
		tries = tries - 1
		spl = rand_int(__tmp_spells_num)
		if (can_spell_random(spl) == typ) and (rand_int(spell(spl).skill_level * 3) < level) then
			break
		end
	end
	if tries > 0 then
		return spl
	else
		return -1
	end
end

-- Execute a lasting spell
function exec_lasting_spell(spl)
	assert(__tmp_spells[spl].lasting, "No lasting effect for spell "..__tmp_spells[spl].name.." but called as such")
	return __tmp_spells[spl].lasting()
end

-- Helper function for spell effect to know if they are or not obvious
function is_obvious(effect, old)
	if old then
		if old == TRUE or effect == TRUE then
			return TRUE
		else
			return FALSE
		end
	else
		return effect
	end
end

-------------------------Sticks-------------------------

-- Fire off the spell
function activate_stick(spl)
	local ret = __spell_spell[spl]()
	local charge, obvious
	if not ret then
		charge = FALSE
		obvious = FALSE
	else
		charge = TRUE
		obvious = ret
	end
	return obvious, charge
end

----------------------------------- Wand, Staves, Rods specific functions ----------------------------

-- Get a spell for a given stick(wand, staff, rod)
function get_random_stick(stick, level)
	local spl, tries

	tries = 1000
	while tries > 0 do
		tries = tries - 1
		spl = rand_int(__tmp_spells_num)
		if __tmp_spells[spl].stick and (type(__tmp_spells[spl].stick[stick]) == "table") then
			if (rand_int(spell(spl).skill_level * 3) < level) and (magik(100 - __tmp_spells[spl].stick[stick].rarity) == TRUE) then
				break
			end
		end
	end
	if tries > 0 then
		return spl
	else
		return -1
	end
end

-- Get a random base level
function get_stick_base_level(stick, level, spl)
	-- Paranoia
	if spl < 0 or spl >= __tmp_spells_num or not __tmp_spells[spl].stick[stick] then return 0 end

	local min, max = __tmp_spells[spl].stick[stick].base_level[1], __tmp_spells[spl].stick[stick].base_level[2]
	local range = max - min;

	-- Ok the basic idea is to have a max possible level of half the dungeon level
	if range * 2 > level then range = level / 2 end

	-- Randomize a bit
	range = m_bonus(range, dun_level)

	-- And get the result
	return min + range
end

-- Get a random max level
function get_stick_max_level(stick, level, spl)
	-- Paranoia
	if spl < 0 or spl >= __tmp_spells_num or not __tmp_spells[spl].stick[stick] then return 0 end

	local min, max = __tmp_spells[spl].stick[stick].max_level[1], __tmp_spells[spl].stick[stick].max_level[2]
	local range = max - min;

	-- Ok the basic idea is to have a max possible level of half the dungeon level
	if range * 2 > level then range = level / 2 end

	-- Randomize a bit
	range = m_bonus(range, dun_level)

	-- And get the result
	return min + range
end

-- Get the number of desired charges
function get_stick_charges(spl)
	return __tmp_spells[spl].stick.charge[1] + randint(__tmp_spells[spl].stick.charge[2]);
end

-- Get activation desc
function get_activation_desc(spl)
	local turns
	if type(__tmp_spells[spl].activate) == 'number' then
		turns = __tmp_spells[spl].activate
	else
		turns = __tmp_spells[spl].activate[1] .. '+d' .. __tmp_spells[spl].activate[2]
	end
	return __tmp_spells[spl].desc[1] .. ' every ' .. turns .. ' turns'
end

-- Compute the timeout of an activation
function get_activation_timeout(spl)
	if type(__tmp_spells[spl].activate) == 'number' then
		return __tmp_spells[spl].activate
	else
		return __tmp_spells[spl].activate[1] + randint(__tmp_spells[spl].activate[2])
	end
end

-- Fire off the spell
function activate_activation(spl, item)
	__spell_spell[spl](item)
end


------- Add new GF type ----------
max_gf = MAX_GF
function add_spell_type(t)
	t.index = max_gf
	max_gf = max_gf + 1
	assert(t.color, "No GF color")
	if not t.monster then t.monster = function() end end
	if not t.angry then t.angry = function() end end
	if not t.object then t.object = function() end end
	if not t.player then t.player = function() end end
	if not t.grid then t.grid = function() end end

	add_hooks
	{
		[HOOK_GF_COLOR] = function (gf, new_gfx)
			local t = %t
			if gf == t.index then return TRUE, t.color[new_gfx + 1] end
		end,
		[HOOK_GF_EXEC] = function (action, who, gf, dam, rad, y, x, extra)
			local t = %t
			if t.index == gf then
				return t[action](who, dam, rad, y, x, extra)
			end
		end,
	}
	return t.index
end
