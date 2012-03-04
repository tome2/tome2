-- Savefile stuff
-- Do not meddle in the affairs of savefiles for they are subtle and quick to be become incompatible

__loadsave_name = {}
__loadsave_max = 0
__loadsave_tmp = 0

function add_loadsave(name, default)
	assert(name, "No variable name to save")
	assert(default, "No default value")

	-- if it is a table we must create many entries
	if type(default) == "table" then
		for k, e in default do
			add_loadsave(name.."."..k, e)
		end
	else
		__loadsave_name[__loadsave_max] = { name = name, default = default }
		__loadsave_max = __loadsave_max + 1
	end
end

-- Example of how to save a table
-- NOTE: { 1, 2, 3 } will NOT work, the key MUST be a string
--[[
add_loadsave("t",
{
	foo = 7,
	tab = {
		a = 1,
		b = 2,
		tab = {
			a=1, b=2, c=3,
		},
	},
})
]]
