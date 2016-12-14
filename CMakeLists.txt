cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig REQUIRED)
find_package(gosu REQUIRED)
find_package(libxml2 REQUIRED)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${GOSU_LIBRARIES}
    ${GOSU_DEPENDENCIES}
    ${LIBXML2_LIBRARIES}
    physfs
)

include_directories(
    ${GOSU_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIR}
)

link_directories(
    ${GOSU_LIBRARY_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

include(${CMAKE_CURRENT_LIST_DIR}/external/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
