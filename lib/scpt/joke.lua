-- Place a monster in a good spot
function gen_joke_place_monster(r_idx)
	local try = 1000
	local x
	local y
	while try > 0 do
		x = randint(cur_hgt - 4) + 2
		y = randint(cur_wid - 4) + 2
		if not (0 == place_monster_one(y, x, r_idx, 0, FALSE, MSTATUS_ENEMY)) then
			return
		end
		try = try - 1
	end
end

-- Check if a special joke monster can be generated here
function gen_joke_monsters()
	if joke_monsters == FALSE then
		return
	end

	-- Neil
	if (current_dungeon_idx == 20) and (dun_level == 72) then
		neil = test_monster_name("Neil, the Sorceror")
		m_allow_special[neil + 1] = TRUE
		gen_joke_place_monster(neil)
		m_allow_special[neil + 1] = FALSE
	end
end

add_hook_script(HOOK_LEVEL_END_GEN, "gen_joke_monsters", "gen_joke_monsters")
