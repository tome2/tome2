------------------------------------------------------------------------------
----------------------- Hook to create birth objects -------------------------
------------------------------------------------------------------------------
function __birth_hook_objects()

	-- Grace delay for adding piety
	GRACE_DELAY = 0

	-- Provide a book of Geyser to Geomancers
	if get_class_name() == "Geomancer" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Geyser")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Provide a book of prayer to priests
	if get_class_name() == "Priest(Eru)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("See the Music")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Priest(Manwe)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Manwe's Blessing")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Druid" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Charm Animal")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Dark-Priest" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Curse")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Paladin" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Divine Aim")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Stonewright" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Firebrand")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Priest(Varda)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Light of Valinor")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Priest(Ulmo)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Song of Belegaer")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end
	if get_class_name() == "Priest(Mandos)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Tears of Luthien")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	if get_class_name() == "Mimic" then
		local obj = create_object(TV_CLOAK, 100);
		obj.pval2 = resolve_mimic_name("Mouse")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the undeads, as undeads with the corruptions
	if get_subrace_name() == "Vampire" then
		player.corruption(CORRUPT_VAMPIRE_TEETH, TRUE)
		player.corruption(CORRUPT_VAMPIRE_STRENGTH, TRUE)
		player.corruption(CORRUPT_VAMPIRE_VAMPIRE, TRUE)
	end

	-- Start the Red (Fire) dragons with a book of Light (Theme)
	if get_subrace_name() == "Red" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Globe of Light")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Black (Water) dragons with a book of Geyser (Theme)
	if get_subrace_name() == "Black" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Geyser")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Green (Air) dragons with a book of Noxious Cloud (Theme)
	if get_subrace_name() == "Green" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Noxious Cloud")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Blue (Earth) dragons with a book of Stone Skin (Theme)
	if get_subrace_name() == "Blue" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Stone Skin")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the White dragons with a book of Sense Monsters (Theme)
	if get_subrace_name() == "White" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Sense Monsters")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Ethereal dragons with a book of Recharge (Theme)
	if get_subrace_name() == "Ethereal" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Recharge")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Aewroeg with a book of Charm (Theme)
	if get_subrace_name() == "(Aewrog)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Charm")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Narroeg with a book of blink (Theme)
	if get_subrace_name() == "(Narrog)" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Phase Door")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Peace-mages with a book of blink (Theme)
	if get_class_name() == "Peace-mage" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Phase Door")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Start the Wainriders with a book of Curse (Theme)
	if get_class_name() == "Wainrider" then
		local obj = create_object(TV_BOOK, 255);
		obj.pval = find_spell("Curse")
		obj.ident = bor(obj.ident, IDENT_MENTAL, IDENT_KNOWN)
		inven_carry(obj, FALSE)
		end_object(obj)
	end

	-- Provide everyone with a scroll of WoR (Theme)
	local obj = create_object(TV_SCROLL, SV_SCROLL_WORD_OF_RECALL);
		inven_carry(obj, FALSE)
		end_object(obj)
		identify_pack_fully()
end

-- Register in the hook list
add_hook_script(HOOK_BIRTH_OBJECTS, "__birth_hook_objects", "__birth_hook_objects")
