# SDL2Config.cmake - minimal config for Android ndk-build SDL2
# Provided by openmw-vr-quest buildscripts for CMake 4.x compatibility

set(SDL2_VERSION "2.0.20")
set(SDL2_VERSION_MAJOR 2)
set(SDL2_VERSION_MINOR 0)
set(SDL2_VERSION_PATCH 20)

get_filename_component(_SDL2_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(SDL2_INCLUDE_DIRS "${_SDL2_PREFIX}/include")
set(SDL2_LIBRARIES "${_SDL2_PREFIX}/lib/libSDL2.so")

if(NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${_SDL2_PREFIX}/lib/libSDL2.so"
        INTERFACE_INCLUDE_DIRECTORIES "${_SDL2_PREFIX}/include"
    )
endif()

if(NOT TARGET SDL2::SDL2main)
    add_library(SDL2::SDL2main INTERFACE IMPORTED)
endif()

set(SDL2_FOUND TRUE)
