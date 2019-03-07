cmake_minimum_required(VERSION 3.13)
project(TsunagariEngine
    DESCRIPTION "Tile-based video game engine"
    HOMEPAGE_URL "https://github.com/pmer/TsunagariC"
    LANGUAGES CXX
)


#
# Options
#

option(AV_NULL "Disable audio and video output")
option(AV_SDL2 "Use SDL2 for audio and video output")
option(AV_GOSU "Use Gosu for audio and video output")

if(NOT AV_SDL2 AND NOT AV_GOSU)
    set(AV_NULL ON)
endif()

option(BUILD_SHARED_LIBS "Build Tsunagari as a shared library")


#
# Targets
#

add_library(tsunagari)
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
target_include_directories(pack-tool PUBLIC src)

target_sources(tsunagari
    PUBLIC  src/config.h
    PRIVATE src/main.cpp
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
    PRIVATE src/core/music-impl.cpp
    PRIVATE src/core/music-worker.cpp
    PUBLIC  src/core/music-worker.h
    PUBLIC  src/core/music.h
    PRIVATE src/core/npc.cpp
    PUBLIC  src/core/npc.h
    PRIVATE src/core/overlay.cpp
    PUBLIC  src/core/overlay.h
    PRIVATE src/core/player.cpp
    PUBLIC  src/core/player.h
    PUBLIC  src/core/resources.h
    PRIVATE src/core/sounds.cpp
    PUBLIC  src/core/sounds.h
    PRIVATE src/core/tile.cpp
    PUBLIC  src/core/tile.h
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
    PRIVATE src/nbcl/nbcl.cpp
    PRIVATE src/nbcl/nbcl.h
)

if (WIN32)
    target_sources(tsunagari
        PUBLIC  src/os/os.h
        PRIVATE src/os/windows.cpp
        PUBLIC  src/os/windows.h
    )
    target_sources(pack-tool
        PUBLIC  src/os/os.h
        PRIVATE src/os/windows.cpp
        PUBLIC  src/os/windows.h
    )
elseif(APPLE)
    target_sources(tsunagari
        PRIVATE src/os/mac-gui.mm
        PUBLIC  src/os/mac-gui.h
        PUBLIC  src/os/os.h
        PRIVATE src/os/unix.cpp
        PUBLIC  src/os/unix.h
    )
    target_sources(pack-tool
        PUBLIC  src/os/os.h
        PRIVATE src/os/unix.cpp
        PUBLIC  src/os/unix.h
    )
else()
    target_sources(tsunagari
        PUBLIC  src/os/os.h
        PRIVATE src/os/unix.cpp
        PUBLIC  src/os/unix.h
    )
    target_sources(pack-tool
        PUBLIC  src/os/os.h
        PRIVATE src/os/unix.cpp
        PUBLIC  src/os/unix.h
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
    PUBLIC  src/pack/file-type.h
    PRIVATE src/pack/main.cpp
    PRIVATE src/pack/pack-reader.cpp
    PUBLIC  src/pack/pack-reader.h
    PRIVATE src/pack/pack-writer.cpp
    PUBLIC  src/pack/pack-writer.h
    PRIVATE src/pack/pool.cpp
    PUBLIC  src/pack/pool.h
    PRIVATE src/pack/ui-log.cpp
    PUBLIC  src/pack/ui.h
    PRIVATE src/pack/walker.cpp
    PUBLIC  src/pack/walker.h
)

target_sources(tsunagari
    PRIVATE src/resources/pack.cpp
)

target_sources(tsunagari
    PUBLIC  src/util/algorithm.h
    PUBLIC  src/util/arc.h
    PRIVATE src/util/assert.cpp
    PUBLIC  src/util/assert.h
    PRIVATE src/util/bitrecord.cpp
    PUBLIC  src/util/bitrecord.h
    PRIVATE src/util/dispatch-queue-impl.cpp
    PUBLIC  src/util/dispatch-queue-impl.h
    PRIVATE src/util/dispatch-queue.cpp
    PUBLIC  src/util/dispatch-queue.h
    PRIVATE src/util/fnv.cpp
    PUBLIC  src/util/fnv.h
    PUBLIC  src/util/hashtable.h
    PUBLIC  src/util/likely.h
    PUBLIC  src/util/list.h
    PUBLIC  src/util/math2.h
    PUBLIC  src/util/meta.h
    PUBLIC  src/util/move.h
    PUBLIC  src/util/new.h
    PUBLIC  src/util/optional.h
    PRIVATE src/util/random.cpp
    PUBLIC  src/util/random.h
    PUBLIC  src/util/rc.h
    PUBLIC  src/util/safe-heap.h
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
    PRIVATE src/util/assert.cpp
    PRIVATE src/util/assert.h
    PRIVATE src/util/fnv.cpp
    PRIVATE src/util/fnv.h
    PRIVATE src/util/hashtable.h
    PRIVATE src/util/list.h
    PRIVATE src/util/math.h
    PRIVATE src/util/meta.h
    PRIVATE src/util/move.h
    PRIVATE src/util/new.h
    PRIVATE src/util/optional.h
    PRIVATE src/util/sort.h
    PRIVATE src/util/string-view.cpp
    PUBLIC  src/util/string-view.h
    PRIVATE src/util/string.cpp
    PUBLIC  src/util/string.h
    PRIVATE src/util/vector.h
)


#
# Source groups for IDEs
#

get_target_property(TSUNAGARI_SOURCES tsunagari SOURCES)
get_target_property(PACK_TOOL_SOURCES pack-tool SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${TSUNAGARI_SOURCES} ${PACK_TOOL_SOURCES})


#
# Compilation options
#

target_compile_features(tsunagari PUBLIC cxx_std_14)
set_target_properties(tsunagari PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(pack-tool PUBLIC cxx_std_14)
set_target_properties(pack-tool PROPERTIES CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set_target_properties(tsunagari PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_target_properties(pack-tool PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()


#
# Linking and externally-managed dependencies
#

if(APPLE)
    # FIXME: Change to be target-specific.
    set(GUI_TYPE MACOSX_BUNDLE)
endif()

find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(tsunagari Threads::Threads)
    target_link_libraries(pack-tool Threads::Threads)
endif()

if(AV_SDL2)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(SDL2 REQUIRED SDL2 sdl2)
    pkg_search_module(SDL2_image REQUIRED SDL2_image)
    pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)

    target_include_directories(tsunagari
        PRIVATE ${SDL2_INCLUDE_DIRS}
                ${SDL2_image_INCLUDE_DIRS}
                ${SDL2_mixer_INCLUDE_DIRS}
    )

    target_link_directories(tsunagari
        PRIVATE ${SDL2_LIBRARY_DIRS}
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
    target_link_libraries(tsunagari PUBLIC ${GOSU_LIBRARIES} ${GOSU_DEPENDENCIES})

    # FIXME
    #if(APPLE)
    #    set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES} /System/Library/Frameworks/Cocoa.framework)
    #endif()
endif()
