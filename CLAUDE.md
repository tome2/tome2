# CLAUDE.md - AI Assistant Guide for ToME2

This document provides guidance for AI assistants working on the ToME2 codebase.

## Project Overview

ToME (Troubles of Middle-Earth) is a rogue-like game derived from Angband, set in Tolkien's Middle-Earth universe. The codebase is written in C++17 and supports multiple frontends (X11, ncurses, GTK2, Windows).

## Build System

### Prerequisites
- CMake 3.15+
- Boost 1.74.0+ (filesystem component required)
- C++17 compatible compiler

### Optional dependencies (for frontends)
- X11: `libx11-dev`
- ncurses: `libncurses5-dev`
- GTK2: `libgtk2.0-dev`

### Auto-fetched dependencies (via CPM.cmake)
- fmt (12.0.0) - string formatting
- jsoncons (1.4.2) - JSON parsing
- pcg-cpp - random number generation

### Build Commands

```bash
# In-place build (recommended for development)
cmake .
make

# Out-of-tree build
mkdir build && cd build
cmake ..
make

# System-wide installation
cmake -DSYSTEM_INSTALL:BOOL=true .
make
sudo make install
```

### Build Outputs
- `src/tome-x11` - X11 frontend
- `src/tome-gcu` - ncurses frontend
- `src/tome-gtk2` - GTK2 frontend
- `src/tome-win` - Windows frontend (when building on Windows)
- `src/harness` - Test executable

## Project Structure

```
tome2/
├── src/                    # C++ source code (~300 files)
│   ├── cmd[1-7].cc        # Command handlers (movement, inventory, spells, etc.)
│   ├── melee[1-2].cc      # Combat system
│   ├── monster[1-3].cc    # Monster logic and AI
│   ├── object[1-2].cc     # Item/object system
│   ├── spells[1-6].cc     # Magic system
│   ├── generate.cc        # Dungeon generation
│   ├── q_*.cc             # Individual quest implementations
│   ├── main-*.cc          # Frontend implementations
│   ├── z-*.cc             # Low-level utilities (terminal, random, formatting)
│   ├── *_type.hpp         # Type definitions for game entities
│   ├── *_flag.hpp         # Bitflag definitions
│   ├── *_fwd.hpp          # Forward declarations (for circular deps)
│   ├── squelch/           # Item filtering/automation subsystem
│   └── include/tome/      # Modern C++ template utilities
├── lib/                    # Game data and resources
│   ├── edit/              # Text-based game data definitions
│   ├── data/              # Binary cache (generated)
│   ├── help/              # In-game help files
│   ├── pref/              # User preferences
│   ├── save/              # Save files
│   └── mods/              # Modular content
├── tests/                  # Test suite (Bandit BDD)
├── vendor/                 # Third-party dependencies
├── cmake/                  # CMake modules
└── doc/                    # Documentation
```

## Coding Conventions

### Type Aliases (from h-basic.hpp)
```cpp
typedef int errr;           // Error codes: Success=0, Failure=-N, Problem=+N
typedef unsigned char byte;
using s16b = int16_t;       // Signed 16-bit
using u16b = uint16_t;      // Unsigned 16-bit
using s32b = int32_t;       // Signed 32-bit
using u32b = uint32_t;      // Unsigned 32-bit
```

### Naming Conventions
- **Types:** snake_case with `_type` suffix: `player_type`, `object_type`, `cave_type`
- **Functions:** snake_case: `get_player_hp()`, `calculate_ac()`
- **Variables:** snake_case, often short (1-3 chars for loops): `int y, x;`, `byte a;`
- **Constants/Macros:** UPPER_CASE: `ANGBAND_TERM_MAX`, `MAX_HGT`
- **Class members:** snake_case: `py` (player Y), `px` (player X), `au` (gold)

### Common Macros
```cpp
#define ABS(a)    (((a) < 0) ? (-(a)) : (a))
#define A2I(X)    ((X) - 'a')   // char to index
#define I2A(X)    ((X) + 'a')   // index to char
#define KTRL(X)   ((X) & 0x1F)  // control character
#define ESCAPE    '\033'
```

### Code Style
- Use `#pragma once` for header guards
- Forward declarations go in `*_fwd.hpp` files
- Prefer C++17 features: `std::string_view`, `std::optional`, structured bindings
- Use Boost for filesystem operations and optional types
- Comments explain "why" not "what"
- Keep functions focused and modular

### Bitflag Pattern
```cpp
// Flags defined via template class
template<std::size_t Tiers> struct flag_set { ... };

// Usage examples
typedef flag_set<6> object_flag_set;
typedef flag_set<4> monster_spell_flag_set;

// Flag operations
flags |= SOME_FLAG;     // Set flag
flags &= ~SOME_FLAG;    // Clear flag
if (flags & SOME_FLAG)  // Test flag
```

## Testing

### Framework
Tests use **Bandit** (BDD-style) with **CppQuickCheck** for property-based testing.

### Running Tests
```bash
cmake . && make
./src/harness
```

### Test Style
```cpp
#include <bandit/bandit.h>
using namespace bandit;

go_bandit([]() {
    describe("feature_name", []() {
        it("should do something specific", [&] {
            // Setup
            auto input = create_input();
            // Exercise
            auto result = function_under_test(input);
            // Verify
            AssertThat(result, Equals(expected));
        });
    });
});
```

### Test Files
- `tests/harness.cc` - Main test runner
- `tests/flag_set.cc` - Bitflag operations
- `tests/get_level_device.cc` - Equipment level calculations
- `tests/grid.cc` - Grid/cave system
- `tests/lua_get_level.cc` - Legacy Lua integration

## Key Architectural Patterns

### Frontend Abstraction
All UI frontends implement the `Frontend` interface:
```cpp
class Frontend {
    virtual void init() = 0;
    virtual void nuke() = 0;
    virtual void process_event(bool wait) = 0;
    virtual void draw_text(int x, int y, int n, byte a, const char *s) = 0;
    // ... other virtual methods
};
```

### Data-Driven Design
Game content is defined in text files (`lib/edit/*.txt`) and compiled to binary cache at startup:
- `k_info.txt` - Object kinds (weapons, armor, potions, etc.)
- `r_info.txt` - Monster races
- `a_info.txt` - Artifacts
- `d_info.txt` - Dungeon definitions
- `e_info.txt` - Ego items (magical variants)
- `f_info.txt` - Terrain features

### Data File Format
```
# Comment line
N:index:name
G:display_char:color
I:type:subtype:weight
W:level:rarity:cost
F:FLAG1 | FLAG2 | FLAG3
D:Description text here
```

### Global Game State
```cpp
extern Game *game;  // Global singleton with game state
// Contains: player data, dungeon state, monster lists, etc.
```

### Quest System
Individual quests are implemented as separate files (`q_*.cc`) with:
- Quest initialization hooks
- Generation callbacks
- Completion handlers
- Quest-specific data structures

## Important Files for Common Tasks

| Task | Key Files |
|------|-----------|
| Add new monster | `lib/edit/r_info.txt`, `src/monster_race.hpp` |
| Add new item | `lib/edit/k_info.txt`, `src/object_kind.hpp` |
| Modify combat | `src/melee1.cc`, `src/melee2.cc` |
| Change spells | `src/spells[1-6].cc`, `src/spell_type.hpp` |
| Dungeon generation | `src/generate.cc`, `src/cave.cc` |
| Player stats | `src/player_type.hpp`, `src/xtra1.cc` |
| UI/Display | `src/z-term.cc`, `src/main-*.cc` |
| Save/Load | `src/loadsave.cc` |
| Add quest | Create `src/q_newquest.cc`, update `src/CMakeLists.txt` |

## CI/CD

GitHub Actions runs on every push/PR:
- Ubuntu 22.04
- Installs Boost
- Builds all frontends

## Common Gotchas

1. **Working Directory:** When running in-place, the executable must be run from the repository root (where `lib/` is accessible)

2. **Circular Dependencies:** Use forward declaration headers (`*_fwd.hpp`) to break cycles

3. **Save File Compatibility:** `SAVEFILE_VERSION` in `loadsave.cc` must be incremented when changing save format

4. **Data File Parsing:** Changes to `lib/edit/*.txt` format require corresponding updates to `src/init1.cc`

5. **Debug Build:** Uses AddressSanitizer and UBSan - useful for catching memory bugs but slower

6. **Global State:** Many systems use global variables (Angband heritage) - be careful with initialization order

## Compiler Flags Reference

| Build Type | Flags |
|------------|-------|
| Common | `-pipe -Wall -Wextra -Wno-unused-value -Wno-unused-parameter` |
| Debug | `-O0 -g -fsanitize=undefined -fsanitize=address -D_GLIBCXX_DEBUG` |
| Release | `-O2` |
