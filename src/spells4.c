#include "angband.h"

#include <assert.h>

static bool_ uses_piety_to_cast(int s)
{
	char buf[128];
	sprintf(buf, "return check_affect(%d, \"piety\", FALSE)", s);
	return exec_lua(buf);
}

static bool_ castable_while_blind(int s)
{
	char buf[128];
	sprintf(buf, "return not check_affect(%d, \"blind\")", s);
	return exec_lua(buf);
}

static bool_ castable_while_confused(int s)
{
	char buf[128];
	sprintf(buf, "return not check_affect(%d, \"confusion\")", s);
	return exec_lua(buf);
}

/* Output the describtion when it is used as a spell */
void print_spell_desc(int s, int y)
{
	int i;

	for (i=0; ; i++)
	{
		char buf[128];
		cptr desc = NULL;

		sprintf(buf, "return __spell_desc[%d][%d]", s, i+1);
		desc = string_exec_lua(buf);
		if (!desc)
		{
			break;
		}

		c_prt(TERM_L_BLUE, desc, y, 0);
		y++;
	}
	
	if (uses_piety_to_cast(s))
	{
		c_prt(TERM_L_WHITE, "It uses piety to cast.", y, 0);
		y++;
	}
	if (castable_while_blind(s))
	{
		c_prt(TERM_ORANGE, "It is castable even while blinded.", y, 0);
		y++;
	}
	if (castable_while_confused(s))
	{
		c_prt(TERM_ORANGE, "It is castable even while confused.", y, 0);
		y++;
	}
}
