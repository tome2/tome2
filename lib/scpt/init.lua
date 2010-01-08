--
-- This file is loaded at the initialisation of ToME
--

-- Load the class specific stuff
tome_dofile("player.lua")

-- Load the ingame contextual help
tome_dofile("help.lua")

-- let the store specific stuff happen!
tome_dofile("stores.lua")

-- Add various 'U' powers
tome_dofile("powers.lua")

-- Add the mimic shapes
tome_dofile("mimic.lua")

-- Add the corruptions
tome_dofile("corrupt.lua")

-- Add the mkey activations
tome_dofile("mkeys.lua")

-- Add the schools of magic
tome_dofile("spells.lua")

-- Add god stuff
tome_dofile("gods.lua")

-- Add some quests
tome_dofile("bounty.lua")
tome_dofile("god.lua")
tome_dofile("fireprof.lua")
tome_dofile("library.lua")

-- Add joke stuff
tome_dofile("drunk.lua")
tome_dofile("joke.lua")

-- Some tests, if the file is not present, this is fine
tome_dofile_anywhere(ANGBAND_DIR_SCPT, "dg_test.lua", FALSE)

-- A nice custom intro :)
tome_dofile("intro.lua")
