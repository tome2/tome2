function tome_intro()
	screen_save()
	Term_clear()

	if (TRUE == drop_text_left(TERM_L_BLUE, "Art thou an adventurer,", 10, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "One who passes through the waterfalls we call danger", 11, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_BLUE, "to find the true nature of the legends beyond them?", 12, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_BLUE, "If this is so, then seeketh me.", 13, -1)) then screen_load() return end

	if (TRUE == drop_text_left(TERM_WHITE, "[Press any key to continue]", 23, -1)) then screen_load() return end

	Term_putch(0, 0, TERM_DARK, 32)
	inkey_scan = FALSE
	inkey()

	Term_clear()

	if (TRUE == drop_text_left(TERM_L_BLUE, "DarkGod", 8, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_WHITE, "in collaboration with", 9, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_L_GREEN, "Eru Iluvatar,", 10, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_GREEN, "Manwe", 11, -1)) then screen_load() return end
	if (TRUE == drop_text_left(TERM_WHITE, "and", 12, 0)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_L_GREEN, "All the T.o.M.E. contributors(see credits.txt)", 13, -1)) then screen_load() return end

	if (TRUE == drop_text_left(TERM_WHITE, "present", 15, 1)) then screen_load() return end
	if (TRUE == drop_text_right(TERM_YELLOW, "T.o.M.E.", 16, 0)) then screen_load() return end

	if (TRUE == drop_text_left(TERM_WHITE, "[Press any key to continue]", 23, -1)) then screen_load() return end
	Term_putch(0, 0, TERM_DARK, 32)

	inkey_scan = FALSE

	inkey()

	screen_load()
	return
end

add_hook_script(HOOK_INIT, "tome_intro", "lua_intro_init")
