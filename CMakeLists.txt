# Project definition.
CMAKE_MINIMUM_REQUIRED (VERSION 3.15...4.0)
PROJECT (tome2)

# We want a readable feature summary.
INCLUDE(FeatureSummary)

# pkg-config support
INCLUDE(FindPkgConfig)

include(CheckIncludeFileCXX)

# Automatically install and find dependencies
INCLUDE(cmake/CPM.cmake)

#
# C++ standard
#
SET(CMAKE_CXX_STANDARD 17)

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
SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${COMMON_COMPILER_FLAGS}")
SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -DJSONCONS_NO_DEPRECATED")
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

CPMAddPackage(NAME fmt
	URL https://github.com/fmtlib/fmt/releases/download/12.0.0/fmt-12.0.0.zip
	URL_HASH SHA256=1c32293203449792bf8e94c7f6699c643887e826f2d66a80869b4f279fb07d25)
SET(LIBS ${LIBS} fmt::fmt)

CPMAddPackage(NAME jsoncons
	URL https://github.com/danielaparker/jsoncons/archive/refs/tags/v1.4.2.tar.gz
	URL_HASH SHA256=e4c012358fdde73036b5f42869db17b373a4f0a95eb99cdd5ba82b37cc71a59b
	OPTIONS "JSONCONS_BUILD_TESTS OFF")
IF(TARGET jsoncons::jsoncons)
	SET(LIBS ${LIBS} jsoncons::jsoncons)
ELSE()
	# Compatibility with older jsoncons
	# https://github.com/danielaparker/jsoncons/commit/84b904cb87bfc6ed5144e014e4f9d1573e6151a2
	SET(LIBS ${LIBS} jsoncons)
ENDIF()

# Add standard math library
SET(LIBS ${LIBS} m)

# PCG
option(USE_SYSTEM_PCG_RANDOM "Use existing pcg_random.hpp found in system")
if(USE_SYSTEM_PCG_RANDOM)
	check_include_file_cxx(pcg_random.hpp PCG_FOUND)
	if (NOT PCG_FOUND)
		MESSAGE(FATAL_ERROR "pcg_random.hpp not found")
	endif()
else()
	CPMAddPackage(NAME pcg
		URL https://github.com/imneme/pcg-cpp/archive/428802d1a5634f96bcd0705fab379ff0113bcf13.tar.gz
		URL_HASH SHA256=79f23706ed0cbc1bb57ea35d50d0abed66898b8f7b0bcf2aaecdfd234863b060)
	INCLUDE_DIRECTORIES(${pcg_SOURCE_DIR}/include)
endif()

#
# BOOST
#
FIND_PACKAGE(Boost 1.54.0 COMPONENTS system filesystem REQUIRED CONFIG)

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
