#include "tome/squelch/object_status_fwd.hpp"
#include "tome/squelch/object_status.hpp"

#include "../inventory.hpp"
#include "../object1.hpp"
#include "../object2.hpp"
#include "../object_type.hpp"
#include "../object_flag.hpp"
#include "../variable.hpp"

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
		{ status_type::NONE, "none" }
	};
	return *m;
}

status_type object_status(object_type *o_ptr)
{
	if (!object_known_p(o_ptr))
	{
		return status_type::NONE;
	}
	else
	{
		s16b slot = wield_slot_ideal(o_ptr, TRUE);

		if (artifact_p(o_ptr))
		{
			if (!(o_ptr->art_flags & TR_CURSED))
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
			if (!(o_ptr->art_flags & TR_CURSED))
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
		else
		{
			return status_type::AVERAGE;
		}
	}
}

} // namespace
