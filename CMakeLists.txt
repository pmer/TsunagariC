cmake_minimum_required(VERSION 3.10)


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
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/deps/rapidjson/include
)


#
# Sources in src
#

target_include_directories(tsunagari PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(pack-tool PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

target_sources(tsunagari
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/config.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)

if(AV_NULL)
    target_sources(tsunagari
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/av/null/images.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/null/music.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/null/sounds.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/null/window.cpp
    )
endif()

if(AV_SDL2)
    target_sources(tsunagari
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/error.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/error.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/images.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/images.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/music.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/music.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/sounds.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/sounds.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/window.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/sdl2/window.h
    )
endif()

if(AV_GOSU)
    target_sources(tsunagari
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-cbuffer.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-cbuffer.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-images.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-images.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-music.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-music.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-sounds.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-sounds.h
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-window.cpp
                ${CMAKE_CURRENT_LIST_DIR}/src/av/gosu/gosu-window.h
    )
endif()

target_sources(tsunagari
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/cache/cache-template.h
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/cache/cache.h
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/cache/readercache.h
)

target_sources(tsunagari
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/algorithm.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/animation.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/animation.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/area.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/area.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/area-json.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/area-json.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/character.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/character.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/client-conf.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/client-conf.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/cooldown.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/cooldown.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/dtds.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/dtds.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/entity.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/entity.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/formatter.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/formatter.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/images.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/images.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/jsons.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/jsons.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/log.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/log.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/measure.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/measure.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/music-impl.cpp
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/music-worker.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/music-worker.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/music.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/npc.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/npc.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/overlay.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/overlay.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/player.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/player.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/resources.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/resources.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/sounds.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/sounds.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/tile.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/tile.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/tile-grid.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/viewport.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/viewport.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/window.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/window.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/core/world.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/core/world.h
)

target_sources(tsunagari
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/data/data-area.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/data/data-area.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/data/data-world.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/data/data-world.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/data/inprogress.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/data/inprogress.h
)

target_sources(tsunagari
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/nbcl/nbcl.cpp
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/nbcl/nbcl.h
)

if (WIN32)
    target_sources(tsunagari
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/windows.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/windows.h
    )
    target_sources(pack-tool
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/windows.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/windows.h
    )
elseif(APPLE)
    target_sources(tsunagari
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/mac-gui.mm
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/mac-gui.h
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.h
    )
    target_sources(pack-tool
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.h
    )
else()
    target_sources(tsunagari
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.h
    )
    target_sources(pack-tool
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/os.h
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.cpp
        PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/os/unix.h
    )
endif()

target_sources(tsunagari
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/file-type.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/file-type.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/pack-file.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/pack-file.h
)

target_sources(pack-tool
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/file-type.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/file-type.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/main.cpp
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/pack-file.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/pack-file.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/pool.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/pool.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/ui-log.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/ui.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/pack/walker.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/pack/walker.h
)

target_sources(tsunagari
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/resources/pack.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/resources/pack.h
)

target_sources(tsunagari
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/algorithm.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/arc.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/assert.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/assert.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/bitrecord.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/bitrecord.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/dispatch-queue-impl.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/dispatch-queue-impl.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/dispatch-queue.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/dispatch-queue.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/likely.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/math2.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/meta.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/move.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/optional.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/random.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/random.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/rc.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/safe-heap.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/string2.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/string2.h
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/transform.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/transform.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/unique.h
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/vector.h
)

target_sources(pack-tool
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/util/assert.cpp
    PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/src/util/assert.h
)


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

if($ENV{TRAVIS})
    target_compile_definitions(tsunagari PUBLIC  TRAVIS)
    target_compile_definitions(pack-tool PRIVATE TRAVIS)
endif()


#
# Linking and externally-managed dependencies
#

if(APPLE)
    # FIXME: Change to be target-specific.
    set(GUI_TYPE MACOSX_BUNDLE)
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

    target_link_directories(
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
