find_library(RakNet_LIBRARY_RELEASE
    NAMES RakNetLibStatic
    PATHS
        ${CMAKE_FIND_ROOT_PATH}
        ${OPENMW_DEPENDENCIES_DIR}
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        $ENV{RAKNET_ROOT}/lib
    PATH_SUFFIXES lib lib64
)

find_library(RakNet_LIBRARY_DEBUG
    NAMES RakNetLibStaticd RakNetLibStatic
    PATHS
        ${CMAKE_FIND_ROOT_PATH}
        ${OPENMW_DEPENDENCIES_DIR}
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        $ENV{RAKNET_ROOT}/lib
    PATH_SUFFIXES lib lib64
)

find_path(RakNet_INCLUDES
    NAMES raknet/RakPeer.h
    PATHS
        ${CMAKE_FIND_ROOT_PATH}
        ${OPENMW_DEPENDENCIES_DIR}
        ENV CPATH
        $ENV{RAKNET_ROOT}/include
    PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RakNet
    REQUIRED_VARS RakNet_LIBRARY_RELEASE RakNet_INCLUDES
)

if (RakNet_FOUND)
    set(RakNet_INCLUDES "${RakNet_INCLUDES}/raknet")

    if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        if (RakNet_LIBRARY_DEBUG)
            set(RakNet_LIBRARY optimized ${RakNet_LIBRARY_RELEASE} debug ${RakNet_LIBRARY_DEBUG})
        else()
            set(RakNet_LIBRARY ${RakNet_LIBRARY_RELEASE})
        endif()
    else()
        set(RakNet_LIBRARY ${RakNet_LIBRARY_RELEASE})
    endif()
endif()

mark_as_advanced(
    RakNet_INCLUDES
    RakNet_LIBRARY
    RakNet_LIBRARY_DEBUG
    RakNet_LIBRARY_RELEASE
)
