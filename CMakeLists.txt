cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig REQUIRED)
find_package(Gosu REQUIRED)
find_package(SDL2 REQUIRED)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${GOSU_LIBRARIES}
    ${GOSU_DEPENDENCIES}
    ${SDL2_LIBRARIES}
)

include_directories(
    ${GOSU_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

link_directories(
    ${GOSU_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

if ($ENV{TRAVIS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRAVIS")
endif($ENV{TRAVIS})

include(${CMAKE_CURRENT_LIST_DIR}/deps/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
