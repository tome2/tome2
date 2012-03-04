function tome_intro()
	screen_save()
	Term_clear()

	if (TRUE == drop_text_left(TERM_L_BLUE, "Three Rings for the Elven-kings under the sky,", 10, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "Seven for the Dwarf-lords in their halls of stone,", 11, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_BLUE, "Nine for Mortal Men doomed to die,", 12, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "One for the Dark Lord on his dark throne", 13, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_BLUE, "In the land of Mordor, where the Shadows lie.", 14, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "One Ring to rule them all, One Ring to find them,", 15, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_BLUE, "One Ring to bring them all and in the darkness bind them", 16, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "In the land of Mordor, where the Shadows lie.", 17, -1)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_GREEN, "--J.R.R. Tolkien", 18, 0)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_WHITE, "[Press any key to continue]", 23, -1)) then screen_load() return end

	Term_putch(0, 0, TERM_DARK, 32)
	inkey_scan = FALSE
	inkey()

	Term_clear()

	if (TRUE == drop_text_left(TERM_L_BLUE, "furiosity", 8, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_WHITE, "in collaboration with", 9, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_GREEN, "DarkGod and all the ToME contributors,", 10, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_GREEN, "module creators, t-o-m-e.net forum posters,", 11, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_WHITE, "and", 12, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_GREEN, "by the grace of the Valar", 13, -1)) then screen_load() return end

	if (TRUE == drop_text_left(TERM_WHITE, "present", 15, 1)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_YELLOW, "Theme (a module for ToME)", 16, 0)) then screen_load() return end

	if (TRUE == drop_text_left(TERM_WHITE, "[Press any key to continue]", 23, -1)) then screen_load() return end
	Term_putch(0, 0, TERM_DARK, 32)

	inkey_scan = FALSE

	inkey()

	screen_load()
	return
end

add_hook_script(HOOK_INIT, "tome_intro", "lua_intro_init")
