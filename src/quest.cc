#include "quest.hpp"

#include "angband.h"
#include "tables.hpp"

#include <cstddef>

void init_hooks_quests()
{
	for (std::size_t i = 0; i < MAX_Q_IDX; i++)
	{
		if (quest[i].init != NULL)
		{
			quest[i].init(i);
		}
	}
}

