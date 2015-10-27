cmake_minimum_required(VERSION 2.8)

project(snowhouse)

option(SNOWHOUSE_BUILD_TESTS    "Build the Snowhouse tests"                 ON)
option(SNOWHOUSE_RUN_TESTS      "Run the Snowhouse tests"                   ON)
option(SNOWHOUSE_IS_CPP11       "Whether to build this as a C++11 project"  OFF)

include_directories("${PROJECT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

set(CMAKE_CXX_FLAGS "-Wfatal-errors -Wall -W -Werror -Wfloat-equal -Wundef -Wendif-labels -Wshadow -pedantic-errors")

if(SNOWHOUSE_IS_CPP11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

  if (CMAKE_HOST_APPLE AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

message(${CMAKE_CXX_FLAGS})

if (SNOWHOUSE_BUILD_TESTS)
    FILE(GLOB SnowhouseSpecSourceFiles example/*.cpp)
    add_executable(snowhouse-tests ${SnowhouseSpecSourceFiles})
endif()

if (SNOWHOUSE_BUILD_TESTS AND SNOWHOUSE_RUN_TESTS)
    add_custom_command(TARGET snowhouse-tests
                       POST_BUILD
                       COMMAND snowhouse-tests
                       WORKING_DIRECTORY ./bin)
elseif (SNOWHOUSE_RUN_TESTS)
    message(WARNING "Unable to run snowhouse tests - set:\n  option(SNOWHOUSE_BUILD_TESTS, \"Build the Snowhouse tests\" ON)\nand clear your CMakeCache.txt")
endif()
