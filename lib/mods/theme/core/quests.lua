-- Quest helper files

-- Quest structs

__quest_hook = {}
__quest_callbacks = {}
__quest_callbacks_max = 0
__quest_dynamic_desc = {}

function add_quest(q)
	local i, index, d, z, qq
	
	assert(q.global, "No quest global name")
	assert(q.name, "No quest name")
	assert(q.desc, "No quest desc")
	assert(q.level, "No quest level")
	assert(q.hooks, "No quest hooks")
	
	i = new_quest(q.name);
	setglobal(q.global, i)

	-- Make it save & load
	add_loadsave("quest("..q.global..").status", QUEST_STATUS_UNTAKEN)

	if type(q.desc) == "table" then
		z = 0
		for index, d in q.desc do
			quest_desc(i, z, d);
			z = z + 1
		end
	else
		__quest_dynamic_desc[i] = q.desc
		quest(i).dynamic_desc = TRUE
	end
	quest(i).level = q.level
	if not q.silent then
		quest(i).silent = FALSE
	else
		quest(i).silent = q.silent
	end
	__quest_hook[i] = q.hooks
	for index, d in q.hooks do
		add_hook_script(index, "__lua__quest_callback"..__quest_callbacks_max, "__lua__quest_callback"..__quest_callbacks_max)
		setglobal("__lua__quest_callback"..__quest_callbacks_max, d)
		__quest_callbacks_max = __quest_callbacks_max + 1
	end
	if q.data then
		for index, d in q.data do
			-- Besure it exists
			setglobal(index, d)

			-- Make it save & load
	     		add_loadsave(index, d)
		end
	end
	return i
end
