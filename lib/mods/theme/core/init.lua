--
-- This file is loaded at the initialisation of ToME
-- Load the system functions
--

-- Very thin xml parser(49 lines ;)
tome_dofile_anywhere(ANGBAND_DIR_CORE, "xml.lua")

-- various vital helper code
tome_dofile_anywhere(ANGBAND_DIR_CORE, "util.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "player.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "objects.lua")
tome_dofile_anywhere(ANGBAND_DIR_CORE, "monsters.lua")
