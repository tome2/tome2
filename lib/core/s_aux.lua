-- Functions to help with spells, do not touch

__schools = {}
__schools_num = 0

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

-- Creates the school books array
__spell_school = {}

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
	local lvl, sch, num, bonus
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
	if check_spell_depends(s) ~= TRUE then
		return min, "n/a"
	end

        local index = 0
        while 1 do
		sch = spell_get_school_idx(s, index)
                if sch == -1 then
                   break
                end
                index = index + 1

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

-- The real get_level, works for schooled magic and for innate powers
function get_level(s, max, min)
   if not max then max = 50 end
   if not min then min = 1  end
   return %get_level(s, max, min)
end

