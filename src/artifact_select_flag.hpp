#pragma once

#include "h-basic.h"

struct artifact_select_flag {
	byte group;		/* Flag group to display it in */
	int flag;		/* item flag to set */
	byte level;		/* Player skill level to start at */
	char *desc;		/* Display this description to select flag */
	u32b xp;		/* xp cost for this flag */
	bool_ pval;		/* indicates this flag benifits from pval */
	char *item_desc;	/* Description of required item */
	char *item_descp;	/* Description of required item; plural */
	byte rtval;		/* Required items' tval */
	byte rsval;		/* Required items' sval */
	int  rpval;		/* Required items' pval (zero for no req) */
	int  rflag[6];	/* Monster Race flags for required Corpses */
};
