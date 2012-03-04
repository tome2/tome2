__building_actions = {}

function add_building_action(a)
	assert(a.index, "No building action index")
	assert(a.action, "No building action action")
	__building_actions[a.index] = a.action
end

function __bact_activate(bact)
	if __building_actions[bact] then
		return __building_actions[bact]()
	end
end

add_hook_script(HOOK_BUILDING_ACTION, "__bact_activate", "__bact_activate")
