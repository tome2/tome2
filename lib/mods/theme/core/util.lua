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
