#include "lua_bind.hpp"
#include "spell_type.hpp"
#include <bandit/bandit.h>
using namespace bandit;

go_bandit([]() {
	describe("lua_get_level", []() {

		auto createEntsPotion = ([]() {
			auto my_spell = spell_type_new("TEST: Ent's Potion");
			spell_type_set_difficulty(my_spell, 6, 35); // Copied from standard Ent's Potion spell.
			return my_spell;
		});
		
		auto createSenseHidden = ([]() {
			auto my_spell = spell_type_new("TEST: Sense Hidden");
			spell_type_set_difficulty(my_spell, 5, 25); // Copied from standard Sense Hidden spell.
			return my_spell;
		});

		//
		// Test cases derived from empirical testing of the code before refactoring.
		//
		
		it("calculates \"Ent's Potion\" level appropriately for Sorcery@1.0", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 100, 50, -50, 0);
			// Verify
			AssertThat(actualResult, Equals(-4));
		});
		
		it("calculates \"Ent's Potion\" cost appropriately for Sorcery@1.0", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 100, 15, 7, 0);
			// Verify
			AssertThat(actualResult, Equals(7));
		});

		it("calculates \"Ent's Potion\" level appropriately for Sorcery@25.0", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 50, 1, 0);
			// Verify
			AssertThat(actualResult, Equals(20));
		});

		it("calculates \"Ent's Potion\" cost appropriately for Sorcery@25.0", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 15, 7, 0);
			// Verify
			AssertThat(actualResult, Equals(7));
		});

		it("calculates \"Ent's Potion\" level appropriately for Sorcery@25.0 with +3 equipment SP bonus", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 50, 1, 300);
			// Verify
			AssertThat(actualResult, Equals(23));
		});

		it("calculates \"Ent's Potion\" cost appropriately for Sorcery@25.0 with +3 equipment SP bonus", [&](){
			// Setup
			auto my_spell = createEntsPotion();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 15, 7, 300);
			// Verify
			AssertThat(actualResult, Equals(7));

		});

		it("calculates \"Sense Hidden\" level appropriately for Sorcery@1.0", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 100, 50, -50, 0);
			// Verify
			AssertThat(actualResult, Equals(-3));
		});
		
		it("calculates \"Sense Hidden\" cost appropriately for Sorcery@1.0", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 100, 10, 2, 0);
			// Verify
			AssertThat(actualResult, Equals(2));
		});

		it("calculates \"Sense Hidden\" level appropriately for Sorcery@25.0", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 50, 1, 0);
			// Verify
			AssertThat(actualResult, Equals(21));
		});

		it("calculates \"Sense Hidden\" cost appropriately for Sorcery@25.0", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 10, 2, 0);
			// Verify
			AssertThat(actualResult, Equals(4));
		});

		it("calculates \"Sense Hidden\" level appropriately for Sorcery@25.0 with +3 equipment SP bonus", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 50, 1, 300);
			// Verify
			AssertThat(actualResult, Equals(24));
		});

		it("calculates \"Sense Hidden\" cost appropriately for Sorcery@25.0 with +3 equipment SP bonus", [&](){
			// Setup
			auto my_spell = createSenseHidden();
			// Exercise
			auto actualResult = lua_get_level(my_spell, 2500, 10, 2, 300);
			// Verify
			AssertThat(actualResult, Equals(4));
		});

	});
});
