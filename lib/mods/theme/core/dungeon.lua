-- Internal lua file in charge of dungeon stuff

function dungeon(d_idx)
	return d_info[1 + d_idx]
end

function explode_dir(dir)
	return ddy[dir + 1], ddx[dir + 1]
end
