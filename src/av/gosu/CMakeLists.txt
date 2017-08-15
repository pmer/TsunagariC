find_package(Gosu REQUIRED)

set(TSUNAGARIC_LIBRARIES ${TSUNAGARIC_LIBRARIES}
    ${GOSU_LIBRARIES}
    ${GOSU_DEPENDENCIES}
)

include_directories(
    ${GOSU_INCLUDE_DIRS}
)

link_directories(
    ${GOSU_LIBRARY_DIRS}
)

set(TSUNAGARIC_SOURCES ${TSUNAGARIC_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/gosu-cbuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-images.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-music.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-sounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gosu-window.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBACKEND_GOSU")
