-- Internal lua file in charge of dungeon stuff

function place_dungeon(y, x, d_idx)
	if d_idx then
		wild_map(y, x).entrance = 1000 + d_idx
       	else
		wild_map(y, x).entrance = 0
	end
end

function dungeon(d_idx)
	return d_info[1 + d_idx]
end

function wild_feat(wild)
	return wf_info[1 + wild.feat]
end

function explode_dir(dir)
	return ddy[dir + 1], ddx[dir + 1]
end

function rotate_dir(dir, mov)
	if mov > 0 then
		if dir == 7 then dir = 8
		elseif dir == 8 then dir = 9
		elseif dir == 9 then dir = 6
		elseif dir == 6 then dir = 3
		elseif dir == 3 then dir = 2
		elseif dir == 2 then dir = 1
		elseif dir == 1 then dir = 4
		elseif dir == 4 then dir = 7
		end
	elseif mov < 0 then
		if dir == 7 then dir = 4
		elseif dir == 4 then dir = 1
		elseif dir == 1 then dir = 2
		elseif dir == 2 then dir = 3
		elseif dir == 3 then dir = 6
		elseif dir == 6 then dir = 9
		elseif dir == 9 then dir = 8
		elseif dir == 8 then dir = 7
		end
	end

	return dir
end

-- Check if the map is a filename or directly a map
function load_map(map, y, x)
	if strsub(map, 1, 5) == "#!map" then
		%load_map(TRUE, map, y, x)
	else
		%load_map(FALSE, map, y, x)
	end
end
function get_map_size(map)
	if strsub(map, 1, 5) == "#!map" then
		return %get_map_size(TRUE, map)
	else
		return %get_map_size(FALSE, map)
	end
end

-- Place a trap for a specific level
function place_trap(y, x, level)
	local old_dun = dun_level
	dun_level = level
	%place_trap(y, x)
	dun_level = old_dun
end

-- Level generators processing
__level_generators = {}

function level_generator(t)
	assert(t.name, "no generator name")
	assert(t.gen, "no generator function")

	if not t.stairs then t.stairs = TRUE end
	if not t.monsters then t.monsters = TRUE end
	if not t.objects then t.objects = TRUE end
	if not t.miscs then t.miscs = TRUE end

	__level_generators[t.name] = t.gen
	add_scripted_generator(t.name, t.stairs, t.monsters, t.objects, t.miscs)
end

function level_generate(name)
	assert(__level_generators[name], "Unknown level generator '"..name.."'")
	return __level_generators[name]()
end

--[[ Example
level_generator
{
	["name"]	= "test",
	["gen"]	 = function()
			print("zog")
			for i = 1, 30 do
				cave(i, 2).feat = 1
			end
			return new_player_spot(get_branch())
	end,
}
]]
