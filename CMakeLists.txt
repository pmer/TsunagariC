cmake_minimum_required(VERSION 3.13)
project(TsunagariEngine
    DESCRIPTION "Tile-based video game engine"
    HOMEPAGE_URL "https://github.com/pmer/TsunagariC"
    LANGUAGES CXX
)


#
# Options
#

option(USE_PKGCONFIG "Use pkg-config to find dependencies" ON)

option(AV_NULL "Disable audio and video output")
option(AV_SDL2 "Use SDL2 for audio and video output")
option(AV_GOSU "Use Gosu for audio and video output")

if(NOT AV_SDL2 AND NOT AV_GOSU)
    set(AV_NULL ON)
endif()

option(BUILD_SHARED_LIBS "Build Tsunagari as a shared library")


#
# Variables
#

# Whether this is a subproject in a larger project.
get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)
if("${SUBPROJECT}" STREQUAL "")
    set(SUBPROJECT 1)
else()
    set(SUBPROJECT 1)
endif()

# Compiler
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set(CLANG 1)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(GCC 1)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    set(MSVC 1)
endif()

# OS
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(LINUX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    set(FREEBSD 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES NetBSD)
    set(NETBSD 1)
endif()

# Build type
set(IS_RELEASE
    "$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>")


#
# Targets
#

add_library(tsunagari)
add_executable(null-world)
add_executable(pack-tool)


#
# Sources in deps
#

target_include_directories(tsunagari
    PUBLIC deps/rapidjson/include
)


#
# Sources in src
#

target_include_directories(tsunagari PUBLIC src)
target_include_directories(null-world PUBLIC src)
target_include_directories(pack-tool PRIVATE src)

target_sources(tsunagari
    PUBLIC  src/config.h
    PRIVATE src/main.cpp
)

target_sources(null-world
    PRIVATE src/null-world.cpp
    PRIVATE src/null-world.h
)

if(AV_NULL)
    target_sources(tsunagari
        PRIVATE src/av/null/images.cpp
                src/av/null/music.cpp
                src/av/null/sounds.cpp
                src/av/null/window.cpp
    )
endif()

if(AV_SDL2)
    target_sources(tsunagari
        PRIVATE src/av/sdl2/error.cpp
                src/av/sdl2/error.h
                src/av/sdl2/images.cpp
                src/av/sdl2/images.h
                src/av/sdl2/music.cpp
                src/av/sdl2/music.h
                src/av/sdl2/sounds.cpp
                src/av/sdl2/sounds.h
                src/av/sdl2/window.cpp
                src/av/sdl2/window.h
    )
endif()

if(AV_GOSU)
    target_sources(tsunagari
        PRIVATE src/av/gosu/cbuffer.cpp
                src/av/gosu/cbuffer.h
                src/av/gosu/images.cpp
                src/av/gosu/images.h
                src/av/gosu/music.cpp
                src/av/gosu/music.h
                src/av/gosu/sounds.cpp
                src/av/gosu/sounds.h
                src/av/gosu/window.cpp
                src/av/gosu/window.h
    )
endif()

target_sources(tsunagari
    PUBLIC src/cache/cache-template.h
    PUBLIC src/cache/cache.h
    PUBLIC src/cache/readercache.h
)

target_sources(tsunagari
    PUBLIC  src/core/algorithm.h
    PRIVATE src/core/animation.cpp
    PUBLIC  src/core/animation.h
    PRIVATE src/core/area.cpp
    PUBLIC  src/core/area.h
    PRIVATE src/core/area-json.cpp
    PUBLIC  src/core/area-json.h
    PRIVATE src/core/character.cpp
    PUBLIC  src/core/character.h
    PRIVATE src/core/client-conf.cpp
    PUBLIC  src/core/client-conf.h
    PRIVATE src/core/cooldown.cpp
    PUBLIC  src/core/cooldown.h
    PRIVATE src/core/display-list.cpp
    PRIVATE src/core/display-list.h
    PRIVATE src/core/entity.cpp
    PUBLIC  src/core/entity.h
    PRIVATE src/core/images.cpp
    PUBLIC  src/core/images.h
    PRIVATE src/core/jsons.cpp
    PUBLIC  src/core/jsons.h
    PRIVATE src/core/log.cpp
    PUBLIC  src/core/log.h
    PRIVATE src/core/measure.cpp
    PUBLIC  src/core/measure.h
    PRIVATE src/core/music.cpp
    PUBLIC  src/core/music-worker.h
    PUBLIC  src/core/music.h
    PRIVATE src/core/npc.cpp
    PUBLIC  src/core/npc.h
    PRIVATE src/core/overlay.cpp
    PUBLIC  src/core/overlay.h
    PRIVATE src/core/player.cpp
    PUBLIC  src/core/player.h
    PUBLIC  src/core/resources.h
    PUBLIC  src/core/sounds.h
    PRIVATE src/core/tile.cpp
    PUBLIC  src/core/tile.h
    PRIVATE src/core/tile-grid.cpp
    PUBLIC  src/core/tile-grid.h
    PRIVATE src/core/viewport.cpp
    PUBLIC  src/core/viewport.h
    PRIVATE src/core/window.cpp
    PUBLIC  src/core/window.h
    PRIVATE src/core/world.cpp
    PUBLIC  src/core/world.h
)

target_sources(tsunagari
    PRIVATE src/data/data-area.cpp
    PUBLIC  src/data/data-area.h
    PRIVATE src/data/data-world.cpp
    PUBLIC  src/data/data-world.h
    PRIVATE src/data/inprogress.cpp
    PUBLIC  src/data/inprogress.h
)

target_sources(tsunagari
    PUBLIC  src/os/c.h
    PUBLIC  src/os/chrono.h
    PUBLIC  src/os/mapped-file.h
    PUBLIC  src/os/mutex.h
    PUBLIC  src/os/thread.h
    PUBLIC  src/os/os.h
)
target_sources(pack-tool
    PRIVATE src/os/c.h
    PRIVATE src/os/condition-variable.h
    PRIVATE src/os/mapped-file.h
    PRIVATE src/os/mutex.h
    PRIVATE src/os/os.h
    PRIVATE src/os/thread.h
)

if (WIN32)
    target_sources(tsunagari
        PRIVATE src/os/memmove.cpp
        PRIVATE src/os/windows-c.cpp
        PUBLIC  src/os/windows-c.h
        PRIVATE src/os/windows-chrono.cpp
        PRIVATE src/os/windows-mapped-file.cpp
        PUBLIC  src/os/windows-mapped-file.h
        PRIVATE src/os/windows.cpp
        PUBLIC  src/os/windows.h
    )
    target_sources(pack-tool
        PRIVATE src/os/memmove.cpp
        PRIVATE src/os/windows-c.cpp
        PRIVATE src/os/windows-c.h
        PRIVATE src/os/windows-condition-variable.h
        PRIVATE src/os/windows-mapped-file.cpp
        PRIVATE src/os/windows-mapped-file.h
        PRIVATE src/os/windows-mutex.h
        PRIVATE src/os/windows-thread.h
        PRIVATE src/os/windows.cpp
        PRIVATE src/os/windows.h
    )
elseif(APPLE)
    target_sources(tsunagari
        PUBLIC  src/os/mac-c.h
        PRIVATE src/os/mac-chrono.cpp
        PRIVATE src/os/mac-gui.mm
        PUBLIC  src/os/mac-gui.h
        PUBLIC  src/os/mac-thread.h
        PRIVATE src/os/unix-mapped-file.cpp
        PUBLIC  src/os/unix-mapped-file.h
        PUBLIC  src/os/unix-mutex.h
        PRIVATE src/os/unix.cpp
    )
    target_sources(pack-tool
        PRIVATE src/os/mac-c.h
        PRIVATE src/os/mac-thread.h
        PRIVATE src/os/unix-condition-variable.h
        PRIVATE src/os/unix-mapped-file.cpp
        PRIVATE src/os/unix-mapped-file.h
        PRIVATE src/os/unix-mutex.h
        PRIVATE src/os/unix.cpp
    )
else()
    target_sources(tsunagari
        PUBLIC  src/os/linux-c.h
        PRIVATE src/os/unix-chrono.cpp
        PRIVATE src/os/unix-mapped-file.cpp
        PUBLIC  src/os/unix-mutex.h
        PUBLIC  src/os/unix-thread.h
        PRIVATE src/os/unix.cpp
    )
    target_sources(pack-tool
        PRIVATE src/os/linux-c.h
        PRIVATE src/os/unix-condition-variable.h
        PRIVATE src/os/unix-mapped-file.cpp
        PRIVATE src/os/unix-mapped-file.h
        PRIVATE src/os/unix-mutex.h
        PRIVATE src/os/unix-thread.h
        PRIVATE src/os/unix.cpp
    )
endif()

target_sources(tsunagari
    PRIVATE src/pack/file-type.cpp
    PUBLIC  src/pack/file-type.h
    PRIVATE src/pack/pack-reader.cpp
    PUBLIC  src/pack/pack-reader.h
)

target_sources(pack-tool
    PRIVATE src/pack/file-type.cpp
    PRIVATE src/pack/file-type.h
    PRIVATE src/pack/main.cpp
    PRIVATE src/pack/pack-reader.cpp
    PRIVATE src/pack/pack-reader.h
    PRIVATE src/pack/pack-writer.cpp
    PRIVATE src/pack/pack-writer.h
    PRIVATE src/pack/ui-log.cpp
    PRIVATE src/pack/ui.h
    PRIVATE src/pack/walker.cpp
    PRIVATE src/pack/walker.h
)

target_sources(tsunagari
    PRIVATE src/resources/pack.cpp
)

target_sources(tsunagari
    PUBLIC  src/util/algorithm.h
    PUBLIC  src/util/align.h
    PUBLIC  src/util/arc.h
    PRIVATE src/util/assert.cpp
    PUBLIC  src/util/assert.h
    PRIVATE src/util/bitrecord.cpp
    PUBLIC  src/util/bitrecord.h
    PUBLIC  src/util/constexpr.h
    PRIVATE src/util/fnv.cpp
    PUBLIC  src/util/fnv.h
    PUBLIC  src/util/function.h
    PRIVATE src/util/hash.cpp
    PUBLIC  src/util/hash.h
    PUBLIC  src/util/hashtable.h
    PUBLIC  src/util/int.h
    PRIVATE src/util/jobs.cpp
    PUBLIC  src/util/jobs.h
    PUBLIC  src/util/likely.h
    PUBLIC  src/util/list.h
    PUBLIC  src/util/math2.h
    PUBLIC  src/util/meta.h
    PUBLIC  src/util/move.h
    PUBLIC  src/util/new.h
    PUBLIC  src/util/noexcept.h
    PUBLIC  src/util/optional.h
    PRIVATE src/util/random.cpp
    PUBLIC  src/util/random.h
    PUBLIC  src/util/rc.h
    PRIVATE src/util/string-view.cpp
    PUBLIC  src/util/string-view.h
    PRIVATE src/util/string.cpp
    PUBLIC  src/util/string.h
    PRIVATE src/util/string2.cpp
    PUBLIC  src/util/string2.h
    PRIVATE src/util/transform.cpp
    PUBLIC  src/util/transform.h
    PUBLIC  src/util/unique.h
    PUBLIC  src/util/vector.h
)

target_sources(pack-tool
    PRIVATE src/util/algorithm.h
    PRIVATE src/util/align.h
    PRIVATE src/util/assert.cpp
    PRIVATE src/util/assert.h
    PRIVATE src/util/constexpr.h
    PRIVATE src/util/fnv.cpp
    PRIVATE src/util/fnv.h
    PRIVATE src/util/function.h
    PRIVATE src/util/hashtable.h
    PRIVATE src/util/int.h
    PRIVATE src/util/jobs.cpp
    PRIVATE src/util/jobs.h
    PRIVATE src/util/list.h
    PRIVATE src/util/meta.h
    PRIVATE src/util/move.h
    PRIVATE src/util/new.h
    PRIVATE src/util/noexcept.h
    PRIVATE src/util/optional.h
    PRIVATE src/util/sort.h
    PRIVATE src/util/string-view.cpp
    PRIVATE src/util/string-view.h
    PRIVATE src/util/string.cpp
    PRIVATE src/util/string.h
    PRIVATE src/util/vector.h
)


#
# Source groups for IDEs
#

get_target_property(TSUNAGARI_SOURCES tsunagari SOURCES)
get_target_property(NULL_WORLD_SOURCES null-world SOURCES)
get_target_property(PACK_TOOL_SOURCES pack-tool SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES ${TSUNAGARI_SOURCES}
          ${NULL_WORLD_SOURCES}
          ${PACK_TOOL_SOURCES}
)


#
# Compilation options
#

target_compile_features(tsunagari PUBLIC cxx_std_14)
target_compile_features(null-world PUBLIC cxx_std_14)
target_compile_features(pack-tool PUBLIC cxx_std_14)
set_target_properties(tsunagari PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(null-world PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(pack-tool PROPERTIES CXX_EXTENSIONS OFF)

# Disable C++ exceptions
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-exceptions")
elseif(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

# Disable C++ run-time type information
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-rtti")
elseif(MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(APPEND CMAKE_CXX_FLAGS " /GR-")
endif()

# Hide most symbols
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fvisibility=hidden")
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -fvisibility=hidden")
endif()

# Reduce code size
if(CLANG)
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -Oz")
endif()

# Enable floating point optimizations
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -ffast-math")
elseif(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /fp:fast")
endif()

# Disable stack canary
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-stack-protector")
elseif(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /GS-")
endif()

# Optimize at link time
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto=full")
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -flto=full")
    string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -flto=thin")
endif()

target_compile_definitions(tsunagari
    PUBLIC $<$<BOOL:${IS_RELEASE}>:NDEBUG>
)
target_compile_definitions(pack-tool
    PRIVATE $<$<BOOL:${IS_RELEASE}>:NDEBUG>
)

# Share variables with parent.
if(IS_SUBPROJECT)
    set(CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELEASE
        ${CMAKE_CXX_FLAGS_RELEASE}
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        ${CMAKE_CXX_FLAGS_MINSIZEREL}
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)
endif()


#
# Linking and externally-managed dependencies
#

# Optimize at link time
if(CLANG OR GCC)
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto=full")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_MINSIZEREL " -flto=full")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto=thin")
endif()

# Do not link to libc++
#if(APPLE)
#   string(APPEND CMAKE_EXE_LINKER_FLAGS " -nostdlib -lSystem")
#endif()

# Strip symbols
if(APPLE AND CLANG)
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -Wl,-x -Wl,-dead_strip")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_MINSIZEREL " -Wl,-x -Wl,-dead_strip")
endif()

# Share variables with parent.
if(IS_SUBPROJECT)
    set(CMAKE_EXE_LINKER_FLAGS
        ${CMAKE_EXE_LINKER_FLAGS}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        ${CMAKE_EXE_LINKER_FLAGS_RELEASE}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
        ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)
endif()

if(APPLE)
    # FIXME: Change to be target-specific.
    set(GUI_TYPE MACOSX_BUNDLE)

    target_link_libraries(tsunagari
        PUBLIC /System/Library/Frameworks/Cocoa.framework
               /System/Library/Frameworks/Foundation.framework
    )
endif()

if(LINUX)
    # Check if we are compiling against glibc, which needs libpthread, or
	# against musl, which doesn't.
    find_package(Threads)
    if(Threads_FOUND)
        target_link_libraries(tsunagari PUBLIC  Threads::Threads)
        target_link_libraries(pack-tool PRIVATE Threads::Threads)
    endif()
endif()
if(FREEBSD OR NETBSD)
    target_link_libraries(tsunagari PUBLIC pthread)
    target_link_libraries(pack-tool PRIVATE pthread)
endif()

if(AV_SDL2)
    if(USE_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2 REQUIRED SDL2 sdl2)
        pkg_search_module(SDL2_image REQUIRED SDL2_image)
        pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)
    else()
        set(SDL2_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2 headers")
        set(SDL2_image_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2_image headers")
        set(SDL2_mixer_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2_mixer headers")

        set(SDL2_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2 library")
        set(SDL2_image_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2_image library")
        set(SDL2_mixer_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2_mixer library")

        set(SDL2_LIBRARIES "" CACHE STRING
            "Name of SDL2 library")
        set(SDL2_image_LIBRARIES "" CACHE STRING
            "Name with SDL2_image library")
        set(SDL2_mixer_LIBRARIES "" CACHE STRING
            "Name with SDL2_mixer library")
    endif()

    if("${SDL2_LIBRARIES}" STREQUAL "")
        message(SEND_ERROR "When not using pkg-config, please manually set"
            " SDL2, SDL2_image, and SDL2_mixer include dirs and libraries")
    endif()

    target_include_directories(tsunagari
        PRIVATE ${SDL2_INCLUDE_DIRS}
                ${SDL2_image_INCLUDE_DIRS}
                ${SDL2_mixer_INCLUDE_DIRS}
    )

    target_link_directories(tsunagari
        PUBLIC ${SDL2_LIBRARY_DIRS}
               ${SDL2_image_LIBRARY_DIRS}
               ${SDL2_mixer_LIBRARY_DIRS}
    )

    target_link_libraries(tsunagari
        PUBLIC ${SDL2_LIBRARIES}
               ${SDL2_image_LIBRARIES}
               ${SDL2_mixer_LIBRARIES}
    )

    # FIXME
    #if(APPLE)
    #    set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES} /System/Library/Frameworks/Cocoa.framework)
    #endif()
endif()

if(AV_GOSU)
    find_package(Gosu REQUIRED)

    target_include_directories(tsunagari PUBLIC ${GOSU_INCLUDE_DIRS})
    target_link_directories(tsunagari PUBLIC ${GOSU_LIBRARY_DIRS})
    target_link_libraries(tsunagari
        PUBLIC ${GOSU_LIBRARIES}
               ${GOSU_DEPENDENCIES}
    )

    # FIXME
    #if(APPLE)
    #    set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES} /System/Library/Frameworks/Cocoa.framework)
    #endif()
endif()

target_link_libraries(null-world tsunagari)
