-- SYSTEM FILE
--
-- Lua object funtions
--

function create_object(tval, sval)
	local obj = new_object()
	object_prep(obj, lookup_kind(tval, sval))
	return (obj)
end

function set_item_tester(tester)
	if tolua.type(tester) == "number" then
		lua_set_item_tester(tester, "")
	end
	if tolua.type(tester) == "string" then
		lua_set_item_tester(0, tester)
	end
	if tolua.type(tester) == "function" then
		__get_item_hook_default = tester
		lua_set_item_tester(0, "__get_item_hook_default")
	end
end

function create_artifact(a_idx)
	local obj
	local tval, sval

	tval = a_info[a_idx + 1].tval
	sval = a_info[a_idx + 1].sval
	obj = create_object(tval, sval)
	obj.name1 = a_idx
	apply_magic(obj, -1, TRUE, TRUE, TRUE)

	return (obj)
end

function get_kind(obj)
	return k_info[obj.k_idx + 1]
end

function get_item(ask, deny, flags, mask)
	set_item_tester(mask)
	return get_item_aux(0, ask, deny, flags)
end
