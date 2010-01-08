-- silly function that allows a drunk to take a bottle of wine/ale from the player

function drunk_takes_wine(m_idx, item)

	m_ptr = monster(m_idx)
	o_ptr = get_object(item)

	if (m_ptr.r_idx == test_monster_name("Singing, happy drunk")) 
			and (o_ptr.tval == TV_FOOD) and ((o_ptr.sval == 38) or (o_ptr.sval == 39)) then

		cmsg_print(TERM_YELLOW, "'Hic!'")

		inven_item_increase(item, -1)
		inven_item_optimize(item)
		return TRUE
	else
		return FALSE
	end
end

add_hook_script(HOOK_GIVE, "drunk_takes_wine", "drunk_takes_wine")
