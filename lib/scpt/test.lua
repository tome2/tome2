--
-- This file takes care of providing the Shiny-Test class
-- with .. erm .. powerful spells
--

------------------------------ MAGICTYPE -- M KEY ---------------------------------

zog_magic = add_magic
{
	["fail"] =      function(chance)
			msg_print("So bad, we had "..chance.." chances to succeed.")
			msg_print("Hooo bad luck the spell backfires !.")
			take_hit("stupidity", 5)
	end,
	["stat"] =      A_CON,
	-- Must return a number between 0 and 50 representing a level
	["get_level"] = function()
			return get_skill_scale(SKILL_MAGIC, 25) + get_skill_scale(SKILL_SPIRITUALITY, 25)
	end,
	["spell_list"] =
	{
		{
			["name"] = "Zog1",
			["desc"] = "dessssc zog1",
			["mana"] = 1,
			["level"] = 1,
			["fail"] = 10,
			["spell"] = function()
				local ret, dir
				-- Get a direction
				ret, dir = get_aim_dir();
				if (ret == FALSE) then return end
				fire_ball(GF_MANA, dir, 2000, 10)
			end,
			["info"] = function()
				return " dam 2000"
			end,
		},
		{
			["name"] = "Zog2",
			["desc"] = "dessssc zog2",
			["mana"] = 3,
			["level"] = 3,
			["fail"] = 30,
			["spell"] = function()
				local ret, item, obj, o_name
	
				-- Ask for an item
				ret, item = get_item("What to uber-ize?",
						     "You have nothing you can uber-ize",
						     bor(USE_INVEN, USE_EQUIP),
						     function (obj)
							if (obj.tval == TV_HAFTED) or (obj.tval == TV_SWORD) or (obj.tval == TV_POLEARM) or (obj.tval == TV_AXE) then
								return TRUE
							end
							return FALSE
						     end
				)
	
				if ret == TRUE then
					-- get the item
					obj = get_object(item)
					-- modify it
					obj.dd = 255
					obj.ds = 255
					obj.to_d = 1000
					obj.to_h = 1000

					-- get the name
					o_name = object_desc(obj, FALSE, 0);
					msg_print("Your "..o_name.." is hit by a pure wave of uber-ification!")
				end
			end,
			["info"] = function()
				return " cooool"
			end,
		},
		{
			["name"] = "Zog3",
			["desc"] = "dessssc zog3",
			["mana"] = 4,
			["level"] = 5,
			["fail"] = 50,
			["spell"] = function()
				local list = {[1] = "Novice Warrior", [2] = "Novice Mage"}
				local x, y, num, max
	
				num = rand_range(1, 2)
				max = damroll(1, 2)
				while (max > 0) do
					y, x = find_position(player.py, player.px)
					place_monster_one(y, x, test_monster_name(list[num]), 0, FALSE, MSTATUS_FRIEND)
					max = max - 1
				end
			end,
			["info"] = function()
				return " summons 1d2 monsters"
			end,
		},
	},
}

-- Register a new magic type
MKEY_SHINY_TEST = 1000
add_mkey
{
	["mkey"] =      MKEY_SHINY_TEST,
	["fct"] =       function()
		execute_magic(zog_magic)

		-- use up some energy
		energy_use = energy_use + 100;
	end
}


------------------------------ EXTRA POWERS ---------------------------------


-- Register a new power (the 'U' menu)
POWER_TEST = add_power
{
	["name"] =      "Test power",
	["desc"] =      "You are a shinny test",
	["desc_get"] =  "You become a shinny test",
	["desc_lose"] = "You are no more a shinny test",
	["level"] =     1,
	["cost"] =      5,
	["stat"] =      A_CON,
	["fail"] =      6,
	["power"] =     function()
		msg_print("Zogzog !")
	end,
}


---- tests

function test_write()
	local conn = zsock:new_connection()
	zsock:setup(conn, "192.168.0.200", 2262, ZSOCK_TYPE_TCP, FALSE)
	zsock:open(conn)

	local res, len = zsock:write(conn, "footest", FALSE)
	msg_print("res "..res.." :: len "..len)

	zsock:close(conn)
	zsock:unsetup(conn)
	zsock:free_connection(conn);
end

function test_read()
	local conn = zsock:new_connection()
	zsock:setup(conn, "192.168.0.200", 2262, ZSOCK_TYPE_TCP, FALSE)
	zsock:open(conn)


	zsock:wait(conn, 50)
	local res, str, len = zsock:read(conn, 9, TRUE)
	msg_print("res "..res.." :: len "..len.." '"..str.."'")

	zsock:close(conn)
	zsock:unsetup(conn)
	zsock:free_connection(conn);
end


-- A level generator being tested

CORRIDOR = 1
ROOM = 2

possible_walls = {}

function level_generator_dungeon2_room(feat, y, x, h, w)
	if feat == CORRIDOR then
		-- Add the possible walls
		for i = x, x + w - 1 do
			tinsert(possible_walls, {y, i})
			tinsert(possible_walls, {y + h - 1, i})
		end
		for i = x, x + w - 1 do
			tinsert(possible_walls, {x, i})
			tinsert(possible_walls, {x, i + h - 1})
		end

		for i = x + 1, x + w - 2 do
			for j = y + 1, y + h - 2 do
				place_floor(j, i)
			end
		end
	else
		-- Add the possible walls
		for i = x, x + w - 1 do
			tinsert(possible_walls, {y, i})
			tinsert(possible_walls, {y + h - 1, i})
		end
		for i = x, x + w - 1 do
			tinsert(possible_walls, {x, i})
			tinsert(possible_walls, {x, i + h - 1})
		end

		for i = x, x + w - 1 do
			for j = y, y + h - 1 do
				cave(j, i).feat = 56
			end
		end
		for i = x + 1, x + w - 2 do
			for j = y + 1, y + h - 2 do
				place_floor(j, i)
			end
		end
	end
end

function select_feature(dir)
	if magik(30) == TRUE then
		return ROOM, rand_range(5, 12), rand_range(7, 17)

	-- Corridor selection
	elseif dir == "up" or dir == "down" then
		return CORRIDOR, rand_range(5, 17), 3
	else
		return CORRIDOR, 3, rand_range(3, 15)
	end
end

function put_feature(feat, y, x, h, w)
	level_generator_dungeon2_room(feat, y, x, h, w)
end

function can_feature(y, x, h, w)
	for i = x, x + w - 1 do
		for j = y, y + h - 1 do
			if j <= 0 or i <= 0 or i >= cur_wid - 1 or j >= cur_hgt - 1 then return nil end

			if cave_is(cave(j, i), FF1_WALL) == FALSE then return nil end
		end
	end
	return not nil
end

function is_near_wall(y, x)
	if y <= 0 or x <= 0 or x >= cur_wid - 1 or y >= cur_hgt - 1 then return nil end

	if cave_is(cave(y, x), FF1_WALL) == FALSE then return nil end
	if cave_is(cave(y - 1, x), FF1_FLOOR) == TRUE then return "down" end
	if cave_is(cave(y + 1, x), FF1_FLOOR) == TRUE then return "up" end
	if cave_is(cave(y, x - 1), FF1_FLOOR) == TRUE then return "right" end
	if cave_is(cave(y, x + 1), FF1_FLOOR) == TRUE then return "left" end
	return nil
end

function find_spot()
--[[	local y, x = 1, 1

	while not is_near_wall(y, x) do
		y, x = rand_range(1, cur_hgt - 2), rand_range(1, cur_wid - 2)
	end]]
	local i = rand_range(1, getn(possible_walls))
	local y, x = possible_walls[i][1], possible_walls[i][2]

	while not is_near_wall(y, x) do
		i = rand_range(1, getn(possible_walls))
		y, x = possible_walls[i][1], possible_walls[i][2]
	end

	tremove(possible_walls, i)
	return is_near_wall(y, x), y, x
end

function adjust_dir(dir, y, x, h, w)
	if dir == "up" then
		y = y - (h - 1)
		x = x - (w / 2)
	elseif dir == "down" then
		x = x - (w / 2)
	elseif dir == "left" then
		y = y - (h / 2)
		x = x - (w - 1)
	elseif dir == "up" then
		y = y - (h / 2)
	end
	return y, x
end

level_generator
{
	["name"] = "dungeon2",
	["stairs"] = FALSE,
	["monsters"] = FALSE,
	["objects"] = FALSE,
	["miscs"] = FALSE,
	["gen"] = function()
		for i = 1, cur_wid - 1 do
			for j = 1, cur_hgt - 1 do
				place_filler(j, i)
			end
		end


		-- the first room
		level_generator_dungeon2_room(ROOM, cur_hgt / 2, cur_wid / 2, rand_range(3, 10), rand_range(5, 15))


		-- Place 10 features
		for nb = 1, 0 do
			-- Find a spot near an empty space
			local dir, y, x = find_spot()

			local feat, h, w = select_feature(dir)

			local sy, sx = adjust_dir(dir, y, x, h, w)

			if can_feature(sy, sx, h, w) then
				put_feature(feat, sy, sx, h, w)
				cave(y, x).feat = 32
			end
		end

		player.py = cur_hgt / 2
		player.px = cur_wid / 2
		return TRUE
	end,
}


-- exmaple of display_list
function input_list_example()
	local list = { "a", "b", "c", "d", "e", "f", "h", "g" }
	local sel = 1
	local begin = 1
	local res = nil

	screen_save()

	while not nil do
		display_list(1, 0, 5, 9, "select", list, begin, sel, TERM_L_BLUE)

		local key = inkey()

		if key == ESCAPE then break
		elseif key == strbyte("8") then
			sel = sel - 1
			if sel < 1 then sel = 1 end
			if sel < begin then
				begin = begin - 1
			end
		elseif key == strbyte("2") then
			sel = sel + 1
			if sel > getn(list) then sel = getn(list) end
			if sel >= begin + 4 then
				begin = begin + 1
			end
		elseif key == strbyte("\r") then
		       res = list[sel]
		       break
		end
	end

	screen_load()

	if res then msg_print("Selected: " .. res) end
end
