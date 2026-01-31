# ToME2 Modernization Plan

**Version:** 1.0
**Date:** January 2026
**Target:** macOS/Linux Terminal Application

---

## Executive Summary

This document outlines the strategy to modernize the ToME2 codebase from a multi-platform, multi-frontend roguelike into a clean, maintainable terminal application targeting modern macOS and Linux systems.

### Current State
- ~180,000 lines of C++17 code across ~300 files
- 4 frontend implementations (X11, GTK2, ncurses, Windows)
- 25+ years of accumulated technical debt (Angband heritage)
- Deep global state dependencies (175+ global variables)
- Custom data file format with 7,000 lines of parsing code
- Module system supporting 2 game variants

### Target State
- Single terminal frontend (ncurses or modern alternative)
- Clean, modern C++ with minimal global state
- Standard data formats where beneficial
- Simplified build system
- ~40-50% code reduction through removal of unused code

### Estimated Code Reduction

| Category | Lines Removable | % of Codebase |
|----------|----------------|---------------|
| Windows frontend | 3,354 | 1.9% |
| X11 frontend | 1,742 | 1.0% |
| GTK2 frontend | 1,965 | 1.1% |
| Module system (Theme) | ~5,000 | 2.8% |
| Lua naming vestiges | 430 | 0.2% |
| Dead code & simplification | ~10,000+ | 5.5%+ |
| **Conservative Total** | **~22,500** | **12.5%** |

---

## Part 1: Code Removal Plan

### 1.1 Frontend Consolidation (7,061 lines)

**Remove completely:**

| File | Lines | Notes |
|------|-------|-------|
| `src/main-win.cc` | 3,354 | Windows-only |
| `src/main-x11.cc` | 1,742 | X11 GUI |
| `src/main-gtk2.cc` | 1,965 | GTK2 GUI |
| `src/angband.rc` | ~50 | Windows resources |
| `src/angband.ico` | - | Windows icon |

**Keep:**
- `src/main-gcu.cc` (926 lines) - ncurses terminal frontend

**Simplification tasks:**
1. Remove frontend selection logic from `main.cc`
2. Remove CMake conditionals for X11, GTK2, Windows
3. Simplify `Frontend` interface to terminal-only requirements
4. Remove graphics/tile support code paths

### 1.2 Module System Removal (~5,000 lines)

The module system supports two game variants: "ToME" (core) and "Theme" (alternative). For a single-game build:

**Remove:**
- `lib/mods/theme/` directory (2.6 MB, 278 files)
- Module selection UI in `modules.cc`
- `module_type` struct and `modules[]` array
- All `MODULE_TOME`/`MODULE_THEME` conditionals (56 locations in 22 files)

**Affected files requiring modification:**

| File | Changes Required |
|------|------------------|
| `birth.cc` | Remove module-specific starting equipment |
| `cmd4.cc`, `cmd6.cc` | Remove module conditionals |
| `corrupt.cc` | Simplify corruption availability |
| `gods.cc` | Remove module field from deities |
| `q_god.cc`, `q_library.cc` | Remove quest variations |
| `spells3.cc`, `spells4.cc`, `spells5.cc` | Consolidate spell implementations |
| `skills.cc`, `xtra2.cc` | Remove module-specific calculations |
| `init2.cc` | Remove `select_module()` |

**Decision required:** Keep ToME or Theme content? Recommend: Keep ToME (the original).

### 1.3 Lua Naming Cleanup (430 lines)

The "lua_" prefix is vestigial—no actual Lua interpreter exists. This is purely C++ code.

**Rename and reorganize:**

| Current | New Location | Purpose |
|---------|--------------|---------|
| `lua_bind.cc/.hpp` | `spell_calc.cc/.hpp` | Spell level/mana calculations |
| `lua_get_level()` | `spell_level_from_skill()` | Core spell scaling |
| `get_level_s()` | Keep name, move file | Used 317 times |
| `load_map()` | `dungeon_io.cc` | Quest map loading |

**Effort:** 2-3 hours. Low risk—pure renaming with no logic changes.

### 1.4 Platform-Specific Code Cleanup

**Remove from `h-basic.hpp`:**
- Windows detection macros (`_Windows`, `__WIN32__`, etc.)
- `PATH_SEP` conditional (hardcode to `"/"`)

**Remove from `util.cc`:**
- Windows-specific user name lookup
- `#ifdef WINDOWS` blocks

**Simplify:**
- Terminal control in `main-gcu.cc` (keep POSIX only, remove `USE_TERMIO`, `USE_TCHARS`)
- File path handling (Boost.Filesystem handles this)

---

## Part 2: Architecture Modernization

### 2.1 Global State Strategy

**Current situation:** 175+ global variables, 85% of files depend on them.

**Pragmatic approach:** Full elimination is a 3-6 month project. Instead:

#### Phase 1: Isolation (Week 1-2)
Encapsulate globals into logical structs without changing access patterns:

```cpp
// Before: scattered globals
extern s16b cur_hgt, cur_wid;
extern byte feeling;
extern s32b turn;

// After: grouped in structs
struct LevelState {
    s16b height, width;
    byte feeling;
};

struct GameClock {
    s32b turn;
    s32b old_turn;
};

extern LevelState level;
extern GameClock clock;
```

#### Phase 2: Accessor Functions (Week 3-4)
Add accessor functions that can later enforce invariants:

```cpp
// Gradual migration path
inline s16b level_height() { return level.height; }
inline void set_level_height(s16b h) { level.height = h; }
```

#### Phase 3: Dependency Injection (Future)
Pass state through function parameters where practical. Target high-value areas:
- Test harness functions
- Pure calculation functions
- New code

**Globals to encapsulate (priority order):**

| Group | Variables | Effort |
|-------|-----------|--------|
| Path/Directory | `ANGBAND_DIR*` (11 vars) | Low |
| Game Clock | `turn`, `old_turn`, `running`, `resting` | Low |
| Level State | `cur_hgt`, `cur_wid`, `dun_level`, `feeling` | Medium |
| UI State | `panel_*`, `target_*`, `health_who` | Medium |
| Player Refs | `p_ptr`, `rp_ptr`, `cp_ptr` | High |
| Level Data | `cave`, `o_list`, `m_list` | Very High |

### 2.2 Source Organization

**Current structure:** Flat directory with 300+ files, Angband naming conventions.

**Proposed structure:**

```
src/
├── core/                    # Core game engine
│   ├── game.hpp/cc         # Game state container
│   ├── clock.hpp/cc        # Turn management
│   ├── rng.hpp/cc          # Random number generation
│   └── config.hpp/cc       # Configuration management
│
├── entity/                  # Game entities
│   ├── player/
│   │   ├── player.hpp/cc   # Player state (from player_type)
│   │   ├── race.hpp/cc     # Race definitions
│   │   ├── class.hpp/cc    # Class definitions
│   │   └── stats.hpp/cc    # Stat calculations
│   ├── monster/
│   │   ├── monster.hpp/cc  # Monster state
│   │   ├── ai.hpp/cc       # Monster AI (from melee2)
│   │   └── race.hpp/cc     # Monster race definitions
│   └── object/
│       ├── object.hpp/cc   # Object state
│       ├── kind.hpp/cc     # Object kinds
│       └── artifact.hpp/cc # Artifacts
│
├── world/                   # World/dungeon systems
│   ├── level.hpp/cc        # Level state
│   ├── cave.hpp/cc         # Cave grid
│   ├── generate.hpp/cc     # Dungeon generation
│   ├── feature.hpp/cc      # Terrain features
│   └── wilderness.hpp/cc   # Overworld
│
├── mechanics/               # Game mechanics
│   ├── combat.hpp/cc       # Combat resolution (from melee1)
│   ├── magic/
│   │   ├── spell.hpp/cc    # Spell system
│   │   ├── school.hpp/cc   # Magic schools
│   │   └── effect.hpp/cc   # Spell effects (from spells1-6)
│   ├── skills.hpp/cc       # Skill system
│   └── quest/              # Quest implementations
│       ├── quest.hpp/cc    # Quest framework
│       └── *.cc            # Individual quests
│
├── ui/                      # User interface
│   ├── terminal.hpp/cc     # Terminal abstraction (from z-term)
│   ├── display.hpp/cc      # Game display
│   ├── input.hpp/cc        # Input handling
│   ├── menu.hpp/cc         # Menu system
│   └── message.hpp/cc      # Message buffer
│
├── io/                      # File I/O
│   ├── save.hpp/cc         # Save/load (from loadsave)
│   ├── data_loader.hpp/cc  # Game data parsing (from init1)
│   └── help.hpp/cc         # Help system
│
├── cmd/                     # Command handlers
│   ├── movement.hpp/cc     # Movement commands (from cmd1)
│   ├── inventory.hpp/cc    # Inventory commands (from cmd2, cmd3)
│   ├── magic.hpp/cc        # Spell commands (from cmd5)
│   ├── ability.hpp/cc      # Ability commands (from cmd6, cmd7)
│   └── info.hpp/cc         # Information commands (from cmd4)
│
└── util/                    # Utilities
    ├── types.hpp           # Type definitions (from h-basic)
    ├── flags.hpp           # Flag system (from flag_set)
    ├── format.hpp/cc       # String formatting (from z-form)
    └── dice.hpp/cc         # Dice notation
```

**Migration approach:**
1. Create new directory structure
2. Move files incrementally, updating includes
3. Split large files (e.g., `spells1.cc` at 5,000+ lines) during moves
4. Update CMakeLists.txt after each batch

### 2.3 Naming Convention Modernization

**Current conventions (Angband heritage):**
- Short variable names: `py`, `px`, `au`, `y`, `x`
- Inconsistent casing: `player_type`, `Term_xtra`, `msg_print`
- Macro-heavy: `ABS()`, `A2I()`, `KTRL()`

**Target conventions:**

| Element | Convention | Example |
|---------|------------|---------|
| Types/Classes | PascalCase | `PlayerState`, `MonsterRace` |
| Functions | snake_case | `calculate_damage()`, `spawn_monster()` |
| Variables | snake_case | `player_x`, `current_level` |
| Constants | SCREAMING_CASE | `MAX_LEVEL`, `TERM_WIDTH` |
| Private members | trailing underscore | `health_`, `position_` |
| Namespaces | lowercase | `tome::entity`, `tome::mechanics` |

**Gradual migration:** Use `// LEGACY:` comments to mark old-style code during transition.

---

## Part 3: Technical Decisions

### 3.1 Terminal Library Selection

#### Option A: Keep ncurses (Recommended)

**Pros:**
- Already implemented and working (`main-gcu.cc`, 926 lines)
- Battle-tested, universally available
- Available via Homebrew: `brew install ncurses`
- Zero migration risk
- Team likely already familiar

**Cons:**
- C API requires wrapping for modern C++
- Limited Unicode support (but sufficient for roguelike)
- No mouse support (acceptable for this game)

**Effort:** None for status quo; optional C++ wrapper ~1 week

#### Option B: notcurses

**Pros:**
- Modern C library (2019+), actively maintained
- Better Unicode and emoji support
- True color support
- Pixel graphics capability (future potential)
- Clean C API, easier to wrap

**Cons:**
- Additional dependency: `brew install notcurses`
- Requires full frontend rewrite (~2-3 weeks)
- Less widespread than ncurses
- Team learning curve

**Effort:** 2-3 weeks for frontend rewrite + testing

#### Option C: FTXUI

**Pros:**
- Native C++ library (no C wrapping needed)
- Declarative/functional UI model
- Built-in components (menus, inputs)
- Modern design patterns

**Cons:**
- Significant paradigm shift from ncurses
- Less suited for character-grid games
- Smaller community
- Would require substantial UI architecture changes

**Effort:** 4-6 weeks, high risk

#### Recommendation

**Keep ncurses** for initial modernization. The existing implementation works, and rewriting the terminal layer is high effort with low gameplay value. Consider notcurses as a future enhancement once core refactoring is complete.

### 3.2 Data File Format

#### Option A: Keep Custom Format

**Pros:**
- Working parser exists (7,000 lines)
- Human-readable and editable
- No migration effort
- Modders (if any) are familiar with it

**Cons:**
- 7,000 lines of custom parsing code to maintain
- Poor error messages
- No schema validation
- Fragile format (typos cause cryptic failures)

**Effort:** None, but carries maintenance burden

#### Option B: Migrate to JSON

**Pros:**
- jsoncons library already vendored
- Standard format, tooling available
- Schema validation possible (JSON Schema)
- Better error messages
- Self-documenting structure

**Cons:**
- 2-3 weeks migration effort
- ~58,000 lines of data files to convert
- More verbose than current format
- Slightly larger file sizes

**Example conversion:**
```
# Current format (k_info.txt)
N:1:Blindness
G:,:d
I:80:1:500
W:5:0:1:0

# JSON equivalent
{
  "id": 1,
  "name": "Blindness",
  "glyph": { "char": ",", "color": "dark" },
  "info": { "type": 80, "subtype": 1, "weight": 500 },
  "depth": 5
}
```

#### Option C: Migrate to YAML

**Pros:**
- More readable than JSON for humans
- Comments supported
- Less verbose

**Cons:**
- Need to add yaml-cpp dependency
- YAML parsing is more complex than JSON
- Whitespace sensitivity can cause issues

#### Option D: Hybrid Approach (Recommended)

1. **Keep current format** for initial release
2. **Refactor parser** into cleaner, more maintainable code
3. **Add JSON export** capability for tooling
4. **Future migration** to JSON when beneficial

**Effort:** 1 week for parser refactoring, defer full migration

### 3.3 C++ Standard Selection

#### Option A: C++17 (Current)

**Pros:**
- Already in use, no changes needed
- Broad compiler support
- `std::optional`, `std::string_view`, structured bindings available

**Cons:**
- Missing some useful features (ranges, concepts, modules)

#### Option B: C++20 (Recommended)

**Pros:**
- Ranges library simplifies many algorithms
- Concepts improve template error messages
- `std::format` for type-safe formatting
- Designated initializers
- Three-way comparison (`<=>`)
- Apple Clang 14+ supports most features (macOS 13+)

**Cons:**
- Some features still have compiler bugs
- Modules support incomplete

**Example improvements:**
```cpp
// C++17
std::vector<Monster*> result;
std::copy_if(monsters.begin(), monsters.end(),
             std::back_inserter(result),
             [](Monster* m) { return m->is_visible(); });

// C++20
auto visible = monsters | std::views::filter(&Monster::is_visible);
```

#### Option C: C++23

**Pros:**
- `std::expected` for error handling
- More ranges improvements
- `std::print` for formatted output

**Cons:**
- Limited compiler support currently
- May cause compatibility issues

#### Recommendation

**Target C++20** with fallback compatibility for C++17. Use feature detection macros where needed.

### 3.4 Refactoring Approach

#### Option A: Incremental Refactoring

**Approach:** Small, continuous improvements while maintaining working builds.

**Pros:**
- Always have working game
- Lower risk per change
- Can ship improvements gradually
- Easier to review and test

**Cons:**
- Slower overall progress
- May accumulate transitional code
- Some large changes hard to do incrementally
- Technical debt may persist longer

**Timeline:** 6-12 months for full modernization

#### Option B: Branch-Based Restructuring

**Approach:** Major restructuring on feature branch, merge when complete.

**Pros:**
- Can make sweeping changes without worrying about intermediate states
- Cleaner final result
- Faster for structural changes

**Cons:**
- Long-lived branch divergence
- Merge conflicts if main branch changes
- No usable game until complete
- Higher risk of introducing bugs

**Timeline:** 3-6 months, but higher risk

#### Option C: Hybrid Approach (Recommended)

1. **Incremental** for: Code removal, renaming, small refactors
2. **Branch-based** for: Directory restructuring, major architectural changes
3. **Feature flags** for: New implementations that can coexist with old

**Specific recommendations:**
- Remove Windows/X11/GTK code incrementally (low risk)
- Restructure directories on a branch (merge within 2 weeks)
- Refactor global state incrementally with compatibility shims
- New data loader (if JSON) on branch, switch atomically

---

## Part 4: Implementation Roadmap

### Phase 0: Preparation (1-2 weeks)

**Objective:** Establish foundation for safe refactoring.

1. **Expand test coverage**
   - Add integration tests for core gameplay loops
   - Add save/load round-trip tests
   - Create test characters at various progression points

2. **Set up CI/CD**
   - macOS build verification
   - Automated test runs
   - Code formatting checks (clang-format)

3. **Documentation**
   - Document current architecture
   - Create dependency graph of major modules
   - Identify highest-risk areas

### Phase 1: Code Removal (2-3 weeks)

**Objective:** Reduce codebase by removing unused code.

**Week 1:**
- Remove Windows frontend (`main-win.cc`, resources)
- Remove X11 frontend (`main-x11.cc`)
- Remove GTK2 frontend (`main-gtk2.cc`)
- Simplify CMake build system
- Verify ncurses build works

**Week 2:**
- Remove module system (keep ToME content only)
- Delete `lib/mods/theme/` directory
- Remove all `MODULE_*` conditionals
- Update save file format (bump version)

**Week 3:**
- Rename Lua vestiges
- Remove platform-specific code paths
- Clean up unused macros and defines
- Remove dead code identified by static analysis

**Deliverable:** Working game with ~20% less code.

### Phase 2: Foundation Modernization (3-4 weeks)

**Objective:** Establish modern code patterns.

**Week 4-5:**
- Group global variables into logical structs
- Create accessor functions for grouped state
- Encapsulate path/directory configuration
- Modernize type aliases

**Week 6-7:**
- Restructure source directory layout
- Update all include paths
- Split oversized files (>2000 lines)
- Apply consistent naming conventions to new code

**Deliverable:** Clean directory structure, reduced global coupling.

### Phase 3: Subsystem Refactoring (4-6 weeks)

**Objective:** Modernize critical subsystems.

**Priority order:**

1. **Save/Load System** (1 week)
   - Modernize serialization
   - Add versioning support
   - Improve error handling

2. **Data Loading** (1-2 weeks)
   - Refactor parser into smaller functions
   - Add validation layer
   - Improve error messages

3. **Terminal/UI Layer** (1-2 weeks)
   - Create clean C++ wrapper around ncurses
   - Separate display logic from game logic
   - Modernize input handling

4. **Combat/Magic Systems** (1-2 weeks)
   - Extract pure calculation functions
   - Add unit tests for formulas
   - Document mechanics

**Deliverable:** Maintainable, well-tested core systems.

### Phase 4: Polish & Optimization (2-3 weeks)

**Objective:** Production-ready quality.

1. **Performance profiling**
   - Identify bottlenecks
   - Optimize hot paths
   - Memory usage analysis

2. **User experience**
   - Improve terminal rendering
   - Better error messages
   - Configuration file support

3. **Documentation**
   - Update CLAUDE.md
   - Add developer documentation
   - Create build instructions for macOS/Linux

4. **Release preparation**
   - Homebrew formula
   - Release notes
   - Version tagging

---

## Part 5: Risk Analysis

### High Risk Areas

| Risk | Impact | Mitigation |
|------|--------|------------|
| Breaking game balance | Players notice immediately | Comprehensive testing, preserve all formulas exactly |
| Save file corruption | Data loss | Version migration, backup on load |
| Combat/magic bugs | Gameplay affected | Unit tests for all calculations |
| Performance regression | Slow gameplay | Profile before/after major changes |

### Medium Risk Areas

| Risk | Impact | Mitigation |
|------|--------|------------|
| Build system breakage | Can't compile | CI/CD on every commit |
| Platform compatibility | Won't run on some Macs | Test on multiple macOS versions |
| Terminal rendering issues | Visual glitches | Test on multiple terminal emulators |

### Low Risk Areas

| Risk | Impact | Mitigation |
|------|--------|------------|
| Code style inconsistency | Maintainability | Gradual migration, document conventions |
| Documentation gaps | Onboarding slower | Update docs as code changes |

---

## Part 6: Success Criteria

### Minimum Viable Modernization

- [ ] Builds and runs on macOS 12+ from terminal
- [ ] All gameplay features functional
- [ ] Single executable, no GUI dependencies
- [ ] Clean build with no warnings
- [ ] Passes all existing tests

### Target Modernization

- [ ] 30%+ reduction in code size
- [ ] No Windows/X11/GTK code remaining
- [ ] Organized directory structure
- [ ] Grouped global state
- [ ] C++20 features where beneficial
- [ ] Improved test coverage (>50% of core systems)
- [ ] Build time under 2 minutes
- [ ] Documented architecture

### Stretch Goals

- [ ] JSON data format with schema validation
- [ ] Full unit test coverage for combat/magic
- [ ] Modern terminal library (notcurses)
- [ ] Dependency injection for testability
- [ ] Homebrew distribution

---

## Appendices

### A. File Inventory by Priority

**Remove immediately:**
- `src/main-win.cc` (3,354 lines)
- `src/main-x11.cc` (1,742 lines)
- `src/main-gtk2.cc` (1,965 lines)
- `src/angband.rc`, `src/angband.ico`
- `lib/mods/theme/` (entire directory)

**Refactor high priority:**
- `src/variable.cc/.hpp` (global state)
- `src/init1.cc` (data parsing)
- `src/loadsave.cc` (serialization)

**Split due to size:**
- `src/generate.cc` (5,500 lines)
- `src/spells1.cc` (5,300 lines)
- `src/cmd6.cc` (4,000 lines)
- `src/cave.cc` (3,200 lines)
- `src/melee2.cc` (4,200 lines)

### B. Dependency Graph (Major Modules)

```
                    ┌──────────┐
                    │  main.cc │
                    └────┬─────┘
                         │
              ┌──────────┼──────────┐
              ▼          ▼          ▼
         ┌────────┐ ┌────────┐ ┌────────┐
         │frontend│ │ game.cc│ │init2.cc│
         └────────┘ └───┬────┘ └───┬────┘
                        │          │
         ┌──────────────┼──────────┤
         ▼              ▼          ▼
    ┌─────────┐   ┌──────────┐ ┌────────┐
    │dungeon.cc│   │variable.cc│ │init1.cc│
    └────┬────┘   └─────┬────┘ └────────┘
         │              │
    ┌────┴────┬────────┴─────┬──────────┐
    ▼         ▼              ▼          ▼
┌───────┐ ┌───────┐    ┌─────────┐ ┌────────┐
│cave.cc│ │cmd*.cc│    │monster*.cc│ │object*.cc│
└───────┘ └───────┘    └─────────┘ └────────┘
```

### C. Build System Changes

**Current CMake structure:**
```cmake
# Complex multi-platform detection
IF(X11_FOUND) ... ENDIF()
IF(GTK2_FOUND) ... ENDIF()
IF(WIN32) ... ENDIF()
```

**Target CMake structure:**
```cmake
# Simple single-platform build
find_package(Curses REQUIRED)
add_executable(tome ${SOURCES})
target_link_libraries(tome PRIVATE ${CURSES_LIBRARIES})
```

### D. Testing Strategy

**Unit tests (new):**
- Combat damage calculations
- Spell effect calculations
- Experience/level calculations
- Item generation probabilities

**Integration tests (expand):**
- Character creation → save → load → verify
- Combat sequence → verify HP changes
- Spell casting → verify effects applied

**Regression tests:**
- Specific bug scenarios
- Edge cases from issue tracker

---

## Document Control

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | Jan 2026 | AI Analysis | Initial version |

---

*This document should be reviewed and updated as implementation progresses. Technical decisions marked as recommendations should be validated by the team before implementation begins.*
