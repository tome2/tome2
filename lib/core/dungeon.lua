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

-- Place a trap for a specific level
function place_trap(y, x, level)
	local old_dun = dun_level
	dun_level = level
	%place_trap(y, x)
	dun_level = old_dun
end
