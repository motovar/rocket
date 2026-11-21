# Vendored third-party code, built as static libraries.

# stb (public domain / MIT): image decoding and Ogg Vorbis decoding
add_library(stb STATIC stb/stb_impl.c)
target_include_directories(stb PUBLIC stb)
if(NOT MSVC)
  target_compile_options(stb PRIVATE -w)
endif()

# IJG libjpeg 6b (libjpeg/README; libjpeg/BYO-POP.md): JPEG decoding, the library the games' own exes decode their
# .jpg images with (PopCap's ImageLib), so pixels match the originals; stb_image rounds some of them differently. Only
# the decoder's sources, unchanged, with jmemnobs.c (no temporary files) and our jconfig.h. Platform::DecodeJPEG
# (platform/JPEGDecoder.cpp) is its one caller. Private to the framework: its headers are not for game code.
add_library(ijgjpeg STATIC
  libjpeg/jcomapi.c libjpeg/jutils.c libjpeg/jerror.c libjpeg/jmemmgr.c libjpeg/jmemnobs.c
  libjpeg/jdapimin.c libjpeg/jdapistd.c libjpeg/jdatasrc.c libjpeg/jdmaster.c libjpeg/jdinput.c libjpeg/jdmarker.c
  libjpeg/jdhuff.c libjpeg/jdphuff.c libjpeg/jdmainct.c libjpeg/jdcoefct.c libjpeg/jdpostct.c libjpeg/jddctmgr.c
  libjpeg/jidctint.c libjpeg/jidctfst.c libjpeg/jidctflt.c libjpeg/jidctred.c libjpeg/jdsample.c libjpeg/jdcolor.c
  libjpeg/jquant1.c libjpeg/jquant2.c libjpeg/jdmerge.c)
target_include_directories(ijgjpeg PUBLIC libjpeg)
if(NOT MSVC)
  target_compile_options(ijgjpeg PRIVATE -w)
endif()

# libxmp-lite 4.7.4 (MIT): MOD/S3M/XM/IT module playback for the game's .s3m music.
# Use the library's own source list so platform shims (snprintf replacements etc.) are included.
include(libxmp-lite/cmake/libxmp-sources.cmake)
list(TRANSFORM LIBXMP_SRC_LIST PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/libxmp-lite/")
add_library(xmp_lite STATIC ${LIBXMP_SRC_LIST})
target_include_directories(xmp_lite PUBLIC libxmp-lite/include/libxmp-lite)
target_include_directories(xmp_lite PRIVATE libxmp-lite/src)
target_compile_definitions(xmp_lite PUBLIC LIBXMP_STATIC)
target_compile_definitions(xmp_lite PRIVATE LIBXMP_CORE_PLAYER _USE_MATH_DEFINES)
if(NOT MSVC)
  target_compile_options(xmp_lite PRIVATE -w)
endif()

# Tremor (Xiph.Org, BSD-style, see tremor/COPYING): the 2002 integer Ogg Vorbis decoder that
# SexyAppFramework 1.2 shipped and decoded sounds with. Platform::Mixer uses it for Ogg files stb_vorbis
# cannot open: Big Money's 2002 sounds use Vorbis floor 0, which stb_vorbis does not support.
file(GLOB TREMOR_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tremor/*.c)
add_library(tremor STATIC ${TREMOR_SOURCES})
target_include_directories(tremor PUBLIC tremor)
if(NOT MSVC)
  # misc.h calls abs() without <stdlib.h> (an error for strict C99 compilers)
  target_compile_options(tremor PRIVATE -w -include stdlib.h)
endif()

set(OPENMPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt)
# minimp3 (CC0, libopenmpt/include/minimp3/LICENSE), which came with libopenmpt, is its own library: libopenmpt decodes
# MP3 samples with it and the framework MP3 sounds (platform/Audio.cpp, Mummy Maze), so both share one copy.
add_library(minimp3 STATIC ${OPENMPT_DIR}/include/minimp3/minimp3.c)
target_include_directories(minimp3 PUBLIC ${OPENMPT_DIR}/include/minimp3)
if(NOT MSVC)
  target_compile_options(minimp3 PRIVATE -w)
endif()
