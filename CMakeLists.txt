# Project definition.
PROJECT (tome2)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1)

# We want a readable feature summary.
INCLUDE(FeatureSummary)

# pkg-config support
INCLUDE(FindPkgConfig)

#
# Basic common compiler flags.
#
SET(COMMON_COMPILER_FLAGS "-pipe -Wall -Wextra -Wno-unused-value -Wno-unused-parameter")

#
# Sanitizer flags for debugging
#
SET(SANITIZER_FLAGS "-fsanitize=undefined -fsanitize=address")

#
# C Compiler Flags
#
SET(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} ${COMMON_COMPILER_FLAGS}")
SET(CMAKE_C_FLAGS_RELEASE   "-O2")
SET(CMAKE_C_FLAGS_DEBUG     "-O0 -g ${SANITIZER_FLAGS}")

#
# C++ Compiler Flags
#
SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${COMMON_COMPILER_FLAGS} --std=c++14")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g ${SANITIZER_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")

#
# GCC Flags
#
IF(CMAKE_COMPILER_IS_GNUCC)
  # Nothing for now
ENDIF()

#
# Clang flags
#
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Nothing for now
ENDIF()

#
# fmt
#
ADD_DEFINITIONS(-DFMT_HEADER_ONLY)

# Add standard math library
SET(LIBS ${LIBS} m)

#
# BOOST
#
FIND_PACKAGE(Boost 1.54.0 COMPONENTS system filesystem)
SET_PACKAGE_PROPERTIES(Boost PROPERTIES TYPE REQUIRED)

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
  # We assume that if we want to compile for SDL that
  # any required dependencies should be installed.
  FIND_PACKAGE(SDL_image)
  FIND_PACKAGE(SDL_ttf)
  SET_PACKAGE_PROPERTIES(SDL_image PROPERTIES TYPE REQUIRED)
  SET_PACKAGE_PROPERTIES(SDL_ttf PROPERTIES TYPE REQUIRED)
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
FEATURE_SUMMARY(FATAL_ON_MISSING_REQUIRED_PACKAGES WHAT PACKAGES_FOUND PACKAGES_NOT_FOUND)

# Add the source subdirectory.
ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (lib)
