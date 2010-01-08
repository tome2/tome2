-- Mimic shapes helper file

__mimics = {}
__mimics_max = 1
__mimics_names = {}

function add_mimic_shape(t)
	assert(t.name, "no mimic name")
	assert(t.desc, "no mimic desc")
	assert(t.calc, "no mimic calc")
	assert(t.level, "no mimic level")
	assert(t.duration, "no mimic duration")

	if not t.limit then t.limit = 0 end

	if not t.obj_name then
		t.obj_name = t.name
	end

	t.show_name = '['..t.name..']'

	-- if it needs hooks, add them
	if t.hooks then
		add_hooks(t.hooks)
	end

	-- Add it in a name to index hash table
	__mimics_names[t.name] = __mimics_max

	__mimics[__mimics_max] = t
	__mimics_max = __mimics_max + 1
end

function resolve_mimic_name(name)
	if __mimics_names[name] then
		return __mimics_names[name]
	else
		return -1
	end
end

function find_random_mimic_shape(level, limit, realm)
	local mimic, tries

	tries = 1000
	while tries > 0 do
		tries = tries - 1
		mimic = rand_range(1, __mimics_max - 1)
		if (not realm) or (__mimics[mimic].realm == realm) then
			if limit >= __mimics[mimic].limit then
				if (rand_int(__mimics[mimic].level * 3) < level) and (__mimics[mimic].rarity < 100)  and (magik(100 - __mimics[mimic].rarity) == TRUE) then
					break
				end
			end
		end
	end
	if tries > 0 then
		return mimic
	else
		return resolve_mimic_name("Abomination")
	end
end

function get_mimic_info(mimic, info)
	if not __mimics[mimic] then return 0 end
	return __mimics[mimic][info]
end

function get_mimic_rand_dur(mimic)
	return rand_range(__mimics[mimic].duration[1], __mimics[mimic].duration[2])
end

function calc_mimic(mimic)
	return __mimics[mimic].calc()
end

function calc_mimic_power(mimic)
	if __mimics[mimic].power then __mimics[mimic].power() end
end

--- Here comes the only vital shape

add_mimic_shape
{
	["name"] =      "Abomination",
	["obj_name"] =  "Abominable Cloak",
	["desc"] = 	"Abominations are failed experiments of powerful wizards.",
	["realm"] =     nil,
	["level"] =     1,
	["rarity"] =    101,
	["duration"] =  {20, 100},
	["calc"] =      function ()
			apply_flags(TR1_SPEED + TR1_STR + TR1_INT + TR1_WIS + TR1_DEX + TR1_CON + TR1_CHR, 0, TR3_AGGRAVATE, 0, 0, 0, -10)
       	end,
}
