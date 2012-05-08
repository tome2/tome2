-- various stuff to make scripters life easier

-- Beware of the scary undefined globals
function safe_getglobal(x)
	local v = rawget(globals(), x)

	if v then
		return v
	else
		error("undefined global variable '"..x.."'")
	end
end

function set_safe_globals()
	settagmethod(tag(nil), "getglobal", safe_getglobal)
end
function unset_safe_globals()
	settagmethod(tag(nil), "getglobal", nil)
end

set_safe_globals()

-- Patch modules
__patch_modules = {}

function patch_version(name, version)
	assert(not __patch_modules[name], "Patch " .. name .. " already loaded!!!")
	__patch_modules[name] = version
end

function patchs_list()
	local k, e, first
	first = FALSE
	for k, e in __patch_modules do
		if first == FALSE then print_hook("\n\n  [Patch modules]\n") first = TRUE end
		print_hook("\n "..k.." version "..e)
	end
	if first == TRUE then print_hook("\n") end
end

function patchs_display()
	local k, e
	for k, e in __patch_modules do
		msg_print("Patch: "..k.." version "..e)
	end
end


-- Better hook interface
__hooks_list_callback = {}
__hooks_list_callback_max = 0

function add_hooks(h_table, name_prefix)
	local k, e

	if not name_prefix then name_prefix = "" end
	for k, e in h_table do
		add_hook_script(k, "__"..name_prefix.."__hooks_list_callback"..__hooks_list_callback_max, "__"..name_prefix.."__hooks_list_callback"..__hooks_list_callback_max)
		setglobal("__"..name_prefix.."__hooks_list_callback"..__hooks_list_callback_max, e)
		__hooks_list_callback_max = __hooks_list_callback_max + 1
	end
end

-- Wrapper for the real msg_print and cmsg_print
-- it understands if we want color or not
function msg_print(c, m)
	if type(c) == "number" then
		cmsg_print(c, m)
	else
		call(%msg_print, { c })
	end
end

-- better timer add function
__timers_callback_max = 0

function new_timer(t)
	assert(t.delay > 0, "no timer delay")
	assert(t.enabled, "no timer enabled state")
	assert(t.callback, "no timer callback")

	local timer
	if type(t.callback) == "function" then
		setglobal("__timers_callback_"..__timers_callback_max, t.callback)
		timer = %new_timer("__timers_callback_"..__timers_callback_max, t.delay)
		__timers_callback_max = __timers_callback_max + 1
	else
		timer = %new_timer(t.callback, t.delay)
	end

	timer.enabled = t.enabled

	return timer
end


-- displays a scrolling list
function display_list(y, x, h, w, title, list, begin, sel, sel_color)
	local l = create_list(getn(list))

	for i = 1, getn(list) do
		add_to_list(l, i - 1, list[i])
	end

	%display_list(y, x, h, w, title, l, getn(list), begin - 1, sel - 1, sel_color)

	delete_list(l, getn(list))
end

-- Easier access to special gene stuff
function set_monster_generation(monster, state)
	if type(monster) == "string" then
		m_allow_special[test_monster_name(monster) + 1] = state
	else
		m_allow_special[monster + 1] = state
	end
end
function set_object_generation(obj, state)
	if type(obj) == "string" then
		m_allow_special[test_item_name(obj) + 1] = state
	else
		m_allow_special[obj + 1] = state
	end
end
function set_artifact_generation(obj, state)
	m_allow_special[obj + 1] = state
end

-- Strings
function strcap(str)
	if strlen(str) > 1 then
		return strupper(strsub(str, 1, 1))..strsub(str, 2)
	elseif strlen(str) == 1 then
		return strupper(str)
	else
		return str
	end
end

function msg_format(...)
	msg_print(call(format, arg))
end

-- A way to  check if the game is now running(as opposed to initialization/character gen)
game = {}
add_hooks
{
	[HOOK_GAME_START] = function ()
		game.started = TRUE
	end
}
