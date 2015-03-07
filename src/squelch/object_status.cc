#include "tome/squelch/object_status_fwd.hpp"
#include "tome/squelch/object_status.hpp"

#include "angband.h"
#include "object1.hpp"

namespace squelch {

EnumStringMap<status_type> &status_mapping()
{
	// TODO: This is quite ugly and leads to valgrind complaints
	static auto m = new EnumStringMap<status_type> {
		{ status_type::BAD, "bad" },
		{ status_type::VERY_BAD, "very bad" },
		{ status_type::AVERAGE, "average" },
		{ status_type::GOOD, "good" },
		{ status_type::VERY_GOOD, "very good" },
		{ status_type::SPECIAL, "special" },
		{ status_type::TERRIBLE, "terrible" },
		{ status_type::NONE, "none" },
		{ status_type::CHEST_EMPTY, "(empty chest)" },
		{ status_type::CHEST_DISARMED, "(disarmed chest)" } };
	return *m;
}

status_type object_status(object_type *o_ptr)
{
	if (!object_known_p(o_ptr))
	{
		switch (o_ptr->sense)
		{
		case SENSE_CURSED: return status_type::BAD;
		case SENSE_WORTHLESS: return status_type::VERY_BAD;
		case SENSE_AVERAGE: return status_type::AVERAGE;
		case SENSE_GOOD_LIGHT: return status_type::GOOD;
		case SENSE_GOOD_HEAVY: return status_type::GOOD;
		case SENSE_EXCELLENT: return status_type::VERY_GOOD;
		case SENSE_SPECIAL: return status_type::SPECIAL;
		case SENSE_TERRIBLE: return status_type::TERRIBLE;
		default: return status_type::NONE;
		}
	}
	else
	{
		s16b slot = wield_slot_ideal(o_ptr, TRUE);

		if (artifact_p(o_ptr))
		{
			if (!(o_ptr->ident & IDENT_CURSED))
			{
				return status_type::SPECIAL;
			}
			else
			{
				return status_type::TERRIBLE;
			}
		}
		else if ((o_ptr->name2 > 0) ||
			 (o_ptr->name2b > 0))
		{
			if (!(o_ptr->ident & IDENT_CURSED))
			{
				return status_type::VERY_GOOD;
			}
			else
			{
				return status_type::VERY_BAD;
			}
		}
		else if ((slot == INVEN_WIELD) ||
			 (slot == INVEN_BOW) ||
			 (slot == INVEN_AMMO) ||
			 (slot == INVEN_TOOL))
		{
			if (o_ptr->to_h + o_ptr->to_d < 0)
			{
				return status_type::BAD;
			}
			else if (o_ptr->to_h + o_ptr->to_d > 0)
			{
				return status_type::GOOD;
			}
			else
			{
				return status_type::AVERAGE;
			}
		}
		else if ((slot >= INVEN_BODY) &&
			 (slot <= INVEN_FEET))
		{
			if (o_ptr->to_a < 0)
			{
				return status_type::BAD;
			}
			else if (o_ptr->to_a > 0)
			{
				return status_type::GOOD;
			}
			else
			{
				return status_type::AVERAGE;
			}
		}
		else if (slot == INVEN_RING)
		{
			if ((o_ptr->to_d + o_ptr->to_h < 0) ||
			    (o_ptr->to_a < 0) ||
			    (o_ptr->pval < 0))
			{
				return status_type::BAD;
			}
			else
			{
				return status_type::AVERAGE;
			}
		}
		else if (slot == INVEN_NECK)
		{
			if (o_ptr->pval < 0)
			{
				return status_type::BAD;
			}
			else
			{
				return status_type::AVERAGE;
			}
		}
		else if (o_ptr->tval == TV_CHEST)
		{
			if (o_ptr->pval == 0)
			{
				return status_type::CHEST_EMPTY;
			}
			else if (o_ptr->pval < 0)
			{
				return status_type::CHEST_DISARMED;
			}
			else
			{
				return status_type::AVERAGE;
			}
		}
		else
		{
			return status_type::AVERAGE;
		}
	}
}

} // namespace
