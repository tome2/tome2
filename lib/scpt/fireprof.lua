-- The Old Mages/Fireproofing quest: Bring back an essence from a fiery cave and get some books/scrolls/staves fireproofed in return

fireproof_quest = {}

-- The map definition itself
fireproof_quest.MAP =
[[#!map
# Created by fearoffours (fearoffours@moppy.co.uk)
# Made for ToME 2.1.x on 03/09/02

# Permanent wall
F:X:63:3

# Floor with dirt
F:.:88:3

# shallow lava
F:f:86:3

# Deep lava
F:F:85:3

### Random Monsters and/or Items
# Random object (upto 3 levels ood)
F:!:88:5:0:*21

# red mold 
F:m:88:5:324

# Chimaera
F:H:88:5:341

# Red dragon bat
F:b:88:5:377

# Hellhound and
# Random object (upto 7 levels ood) on normal floor
F:C:88:5:613:*25

# Quest exit
F:<:6:3

D:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
D:X......m.................H.........b...m......X
D:X.............b...............................X
D:X.......................m.....m.......H...b.C.X
D:X...............m.!........b.............FFFf.X
D:X.........m!............H....!........fffFFFffX
D:X..................................fffFFFFFFfFX
D:XFFf..............................fFFFFff..fffX
D:XFFFff........FFFFFF...........fffFFFfff......X
D:XfFFFFfff....FFFFFFFf.......fffFFFFFf.........X
D:X.fFFFFFFff.FFFFFFFFFfF..fffFFFFFFff..........X
D:X..fFFFFFFFffFFFfffFFFfffFFFFFFFFf............X
D:X...fFFFFFFFFFFff.ffFFFFFFFFFFFff.............X
D:X....fffFFFFFFff...ffFFFFFFFFFf...............X
D:X.......ffFFFf.......ffffFFfff................X
D:X.........fff.................................X
D:X.............................................X
D:X.............................................X
D:X.............................................X
D:X..................................<..........X
D:X.............................................X
D:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

# Starting position
P:22:26
]]


-- change this constant (and the FOO_POINTS ones) to adjust the no of items fire-proofed as a reward
fireproof_quest.TOTAL_ITEM_POINTS = 12

-- These constants are how many 'points' each type of item will take up. So currently, you can fireproof 3 books, 4 staves or 12 scrolls.
fireproof_quest.BOOK_POINTS = 4
fireproof_quest.STAFF_POINTS = 3
fireproof_quest.SCROLL_POINTS = 1

add_quest
{
	["global"] =    "FIREPROOF_QUEST",
	["name"] =      "Old Mages quest",
	["desc"] =      function()
			local num_books, num_staff, num_scroll

			num_books = fireproof_quest.item_points_remaining / fireproof_quest.BOOK_POINTS
			num_staff = fireproof_quest.item_points_remaining / fireproof_quest.STAFF_POINTS
			num_scroll = fireproof_quest.item_points_remaining / fireproof_quest.SCROLL_POINTS

			-- Quest taken
			if (quest(FIREPROOF_QUEST).status == QUEST_STATUS_TAKEN)  then
				print_hook("#####yAn Old Mages Quest!\n")
				print_hook("Retrieve the strange essence for the old mage in Lothlorien.\n")
				print_hook("\n")			
			-- essence retrieved, not taken to mage
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_COMPLETED) then
				print_hook("#####yAn Old Mages Quest!\n")
				print_hook("You have retrieved the essence for the old mage in Lothlorien. Perhaps you \n")
				print_hook("should see about a reward.\n")
				print_hook("\n")
			-- essence returned, not all books fireproofed
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_FINISHED) and (fireproof_quest.item_points_remaining > 0) then
				print_hook("#####yAn Old Mages Quest!\n")
				print_hook("You have retrieved the essence for the old mage in Lothlorien. He will still \n")
				print_hook("fireproof "..num_books.." book(s) or "..num_staff.." staff/staves or "..num_scroll.." scroll(s) for you.\n")
				print_hook("\n")
			end
	end,
	["level"] =     20,
	["data"] =      {
			-- store some variables
			["fireproof_quest.item_points_remaining"] = fireproof_quest.TOTAL_ITEM_POINTS, 
			["fireproof_quest.essence"] = 0,
	},
	["hooks"] =     {
			-- Start the game without the quest, need to request it
			[HOOK_BIRTH_OBJECTS] = function()
				quest(FIREPROOF_QUEST).status = QUEST_STATUS_UNTAKEN

				-- reset some variables on birth
				fireproof_quest.item_points_remaining = fireproof_quest.TOTAL_ITEM_POINTS
				fireproof_quest.essence = 0
			end,
			[HOOK_GEN_QUEST] = function()
				local essence, y, x, traps, tries, trap_y, trap_x, grid

				-- Only if player doing this quest
				if (player.inside_quest ~= FIREPROOF_QUEST) then
					return FALSE
				else
					-- load the map
					load_map(fireproof_quest.MAP, 2, 2)

					-- no teleport
					level_flags2 = DF2_NO_TELEPORT

					-- determine type of essence
					fireproof_quest.essence = randint(18)

					-- create essence
					essence = create_object(TV_BATERIE, fireproof_quest.essence)

					-- mark essence
					essence.pval2 = fireproof_quest.essence
					essence.note = quark_add("quest")

					-- roll for co-ordinates in top half of map
					y = randint(3) + 2
					x = randint(45) + 2

					-- drop it
					drop_near(essence, -1, y, x)

					-- how many traps to generate
					traps = rand_range(10, 30)
					
					-- generate the traps
					while (traps > 0) do

						-- initialise tries variable
						tries = 0
						
						-- make sure it's a safe place				  
						while (tries == 0) do

							-- get grid coordinates
							trap_y = randint(19) + 2
							trap_x = randint(45) + 2
							grid = cave(trap_y, trap_x)
	
							-- are the coordinates on a stair, or a wall?
							if (cave_is(grid, FF1_PERMANENT) ~= 0) or (cave_is(grid, FF1_FLOOR) == 0) then
		
								-- try again
								tries = 0
							else
								-- not a stair, then stop this 'while'
								tries = 1
							end
						end
	
						-- randomise level of trap
						trap_level = rand_range(20, 40)

						-- put the trap there
						place_trap(trap_y, trap_x, trap_level)

						-- that's one less trap to place
						traps = traps - 1
					end
					return TRUE
				end
			end,
			[HOOK_STAIR] = function()
				local ret

				-- only ask this if player about to go up stairs of quest and hasn;t retrieved essence
				if (player.inside_quest ~= FIREPROOF_QUEST) or 
				(quest(FIREPROOF_QUEST).status == QUEST_STATUS_COMPLETED) then
					return FALSE
				else
					if cave(player.py, player.px).feat ~= FEAT_LESS then return end

					-- flush all pending input
					flush()

					-- confirm
					ret = get_check("Really abandon the quest?")

					-- if yes, then
					if  (ret == TRUE) then

						-- fail the quest
						quest(FIREPROOF_QUEST).status = QUEST_STATUS_FAILED
						return FALSE
					else 
						-- if no, they stay in the quest
						return TRUE
					end
				end
			end,
			[HOOK_GET] = function(o_ptr)

				-- if they're in the quest and haven't picked up the essence already, continue
				if (player.inside_quest ~= FIREPROOF_QUEST) or 
				(quest(FIREPROOF_QUEST).status == QUEST_STATUS_COMPLETED) then
					return FALSE
				else

					-- check that it's the real essence and not another one generated via the random object placing in fireproof.map
					if (o_ptr.pval2 == fireproof_quest.essence) then

						-- ok mark the quest 'completed'
						quest(FIREPROOF_QUEST).status = QUEST_STATUS_COMPLETED
						msg_print(TERM_YELLOW, "Fine! Looks like you've found it.")
					end
				end
			end,
				
	},
}

-- add the bit that determines what happens when the request 'q'uest bit is done in the wizard spire
add_building_action
{
	-- Index is used in ba_info.txt to set the actions
	["index"] =     56,
	["action"] =    function()

			local num_books, num_staff, num_scroll

			num_books = fireproof_quest.item_points_remaining / fireproof_quest.BOOK_POINTS
			num_staff = fireproof_quest.item_points_remaining / fireproof_quest.STAFF_POINTS
			num_scroll = fireproof_quest.item_points_remaining / fireproof_quest.SCROLL_POINTS

			-- the quest hasn;t been requested already, right?
			if quest(FIREPROOF_QUEST).status == QUEST_STATUS_UNTAKEN then

				-- quest has been taken now
				quest(FIREPROOF_QUEST).status = QUEST_STATUS_TAKEN
				fireproof_quest.item_points_remaining = fireproof_quest.TOTAL_ITEM_POINTS

				-- issue instructions
				msg_print("I need a very special essence for a spell I am working on. I am too old to ")
				msg_print("fetch it myself. Please bring it back to me. You can find it north of here.")
				msg_print("Be careful with it, it's fragile and might be destroyed easily.")

				return TRUE, FALSE, TRUE
			-- if quest completed (essence was retrieved)
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_COMPLETED) then

				-- ask for essence
				ret, item = get_item("Which essence?",
						     "You have no essences to return",
						     bor(USE_INVEN),
						     function (obj)

							-- check it's the 'marked' essence
							if (obj.tval == TV_BATERIE) and (obj.sval == fireproof_quest.essence) and (obj.pval2 == fireproof_quest.essence) then
								return TRUE
							end
							return FALSE
						     end
				)

				-- didn't get the essence?
				if (ret == FALSE) then 
					return TRUE

				-- got the essence!
				else

					-- take essence
					inven_item_increase(item, -1)
					inven_item_optimize(item)
					msg_print("Great! Let me fireproof some of your items in thanks. I can do "..num_books.." books, ")
					msg_print(num_staff.." staves, or "..num_scroll.." scrolls.")

					-- how many items to proof?
					local items = fireproof_quest.item_points_remaining

					-- repeat till up to 3 (value defined as TOTAL_ITEM_POINTS constant) books fireproofed
					while items > 0 do
						ret = fireproof()

						-- don't loop the fireproof if there's nothing to fireproof
						if ret == FALSE then 
							break 
						end

						-- subtract item points
						items = fireproof_quest.item_points_remaining
					end

					-- have they all been done?
					if (fireproof_quest.item_points_remaining == 0) then 
						-- mark quest to make sure no more quests are given
						quest(FIREPROOF_QUEST).status = QUEST_STATUS_REWARDED 
					else
						-- mark in preparation of anymore books to fireproof
						quest(FIREPROOF_QUEST).status = QUEST_STATUS_FINISHED
					end


				end	     

			-- if the player asks for a quest when they already have it, but haven't failed it, give them some extra instructions
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_TAKEN) then
				msg_print("The essence is in a cave just behind the shop.")

			-- ok not all books have been fireproofed... lets do the rest
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_FINISHED) then

				-- how many books still to proof?
				local items = fireproof_quest.item_points_remaining

				-- repeat as necessary
				while items > 0 do
					ret = fireproof()

					-- don't loop the fireproof if there's nothing to fireproof
					if ret == FALSE then 
						break 
					else 
						-- have they all been done?
						if (fireproof_quest.item_points_remaining == 0) then quest(FIREPROOF_QUEST).status = QUEST_STATUS_REWARDED end
					end

					-- subtract item points
					items = fireproof_quest.item_points_remaining
				end

			-- quest failed or completed, then give no more quests
			elseif (quest(FIREPROOF_QUEST).status == QUEST_STATUS_FAILED) or (quest(FIREPROOF_QUEST).status == QUEST_STATUS_REWARDED) then
				msg_print("I have no more quests for you")
			end
			return TRUE
	end,
}

-- the routine that checks for a book and actually fireproofs it
function fireproof()

	local ret, item, obj2, stack, obj3, carry_it

	ret, item = get_item("Which item shall I fireproof?",
			"You have no more items I can fireproof, come back when you have some.",
			bor(USE_INVEN),
			function (obj)

				-- get some flags
				local f1, f2, f3, f4, f5, esp = object_flags(obj)

				-- is it a book/staff/scroll, is it already fireproof?
				if ((obj.tval == TV_BOOK) or (obj.tval == TV_SCROLL) or (obj.tval == TV_STAFF)) and (band(f3, TR3_IGNORE_FIRE) == 0) then
					return TRUE
				end
				return FALSE
			end
		)

	-- get the object type from the number
	obj2 = get_object(item)

	-- check we have enough points (if we 'got' an item)
	if (ret == TRUE) then 
		ret2, stack = enough_points(obj2)
	end

	-- did either routine fail?
	if (ret == FALSE) or (ret2 == FALSE)  then 
		return FALSE
	else

		-- are we part of the items from a stack?
		if (obj2.number ~= stack) then 

			-- make a new object to handle
			object_copy(obj_forge, obj2)

			-- give it the right number of items
			obj_forge.number = stack

			-- adjust for number of items in pack not to be fireproofed 
			obj2.number = obj2.number - stack
			obj3 = obj_forge

			-- we'll need to add this to the inventory after fireproofing
			carry_it = TRUE
		else

			-- use the whole stack
			obj3 = obj2

			-- we'll be dealing this while it's still in the inventory
			carry_it = FALSE
		end

		-- make it fireproof
		obj3.name2 = 149

		-- apply it, making sure the pvals don't change with apply_magic (it would change the type of book!)
		local oldpval = obj3.pval
		local oldpval2 = obj3.pval2
		local oldpval3 = obj3.pval3
		apply_magic(obj3, -1, FALSE, FALSE, FALSE)
		obj3.pval = oldpval
		obj3.pval2 = oldpval2
		obj3.pval3 = oldpval3

		-- put it in the inventory if it's only part of a stack
		if (carry_it == TRUE) then
			inven_carry(obj3, TRUE)
		end

		-- id and notice it
		set_known(obj3)
		set_aware(obj3)

		return TRUE
	end
end

-- This function makes sure the player has enough 'points' left to fireproof stuff.
function enough_points(obj)
	local item_value, stack

	-- are the items in a stack?
	if (obj.number > 1) then 

		-- how many to fireproof?
		stack = get_quantity("How many would you like fireproofed?", obj.number)
	else 
		stack = 1
	end
	
	-- check for item type and multiply number in the stack by the amount of points per item of that type
	if (obj.tval == TV_BOOK) then
		item_value = fireproof_quest.BOOK_POINTS * stack
	elseif (obj.tval == TV_STAFF) then
		item_value = fireproof_quest.STAFF_POINTS * stack
	elseif (obj.tval == TV_SCROLL) then
		item_value = fireproof_quest.SCROLL_POINTS * stack
	end

	-- do we have enough points?
	if (item_value > fireproof_quest.item_points_remaining) then
		msg_print("I do not have enough fireproofing material for that.")
		return FALSE
	else 
		-- if so then subtract those points before we do the fireproofing
		fireproof_quest.item_points_remaining = fireproof_quest.item_points_remaining - item_value
	end

	-- Used all the points? the quest is completely rewarded.
	if fireproof_quest.item_points_remaining == 0 then quest(FIREPROOF_QUEST).status = QUEST_STATUS_REWARDED end

	return TRUE, stack
end

