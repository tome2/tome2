#include "tome/squelch/condition_metadata.hpp"
#include "tome/squelch/condition.hpp"

#include <vector>

#include "tome/squelch/object_status.hpp"
#include "lua_bind.hpp"
#include "skills.hpp"
#include "util.hpp"
#include "util.h"
#include "z-term.h"

namespace squelch {

static std::shared_ptr<Condition> create_condition_name()
{
	auto s = input_box_auto("Object name to match?", 79);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<NameCondition>(s);
}

static std::shared_ptr<Condition> create_condition_contain()
{
	auto s = input_box_auto("Word to find in object name?", 79);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<ContainCondition>(s);
}

static std::shared_ptr<Condition> create_condition_inscribed()
{
	auto s = input_box_auto("Word to find in object inscription?", 79);
	if (s.empty() == 0)
	{
		return nullptr;
	}

	return std::make_shared<InscriptionCondition>(s);
}

static std::shared_ptr<Condition> create_condition_discount()
{
	int min, max;

	{
		auto s = input_box_auto("Min discount?", 79);
		if (sscanf(s.c_str(), "%d", &min) < 1)
		{
			return nullptr;
		}
	}

	{
		auto s = input_box_auto("Max discount?", 79);
		if (sscanf(s.c_str(), "%d", &max) < 1)
		{
			return nullptr;
		}
	}

	return std::make_shared<DiscountCondition>(min, max);
}

static std::shared_ptr<Condition> create_condition_symbol()
{
	auto s = input_box_auto("Symbol to match?", 1);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<SymbolCondition>(s[0]);
}

static std::shared_ptr<Condition> create_condition_status()
{
	status_type status;

	auto c = msg_box_auto("[t]errible, [v]ery bad, [b]ad, "
			"[a]verage, [G]ood, [V]ery good, [S]pecial?");

	switch (c)
	{
	case 't': status = status_type::TERRIBLE; break;
	case 'v': status = status_type::VERY_BAD; break;
	case 'b': status = status_type::BAD; break;
	case 'a': status = status_type::AVERAGE; break;
	case 'G': status = status_type::GOOD; break;
	case 'V': status = status_type::VERY_GOOD; break;
	case 'S': status = status_type::SPECIAL; break;
	default: return nullptr;
	}

	return std::make_shared<StatusCondition>(status);
}

static std::shared_ptr<Condition> create_condition_state()
{
	char c = msg_box_auto("[i]dentified, [n]on identified?");

	identification_state s;
	switch (c)
	{
	case 'i': s = identification_state::IDENTIFIED; break;
	case 'n': s = identification_state::NOT_IDENTIFIED; break;
	default: return nullptr;
	}

	return std::make_shared<StateCondition>(s);
}

static bool in_byte_range(int x)
{
	return (x >= 0) && (x < 256);
}

static std::shared_ptr<Condition> create_condition_tval()
{
	auto s = input_box_auto("Tval to match?", 79);
	int tval;
	if (sscanf(s.c_str(), "%d", &tval) < 1)
	{
		return nullptr;
	}

	if (!in_byte_range(tval))
	{
		return nullptr;
	}

	return std::make_shared<TvalCondition>(tval);
}

static std::shared_ptr<Condition> create_condition_sval()
{
	int sval_min, sval_max;

	{
		auto s = input_box_auto("Min sval?", 79);
		if ((sscanf(s.c_str(), "%d", &sval_min) < 1) ||
		    (!in_byte_range(sval_min)))
		{
			return nullptr;
		}
	}

	{
		auto s = input_box_auto("Max sval?", 79);
		if ((sscanf(s.c_str(), "%d", &sval_max) < 1) ||
		    (!in_byte_range(sval_max)))
		{
			return nullptr;
		}
	}

	return std::make_shared<SvalCondition>(sval_min, sval_max);
}

static std::shared_ptr<Condition> create_condition_race()
{
	auto s = input_box_auto("Player race to match?", 79);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<RaceCondition>(s);
}

static std::shared_ptr<Condition> create_condition_subrace()
{
	auto s = input_box_auto("Player subrace to match?", 79);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<SubraceCondition>(s);
}

static std::shared_ptr<Condition> create_condition_class()
{
	auto s = input_box_auto("Player class to match?", 79);
	if (s.empty())
	{
		return nullptr;
	}

	return std::make_shared<ClassCondition>(s);
}

static std::shared_ptr<Condition> create_condition_level()
{
	int min, max;

	{
		auto s = input_box_auto("Min player level?", 79);
		if (sscanf(s.c_str(), "%d", &min) < 1)
		{
			return nullptr;
		}
	}

	{
		auto s = input_box_auto("Max player level?", 79);
		if (sscanf(s.c_str(), "%d", &max) < 1)
		{
			return nullptr;
		}
	}

	return std::make_shared<LevelCondition>(min, max);
}

static std::shared_ptr<Condition> create_condition_skill()
{
	int min, max;

	{
		auto s = input_box_auto("Min skill level?", 79);
		if (sscanf(s.c_str(), "%d", &min) < 1)
		{
			return nullptr;
		}
	}

	{
		auto s = input_box_auto("Max skill level?", 79);
		if (sscanf(s.c_str(), "%d", &max) < 1)
		{
			return nullptr;
		}
	}

	s16b skill_idx;
	{
		auto s = input_box_auto("Skill name?", 79);
		if (s.empty() == 0)
		{
			return nullptr;
		}

		skill_idx = find_skill_i(s.c_str());
		if (skill_idx < 0)
		{
			return nullptr;
		}
	}

	return std::make_shared<SkillCondition>(skill_idx, min, max);
}

static std::shared_ptr<Condition> create_condition_ability()
{
	auto s = input_box_auto("Ability name?", 79);
	if (s.empty() == 0)
	{
		return nullptr;
	}

	s16b ai = find_ability(s.c_str());
	if (ai < 0)
	{
		return nullptr;
	}

	return std::make_shared<AbilityCondition>(ai);
}

static void display_desc(match_type match_type_)
{
	int i = 0;
	auto line = [&i] (const char *s) {
		c_prt(TERM_WHITE, s, i + 1, 17);
		i++;
	};

	switch (match_type_)
	{
	case match_type::AND:
		line("Check is true if all rules within it are true");
		break;

	case match_type::OR:
		line("Check is true if at least one rule within it is true");
		break;

	case match_type::NOT:
		line("Invert the result of its child rule");
		break;

	case match_type::NAME:
		line("Check is true if object name matches name");
		break;

	case match_type::CONTAIN:
		line("Check is true if object name contains word");
		break;

	case match_type::INSCRIBED:
		line("Check is true if object inscription contains word");
		break;

	case match_type::DISCOUNT:
		line("Check is true if object discount is between two values");
		break;

	case match_type::SYMBOL:
		line("Check is true if object symbol is ok");
		break;

	case match_type::STATE:
		line("Check is true if object is identified/unidentified");
		break;

	case match_type::STATUS:
		line("Check is true if object status is ok");
		break;

	case match_type::TVAL:
		line("Check is true if object tval(from k_info.txt) is ok");

	case match_type::SVAL:
		line("Check is true if object sval(from k_info.txt) is between");
		line("two values");
		break;

	case match_type::RACE:
		line("Check is true if player race is ok");
		break;

	case match_type::SUBRACE:
		line("Check is true if player subrace is ok");
		break;

	case match_type::CLASS:
		line("Check is true if player class is ok");
		break;

	case match_type::LEVEL:
		line("Check is true if player level is between 2 values");
		break;

	case match_type::SKILL:
		line("Check is true if player skill level is between 2 values");
		break;

	case match_type::ABILITY:
		line("Check is true if player has the ability");
		break;

	case match_type::INVENTORY:
		line("Check is true if something in player's inventory matches");
		line("the contained rule");
		break;

	case match_type::EQUIPMENT:
		line("Check is true if something in player's equipment matches");
		line("the contained rule");
		break;
	}
}

std::shared_ptr<Condition> new_condition_interactive()
{
	static std::vector<match_type> condition_types = {
		match_type::AND,
		match_type::OR,
		match_type::NOT,
		match_type::NAME,
		match_type::CONTAIN,
		match_type::INSCRIBED,
		match_type::DISCOUNT,
		match_type::SYMBOL,
		match_type::STATE,
		match_type::STATUS,
		match_type::TVAL,
		match_type::SVAL,
		match_type::RACE,
		match_type::SUBRACE,
		match_type::CLASS,
		match_type::LEVEL,
		match_type::SKILL,
		match_type::ABILITY,
		match_type::INVENTORY,
		match_type::EQUIPMENT
	};
	static std::vector<const char *> condition_type_names;

	// Fill in types names?
	if (condition_type_names.empty())
	{
		for (auto condition_type : condition_types)
		{
			condition_type_names.push_back(
				match_mapping().stringify(condition_type));
		}
	}

	// Choose
	int begin = 0, sel = 0;
	while (1)
	{
		int wid, hgt;
		Term_clear();
		Term_get_size(&wid, &hgt);

		display_list(0, 0, hgt - 1, 15, "Rule types", condition_type_names.data(), condition_types.size(), begin, sel, TERM_L_GREEN);

		display_desc(condition_types[sel]);

		char c = inkey();

		if (c == ESCAPE) break;
		else if (c == '8')
		{
			sel--;
			if (sel < 0)
			{
				sel = condition_types.size() - 1;
				begin = condition_types.size() - hgt;
				if (begin < 0) begin = 0;
			}
			if (sel < begin) begin = sel;
		}
		else if (c == '2')
		{
			sel++;
			if (sel >= static_cast<int>(condition_types.size()))
			{
				sel = 0;
				begin = 0;
			}
			if (sel >= begin + hgt - 1) begin++;
		}
		else if (c == '\r')
		{
			switch (condition_types[sel])
			{
			case match_type::AND:
				return std::make_shared<AndCondition>();
			case match_type::OR:
				return std::make_shared<OrCondition>();
			case match_type::NOT:
				return std::make_shared<NotCondition>();
			case match_type::NAME:
				return create_condition_name();
			case match_type::CONTAIN:
				return create_condition_contain();
			case match_type::INSCRIBED:
				return create_condition_inscribed();
			case match_type::DISCOUNT:
				return create_condition_discount();
			case match_type::SYMBOL:
				return create_condition_symbol();
			case match_type::STATE:
				return create_condition_state();
			case match_type::STATUS:
				return create_condition_status();
			case match_type::TVAL:
				return create_condition_tval();
			case match_type::SVAL:
				return create_condition_sval();
			case match_type::RACE:
				return create_condition_race();
			case match_type::SUBRACE:
				return create_condition_subrace();
			case match_type::CLASS:
				return create_condition_class();
			case match_type::LEVEL:
				return create_condition_level();
			case match_type::SKILL:
				return create_condition_skill();
			case match_type::ABILITY:
				return create_condition_ability();
			case match_type::INVENTORY:
				return std::make_shared<InventoryCondition>();
			case match_type::EQUIPMENT:
				return std::make_shared<EquipmentCondition>();

			}
		}
	}
	return nullptr;
}

} // namespace
