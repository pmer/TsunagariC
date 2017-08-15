cmake_minimum_required(VERSION 3.6)

find_package(PkgConfig REQUIRED)
find_package(Threads)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    Threads::Threads
)

if (APPLE)
    set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
        /System/Library/Frameworks/Cocoa.framework
    )
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")

IF(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
ENDIF()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated")

if ($ENV{TRAVIS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRAVIS")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/deps/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)
