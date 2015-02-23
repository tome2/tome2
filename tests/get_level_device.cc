#include "angband.h"
#include "spell_type.h"
#include <bandit/bandit.h>
using namespace bandit;

//
// Declarations for testing purposes:
//

s32b get_level_device(spell_type *spell, s32b max, s32b min, s32b device_skill, std::function<s32b(spell_type *, s32b, s32b, s32b, s32b)> lua_get_level = lua_get_level);

//
// Tests
//

go_bandit([]() {

	describe("get_level_device", []() {

		s32b passed_in_max;
		s32b passed_in_min;

		// Fake get_level function we can use to detect what's being passed to the real one.
		auto fake_get_level = [&](struct spell_type *spell, s32b lvl, s32b max, s32b min, s32b bonus) -> s32b {
			// Store the passed input values for verification purposes
			passed_in_max = max;
			passed_in_min = min;
			// Return the input "lvl" unmodified.
			return lvl;
		};

		before_each([&]() {
			// Reset saved state
			passed_in_max = -1;
			passed_in_min = -1;
		});

		// Magic-Device skill levels that we've tested at.
		const std::vector<s32b> device_skill_values {
			15300, // @15.3
			35300, // @35.3
			45300, // @45.3
			50000  // @50.0
		};

		// "Remove Curses" spell and expected result levels.
		auto remove_curses_spell = spell_type_new("TEST: Remove Curses");
		spell_type_set_difficulty(remove_curses_spell, 10, 42 /* notused */);

		const std::map<s32b, s32b> remove_curses_expected_levels {
			{ 15300, 7 },
			{ 35300, 15 },
			{ 45300, 15 },
			{ 50000, 15 }
		};

		// "Wish" spell and expected result levels.
		auto wish_spell = spell_type_new("TEST: Wish");
		spell_type_set_difficulty(wish_spell, 50, 42 /* notused */);

		const std::map<s32b, s32b> wish_expected_levels {
			{ 15300, 1 },
			{ 35300, 1 },
			{ 45300, 1 },
			{ 50000, 2 }
		};

		// "Heal Monster" spell and expected result levels.
		auto heal_monster_spell = spell_type_new("TEST: Heal Monster");
		spell_type_set_difficulty(heal_monster_spell, 3, 42 /* notused */);

		const std::map<s32b, s32b> heal_monster_expected_levels {
			{ 15300, 108 },
			{ 35300, 152 },
			{ 45300, 152 },
			{ 50000, 152 }
		};

		// "Teleport Away" spell and expected result levels.
		auto teleport_away_spell = spell_type_new("TEST: Teleport Away");
		spell_type_set_difficulty(teleport_away_spell, 23, 42 /* notused */);

		const std::map<s32b, s32b> teleport_away_expected_levels {
			{ 15300, 1 },
			{ 35300, 16 },
			{ 45300, 20 },
			{ 50000, 20 }
		};

		//
		// Basic tests for "min <= 0" and "max <= 0".
		//
		
		it("should clamp 'min' parameter to 1", [&]() {
			// Setup
			s32b device_skill = 100; /* doesn't matter for this test */
			get_level_max_stick = 1; /* doesn't matter for this test */
			get_level_use_stick = 1; /* doesn't matter for this test */
			auto spell = remove_curses_spell; /* doesn't matter for this test */
			s32b max = 100; /* doesn't matter for this test */
			s32b min = 0;
			// Exercise
			get_level_device(spell, max, min, device_skill, fake_get_level);
			// Verify
			AssertThat(passed_in_min, Equals(1));
		});

		it("should use 50 as default for 'max' parameter if zero or less", [&]() {
			// Setup
			s32b device_skill = 100; /* doesn't matter for this test */
			get_level_max_stick = 1; /* doesn't matter for this test */
			get_level_use_stick = 1; /* doesn't matter for this test */
			auto spell = remove_curses_spell; /* doesn't matter for this test */
			s32b max = 0;
			s32b min = 25; /* doesn't matter for this test */
			// Exercise
			get_level_device(spell, max, min, device_skill, fake_get_level);
			// Verify
			AssertThat(passed_in_max, Equals(50));
		});

		//
		// Table-driven tests derived from empirical testing
		// using printf.
		//

		for (auto device_skill: device_skill_values)
		{
			it("calculates 'Remove Curses' staff level correctly for different magic device levels" , [&] {
				// Setup: Device values for Remove Curses staff
				get_level_use_stick = 1;
				get_level_max_stick = 15;
				// Setup: Max and min
				s32b max = 50;
				s32b min = 1;
				// Exercise
				s32b actualLevel = get_level_device(remove_curses_spell, max, min, device_skill);
				// Verify: Check expected levels.
				AssertThat(actualLevel, Equals(remove_curses_expected_levels.at(device_skill)));
			});

			it("calculates 'Wish' staff level correctly for different magic device levels", [&] {
				// Setup: Device values for Wish staff
				get_level_use_stick = 1;
				get_level_max_stick = 1;
				// Setup: Max and min
				s32b max = 50;
				s32b min = 1;
				// Exercise
				s32b actualLevel = get_level_device(wish_spell, max, min, device_skill);
				// Verify: Check expected levels.
				AssertThat(actualLevel, Equals(wish_expected_levels.at(device_skill)));
			});

			it("calculates 'Heal Monster' wand level correctly for different magic device levels", [&] {
				// Setup: Device values for Heal Monster wand
				get_level_use_stick = 1;
				get_level_max_stick = 20;
				// Setup: Max and min
				s32b max = 380;
				s32b min = 1;
				// Exercise
				s32b actualLevel = get_level_device(heal_monster_spell, max, min, device_skill);
				// Verify: Check expected levels.
				AssertThat(actualLevel, Equals(heal_monster_expected_levels.at(device_skill)));
			});

			it("calculates 'Teleport Away' wand level correctly for different magic device levels", [&] {
				// Setup: Device values for Teleport Away wand
				get_level_use_stick = 3;
				get_level_max_stick = 20;
				// Setup: Max and min
				s32b max = 50;
				s32b min = 1;
				// Exercise
				s32b actualLevel = get_level_device(teleport_away_spell, max, min, device_skill);
				// Verify: Check expected levels.
				AssertThat(actualLevel, Equals(teleport_away_expected_levels.at(device_skill)));
			});
		}

	});

});
