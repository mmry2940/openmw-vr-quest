set(BULLETMESHTOOL
    main.cpp
)
source_group(apps\\bulletobjecttool FILES ${BULLETMESHTOOL})

openmw_add_executable(openmw-bulletobjecttool ${BULLETMESHTOOL})

if (ANDROID)
    target_sources(openmw-bulletobjecttool PRIVATE
        ../openmw/android_stdio64_compat.cpp
    )
endif()

target_link_libraries(openmw-bulletobjecttool
    Boost::program_options
    components
)

if (ANDROID)
    target_link_libraries(openmw-bulletobjecttool
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

if (BUILD_WITH_CODE_COVERAGE)
    target_compile_options(openmw-bulletobjecttool PRIVATE --coverage)
    target_link_libraries(openmw-bulletobjecttool gcov)
endif()

if (WIN32)
    install(TARGETS openmw-bulletobjecttool RUNTIME DESTINATION ".")
endif()

if (MSVC AND PRECOMPILE_HEADERS_WITH_MSVC)
    target_precompile_headers(openmw-bulletobjecttool PRIVATE
        <string>
        <vector>
    )
endif()
