#include "grid.hpp"
#include <bandit/bandit.h>
using namespace bandit;

go_bandit([]() {
	describe("grid<>", []() {

		auto w0 = size_t { 123 };
		auto h0 = size_t { 42 };

		it("width(...) properly sets returned width", [&](){
			// Setup
			grid<int> g;
			// Exercise
			g.width(w0);
			// Verify
			AssertThat(g.width(), Equals(w0));
		});

		it("height(...) properly sets returned height", [&](){
			// Setup
			grid<int> g;
			// Exercise
			g.height(h0);
			// Verify
			AssertThat(g.height(), Equals(h0));
		});

		it("resize(w, h) properly sets returned width and height", [&](){
			// Setup
			grid<int> g;
			// Exercise
			g.resize(w0, h0);
			// Verify
			AssertThat(g.width(), Equals(w0));
			AssertThat(g.height(), Equals(h0));
		});

		it("operator () can access ((w-1), (h-1)) element", [&](){
			// Class with 'magic' default value
			struct X { int magic = 1001; };
			// Setup
			grid<X> g;
			g.resize(w0 + 1, h0 + 1);
			// Exercise
			auto x = g(w0, h0);
			// Verify
			AssertThat(x.magic, Equals(1001));
		});

	});
});
