INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
ADD_SUBDIRECTORY (squelch)

# Sources
SET(SRCS
  main-gcu.c main-x11.c main-xaw.c main-sdl.c main-gtk2.c
  z-rand.c z-util.c z-form.c z-virt.c z-term.c
  variable.cc tables.cc hooks.cc util.cc cave.cc dungeon.cc
  melee1.cc melee2.cc messages.cc modules.cc
  q_god.cc q_library.cc q_fireprof.cc q_bounty.cc q_thrain.cc
  q_narsil.cc q_evil.cc q_betwen.cc q_haunted.cc q_invas.cc
  q_nirna.cc q_eol.cc q_god.cc q_dragons.cc q_poison.cc
  q_spider.cc q_wolves.cc q_shroom.cc q_nazgul.cc q_wight.cc
  q_troll.cc q_hobbit.cc q_thief.cc q_ultrae.cc q_ultrag.cc
  q_one.cc q_main.cc q_rand.cc
  object1.cc object2.cc randart.cc squeltch.cc traps.cc
  monster1.cc monster2.cc monster3.cc
  xtra1.cc xtra2.cc skills.cc powers.cc gods.cc
  spells1.cc spells2.cc spells3.cc spells4.cc spells5.cc spells6.cc
  spell_type.cc device_allocation.cc
  corrupt.cc joke.cc mimic.cc
  status.cc files.cc notes.cc loadsave.cc string_list.cc
  cmd1.cc cmd2.cc cmd3.cc cmd4.cc cmd5.cc cmd6.cc cmd7.cc
  help.cc hiscore.cc range.cc dice.cc
  generate.cc gen_maze.cc gen_evol.cc wild.cc levels.cc store.cc bldg.cc
  cmovie.cc
  wizard1.cc wizard2.cc init1.cc init2.cc birth.cc main.c
  quark.cc
  # Lua bits:
  lua_bind.cc script.cc
)

# Need a few additional source files for Windows.
if(WIN32)
  SET(SRCS ${SRCS} main-win.c readdib.c)
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
ADD_EXECUTABLE(tome ${EXECUTABLE_OPTIONS} ${SRCS})
TARGET_LINK_LIBRARIES(tome squelch ${LIBS})

# Installation
INSTALL(TARGETS tome
  RUNTIME DESTINATION bin
)
