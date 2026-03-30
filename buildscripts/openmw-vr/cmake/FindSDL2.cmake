# Fallback SDL2 finder for Android toolchain builds where SDL2Config.cmake
# is not installed by the dependency build script.

find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    PATH_SUFFIXES SDL2 include/SDL2 include
)

find_library(SDL2_LIBRARY
    NAMES SDL2 libSDL2
    PATH_SUFFIXES lib
)

# Best-effort version extraction from SDL_version.h.
set(SDL2_VERSION "")
if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" _sdl2_major REGEX "^#define SDL_MAJOR_VERSION[ \t]+[0-9]+")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" _sdl2_minor REGEX "^#define SDL_MINOR_VERSION[ \t]+[0-9]+")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" _sdl2_patch REGEX "^#define SDL_PATCHLEVEL[ \t]+[0-9]+")
    string(REGEX REPLACE ".* ([0-9]+)$" "\\1" _sdl2_major "${_sdl2_major}")
    string(REGEX REPLACE ".* ([0-9]+)$" "\\1" _sdl2_minor "${_sdl2_minor}")
    string(REGEX REPLACE ".* ([0-9]+)$" "\\1" _sdl2_patch "${_sdl2_patch}")
    if(_sdl2_major AND _sdl2_minor AND _sdl2_patch)
        set(SDL2_VERSION "${_sdl2_major}.${_sdl2_minor}.${_sdl2_patch}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARY
)

if(SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 SHARED IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)
