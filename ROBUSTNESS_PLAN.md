# Robustness Plan: Memory Errors & Logic Bugs

This plan lays out concrete, prioritized work to reduce memory-safety bugs
(crashes, corruption, UB) and logic bugs (incorrect game behavior) in the
ToME codebase. It's grounded in the current state of the tree, not generic
advice — see "Current state" below for the numbers behind each phase.

## Current state (baseline)

- ~185K lines of C++17 across 398 files in `src/`, a modernized fork of an
  originally-C Angband variant.
- Sanitizers are already configured in `CMakeLists.txt`
  (`-fsanitize=undefined -fsanitize=address` plus `_GLIBCXX_DEBUG` /
  `_GLIBCXX_DEBUG_PEDANTIC` on Debug builds), but **CI never uses them** —
  `.github/workflows/build.yml` does a plain `cmake ..` (no build type set)
  and never runs the test binary.
- Legacy C-isms are still widespread: 185 `strcpy`, 199 `sprintf`, ~455
  unsafe libc string/memory calls total, 385 raw `new`/`delete` sites vs.
  only 158 smart-pointer uses, and roughly 655 fixed-size global/stack
  C-arrays (the classic Angband out-of-bounds-write pattern — arrays
  indexed by monster/object/feature ID).
- Test coverage is thin: only 6 files under `tests/` (bandit framework) —
  `grid.cc`, `flag_set.cc`, `get_level_device.cc`, `lua_get_level.cc`,
  `boost_optional.cc`, plus the harness. Nothing covers combat math,
  dungeon generation, or save/load.
- 190 `assert()` calls exist but compile out under `NDEBUG`/Release, so
  they protect nothing in the build players actually run.
- `src/loadsave.cc` / `.hpp` parses the save-file format — the highest-value
  target for both fuzzing and logic-bug regression tests — and nothing
  currently touches it.
- No static analysis (clang-tidy/cppcheck), no `-Werror`, no fuzz harness
  anywhere in the tree. `fmt` is already a dependency and in use in 32
  files, so a safe formatting path already exists.

## Phase 1 — Turn on what already exists (cheap, immediate)

1. Add a CI job that builds with `CMAKE_BUILD_TYPE=Debug` (activates the
   existing ASAN/UBSAN/`_GLIBCXX_DEBUG` flags) and runs `ctest` / the
   bandit test binary, alongside the current build.
2. Make asserts live in shipped builds: introduce a project `ASSERT()`
   macro that stays active regardless of `NDEBUG` (or build Release with
   `-UNDEBUG` for asserts specifically), and migrate the 190 call sites
   incrementally.
3. Add `-Werror` for newly-touched files only (per-directory or
   incremental flag), so warnings stop regressing without forcing a
   repo-wide cleanup up front.

## Phase 2 — Static analysis

4. Wire `clang-tidy` into CI (start with `bugprone-*` and
   `cppcoreguidelines-*` memory checks, warnings-only, non-blocking) to
   catch new instances of the patterns below without demanding a full
   historical cleanup.
5. Add `cppcheck` as a second, cheap pass focused on array-bounds and
   use-after-free patterns.

## Phase 3 — Memory-safety cleanup, prioritized by blast radius

6. Replace `sprintf`/`strcpy`/`strcat` with `fmt::format` or `snprintf`,
   starting with the highest-traffic modules (`cmd*.cc`, `spells*.cc`)
   rather than a mechanical repo-wide sweep.
7. Audit the fixed-size C-arrays indexed by game IDs (monster/object/
   feature/dungeon indices) — replace raw indexing with a small
   bounds-checked wrapper, or `std::array`/`std::vector` + `.at()` in
   debug builds, focusing first on save/load and dungeon-generation code
   where corrupted indices are most likely to originate.
8. Convert remaining raw `new`/`delete` pairs to RAII (`unique_ptr` /
   containers) opportunistically as files are touched — no dedicated
   migration sprint needed given only 385 sites remain and 158 are
   already converted.

## Phase 4 — Fuzzing and save-file hardening

9. Build a libFuzzer harness around `loadsave.cc`'s load path (ASAN is
   already configured, so this is mostly plumbing) — save files are
   exactly the kind of semi-external input that causes both crashes and
   silent-corruption logic bugs.
10. Add round-trip tests (save → load → compare) as regression coverage
    for the loader.

## Phase 5 — Logic-bug coverage via tests

11. Expand the bandit suite past infrastructure tests into gameplay
    invariants: combat/damage formulas, spell effect resolution,
    dungeon-generation connectivity (does every generated level have a
    reachable stair/exit?), and inventory/object-index consistency.
12. Treat every bug fix from here on as requiring an accompanying
    regression test in the same PR — the cheapest way to build the suite
    over time without a big-bang effort.

## Ongoing process

13. Add a short code-review checklist item (and/or a `CLAUDE.md` note,
    which doesn't currently exist) calling out: no new raw arrays without
    bounds checks, no new `sprintf`/`strcpy`, prefer `fmt::format`, add
    asserts on invariants at module boundaries.

## Suggested sequencing

Phases 1 and 2 first — days, not weeks, and immediately give crash
detection and static-analysis signal on every PR. Phases 3–5 are an
ongoing backlog worked incrementally as files are touched and bugs are
fixed, rather than a blocking cleanup effort.
