--
-- This file is loaded at the initialisation of ToME
-- Load the system functions
--

-- Name of globals to save
tome_dofile_anywhere(ANGBAND_DIR_CORE, "load.lua")

-- Very thin xml parser(49 lines ;)
tome_dofile_anywhere(ANGBAND_DIR_CORE, "xml.lua")

-- various vital helper code
tome_dofile_anywhere(ANGBAND_DIR_CORE, "util.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "player.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "objects.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "monsters.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "powers.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "building.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "dungeon.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "s_aux.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "crpt_aux.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "mimc_aux.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "quests.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "gods.lua")

-- Load the ingame contextual help
tome_dofile_anywhere(ANGBAND_DIR_CORE, "help.lua")

-- let the store specific stuff happen!
tome_dofile_anywhere(ANGBAND_DIR_CORE, "stores.lua")

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
-------------Here we load the non vital scripts---------------
-----------------------from lib/scpt--------------------------
--------------------------------------------------------------
--------------------------------------------------------------
tome_dofile("init.lua")

-- The dofile functions for each patch
patch_dofile = {}

-- Now load patches
function load_patches()
	scansubdir(ANGBAND_DIR_PATCH)
	for i = 0, scansubdir_max - 1 do
		if (scansubdir_result[i + 1] ~= ".") and (scansubdir_result[i + 1] ~= "..") then
			local dir = path_build(ANGBAND_DIR_PATCH, scansubdir_result[i + 1])
			local file = path_build(dir, "patch.lua")
			if file_exist(file) == TRUE then
				patch_init = nil
				tome_dofile_anywhere(dir, "patch.lua", TRUE)
				unset_safe_globals()
				if patch_init == nil then
					set_safe_globals()
					quit("Patch in "..file.." did not include a patch_init() function")
				else
					set_safe_globals()

					-- create the dofile function
					patch_dofile[scansubdir_result[i + 1]] = function(f)
						tome_dofile_anywhere(%dir, f, TRUE)
					end

					local name, version = patch_init()
					if name == nil or version == nil then
						quit("Patch in "..file.." did not return valid name or version.\nIt must return name, version")
					end
					patch_version(name, version)
				end
			end
		end
	end
end
load_patches()

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
--
-- Do not thouch after this line
--
tome_dofile_anywhere(ANGBAND_DIR_CORE, "load2.lua")
