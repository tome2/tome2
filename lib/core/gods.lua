-- Gods helper files

-- Gods structs

__gods_hook = {}
__gods_callbacks = {}
__gods_callbacks_max = 0

function add_god(q)
	local i, index, d, z, qq
	
	assert(q.name, "No god name")
	assert(q.desc, "No god desc")
	assert(q.hooks, "No god hooks")
	
	i = add_new_gods(q.name);

	z = 0
	for index, d in q.desc do
		desc_god(i, z, d);
		z = z + 1
	end

	__gods_hook[i] = q.hooks
	for index, d in q.hooks do
		add_hook_script(index, "__lua__gods_callback"..__gods_callbacks_max, "__lua__gods_callback"..__gods_callbacks_max)
		setglobal("__lua__gods_callback"..__gods_callbacks_max, d)
		__gods_callbacks_max = __gods_callbacks_max + 1
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
