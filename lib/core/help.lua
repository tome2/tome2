-- Ingame contextual help

-- We use our own hook list as to not overburn the hook proccessor
-- with many hooks that would slow down things
-- It would be very meaningless if the option is not even on
__ingame_hooks = {}

__ingame_help_max = 0

function ingame_help(t, ...)
	-- This function can also be used to call the callbacks
	if type(t) == "string" then
		local f = getglobal("__ingame_help_fct_"..t)
		call(f, arg)
		return
	end

	assert(t.desc or t.fct, "no ingame help desc/fct")
	assert(t.hook or t.callback, "no ingame help hook/callback")
	if t.hook then assert(t.event, "no ingame hepl event needed by hook") end

	-- Set it to only trigger once
	setglobal("__ingame_help_activated_"..__ingame_help_max, FALSE)
	-- Save/load it
	add_loadsave("__ingame_help_activated_"..__ingame_help_max, FALSE)

	if t.hook then
		-- If the hok list didnt exist yet, add it
		if not __ingame_hooks[t.hook] then
			-- Set it to empty, we'll fill it later
			__ingame_hooks[t.hook] = {}
			-- Add the global hook
			add_hooks
			{
				[t.hook] = function (...)
					if option_ingame_help ~= TRUE then return end
					local k, e
					for k, e in __ingame_hooks[%t.hook] do
						if k ~= "n" then
							call(e, arg)
						end
					end
				end
			}
		end
		if t.desc then
			tinsert(__ingame_hooks[t.hook],
				function (...)
					local tbl = %t
					if getglobal("__ingame_help_activated_"..%__ingame_help_max) == FALSE then
						if call(tbl.event, arg) == TRUE then
							local k, e
							for k, e in tbl.desc do
								msg_print(TERM_YELLOW, e)
							end
							setglobal("__ingame_help_activated_"..%__ingame_help_max, TRUE)
						end
					end
				end
			)
		elseif t.fct then
			tinsert(__ingame_hooks[t.hook],
				function (...)
					local tbl = %t
					if getglobal("__ingame_help_activated_"..%__ingame_help_max) == FALSE then
						if call(tbl.event, arg) == TRUE then
							if tbl.fct() == TRUE then
								setglobal("__ingame_help_activated_"..%__ingame_help_max, TRUE)
							end
						end
					end
				end
			)
		end
	else
		local no_test = FALSE
		if t.no_test == TRUE then no_test = TRUE end
		if t.desc then
			setglobal
			(
				"__ingame_help_fct_"..(t.callback),
				function (...)
					local tbl = %t
					if ((option_ingame_help == TRUE) or (%no_test == TRUE)) and (getglobal("__ingame_help_activated_"..%__ingame_help_max) == FALSE) then
						local k, e
						for k, e in tbl.desc do
							msg_print(TERM_YELLOW, e)
						end
						setglobal("__ingame_help_activated_"..%__ingame_help_max, TRUE)
					end
				end
			)
		elseif t.fct then
			setglobal
			(
				"__ingame_help_fct_"..(t.callback),
				function (...)
					local tbl = %t
					if ((option_ingame_help == TRUE) or (%no_test == TRUE)) and (getglobal("__ingame_help_activated_"..%__ingame_help_max) == FALSE) then
						if call(tbl.fct, arg) == TRUE then
							setglobal("__ingame_help_activated_"..%__ingame_help_max, TRUE)
						end
					end
				end
			)
		end
	end

	__ingame_help_max = __ingame_help_max + 1
end

-- Clean up the ingame help seen at birth
add_hooks
{
	[HOOK_BIRTH_OBJECTS] = function()
		local i
		for i = 0, __ingame_help_max - 1 do
			setglobal("__ingame_help_activated_"..i, FALSE)
		end
	end
}

function ingame_clean()
	local i
	for i = 0, __ingame_help_max - 1 do
		setglobal("__ingame_help_activated_"..i, FALSE)
	end
end

-- helper function, brings up a doc
function ingame_help_doc(name, anchor)
	-- Save screen
	screen_save();

	-- Peruse the help file
	if not anchor then anchor = 0 end
	show_file(name, 0, -anchor, 0)

	-- Load screen
	screen_load()
end
