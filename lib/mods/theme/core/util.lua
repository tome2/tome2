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

-- Wrapper for the real msg_print and cmsg_print
-- it understands if we want color or not
function msg_print(c, m)
	if type(c) == "number" then
		cmsg_print(c, m)
	else
		call(%msg_print, { c })
	end
end


-- Strings
function msg_format(...)
	msg_print(call(format, arg))
end
