cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig REQUIRED)
find_package(Gosu REQUIRED)
pkg_search_module(SDL2 REQUIRED SDL2)
pkg_search_module(SDL2_image REQUIRED SDL2_image)
pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)
find_package(Threads)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${GOSU_LIBRARIES}
    ${GOSU_DEPENDENCIES}
    ${SDL2_LIBRARIES}
    ${SDL2_image_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
    Threads::Threads
)

include_directories(
    ${GOSU_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_image_INCLUDE_DIRS}
    ${SDL2_mixer_INCLUDE_DIRS}
)

link_directories(
    ${GOSU_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_image_LIBRARY_DIRS}
    ${SDL2_mixer_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")

IF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
ENDIF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

if ($ENV{TRAVIS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRAVIS")
endif($ENV{TRAVIS})

include(${CMAKE_CURRENT_LIST_DIR}/deps/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
