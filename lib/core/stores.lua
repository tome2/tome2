-- Take care of all concerning stores
function store_buy_list(t)
	assert(type(t) == "table", "store_buy_list got no table")
	add_hooks
	{
		[HOOK_STORE_BUY] = function (index, name, obj)
			local tbl = %t
			local elt = tbl[index]
			if not elt then
				elt = tbl[name]
			end
			if elt then
				if elt then
					if type(elt) == "function" then
						return TRUE, elt(obj)
					elseif type(elt) == "table" then
						local k, e
						for k, e in elt do
							if type(e) == "number" then
								if obj.tval == e then return TRUE, TRUE end
							else
								if (obj.tval == e[1]) and (obj.sval >= e[2])  and (obj.sval <= e[3]) then return TRUE, TRUE end
							end
						end
					elseif elt == -1 then
						return TRUE, FALSE
					end
				end
			end
		end,
	}
end
