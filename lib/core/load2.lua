-- Savefile helpers

-- function called when a key in the variable part ofthe savefile is read
-- if the key matches what we need, we use it, otehrwise just ignore it
function __savefile_load(key, val)
	local index, elem
	
	for index, elem in __loadsave_name do
		if (key == elem.name) then
			dostring(elem.name.." = "..val)
	       	end
	end
end

function dump_loadsave()
	local k, e
	for k, e in __loadsave_name do
		msg_print(k.." :: ".. e.name.." ["..e.default.."]")
	end
end

-- called when the game is saved, can only save numbers
-- assosiate a key with them to allow the loading code to recognize them
function __savefile_save()
	local index, elem
	for index, elem in __loadsave_name do
		dostring("__loadsave_tmp = "..elem.name)
		save_number_key(elem.name, __loadsave_tmp);
	end
end

register_savefile(__loadsave_max)
add_hook_script(HOOK_LOAD_GAME, "__savefile_load", "__hook_load")
add_hook_script(HOOK_SAVE_GAME, "__savefile_save", "__hook_save")

-- Parse a flattened(i.e: foo.bar.zog) table path and recrate tables
function reconstruct_table(name)
	for i = 1, strlen(name) - 1 do
		if strsub(name, i, i) == "." then
			local tbl = strsub(name, 1, i - 1)

			if dostring("return "..tbl) == nil then
				dostring(tbl.."={}")
			end
		end
	end
end

-- Automagically set unkown variables, otherwise the savefile code
-- might get VERY upset
do
	local k, e
	-- We need to be able to check for unknown globals
	unset_safe_globals()
	for k, e in __loadsave_name do
		reconstruct_table(e.name)
		if dostring("return "..(e.name)) == nil then
			dostring((e.name).." = "..(e.default))
		end
	end
	-- Now taht we did, we set it back, for it is usefull ;)
	set_safe_globals()
end
