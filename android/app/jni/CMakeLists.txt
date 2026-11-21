# Android build of Rocket Mania Deluxe: one shared library, libmain.so, that SDL's SDLActivity loads and starts (SDL_main).
# It holds the game (games/rocketmania), the SexyAppFramework port (framework/, platform/) and the vendored libraries
# (third_party/). SDL itself is built from source beside it as libSDL2.so; android/fetch-sdl.sh puts SDL's source in ./SDL2.
cmake_minimum_required(VERSION 3.22)
project(rocket C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/SDL2/CMakeLists.txt")
  message(FATAL_ERROR "SDL2 source not found in android/app/jni/SDL2. Run android/fetch-sdl.sh first.")
endif()

# --- SDL: shared library only, no tests -------------------------------------------------------------------------------------
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(SDL2 ${CMAKE_BINARY_DIR}/SDL2-build)

# --- the framework's floating point must round the way the original's did ---------------------------------------------------
# No fused multiply-add (clang's default on arm64 fuses within an expression), so doubles round as they did on the x87/SSE2
# original: the game's sizes and timings are computed from them.
add_compile_options(-ffp-contract=off -ffunction-sections -fdata-sections)

# framework/CMakeLists.txt links ${PORT_SDL}; here that is SDL's own shared library target
set(PORT_SDL SDL2)

add_subdirectory(${REPO_ROOT}/third_party ${CMAKE_BINARY_DIR}/third_party)
add_subdirectory(${REPO_ROOT}/framework ${CMAKE_BINARY_DIR}/framework)

# --- the game -----------------------------------------------------------------------------------------------------------------
file(GLOB ROCKETMANIA_SOURCES CONFIGURE_DEPENDS ${REPO_ROOT}/games/rocketmania/src/*.cpp)
file(GLOB ROCKETMANIA_TESTHOOK_SOURCES CONFIGURE_DEPENDS ${REPO_ROOT}/games/rocketmania/src/testhooks/*.cpp)

# SDLActivity loads "main" last and calls SDL_main in it (main.cpp's main(), renamed by SDL_main.h)
add_library(main SHARED ${ROCKETMANIA_SOURCES} ${ROCKETMANIA_TESTHOOK_SOURCES})
target_include_directories(main PRIVATE ${REPO_ROOT}/games/rocketmania/src)
target_compile_options(main PRIVATE -Wall -Wno-unused-private-field)
target_link_libraries(main PRIVATE sexyframework sexycompat2004 SDL2)
if(ANDROID)
  target_link_libraries(main PRIVATE log android)
endif()
target_link_options(main PRIVATE -Wl,--gc-sections)
