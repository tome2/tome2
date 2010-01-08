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

-- Returns the direction of the compass that y2, x2 is from y, x
-- the return value will be one of the following: north, south,
-- east, west, north-east, south-east, south-west, north-west,
-- or "close" if it is within 2 tiles.
function compass(y, x, y2, x2)
	local y_axis, x_axis, y_diff, x_diff, compass_dir

	-- is it close to the north/south meridian?
	y_diff = y2 - y

	-- determine if y2, x2 is to the north or south of y, x
	if (y_diff > -3) and (y_diff < 3) then
		y_axis = nil
	elseif y2 > y then
		y_axis = "south"
	else
		y_axis = "north"
	end

	-- is it close to the east/west meridian?
	x_diff = x2 - x

	-- determine if y2, x2 is to the east or west of y, x
	if (x_diff > -3) and (x_diff < 3) then
		x_axis = nil
	elseif x2 > x then
		x_axis = "east"
	else
		x_axis = "west"
	end

	-- Maybe it is very close
	if ((not x_axis) and (not y_axis)) then compass_dir = "close"
	-- Maybe it is (almost) due N/S
		elseif not x_axis then compass_dir = y_axis
	-- Maybe it is (almost) due E/W
		elseif not y_axis then compass_dir = x_axis
	-- or if it is neither
		else compass_dir = y_axis.."-"..x_axis
	end

	return compass_dir
end

-- Returns a relative approximation of the 'distance' of y2, x2 from y, x.
function approximate_distance(y, x, y2, x2)
	local y_diff, x_diff, most_dist

	-- how far to away to the north/south
	y_diff = y2 - y

	-- make sure it's a positive integer
	if y_diff < 0 then
		y_diff = 0 - y_diff
	end

	-- how far to away to the east/west
	x_diff = x2 - x

	-- make sure it's a positive integer
	if x_diff < 0 then
		x_diff = 0 - x_diff
	end

	-- find which one is the larger distance
	if x_diff > y_diff then
		most_dist = x_diff
	else
		most_dist = y_diff
	end

	-- how far away then?
	if most_dist >= 41 then
		how_far = "a very long way"
	elseif most_dist >= 25 then
		how_far = "a long way"
	elseif most_dist >= 8 then
		how_far = "quite some way"
	else
		how_far = "not very far"
	end

	return how_far

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

-- saves all timer values
function save_timer(name)
	add_loadsave(name..".enabled", FALSE)
	add_loadsave(name..".delay", 1)
	add_loadsave(name..".countdown", 1)
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

-- Stacks
function stack_push(stack, val)
	tinsert(stack, val)
end
function stack_pop(stack)
	if getn(stack) >= 1 then
		return tremove(stack)
	else
		error("Tried to unstack an empty stack")
		return nil
	end
end

-- A way to  check if the game is now running(as opposed to initialization/character gen)
game = {}
add_hooks
{
	[HOOK_GAME_START] = function ()
		game.started = TRUE
	end
}
