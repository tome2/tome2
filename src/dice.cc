#include "dice.hpp"

#include "z-rand.hpp"

#include <cassert>
#include <cstring>

void dice_init(dice_type *dice, long base, long num, long sides)
{
	assert(dice != NULL);

	dice->base = base;
	dice->num = num;
	dice->sides = sides;
}

static bool dice_parse(dice_type *dice, const char *s)
{
	long base, num, sides;

	if (sscanf(s, "%ld+%ldd%ld", &base, &num, &sides) == 3)
	{
		dice_init(dice, base, num, sides);
		return true;
	}

	if (sscanf(s, "%ld+d%ld", &base, &sides) == 2)
	{
		dice_init(dice, base, 1, sides);
		return true;
	}

	if (sscanf(s, "d%ld", &sides) == 1)
	{
		dice_init(dice, 0, 1, sides);
		return true;
	}

	if (sscanf(s, "%ldd%ld", &num, &sides) == 2)
	{
		dice_init(dice, 0, num, sides);
		return true;
	}

	if (sscanf(s, "%ld", &base) == 1)
	{
		dice_init(dice, base, 0, 0);
		return true;
	}

	return false;
}

void dice_parse_checked(dice_type *dice, const char *s)
{
	if (!dice_parse(dice, s))
	{
		abort();
	}
}

long dice_roll(dice_type *dice)
{
	assert(dice != NULL);
	return dice->base + damroll(dice->num, dice->sides);
}

void dice_print(dice_type *dice, char *output)
{
	char buf[16];

	output[0] = '\0';

	if (dice->base > 0)
	{
		sprintf(buf, "%ld", dice->base);
		strcat(output, buf);
	}

	if ((dice->num > 0) || (dice->sides > 0))
	{
		if (dice->base > 0)
		{
			strcat(output, "+");
		}

		if (dice->num > 1)
		{
			sprintf(buf, "%ld", dice->num);
			strcat(output, buf);
		}

		strcat(output, "d");

		sprintf(buf, "%ld", dice->sides);
		strcat(output, buf);
	}
}
