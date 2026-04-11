set(NAVMESHTOOL_LIB
    worldspacedata.cpp
    navmesh.cpp
)

source_group(apps\\navmeshtool FILES ${NAVMESHTOOL_LIB} main.cpp)

add_library(openmw-navmeshtool-lib STATIC ${NAVMESHTOOL_LIB})

if (ANDROID)
    target_sources(openmw-navmeshtool-lib PRIVATE
        ../openmw/android_stdio64_compat.cpp
    )
endif()

if (ANDROID)
    add_library(openmw-navmeshtool SHARED main.cpp)
else()
    openmw_add_executable(openmw-navmeshtool main.cpp)
endif()

target_link_libraries(openmw-navmeshtool openmw-navmeshtool-lib)

if (ANDROID)
    target_link_libraries(openmw-navmeshtool
        "-Wl,--start-group"
        "${CMAKE_INSTALL_PREFIX}/lib/libboost_system.a"
        "${CMAKE_INSTALL_PREFIX}/lib/libboost_filesystem.a"
        "${CMAKE_INSTALL_PREFIX}/lib/libpcre.a"
        "${CMAKE_INSTALL_PREFIX}/lib/libpcrecpp.a"
        "${CMAKE_INSTALL_PREFIX}/lib/libxml2.a"
        "${CMAKE_INSTALL_PREFIX}/lib/libcollada-dom2.5-dp.a"
        "${CMAKE_BINARY_DIR}/../../../collada-dom-prefix/src/collada-dom-build/dom/external-libs/minizip-1.1/libminizip.a"
        "-Wl,--end-group"
        log
    )
endif()

target_link_libraries(openmw-navmeshtool-lib
    Boost::program_options
    components
)

if (BUILD_WITH_CODE_COVERAGE)
    target_compile_options(openmw-navmeshtool PRIVATE --coverage)
    target_link_libraries(openmw-navmeshtool gcov)
endif()

if (WIN32)
    install(TARGETS openmw-navmeshtool RUNTIME DESTINATION ".")
endif()

if (MSVC AND PRECOMPILE_HEADERS_WITH_MSVC)
    target_precompile_headers(openmw-navmeshtool PRIVATE
        <algorithm>
        <memory>
        <string>
        <vector>
    )
endif()
