-- Ok some functions that we dont need are dangerous
--[[
execute = nil
getenv = nil
setlocale = nil
exit = nil
openfile = nil
writeto = nil
readfrom = nil
appendto = nil
remove = nil
rename = nil
tmpname = nil
]]
modules = {}

current_module = nil

function setup_module(mod)
	-- For standart game, nothing needs to be done
	if not mod.layout then return end

	for k, e in mod.layout do
		module_reset_dir(k, e)
	end
end

function init_module(i)
	setup_module(get_module(i))
end

function max_modules()
	local i = 0
	for k, e in modules do
		if type(k) == "number" and type(e) == "table" then
			i = i + 1
		end
	end
	return i
end

function get_module_name(j)
	local i = 0
	for k, e in modules do
		if type(k) == "number" and type(e) == "table" then
			if i == j then return e.name end
			i = i + 1
		end
	end
end

function get_module_desc(j)
	local i = 0
	for k, e in modules do
		if type(k) == "number" and type(e) == "table" then
			if i == j then return e.desc end
			i = i + 1
		end
	end
end

function get_module(j)
	local i = 0
	for k, e in modules do
		if type(k) == "number" and type(e) == "table" then
			if i == j then return e end
			i = i + 1
		end
	end
end

function find_module(name)
	local i = 0
	for k, e in modules do
		if type(k) == "number" and type(e) == "table" then
			if name == e.name then return i end
			i = i + 1
		end
	end
end

function assign_current_module(name)
	current_module = get_module(find_module(name))
end

function get_module_info(type, subtype)
	if subtype then
		return current_module[type][subtype]
	else
		return current_module[type]
	end
end

function exec_module_info(type, ...)
	return call(current_module[type], arg)
end

function module_savefile_loadable(savefile_mod, savefile_death)
	for _, e in current_module.mod_savefiles do
		if e[1] == savefile_mod then
			if e[2] == "all" then
				return TRUE
			elseif e[2] == "alive" and savefile_death == FALSE then
				return TRUE
			elseif e[2] == "dead" and savefile_death == TRUE then
				return TRUE
			end
		end
	end
	return FALSE
end

function scan_extra_modules()
	scansubdir(ANGBAND_DIR_MODULES)
	for i = 0, scansubdir_max - 1 do
		if (scansubdir_result[i + 1] ~= ".") and (scansubdir_result[i + 1] ~= "..") then
			local dir = path_build(ANGBAND_DIR_MODULES, scansubdir_result[i + 1])
			local file = path_build(dir, "module.lua")
			if file_exist(file) == TRUE then
				tome_dofile_anywhere(dir, "module.lua")
			end
		end
	end
end

function add_module(t)
	assert(t.name, "No module name")
	assert(type(t.version) == "table", "No module version")
	assert(t.desc, "No module desc")
	assert(t.author, "No module author")
	assert(t.mod_savefiles, "No loadable savefiles module mark")

	for _, e in modules do
		if type(e) == "table" and e.name == t.name then
			error("Module name already defined: "..t.name)
		end
	end

	if type(t.author) == "string" then
		t.author = { t.author, "unknown@unknown.net" }
	end

	for k, e in t.mod_savefiles do
		if type(e) == "string" then t.mod_savefiles[k] = { e, "all" } end
	end

	if type(t.desc) == "table" then
		local d = ""
		for k, e in t.desc do
			d = d .. e
			if k < getn(t.desc) then
				d = d .. "\n"
			end
		end
		t.desc = d
	end

	if not t.rand_quest then t.rand_quest = FALSE end
	if not t.C_quest then t.C_quest = FALSE end

	if not t.base_dungeon then t.base_dungeon = 4 end
	if not t.death_dungeon then t.death_dungeon = 28 end

	if not t.astral_dungeon then t.astral_dungeon = 8 end
	if not t.astral_wild_x then t.astral_wild_x = 45 end
	if not t.astral_wild_y then t.astral_wild_y = 19 end

	if not t.random_artifact_weapon_chance then
		t.random_artifact_weapon_chance = 30
	end
	if not t.random_artifact_armor_chance then
		t.random_artifact_armor_chance = 20
	end
	if not t.random_artifact_jewelry_chance then
		t.random_artifact_jewelry_chance = 20
	end

	if not t.max_plev then t.max_plev = 50 end
	if not t.max_skill_overage then t.max_skill_overage = 4 end
	if not t.skill_per_level then t.skill_per_level = function() return 6 end end

	if not t.allow_birth then t.allow_birth = TRUE end

	tinsert(modules, t)
end
