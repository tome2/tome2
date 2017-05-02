/*
 * Copyright (c) 2001 James E. Wilson, Robert A. Koeneke, DarkGod
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */
#include "hooks.hpp"

#include <algorithm>
#include <assert.h>
#include <unordered_map>
#include <vector>

/******** Hooks stuff *********/

struct hook_data
{
private:
	hook_func_t m_hook_func;
	void *m_hook_data;
public:
	hook_data(hook_func_t hook_func, void *hook_data)
		: m_hook_func(hook_func)
		, m_hook_data(hook_data) {
	}

	hook_data() = delete;

	/**
	 * Check if the given hook points to the given function.
	 */
	bool is(hook_func_t hook_func) const {
		return m_hook_func == hook_func;
	}

	/**
	 * Invoke the hook with the given input and output pointers.
	 */
	bool_ invoke(void *in, void *out) const {
		return m_hook_func(m_hook_data, in, out);
	}
};

std::unordered_map<size_t, std::vector<hook_data>> &hooks_instance()
{
	static auto instance = new std::unordered_map<size_t, std::vector<hook_data>>();
	return *instance;
}


int process_hooks_restart = FALSE;

static std::vector<hook_data>::iterator find_hook(std::vector<hook_data> &hooks, hook_func_t hook_func)
{
	return std::find_if(hooks.begin(),
			    hooks.end(),
			    [&](const hook_data &hook_data) {
				    return hook_data.is(hook_func);
			    });
}

void add_hook_new(int h_idx, hook_func_t hook_func, cptr name, void *data)
{
	auto &hooks = hooks_instance()[h_idx];
	// Only insert if not already present.
	if (find_hook(hooks, hook_func) == hooks.end()) {
		hooks.emplace_back(hook_func, data);
	}
}

void del_hook_new(int h_idx, hook_func_t hook_func)
{
	auto &hooks = hooks_instance()[h_idx];

	/* Find it */
	auto found_it = find_hook(hooks, hook_func);
	if (found_it != hooks.end())
	{
		hooks.erase(found_it);
	}
}

bool process_hooks_new(int h_idx, void *in, void *out)
{
	auto const &hooks = hooks_instance()[h_idx];

	auto hooks_it = hooks.begin();
	while (hooks_it != hooks.end())
	{
		auto &hook_data = *hooks_it;

		/* Invoke hook function; stop processing if the hook
		   returns TRUE */
		if (hook_data.invoke(in, out))
		{
			return true;
		}

		/* Should we restart processing at the beginning? */
		if (process_hooks_restart)
		{
			hooks_it = hooks.begin();
			process_hooks_restart = FALSE;
		}
		else
		{
			hooks_it++;
		}
	}

	return false;
}
