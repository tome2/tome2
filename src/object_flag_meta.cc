#include "object_flag_meta.hpp"

#include "object_flag.hpp"

std::vector<object_flag_meta const *> const &object_flags_meta()
{
	static std::vector<object_flag_meta const *> instance;

	if (instance.empty())
	{
#define NUMERIC 'n'
#define BINARY 'b'
#define TERNARY(n) ((n == 1) ? '+' : ((n == 2) ? '*' : '?'))
#define FIXED(n) ((n == 1) ? '1' : ((n == 2) ? '2' : ((n == 3) ? '3' : '?')))
#define TR(tier, index, name, e_name, c_name, c_page, c_col, c_row, c_type, c_prio, is_pval, is_esp) \
	        instance.emplace_back(new object_flag_meta { \
	                name, \
                        #name, \
                        #e_name, \
	                c_name, \
	                c_page, \
	                c_col, \
	                c_row, \
	                c_type, \
			c_prio, \
	                is_pval, \
	                is_esp \
	        });
#include "object_flag_list.hpp"
#undef TR
#undef FIXED
#undef TERNARY
#undef BINARY
#undef NUMERIC
	};

	return instance;
}

object_flag_set const &object_flags_esp()
{
	static object_flag_set instance;
	static bool initialized = false;

	if (!initialized)
	{
		for (auto const object_flag_meta: object_flags_meta())
		{
			if (object_flag_meta->is_esp)
			{
				instance |= object_flag_meta->flag_set;
			}
		}

		initialized = true;
	}

	return instance;
}
