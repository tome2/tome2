INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/bandit)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/fmt)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/pcg-cpp/include)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/jsoncons)

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
  format_ext.cc
  game.cc
  game_edit_data.cc
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
  level_data.cc
  level_marker.cc
  levels.cc
  loadsave.cc
  lua_bind.cc
  melee1.cc
  melee2.cc
  message.cc
  messages.cc
  mimic.cc
  modules.cc
  monster_spell.cc
  monster_type.cc
  monster1.cc
  monster2.cc
  monster3.cc
  notes.cc
  object1.cc
  object2.cc
  object_filter.cc
  object_flag_meta.cc
  options.cc
  player_type.cc
  powers.cc
  program_args.cc
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
  quest.cc
  randart.cc
  range.cc
  skills.cc
  spell_type.cc
  spells1.cc
  spells2.cc
  spells3.cc
  spells4.cc
  spells5.cc
  spells6.cc
  squeltch.cc
  store.cc
  tables.cc
  util.cc
  variable.cc
  wild.cc
  wizard2.cc
  seed.cc
  xtra1.cc
  xtra2.cc
  z-form.c
  z-rand.cc
  z-term.c
  z-util.c
)

# Sources (TEST)
SET(SRCS_TESTS
  ../tests/get_level_device.cc
  ../tests/harness.cc
  ../tests/lua_get_level.cc
  ../tests/flag_set.cc
  ../tests/grid.cc
)

ADD_LIBRARY(game
  ${SRCS_COMMON} ${SRCS_VENDOR}
)

ADD_LIBRARY(game_main
  main.cc)

# Need a few additional source files for Windows.
IF(WIN32)
  SET(SRCS ${SRCS} main-win.c)
  # Resource files require a little workaround.
  IF(MINGW)
    # Workaround for resource compilation for mingw on CMake.
    # See http://www.cmake.org/Bug/view.php?id=4068
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                           -i${CMAKE_CURRENT_SOURCE_DIR}/angband.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o)
    SET(SRCS ${SRCS} ${CMAKE_CURRENT_BINARY_DIR}/angband_rc.o)
  ELSE(MINGW)
    SET(SRCS ${SRCS} angband.rc)
  ENDIF(MINGW)
  # Executable for Win32
  ADD_EXECUTABLE(tome-win WIN32 main-win.c)
  TARGET_LINK_LIBRARIES(tome-win game squelch ${LIBS} winmm wsock32)
  INSTALL(TARGETS tome-win RUNTIME DESTINATION bin)
ENDIF(WIN32)

# tome executables
IF(X11_FOUND)
  INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
  ADD_EXECUTABLE(tome-x11 main-x11.c)
  TARGET_LINK_LIBRARIES(tome-x11 game_main game squelch ${LIBS} ${X11_LIBRARIES})
  INSTALL(TARGETS tome-x11 RUNTIME DESTINATION bin)
ENDIF()

IF(SDL_FOUND AND SDLIMAGE_FOUND AND SDLTTF_FOUND)
  INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR})
  ADD_EXECUTABLE(tome-sdl main-sdl.c)
  TARGET_LINK_LIBRARIES(tome-sdl game_main game squelch ${LIBS} ${SDLIMAGE_LIBRARY} ${SDLTTF_LIBRARY} ${SDL_LIBRARY} m)
  INSTALL(TARGETS tome-sdl RUNTIME DESTINATION bin)
ENDIF()

IF(CURSES_FOUND)
  INCLUDE_DIRECTORIES(${CURSES_INCLUDE_DIR})
  ADD_EXECUTABLE(tome-gcu main-gcu.c)
  TARGET_LINK_LIBRARIES(tome-gcu game_main game squelch ${LIBS} ${CURSES_LIBRARIES})
  INSTALL(TARGETS tome-gcu RUNTIME DESTINATION bin)
ENDIF()

IF(GTK2_FOUND)
  INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})
  ADD_EXECUTABLE(tome-gtk2 main-gtk2.c)
  TARGET_LINK_LIBRARIES(tome-gtk2 game_main game squelch ${LIBS} ${GTK2_LIBRARIES})
  INSTALL(TARGETS tome-gtk2 RUNTIME DESTINATION bin)
ENDIF()

# test harness executable
ADD_EXECUTABLE(harness ${SRCS_TESTS})
TARGET_LINK_LIBRARIES(harness game squelch ${LIBS})
