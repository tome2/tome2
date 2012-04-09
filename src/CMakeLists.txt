# Lua support code.
ADD_SUBDIRECTORY(lua)

SET(SRCS
  main-gcu.c main-x11.c main-xaw.c main-sdl.c main-gtk2.c
  z-rand.c z-util.c z-form.c z-virt.c z-term.c
  variable.c tables.c plots.c util.c cave.c dungeon.c 
  melee1.c melee2.c modules.c 
  q_god.c
  object1.c object2.c randart.c squeltch.c traps.c 
  monster1.c monster2.c monster3.c
  xtra1.c xtra2.c skills.c powers.c gods.c
  spells1.c spells2.c corrupt.c mimic.c
  status.c files.c notes.c loadsave.c
  cmd1.c cmd2.c cmd3.c cmd4.c cmd5.c cmd6.c cmd7.c
  help.c
  generate.c gen_maze.c gen_evol.c wild.c levels.c store.c bldg.c
  cmovie.c
  wizard2.c init2.c birth.c wizard1.c init1.c main.c 
  # Lua bits:
  lua_bind.c script.c w_mnster.c w_player.c w_play_c.c w_z_pack.c
  w_obj.c w_util.c w_spells.c w_dun.c
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


# Macro for defining tolua targets.
MACRO(TOLUA_FILE MODULE_NAME OUTPUT_FILE_NAME)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${OUTPUT_FILE_NAME}
    COMMAND tolua -n ${MODULE_NAME} -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.pkg
    DEPENDS tolua ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.pkg
    )
  SET_SOURCE_FILES_PROPERTIES("${OUTPUT_FILE_NAME}" PROPERTIES GENERATED TRUE)
ENDMACRO(TOLUA_FILE)

# Process all the needed modules.
TOLUA_FILE(monster w_mnster.c)
TOLUA_FILE(player w_player.c)
TOLUA_FILE(player_c w_play_c.c)
TOLUA_FILE(z_pack w_z_pack.c)
TOLUA_FILE(object w_obj.c)
TOLUA_FILE(util w_util.c)
TOLUA_FILE(spells w_spells.c)
TOLUA_FILE(dungeon w_dun.c)

# tome executable
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/lua)
ADD_EXECUTABLE(tome ${EXECUTABLE_OPTIONS} ${SRCS})
TARGET_LINK_LIBRARIES(tome lua ${LIBS})

# Installation
INSTALL(TARGETS tome
  RUNTIME DESTINATION bin
)
