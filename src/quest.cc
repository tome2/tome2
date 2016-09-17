#include "quest.hpp"

#include "tables.hpp"

#include <cstddef>

void init_hooks_quests()
{
	for (auto const &q: quest)
	{
		if (q.init)
		{
			q.init();
		}
	}
}
