cmake_minimum_required(VERSION 3.20)
# The game (games/rocketmania), the SexyAppFramework port it runs on (framework/, platform/) and the libraries they
# use (third_party/), for Windows. Generated from byo-pop's top-level CMakeLists.txt by its tools/publish.py.
project(byo-pop-rocketmania C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# the exe and the DLLs it loads land in the build folder itself
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# One exe that holds everything but the player's game files: SDL (the small static build of
# third_party/sdl-small, below), the C++ runtime (libc++ and libunwind linked in) and the game. It is built
# without exceptions and unwind tables (nothing in it catches one; an error that would throw ends the
# program), without the functions and data nothing refers to and, in a Release build, without its symbol
# table. The code is compiled and optimised as it is in the trunk; only what is linked, and the tables for
# exceptions, change.
if(MINGW)
  add_compile_options(-ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables
    -fno-asynchronous-unwind-tables)
  add_link_options(-static-libstdc++ -static-libgcc -Wl,--gc-sections $<$<CONFIG:Release>:-s>)
endif()

if(NOT SDL2_DIR AND DEFINED ENV{SDL2_DIR})
  set(SDL2_DIR "$ENV{SDL2_DIR}")
endif()
# SDL is built into the exe: SDL2_DIR is the folder that third_party/sdl-small/build.py installed it in
# (its lib/cmake/SDL2), and the static target is the one linked
find_package(SDL2 REQUIRED CONFIG)
set(PORT_SDL SDL2::SDL2-static SDL2::SDL2main)

add_subdirectory(third_party)

# Shared by every game: the portable SexyAppFramework (framework/) and the platform layer (platform/),
# built together as the sexyframework library (framework/CMakeLists.txt).
add_subdirectory(framework)

add_subdirectory(games/rocketmania)
