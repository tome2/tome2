INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/bandit)

# Add subdirectories
ADD_SUBDIRECTORY (squelch)

# Sources (common)
SET(SRCS_COMMON
  birth.cc
  bldg.cc
  cave.cc
  cmd1.cc
  cmd2.cc
  cmd3.cc
  cmd4.cc
  cmd5.cc
  cmd6.cc
  cmd7.cc
  corrupt.cc
  device_allocation.cc
  dice.cc
  dungeon.cc
  files.cc
  gen_evol.cc
  gen_maze.cc
  generate.cc
  gods.cc
  help.cc
  hiscore.cc
  hooks.cc
  init1.cc
  init2.cc
  joke.cc
  levels.cc
  loadsave.cc
  lua_bind.cc
  melee1.cc
  melee2.cc
  messages.cc
  mimic.cc
  modules.cc
  monster1.cc
  monster2.cc
  monster3.cc
  notes.cc
  object1.cc
  object2.cc
  object_filter.cc
  options.cc
  powers.cc
  q_betwen.cc
  q_bounty.cc
  q_dragons.cc
  q_eol.cc
  q_evil.cc
  q_fireprof.cc
  q_god.cc
  q_god.cc
  q_haunted.cc
  q_hobbit.cc
  q_invas.cc
  q_library.cc
  q_main.cc
  q_narsil.cc
  q_nazgul.cc
  q_nirna.cc
  q_one.cc
  q_poison.cc
  q_rand.cc
  q_shroom.cc
  q_spider.cc
  q_thief.cc
  q_thrain.cc
  q_troll.cc
  q_ultrae.cc
  q_ultrag.cc
  q_wight.cc
  q_wolves.cc
  quark.cc
  quest.cc
  randart.cc
  range.cc
  script.cc
  skills.cc
  spell_type.cc
  spells1.cc
  spells2.cc
  spells3.cc
  spells4.cc
  spells5.cc
  spells6.cc
  squeltch.cc
  status.cc
  store.cc
  tables.cc
  traps.cc
  util.cc
  variable.cc
  wild.cc
  wizard1.cc
  wizard2.cc
  xtra1.cc
  xtra2.cc
  z-form.c
  z-rand.c
  z-term.c
  z-util.c
)

# Sources (PROGRAM)
SET(SRCS_PROGRAM
  main-gcu.c
  main-gtk2.c
  main-sdl.c
  main-x11.c
  main.c
)

# Sources (TEST)
SET(SRCS_TESTS
  ../tests/get_level_device.cc
  ../tests/harness.cc
  ../tests/lua_get_level.cc
)

ADD_LIBRARY(game
  ${SRCS_COMMON}
)

# Need a few additional source files for Windows.
if(WIN32)
  SET(SRCS ${SRCS} main-win.c)
  # Resource files require a little workaround.
  if(MINGW)
    # Workaround for resource compilation for mingw on CMake.
    # See http://www.cmake.org/Bug/view.php?id=4068
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                           -i${CMAKE_CURRENT_SOURCE_DIR}/angband.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o)
    SET(SRCS ${SRCS} ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o)
  else(MINGW)
    SET(SRCS ${SRCS} angband.rc)
  endif(MINGW)
endif(WIN32)

# tome executable
ADD_EXECUTABLE(tome ${EXECUTABLE_OPTIONS} ${SRCS_PROGRAM})
TARGET_LINK_LIBRARIES(tome game squelch ${LIBS})

# test harness executable
ADD_EXECUTABLE(harness ${EXECUTABLE_OPTIONS} ${SRCS_TESTS})
TARGET_LINK_LIBRARIES(harness game squelch ${LIBS})

# Installation
INSTALL(TARGETS tome
  RUNTIME DESTINATION bin
)
