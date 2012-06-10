--
-- This file is loaded at the initialisation of ToME
--

-- Add the schools of magic
schools_init()
school_spells_init()
init_school_books()

-- Post-spell creation initialization
initialize_bookable_spells()

-- Add joke stuff
tome_dofile("joke.lua")

-- Some tests, if the file is not present, this is fine
tome_dofile_anywhere(ANGBAND_DIR_SCPT, "dg_test.lua", FALSE)
