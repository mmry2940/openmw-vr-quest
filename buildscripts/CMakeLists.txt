cmake_minimum_required(VERSION 3.5.0)

# https://github.com/libjpeg-turbo/libjpeg-turbo/releases
set(LIBJPEG_TURBO_VERSION 1.5.3)
set(LIBJPEG_TURBO_HASH SHA256=b24890e2bb46e12e72a79f7e965f409f4e16466d00e1dd15d93d73ee6b592523)

# http://www.libpng.org/pub/png/libpng.html
set(LIBPNG_VERSION 1.6.37)
set(LIBPNG_HASH SHA256=505e70834d35383537b6491e7ae8641f1a4bed1876dbfe361201fc80868d88ca)

# https://www.freetype.org/index.html#news
set(FREETYPE2_VERSION 2.10.4)
set(FREETYPE2_HASH SHA256=5eab795ebb23ac77001cfb68b7d4d50b5d6c7469247b0b01b2c953269f658dac)

# https://github.com/kcat/openal-soft/releases
set(OPENAL_VERSION 1.21.1)
set(OPENAL_HASH SHA256=8ac17e4e3b32c1af3d5508acfffb838640669b4274606b7892aa796ca9d7467f)

# http://www.boost.org/users/download/
set(BOOST_VERSION 1.79.0)
set(BOOST_HASH SHA256=475d589d51a7f8b3ba2ba4eda022b170e562ca3b760ee922c146b6c65856ef39)

# https://www.ffmpeg.org/download.html
set(FFMPEG_VERSION 4.4)
set(FFMPEG_HASH SHA256=42093549751b582cf0f338a21a3664f52e0a9fbe0d238d3c992005e493607d0e)

# https://www.libsdl.org/download-2.0.php
set(SDL2_VERSION 2.0.12)
set(SDL2_HASH SHA256=349268f695c02efbc9b9148a70b85e58cefbbf704abd3e91be654db7f1e2c863)

# https://github.com/bulletphysics/bullet3/releases
set(BULLET_VERSION 3.17)
set(BULLET_HASH SHA256=baa642c906576d4d98d041d0acb80d85dd6eff6e3c16a009b1abf1ccd2bc0a61)

# https://github.com/MyGUI/mygui/releases
set(MYGUI_VERSION 3.4.3)
set(MYGUI_HASH SHA256=33c91b531993047e77cace36d6fea73634b8c17bd0ed193d4cd12ac7c6328abd)

# https://github.com/ptitSeb/gl4es/releases
set(GL4ES_VERSION v1.1.4)
set(GL4ES_HASH SHA256=b565e717c7d192e936bda25f3cb90ad8db398af56414ec08294b6716574c1a6d)

# https://github.com/openscenegraph/OpenSceneGraph/releases
set(OSG_VERSION 3.6.5)
set(OSG_HASH SHA256=aea196550f02974d6d09291c5d83b51ca6a03b3767e234a8c0e21322927d1e12)

# https://www.sqlite.org/download.html
set(SQLITE_VERSION 3390400)
set(SQLITE_HASH SHA256=f31d445b48e67e284cf206717cc170ab63cbe4fd7f79a82793b772285e78fdbb)

# https://luajit.org/download.html
set(LUAJIT_VERSION 2.1.0-beta3)
set(LUAJIT_HASH SHA256=409f7fe570d3c16558e594421c47bdd130238323c9d6fd6c83dedd2aaeb082a8)

# https://github.com/fmtlib/fmt/releases
set(FMT_VERSION 8.1.1)
set(FMT_HASH SHA256=3d794d3cf67633b34b2771eb9f073bde87e846e0d395d254df7b211ef1ec7346)

# https://github.com/lz4/lz4/releases
set(LZ4_VERSION 1.9.3)
set(LZ4_HASH SHA256=030644df4611007ff7dc962d981f390361e6c97a34e5cbc393ddfbe019ffe2c1)

# https://download.gnome.org/sources/libxml2/
set(LIBXML2_VERSION 2.9.14)
set(LIBXML2_HASH SHA256=60d74a257d1ccec0475e749cba2f21559e48139efba6ff28224357c7c798dfee)

# https://www.pcre.org/
set(PCRE_VERSION 8.45)
set(PCRE_HASH SHA256=4dae6fdcd2bb0bb6c37b5f97c33c2be954da743985369cddac3546e3218bffb8)

# https://github.com/rdiankov/collada-dom
set(COLLADA_DOM_VERSION 2.5.0)
set(COLLADA_DOM_HASH SHA256=3be672407a7aef60b64ce4b39704b32816b0b28f61ebffd4fbd02c8012901e0d)

# https://github.com/OpenMW/openmw/commits/master
set(OPENMW_VERSION f27d7c66ff61502a70b2f93bde6d8942c16ec1c3)
set(OPENMW_HASH SHA256=f07e97bf49c4a9710a5d7b967792e9addc0b4c054ad24bbcdea1a1e5a1daf5e0)

# https://github.com/TES3MP/CrabNet/commits/main
set(CRABNET_VERSION 19e66190e83f53bcdcbcd6513238ed2e54878a21)

# End of configurable options

project(openmw-android)

option(BUILD_OPENMW_MP "Build TES3MP/OpenMW-MP client support" OFF)

include(ExternalProject)

set(wrapper_command ${CMAKE_BINARY_DIR}/command_wrapper.sh)
set(ndk_triplet ${NDK_TRIPLET})
set(download_dir ${CMAKE_SOURCE_DIR}/downloads/)
set(prefix ${CMAKE_INSTALL_PREFIX})
set(host ${ndk_triplet})
set(android_platform android-${ANDROID_API})
set(app_abi ${ABI})

set(COMMON_CMAKE_ARGS
	-DCMAKE_POLICY_VERSION_MINIMUM=3.5
	-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/toolchain/ndk/build/cmake/android.toolchain.cmake
	-DANDROID_ABI=${app_abi}
	-DANDROID_PLATFORM=${android_platform}
	-DANDROID_STL=c++_shared
	-DANDROID_CPP_FEATURES=rtti\ exceptions
	-DCMAKE_C_FLAGS=-DMYGUI_DONT_REPLACE_NULLPTR\ -I${prefix}/include/\ $ENV{CFLAGS}
	-DCMAKE_CXX_FLAGS=-DMYGUI_DONT_REPLACE_NULLPTR\ -I${prefix}/include/\ $ENV{CXXFLAGS}
	-DCMAKE_SHARED_LINKER_FLAGS=$ENV{LDFLAGS}
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE}
	-DCMAKE_DEBUG_POSTFIX=
	-DCMAKE_INSTALL_PREFIX=${prefix}
	-DCMAKE_FIND_ROOT_PATH=${prefix}
)

set(COMMON_NDK_BUILD_FLAGS
	NDK_PROJECT_PATH=.
	APP_BUILD_SCRIPT=<SOURCE_DIR>/Android.mk
	APP_PLATFORM=${android_platform}
	APP_ABI=${app_abi}
	LOCAL_LDFLAGS=$ENV{LDFLAGS}
	-j4
)

set(COMMON_AUTOCONF_FLAGS
	--enable-static
	--disable-shared
	--prefix=${prefix}
)

# ffmpeg doesn't like --host, so sandwich it between adding --host to the flags
set(ffmpeg_configure_flags ${COMMON_AUTOCONF_FLAGS})
if (${BUILD_TYPE} STREQUAL "debug")
	# ffmpeg fails to compile if we have asm enabled and optimizations disabled, so disable both
	set(ffmpeg_configure_flags ${ffmpeg_configure_flags} --disable-asm --disable-optimizations)
endif()

if ((${ARCH} STREQUAL "x86_64") OR (${ARCH} STREQUAL "x86"))
	# ffmpeg fails to link with assembly optimizations on x86-64
	set(ffmpeg_configure_flags ${ffmpeg_configure_flags} --disable-asm)
endif()

if (${ARCH} STREQUAL "arm64")
	# Some arm64 asm objects can produce relocations incompatible with linking into libopenmw.so.
	set(ffmpeg_configure_flags ${ffmpeg_configure_flags} --disable-asm)
endif()

set(COMMON_AUTOCONF_FLAGS
	--host=${host}
	${COMMON_AUTOCONF_FLAGS}
)

if (${ARCH} STREQUAL "arm")
	set(ffmpeg_configure_flags ${ffmpeg_configure_flags} --enable-thumb)
endif()

# on x86 enabling simd results in text relocations, which are banned on Android
set(libjpeg_turbo_flags "")
if (${ARCH} STREQUAL "x86")
	set(libjpeg_turbo_flags ${libjpeg_turbo_flags} --without-simd)
endif()

ExternalProject_Add(libjpeg-turbo
	URL https://sourceforge.net/projects/libjpeg-turbo/files/${LIBJPEG_TURBO_VERSION}/libjpeg-turbo-${LIBJPEG_TURBO_VERSION}.tar.gz/download
	https://github.com/xyzz/openmw-deps/releases/download/v0/libjpeg-turbo-${LIBJPEG_TURBO_VERSION}.tar.gz
	URL_HASH ${LIBJPEG_TURBO_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${COMMON_AUTOCONF_FLAGS}
	${libjpeg_turbo_flags}

	BUILD_COMMAND ${wrapper_command} $(MAKE) PROGRAMS=

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install-libLTLIBRARIES install-data-am
)

ExternalProject_Add(libpng
	URL http://prdownloads.sourceforge.net/libpng/libpng-${LIBPNG_VERSION}.tar.xz
	https://github.com/xyzz/openmw-deps/releases/download/v0/libpng-${LIBPNG_VERSION}.tar.xz
	URL_HASH ${LIBPNG_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${COMMON_AUTOCONF_FLAGS}

	BUILD_COMMAND ${wrapper_command} $(MAKE) check_PROGRAMS= bin_PROGRAMS=

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install check_PROGRAMS= bin_PROGRAMS=
)

ExternalProject_Add(freetype2
	URL https://download.savannah.gnu.org/releases/freetype/freetype-${FREETYPE2_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/freetype-${FREETYPE2_VERSION}.tar.gz
	URL_HASH ${FREETYPE2_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${COMMON_AUTOCONF_FLAGS}
	--with-png=no

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(openal
	URL https://github.com/kcat/openal-soft/archive/${OPENAL_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/openal-soft-${OPENAL_VERSION}.tar.gz
	URL_HASH ${OPENAL_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
	${COMMON_CMAKE_ARGS}
	-DALSOFT_EXAMPLES=OFF
	-DALSOFT_TESTS=OFF
	-DALSOFT_UTILS=OFF
	-DALSOFT_NO_CONFIG_UTIL=ON
	-DALSOFT_BACKEND_OPENSL=ON
	-DALSOFT_BACKEND_WAVE=OFF

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

string(REPLACE "." "_" BOOST_VERSION_UNDERSCORE ${BOOST_VERSION})
# Generate a user-config.jam pointing the clang toolset at the NDK cross-compiler.
# "toolset=clang-android" is broken in Boost 1.70+ because b2 parses "android"
# as a numeric version tag and rejects it.  An explicit user-config with no
# version tag avoids that problem.
file(WRITE ${CMAKE_BINARY_DIR}/boost-user-config.jam
"using clang : : ${host}-clang++ ;\n")

ExternalProject_Add(boost
	URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_UNDERSCORE}.tar.bz2
	https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/${BOOST_VERSION_UNDERSCORE}.tar.bz2
	URL_HASH ${BOOST_HASH}
	DOWNLOAD_DIR ${download_dir}

	BUILD_IN_SOURCE 1

	# Bootstrap with native compiler so b2 itself builds for the host.
	# The NDK env-vars (CC/CXX/...) are cleared since bootstrap.sh must use gcc.
	CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
		--unset=CC --unset=CXX --unset=AR --unset=RANLIB --unset=STRIP
		--unset=CFLAGS --unset=CXXFLAGS --unset=LDFLAGS
		<SOURCE_DIR>/bootstrap.sh
		--with-toolset=gcc
		--prefix=${prefix}

	BUILD_COMMAND ""

	# wrapper_command puts NDK bin dir on PATH so ${host}-clang++ is found.
	# toolset=clang matches the "using clang : :" entry in user-config.
	INSTALL_COMMAND ${wrapper_command} ./b2
	-j4
	--with-filesystem
	--with-program_options
	--with-system
	--with-iostreams
	--ignore-site-config
	--user-config=${CMAKE_BINARY_DIR}/boost-user-config.jam
	toolset=clang
	architecture=${BOOST_ARCH}
	address-model=${BOOST_ADDRESS_MODEL}
	cflags=$ENV{CFLAGS}
	cxxflags=$ENV{CXXFLAGS}
	variant=release
	target-os=android
	threading=multi
	threadapi=pthread
	link=static
	runtime-link=static
	install
	COMMAND ${host}-ranlib ${prefix}/lib/libboost_filesystem.a
	COMMAND ${host}-ranlib ${prefix}/lib/libboost_program_options.a
	COMMAND ${host}-ranlib ${prefix}/lib/libboost_system.a
	COMMAND ${host}-ranlib ${prefix}/lib/libboost_iostreams.a
)

ExternalProject_Add(ffmpeg
	URL http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2
	https://github.com/xyzz/openmw-deps/releases/download/v0/ffmpeg-${FFMPEG_VERSION}.tar.bz2
	URL_HASH ${FFMPEG_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${ffmpeg_configure_flags}
	--target-os=android
	--enable-cross-compile
	--cross-prefix=${host}-
	--cc=${host}-clang
	--arch=${ARCH}
	--cpu=${FFMPEG_CPU}
	--enable-version3
	--enable-pic
	--disable-everything
	--disable-doc
	--disable-programs
	--disable-autodetect
	--disable-iconv
	--enable-decoder=mp3
	--enable-demuxer=mp3
	--enable-decoder=bink
	--enable-decoder=binkaudio_rdft
	--enable-decoder=binkaudio_dct
	--enable-demuxer=bink
	--enable-demuxer=wav
	--enable-decoder=pcm_*
	--enable-decoder=vp8
	--enable-decoder=vp9
	--enable-decoder=opus
	--enable-decoder=vorbis
	--enable-demuxer=matroska
	--enable-demuxer=ogg

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(sdl2
	URL https://www.libsdl.org/release/SDL2-${SDL2_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/SDL2-${SDL2_VERSION}.tar.gz
	URL_HASH ${SDL2_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/sdl2-android-120hz-gyroscope.patch
		COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/sdl2-egl-bad-display.patch

	CONFIGURE_COMMAND ""

	BUILD_COMMAND ${wrapper_command} ndk-build
	${COMMON_NDK_BUILD_FLAGS}

	INSTALL_COMMAND mkdir -p ${prefix}/lib/
	COMMAND cp libs/${app_abi}/libSDL2.so ${prefix}/lib/
	COMMAND cp libs/${app_abi}/libhidapi.so ${prefix}/lib/
	COMMAND cp -r <SOURCE_DIR>/include ${prefix}/
	COMMAND mkdir -p ${prefix}/lib/cmake/SDL2
	COMMAND cp ${CMAKE_SOURCE_DIR}/patches/sdl2/SDL2Config.cmake ${prefix}/lib/cmake/SDL2/SDL2Config.cmake
	COMMAND cp ${CMAKE_SOURCE_DIR}/patches/sdl2/SDL2ConfigVersion.cmake ${prefix}/lib/cmake/SDL2/SDL2ConfigVersion.cmake
)

ExternalProject_Add(bullet
	URL https://github.com/bulletphysics/bullet3/archive/${BULLET_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/bullet3-${BULLET_VERSION}.tar.gz
	URL_HASH ${BULLET_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/bullet-fix-cmake.patch

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
	${COMMON_CMAKE_ARGS}
	-DBUILD_BULLET2_DEMOS=OFF
	-DBUILD_CPU_DEMOS=OFF
	-DBUILD_UNIT_TESTS=OFF
	-DBUILD_EXTRAS=OFF
	-DUSE_DOUBLE_PRECISION=ON

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND $(MAKE) install
)

ExternalProject_Add(gl4es
	URL https://github.com/ptitSeb/gl4es/archive/${GL4ES_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/gl4es-${GL4ES_VERSION}.gz
	URL_HASH ${GL4ES_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/gl4es/shared-library.patch
	COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/gl4es/gamma.patch
	COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/gl4es/keyword-usage.patch

	CONFIGURE_COMMAND ""

	BUILD_COMMAND ${wrapper_command} ndk-build
	${COMMON_NDK_BUILD_FLAGS}

	INSTALL_COMMAND mkdir -p ${prefix}/lib/
	COMMAND cp libs/${app_abi}/libGL.so ${prefix}/lib/
	COMMAND cp -r <SOURCE_DIR>/include ${prefix}/
)

ExternalProject_Add(mygui
	DEPENDS freetype2

	URL https://github.com/MyGUI/mygui/archive/MyGUI${MYGUI_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/MyGUI${MYGUI_VERSION}.tar.gz
	URL_HASH ${MYGUI_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/mygui-dont-set-debug-postfix.patch

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
	${COMMON_CMAKE_ARGS}
	-DMYGUI_RENDERSYSTEM=8
	-DMYGUI_BUILD_DEMOS=OFF
	-DMYGUI_BUILD_TOOLS=OFF
	-DMYGUI_BUILD_PLUGINS=OFF
	-DFREETYPE_FT2BUILD_INCLUDE_DIR=${prefix}/include/freetype2/

	-DMYGUI_STATIC=ON

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(lz4
	URL https://github.com/lz4/lz4/archive/v${LZ4_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/v${LZ4_VERSION}.tar.gz
	URL_HASH ${LZ4_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>/build/cmake/
	${COMMON_CMAKE_ARGS}
	-DBUILD_STATIC_LIBS=ON
	-DBUILD_SHARED_LIBS=OFF

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
	)

ExternalProject_Add(libxml2
	URL https://download.gnome.org/sources/libxml2/2.9/libxml2-${LIBXML2_VERSION}.tar.xz
	URL_HASH ${LIBXML2_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${COMMON_AUTOCONF_FLAGS}
	--without-python
	--without-http
	--without-ftp
	--without-html
	--without-iconv
	--without-readline

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(pcre
	URL https://downloads.sourceforge.net/pcre/pcre-${PCRE_VERSION}.tar.bz2
	URL_HASH ${PCRE_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
	${COMMON_AUTOCONF_FLAGS}
	--enable-pcrecpp
	--enable-unicode-properties
	--enable-utf8
	--disable-jit

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(collada-dom
	DEPENDS libxml2 pcre boost

	URL https://github.com/rdiankov/collada-dom/archive/v${COLLADA_DOM_VERSION}.tar.gz
	URL_HASH ${COLLADA_DOM_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND patch -d <SOURCE_DIR> -p0 -t -N < ${CMAKE_SOURCE_DIR}/patches/collada-dom/static.patch

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
	${COMMON_CMAKE_ARGS}
	-DOPT_COMPILE_TESTS=OFF
	-DOPT_BUILD_VIEWER=OFF
	-DOPT_BUILD_PACKAGES=OFF
	-DOPT_COMPILE_FX=OFF
	-DOPT_COMPILE_RT=OFF
	-DOPT_COLLADA15=OFF
	-DBOOST_ROOT=${prefix}
	-DBoost_USE_STATIC_LIBS=ON
	-DBoost_USE_STATIC_RUNTIME=ON
	-DBoost_NO_SYSTEM_PATHS=ON

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

set(OSG_COMMON
	-DOPENGL_PROFILE="GL1"
	-DDYNAMIC_OPENTHREADS=OFF
	-DDYNAMIC_OPENSCENEGRAPH=OFF
	-DBUILD_OSG_PLUGIN_OSG=ON
	-DBUILD_OSG_PLUGIN_DDS=ON
	-DBUILD_OSG_PLUGIN_TGA=ON
	-DBUILD_OSG_PLUGIN_BMP=ON
	-DBUILD_OSG_PLUGIN_JPEG=ON
	-DBUILD_OSG_PLUGIN_PNG=ON
	-DBUILD_OSG_PLUGIN_FREETYPE=ON
	-DBUILD_OSG_PLUGIN_DAE=ON
	-DCOLLADA_FOUND=YES
	-DCOLLADA_USE_STATIC=ON
	-DCOLLADA_DOM_2_4_OR_LATER=TRUE
	-DCOLLADA_INCLUDE_DIR=${prefix}/include/collada-dom2.5
	-DCOLLADA_STATIC_LIBRARY=${prefix}/lib/libcollada-dom2.5-dp.a
	-DCOLLADA_LIBXML_LIBRARY=${prefix}/lib/libxml2.a
	-DCOLLADA_PCRECPP_LIBRARY=${prefix}/lib/libpcrecpp.a
	-DCOLLADA_PCRE_LIBRARY=${prefix}/lib/libpcre.a
	-DCOLLADA_BOOST_FILESYSTEM_LIBRARY=${prefix}/lib/libboost_filesystem.a
	-DCOLLADA_BOOST_SYSTEM_LIBRARY=${prefix}/lib/libboost_system.a
	-DJPEG_INCLUDE_DIR=${prefix}/include/
	-DPNG_INCLUDE_DIR=${prefix}/include/
	-DFREETYPE_DIR=${prefix}
	-DOSG_CPP_EXCEPTIONS_AVAILABLE=TRUE
	-DOSG_GL1_AVAILABLE=ON
	-DOSG_GL2_AVAILABLE=OFF
	-DOSG_GL3_AVAILABLE=OFF
	-DOSG_GLES1_AVAILABLE=OFF
	-DOSG_GLES2_AVAILABLE=OFF
	-DOSG_GL_LIBRARY_STATIC=OFF
	-DOSG_GL_DISPLAYLISTS_AVAILABLE=OFF
	-DOSG_GL_MATRICES_AVAILABLE=ON
	-DOSG_GL_VERTEX_FUNCS_AVAILABLE=ON
	-DOSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE=ON
	-DOSG_GL_FIXED_FUNCTION_AVAILABLE=ON
	-DBUILD_OSG_APPLICATIONS=OFF
	-DBUILD_OSG_PLUGINS_BY_DEFAULT=OFF
	-DBUILD_OSG_DEPRECATED_SERIALIZERS=OFF
)

set(OSG_PATCH
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/disable-polygon-offset.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/remove-lib-prefix-from-plugins.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/fix-freetype-include-dirs.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/0001-Replace-Atomic-impl-with-std-atomic.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/0002-BufferObject-make-numClients-atomic.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/0004-IncrementalCompileOperation-wrap-some-stuff-in-atomi.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/osg/0005-CullSettings-make-inheritanceMask-atomic-to-silence-.patch
)

ExternalProject_Add(osg
	DEPENDS libjpeg-turbo libpng gl4es freetype2 collada-dom

	URL https://github.com/openscenegraph/OpenSceneGraph/archive/OpenSceneGraph-${OSG_VERSION}.tar.gz
	https://github.com/xyzz/openmw-deps/releases/download/v0/OpenSceneGraph-${OSG_VERSION}.tar.gz
	URL_HASH ${OSG_HASH}
	DOWNLOAD_DIR ${download_dir}

	PATCH_COMMAND ${OSG_PATCH}

	CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
	${COMMON_CMAKE_ARGS}
	${OSG_COMMON}

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND mkdir -p <SOURCE_DIR>/3rdparty/build/libpng <SOURCE_DIR>/3rdparty/build/libjpeg <SOURCE_DIR>/3rdparty/build/freetype
	COMMAND $(MAKE) install
)

ExternalProject_Add(sqlite3
	URL https://www.sqlite.org/2022/sqlite-autoconf-${SQLITE_VERSION}.tar.gz
	URL_HASH ${SQLITE_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ${wrapper_command} <SOURCE_DIR>/configure
		${COMMON_AUTOCONF_FLAGS}
		--disable-readline
		--disable-dynamic-extensions

	BUILD_COMMAND ${wrapper_command} $(MAKE)

	INSTALL_COMMAND ${wrapper_command} $(MAKE) install
)

ExternalProject_Add(luajit
	URL https://github.com/LuaJIT/LuaJIT/archive/refs/tags/v${LUAJIT_VERSION}.tar.gz
	URL_HASH ${LUAJIT_HASH}
	DOWNLOAD_DIR ${download_dir}

	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ""

	BUILD_COMMAND ${wrapper_command} make -j4
		PREFIX=${prefix}
		CROSS=${host}-
		HOST_CC=gcc
		CC=clang
		TARGET_SYS=Linux
		BUILDMODE=static

	INSTALL_COMMAND ${wrapper_command} make install
		PREFIX=${prefix}
		CROSS=${host}-
		HOST_CC=gcc
		CC=clang
		TARGET_SYS=Linux
		BUILDMODE=static
)

ExternalProject_Add(fmt
	URL https://github.com/fmtlib/fmt/archive/refs/tags/${FMT_VERSION}.tar.gz
	URL_HASH ${FMT_HASH}
	DOWNLOAD_DIR ${download_dir}

	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND mkdir -p ${prefix}/include
	COMMAND cp -r <SOURCE_DIR>/include/fmt ${prefix}/include/
<<<<<<< HEAD
	COMMAND cp ${CMAKE_SOURCE_DIR}/patches/fmt/format.h ${prefix}/include/format
)

if (BUILD_OPENMW_MP)
	ExternalProject_Add(crabnet
		GIT_REPOSITORY https://github.com/TES3MP/CrabNet.git
		GIT_TAG ${CRABNET_VERSION}
		GIT_SHALLOW TRUE
		UPDATE_DISCONNECTED TRUE

		PATCH_COMMAND patch -p1 -N --no-backup-if-mismatch < ${CMAKE_SOURCE_DIR}/patches/crabnet-android-no-asm-io.patch || true

		CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
		${COMMON_CMAKE_ARGS}
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON
		-DCRABNET_ENABLE_DLL=OFF
		-DCRABNET_ENABLE_SAMPLES=OFF
		-DCRABNET_ENABLE_STATIC=ON

		BUILD_COMMAND ${wrapper_command} $(MAKE)

		INSTALL_COMMAND ${wrapper_command} $(MAKE) install
	)
endif ()

=======
)

>>>>>>> 3ecc687e950b13580a4e709d17a2dd7170894a4e
# Build ICU 70.1 for the host (native Linux) so the cross-compile ICU build
# inside OpenMW's extern/ can use the host tools (genrb, icupkg, etc.).
# We explicitly unset NDK env vars (CC, CXX, AR, etc.) so autotools picks up
# the system compiler instead of the Android cross-compiler.
ExternalProject_Add(icu_host
	URL https://github.com/unicode-org/icu/archive/refs/tags/release-70-1.zip
	URL_HASH MD5=49d5e2e5bab93ae1a4b56e916150544c
	DOWNLOAD_DIR ${download_dir}
	SOURCE_DIR ${CMAKE_BINARY_DIR}/icu-host-src
	BINARY_DIR ${CMAKE_BINARY_DIR}/icu-host-build
	CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env
		--unset=CC --unset=CXX --unset=AR --unset=RANLIB --unset=STRIP
		--unset=CFLAGS --unset=CXXFLAGS --unset=LDFLAGS
		CC=/usr/bin/gcc CXX=/usr/bin/g++
		<SOURCE_DIR>/icu4c/source/configure
		--enable-static --disable-shared
		--disable-tests --disable-samples --disable-icuio --disable-extras
	BUILD_COMMAND ${CMAKE_COMMAND} -E env
		--unset=CC --unset=CXX --unset=AR --unset=RANLIB --unset=STRIP
		--unset=CFLAGS --unset=CXXFLAGS --unset=LDFLAGS
		CC=/usr/bin/gcc CXX=/usr/bin/g++
		$(MAKE) -j4
	INSTALL_COMMAND ""
)

set(OPENMW_COMMON
	-DBUILD_BSATOOL=0
	-DBUILD_NIFTEST=0
	-DBUILD_ESMTOOL=0
	-DBUILD_LAUNCHER=0
	-DBUILD_MWINIIMPORTER=0
	-DBUILD_ESSIMPORTER=0
	-DBUILD_OPENCS=0
	-DBUILD_WIZARD=0
	-DBUILD_MYGUI_PLUGIN=0
	-DOPENAL_INCLUDE_DIR=${prefix}/include/AL/
	-DBullet_INCLUDE_DIR=${prefix}/include/bullet/
	-DOPENGL_ES=OFF
	-DOSG_STATIC=TRUE
	-DMyGUI_LIBRARY=${prefix}/lib/libMyGUIEngineStatic.a
	-DOPENMW_USE_SYSTEM_ICU=OFF
	-DOPENMW_ICU_HOST_BUILD_DIR=${CMAKE_BINARY_DIR}/icu-host-build
	-DOPENMW_USE_SYSTEM_YAML_CPP=OFF
	-DBoost_USE_STATIC_LIBS=ON
	-DBoost_USE_STATIC_RUNTIME=ON
	-DLuaJit_INCLUDE_DIR=${prefix}/include/luajit-2.1
	-DLuaJit_LIBRARY=${prefix}/lib/libluajit-5.1.a
	-DSQLite3_INCLUDE_DIR=${prefix}/include
	-DSQLite3_LIBRARY=${prefix}/lib/libsqlite3.a
)

set(OPENMW_DEPENDENCIES
	boost
	openal
	osg
	mygui
	ffmpeg
	sdl2
	bullet
	lz4
	icu_host
	luajit
	sqlite3
	fmt
)

if (BUILD_OPENMW_MP)
	list(APPEND OPENMW_DEPENDENCIES crabnet)
	list(APPEND OPENMW_COMMON -DBUILD_OPENMW_MP=ON)
endif ()

set(OPENMW_PATCH
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/gamma.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/0001-loadingscreen-disable-for-now.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/0009-windowmanagerimp-always-show-mouse-when-possible-pat.patch &&
    patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/0010-android-fix-context-being-lost-on-app-minimize.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/0014-settingswindow-save-user-settings-file-when-ok-is-pr.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/0012-components-misc-stringops-use-boost-format-instead-o.patch &&
	patch -d <SOURCE_DIR> -p1 -t -N < ${CMAKE_SOURCE_DIR}/patches/openmw/fix-build.patch &&
	cp ${CMAKE_SOURCE_DIR}/patches/openmw/android_main.cpp <SOURCE_DIR>/apps/openmw/android_main.cpp
)

option(USE_LOCAL_OPENMW_VR "Use local buildscripts/openmw-vr source for OpenMW build" ON)
option(ALLOW_OPENMW_UPSTREAM_FALLBACK "Allow fallback to upstream non-VR OpenMW source" OFF)
set(OPENMW_LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/openmw-vr)

if(USE_LOCAL_OPENMW_VR AND EXISTS ${OPENMW_LOCAL_SOURCE_DIR}/CMakeLists.txt)
	message(STATUS "OpenMW source: local openmw-vr (${OPENMW_LOCAL_SOURCE_DIR})")
	ExternalProject_Add(openmw
<<<<<<< HEAD
		DEPENDS ${OPENMW_DEPENDENCIES}
=======
		DEPENDS boost openal osg mygui ffmpeg sdl2 bullet lz4 icu_host luajit sqlite3 fmt
>>>>>>> 3ecc687e950b13580a4e709d17a2dd7170894a4e

		SOURCE_DIR ${OPENMW_LOCAL_SOURCE_DIR}
		DOWNLOAD_COMMAND ""
		UPDATE_COMMAND ""
		PATCH_COMMAND ""

		CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
		${COMMON_CMAKE_ARGS}
		${OPENMW_COMMON}
		-DBUILD_OPENMW_VR=ON

		BUILD_COMMAND $(MAKE)

		INSTALL_COMMAND ""
	)
elseif(USE_LOCAL_OPENMW_VR AND NOT ALLOW_OPENMW_UPSTREAM_FALLBACK)
	message(FATAL_ERROR
		"USE_LOCAL_OPENMW_VR is ON but local source was not found at ${OPENMW_LOCAL_SOURCE_DIR}.\n"
		"This project targets OpenMW-VR on Quest and blocks non-VR fallback by default.\n"
		"Set -DALLOW_OPENMW_UPSTREAM_FALLBACK=ON only if you intentionally want a non-VR build."
	)
else()
	message(STATUS "OpenMW source: upstream OpenMW ${OPENMW_VERSION}")
	ExternalProject_Add(openmw
<<<<<<< HEAD
		DEPENDS ${OPENMW_DEPENDENCIES}
=======
		DEPENDS boost openal osg mygui ffmpeg sdl2 bullet lz4 icu_host luajit sqlite3 fmt
>>>>>>> 3ecc687e950b13580a4e709d17a2dd7170894a4e

		URL https://github.com/OpenMW/openmw/archive/${OPENMW_VERSION}.tar.gz
		URL_HASH ${OPENMW_HASH}
		DOWNLOAD_DIR ${download_dir}

		PATCH_COMMAND ${OPENMW_PATCH}

		CONFIGURE_COMMAND ${wrapper_command} cmake <SOURCE_DIR>
		${COMMON_CMAKE_ARGS}
		${OPENMW_COMMON}

		BUILD_COMMAND $(MAKE)

		INSTALL_COMMAND ""
	)
endif()
