#include "flag_set.hpp"
#include <bandit/bandit.h>
using namespace bandit;

//
// Tests
//

go_bandit([]() {

	describe("flag_set", []() {

		// Convenience typedef
		typedef flag_set<2> fs_t;

		it("make function should handle tier=0, index=31 properly", [&] {
			// Setup
			fs_t fs = fs_t::make(0, 31);
			// Exercise
			auto result = fs;
			// Verify
			AssertThat(result[0], Equals(2147483648UL));
			AssertThat(result[1], Equals(0UL));
		});

		it("make function should handle tier=1, index=31 properly", [&] {
			// Setup
			fs_t fs = fs_t::make(1, 31);
			// Exercise
			auto result = fs;
			// Verify
			AssertThat(result[0], Equals(0UL));
			AssertThat(result[1], Equals(2147483648UL));
		});

		it("make function should respect the tier and index", [&] {
			// Exercise
			fs_t fs = fs_t::make(1, 7);
			// Verify
			AssertThat(fs[0], Equals(0UL));
			AssertThat(fs[1], Equals(128UL));
		});

		it("bool conversion should return false for zero flags", [&] {
			// Setup
			fs_t fs = fs_t();
			// Exercise
			auto result = bool(fs);
			// Verify
			AssertThat(result, Equals(false));
		});

		it("bool conversion should return true for non-zero flags", [&] {
			// Setup
			fs_t fs = fs_t::make(1, 3);
			// Exercise
			auto result = bool(fs);
			// Verify
			AssertThat(result, Equals(true));
		});

		it("== operator should compare equals as equals", [&] {
			// Setup
			fs_t fs1 = fs_t::make(1, 3);
			fs_t fs2 = fs_t::make(1, 3);
			// Exercise
			auto result = (fs1 == fs2);
			// Verify
			AssertThat(result, Equals(true));
		});

		it("| operator should respect the tier and index", [&] {
			// Setup
			fs_t fs1 = fs_t::make(0, 31);
			fs_t fs2 = fs_t::make(1, 3);
			// Exercise
			fs_t fs = fs1 | fs2;
			// Verify
			AssertThat(fs[0], Equals(2147483648UL));
			AssertThat(fs[1], Equals(8UL));
		});

		it("& operator should respect the tier and index", [&] {
			// Setup
			fs_t fs = fs_t::make(0, 31) | fs_t::make(1, 3);
			// Exercise
			fs_t result = fs & fs;
			// Verify
			AssertThat(result[0], Equals(2147483648UL));
			AssertThat(result[1], Equals(8UL));
		});

		it("make_bit(5) should set the 5th bit in the 1st tier", [&] {
			// Exercise
			fs_t result = fs_t::make_bit(5);
			// Verify
			AssertThat(result[0], Equals(32L));
			AssertThat(result[1], Equals(0L));
		});

		it("make_bit(37) should set the 5th bit in the 2nd tier", [&] {
			// Exercise
			fs_t result = fs_t::make_bit(37);
			// Verify
			AssertThat(result[0], Equals(0L));
			AssertThat(result[1], Equals(32L));
		});
	});

});
