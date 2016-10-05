# Project definition.
PROJECT (tome2)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

# We want a readable feature summary.
INCLUDE(FeatureSummary)

# pkg-config support
INCLUDE(FindPkgConfig)

#
# Basic common compiler flags.
#
SET(COMMON_COMPILER_FLAGS "-pipe -Wall -Wextra -Wno-unused-value -Wno-unused-parameter -fsanitize=undefined -fsanitize=address")

#
# GCC/G++ flags
#
IF(CMAKE_COMPILER_IS_GNUCC)
  # Let's set sensible options.
  SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} ${COMMON_COMPILER_FLAGS}")
  SET(CMAKE_C_FLAGS_RELEASE "-O2")
  SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g")
  SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} ${COMMON_COMPILER_FLAGS} --std=c++14 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
ENDIF()

#
# Clang flags
#
IF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} ${COMMON_COMPILER_FLAGS}")
  SET(CMAKE_C_FLAGS_RELEASE "-O2")
  SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g")
ENDIF()
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS       "${CMAKE_CXX_FLAGS} ${COMMON_COMPILER_FLAGS} --std=c++14 -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
ENDIF()

# Add standard math library
SET(LIBS ${LIBS} m)

#
# BOOST
#
FIND_PACKAGE(Boost 1.54.0 REQUIRED COMPONENTS system filesystem)

IF(Boost_FOUND)
  ADD_DEFINITIONS(-DBOOST_FILESYSTEM_NO_DEPRECATED)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  SET(LIBS ${LIBS} ${Boost_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
ENDIF()

#
# X11 support (OPTIONAL)
#
FIND_PACKAGE(X11)

#
# GTK2 support (OPTIONAL)
#
FIND_PACKAGE(GTK2)

#
# SDL support (OPTIONAL)
#
FIND_PACKAGE(SDL)
IF(SDL_FOUND)
  # This is a bit roundabout, but we're working around
  # the FindSDL_* scripts not respecting the REQUIRED
  # flag.
  #
  # the SDL port also requires SDL_image and SDL_ttf
  FIND_PACKAGE(SDL_image)
  FIND_PACKAGE(SDL_ttf)
  IF(SDLIMAGE_FOUND AND SDLTTF_FOUND)
    # Handled in src/CMakeLists.txt
  ELSE()
    # Let user know that (and why) we haven't enabled SDL.
    IF(SDLIMAGE_FOUND)
      MESSAGE(STATUS "Found SDL and SDL_image, but not SDL_ttf!")
    ELSEIF(SDLTTF_FOUND)
      MESSAGE(STATUS "Found SDL and SDL_ttf, but not SDL_image!")
    ELSE()
      MESSAGE(STATUS "Found SDL, but not SDL_image nor SDL_ttf!")
    ENDIF()
    # add info about finding but not enabling SDL
    SET_FEATURE_INFO(SDL "not enabled")
  ENDIF()
ENDIF()

#
# Curses support (OPTIONAL)
#
FIND_PACKAGE(Curses)

#
# Windows support
#
if(WIN32)
  # This definition is required for more than just the main-win file,
  # so we need to have it here.
  ADD_DEFINITIONS(-DWINDOWS)
endif(WIN32)

#
# Set the path for loading the library bits.
#
IF(SYSTEM_INSTALL)
  SET(DEFAULT_PATH "${CMAKE_INSTALL_PREFIX}/lib/tome")
ELSE()
  SET(DEFAULT_PATH "./lib")
ENDIF()
ADD_DEFINITIONS(-DDEFAULT_PATH="${DEFAULT_PATH}")

# Print out a summary of features.
PRINT_ENABLED_FEATURES()

# Add the source subdirectory.
ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (lib)
